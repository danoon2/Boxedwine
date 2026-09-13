"""Compare focused D3D9 sample capabilities with independent guest GL queries.

This checks reported per-format sample policy, not extension availability or
general renderability. Run the color/depth probes too: their pixels verify that
advertised combinations actually work. Browser identity, diagnostics and cleanup
still require the normal graphics harness audit.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re


COLOR_FORMATS = {
    "A8R8G8B8": "RGBA8", "X8R8G8B8": "RGBA8", "R5G6B5": "RGB565",
    "X1R5G5B5": "RGBA8", "A1R5G5B5": "RGBA8", "A4R4G4B4": "RGBA4",
    "A2R10G10B10": "RGB10_A2", "A2B10G10R10": "RGB10_A2",
    "R16F": "R16F", "G16R16F": "RG16F", "A16B16G16R16F": "RGBA16F",
    "R32F": "R32F", "G32R32F": "RG32F", "A32B32G32R32F": "RGBA32F",
}
# The color probe currently covers A2R10G10B10, not A2B10G10R10.
REQUIRED_COLORS = set(COLOR_FORMATS) - {"A2B10G10R10"}
DEPTH_FORMATS = {"80": "DEPTH16", "77": "DEPTH24", "75": "DEPTH24_STENCIL8",
                 "82": "DEPTH32F", "83": "DEPTH32F_STENCIL8"}
QUERY_FORMATS = set(COLOR_FORMATS.values()) | set(DEPTH_FORMATS.values())
NOT_AVAILABLE = "8876086a"


def _rows(pattern, output, label):
    result = {}
    for row in re.findall(pattern, output, re.M):
        key, value = row[:-1], row[-1]
        if key in result and result[key] != value:
            raise ValueError(f"Disagreeing {label} records for {key}")
        result[key] = value
    return result


def compare(query, color, depth=None):
    errors, comparisons = [], []
    counts = _rows(r"^FORMAT_SAMPLES format=(\w+) count=(\d+) error=(\d+)\s*$", query, "query count")
    samples = _rows(r"^FORMAT_SAMPLES format=(\w+) index=(\d+) samples=(\d+)\s*$", query, "query list")
    lists = {}
    for (name, count), error in counts.items():
        if error != "0" or name in lists:
            raise ValueError(f"Invalid or disagreeing query count for {name}")
        indices = {int(i): int(value) for (fmt, i), value in samples.items() if fmt == name}
        if set(indices) != set(range(int(count))):
            raise ValueError(f"Incomplete sample list for {name}")
        values = [indices[i] for i in range(int(count))]
        if values != sorted(set(values), reverse=True) or any(v <= 0 for v in values):
            raise ValueError(f"Invalid sample list for {name}")
        lists[name] = values
    if set(lists) != QUERY_FORMATS or any(fmt not in lists for fmt, _ in samples):
        raise ValueError("Missing or unexpected queried formats")

    def check_output(output, kind, mappings, required, cap_pattern, sample_pattern):
        caps = _rows(cap_pattern, output, kind + " usage")
        if {key[0] for key in caps} != required:
            raise ValueError(f"Incomplete {kind} format coverage")
        # Capture result/quality as a single field so duplicate records can be
        # checked without silently accepting differing return codes.
        rows = _rows(sample_pattern, output, kind + " samples")
        for (name,), cap in caps.items():
            if cap not in ("00000000", NOT_AVAILABLE):
                raise ValueError(f"Undefined {kind} capability result: {name} {cap}")
            actual = {int(s): value for (fmt, s), value in rows.items() if fmt == name}
            if cap == NOT_AVAILABLE:
                if actual:
                    raise ValueError(f"Unexpected samples for unsupported {kind} format {name}")
                comparisons.append(dict(kind=kind, format=name, renderable=False))
                continue
            if set(actual) != set(range(17)):
                raise ValueError(f"Incomplete {kind} sample coverage for {name}")
            reported = lists[mappings[name]]
            # Wine's existing NONMASKABLE quality indices enumerate all positive
            # sample bits, including sample 1 when the GL driver reports it.
            supported = [v for v in reported if v <= 32]
            for s, value in sorted(actual.items()):
                hr, quality = re.fullmatch(r"([0-9a-f]{8}) quality=(\d+)", value).groups()
                expected = s == 0 or (bool(supported) if s == 1 else s in supported)
                expected_hr = "00000000" if expected else NOT_AVAILABLE
                expected_quality = len(supported) if s == 1 else 1
                if hr != expected_hr or (expected and int(quality) != expected_quality):
                    errors.append(f"{kind} {name} samples={s}: {value}; expected "
                                  f"{expected_hr}" + (f" quality={expected_quality}" if expected else ""))
            comparisons.append(dict(kind=kind, format=name, renderable=True,
                                    gl_format=mappings[name], gl_samples=reported, checked_sample_types=17))
        if any(fmt not in required for fmt, _ in rows):
            raise ValueError(f"Unexpected {kind} sample format")

    check_output(color, "color", COLOR_FORMATS, REQUIRED_COLORS,
                 r"^FORMAT_CAP format=(\w+) render_target=([0-9a-f]{8})\s*$",
                 r"^FORMAT_CAP format=(\w+) samples=(\d+) result=([0-9a-f]{8} quality=\d+)\s*$")
    if depth is not None:
        check_output(depth, "depth", DEPTH_FORMATS, set(DEPTH_FORMATS),
                     r"^DEPTH_CAP depth=(\d+) usage=([0-9a-f]{8})\s*$",
                     r"^DEPTH_CAP depth=(\d+) samples=(\d+) result=([0-9a-f]{8} quality=\d+) color_result=[0-9a-f]{8} color_quality=\d+\s*$")
    return dict(passed=not errors, errors=errors, queried_samples=lists, comparisons=comparisons)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--query-log", type=Path, required=True)
    parser.add_argument("--color-log", type=Path, required=True)
    parser.add_argument("--depth-log", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    paths = [args.query_log, args.color_log] + ([args.depth_log] if args.depth_log else [])
    result = compare(*(path.read_text(errors="replace") for path in paths))
    result["inputs"] = [{"path": str(path.resolve()), "bytes": path.stat().st_size,
                         "sha256": hashlib.sha256(path.read_bytes()).hexdigest()} for path in paths]
    with args.output.open("x") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
