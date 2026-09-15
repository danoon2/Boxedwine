#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Install the pinned public-API MoltenVK release. No runtime downloads.

Only the named regular file is read from the tar; archive paths are never
extracted. A failed download/validation preserves the previous library.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "resources/moltenvk.lock.json"
DESTINATION = ROOT / "lib/mac/vulkan/lib/libMoltenVK.dylib"


def digest(path):
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def matches(path, size, sha):
    return path.is_file() and path.stat().st_size == size and digest(path) == sha


def library_bytes(archive, pin):
    if not matches(archive, pin["bytes"], pin["sha256"]):
        raise ValueError("MoltenVK archive does not match the pinned size and SHA-256")
    with tarfile.open(archive, "r:") as package:
        members = [m for m in package if m.name == pin["member"]]
        if len(members) != 1 or not members[0].isfile() or members[0].size != pin["libraryBytes"]:
            raise ValueError("MoltenVK archive must contain one regular library of the expected size")
        data = package.extractfile(members[0]).read()
    if hashlib.sha256(data).hexdigest() != pin["librarySHA256"]:
        raise ValueError("MoltenVK library does not match the pinned SHA-256")
    return data


def install(pin, destination, cache, archive=None, offline=False):
    if matches(destination, pin["libraryBytes"], pin["librarySHA256"]):
        return
    cache.mkdir(parents=True, exist_ok=True)
    cached = cache / (pin["sha256"] + ".tar")
    if archive is not None:
        data = library_bytes(archive, pin)
    elif matches(cached, pin["bytes"], pin["sha256"]):
        data = library_bytes(cached, pin)
    else:
        if offline:
            raise ValueError("Pinned MoltenVK is not cached. Connect to the network and rebuild.")
        with tempfile.TemporaryDirectory(prefix="download-", dir=cache) as temporary:
            download = Path(temporary) / "MoltenVK.tar"
            subprocess.run(["/usr/bin/curl", "--fail", "--location", "--proto", "=https",
                            "--proto-redir", "=https", "--connect-timeout", "10", "--max-time", "180",
                            "--max-filesize", str(pin["bytes"]),
                            "--silent", "--show-error", "--output", str(download), pin["url"]], check=True)
            data = library_bytes(download, pin)
            os.replace(download, cached)
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Unique temporary files make concurrent builds safe; every writer installs
    # the same validated bytes. Never truncate a library that a linker is reading.
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=destination.parent, prefix=".MoltenVK-", delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
        temporary.chmod(0o755)
        os.replace(temporary, destination)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, help="use a previously downloaded pinned archive")
    parser.add_argument("--offline", action="store_true")
    parser.add_argument("--output", type=Path, default=DESTINATION)
    parser.add_argument("--cache", type=Path, default=Path(os.environ.get(
        "BOXEDWINE_MOLTENVK_CACHE", str(Path.home() / "Library/Caches/BoxedwineBuild/moltenvk"))))
    args = parser.parse_args()
    try:
        pin = json.loads(LOCK.read_text())
        install(pin, args.output, args.cache, args.archive, args.offline)
    except (OSError, ValueError, tarfile.TarError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"error: {error}\n")
    print(f"MoltenVK {pin['version']}: verified {args.output}")


if __name__ == "__main__":
    main()
