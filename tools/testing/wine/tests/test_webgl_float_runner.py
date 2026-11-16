"""Exercise the standalone CLI's failure reports without Chrome or a GPU."""
from contextlib import ExitStack, redirect_stdout
import copy
import io
import json
from pathlib import Path
import runpy
import sys
import tempfile
import threading
import unittest
from unittest.mock import patch

WORK = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(WORK))
import wineGraphicsBrowser as browser

SCRIPT = WORK / 'runWebGLFloatSamples.py'
GOOD = json.loads(Path(__file__).with_name('webgl_float_results.json').read_text())['samples']['result']


class FakeServer:
    instances = []

    def __init__(self, address, handler):
        self.handler = handler
        self.server_port = 18765
        self.stop = threading.Event()
        self.closed = False
        self.instances.append(self)

    def serve_forever(self):
        self.stop.wait()

    def shutdown(self):
        self.stop.set()

    def server_close(self):
        self.closed = True


class FakeProcess:
    def __init__(self, returncode=None):
        self.returncode = returncode

    def poll(self):
        return self.returncode


class FloatRunnerTests(unittest.TestCase):
    script = SCRIPT
    page_path = WORK / 'tests/webgl_float_samples.html'
    good_payload = GOOD

    def run_cli(self, *, payload=None, launch_error=None, discovery_error=None,
                cleanup_error=None, early_exit=False, timeout=False,
                log=b'Chrome synthetic control\n', changed_input=False, missing_input=False,
                extra_args=()):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        output = Path(temporary.name) / 'run'
        FakeServer.instances = []
        process = FakeProcess(0 if early_exit else None)
        if payload is None:
            payload = self.good_payload

        def launch(command, **kwargs):
            if launch_error:
                raise launch_error
            kwargs['stdout'].write(log)
            kwargs['stdout'].flush()
            if not early_exit and not timeout:
                scope = FakeServer.instances[-1].handler.do_POST.__globals__
                scope['payload'].update(copy.deepcopy(payload))
                scope['done'].set()
            return process

        def terminate(actual):
            self.assertIs(actual, process)
            process.returncode = 0
            if cleanup_error:
                raise cleanup_error

        page_path = self.page_path
        read_bytes = Path.read_bytes
        page_reads = 0

        def read_input(path):
            nonlocal page_reads
            if path == page_path:
                page_reads += 1
                if page_reads > 1 and missing_input:
                    raise FileNotFoundError('synthetic input disappearance')
                if page_reads > 1 and changed_input:
                    return read_bytes(path) + b'changed'
            return read_bytes(path)

        with ExitStack() as stack:
            stack.enter_context(patch.object(sys, 'argv', [str(self.script), '--output', str(output), *extra_args]))
            stack.enter_context(patch('http.server.ThreadingHTTPServer', FakeServer))
            stack.enter_context(patch.object(browser, 'find_chrome', side_effect=discovery_error,
                                             return_value=Path('synthetic-chrome.exe')))
            popen = stack.enter_context(patch('subprocess.Popen', side_effect=launch))
            cleanup = stack.enter_context(patch.object(browser, '_terminate_process_tree', side_effect=terminate))
            stack.enter_context(patch.object(Path, 'read_bytes', read_input))
            if timeout:
                stack.enter_context(patch('time.monotonic', side_effect=[0, 61, 62]))
            stack.enter_context(redirect_stdout(io.StringIO()))
            with self.assertRaises(SystemExit) as exit_status:
                runpy.run_path(str(self.script), run_name='__main__')
        report = json.loads((output / 'report.json').read_text(encoding='utf-8'))
        self.assertEqual(exit_status.exception.code, 0 if report['passed'] else 1)
        self.assertTrue((output / 'executed-page.html').is_file())
        self.assertTrue((output / 'server.log').is_file())
        self.assertTrue(FakeServer.instances[0].closed)
        self.assertTrue(FakeServer.instances[0].stop.is_set())
        self.assertTrue(report['launch_url'].startswith('http://127.0.0.1:18765/?token='))
        if not discovery_error:
            self.assertEqual(report['command'][-1], report['launch_url'])
        self.assertEqual(cleanup.call_count, 0 if launch_error or discovery_error else 1)
        self.assertEqual(popen.call_count, 0 if discovery_error else 1)
        return report

    def test_launch_failure_retains_url_inputs_and_error(self):
        report = self.run_cli(launch_error=OSError('synthetic launch failure'))
        self.assertFalse(report['passed'])
        self.assertFalse(report['complete'])
        self.assertFalse(report['browser_started'])
        self.assertTrue(report['inputs_unchanged'])
        self.assertEqual(report['problems'], ['OSError: synthetic launch failure'])
        self.assertEqual(len(report['page_sha256']), 64)
        self.assertEqual(len(report['runner_sha256']), 64)

    def test_discovery_failure_retains_report_and_stops_server(self):
        report = self.run_cli(discovery_error=FileNotFoundError('synthetic missing Chrome'))
        self.assertFalse(report['passed'])
        self.assertFalse(report['browser_started'])
        self.assertEqual(report['command'], [])
        self.assertEqual(report['problems'], ['FileNotFoundError: synthetic missing Chrome'])

    def test_complete_synthetic_result_passes(self):
        report = self.run_cli()
        self.assertTrue(report['passed'])
        self.assertTrue(report['complete'])
        self.assertTrue(report['browser_started'])
        self.assertEqual(report['problems'], [])

    def test_timeout_is_recorded(self):
        report = self.run_cli(timeout=True)
        self.assertFalse(report['passed'])
        self.assertFalse(report['complete'])
        self.assertTrue(report['timed_out'])
        self.assertFalse(report['exited_early'])

    def test_early_exit_is_recorded(self):
        report = self.run_cli(early_exit=True)
        self.assertFalse(report['passed'])
        self.assertFalse(report['complete'])
        self.assertTrue(report['exited_early'])
        self.assertFalse(report['timed_out'])

    def test_cleanup_failure_cannot_pass(self):
        report = self.run_cli(cleanup_error=OSError('synthetic cleanup failure'))
        self.assertTrue(report['complete'])
        self.assertFalse(report['passed'])
        self.assertEqual(report['problems'], ['Browser cleanup: OSError: synthetic cleanup failure'])

    def test_changed_input_cannot_pass(self):
        report = self.run_cli(changed_input=True)
        self.assertFalse(report['passed'])
        self.assertFalse(report['inputs_unchanged'])

    def test_missing_input_still_retains_report(self):
        report = self.run_cli(missing_input=True)
        self.assertFalse(report['passed'])
        self.assertFalse(report['inputs_unchanged'])
        self.assertIn('synthetic input disappearance', report['problems'][0])

    def test_empty_or_warning_log_cannot_pass(self):
        for log in (b'', b'WebGL: INVALID_OPERATION: synthetic diagnostic\n'):
            with self.subTest(log=log):
                self.assertFalse(self.run_cli(log=log)['passed'])

    def test_malformed_or_failed_result_cannot_pass(self):
        for field, value in (('passed', 'true'), ('passed', False),
                             ('observationSeconds', '15'), ('observationSeconds', 14.99),
                             ('observationSeconds', float('inf')), ('observationSeconds', float('nan')),
                             ('checks', []), ('checks', None), ('checks', [{}]),
                             ('checks', [{'passed': False}]), ('checks', [True])):
            with self.subTest(field=field, value=value):
                payload = copy.deepcopy(self.good_payload)
                payload[field] = value
                self.assertFalse(self.run_cli(payload=payload)['passed'])


if __name__ == '__main__':
    unittest.main()
