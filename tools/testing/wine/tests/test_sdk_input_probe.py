from pathlib import Path
import sys
import tempfile
import unittest

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from runSdkInputProbe import check_inputs, check_scene, scene_layout_ready
from compareGameFrames import identity


class SdkInputProbeTests(unittest.TestCase):
    def test_caps_canvas_does_not_mean_scene_is_ready(self):
        state = {'frameCanvasSize': [800, 600], 'canvases': [
            {'width': 800, 'height': 600, 'display': 'block', 'visibility': 'visible'},
            {'width': 640, 'height': 480, 'display': 'block', 'visibility': 'visible'}]}
        self.assertFalse(scene_layout_ready(state))
        state['frameCanvasSize'] = [640, 480]
        self.assertTrue(scene_layout_ready(state))
        state['canvases'][-1]['visibility'] = 'hidden'
        self.assertFalse(scene_layout_ready(state))
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.folder = Path(self.temporary.name)
        self.base = Image.new('RGB', (640, 480), (30, 40, 50))
        draw = ImageDraw.Draw(self.base)
        for x in range(0, 640, 16):
            draw.rectangle((x, 0, x + 7, 479), fill=(70, 80, 90))
        self.blur = self.base.copy()
        ImageDraw.Draw(self.blur).rectangle((180, 95, 390, 395), fill=(90, 55, 20))

    def save(self, name, image):
        path = self.folder / name
        image.save(path)
        return path

    def test_selected_effect_matches_and_changes_scene(self):
        base = self.save('base.png', self.base)
        blur = self.save('blur.png', self.blur)
        result = check_scene(blur, blur, base)
        self.assertTrue(result['input_probe_scene_passed'])

    def test_ui_change_cannot_substitute_for_effect(self):
        base = self.save('base.png', self.base)
        reference = self.save('reference.png', self.blur)
        actual = self.base.copy()
        ImageDraw.Draw(actual).rectangle((415, 0, 639, 479), fill='white')
        result = check_scene(self.save('actual.png', actual), reference, base)
        self.assertFalse(result['input_probe_scene_passed'])
        self.assertFalse(result['regions'][0]['progress_passed'])

    def test_scene_pass_does_not_erase_whole_frame_failure(self):
        base = self.save('base.png', self.base)
        reference = self.save('reference.png', self.blur)
        actual = self.blur.copy()
        ImageDraw.Draw(actual).rectangle((415, 0, 639, 479), fill='white')
        result = check_scene(self.save('actual.png', actual), reference, base)
        self.assertTrue(result['input_probe_scene_passed'])
        self.assertFalse(result['passed'])
        self.assertIn('canvas differs from reference', result['problems'])

    def test_wrong_size_cannot_pass(self):
        actual = self.save('actual.png', self.blur.resize((632, 446)))
        result = check_scene(actual, self.save('reference.png', self.blur))
        self.assertFalse(result['input_probe_scene_passed'])

    def test_changed_or_removed_input_is_rejected(self):
        source = self.folder / 'input.bin'
        source.write_bytes(b'original')
        config = {'inputs': [identity(source)]}
        check_inputs(config)
        source.write_bytes(b'modified')
        with self.assertRaisesRegex(ValueError, 'Changed input'):
            check_inputs(config)
        source.unlink()
        with self.assertRaises(FileNotFoundError):
            check_inputs(config)


if __name__ == '__main__':
    unittest.main()
