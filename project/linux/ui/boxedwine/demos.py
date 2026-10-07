# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import re
from pathlib import Path
import urllib.parse
import xml.etree.ElementTree as ET
import zipfile
from .files import beneath, delete_tree, extract_zip
from .library import DRIVE_C, new_app, preference, resolution, check_installer
from .packages import download, trusted_url


def load_catalog(data):
    if len(data) > 1024**2 or b'<!DOCTYPE' in data.upper() or b'<!ENTITY' in data.upper():
        raise ValueError('Invalid demo catalog XML.')
    root = ET.fromstring(data)
    if root.tag != 'XML' or not 1 <= int(root.get('schemaVersion', '0')) <= 7 or not root.get('release'):
        raise ValueError('Unsupported demo catalog.')
    recipes, ids = [], set()
    for element in root:
        values = {child.tag: (child.text or '').strip() for child in element}
        if element.tag != 'Demo' or len(values) != len(element) or len(recipes) >= 256:
            raise ValueError('Invalid or duplicate demo fields.')
        def field(name):
            return values.get(name, '')
        def boolean(name):
            value = values.get(name)
            if value is not None and value not in ('true', 'false'):
                raise ValueError('Invalid demo boolean: ' + name)
            return None if value is None else value == 'true'
        identifier = field('ID')
        shortcut = field('ShortcutExe')
        url, size, checksum = field('FileURL'), int(field('FileSizeBytes')), field('FileSHA256')
        if (not re.fullmatch('[a-z0-9][a-z0-9-]{0,63}', identifier) or identifier in ids or not 0 < len(field('Name')) <= 256 or
                not trusted_url(url) or not 0 < size <= 1024**3 or not re.fullmatch('[a-f0-9]{64}', checksum) or
                not shortcut.lower().endswith('.exe') or '/' in shortcut or field('InstallType') not in ('Zip', 'Installer')):
            raise ValueError('Invalid demo identity or package.')
        beneath('/tmp/boxedwine-catalog-validation', shortcut)
        if field('Icon'):
            beneath('/tmp/boxedwine-catalog-validation', field('Icon'))
        ids.add(identifier)
        if any(field(key) for key in ('Options', 'InstallOptions', 'Options_Mac', 'InstallOptions_Mac')):
            raise ValueError('Unsupported legacy options in demo ' + field('Name'))
        settings = {}
        for source, key in [('WindowsVersion', 'windowsVersion'), ('Resolution', 'resolution'), ('InstallResolution', 'installResolution')]:
            if source in values:
                settings[key] = values[source]
        for source, key in [('BitsPerPixel', 'bitsPerPixel'), ('CPUCount', 'cpuCount')]:
            if source in values:
                settings[key] = int(values[source])
        for source, key in [('GDIRenderer', 'gdi'), ('NativeOpenGL', 'nativeOpenGL'), ('UseEGL', 'useEGL'), ('CNCDDraw', 'cncDDraw'), ('DisableHideCursor', 'disableHideCursor'), ('ForceRelativeMouse', 'forceRelativeMouse')]:
            value = boolean(source)
            if value is not None:
                settings[key] = value
        for key in ('resolution', 'installResolution'):
            if key in settings:
                resolution(settings[key])
        if (settings.get('windowsVersion') not in (None, 'win98', 'winxp') or settings.get('nativeOpenGL') is False or
                settings.get('bitsPerPixel', 32) not in (8, 16, 32) or not 1 <= settings.get('cpuCount', 1) <= 64):
            raise ValueError('Unsupported demo settings.')
        zipped = urllib.parse.urlsplit(url).path.lower().endswith('.zip')
        if field('InstallType') == 'Zip' and not zipped:
            raise ValueError('Portable demos require a ZIP.')
        if field('InstallType') == 'Installer' and zipped:
            beneath('/tmp/boxedwine-catalog-validation', field('InstallExe'))
        if values.get('Glide') not in (None, 'psVoodoo') or values.get('CNCDDrawRenderer') not in (None, 'gdi', 'opengl', 'direct3d9'):
            raise ValueError('Unsupported demo graphics recipe.')
        if 'CNCDDrawFakeMode' in values and not re.fullmatch(r'[1-9][0-9]{0,3}x[1-9][0-9]{0,3}x(8|16|32)', values['CNCDDrawFakeMode']):
            raise ValueError('Invalid CNC DDraw mode.')
        recipes.append(dict(origin=dict(id=identifier, catalogRelease=root.get('release'), packageSHA256=checksum, shortcutExe=shortcut),
                            name=field('Name'), summary=field('Summary'), help=field('Help').replace('\\n', '\n').replace('\\t', '    '),
                            icon=field('Icon'), url=url, bytes=size, installType=field('InstallType'), installExe=field('InstallExe'),
                            wineVersion=field('WineVersion'), settings=settings, glide=values.get('Glide'),
                            cncRenderer=values.get('CNCDDrawRenderer'), cncUncapped=boolean('CNCDDrawUncapped') is True,
                            cncMode=values.get('CNCDDrawFakeMode')))
    return recipes


