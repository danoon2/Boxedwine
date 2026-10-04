#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Package a staged Linux UI and CLI on a matching Debian-family build host."""
import argparse
from email.utils import formatdate
import gzip
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time

LINUX = Path(__file__).resolve().parent
REPO = LINUX.parent.parent
ARCHITECTURES = {'amd64': 62, 'arm64': 183}  # ELF e_machine
MAINTAINER = 'James Bryant <jwbryant72@yahoo.com>'
UI_DEPENDENCIES = ('python3 (>= 3.10)', 'python3-gi', 'gir1.2-gtk-4.0 (>= 4.14)',
                   'gir1.2-adw-1 (>= 1.5)', 'ca-certificates', 'hicolor-icon-theme',
                   'libgl1', 'libx11-6', 'libvulkan1')


def output(command, **kwargs):
    return subprocess.check_output(command, text=True, **kwargs).strip()


def upstream_version():
    text = (REPO / 'include/boxedwine.h').read_text()
    return re.search(r'#define BOXEDWINE_VERSION_DISPLAY "([^"]+)"', text)[1]


def check_binary(path, architecture):
    if not path.is_file() or not os.access(path, os.X_OK):
        raise ValueError(f'Missing executable: {path}; stage with ui/build.py --console first')
    with path.open('rb') as stream:
        header = stream.read(64)
    if (len(header) != 64 or header[:6] != b'\x7fELF\x02\x01'
            or int.from_bytes(header[18:20], 'little') != ARCHITECTURES[architecture]):
        raise ValueError(f'{path} is not a 64-bit little-endian {architecture} ELF binary')


def shared_dependencies(binaries, workspace):
    # dpkg-shlibdeps requires source-package metadata even when creating a .deb
    # directly. Resolve against this worker's installed libraries, never guess
    # SONAME-to-package mappings or silently omit missing dependencies.
    debian = workspace / 'debian'
    debian.mkdir(exist_ok=True)
    (debian / 'control').write_text(
        f'Source: boxedwine\nSection: otherosfs\nPriority: optional\nMaintainer: {MAINTAINER}\n\n'
        'Package: boxedwine\nArchitecture: any\nDescription: Windows application emulator\n')
    result = output(['dpkg-shlibdeps', '-O', *['-e' + str(p) for p in binaries]], cwd=workspace)
    for line in result.splitlines():
        if line.startswith('shlibs:Depends=') and line.partition('=')[2]:
            return line.partition('=')[2]
    raise ValueError('dpkg-shlibdeps did not produce native runtime dependencies')


def write_documentation(destination, version):
    destination.mkdir(parents=True)
    (destination / 'copyright').write_text(
        'Boxedwine\nhttps://github.com/danoon2/Boxedwine\n\n'
        'Copyright (C) The Boxedwine Team and contributors.\n'
        'Boxedwine is licensed under GPL-2.0-or-later.\n'
        'The complete GPL version 2 text is in /usr/share/common-licenses/GPL-2.\n'
        'Third-party notices are included in the licenses directory.\n'
        'Wine icon notices are also included under /usr/lib/boxedwine/ui/resources/AppIcons.\n')
    licenses = destination / 'licenses'; licenses.mkdir()
    for component, filename in (
            ('asmjit', 'LICENSE.md'), ('softfloat', 'COPYING.txt'), ('glew', 'LICENSE.txt'),
            ('imgui', 'LICENSE.txt'), ('pugixml', 'LICENSE.md'), ('simde', 'COPYING'),
            ('tiny-process', 'LICENSE')):
        shutil.copy2(REPO / 'lib' / component / filename, licenses / (component + '.txt'))
    # These bundled components keep their notice in their source header.
    for component, filename in (('zlib', 'lib/zlib/zlib.h'),
                                ('minizip', 'lib/zlib/contrib/minizip/unzip.h'),
                                ('tinyfiledialogs', 'lib/imgui/addon/imguitinyfiledialogs.cpp')):
        source = (REPO / filename).read_text()
        (licenses / (component + '.txt')).write_text(source[:source.index('*/') + 2] + '\n')
    date = formatdate(int(os.environ.get('SOURCE_DATE_EPOCH', time.time())), localtime=False)
    changelog = (f'boxedwine ({version}) unstable; urgency=medium\n\n'
                 '  * Package the native Linux UI, private runtime and standalone command.\n\n'
                 f' -- {MAINTAINER}  {date}\n')
    (destination / 'changelog.Debian.gz').write_bytes(gzip.compress(changelog.encode(), mtime=0))


