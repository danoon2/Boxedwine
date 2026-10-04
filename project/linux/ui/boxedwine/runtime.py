# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Child-process runtime, launch arguments, and verified Wine configuration."""
import copy
import os
from pathlib import Path
import re
import shlex
import shutil
import signal
import subprocess
import threading
import time
import uuid
from .files import beneath, delete_tree, no_links
from .library import DRIVE_C, WINDOWS, built_in, check_installer, preference

FLAGS = {'-nosound', '-p2', '-p3', '-dpiAware', '-disableHideCursor', '-forceRelativeMouse', '-cacheReads', '-disableLinearMemory'}
RANGES = {'-vsync': (0, 2), '-scale': (1, 1000), '-cpuAffinity': (1, 64), '-pollRate': (0, 10000), '-skipFrameFPS': (0, 1000), '-rel_mouse_sensitivity': (0, 1000)}
CHOICES = {'-bpp': ('8', '16', '32'), '-scale_quality': ('0', '1', '2', 'nearest', 'linear', 'best'), '-dxvk': ('0', '1', 'false', 'true', 'no', 'yes'), '-opengl': ('osmesa',)}
OPTIONS_HELP = ('Put each option and each value on a separate line. Spaces inside a value are preserved; do not add shell quotes.\n\n' +
                ', '.join(sorted(FLAGS)) + '\n\n' + '\n'.join(f'{k}: {a}–{b}' for k, (a, b) in RANGES.items()) + '\n\n' +
                '\n'.join(k + ': ' + ', '.join(v) for k, v in CHOICES.items() if k != '-opengl') +
                '\n-env: NAME=value\n-glext: allowed OpenGL extensions\n\nPaths, Wine packages, window size, and mounts are managed by Boxedwine.')


def lines(text):
    return [line for line in text.replace('\r\n', '\n').split('\n') if line.strip()]


def validate_overrides(arguments):
    if (not isinstance(arguments, list) or len(arguments) > 256 or
            any(not isinstance(a, str) or not a or len(a) > 8192 or any(c in a for c in '\0\r\n') for a in arguments) or
            sum(map(len, arguments)) > 65536):
        raise ValueError('Use at most 256 Boxedwine arguments, without empty values or control characters.')
    iterator = iter(arguments)
    for option in iterator:
        if option in FLAGS:
            continue
        if option not in RANGES and option not in CHOICES and option not in ('-env', '-glext'):
            raise ValueError(option + ' is unsupported or managed by the launcher. See Supported Options.')
        value = next(iterator, None)
        if not value or value.startswith('-'):
            raise ValueError(option + ' needs a value on the next line.')
        if option in RANGES:
            low, high = RANGES[option]
            if not re.fullmatch(r'0|[1-9][0-9]*', value) or not low <= int(value) <= high:
                raise ValueError(f'{option} needs a whole number from {low} to {high}.')
        if option in CHOICES and value not in CHOICES[option]:
            raise ValueError(option + ' accepts ' + ', '.join(CHOICES[option]))
        if option == '-env' and not re.match('[A-Za-z_][A-Za-z0-9_]*=', value):
            raise ValueError('Use NAME=value after -env.')


def overrides(arguments):
    validate_overrides(arguments)
    result = []
    iterator = iter(arguments)
    for option in iterator:
        value = None if option in FLAGS else next(iterator)
        if option != '-opengl':  # Legacy Mac metadata; Linux uses the native renderer.
            result.append(option)
            if value is not None:
                result.append(value)
    return result


def demo_arguments(app, cwd):
    settings = app.get('demoSettings') or {}
    result = []
    for field, flag in [('bitsPerPixel', '-bpp'), ('cpuCount', '-cpuAffinity')]:
        if settings.get(field) is not None:
            result.extend([flag, str(settings[field])])
    if settings.get('cncDDraw'):
        result.extend(['-ddrawOverride', cwd])
    for field, flag in [('disableHideCursor', '-disableHideCursor'), ('forceRelativeMouse', '-forceRelativeMouse')]:
        if settings.get(field):
            result.append(flag)
    return result


