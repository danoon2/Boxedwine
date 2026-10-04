# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Bounded, cancellable file operations. Host links are never followed."""
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import stat
import threading
import uuid
import zipfile

class Cancelled(Exception):
    pass

class Work:
    def __init__(self, progress=None):
        self.cancel = threading.Event()
        self.progress = progress or (lambda *args: None)

    def check(self):
        if self.cancel.is_set():
            raise Cancelled('Operation cancelled. Original files were kept.')

    def report(self, message, done=0, total=0):
        self.check()
        self.progress(message, done, total)

def no_links(path):
    path = Path(os.path.abspath(path))
    for part in (path, *path.parents):
        try:
            mode = part.lstat().st_mode
        except FileNotFoundError:
            continue
        if stat.S_ISLNK(mode):
            raise ValueError(f'Host symbolic links are not supported here: {part}')
        if not (stat.S_ISREG(mode) or stat.S_ISDIR(mode)):
            raise ValueError(f'Not a regular file or directory: {part}')
    return path

def beneath(root, relative):
    if not isinstance(relative, str) or not relative or '\\' in relative or any(ord(c) < 32 for c in relative):
        raise ValueError('Invalid relative path.')
    parts = relative.split('/')
    if any(p in ('', '.', '..') for p in parts) or PurePosixPath(relative).is_absolute():
        raise ValueError('A file path escapes its folder.')
    return no_links(Path(root).joinpath(*parts))

def within(path, root):
    path, root = Path(os.path.abspath(path)), Path(os.path.abspath(root))
    return path != root and root in path.parents

def read_json(path, limit=32 * 1024 * 1024):
    path = no_links(path)
    with path.open('rb') as stream:
        data = stream.read(limit + 1)
    if len(data) > limit:
        raise ValueError('Metadata exceeds its size limit.')
    return json.loads(data)

def atomic_json(path, value):
    path = no_links(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.' + uuid.uuid4().hex + '.partial')
    try:
        with temporary.open('x', encoding='utf-8') as stream:
            json.dump(value, stream, indent=2, ensure_ascii=False, allow_nan=False)
            stream.write('\n')
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
        descriptor = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
        try:
            os.fsync(descriptor)
        finally:
            os.close(descriptor)
    finally:
        temporary.unlink(missing_ok=True)

def digest(path, work=None):
    work = work or Work()
    result = hashlib.sha256()
    with no_links(path).open('rb') as stream:
        while chunk := stream.read(1024 * 1024):
            work.check()
            result.update(chunk)
    return result.hexdigest()

def walk(root):
    root = no_links(root)
    for current, directories, files in os.walk(root, followlinks=False):
        for name in sorted(directories + files):
            yield no_links(Path(current) / name)

def copy_file(source, target, work):
    source, target = no_links(source), no_links(target)
    target.parent.mkdir(parents=True, exist_ok=True)
    size, done = source.stat().st_size, 0
    with source.open('rb') as incoming, target.open('xb') as outgoing:
        while chunk := incoming.read(1024 * 1024):
            work.report('Copying ' + source.name, done, size)
            outgoing.write(chunk)
            done += len(chunk)
        outgoing.flush()
        os.fsync(outgoing.fileno())

def copy_tree(source, target, work):
    source, target = no_links(source), no_links(target)
    if source == target or within(target, source):
        raise ValueError('The destination must be outside the source folder.')
    target.mkdir(parents=True, exist_ok=True)
    for path in walk(source):
        work.check()
        destination = beneath(target, path.relative_to(source).as_posix())
        if path.is_dir():
            destination.mkdir(parents=True, exist_ok=True)
        else:
            copy_file(path, destination, work)

def delete_tree(path, parent):
    path = no_links(path)
    if not within(path, parent):
        raise ValueError('Refusing to delete outside the owned folder.')
    if path.exists():
        # Inspect before starting deletion; shutil.rmtree also uses fd-based
        # symlink attack protection on our supported Linux/Python versions.
        for _ in walk(path):
            pass
        shutil.rmtree(path)

def inventory(root, work=None):
    work = work or Work()
    entries = []
    for path in walk(root):
        name = path.relative_to(root).as_posix()
        work.report('Checking ' + name)
        entries.append({'path': name, 'kind': 'directory'} if path.is_dir() else
                       {'path': name, 'kind': 'file', 'size': path.stat().st_size, 'digest': digest(path, work)})
        if len(entries) > 100000:
            raise ValueError('This app contains too many files.')
    return sorted(entries, key=lambda e: e['path'])

def canonical_entries(entries):
    if not isinstance(entries, list) or len(entries) > 100000:
        raise ValueError('Invalid backup inventory.')
    normalized = []
    seen = set()
    for entry in entries:
        name, kind = entry['path'], entry['kind']
        beneath('/tmp/boxedwine-inventory-validation', name)
        if name in seen or kind not in ('directory', 'file'):
            raise ValueError('Duplicate or unsupported backup entry.')
        seen.add(name)
        normalized.append({'path': name, 'kind': kind} if kind == 'directory' else
                          {'path': name, 'kind': kind, 'size': entry['size'], 'digest': entry['digest']})
    return sorted(normalized, key=lambda e: e['path'])

def extract_zip(source, target, work):
    with zipfile.ZipFile(no_links(source)) as archive:
        if len(archive.infolist()) > 100000:
            raise ValueError('Too many archive entries.')
        expanded, seen = 0, set()
        for i, entry in enumerate(archive.infolist()):
            name = entry.filename.replace('\\', '/').rstrip('/')
            path = beneath(target, name)
            kind = (entry.external_attr >> 16) & 0xf000
            expanded += entry.file_size
            if (entry.orig_filename != entry.filename or name.casefold() in seen or
                    kind not in (0, stat.S_IFREG, stat.S_IFDIR) or expanded > 8 * 1024**3 or entry.file_size > 2 * 1024**3):
                raise ValueError('Linked, duplicate, or oversized archive entry.')
            seen.add(name.casefold())
            work.report('Extracting ' + name, i, len(archive.infolist()))
            if entry.is_dir() or entry.filename.endswith('\\'):
                path.mkdir(parents=True, exist_ok=True)
            else:
                path.parent.mkdir(parents=True, exist_ok=True)
                with archive.open(entry) as incoming, path.open('xb') as outgoing:
                    copied = 0
                    while chunk := incoming.read(1024 * 1024):
                        work.check()
                        copied += len(chunk)
                        if copied > entry.file_size:
                            raise ValueError('Archive file exceeds declared size.')
                        outgoing.write(chunk)
                    if copied != entry.file_size:
                        raise ValueError('Incomplete archive entry.')
