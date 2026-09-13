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
from auditD3D9Texgen import audit_texgen
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics

FIXTURE = json.loads(Path(__file__).with_name('d3d9_texgen_results.json').read_text(encoding='utf-8'))


class TexgenAuditTests(unittest.TestCase):
    def test_real_windows_and_candidate_vectors(self):
        for label in ('windows', 'candidate_wine'):
            with self.subTest(label=label):
                result = audit_texgen(FIXTURE[label]['log'])
                self.assertTrue(result['passed'], result)
                self.assertEqual((result['cases'], result['pixels'], result['components']), (96, 96, 384))

    def test_old_wine_and_browser_crash_rejected(self):
        for label in ('original_wine', 'original_browser'):
            with self.subTest(label=label):
                log = FIXTURE[label]['log']
                self.assertFalse(audit_texgen(log)['passed'])
                forged = '\n'.join(row for row in log.splitlines() if 'Test failed:' not in row)
                forged = forged.replace('121 failures', '0 failures').replace('completed=72', 'completed=96')
                self.assertFalse(audit_texgen(forged)['passed'])

    def test_wrong_nonfinite_and_forged_vectors_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        for value in ('0,0,0,0', 'nan,0,0,1', 'inf,0,0,1', 'bad,0,0,1', '0,0,1'):
            mutant, count = re.subn(r'(^TCI_VALUE .*?actual=)\S+ expected=\S+',
                lambda match: match[1] + value + ' expected=' + value, log, count=1, flags=re.M)
            with self.subTest(value=value):
                self.assertEqual(count, 1)
                self.assertNotEqual(mutant, log)
                self.assertFalse(audit_texgen(mutant)['passed'])

    def test_missing_duplicate_position_and_summary_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        for prefix in ('TCI_CASE ', 'TCI_VALUE '):
            row = next(row for row in log.splitlines(True) if row.startswith(prefix))
            for mutant in (log.replace(row, '', 1), log.replace(row, row + row, 1)):
                self.assertFalse(audit_texgen(mutant)['passed'])
        for mutant in ('', log.replace('x=8 y=8', 'x=0 y=8', 1), log.replace('2236 tests', '2235 tests'),
                       log.replace('completed=96', 'completed=95'), log.replace('0 failures', '1 failures')):
            self.assertNotEqual(mutant, log)
            self.assertFalse(audit_texgen(mutant)['passed'])

    def test_duplicate_complete_output_must_agree(self):
        log = FIXTURE['candidate_wine']['log']
        self.assertTrue(audit_texgen(log + log)['passed'])
        self.assertFalse(audit_texgen(log + FIXTURE['original_wine']['log'])['passed'])

    def test_cli_enforces_vectors_and_cleanup_default(self):
        for label, expected in [('candidate_wine', 0), ('original_wine', 1), ('original_browser', 1)]:
            with self.subTest(label=label), tempfile.TemporaryDirectory() as folder:
                base = Path(folder)
                log = base / 'wine.log'
                log.write_text(FIXTURE[label]['log'], encoding='utf-8')
                filesystem = base / 'filesystem.zip'
                filesystem.touch()
                manifest = dict(artifacts=dict(wine_log=str(log)), result=dict(passed=True, reason='ok'))
                result = graphics.GraphicsTestResult('d3d9-texgen', 'texgen', 2236, 0, 0, 0,
                    True, 'ok', (), (), 0)
                args = ['probe', '--probe', 'texgen', '--executable', str(base / 'probe.exe'),
                    '--filesystem', str(filesystem), '--build-dir', str(base), '--output', str(base)]
                with patch.object(sys, 'argv', args), patch.object(graphics, 'validate_web_build'), \
                        patch.object(graphics, 'validate_test_executable'), patch.object(graphics, 'find_chrome'), \
                        patch.object(graphics, 'run_browser_test', return_value=(result, manifest)) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(), expected)
                self.assertEqual(run.call_args.kwargs['suite'].cleanup_wait_seconds, 15)
                self.assertEqual(run.call_args.kwargs['group'], 'texgen')
                stored = json.loads((base / 'manifest.json').read_text())
                self.assertEqual(stored['result']['passed'], expected == 0)
                self.assertEqual(stored['texgen_audit']['passed'], expected == 0)


if __name__ == '__main__':
    unittest.main()
