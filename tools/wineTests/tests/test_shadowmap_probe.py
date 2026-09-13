"""Browser-free scene, stability and retained-report controls for ShadowMap."""
from contextlib import redirect_stdout
import io
import json
from pathlib import Path
import shutil
from types import SimpleNamespace
import sys
import tempfile
import unittest
from unittest.mock import patch

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import runShadowMapProbe as probe


VISIBLE = dict(frameCanvasSize=[640, 480], canvases=[
    dict(display='block', visibility='visible', width=640, height=480)])


class ShadowMapTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.reference = self.root / 'reference.png'
        image = Image.new('RGB', (640, 480), (40, 80, 120))
        draw = ImageDraw.Draw(image)
        for y in range(0, 480, 20):
            for x in range(0, 640, 20):
                if (x // 20 + y // 20) % 2:
                    draw.rectangle((x, y, x + 19, y + 19), fill=(180, 150, 90))
        image.save(self.reference)
        self.variant = self.root / 'variant.png'
        image.putpixel((1, 1), (41, 80, 120))
        image.save(self.variant)
        self.bad_shadow = self.root / 'bad-shadow.png'
        draw.rectangle((170, 270, 269, 379), fill=(255, 0, 255))
        image.save(self.bad_shadow)

    def scenario(self, frames, states=None, seconds=30):
        output = self.root / 'scene-output'
        output.mkdir(exist_ok=True)
        checkpoints, actions = [], []
        now, index, state_index = 0, 0, 0

        def action(kind, **values):
            nonlocal now, index, state_index
            actions.append(dict(type=kind, **values))
            if kind == 'state':
                state = states[min(state_index, len(states) - 1)] if states else VISIBLE
                state_index += 1
                return state
            if kind == 'native-capture':
                shutil.copyfile(frames[min(index, len(frames) - 1)],
                                output / (values['name'] + '-native.png'))
                index += 1
            if kind == 'wait':
                now += values['milliseconds'] / 1000
            return {}

        probe.wait_for_scene(action, output, self.reference, checkpoints, seconds, clock=lambda: now)
        return checkpoints, actions

    def test_stable_pair_matches_full_frame_and_both_regions(self):
        checkpoints, _ = self.scenario([self.reference])
        self.assertEqual(len(checkpoints), 2)
        self.assertTrue(all(c['passed'] for c in checkpoints))
        self.assertEqual([r['name'] for r in checkpoints[-1]['regions']], ['shadow-wall', 'shadow-ground'])

    def test_passing_but_changing_frames_need_identical_pair(self):
        checkpoints, _ = self.scenario([self.reference, self.variant, self.variant])
        self.assertEqual(len(checkpoints), 3)
        self.assertTrue(all(c['passed'] for c in checkpoints))

    def test_hidden_canvas_interrupts_pair(self):
        checkpoints, actions = self.scenario([self.reference], states=[VISIBLE, {}, VISIBLE, VISIBLE])
        self.assertEqual(len(checkpoints), 3)
        self.assertEqual(sum(a['type'] == 'state' for a in actions), 4)

    def test_local_shadow_defect_is_rejected_even_with_global_match(self):
        comparison = probe.check_scene(self.bad_shadow, self.reference)
        self.assertLess(comparison['comparison']['bad_fraction'], 0.05)
        self.assertFalse(comparison['regions'][1]['passed'])
        with self.assertRaisesRegex(RuntimeError, 'timed out'):
            self.scenario([self.bad_shadow], seconds=10)

    def test_early_exit_and_blank_scene_never_pass(self):
        with self.assertRaisesRegex(RuntimeError, 'exited before'):
            self.scenario([self.reference], states=[dict(lifecycle='BW_GAME_EXIT:0')])
        blank = self.root / 'blank.png'
        Image.new('RGB', (640, 480)).save(blank)
        with self.assertRaisesRegex(RuntimeError, 'timed out'):
            self.scenario([blank], seconds=10)

    def wrapper(self, *, missing_driver=False, capture_error=False, mutate_config=False,
                performance=False, measurement=None):
        output = self.root / 'wrapper-output'
        config = self.root / 'config.json'
        original = json.dumps(dict(output=str(output), inputs=[], launchPath='/st/boxedwine.html',
                                   performance=performance)) + '\r\n'
        config.write_bytes(original.encode())
        args = SimpleNamespace(config=config, reference=self.reference, node=Path('node'), startup_timeout=30)
        actions = []
        reference = self.reference

        class Driver:
            process = SimpleNamespace(returncode=0)
            transcript = ['synthetic driver output\n']

            def __init__(self, command):
                if missing_driver:
                    raise FileNotFoundError('synthetic missing Node')
                output.mkdir()

            def receive(self):
                return dict(ready=True, url='http://127.0.0.1:12345/st/boxedwine.html')

            def action(self, kind, **values):
                actions.append(dict(type=kind, **values))
                if kind == 'state':
                    return VISIBLE
                if kind == 'native-capture':
                    shutil.copyfile(reference, output / (values['name'] + '-native.png'))
                if kind == 'measure-performance':
                    return measurement
                return {}

            def finish(self):
                capture = dict(launchUrl='http://127.0.0.1:12345/st/boxedwine.html', errors=[],
                    applicationExitObserved=True, cleanupObserved=True, cleanupWaitSatisfied=True,
                    cleanupObservationMilliseconds=15000)
                if capture_error:
                    capture['cleanupObservationMilliseconds'] = 14999
                (output / 'capture.json').write_text(json.dumps(capture))
                (output / 'final-state.json').write_text(json.dumps(dict(lifecycle='BW_GAME_EXIT:0\nBW_GAME_CLEANUP:0\n')))
                (output / 'chrome.log').write_text('synthetic clean Chrome log\n')
                if mutate_config:
                    config.write_text(original + ' ')

        with patch.object(probe, 'Client', Driver), redirect_stdout(io.StringIO()):
            code = probe.run(args)
        report = json.loads((output / 'shadowmap-checkpoint.json').read_text())
        self.assertEqual((output / 'shadowmap-config.json').read_bytes(), original.encode())
        self.assertEqual(code, 0 if report['passed'] else 1)
        return report, actions

    def test_complete_scene_and_lifecycle_pass(self):
        report, actions = self.wrapper()
        self.assertTrue(report['passed'])
        self.assertTrue(report['rendered_and_stable'])
        self.assertTrue(report['exit_observed'])
        self.assertEqual(actions[-2:], [dict(type='key', key='Escape'), dict(type='observe-exit', timeoutSeconds=60)])

    def test_failed_launch_retains_config_command_and_unknown_exit(self):
        report, _ = self.wrapper(missing_driver=True)
        self.assertFalse(report['passed'])
        self.assertFalse(report['driver_started'])
        self.assertIsNone(report['driver_exit_code'])
        self.assertIn('synthetic missing Node', report['problems'])
        self.assertTrue(report['command'][-1].endswith('config.json'))

    def test_short_cleanup_fails_despite_matching_scene(self):
        report, _ = self.wrapper(capture_error=True)
        self.assertFalse(report['passed'])
        self.assertTrue(report['rendered_and_stable'])
        self.assertTrue(any('lifecycle' in p for p in report['problems']))

    def test_configuration_mutation_retains_original_and_fails(self):
        report, _ = self.wrapper(mutate_config=True)
        self.assertFalse(report['passed'])
        self.assertTrue(any('configuration changed' in p for p in report['problems']))

    def test_optional_measurement_is_after_stable_scene_and_before_exit(self):
        report, actions = self.wrapper(performance=True, measurement={'measurementValid': True})
        self.assertTrue(report['passed'])
        self.assertFalse(report['performance_acceptance'])
        self.assertEqual(report['performance'], {'scene-steady': {'measurementValid': True}})
        self.assertEqual(actions[-5:], [dict(type='wait', milliseconds=5000),
            dict(type='wait', milliseconds=5000),
            dict(type='measure-performance', name='scene-steady', milliseconds=30000),
            dict(type='key', key='Escape'), dict(type='observe-exit', timeoutSeconds=60)])

    def test_invalid_measurement_fails_without_skipping_clean_exit(self):
        report, _ = self.wrapper(performance=True, measurement={'measurementValid': False})
        self.assertFalse(report['passed'])
        self.assertTrue(report['input_lifecycle_passed'])
        self.assertTrue(report['exit_observed'])

    def test_default_run_has_no_measurement_actions(self):
        report, actions = self.wrapper()
        self.assertTrue(report['passed'])
        self.assertEqual(report['performance'], {})
        self.assertFalse(any(a['type'] == 'measure-performance' for a in actions))


if __name__ == '__main__':
    unittest.main()
