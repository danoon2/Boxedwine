# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""User-selected sources and persistent collection snapshots (shared native format)."""
import copy
import hashlib
from http.client import HTTPException
import json
from pathlib import Path
import re
import tempfile
import urllib.parse
import urllib.request
import urllib.error
import time
import struct
import uuid
import xml.etree.ElementTree as ET
from .files import Work, atomic_json, beneath, extract_zip, no_links, read_json, walk
from .packages import NoRedirect

MAX_DOWNLOAD = 1024**3
FIELDS = set(('ID Name Summary Help Icon FileURL FileSizeBytes FileSHA256 InstallType InstallExe ShortcutExe WineVersion '
              'WindowsVersion GDIRenderer Resolution InstallResolution BitsPerPixel CPUCount NativeOpenGL UseEGL CNCDDraw '
              'DisableHideCursor ForceRelativeMouse Glide CNCDDrawRenderer CNCDDrawUncapped CNCDDrawFakeMode WineDllOverrides AppArguments CDROMImages').split())


def app_arguments(value):
    """One literal argument per line, matching App Settings; no shell parsing."""
    if value is None or value == '':
        return []
    arguments = value.replace('\r\n', '\n').split('\n')
    if (len(arguments) > 256 or sum(len(a.encode()) for a in arguments) > 65536 or
            any(len(a.encode()) > 8192 or '\0' in a or '\r' in a for a in arguments)):
        raise ValueError('AppArguments needs at most 256 arguments, one per line, with at most 8192 bytes each and 65536 bytes total, without NUL or bare carriage returns.')
    return arguments


def valid_url(value):
    try:
        url = urllib.parse.urlsplit(value)
        return (url.scheme in ('http', 'https') and bool(url.hostname) and url.port != 0 and
                not url.username and not url.password and not url.fragment and not any(ord(c) < 33 for c in value))
    except (ValueError, TypeError):
        return False


def download_file(url, destination, work, limit=MAX_DOWNLOAD, timeout=1800):
    if not valid_url(url):
        raise ValueError('Use a complete HTTP or HTTPS file link without a login or fragment.')
    original_scheme = urllib.parse.urlsplit(url).scheme
    opener = urllib.request.build_opener(NoRedirect)
    deadline = time.monotonic() + timeout
    target = no_links(destination)
    try:
        for redirects in range(6):
            work.check()
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError('The download exceeded its time limit.')
            try:
                response = opener.open(urllib.request.Request(url, headers={'User-Agent': 'Boxedwine/1', 'Accept-Encoding': 'identity'}), timeout=min(20, remaining))
                break
            except urllib.error.HTTPError as error:
                location = error.headers.get('Location')
                error.close()
                if error.code not in (301, 302, 303, 307, 308) or redirects == 5 or not location:
                    raise
                url = urllib.parse.urljoin(url, location)
                if not valid_url(url) or original_scheme == 'https' and urllib.parse.urlsplit(url).scheme != 'https':
                    raise ValueError('The download redirected to an unsupported location.')
        with response, target.open('xb') as output:
            if response.status != 200:
                raise ValueError('The server did not return a file.')
            total = int(response.headers.get('Content-Length', '0'))
            if not 0 <= total <= limit:
                raise ValueError('The download is too large.')
            done = 0
            while chunk := response.read1(128 * 1024):
                work.check()
                done += len(chunk)
                if done > limit or time.monotonic() > deadline:
                    raise ValueError('The download exceeded its size or time limit.')
                output.write(chunk)
                work.report('Downloading ' + urllib.parse.urlsplit(url).hostname, done, total)
            if done == 0 or total and done != total:
                raise ValueError('The downloaded file is empty or incomplete.')
        work.check()
    except BaseException:
        target.unlink(missing_ok=True)
        raise


