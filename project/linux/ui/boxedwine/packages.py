# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Pinned downloads and validation of the guest's Wine filesystem."""
import importlib.util
import os
from pathlib import Path
import re
import stat
import time
import urllib.error
import urllib.parse
import urllib.request
import uuid
import zipfile
from .files import Work, atomic_json, beneath, copy_file, digest, no_links, read_json
from .library import validate_reference


class Resources:
    def __init__(self, directory=None):
        self.directory = Path(directory) if directory else Path(__file__).resolve().parents[1] / 'resources'
        self.repo = Path(__file__).resolve().parents[4]

    def path(self, name):
        local = self.directory / name
        if local.exists():
            return local
        shared = self.repo / 'project/mac-xcode/Boxedwine/BoxedwineUI/Resources'
        alternatives = {'org.boxedwine.Boxedwine.png': self.repo / 'project/mac-xcode/Boxedwine/Media.xcassets/AppIcon.appiconset/icon_256x256.png',
                        'demo-catalog.lock.json': self.repo / 'resources/demo-catalog.lock.json',
                        'demo_catalog.py': self.repo / 'tools/demo_catalog.py',
                        'LICENSE': self.repo / 'license.txt'}
        path = alternatives.get(name, shared / name)
        if not path.is_file():
            raise ValueError('Missing application resource: ' + name + '. Run make native-ui.')
        return path

    def wines(self):
        result = []
        for url, pin in read_json(self.path('WindowsSupport/packages.json')).items():
            match = re.search(r'Wine([0-9.]+)\.zip$', url)
            if not match or not trusted_url(url):
                raise ValueError('Invalid Wine release catalog.')
            wine = dict(sha256=pin['sha256'], bytes=pin['bytes'], wineVersion=match[1], filesystemVersion=pin['filesystemVersion'])
            validate_reference(wine)
            result.append(dict(url=url, reference=wine))
        return sorted(result, key=lambda w: tuple(int(p) for p in w['reference']['wineVersion'].split('.')), reverse=True)


def trusted_url(url):
    parsed = urllib.parse.urlsplit(url)
    return (parsed.scheme == 'https' and parsed.hostname in ('boxedwine.org', 'www.boxedwine.org') and
            parsed.port in (None, 443) and not parsed.username and not parsed.password and not parsed.fragment)


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


def download(url, size, sha256, destination, work):
    if not trusted_url(url) or not 0 < size <= 4 * 1024**3 or not re.fullmatch('[a-f0-9]{64}', sha256):
        raise ValueError('Downloads require a supported URL and a pinned checksum.')
    target = no_links(destination)
    target.parent.mkdir(parents=True, exist_ok=True)
    partial = target.with_name(target.name + '.' + uuid.uuid4().hex + '.partial')
    deadline = time.monotonic() + 1800
    opener = urllib.request.build_opener(NoRedirect)
    try:
        for redirects in range(6):
            work.check()
            try:
                response = opener.open(urllib.request.Request(url, headers={'User-Agent': 'Boxedwine-Linux/1'}), timeout=20)
                break
            except urllib.error.HTTPError as error:
                if error.code not in (301, 302, 303, 307, 308) or redirects == 5:
                    raise
                location = error.headers.get('Location')
                error.close()
                url = urllib.parse.urljoin(url, location or '')
                if not location or not trusted_url(url):
                    raise ValueError('Download redirected outside boxedwine.org.')
        with response, partial.open('xb') as output:
            if response.headers.get('Content-Length') and int(response.headers['Content-Length']) != size:
                raise ValueError('Download size differs from the release catalog.')
            copied = 0
            while chunk := response.read(128 * 1024):
                work.check()
                if time.monotonic() > deadline:
                    raise TimeoutError('Download timed out. Try again.')
                copied += len(chunk)
                if copied > size:
                    raise ValueError('Download exceeds its expected size.')
                output.write(chunk)
                work.report('Downloading ' + Path(urllib.parse.urlsplit(url).path).name, copied, size)
            output.flush()
            os.fsync(output.fileno())
        if copied != size or digest(partial, work) != sha256:
            raise ValueError('The download failed verification. Nothing was installed.')
        work.check()
        os.replace(partial, target)
    finally:
        partial.unlink(missing_ok=True)


