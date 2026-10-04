# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Mac/Windows-compatible app metadata with Linux-owned recovery and locking."""
import base64
import copy
import fcntl
import math
import os
from pathlib import Path
import re
import threading
import time
import uuid
from .files import (Work, atomic_json, beneath, canonical_entries, copy_file,
                    copy_tree, delete_tree, inventory, no_links, read_json, walk, within)

DRIVE_C = 'home/username/.wine/drive_c'
EPOCH = 978307200
WINDOWS = {'wineDefault': "Use Wine’s default", 'win11': 'Windows 11', 'win10': 'Windows 10',
           'win81': 'Windows 8.1', 'win8': 'Windows 8', 'win7': 'Windows 7', 'vista': 'Windows Vista',
           'winxp': 'Windows XP', 'win2k': 'Windows 2000', 'winme': 'Windows ME', 'win98': 'Windows 98',
           'win95': 'Windows 95', 'nt40': 'Windows NT 4.0', 'win31': 'Windows 3.1'}
RENDERERS = {'wineDefault': "Use Wine’s default", 'openGL': 'OpenGL', 'gdi': 'GDI'}

def now():
    return time.time() - EPOCH

def identity(value):
    result = uuid.UUID(value)
    if result.int == 0:
        raise ValueError('Invalid app identity.')
    return str(result).upper()

def new_app(name, wine):
    return dict(id=str(uuid.uuid4()).upper(), name=name, createdAt=now(), resolution='1024x768',
                fullScreen=False, arguments=[], winePackage=copy.deepcopy(wine), savedWineVersion=wine['wineVersion'])

def built_in(app):
    return app.get('builtInProgram') or ('notepad' if app.get('isNotepad') else None)

def preference(app, kind):
    if app.get(kind) is not None:
        return app[kind]
    demo = app.get('demoSettings') or {}
    if kind == 'windowsVersion':
        return demo.get(kind, 'wineDefault')
    if kind == 'wineRenderer' and 'gdi' in demo:
        return 'gdi' if demo['gdi'] else 'openGL'
    if kind == 'openGLBackend' and 'useEGL' in demo:
        return 'egl' if demo['useEGL'] else 'glx'
    return 'wineDefault'

def resolution(value):
    if not isinstance(value, str) or not re.fullmatch(r'[1-9]\d{2,3}x[1-9]\d{2,3}', value) or not all(320 <= int(x) <= 8192 for x in value.split('x')):
        raise ValueError('Use a resolution from 320x320 to 8192x8192.')

def validate_reference(wine):
    if (not re.fullmatch('[a-f0-9]{64}', wine['sha256']) or not 0 < wine['bytes'] <= 4 * 1024**3 or
        not re.fullmatch(r'\d{1,3}\.\d{1,3}(?:[.\-][A-Za-z0-9]+)*', wine['wineVersion']) or int(wine['filesystemVersion']) < 1):
        raise ValueError('Invalid Wine package identity.')

def maintenance(path):
    return bool(re.match(r'(unins|unwise|setup|install|update|crash)', Path(path).stem, re.I))

