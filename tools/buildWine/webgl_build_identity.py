"""Content identities for a Boxedwine browser runtime and its Wine root.

A supplied commit is a build label, not proof that an existing binary came from
the current checkout. Runtime hashes remain authoritative for the served bytes.
"""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import zipfile


RUNTIME_FILES = ('boxedwine.html', 'boxedwine.js', 'boxedwine.wasm',
                 'boxedwine-shell.js', 'boxedwine.css')
WINE_DLL = 'home/username/.wine/drive_c/webgl/wined3d.dll'
MODES = {'st': 'single-threaded-non-jit', 'st-jit': 'single-threaded-jit',
         'mt': 'multi-threaded-non-jit', 'mt-jit': 'multi-threaded-jit'}


def file_identity(path: Path) -> dict:
    digest = hashlib.sha256()
    size = 0
    with Path(path).open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
            size += len(chunk)
    return {'bytes': size, 'sha256': digest.hexdigest()}


def runtime_identity(directory: Path) -> dict:
    return {name: file_identity(Path(directory) / name) for name in RUNTIME_FILES}


def root_identity(path: Path, *, require_wined3d: bool = True) -> dict:
    path = Path(path)
    before = path.stat()
    archive_identity = file_identity(path)
    with zipfile.ZipFile(path) as archive:
        matches = [item for item in archive.infolist() if item.filename == WINE_DLL]
        if len(matches) > 1 or (require_wined3d and not matches):
            raise ValueError(f'{path.name} must contain exactly one {WINE_DLL}')
        dll = None
        if matches:
            digest = hashlib.sha256()
            size = 0
            with archive.open(matches[0]) as stream:
                for chunk in iter(lambda: stream.read(1024 * 1024), b''):
                    digest.update(chunk)
                    size += len(chunk)
            dll = {'entry': WINE_DLL, 'bytes': size, 'sha256': digest.hexdigest()}
    after = path.stat()
    if (before.st_size, before.st_mtime_ns, before.st_ino) != (after.st_size, after.st_mtime_ns, after.st_ino):
        raise ValueError(f'Filesystem changed while identifying {path}')
    return {'archive': archive_identity,
            'webgl_wined3d': dll}


def build_identity(*, mode: str, runtime: dict, filesystem: dict,
                   commit: str | None = None, source_dirty: bool | None = None) -> dict:
    mode = MODES.get(mode, mode)
    if mode not in MODES.values():
        raise ValueError(f'Unknown browser mode: {mode}')
    commit = commit.lower() if commit else None
    if commit and not re.fullmatch(r'(?:[0-9a-f]{40}|[0-9a-f]{64})', commit):
        raise ValueError('Build commit must be a full Git object ID or unknown')
    if source_dirty is not None and type(source_dirty) is not bool:
        raise ValueError('Source dirty state must be true, false or unknown')
    if set(runtime) != set(RUNTIME_FILES):
        raise ValueError('All five runtime files are required')
    entries = [*runtime.values(), filesystem['archive']]
    if filesystem['webgl_wined3d'] is not None:
        entries.append(filesystem['webgl_wined3d'])
    for item in entries:
        if not re.fullmatch(r'[0-9a-f]{64}', item.get('sha256', '')) or type(item.get('bytes')) is not int or item['bytes'] < 0:
            raise ValueError('Content identities require a byte count and full SHA-256')
    content = {'schema_version': 1, 'mode': mode,
               'source': {'commit': commit, 'dirty': source_dirty},
               'runtime': runtime, 'filesystem': filesystem}
    canonical = json.dumps(content, sort_keys=True, separators=(',', ':'), ensure_ascii=True).encode()
    # Retain a snapshot, not references that a caller can mutate after hashing.
    return {'id': 'bwgl-' + hashlib.sha256(canonical).hexdigest(), **json.loads(canonical)}
