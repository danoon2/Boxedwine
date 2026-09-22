#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Offline SDL pin and installation failure tests; no native code is executed."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import fetch_sdl as module


class SDLTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.destination = self.root / "SDL2.framework"
        self.destination.mkdir()
        (self.destination / "SDL2").write_bytes(b"old framework")
        self.cache = self.root / "cache"
        self.archive = self.root / "SDL.dmg"
        self.archive.write_bytes(b"fixture disk image, never mounted")
        self.pin = dict(bytes=self.archive.stat().st_size, sha256=module.digest(self.archive),
                        frameworkTreeSHA256="0" * 64)

    def test_matching_install_needs_no_cache_or_mount(self):
        self.pin["frameworkTreeSHA256"] = module.framework_digest(self.destination)
        with patch.object(module.subprocess, "run", side_effect=AssertionError("unexpected command")):
            module.install(self.pin, self.destination, self.cache, offline=True)
        self.assertFalse(self.cache.exists())

    def test_corrupt_archive_preserves_old_framework_without_mounting(self):
        self.archive.write_bytes(b"corrupt")
        with patch.object(module.subprocess, "run", side_effect=AssertionError("unexpected mount")):
            with self.assertRaisesRegex(ValueError, "archive does not match"):
                module.install(self.pin, self.destination, self.cache, self.archive)
        self.assertEqual((self.destination / "SDL2").read_bytes(), b"old framework")

    def test_missing_offline_cache_preserves_old_framework(self):
        with self.assertRaisesRegex(ValueError, "not cached"):
            module.install(self.pin, self.destination, self.cache, offline=True)
        self.assertEqual((self.destination / "SDL2").read_bytes(), b"old framework")

    def test_tree_digest_includes_headers_resources_and_symlink_targets(self):
        original = module.framework_digest(self.destination)
        header = self.destination / "SDL.h"
        header.write_text("header")
        self.assertNotEqual(original, module.framework_digest(self.destination))
        header.unlink()
        link = self.destination / "Headers"
        link.symlink_to("Versions/A/Headers")
        linked = module.framework_digest(self.destination)
        link.unlink()
        link.symlink_to("Versions/B/Headers")
        self.assertNotEqual(linked, module.framework_digest(self.destination))

    def test_failed_replacement_restores_old_framework(self):
        def stage(archive, pin, destination):
            destination.mkdir()
            (destination / "SDL2").write_bytes(b"new framework")

        replace = module.os.replace

        def fail_install(source, destination):
            if Path(source).name == "SDL2.framework":
                if Path(source).parent != self.root:
                    raise OSError("simulated replacement failure")
            return replace(source, destination)

        with patch.object(module, "stage", side_effect=stage), patch.object(module.os, "replace", side_effect=fail_install):
            with self.assertRaisesRegex(OSError, "replacement failure"):
                module.install(self.pin, self.destination, self.cache, self.archive)
        self.assertEqual((self.destination / "SDL2").read_bytes(), b"old framework")

    def test_license_record_matches_build_pin(self):
        pin = json.loads(module.LOCK.read_text())
        path = module.ROOT / "project/mac-xcode/Boxedwine/BoxedwineUI/Licensing/native-dependencies.json"
        record = json.loads(path.read_text())["sdl"]
        for key in ("version", "frameworkSHA256", "frameworkTreeSHA256"):
            self.assertEqual(pin[key], record[key])


if __name__ == "__main__":
    unittest.main()