def validate_wine(path, work=None):
    work = work or Work()
    path = no_links(path)
    size = path.stat().st_size
    if not 0 < size <= 4 * 1024**3:
        raise ValueError('Choose a complete Wine filesystem ZIP, at most 4 GB.')
    checksum = digest(path, work)
    records, metadata = {}, {}
    with zipfile.ZipFile(path) as archive:
        entries = archive.infolist()
        if not 0 < len(entries) <= 100000:
            raise ValueError('Wine ZIP is empty or contains too many files.')
        expanded = 0
        for index, entry in enumerate(entries):
            work.report('Checking Wine package', index, len(entries))
            name = entry.filename.rstrip('/')
            # Validate without using a host path: .link files represent guest links.
            if (entry.orig_filename != entry.filename or not name or len(name) > 1022 or '\\' in name or any(ord(c) < 32 for c in name)
                    or any(p in ('', '.', '..') for p in name.split('/'))):
                raise ValueError('Unsafe path in Wine ZIP.')
            directory = entry.is_dir()
            mode = (entry.external_attr >> 16) & 0xf000
            expanded += entry.file_size
            if mode not in (0, stat.S_IFREG, stat.S_IFDIR) or (mode == stat.S_IFDIR and not directory):
                raise ValueError('Wine ZIP contains native links or devices.')
            link = not directory and name.endswith('.link')
            meta = name in ('wineVersion.txt', 'version.txt', 'name.txt', 'depends.txt')
            if (expanded > 8 * 1024**3 or entry.file_size > 1024**3 or
                    ((link or meta) and entry.file_size > 1022) or (directory and entry.file_size)):
                raise ValueError('Wine ZIP exceeds its size limits.')
            prefix, count = b'', 0
            with archive.open(entry) as stream:
                while chunk := stream.read(128 * 1024):
                    work.check()
                    count += len(chunk)
                    prefix += chunk[:max(0, (1022 if link or meta else 64) - len(prefix))]
                    if count > entry.file_size:
                        raise ValueError('Invalid Wine ZIP entry size.')
            if count != entry.file_size:
                raise ValueError('Incomplete Wine ZIP entry.')
            target = None
            if link or meta:
                value = prefix.decode('utf-8')
                if '\0' in value:
                    raise ValueError('Invalid Wine metadata.')
                if link:
                    if not value or any(c in value for c in '\r\n\\'):
                        raise ValueError('Malformed guest link.')
                    target = value
                else:
                    metadata[name] = value.strip()
            key = name[:-5] if link else name
            record = (directory, target, prefix, count)
            if key in records:
                previous = records[key]
                # Some released filesystems contain both a tiny regular stub
                # and its guest .link entry with identical contents.
                if directory or previous[0] or meta or count > 64 or count != previous[3] or prefix != previous[2]:
                    raise ValueError('Conflicting Wine ZIP entries.')
            else:
                records[key] = record
    wine = dict(sha256=checksum, bytes=size, wineVersion=metadata.get('wineVersion.txt', ''), filesystemVersion=metadata.get('version.txt', ''))
    validate_reference(wine)
    if metadata.get('depends.txt'):
        raise ValueError('Choose a complete Wine filesystem, without external dependencies.')
    for key in records:
        for parent in Path(key).parents:
            record = records.get(parent.as_posix())
            if record and not record[0] and not record[1]:
                raise ValueError('A Wine ZIP file is also used as a directory.')

    def resolve(name, visited):
        if name in visited or len(visited) >= 40:
            raise ValueError('Wine guest links form a loop.')
        visited.add(name)
        parts = name.split('/')
        for i in range(1, len(parts) + 1):
            record = records.get('/'.join(parts[:i]))
            if record and record[1]:
                target = record[1]
                combined = ('' if target.startswith('/') else '/'.join(parts[:i - 1]) + '/') + target + '/' + '/'.join(parts[i:])
                normalized = []
                for part in combined.split('/'):
                    if part in ('', '.'):
                        continue
                    if part == '..':
                        if not normalized:
                            raise ValueError('Guest link escapes its root.')
                        normalized.pop()
                    else:
                        normalized.append(part)
                return resolve('/'.join(normalized), visited)
        if name not in records:
            raise ValueError('The package is missing /bin/wine or its link target.')
        return records[name]
    prefix = resolve('bin/wine', set())[2]
    if len(prefix) < 52 or prefix[:7] != b'\x7fELF\x01\x01\x01' or prefix[16] not in (2, 3) or prefix[18] != 3:
        raise ValueError('Wine must be a 32-bit x86 Linux executable for Boxedwine.')
    for dll in ('ntdll', 'kernel32'):
        if not any(not r[0] and not r[1] and (n.endswith('/' + dll + '.dll') or n.endswith('/' + dll + '.dll.so')) for n, r in records.items()):
            raise ValueError('Wine ZIP is missing ' + dll + '.')
    return wine


