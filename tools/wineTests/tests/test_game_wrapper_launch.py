"""Outer game-wrapper reports, isolated from asset preflight and real graphics."""
from contextlib import ExitStack, redirect_stdout
import hashlib
import io
import json
from pathlib import Path
from types import SimpleNamespace
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import runMechWarriorProbe as mech
import runSdkInputProbe as sdk

URL = 'http://127.0.0.1:18766/st-jit/boxedwine.html?args=cmd%20%26%26%20demo.exe'


class FinishedDriver:
    def __init__(self, ready):
        self.returncode = 0
        self.stdin = io.StringIO()
        self.stdout = io.StringIO(json.dumps(ready) + '\n')
        self.stderr = io.StringIO('synthetic driver stderr\n')

    def poll(self):
        return self.returncode

    def wait(self, timeout):
        return self.returncode


class WrapperLaunchTests(unittest.TestCase):
    def run_wrapper(self, kind, *, launch_error=None, ready=None, capture=None,
                    mutate_config=False, successful=False, performance=False):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        output = root / 'new-parent' / 'run'
        config = dict(output=str(output), inputs=[], launchPath=URL.split(':18766', 1)[1],
                      buildId='synthetic-build-id', performance=performance)
        config_path = root / 'config.json'
        config_bytes = (json.dumps(config, indent=2) + '\r\n').encode('utf-8')
        config_path.write_bytes(config_bytes)
        reference = root / 'reference.bin'
        reference.write_bytes(b'synthetic reference identity; no image comparison in these controls')
        templates = root / 'templates.json'
        templates.write_text('{}')
        args = SimpleNamespace(config=config_path, node=root / 'node.exe', templates=templates,
            native_base=reference, native_blur=reference, desktop_ui_base=None, desktop_ui_blur=None,
            startup_timeout=1)
        ready = {'ready': False, 'url': URL} if ready is None else ready
        process = FinishedDriver(ready)

        def launch(command, **kwargs):
            if launch_error:
                raise launch_error
            if mutate_config:
                config_path.write_text(json.dumps(dict(config, changed=True)))
            if capture is not None:
                output.mkdir(parents=True)
                (output / 'capture.json').write_text(json.dumps(capture))
            if successful:
                (output / 'final-state.json').write_text(json.dumps({'lifecycle': 'BW_GAME_EXIT:0\nBW_GAME_CLEANUP:0\n'}))
                (output / 'chrome.log').write_text('synthetic clean Chrome log\n')
            return process

        def scenario_done(scenario):
            scenario.measurement = {'measurementValid': True} if performance else None

        module = sdk if kind == 'sdk' else mech
        with ExitStack() as stack:
            # Mech's specific game archive/template preflight has separate
            # tests; bypass it here to reach the process boundary without assets.
            if kind == 'mech':
                stack.enter_context(patch.object(mech, 'validate_config'))
                stack.enter_context(patch.object(mech, 'Templates', return_value=SimpleNamespace(pins=[])))
                if successful:
                    stack.enter_context(patch.object(mech.Scenario, 'run', scenario_done))
            popen = stack.enter_context(patch.object(module.subprocess, 'Popen', side_effect=launch))
            stack.enter_context(redirect_stdout(io.StringIO()))
            code = module.run(args)
        name = 'sdk-input-probe.json' if kind == 'sdk' else 'mech-probe.json'
        snapshot = 'sdk-input-config.json' if kind == 'sdk' else 'mech-input-config.json'
        report = json.loads((output / name).read_text())
        self.assertEqual(code, 0 if report['passed'] else 1)
        self.assertEqual(popen.call_count, 1)
        self.assertEqual((output / snapshot).read_bytes(), config_bytes)
        self.assertEqual(report['config']['sha256'], hashlib.sha256(config_bytes).hexdigest())
        self.assertEqual(report['launch_path'], config['launchPath'])
        self.assertEqual(report['command'][0], str(args.node))
        self.assertEqual(report['command'][-1], str(config_path.resolve()))
        self.assertFalse(report['sdk_acceptance'] if kind == 'sdk' else report['game_acceptance'])
        if not successful:
            self.assertFalse(report['passed'])
        if kind == 'mech':
            self.assertEqual(report['performance_requested'], performance)
        return report

    def test_sdk_missing_driver_retains_failed_report(self):
        report = self.run_wrapper('sdk', launch_error=FileNotFoundError('synthetic missing Node'))
        self.assertFalse(report['driver_started'])
        self.assertIsNone(report['driver_exit_code'])
        self.assertIsNone(report['launch_url'])
        self.assertIn('synthetic missing Node', report['problems'])

    def test_mech_missing_driver_retains_failed_report(self):
        report = self.run_wrapper('mech', launch_error=FileNotFoundError('synthetic missing Node'), performance=True)
        self.assertFalse(report['driver_started'])
        self.assertIsNone(report['driver_exit_code'])
        self.assertIsNone(report['launch_url'])
        self.assertIsNone(report['performance'])
        self.assertIn('synthetic missing Node', report['problems'])

    def test_ready_message_url_survives_failed_startup(self):
        for kind in ('sdk', 'mech'):
            with self.subTest(kind=kind):
                report = self.run_wrapper(kind)
                self.assertTrue(report['driver_started'])
                self.assertEqual(report['launch_url'], URL)
                self.assertEqual(report['driver_exit_code'], 0)

    def test_partial_capture_supplies_actual_url(self):
        for kind in ('sdk', 'mech'):
            with self.subTest(kind=kind):
                report = self.run_wrapper(kind, ready={'ready': False},
                                          capture={'launchUrl': URL, 'errors': ['startup failed']})
                self.assertEqual(report['launch_url'], URL)

    def test_malformed_capture_retains_failed_report(self):
        for kind in ('sdk', 'mech'):
            with self.subTest(kind=kind):
                report = self.run_wrapper(kind, capture=[])
                self.assertIn('Malformed capture metadata', report['problems'])

    def test_sdk_malformed_ready_retains_failed_report(self):
        report = self.run_wrapper('sdk', ready=[])
        self.assertIn('Malformed capture driver response', report['problems'])

    def test_changed_config_retains_original_bytes_and_fails(self):
        for kind in ('sdk', 'mech'):
            with self.subTest(kind=kind):
                report = self.run_wrapper(kind, mutate_config=True)
                self.assertTrue(any('changed' in problem.lower() for problem in report['problems']))

    def test_complete_mech_lifecycle_keeps_requested_measurement_gate(self):
        capture = dict(launchUrl=URL, errors=[], applicationExitObserved=True,
                       cleanupObserved=True, cleanupWaitSatisfied=True, cleanupObservationMilliseconds=15000)
        for performance in (False, True):
            with self.subTest(performance=performance):
                report = self.run_wrapper('mech', ready={'ready': True, 'url': URL},
                    capture=capture, successful=True, performance=performance)
                self.assertTrue(report['passed'])
                self.assertTrue(report['input_lifecycle_passed'])
                self.assertEqual(report['performance'], {'measurementValid': True} if performance else None)


if __name__ == '__main__':
    unittest.main()