class Library:
    def __init__(self, directory):
        self.directory = no_links(directory)
        self.directory.mkdir(parents=True, exist_ok=True)
        self.mutex = threading.RLock()
        self.lock = no_links(self.directory / '.linux-library.lock').open('a+b')
        try:
            fcntl.flock(self.lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            self.lock.close()
            raise ValueError('This library is already open in another copy of Boxedwine.') from None
        try:
            self.load()
        except Exception:
            self.close()
            raise

    def close(self):
        self.lock.close()

    def path(self, app):
        return beneath(self.directory, 'Applications/' + identity(app['id']))

    def root(self, app):
        return beneath(self.path(app), 'root')

    def package_path(self, wine):
        validate_reference(wine)
        return beneath(self.directory, 'WinePackages/' + wine['sha256'] + '.zip')

    def default_wine(self):
        path = beneath(self.directory, 'WindowsSupport/imported-wine.json')
        wine = read_json(path) if path.exists() else None
        if wine:
            validate_reference(wine)
        return wine

    def set_default_wine(self, wine):
        validate_reference(wine)
        atomic_json(beneath(self.directory, 'WindowsSupport/imported-wine.json'), wine)

    def wine_path(self, app):
        if app.get('winePackage'):
            return self.package_path(app['winePackage'])
        if app.get('savedWineVersion'):
            return beneath(self.path(app), 'WindowsSupport/wine.zip')
        default = self.default_wine()
        if not default:
            raise ValueError('Windows support is missing. Set it up in Settings.')
        return self.package_path(default)

    def settings(self):
        path = self.directory / 'linux-settings.json'
        return read_json(path) if path.exists() else {'theme': 'System', 'deleteImmediately': False}

    def save_settings(self, settings):
        atomic_json(self.directory / 'linux-settings.json', settings)

    def validate_app(self, app):
        from .runtime import validate_overrides
        identity(app['id'])
        if not isinstance(app['name'], str) or not app['name'].strip() or len(app['name']) > 1024:
            raise ValueError('Enter an app name of at most 1024 characters.')
        if not math.isfinite(app['createdAt']):
            raise ValueError('Invalid creation date.')
        resolution(app.get('resolution', '1024x768'))
        validate_overrides(app.get('boxedwineArguments') or [])
        args = app.get('arguments', [])
        if not isinstance(args, list) or len(args) > 256 or any(not isinstance(a, str) or len(a) > 8192 or '\0' in a for a in args):
            raise ValueError('Invalid app arguments.')
        if app.get('executable'):
            executable = app['executable']
            if not executable.startswith(DRIVE_C + '/') or not executable.lower().endswith('.exe'):
                raise ValueError('Choose an .exe inside the app’s drive C.')
            beneath(self.root(app), executable)
        if app.get('installer'):
            beneath(self.path(app), app['installer'])
        if app.get('winePackage'):
            validate_reference(app['winePackage'])
            if app['winePackage']['wineVersion'] != app.get('savedWineVersion'):
                raise ValueError('Wine version does not match its package.')
        for field, choices in [('windowsVersion', WINDOWS), ('wineRenderer', RENDERERS), ('openGLBackend', ('wineDefault', 'egl', 'glx'))]:
            if preference(app, field) not in choices or (app.get(field + 'Pending') and not app.get(field)):
                raise ValueError('Invalid Wine configuration setting.')
        observed = app.get('wineConfigurationCustom', {})
        if (not isinstance(observed, dict) or any(k not in ('windowsVersion', 'wineRenderer', 'openGLBackend') or
                not isinstance(v, str) or not 0 < len(v) <= 512 for k, v in observed.items()) or
                not isinstance(app.get('wineConfigurationRefreshPending', False), bool)):
            raise ValueError('Invalid observed Wine configuration.')
        if built_in(app) not in (None, 'notepad', 'minesweeper'):
            raise ValueError('Unknown built-in program.')
        if app.get('customIconPNG'):
            png = base64.b64decode(app['customIconPNG'], validate=True)
            if len(png) > 4 * 1024**2 or not png.startswith(b'\x89PNG\r\n\x1a\n'):
                raise ValueError('The app icon must be a PNG of at most 4 MB.')
        demo = app.get('demoSettings') or {}
        for field in ('resolution', 'installResolution'):
            if demo.get(field):
                resolution(demo[field])
        if demo.get('bitsPerPixel', 32) not in (8, 16, 32) or not 1 <= demo.get('cpuCount', 1) <= 64:
            raise ValueError('Invalid demo display or CPU setting.')

    def validate(self, document):
        if not 1 <= document['version'] <= 13 or not isinstance(document['apps'], list) or not isinstance(document.get('removedApps', []), list):
            raise ValueError('Unsupported or incomplete library format. Your files were kept.')
        seen = set()
        for app in document['apps'] + [r['app'] for r in document.get('removedApps', [])]:
            self.validate_app(app)
            key = identity(app['id'])
            if key in seen:
                raise ValueError('Duplicate app identity.')
            seen.add(key)

    def load(self):
        with self.mutex:
            path = self.directory / 'library.json'
            result = read_json(path) if path.exists() else dict(version=13, apps=[], removedApps=[])
            self.validate(result)
            result.setdefault('removedApps', [])
            return result

    def save(self, document):
        with self.mutex:
            self.validate(document)
            path = self.directory / 'library.json'
            if path.exists():
                atomic_json(self.directory / 'library-previous.json', self.load())
            document['version'] = 13
            atomic_json(path, document)

    def update(self, app):
        with self.mutex:
            document = self.load()
            for i, current in enumerate(document['apps']):
                if identity(current['id']) == identity(app['id']):
                    document['apps'][i] = copy.deepcopy(app)
                    self.save(document)
                    return
            raise ValueError('This app is no longer in the library.')

    def programs(self, app):
        drive = beneath(self.root(app), DRIVE_C)
        if not drive.exists():
            return []
        paths = [p.relative_to(self.root(app)).as_posix() for p in walk(drive) if p.is_file() and p.suffix.lower() == '.exe']
        return sorted(paths, key=lambda p: (maintenance(p), Path(p).name.casefold(), p))

    def select_installed_demo(self, app):
        candidates = [p for p in self.programs(app) if not p.casefold().startswith(DRIVE_C + '/windows/')]
        if app.get('executable') in candidates:
            return app
        if app.get('demo'):
            matches = [p for p in candidates if Path(p).name.casefold() == app['demo']['shortcutExe'].casefold()]
            if len(matches) == 1:
                app = copy.deepcopy(app); app['executable'] = matches[0]; self.update(app)
        return app

    def operation_path(self, operation):
        return beneath(self.directory, 'LinuxOperations/' + identity(operation['id']) + '.json')

    def record(self, operation):
        atomic_json(self.operation_path(operation), operation)

    def begin(self, app, kind):
        self.validate_app(app)
        if self.path(app).exists():
            raise ValueError('The app directory already exists.')
        operation = dict(id=app['id'], app=app, kind=kind, name=app['name'], ready=False, createdAt=now())
        self.record(operation)
        self.root(app).mkdir(parents=True)
        return operation

    def commit(self, operation, work):
        operation['entries'] = inventory(self.path(operation['app']), work)
        work.check()
        operation['ready'] = True
        self.record(operation)
        self.finish(operation, work)

    def finish(self, operation, work):
        if operation.get('problem'):
            raise ValueError(operation['problem'])
        if operation['kind'] == 'backup':
            return self.finish_backup(operation, work)
        if not operation.get('ready') or identity(operation['id']) != identity(operation['app']['id']):
            raise ValueError('This partial copy is not ready. Inspect its files or discard it.')
        with self.mutex:
            document = self.load()
            all_apps = document['apps'] + [r['app'] for r in document['removedApps']]
            if not any(identity(a['id']) == identity(operation['id']) for a in all_apps):
                if inventory(self.path(operation['app']), work) != canonical_entries(operation['entries']):
                    raise ValueError('The completed copy has changed. Its files were kept for review.')
                document['apps'].append(operation['app']); self.save(document)
            self.operation_path(operation).unlink()

    def pending(self):
        folder = beneath(self.directory, 'LinuxOperations')
        result = []
        for path in sorted(folder.glob('*.json')):
            try:
                operation = read_json(path)
                if identity(operation['id']) != identity(path.stem):
                    raise ValueError('Invalid recovery identity.')
                if operation['kind'] == 'backup':
                    staging = self.export_staging(operation)
                    # The manifest is written before the ready journal update.
                    # Offer verification/finish after a crash in that gap too.
                    if (staging / 'Manifest.json').is_file():
                        self.check_export_marker(operation)
                        operation['ready'] = True
                else:
                    self.validate_app(operation['app'])
                    if identity(operation['app']['id']) != identity(operation['id']):
                        raise ValueError('Invalid app recovery identity.')
                result.append(operation)
            except Exception as error:
                result.append(dict(id=path.stem, name='Unreadable recovery record', problem=str(error), ready=False))
        return result

    def discard(self, operation):
        if operation.get('problem'):
            raise ValueError('Inspect the unreadable recovery record before removing any files.')
        if operation['kind'] == 'backup':
            staging = self.export_staging(operation)
            if staging.exists():
                self.check_export_marker(operation)
                if (staging / 'Manifest.json').exists():
                    raise ValueError('This backup may be complete. Finish it or inspect its folder.')
                delete_tree(staging, staging.parent)
        else:
            document = self.load()
            apps = document['apps'] + [r['app'] for r in document['removedApps']]
            if not any(identity(a['id']) == identity(operation['id']) for a in apps):
                if identity(operation['id']) != identity(operation['app']['id']):
                    raise ValueError('Invalid recovery identity.')
                delete_tree(self.path(operation['app']), self.directory / 'Applications')
        self.operation_path(operation).unlink(missing_ok=True)

    def import_app(self, source, kind, wine, work, windows='wineDefault', installer=None, name=None):
        source = no_links(source)
        if source == self.directory or within(source, self.directory) or within(self.directory, source):
            raise ValueError('Choose a source outside the Boxedwine library.')
        app = new_app(name or (source.stem if kind == 'installer' else source.name), wine)
        if windows != 'wineDefault':
            app.update(windowsVersion=windows, windowsVersionPending=True)
        operation = self.begin(app, kind)
        try:
            if kind == 'folder':
                if not source.is_dir():
                    raise ValueError('Choose a portable app folder.')
                copy_tree(source, beneath(self.root(app), DRIVE_C + '/App'), work)
                programs = self.programs(app)
                if not programs:
                    raise ValueError('This folder has no Windows programs (.exe).')
                primary = [p for p in programs if not maintenance(p)]
                if len(primary) == 1:
                    app['executable'] = primary[0]
            elif kind == 'installerFolder':
                installer = check_installer(installer)
                if not within(installer, source):
                    raise ValueError('Choose an installer inside the selected folder.')
                app['installer'] = 'Installer/' + installer.relative_to(source).as_posix()
                copy_tree(source, beneath(self.path(app), 'Installer'), work)
            elif kind == 'installer':
                check_installer(source)
                app['installer'] = 'Installer/' + source.name
                copy_file(source, beneath(self.path(app), app['installer']), work)
            else:
                raise ValueError('Unknown import type.')
            self.commit(operation, work)
            return app
        except Exception:
            if not operation['ready']:
                self.discard(operation)
            raise

    def add_builtin(self, program, wine, work):
        app = new_app('Notepad' if program == 'notepad' else 'Minesweeper', wine)
        app['builtInProgram'] = program
        operation = self.begin(app, 'builtin')
        try:
            self.commit(operation, work)
        except Exception:
            if not operation['ready']:
                self.discard(operation)
            raise
        return app

    def remove(self, app):
        with self.mutex:
            document = self.load()
            saved = next(a for a in document['apps'] if a['id'] == app['id'])
            document['apps'].remove(saved)
            document['removedApps'].insert(0, dict(app=saved, removedAt=now()))
            self.save(document)

    def restore(self, app):
        with self.mutex:
            document = self.load()
            removed = next(r for r in document['removedApps'] if r['app']['id'] == app['id'])
            if removed.get('deletionStartedAt') is not None:
                raise ValueError('Deletion has started. Finish deleting this app.')
            document['removedApps'].remove(removed); document['apps'].append(removed['app']); self.save(document)

    def delete(self, app):
        with self.mutex:
            document = self.load()
            removed = next(r for r in document['removedApps'] if r['app']['id'] == app['id'])
            removed['deletionStartedAt'] = now(); self.save(document)
            delete_tree(self.path(app), self.directory / 'Applications')
            document['removedApps'].remove(removed); self.save(document)

    def export_staging(self, operation):
        if operation.get('kind') != 'backup' or operation.get('app') or not Path(operation['exportPath']).is_absolute():
            raise ValueError('Invalid backup recovery record.')
        target = no_links(operation['exportPath'])
        if target == self.directory or within(target, self.directory):
            raise ValueError('Save backups outside the library.')
        return no_links(str(target) + '.partial-' + uuid.UUID(operation['id']).hex)

    def check_export_marker(self, operation):
        marker = no_links(self.export_staging(operation) / '.BoxedwineExport')
        if marker.stat().st_size > 128 or marker.read_text() != operation['id']:
            raise ValueError('The interrupted backup folder cannot be identified. Its files were kept.')

    def backup(self, app, target, work):
        from .packages import validate_wine
        target = no_links(target)
        if target.exists():
            raise ValueError('Choose a new backup name. Existing backups are never overwritten.')
        operation = dict(id=str(uuid.uuid4()), kind='backup', name=app['name'], exportPath=str(target), ready=False, createdAt=now())
        staging = self.export_staging(operation)
        if staging.exists():
            raise ValueError('The temporary backup folder already exists.')
        self.record(operation)
        staging.mkdir(parents=True)
        (staging / '.BoxedwineExport').write_text(operation['id'])
        try:
            source = self.path(app); application = staging / 'Application'
            before = inventory(source, work)
            copy_tree(source, application, work)
            if inventory(application, work) != before or inventory(source, work) != before:
                raise ValueError('The app changed while its backup was copied.')
            snapshot = beneath(application, 'WindowsSupport/wine.zip')
            snapshot.unlink(missing_ok=True)
            copy_file(self.wine_path(app), snapshot, work)
            reference = validate_wine(snapshot, work)
            if app.get('winePackage') and reference != app['winePackage']:
                raise ValueError('The Wine package changed while backing up.')
            saved = copy.deepcopy(app); saved.pop('winePackage', None); saved['savedWineVersion'] = reference['wineVersion']
            atomic_json(staging / 'Manifest.json', dict(format=8, createdAt=now(), app=saved, wineVersion=reference['wineVersion'], entries=inventory(application, work)))
            operation['ready'] = True; self.record(operation)
            self.finish_backup(operation, work)
        except Exception:
            if not operation['ready'] and not (staging / 'Manifest.json').exists():
                self.discard(operation)
            raise

    def finish_backup(self, operation, work):
        target = no_links(operation['exportPath']); staging = self.export_staging(operation)
        # Recover a crash after rename, before removing the journal.
        if target.exists() and not staging.exists():
            marker = no_links(target / '.BoxedwineExport')
            if marker.read_text() != operation['id']:
                raise ValueError('The destination belongs to a different backup.')
            self.check_backup(target, work)
            self.operation_path(operation).unlink(); marker.unlink(); return
        self.check_export_marker(operation)
        self.check_backup(staging, work)
        if target.exists():
            raise ValueError('The backup destination already exists.')
        work.check()
        os.rename(staging, target)
        self.operation_path(operation).unlink()
        (target / '.BoxedwineExport').unlink()

    def check_backup(self, source, work):
        manifest = read_json(no_links(source) / 'Manifest.json')
        if not 1 <= manifest['format'] <= 8:
            raise ValueError('Unsupported backup format.')
        self.validate_app(manifest['app'])
        if inventory(beneath(source, 'Application'), work) != canonical_entries(manifest['entries']):
            raise ValueError('The backup inventory does not match its files.')
        return manifest

    def restore_backup(self, source, work):
        from .packages import import_wine
        source = no_links(source)
        manifest = self.check_backup(source, work)
        app = copy.deepcopy(manifest['app'])
        app.update(id=str(uuid.uuid4()).upper(), createdAt=now())
        app.pop('lastOpened', None)
        operation = self.begin(app, 'restore')
        try:
            copy_tree(source / 'Application', self.path(app), work)
            if inventory(self.path(app), work) != canonical_entries(manifest['entries']):
                raise ValueError('The backup changed during restore.')
            snapshot = beneath(self.path(app), 'WindowsSupport/wine.zip')
            wine = import_wine(self, snapshot, work)
            if wine['wineVersion'] != manifest['wineVersion']:
                raise ValueError('The backup Wine version does not match its manifest.')
            app.update(winePackage=wine, savedWineVersion=wine['wineVersion'])
            snapshot.unlink()
            self.commit(operation, work); return app
        except Exception:
            if not operation['ready']:
                self.discard(operation)
            raise

    def prune_wine(self):
        document = self.load(); pending = self.pending()
        if any(p.get('problem') for p in pending):
            raise ValueError('Review unreadable recovery records before cleaning Wine packages.')
        apps = document['apps'] + [r['app'] for r in document['removedApps']] + [p['app'] for p in pending if p.get('app')]
        referenced = {a['winePackage']['sha256'] for a in apps if a.get('winePackage')}
        if default := self.default_wine():
            referenced.add(default['sha256'])
        for path in beneath(self.directory, 'WinePackages').glob('*.zip'):
            if re.fullmatch('[a-f0-9]{64}', path.stem) and path.stem not in referenced:
                no_links(path).unlink()

def check_installer(path):
    path = no_links(path)
    if not path.is_file() or path.suffix.lower() not in ('.exe', '.msi'):
        raise ValueError('Choose a Windows .exe or .msi installer.')
    return path
