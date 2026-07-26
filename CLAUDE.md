# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build / Test / Run

```sh
meson setup build
ninja -C build
meson test -C build --print-errorlogs   # runs both test_maploader and test_tlsloader
./build/netpanzer-editor
```

Run a single test executable directly (Qt Test supports filtering):

```sh
./build/test_maploader                       # all 14 cases
./build/test_maploader testRoundTrip         # one slot
./build/test_tlsloader -platform offscreen   # tlsloader needs an offscreen QPA
```

Sanitizer build (off by default — third-party leaks suppressed via `lsan.supp`):

```sh
meson setup build_asan -Db_sanitize=address,undefined
ninja -C build_asan
```

AppImage build is containerized via `docker compose -f appimage/docker-compose.yml run --rm build` (see BUILD.md). All docker assets live under `appimage/`.

## Architecture

This is a Qt5 + Meson C++17 port of an older ClanLib + SCons + Ruby editor. The original Flexlay code is preserved in attribution only; everything in `src/` is the new port.

### Map format ↔ in-memory model

A netPanzer map is up to three sibling files sharing a base name: binary `.npm` (header + `uint16` tile grid + optional palette-indexed thumbnail), plain-text `.opt` (outposts) and `.spn` (spawn points). All multi-byte ints are little-endian. The full binary layout is in `docs/map-format.md`; the offsets are mirrored in `src/npmformat.h` and any change to one must match the other. `.spn` files in the wild sometimes omit newlines and concatenate tokens — the parser normalises by inserting a newline before each `Location:` token, so don't "fix" that as a bug.

`src/objects.h` defines the model: `Map` holds tiles as `QVector<uint16_t>` plus metadata; `ObjectRef` (type, name, x, y) covers both outposts and spawn points. `maploader.{h,cpp}` is the only code that touches the on-disk binary format — round-trip is verified by `tests/test_maploader.cpp`.

### Tileset rendering

`.tls` files embed a 256-colour palette, but the editor **does not** use it — it walks parent directories of the loaded `.tls` looking for `netp.act` (Adobe Color Table) so colours match netPanzer's runtime. `tlsloader.{h,cpp}` builds a QImage atlas plus a per-tile QImage cache. Tests require `QT_QPA_PLATFORM=offscreen` because `Tileset` constructs `QImage`s.

### Tool / command model

`MapView` (`src/mapview.{h,cpp}`) owns a `Tool` enum (TilePaint, Ellipse, RectSelect/Fill/Outline, StampPaint, Pick, PlaceOutpost, PlaceSpawnpoint, SelectObject). Edits go through polymorphic `Command` subclasses in `src/commands.h` (TileBatch, AddObject, RemoveObject, MoveObject, RenameObject) pushed onto an undo stack — drag-paint strokes batch into a single TileBatch so undo restores the whole stroke.

### Qt build wiring

`meson.build` lists MOC headers and qresources explicitly under `qt5.preprocess(...)` — when you add a new `Q_OBJECT` class, also add its header to the `moc_headers` array or the link will fail with missing vtable / staticMetaObject symbols. `tests/test_maploader.cpp` uses `QTEST_MAIN` which expands to `Q_OBJECT`, so it's preprocessed via `moc_sources` (note: `moc_sources`, not `moc_headers`) and the generated `.moc` is `#include`d at the bottom of the test file.

### Stamps

`data/stamps/*.stamp.json` is auto-loaded at startup; users can also save selections to arbitrary paths. The `StampWidget::addStamp` path was previously a footgun — adding a stamp must auto-select it and emit `stampSelected`, otherwise the subsequent stamp-paint silently no-ops.

### Game data location

When testing against real maps/tilesets, the reference install is at `/home/andy/src/netpanzer/data/` — maps in `maps/`, tileset at `wads/summer12mb/SummerDay/summer12mb.tls`, palette at `wads/netp.act`.

## Things worth knowing

- The Qt port is the only living branch — don't resurrect ClanLib/SCons/Ruby code from history.
- `appimage/make-appimage.sh` runs inside `andy5995/linuxdeploy:v3-jammy`; the host doesn't need linuxdeploy.
- Generated `config.h` lives in the build dir; `meson.build` exposes it via the `build_inc` include directory — include as `<config.h>`, not a relative path.
- `lsan.supp` is wired into both test targets via `LSAN_OPTIONS=suppressions=…` in `meson.build`; don't duplicate it elsewhere.
- The dock layout and window geometry persist via `QSettings` (`mainWindow/state`, `mainWindow/geometry`), saved in `MainWindow::closeEvent`. The docks' `objectName`s (`tileBrowserDock`, `stampPanelDock`, `minimapDock`) are load-bearing: `restoreState()` matches on them and silently skips any dock without one, so renaming an objectName quietly breaks layout restore for every existing user. Docks are deliberately not floatable and are confined to the left/right areas.
- Icons are generated from `packaging/netpanzer-editor.svg` (detailed, 48-256) and `packaging/netpanzer-editor-small.svg` (simplified, 16/32) by `packaging/make-icons.sh`. The `.png`/`.ico` outputs are committed because CI has no rsvg-convert — regenerate both masters' outputs together after editing either SVG.
- `tools/tlsfile.py` holds the shared `.tls` offsets, palette resolution and quantizer for the Python tile tools — add to it rather than re-deriving offsets in a new script. Tilesets can carry trailing bytes past the last tile, so splice new pixel data at the end of the *pixel data*, not the end of the file.
