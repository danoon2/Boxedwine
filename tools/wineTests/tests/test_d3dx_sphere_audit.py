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
from auditD3DXSphere import audit_sphere
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics

FIXTURE = json.loads(Path(__file__).with_name('d3dx_sphere_results.json').read_text(encoding='utf-8'))


class SphereAuditTests(unittest.TestCase):
    def test_real_windows_and_candidate_observations(self):
        for label in ('windows', 'candidate_wine'):
            with self.subTest(label=label):
                result = audit_sphere(FIXTURE[label]['log'])
                self.assertTrue(result['passed'], result)
                self.assertEqual((result['cases'], result['points'], result['components']), (18, 104, 72))

    def test_real_old_wine_rejected_even_with_passing_summary(self):
        log = FIXTURE['original_wine']['log']
        self.assertFalse(audit_sphere(log)['passed'])
        forged = '\n'.join(row for row in log.splitlines() if 'Test failed:' not in row)
        forged = forged.replace('31 failures', '0 failures')
        self.assertFalse(audit_sphere(forged)['passed'])

    def test_wrong_and_nonfinite_spheres_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        for field, value in [('radius', '1'), ('radius', '-1'), ('radius', 'nan'),
                             ('center', 'inf,0,0'), ('center', 'bad,0,0'), ('hr', '8876086c')]:
            mutant, count = re.subn(r'(^SPHERE_OBSERVATION shear .*?\b' + field + r'=)\S+',
                lambda match: match[1] + value, log, count=1, flags=re.M)
            with self.subTest(field=field, value=value):
                self.assertEqual(count, 1)
                self.assertNotEqual(mutant, log)
                self.assertFalse(audit_sphere(mutant)['passed'])

    def test_changed_input_and_inclusion_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        mutations = [
            log.replace('local=9.99999968e-21,0,0', 'local=0,0,0'),
            log.replace('world=9.99999968e-21,0,0', 'world=0,0,0'),
            log.replace('root-sibling mesh=1 index=0 included=0', 'root-sibling mesh=1 index=0 included=1'),
            log.replace('local=8,0,0', 'local=9,0,0', 1),
            log.replace('world=8,0,0', 'world=nan,0,0', 1),
        ]
        for mutant in mutations:
            self.assertNotEqual(mutant, log)
            self.assertFalse(audit_sphere(mutant)['passed'])

    def test_missing_duplicate_and_failed_coverage_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        for prefix in ('SPHERE_OBSERVATION ', 'SPHERE_INPUT '):
            row = next(row for row in log.splitlines(True) if row.startswith(prefix))
            for mutant in (log.replace(row, '', 1), log.replace(row, row + row, 1)):
                with self.subTest(prefix=prefix):
                    self.assertFalse(audit_sphere(mutant)['passed'])
        for mutant in ('', log.replace('cases=18', 'cases=17'), log.replace('326 tests', '325 tests'),
                       log.replace('0 failures), 0 skipped.', '1 failures), 0 skipped.')):
            self.assertFalse(audit_sphere(mutant)['passed'])

    def test_duplicate_complete_output_must_agree(self):
        log = FIXTURE['candidate_wine']['log']
        self.assertTrue(audit_sphere(log + log)['passed'])
        self.assertFalse(audit_sphere(log + FIXTURE['original_wine']['log'])['passed'])

    def test_cli_enforces_audit_and_cleanup_default(self):
        for label, expected in [('candidate_wine', 0), ('original_wine', 1)]:
            with self.subTest(label=label), tempfile.TemporaryDirectory() as folder:
                base = Path(folder)
                log = base / 'wine.log'
                log.write_text(FIXTURE[label]['log'], encoding='utf-8')
                filesystem = base / 'filesystem.zip'
                filesystem.touch()
                manifest = dict(artifacts=dict(wine_log=str(log)), result=dict(passed=True, reason='ok'))
                result = graphics.GraphicsTestResult('d3dx-sphere', 'd3dxsphere', 326, 0, 0, 0,
                    True, 'ok', (), (), 0)
                args = ['probe', '--probe', 'd3dx-sphere', '--executable', str(base / 'probe.exe'),
                    '--filesystem', str(filesystem), '--build-dir', str(base), '--output', str(base)]
                with patch.object(sys, 'argv', args), patch.object(graphics, 'validate_web_build'), \
                        patch.object(graphics, 'validate_test_executable'), patch.object(graphics, 'find_chrome'), \
                        patch.object(graphics, 'run_browser_test', return_value=(result, manifest)) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(), expected)
                self.assertEqual(run.call_args.kwargs['suite'].cleanup_wait_seconds, 15)
                self.assertEqual(run.call_args.kwargs['group'], 'd3dxsphere')
                stored = json.loads((base / 'manifest.json').read_text())
                self.assertEqual(stored['result']['passed'], expected == 0)
                self.assertEqual(stored['sphere_audit']['passed'], expected == 0)


if __name__ == '__main__':
    unittest.main()
