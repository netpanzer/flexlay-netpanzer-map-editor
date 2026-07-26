#!/usr/bin/env python3
"""Generate river-to-water transition tiles for all summer12mb tileset variants.

This script is specific to the summer12mb tileset family.  It blends the three
river edge tiles (IDs 9467–9469) into the matching deep-water tiles (IDs
10967–10969) across the full tile height, then appends the results as new tiles
to each variant's .tls file.

The generated tile IDs are:
  Desert    — 11960, 11961, 11962  (Desert base count is 11960)
  all others — 11961, 11962, 11963

Usage:
    python3 tools/gen_river_water_transitions.py --wads /path/to/netpanzer/data/wads

A .tls.bak backup is written beside each .tls before any changes are made.
"""

import argparse
import pathlib
import struct

import numpy as np

import tlsfile
from tlsfile import TILE_PIXELS, TLS_OFF_HEADERS, TLS_OFF_TILE_COUNT

RIVER_IDS  = [9467, 9468, 9469]
WATER_IDS  = [10967, 10968, 10969]
# Original tile counts before any transition tiles were added
BASE_COUNTS  = {'Desert': 11960}   # Desert is missing the placeholder at 11960
DEFAULT_BASE = 11961
# All variants should reach 11961 before transition tiles are added.
# Desert is missing the empty placeholder tile at 11960; we insert it here
# so transition tiles land at 11961-11963 across all variants.
NOTILE_PIXELS = bytes([1] * TILE_PIXELS)  # near-black placeholder, matches other variants


def extract_tile_rgb(data: bytes, tid: int, act_pal: np.ndarray) -> np.ndarray:
    raw = np.frombuffer(tlsfile.tile_pixels(data, tid), dtype=np.uint8)
    return act_pal[raw].reshape(32, 32, 3).astype(np.float32)


def blend_transition(river: np.ndarray, water: np.ndarray) -> np.ndarray:
    """Linear blend from river (top) to water (bottom) across the full tile."""
    h = river.shape[0]
    out = river.copy().astype(np.float32)
    for row in range(h):
        t = row / (h - 1)
        out[row] = (1.0 - t) * river[row] + t * water[row]
    return np.clip(out, 0, 255).astype(np.uint8)


def strip_to(data: bytearray, target: int):
    count = tlsfile.tile_count(data)
    if count <= target:
        return
    px_base  = TLS_OFF_HEADERS + count * 3
    new_data = bytearray(
        bytes(data[:TLS_OFF_HEADERS])
        + bytes(data[TLS_OFF_HEADERS : TLS_OFF_HEADERS + target * 3])
        + bytes(data[px_base : px_base + target * TILE_PIXELS])
    )
    struct.pack_into('<H', new_data, TLS_OFF_TILE_COUNT, target)
    data[:] = new_data


def process_variant(tls_path: pathlib.Path, act_pal: np.ndarray):
    name = tls_path.parent.name
    data = bytearray(tls_path.read_bytes())
    count = tlsfile.tile_count(data)

    base = BASE_COUNTS.get(name, DEFAULT_BASE)
    if count > base:
        strip_to(data, base)
        count = base

    # Ensure all variants are at DEFAULT_BASE before adding transition tiles
    if count < DEFAULT_BASE:
        tlsfile.append_tile(data, NOTILE_PIXELS)
        count = tlsfile.tile_count(data)

    # Palette indices used by the river and water tiles — keeps new tiles
    # visually consistent with their neighbours
    allowed_list = tlsfile.indices_used_by(data, RIVER_IDS + WATER_IDS)

    new_ids = []
    for r_id, w_id in zip(RIVER_IDS, WATER_IDS):
        if r_id >= count or w_id >= count:
            print(f'  {name}: skipping {r_id}/{w_id} — out of range')
            continue
        river = extract_tile_rgb(bytes(data), r_id, act_pal)
        water = extract_tile_rgb(bytes(data), w_id, act_pal)
        blended = blend_transition(river, water)
        pixels  = tlsfile.quantize(blended, act_pal, allowed_list)
        new_ids.append(tlsfile.append_tile(data, pixels))

    tls_path.with_suffix('.tls.bak').write_bytes(tls_path.read_bytes())
    tls_path.write_bytes(data)
    print(f'  {name}: transition tiles {new_ids}')


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--wads', required=True,
                    help='Path to netpanzer data/wads directory')
    args = ap.parse_args()

    wads = pathlib.Path(args.wads)
    act_pal = tlsfile.load_act_palette(wads)
    if act_pal is None:
        ap.error(f'netp.act not found in or above {wads}')

    variants = sorted((wads / 'summer12mb').glob('*/summer12mb.tls'))
    if not variants:
        ap.error(f'No summer12mb variants found under {wads}/summer12mb/')

    print(f'Processing {len(variants)} variants...')
    for tls_path in variants:
        process_variant(tls_path, act_pal)
    print('Done.')


if __name__ == '__main__':
    main()
