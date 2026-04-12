# Build Instructions

## Dependencies

**Ubuntu/Debian:**
```sh
sudo apt install build-essential meson ninja-build qtbase5-dev qtbase5-dev-tools pkg-config
```

**Fedora:**
```sh
sudo dnf install gcc-c++ meson ninja-build qt5-qtbase-devel pkg-config
```

**Arch:**
```sh
sudo pacman -S base-devel meson qt5-base
```

## Building

```sh
meson setup build
ninja -C build
```

## Running

```sh
NETPANZER_DATADIR=/path/to/netpanzer/data ./build/netpanzer-editor
```

`NETPANZER_DATADIR` must point to the netPanzer game data directory containing
`maps/` and `wads/`. This is typically installed with the netpanzer game package
or built from the netpanzer source.

## Testing

```sh
meson test -C build --print-errorlogs
```

## AppImage via Docker

The easiest way to produce a self-contained AppImage is via the
`andy5995/linuxdeploy:v3-jammy` container (available on Docker Hub),
which has all build tools and linuxdeploy pre-installed:

```sh
VERSION=0.1 docker compose up
```

The resulting AppImage is written to `out/`.

> [!CAUTION]
> The AppImage does **not** bundle netPanzer game data. You must set
> `NETPANZER_DATADIR` before launching:
>
> ```sh
> NETPANZER_DATADIR=/path/to/netpanzer/data ./out/netpanzer-editor-0.1-x86_64.AppImage
> ```

### Testing changes to `make-appimage.sh`

Use `docker-compose.dev.yml` to get an interactive shell inside the container:

```sh
docker compose -f docker-compose.dev.yml run --rm build
```

From there you can run `./make-appimage.sh` directly or step through it manually.

## AddressSanitizer

The Meson build enables ASan/UBSan by default (configured in `meson.build`).
Known third-party leaks from fontconfig/Pango/GTK3 are suppressed via `asan.supp`.
`meson test` picks up the suppressions automatically via `LSAN_OPTIONS`.

To run the editor with suppression:

```sh
LSAN_OPTIONS=suppressions=$(pwd)/asan.supp ./build/netpanzer-editor
```