class CollectionIcons:
    """Optional PNG artwork, cached by its complete URL for offline use."""
    def __init__(self, directory):
        self.directory = Path(directory) / 'CollectionIcons'

    @staticmethod
    def valid_png(data):
        return (24 <= len(data) <= 1024**2 and data[:16] == b'\x89PNG\r\n\x1a\n\0\0\0\rIHDR' and
                all(0 < size <= 1024 for size in struct.unpack('>II', data[16:24])))

    def path(self, url):
        return beneath(self.directory, hashlib.sha256(url.encode()).hexdigest() + '.png')

    def read(self, url):
        if not valid_url(url):
            return None
        try:
            with self.path(url).open('rb') as stream:
                data = stream.read(1024**2 + 1)
            return data if self.valid_png(data) else None
        except (OSError, ValueError):
            return None

    def fetch(self, url):
        if not valid_url(url) or self.read(url) is not None:
            return
        temporary = None
        try:
            no_links(self.directory).mkdir(parents=True, exist_ok=True)
            temporary = beneath(self.directory, str(uuid.uuid4()) + '.tmp')
            download_file(url, temporary, Work(), 1024**2, timeout=20)
            if self.valid_png(temporary.read_bytes()):
                temporary.replace(self.path(url))
        except (OSError, ValueError, HTTPException):
            pass  # Artwork is optional; failed downloads keep the normal fallback.
        finally:
            if temporary is not None:
                try:
                    temporary.unlink(missing_ok=True)
                except OSError:
                    pass


class PreparedSource:
    def __init__(self, source, work, remote=False, collection=False):
        self.temporary = tempfile.TemporaryDirectory(prefix='boxedwine-import-')
        self.original = str(source)
        self.directory = Path(self.temporary.name).resolve()
        try:
            work.check()
            if remote:
                if not valid_url(source):
                    raise ValueError('Use a complete HTTP or HTTPS file link.')
                filename = Path(urllib.parse.unquote(urllib.parse.urlsplit(source).path)).name
                if not filename or Path(filename).suffix.lower() not in ('.zip', '.exe', '.msi', '.json'):
                    raise ValueError('Use a direct link to a ZIP, EXE, MSI, or collection JSON file.')
                self.source = beneath(self.directory, filename)
                download_file(source, self.source, work, 1024**2 if collection else MAX_DOWNLOAD)
            else:
                self.source = no_links(source)
            self.name = self.source.stem if self.source.is_file() else self.source.name
            self.collection = load_collection(read_json(self.source, 1024**2)) if self.source.is_file() and self.source.suffix.lower() == '.json' else None
            if collection and self.collection is None:
                raise ValueError('Choose a Boxedwine collection JSON file.')
            if self.source.is_file() and self.source.suffix.lower() == '.zip':
                target = self.directory / 'Contents'
                extract_zip(self.source, target, work)
                self.source = target
            self.programs = []
            if self.collection is None:
                if self.source.is_dir():
                    for path in walk(self.source):
                        work.check()
                        if path.is_file() and path.suffix.lower() in ('.exe', '.msi'):
                            self.programs.append(path.relative_to(self.source).as_posix())
                        if len(self.programs) > 4096:
                            raise ValueError('Too many Windows programs in this folder.')
                elif self.source.suffix.lower() in ('.exe', '.msi'):
                    self.programs = [self.source.name]
                if not self.programs:
                    raise ValueError('No Windows program was found. Choose an app folder, ZIP, EXE, or MSI.')
            work.check()
            self.programs.sort(key=lambda p: (bool(re.match(r'(unins|unwise|update|crash)', Path(p).stem, re.I)), p.lower()))
        except BaseException:
            self.close()
            raise

    @property
    def suggested_action(self):
        setups = [p for p in self.programs if Path(p).suffix.lower() == '.msi' or re.match(r'^(setup|install)', Path(p).stem, re.I)]
        apps = [p for p in self.programs if Path(p).suffix.lower() == '.exe' and not re.match(r'^(setup|install|unins|unwise|update|crash)', Path(p).stem, re.I)]
        if setups and not apps:
            return 'installer'
        if apps and not setups and self.source.is_dir():
            return 'app'
        return ''

    def close(self):
        self.temporary.cleanup()


