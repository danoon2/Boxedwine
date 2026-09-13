"""Build PE32 point-size capability and focused upstream Wine rendering probes.

Run under Linux/WSL with MinGW and an already configured Wine 11 i386 build.
The source and build directories must belong to the same configured tree.
Use a fresh output directory and do not build that Wine tree concurrently.
The focused executables retain Wine's test_pointsize() assertions unchanged;
they are diagnostic executables, not replacements for the full Wine tests.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine-source", type=Path, required=True)
    parser.add_argument("--wine-build", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cc", default="i686-w64-mingw32-gcc")
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--source-date-epoch", default="1768319767")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    output = args.output.resolve()
    source = args.wine_source.resolve()
    build = args.wine_build.resolve()
    caps_source = Path(__file__).resolve().parent / "tests/point_size_caps_probe.c"
    output.mkdir(parents=True, exist_ok=False)
    environment = dict(os.environ, SOURCE_DATE_EPOCH=args.source_date_epoch)
    records = []
    for version in (8, 9):
        caps = output / f"D3D{version}PointSizeCapsProbe.exe"
        command = [args.cc, "-O2", "-Wall", "-Wextra"]
        if version == 8:
            command.append("-DTEST_D3D8")
        command += [str(caps_source), "-o", str(caps), f"-ld3d{version}", "-lopengl32", "-lgdi32"]
        with (output / f"build-caps-d3d{version}.log").open("x") as log:
            subprocess.run(command, env=environment, stdout=log, stderr=subprocess.STDOUT, check=True)

        visual_source = source / f"dlls/d3d{version}/tests/visual.c"
        built_exe = build / f"dlls/d3d{version}/tests/d3d{version}_test.exe"
        original = visual_source.read_bytes()
        original_exe = built_exe.read_bytes() if built_exe.exists() else None
        marker = b"START_TEST(visual)\n{"
        if original.count(marker) != 1 or original.count(b"static void test_pointsize(void)") != 1:
            raise RuntimeError(f"Expected Wine 11 point-size test entry point in {visual_source}")
        focused = original.replace(marker, marker + b"\n    test_pointsize();\n    return;\n")
        (output / f"original-d3d{version}-visual.c").write_bytes(original)
        (output / f"focused-d3d{version}-visual.c").write_bytes(focused)
        visual = output / f"D3D{version}WinePointSizeProbe.exe"
        try:
            visual_source.write_bytes(focused)
            with (output / f"build-visual-d3d{version}.log").open("x") as log:
                subprocess.run(["make", f"-j{args.jobs}", f"dlls/d3d{version}/tests/d3d{version}_test.exe"],
                               cwd=build, env=environment, stdout=log, stderr=subprocess.STDOUT, check=True)
            visual.write_bytes(built_exe.read_bytes())
        finally:
            # Never leave the full test target replaced by the focused diagnostic.
            if original_exe is not None:
                built_exe.write_bytes(original_exe)
            else:
                built_exe.unlink(missing_ok=True)
            # Touch the original source last so make also refreshes its object next time.
            visual_source.write_bytes(original)
        records.append({"version": version, "caps_command": command,
                        "caps_source_sha256": sha256(caps_source.read_bytes()),
                        "caps_executable_sha256": sha256(caps.read_bytes()),
                        "original_visual_source_sha256": sha256(original),
                        "focused_visual_source_sha256": sha256(focused),
                        "visual_executable_sha256": sha256(visual.read_bytes())})
        print(f"Built D3D{version} probes; restored the Wine source and full test executable.", flush=True)
    (output / "manifest.json").write_text(json.dumps({"source_date_epoch": args.source_date_epoch,
                                                    "probes": records}, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
