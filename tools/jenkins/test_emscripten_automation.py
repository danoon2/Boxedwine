from concurrent.futures import ThreadPoolExecutor
import errno
import json
from pathlib import Path
import tempfile
import sys
import threading
import time
import unittest
from unittest.mock import patch
from urllib.request import urlopen
import zipfile

import run_emscripten_automation as runner


class AutomationTests(unittest.TestCase):
    def test_parallel_http_downloads_resume_after_partial_send_and_enobufs(self):
        class ConstrainedSocket:
            def __init__(self, connection):
                self.connection = connection
                self.step = 0

            def __getattr__(self, name):
                return getattr(self.connection, name)

            def send(self, data):
                if self.step == 0 and len(data) >= 65536:
                    self.step = 1
                    return self.connection.send(data[:1234])
                if self.step in (1, 2):
                    self.step += 1
                    raise OSError(errno.ENOBUFS, 'injected kernel buffer exhaustion')
                return self.connection.send(data)

            def sendall(self, data):
                # Model the original stdlib writer: its error loses the byte
                # count after a partial send, truncating the HTTP response.
                if self.step == 0 and len(data) >= 65536:
                    self.connection.sendall(data[:1234])
                    self.step = 1
                    raise OSError(errno.ENOBUFS, 'injected kernel buffer exhaustion')
                return self.connection.sendall(data)

        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            payload = bytes(range(256)) * 8192
            (base / 'filesystem.zip').write_bytes(payload)
            server = runner.AutomationServer(base, base, 'app', 0)
            get_request = server.get_request
            sockets = []
            def accept():
                connection, address = get_request()
                wrapped = ConstrainedSocket(connection)
                sockets.append(wrapped)
                return wrapped, address
            server.get_request = accept
            worker = threading.Thread(target=server.serve_forever, daemon=True)
            worker.start()
            try:
                url = f'http://127.0.0.1:{server.server_port}/filesystem.zip'
                def download(_):
                    with urlopen(url, timeout=10) as response:
                        return response.read()
                with ThreadPoolExecutor(max_workers=4) as pool:
                    self.assertEqual(list(pool.map(download, range(4))), [payload] * 4)
                self.assertEqual([s.step for s in sockets], [3] * 4)
            finally:
                server.shutdown()
                server.server_close()
                worker.join()

    def test_http_writer_bounds_buffer_recovery_and_preserves_other_errors(self):
        from unittest.mock import Mock
        connection = Mock()
        connection.send.side_effect = OSError(errno.ENOBUFS, 'persistent exhaustion')
        with patch.object(runner.time, 'monotonic', side_effect=[0, 1, 6]), \
                patch.object(runner.time, 'sleep') as sleep, self.assertRaises(OSError):
            runner.SocketWriter(connection).write(b'payload')
        self.assertEqual(connection.send.call_count, 2)
        sleep.assert_called_once_with(0.05)
        connection.reset_mock()
        connection.send.side_effect = BrokenPipeError(errno.EPIPE, 'peer closed')
        with self.assertRaises(BrokenPipeError):
            runner.SocketWriter(connection).write(b'payload')
        self.assertEqual(connection.send.call_count, 1)

    def test_worker_reuses_verified_assets_and_rebuilds_changed_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            source, filesystem, cache = base / 'assets.zip', base / 'wine.zip', base / 'cache'
            with zipfile.ZipFile(source, 'w') as archive:
                archive.writestr('automation/scripts/app/Play.txt', '/files/\napp.exe\n')
                archive.writestr('automation/scripts/app/files/app.exe', b'original app')
                archive.writestr('automation/scripts/app/play/script.txt', b'DONE\r\n')
            def pin(path):
                return dict(url=path.as_uri(), size=path.stat().st_size, sha256=runner.digest(path))
            def wine(value):
                with zipfile.ZipFile(filesystem, 'w') as archive:
                    archive.writestr('home/username/.wine/user.reg', 'WINE REGISTRY Version 2\n' + value)
                return pin(filesystem)
            manifest = dict(assets=pin(source), filesystem=wine('; first root\n'), apps=['app'])
            with patch.object(runner, 'prepare', wraps=runner.prepare) as prepare:
                launches = runner.reuse_prepared(cache, manifest)
                self.assertEqual(runner.reuse_prepared(cache, manifest), launches)
                self.assertEqual(prepare.call_count, 1)
                (cache / 'app.zip').write_bytes(b'corrupt overlay')
                self.assertEqual(runner.reuse_prepared(cache, manifest), launches)
                self.assertEqual(prepare.call_count, 2)
                with zipfile.ZipFile(cache / 'app.zip') as archive:
                    self.assertEqual(archive.read('files/app.exe'), b'original app')
                manifest['filesystem'] = wine('; second root\n')
                runner.reuse_prepared(cache, manifest)
                self.assertEqual(prepare.call_count, 3)
                with zipfile.ZipFile(cache / 'app.zip') as archive:
                    self.assertIn(b'; second root', archive.read('home/username/.wine/user.reg'))

    def test_discovers_macos_chrome_outside_path(self):
        chrome = Path('/Applications/Google Chrome.app/Contents/MacOS/Google Chrome')
        with patch('wineGraphicsBrowser.sys.platform', 'darwin'), \
                patch('wineGraphicsBrowser.shutil.which', return_value=None), \
                patch.object(Path, 'is_file', lambda path: path == chrome):
            self.assertEqual(runner.find_chrome(), chrome)

    def test_native_pin_change_invalidates_browser_cache(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            (base / 'manifest.json').write_text(json.dumps(dict(filesystem_properties='filesystem.properties')))
            properties = base / 'filesystem.properties'
            properties.write_text('# shared pin\nurl=https://example.invalid/wine.zip\nsize=123\nsha256=old\n')
            manifest = runner.load_manifest(base / 'manifest.json')
            self.assertEqual(manifest['filesystem']['size'], 123)
            (base / 'prepared.json').write_text(json.dumps(dict(manifest=manifest, files={})))
            properties.write_text(properties.read_text().replace('sha256=old', 'sha256=new'))
            with self.assertRaisesRegex(ValueError, 'different manifest'):
                runner.read_prepared(base, runner.load_manifest(base / 'manifest.json'))

    def test_phases_navigate_one_browser_and_retries_start_from_install(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            build, cache, output = base / 'build', base / 'cache', base / 'results'
            build.mkdir()
            cache.mkdir()
            (build / 'boxedwine-shell.js').write_text('')
            client = base / 'browser.py'
            # A real HTTP client exercises the handoff, including the second
            # navigation and a failed install followed by a fresh attempt.
            client.write_text('''import json, re, sys
from urllib.request import urlopen, Request
from urllib.parse import urlsplit
url, attempt = sys.argv[1:]
origin = 'http://' + urlsplit(url).netloc
while url:
    script = urlopen(origin + '/boxedwine-shell.js').read().decode()
    config = json.loads(re.search(r'const config = (.*);', script).group(1))
    code = 2 if attempt == '1' and config['phase'] == 'install2' else 111
    payload = json.dumps(dict(code=code, syncError=None, browserErrors=[])).encode()
    response = urlopen(Request(origin + '/automation/' + config['token'] + '/result', data=payload), timeout=10)
    url = json.load(response)['url']
''')
            profiles = []
            def command(chrome, profile, url, **kwargs):
                profiles.append(profile)
                return [sys.executable, str(client), url, str(len(profiles))]
            phases = [dict(phase=p) for p in ('install', 'install2', 'play')]
            with patch.object(runner, 'build_chrome_command', side_effect=command):
                result = runner.run_app(Path('unused'), build, cache, 'app', phases, output, 0, 10, 2)
            self.assertTrue(result['passed'])
            self.assertEqual(len(profiles), 2)
            self.assertNotEqual(profiles[0], profiles[1])
            self.assertEqual([[p['phase'] for p in attempt] for attempt in result['attempts']],
                             [['install', 'install2'], ['install', 'install2', 'play']])

    def test_native_launch_options_and_program_arguments_stay_separate(self):
        launch = runner.parse_launch('/files/\napp.exe\n-resolution\n800x600\n-env\nWINEDLLOVERRIDES=glide2x,glide3x=\n-cpuAffinity\n1\nARGS\n-v\nwin32\n', 'app', 'Play')
        self.assertEqual(launch['args'], ['-v', 'win32'])
        self.assertEqual(launch['options'], ['-resolution', '800x600', '-env', 'WINEDLLOVERRIDES=glide2x,glide3x=', '-cpuAffinity', '1'])
        with self.assertRaises(ValueError):
            runner.parse_launch('/files/\napp.exe\n-new-option\n1', 'app', 'Play')

    def test_killed_tab_fails_promptly_and_retries_with_fresh_install(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            build, cache = base / 'build', base / 'cache'
            build.mkdir()
            cache.mkdir()
            (build / 'boxedwine-shell.js').write_text('')
            client = base / 'browser.py'
            client.write_text('''import json, re, sys, time
from urllib.request import urlopen, Request
from urllib.parse import urlsplit
url, attempt = sys.argv[1:]
origin = 'http://' + urlsplit(url).netloc
while url:
    script = urlopen(origin + '/boxedwine-shell.js').read().decode()
    config = json.loads(re.search(r'const config = (.*);', script).group(1))
    print('[1:1:1125/201334:WARNING:chrome/browser/ui/sad_tab.cc:257] Tab Killed: http://example.invalid/', flush=True)
    print('[1:1:1125/201334:INFO:CONSOLE:1] "example :WARNING:chrome/browser/ui/sad_tab.cc:257] Tab Killed: ' + url + '"', flush=True)
    time.sleep(0.4)
    if attempt == '1' and config['phase'] == 'install2':
        # The parent stays alive after its renderer dies. Split the diagnostic
        # across reads, as a concurrent Chromium log writer can do.
        print('[1:1:1125/201334:WARNING:chrome/browser/ui/sad_tab.cc:257] Tab Kil', end='', flush=True)
        time.sleep(0.4)
        print('led: ' + url, flush=True)
        time.sleep(30)
        raise SystemExit(9)
    payload = json.dumps(dict(code=111, syncError=None, browserErrors=[])).encode()
    response = urlopen(Request(origin + '/automation/' + config['token'] + '/result', data=payload), timeout=10)
    url = json.load(response)['url']
''')
            profiles = []
            def command(chrome, profile, url, **kwargs):
                profiles.append(profile)
                return [sys.executable, str(client), url, str(len(profiles))]
            start = time.monotonic()
            with patch.object(runner, 'build_chrome_command', side_effect=command):
                result = runner.run_app(Path('unused'), build, cache, 'app',
                                        [dict(phase=p) for p in ('install', 'install2', 'play')],
                                        base / 'results', 0, 20, 2)
            self.assertLess(time.monotonic() - start, 10)
            self.assertTrue(result['passed'])
            self.assertEqual(len(profiles), 2)
            self.assertNotEqual(profiles[0], profiles[1])
            self.assertEqual([[p['phase'] for p in attempt] for attempt in result['attempts']],
                             [['install', 'install2'], ['install', 'install2', 'play']])
            failure = result['attempts'][0][-1]
            self.assertEqual(failure['reason'], 'renderer_terminated')
            self.assertIn('Chromium automation tab killed:', failure['browserErrors'][0])
            self.assertFalse(failure['passed'])

    def test_preparation_preserves_recordings_and_binary_bytes(self):
        with tempfile.TemporaryDirectory() as directory:
            cache = Path(directory)
            recording = b'VERSION=2\r\nSCREENSHOT=1,2,3,4,final-dialog-body.bmp\r\nWHILEWAITING=5,LBUTTON,630,14,550,448\r\nDONE\r\n'
            with zipfile.ZipFile(cache / 'automation-assets.zip', 'w') as archive:
                archive.writestr('automation/scripts/app/files/app.exe', b'original executable\0\xff')
                for phase in runner.PHASES:
                    archive.writestr(f'automation/scripts/app/{phase}.txt', '/files/\napp.exe\n-resolution\n1024x768')
                    archive.writestr(f'automation/scripts/app/{phase.lower()}/script.txt', recording)
                    archive.writestr(f'automation/scripts/app/{phase.lower()}/screenshot1.bmp', b'original bitmap')
                    archive.writestr(f'automation/scripts/app/{phase.lower()}/final-dialog-body.bmp', b'packaged cropped bitmap')
            with zipfile.ZipFile(cache / 'filesystem.zip', 'w') as archive:
                archive.writestr('home/username/.wine/user.reg', 'WINE REGISTRY Version 2\n')
            with patch.object(runner, 'download'):
                launches = runner.prepare(cache, dict(assets={}, filesystem={}, apps=['app']))
            self.assertEqual([item['phase'] for item in launches['app']], ['install', 'install2', 'play'])
            with zipfile.ZipFile(cache / 'app.zip') as overlay:
                self.assertEqual(overlay.read('files/app.exe'), b'original executable\0\xff')
                for phase in ('install', 'install2', 'play'):
                    self.assertEqual(overlay.read(f'automation/{phase}/script.txt'), recording)
                    self.assertEqual(overlay.read(f'automation/{phase}/screenshot1.bmp'), b'original bitmap')
                    self.assertEqual(overlay.read(f'automation/{phase}/final-dialog-body.bmp'), b'packaged cropped bitmap')

    def test_registry_changes_only_native_automation_settings(self):
        original = b'WINE REGISTRY Version 2\n\n[Software\\\\Wine\\\\Direct3D] 123\n"renderer"="gl"\n"unrelated"="keep"\n\n[Software\\\\Other]\n"renderer"="keep"\n'
        updated = runner.automation_registry(original)
        self.assertIn(b'"renderer"="gdi"', updated)
        self.assertIn(b'"DirectDrawRenderer"="gdi"', updated)
        self.assertIn(b'"unrelated"="keep"', updated)
        self.assertIn(b'[Software\\\\Other]\n"renderer"="keep"', updated)
        self.assertNotIn(b'"renderer"="gl"', updated)
        self.assertEqual(updated, runner.automation_registry(updated))

    def test_download_rejects_corruption(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'cached.zip'
            source = Path(directory) / 'source.zip'
            source.write_bytes(b'bad archive')
            pin = dict(url=source.as_uri(), size=source.stat().st_size, sha256='0' * 64)
            with patch.object(runner.time, 'sleep'), self.assertRaisesRegex(ValueError, 'mismatch'):
                runner.download(pin, path)
            self.assertFalse(path.exists())

    def test_junit_reports_failed_app_and_passed_retry(self):
        results = [dict(target='mt', app='first', passed=False, attempts=[[dict(code=0)]]),
                   dict(target='jit', app='second', passed=True, attempts=[[dict(code=2)], [dict(code=111)]])]
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            runner.write_results(output, results)
            suite = runner.ET.parse(output / 'junit.xml').getroot()
            self.assertEqual(suite.attrib['tests'], '2')
            self.assertEqual(suite.attrib['failures'], '1')
            self.assertEqual(suite.find('testcase/failure').attrib['message'], 'Browser automation failed')
            self.assertEqual(json.loads((output / 'results.json').read_text()), results)


if __name__ == '__main__':
    unittest.main()
