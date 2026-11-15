from pathlib import Path
import json
import sys
import tempfile
import unittest
from unittest import mock

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from runMechWarriorProbe import Client, Scenario, Templates, TEMPLATES, audit_exit, validate_config, verify_pins
from compareGameFrames import identity


class MechProbeTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.folder = Path(temporary.name)

    def templates(self):
        entries = {}
        for name, (box, color) in TEMPLATES.items():
            image = Image.new('RGB', (886, 664), (10, 20, 30))
            draw = ImageDraw.Draw(image)
            draw.rectangle((box[0], box[1], box[0] + 10, box[1] + 10),
                           fill={'yellow': (220, 170, 0), 'green': (0, 220, 0), 'white': (230, 230, 230)}[color])
            path = self.folder / (name + '.png')
            image.save(path)
            entries[name] = identity(path)
        manifest = self.folder / 'templates.json'
        manifest.write_text(json.dumps({'schema_version': 1, 'templates': entries}))
        return manifest

    def test_wrong_screen_or_size_does_not_release_transition(self):
        refs = Templates(self.templates())
        self.assertEqual(refs.match('main', self.folder / 'main.png'), 1)
        self.assertEqual(refs.match('main', self.folder / 'results.png'), 0)
        small = self.folder / 'small.png'
        Image.new('RGB', (885, 664), 'yellow').save(small)
        self.assertEqual(refs.match('main', small), 0)

    def test_blank_or_unpinned_reference_is_rejected(self):
        manifest = self.templates()
        Image.new('RGB', (886, 664), 'black').save(self.folder / 'main.png')
        with self.assertRaisesRegex(ValueError, 'Changed input'):
            Templates(manifest)
        data = json.loads(manifest.read_text())
        data['templates']['main'] = identity(self.folder / 'main.png')
        manifest.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'too few foreground'):
            Templates(manifest)

    def test_missing_reference_and_mutation_after_load_are_rejected(self):
        manifest = self.templates()
        refs = Templates(manifest)
        (self.folder / 'hud.png').unlink()
        with self.assertRaises(FileNotFoundError):
            verify_pins(refs.pins)
        data = json.loads(manifest.read_text())
        del data['templates']['hud']
        manifest.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'eight named'):
            Templates(manifest)

    def test_menu_wait_stops_on_exit_and_cannot_click_next_screen(self):
        action = mock.Mock(return_value={'lifecycle': 'BW_GAME_EXIT:0\n'})
        scenario = Scenario(action, self.folder, mock.Mock())
        with self.assertRaisesRegex(RuntimeError, 'exited before main'):
            scenario.until('main', 10)
        self.assertEqual(action.call_count, 1)
        scenario.templates.match.assert_not_called()

    def test_timeout_retains_observations_without_advancing(self):
        action = mock.Mock(return_value={})
        clock = iter([0, 0, 1, 2])
        scenario = Scenario(action, self.folder, mock.Mock(match=mock.Mock(return_value=0)),
                            clock=lambda: next(clock))
        with self.assertRaisesRegex(RuntimeError, 'not observed: pause'):
            scenario.until('pause', 2)
        self.assertEqual(len(scenario.observations), 2)
        self.assertTrue(all(call.args[0] in ('capture', 'wait') for call in action.call_args_list))

    def test_retained_cleanup_must_be_zero_complete_and_observed(self):
        capture = dict(applicationExitObserved=True, cleanupObserved=True,
                       cleanupWaitSatisfied=True, cleanupObservationMilliseconds=15000, errors=[])
        state = {'lifecycle': 'BW_GAME_EXIT:0\nBW_GAME_CLEANUP:0\n'}
        self.assertEqual(audit_exit(capture, state, 'ordinary chrome output\n', 0, True), [])
        negatives = [({'cleanupObservationMilliseconds': 14999}, state, 0, True),
                     ({'cleanupObservationMilliseconds': None}, state, 0, True),
                     ({'cleanupObservationMilliseconds': float('nan')}, state, 0, True),
                     ({'cleanupObservationMilliseconds': float('inf')}, state, 0, True),
                     ({'cleanupWaitSatisfied': False}, state, 0, True),
                     ({'cleanupWaitSatisfied': 'false'}, state, 0, True),
                     ({'errors': None}, state, 0, True),
                     ({'errors': ['late failure']}, state, 0, True),
                     ({}, {'lifecycle': None}, 0, True),
                     ({}, {'lifecycle': 'BW_GAME_EXIT:0\nBW_GAME_CLEANUP:1\n'}, 0, True),
                     ({}, state, 1, True), ({}, state, 0, False)]
        for changes, final, code, observed in negatives:
            with self.subTest(changes=changes, final=final, code=code, observed=observed):
                with self.assertRaisesRegex(ValueError, 'lifecycle'):
                    audit_exit(dict(capture, **changes), final, 'ordinary output', code, observed)
        for incomplete in ({}, [], None, {k: v for k, v in capture.items() if k != 'errors'}):
            with self.assertRaisesRegex(ValueError, 'lifecycle'):
                audit_exit(incomplete, state, 'ordinary output', 0, True)
        for log in ['', 'WebGL: INVALID_OPERATION: drawArrays: no valid shader program\n' + 'ordinary\n' * 1000]:
            with self.assertRaisesRegex(ValueError, 'Full Chrome log'):
                audit_exit(capture, state, log, 0, True)

    def test_existing_output_is_rejected_before_preparing_or_launching(self):
        with mock.patch('runMechWarriorProbe.prepare') as prepare:
            with self.assertRaisesRegex(ValueError, 'already exists'):
                validate_config({'output': str(self.folder)})
            prepare.assert_not_called()


class ClientTests(unittest.TestCase):
    def child(self, source):
        client = Client([sys.executable, '-u', '-c', source])
        self.addCleanup(client.finish)
        return client

    def test_close_acknowledgement_prevents_the_retained_double_close(self):
        source = '''import json, sys, time
print(json.dumps({'ready': True}), flush=True)
command = json.loads(sys.stdin.readline())
assert command == {'type': 'close'}
print(json.dumps({'closed': 'requested'}), flush=True)
time.sleep(.2)
extra = sys.stdin.read()
assert not extra, repr(extra)
'''
        client = self.child(source)
        self.assertTrue(client.receive(5)['ready'])
        client.action('close')
        with self.assertRaisesRegex(RuntimeError, 'already closed'):
            client.action('close')
        client.finish()
        self.assertEqual(client.process.returncode, 0)

    def test_wrong_response_or_early_eof_cannot_pass_an_action(self):
        source = '''import json, sys
print(json.dumps({'ready': True}), flush=True)
json.loads(sys.stdin.readline())
print(json.dumps({'action': {'type': 'wrong'}, 'result': {}}), flush=True)
'''
        client = self.child(source)
        client.receive(5)
        with self.assertRaisesRegex(RuntimeError, 'Mismatched'):
            client.action('capture', name='scene')
        client.process.wait(timeout=5)
        with self.assertRaisesRegex(RuntimeError, 'stdout closed'):
            client.receive(5)


if __name__ == '__main__':
    unittest.main()
