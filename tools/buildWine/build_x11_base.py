#!/usr/bin/env python3
"""Add Boxedwine's gamma bridge to the pinned media base, retaining its bytes."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import zipfile

import build_media_base as base

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
BASE_URL = 'https://boxedwine.org/v2/28/TinyCore15WineBase.zip'
BASE_SHA = 'bb7bf1807721deafaf98dff10091f27b21e5471d357c7b07330b84cc299aee83'
LIBRARY = 'usr/local/lib/libXxf86vm.so.1'
DOCS = 'usr/local/share/doc/boxedwine-xf86vm/'


def build_shim(root):
    source = REPO / 'tools/x11'
    target = root / LIBRARY
    target.parent.mkdir(parents=True, exist_ok=True)
    command = ['gcc', '-Wall', '-Werror', '-Wno-return-type', '-fPIC', '-m32',
               '-march=i586', '-shared', '-Wl,-soname,libXxf86vm.so.1',
               'Xxf86vm.c', '-o', str(target), '-lX11']
    base.run(command, cwd=source)
    docs = root / DOCS
    docs.mkdir(parents=True)
    sources = {}
    for name in ('Xxf86vm.c', 'X11_def.h'):
        shutil.copy2(source / name, docs / name)
        sources[name] = base.sha(docs / name)
    shutil.copy2('/usr/share/common-licenses/GPL-2', docs / 'COPYING')
    shutil.copy2(Path(__file__), docs / 'build_x11_base.py')
    metadata = {'sources': sources, 'sha256': base.sha(target),
                'command': command, 'license': 'GPL-2.0-or-later',
                'compiler': subprocess.check_output(['gcc', '--version'], text=True).splitlines()[0]}
    base.put(root, DOCS + 'build.json', (json.dumps(metadata, indent=2) + '\n').encode())
    return metadata


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--base', type=Path, required=True)
    parser.add_argument('--boxedwine', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    base.fetch(args.base, BASE_URL, BASE_SHA)
    root = work / 'x11-overlay'
    if root.exists():
        raise RuntimeError(f'Use a fresh overlay directory: {root}')
    root.mkdir()
    shim = build_shim(root)
    metadata = {'revision': '31', 'base_url': BASE_URL, 'base_sha256': BASE_SHA,
                'library': LIBRARY, 'gamma_bridge': shim,
                'media_sources_url': 'https://boxedwine.org/v2/28/TinyCore15WineBase-media-sources.zip'}
    base.put(root, 'base-build.json', (json.dumps(metadata, indent=2) + '\n').encode())
    with zipfile.ZipFile(args.base) as old:
        instructions = old.read('recreateInstructions.txt')
    base.put(root, 'recreateInstructions.txt', instructions +
             b'\nX11 gamma update: tools/buildWine/build_x11_base.py; see base-build.json and usr/local/share/doc/boxedwine-xf86vm for sources, license and build flags.\n')
    # Match the existing X11 libraries' location and refresh the guest loader cache.
    base.put(root, 'x11-ldconfig.sh', b'/sbin/ldconfig\nprintf "%s" "$?" > /x11-ldconfig.status\n')
    with (work / 'ldconfig.log').open('w') as log:
        result = subprocess.run(list(map(str, [args.boxedwine.resolve(), '-root', root,
            '-zip', args.base.resolve(), '-uid', '0', '/bin/sh', '/x11-ldconfig.sh'])),
            stdout=log, stderr=subprocess.STDOUT, timeout=120)
    status = root / 'x11-ldconfig.status'
    if result.returncode not in (0, 1) or not status.exists() or status.read_text() != '0':
        raise RuntimeError('Guest ldconfig failed; inspect ldconfig.log')
    status.unlink()
    (root / 'x11-ldconfig.sh').unlink()
    (root / 'etc/ld.so.cache~').unlink(missing_ok=True)
    if ('/' + LIBRARY).encode() not in (root / 'etc/ld.so.cache').read_bytes():
        raise RuntimeError('Guest ldconfig did not cache the gamma library')
    output = work / 'TinyCore15WineBase.zip'
    base.package(args.base, root, output)
    result = {'output': str(output), 'size': output.stat().st_size, 'sha256': base.sha(output), **metadata}
    (work / 'x11-base-result.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