def folder_name(value):
    if (not isinstance(value, str) or not value.strip() or len(value) > 80 or
            any(ord(c) < 32 or c in '/\\' for c in value)):
        raise ValueError('Use a collection folder name of 1–80 characters without slashes.')
    value = value.strip()
    if value.lower() in ('all apps', 'recently opened', 'removed apps', 'recovery', 'unfinished work'):
        raise ValueError('That name is reserved for a library view.')
    return 'Demos' if value.lower() == 'demos' else value


def load_collection(document):
    from .demos import load_catalog
    if (not isinstance(document, dict) or set(document) != {'schemaVersion', 'folder', 'apps'} or
            document['schemaVersion'] != 1 or not isinstance(document['apps'], list) or not 1 <= len(document['apps']) <= 256):
        raise ValueError('Use a Boxedwine collection with schemaVersion 1, folder, and 1–256 apps.')
    result = copy.deepcopy(document)
    result['folder'] = folder_name(result['folder'])
    root = ET.Element('XML', schemaVersion='8', release='custom-collection')
    for entry in result['apps']:
        if not isinstance(entry, dict) or not set(entry) <= FIELDS or any(not isinstance(v, str) for v in entry.values()):
            raise ValueError('Collection app fields must be supported demo fields with string values.')
        if not re.fullmatch('[a-z0-9][a-z0-9-]{0,63}', entry.get('ID', '')):
            raise ValueError('App IDs must be stable lowercase letters, numbers, and hyphens.')
        wine = entry.get('WineVersion', '').strip()
        icon = entry.get('Icon', '').strip()
        extension = Path(urllib.parse.urlsplit(entry.get('FileURL', '')).path).suffix.lower()
        installer = entry.get('InstallExe', '').strip()
        needs_setup = entry.get('InstallType', '').strip() == 'Installer' and extension == '.zip'
        if ('WineVersion' in entry and (not wine or len(wine.encode()) > 128) or extension not in ('.zip', '.exe', '.msi') or
                icon and not valid_url(icon) and ('/' in icon or '\\' in icon or not icon.endswith('.png')) or
                needs_setup and Path(installer).suffix.lower() not in ('.exe', '.msi') or
                not needs_setup and installer):
            raise ValueError('Invalid collection Wine version, icon, or installation recipe.')
        node = ET.SubElement(root, 'Demo')
        app_arguments(entry.get('AppArguments'))
        for key, value in entry.items():
            if key != 'AppArguments':
                ET.SubElement(node, key).text = value
    load_catalog(ET.tostring(root), allow_external=True)
    return result


