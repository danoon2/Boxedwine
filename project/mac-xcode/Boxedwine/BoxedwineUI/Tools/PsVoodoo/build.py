#!/usr/bin/env python3
"""Build the pinned Boxedwine psVoodoo fork."""
import argparse
from pathlib import Path
import subprocess
import sys

REPOSITORY = "https://github.com/danoon2/psVoodoo.git"
REVISION = "67fcb0a0eb1be9c5f77d04250ad715b2f481754b"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path, help="Clean Boxedwine psVoodoo fork checkout")
    parser.add_argument("--toolchain", required=True, type=Path, help="LLVM-MinGW bin directory")
    parser.add_argument("--output", required=True, type=Path, help="New build output directory")
    parser.add_argument("--d3d9-only", action="store_true", help="Use the tested WineD3D path")
    parser.add_argument("--build-probes", action="store_true", help="Also compile the fork's regressions")
    args = parser.parse_args()
    source = args.source.resolve()
    revision = subprocess.check_output(
        ["git", "-C", str(source), "rev-parse", "HEAD"], text=True).strip()
    if revision != REVISION:
        raise RuntimeError(f"Expected fork commit {REVISION}, found {revision}. "
                           f"Use a checkout of {REPOSITORY} at the pinned commit.")
    subprocess.run(["git", "-C", str(source), "diff", "--exit-code", "HEAD", "--"], check=True)
    command = [sys.executable, str(source / "tools/build.py"),
               "--source", str(source), "--toolchain", str(args.toolchain.resolve()),
               "--output", str(args.output.resolve())]
    if args.d3d9_only:
        command.append("--d3d9-only")
    if args.build_probes:
        command.append("--build-probes")
    subprocess.run(command, check=True)


if __name__ == "__main__":
    main()
