#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Offline tests of catalog pinning, cache, ZIP handling and build failure policy."""

import contextlib
from concurrent.futures import ThreadPoolExecutor
import io
import json
from pathlib import Path
import stat
import subprocess
import sys
import tempfile
import unittest
from unittest import mock
import zipfile

import demo_catalog as catalog


class CatalogTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.base = Path(self.temporary.name)
        self.cache = self.base / "cache"
        self.output = self.base / "output"
        self.lock = self.base / "lock.json"
        self.source = self.base / "source"
        self.source.mkdir()
        self.archive = self.base / "catalog.zip"
        self.make_package()

    def make_package(self, version="test-1"):
        (self.source / "catalog.xml").write_text(
            f'<XML schemaVersion="7" release="{version}"><Demo><ID>test</ID><Icon>test.png</Icon></Demo></XML>')
        (self.source / "test.png").write_bytes(catalog.PNG + b"fixture")
        with contextlib.redirect_stdout(io.StringIO()):
            catalog.pack(self.source, version, "https://boxedwine.org/catalogs/" + version + ".zip", self.archive, self.lock)
        self.pin = catalog.read_pin(self.lock)
        self.data = self.archive.read_bytes()

    def seed(self):
        catalog.atomic_write(self.cache / (self.pin["sha256"] + ".zip"), self.data)

    def stage(self, **options):
        with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            return catalog.stage(self.pin, self.cache, self.output, **options)

    def changed_zip(self, entries):
        stream = io.BytesIO()
        with zipfile.ZipFile(stream, "w") as archive:
            for name, data in entries:
                archive.writestr(name, data)
        data = stream.getvalue()
        return data, {**self.pin, "bytes": len(data), "sha256": catalog.digest(data)}

    def test_pack_is_repeatable_and_does_not_rewrite_source(self):
        before = {p.name: p.read_bytes() for p in self.source.iterdir()}
        pin, data = self.pin, self.data
        self.make_package()
        self.assertEqual((self.pin, self.data), (pin, data))
        self.assertEqual({p.name: p.read_bytes() for p in self.source.iterdir()}, before)

    def test_exact_cache_is_used_offline_and_bundle_audits(self):
        self.seed()
        with mock.patch.object(catalog, "download", side_effect=AssertionError("Network used")):
            self.assertTrue(self.stage(required=True, offline=True))
        report = catalog.audit_staged(self.output, self.pin)
        self.assertEqual((report["demos"], report["icons"], report["sha256"]), (1, 1, self.pin["sha256"]))

    def test_other_branch_cache_is_never_a_fallback(self):
        self.seed()
        self.assertTrue(self.stage(offline=True))
        old_pin = self.pin
        self.make_package("test-2")
        self.assertFalse(self.stage(offline=True))
        self.assertFalse(self.output.exists())
        self.assertTrue((self.cache / (old_pin["sha256"] + ".zip")).exists())
        with self.assertRaisesRegex(ValueError, "test-2 is unavailable"):
            self.stage(required=True, offline=True)
        self.seed()
        self.assertTrue(self.stage(required=True, offline=True))
        self.pin = old_pin
        self.assertTrue(self.stage(required=True, offline=True))

    def test_successful_download_is_verified_and_reused(self):
        def download(pin, destination):
            destination.write_bytes(self.data)
        with mock.patch.object(catalog, "download", side_effect=download) as transfer:
            self.assertTrue(self.stage(required=True))
            self.assertTrue(self.stage(required=True))
            transfer.assert_called_once()
        self.assertEqual((self.cache / (self.pin["sha256"] + ".zip")).read_bytes(), self.data)
        self.assertFalse(list(self.cache.glob(".download-*")))

    def test_bad_downloads_are_not_cached_or_staged(self):
        for data in (b"", self.data[:-1], self.data + b"extra", b"x" * len(self.data)):
            with self.subTest(size=len(data)), mock.patch.object(catalog, "download", side_effect=lambda pin, path: path.write_bytes(data)):
                self.assertFalse(self.stage())
                self.assertFalse(self.output.exists())
                self.assertFalse(list(self.cache.glob("*.zip")))
                self.assertFalse(list(self.cache.glob(".download-*")))

    def test_corrupt_cache_is_detected_and_can_be_repaired(self):
        self.seed()
        (self.cache / (self.pin["sha256"] + ".zip")).write_bytes(b"bad")
        self.assertFalse(self.stage(offline=True))
        with mock.patch.object(catalog, "download", side_effect=lambda pin, path: path.write_bytes(self.data)):
            self.assertTrue(self.stage(required=True))
        catalog.audit_staged(self.output, self.pin)

    def test_concurrent_builds_publish_only_complete_cache_files(self):
        def download(pin, destination):
            destination.write_bytes(self.data)
        with mock.patch.object(catalog, "download", side_effect=download):
            with ThreadPoolExecutor(max_workers=4) as workers:
                results = list(workers.map(lambda _: catalog.resolve(self.pin, self.cache), range(8)))
        self.assertTrue(all(files == results[0] for files in results))
        catalog.read_zip(self.cache / (self.pin["sha256"] + ".zip"), self.pin)
        self.assertEqual(len(list(self.cache.iterdir())), 1)

    def test_network_failure_or_timeout_obeys_build_policy(self):
        for error in (OSError("offline"), ValueError("HTTP 404"), subprocess.TimeoutExpired("download", 20)):
            with self.subTest(error=str(error)), mock.patch.object(catalog, "download", side_effect=error):
                self.assertFalse(self.stage())
                with self.assertRaises(ValueError):
                    self.stage(required=True)
                self.assertFalse(self.output.exists())

    def test_transfer_has_a_total_timeout(self):
        with mock.patch.object(catalog.subprocess, "run", side_effect=subprocess.TimeoutExpired("download", 20)) as run:
            with self.assertRaises(subprocess.TimeoutExpired):
                catalog.download(self.pin, self.archive)
            self.assertEqual(run.call_args.kwargs["timeout"], 20)

    def test_lock_rejects_unsafe_urls_and_malformed_fields(self):
        for change in ({"formatVersion": True}, {"formatVersion": 2}, {"version": "../bad"}, {"bytes": True},
                       {"bytes": 0}, {"sha256": "123"}, {"url": "http://boxedwine.org/test.zip"},
                       {"url": "https://boxedwine.org.evil.example/test.zip"}, {"url": "https://user@boxedwine.org/test.zip"},
                       {"url": "https://boxedwine.org:444/test.zip"}, {"url": "https://boxedwine.org/test.zip?q=1"},
                       {"url": "https://boxedwine.org/test.zip#fragment"}):
            with self.subTest(change=change), self.assertRaises(ValueError):
                catalog.validate_pin({**self.pin, **change})

    def test_redirects_cannot_leave_allowed_https_hosts(self):
        handler = catalog.CatalogRedirect()
        with self.assertRaises(ValueError):
            handler.redirect_request(None, None, 302, "", {}, "https://example.com/catalog.zip")
        with self.assertRaises(ValueError):
            handler.redirect_request(None, None, 302, "", {}, "http://boxedwine.org/catalog.zip")

    def test_zip_rejects_paths_links_duplicates_and_expansion(self):
        files = [(p.name, p.read_bytes()) for p in self.source.iterdir()]
        link = zipfile.ZipInfo("link.png")
        link.create_system = 3
        link.external_attr = (stat.S_IFLNK | 0o777) << 16
        for extra in ([("../escape.png", catalog.PNG)], [("/escape.png", catalog.PNG)], [("C:\\escape.png", catalog.PNG)],
                      [("nested/test.png", catalog.PNG)], [(link, b"test.png")], [("test.png", catalog.PNG)],
                      [("TEST.png", catalog.PNG)], [("large.png", b"x" * (catalog.MAX_FILE + 1))]):
            with self.subTest(extra=str(extra[0][0])):
                with contextlib.redirect_stderr(io.StringIO()):
                    data, pin = self.changed_zip(files + extra)
                with self.assertRaises(ValueError):
                    catalog.unpack(data, pin)
        with mock.patch.object(catalog, "MAX_EXPANDED", 8), self.assertRaises(ValueError):
            catalog.unpack(self.data, self.pin)

    def test_catalog_checks_release_xml_and_icon_references(self):
        files = {p.name: p.read_bytes() for p in self.source.iterdir()}
        xml = files["catalog.xml"]
        for altered in (xml.replace(b"test-1", b"test-2"), xml.replace(b"test.png", b"missing.png"),
                        b"<!DOCTYPE XML>" + xml, xml[:-10], xml.replace(b"</Demo>", b"<ID>duplicate</ID></Demo>")):
            with self.subTest(xml=altered), self.assertRaises(ValueError):
                catalog.validate_files({**files, "catalog.xml": altered}, "test-1")
        with self.assertRaises(ValueError):
            catalog.validate_files({**files, "test.png": b"not png"}, "test-1")

    def test_bundle_audit_rejects_changes_and_wrong_pin(self):
        self.seed()
        self.stage(offline=True)
        with self.assertRaises(ValueError):
            catalog.audit_staged(self.output, {**self.pin, "sha256": "a" * 64})
        (self.output / "test.png").write_bytes(catalog.PNG + b"changed")
        with self.assertRaises(ValueError):
            catalog.audit_staged(self.output, self.pin)

    def test_cli_local_succeeds_and_strict_fails_without_a_cached_package(self):
        command = [sys.executable, str(Path(catalog.__file__)), "stage", "--lock", str(self.lock),
                   "--cache", str(self.cache), "--output", str(self.output), "--offline"]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("warning:", result.stderr)
        self.assertFalse(self.output.exists())
        result = subprocess.run(command + ["--required"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("error:", result.stderr)


if __name__ == "__main__":
    unittest.main()
