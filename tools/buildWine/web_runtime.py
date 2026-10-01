"""Audited removals for the pinned TinyCore web runtime; never alias writable files."""
import hashlib
import json
from pathlib import Path
import zipfile

import build_wine

POLICY = Path(__file__).with_name('web_runtime_policy.json')
DRIVE = 'home/username/.wine/drive_c/'
SDK_TOOLS = {'function_grep.pl', 'widl', 'winebuild', 'winecpp', 'winedump',
             'wineg++', 'winegcc', 'winemaker', 'wmc', 'wrc'}


def load_policy():
    return json.loads(POLICY.read_text(encoding='utf-8'))


def exclusions(base_sha256):
    policy = load_policy()
    if policy['schema_version'] != 1 or policy['base_sha256'] != base_sha256:
        raise build_wine.BuildError('Web package removals must be audited for this base filesystem')
    files = {name for group in policy['remove_files'].values() for name in group}
    def excluded(name):
        if name in files or build_wine.is_wine_development_file(name):
            return True
        if name.startswith(('dep/', DRIVE + 'dxvk/', DRIVE + 'windows/system32/gecko/')):
            return True
        if name in {'packages.txt', 'recreateInstructions.txt', 'usr/local/bin/winetricks',
                    DRIVE + 'webgl/ddraw_test.exe'}:
            return True
        base = name.rsplit('/', 1)[-1].removesuffix('.link')
        if base in {'libvulkan.so', 'libvulkan.so.1', 'winevulkan.dll', 'winevulkan.dll.so', 'winevulkan.so',
                    'vulkan-1.dll', 'vulkan-1.dll.so'}:
            return True
        return name.startswith('opt/wine/bin/') and base in SDK_TOOLS
    return excluded


def write_metadata(overlay, base_zip):
    policy = load_policy()
    with zipfile.ZipFile(base_zip) as source:
        installed = source.read('installed.txt').decode().split()
    (overlay / 'installed.txt').write_text(
        ''.join(name + '\n' for name in dict.fromkeys(installed) if name not in policy['remove_packages']),
        encoding='utf-8')
    (overlay / 'web-runtime.json').write_text(json.dumps({
        'base_sha256': policy['base_sha256'], 'max_bytes': policy['max_bytes'],
        'policy_sha256': hashlib.sha256(POLICY.read_bytes()).hexdigest(),
        'omitted_packages': policy['remove_packages'],
        'gecko': False, 'dxvk': False, 'vulkan': False,
        'independent_prefix_files': True,
    }, indent=2) + '\n', encoding='utf-8')


def validate_archive(path):
    policy = load_policy()
    size = path.stat().st_size
    if size >= policy['max_bytes']:
        raise build_wine.BuildError(f'Web filesystem is {size:,} bytes; it must be below {policy["max_bytes"]:,}')
    excluded = exclusions(policy['base_sha256'])
    with zipfile.ZipFile(path) as archive:
        names = set(archive.namelist())
        if len(names) != len(archive.infolist()):
            raise build_wine.BuildError('Duplicate names in web filesystem')
        forbidden = sorted(n for n in names if excluded(n))
        if forbidden:
            raise build_wine.BuildError(f'Excluded files remain in web filesystem: {forbidden}')
        # This packaging policy must not reintroduce the prototype's symlink
        # deduplication. Prefix updates must not write into /opt/wine through links.
        for name in names:
            if name.startswith(DRIVE + 'windows/') and name.endswith('.link'):
                if archive.read(name).startswith(b'/opt/wine/'):
                    raise build_wine.BuildError(f'Prefix file aliases the Wine installation: {name}')
        required = {'opt/wine/lib/wine/i386-unix/wine', DRIVE + 'windows/system32/shell32.dll',
                    DRIVE + 'webgl/wined3d.dll', DRIVE + 'windows/system32/glide2x.dll',
                    DRIVE + 'users/username/AppData/Local/Temp/', DRIVE + 'windows/temp/'}
        if required - names:
            raise build_wine.BuildError(f'Web runtime is missing required files: {sorted(required - names)}')
        bad = archive.testzip()
        if bad:
            raise build_wine.BuildError(f'Corrupt web filesystem member: {bad}')
    return {'size': size, 'max_bytes': policy['max_bytes'], 'files': len(names),
            'sha256': build_wine.file_sha256(path), 'independent_prefix_files': True}
