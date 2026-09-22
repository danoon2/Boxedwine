#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Install the pinned SDL 2 Mac framework, preserving an existing install on failure."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from fetch_moltenvk import digest, matches

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "resources/sdl-mac.lock.json"
DESTINATION = ROOT / "lib/mac/SDL2.framework"


def framework_digest(framework):
    """Hash paths, symlink targets and file bytes; ignore signing xattrs/timestamps."""
    value = hashlib.sha256()
    for path in sorted(framework.rglob("*")):
        name = path.relative_to(framework).as_posix()
        if path.is_symlink():
            entry = [name, "link", os.readlink(path)]
        elif path.is_file():
            entry = [name, "file", digest(path)]
        else:
            continue
        value.update((json.dumps(entry, separators=(",", ":")) + "\n").encode())
    return value.hexdigest()


def stage(archive, pin, staging):
    if not matches(archive, pin["bytes"], pin["sha256"]):
        raise ValueError("SDL archive does not match the pinned size and SHA-256")
    with tempfile.TemporaryDirectory(prefix="boxedwine-sdl-mount-") as temporary:
        mount = Path(temporary) / "volume"
        subprocess.run(["/usr/bin/hdiutil", "attach", "-readonly", "-nobrowse", "-quiet",
                        "-mountpoint", str(mount), str(archive.resolve())], check=True)
        try:
            shutil.copytree(mount / "SDL2.framework", staging, symlinks=True)
        finally:
            subprocess.run(["/usr/bin/hdiutil", "detach", "-quiet", str(mount)], check=True)
    if framework_digest(staging) != pin["frameworkTreeSHA256"]:
        raise ValueError("SDL framework does not match the pinned SHA-256")


def install(pin, destination, cache, archive=None, offline=False):
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Serialize framework directory replacements across simultaneous builds.
    with (destination.parent / ".sdl-install.lock").open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        if destination.is_dir() and framework_digest(destination) == pin["frameworkTreeSHA256"]:
            return
        cache.mkdir(parents=True, exist_ok=True)
        cached = cache / (pin["sha256"] + ".dmg")
        with tempfile.TemporaryDirectory(prefix=".sdl-", dir=destination.parent) as temporary:
            temporary = Path(temporary)
            if archive is None:
                if not matches(cached, pin["bytes"], pin["sha256"]):
                    if offline:
                        raise ValueError("Pinned SDL is not cached. Connect to the network and rebuild.")
                    download = temporary / "SDL.dmg"
                    subprocess.run(["/usr/bin/curl", "--fail", "--location", "--proto", "=https",
                                    "--proto-redir", "=https", "--connect-timeout", "10", "--max-time", "180",
                                    "--max-filesize", str(pin["bytes"]), "--silent", "--show-error",
                                    "--output", str(download), pin["url"]], check=True)
                    if not matches(download, pin["bytes"], pin["sha256"]):
                        raise ValueError("SDL archive does not match the pinned size and SHA-256")
                    # The cache may be on another filesystem.
                    with tempfile.NamedTemporaryFile(dir=cache, delete=False) as stream:
                        pending = Path(stream.name)
                    try:
                        shutil.copyfile(download, pending)
                        os.replace(pending, cached)
                    finally:
                        pending.unlink(missing_ok=True)
                archive = cached
            staging = temporary / "SDL2.framework"
            stage(archive, pin, staging)
            previous = temporary / "previous.framework"
            if destination.exists():
                os.replace(destination, previous)
            try:
                os.replace(staging, destination)
            except OSError:
                if previous.exists():
                    os.replace(previous, destination)
                raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, help="use a previously downloaded pinned disk image")
    parser.add_argument("--offline", action="store_true")
    parser.add_argument("--output", type=Path, default=DESTINATION)
    parser.add_argument("--cache", type=Path, default=Path(os.environ.get(
        "BOXEDWINE_SDL_CACHE", str(Path.home() / "Library/Caches/BoxedwineBuild/sdl"))))
    args = parser.parse_args()
    try:
        pin = json.loads(LOCK.read_text())
        install(pin, args.output, args.cache, args.archive, args.offline)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"error: {error}\n")
    print(f"SDL {pin['version']}: verified {args.output}")


if __name__ == "__main__":
    main()
