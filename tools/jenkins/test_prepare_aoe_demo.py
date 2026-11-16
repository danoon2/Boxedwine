import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from urllib.parse import parse_qs, urlsplit
import zipfile

import build_site
import prepare_aoe_demo as aoe


class AgeOfEmpiresDemoTests(unittest.TestCase):
    def test_overlay_and_both_catalogs_keep_required_launch_settings(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            source = base / "source.dat"
            with zipfile.ZipFile(source, "w") as archive:
                archive.writestr("empires.exe", b"trial executable")
                archive.writestr("data/empires.dat", b"trial data")
            output = base / "aoe-overlay.zip"
            with patch.object(aoe, "SOURCE_SHA256", hashlib.sha256(source.read_bytes()).hexdigest()):
                aoe.prepare(source, output)
                second = base / "repeat.dat"
                aoe.prepare(source, second)
                self.assertEqual(output.read_bytes(), second.read_bytes())
                with self.assertRaises(FileExistsError):
                    aoe.prepare(source, output)
            with zipfile.ZipFile(output) as archive:
                self.assertEqual(archive.read(aoe.DRIVE + aoe.APP + "empires.exe"), b"trial executable")
                self.assertNotIn("home/username/.wine/system.reg", archive.namelist())
                registry = archive.read(aoe.DRIVE + "aoe-demo.reg")
                self.assertIn(b"InstallationDirectory", registry)
                self.assertIn(b"DirectPlay\\Applications\\Age of Empires Trial", registry)
                launcher = archive.read(aoe.DRIVE + "aoe.bat")
                self.assertLess(launcher.index(b"regedit /S"), launcher.index(b"empires.exe"))
                self.assertIn(b'cd /d "C:\\Program Files\\Microsoft Games\\Age of Empires Trial"', launcher)
            example = json.loads(Path(__file__).with_name("demos.example.json").read_text())
            (base / "demos.json").write_text(json.dumps(example))
            demo, = build_site.discover_demos(base)
            self.assertEqual("overlay", demo["zipParam"])
            for mode, _, _ in build_site.DEMO_RUNNER_SPECS:
                for url in (build_site.build_demo_launch_url(mode, demo),
                            build_site.demo_launch_url("branch", "1", mode, demo)):
                    query = parse_qs(urlsplit(url).query)
                    self.assertEqual(["aoe-overlay.zip"], query["overlay"])
                    self.assertNotIn("app", query)
                    self.assertEqual(["c:/aoe.bat"], query["p"])
                    self.assertEqual(["gdi,winxp"], query["registry"])
                    self.assertEqual(["1024x768"], query["resolution"])
                    self.assertEqual(["32"], query["bpp"])

    def test_unrecognized_source_is_rejected_before_creating_overlay(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            source = base / "source.zip"
            source.write_bytes(b"different game or changed download")
            output = base / "overlay.zip"
            with self.assertRaisesRegex(ValueError, "pinned public"):
                aoe.prepare(source, output)
            self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
