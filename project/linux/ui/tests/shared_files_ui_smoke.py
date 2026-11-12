#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise app-card drop controllers and shared-file actions in an isolated UI."""
import os
from pathlib import Path
import sys
import tempfile
import threading
import time
import traceback
from types import SimpleNamespace
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from boxedwine.app import Application, GLib, Gtk, Adw, Gdk, Gio
from boxedwine import dialogs, shared_files
from boxedwine.files import Work
from boxedwine.library import Library
from boxedwine.packages import import_wine


def main():
    with tempfile.TemporaryDirectory(prefix='boxedwine-shared-ui-') as temporary:
        root = Path(temporary)
        os.environ['XDG_DATA_HOME'] = str(root / 'desktop')
        library = Library(root / 'library')
        wine_fixture(root / 'wine.zip')
        wine = import_wine(library, root / 'wine.zip', Work())
        saved = library.add_builtin('notepad', wine, Work())
        library.close()
        source = root / 'patch.exe'; source.write_bytes(b'patch')
        application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
        state = {'done': False, 'failed': False}
        def steps():
            window = application.window
            calls, errors = [], []
            window.launch = lambda app, **kwargs: calls.append((app['id'], kwargs))
            window.error = lambda error: errors.append(str(error))
            for theme in (Adw.ColorScheme.FORCE_LIGHT, Adw.ColorScheme.FORCE_DARK):
                Adw.StyleManager.get_default().set_color_scheme(theme)
                for running in (False, True):
                    if running:
                        shown = threading.Event(); shown.set()
                        window.sessions[saved['id']] = SimpleNamespace(installing=False, window=shown)
                    card = window.cards[saved['id']]
                    model = card.observe_controllers()
                    drop = next(model.get_item(i) for i in range(model.get_n_items()) if isinstance(model.get_item(i), Gtk.DropTarget))
                    assert drop.emit('drop', Gio.File.new_for_path(str(source)), 12., 12.)
                    while window.busy or window.get_visible_dialog() is None:
                        yield
                    dialog = window.get_visible_dialog()
                    assert isinstance(dialog, Adw.AlertDialog), errors
                    assert dialog.get_response_enabled('run') == (not running)
                    assert dialog.get_response_enabled('folder')
                    shared = shared_files.directory(window.library, saved)
                    assert (shared / 'patch.exe').read_bytes() == b'patch'
                    assert len(window.document['apps']) == 1, 'Card drop must not import a new app'
                    if not running:
                        with patch.object(dialogs, 'choose_file', side_effect=lambda owner, title, callback, **kwargs: callback(shared / 'patch.exe')):
                            dialog.emit('response', 'run')
                        assert calls[-1] == (saved['id'], {'external': shared / 'patch.exe'})
                    else:
                        dialog.emit('response', 'done')
                    dialog.force_close()
                    window.sessions.pop(saved['id'], None)
                    until = time.monotonic() + .4
                    while time.monotonic() < until:
                        yield
            assert not errors, errors
            assert len(list(shared.iterdir())) == 4
            print('PASS Shared Files: app-card copies, duplicates, stopped Run dispatch, running access, both themes')
            state['done'] = True
        sequence = steps(); started = time.monotonic()
        def tick():
            try:
                assert time.monotonic() - started < 35, 'Shared Files UI test timed out'
                next(sequence)
                return GLib.SOURCE_CONTINUE
            except StopIteration:
                application.quit()
                return GLib.SOURCE_REMOVE
            except Exception:
                traceback.print_exc()
                state['failed'] = True
                application.window.sessions.clear()
                application.quit()
                return GLib.SOURCE_REMOVE
        GLib.timeout_add(40, tick)
        application.run([sys.argv[0]])
        return int(state['failed'] or not state['done'])


if __name__ == '__main__':
    sys.exit(main())