class CollectionStore:
    def __init__(self, library):
        self.default_ids = set()
        self.path = library.directory / 'collections.json'

    def load(self):
        doc = read_json(self.path) if self.path.exists() else {'version': 1, 'folders': []}
        if set(doc) != {'version', 'folders'} or doc['version'] != 1 or not isinstance(doc['folders'], list) or len(doc['folders']) > 64:
            raise ValueError('Unsupported saved collections. The file was kept.')
        seen = set()
        for folder in doc['folders']:
            load_collection(dict(schemaVersion=1, folder=folder['name'], apps=folder['apps']))
            sources = folder.setdefault('sources', [])
            self.validate_sources(sources)
            key = folder['name'].lower()
            if key in seen:
                raise ValueError('Duplicate collection folder.')
            seen.add(key)
        return doc

    @staticmethod
    def validate_sources(sources):
        if (not isinstance(sources, list) or len(sources) > 32 or
                any(not isinstance(url, str) or len(url) > 8192 or not valid_url(url) for url in sources) or
                len(set(sources)) != len(sources)):
            raise ValueError('A collection can remember up to 32 unique HTTP or HTTPS source URLs.')

    def preview(self, incoming, destination):
        destination = folder_name(destination)
        doc = self.load()
        folder = next((f for f in doc['folders'] if f['name'].lower() == destination.lower()), None)
        return self.changes(incoming, destination, folder['apps'] if folder else [])

    def changes(self, incoming, destination, previous):
        old = {a['ID']: a for a in previous}
        defaults = self.default_ids if destination == 'Demos' else set()
        added = sum(a['ID'] not in old and a['ID'] not in defaults for a in incoming['apps'])
        updated = sum((a['ID'] in old and old[a['ID']] != a) or (a['ID'] not in old and a['ID'] in defaults) for a in incoming['apps'])
        return added, updated, len(incoming['apps']) - added - updated

    def add(self, incoming, destination, source=None, expected=None):
        incoming = load_collection(incoming)
        destination = folder_name(destination)
        doc = self.load()
        folder = next((f for f in doc['folders'] if f['name'].lower() == destination.lower()), None)
        if expected is not None and folder != expected:
            raise ValueError('This collection changed while refreshing. Refresh again to review the latest changes.')
        if folder is None:
            if len(doc['folders']) >= 64:
                raise ValueError('The library already has 64 collection folders.')
            folder = {'name': destination, 'apps': [], 'sources': []}; doc['folders'].append(folder)
        if source is not None and source not in folder['sources']:
            folder['sources'].append(source)
        self.validate_sources(folder['sources'])
        merged = {a['ID']: a for a in folder['apps']}
        merged.update({a['ID']: a for a in incoming['apps']})
        if len(merged) > 256:
            raise ValueError('A collection folder can contain up to 256 custom listings.')
        folder['apps'] = list(merged.values())
        load_collection(dict(schemaVersion=1, folder=folder['name'], apps=folder['apps']))
        if len(json.dumps(doc, indent=2, ensure_ascii=False).encode()) + 1 > 32 * 1024 * 1024:
            raise ValueError('The collections file is too large. Existing data was kept.')
        atomic_json(self.path, doc)
        return folder['name']

    def review_refresh(self, before, snapshots):
        if not before['sources'] or len(snapshots) != len(before['sources']):
            raise ValueError('Every saved collection source must be downloaded before applying changes.')
        merged = {}
        for snapshot in snapshots:
            merged.update({a['ID']: a for a in load_collection(snapshot)['apps']})
        incoming = load_collection(dict(schemaVersion=1, folder=before['name'], apps=list(merged.values())))
        if len(set(merged) | {a['ID'] for a in before['apps']}) > 256:
            raise ValueError('A collection folder can contain up to 256 custom listings.')
        return dict(before=copy.deepcopy(before), incoming=incoming,
                    changes=self.changes(incoming, before['name'], before['apps']))

    def refresh(self, name, work):
        before = next((f for f in self.load()['folders'] if f['name'].lower() == name.lower()), None)
        if before is None or not before['sources']:
            raise ValueError('This collection has no saved source URLs. Import it from a URL first.')
        snapshots = []
        for source in before['sources']:
            work.check()
            prepared = PreparedSource(source, work, remote=True, collection=True)
            try:
                snapshots.append(prepared.collection)
            finally:
                prepared.close()
        work.check()
        return self.review_refresh(before, snapshots)

    def apply_refresh(self, review):
        return self.add(review['incoming'], review['before']['name'], expected=review['before'])

    def remove(self, name):
        doc = self.load()
        doc['folders'] = [f for f in doc['folders'] if f['name'].lower() != name.lower()]
        atomic_json(self.path, doc)

    def recipes(self, name, defaults=(), document=None):
        from .demos import load_catalog
        result = {r['origin']['id']: r for r in defaults} if name == 'Demos' else {}
        folder = next((f for f in (document if document is not None else self.load())['folders'] if f['name'].lower() == name.lower()), None)
        if folder:
            root = ET.Element('XML', schemaVersion='8', release='custom-collection')
            for entry in folder['apps']:
                node = ET.SubElement(root, 'Demo')
                for key, value in entry.items():
                    if key != 'AppArguments':
                        ET.SubElement(node, key).text = value
            for recipe, entry in zip(load_catalog(ET.tostring(root), allow_external=True), folder['apps']):
                recipe['appArguments'] = app_arguments(entry.get('AppArguments'))
                if name != 'Demos':
                    recipe['origin']['id'] = 'collection-' + hashlib.sha256((folder['name'].lower() + '\0' + recipe['origin']['id']).encode()).hexdigest()[:48]
                result[recipe['origin']['id']] = recipe
        return list(result.values())
