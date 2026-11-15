import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from auditD3DXTangent import audit_tangent
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics

FIXTURE = json.loads(Path(__file__).with_name('d3dx_tangent_results.json').read_text(encoding='utf-8'))


class TangentAuditTests(unittest.TestCase):
    def test_real_windows_and_candidate_vectors(self):
        for label in ('windows', 'candidate_wine'):
            with self.subTest(label=label):
                result = audit_tangent(FIXTURE[label]['log'])
                self.assertTrue(result['passed'], result)
                self.assertEqual((result['cases'], result['vertices'], result['components']), (8, 24, 216))

    def test_real_original_wine_rejected(self):
        self.assertFalse(audit_tangent(FIXTURE['original_wine']['log'])['passed'])

    def test_wrong_orthogonal_frame_rejected(self):
        log = FIXTURE['candidate_wine']['log'].replace(
            'tangent=0.923879564,-0.382683486,0 binormal=0.382683426,0.923879564,0',
            'tangent=1,0,0 binormal=0,1,0')
        self.assertNotEqual(log, FIXTURE['candidate_wine']['log'])
        self.assertFalse(audit_tangent(log)['passed'])

    def test_coverage_clone_and_nonfinite_mutations_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        rows = log.splitlines()
        mutations = [log.replace('TANGENT_CASE normal-32\n', ''),
            log.replace('TANGENT_CLONE clone-16 distinct=1', 'TANGENT_CLONE clone-16 distinct=0'),
            log.replace('tangent=1,0,0', 'tangent=nan,0,0', 1),
            log.replace('tangent=1,0,0', 'tangent=broken', 1),
            log.replace('0 failures), 0 skipped.', '1 failures), 0 skipped.'),
            '\n'.join(row for row in rows if not row.startswith('TANGENT_VERTEX normal-16 vertex=1 ')),
            log.replace('TANGENT_CASE skew-16', 'TANGENT_CASE skew-16\nTANGENT_CASE skew-16')]
        for index, mutant in enumerate(mutations):
            with self.subTest(index=index):
                self.assertNotEqual(mutant, log)
                self.assertFalse(audit_tangent(mutant)['passed'])

    def test_duplicate_complete_output_must_agree(self):
        log = FIXTURE['candidate_wine']['log']
        self.assertTrue(audit_tangent(log + log)['passed'])
        self.assertFalse(audit_tangent(log + FIXTURE['original_wine']['log'])['passed'])

    def test_cli_gates_vector_audit_and_preserves_cleanup_default(self):
        for label, expected in [('candidate_wine', 0), ('original_wine', 1)]:
            with self.subTest(label=label), tempfile.TemporaryDirectory() as folder:
                base = Path(folder)
                log = base / 'wine.log'
                log.write_text(FIXTURE[label]['log'], encoding='utf-8')
                filesystem = base / 'filesystem.zip'
                filesystem.touch()
                manifest = dict(artifacts=dict(wine_log=str(log)), result=dict(passed=True, reason='ok'))
                result = graphics.GraphicsTestResult('d3dx-tangent', 'd3dxtangent', 338, 0, 0, 0,
                    True, 'ok', (), (), 0)
                args = ['probe', '--probe', 'd3dx-tangent', '--executable', str(base / 'probe.exe'),
                    '--filesystem', str(filesystem), '--build-dir', str(base), '--output', str(base)]
                with patch.object(sys, 'argv', args), patch.object(graphics, 'validate_web_build'), \
                        patch.object(graphics, 'validate_test_executable'), patch.object(graphics, 'find_chrome'), \
                        patch.object(graphics, 'run_browser_test', return_value=(result, manifest)) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(), expected)
                self.assertEqual(run.call_args.kwargs['suite'].cleanup_wait_seconds, 15)
                self.assertEqual(run.call_args.kwargs['group'], 'd3dxtangent')
                stored = json.loads((base / 'manifest.json').read_text())
                self.assertEqual(stored['result']['passed'], expected == 0)
                self.assertEqual(stored['tangent_audit']['passed'], expected == 0)


if __name__ == '__main__':
    unittest.main()
