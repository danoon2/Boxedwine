#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Real engine/Wine integration test. Requires a display and a release Wine ZIP."""
import argparse
from pathlib import Path
import sys
import tempfile
import time
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from boxedwine.files import Work, beneath
from boxedwine.library import Library
from boxedwine.packages import Resources, import_wine
from boxedwine.runtime import Session, apply_configuration, build_arguments


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wine', type=Path, required=True)
    parser.add_argument('--engine', type=Path, default=Path(__file__).resolve().parents[2] / 'Build/Native/boxedwine-engine')
    options = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='boxedwine-runtime-') as temporary:
        library = Library(temporary)
        try:
            wine = import_wine(library, options.wine, Work())
            if wine not in [release['reference'] for release in Resources().wines()]:
                raise ValueError('Use the pinned release Wine package for this integration test.')
            app = library.add_builtin('notepad', wine, Work())
            app.update(windowsVersion='win98', windowsVersionPending=True, wineRenderer='gdi', wineRendererPending=True)
            library.update(app)
            package = library.wine_path(app)
            app = apply_configuration(options.engine, library, app, package, Work(lambda message, *_: print(message, flush=True)))
            print('Verified Windows 98 and GDI configuration.', flush=True)
            app.update(windowsVersion='wineDefault', windowsVersionPending=True, wineRenderer='wineDefault', wineRendererPending=True,
                       openGLBackend='glx', openGLBackendPending=True)
            library.update(app)
            app = apply_configuration(options.engine, library, app, package, Work(lambda message, *_: print(message, flush=True)))
            print('Verified default Windows/renderer reset and Wine GLX.', flush=True)
            for program in ('notepad', 'minesweeper'):
                if program == 'minesweeper':
                    app = library.add_builtin(program, wine, Work())
                session = Session(options.engine, build_arguments(library, app, package), package,
                                  beneath(library.path(app), 'Logs/latest.log'))
                try:
                    if not session.window.wait(45):
                        raise RuntimeError(program + ' did not display a window. Exit: ' + str(session.code))
                    # The first SDL window can appear before Wine finishes its
                    # initial startup; let the normal event loop settle.
                    time.sleep(2)
                    session.stop()
                    if session.code not in (0, -9) or session.error or not session.stopped:
                        raise RuntimeError(program + ' did not stop cleanly: ' + str(session.error or session.code))
                    print(program + ': window shown; ' + ('clean stop.' if session.code == 0 else 'stopped by the process-group timeout fallback.'), flush=True)
                finally:
                    if not session.done.is_set():
                        session.force_stop(); session.done.wait()
        finally:
            library.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
