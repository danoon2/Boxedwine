#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Offline archive/error tests; no fixture library is executed."""
import hashlib
import io
from pathlib import Path
import tarfile
import tempfile
import unittest
from unittest.mock import patch

import fetch_moltenvk as module


class MoltenVKTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.archive = self.root / "package.tar"
        self.destination = self.root / "libMoltenVK.dylib"
        self.data = b"fixture library, never executed"
        self.pin = dict(member="package/libMoltenVK.dylib", libraryBytes=len(self.data),
                        librarySHA256=hashlib.sha256(self.data).hexdigest())
        self.make_archive()

    def make_archive(self, duplicate=False, symlink=False):
        with tarfile.open(self.archive, "w") as archive:
            member = tarfile.TarInfo(self.pin["member"])
            member.size = len(self.data)
            if symlink:
                member.type = tarfile.SYMTYPE
                member.linkname = "/outside"
            archive.addfile(member, io.BytesIO(self.data))
            if duplicate:
                archive.addfile(member, io.BytesIO(self.data))
        self.pin.update(bytes=self.archive.stat().st_size, sha256=module.digest(self.archive))

    def test_verified_library_works_offline_without_cache(self):
        module.install(self.pin, self.destination, self.root / "cache", self.archive)
        self.assertEqual(self.destination.read_bytes(), self.data)
        with patch.object(module.subprocess, "run", side_effect=AssertionError("unexpected network")):
            module.install(self.pin, self.destination, self.root / "cache", offline=True)

    def test_corrupt_archive_preserves_installed_library(self):
        self.destination.write_bytes(b"old library")
        self.archive.write_bytes(b"corrupt")
        with self.assertRaisesRegex(ValueError, "archive does not match"):
            module.install(self.pin, self.destination, self.root / "cache", self.archive)
        self.assertEqual(self.destination.read_bytes(), b"old library")

    def test_library_hash_is_checked_independently(self):
        self.pin["librarySHA256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "library does not match"):
            module.library_bytes(self.archive, self.pin)

    def test_duplicate_and_symlink_members_are_rejected(self):
        for options in (dict(duplicate=True), dict(symlink=True)):
            with self.subTest(options=options):
                self.make_archive(**options)
                with self.assertRaisesRegex(ValueError, "one regular library"):
                    module.library_bytes(self.archive, self.pin)

    def test_missing_offline_archive_preserves_previous_library(self):
        self.destination.write_bytes(b"old library")
        with self.assertRaisesRegex(ValueError, "not cached"):
            module.install(self.pin, self.destination, self.root / "cache", offline=True)
        self.assertEqual(self.destination.read_bytes(), b"old library")

    def test_failed_download_preserves_previous_library(self):
        self.destination.write_bytes(b"old library")
        self.pin["url"] = "https://example.invalid/package.tar"
        with patch.object(module.subprocess, "run", side_effect=OSError("network unavailable")):
            with self.assertRaises(OSError):
                module.install(self.pin, self.destination, self.root / "cache")
        self.assertEqual(self.destination.read_bytes(), b"old library")
        self.assertEqual(list((self.root / "cache").iterdir()), [])


if __name__ == "__main__":
    unittest.main()
