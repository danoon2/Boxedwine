"""Check performance-control CLI reports with fake browser/server boundaries."""
from contextlib import ExitStack, redirect_stdout
import copy
import inspect
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import test_webgl_float_runner as controls
import runGamePerformanceControl as runner

GOOD = dict(kind='game-performance-browser-control', passed=True, errors=[], checks=[
    dict(name=name, passed=True) for name in ('module_uninstalled', 'baseline_pixels', 'native_exception',
        'drawImage', 'putImageData', 'transferFromImageBitmap', 'hidden_parent', 'overhead_counts',
        'restored', 'gl_error')])


class PerformanceRunnerTests(unittest.TestCase):
    def run_cli(self, *, payload=GOOD, discovery_error=None, launch_error=None,
                cleanup_error=None, timeout=False, early_exit=False, observation_exit=False,
                source_change=False, source_missing=False, log=b'Chrome synthetic control\n'):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        output = Path(temporary.name) / 'run'
        controls.FakeServer.instances = []
        process = controls.FakeProcess(0 if early_exit else None)
        launched = False

        def launch(command, **kwargs):
            nonlocal launched
            if launch_error:
                raise launch_error
            launched = True
            kwargs['stdout'].write(log)
            kwargs['stdout'].flush()
            if not timeout and not early_exit:
                scope = inspect.getclosurevars(controls.FakeServer.instances[-1].handler.do_POST).nonlocals
                scope['payload'].update(copy.deepcopy(payload))
                scope['done'].set()
            return process

        def terminate(actual):
            self.assertIs(actual, process)
            process.returncode = 0
            if cleanup_error:
                raise cleanup_error

        def observe(seconds):
            self.assertEqual(seconds, 2)
            if observation_exit:
                process.returncode = 0

        original_open = Path.open
        source = controls.WORK / 'gamePerformance.mjs'

        def open_input(path, mode='r', *args, **kwargs):
            if path == source and launched and mode == 'rb':
                if source_missing:
                    raise FileNotFoundError('synthetic source disappearance')
                if source_change:
                    return io.BytesIO(b'synthetic changed source')
            return original_open(path, mode, *args, **kwargs)

        with ExitStack() as stack:
            stack.enter_context(patch.object(sys, 'argv', [runner.__file__, '--output', str(output)]))
            stack.enter_context(patch.object(runner, 'ThreadingHTTPServer', controls.FakeServer))
            stack.enter_context(patch.object(runner.browser, 'find_chrome', side_effect=discovery_error,
                return_value=Path('synthetic-chrome.exe')))
            popen = stack.enter_context(patch.object(runner.subprocess, 'Popen', side_effect=launch))
            cleanup = stack.enter_context(patch.object(runner.browser, '_terminate_process_tree', side_effect=terminate))
            stack.enter_context(patch.object(runner.time, 'sleep', side_effect=observe))
            stack.enter_context(patch.object(Path, 'open', open_input))
            if timeout:
                stack.enter_context(patch.object(runner.time, 'monotonic', side_effect=[0, 91, 92]))
            stack.enter_context(redirect_stdout(io.StringIO()))
            status = runner.main()
        report = json.loads((output / 'report.json').read_text())
        self.assertEqual(status, 0 if report['passed'] else 1)
        self.assertFalse(report['game_performance_acceptance'])
        self.assertTrue((output / 'index.html').is_file())
        self.assertTrue((output / 'gamePerformance.mjs').is_file())
        self.assertTrue((output / 'game-performance-control.mjs').is_file())
        self.assertTrue((output / 'server.log').is_file())
        self.assertEqual(len(report['sources']), 3)
        self.assertTrue(report['launch_url'].startswith('http://127.0.0.1:18765/?token='))
        self.assertTrue(controls.FakeServer.instances[0].closed)
        self.assertTrue(controls.FakeServer.instances[0].stop.is_set())
        self.assertEqual(cleanup.call_count, 0 if launch_error or discovery_error else 1)
        self.assertEqual(popen.call_count, 0 if discovery_error else 1)
        if not discovery_error:
            self.assertEqual(report['command'][-1], report['launch_url'])
        return report

    def test_discovery_failure_retains_report(self):
        report = self.run_cli(discovery_error=FileNotFoundError('synthetic missing Chrome'))
        self.assertFalse(report['passed'])
        self.assertFalse(report['browser_started'])
        self.assertEqual(report['command'], [])
        self.assertIn('FileNotFoundError: synthetic missing Chrome', report['problems'])

    def test_launch_failure_retains_url_and_sources(self):
        report = self.run_cli(launch_error=OSError('synthetic launch failure'))
        self.assertFalse(report['passed'])
        self.assertFalse(report['browser_started'])
        self.assertIn('OSError: synthetic launch failure', report['problems'])

    def test_malformed_check_retains_report(self):
        payload = copy.deepcopy(GOOD)
        payload['checks'][0]['name'] = []
        report = self.run_cli(payload=payload)
        self.assertFalse(report['passed'])
        self.assertIn('Missing or failed browser controls', report['problems'])

    def test_complete_payload_passes(self):
        report = self.run_cli()
        self.assertTrue(report['passed'])
        self.assertTrue(report['complete'])
        self.assertEqual(report['result'], GOOD)
        self.assertEqual(report['problems'], [])

    def test_timeout_is_recorded(self):
        report = self.run_cli(timeout=True)
        self.assertFalse(report['passed'])
        self.assertTrue(report['timed_out'])
        self.assertFalse(report['exited_early'])

    def test_exit_before_payload_cannot_pass(self):
        report = self.run_cli(early_exit=True)
        self.assertFalse(report['passed'])
        self.assertTrue(report['exited_early'])
        self.assertFalse(report['complete'])

    def test_exit_during_observation_cannot_pass(self):
        report = self.run_cli(observation_exit=True)
        self.assertFalse(report['passed'])
        self.assertTrue(report['exited_early'])
        self.assertTrue(report['complete'])

    def test_cleanup_failure_cannot_pass(self):
        report = self.run_cli(cleanup_error=OSError('synthetic cleanup failure'))
        self.assertFalse(report['passed'])
        self.assertIn('Browser cleanup: OSError: synthetic cleanup failure', report['problems'])

    def test_source_change_or_disappearance_cannot_pass(self):
        for flags in ({'source_change': True}, {'source_missing': True}):
            with self.subTest(flags=flags):
                report = self.run_cli(**flags)
                self.assertFalse(report['passed'])
                self.assertTrue(any('source' in problem for problem in report['problems']))

    def test_empty_or_diagnostic_log_cannot_pass(self):
        for log in (b'', b'WebGL: INVALID_OPERATION: synthetic warning\n'):
            with self.subTest(log=log):
                self.assertFalse(self.run_cli(log=log)['passed'])


if __name__ == '__main__':
    unittest.main()
