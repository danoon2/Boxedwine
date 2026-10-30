import json
from pathlib import Path
import tempfile
import unittest
import zipfile

import build_site
import prepare_3dmark2001_demo as packager


class Prepare3DMarkTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.root = self.base / "root"
        self.app = self.root / packager.DRIVE / packager.APP
        for name in packager.REQUIRED:
            path = self.app / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"installed fixture " + name.encode())
        (self.app / "error.log").write_text("local diagnostics")
        (self.app / "result.3dr").write_text("local result")
        (self.app / "settings.3dp").write_text("local results and settings")
        (self.root / "home/username/.wine/user.reg").write_text("private registry")

    def test_overlay_is_deterministic_and_launches_from_c_drive(self):
        first = self.base / "3dmark2001se.zip"
        report = packager.prepare(self.root, first)
        second = self.base / "second.zip"
        packager.prepare(self.root, second)
        self.assertEqual(first.read_bytes(), second.read_bytes())
        second.unlink()
        with zipfile.ZipFile(first) as archive:
            self.assertIsNone(archive.testzip())
            self.assertTrue(all(n.startswith(packager.DRIVE) for n in archive.namelist()))
            self.assertFalse(any(n.endswith((".reg", ".log", ".3dp", ".3dr")) for n in archive.namelist()))
            for name in packager.REQUIRED:
                self.assertEqual(archive.read(packager.DRIVE + packager.APP + name),
                                 (self.app / name).read_bytes())
            launcher = archive.read(packager.DRIVE + "3dmark2001se.bat")
            self.assertIn(b'cd /d "C:\\Program Files\\MadOnion.com\\3DMark2001 SE"\r\n', launcher)
        self.assertEqual(report["omitted_app_files"], ["error.log", "result.3dr", "settings.3dp"])
        example = json.loads(Path(__file__).with_name("demos.example.json").read_text())
        (self.base / "demos.json").write_text(json.dumps(example))
        demo, = build_site.discover_demos(self.base)
        self.assertEqual(demo["zipParam"], "overlay")
        self.assertEqual(demo["program"], "c:/3dmark2001se.bat")
        for mode in ("st", "mt", "st-jit", "mt-jit"):
            url = build_site.build_demo_launch_url(mode, demo)
            self.assertIn("overlay=3dmark2001se.zip", url)
            self.assertIn("p=c:/3dmark2001se.bat", url)

    def test_missing_converted_textures_rejected(self):
        (self.app / "textures.ras").unlink()
        with self.assertRaisesRegex(ValueError, "textures.ras"):
            packager.prepare(self.root, self.base / "missing.zip")
        self.assertFalse((self.base / "missing.zip").exists())

    def test_existing_output_preserved(self):
        output = self.base / "existing.zip"
        output.write_bytes(b"keep")
        with self.assertRaises(FileExistsError):
            packager.prepare(self.root, output)
        self.assertEqual(output.read_bytes(), b"keep")


if __name__ == "__main__":
    unittest.main()
