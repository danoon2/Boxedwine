#!/usr/bin/env python3
"""Build a pristine, versioned PE32 Wine graphics test bundle on Linux."""

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tarfile
import zipfile

WINE_COMMIT = "db11d0fe6a169c457e23d007e20404643d067aa8"
SUITES = ("vulkan-1", "d3d8", "d3d9", "d3d10", "d3d10_1", "d3d11", "dxgi")


def sha256(path):
    with path.open("rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine-repository", type=Path, required=True)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=8)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=False)
    source, build = work / "source", work / "build"
    source.mkdir()
    build.mkdir()
    # Export a known commit, never a developer's patched working tree.
    archive = work / "source.tar"
    subprocess.run(["git", "-C", str(args.wine_repository.resolve()), "archive",
                    "--format=tar", "-o", str(archive), WINE_COMMIT], check=True)
    with tarfile.open(archive) as src:
        src.extractall(source, filter="data")
    configure = [str(source / "configure"), "--enable-archs=i386",
                 "--without-x", "--without-freetype"]
    with (work / "configure.log").open("w") as log:
        subprocess.run(configure, cwd=build, stdout=log, stderr=subprocess.STDOUT, check=True)
    targets = [f"dlls/{suite}/tests/i386-windows/{suite}_test.exe" for suite in SUITES]
    with (work / "build.log").open("w") as log:
        subprocess.run(["make", f"-j{args.jobs}", *targets], cwd=build,
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    files = {Path(target).name: build / target for target in targets}
    files["COPYING.LIB"] = source / "COPYING.LIB"
    manifest = {"schema_version": 1, "wine_tag": "wine-11.0", "wine_commit": WINE_COMMIT,
                "architecture": "i386", "test_patches": [], "configure": configure[1:],
                "sha256": {name: sha256(path) for name, path in files.items()}}
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "x", compression=zipfile.ZIP_DEFLATED) as dst:
        for name, path in files.items():
            dst.write(path, name)
        dst.writestr("manifest.json", json.dumps(manifest, indent=2) + "\n")
        dst.writestr("SHA256SUMS", "".join(f"{sha256(path)}  {name}\n" for name, path in files.items()))
    print(json.dumps({"bundle": str(output), "sha256": sha256(output), "manifest": manifest}, indent=2))


if __name__ == "__main__":
    main()
