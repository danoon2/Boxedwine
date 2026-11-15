"""Exercise public CLI failure gates with synthetic envelopes around real probe records.

These are harness controls only; they do not execute a graphics API.
"""
from contextlib import redirect_stdout, redirect_stderr
import io
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import runSampleMaskProbe as runner

FIXTURES = json.loads((Path(__file__).with_name('sample_mask_results.json')).read_text(encoding='utf-8'))


class SampleMaskCli(unittest.TestCase):
    def run_capture(self, *, guest=None, mutate=None, chrome_suffix='', extra_args=()):
        guest = FIXTURES['capability_present'] if guest is None else guest
        with tempfile.TemporaryDirectory(prefix='sample-mask-cli-') as directory:
            output = Path(directory)
            payload = dict(kind='complete', cleanupWaitSatisfied=True, browserEvents=[],
                           redirectedRevision=1,
                           consoleTail=['log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n' + guest],
                           output='CLEANUP\nBOXEDWINE_WINESERVER_KILL_STATUS:0\n'
                                  'BOXEDWINE_WINESERVER_WAIT_STATUS:0\n' + guest)
            manifest = dict(artifacts=dict(browser_payload=str(output / 'browser-payload.json'),
                                           chrome_log=str(output / 'chrome.log')),
                            launch_url='http://127.0.0.1:1/boxedwine.html',
                            cleanup_marker='CLEANUP', cleanup_wait_seconds=15,
                            browser=dict(timed_out=False, exited_early=False),
                            result=dict(exit_status=0), input_identity_verified=True,
                            input_identity_problems=[])
            result = SimpleNamespace(passed=True)
            if mutate:
                mutate(payload, manifest, result)
            chrome = ('MASK_EXTENSION_CONTROL '
                      '{"denied":false,"available":true,"realm":"page"}\n' + chrome_suffix)
            (output / 'browser-payload.json').write_text(json.dumps(payload), encoding='utf-8')
            (output / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
            (output / 'chrome.log').write_text(chrome, encoding='utf-8')
            args = ['runSampleMaskProbe.py', '--kind', 'capabilities', '--executable', str(output / 'probe.exe'),
                    '--filesystem', str(output / 'root.zip'), '--build-dir', str(output / 'runtime'),
                    '--output', str(output), *extra_args]
            previous_prelude = runner.browser._worker_error_observer_script
            with patch.object(sys, 'argv', args), \
                    patch.object(runner.browser, 'find_chrome', return_value=output / 'chrome.exe'), \
                    patch.object(runner.browser, 'run_browser_test', return_value=(result, manifest)) as launch, \
                    redirect_stdout(io.StringIO()):
                status = runner.main()
            self.assertIs(runner.browser._worker_error_observer_script, previous_prelude)
            saved = json.loads((output / 'sample-mask-audit.json').read_text(encoding='utf-8'))
            return status, saved, launch.call_args.kwargs

    def test_valid_capture_and_st_jit_default(self):
        status, saved, arguments = self.run_capture()
        self.assertEqual(status, 0)
        self.assertTrue(saved['passed'])
        self.assertEqual(arguments['mode'], 'single-threaded-jit')

    def test_mt_jit_selection(self):
        status, saved, arguments = self.run_capture(extra_args=('--mode', 'multi-threaded-jit'))
        self.assertEqual(status, 0)
        self.assertEqual(arguments['mode'], 'multi-threaded-jit')

    def test_reject_non_jit(self):
        for mode in ('single-threaded-non-jit', 'multi-threaded-non-jit'):
            with self.subTest(mode=mode), redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as raised:
                self.run_capture(extra_args=('--mode', mode))
            self.assertEqual(raised.exception.code, 2)

    def test_guest_failure(self):
        status, saved, _ = self.run_capture(mutate=lambda p, m, r: m['result'].update(exit_status=1))
        self.assertEqual(status, 1)
        self.assertFalse(saved['passed'])

    def test_browser_failure(self):
        status, _, _ = self.run_capture(mutate=lambda p, m, r: setattr(r, 'passed', False))
        self.assertEqual(status, 1)

    def test_incomplete_lifecycle(self):
        changes = (
            lambda p, m, r: p.update(kind='timeout'),
            lambda p, m, r: p.update(cleanupWaitSatisfied=False),
            lambda p, m, r: p.update(browserEvents=[dict(type='error', message='late crash')]),
            lambda p, m, r: m.update(cleanup_wait_seconds=14),
            lambda p, m, r: m['browser'].update(timed_out=True),
            lambda p, m, r: m['browser'].update(exited_early=True),
            lambda p, m, r: p.update(output=p['output'].replace('CLEANUP\n', '')),
            lambda p, m, r: p.update(output=p['output'].replace('KILL_STATUS:0', 'KILL_STATUS:2')),
            lambda p, m, r: p.update(output=p['output'].replace('WAIT_STATUS:0', 'WAIT_STATUS:1')),
        )
        for index, change in enumerate(changes):
            with self.subTest(index=index):
                status, saved, _ = self.run_capture(mutate=change)
                self.assertEqual(status, 1)
                self.assertFalse(saved['cleanup'])

    def test_input_changed(self):
        status, _, _ = self.run_capture(mutate=lambda p, m, r: m.update(
            input_identity_verified=False, input_identity_problems=['root changed']))
        self.assertEqual(status, 1)

    def test_missing_or_conflicting_extension_observation(self):
        for marker in ('', 'MASK_EXTENSION_CONTROL {"denied":true,"available":false,"realm":"page"}\n'):
            with self.subTest(marker=marker):
                # Adding a conflicting observation must fail even after a good one.
                # The empty case instead removes all observations at read time.
                if marker:
                    status, saved, _ = self.run_capture(chrome_suffix=marker)
                else:
                    original = Path.read_text

                    def without_marker(path, *args, **kwargs):
                        text = original(path, *args, **kwargs)
                        return '' if path.name == 'chrome.log' else text

                    with patch.object(Path, 'read_text', without_marker):
                        status, saved, _ = self.run_capture()
                self.assertEqual(status, 1)
                self.assertFalse(saved['extension_verified'])

    def test_capture_disagreement(self):
        status, saved, _ = self.run_capture(mutate=lambda p, m, r: p.update(output=p['output'] + 'different'))
        self.assertEqual(status, 1)
        self.assertIsNotNone(saved['capture_error'])

    def test_wrong_pixel(self):
        status, saved, _ = self.run_capture(guest=FIXTURES['capability_present'].replace('color=ffffffff', 'color=ffff0000', 1))
        self.assertEqual(status, 1)
        self.assertFalse(saved['audit']['passed'])

    def test_truncated_assertion_count(self):
        guest = FIXTURES['capability_present'].replace('84 tests executed', '83 tests executed')
        self.assertNotEqual(guest, FIXTURES['capability_present'])
        status, saved, _ = self.run_capture(guest=guest)
        self.assertEqual(status, 1)
        self.assertFalse(saved['coverage'])

    def test_shader_and_resource_errors(self):
        for line in ('0024:err:d3d_shader:compile_shader failed\n',
                     '0024:err:d3d:resource_cleanup Leftover resource 123\n'):
            with self.subTest(line=line):
                status, _, _ = self.run_capture(guest=FIXTURES['capability_present'] + line)
                self.assertEqual(status, 1)


if __name__ == '__main__':
    unittest.main()
