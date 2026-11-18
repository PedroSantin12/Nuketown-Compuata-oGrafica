"""Build the scene's seamless tangent-space normal maps with Python's stdlib.

No dependency is needed at run time. Each height field is periodic, in metres;
central differences wrap across the tile before the normals are encoded as RGB.
The positive-height BMP stores v=0 on its bottom row. Load with
CTextures::LoadTextureAnisotropic(..., true) to keep OpenGL's +V/+Y convention.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import random
import struct


SIZE = 512


def periodic_noise(size: int, cells: int, seed: int) -> list[float]:
    """Smooth periodic value noise, sampled at texel centres."""
    rng = random.Random(seed)
    lattice = [rng.uniform(-1.0, 1.0) for _ in range(cells * cells)]
    positions = []
    for pixel in range(size):
        coordinate = (pixel + 0.5) * cells / size
        floor = math.floor(coordinate)
        fraction = coordinate - floor
        weight = fraction * fraction * fraction * (fraction * (fraction * 6.0 - 15.0) + 10.0)
        positions.append((floor % cells, (floor + 1) % cells, weight))
    result = []
    for y0, y1, fy in positions:
        for x0, x1, fx in positions:
            a = lattice[y0 * cells + x0] * (1.0 - fx) + lattice[y0 * cells + x1] * fx
            b = lattice[y1 * cells + x0] * (1.0 - fx) + lattice[y1 * cells + x1] * fx
            result.append(a * (1.0 - fy) + b * fy)
    return result


def height_field(size: int, kind: str) -> tuple[float, list[float]]:
    if kind == "concrete":
        tile_metres = 0.64
        octaves = ((8, 0.00036, 1201), (32, 0.00016, 1202), (96, 0.000075, 1203))
        pit_count, pit_depth, radius_range = 140, 0.00065, (0.0018, 0.0050)
        seed = 3101
    else:
        tile_metres = 0.80
        octaves = ((12, 0.00033, 2201), (48, 0.00029, 2202), (128, 0.00021, 2203))
        pit_count, pit_depth, radius_range = 90, 0.00040, (0.0020, 0.0046)
        seed = 4101
    height = [0.0] * (size * size)
    for cells, amplitude, noise_seed in octaves:
        octave = periodic_noise(size, cells, noise_seed)
        height = [base + sample * amplitude for base, sample in zip(height, octave)]

    # Sparse closed pores, with zero slope at their rim. Toroidal indexing also
    # places a pore crossing an edge on its matching neighbouring tile.
    rng = random.Random(seed)
    for _ in range(pit_count):
        cx, cy = rng.uniform(0, size), rng.uniform(0, size)
        radius = rng.uniform(*radius_range) * size / tile_metres
        depth = pit_depth * rng.uniform(0.45, 1.0)
        for y in range(math.floor(cy - radius), math.ceil(cy + radius) + 1):
            for x in range(math.floor(cx - radius), math.ceil(cx + radius) + 1):
                distance_squared = ((x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2) / (radius * radius)
                if distance_squared < 1.0:
                    profile = (1.0 - distance_squared) ** 3
                    height[(y % size) * size + x % size] -= depth * profile
    relief_scale = 2.4 if kind == "concrete" else 1.5
    return tile_metres, [value * relief_scale for value in height]


def normal_pixels(size: int, tile_metres: float, heights: list[float]) -> tuple[bytes, dict]:
    texel_metres = tile_metres / size
    rgb = bytearray()
    slopes = []
    encoded_lengths = []
    edge_steps = []
    for y in range(size):
        for x in range(size):
            du = (heights[y * size + (x + 1) % size] - heights[y * size + (x - 1) % size]) / (2 * texel_metres)
            dv = (heights[((y + 1) % size) * size + x] - heights[((y - 1) % size) * size + x]) / (2 * texel_metres)
            length = math.sqrt(du * du + dv * dv + 1.0)
            normal = (-du / length, -dv / length, 1.0 / length)
            encoded = [max(0, min(255, round((value * 0.5 + 0.5) * 255))) for value in normal]
            rgb.extend(encoded)
            encoded_lengths.append(math.sqrt(sum((value / 255 * 2 - 1) ** 2 for value in encoded)))
            slopes.append(math.degrees(math.acos(normal[2])))
        edge_steps.append(abs(heights[y * size] - heights[y * size + size - 1]))
    edge_steps.extend(abs(heights[x] - heights[(size - 1) * size + x]) for x in range(size))
    ordered_slopes = sorted(slopes)
    assert all(math.isfinite(value) for value in heights + slopes + encoded_lengths)
    assert min(encoded_lengths) > 0.99 and max(encoded_lengths) < 1.01
    stats = {
        "width": size,
        "height": size,
        "tile_metres": tile_metres,
        "height_range_metres": [min(heights), max(heights)],
        "normal_length_range_after_rgb8": [min(encoded_lengths), max(encoded_lengths)],
        "normal_tilt_degrees_mean": sum(slopes) / len(slopes),
        "normal_tilt_degrees_p95": ordered_slopes[int(len(slopes) * 0.95)],
        "normal_tilt_degrees_max": max(slopes),
        "largest_wrapped_height_step_metres": max(edge_steps),
    }
    return bytes(rgb), stats


def write_bmp(path: Path, size: int, rgb: bytes) -> None:
    stride = (size * 3 + 3) & ~3
    payload = bytearray()
    for y in range(size):
        row = rgb[y * size * 3 : (y + 1) * size * 3]
        for x in range(size):
            r, g, b = row[x * 3 : x * 3 + 3]
            payload.extend((b, g, r))
        payload.extend(b"\0" * (stride - size * 3))
    header = struct.pack("<2sIHHI", b"BM", 54 + len(payload), 0, 0, 54)
    info = struct.pack("<IiiHHIIiiII", 40, size, size, 1, 24, 0, len(payload), 2835, 2835, 0, 0)
    path.write_bytes(header + info + payload)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parent)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    report = {"generator": "MakeNormalMaps.py", "seeded": True, "encoding": "RGB8 linear OpenGL +Y"}
    for kind in ("concrete", "asphalt"):
        tile_metres, heights = height_field(SIZE, kind)
        rgb, stats = normal_pixels(SIZE, tile_metres, heights)
        target = args.output / f"{kind.upper()}_NORMAL.BMP"
        write_bmp(target, SIZE, rgb)
        stats["sha256"] = hashlib.sha256(target.read_bytes()).hexdigest()
        report[kind] = stats
    target = args.output / "normal-map-stats.json"
    target.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