WINE_TOOLS = {
    'winecfg': ('Wine Configuration', 'Windows version, DLL overrides, graphics, audio, and drives', ('winecfg',)),
    'regedit': ('Registry Editor', 'Edit this app’s Windows registry', ('regedit',)),
    'cmd': ('Command Prompt', 'Run Windows commands and batch files', ('wineconsole', 'cmd')),
    'winefile': ('File Manager', 'Browse this app’s Windows drives and files', ('winefile',)),
    'uninstaller': ('Add/Remove Programs', 'Manage installed programs and components', ('uninstaller',)),
    'iexplore': ('Wine Internet Explorer', 'Open Wine’s browser in this environment', ('iexplore', 'about:blank')),
    'notepad': ('Notepad', 'Edit text files in this app’s environment', ('notepad',)),
}


def build_arguments(library, app, wine, installing=False, alternate=None, external=None, tool=None):
    library.validate_app(app)
    if any(app.get(f + 'Pending') for f in ('windowsVersion', 'wineRenderer', 'openGLBackend')):
        raise ValueError('Wine settings must be prepared before launching this app.')
    if sum((installing, alternate is not None, external is not None, tool is not None)) > 1:
        raise ValueError('Choose one launch mode.')
    if tool is not None and tool not in WINE_TOOLS:
        raise ValueError('Unknown Wine tool.')
    size = (app.get('demoSettings') or {}).get('installResolution', '1024x768') if installing else app.get('resolution', '1024x768')
    title = WINE_TOOLS[tool][0] + ' · ' + app['name'] if tool else app['name']
    result = ['-root', str(library.root(app)), '-zip', str(wine), '-title', title, '-resolution', size]
    if app.get('fullScreen'):
        result.append('-fullscreenAspect')
    extra = overrides(app.get('boxedwineArguments') or [])
    if tool is not None:
        return result + extra + ['-w', '/home/username', '/bin/wine', *WINE_TOOLS[tool][2]]
    builtin = built_in(app)
    if builtin and not installing and alternate is None and external is None:
        return result + extra + ['/bin/wine', 'notepad' if builtin == 'notepad' else 'winemine'] + app.get('arguments', [])
    if external is not None:
        source = check_installer(external)
        cwd = '/home/username/boxedwine-program'
        guest = cwd + '/' + source.name
        result += demo_arguments(app, cwd) + extra + ['-mount', str(source.parent), cwd]
    elif installing:
        if not app.get('installer'):
            raise ValueError('This app has no saved installer.')
        source = check_installer(beneath(library.path(app), app['installer']))
        staged = app['installer'].startswith('Installer/')
        guest = '/mnt/installer/' + (app['installer'][10:] if staged else source.name)
        cwd = guest.rsplit('/', 1)[0]
        mount = beneath(library.path(app), 'Installer') if staged else source.parent
        result += extra + ['-mount', str(mount), '/mnt/installer']
    else:
        executable = alternate or app.get('executable')
        if (not executable or not executable.startswith(DRIVE_C + '/') or not executable.lower().endswith('.exe') or
                not beneath(library.root(app), executable).is_file()):
            raise ValueError('The selected Windows program is missing. Choose a program in App Settings.')
        guest = '/' + executable
        cwd = guest.rsplit('/', 1)[0]
        result += demo_arguments(app, cwd) + extra
    result += ['-w', cwd, '/bin/wine']
    if guest.lower().endswith('.msi'):
        result += ['start', '/wait', '/unix']
    result.append(guest)
    if not installing and alternate is None and external is None:
        result += app.get('arguments', [])
    return result


class WindowMarker:
    def __init__(self):
        self.pending = b''
        self.ignoring = False
        self.reported = False

    def consume(self, data):
        if self.reported:
            return False
        for value in data:
            if value == 10:
                if not self.ignoring and self.pending in (b'Showing Window', b'Showing Window\r'):
                    self.reported = True
                    return True
                self.pending, self.ignoring = b'', False
            elif not self.ignoring:
                self.pending += bytes([value])
                if len(self.pending) > 15:
                    self.pending, self.ignoring = b'', True
        return False


def default_emulator(directory=None):
    """Resolve relative to the launcher, independent of the working directory."""
    directory = Path(directory) if directory is not None else Path(__file__).resolve().parents[2]
    for relative in ('Runtime/boxedwine-engine', 'boxedwine-engine'):
        candidate = directory / relative
        if candidate.is_file():
            return candidate
    # Direct source-tree launches use the native-runtime build output.
    return directory / 'Build/Native/boxedwine-engine'


