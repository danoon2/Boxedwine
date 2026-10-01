#!/usr/bin/env python3
"""Build a complete Wine filesystem from pinned inputs; an old full ZIP is optional QA only."""
from __future__ import annotations

import argparse
import copy
import hashlib
from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import threading
import time
import urllib.parse
import urllib.request
import uuid
import zipfile

import build_wine
import webgl_filesystem
import web_runtime

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
PREFIX = 'home/username/.wine/'
DRIVE = PREFIX + 'drive_c/'


def select_variant(profile, variant):
    profile = copy.deepcopy(profile)
    if variant == 'web':
        profile.update(variant='web', gecko=None, dxvk=None)
    return profile


def is_web(profile):
    return profile.get('variant') == 'web'


def output_name(profile):
    return 'TinyCore15Wine11.0-web.zip' if is_web(profile) else 'TinyCore15Wine11.0.zip'


def wine_config(profile):
    config = build_wine.load_config(HERE / profile['wine_config'])
    if is_web(profile):
        args = config['build']['configure_args']
        config['build']['configure_args'] = [a for a in args if a not in ('--with-vulkan', '--without-vulkan')]
        config['build']['configure_args'] += ['--without-vulkan', '--disable-winevulkan']
        config['required_config_checks'] = [c for c in config['required_config_checks']
                                           if c['checking'] != 'checking for -lvulkan']
    return config


def run(command, **kwargs):
    print('+', build_wine.command_text(command), flush=True)
    return subprocess.run([str(x) for x in command], check=True, **kwargs)


def digest(path):
    return build_wine.file_sha256(path)


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + '\n', encoding='utf-8')


def fresh_directory(work, name):
    path = work / name
    if path.is_symlink():
        raise build_wine.BuildError(f'Refusing to replace symlinked build directory: {path}')
    if path.exists():
        path.rename(work / (name + '-previous-' + uuid.uuid4().hex[:8]))
    path.mkdir()
    return path


def checked_name(name):
    if not name or '\\' in name or name.startswith('/') or any(p in ('', '.', '..') for p in name.rstrip('/').split('/')):
        raise build_wine.BuildError(f'Unsafe archive member: {name!r}')
    return name


def download(spec, cache):
    name = build_wine._safe_relative_name(spec['filename'], 'download filename', 'file name')
    expected = spec['sha256']
    if not isinstance(expected, str) or not re.fullmatch('[0-9a-f]{64}', expected):
        raise build_wine.BuildError(f'Missing SHA-256 pin for {name}')
    cache.mkdir(parents=True, exist_ok=True)
    path = cache / name
    def valid(candidate):
        return candidate.is_file() and candidate.stat().st_size == spec['size'] and digest(candidate) == expected
    if valid(path):
        return path
    with tempfile.NamedTemporaryFile(dir=cache, prefix=name + '.', delete=False) as stream:
        temporary = Path(stream.name)
    try:
        print('Downloading', spec['url'], flush=True)
        request = urllib.request.Request(spec['url'], headers={'User-Agent': 'Boxedwine-filesystem-builder'})
        with urllib.request.urlopen(request, timeout=120) as response, temporary.open('wb') as stream:
            shutil.copyfileobj(response, stream)
        if not valid(temporary):
            raise build_wine.BuildError(f'Download does not match size/SHA-256 pin: {name}')
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)
    return path


