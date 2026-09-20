#!/usr/bin/env python3
"""Verify and extract the pinned DXVK x32 DLL set into a new test directory."""

import argparse
import hashlib
import json
from pathlib import Path
import tarfile

from wineGraphicsNative import DXVK_DLLS, sha256
from wineGraphicsBrowser import _validate_pe32_i386

VERSION = "3.1.1"
URL = f"https://github.com/doitsujin/dxvk/releases/download/v{VERSION}/dxvk-{VERSION}.tar.gz"
ARCHIVE_SHA256 = "40565b4a724aadc4433fa4e010b4b23916d9b1f1baeee64e17186db94f54e608"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if sha256(args.archive) != ARCHIVE_SHA256:
        parser.error("DXVK archive does not match the upstream 3.1.1 release checksum")
    files = {}
    with tarfile.open(args.archive) as archive:
        for name in DXVK_DLLS:
            member = archive.getmember(f"dxvk-{VERSION}/x32/{name}")
            if not member.isfile():
                parser.error(f"invalid DLL entry: {name}")
            image = archive.extractfile(member).read()
            _validate_pe32_i386(image, name)
            files[name] = image
    args.output.mkdir(parents=True, exist_ok=False)
    manifest = {"version": VERSION, "architecture": "i386", "url": URL,
                "archive_sha256": ARCHIVE_SHA256, "sha256": {}}
    for name, image in files.items():
        (args.output / name).write_bytes(image)
        manifest["sha256"][name] = hashlib.sha256(image).hexdigest()
    (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Verified DXVK {VERSION} x32: {args.output.resolve()}")


if __name__ == "__main__":
    main()
