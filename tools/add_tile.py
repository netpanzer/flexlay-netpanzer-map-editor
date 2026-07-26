#!/usr/bin/env python3
"""Append a 32×32 PNG as a new tile in a netPanzer .tls file.

The PNG must be 32×32 pixels.  It may be RGB, RGBA, or palette-indexed (mode P).
The script quantizes the image to the tileset's palette, so no manual
indexed-mode conversion is needed — exporting as plain RGB from GIMP works.

Usage:
    python3 tools/add_tile.py --tls path/to/summer12mb.tls --tile mytile.png

The new tile is appended at the end; its tile ID will be the old tile count.
A .bak backup of the original file is written before any changes are made.
"""

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow and numpy are required:  pip install Pillow numpy")

import tlsfile
from tlsfile import TILE_PIXELS, TLS_OFF_HEADERS


def load_tile_pixels(png_path: Path, tls_data: bytes, tls_path: Path = None,
                     neighbor_ids: list = None) -> bytes:
    img = Image.open(png_path)
    if img.size != (32, 32):
        sys.exit(f"Tile must be 32×32 pixels, got {img.size[0]}×{img.size[1]}")
    palette = tlsfile.resolve_palette(tls_data, tls_path)
    allowed = tlsfile.indices_used_by(tls_data, neighbor_ids) if neighbor_ids else None
    return tlsfile.quantize_image(img, palette, allowed)


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("--tls",        required=True,
                    help="Path to the .tls file (modified in-place after backup)")
    ap.add_argument("--tile",       required=True,
                    help="32×32 PNG to append (RGB, RGBA, or indexed)")
    ap.add_argument("--attrib",     type=int, default=0,
                    help="Tile attribute byte — 0=normal, 1=impassable (default 0)")
    ap.add_argument("--move-value", type=int, default=0,
                    help="Movement cost byte (default 0)")
    ap.add_argument("--dry-run",    action="store_true",
                    help="Print what would happen without writing anything")
    args = ap.parse_args()

    tls_path  = Path(args.tls)
    tile_path = Path(args.tile)

    if not tls_path.exists():
        sys.exit(f"Not found: {tls_path}")
    if not tile_path.exists():
        sys.exit(f"Not found: {tile_path}")

    original = tls_path.read_bytes()
    data     = bytearray(original)

    count    = tlsfile.tile_count(data)
    expected = TLS_OFF_HEADERS + count * 3 + count * TILE_PIXELS
    if len(data) < expected:
        sys.exit(
            f"File too short: expected at least {expected} bytes for "
            f"{count} tiles, got {len(data)}"
        )

    pixels  = load_tile_pixels(tile_path, data, tls_path)
    avg_col = tlsfile.dominant_index(pixels)

    print(f"Tileset    : {tls_path}")
    print(f"New tile   : {tile_path}")
    print(f"Tile count : {count} → {count + 1}  (new tile id = {count})")
    print(f"Header     : attrib={args.attrib}  move_value={args.move_value}"
          f"  avg_color={avg_col}")

    if args.dry_run:
        print("Dry run — nothing written.")
        return

    tlsfile.append_tile(data, pixels, args.attrib, args.move_value)

    backup = tls_path.with_suffix(tls_path.suffix + ".bak")
    backup.write_bytes(original)
    print(f"Backup     : {backup}")

    tls_path.write_bytes(data)
    print("Done.")


if __name__ == "__main__":
    main()
