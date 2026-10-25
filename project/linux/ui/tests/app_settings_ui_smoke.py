#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Settings parity checks using isolated metadata and the real GTK widgets."""
import argparse
import copy
from pathlib import Path
import sys
import tempfile
import time
import traceback
from types import SimpleNamespace
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from boxedwine.app import Application, Window, Adw, Gdk, Gio, GLib, Gtk
from boxedwine import dialogs
from boxedwine.files import Work
from boxedwine.library import Library
from boxedwine.packages import import_wine
from boxedwine.runtime import MOUSE_SENSITIVITY, build_arguments


def widgets(widget):
    yield widget
    child = widget.get_first_child()
    while child:
        yield from widgets(child)
        child = child.get_next_sibling()


def delay(seconds=.35):
    until = time.monotonic() + seconds
    while time.monotonic() < until:
        yield


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--preview', action='store_true', help='Pause on each tab for visual review')
    options = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='boxedwine-settings-ui-') as temporary:
        root = Path(temporary)
        library = Library(root / 'library'); wine_fixture(root / 'wine.zip')
        wine = import_wine(library, root / 'wine.zip', Work())
        source = root / 'Game'; source.mkdir(); (source / 'Game.exe').write_bytes(b'MZ')
        original = library.import_app(source, 'folder', wine, Work())
        original.update(name='A Portable Windows Game', boxedwineArguments=['-nosound', MOUSE_SENSITIVITY, '200'], arguments=['one value'])
        library.update(original); library.close()
        discs = []
        for i in range(12):
            path = root / f'Disc {i + 1}.iso'; path.write_bytes(b'fixture'); discs.append(str(path))
        application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
        state = {'failed': False, 'done': False}

        def steps():
            window = application.window; assert isinstance(window, Window)
            yield from delay(1)
            window.set_default_size(1180, 800); yield from delay()
            errors = []; window.error = lambda error: errors.append(str(error))
            for scheme in (Adw.ColorScheme.FORCE_LIGHT, Adw.ColorScheme.FORCE_DARK):
                Adw.StyleManager.get_default().set_color_scheme(scheme)
                window.library.update(original)
                dialog = dialogs.app_settings(window, copy.deepcopy(original)); yield from delay()
                assert dialog.mouse.get_value() == 200
                assert dialog.boxed_value() == '-nosound'
                assert dialog.arguments_value() == 'one value'
                assert dialog.windows() == 'wineDefault'
                assert not any(isinstance(w, Gtk.Label) and 'saved for this app' in w.get_text() for w in widgets(dialog.general))
                assert any(isinstance(w, Gtk.Label) and 'saved for this app' in w.get_text() for w in widgets(dialog.advanced))
                height = dialog.get_height()
                scroll = next(w for w in widgets(dialog.general) if isinstance(w, Gtk.ScrolledWindow))
                adjustment = scroll.get_vadjustment()
                initially_fits = adjustment.get_upper() <= adjustment.get_page_size() + 1
                if window.get_height() >= 760:
                    assert initially_fits, 'General should fit at the normal desktop size'
                print('General initial dimensions:', dialog.get_width(), height, 'owner:', window.get_width(), window.get_height(), 'scroll:', adjustment.get_upper(), adjustment.get_page_size(), flush=True)
                print('PREVIEW General', scheme.value_nick, flush=True)
                if options.preview:
                    yield from delay(12)
                dialog.discs.add_images(discs + [discs[0]])
                assert dialog.discs.images == discs
                yield from delay()
                assert dialog.get_height() == height, 'Adding images must not resize the dialog'
                assert adjustment.get_upper() > adjustment.get_page_size() + 1
                assert dialog.save_button.get_mapped(), 'Save remains outside the scroller'
                dialog.discs.move(0, 2)
                assert dialog.discs.images[:3] == [discs[1], discs[2], discs[0]]
                assert dialog.discs.drop(dialog.discs.token + '2', 0)
                assert dialog.discs.images == discs
                assert not dialog.discs.drop('another-dialog:0', 1)
                row = dialog.discs.rows.get_row_at_index(0)
                keys = next(c for c in row.observe_controllers() if isinstance(c, Gtk.EventControllerKey))
                assert keys.emit('key-pressed', Gdk.KEY_Down, 0, Gdk.ModifierType.ALT_MASK)
                assert dialog.discs.images[:2] == [discs[1], discs[0]]
                while dialog.discs.images:
                    dialog.discs.remove(0)
                yield from delay()
                assert dialog.get_height() == height
                print('General empty scroll:', adjustment.get_upper(), adjustment.get_page_size(), flush=True)
                if initially_fits:
                    assert adjustment.get_upper() <= adjustment.get_page_size() + 1, 'No scrollbar when everything fits'
                # Exercise the multi-file chooser callback without touching the
                # user's filesystem or clipboard through an interactive picker.
                with patch('boxedwine.app_settings.Gtk.FileDialog') as factory:
                    picker = factory.return_value
                    picker.open_multiple_finish.return_value = [Gio.File.new_for_path(path) for path in discs[:2]]
                    picker.open_multiple.side_effect = lambda owner, cancellable, callback: callback(picker, None)
                    dialog.discs.add_button.emit('clicked')
                assert dialog.discs.images == discs[:2]
                if options.preview:
                    print('PREVIEW General with discs', scheme.value_nick, flush=True)
                    yield from delay(12)
                dialog.set_visible_page_name('Advanced'); yield from delay()
                print('PREVIEW Advanced', scheme.value_nick, flush=True)
                if options.preview:
                    yield from delay(12)
                dialog.set_icon(window.resources.path('AppIcons/notepad.png'))
                assert dialog.icon.get_paintable() and dialog.automatic_icon.get_sensitive()
                assert dialog.icon_row.get_subtitle() == 'Custom icon'
                dialog.automatic_icon.emit('clicked')
                assert not dialog.saved.get('customIconPNG') and not dialog.automatic_icon.get_sensitive()
                dialog.set_icon(window.resources.path('AppIcons/notepad.png'))
                # Failed validation must not retain a subsequently reverted Wine
                # selection or write anything to the original app.
                windows = next(w for w in widgets(dialog.general) if isinstance(w, Adw.ComboRow) and w.get_title() == 'Windows version')
                windows.set_selected(1); dialog.name.set_text('')
                dialog.save_button.emit('clicked')
                assert errors and 'app name' in errors.pop()
                assert window.library.load()['apps'][0] == original
                windows.set_selected(0); dialog.name.set_text(original['name'])
                buffer = dialog.boxed.get_buffer()
                pasted = '-nosound\n\n' + MOUSE_SENSITIVITY + '\n250\n-env\nNAME=a b\n'
                buffer.set_text(pasted); buffer.place_cursor(buffer.get_end_iter())
                buffer.emit('paste-done', Gdk.Display.get_default().get_clipboard())
                assert dialog.mouse.get_value() == 250 and dialog.transfer_notice.get_visible()
                assert dialog.boxed_value() == '-nosound\n\n-env\nNAME=a b\n'
                assert buffer.get_iter_at_mark(buffer.get_insert()).get_offset() == len(dialog.boxed_value())
                buffer.set_text(MOUSE_SENSITIVITY + '\n1')
                buffer.insert(buffer.get_end_iter(), '23')
                assert dialog.mouse.get_value() == 250, 'Do not consume partially typed numbers'
                focus = next(c for c in dialog.boxed.observe_controllers() if isinstance(c, Gtk.EventControllerFocus))
                focus.emit('leave')
                assert dialog.mouse.get_value() == 123 and dialog.boxed_value() == ''
                buffer.set_text(MOUSE_SENSITIVITY + '\n1001')
                buffer.emit('paste-done', Gdk.Display.get_default().get_clipboard())
                assert dialog.mouse.get_value() == 123 and '1001' in dialog.boxed_value()
                dialog.save_button.emit('clicked')
                assert errors and '1000' in errors.pop()
                assert window.library.load()['apps'][0] == original
                for value, expected in ((0, 100), (1, 1), (1000, 1000)):
                    buffer.set_text(MOUSE_SENSITIVITY + '\n' + str(value)); focus.emit('leave')
                    assert dialog.mouse.get_value() == expected
                buffer.set_text('-nosound\n' + MOUSE_SENSITIVITY + '\n175')
                # Save also consumes complete input even if focus has not left it.
                dialog.save_button.emit('clicked'); yield from delay()
                saved = window.library.load()['apps'][0]
                assert saved['cdromImages'] == discs[:2]
                assert saved['boxedwineArguments'] == ['-nosound', MOUSE_SENSITIVITY, '175']
                assert saved['arguments'] == ['one value'] and not saved.get('windowsVersionPending')
                assert saved['customIconPNG']
                argv = build_arguments(window.library, saved, window.library.wine_path(saved))
                assert argv.count(MOUSE_SENSITIVITY) == 1 and argv.count('-cdrom') == 2
                dialog = dialogs.app_settings(window, saved); yield from delay()
                assert dialog.mouse.get_value() == 175 and dialog.boxed_value() == '-nosound'
                assert dialog.discs.images == discs[:2]
                dialog.discs.remove(0); dialog.mouse.set_value(300); dialog.name.set_text('Cancel me'); dialog.automatic_icon.emit('clicked')
                cancel = next(w for w in widgets(dialog) if isinstance(w, Gtk.Button) and w.get_label() == 'Cancel')
                cancel.emit('clicked'); yield from delay()
                assert window.library.load()['apps'][0] == saved
                dialog = dialogs.app_settings(window, saved); yield from delay()
                dialog.mouse_reset.emit('clicked'); dialog.save_button.emit('clicked'); yield from delay()
                assert window.library.load()['apps'][0]['boxedwineArguments'] == ['-nosound']
                assert not errors
                print('PASS settings:', scheme.value_nick, flush=True)
            state['done'] = True

        sequence = steps(); started = time.monotonic()
        def tick():
            try:
                assert time.monotonic() - started < (150 if options.preview else 60), 'Settings test timed out'
                next(sequence); return GLib.SOURCE_CONTINUE
            except StopIteration:
                application.quit(); return GLib.SOURCE_REMOVE
            except Exception:
                traceback.print_exc(); state['failed'] = True; application.quit(); return GLib.SOURCE_REMOVE
        GLib.timeout_add(40, tick); application.run([sys.argv[0]])
        return int(state['failed'] or not state['done'])


if __name__ == '__main__':
    sys.exit(main())
