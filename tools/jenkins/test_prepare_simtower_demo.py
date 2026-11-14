import hashlib
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

import prepare_simtower_demo as package


class SimTowerOverlayTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.root = self.base / "root"
        self.source = self.base / "towerdmo.zip"
        self.source.write_bytes(b"fixture demo installer")
        self.expected = {}
        for name in package.FILES:
            data = ("fixture " + name).encode()
            path = self.root / package.DRIVE / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            self.expected[name] = hashlib.sha256(data).hexdigest()
        self.pins = patch.multiple(package, FILES=self.expected,
                                  SOURCE_SHA256=package.sha(self.source.read_bytes()))
        self.pins.start()
        self.addCleanup(self.pins.stop)

    def prepare(self, name="demo.zip"):
        return package.prepare(self.root, self.source, self.base / name)

    def test_retains_only_demo_and_required_windows_files(self):
        for name in ["home/username/.wine/user.reg", package.DRIVE + "SIMTDEMO/private.tdt",
                     package.DRIVE + "windows/unrelated.dll"]:
            p = self.root / name
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(b"must not be published")
        report = self.prepare()
        with zipfile.ZipFile(report["output"]["path"]) as archive:
            self.assertEqual(set(archive.namelist()),
                             {package.DRIVE + name for name in self.expected} |
                             {package.DRIVE + "simtower-demo.bat"})
            self.assertEqual(archive.read(package.DRIVE + "simtower-demo.bat"), package.LAUNCHER)

    def test_requires_exact_demo_installer(self):
        self.source.write_bytes(b"full game or wrong installer")
        with self.assertRaisesRegex(ValueError, "pinned towerdmo"):
            self.prepare()
        self.assertFalse((self.base / "demo.zip").exists())

    def test_rejects_missing_wing(self):
        (self.root / package.DRIVE / "windows/system/WING.DLL").unlink()
        with self.assertRaisesRegex(ValueError, "missing or linked"):
            self.prepare()

    def test_rejects_changed_game(self):
        (self.root / package.DRIVE / "SIMTDEMO/SIMTDEMO.EXE").write_bytes(b"modified game")
        with self.assertRaisesRegex(ValueError, "does not match"):
            self.prepare()

    def test_rejects_linked_game(self):
        path = self.root / package.DRIVE / "SIMTDEMO/SIMTDEMO.EXE"
        original = path.read_bytes()
        path.unlink()
        outside = self.base / "outside.exe"
        outside.write_bytes(original)
        try:
            path.symlink_to(outside)
        except OSError:
            self.skipTest("symlink creation unavailable")
        with self.assertRaisesRegex(ValueError, "missing or linked"):
            self.prepare()

    def test_deterministic_and_no_overwrite(self):
        first, second = self.prepare("first.zip"), self.prepare("second.zip")
        self.assertEqual(first["output"]["sha256"], second["output"]["sha256"])
        with self.assertRaises(FileExistsError):
            self.prepare("first.zip")


if __name__ == "__main__":
    unittest.main()
