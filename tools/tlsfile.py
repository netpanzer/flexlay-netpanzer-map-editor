#!/usr/bin/env python3
"""Shared .tls reading/quantizing helpers for the tools in this directory.

The binary layout mirrors src/npmformat.h — see docs/map-format.md.

Quantization uses netp.act when it can be found by walking up from the .tls
file, because that is the palette the editor and the game render with; the
palette embedded in the .tls is only a fallback. Colours are matched against
the terrain portion of the palette (indices 0-84) since indices 85+ are UI
and special colours that must not appear in terrain art.
"""

from collections import Counter
from pathlib import Path
import struct

import numpy as np

TLS_OFF_TILE_COUNT  = 70
TLS_OFF_PALETTE     = 72    # 768 bytes = 256 x 3 RGB
TLS_OFF_HEADERS     = 840   # 72-byte fixed header + 768-byte palette
TILE_PIXELS         = 32 * 32
TERRAIN_PALETTE_END = 85    # indices 0-84 are terrain; 85+ are UI/special


def tile_count(data: bytes) -> int:
    return struct.unpack_from("<H", data, TLS_OFF_TILE_COUNT)[0]


def pixel_base(data: bytes) -> int:
    """Byte offset of the first tile's pixel data (all tile headers precede it)."""
    return TLS_OFF_HEADERS + tile_count(data) * 3


def tile_pixels(data: bytes, tile_id: int) -> bytes:
    base = pixel_base(data) + tile_id * TILE_PIXELS
    return bytes(data[base : base + TILE_PIXELS])


def load_act_palette(start: Path):
    """Walk up from `start` looking for netp.act. Returns (256, 3) or None."""
    d = start if start.is_dir() else start.parent
    for _ in range(6):
        candidate = d / "netp.act"
        if candidate.exists():
            return np.frombuffer(
                candidate.read_bytes()[:768], dtype=np.uint8
            ).reshape(256, 3)
        d = d.parent
    return None


def resolve_palette(tls_data: bytes, tls_path: Path = None) -> np.ndarray:
    """netp.act if findable, else the palette embedded in the .tls."""
    if tls_path is not None:
        act_pal = load_act_palette(tls_path)
        if act_pal is not None:
            return act_pal
    return np.frombuffer(
        bytes(tls_data[TLS_OFF_PALETTE : TLS_OFF_PALETTE + 768]), dtype=np.uint8
    ).reshape(256, 3)


def indices_used_by(data: bytes, tile_ids) -> list:
    """Sorted palette indices appearing in the given tiles, for blend-in edits."""
    count = tile_count(data)
    used = set()
    for tid in tile_ids:
        if 0 <= tid < count:
            used.update(tile_pixels(data, tid))
    return sorted(used)


def quantize(rgb: np.ndarray, palette: np.ndarray, allowed_indices=None) -> bytes:
    """Map an (H, W, 3) or (N, 3) RGB array to nearest palette indices."""
    if not allowed_indices:
        allowed_indices = list(range(TERRAIN_PALETTE_END))
    sub_pal = palette[allowed_indices].astype(np.int32)
    pixels  = np.asarray(rgb, dtype=np.int32).reshape(-1, 3)
    diff    = pixels[:, np.newaxis, :] - sub_pal[np.newaxis, :, :]
    nearest = (diff * diff).sum(axis=2).argmin(axis=1)
    return bytes(np.array(allowed_indices, dtype=np.uint8)[nearest].tobytes())


def quantize_image(img, palette: np.ndarray, allowed_indices=None) -> bytes:
    return quantize(np.array(img.convert("RGB")), palette, allowed_indices)


def dominant_index(pixels: bytes) -> int:
    """Most common palette index — what the .tls stores as the tile's avg_color."""
    return Counter(pixels).most_common(1)[0][0]


def append_tile(data: bytearray, pixels: bytes,
                attrib: int = 0, move_value: int = 0) -> int:
    """Append a tile, fixing up the count. Returns the new tile's 0-based id.

    The new header must be spliced in after the existing headers rather than
    at the end, since all headers precede all pixel data.
    """
    count     = tile_count(data)
    px_offset = TLS_OFF_HEADERS + count * 3
    header    = bytes([attrib & 0xFF, move_value & 0xFF, dominant_index(pixels)])
    new_data  = bytearray(
        bytes(data[:px_offset]) + header + bytes(data[px_offset:]) + pixels
    )
    struct.pack_into("<H", new_data, TLS_OFF_TILE_COUNT, count + 1)
    data[:] = new_data
    return count
