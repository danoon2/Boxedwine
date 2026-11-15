"""Compare D3D9 float capabilities/operation coverage with raw WebGL results.

The normal browser harness must separately verify identities, diagnostics and
shutdown. This comparator rejects unavailable or unexecuted raw controls.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

FORMATS = {"R16F": "R16F", "G16R16F": "RG16F", "A16B16G16R16F": "RGBA16F",
           "R32F": "R32F", "G32R32F": "RG32F", "A32B32G32R32F": "RGBA32F"}
CAPABILITIES = ("texture", "filter", "rt_texture", "rt_surface", "blend_texture", "blend_surface")


def compare(raw, output):
    if not raw.get("complete") or not raw.get("passed") or raw.get("diagnostics") or not raw.get("inputs_unchanged"):
        raise ValueError("Raw WebGL control did not complete its harness checks")
    result = raw["result"]
    if not result.get("passed") or result.get("errors") or result.get("observationSeconds", 0) < 15:
        raise ValueError("Incomplete or failed raw GPU observation")
    if not result.get("checks") or any(c.get("passed") is not True for c in result["checks"]):
        raise ValueError("Raw GPU checks did not pass")
    formats = {f["name"]: f for f in result["formats"]}
    if len(formats) != len(result["formats"]) or set(formats) != set(FORMATS.values()):
        raise ValueError("Incomplete or duplicate raw format coverage")
    rows = {}
    pattern = r"^FLOAT_CAP format=(\w+) " + " ".join(c + r"=([0-9a-f]{8})" for c in CAPABILITIES) + r"\s*$"
    for name, *values in re.findall(pattern, output, re.M):
        if name in rows and rows[name] != values:
            raise ValueError("Disagreeing D3D capability captures for " + name)
        rows[name] = values
    if set(rows) != set(FORMATS):
        raise ValueError("Incomplete D3D capability coverage")
    errors, comparisons, expected_cases, expected_skips = [], [], 0, 0
    for name, values in rows.items():
        f = formats[FORMATS[name]]
        operations = {op["operation"] for op in f["operations"]}
        expected_operations = {"point"}
        if f["filtering"]:
            expected_operations.update(("linear-mag", "linear-min"))
        if f["rendering"]:
            expected_operations.add("render")
        if f["blending"]:
            expected_operations.add("blend")
        if operations != expected_operations or len(operations) != len(f["operations"]):
            raise ValueError("Raw operations missing or duplicated for " + name)
        supported = (True, f["filtering"], f["rendering"], f["rendering"], f["blending"], f["blending"])
        expected_cases += 1 + 2 * f["filtering"] + 2 * f["rendering"] + 2 * f["blending"]
        expected_skips += 2 * (not f["filtering"]) + 2 * (not f["rendering"] or not f["blending"])
        for capability, actual, present in zip(CAPABILITIES, values, supported):
            expected = "00000000" if present else "8876086a"
            if actual != expected:
                errors.append(f"{name} {capability}: {actual}; expected {expected}")
        comparisons.append(dict(format=name, raw_format=f["name"], actual=dict(zip(CAPABILITIES, values)), expected=dict(zip(CAPABILITIES, supported))))
    coverage = set(re.findall(r"^FLOAT_COVERAGE attempted=(\d+) completed=(\d+) unsupported=(\d+)\s*$", output, re.M))
    if len(coverage) != 1:
        raise ValueError("Missing or disagreeing D3D coverage summaries")
    attempted, completed, skipped = map(int, next(iter(coverage)))
    if attempted != completed or attempted != expected_cases or skipped != expected_skips:
        errors.append(f"Operation coverage {attempted}/{completed}, skips {skipped}; expected {expected_cases}/{expected_cases}, skips {expected_skips}")
    return dict(passed=not errors, errors=errors, comparisons=comparisons, expected_cases=expected_cases,
                attempted=attempted, completed=completed, skipped=skipped)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw-report", type=Path, required=True)
    parser.add_argument("--wine-log", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = compare(json.loads(args.raw_report.read_text(encoding="utf-8")), args.wine_log.read_text(encoding="utf-8", errors="replace"))
    result["inputs"] = [dict(path=str(p.resolve()), bytes=p.stat().st_size,
                             sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (args.raw_report, args.wine_log)]
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
