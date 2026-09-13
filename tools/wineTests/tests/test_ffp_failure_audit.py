import contextlib
import io
import json
from pathlib import Path
import re
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from auditFFPFailure import audit_ffp_failure
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics
FIXTURE = json.loads(Path(__file__).with_name('ffp_failure_results.json').read_text(encoding='utf-8'))

class FFPFailureAuditTests(unittest.TestCase):
    def test_native_normal_and_all_injected_paths(self):
        for name in ('windows', 'production', 'compile', 'early', 'late'):
            with self.subTest(name=name):
                result = audit_ffp_failure(FIXTURE[name]['log'], name not in ('windows','production'))
                self.assertTrue(result['passed'], result)
                self.assertEqual((result['pixels'],result['components'],result['max_channel_delta']), (135,540,0))

    def test_real_crash_and_forged_summary_rejected(self):
        log = FIXTURE['old_crash']['log']
        self.assertFalse(audit_ffp_failure(log, True)['passed'])
        self.assertFalse(audit_ffp_failure(log + '\nFFP_FAILURE_COVERAGE completed=27\n'
            '0000:ffpfailure: 907 tests executed (0 marked as todo, 0 failures), 0 skipped.\n', True)['passed'])

    def test_wrong_oracle_mode_rejected(self):
        self.assertFalse(audit_ffp_failure(FIXTURE['production']['log'],True)['passed'])
        self.assertFalse(audit_ffp_failure(FIXTURE['compile']['log'],False)['passed'])

    def test_forged_expected_and_actual_pixels_rejected(self):
        log = FIXTURE['compile']['log']
        for replacement in ('color=ff8040c0 expected=ff8040c0', 'color=ff123458 expected=ff123456',
                'color=00123456 expected=ff123456', 'color=garbage expected=garbage'):
            mutant = log.replace('color=ff123456 expected=ff123456',replacement,1)
            self.assertNotEqual(mutant,log)
            self.assertFalse(audit_ffp_failure(mutant,True)['passed'])

    def test_coverage_and_device_release_rejected(self):
        log = FIXTURE['compile']['log']
        for prefix in ('FFP_FAILURE_CASE ', 'FFP_FAILURE_PIXEL ', 'FFP_FAILURE_RELEASE '):
            row = next(row for row in log.splitlines(True) if row.startswith(prefix))
            for mutant in (log.replace(row,'',1),log.replace(row,row+row,1)):
                self.assertFalse(audit_ffp_failure(mutant,True)['passed'])
        for a,b in [('refs=0','refs=1'),('907 tests','906 tests'),('completed=27','completed=26'),('index=0','index=9')]:
            self.assertFalse(audit_ffp_failure(log.replace(a,b,1),True)['passed'])

    def test_duplicate_complete_blocks_must_agree(self):
        log = FIXTURE['compile']['log']
        self.assertTrue(audit_ffp_failure(log+log,True)['passed'])
        self.assertFalse(audit_ffp_failure(log+FIXTURE['early']['log'],True)['passed'])

    def test_cli_enforces_pixels_fault_mode_and_cleanup(self):
        for name,injection,expected in [('production','none',0),('compile','compile',0),
                ('early','early',0),('late','late',0),('production','compile',1),('old_crash','compile',1)]:
            with self.subTest(name=name,injection=injection), tempfile.TemporaryDirectory() as folder:
                p=Path(folder); log=p/'wine.log'; log.write_text(FIXTURE[name]['log'],encoding='utf-8')
                filesystem=p/'root.zip';filesystem.touch()
                manifest=dict(artifacts=dict(wine_log=str(log)),result=dict(passed=True,reason='ok'))
                result=graphics.GraphicsTestResult('ffp-failure','ffpfailure',907,0,0,0,True,'ok',(),(),0)
                argv=['probe','--probe','ffp-failure','--ffp-injection',injection,'--executable',str(p/'probe.exe'),
                    '--filesystem',str(filesystem),'--build-dir',str(p),'--output',str(p)]
                with patch.object(sys,'argv',argv),patch.object(graphics,'validate_web_build'), \
                        patch.object(graphics,'validate_test_executable'),patch.object(graphics,'find_chrome'), \
                        patch.object(graphics,'run_browser_test',return_value=(result,manifest)) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(),expected)
                suite=run.call_args.kwargs['suite']
                self.assertEqual(suite.cleanup_wait_seconds,15)
                self.assertEqual(suite.group_arguments,() if injection=='none' else ('--fault',))
                self.assertEqual(suite.environment,('BW_TEST_FFP_FAILURE='+injection,))
                self.assertEqual(json.loads((p/'manifest.json').read_text())['ffp_failure_audit']['passed'],expected==0)

if __name__=='__main__': unittest.main()
