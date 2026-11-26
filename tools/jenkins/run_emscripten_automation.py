#!/usr/bin/env python3
"""Replay the native app suite in headless Chrome, including installer phases."""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import hashlib
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import re
import secrets
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from urllib.parse import urlencode, unquote, urlparse
from urllib.request import urlopen
import xml.etree.ElementTree as ET
import zipfile

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
sys.path.insert(0, str(REPO / 'tools/testing/wine'))
from wineGraphicsBrowser import find_chrome, build_chrome_command, _terminate_process_tree

TARGETS = {
    'st': 'Automation', 'mt': 'AutomationMultiThreaded',
    'jit': 'AutomationJit', 'mtjit': 'AutomationMultiThreadedJit',
}
PHASES = ('Install', 'Install2', 'Play')


def load_manifest(path):
    manifest = json.loads(path.read_text())
    if 'filesystem_properties' in manifest:
        # Match desktop installation flows (Gecko/file associations included),
        # with one release pin shared by the native and browser suites.
        properties = path.parent / manifest['filesystem_properties']
        pin = dict(line.split('=', 1) for line in properties.read_text().splitlines()
                   if line and not line.startswith('#'))
        manifest['filesystem'] = dict(url=pin['url'], sha256=pin['sha256'], size=int(pin['size']))
    return manifest


def digest(path):
    result = hashlib.sha256()
    with path.open('rb') as source:
        for block in iter(lambda: source.read(1024 * 1024), b''):
            result.update(block)
    return result.hexdigest()


def download(pin, path):
    def matches(candidate):
        return candidate.is_file() and candidate.stat().st_size == pin['size'] and digest(candidate) == pin['sha256']
    if matches(path):
        return
    temporary = path.with_suffix('.download')
    for attempt in range(3):
        try:
            with urlopen(pin['url'], timeout=60) as source, temporary.open('wb') as out:
                shutil.copyfileobj(source, out)
            if not matches(temporary):
                raise ValueError('download size/SHA-256 mismatch: ' + pin['url'])
            temporary.replace(path)
            return
        except Exception:
            temporary.unlink(missing_ok=True)
            if attempt == 2:
                raise
            time.sleep(5)


def parse_launch(text, app, phase):
    lines = text.splitlines()
    if len(lines) < 2:
        raise ValueError(f'{app}/{phase}: missing working directory or executable')
    options, args = lines[2:], []
    if 'ARGS' in options:
        index = options.index('ARGS')
        options, args = options[:index], options[index + 1:]
    # Fail closed when the native runner gains options this harness has not
    # qualified. Pass these through verbatim, including affinity and overrides.
    allowed = {'-resolution', '-bpp', '-cpuAffinity', '-env'}
    if len(options) % 2 or any(option not in allowed for option in options[::2]):
        raise ValueError(f'{app}/{phase}: unsupported emulator options {options}')
    return dict(cwd=lines[0], program=lines[1], options=options, args=args,
                script=f'/automation/{phase.lower()}/script.txt')


def automation_registry(data):
    text = data.decode('utf-8')
    section = r'[Software\\Wine\\Direct3D]'
    start = text.find(section)
    if start >= 0:
        end = text.find('\n[', start + len(section))
        end = len(text) if end < 0 else end
        body = text[start:end]
        body = re.sub(r'^"(?:VideoMemorySize|DirectDrawRenderer|renderer)"=.*\n?', '', body, flags=re.M)
        body = body.rstrip() + '\n"VideoMemorySize"="256"\n"DirectDrawRenderer"="gdi"\n"renderer"="gdi"\n'
        text = text[:start] + body + text[end:]
    else:
        text += '\n' + section + '\n"VideoMemorySize"="256"\n"DirectDrawRenderer"="gdi"\n"renderer"="gdi"\n'
    return text.encode('utf-8')


