#!/usr/bin/env python3
"""Run an Emscripten Python entry point without discarding its profiler on exec.

Use only with EMPROFILE=1. Compiler arguments and output are unchanged, but the
Python driver waits for Clang so its profiling blocks and atexit handler close.
This adds process-wrapper overhead; its timings are diagnostic measurements.
"""

from __future__ import annotations

import argparse
import importlib
import os
from pathlib import Path
import runpy
import sys


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tool", type=Path, help="installed emcc.py or em++.py")
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    args = parser.parse_args(argv)
    if os.environ.get("EMPROFILE") != "1":
        parser.error("this diagnostic wrapper requires EMPROFILE=1")
    tool = args.tool.resolve()
    if tool.name not in ("emcc.py", "em++.py") or not tool.is_file():
        parser.error("tool must be an existing Emscripten emcc.py or em++.py")
    if not (tool.parent / "tools" / "utils.py").is_file():
        parser.error("Emscripten tools/utils.py is missing beside the entry point")

    previous_path, previous_argv = sys.path[:], sys.argv[:]
    sys.path.insert(0, str(tool.parent))
    sys.argv = [str(tool), *args.arguments]
    utils = importlib.import_module("tools.utils")
    original_exec = utils.exec

    def wait_for_compiler(command):
        result = utils.run_process(command, stdin=sys.stdin, check=False)
        # Shell-compatible status for a compiler killed by a POSIX signal.
        status = result.returncode if result.returncode >= 0 else 128 - result.returncode
        # Use sys.exit so Emscripten's profiler also records the actual status.
        sys.exit(status)

    try:
        utils.exec = wait_for_compiler
        runpy.run_path(str(tool), run_name="__main__")
        return 0
    finally:
        utils.exec = original_exec
        sys.path[:] = previous_path
        sys.argv[:] = previous_argv


if __name__ == "__main__":
    raise SystemExit(main())
