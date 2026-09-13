from pathlib import Path
import tempfile
import unittest
import zipfile

import prepare_demo_overlay as overlay


class OverlayTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.source = self.root / "source.zip"
        self.output = self.root / "app.zip"
        self.game = overlay.DRIVE + "SIERRA/CAESAR3DEMO/"
        self.batch = overlay.DRIVE + "c3.bat"
        self.files = {self.game + "c3.exe": b"application", self.game + "textures.dat": b"textures",
                      self.batch: b"c:\r\ncd \\SIERRA\\CAESAR3DEMO\r\nc3.exe",
                      "opt/wine/bin/wine": b"old wine", "etc/ld.so.cache": b"old cache",
                      "lib/libc.so.6": b"old libc", "home/username/.wine/system.reg": b"old registry",
                      overlay.DRIVE + "windows/system32/ddraw.dll": b"old ddraw"}
        self.create()

    def create(self):
        with zipfile.ZipFile(self.source, "w") as archive:
            for name, data in self.files.items():
                info = zipfile.ZipInfo(name)
                # Preserve malformed separators in the fixture even on Windows.
                info.filename = name
                archive.writestr(info, data)

    def test_keeps_exact_application_bytes_without_system_overlay(self):
        before = self.source.read_bytes()
        report = overlay.prepare_overlay(self.source, self.output, [self.game, self.batch])
        with zipfile.ZipFile(self.output) as archive:
            self.assertEqual(set(archive.namelist()), {self.game + "c3.exe", self.game + "textures.dat", self.batch})
            self.assertIsNone(archive.testzip())
            for name in archive.namelist():
                self.assertEqual(archive.read(name), self.files[name])
        self.assertEqual(self.source.read_bytes(), before)
        self.assertIn("etc/ld.so.cache", report["omitted"])

    def test_output_is_reproducible_and_existing_files_are_preserved(self):
        other = self.root / "other.zip"
        first = overlay.prepare_overlay(self.source, self.output, [self.batch, self.game])
        second = overlay.prepare_overlay(self.source, other, [self.game, self.batch])
        self.assertEqual(first["output"]["sha256"], second["output"]["sha256"])
        with self.assertRaises(FileExistsError):
            overlay.prepare_overlay(self.source, self.output, [self.game])
        self.assertEqual(self.output.read_bytes(), other.read_bytes())

    def test_misspelled_or_whole_prefix_selection_is_rejected(self):
        for name in [overlay.DRIVE, self.game + "missing/", "opt/wine/", self.game + "../", self.game + "C:/"]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                overlay.prepare_overlay(self.source, self.output, [name])
        self.assertFalse(self.output.exists())

    def test_traversal_and_link_payloads_inside_selected_folder_are_rejected(self):
        for suffix in ["../outside.exe", "redirect.link", "old.dll.deleted", "bad\\path"]:
            name = self.game + suffix
            self.files[name] = b"bad"
            self.create()
            with self.subTest(name=name), self.assertRaises(ValueError):
                overlay.prepare_overlay(self.source, self.output, [self.game])
            del self.files[name]
        self.assertFalse(self.output.exists())

    def test_empty_selection_is_rejected(self):
        with self.assertRaises(ValueError):
            overlay.prepare_overlay(self.source, self.output, [])


if __name__ == "__main__":
    unittest.main()