def build_package(stage, destination, architecture, version):
    stage, destination = Path(stage).resolve(), Path(destination).resolve()
    if architecture not in ARCHITECTURES:
        raise ValueError('Supported Debian architectures: amd64, arm64')
    if output(['dpkg', '--print-architecture']) != architecture:
        raise ValueError(f'Build the {architecture} package on a matching Debian/Ubuntu worker')
    # Also constrain filenames/control data; dpkg alone accepts colons (epochs).
    if not re.fullmatch(r'[0-9][A-Za-z0-9.+~\-]*', version):
        raise ValueError('Invalid package version (use a numeric version without an epoch)')
    subprocess.run(['dpkg', '--validate-version', version], check=True)
    binaries = [stage / 'Runtime/boxedwine-engine', stage / 'CommandLine/boxedwine']
    for binary in binaries:
        check_binary(binary, architecture)
    for relative in ('boxedwine-ui', 'ui/boxedwine/app.py', 'ui/resources/org.boxedwine.Boxedwine.png',
                     'org.boxedwine.Boxedwine.desktop', 'README.md'):
        if not (stage / relative).is_file():
            raise ValueError(f'Incomplete UI stage: missing {relative}')
    destination.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.boxedwine-deb-', dir=destination) as temporary:
        workspace = Path(temporary)
        root = workspace / 'debian/boxedwine'
        install = root / 'usr/lib/boxedwine'
        install.mkdir(parents=True)
        control = root / 'DEBIAN'; control.mkdir(mode=0o755)
        for name in ('ui', 'Runtime', 'CommandLine'):
            shutil.copytree(stage / name, install / name,
                            ignore=shutil.ignore_patterns('__pycache__', '*.pyc'))
        dependencies = shared_dependencies([install / binary.relative_to(stage) for binary in binaries], workspace)
        launcher = install / 'boxedwine-ui'
        launcher.write_text((stage / 'boxedwine-ui').read_text().replace(
            '#!/usr/bin/env python3', '#!/usr/bin/python3', 1))
        launcher.chmod(0o755)
        bin_dir = root / 'usr/bin'; bin_dir.mkdir()
        # The UI resolves its own real path, so both symlinks preserve lookup.
        (bin_dir / 'boxedwine-ui').symlink_to('../lib/boxedwine/boxedwine-ui')
        (bin_dir / 'boxedwine').symlink_to('../lib/boxedwine/CommandLine/boxedwine')
        desktop = root / 'usr/share/applications'; desktop.mkdir(parents=True)
        shutil.copy2(stage / 'org.boxedwine.Boxedwine.desktop', desktop)
        icons = root / 'usr/share/icons/hicolor/256x256/apps'; icons.mkdir(parents=True)
        shutil.copy2(stage / 'ui/resources/org.boxedwine.Boxedwine.png', icons)
        docs = root / 'usr/share/doc/boxedwine'
        write_documentation(docs, version)
        shutil.copy2(stage / 'README.md', docs / 'README.md')
        # Normalize permissions instead of inheriting a CI worker's umask.
        files = sorted(p for p in root.rglob('*') if not p.is_symlink() and p.is_file())
        for path in root.rglob('*'):
            if not path.is_symlink():
                path.chmod(0o755 if path.is_dir() or os.access(path, os.X_OK) else 0o644)
        root.chmod(0o755)
        installed_size = sum((p.stat().st_size + 1023) // 1024 for p in files)
        depends = ', '.join(dict.fromkeys((*UI_DEPENDENCIES, *dependencies.split(', '))))
        (control / 'control').write_text(
            f'Package: boxedwine\nVersion: {version}\nArchitecture: {architecture}\n'
            f'Maintainer: {MAINTAINER}\nSection: otherosfs\nPriority: optional\n'
            f'Installed-Size: {installed_size}\nHomepage: https://www.boxedwine.org/\n'
            f'Depends: {depends}\n'
            'Description: Run Windows applications in an emulated environment\n'
            ' Boxedwine includes a native GTK 4/libadwaita application library,\n'
            ' its emulator runtime, and a standalone boxedwine command.\n'
            ' Wine environments and applications are stored in your home directory.\n')
        (control / 'md5sums').write_text(''.join(
            hashlib.md5(p.read_bytes()).hexdigest() + '  ' + p.relative_to(root).as_posix() + '\n'
            for p in files))
        for path in control.iterdir():
            path.chmod(0o644)
        filename = f'boxedwine_{version}_{architecture}.deb'
        subprocess.run(['dpkg-deb', '--root-owner-group', '-Zxz', '--build', str(root),
                        str(workspace / filename)], check=True)
        # A failed build leaves any previous successful artifact untouched.
        os.replace(workspace / filename, destination / filename)
    return destination / filename


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stage', type=Path, default=LINUX / 'Build/NativeUI')
    parser.add_argument('--output', type=Path, default=LINUX / 'Build/Packages')
    parser.add_argument('--architecture', choices=ARCHITECTURES)
    parser.add_argument('--version', help='Full Debian version; default: display version plus revision')
    parser.add_argument('--revision', default='1', help='Debian revision (CI uses 0~ciBUILD_NUMBER)')
    args = parser.parse_args()
    try:
        architecture = args.architecture or output(['dpkg', '--print-architecture'])
        version = args.version or upstream_version() + '-' + args.revision
        print(build_package(args.stage, args.output, architecture, version))
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Debian packaging failed: {error}\n')


if __name__ == '__main__':
    main()
