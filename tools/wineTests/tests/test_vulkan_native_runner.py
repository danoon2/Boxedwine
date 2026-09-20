"""Guard against false passes in native Vulkan/D3D comparison runs."""

from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import wineGraphicsNative as native


class NativeGraphicsTests(unittest.TestCase):
    output = ('0024:trace:loaddll:build_module Loaded L"winevulkan.dll" at 12340000: builtin\n'
              '0024:vulkan: 24 tests executed (0 marked as todo, 0 failures), 2 skipped.\n'
              'BOXEDWINE_TEST_EXIT:0\nBOXEDWINE_WINESERVER_CLEANUP_OK\n')

    def assess(self, output=None, **process):
        return native.assess(self.output if output is None else output, 'vulkan', 'vulkan',
                             dict(exit_code=0, timed_out=False) | process)

    def test_complete_run_and_recorder_exit(self):
        self.assertTrue(self.assess()['passed'])
        self.assertFalse(self.assess(exit_code=1)['passed'])
        self.assertTrue(self.assess(exit_code=1, expected_exit_codes=[0, 1])['passed'])

    def test_truncation_timeout_and_mismatched_exit_fail(self):
        for output in (self.output.split('0024:vulkan:')[0],
                       self.output.replace('BOXEDWINE_TEST_EXIT:0', 'BOXEDWINE_TEST_EXIT:5'),
                       self.output + 'BOXEDWINE_TEST_EXIT:0\n',
                       self.output.replace('BOXEDWINE_WINESERVER_CLEANUP_OK', '')):
            self.assertFalse(self.assess(output)['passed'])
        self.assertFalse(self.assess(timed_out=True)['passed'])

    def test_command_echo_does_not_prove_backend(self):
        output = self.output.replace(self.output.splitlines()[0],
                                     'Command: -env WINEDEBUG=+loaddll -env WINEDLLOVERRIDES=winevulkan=b')
        self.assertFalse(self.assess(output)['backend']['verified'])

    def test_dxvk_fallback_rejected(self):
        for output in ('info: DXVK: v3.1.1\n',
                       'info: DXVK: v3.1.1\n0024:trace:loaddll:build_module Loaded L"d3d9.dll": builtin\n'):
            self.assertFalse(native.backend_evidence(output, 'dxvk')['verified'])
        output += '0024:trace:loaddll:build_module Loaded L"d3d9.dll": native\n'
        self.assertTrue(native.backend_evidence(output, 'dxvk')['verified'])

    def test_reference_cannot_accept_broken_infrastructure_or_new_failures(self):
        reference = self.assess()
        self.assertTrue(native.assess(self.output, 'vulkan', 'vulkan',
                                     dict(exit_code=0, timed_out=False), reference)['passed'])
        reference['infrastructure_ok'] = False
        self.assertFalse(native.assess(self.output, 'vulkan', 'vulkan',
                                      dict(exit_code=0, timed_out=False), reference)['passed'])
        reference = self.assess()
        output = self.output.replace('0 failures', '1 failures').replace('EXIT:0', 'EXIT:1')
        self.assertFalse(native.assess(output, 'vulkan', 'vulkan',
                                      dict(exit_code=0, timed_out=False), reference)['passed'])

    def test_timeout_retains_child_output(self):
        with tempfile.TemporaryDirectory() as temp:
            log = Path(temp) / 'child.log'
            result = native.run_process([sys.executable, '-u', '-c',
                                         'import time; print("started"); time.sleep(20)'],
                                        temp, None, 0.3, log)
            self.assertTrue(result['timed_out'])
            self.assertLess(result['seconds'], 10)
            self.assertIn('started', log.read_text())

    def test_host_validation_error_fails_a_passing_guest(self):
        output = self.output + '\nVulkan validation ERROR: a driver-side problem\n'
        self.assertFalse(self.assess(output)['passed'])
        self.assertEqual(len(self.assess(output)['validation_errors']), 1)


if __name__ == '__main__':
    unittest.main()