def import_wine(library, source, work):
    staging = beneath(library.directory, 'Downloads/' + uuid.uuid4().hex + '.zip')
    try:
        copy_file(source, staging, work)
        wine = validate_wine(staging, work)
        destination = library.package_path(wine)
        destination.parent.mkdir(parents=True, exist_ok=True)
        if destination.exists():
            if destination.stat().st_size != wine['bytes'] or digest(destination, work) != wine['sha256']:
                raise ValueError('An existing Wine package is damaged. Its files were kept for review.')
        else:
            os.rename(staging, destination)
        return wine
    finally:
        staging.unlink(missing_ok=True)


def ensure_wine(library, release, work):
    wine = release['reference']
    target = library.package_path(wine)
    if target.exists():
        if validate_wine(target, work) != wine:
            raise ValueError('The saved Wine package differs from the catalog.')
        return wine
    staging = beneath(library.directory, 'Downloads/' + uuid.uuid4().hex + '.zip')
    try:
        download(release['url'], wine['bytes'], wine['sha256'], staging, work)
        if validate_wine(staging, work) != wine:
            raise ValueError('Wine package metadata differs from the catalog.')
        target.parent.mkdir(parents=True, exist_ok=True)
        os.rename(staging, target)
        return wine
    finally:
        staging.unlink(missing_ok=True)


class WineChecks:
    def __init__(self):
        self.verified = {}

    def verify(self, path, expected, work):
        path = no_links(path)
        info = path.stat()
        stamp = (info.st_dev, info.st_ino, info.st_size, info.st_mtime_ns, info.st_ctime_ns)
        cached = self.verified.get(path)
        if cached and cached[0] == stamp and (expected is None or expected == cached[1]):
            return cached[1]
        work.report('Checking Wine checksum…')
        if expected:
            validate_reference(expected)
            if info.st_size != expected['bytes'] or digest(path, work) != expected['sha256']:
                raise ValueError('This app’s Wine package has changed. Its Windows files were kept.')
            result = expected
        else:
            result = validate_wine(path, work)
        self.verified[path] = (stamp, result)
        return result


def catalog_files(resources, library, work, allow_download=True):
    """Use the shared release validator; cache only a pinned, verified archive."""
    spec = importlib.util.spec_from_file_location('boxedwine_demo_catalog', resources.path('demo_catalog.py'))
    helper = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(helper)
    pin = helper.read_pin(resources.path('demo-catalog.lock.json'))
    path = beneath(library.directory, 'Catalogs/' + pin['sha256'] + '.zip')
    if not path.exists():
        bundled = resources.directory / 'catalog.zip'
        if bundled.exists():
            helper.read_zip(bundled, pin)
            copy_file(bundled, path, work)
        elif allow_download:
            download(pin['url'], pin['bytes'], pin['sha256'], path, work)
        else:
            return None
    return helper.read_zip(path, pin)[1]
