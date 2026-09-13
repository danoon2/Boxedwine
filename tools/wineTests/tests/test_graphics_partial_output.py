"""Exercise artifact retention with a real HTTP-reporting child, without a GPU."""

from dataclasses import replace
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import zipfile
from urllib.parse import parse_qs, urlsplit
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import wineGraphicsBrowser as graphics


HELPER = r'''
import json, re, sys, time
from urllib.request import Request, urlopen
from urllib.parse import urlsplit

url, mode = sys.argv[1:]
with urlopen(url, timeout=5) as response:
    html = response.read().decode()
token = json.loads(re.search(r'const token = ("[^"\n]+")', html).group(1))
parts = urlsplit(url)
endpoint = parts.scheme + '://' + parts.netloc + '/__boxedwine_graphics_progress?token=' + token
payload = {
    'kind': 'progress',
    'output': 'visual: fixture phase started\nvisual.c:123: Test failed: retained fixture failure\n',
    'status': 'Running...', 'heapBytes': 1048576, 'browserEvents': [],
    'cleanupWaitSatisfied': False, 'consoleTail': ['log: retained partial fixture'],
    'userAgent': 'artifact-retention-fixture', 'href': url,
    'timestamp': '2026-09-10T00:00:00Z'
}
request = Request(endpoint, data=json.dumps(payload).encode(),
    headers={'Content-Type': 'application/json'})
with urlopen(request, timeout=5) as response:
    assert response.status == 204
print('PARTIAL_OUTPUT_FIXTURE_POSTED', flush=True)
if mode in ('late-progress', 'changed-input', 'removed-input'):
    if mode in ('changed-input', 'removed-input'):
        from pathlib import Path
        runtime_js = Path(__file__).parent / 'runtime/boxedwine-shell.js'
        if mode == 'changed-input':
            runtime_js.write_bytes(b'changed during execution')
        else:
            runtime_js.unlink()
    complete = dict(payload, kind='complete', cleanupWaitSatisfied=True,
        output='0020:visual: 3 tests executed (0 marked as todo, 0 failures), 0 skipped.\n'
            'BOXEDWINE_TEST_EXIT:0\nBOXEDWINE_WINESERVER_CLEANUP_OK\n')
    for value in (complete, payload):
        request = Request(endpoint, data=json.dumps(value).encode(),
            headers={'Content-Type': 'application/json'})
        with urlopen(request, timeout=5) as response:
            assert response.status == 204
    print('LATE_PROGRESS_FIXTURE_POSTED', flush=True)
    sys.exit(0)
if mode == 'exit':
    sys.exit(13)
time.sleep(60)
'''