class Session:
    def __init__(self, emulator, arguments, wine, log, installing=False, rotate=True, on_window=None, on_exit=None, environment=None):
        emulator = Path(emulator).absolute()
        if not emulator.is_file() or not os.access(emulator, os.X_OK):
            raise ValueError('Choose an executable Boxedwine engine in Settings, or run make native-ui.')
        self.installing, self.stopped, self.code = installing, False, None
        self.error = None
        self.done, self.window = threading.Event(), threading.Event()
        self.log_path = no_links(log)
        self.log_path.parent.mkdir(parents=True, exist_ok=True)
        self.gate = threading.Lock()
        self.truncated = False
        self.on_window, self.on_exit = on_window, on_exit
        self.lease = no_links(wine).open('rb')
        self.log = None
        try:
            if rotate and self.log_path.exists():
                os.replace(self.log_path, no_links(self.log_path.parent / 'previous.log'))
            self.log = self.log_path.open('wb' if rotate else 'ab')
            self.count = self.log.tell()
            self.write(('Boxedwine launch · ' + time.strftime('%Y-%m-%d %H:%M:%S') + '\n' + shlex.join(arguments) + '\n').encode())
            self.process = subprocess.Popen([str(emulator), *arguments], cwd=emulator.parent, stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, start_new_session=True,
                                            env={**os.environ, **(environment or {})})
        except Exception:
            self.lease.close()
            if self.log:
                self.log.close()
            raise
        threading.Thread(target=self.observe, name='boxedwine-runtime', daemon=True).start()

    def write(self, data):
        with self.gate:
            if self.count >= 4 * 1024**2:
                if not self.truncated:
                    self.log.write(b'\n[Log truncated at 4 MB; output continues to be drained.]\n')
                    self.log.flush()
                    self.truncated = True
                return
            data = data[:4 * 1024**2 - self.count]
            self.log.write(data)
            self.log.flush()
            self.count += len(data)

    def drain(self, stream):
        marker = WindowMarker()
        try:
            while data := stream.read1(4096):
                self.write(data)
                if marker.consume(data) and not self.stopped and not self.window.is_set():
                    self.window.set()
                    if self.on_window:
                        self.on_window(self)
        except Exception as error:
            self.error = str(error)
            self.force_stop()
        finally:
            stream.close()

    def observe(self):
        readers = [threading.Thread(target=self.drain, args=(s,), daemon=True) for s in (self.process.stdout, self.process.stderr)]
        for reader in readers:
            reader.start()
        try:
            self.code = self.process.wait()
            # A descendant must not retain our pipes after the emulator exits.
            try:
                os.killpg(self.process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            for reader in readers:
                reader.join()
            self.write(f'\nProcess exited with code {self.code}{" (stopped)" if self.stopped else ""}.\n'.encode())
            if self.code and not self.stopped:
                shutil.copyfile(self.log_path, no_links(self.log_path.parent / 'last-failed.log'))
        except Exception as error:
            self.error = str(error)
        finally:
            self.process.stdin.close()
            self.log.close()
            self.lease.close()
            self.done.set()
            if self.on_exit:
                self.on_exit(self)

    def force_stop(self):
        self.stopped = True
        try:
            os.killpg(self.process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass

    def stop(self):
        if self.done.is_set():
            return
        self.stopped = True
        try:
            self.process.stdin.write(b'quit\n')
            self.process.stdin.flush()
        except (BrokenPipeError, OSError, ValueError):
            pass
        if not self.done.wait(5):
            self.force_stop()
        self.done.wait()


VERSIONS = set(WINDOWS) - {'wineDefault'} | {'win2008r2', 'win2008', 'win2003', 'winxp64', 'nt351', 'win30', 'win20'}


def read_configuration(text, kind):
    lines = [line.strip() for line in text.splitlines()]
    if kind == 'version':
        versions = [line for line in lines if line in VERSIONS]
        if len(versions) != 1:
            raise ValueError('Wine did not confirm a Windows version. See the launch log.')
        return versions[0]
    key = 'HKEY_CURRENT_USER\\Software\\Wine\\' + ('X11 Driver' if kind == 'backend' else 'Direct3D')
    if sum(line.casefold() == key.casefold() for line in lines) != 1:
        raise ValueError('Wine did not confirm its graphics settings. See the launch log.')

    def value(name):
        matches = [line.split() for line in lines if line.split() and line.split()[0].casefold() == name.casefold()]
        if not matches:
            return None
        if len(matches) != 1 or len(matches[0]) != 3 or matches[0][1] != 'REG_SZ':
            raise ValueError('Unexpected Wine registry output.')
        return matches[0][2]
    if kind == 'backend':
        result = {None: 'wineDefault', 'Y': 'egl', 'N': 'glx'}.get(value('UseEGL'))
    else:
        result = {(None, None): 'wineDefault', ('gdi', 'gdi'): 'gdi', ('opengl', 'gl'): 'openGL'}.get((value('DirectDrawRenderer'), value('renderer')))
    if result is None:
        raise ValueError('Wine reported unexpected graphics settings.')
    return result


def configuration_script(kind, value, token):
    uuid.UUID(token)
    if kind == 'version':
        if value is not None and value not in VERSIONS:
            raise ValueError('Unknown Windows version.')
        body = ('/bin/wine winecfg /v ' + value + ' &&\n' if value else '') + '/bin/wine winecfg /v > /tmp/boxedwine-configuration/result.txt 2>&1'
    else:
        key = "'HKCU\\Software\\Wine\\" + ("X11 Driver'" if kind == 'backend' else "Direct3D'")
        if kind == 'backend':
            if value not in ('wineDefault', 'egl', 'glx'):
                raise ValueError('Unknown Wine display backend.')
            values = [('UseEGL', 'Y' if value == 'egl' else 'N')]
        elif kind == 'renderer' and value in ('wineDefault', 'gdi', 'openGL'):
            values = [('DirectDrawRenderer', 'gdi' if value == 'gdi' else 'opengl'), ('renderer', 'gdi' if value == 'gdi' else 'gl')]
        else:
            raise ValueError('Unknown Wine renderer.')
        if value == 'wineDefault':
            body = f'/bin/wine reg add {key} /f && {{ ' + '; '.join(f'/bin/wine reg delete {key} /v {name} /f' for name, _ in values) + '; true; }'
        else:
            body = ' && '.join(f'/bin/wine reg add {key} /v {name} /t REG_SZ /d {setting} /f' for name, setting in values)
        body += f' && /bin/wine reg query {key} > /tmp/boxedwine-configuration/result.txt 2>&1'
    return "if " + body + "\nthen\n/bin/cat /tmp/boxedwine-configuration/result.txt\nprintf '%s' '" + token + "' > /tmp/boxedwine-configuration/completed\nfi\n/opt/wine/bin/wineserver -k\n"


def run_configuration(emulator, root, wine, log, kind, value, jobs, work):
    token = str(uuid.uuid4())
    directory = beneath(jobs, token)
    directory.mkdir(parents=True)
    try:
        arguments = ['-root', str(root), '-zip', str(wine), '-hideWindow', '-disableLinearMemory', '-title', 'Preparing Windows',
                     '-mount', str(directory), '/tmp/boxedwine-configuration', '-w', '/home/username', '/bin/sh', '-c', configuration_script(kind, value, token)]
        session = Session(emulator, arguments, wine, log, rotate=False)
        try:
            deadline = time.monotonic() + 120
            while not session.done.wait(.1):
                work.check()
                if time.monotonic() > deadline:
                    raise TimeoutError('Preparing Windows took more than two minutes. Your settings remain pending.')
        except Exception:
            session.force_stop()
            session.done.wait()
            raise
        if session.code != 0 or session.stopped or session.error:
            raise ValueError('The emulator stopped while preparing Wine settings. See the launch log.')
        completion, output = beneath(directory, 'completed'), beneath(directory, 'result.txt')
        if (not completion.is_file() or completion.stat().st_size > 128 or completion.read_text() != token or
                not output.is_file() or output.stat().st_size > 65536):
            raise ValueError('Wine did not finish preparing its settings. Check the launch log and try again.')
        return read_configuration(output.read_text(errors='replace'), kind)
    finally:
        delete_tree(directory, jobs)


def apply_configuration(emulator, library, app, wine, work):
    fields = [('windowsVersion', 'version'), ('wineRenderer', 'renderer'), ('openGLBackend', 'backend')]
    if not any(app.get(field + 'Pending') for field, _ in fields):
        return app
    jobs = beneath(library.directory, 'ConfigurationJobs')
    log = beneath(library.path(app), 'Logs/latest.log')
    for field, kind in fields:
        if not app.get(field + 'Pending'):
            continue
        expected = preference(app, field)
        work.report('Applying and checking Wine settings…')
        if kind == 'version' and expected == 'wineDefault':
            scratch = beneath(jobs, 'default-' + uuid.uuid4().hex)
            scratch.mkdir(parents=True)
            try:
                expected = run_configuration(emulator, scratch, wine, log, kind, None, jobs, work)
            finally:
                delete_tree(scratch, jobs)
        if run_configuration(emulator, library.root(app), wine, log, kind, expected, jobs, work) != expected:
            raise ValueError('Wine reported different settings. Your app has not been started.')
    work.check()
    ready = copy.deepcopy(app)
    for field, _ in fields:
        ready.pop(field + 'Pending', None)
    library.update(ready)
    return ready


def registry_values(text, section, names):
    """Read selected values from Wine's persisted user.reg without modifying it."""
    if not text.startswith('WINE REGISTRY Version 2\n') or '\0' in text:
        raise ValueError('Wine’s saved registry could not be read.')
    header = '[' + section.replace('\\', '\\\\') + ']'
    values, inside, seen = {}, False, False
    for line in text.splitlines():
        if line.startswith('['):
            inside = line.casefold().startswith(header.casefold())
            if inside and seen:
                raise ValueError('Duplicate Wine registry section.')
            seen |= inside
        elif inside:
            match = re.fullmatch(r'"([^"\\]+)"=(.*)', line)
            if match and match[1].casefold() in names:
                key = match[1].casefold()
                if key in values:
                    raise ValueError('Duplicate Wine registry value.')
                values[key] = match[2]
    return values


def read_graphics_settings(text):
    renderer = registry_values(text, r'Software\Wine\Direct3D', {'directdrawrenderer', 'renderer'})
    backend = registry_values(text, r'Software\Wine\X11 Driver', {'useegl'})
    draw, d3d = renderer.get('directdrawrenderer'), renderer.get('renderer')
    values = {
        'wineRenderer': {(None, None): 'wineDefault', ('"gdi"', '"gdi"'): 'gdi',
                         ('"opengl"', '"gl"'): 'openGL'}.get((draw, d3d)),
        'openGLBackend': {None: 'wineDefault', '"Y"': 'egl', '"N"': 'glx'}.get(backend.get('useegl')),
    }
    custom = {}
    if values['wineRenderer'] is None:
        custom['wineRenderer'] = f'DirectDrawRenderer={draw or "default"}; renderer={d3d or "default"}'[:512]
    if values['openGLBackend'] is None:
        custom['openGLBackend'] = ('UseEGL=' + backend['useegl'])[:512]
    return values, custom


def refresh_configuration(emulator, library, app, wine, work):
    """Read back auxiliary-program edits. Never apply settings during readback."""
    work.report('Reading Wine settings…')
    version = run_configuration(emulator, library.root(app), wine,
                                beneath(library.path(app), 'Logs/configuration.log'), 'version', None,
                                beneath(library.directory, 'ConfigurationJobs'), work)
    registry = beneath(library.root(app), 'home/username/.wine/user.reg')
    # The read-only winecfg query above initializes and flushes the prefix, so
    # this is Wine's current registry, not a guessed package default.
    with registry.open('rb') as stream:
        data = stream.read(16 * 1024**2 + 1)
    if len(data) > 16 * 1024**2:
        raise ValueError('Wine’s saved registry is too large to read back.')
    values, custom = read_graphics_settings(data.decode('utf-8'))
    values['windowsVersion'] = version if version in WINDOWS else None
    if values['windowsVersion'] is None:
        custom['windowsVersion'] = version
    work.check()
    ready = copy.deepcopy(app)
    observed = ready.setdefault('wineConfigurationCustom', {})
    for field, value in values.items():
        if ready.get(field + 'Pending'):
            continue  # Preserve a newer, explicitly requested UI setting.
        if value is None:
            observed[field] = custom[field]
        else:
            ready[field] = value
            observed.pop(field, None)
    if not observed:
        ready.pop('wineConfigurationCustom', None)
    ready.pop('wineConfigurationRefreshPending', None)
    library.update(ready)
    return ready
