"""Copy a browser runtime and restrict sample queries for a negative control.

Only boxedwine.html changes. The fixture uses subsets of measured [8,4,2,1]
sample lists and throws if the current GPU cannot support a requested value.
All allocation, draw, resolve and readback methods remain real browser calls.
This diagnostic runtime must never be used as a production/demo build.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

from wineGraphicsBrowser import validate_web_build


def pin(path):
    data = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    validate_web_build(args.build_dir)
    html = (args.build_dir / "boxedwine.html").read_text()
    if html.count("</head>") != 1 or "SAMPLE_FILTER" in html:
        parser.error("Expected one closing head tag and an unmodified runtime")
    script = Path(__file__).with_name("tests") / "webgl_sample_filter.js"
    inputs = sorted(path for path in args.build_dir.iterdir() if path.is_file())
    identities = [pin(path) for path in inputs]
    args.output.mkdir(parents=True, exist_ok=False)
    for path in inputs:
        shutil.copyfile(path, args.output / path.name)
    (args.output / "boxedwine.html").write_text(
        html.replace("</head>", "<script>" + script.read_text() + "</script></head>"), newline="\n")
    changed = [p.name for p in inputs if p.read_bytes() != (args.output / p.name).read_bytes()]
    if changed != ["boxedwine.html"] or [pin(path) for path in inputs] != identities:
        raise RuntimeError("Unexpected runtime change during fixture preparation")
    record = dict(scope=__doc__, inputs=identities, fixture=pin(script),
                  outputs=[pin(args.output / p.name) for p in inputs], changed_files=changed)
    (args.output / "filter-build.json").write_text(json.dumps(record, indent=2) + "\n")
    print(f"Prepared test-only sample query control at {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
