"""Build focused PE32 sample-mask probes using the Wine 11 visual-test helpers.

Run in Linux/WSL with a configured i386 Wine source/build pair. The selected
source and executable are restored even after a build failure. Serialize this
with other builds or native/browser graphics jobs using the same machine.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess


def identity(path):
    raw = path.read_bytes()
    return {"path": str(path.resolve()), "bytes": len(raw), "sha256": hashlib.sha256(raw).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine-source", type=Path, required=True)
    parser.add_argument("--wine-build", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--kind", choices=("transitions", "capabilities"), required=True)
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    source = args.wine_source.resolve() / "dlls/d3d9/tests/visual.c"
    build = args.wine_build.resolve()
    executable = build / "dlls/d3d9/tests/d3d9_test.exe"
    body = Path(__file__).resolve().parent / "tests" / ("sample_mask_" + args.kind + ".c")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    originals = {path: path.read_bytes() for path in (source, executable)}
    text = originals[source].decode("utf-8")
    if text.count("START_TEST(visual)") != 1:
        raise ValueError("Expected the complete Wine 11 visual-test source")
    start = text.index("START_TEST(visual)")
    end = text.index("    test_sanity();", start)
    prefix = text[start:end].replace("START_TEST(visual)\n{", "START_TEST(visual)\n{\n    char mask_module[MAX_PATH];\n    DWORD mask_module_length;", 1)
    label = "transitions" if args.kind == "transitions" else "capability"
    calls = "    test_mask_transition_cycle(0);\n    test_mask_transition_cycle(1);" if args.kind == "transitions" else "    test_mask_capability_bound();"
    focused = text[:start] + body.read_text(encoding="utf-8") + "\n" + prefix + '''    mask_module_length = GetModuleFileNameA(GetModuleHandleA("d3d9.dll"), mask_module, sizeof(mask_module));
    trace("MASK_MODULE d3d9.dll length=%lu path=%s\\n", mask_module_length, mask_module_length ? mask_module : "unavailable");
    trace("MASK_SELECTED_BEGIN LABEL\\n");
CALLS
    trace("MASK_SELECTED_END LABEL\\n");
}
'''.replace("LABEL", label).replace("CALLS", calls)
    (output / "focused-visual.c").write_text(focused, encoding="utf-8", newline="\n")
    for path, raw in originals.items():
        (output / ("original-" + path.name)).write_bytes(raw)
    record = {"complete": False, "passed": False, "kind": args.kind,
              "generator": identity(Path(__file__)), "body": identity(body),
              "source": identity(output / "focused-visual.c"), "before": [identity(p) for p in originals]}
    command = ["make", "-j" + str(args.jobs), "dlls/d3d9/tests/d3d9_test.exe"]
    try:
        source.write_bytes((output / "focused-visual.c").read_bytes())
        with (output / "build.log").open("xb") as log:
            result = subprocess.run(command, cwd=build,
                env=dict(os.environ, SOURCE_DATE_EPOCH="1768319767"), stdout=log, stderr=subprocess.STDOUT)
        record.update(command=command, exit_code=result.returncode)
        if result.returncode:
            raise RuntimeError("Wine probe build failed; see build.log")
        target = output / ("SampleMask" + args.kind.title() + ".exe")
        target.write_bytes(executable.read_bytes())
        record.update(passed=True, executable=identity(target))
    finally:
        # Restore source last so the next make invocation rebuilds its object.
        executable.write_bytes(originals[executable])
        source.write_bytes(originals[source])
        record.update(complete=True, restored=all(path.read_bytes() == raw for path, raw in originals.items()))
        (output / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(record, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
