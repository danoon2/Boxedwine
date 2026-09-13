"""Prepare or build focused Wine 11 float visual diagnostics.

Retains the original test bodies, expected pixels and tolerances. Removes only
the selected whole-function WebGL skip. Optional strict variants also remove
the selected test's old TODO allowance, making those assertions unconditional.
Build under Linux/WSL with matching configured Wine source/build directories,
and never compile or run another graphics job concurrently.
"""
import argparse
from datetime import datetime, timezone
import difflib
import hashlib
import json
import os
from pathlib import Path
import subprocess


SELECTIONS = {
    "fog": ("test_fog", "test_texture_transform_flags", "D3D9FloatFogProbe.exe"),
    "texture-transform": ("test_texture_transform_flags", "test_lighting_matrices", "D3D9FloatTextureTransformProbe.exe"),
}


def prepare(original, probe, strict=False):
    function, following, _ = SELECTIONS[probe]
    start = f"static void {function}(void)".encode()
    end = f"static void {following}(void)".encode()
    entry = b"START_TEST(visual)\n{"
    if any(original.count(marker) != 1 for marker in (start, end, entry)):
        raise ValueError("Expected the original Wine 11 functions and visual entry point")
    first, last = original.index(start), original.index(end)
    if first >= last or last >= original.index(entry):
        raise ValueError("Unexpected Wine test layout")
    body = original[first:last]
    skip = ("    if (is_webgl())\n    {\n"
            "        skip(\"WebGL does not support D3DFMT_A32B32G32R32F render targets for this "
            + probe + " test.\\n\");\n        return;\n    }\n").encode()
    if body.count(skip) > 1:
        raise ValueError("Duplicate whole-function skip")
    removed_skip = skip in body
    body = body.replace(skip, b"\n" * skip.count(b"\n"))
    if b"if (is_webgl())" in body:
        raise ValueError("Unexpected remaining WebGL condition in selected test")
    if strict:
        todo = (b"                    todo_wine_if (attrib_count == 1 && vs_mode == VS_MODE_FFP && ps_mode <= 1 && i == 7)\n"
                if probe == "texture-transform" else
                b"todo_wine_if ((fog_mode_tests[pixel_mode] != D3DFOG_NONE && (vs_mode == VS_MODE_FFP || (!ortho_fog && vs_mode == VS_MODE_RHW)))\n"
                b"        || (fog_mode_tests[pixel_mode] == D3DFOG_NONE && fog_mode_tests[vertex_mode] != D3DFOG_NONE && vs_mode == VS_MODE_FFP))\n")
        if body.count(todo) != 1:
            raise ValueError(f"Expected the original {probe} TODO condition")
        body = body.replace(todo, b"\n" * todo.count(b"\n"))
    focused = original[:first] + body + original[last:]
    focused = focused.replace(entry, entry + (
        f'\n    trace("BW_FLOAT_VISUAL_BEGIN {probe}\\n");\n'
        f'    {function}();\n    trace("BW_FLOAT_VISUAL_END {probe}\\n");\n    return;\n').encode())
    return focused, removed_skip


def pin(path):
    data = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine-source", type=Path, required=True)
    parser.add_argument("--wine-build", type=Path)
    parser.add_argument("--probe", choices=SELECTIONS, required=True)
    parser.add_argument("--strict-texture-todo", action="store_true")
    parser.add_argument("--strict-fog-todo", action="store_true")
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--source-date-epoch", default="1768319767")
    args = parser.parse_args()
    if ((args.strict_texture_todo and args.probe != "texture-transform")
            or (args.strict_fog_todo and args.probe != "fog")):
        parser.error("The strict TODO option must match the selected probe")
    if args.jobs < 1 or (not args.prepare_only and args.wine_build is None):
        parser.error("Positive --jobs and a --wine-build directory are required for compilation")
    source = args.wine_source.resolve() / "dlls/d3d9/tests/visual.c"
    original = source.read_bytes()
    focused, removed_skip = prepare(original, args.probe, args.strict_texture_todo or args.strict_fog_todo)
    out = args.output.resolve(); out.mkdir(parents=True, exist_ok=False)
    (out / "original-visual.c").write_bytes(original)
    snapshot = out / "focused-visual.c"; snapshot.write_bytes(focused)
    (out / "diagnostic-only.diff").write_text("".join(difflib.unified_diff(
        original.decode().splitlines(True), focused.decode().splitlines(True),
        fromfile="original-visual.c", tofile="focused-visual.c")), encoding="utf-8", newline="\n")
    record = dict(started_at=datetime.now(timezone.utc).isoformat(), probe=args.probe, builder=pin(Path(__file__)),
        original=pin(source), snapshot=pin(snapshot), removed_whole_function_skip=removed_skip,
        strict_texture_todo=args.strict_texture_todo, strict_fog_todo=args.strict_fog_todo,
        compiled=False, complete=False)

    def save():
        (out / "manifest.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")

    save()
    if not args.prepare_only:
        build = args.wine_build.resolve()
        executable = build / "dlls/d3d9/tests/d3d9_test.exe"
        original_executable = executable.read_bytes()
        (out / "original-d3d9_test.exe").write_bytes(original_executable)
        record["command"] = ["make", f"-j{args.jobs}", "dlls/d3d9/tests/d3d9_test.exe"]
        try:
            source.write_bytes(focused)
            with (out / "build.log").open("xb") as log:
                result = subprocess.run(record["command"], cwd=build,
                    env=dict(os.environ, SOURCE_DATE_EPOCH=args.source_date_epoch),
                    stdout=log, stderr=subprocess.STDOUT)
            record["exit_code"] = result.returncode
            if result.returncode:
                raise RuntimeError("Compilation failed; see retained build.log")
            name = ("D3D9NormalTexgenProbe.exe" if args.strict_texture_todo else
                    "D3D9StrictFogProbe.exe" if args.strict_fog_todo else SELECTIONS[args.probe][2])
            output = out / name; output.write_bytes(executable.read_bytes())
            record.update(compiled=True, executable=pin(output))
        finally:
            executable.write_bytes(original_executable)
            source.write_bytes(original)
            record["restored"] = executable.read_bytes() == original_executable and source.read_bytes() == original
            save()
    record["source_unchanged"] = source.read_bytes() == original
    if not record["source_unchanged"]:
        raise RuntimeError("Wine source changed during preparation")
    record["complete"] = True
    save()
    print(f"Prepared {args.probe}; compiled={record['compiled']}; source unchanged.", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