def source_checkout(spec, destination):
    if not destination.exists():
        run(['git', 'clone', '--no-checkout', spec['repo'], destination])
        run(['git', '-C', destination, 'checkout', '--detach', spec['commit']])
    actual = subprocess.check_output(['git', '-C', str(destination), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != spec['commit']:
        raise build_wine.BuildError(f'Wrong source revision in {destination}: {actual}')
    return destination


def install_selected_zip(archive, destination, predicate):
    with zipfile.ZipFile(archive) as source:
        for entry in source.infolist():
            checked_name(entry.filename)
            if entry.is_dir() or not predicate(entry.filename):
                continue
            target = destination / entry.filename
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(source.read(entry))


def prepare_toolchain(profile, work):
    spec = profile['llvm_mingw']
    archive = download(spec, work / 'downloads')
    toolchain = work / 'toolchains' / spec['directory']
    stamp = toolchain / '.archive-sha256'
    if not stamp.is_file() or stamp.read_text().strip() != spec['sha256']:
        toolchain.parent.mkdir(parents=True, exist_ok=True)
        with tarfile.open(archive) as source:
            source.extractall(toolchain.parent, filter='data')
        stamp.write_text(spec['sha256'] + '\n')
    return toolchain


def psvoodoo_build_inputs(spec, toolchain):
    return {'base_revision': spec['commit'], 'd3d9_only': True,
            'toolchain_sha256': (toolchain / '.archive-sha256').read_text().strip(),
            'patches': [{'name': Path(name).name, 'sha256': digest(HERE / name)}
                        for name in spec.get('patches', [])]}


def build_psvoodoo(spec, source, toolchain, work):
    inputs = psvoodoo_build_inputs(spec, toolchain)
    output = work / 'psvoodoo-build'
    manifest = output / 'build.json'
    metadata = json.loads(manifest.read_text()) if manifest.is_file() else {}
    if metadata.get('build_inputs') != inputs:
        if output.exists():
            output.rename(work / ('psvoodoo-previous-' + uuid.uuid4().hex[:8]))
        # Upstream's builder requires committed sources. Make a private snapshot
        # from the pinned tree, never commit to or modify the source checkout.
        snapshot = fresh_directory(work, 'psvoodoo-patched-source')
        with tempfile.TemporaryFile() as stream:
            run(['git', '-C', source, 'archive', spec['commit']], stdout=stream)
            stream.seek(0)
            with tarfile.open(fileobj=stream) as archive:
                archive.extractall(snapshot, filter='data')
        run(['git', 'init', '--quiet', snapshot])
        run(['git', '-C', snapshot, 'config', 'core.autocrlf', 'false'])
        for name in spec.get('patches', []):
            run(['git', '-C', snapshot, 'apply', '--whitespace=nowarn', (HERE / name).resolve()])
        run(['git', '-C', snapshot, 'add', '--all'])
        environment = dict(os.environ, GIT_AUTHOR_NAME='Boxedwine filesystem builder',
                           GIT_AUTHOR_EMAIL='builder@boxedwine.invalid',
                           GIT_COMMITTER_NAME='Boxedwine filesystem builder',
                           GIT_COMMITTER_EMAIL='builder@boxedwine.invalid',
                           GIT_AUTHOR_DATE='2000-01-01T00:00:00+0000',
                           GIT_COMMITTER_DATE='2000-01-01T00:00:00+0000')
        run(['git', '-C', snapshot, '-c', 'core.hooksPath=/dev/null', 'commit',
             '--quiet', '--no-gpg-sign', '-m', 'Boxedwine build snapshot'], env=environment)
        sdk_aliases = work / 'psvoodoo-sdk-includes'
        sdk_aliases.mkdir(exist_ok=True)
        for name in ('D3D9.h', 'D3DX9.h'):
            (sdk_aliases / name).write_text(f'#include <{name.lower()}>\n')
        environment = dict(os.environ, CPATH=str(sdk_aliases))
        run([sys.executable, snapshot / 'tools/build.py', '--toolchain', toolchain / 'bin',
             '--output', output, '--d3d9-only'], env=environment)
        metadata = json.loads(manifest.read_text())
        # "revision" identifies the local snapshot, not an upstream commit.
        metadata['build_inputs'] = inputs
        write_json(manifest, metadata)
    if digest(output / 'glide2x.dll') != metadata['dll_sha256']:
        raise build_wine.BuildError('psVoodoo output does not match its build manifest')
    return output


def archive_psvoodoo_source(output, destination):
    metadata = json.loads((output / 'build.json').read_text())
    with tarfile.open(destination, 'w:gz') as archive:
        for name, expected in sorted(metadata['source_manifest'].items()):
            checked_name(name)
            path = output / 'source' / name
            if digest(path) != expected:
                raise build_wine.BuildError(f'psVoodoo source does not match its build manifest: {name}')
            archive.add(path, arcname='psVoodoo/' + name, recursive=False)


def build_addons(profile, work, wine_repository, jobs):
    overlay = fresh_directory(work, 'addon-overlay')
    sources = work / 'sources'
    sources.mkdir(exist_ok=True)
    downloads = work / 'downloads'
    toolchain = prepare_toolchain(profile, work)
    if profile.get('gecko'):
        download(profile['gecko'], downloads)

    if profile.get('dxvk'):
        dxvk = profile['dxvk']
        destination = overlay / DRIVE / 'dxvk'
        destination.mkdir(parents=True, exist_ok=True)
        with tarfile.open(download(dxvk, downloads)) as source:
            for name in ('d3d8.dll', 'd3d9.dll', 'd3d10core.dll', 'd3d11.dll', 'dxgi.dll'):
                member = source.extractfile(f'dxvk-{dxvk["version"]}/x32/{name}')
                if member is None:
                    raise build_wine.BuildError(f'Missing DXVK {name}')
                (destination / name).write_bytes(member.read())
        (destination / 'version.txt').write_bytes(f'dxvk {dxvk["version"]}\r\nhttps://github.com/doitsujin/dxvk'.encode())

    cnc = profile['cnc_ddraw']
    source = source_checkout(cnc, sources / 'cnc-ddraw')
    patch_source = (HERE / cnc['patch']).resolve()
    patch = work / 'cnc-ddraw-opengl.patch'
    patch.write_bytes(build_wine._normalize_patch_line_endings(patch_source.read_bytes()))
    applied = subprocess.run(['git', '-C', str(source), 'apply', '--reverse', '--check', str(patch)], capture_output=True).returncode == 0
    if not applied:
        run(['git', '-C', source, 'apply', patch])
    cnc_output = work / 'cnc-ddraw-build'
    run([sys.executable, REPO / 'tools/cnc-ddraw/build.py', '--source', source,
         '--toolchain', toolchain / 'bin', '--output', cnc_output])
    destination = overlay / DRIVE / 'ddraw'
    install_selected_zip(download(cnc, downloads), destination,
                         lambda n: n == 'ddraw.ini' or n.startswith('Shaders/'))
    shutil.copy2(cnc_output / 'ddraw.dll', destination / 'ddraw.dll')
    docs = overlay / 'usr/local/share/doc/cnc-ddraw'
    docs.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source / 'LICENSE', docs / 'LICENSE')
    shutil.copy2(patch, docs / patch.name)

    spec = profile['psvoodoo']
    source = source_checkout(spec, sources / 'psvoodoo')
    output = build_psvoodoo(spec, source, toolchain, work)
    destination = overlay / DRIVE / 'windows/system32/glide2x.dll'
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(output / 'glide2x.dll', destination)
    docs = overlay / 'usr/local/share/doc/psvoodoo'
    docs.mkdir(parents=True, exist_ok=True)
    for name in ('NOTICE.md', 'build.json'):
        shutil.copy2(output / name, docs / name)
    for name in ('licenses', 'docs'):
        shutil.copytree(output / name, docs / name, dirs_exist_ok=True)
    licenses = docs / 'licenses/llvm-mingw'
    licenses.mkdir(parents=True, exist_ok=True)
    shutil.copy2(toolchain / 'LICENSE.TXT', licenses / 'LLVM-LICENSE.TXT')
    for name in ('COPYING', 'COPYING.MinGW-w64-runtime.txt', 'COPYING.MinGW-w64.txt',
                 'COPYING.winpthreads.txt', 'COPYING.winstorecompat.txt'):
        shutil.copy2(toolchain / 'i686-w64-mingw32/share/mingw32' / name, licenses / name)
    for name, source_name in (('GPL-2.0.txt', 'GPL-2'), ('LGPL-2.1.txt', 'LGPL-2.1')):
        shutil.copy2(Path('/usr/share/common-licenses') / source_name, docs / 'licenses' / name)
    archive = docs / f'psVoodoo-{spec["commit"]}-boxedwine.tar.gz'
    archive_psvoodoo_source(output, archive)
    for name in spec.get('patches', []):
        patches = docs / 'patches'
        patches.mkdir(exist_ok=True)
        shutil.copy2(HERE / name, patches / Path(name).name)
    (docs / 'README.txt').write_text(
        f'psVoodoo for Boxedwine, base revision {spec["commit"]}\nSource: {spec["repo"]}\n'
        'Built with the LLVM-MinGW toolchain pinned in filesystem_wine11.json.\n'
        'The source archive contains the patched sources used to build this DLL.\n'
        'build.json records the upstream base, patch hashes, toolchain pin, and source hashes.\n'
        'Its revision is a local build snapshot, not an upstream commit.\n'
        'Rebuild with Boxedwine tools/buildWine/build_filesystem.py and filesystem_wine11.json.\n'
        'To use the standalone upstream builder, extract the source archive and initialize\n'
        'a local Git repository: git init; git add .; git commit -m "Build snapshot"\n'
        'Then run python3 tools/build.py --toolchain /path/to/llvm-mingw/bin --output /path/to/new-build --d3d9-only\n'
        'On Linux, supply D3D9.h and D3DX9.h include aliases as the filesystem builder does.\n'
        'Patch files, notices, and license texts are included here.\n')

    webgl_config = webgl_filesystem.load_config(HERE / profile['webgl_config'])
    patches = webgl_filesystem.verify_patch_series(webgl_config)
    webgl_work = work / 'webgl'
    dlls = webgl_work / 'wine-build/boxedwine-webgl-dlls'
    if not dlls.is_dir():
        dlls = webgl_filesystem.prepare_and_build(webgl_work, wine_repository, jobs,
                                                 webgl_config, patches, prepare_only=False)
    webgl_filesystem.validate_dll_directory(dlls, webgl_config)
    destination = overlay / DRIVE / 'webgl'
    destination.mkdir(parents=True, exist_ok=True)
    for name in webgl_config['dlls']:
        shutil.copy2(dlls / name, destination / name)
    if not is_web(profile):
        shutil.copy2(webgl_work / 'wine-build/boxedwine-webgl-tests/ddraw_test.exe', destination / 'ddraw_test.exe')
    write_json(work / 'addons-manifest.json', {'profile': profile, 'files': directory_inventory(overlay),
        'cnc_patch_sha256': digest(patch_source), 'webgl_patch_manifest': webgl_config['patch_manifest']})
    return overlay


def directory_inventory(root):
    return {p.relative_to(root).as_posix(): {'size': p.stat().st_size, 'sha256': digest(p)}
            for p in sorted(root.rglob('*')) if p.is_file()}


def zip_inventory(path):
    result = {}
    with zipfile.ZipFile(path) as archive:
        for entry in archive.infolist():
            checked_name(entry.filename)
            if entry.filename in result:
                raise build_wine.BuildError(f'Duplicate archive member: {entry.filename}')
            if not entry.is_dir():
                data = archive.read(entry)
                result[entry.filename] = {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
    return result


def compare_archives(reference, candidate):
    old, new = zip_inventory(reference), zip_inventory(candidate)
    old_directories, new_directories = zip_directories(reference), zip_directories(candidate)
    return {'reference': str(reference), 'candidate': str(candidate),
            'missing': sorted(old.keys() - new.keys()), 'added': sorted(new.keys() - old.keys()),
            'missing_directories': sorted(old_directories - new_directories),
            'added_directories': sorted(new_directories - old_directories),
            'changed': sorted(n for n in old.keys() & new.keys() if old[n] != new[n]),
            'identical_count': sum(old[n] == new[n] for n in old.keys() & new.keys())}


def zip_directories(path):
    directories = set()
    with zipfile.ZipFile(path) as archive:
        for entry in archive.infolist():
            parts = checked_name(entry.filename).rstrip('/').split('/')
            # Parent directories exist implicitly even without their own ZIP entry.
            for end in range(1, len(parts) + int(entry.is_dir())):
                directories.add('/'.join(parts[:end]) + '/')
    return directories


def combine_zip(source, overlay, destination, remove_prefix=None, exclude=None, compresslevel=None):
    # Wine's initialized prefix contains required empty directories, notably the
    # TEMP/TMP destination. Files alone cannot reconstruct those directories.
    entries = {}
    for path in overlay.rglob('*'):
        if path.is_file() or path.is_dir():
            name = path.relative_to(overlay).as_posix() + ('/' if path.is_dir() else '')
            if not build_wine.is_wine_development_file(name) and not (exclude and exclude(name)):
                entries[name] = path
    with tempfile.NamedTemporaryFile(dir=destination.parent, prefix=destination.name + '.', delete=False) as temp:
        temporary = Path(temp.name)
    try:
        with zipfile.ZipFile(source) as original, zipfile.ZipFile(temporary, 'w', compression=zipfile.ZIP_DEFLATED) as output:
            for entry in original.infolist():
                checked_name(entry.filename)
                if build_wine.is_wine_development_file(entry.filename) or (exclude and exclude(entry.filename)):
                    continue
                if entry.is_dir() and entry.filename in entries and entries[entry.filename].is_dir():
                    # Updating a child file need not change its existing parents.
                    output.writestr(entry, b'')
                    del entries[entry.filename]
                    continue
                if entry.filename in entries or (remove_prefix and entry.filename.startswith(remove_prefix)):
                    continue
                output.writestr(entry, original.read(entry), compresslevel=compresslevel)
            for name, path in sorted(entries.items()):
                output.write(path, name, compresslevel=compresslevel)
        temporary.replace(destination)
    finally:
        temporary.unlink(missing_ok=True)


def boxedwine_command(boxedwine, root, archive, log, command, env=()):
    args = [boxedwine, '-root', root, '-zip', archive, '-cacheReads', '-nosound',
            '-log', log, '-env', 'WINEDEBUG=-all', '-env', 'WINELOADERNOEXEC=1']
    for value in env:
        args += ['-env', value]
    return args + command


def remove_gecko_installer_cache(home, installer_sha256):
    # MSI chooses the cache filename. Match the pinned package's contents so
    # unrelated installers and the installed Gecko payload remain untouched.
    cache = home / 'drive_c/windows/Installer'
    removed = []
    for path in sorted(cache.glob('*')):
        if path.suffix.lower() == '.msi' and path.is_file() and digest(path) == installer_sha256:
            path.unlink()
            removed.append(path.relative_to(home).as_posix())
    return removed


def initialize_prefix(profile, work, boxedwine, runtime_zip):
    root = work / 'prefix-root'
    # This is a disposable prefix, never the user's existing prefix.
    if root.exists():
        if root.is_symlink() or not (root / 'home/username/initialize-prefix.sh').is_file():
            raise build_wine.BuildError(f'Refusing to move an unrecognized prefix directory: {root}')
        root.rename(work / ('prefix-previous-' + uuid.uuid4().hex[:8]))
    (root / 'home/username').mkdir(parents=True)
    guest_script = root / 'home/username/initialize-prefix.sh'
    guest_script.write_text(
        '#!/bin/sh\nexport WINEDLLOVERRIDES="mscoree,mshtml="\n'
        '/opt/wine/bin/wine wineboot.exe -u > /home/username/wineboot.txt 2>&1\n'
        'result=$?\n/opt/wine/bin/wineserver -w\n'
        # The first update may defer replacement of DLLs that are still loaded.
        # A restart processes those renames without forcing another update.
        'if [ "$result" -eq 0 ]; then\n'
        '  /opt/wine/bin/wine wineboot.exe -r > /home/username/prefix-finalize.txt 2>&1\n'
        '  result=$?\n  /opt/wine/bin/wineserver -w\nfi\n'
        'echo "$result" > /home/username/wineboot.exit\nexit "$result"\n')
    command = boxedwine_command(boxedwine, root, runtime_zip, work / 'wineboot.log',
                               ['/bin/sh', '/home/username/initialize-prefix.sh'])
    print('+', build_wine.command_text(command), flush=True)
    result = subprocess.run([str(x) for x in command], timeout=600)
    status = root / 'home/username/wineboot.exit'
    if result.returncode not in (0, 1) or not status.is_file() or status.read_text().strip() != '0':
        raise build_wine.BuildError('wineboot did not finish successfully; see wineboot.log and prefix-root/home/username/wineboot.txt')
    home = root / PREFIX
    for name in ('system.reg', 'user.reg', 'userdef.reg'):
        if not (home / name).is_file():
            raise build_wine.BuildError(f'wineboot did not create {name}')
    if ('"PendingFileRenameOperations"=' in (home / 'system.reg').read_text(errors='replace')
            or list((home / 'drive_c/windows/system32').glob('dll*.tmp'))):
        raise build_wine.BuildError('Wine prefix still has pending DLL replacements; see prefix-finalize.txt')
    build_wine.apply_window_manager_registry(home)
    if not profile.get('gecko'):
        return home
    installer = download(profile['gecko'], work / 'downloads')
    shutil.copy2(installer, root / 'home/username/gecko.msi')
    guest_script = root / 'home/username/install-gecko.sh'
    guest_script.write_text(
        '#!/bin/sh\nexport WINEDLLOVERRIDES="mscoree="\n'
        "/opt/wine/bin/wine msiexec.exe /i 'Z:\\home\\username\\gecko.msi' /qn /norestart "
        '> /home/username/gecko-install.txt 2>&1\nresult=$?\n'
        'if [ "$result" -eq 0 ]; then\n'
        '  /opt/wine/bin/wine regsvr32.exe /s mshtml.dll > /home/username/mshtml-register.txt 2>&1\n'
        '  result=$?\nfi\n'
        '/opt/wine/bin/wineserver -w\necho "$result" > /home/username/gecko.exit\nexit "$result"\n')
    command = boxedwine_command(boxedwine, root, runtime_zip, work / 'gecko-install.log',
                               ['/bin/sh', '/home/username/install-gecko.sh'])
    print('+', build_wine.command_text(command), flush=True)
    result = subprocess.run([str(x) for x in command], timeout=600)
    status = root / 'home/username/gecko.exit'
    if result.returncode not in (0, 1) or not status.is_file() or status.read_text().strip() != '0':
        raise build_wine.BuildError('Gecko MSI installation failed; see gecko-install.log')
    registry = (home / 'system.reg').read_text(errors='replace')
    if 'GeckoPath' not in registry or profile['gecko']['version'] not in registry:
        raise build_wine.BuildError('Gecko registration was not created')
    if not list((home / 'drive_c/windows/system32/gecko').rglob('xul.dll')):
        raise build_wine.BuildError('Gecko xul.dll was not installed')
    for name in remove_gecko_installer_cache(home, digest(installer)):
        print('Removed Gecko installer cache:', name, flush=True)
    return home


def filesystem_changes(profile):
    changes = (HERE / profile['changes_file']).read_text(encoding='utf-8')
    if not changes.startswith(f'v{profile["filesystem_version"]} '):
        raise build_wine.BuildError('Changelog does not start with the configured filesystem version')
    if is_web(profile):
        web_changes = (HERE / 'changes_wine11_web.txt').read_text(encoding='utf-8')
        if not web_changes.startswith(f'v{profile["filesystem_version"]} '):
            raise build_wine.BuildError('Web changelog does not start with the configured filesystem version')
        changes = web_changes.rstrip() + '\n\nFull file system history:\n\n' + changes
    return changes


def assemble(profile, work, boxedwine, compare_to=None):
    changes = filesystem_changes(profile)
    config = wine_config(profile)
    exclude = None
    if is_web(profile):
        stamp = work / 'wine-build-config.json'
        if not stamp.is_file() or json.loads(stamp.read_text()) != config:
            raise build_wine.BuildError('Build Wine for the web variant first; its configuration stamp must match')
        exclude = web_runtime.exclusions(config['base_filesystem']['sha256'])
        header = (work / 'wine-git/include/config.h').read_text()
        if re.search(r'^#define SONAME_LIBVULKAN\b', header, re.M):
            raise build_wine.BuildError('The web Wine build still enables Vulkan')
    wine_zip = work / 'Wine-11.0.zip'
    if not wine_zip.is_file() or not (work / 'tmp_install/opt/wine').is_dir():
        raise build_wine.BuildError('Build Wine first; its archive and install staging tree are required')
    actual = subprocess.check_output(['git', '-C', str(work / 'wine-git'), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != profile['wine_commit']:
        raise build_wine.BuildError(f'Wine source is not at the pinned commit: {actual}')
    overlay = work / 'addon-overlay'
    manifest = json.loads((work / 'addons-manifest.json').read_text())
    if manifest['profile'] != profile or manifest['files'] != directory_inventory(overlay):
        raise build_wine.BuildError('Addon files do not match their manifest; rebuild addons')
    empty = work / 'empty-overlay'
    empty.mkdir(exist_ok=True)
    runtime_zip = work / 'wine-uninitialized.zip'
    combine_zip(wine_zip, empty, runtime_zip, remove_prefix=PREFIX, exclude=exclude)
    home = initialize_prefix(profile, work, boxedwine, runtime_zip)
    # Preserve non-Wine payloads supplied by the base (e.g. Glide), then apply rebuilt addons.
    base_zip = build_wine.ensure_base_filesystem(config, work)
    with zipfile.ZipFile(base_zip) as base:
        for entry in base.infolist():
            if entry.filename.startswith(PREFIX) and not entry.is_dir() and not (exclude and exclude(entry.filename)):
                target = home / entry.filename[len(PREFIX):]
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(base.read(entry))
    prefix_overlay = overlay / PREFIX
    shutil.copytree(prefix_overlay, home, dirs_exist_ok=True)
    complete = fresh_directory(work, 'complete-overlay')
    shutil.copytree(overlay, complete, dirs_exist_ok=True)
    shutil.copytree(home, complete / PREFIX, dirs_exist_ok=True)
    (complete / 'version.txt').write_text(profile['filesystem_version'])
    (complete / 'changes.txt').write_text(changes, encoding='utf-8')
    if is_web(profile):
        web_runtime.write_metadata(complete, base_zip)
        (complete / 'name.txt').write_text('Wine 11.0 Web')
    write_json(complete / 'filesystem-build.json', {
        'profile': profile, 'wine_config': config,
        'base_sha256': digest(base_zip), 'wine_archive_sha256': digest(wine_zip),
        'boxedwine_sha256': digest(boxedwine), 'addons': manifest['files'],
        'wine_patches': {op.value: digest(HERE / 'patches' / op.value)
                         for op in build_wine.select_operations(config, (11, 0)) if op.kind == 'apply_patch'},
    })
    output = work / output_name(profile)
    candidate = output.with_suffix('.candidate.zip')
    combine_zip(runtime_zip, complete, candidate, exclude=exclude, compresslevel=9 if is_web(profile) else None)
    if is_web(profile):
        report = web_runtime.validate_archive(candidate)
        write_json(work / 'web-runtime-validation.json', report)
    inventory = zip_inventory(candidate)  # Read every member, including CRC validation.
    candidate.replace(output)
    write_json(work / 'filesystem-inventory.json', inventory)
    write_json(work / 'filesystem-build-result.json', {'output': str(output), 'sha256': digest(output),
               'size': output.stat().st_size, 'files': len(inventory), 'profile': profile})
    if compare_to:
        write_json(work / 'filesystem-comparison.json', compare_archives(compare_to, output))
    print('Created', output, flush=True)
    return output


def smoke_test(work, boxedwine, profile=None):
    profile = profile or {'gecko': True}
    root = work / ('smoke-root-' + uuid.uuid4().hex[:8])
    drive = root / DRIVE
    drive.mkdir(parents=True)
    run(['i686-w64-mingw32-gcc', '-O2', '-Wall', '-Wextra', HERE / 'probes/uri.c',
         '-o', drive / 'uri-probe.exe', '-lurlmon', '-lole32'])
    received = threading.Event()
    browser_result = {}
    token = '/' + uuid.uuid4().hex
    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            parsed = urllib.parse.urlsplit(self.path)
            if parsed.path != token:
                self.send_error(404)
                return
            browser_result.update({k: v[0] for k, v in urllib.parse.parse_qs(parsed.query).items()})
            self.send_response(204)
            self.end_headers()
            received.set()
        def log_message(self, *_args):
            pass
    server = HTTPServer(('127.0.0.1', 0), Handler)
    callback = f'http://127.0.0.1:{server.server_port}{token}'
    (drive / 'gecko-probe.html').write_text((HERE / 'probes/gecko.html').read_text().replace('__CALLBACK_URL__', callback))
    script = root / 'home/username/smoke.sh'
    overrides = 'mscoree=' if profile.get('gecko') else 'mscoree,mshtml='
    script.write_text(
        f'#!/bin/sh\nexport WINEDLLOVERRIDES="{overrides}"\n'
        'tempdir=/home/username/.wine/drive_c/users/username/AppData/Local/Temp\n'
        'test -d "$tempdir" && touch "$tempdir/boxedwine-temp-smoke" && rm "$tempdir/boxedwine-temp-smoke"\n'
        'echo "$?" > /home/username/temp-directory.exit\n'
        "/opt/wine/bin/wine 'C:\\uri-probe.exe' > /home/username/uri-smoke.txt 2>&1\n"
        'echo "$?" > /home/username/uri.exit\n'
        + ("/opt/wine/bin/wine iexplore.exe 'file://localhost/C:/gecko-probe.html' > /home/username/iexplore.txt 2>&1\n"
           if profile.get('gecko') else '/opt/wine/bin/wineserver -w\n'))
    command = boxedwine_command(boxedwine, root, work / output_name(profile),
                               work / 'filesystem-smoke.log', ['/bin/sh', '/home/username/smoke.sh'])
    print('+', build_wine.command_text(command), flush=True)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    process = None
    completed_returncode = None
    try:
        process = subprocess.Popen([str(x) for x in command])
        deadline = time.monotonic() + 180
        while not received.wait(0.5) and process.poll() is None and time.monotonic() < deadline:
            pass
    finally:
        if process is not None and process.poll() is not None:
            completed_returncode = process.returncode
        # The browser has no reliable script-close API; close only this owned test process.
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        server.shutdown()
        server.server_close()
        thread.join()
    report = {'root': str(root), 'browser': browser_result, 'callback_received': received.is_set(),
              'completed_returncode': completed_returncode}
    for name in ('uri.exit', 'uri-smoke.txt', 'temp-directory.exit'):
        path = root / 'home/username' / name
        report[name] = path.read_text(errors='replace') if path.exists() else None
    write_json(work / 'filesystem-smoke.json', report)
    if (report['uri.exit'] != '0\n' or report['temp-directory.exit'] != '0\n'
            or '8 tests, 0 failures' not in (report['uri-smoke.txt'] or '')
            or (not profile.get('gecko') and completed_returncode not in (0, 1))
            or (profile.get('gecko') and (not received.is_set()
                or browser_result.get('html') != 'GECKO_HTML_OK' or browser_result.get('js') != 'GECKO_JAVASCRIPT_OK'
                or not browser_result.get('width', '').isdigit() or int(browser_result['width']) <= 0))):
        raise build_wine.BuildError(f'Packaged filesystem smoke test failed; see {work / "filesystem-smoke.json"}')
    print('Packaged Temp directory and URI conversion checks passed.' +
          (' Gecko HTML, JavaScript, and layout checks passed.' if profile.get('gecko') else ' Gecko omitted.'), flush=True)
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--profile', type=Path, default=HERE / 'filesystem_wine11.json')
    parser.add_argument('--variant', choices=('main', 'web'), default='main',
                        help='Web omits Gecko, DXVK, Vulkan and unused Linux packages')
    parser.add_argument('--work-dir', type=Path, required=True, help='Dedicated WSL-native build directory')
    parser.add_argument('--phase', choices=('all', 'wine', 'addons', 'assemble', 'smoke'), default='all')
    parser.add_argument('--wine-repository', help='Optional local Wine Git cache; the pinned commit is still checked')
    parser.add_argument('--boxedwine', type=Path, help='Existing native Linux Boxedwine executable')
    parser.add_argument('--jobs', type=build_wine.positive_int, default=min(os.cpu_count() or 1, 12))
    parser.add_argument('--compare-to', type=Path, help='Optional old full ZIP, used only for the final comparison report')
    args = parser.parse_args(argv)
    profile = select_variant(json.loads(args.profile.read_text()), args.variant)
    if profile['schema_version'] != 1 or profile['wine_tag'] != 'wine-11.0':
        parser.error('This complete-filesystem profile currently supports Wine 11.0')
    work = args.work_dir.resolve()
    if build_wine.wsl_windows_mount_checkout(work):
        parser.error('--work-dir must be on the native Linux filesystem, not /mnt/c')
    work.mkdir(parents=True, exist_ok=True)
    config = wine_config(profile)
    wine_repo = args.wine_repository or config['wine']['repo']
    if args.phase in ('all', 'wine'):
        (work / 'oss').mkdir(exist_ok=True)
        shutil.copy2(HERE / 'oss/soundcard.h', work / 'oss/soundcard.h')
        build_config = copy.deepcopy(config)
        build_config['wine']['repo'] = wine_repo
        build_wine.build_wine(profile['wine_tag'], work, build_config, build_wine.CommandRunner(),
                              args.jobs, HERE / 'patches', initialize_home=False)
        write_json(work / 'wine-build-config.json', config)
    if args.phase in ('all', 'addons'):
        build_addons(profile, work, wine_repo, args.jobs)
    if args.phase in ('all', 'assemble', 'smoke'):
        boxedwine = args.boxedwine
        if boxedwine is None:
            run(['make', 'release'], cwd=REPO / 'project/linux')
            boxedwine = REPO / 'project/linux/Build/Release/boxedwine'
        if args.phase != 'smoke':
            assemble(profile, work, boxedwine.resolve(), args.compare_to)
        smoke_test(work, boxedwine.resolve(), profile)
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (build_wine.BuildError, webgl_filesystem.ValidationError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
