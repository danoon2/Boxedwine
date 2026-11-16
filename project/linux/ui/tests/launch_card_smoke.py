#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise launch controls and stable card/icon positions with real GTK layout.

Run in an isolated X11 or Wayland session. No emulator or user library is used.
"""
from pathlib import Path
from threading import Event
from types import SimpleNamespace
import os
import sys
import tempfile
import traceback

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from ui_layout import settle_layout
from boxedwine.app import Application, Adw, Gdk, GLib, Gtk
from boxedwine.files import Work
from boxedwine.library import Library
from boxedwine.packages import import_wine


def children(widget):
    child = widget.get_first_child()
    while child:
        yield child
        child = child.get_next_sibling()


def bounds(widget, relative):
    valid, rect = widget.compute_bounds(relative)
    assert valid and rect.get_width() > 0 and rect.get_height() > 0, 'Widget must have a nonempty layout'
    return (rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height())


def main():
    with tempfile.TemporaryDirectory(prefix='boxedwine-launch-cards-') as temporary:
        root = Path(temporary)
        os.environ['XDG_DATA_HOME'] = str(root / 'desktop-data')
        library = Library(root / 'library')
        wine_fixture(root / 'wine.zip')
        wine = import_wine(library, root / 'wine.zip', Work())
        for index in range(30):
            app = library.add_builtin('notepad', wine, Work())
            app['name'] = f'App {index:02} with a long two-line name'
            library.update(app)
        library.close()
        application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
        failed = False

        def steps():
            window = application.window
            yield from settle_layout(window, 'Initial library')
            manager = Adw.StyleManager.get_default()

            def geometry():
                return {key: (bounds(card, window), bounds(card.get_child().get_first_child(), window))
                        for key, card in window.cards.items()}

            def unchanged(before, selected):
                after = geometry()
                assert before.keys() == after.keys()
                for key in before:
                    for old, new in zip(before[key], after[key]):
                        assert all(abs(a - b) < 1 for a, b in zip(old, new)), f'Card/icon moved: {key}: {old} -> {new}'
                assert window.selected == selected, 'Launch changed selection'

            def controls(identifier):
                tile = window.cards[identifier].get_parent()
                assert isinstance(tile, Gtk.Overlay)
                return ([child for child in children(tile) if isinstance(child, Gtk.ProgressBar)],
                        [child for child in children(tile) if isinstance(child, Gtk.Button) and child != window.cards[identifier]])

            for scheme in (Adw.ColorScheme.FORCE_LIGHT, Adw.ColorScheme.FORCE_DARK):
                manager.set_color_scheme(scheme)
                yield from settle_layout(window, 'Color scheme')
                for target in (window.document['apps'][0], window.document['apps'][-1]):
                    identifier = target['id']
                    window.reveal_app(target)
                    yield from settle_layout(window, 'Reveal launch target')
                    _, y, _, height = bounds(window.cards[identifier], window.scroll)
                    assert y >= -1 and y + height <= window.scroll.get_height() + 1, 'Target card must be visible'
                    before = geometry()
                    stopped = Event()
                    session = SimpleNamespace(window=Event(), installing=False, stop=stopped.set)
                    window.sessions[identifier] = session
                    window.refresh()
                    window.refresh()  # Rapid notifications must retain the scroll position.
                    yield from settle_layout(window, 'Launch')
                    unchanged(before, identifier)
                    progress, stop = controls(identifier)
                    assert len(progress) == len(stop) == 1 and stop[0].get_sensitive()
                    assert not progress[0].get_can_target(), 'Progress must not intercept clicks'
                    assert all(not controls(key)[0] for key in window.cards if key != identifier)
                    # Pointer hit testing must reach Stop, not the card's double-click handler.
                    tile = window.cards[identifier].get_parent()
                    x, y, width, height = bounds(stop[0], tile)
                    hit = tile.pick(x + width / 2, y + height / 2, Gtk.PickFlags.DEFAULT)
                    assert hit == stop[0] or hit.is_ancestor(stop[0]), 'Stop is covered by the card'

                    session.window.set()
                    window.refresh()
                    yield from settle_layout(window, 'Window ready')
                    unchanged(before, identifier)
                    assert not controls(identifier)[0] and len(controls(identifier)[1]) == 1
                    del window.sessions[identifier]
                    window.refresh()
                    yield from settle_layout(window, 'Exit')
                    unchanged(before, identifier)
                    assert controls(identifier) == ([], [])

                    session.window.clear()
                    window.sessions[identifier] = session
                    window.refresh()
                    yield from settle_layout(window, 'Second launch')
                    controls(identifier)[1][0].emit('clicked')
                    yield from settle_layout(window, 'Stop', stopped.is_set)
                    assert stopped.is_set(), 'Stop must reach the runtime session'
                    unchanged(before, identifier)
                    assert not controls(identifier)[0] and not controls(identifier)[1][0].get_sensitive()
                    window.sessions.clear()
                    window.stopping.clear()
                    window.refresh()
                    yield from settle_layout(window, 'Stopped')
                    unchanged(before, identifier)
                    print(f'PASS: {scheme.value_nick}, {target["name"]}: launch, window ready, exit, stop, selection and card/icon positions', flush=True)
            print('PASS: backend ' + type(Gdk.Display.get_default()).__name__, flush=True)

        iterator = steps()

        def advance():
            nonlocal failed
            try:
                GLib.timeout_add(next(iterator), advance)
            except StopIteration:
                application.quit()
            except Exception:
                traceback.print_exc()
                failed = True
                if hasattr(application.window, 'sessions'):
                    application.window.sessions.clear()
                application.quit()
            return GLib.SOURCE_REMOVE

        GLib.timeout_add(1000, advance)
        application.run([sys.argv[0]])
        return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
