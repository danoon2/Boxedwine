#!/usr/bin/env python3
"""Launch the prepared, isolated filesystem-11 Motorhead experiment."""

import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import time
import zipfile


def main():
    repository = Path(__file__).resolve().parents[6]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--research", type=Path,
                        default=repository / "tmp/glide-redistribution-research")
    parser.add_argument("--timeout", type=int, default=0,
                        help="Stop after this many seconds; zero means no time limit")
    parser.add_argument("--trace-exceptions", action="store_true")
    args = parser.parse_args()
    base = args.research.resolve()
    runtime = base / "runtime-fs11/build/Build/Products/Release/Boxedwine.app/Contents/MacOS/Boxedwine"
    filesystem = repository / "tmp/native-ui-test/import-wine/catalog-packages/TinyCore15Wine11.0.zip"
    template = base / "prototype/root-motorhead-final11"
    root = base / "prototype/root-motorhead-play11"
    game = "home/username/.wine/drive_c/Program Files/Digital Illusions/Motorhead Playable 3DFX Demo"
    for path in (runtime, filesystem, template / game / "motor.exe"):
        if not path.is_file():
            parser.error(f"Prepared experiment file is missing: {path}")
    with zipfile.ZipFile(filesystem) as archive:
        if archive.read("version.txt").strip() != b"11":
            parser.error("Motorhead requires filesystem version 11 for this experiment")
    if not root.exists():
        shutil.copytree(template, root)
    if not (root / game / "motor.exe").is_file():
        parser.error(f"Incomplete test prefix: {root}")
    lock = root / ".motorhead-running"
    try:
        lock.mkdir()
    except FileExistsError:
        parser.error(f"This test prefix is already in use. If a previous launcher crashed, remove {lock}")
    logs = base / "prototype/motorhead-runs"
    logs.mkdir(exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    log = logs / f"{stamp}.log"
    command = "export WINEDLLOVERRIDES=d3d9=b; export WINE_D3D_CONFIG=renderer=gl; "
    if args.trace_exceptions:
        command += "export WINEDEBUG=+seh; "
    command += "/bin/wine motor.exe; echo MOTORHEAD_EXIT=$?; /opt/wine/bin/wineserver -k"
    argv = [str(runtime), "-root", str(root), "-zip", str(filesystem),
            "-title", "Motorhead — psVoodoo / OpenGL / filesystem 11",
            "-w", "/" + game, "/bin/sh", "-c", command]
    process = None
    started = time.monotonic()
    stopped = False
    try:
        with log.open("wb") as output:
            process = subprocess.Popen(argv, cwd=base, stdin=subprocess.PIPE,
                                       stdout=output, stderr=subprocess.STDOUT)
            print(f"Motorhead PID {process.pid}\nLog: {log}\n"
                  "Enter: start; arrows: drive; F1/F2/F3: camera; Escape: exit.\n"
                  "Press Ctrl-C in this terminal to stop the isolated runtime.", flush=True)
            try:
                process.wait(timeout=args.timeout or None)
            except (KeyboardInterrupt, subprocess.TimeoutExpired):
                stopped = True
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            finally:
                process.stdin.close()
    finally:
        lock.rmdir()
    result = {"host_exit": process.returncode, "externally_stopped": stopped,
              "seconds": round(time.monotonic() - started, 2), "log": str(log),
              "runtime_sha256": hashlib.sha256(runtime.read_bytes()).hexdigest(),
              "filesystem_version": 11, "wine": "11.0", "backend": "WineD3D OpenGL"}
    log.with_suffix(".json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