class PartialOutputArtifactTests(unittest.TestCase):
    def run_interrupted_child(self, mode):
        with tempfile.TemporaryDirectory(prefix='boxedwine-partial-output-') as temp:
            base = Path(temp)
            runtime = base / 'runtime'
            runtime.mkdir()
            (runtime / 'boxedwine.html').write_text(
                '<html><head></head><body><textarea id="output"></textarea>'
                '<script src="boxedwine-shell.js"></script>'
                '<script src="boxedwine.js"></script></body></html>')
            (runtime / 'boxedwine.wasm').write_bytes(b'artifact fixture, not executable WASM')
            for name in ('boxedwine.js', 'boxedwine-shell.js', 'boxedwine.css'):
                (runtime / name).write_bytes(('fixture ' + name).encode())
            root = base / 'root.zip'
            with zipfile.ZipFile(root, 'w') as archive:
                archive.writestr('etc/hostname', b'fixture, not an emulated filesystem')
            executable = base / 'd3d9_test.exe'
            # Only the packaging header is required; the child never executes it.
            image = bytearray(0x200)
            image[:2] = b'MZ'
            struct.pack_into('<I', image, 0x3c, 0x80)
            image[0x80:0x84] = b'PE\0\0'
            struct.pack_into('<H', image, 0x84, 0x014c)
            struct.pack_into('<H', image, 0x98, 0x010b)
            executable.write_bytes(image)
            helper = base / 'report_partial.py'
            helper.write_text(HELPER)
            suite = replace(graphics.GRAPHICS_SUITES['d3d9'],
                cleanup_wait_seconds=15, exit_status_policy='wine')

            def child_command(chrome, profile, launch_url, *, headless):
                return [sys.executable, str(helper), launch_url, mode]

            with mock.patch.object(graphics, 'build_chrome_command', child_command):
                result, manifest = graphics.run_browser_test(
                    suite=suite, group='visual', build_dir=runtime, filesystem=root,
                    test_executable=executable, chrome=Path(sys.executable),
                    run_dir=base / 'run', timeout=3, headless=True,
                    keep_browser_profile=False, mode='single-threaded-non-jit')

            artifacts = base / 'run'
            payload = json.loads((artifacts / 'browser-payload.json').read_text())
            retained = (artifacts / 'wine.log').read_text()
            self.assertEqual(payload['output'], retained)
            identity = json.loads((artifacts / 'build-identity.json').read_text())
            self.assertEqual(identity, manifest['build_identity'])
            self.assertEqual([identity['id']], parse_qs(urlsplit(manifest['launch_url']).query)['buildid'])
            self.assertEqual({'commit': None, 'dirty': None}, identity['source'])
            self.assertIsNone(identity['filesystem']['webgl_wined3d'])
            self.assertEqual(set(graphics.REQUIRED_WEB_FILES), set(identity['runtime']))
            if mode in ('changed-input', 'removed-input'):
                self.assertFalse(result.passed)
                self.assertFalse(manifest['input_identity_verified'])
                self.assertEqual(1, len(manifest['input_identity_problems']))
                self.assertIn('boxedwine-shell.js', result.reason)
                self.assertEqual(3, result.tests, 'Keep the completed test result with failed identity validation')
                self.assertEqual(0, result.exit_status)
                return
            self.assertTrue(manifest['input_identity_verified'])
            if mode == 'late-progress':
                self.assertEqual('complete', payload['kind'])
                self.assertTrue(result.passed, result.reason)
                self.assertEqual(3, result.tests)
                self.assertEqual(0, result.exit_status)
                self.assertTrue(payload['cleanupWaitSatisfied'])
                self.assertIn('LATE_PROGRESS_FIXTURE_POSTED', (artifacts / 'chrome.log').read_text())
                return
            self.assertIn('visual.c:123: Test failed: retained fixture failure', retained)
            self.assertIn('PARTIAL_OUTPUT_FIXTURE_POSTED', (artifacts / 'chrome.log').read_text())
            self.assertIn('POST /__boxedwine_graphics_progress?', (artifacts / 'server.log').read_text())
            self.assertEqual('progress', payload['kind'])
            self.assertFalse(payload['cleanupWaitSatisfied'])
            self.assertFalse(result.passed)
            self.assertIsNone(result.tests, 'Partial logs cannot establish a full test count')
            self.assertIsNone(result.exit_status, 'Child/browser exit is not the Wine exit status')
            self.assertEqual(mode == 'timeout', manifest['browser']['timed_out'])
            self.assertEqual(mode == 'exit', manifest['browser']['exited_early'])
            self.assertFalse(json.loads((artifacts / 'manifest.json').read_text())['result']['passed'])

    def test_timeout_preserves_acknowledged_partial_output(self):
        self.run_interrupted_child('timeout')

    def test_early_child_exit_preserves_acknowledged_partial_output(self):
        self.run_interrupted_child('exit')

    def test_terminal_result_survives_a_late_progress_request(self):
        self.run_interrupted_child('late-progress')

    def test_changed_input_invalidates_an_otherwise_complete_result(self):
        self.run_interrupted_child('changed-input')

    def test_missing_final_input_retains_a_failed_manifest(self):
        self.run_interrupted_child('removed-input')


if __name__ == '__main__':
    unittest.main()