def prepare(cache, manifest):
    cache.mkdir(parents=True, exist_ok=True)
    assets, filesystem = cache / 'automation-assets.zip', cache / 'filesystem.zip'
    download(manifest['assets'], assets)
    download(manifest['filesystem'], filesystem)
    launches = {}
    with zipfile.ZipFile(assets) as source, zipfile.ZipFile(filesystem) as root:
        apps = sorted({n.split('/')[2] for n in source.namelist() if n.startswith('automation/scripts/') and n.count('/') > 3})
        if apps != sorted(manifest['apps']):
            raise ValueError('asset suite differs from the pinned app list')
        registry = automation_registry(root.read('home/username/.wine/user.reg'))
        for app in apps:
            prefix = f'automation/scripts/{app}/'
            launches[app] = []
            with zipfile.ZipFile(cache / (app + '.zip'), 'w', zipfile.ZIP_DEFLATED) as overlay:
                overlay.writestr('home/username/.wine/user.reg', registry)
                for info in source.infolist():
                    if not info.filename.startswith(prefix) or info.is_dir():
                        continue
                    relative = info.filename[len(prefix):]
                    if relative in [phase + '.txt' for phase in PHASES]:
                        continue
                    if relative.startswith('files/'):
                        destination = relative
                    elif relative.split('/')[0] in ('install', 'install2', 'play'):
                        destination = 'automation/' + relative
                    else:
                        raise ValueError('unexpected app asset: ' + info.filename)
                    # Use the same packaged binaries, recordings, and references as native.
                    overlay.writestr(destination, source.read(info))
            for phase in PHASES:
                filename = prefix + phase + '.txt'
                if filename in source.namelist():
                    launch = parse_launch(source.read(filename).decode('utf-8'), app, phase)
                    if prefix + phase.lower() + '/script.txt' not in source.namelist():
                        raise ValueError('missing recording for ' + filename)
                    launches[app].append(dict(phase=phase.lower(), **launch))
            if not launches[app] or launches[app][-1]['phase'] != 'play':
                raise ValueError('missing Play.txt for ' + app)
    (cache / 'launches.json').write_text(json.dumps(launches, indent=2) + '\n')
    (cache / 'prepared.json').write_text(json.dumps(dict(manifest=manifest, files={
        name: digest(cache / name) for name in ['launches.json', 'filesystem.zip', *[app + '.zip' for app in apps]]
    }), indent=2) + '\n')
    return launches


def read_prepared(cache, manifest):
    prepared = json.loads((cache / 'prepared.json').read_text())
    if prepared['manifest'] != manifest:
        raise ValueError('prepared assets use a different manifest; run --prepare-only')
    for name, expected in prepared['files'].items():
        if Path(name).name != name or digest(cache / name) != expected:
            raise ValueError('prepared asset changed: ' + name)
    return json.loads((cache / 'launches.json').read_text())


def reuse_prepared(cache, manifest):
    """Share verified assets between queued cases on one worker, rebuilding stale caches."""
    try:
        return read_prepared(cache, manifest)
    except (OSError, ValueError, KeyError) as error:
        print(f'Preparing automation assets: {error}', flush=True)
        return prepare(cache, manifest)


class AutomationServer(ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self, build, cache, app, port):
        self.build, self.cache, self.app = build, cache, app
        self.archives = {name: dict(size=path.stat().st_size, sha256=digest(path))
                         for name, path in {'filesystem.zip': cache / 'filesystem.zip',
                                            'app.zip': cache / (app + '.zip')}.items()}
        self.completed = threading.Event()
        self.next_page = threading.Event()
        self.finishing = False
        self.result = None
        self.lock = threading.Lock()
        super().__init__(('127.0.0.1', port), Handler)

    def phase(self, launch, output):
        self.next_page.clear()
        self.completed.clear()
        self.result = None
        self.output = output
        output.mkdir(parents=True)
        self.token = secrets.token_hex(16)
        config = dict(launch, token=self.token, archives=self.archives)
        self.client = (HERE / 'emscripten-automation-client.js').read_text().replace('AUTOMATION_CONFIG', json.dumps(config))
        (output / 'launch.json').write_text(json.dumps(dict(launch,
            client_sha256=hashlib.sha256(self.client.encode()).hexdigest()), indent=2) + '\n')


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(args[2].build), **kwargs)

    def log_message(self, *args):
        pass

    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def do_GET(self):
        path = urlparse(self.path).path
        if path == '/boxedwine-shell.js':
            data = (self.server.build / 'boxedwine-shell.js').read_bytes() + b'\n' + self.server.client.encode()
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.send_header('Content-Length', str(len(data)))
            self.end_headers()
            self.wfile.write(data)
        elif path in ('/filesystem.zip', '/app.zip'):
            file = self.server.cache / ('filesystem.zip' if path == '/filesystem.zip' else self.server.app + '.zip')
            self.send_response(200)
            self.send_header('Content-Type', 'application/zip')
            self.send_header('Content-Length', str(file.stat().st_size))
            self.end_headers()
            with file.open('rb') as source:
                shutil.copyfileobj(source, self.wfile)
        else:
            super().do_GET()

    def do_POST(self):
        prefix = '/automation/' + self.server.token + '/'
        if not self.path.startswith(prefix):
            # The SDK --emrun hooks are still present; this harness waits for
            # onExit and the filesystem flush, rather than that earlier exit.
            self.send_error(404)
            return
        suffix = self.path[len(prefix):]
        size = int(self.headers.get('Content-Length', '0'))
        if size > 64 * 1024 * 1024:
            self.send_error(413)
            return
        data = self.rfile.read(size)
        token = self.server.token
        with self.server.lock:
            if suffix == 'log':
                with (self.server.output / 'console.log').open('a', encoding='utf-8') as out:
                    out.write('\n'.join(json.loads(data)) + '\n')
            elif suffix == 'result':
                self.server.result = json.loads(data)
                self.server.completed.set()
            elif suffix.startswith('file/') and unquote(suffix[5:]) in ('failed.bmp', 'failed_diff.bmp', 'failed-screen.bmp'):
                (self.server.output / unquote(suffix[5:])).write_bytes(data)
            else:
                self.send_error(400)
                return
        self.send_response(200)
        response = b''
        if suffix == 'result':
            # Keep one browser and one tab throughout installation and playback.
            # Relaunching a forcibly closed profile may restore the previous tab
            # alongside the new one, running two guests against the same root.
            deadline = time.monotonic() + 60
            while self.server.token == token and not self.server.finishing and time.monotonic() < deadline:
                self.server.next_page.wait(max(0, deadline - time.monotonic()))
                self.server.next_page.clear()
            response = json.dumps(dict(url=self.server.launch_url if self.server.token != token else None)).encode()
            self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(response)))
        try:
            self.end_headers()
            self.wfile.write(response)
        except (BrokenPipeError, ConnectionResetError):
            # The final result is already received; Chrome may close before its
            # acknowledgement is written when the runner ends the attempt.
            if not self.server.finishing:
                raise


