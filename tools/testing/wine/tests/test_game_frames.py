import importlib.util
import argparse
import json
from pathlib import Path
import tempfile
import unittest

try:
    from PIL import Image, ImageDraw
except ImportError:
    Image = None

if Image is not None:
    spec = importlib.util.spec_from_file_location("compareGameFrames", Path(__file__).resolve().parents[1] / "compareGameFrames.py")
    frames = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(frames)


@unittest.skipIf(Image is None, "optional frame comparator requires requirements-frames.txt")
class GameFrameTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.reference = self.root / "reference.png"
        self.actual = self.root / "actual.png"
        self.scene = Image.new("RGB", (100, 100))
        self.scene.putdata([(x * 2, y * 2, (x + y) % 256) for y in range(100) for x in range(100)])
        self.scene.save(self.reference)
        self.scene.save(self.actual)

    def compare(self, **kwargs):
        return frames.compare_frames(self.actual, self.reference, **kwargs)

    def test_matching_textured_canvas_passes_with_artifact_hashes(self):
        report = self.compare()
        self.assertTrue(report["passed"])
        self.assertEqual(report["comparison"]["bad_pixels"], 0)
        self.assertEqual(len(report["actual"]["sha256"]), 64)

    def test_uniform_black_blue_gray_and_white_cannot_be_references(self):
        for color in [(0, 0, 0), (0, 0, 255), (128, 128, 128), (255, 255, 255)]:
            with self.subTest(color=color):
                solid = Image.new("RGB", (100, 100), color)
                solid.save(self.reference)
                solid.save(self.actual)
                report = self.compare()
                self.assertFalse(report["passed"])
                self.assertEqual(report["comparison"]["bad_pixels"], 0)
                self.assertIn("reference canvas is blank or almost solid", report["problems"])

    def test_small_status_indicator_does_not_make_blank_canvas_pass(self):
        solid = Image.new("RGB", (100, 100), (0, 0, 255))
        ImageDraw.Draw(solid).rectangle((0, 0, 9, 9), fill=(255, 255, 255))
        solid.save(self.actual)
        self.assertIn("actual canvas is blank or almost solid", self.compare()["problems"])

    def test_region_detects_missing_scene_despite_global_tolerance(self):
        # A local regression can occupy too little of the canvas for a global limit.
        ImageDraw.Draw(self.scene).rectangle((40, 40, 49, 49), fill=(128, 128, 128))
        self.scene.save(self.actual)
        self.assertTrue(self.compare()["passed"])
        report = self.compare(regions=[{"name": "shadow", "box": [40, 40, 10, 10], "max_bad_fraction": 0.01}])
        self.assertFalse(report["passed"])
        self.assertIn("region differs from reference: shadow", report["problems"])

    def test_unchanged_valid_frame_does_not_prove_progress(self):
        report = self.compare(previous=self.reference)
        self.assertFalse(report["passed"])
        self.assertEqual(report["progress"]["bad_fraction"], 0)

    def test_changed_frame_still_needs_reference_match(self):
        earlier = self.root / "earlier.png"
        Image.new("RGB", (100, 100), (0, 0, 255)).save(earlier)
        self.assertTrue(self.compare(previous=earlier)["passed"])
        Image.new("RGB", (100, 100), (0, 255, 0)).save(self.actual)
        self.assertFalse(self.compare(previous=earlier)["passed"])

    def test_hud_changes_do_not_prove_scene_progress(self):
        earlier = self.root / "earlier.png"
        prior = self.scene.copy()
        ImageDraw.Draw(prior).rectangle((0, 0, 99, 9), fill=(255, 255, 255))
        prior.save(earlier)
        self.assertTrue(self.compare(previous=earlier)["passed"])
        report = self.compare(previous=earlier, regions=[{
            "name": "scene", "box": [20, 20, 60, 60],
            "max_bad_fraction": 0, "min_progress_fraction": 0.1}])
        self.assertFalse(report["passed"])
        self.assertEqual(report["progress"]["bad_fraction"], 0.1)
        self.assertEqual(report["regions"][0]["progress"]["bad_fraction"], 0)

    def test_region_progress_boundary_and_reference_are_both_required(self):
        earlier = self.root / "earlier.png"
        prior = self.scene.copy()
        ImageDraw.Draw(prior).rectangle((20, 20, 29, 29), fill=(255, 255, 255))
        prior.save(earlier)
        rule = {"name": "effect", "box": [20, 20, 20, 20],
                "max_bad_fraction": 0, "min_progress_fraction": 0.25}
        report = self.compare(previous=earlier, regions=[rule])
        self.assertTrue(report["passed"])
        self.assertEqual(report["regions"][0]["progress"]["bad_fraction"], 0.25)
        self.assertFalse(self.compare(previous=earlier, regions=[{
            **rule, "min_progress_fraction": 0.2501}])["passed"])
        changed = self.scene.copy()
        ImageDraw.Draw(changed).rectangle((20, 20, 39, 39), fill=(0, 255, 0))
        changed.save(self.actual)
        report = self.compare(previous=earlier, regions=[rule])
        self.assertFalse(report["passed"])
        self.assertTrue(report["regions"][0]["progress_passed"])
        self.assertFalse(report["regions"][0]["reference_passed"])

    def test_each_named_region_must_make_progress(self):
        earlier = self.root / "earlier.png"
        prior = self.scene.copy()
        ImageDraw.Draw(prior).rectangle((20, 20, 39, 39), fill=(255, 255, 255))
        prior.save(earlier)
        rules = [{"name": name, "box": [x, 20, 20, 20],
                  "max_bad_fraction": 0, "min_progress_fraction": 0.1}
                 for name, x in [("left", 20), ("right", 60)]]
        report = self.compare(previous=earlier, regions=rules)
        self.assertFalse(report["passed"])
        self.assertEqual([rule["progress_passed"] for rule in report["regions"]], [True, False])

    def test_region_progress_requires_previous_and_positive_finite_minimum(self):
        rule = {"name": "effect", "box": [20, 20, 20, 20], "max_bad_fraction": 0}
        with self.assertRaises(ValueError):
            self.compare(regions=[{**rule, "min_progress_fraction": 0.1}])
        for minimum in (0, -0.1, 1.1, float("nan")):
            with self.subTest(minimum=minimum), self.assertRaises((ValueError, argparse.ArgumentTypeError)):
                self.compare(previous=self.reference, regions=[{**rule, "min_progress_fraction": minimum}])
        previous = self.root / "wrong-size.png"
        self.scene.resize((50, 50)).save(previous)
        report = self.compare(previous=previous, regions=[{**rule, "min_progress_fraction": 0.1}])
        self.assertFalse(report["passed"])
        self.assertFalse(report["regions"][0]["progress_passed"])

    def test_cli_region_progress_is_recorded(self):
        previous = self.root / "previous.png"
        Image.new("RGB", (100, 100), (0, 0, 255)).save(previous)
        output = self.root / "comparison.json"
        status = frames.main([str(self.actual), "--reference", str(self.reference),
            "--output", str(output), "--previous", str(previous),
            "--region", "scene,20,20,60,60,0", "--region-progress", "scene,0.5"])
        self.assertEqual(status, 0)
        rule = json.loads(output.read_text())["regions"][0]
        self.assertTrue(rule["progress_passed"])
        self.assertEqual(rule["min_progress_fraction"], 0.5)

    def test_cli_rejects_unknown_duplicate_or_unusable_progress_requests(self):
        base = [str(self.actual), "--reference", str(self.reference),
                "--output", str(self.root / "invalid.json"), "--region", "scene,20,20,60,60,0"]
        cases = [["--region-progress", "scene,0.1"],
                 ["--region-progress", "missing,0.1", "--previous", str(self.reference)],
                 ["--region-progress", "scene,0.1", "--region-progress", "scene,0.2", "--previous", str(self.reference)],
                 ["--region-progress", "scene,0", "--previous", str(self.reference)]]
        for arguments in cases:
            with self.subTest(arguments=arguments), self.assertRaises(SystemExit) as error:
                frames.main(base + arguments)
            self.assertEqual(error.exception.code, 2)
        self.assertFalse((self.root / "invalid.json").exists())

    def test_different_dimensions_are_not_silently_resized(self):
        self.scene.resize((200, 200)).save(self.actual)
        self.assertFalse(self.compare()["passed"])

    def test_progress_size_change_is_not_content_proof(self):
        earlier = self.root / "earlier.png"
        self.scene.resize((50, 50)).save(earlier)
        self.assertFalse(self.compare(previous=earlier)["passed"])

    def test_channel_tolerance_has_an_exact_boundary(self):
        self.scene.point(lambda value: value + 4).save(self.actual)
        self.assertTrue(self.compare(tolerance=4)["passed"])
        self.assertFalse(self.compare(tolerance=3)["passed"])

    def test_invalid_or_empty_regions_are_rejected(self):
        for box in [[0, 0, 0, 20], [-1, 0, 5, 5], [95, 95, 10, 10], [0.5, 0, 5, 5]]:
            with self.subTest(box=box), self.assertRaises(ValueError):
                self.compare(regions=[{"name": "scene", "box": box, "max_bad_fraction": 0}])
        rule = {"name": "scene", "box": [0, 0, 10, 10], "max_bad_fraction": 0}
        with self.assertRaises(ValueError):
            self.compare(regions=[rule, rule])

    def test_transparent_guest_readback_is_not_compositor_evidence(self):
        self.scene.convert("RGBA").save(self.actual)
        self.assertTrue(self.compare()["passed"])
        image = self.scene.convert("RGBA")
        image.putalpha(0)
        image.save(self.actual)
        with self.assertRaises(ValueError):
            self.compare()

    def test_cli_requires_reference_and_preserves_existing_report(self):
        output = self.root / "result.json"
        output.write_text("retained result")
        with self.assertRaises(SystemExit) as error:
            frames.main([str(self.actual), "--reference", str(self.reference), "--output", str(output)])
        self.assertEqual(error.exception.code, 2)
        self.assertEqual(output.read_text(), "retained result")


if __name__ == "__main__":
    unittest.main()