def configure_cnc(ini, recipe):
    rows = ini.replace('\r\n', '\n').split('\n')
    def set_value(section, key, value):
        matches = [i for i, row in enumerate(rows) if row.strip().casefold() == ('[' + section + ']').casefold()]
        if len(matches) > 1:
            raise ValueError('Duplicate CNC DDraw sections.')
        if not matches:
            rows.extend(['', '[' + section + ']', key + '=' + value])
            return
        start = matches[0]
        end = next((i for i in range(start + 1, len(rows)) if rows[i].strip().startswith('[')), len(rows))
        for i in range(end - 1, start, -1):
            if rows[i].split('=')[0].strip().casefold() == key.casefold():
                del rows[i]
        rows.insert(start + 1, key + '=' + value)
    if recipe.get('cncRenderer'):
        set_value('ddraw', 'renderer', recipe['cncRenderer'])
    if recipe.get('cncUncapped'):
        for key, value in [('maxfps', '0'), ('vsync', 'false'), ('maxgameticks', '-1')]:
            set_value('ddraw', key, value)
    if recipe.get('cncMode'):
        set_value(Path(recipe['origin']['shortcutExe']).stem, 'fake_mode', recipe['cncMode'])
    return ('\r\n' if '\r\n' in ini else '\n').join(rows)


def install(library, recipe, wine, work):
    document = library.load()
    if any((a.get('demo') or {}).get('id') == recipe['origin']['id'] for a in document['apps'] + [r['app'] for r in document['removedApps']]):
        raise ValueError('This demo is already in the library or Removed Apps. Open or restore that copy.')
    if wine['wineVersion'] != recipe['wineVersion']:
        raise ValueError('This demo requires Wine ' + recipe['wineVersion'])
    app = new_app(recipe['name'], wine)
    app.update(demo=recipe['origin'], demoSettings=recipe['settings'], resolution=recipe['settings'].get('resolution', '1024x768'))
    # Seed an editable per-app value only on first install.
    if recipe['origin']['id'] == 'alice':
        app['boxedwineArguments'] = ['-rel_mouse_sensitivity', '200']
    for setting, field in [('windowsVersion', 'windowsVersion'), ('gdi', 'wineRenderer'), ('useEGL', 'openGLBackend')]:
        if setting in recipe['settings']:
            app[field] = preference(app, field)
            app[field + 'Pending'] = True
    operation = library.begin(app, 'demo')
    try:
        filename = Path(urllib.parse.urlsplit(recipe['url']).path).name
        package = beneath(library.path(app), 'Download/' + filename)
        download(recipe['url'], recipe['bytes'], recipe['origin']['packageSHA256'], package, work)
        installer = recipe['installType'] == 'Installer'
        payload = beneath(library.path(app), 'Installer') if installer else beneath(library.root(app), DRIVE_C + '/App')
        if filename.lower().endswith('.zip'):
            extract_zip(package, payload, work)
            if installer:
                check_installer(beneath(payload, recipe['installExe']))
                app['installer'] = 'Installer/' + recipe['installExe']
        else:
            payload.mkdir(parents=True, exist_ok=True)
            app['installer'] = 'Installer/' + filename
            check_installer(package)
            package.rename(beneath(library.path(app), app['installer']))
        if not installer:
            matches = [p for p in library.programs(app) if Path(p).name.casefold() == recipe['origin']['shortcutExe'].casefold()]
            if len(matches) != 1:
                raise ValueError('The demo’s program could not be identified uniquely.')
            app['executable'] = matches[0]
        with zipfile.ZipFile(library.package_path(wine)) as archive:
            if recipe.get('glide') and DRIVE_C + '/windows/system32/glide2x.dll' not in archive.namelist():
                raise ValueError('This demo needs a Wine package with built-in Glide support.')
            if recipe['settings'].get('cncDDraw'):
                relative = DRIVE_C + '/ddraw/ddraw.ini'
                if archive.getinfo(relative).file_size > 65536:
                    raise ValueError('Invalid CNC DDraw settings.')
                ini = configure_cnc(archive.read(relative).decode('utf-8'), recipe)
                target = beneath(library.root(app), relative)
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text(ini)
        delete_tree(library.path(app) / 'Download', library.path(app))
        library.commit(operation, work)
        return app
    except Exception:
        if not operation['ready']:
            library.discard(operation)
        raise
