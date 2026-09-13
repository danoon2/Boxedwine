#!/usr/bin/env python3
"""Check retained fullscreen compositor pixels without starting another browser."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "wineTests"))
from auditGraphicsMatrix import diagnostics


EXPECTED = {
    "original-windowed-gdi": "gdi",
    "original-fullscreen-gdi": "input",
    "original-exited": "gdi",
    "fixed-windowed-gdi": "gdi",
    "fixed-fullscreen-gdi": "gdi",
    "fixed-fullscreen-gl-layer": "input",
    "fixed-fullscreen-gdi-return": "gdi",
    "fixed-fullscreen-input": "gdi",
    "fixed-exited": "gdi",
}
COLORS = {"gdi": (36, 180, 92), "input": (18, 60, 190)}


def audit(directory):
    report_bytes = (directory / "report.json").read_bytes()
    report = json.loads(report_bytes)
    if not report["passed"] or report["errors"]:
        raise ValueError("Browser layer/input control did not pass")
    if [row["name"] for row in report["states"]] != list(EXPECTED):
        raise ValueError("Missing, duplicate or unexpected compositor checkpoints")
    for key, filename in (("shell", "boxedwine-shell.js"), ("css", "boxedwine.css"), ("driver", "driver.mjs")):
        if hashlib.sha256((directory / filename).read_bytes()).hexdigest() != report["inputs"][key]:
            raise ValueError("Retained source changed: " + filename)
    if "template" in report["inputs"]:
        if hashlib.sha256((directory / "shell.html").read_bytes()).hexdigest() != report["inputs"]["template"]:
            raise ValueError("Retained HTML template changed")
    rows = []
    for row in report["states"]:
        name = row["name"]
        if row["artifact"] != name + ".png" or row["state"]["dpr"] != 1:
            raise ValueError("Unexpected screenshot path or device scale")
        path = directory / row["artifact"]
        if hashlib.sha256(path.read_bytes()).hexdigest() != row["sha256"]:
            raise ValueError("Screenshot changed: " + name)
        expected = COLORS[EXPECTED[name]]
        box = row["state"]["gdi"]
        if box["width"] <= 0 or box["height"] <= 0:
            raise ValueError("Empty canvas: " + name)
        pixels = []
        with Image.open(path) as frame:
            image = frame.convert("RGB")
            for y in (.2, .5, .8):
                for x in (.2, .5, .8):
                    point = (int(box["x"] + box["width"] * x), int(box["y"] + box["height"] * y))
                    actual = image.getpixel(point)
                    pixels.append(dict(point=point, expected=expected, actual=actual, passed=actual == expected))
        rows.append(dict(name=name, image_sha256=row["sha256"], pixels=pixels,
                         passed=all(pixel["passed"] for pixel in pixels)))
    log = (directory / "chrome.log").read_bytes()
    emitted = diagnostics(log.decode(errors="replace"))
    return dict(scope="Nine fullscreen compositor checkpoints; no game image acceptance",
                passed=all(row["passed"] for row in rows) and not emitted,
                report_sha256=hashlib.sha256(report_bytes).hexdigest(),
                chrome_log_sha256=hashlib.sha256(log).hexdigest(),
                checked_pixels=sum(len(row["pixels"]) for row in rows),
                states=rows, diagnostics=emitted)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--output", type=Path, required=True, help="New JSON report")
    args = parser.parse_args()
    try:
        result = audit(args.directory)
    except (OSError, ValueError, KeyError, TypeError, IndexError) as error:
        result = dict(passed=False, error=str(error))
    with args.output.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)
        stream.write("\n")
    print(json.dumps({key: value for key, value in result.items() if key != "states"}))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
