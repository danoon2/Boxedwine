# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""User-selected sources and persistent collection snapshots (shared native format)."""
import copy
import hashlib
import json
from pathlib import Path
import re
import tempfile
import urllib.parse
import urllib.request
import urllib.error
import time
import xml.etree.ElementTree as ET
from .files import atomic_json, beneath, extract_zip, no_links, read_json, walk
from .packages import NoRedirect

MAX_DOWNLOAD = 1024**3
FIELDS = set(('ID Name Summary Help Icon FileURL FileSizeBytes FileSHA256 InstallType InstallExe ShortcutExe WineVersion '
              'WindowsVersion GDIRenderer Resolution InstallResolution BitsPerPixel CPUCount NativeOpenGL UseEGL CNCDDraw '
              'DisableHideCursor ForceRelativeMouse Glide CNCDDrawRenderer CNCDDrawUncapped CNCDDrawFakeMode WineDllOverrides').split())


def valid_url(value):
    try:
        url = urllib.parse.urlsplit(value)
        return (url.scheme in ('http', 'https') and bool(url.hostname) and url.port != 0 and
                not url.username and not url.password and not url.fragment and not any(ord(c) < 33 for c in value))
    except (ValueError, TypeError):
        return False


def download_file(url, destination, work, limit=MAX_DOWNLOAD):
    if not valid_url(url):
        raise ValueError('Use a complete HTTP or HTTPS file link without a login or fragment.')
    original_scheme = urllib.parse.urlsplit(url).scheme
    opener = urllib.request.build_opener(NoRedirect)
    deadline = time.monotonic() + 1800
    target = no_links(destination)
    try:
        for redirects in range(6):
            work.check()
            try:
                response = opener.open(urllib.request.Request(url, headers={'User-Agent': 'Boxedwine/1', 'Accept-Encoding': 'identity'}), timeout=20)
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
            while chunk := response.read(128 * 1024):
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
        if (not wine or len(wine.encode()) > 128 or extension not in ('.zip', '.exe', '.msi') or
                icon and ('/' in icon or '\\' in icon or not icon.endswith('.png')) or
                needs_setup and Path(installer).suffix.lower() not in ('.exe', '.msi') or
                not needs_setup and installer):
            raise ValueError('Invalid collection Wine version, icon, or installation recipe.')
        node = ET.SubElement(root, 'Demo')
        for key, value in entry.items():
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
            key = folder['name'].lower()
            if key in seen:
                raise ValueError('Duplicate collection folder.')
            seen.add(key)
        return doc

    def preview(self, incoming, destination):
        destination = folder_name(destination)
        doc = self.load()
        folder = next((f for f in doc['folders'] if f['name'].lower() == destination.lower()), None)
        old = {a['ID']: a for a in folder['apps']} if folder else {}
        defaults = self.default_ids if destination == 'Demos' else set()
        added = sum(a['ID'] not in old and a['ID'] not in defaults for a in incoming['apps'])
        updated = sum((a['ID'] in old and old[a['ID']] != a) or (a['ID'] not in old and a['ID'] in defaults) for a in incoming['apps'])
        return added, updated, len(incoming['apps']) - added - updated

    def add(self, incoming, destination):
        incoming = load_collection(incoming)
        destination = folder_name(destination)
        doc = self.load()
        folder = next((f for f in doc['folders'] if f['name'].lower() == destination.lower()), None)
        if folder is None:
            if len(doc['folders']) >= 64:
                raise ValueError('The library already has 64 collection folders.')
            folder = {'name': destination, 'apps': []}; doc['folders'].append(folder)
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

    def remove(self, name):
        doc = self.load()
        doc['folders'] = [f for f in doc['folders'] if f['name'].lower() != name.lower()]
        atomic_json(self.path, doc)

    def recipes(self, name, defaults=()):
        from .demos import load_catalog
        result = {r['origin']['id']: r for r in defaults} if name == 'Demos' else {}
        folder = next((f for f in self.load()['folders'] if f['name'].lower() == name.lower()), None)
        if folder:
            root = ET.Element('XML', schemaVersion='8', release='custom-collection')
            for entry in folder['apps']:
                node = ET.SubElement(root, 'Demo')
                for key, value in entry.items():
                    ET.SubElement(node, key).text = value
            for recipe in load_catalog(ET.tostring(root), allow_external=True):
                if name != 'Demos':
                    recipe['origin']['id'] = 'collection-' + hashlib.sha256((folder['name'].lower() + '\0' + recipe['origin']['id']).encode()).hexdigest()[:48]
                result[recipe['origin']['id']] = recipe
        return list(result.values())