class BrowserLogMonitor:
    """A crashed renderer can leave the browser parent alive indefinitely."""

    def __init__(self, stream):
        self.stream = stream
        self.tail = ''

    def failure(self, launch_url):
        data = self.tail + self.stream.read()
        self.tail = data[-4096:]
        origin = urlparse(launch_url).netloc
        # Match Chromium's own sad-tab diagnostic, not application console text
        # or a GPU subprocess restarting. This browser has one automation tab.
        for match in re.finditer(r'^\[[^\]\r\n]+:(?:WARNING|ERROR):[^\]\r\n]*sad_tab\.cc:\d+\] Tab (Killed|Crashed): (https?://[^\s"\']+)', data, re.MULTILINE):
            if urlparse(match[2]).netloc == origin:
                return f'Chromium automation tab {match[1].lower()}: {match[2]}'
        return None


@contextmanager
def browser_profile():
    profile = tempfile.TemporaryDirectory(prefix='boxedwine-automation-')
    try:
        yield profile.name
    finally:
        # taskkill can return before Chrome's children release their profile
        # handles on Windows. Retry only this owned profile, with a fixed bound;
        # a persistent cleanup failure must still fail the runner.
        deadline = time.monotonic() + 5
        while True:
            try:
                profile.cleanup()
                break
            except PermissionError:
                if sys.platform != 'win32' or time.monotonic() >= deadline:
                    raise
                time.sleep(0.25)


