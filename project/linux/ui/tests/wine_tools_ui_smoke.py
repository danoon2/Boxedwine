#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Native picker, auxiliary-exit readback, retry, and custom-settings checks."""
import copy
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import traceback
from types import SimpleNamespace
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from boxedwine.app import Application, Window, GLib, Gtk, Adw, gi
from boxedwine import dialogs
from boxedwine.files import Work
from boxedwine.library import Library
from boxedwine.packages import import_wine
from boxedwine.runtime import WINE_TOOLS


def widgets(widget):
    yield widget
    child = widget.get_first_child()
    while child:
        yield from widgets(child)
        child = child.get_next_sibling()


def main():
    with tempfile.TemporaryDirectory(prefix='boxedwine-tools-ui-') as temporary:
        root = Path(temporary); os.environ['XDG_DATA_HOME'] = str(root / 'desktop')
        library = Library(root / 'library'); wine_fixture(root / 'wine.zip')
        wine = import_wine(library, root / 'wine.zip', Work()); saved = library.add_builtin('notepad', wine, Work()); library.close()
        application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
        state = {'failed': False, 'done': False}
        def steps():
            nonlocal saved
            window = application.window; assert isinstance(window, Window)
            calls, errors = [], []
            window.launch = lambda app, **kwargs: calls.append((app['id'], kwargs))
            window.error = lambda error: errors.append(str(error))
            window.run_another(saved)
            while window.busy or window.get_visible_dialog() is None:
                yield
            dialog = window.get_visible_dialog()
            rows = {w.get_title(): w for w in widgets(dialog) if isinstance(w, Adw.ActionRow)}
            assert set(name for name, _, _ in WINE_TOOLS.values()) <= set(rows)
            search = next(w for w in widgets(dialog) if isinstance(w, Gtk.SearchEntry))
            search.set_text('regedit'); search.emit('search-changed')
            assert rows['Registry Editor'].get_visible() and not rows['Wine Configuration'].get_visible()
            search.set_text(''); search.emit('search-changed')
            until = time.monotonic() + .5
            while time.monotonic() < until:
                yield
            gi.require_version('GdkX11', '4.0')
            from gi.repository import GdkX11
            xid = GdkX11.X11Surface.get_xid(window.get_surface())
            subprocess.run(['import', '-window', str(xid), '/tmp/boxedwine-wine-tools-picker.png'], check=True)
            rows['Registry Editor'].emit('activated')
            assert calls == [(saved['id'], {'tool': 'regedit'})]
            until = time.monotonic() + .4
            while time.monotonic() < until:
                yield
            # Failed readback releases the UI but retains a durable retry marker.
            saved['wineConfigurationRefreshPending'] = True; window.library.update(saved)
            session = SimpleNamespace(auxiliary=True, error=None, code=0, stopped=False, installing=False)
            with patch('boxedwine.app.refresh_configuration', side_effect=ValueError('Expected readback failure')):
                window.runtime_finished(saved, session)
                while window.busy:
                    yield
            assert errors and not window.syncing
            assert window.find_app(saved['id'])['wineConfigurationRefreshPending']
            errors.clear()
            def readback(emulator, library, app, wine, work):
                updated = copy.deepcopy(app)
                updated.pop('wineConfigurationRefreshPending', None)
                updated.update(windowsVersion='win98', wineRenderer='gdi', openGLBackend='glx')
                library.update(updated)
                return updated
            with patch('boxedwine.app.refresh_configuration', side_effect=readback):
                dialogs.app_settings(window, window.find_app(saved['id']))
                while window.busy:
                    yield
            dialog = window.get_visible_dialog(); assert isinstance(dialog, Adw.PreferencesDialog)
            rows = {w.get_title(): w for w in widgets(dialog) if isinstance(w, Adw.ComboRow)}
            assert rows['Windows version'].get_selected_item().get_string() == 'Windows 98'
            assert rows['Wine renderer'].get_selected_item().get_string() == 'GDI'
            assert rows['Wine display interface'].get_selected_item().get_string() == 'GLX'
            dialog.close()
            until = time.monotonic() + .4
            while time.monotonic() < until:
                yield
            # Saving unrelated settings must not overwrite a custom registry value.
            saved = window.find_app(saved['id']); saved['wineConfigurationCustom'] = {'wineRenderer': 'renderer="vulkan"'}
            window.library.update(saved)
            dialog = dialogs.app_settings(window, saved)
            renderer = next(w for w in widgets(dialog) if isinstance(w, Adw.ComboRow) and w.get_title() == 'Wine renderer')
            assert renderer.get_selected_item().get_string() == 'Custom (set in Wine)'
            save = next(w for w in widgets(dialog) if isinstance(w, Gtk.Button) and w.get_label() == 'Save')
            until = time.monotonic() + .4
            while time.monotonic() < until:
                yield
            save.emit('clicked')
            saved = window.find_app(saved['id'])
            assert saved['wineConfigurationCustom']['wineRenderer'] == 'renderer="vulkan"'
            assert not saved.get('wineRendererPending')
            assert not errors
            print('PASS: Wine tools picker/search/dispatch, readback after exit, failure retry, live settings, custom-value preservation')
            state['done'] = True
        sequence = steps(); started = time.monotonic()
        def tick():
            try:
                assert time.monotonic() - started < 30, 'UI test timed out'
                next(sequence); return GLib.SOURCE_CONTINUE
            except StopIteration:
                application.quit(); return GLib.SOURCE_REMOVE
            except Exception:
                traceback.print_exc(); state['failed'] = True; application.quit(); return GLib.SOURCE_REMOVE
        GLib.timeout_add(40, tick); application.run([sys.argv[0]])
        return int(state['failed'] or not state['done'])


if __name__ == '__main__':
    sys.exit(main())
