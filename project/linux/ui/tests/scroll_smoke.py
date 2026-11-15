#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Check scroll retention with real GTK layout in an isolated library."""
from pathlib import Path
from types import SimpleNamespace
import os
import sys
import tempfile
import traceback

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from test_core import wine_fixture
from ui_layout import settle_layout
from boxedwine.app import Application, GLib
from boxedwine.files import Work
from boxedwine.library import Library
from boxedwine.packages import import_wine


def main():
    with tempfile.TemporaryDirectory(prefix='boxedwine-scroll-') as temporary:
        root = Path(temporary)
        os.environ['XDG_DATA_HOME'] = str(root / 'desktop-data')
        library = Library(root / 'library')
        wine_fixture(root / 'wine.zip')
        wine = import_wine(library, root / 'wine.zip', Work())
        for index in range(45):
            app = library.add_builtin('notepad', wine, Work())
            app['name'] = f'App {index:02}'
            library.update(app)
        library.close()
        application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
        failed = False

        def steps():
            window = application.window
            yield from settle_layout(window, 'Initial library')
            window.cards[window.document['apps'][-1]['id']].grab_focus()
            yield from settle_layout(window, 'Focused card')
            adjustment = window.scroll.get_vadjustment()
            bottom = adjustment.get_upper() - adjustment.get_page_size()
            assert bottom > 0, 'Fixture must scroll'
            adjustment.set_value(bottom)
            position = adjustment.get_value()
            window.refresh()
            window.refresh()  # A window-ready notification can precede layout.
            yield from settle_layout(window, 'Rapid refresh')
            assert abs(adjustment.get_value() - position) < 1, 'Status refresh jumped to the top'
            window.sessions[window.document['apps'][-1]['id']] = SimpleNamespace(
                installing=False, window=SimpleNamespace(is_set=lambda: True))
            window.refresh()
            yield from settle_layout(window, 'Running status')
            assert abs(adjustment.get_value() - position) < 1, f'Running status reset scroll: {position} -> {adjustment.get_value()}, upper={adjustment.get_upper()}, page={adjustment.get_page_size()}'
            window.sessions.clear()
            window.refresh()
            yield from settle_layout(window, 'Exit')
            assert abs(adjustment.get_value() - position) < 1, 'Exit refresh reset scroll'
            window.search.grab_focus()
            window.search.set_text('App 00')
            yield from settle_layout(window, 'Search', lambda: window.query == 'app 00')
            assert adjustment.get_value() == 0, 'Search must reset scroll'
            assert len(window.cards) == 1, 'Search must show only the matching app'
            window.search.set_text('')
            yield from settle_layout(window, 'Clear search', lambda: window.query == '')
            adjustment.set_value(adjustment.get_upper() - adjustment.get_page_size())
            window.refresh()
            window.navigate('Recently Opened')  # Cancel a pending restoration.
            yield from settle_layout(window, 'Navigation')
            assert adjustment.get_value() == 0, 'Navigation restored stale scroll'
            target = window.document['apps'][-1]
            window.reveal_app(target)
            window.refresh()  # Launch preflight must retain the reveal request.
            yield from settle_layout(window, 'Reveal last card')
            valid, bounds = window.cards[target['id']].compute_bounds(window.scroll)
            assert valid and bounds.get_y() >= -1 and bounds.get_y() + bounds.get_height() <= window.scroll.get_height() + 1, f'Revealed app must be visible: y={bounds.get_y()}, height={bounds.get_height()}, viewport={window.scroll.get_height()}, pos={adjustment.get_value()}, upper={adjustment.get_upper()}'
            position = adjustment.get_value()
            window.refresh()
            yield from settle_layout(window, 'Refresh revealed card')
            assert abs(adjustment.get_value() - position) < 1, 'Refresh lost the revealed position'
            window.reveal_app(window.document['apps'][0])
            yield from settle_layout(window, 'Reveal first card')
            valid, bounds = window.cards[window.selected].compute_bounds(window.scroll)
            assert valid and bounds.get_y() >= -1, 'Reveal must also scroll upward'
            window.busy = True  # Keep this isolated fixture from loading a catalog.
            window.navigate('Demos')
            window.search.set_text('Winroids')
            yield from settle_layout(window, 'Search demos', lambda: window.query == 'winroids')
            source = root / 'Winroids'
            source.mkdir()
            (source / 'winroids.exe').write_bytes(b'MZ portable game')
            added = window.library.import_app(source, 'folder', wine, Work())
            window.busy = False
            window.refresh()  # perform() refreshes before its completion callback.
            window.app_added(added)
            yield from settle_layout(window, 'Reveal imported app')
            assert window.view == 'All Apps' and window.selected == added['id']
            card = window.cards[added['id']]
            valid, bounds = card.compute_bounds(window.scroll)
            assert valid and bounds.get_y() >= -1 and bounds.get_y() + bounds.get_height() <= window.scroll.get_height() + 1, f'New portable app must be visible: y={bounds.get_y()}, height={bounds.get_height()}, viewport={window.scroll.get_height()}, pos={adjustment.get_value()}'
            print('PASS: refresh, rapid refresh, running status, exit, search, navigation, selected-card reveal, and newly added portable app.', flush=True)

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
                application.quit()
            return GLib.SOURCE_REMOVE
        GLib.timeout_add(1000, advance)
        application.run([sys.argv[0]])
        return int(failed)


if __name__ == '__main__':
    raise SystemExit(main())
