"""SDK measurement sequencing with a simulated capture driver, no browser."""
from contextlib import redirect_stdout
import io
import json
from pathlib import Path
import queue
import shutil
from types import SimpleNamespace
import sys
import tempfile
import unittest
from unittest.mock import patch

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import runSdkInputProbe as sdk
from gamePerformancePhase import measurements_pass


class SdkPerformanceTests(unittest.TestCase):
    def test_requested_phase_set_and_boolean_validity_are_required(self):
        valid = {'measurementValid': True}
        self.assertTrue(measurements_pass({'base': valid, 'blur': valid}, ('base', 'blur')))
        for values in ({}, {'base': valid}, {'base': valid, 'blur': valid, 'extra': valid},
                       {'base': [], 'blur': valid}, {'base': None, 'blur': valid}):
            self.assertFalse(measurements_pass(values, ('base', 'blur')))
        for value in (False, None, 1, 'true'):
            self.assertFalse(measurements_pass({'base': {'measurementValid': value}}, ('base',)))

    def run_sdk(self, performance=False, invalid_phase=None):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name)
        output = root / 'capture'
        base = root / 'base.png'
        blur = root / 'blur.png'
        image = Image.new('RGB', (640, 480), (30, 40, 50))
        draw = ImageDraw.Draw(image)
        for x in range(0, 640, 16):
            draw.rectangle((x, 0, x + 7, 479), fill=(70, 80, 90))
        image.save(base)
        draw.rectangle((180, 95, 390, 395), fill=(90, 55, 20))
        image.save(blur)
        config = root / 'config.json'
        config.write_text(json.dumps(dict(output=str(output), inputs=[], performance=performance,
                                         launchPath='/st/boxedwine.html')))
        args = SimpleNamespace(config=config, node=Path('node'), native_base=base, native_blur=blur,
            desktop_ui_base=base, desktop_ui_blur=blur, startup_timeout=10)
        actions, replies, receive_timeouts = [], queue.Queue(), []
        current = base
        url = 'http://127.0.0.1:12345/st/boxedwine.html'
        replies.put(dict(ready=True, url=url))

        class Output:
            def __iter__(self):
                while True:
                    reply = replies.get(timeout=10)
                    if reply is None:
                        return
                    yield json.dumps(reply) + '\n'

        class Input:
            def write(self, text):
                nonlocal current
                request = json.loads(text)
                actions.append(request)
                kind = request['type']
                value = {}
                if kind == 'state':
                    value = dict(frameCanvasSize=[640, 480], canvases=[
                        dict(display='block', visibility='visible', width=640, height=480)])
                elif kind == 'click' and request['x'] == 470 / 640:
                    current = blur
                elif kind == 'native-capture':
                    shutil.copyfile(current, output / (request['name'] + '-native.png'))
                elif kind == 'measure-performance':
                    value = dict(name=request['name'], measurementValid=request['name'] != invalid_phase,
                                 durationMilliseconds=30000)
                elif kind == 'close':
                    (output / 'capture.json').write_text(json.dumps(dict(launchUrl=url, errors=[],
                        applicationExitObserved=True, cleanupObserved=True, cleanupWaitSatisfied=True,
                        cleanupObservationMilliseconds=15000)))
                    (output / 'chrome.log').write_text('synthetic clean Chrome log\n')
                    process.returncode = 0
                    replies.put(dict(closed='requested'))
                    replies.put(None)
                    return
                replies.put(dict(action=request, result=value))

            def flush(self):
                pass

        process = SimpleNamespace(stdin=Input(), stdout=Output(), stderr=io.StringIO(), returncode=None)
        process.poll = lambda: process.returncode
        process.wait = lambda timeout: process.returncode

        def launch(*args, **kwargs):
            output.mkdir()
            return process

        # Capture the actual queue timeout for each outer protocol read. The
        # measurement lasts thirty seconds before serializing its reply.
        original_get = queue.Queue.get

        def get(instance, *args, **kwargs):
            if instance is not replies:
                receive_timeouts.append(kwargs.get('timeout'))
            return original_get(instance, *args, **kwargs)

        with patch.object(sdk.subprocess, 'Popen', launch), patch.object(queue.Queue, 'get', get), redirect_stdout(io.StringIO()):
            code = sdk.run(args)
        report = json.loads((output / 'sdk-input-probe.json').read_text())
        self.assertEqual(code, 0 if report['passed'] else 1)
        self.assertEqual(report['performance_requested'], performance)
        self.assertFalse(report['performance_acceptance'])
        self.assertTrue(report['input_lifecycle_passed'])
        self.assertTrue(report['stable_desktop_wine_ui_passed'])
        self.assertTrue(report['exit_observed'])
        return report, actions, receive_timeouts

    def test_default_sdk_sequence_does_not_request_measurements(self):
        report, actions, _ = self.run_sdk()
        self.assertTrue(report['passed'])
        self.assertEqual(report['performance'], {})
        self.assertFalse(any(a['type'] == 'measure-performance' for a in actions))

    def test_base_and_blur_measured_after_image_gates_with_warmup(self):
        report, actions, timeouts = self.run_sdk(performance=True)
        self.assertTrue(report['passed'])
        self.assertEqual(set(report['performance']), {'base-steady', 'blur-steady'})
        phases = [i for i, a in enumerate(actions) if a['type'] == 'measure-performance']
        self.assertEqual([actions[i]['name'] for i in phases], ['base-steady', 'blur-steady'])
        blur_click = next(i for i, a in enumerate(actions) if a['type'] == 'click' and a['x'] == 470 / 640)
        self.assertLess(phases[0], blur_click)
        self.assertGreater(phases[1], blur_click)
        for i in phases:
            self.assertEqual(actions[i - 2:i], [dict(type='wait', milliseconds=5000)] * 2)
            self.assertEqual(actions[i]['milliseconds'], 30000)
            self.assertEqual(actions[i - 3]['type'], 'native-capture')
            self.assertGreater(timeouts[i + 1], 30)  # One ready message precedes actions.

    def test_either_invalid_phase_fails_but_keeps_exit_and_both_results(self):
        for name in ('base-steady', 'blur-steady'):
            with self.subTest(name=name):
                report, _, _ = self.run_sdk(performance=True, invalid_phase=name)
                self.assertFalse(report['passed'])
                self.assertEqual(set(report['performance']), {'base-steady', 'blur-steady'})
                self.assertFalse(report['performance'][name]['measurementValid'])


if __name__ == '__main__':
    unittest.main()
