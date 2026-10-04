#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build native Ubuntu executables and a .deb using Podman or Docker on Linux."""
import argparse
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import tarfile
import tempfile

LINUX = Path(__file__).resolve().parent
REPO = LINUX.parent.parent
ARCHITECTURES = {'x86_64': ('amd64', 'x64'), 'aarch64': ('arm64', 'arm64')}
SOURCE_PATHS = (
    'include', 'source', 'platform', 'lib', 'resources', 'license.txt',
    'tools/demo_catalog.py', 'tools/opengl', 'tools/x11', 'project/linux',
    'project/mac-xcode/Boxedwine/BoxedwineUI/Resources',
    'project/mac-xcode/Boxedwine/Media.xcassets/AppIcon.appiconset/icon_256x256.png',
)
EXCLUDED = {'Build', 'Deploy', 'linux_build', 'msvc-remote', 'automation',
            '__pycache__', '.git', '.codex', '.agents'}


def source_filter(member):
    parts = Path(member.name).parts
    if any(part in EXCLUDED for part in parts) or member.name.endswith(('.pyc', '.o', '.d')):
        return None
    return member


def source_archive(destination):
    # Send only build inputs. Never mount/relabel the Jenkins checkout, include
    # its Git credentials, or accidentally reuse binaries built on the host OS.
    with tarfile.open(destination, 'w') as archive:
        for relative in SOURCE_PATHS:
            archive.add(REPO / relative, arcname=relative, filter=source_filter)


def select_engine(requested=None):
    candidates = [requested] if requested else ['podman', 'docker']
    for candidate in candidates:
        if shutil.which(candidate):
            return candidate
    raise ValueError('Install Podman or Docker on the build worker and make it usable by the Jenkins account. '
                     'On Fedora Asahi: sudo dnf install podman; then verify podman info as that account.')


def run_command(engine, image, exported, architecture, revision, jobs):
    command = [engine, 'run', '--rm', '-i', '--platform', 'linux/' + architecture,
               '--user', f'{os.getuid()}:{os.getgid()}']
    if engine == 'podman':
        command += ['--userns=keep-id']
    # Only this temporary output directory is mounted/relabelled for SELinux.
    command += ['--volume', f'{exported}:/output:Z', image, 'bash', '-euc',
                'mkdir /tmp/boxedwine; tar --no-same-owner -xf - -C /tmp/boxedwine; '
                'exec bash /tmp/boxedwine/project/linux/packaging/build-in-container.sh "$@"',
                'boxedwine-build', architecture, revision, str(jobs)]
    return command


def build(engine, architecture, revision, jobs):
    native = ARCHITECTURES.get(platform.machine())
    if native is None or native[0] != architecture:
        raise ValueError('Run this build on a Linux machine matching the requested CPU architecture')
    if not re.fullmatch(r'[0-9][A-Za-z0-9.+~]*', revision):
        raise ValueError('Invalid Debian revision')
    if jobs < 1:
        raise ValueError('JOBS must be positive')
    # Fail before creating an image or compiling if the Jenkins account cannot
    # access the engine. In particular, do not silently fall back to host builds.
    subprocess.run([engine, 'info'], check=True, stdout=subprocess.DEVNULL)
    build_dir = LINUX / 'Build'
    build_dir.mkdir(exist_ok=True)
    destination = build_dir / 'Deploy/Linux' / native[1]
    with tempfile.TemporaryDirectory(prefix='deb-container-', dir=build_dir) as temporary:
        work = Path(temporary)
        image_id = work / 'image-id'
        subprocess.run([engine, 'build', '--platform', 'linux/' + architecture,
                        '--iidfile', str(image_id), '-f', str(LINUX / 'packaging/Dockerfile'),
                        str(LINUX / 'packaging')], check=True)
        image = image_id.read_text().strip()
        exported = work / 'output'; exported.mkdir()
        archive = work / 'source.tar'
        source_archive(archive)
        with archive.open('rb') as stream:
            subprocess.run(run_command(engine, image, exported, architecture, revision, jobs),
                           stdin=stream, check=True)
        if len(list(exported.glob(f'boxedwine_*_{architecture}.deb'))) != 1:
            raise ValueError('Container did not export exactly one package for ' + architecture)
        for relative in ('portable/boxedwine-ui', 'portable/Runtime/boxedwine-engine',
                         'portable/CommandLine/boxedwine'):
            if not (exported / relative).is_file():
                raise ValueError('Incomplete container output: missing ' + relative)
        # Publish only complete builds; preserve previous artifacts on failure.
        destination.parent.mkdir(parents=True, exist_ok=True)
        previous = work / 'previous'
        if destination.exists():
            destination.rename(previous)
        try:
            exported.rename(destination)
        except OSError:
            if previous.exists():
                previous.rename(destination)
            raise
    print('Built ' + str(destination), flush=True)
    return destination


def main():
    native = ARCHITECTURES.get(platform.machine())
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture', choices=('amd64', 'arm64'), default=native[0] if native else None)
    parser.add_argument('--engine', choices=('podman', 'docker'), default=os.environ.get('BOXEDWINE_CONTAINER_ENGINE'))
    parser.add_argument('--revision', default='1')
    parser.add_argument('--jobs', type=int, default=8)
    args = parser.parse_args()
    try:
        build(select_engine(args.engine), args.architecture, args.revision, args.jobs)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Container build failed: {error}\n')


if __name__ == '__main__':
    main()
