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

**macOS (Homebrew):**
```sh
brew install meson ninja qt@5
```

## Building

**Linux / macOS:**
```sh
meson setup build
ninja -C build
```

On macOS, if meson cannot find Qt5, add it to your PATH first:
```sh
export PATH="$(brew --prefix qt@5)/bin:$PATH"
```

**MSYS2 (MINGW64):**
```sh
meson setup build
ninja -C build
```

**MSVC (from a Developer Command Prompt):**
```sh
meson setup build --backend=ninja
ninja -C build
```

## Running

```sh
./build/netpanzer-editor
```

Use **File → Open** to open a `.npm` map file. The editor locates the tileset
relative to the map file automatically.

## Testing

```sh
meson test -C build --print-errorlogs
```

## AddressSanitizer

ASan/UBSan are **not** enabled by default. To opt in:

```sh
meson setup build -Db_sanitize=address,undefined
ninja -C build
```

Known third-party leaks from fontconfig/Pango/GTK3 are suppressed via `lsan.supp`.
`meson test` picks up the suppressions automatically via `LSAN_OPTIONS`.

To run the editor with suppression active:

```sh
LSAN_OPTIONS=suppressions=$(pwd)/lsan.supp ./build/netpanzer-editor
```

## AppImage via container

The easiest way to produce a self-contained AppImage is via the
`andy5995/linuxdeploy:v3-jammy` container (available on Docker Hub),
which has all build tools and linuxdeploy pre-installed.

All container assets live under `appimage/`.

### Setup

`VERSION` controls the AppImage filename (e.g.
`netpanzer-editor-0.1-x86_64.AppImage`) and must be set. Copy
`appimage/.env.example` to `appimage/.env` and edit, or just export
`VERSION` in your shell before running compose.

The container auto-detects host UID/GID from the bind-mounted
workspace; `HOSTUID`/`HOSTGID` are exposed in the compose file as
opt-in overrides (uncomment in `.env` if you need to override).

### Building

Run from the repo root:

```sh
docker compose -f appimage/docker-compose.yml run --rm build
```

The resulting AppImage is written to `out/`.

Launch the AppImage and use **File → Open** to open a `.npm` map file.

### Interactive shell in the container

For poking around the container or stepping through `make-appimage.sh`
manually:

```sh
docker compose -f appimage/docker-compose.yml run --rm --entrypoint bash build -i
```

The trailing `-i` overrides the compose command so `bash` starts an
interactive shell instead of running the build script. From there you
can run `./appimage/make-appimage.sh` directly.
