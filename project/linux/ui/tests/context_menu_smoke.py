#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise card menus with real X11 pointer/keyboard events in a temporary library."""
from pathlib import Path
import sys
import tempfile
import traceback
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from pointer_smoke import Pointer
from test_core import wine_fixture
from boxedwine.app import Application, GLib, Gtk
from boxedwine.files import Work, storage_size
from boxedwine.library import Library, now
from boxedwine.packages import import_wine


def widgets(widget):
    yield widget
    child = widget.get_first_child()
    while child:
        yield from widgets(child)
        child = child.get_next_sibling()


def entries(card):
    assert card.menu and card.menu.get_visible(), 'The context menu must be visible'
    return {next(w.get_text() for w in widgets(button) if isinstance(w, Gtk.Label)): button
            for button in widgets(card.menu) if button.get_css_name() == 'modelbutton'}


def main():
    temporary = tempfile.TemporaryDirectory(prefix='boxedwine-context-')
    root = Path(temporary.name)
    library = Library(root / 'library')
    wine_fixture(root / 'wine.zip'); wine = import_wine(library, root / 'wine.zip', Work())
    apps = []
    for i in range(3):
        app = library.add_builtin('notepad', wine, Work())
        app.update(name=f'App {i}', lastOpened=now()); library.update(app); apps.append(app)
    removed = library.add_builtin('notepad', wine, Work()); library.remove(removed)
    (library.root(apps[1]) / 'game.dat').write_bytes(b'x' * (2 * 1024**2))
    size = storage_size(library.path(apps[1]), Work())
    library.close()
    application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
    state = {'failed': False, 'pointer': None}
    calls = []

    def steps():
        window = application.window
        pointer = state['pointer'] = Pointer(window)
        window.open_app = lambda app: calls.append(('open', app['id']))
        window.install_demo = lambda recipe: calls.append(('install', recipe['origin']['id']))
        window.export_backup = lambda app: calls.append(('backup', app['id']))
        window.stop = lambda app: calls.append(('stop', app['id']))
        window.delete_app = lambda app: calls.append(('delete', app['id']))
        window.restore_app = lambda app: calls.append(('restore', app['id']))
        target = apps[1]['id']
        card = window.cards[target]
        pointer.point_at(card); pointer.click(3); yield 500
        assert window.selected == target and not calls, 'Right-click selects its target without launching it'
        choices = entries(card)
        assert set(choices) == {'Open App', 'Run Another Program…', 'App Settings…', 'Troubleshooting…',
                                'View Launch Logs…', 'Export Backup…', 'Remove from Library…'}, choices
        labels = [w.get_text() for w in widgets(window.details_toolbar) if isinstance(w, Gtk.Label)]
        assert 'Storage' in labels and GLib.format_size(size) in labels, labels
        assert not any(' on disk' in label or ' in files' in label for label in labels), labels
        pointer.point_at(choices['Open App']); pointer.click(); yield 300
        assert calls == [('open', target)] and card.menu is None, calls
        calls.clear()
        # Closing with Escape preserves selection and never activates a command.
        pointer.point_at(card); pointer.click(3); yield 250
        pointer.key('Escape'); yield 250
        assert card.menu is None and not calls and window.selected == target
        # Keyboard access uses the focused card, even when another was selected.
        card = window.cards[apps[2]['id']]; card.grab_focus()
        pointer.key('F10', ('Shift_L',)); yield 250
        assert window.selected == apps[2]['id']
        choices = entries(card)
        pointer.point_at(choices['App Settings…']); pointer.click(); yield 400
        dialog = window.get_visible_dialog()
        assert dialog and dialog.get_title() == 'App Settings', 'Settings must open after dismissing the popover'
        dialog.close(); yield 400
        card.grab_focus(); pointer.key('Menu'); yield 250
        choices = entries(card)
        pointer.point_at(choices['Export Backup…']); pointer.click(); yield 250
        assert calls == [('backup', apps[2]['id'])], calls
        calls.clear()
        # A running app exposes Stop and disables actions that change its files.
        window.sessions[target] = object()
        card = window.cards[target]
        pointer.point_at(card); pointer.click(3); yield 250
        choices = entries(card)
        for label in ('Open App', 'App Settings…', 'Run Another Program…', 'Export Backup…', 'Remove from Library…'):
            assert not choices[label].is_sensitive(), label
        assert choices['Stop App'].is_sensitive() and choices['View Launch Logs…'].is_sensitive()
        pointer.point_at(choices['Stop App']); pointer.click(); yield 250
        assert calls == [('stop', target)], calls
        window.stopping.add(target)
        pointer.point_at(card); pointer.click(3); yield 250
        assert not entries(card)['Stopping…'].is_sensitive()
        card.close_menu(); window.sessions.clear(); window.stopping.clear(); calls.clear()
        window.syncing.add(target)
        pointer.point_at(card); pointer.click(3); yield 250
        assert not entries(card)['App Settings…'].is_sensitive()
        card.close_menu(); window.syncing.clear()
        # Replacing the card grid must dismiss and detach an open menu.
        pointer.point_at(card); pointer.click(3); yield 250
        old_menu = card.menu
        window.navigate('Recently Opened'); yield 250
        assert card.menu is None and old_menu.get_parent() is None
        card = window.cards[target]
        pointer.point_at(card); pointer.click(3); yield 250
        assert 'Open App' in entries(card)
        window.navigate('Removed Apps'); yield 250
        card = window.cards[removed['id']]
        pointer.point_at(card); pointer.click(3); yield 250
        assert set(entries(card)) == {'Restore App', 'Delete Permanently…'}
        pointer.point_at(entries(card)['Restore App']); pointer.click(); yield 250
        assert calls == [('restore', removed['id'])], calls
        calls.clear()
        # Incomplete deletion cannot offer a restore action.
        window.document['removedApps'][0]['deletionStartedAt'] = now()
        pointer.point_at(card); pointer.click(3); yield 250
        assert set(entries(card)) == {'Finish Deleting'}
        pointer.point_at(entries(card)['Finish Deleting']); pointer.click(); yield 250
        assert calls == [('delete', removed['id'])], calls
        calls.clear()
        recipe = dict(origin=dict(id='context-demo'), name='Context Demo', summary='Test recipe', icon='', bytes=1024,
                      wineVersion='11.0', help='', url='https://example.org/demo.zip')
        window.default_catalog = [recipe]; window.catalog_loaded = True
        window.navigate('Demos'); yield 250
        card = window.cards['context-demo']
        pointer.point_at(card); pointer.click(3); yield 250
        assert set(entries(card)) == {'Download & Install'} and not calls
        pointer.point_at(entries(card)['Download & Install']); pointer.click(); yield 250
        assert calls == [('install', 'context-demo')], calls
        calls.clear()
        window.set_default_size(680, 780); yield 500
        window.navigate('All Apps'); yield 500
        assert window.details_split.get_collapsed() and not window.details_split.get_show_sidebar()
        card = window.cards[target]
        pointer.point_at(card); pointer.click(3); yield 500
        assert window.selected == target and not window.details_split.get_show_sidebar(), 'A context menu must not cover the narrow grid with details'
        assert entries(card) and not calls
        pointer.key('Escape'); yield 250
        assert card.menu is None
        # Primary double-click remains independent of the secondary gesture.
        pointer.point_at(card); pointer.click(); yield 100
        pointer.click(); yield 250
        assert calls == [('open', target)], calls
        print('PASS: app/removed/demo menus, target selection, action routing, running/syncing guards, keyboard access, cleanup, narrow layout, double-click, and simple storage.', flush=True)

    iterator = None
    def advance():
        nonlocal iterator
        try:
            if iterator is None:
                iterator = steps()
            GLib.timeout_add(next(iterator), advance)
        except StopIteration:
            application.quit()
        except Exception:
            traceback.print_exc(); state['failed'] = True; application.quit()
        return GLib.SOURCE_REMOVE
    GLib.timeout_add(1000, advance)
    application.run([sys.argv[0]])
    if state['pointer']:
        state['pointer'].x.XCloseDisplay(state['pointer'].display)
    temporary.cleanup()
    return int(state['failed'])


if __name__ == '__main__':
    sys.exit(main())
