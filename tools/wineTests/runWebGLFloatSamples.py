from pathlib import Path
import argparse
import hashlib
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import math
import secrets
import subprocess
import sys
import threading
import time
from urllib.parse import parse_qs, urlsplit

work = Path(__file__).resolve().parent
repo = work.parents[1]
sys.path.insert(0, str(repo / 'tools/wineTests'))
import wineGraphicsBrowser as browser
from auditGraphicsMatrix import diagnostics
from auditFloatRaw import audit_float
from prepareFloatExtensionControl import EXTENSIONS, FIXTURE, filter_script

parser = argparse.ArgumentParser(description='Standalone WebGL control; run alone after the matrix.')
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--probe', choices=('samples', 'capabilities'), default='samples')
parser.add_argument('--disable-extension', choices=EXTENSIONS, action='append', default=[])
args = parser.parse_args()
if args.disable_extension and args.probe != 'capabilities':
    parser.error('Extension controls require --probe capabilities')
args.output.mkdir(parents=True, exist_ok=False)
page_path = work / 'tests' / ('webgl_float_samples.html' if args.probe == 'samples' else 'webgl_float_caps.html')
original_page = page_path.read_bytes()
page = original_page
fixture = FIXTURE.read_bytes()
helper_path = work / 'prepareFloatExtensionControl.py'
helper = helper_path.read_bytes()
audit_path = work / 'auditFloatRaw.py'
audit_source = audit_path.read_bytes()
(args.output / 'auditFloatRaw.py').write_bytes(audit_source)
if args.disable_extension:
    if page.count(b'<script>') != 1:
        parser.error('Unexpected capability page script tags')
    page = page.replace(b'<script>', ('<script>\n' + filter_script(args.disable_extension)).encode('utf-8'))
(args.output / 'executed-page.html').write_bytes(page)
runner_hash = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
token = secrets.token_urlsafe(24)
done = threading.Event()
payload = {}
requests = []

class Handler(BaseHTTPRequestHandler):
    def log_message(self, message, *values):
        requests.append(message % values)

    def authorized(self, path):
        parsed = urlsplit(self.path)
        return parsed.path == path and parse_qs(parsed.query).get('token') == [token]

    def do_GET(self):
        if not self.authorized('/'):
            self.send_error(404); return
        self.send_response(200)
        self.send_header('Content-Type', 'text/html; charset=utf-8')
        self.send_header('Content-Length', str(len(page)))
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        self.wfile.write(page)

    def do_POST(self):
        if not self.authorized('/result'):
            self.send_error(404); return
        size = int(self.headers.get('Content-Length', '0'))
        if not 0 < size <= 2 * 1024 * 1024:
            self.send_error(413); return
        value = json.loads(self.rfile.read(size))
        if not isinstance(value, dict):
            self.send_error(400); return
        payload.update(value)
        self.send_response(204); self.end_headers()
        done.set()

server = thread = process = None
thread_started = False
url = None
command = []
problems = []
timed_out = exited_early = False
started = time.monotonic()
try:
    server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    thread_started = True
    url = f'http://127.0.0.1:{server.server_port}/?token={token}'
    command = browser.build_chrome_command(browser.find_chrome(), args.output / 'profile', url, headless=True)
    with (args.output / 'chrome.log').open('xb') as log:
        process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT,
            start_new_session=True, creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        while not done.wait(.25):
            if process.poll() is not None:
                exited_early = True
                break
            if time.monotonic() - started > 60:
                timed_out = True
                break
except Exception as error:
    problems.append(f'{type(error).__name__}: {error}')
finally:
    if process is not None:
        try:
            browser._terminate_process_tree(process)
        except Exception as error:
            problems.append(f'Browser cleanup: {type(error).__name__}: {error}')
    if server is not None:
        try:
            if thread_started:
                server.shutdown()
        except Exception as error:
            problems.append(f'Server shutdown: {type(error).__name__}: {error}')
        finally:
            try:
                server.server_close()
            except Exception as error:
                problems.append(f'Server close: {type(error).__name__}: {error}')
    if thread_started:
        thread.join(timeout=5)
        if thread.is_alive():
            problems.append('Server thread did not stop')
chrome_path = args.output / 'chrome.log'
chrome_log = chrome_path.read_bytes() if chrome_path.exists() else b''
browser_diagnostics = diagnostics(chrome_log.decode('utf-8', errors='replace'))
try:
    inputs_unchanged = (page_path.read_bytes() == original_page and FIXTURE.read_bytes() == fixture
        and helper_path.read_bytes() == helper and audit_path.read_bytes() == audit_source
        and hashlib.sha256(Path(__file__).read_bytes()).hexdigest() == runner_hash)
except OSError as error:
    inputs_unchanged = False
    problems.append(f'Input verification: {type(error).__name__}: {error}')
observation = payload.get('observationSeconds')
checks = payload.get('checks')
result_audit = audit_float(payload, args.probe, args.disable_extension)
result_valid = (payload.get('passed') is True and isinstance(observation, (int, float))
    and not isinstance(observation, bool) and math.isfinite(observation) and observation >= 15
    and isinstance(checks, list) and bool(checks)
    and all(isinstance(check, dict) and check.get('passed') is True for check in checks))
passed = bool(done.is_set() and result_valid and result_audit['passed'] and not timed_out and not exited_early and not problems
    and chrome_log.strip() and not browser_diagnostics and inputs_unchanged)
report = {'command': command, 'page': str(page_path), 'page_sha256': hashlib.sha256(page).hexdigest(),
    'source_page_sha256': hashlib.sha256(original_page).hexdigest(), 'disabled_extensions': args.disable_extension,
    'fixture_sha256': hashlib.sha256(fixture).hexdigest(), 'helper_sha256': hashlib.sha256(helper).hexdigest(),
    'audit_sha256': hashlib.sha256(audit_source).hexdigest(),
    'runner_sha256': runner_hash, 'chrome_log_sha256': hashlib.sha256(chrome_log).hexdigest(),
    'launch_url': url, 'browser_started': process is not None,
    'browser_exit_code': process.returncode if process is not None else None,
    'timed_out': timed_out, 'exited_early': exited_early, 'problems': problems,
    'duration_seconds': time.monotonic() - started, 'complete': done.is_set(), 'result': payload,
    'diagnostics': browser_diagnostics, 'result_audit': result_audit, 'inputs_unchanged': inputs_unchanged, 'passed': passed}
(args.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
(args.output / 'server.log').write_text('\n'.join(requests) + '\n', encoding='utf-8')
print(json.dumps(dict(complete=report['complete'], passed=passed,
    checks=len(checks) if isinstance(checks, list) else 0,
    allocations=len(payload['multisampleAllocations']) if isinstance(payload.get('multisampleAllocations'), list) else None,
    diagnostics=browser_diagnostics, result_problems=result_audit['problems'], problems=problems)))
raise SystemExit(0 if passed else 1)
