#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Native source-dialog failures, cancellation, and corrected-URL retries."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import os
from pathlib import Path
import sys
import tempfile
import threading
import time
import traceback
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from boxedwine.app import Application, Window, GLib, Gtk, Adw
from boxedwine import dialogs


def widgets(widget):
    yield widget
    child = widget.get_first_child()
    while child:
        yield from widgets(child)
        child = child.get_next_sibling()


def main():
    payload = (Path(__file__).resolve().parents[4] / 'resources/examples/collection.json').read_bytes()
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args): pass
        def do_GET(self):
            if self.path == '/slow.json':
                time.sleep(.7)
            body = payload if self.path == '/valid.json' else b'not a collection or ZIP'
            self.send_response(404 if self.path.startswith('/missing.') else 200)
            self.send_header('Content-Length', str(len(body))); self.end_headers()
            try: self.wfile.write(body)
            except (BrokenPipeError, ConnectionResetError): pass
    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    address = f'http://127.0.0.1:{server.server_port}'
    try:
        with tempfile.TemporaryDirectory(prefix='boxedwine-source-ui-') as temporary:
            root = Path(temporary); os.environ['XDG_DATA_HOME'] = str(root / 'desktop')
            application = Application(SimpleNamespace(library=root / 'library', resources=None, emulator=None))
            state = {'failed': False, 'done': False}
            def settle():
                until = time.monotonic() + .4
                while time.monotonic() < until:
                    yield
            def click(dialog, label):
                next(w for w in widgets(dialog) if isinstance(w, Gtk.Button) and w.get_label() == label).emit('clicked')
            def steps():
                window = application.window; assert isinstance(window, Window)
                alerts = []
                window.error = lambda error: alerts.append(str(error))
                for theme in (Adw.ColorScheme.FORCE_LIGHT, Adw.ColorScheme.FORCE_DARK):
                    Adw.StyleManager.get_default().set_color_scheme(theme)
                    for purpose in ('Collection', 'App or installer'):
                        start = dialogs.add_app(window); yield from settle()
                        choice = next(w for w in widgets(start) if isinstance(w, Adw.ActionRow) and w.get_title() == purpose)
                        choice.get_activatable_widget().emit('clicked'); yield from settle()
                        source = window.get_visible_dialog()
                        link = next(w for w in widgets(source) if isinstance(w, Adw.EntryRow))
                        error = next(w for w in widgets(source) if isinstance(w, Gtk.Label) and w.has_css_class('error'))
                        link.set_text('typo'); click(source, 'Download & Continue')
                        assert error.get_visible() and link.get_text() == 'typo' and not alerts
                        for path in ('/missing.json', '/invalid.json' if purpose == 'Collection' else '/invalid.zip'):
                            link.set_text(address + path); assert not error.get_visible()
                            click(source, 'Download & Continue')
                            while window.busy:
                                yield
                            assert window.get_visible_dialog() is source and not alerts
                            assert error.get_visible() and error.get_text() and link.get_text() == address + path
                            assert link.is_sensitive() and source.get_can_close()
                        link.set_text(address + '/slow.json'); click(source, 'Download & Continue')
                        while window.get_visible_dialog() is source:
                            yield
                        click(window.get_visible_dialog(), 'Cancel')
                        while window.busy:
                            yield
                        assert window.get_visible_dialog() is source and not alerts and not error.get_visible()
                        assert link.get_text() == address + '/slow.json' and link.is_sensitive()
                        link.set_text(address + '/valid.json'); click(source, 'Download & Continue')
                        while window.busy or window.get_visible_dialog() is None or window.get_visible_dialog() is source:
                            yield
                        review = window.get_visible_dialog()
                        assert review.get_title() == 'Review Collection' and not alerts
                        # Cancelling review must not add a collection or reveal a stale error.
                        yield from settle(); review.close(); yield from settle()
                        assert window.get_visible_dialog() is None and not alerts
                        assert not window.collections.load()['folders']
                print('PASS: inline URL/HTTP/content errors, cancellation, corrected retry, and no delayed alerts in both themes')
                state['done'] = True
            sequence = steps(); started = time.monotonic()
            def tick():
                try:
                    assert time.monotonic() - started < 40, 'Source import UI test timed out'
                    next(sequence); return GLib.SOURCE_CONTINUE
                except StopIteration:
                    application.quit(); return GLib.SOURCE_REMOVE
                except Exception:
                    traceback.print_exc(); state['failed'] = True; application.quit(); return GLib.SOURCE_REMOVE
            GLib.timeout_add(40, tick); application.run([sys.argv[0]])
            return int(state['failed'] or not state['done'])
    finally:
        server.shutdown(); server.server_close()


if __name__ == '__main__':
    sys.exit(main())
