#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise native screens against an isolated fixture library, with X11 captures."""
import argparse
import copy
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import traceback
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from boxedwine.app import Application, Window, Adw, Gdk, Gio, GLib, Gtk, gi
from boxedwine import dialogs, demos
from boxedwine.files import Work, atomic_json, copy_file
from boxedwine.library import Library, new_app, now
from boxedwine.packages import Resources, import_wine, catalog_files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--catalog', type=Path, help='Optional exact pinned catalog ZIP')
    options = parser.parse_args()
    options.output.mkdir(parents=True, exist_ok=True)
    temporary = tempfile.TemporaryDirectory(prefix='boxedwine-gui-')
    root = Path(temporary.name)
    library = Library(root / 'library')
    wine_fixture(root / 'wine.zip'); wine = import_wine(library, root / 'wine.zip', Work()); library.set_default_wine(wine)
    notepad = library.add_builtin('notepad', wine, Work())
    mines = library.add_builtin('minesweeper', wine, Work())
    source = root / 'Portable Game'; source.mkdir()
    (source / 'game.exe').write_bytes(b'MZ'); (source / 'config.exe').write_bytes(b'MZ')
    game = library.import_app(source, 'folder', wine, Work(), name='A Portable Windows Game')
    game.update(executable=library.programs(game)[0], lastOpened=now()); library.update(game)
    removed = library.add_builtin('notepad', wine, Work()); removed['name'] = 'An Older App'; library.update(removed); library.remove(removed)
    library.begin(new_app('Interrupted Import', wine), 'folder')
    catalog = None
    if options.catalog:
        pin = __import__('json').loads(Resources().path('demo-catalog.lock.json').read_text())
        copy_file(options.catalog, library.directory / 'Catalogs' / (pin['sha256'] + '.zip'), Work())
        catalog = catalog_files(Resources(), library, Work(), False)
    library.close()
    app = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
    state = {'step': 0, 'dialog': None, 'error': None, 'ticks': 0}
    captures = []

    def capture(name):
        window = app.window
        if Gdk.Display.get_default().get_name().startswith(':') and shutil.which('import'):
            gi.require_version('GdkX11', '4.0')
            from gi.repository import GdkX11
            surface = window.get_surface()
            xid = GdkX11.X11Surface.get_xid(surface)
            destination = options.output / (name + '.png')
            subprocess.run(['import', '-window', str(xid), str(destination)], check=True, timeout=10)
            captures.append(str(destination))

    def open_dialog(callback):
        if visible := app.window.get_visible_dialog():
            visible.close()
        state['dialog'] = callback()

    def section(title):
        if visible := app.window.get_visible_dialog():
            visible.close()
        state['dialog'] = None
        app.window.navigate(title)

    def widgets(widget):
        yield widget
        child = widget.get_first_child()
        while child:
            yield from widgets(child)
            child = child.get_next_sibling()

    def save_settings():
        dialog = state['dialog']
        name = next(w for w in widgets(dialog) if isinstance(w, Adw.EntryRow) and w.get_title() == 'Name')
        name.set_text('Renamed Test App')
        action = next(w for w in widgets(dialog) if isinstance(w, Gtk.Button) and w.get_label() == 'Save')
        action.emit('clicked')
        assert app.window.library.load()['apps'][2]['name'] == 'Renamed Test App'
        state['dialog'] = None

    steps = [
        ('library', lambda: app.window.show_app(notepad)),
        ('app-settings', lambda: open_dialog(lambda: dialogs.app_settings(app.window, game))),
        ('app-advanced', lambda: state['dialog'].set_visible_page_name('Advanced')),
        ('settings-saved', save_settings),
        ('program-picker', lambda: open_dialog(lambda: dialogs.program_picker(app.window, game, lambda _: None, external=True))),
        ('add-app', lambda: open_dialog(lambda: dialogs.add_app(app.window, source))),
        ('settings', lambda: open_dialog(lambda: dialogs.settings(app.window))),
        ('troubleshooting', lambda: open_dialog(lambda: dialogs.troubleshoot(app.window, game))),
        ('logs', lambda: open_dialog(lambda: dialogs.logs(app.window, game))),
        ('removed', lambda: (section('Removed Apps'), app.window.show_app(removed))),
        ('recovery', lambda: section('Recovery')),
        ('recent', lambda: section('Recently Opened')),
        ('empty-search', lambda: app.window.search.set_text('not a matching app')),
        ('dark', lambda: (section('All Apps'), Adw.StyleManager.get_default().set_color_scheme(Adw.ColorScheme.FORCE_DARK), app.window.show_app(app.window.find_app(game['id'])))),
        ('compact', lambda: app.window.set_default_size(640, 700)),
    ]
    if catalog:
        steps.insert(11, ('demos', lambda: (section('Demos'), app.window.show_demo(app.window.catalog[0]))))

    def tick():
        try:
            if not isinstance(app.window, Window):
                raise AssertionError('Application window failed to initialize')
            if state['ticks'] == 0:
                def unexpected(error):
                    raise AssertionError('Unexpected UI error: ' + str(error))
                app.window.error = unexpected
                if catalog:
                    app.window.catalog_data = catalog; app.window.catalog = demos.load_catalog(catalog['catalog.xml']); app.window.catalog_loaded = True
            state['ticks'] += 1
            index = state['step']
            if index and index <= len(steps):
                print(steps[index - 1][0], app.window.view, app.window.sections.get_selected_row().title, flush=True)
                capture(steps[index - 1][0])
            if index == len(steps):
                print('Visited ' + str(len(steps)) + ' native screens; ' + str(len(captures)) + ' screenshots in ' + str(options.output))
                app.quit(); return GLib.SOURCE_REMOVE
            steps[index][1]()
            state['step'] += 1
            return GLib.SOURCE_CONTINUE
        except Exception:
            state['error'] = traceback.format_exc(); print(state['error'], file=sys.stderr)
            app.quit(); return GLib.SOURCE_REMOVE
    GLib.timeout_add(1000, tick)
    app.run([sys.argv[0]])
    temporary.cleanup()
    return 1 if state['error'] else 0


if __name__ == '__main__':
    sys.exit(main())
