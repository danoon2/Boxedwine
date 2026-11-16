import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from auditD3DXStates import audit_states
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics

FIXTURE = json.loads(Path(__file__).with_name('d3dx_states_results.json').read_text(encoding='utf-8'))


class StatesAuditTests(unittest.TestCase):
    def test_real_windows_and_candidate_observations(self):
        for label in ('windows', 'candidate_wine'):
            with self.subTest(label=label):
                result = audit_states(FIXTURE[label]['log'])
                self.assertTrue(result['passed'], result)
                self.assertEqual((result['sprite_pixels'], result['font_cases'], result['effect_reset_cases']), (16, 5, 6))

    def test_real_old_wine_and_first_candidate_rejected(self):
        for label in ('original_wine', 'first_candidate'):
            with self.subTest(label=label):
                self.assertFalse(audit_states(FIXTURE[label]['log'])['passed'])

    def test_pixel_filter_batch_and_reset_mutations_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        mutations = [
            log.replace('mip=2 x=2 y=2 color=ff0000ff', 'mip=2 x=2 y=2 color=ffff0000'),
            log.replace('mip=1 actual=2 expected=2', 'mip=1 actual=0 expected=0'),
            log.replace('FONT_BATCH_VALUE font-single color=ffff0000', 'FONT_BATCH_VALUE font-single color=ff0000ff'),
            log.replace('FONT_BATCH_VALUE flush-control color=ff0000ff', 'FONT_BATCH_VALUE flush-control color=ffff0000'),
            log.replace('EFFECT_RESET_REFS save-state-2 before=2 after=1', 'EFFECT_RESET_REFS save-state-2 before=2 after=2'),
            log.replace('EFFECT_RESET_RESULT save-state-2 hr=00000000', 'EFFECT_RESET_RESULT save-state-2 hr=8876086c'),
            log.replace('EFFECT_RESET_STATE save-state-2 bound=null', 'EFFECT_RESET_STATE save-state-2 bound=texture'),
            log.replace('EFFECT_RESET_STATE no-save-state-2 bound=texture', 'EFFECT_RESET_STATE no-save-state-2 bound=null'),
        ]
        for mutant in mutations:
            with self.subTest(mutant=mutant):
                self.assertNotEqual(mutant, log)
                self.assertFalse(audit_states(mutant)['passed'])

    def test_missing_duplicate_and_failed_coverage_rejected(self):
        log = FIXTURE['candidate_wine']['log']
        for prefix in ('SPRITE_CASE', 'SPRITE_MIP_FILTER', 'SPRITE_MIP_PIXEL', 'FONT_BATCH_CASE',
                       'FONT_BATCH_VALUE', 'EFFECT_RESET_CASE', 'EFFECT_RESET_REFS',
                       'EFFECT_RESET_RESULT', 'EFFECT_RESET_STATE'):
            row = next(row for row in log.splitlines(True) if row.startswith(prefix))
            for mutant in (log.replace(row, '', 1), log.replace(row, row + row, 1)):
                with self.subTest(prefix=prefix):
                    self.assertFalse(audit_states(mutant)['passed'])
        for mutant in ('', log.replace('361 tests', '360 tests'),
                       log.replace('0 failures), 0 skipped.', '1 failures), 0 skipped.')):
            self.assertFalse(audit_states(mutant)['passed'])

    def test_duplicate_complete_output_must_agree(self):
        log = FIXTURE['candidate_wine']['log']
        self.assertTrue(audit_states(log + log)['passed'])
        self.assertFalse(audit_states(log + FIXTURE['original_wine']['log'])['passed'])

    def test_cli_enforces_audit_and_cleanup_default(self):
        for label, expected in [('candidate_wine', 0), ('original_wine', 1), ('first_candidate', 1)]:
            with self.subTest(label=label), tempfile.TemporaryDirectory() as folder:
                base = Path(folder)
                log = base / 'wine.log'
                log.write_text(FIXTURE[label]['log'], encoding='utf-8')
                filesystem = base / 'filesystem.zip'
                filesystem.touch()
                manifest = dict(artifacts=dict(wine_log=str(log)), result=dict(passed=True, reason='ok'))
                result = graphics.GraphicsTestResult('d3dx-states', 'd3dxstates', 361, 0, 0, 0,
                    True, 'ok', (), (), 0)
                args = ['probe', '--probe', 'd3dx-states', '--executable', str(base / 'probe.exe'),
                    '--filesystem', str(filesystem), '--build-dir', str(base), '--output', str(base)]
                with patch.object(sys, 'argv', args), patch.object(graphics, 'validate_web_build'), \
                        patch.object(graphics, 'validate_test_executable'), patch.object(graphics, 'find_chrome'), \
                        patch.object(graphics, 'run_browser_test', return_value=(result, manifest)) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(), expected)
                self.assertEqual(run.call_args.kwargs['suite'].cleanup_wait_seconds, 15)
                self.assertEqual(run.call_args.kwargs['group'], 'd3dxstates')
                stored = json.loads((base / 'manifest.json').read_text())
                self.assertEqual(stored['result']['passed'], expected == 0)
                self.assertEqual(stored['states_audit']['passed'], expected == 0)


if __name__ == '__main__':
    unittest.main()
