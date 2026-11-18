"""Validate the real Max RTT multiplier, including UV orientation and polarity.

Usage: python ValidateBake.py [path/to/GarageShadow.bmp]
No third-party Python packages are needed. This does not create or modify images.
"""
from pathlib import Path
import json
import struct
import sys


def validate(path: Path) -> dict:
    data = path.read_bytes()
    assert data[:2] == b"BM", "Expected the original Max BMP output"
    offset = struct.unpack_from("<I", data, 10)[0]
    width, signed_height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    assert bpp == 24 and compression == 0, "Expected uncompressed 24-bit RGB BMP"
    height = abs(signed_height)
    assert width == height and width >= 256, "Expected square RTT image >= 256"
    stride = (width * 3 + 3) & ~3

    def red(x: int, y_top: int) -> int:
        row = height - 1 - y_top if signed_height > 0 else y_top
        return data[offset + row * stride + x * 3 + 2]

    def at_local(x: float, z: float) -> int:
        u, v = (x + 2.33) / 4.35, (z + 3.24) / 6.48
        ix = max(0, min(width - 1, int(u * width)))
        iy_top = max(0, min(height - 1, int((1.0 - v) * height)))
        return red(ix, iy_top)

    values = [red(x, y) for y in range(height) for x in range(width)]
    samples = {
        "clear_center": at_local(0.0, 0.0),
        "clear_rear_aisle": at_local(0.0, -2.0),
        "shadow_behind_stacked_crates": at_local(-1.7, -2.9),
        "shadow_front_crate": at_local(-1.9, 0.8),
        "shadow_workbench": at_local(1.6, 0.8),
    }
    assert min(values) <= 5 and max(values) >= 250, "Missing usable light/shadow range"
    assert samples["clear_center"] >= 245 and samples["clear_rear_aisle"] >= 245, "Clear aisle should retain point lighting"
    assert all(samples[key] <= 10 for key in samples if key.startswith("shadow_")), "Shadow silhouette or UV orientation is incorrect"
    return {
        "file": path.name, "width": width, "height": height,
        "minimum": min(values), "maximum": max(values),
        "shadow_pixels": sum(value <= 5 for value in values),
        "lit_pixels": sum(value >= 250 for value in values),
        "local_samples": samples,
    }


if __name__ == "__main__":
    target = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).with_name("GarageShadow.bmp")
    print(json.dumps(validate(target), indent=2, ensure_ascii=False))
