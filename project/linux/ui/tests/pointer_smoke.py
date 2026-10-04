#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""X11 regression check using real pointer events on an isolated native window.

Requires libXtst. Launch/install actions are recorded rather than executed.
"""
import ctypes as C
from pathlib import Path
import sys
import tempfile
import traceback
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from boxedwine.app import Application, Window, GLib, Gdk, gi
from boxedwine.files import Work
from boxedwine.library import Library, now
from boxedwine.packages import import_wine


class Pointer:
    def __init__(self, window):
        gi.require_version('GdkX11', '4.0')
        from gi.repository import GdkX11
        self.window = window
        self.xid = GdkX11.X11Surface.get_xid(window.get_surface())
        self.x = C.CDLL('libX11.so.6'); self.test = C.CDLL('libXtst.so.6')
        self.x.XOpenDisplay.argtypes = [C.c_char_p]; self.x.XOpenDisplay.restype = C.c_void_p
        self.display = self.x.XOpenDisplay(None)
        if not self.display:
            raise RuntimeError('An X11 display is required')
        self.x.XDefaultRootWindow.argtypes = [C.c_void_p]; self.x.XDefaultRootWindow.restype = C.c_ulong
        self.root = self.x.XDefaultRootWindow(self.display)
        self.x.XTranslateCoordinates.argtypes = [C.c_void_p, C.c_ulong, C.c_ulong, C.c_int, C.c_int, C.POINTER(C.c_int), C.POINTER(C.c_int), C.POINTER(C.c_ulong)]
        self.x.XRaiseWindow.argtypes = [C.c_void_p, C.c_ulong]
        self.x.XFlush.argtypes = [C.c_void_p]
        self.x.XCloseDisplay.argtypes = [C.c_void_p]
        self.test.XTestFakeMotionEvent.argtypes = [C.c_void_p, C.c_int, C.c_int, C.c_int, C.c_ulong]
        self.test.XTestFakeButtonEvent.argtypes = [C.c_void_p, C.c_uint, C.c_int, C.c_ulong]
        self.x.XRaiseWindow(self.display, self.xid)

    def point_at(self, card):
        success, bounds = card.compute_bounds(self.window)
        assert success
        x, y, child = C.c_int(), C.c_int(), C.c_ulong()
        self.x.XTranslateCoordinates(self.display, self.xid, self.root, 0, 0, C.byref(x), C.byref(y), C.byref(child))
        self.test.XTestFakeMotionEvent(self.display, -1, x.value + int(bounds.get_x() + bounds.get_width() / 2),
                                       y.value + int(bounds.get_y() + bounds.get_height() / 2), 0)
        self.x.XFlush(self.display)

    def click(self):
        self.test.XTestFakeButtonEvent(self.display, 1, 1, 0)
        self.test.XTestFakeButtonEvent(self.display, 1, 0, 0)
        self.x.XFlush(self.display)


def main():
    temporary = tempfile.TemporaryDirectory(prefix='boxedwine-pointer-')
    root = Path(temporary.name)
    library = Library(root / 'library')
    wine_fixture(root / 'wine.zip'); wine = import_wine(library, root / 'wine.zip', Work())
    apps = []
    for i in range(8):
        app = library.add_builtin('notepad', wine, Work())
        app.update(name=f'App {i}', lastOpened=now()); library.update(app); apps.append(app)
    removed = library.add_builtin('notepad', wine, Work()); library.remove(removed)
    library.close()
    application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
    state = {'failed': False, 'pointer': None}
    calls = []

    def steps():
        window = application.window
        assert isinstance(window, Window)
        pointer = state['pointer'] = Pointer(window)
        window.open_app = lambda app: calls.append(('app', app['id']))
        window.install_demo = lambda recipe: calls.append(('demo', recipe['origin']['id']))
        assert window.selected == apps[0]['id'], 'Select the first app on entry'
        assert window.details_split.get_show_sidebar(), 'Reserve details space before selecting another app'
        # Choose a card near the end of the first row: revealing the details
        # pane used to move it out from underneath the second click.
        target = apps[4]['id']
        card = window.cards[target]
        pointer.point_at(card); pointer.click(); yield 100
        assert window.selected == target and not calls, 'Single click should only select'
        assert window.cards[target] is card, 'Selection must preserve the gesture widget'
        assert window.details_split.get_show_sidebar(), 'Keep details visible between clicks'
        pointer.click(); yield 150
        assert calls == [('app', target)], calls
        yield 500
        assert calls == [('app', target)], 'Double click should activate only once'
        calls.clear()
        pointer.point_at(window.cards[apps[0]['id']]); pointer.click(); yield 600
        assert not calls and window.details_split.get_show_sidebar(), 'Single click should reveal details'
        # A double click on an already selected card must also work.
        pointer.point_at(window.cards[apps[0]['id']]); pointer.click(); yield 100
        pointer.click(); yield 150
        assert calls == [('app', apps[0]['id'])], calls
        calls.clear(); window.navigate('Recently Opened'); yield 500
        assert window.selected == apps[-1]['id'], 'Select the first app in recent order'
        pointer.point_at(window.cards[target]); pointer.click(); yield 100
        pointer.click(); yield 150
        assert calls == [('app', target)], calls
        calls.clear()
        recipe = dict(origin=dict(id='pointer-demo'), name='Pointer Demo', summary='Test recipe', icon='', bytes=1024,
                      wineVersion='11.0', help='')
        window.catalog = [recipe]; window.catalog_loaded = True
        window.navigate('Demos'); yield 500
        assert window.selected == 'pointer-demo' and window.details_split.get_show_sidebar()
        assert not calls, 'Automatic selection must not install a demo'
        pointer.point_at(window.cards['pointer-demo']); pointer.click(); yield 100
        assert not calls, 'Single click must not install a demo'
        pointer.click(); yield 150
        assert calls == [('demo', 'pointer-demo')], calls
        calls.clear(); window.navigate('Removed Apps'); yield 500
        pointer.point_at(window.cards[removed['id']]); pointer.click(); yield 100
        pointer.click(); yield 150
        assert not calls, 'Removed apps must not launch or restore on double click'
        window.search.set_text('no matching apps'); window.filter('no matching apps')
        window.navigate('All Apps')
        assert window.selected == apps[0]['id'] and len(window.cards) == len(apps), 'Navigation must clear the previous search immediately'
        window.catalog = []; window.navigate('Demos')
        assert window.selected is None and not window.details_split.get_show_sidebar()
        window.catalog = [recipe]; window.refresh()
        assert window.selected == 'pointer-demo', 'Select the first demo when the catalog finishes loading'
        window.set_default_size(680, 780); yield 500
        window.navigate('All Apps'); yield 500
        assert window.selected == apps[0]['id'] and window.details_split.get_collapsed()
        assert not window.details_split.get_show_sidebar(), 'Automatic selection must not cover a narrow grid'
        assert not calls, 'Automatic selection must not launch or install'
        print('PASS: navigation selection, delayed catalog, narrow layout, double-click app/demo activation, and removed-app guard.', flush=True)

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
