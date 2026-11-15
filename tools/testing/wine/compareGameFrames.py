#!/usr/bin/env python3
"""Compare final browser canvas screenshots with explicit local references.

Requires Pillow. Capture the canvas with the browser compositor, not guest
ReadPixels or the SDL recorder. No resizing or automatic reference updates.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path

from PIL import Image, ImageChops


def fraction(value: str) -> float:
    result = float(value)
    if not math.isfinite(result) or not 0 <= result <= 1:
        raise argparse.ArgumentTypeError("must be between 0 and 1")
    return result


def region(value: str) -> dict:
    try:
        name, x, y, width, height, limit = value.split(",")
        result = {"name": name, "box": [int(x), int(y), int(width), int(height)],
                  "max_bad_fraction": fraction(limit)}
        if not name or min(result["box"][:2]) < 0 or min(result["box"][2:]) <= 0:
            raise ValueError("invalid name or rectangle")
        return result
    except (ValueError, argparse.ArgumentTypeError) as error:
        raise argparse.ArgumentTypeError("expected NAME,X,Y,WIDTH,HEIGHT,MAX_BAD_FRACTION") from error


def region_progress(value: str) -> tuple[str, float]:
    try:
        name, minimum = value.split(",")
        minimum = fraction(minimum)
        if not name or minimum <= 0:
            raise ValueError("progress must be positive")
        return name, minimum
    except (ValueError, argparse.ArgumentTypeError) as error:
        raise argparse.ArgumentTypeError("expected NAME,MIN_PROGRESS_FRACTION with 0 < fraction <= 1") from error


def identity(path: Path) -> dict:
    data = path.read_bytes()
    return {"path": str(path.resolve()), "bytes": len(data),
            "sha256": hashlib.sha256(data).hexdigest()}


def load_frame(path: Path) -> Image.Image:
    with Image.open(path) as source:
        source.load()
        rgba = source.convert("RGBA")
        if rgba.getchannel("A").getextrema() != (255, 255):
            raise ValueError(f"{path}: expected an opaque browser compositor screenshot")
        return rgba.convert("RGB")


def difference(actual: Image.Image, expected: Image.Image, tolerance: int) -> dict:
    channels = ImageChops.difference(actual, expected).split()
    largest = ImageChops.lighter(ImageChops.lighter(channels[0], channels[1]), channels[2])
    histogram = largest.histogram()
    pixels = actual.width * actual.height
    bad = sum(histogram[tolerance + 1:])
    channel_error = sum(sum(value * count for value, count in enumerate(channel.histogram()))
                        for channel in channels)
    return {"pixels": pixels, "bad_pixels": bad, "bad_fraction": bad / pixels,
            "mean_channel_error": channel_error / (pixels * 3)}


def dominant_fraction(frame: Image.Image) -> float:
    # Quantization also rejects almost-solid frames with small dithering/noise.
    buckets = frame.point(lambda value: value & 0xf0).getcolors(maxcolors=4096)
    return max(count for count, _color in buckets) / (frame.width * frame.height)


def compare_frames(actual: Path, reference: Path, *, tolerance: int = 12,
                   max_bad_fraction: float = 0.05, regions: list[dict] | None = None,
                   previous: Path | None = None, min_progress_fraction: float = 0.01,
                   max_dominant_fraction: float = 0.98) -> dict:
    if not isinstance(tolerance, int) or not 0 <= tolerance <= 255:
        raise ValueError("channel tolerance must be an integer between 0 and 255")
    for value in (max_bad_fraction, min_progress_fraction, max_dominant_fraction):
        fraction(str(value))
    actual_image, reference_image = load_frame(actual), load_frame(reference)
    report = {"schema_version": 1, "scope": "Final canvas pixels only; not input, log or shutdown acceptance.",
              "passed": False, "comparator": identity(Path(__file__)),
              "actual": identity(actual), "reference": identity(reference),
              "tolerance": tolerance, "max_bad_fraction": max_bad_fraction,
              "max_dominant_fraction": max_dominant_fraction,
              "actual_size": list(actual_image.size), "reference_size": list(reference_image.size),
              "regions": [], "problems": []}
    problems = report["problems"]
    if actual_image.size != reference_image.size:
        problems.append("canvas dimensions differ; images are never resized to make a comparison pass")
        return report
    for label, frame in (("actual", actual_image), ("reference", reference_image)):
        dominant = dominant_fraction(frame)
        report[label + "_dominant_fraction"] = dominant
        if dominant >= max_dominant_fraction:
            problems.append(f"{label} canvas is blank or almost solid")
    report["comparison"] = difference(actual_image, reference_image, tolerance)
    if report["comparison"]["bad_fraction"] > max_bad_fraction:
        problems.append("canvas differs from reference")
    previous_image = None
    if previous is not None:
        previous_image = load_frame(previous)
        report["previous"] = identity(previous)
        report["min_progress_fraction"] = min_progress_fraction
        if previous_image.size != actual_image.size:
            problems.append("previous canvas dimensions differ; frame progress cannot be established")
        else:
            report["progress"] = difference(actual_image, previous_image, tolerance)
            if report["progress"]["bad_fraction"] < min_progress_fraction:
                problems.append("insufficient canvas progress from the previous checkpoint")
    names = set()
    for rule in regions or []:
        name = rule["name"]
        x, y, width, height = rule["box"]
        limit = fraction(str(rule["max_bad_fraction"]))
        if (not name or name in names or any(type(v) is not int for v in rule["box"])
                or min(x, y) < 0 or min(width, height) <= 0
                or x + width > actual_image.width or y + height > actual_image.height):
            raise ValueError(f"invalid, duplicate or out-of-bounds region: {name}")
        names.add(name)
        box = (x, y, x + width, y + height)
        measured = difference(actual_image.crop(box), reference_image.crop(box), tolerance)
        passed = measured["bad_fraction"] <= limit
        result = dict(rule, **measured, reference_passed=passed, passed=passed)
        report["regions"].append(result)
        if not passed:
            problems.append(f"region differs from reference: {name}")
        if "min_progress_fraction" in rule:
            minimum = fraction(str(rule["min_progress_fraction"]))
            if minimum <= 0 or previous_image is None:
                raise ValueError(f"region progress requires a positive minimum and a previous frame: {name}")
            progress_passed = False
            if previous_image.size == actual_image.size:
                result["progress"] = difference(actual_image.crop(box), previous_image.crop(box), tolerance)
                progress_passed = result["progress"]["bad_fraction"] >= minimum
            result["progress_passed"] = progress_passed
            result["passed"] = passed and progress_passed
            if not progress_passed:
                problems.append(f"insufficient scene progress in region: {name}")
    report["passed"] = not problems
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("actual", type=Path)
    parser.add_argument("--reference", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path, help="new JSON report; never overwritten")
    parser.add_argument("--tolerance", type=int, default=12, help="allowed 8-bit channel error (default: 12)")
    parser.add_argument("--max-bad-fraction", type=fraction, default=0.05)
    parser.add_argument("--region", type=region, action="append", default=[])
    parser.add_argument("--region-progress", type=region_progress, action="append", default=[],
                        help="NAME,MIN_PROGRESS_FRACTION for an existing --region; requires --previous")
    parser.add_argument("--previous", type=Path, help="optional earlier checkpoint that must visibly differ")
    parser.add_argument("--min-progress-fraction", type=fraction, default=0.01)
    parser.add_argument("--max-dominant-fraction", type=fraction, default=0.98)
    args = parser.parse_args(argv)
    try:
        progress_names = set()
        for name, minimum in args.region_progress:
            matches = [rule for rule in args.region if rule["name"] == name]
            if len(matches) != 1 or name in progress_names or args.previous is None:
                raise ValueError("region progress needs one matching region, one minimum and --previous: " + name)
            progress_names.add(name)
            matches[0]["min_progress_fraction"] = minimum
        report = compare_frames(args.actual, args.reference, tolerance=args.tolerance,
                                max_bad_fraction=args.max_bad_fraction, regions=args.region,
                                previous=args.previous, min_progress_fraction=args.min_progress_fraction,
                                max_dominant_fraction=args.max_dominant_fraction)
        with args.output.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(report, stream, indent=2)
            stream.write("\n")
    except (OSError, ValueError, argparse.ArgumentTypeError) as error:
        parser.exit(2, f"frame comparison failed: {error}\n")
    print(f"{'PASS' if report['passed'] else 'FAIL'}: {args.actual}")
    for problem in report["problems"]:
        print(f"  {problem}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
