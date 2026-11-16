"""Prepare a test runtime withholding optional WebGL float extensions.

Changes only HTML. Withholding discovery does not disable features that a
different extension enables implicitly. Never publish this diagnostic runtime.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

from wineGraphicsBrowser import validate_web_build

EXTENSIONS = ("EXT_color_buffer_float", "EXT_color_buffer_half_float", "EXT_float_blend", "OES_texture_float_linear")
FIXTURE = Path(__file__).with_name("tests") / "webgl_float_extension_filter.js"


def filter_script(disabled):
    if not disabled or not set(disabled) <= set(EXTENSIONS):
        raise ValueError("Select one or more supported float-extension controls")
    text = FIXTURE.read_text(encoding="utf-8")
    if text.count("/*__DISABLED__*/") != 1:
        raise ValueError("Unexpected extension fixture")
    return text.replace("/*__DISABLED__*/", json.dumps(sorted({s.lower() for s in disabled})))


def pin(path):
    data = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--disable-extension", choices=EXTENSIONS, action="append", required=True)
    args = parser.parse_args()
    validate_web_build(args.build_dir)
    html = (args.build_dir / "boxedwine.html").read_text(encoding="utf-8")
    if html.count("</head>") != 1 or "FLOAT_EXTENSION_FILTER" in html:
        parser.error("Expected an unmodified runtime with one closing head tag")
    script = filter_script(args.disable_extension)
    files = sorted(path for path in args.build_dir.iterdir() if path.is_file())
    inputs = [pin(path) for path in files]
    args.output.mkdir(parents=True, exist_ok=False)
    for path in files:
        shutil.copyfile(path, args.output / path.name)
    (args.output / "boxedwine.html").write_text(html.replace("</head>", "<script>" + script + "</script></head>"), encoding="utf-8", newline="\n")
    changed = [p.name for p in files if p.read_bytes() != (args.output / p.name).read_bytes()]
    if changed != ["boxedwine.html"] or inputs != [pin(p) for p in files]:
        raise RuntimeError("Unexpected runtime change")
    record = dict(scope=__doc__, disabled=args.disable_extension, inputs=inputs, fixture=pin(FIXTURE),
                  preparer=pin(Path(__file__)), changed_files=changed, outputs=[pin(args.output / p.name) for p in files])
    (args.output / "filter-build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(f"Prepared test-only extension control at {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
