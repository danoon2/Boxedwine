#!/usr/bin/env python3
"""Install the pinned Wine 11 Vulkan registry and C headers from a local archive."""

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import tarfile

VERSION = "1.4.335"
URL = f"https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v{VERSION}.tar.gz"
ARCHIVE_SHA256 = "8ee39bee575bdccc1ecacd0bdb26f181841de35d7409a3eca468c6b622daa2f1"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    args = parser.parse_args()
    if hashlib.sha256(args.archive.read_bytes()).hexdigest() != ARCHIVE_SHA256:
        parser.error("archive checksum does not match pinned Vulkan-Headers 1.4.335")
    repo = Path(__file__).resolve().parents[2]
    files = {}
    prefix = f"Vulkan-Headers-{VERSION}/"
    with tarfile.open(args.archive) as source:
        for member in source.getmembers():
            if not member.isfile() or not member.name.startswith(prefix):
                continue
            name = member.name[len(prefix):]
            if name == "registry/vk.xml":
                destination = "lib/mesa/vkRegistry/vk.xml"
            elif name in ("include/vulkan/vulkan.h", "include/vulkan/vulkan_core.h", "include/vulkan/vk_platform.h"):
                destination = "source/vulkan/vk/" + PurePosixPath(name).name
            elif name.startswith("include/vk_video/") and name.endswith(".h"):
                destination = "source/vulkan/vk/vk_video/" + PurePosixPath(name).name
            else:
                continue
            files[destination] = source.extractfile(member).read()
    if "lib/mesa/vkRegistry/vk.xml" not in files or len(files) < 10:
        parser.error("incomplete archive")
    for name, contents in files.items():
        (repo / name).parent.mkdir(parents=True, exist_ok=True)
        (repo / name).write_bytes(contents)
    provenance = {"version": VERSION, "url": URL, "archive_sha256": ARCHIVE_SHA256,
                  "files_sha256_lf": {name: hashlib.sha256(contents).hexdigest()
                                      for name, contents in sorted(files.items())}}
    (repo / "lib/mesa/vkRegistry/provenance.json").write_text(json.dumps(provenance, indent=2) + "\n")
    print(f"Installed Vulkan-Headers {VERSION}: {len(files)} files")


if __name__ == "__main__":
    main()
