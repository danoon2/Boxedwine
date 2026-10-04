#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Launch native Wine tools and verify registry readback in an isolated prefix."""
import argparse
from pathlib import Path
import sys
import tempfile
import time
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from boxedwine.files import Work, beneath
from boxedwine.library import Library
from boxedwine.packages import import_wine
from boxedwine.runtime import Session, WINE_TOOLS, build_arguments, refresh_configuration


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wine', type=Path, required=True)
    parser.add_argument('--engine', type=Path, default=Path(__file__).resolve().parents[2] / 'Build/Native/boxedwine-engine')
    parser.add_argument('--tools', nargs='*', choices=list(WINE_TOOLS), default=list(WINE_TOOLS))
    options = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='boxedwine-wine-tools-') as temporary:
        library = Library(temporary)
        try:
            reference = import_wine(library, options.wine, Work())
            app = library.add_builtin('notepad', reference, Work())
            package = library.wine_path(app)
            log = beneath(library.path(app), 'Logs/latest.log')
            for tool in options.tools:
                session = Session(options.engine, build_arguments(library, app, package, tool=tool), package, log)
                try:
                    deadline = time.monotonic() + 45
                    while not session.window.wait(.1):
                        if session.done.is_set() or time.monotonic() > deadline:
                            raise RuntimeError(tool + ' did not show a window: ' + log.read_text(errors='replace')[-3000:])
                    time.sleep(2)
                    if session.done.is_set():
                        raise RuntimeError(tool + ' closed unexpectedly: ' + log.read_text(errors='replace')[-3000:])
                    session.stop()
                    if session.code not in (0, -9):
                        raise RuntimeError(tool + ' did not stop cleanly: ' + log.read_text(errors='replace')[-3000:])
                    print(tool + ': window shown; ' + ('clean stop' if session.code == 0 else 'stopped with timeout fallback'), flush=True)
                finally:
                    if not session.done.is_set():
                        session.force_stop(); session.done.wait()
            # Run actual Wine commands, independent of our settings setter. This
            # exercises the same edits a user makes through the console/regedit.
            def commands(command):
                arguments = ['-root', str(library.root(app)), '-zip', str(package), '-hideWindow',
                             '-w', '/home/username', '/bin/sh', '-c', command + '\n/opt/wine/bin/wineserver -k']
                session = Session(options.engine, arguments, package, log)
                try:
                    if not session.done.wait(60) or session.code != 0:
                        raise RuntimeError('Wine commands failed: ' + log.read_text(errors='replace')[-3000:])
                finally:
                    if not session.done.is_set():
                        session.force_stop(); session.done.wait()
            commands("/bin/wine winecfg /v win98 && /bin/wine reg add 'HKCU\\Software\\Wine\\Direct3D' /v DirectDrawRenderer /t REG_SZ /d gdi /f && /bin/wine reg add 'HKCU\\Software\\Wine\\Direct3D' /v renderer /t REG_SZ /d gdi /f && /bin/wine reg add 'HKCU\\Software\\Wine\\X11 Driver' /v UseEGL /t REG_SZ /d N /f")
            app['wineConfigurationRefreshPending'] = True; library.update(app)
            app = refresh_configuration(options.engine, library, app, package, Work())
            assert (app['windowsVersion'], app['wineRenderer'], app['openGLBackend']) == ('win98', 'gdi', 'glx'), app
            assert not app.get('wineConfigurationRefreshPending')
            print('Readback verified: Windows 98, GDI, GLX', flush=True)
            commands("/bin/wine winecfg /v win10 && /bin/wine reg add 'HKCU\\Software\\Wine\\Direct3D' /v DirectDrawRenderer /t REG_SZ /d opengl /f && /bin/wine reg add 'HKCU\\Software\\Wine\\Direct3D' /v renderer /t REG_SZ /d gl /f && /bin/wine reg add 'HKCU\\Software\\Wine\\X11 Driver' /v UseEGL /t REG_SZ /d Y /f")
            app = refresh_configuration(options.engine, library, app, package, Work())
            assert (app['windowsVersion'], app['wineRenderer'], app['openGLBackend']) == ('win10', 'openGL', 'egl'), app
            print('Readback verified: Windows 10, OpenGL, EGL', flush=True)
            commands("/bin/wine reg delete 'HKCU\\Software\\Wine\\Direct3D' /f && /bin/wine reg delete 'HKCU\\Software\\Wine\\X11 Driver' /f")
            app = refresh_configuration(options.engine, library, app, package, Work())
            assert (app['wineRenderer'], app['openGLBackend']) == ('wineDefault', 'wineDefault'), app
            print('Readback verified: removed keys become Wine defaults', flush=True)
        finally:
            library.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
