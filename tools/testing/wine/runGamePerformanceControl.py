#!/usr/bin/env python3
"""Run the standalone canvas observer control in Chrome, with no emulator/game.

Run serially after other graphics jobs. This checks browser API behavior and
retains small-canvas overhead samples; it does not accept game performance.
"""
import argparse
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import secrets
import subprocess
import threading
import time
from urllib.parse import parse_qs, urlsplit

from auditGraphicsMatrix import diagnostics
import wineGraphicsBrowser as browser


def identity(path):
    return dict(path=str(path.resolve()), **browser.build_ids.file_identity(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    folder = Path(__file__).resolve().parent
    names = ['gamePerformance.mjs', 'fixtures/game-performance-control.mjs']
    sources = {name: (folder / name).read_bytes() for name in names}
    for name, data in sources.items():
        (output / Path(name).name).write_bytes(data)
    pins = [identity(folder / name) for name in names] + [identity(Path(__file__))]
    token = secrets.token_urlsafe(24)
    html = ('<!doctype html><meta charset="utf-8"><title>Presentation observer control</title>'
        '<div id="frame"></div><pre id="result">Running</pre>'
        '<script type="module" src="/fixtures/game-performance-control.mjs?token=' + token + '"></script>').encode()
    (output / 'index.html').write_bytes(html)
    done = threading.Event()
    payload, requests = {}, []

    class Handler(BaseHTTPRequestHandler):
        def log_message(self, message, *values):
            requests.append(message % values)

        def route(self):
            parsed = urlsplit(self.path)
            return parsed.path if parse_qs(parsed.query).get('token') == [token] else None

        def do_GET(self):
            path = self.route()
            if path == '/':
                data, mime = html, 'text/html; charset=utf-8'
            elif path and path[1:] in sources:
                data, mime = sources[path[1:]], 'text/javascript; charset=utf-8'
            else:
                self.send_error(404); return
            self.send_response(200)
            self.send_header('Content-Type', mime)
            self.send_header('Content-Length', str(len(data)))
            self.send_header('Cache-Control', 'no-store')
            self.end_headers(); self.wfile.write(data)

        def do_POST(self):
            if self.route() != '/result' or done.is_set():
                self.send_error(404); return
            try:
                size = int(self.headers.get('Content-Length', '0'))
                if not 0 < size <= 8 * 1024 * 1024:
                    raise ValueError('Invalid payload size')
                value = json.loads(self.rfile.read(size))
                if not isinstance(value, dict):
                    raise ValueError('Invalid payload')
            except (ValueError, TypeError):
                self.send_error(400); return
            payload.update(value)
            self.send_response(204); self.end_headers(); done.set()

    record = dict(started_at=datetime.now(timezone.utc).isoformat(), scope=__doc__, command=[],
        sources=pins, passed=False, game_performance_acceptance=False, problems=[], launch_url=None,
        timed_out=False, exited_early=False)
    server = server_thread = process = None
    thread_started = False
    started = time.monotonic()
    try:
        server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        server_thread = threading.Thread(target=server.serve_forever, daemon=True)
        server_thread.start()
        thread_started = True
        record['launch_url'] = f'http://127.0.0.1:{server.server_port}/?token={token}'
        record['command'] = browser.build_chrome_command(browser.find_chrome(), output / 'profile',
                                                        record['launch_url'], headless=True)
        with (output / 'chrome.log').open('xb') as log:
            process = subprocess.Popen(record['command'], stdout=log, stderr=subprocess.STDOUT,
                start_new_session=os.name != 'nt', creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            while not done.wait(.25):
                if process.poll() is not None:
                    record['exited_early'] = True
                    break
                if time.monotonic() - started > 90:
                    record['timed_out'] = True
                    break
            if done.is_set():
                time.sleep(2)  # Retain late browser exceptions after the payload.
                if process.poll() is not None:
                    record['exited_early'] = True
    except Exception as error:
        record['problems'].append(f'{type(error).__name__}: {error}')
    finally:
        if process is not None:
            try:
                browser._terminate_process_tree(process)
            except Exception as error:
                record['problems'].append(f'Browser cleanup: {type(error).__name__}: {error}')
        if server is not None:
            try:
                if thread_started:
                    server.shutdown()
            except Exception as error:
                record['problems'].append(f'Server shutdown: {type(error).__name__}: {error}')
            finally:
                try:
                    server.server_close()
                except Exception as error:
                    record['problems'].append(f'Server close: {type(error).__name__}: {error}')
        if thread_started:
            server_thread.join(timeout=5)
            if server_thread.is_alive():
                record['problems'].append('Server thread did not stop')
    if record['timed_out']:
        record['problems'].append('Browser control timed out')
    if record['exited_early']:
        record['problems'].append('Browser exited before the observation finished')
    required = {'module_uninstalled', 'baseline_pixels', 'native_exception', 'drawImage', 'putImageData',
        'transferFromImageBitmap', 'hidden_parent', 'overhead_counts', 'restored', 'gl_error'}
    checks = payload.get('checks')
    if (not done.is_set() or payload.get('kind') != 'game-performance-browser-control'
            or payload.get('passed') is not True or payload.get('errors') != []
            or not isinstance(checks, list) or not all(isinstance(row, dict) for row in checks)
            or not all(isinstance(row.get('name'), str) for row in checks)
            or len(checks) != len(required) or {row.get('name') for row in checks} != required
            or not all(row.get('passed') is True for row in checks)):
        record['problems'].append('Missing or failed browser controls')
    log = ''
    try:
        if (output / 'chrome.log').exists():
            log = (output / 'chrome.log').read_text(errors='replace')
    except OSError as error:
        record['problems'].append('Chrome log unavailable: ' + str(error))
    record['diagnostics'] = diagnostics(log)
    if not log.strip() or record['diagnostics']:
        record['problems'].append('Empty Chrome log or browser/graphics diagnostics')
    try:
        if any(identity(Path(pin['path'])) != pin for pin in pins):
            record['problems'].append('Control source changed during execution')
    except OSError as error:
        record['problems'].append('Control source unavailable after execution: ' + str(error))
    record.update(passed=not record['problems'], result=payload, complete=done.is_set(),
        browser_started=process is not None, browser_exit_code=process.returncode if process is not None else None,
        duration_seconds=time.monotonic() - started, finished_at=datetime.now(timezone.utc).isoformat())
    (output / 'report.json').write_text(json.dumps(record, indent=2) + '\n', newline='\n')
    (output / 'server.log').write_text('\n'.join(requests) + '\n', newline='\n')
    print(('PASS' if record['passed'] else 'FAIL') + ': browser performance observer control: ' + str(output))
    return 0 if record['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
