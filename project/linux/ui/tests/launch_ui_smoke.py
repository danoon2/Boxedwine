#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise progress closure, icon export, and desktop activation on a real display."""
import base64
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import traceback
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from boxedwine.app import Application, Window, GLib, Gtk, Gdk, gi
from boxedwine import desktop
from boxedwine.files import Work, Cancelled
from boxedwine.library import Library
from boxedwine.packages import import_wine


def main():
    with tempfile.TemporaryDirectory(prefix='boxedwine-launch-ui-') as temporary:
        root = Path(temporary)
        os.environ['XDG_DATA_HOME'] = str(root / 'desktop data')
        library = Library(root / 'library')
        wine_fixture(root / 'wine.zip'); wine = import_wine(library, root / 'wine.zip', Work())
        saved = library.add_builtin('notepad', wine, Work())
        library.close()
        application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
        state = {'failed': False, 'done': False}

        def steps():
            window = application.window
            assert isinstance(window, Window)
            theme = Gtk.IconTheme.get_for_display(Gdk.Display.get_default())
            assert theme.has_icon(desktop.ICON)
            assert window.get_icon_name() == desktop.ICON
            if Gdk.Display.get_default().get_name().startswith(':'):
                gi.require_version('GdkX11', '4.0')
                from gi.repository import GdkX11
                start = time.monotonic()
                while time.monotonic() - start < .2:
                    yield
                xid = GdkX11.X11Surface.get_xid(window.get_surface())
                prop = subprocess.check_output(['xprop', '-id', str(xid), '-len', '32', '_NET_WM_ICON'], text=True)
                assert 'CARDINAL' in prop, 'Launcher has no X11 window icon'
            environment = desktop.app_environment(window.library, saved, window.icon(saved, 64), window.resources)
            assert len(base64.b64decode(environment['BOXEDWINE_APP_ICON_BGRA'])) == 64 * 64 * 4
            for entry in (desktop.data_home() / 'applications').glob('*.desktop'):
                subprocess.run(['desktop-file-validate', str(entry)], check=True)
                data = GLib.KeyFile(); data.load_from_file(str(entry), GLib.KeyFileFlags.NONE)
                assert Path(data.get_string('Desktop Entry', 'Icon')).is_file()
            # A completion callback may immediately start another operation.
            # It must never see a previous modal, even with animations enabled.
            completions = []
            def complete(value):
                assert window.get_visible_dialog() is None
                assert not window.busy and window.work is None
                completions.append(value)
            for duration in (0, 0.05, 0.31, 0.36, 0.6):
                window.perform('Preparing regression', lambda work: (time.sleep(duration), duration)[1], complete)
                deadline = time.monotonic() + 4
                while window.busy:
                    assert time.monotonic() < deadline, f'Progress did not finish: duration={duration}, work={window.work}, visible={window.get_visible_dialog()}'
                    yield
                assert window.get_visible_dialog() is None
            assert len(completions) == 5
            window.perform('First', lambda work: time.sleep(.35),
                           lambda _: window.perform('Second', lambda work: time.sleep(.35), complete))
            while window.busy:
                yield
            assert len(completions) == 6
            def cancelled(work):
                while not work.cancel.wait(.02):
                    work.check()
                raise Cancelled()
            window.perform('Cancel preparation', cancelled, complete)
            start = time.monotonic()
            while time.monotonic() - start < .5:
                yield
            assert window.get_visible_dialog() is not None
            window.work.cancel.set()
            while window.busy:
                yield
            assert window.get_visible_dialog() is None
            errors = []
            window.error = lambda error: errors.append(str(error))
            def failing(work):
                time.sleep(.35)
                raise ValueError('Expected test failure')
            window.perform('Failed preparation', failing)
            while window.busy:
                yield
            assert errors == ['Expected test failure'] and window.get_visible_dialog() is None
            # The hidden app entry can target a running launcher over GApplication.
            opened = []
            window.open_app = lambda app: opened.append(app['id'])
            script = Path(__file__).resolve().parents[2] / 'boxedwine-ui'
            child = subprocess.Popen([sys.executable, str(script), '--library', str(root / 'library'), '--open', saved['id']])
            deadline = time.monotonic() + 5
            while child.poll() is None or not opened:
                assert time.monotonic() < deadline, 'Desktop activation was not forwarded'
                yield
            assert child.returncode == 0 and opened == [saved['id']]
            print('PASS: fast/animated/chained/cancelled/failed preparation, icons, and desktop activation')
            state['done'] = True

        sequence = steps()
        start = time.monotonic()
        def tick():
            try:
                assert time.monotonic() - start < 25, 'UI test timed out'
                next(sequence)
                return GLib.SOURCE_CONTINUE
            except StopIteration:
                application.quit(); return GLib.SOURCE_REMOVE
            except Exception:
                traceback.print_exc(); state['failed'] = True
                application.quit(); return GLib.SOURCE_REMOVE
        GLib.timeout_add(30, tick)
        application.run([sys.argv[0]])
        return int(state['failed'] or not state['done'])


if __name__ == '__main__':
    sys.exit(main())
