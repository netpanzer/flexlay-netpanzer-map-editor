#!/usr/bin/env bash
# Regenerate the icon assets from the committed SVG masters.
# Requires rsvg-convert (librsvg) and python3 with Pillow.
#
#   bash packaging/make-icons.sh
#
# Two masters on purpose: the detailed artwork is unreadable below ~48px, so
# the 16 and 32 frames use a simplified mark instead. Every PNG is rendered
# natively from SVG rather than downscaled, so strokes snap to the pixel grid.

set -euo pipefail
cd "$(dirname "$0")/.."

BIG=packaging/netpanzer-editor.svg
SMALL=packaging/netpanzer-editor-small.svg
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

for sz in 48 64 128 256; do rsvg-convert -w $sz -h $sz "$BIG"   -o "$TMP/i-$sz.png"; done
for sz in 16 32;         do rsvg-convert -w $sz -h $sz "$SMALL" -o "$TMP/i-$sz.png"; done

# Linux .desktop / AppImage icon, and the 64px copy embedded via resources.qrc
cp "$TMP/i-256.png" data/images/netpanzer-editor.png
cp "$TMP/i-64.png"  data/images/netpanzer-editor-64.png

# Windows .ico. The 256 frame is stored PNG-encoded: Inno Setup embeds
# SetupIconFile frames into setup.exe uncompressed, so a raw BGRA 256 frame
# would add ~270 KB to the installer.
python3 - "$TMP" <<'PY'
import struct, sys
from pathlib import Path
tmp = Path(sys.argv[1])
sizes = [16, 32, 48, 64, 128, 256]
frames = []
for sz in sizes:
    data = (tmp / f"i-{sz}.png").read_bytes()
    frames.append((sz, data))
out = bytearray(struct.pack("<HHH", 0, 1, len(frames)))
off = 6 + 16 * len(frames)
body = bytearray()
for sz, data in frames:
    out += struct.pack("<BBBBHHII", sz & 0xFF, sz & 0xFF, 0, 0, 1, 32, len(data), off + len(body))
    body += data
Path("packaging/windows/netpanzer-editor.ico").write_bytes(bytes(out) + bytes(body))
print("wrote packaging/windows/netpanzer-editor.ico")
PY
echo "Done."