def run_app(chrome, build, cache, app, phases, output, port, timeout, retries):
    server = AutomationServer(build, cache, app, port)
    worker = threading.Thread(target=server.serve_forever, daemon=True)
    worker.start()
    attempts = []
    try:
        for attempt in range(1, retries + 1):
            server.finishing = False
            records = []
            attempts.append(records)
            with browser_profile() as profile:
                attempt_dir = output / f'attempt-{attempt}'
                attempt_dir.mkdir(parents=True)
                process = None
                with (attempt_dir / 'chrome.log').open('w') as log, \
                        (attempt_dir / 'chrome.log').open(errors='replace') as log_reader:
                    monitor = BrowserLogMonitor(log_reader)
                    try:
                        for launch in phases:
                            folder = attempt_dir / launch['phase']
                            server.phase(launch, folder)
                            query = urlencode(dict(root='filesystem', overlay='app', storage='indexeddb', storageKey='automation', phase=server.token))
                            server.launch_url = f'http://127.0.0.1:{server.server_port}/boxedwine.html?{query}'
                            start = time.monotonic()
                            if process is None:
                                command = build_chrome_command(chrome, Path(profile), server.launch_url, headless=True)
                                process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                            else:
                                server.next_page.set()
                            failure = {}
                            while not server.completed.wait(0.25):
                                browser_error = monitor.failure(server.launch_url)
                                if browser_error:
                                    failure = dict(reason='renderer_terminated', browserErrors=[browser_error])
                                    break
                                if process.poll() is not None:
                                    failure = dict(reason='browser_exited', browserExitCode=process.returncode)
                                    break
                                if time.monotonic() - start > timeout:
                                    failure = dict(reason='phase_timeout')
                                    break
                            result = dict(server.result or failure, phase=launch['phase'], seconds=round(time.monotonic() - start, 2))
                            result['passed'] = (result.get('code') == 111 and 'syncError' in result
                                                and result['syncError'] is None and not result.get('browserErrors'))
                            records.append(result)
                            (folder / 'result.json').write_text(json.dumps(result, indent=2) + '\n')
                            print(f'{build.name}/{app}/{launch["phase"]} attempt {attempt}: {result}', flush=True)
                            if not result['passed']:
                                break
                    finally:
                        server.finishing = True
                        server.next_page.set()
                        if process is not None:
                            _terminate_process_tree(process)
            if len(records) == len(phases) and all(r['passed'] for r in records):
                return dict(passed=True, attempts=attempts)
        return dict(passed=False, attempts=attempts)
    finally:
        server.shutdown()
        server.server_close()
        worker.join()


def write_results(output, results):
    (output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
    suite = ET.Element('testsuite', name='Emscripten automation', tests=str(len(results)),
                       failures=str(sum(not r['passed'] for r in results)))
    for result in results:
        case = ET.SubElement(suite, 'testcase', classname=result['target'], name=result['app'])
        if not result['passed']:
            ET.SubElement(case, 'failure', message='Browser automation failed').text = json.dumps(result['attempts'])
    ET.ElementTree(suite).write(output / 'junit.xml', encoding='utf-8', xml_declaration=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, default=HERE / 'emscripten-automation.json')
    parser.add_argument('--cache', type=Path, default=REPO / 'project/emscripten/automation-assets')
    parser.add_argument('--output', type=Path)
    parser.add_argument('--prepare-only', action='store_true')
    cache_mode = parser.add_mutually_exclusive_group()
    cache_mode.add_argument('--prepared', action='store_true', help='verify and reuse assets from --prepare-only')
    cache_mode.add_argument('--reuse-prepared', action='store_true',
                            help='reuse verified assets, preparing them when missing, stale, or corrupt')
    parser.add_argument('--target', choices=TARGETS, action='append')
    parser.add_argument('--app', action='append')
    parser.add_argument('--chrome', type=Path)
    parser.add_argument('--port', type=int, default=0)
    parser.add_argument('--timeout', type=int, default=900)
    parser.add_argument('--retries', type=int, default=3)
    args = parser.parse_args()
    manifest = load_manifest(args.manifest)
    apps = manifest['apps']
    if args.app and set(args.app) - set(apps):
        parser.error('unknown app')
    if args.retries < 1 or args.timeout < 1:
        parser.error('retries and timeout must be positive')
    prepare_assets = read_prepared if args.prepared else reuse_prepared if args.reuse_prepared else prepare
    launches = prepare_assets(args.cache.resolve(), manifest)
    if args.prepare_only:
        return 0
    if not args.output:
        parser.error('--output is required when running tests')
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    chrome = find_chrome(args.chrome)
    builds = {target: REPO / 'project/emscripten/Build' / TARGETS[target] for target in args.target or TARGETS}
    inputs = dict(manifest=manifest, chrome=str(chrome), builds={
        target: {name: digest(build / name) for name in ('boxedwine.js', 'boxedwine.wasm', 'boxedwine-shell.js')}
        for target, build in builds.items()
    })
    inputs['harness'] = {path.name: digest(path) for path in (Path(__file__), HERE / 'emscripten-automation-client.js')}
    (output / 'inputs.json').write_text(json.dumps(inputs, indent=2) + '\n')
    results = []
    for target, build in builds.items():
        for app in args.app or apps:
            result = run_app(chrome, build, args.cache.resolve(), app, launches[app], output / target / app,
                             args.port, args.timeout, args.retries)
            results.append(dict(target=target, app=app, **result))
            write_results(output, results)
    return 0 if all(result['passed'] for result in results) else 1


if __name__ == '__main__':
    sys.exit(main())
