"""EGL runner regressions with real parsing/packaging and no browser or GPU."""
from contextlib import redirect_stderr, redirect_stdout
from dataclasses import asdict
import importlib.util
import io
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
import unittest
from unittest.mock import patch
import zipfile

SCRIPT = Path(__file__).resolve().parents[2] / 'openglTest/runEGLBrowserTest.py'
SPEC = importlib.util.spec_from_file_location('egl_browser_cleanup_runner', SCRIPT)
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class EGLBrowserCleanupTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.output = (runner.PASS_MARKER + '\nBOXEDWINE_EGL_PROBE_STATUS:0\n'
            'Summary: 1 passed, 0 failed, 0 skipped\n' + runner.RETURN_MARKER + '\n')

    def evidence(self, *, output=None, observed=True, events=None, timed_out=False,
                 early=False, stderr='Chrome started\n', kind='complete'):
        payload = dict(output=self.output if output is None else output,
            kind=kind, cleanupWaitSatisfied=observed, browserEvents=[] if events is None else events)
        result = runner.browser.parse_graphics_result(runner.SUITE, runner.SUITE.groups[0], payload,
            timed_out=timed_out, browser_exited_early=early)
        (self.root / 'payload.json').write_text(json.dumps(payload), encoding='utf-8')
        (self.root / 'chrome.log').write_text(stderr, encoding='utf-8')
        manifest = dict(finished_at='2026-09-09T00:00:00Z', cleanup_wait_seconds=15,
            cleanup_marker=runner.RETURN_MARKER, result=asdict(result),
            browser=dict(timed_out=timed_out, exited_early=early),
            artifacts=dict(browser_payload=str(self.root / 'payload.json'), chrome_log=str(self.root / 'chrome.log')))
        return result, manifest

    def audit(self, **kwargs):
        return runner.audit_result(*self.evidence(**kwargs))

    def elf(self, path):
        header = bytearray(20)
        header[:6] = b'\x7fELF\x01\x01'
        struct.pack_into('<H', header, 18, 3)
        path.write_bytes(header)
        return path

    def test_success_requires_probe_exit_and_full_observation(self):
        audit = self.audit()
        self.assertTrue(audit['passed'], audit)
        self.assertFalse(audit['launches_wine'])
        self.assertEqual(audit['exit_observation_seconds'], 15)

    def test_old_pass_only_and_embedded_command_cannot_pass(self):
        for output in [runner.PASS_MARKER, 'command: echo ' + self.output.replace('\n', '; '),
                       self.output.replace(runner.RETURN_MARKER, 'echo ' + runner.RETURN_MARKER)]:
            with self.subTest(output=output):
                self.assertFalse(self.audit(output=output)['passed'])

    def test_successful_exit_without_exact_probe_result_fails(self):
        self.assertFalse(self.audit(output=self.output.replace(runner.PASS_MARKER, 'PASS unrelated'))['passed'])

    def test_nonzero_missing_or_conflicting_exit_status_fails(self):
        for output in [self.output.replace('PROBE_STATUS:0', 'PROBE_STATUS:5'),
                       self.output.replace('BOXEDWINE_EGL_PROBE_STATUS:0\n', ''),
                       self.output + 'BOXEDWINE_EGL_PROBE_STATUS:1\n']:
            with self.subTest(output=output):
                self.assertFalse(self.audit(output=output)['passed'])

    def test_late_failure_in_guest_output_fails(self):
        self.assertFalse(self.audit(output=self.output + 'FAIL delayed resource cleanup\n')['passed'])

    def test_unfinished_observation_timeout_and_early_exit_fail(self):
        for kwargs in [dict(observed=False), dict(timed_out=True), dict(early=True), dict(kind='progress')]:
            with self.subTest(kwargs=kwargs):
                self.assertFalse(self.audit(**kwargs)['passed'])

    def test_late_browser_exception_fails(self):
        self.assertFalse(self.audit(events=[dict(kind='error', message='late worker trap')])['passed'])

    def test_full_stderr_warning_outside_console_tail_fails(self):
        log = '[123:456:0909/120000.000:INFO:CONSOLE:30] "WebGL: INVALID_OPERATION: bad draw", source: x (30)\n'
        audit = self.audit(stderr=log + 'ordinary output\n' * 2000)
        self.assertFalse(audit['passed'])
        self.assertEqual(len(audit['diagnostics']), 1)
        self.assertEqual(audit['diagnostics'][0]['line'], 1)
        self.assertFalse(self.audit(stderr='')['passed'])

    def test_wrong_cleanup_manifest_fails(self):
        for key, value in [('cleanup_wait_seconds', 0), ('cleanup_marker', 'other'), ('finished_at', None)]:
            result, manifest = self.evidence()
            manifest[key] = value
            self.assertFalse(runner.audit_result(result, manifest)['passed'])

    def test_installed_and_bundled_library_packages_are_distinct(self):
        executable = self.elf(self.root / 'probe')
        libraries = []
        for name in runner.LIBRARIES:
            library = self.root / name
            library.write_bytes(name.encode())
            libraries.append(library)
        for name, files in [('installed.zip', []), ('bundled.zip', libraries)]:
            target = runner.create_app(executable, runner.SUITE, self.root / name, files)
            with zipfile.ZipFile(target) as package:
                self.assertEqual(package.namelist(), [runner.SUITE.executable] + ['lib/' + file.name for file in files])
                self.assertEqual(package.read(runner.SUITE.executable), executable.read_bytes())
            with self.assertRaises(FileExistsError):
                runner.create_app(executable, runner.SUITE, target, files)

    def test_wrong_architecture_is_rejected(self):
        executable = self.root / 'wrong'
        for image in [b'MZ' + bytes(100), b'\x7fELF\x02\x01' + bytes(14),
                      b'\x7fELF\x01\x02' + bytes(14), b'\x7fELF\x01\x01' + bytes(14)]:
            executable.write_bytes(image)
            with self.assertRaises(runner.browser.RunnerError):
                runner.validate_elf(executable)

    def test_cli_uses_common_backend_and_restores_adapters(self):
        for filename in runner.browser.REQUIRED_WEB_FILES:
            (self.root / filename).write_bytes(b'build fixture')
        filesystem = self.root / 'root.zip'
        filesystem.write_bytes(b'root fixture')
        executable = self.elf(self.root / 'probe')
        original = (runner.browser.validate_test_executable, runner.browser.create_test_app_zip,
                    runner.browser.build_guest_test_command)
        calls = []

        def fake_browser(**kwargs):
            calls.append(kwargs)
            kwargs['run_dir'].mkdir(parents=True)
            package_path = kwargs['run_dir'] / 'app.zip'
            runner.browser.create_test_app_zip(kwargs['test_executable'], kwargs['suite'], package_path)
            with zipfile.ZipFile(package_path) as package:
                self.assertEqual(package.namelist(), [runner.SUITE.executable])
            command = runner.browser.build_guest_test_command(kwargs['suite'], kwargs['group'])
            self.assertNotIn('/bin/wine', command)
            self.assertIn('unset LD_LIBRARY_PATH;', command)
            return self.evidence()

        with patch.object(runner.browser, 'run_browser_test', side_effect=fake_browser), \
             patch.object(runner.browser, 'find_chrome', return_value=self.root / 'chrome.exe'), \
             redirect_stdout(io.StringIO()):
            code = runner.main(['--filesystem', str(filesystem), '--build-dir', str(self.root),
                '--executable', str(executable), '--use-filesystem-libraries', '--build-mode', 'mt-jit',
                '--build-commit', 'a' * 40, '--build-source-dirty', 'false',
                '--output-dir', str(self.root / 'runs')])
        self.assertEqual(code, 0)
        self.assertEqual(calls[0]['mode'], 'multi-threaded-jit')
        self.assertEqual(calls[0]['build_commit'], 'a' * 40)
        self.assertIs(calls[0]['build_source_dirty'], False)
        self.assertEqual(calls[0]['suite'].cleanup_wait_seconds, 15)
        self.assertTrue(calls[0]['keep_browser_profile'])
        self.assertEqual((runner.browser.validate_test_executable, runner.browser.create_test_app_zip,
                         runner.browser.build_guest_test_command), original)

    def test_invalid_options_are_rejected(self):
        for options in [['--timeout', '15'], ['--timeout', '0'],
                        ['--use-filesystem-libraries', '--library-dir', str(self.root)]]:
            with self.subTest(options=options), redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as raised:
                    runner.main(options)
                self.assertEqual(raised.exception.code, 2)

    @unittest.skipUnless(os.name == 'posix', 'execute the guest shell wrapper on Linux/WSL')
    def test_shell_preserves_failure_after_printing_pass(self):
        executable = self.root / 'fake probe'
        log = self.root / 'guest log'
        for status in (0, 7):
            executable.write_text('#!/bin/sh\nprintf \'%s\\n\' ' + shlex.quote(runner.PASS_MARKER)
                                  + '\nexit ' + str(status) + '\n')
            executable.chmod(0o700)
            command = runner.guest_command(True).replace(
                shlex.quote('/home/username/.wine/dosdevices/c:/files/' + runner.SUITE.executable),
                shlex.quote(str(executable))).replace('/tmp/boxedwine-graphics-test.log', shlex.quote(str(log)))
            completed = subprocess.run(['/bin/sh', '-c', command], capture_output=True, text=True)
            self.assertEqual(completed.returncode, status)
            self.assertIn('BOXEDWINE_EGL_PROBE_STATUS:' + str(status), log.read_text())
            self.assertEqual(self.audit(output=log.read_text())['passed'], status == 0)


if __name__ == '__main__':
    unittest.main()
