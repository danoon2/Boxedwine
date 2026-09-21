#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import hashlib
import importlib.util
import json
import plistlib
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from pathlib import Path

sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location("editions", Path(__file__).with_name("prepare-distribution-resources.py"))
editions = importlib.util.module_from_spec(spec)
spec.loader.exec_module(editions)


class DistributionResourcesTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.source = self.root / "source"
        support = self.source / "WindowsSupport"
        support.mkdir(parents=True)
        self.output = self.root / "bundle"
        (self.output / "Demos").mkdir(parents=True)
        (self.output / "Demos/catalog.xml").write_text("old catalog")
        self.wine = self.root / "wine.zip"
        self.wine.write_bytes(b"test package")
        self.pins = {}
        xml = ET.Element("XML")
        for version in ("11.0", "10.0"):
            entry = ET.SubElement(xml, "Wine")
            url = "https://boxedwine.org/" + version + ".zip"
            for key, value in {"WineVersion": version, "FileURL": url}.items():
                ET.SubElement(entry, key).text = value
            self.pins[url] = {"bytes": self.wine.stat().st_size, "sha256": hashlib.sha256(self.wine.read_bytes()).hexdigest(), "filesystemVersion": "12"}
        (support / "filesV2.xml").write_bytes(ET.tostring(xml))
        (support / "packages.json").write_text(json.dumps(self.pins))
        (self.source / "PrivacyInfo.xcprivacy").write_bytes(plistlib.dumps({"NSPrivacyTracking": False, "NSPrivacyCollectedDataTypes": [{"test": "download logs"}]}))

    def prepare(self, variant, wine=None):
        editions.prepare(self.output, variant, wine, self.source)

    def test_store_has_one_wine_no_catalog_and_no_collection(self):
        self.prepare(editions.STORE, self.wine)
        self.assertFalse((self.output / "Demos").exists())
        self.assertEqual((self.output / "WindowsSupport/wine.zip").read_bytes(), self.wine.read_bytes())
        self.assertEqual([e.findtext("WineVersion") for e in ET.parse(self.output / "WindowsSupport/filesV2.xml").findall("Wine")], ["11.0"])
        self.assertEqual(plistlib.loads((self.output / "PrivacyInfo.xcprivacy").read_bytes())["NSPrivacyCollectedDataTypes"], [])

    def test_store_requires_pinned_package_even_if_old_bundle_has_wine(self):
        self.prepare(editions.STORE, self.wine)
        for wine in (None, self.root / "missing.zip"):
            with self.assertRaises(ValueError):
                self.prepare(editions.STORE, wine)
        self.wine.write_bytes(b"fake package")  # Same byte count, different content.
        with self.assertRaisesRegex(ValueError, "checksum"):
            self.prepare(editions.STORE, self.wine)

    def test_switching_back_to_jenkins_restores_download_pins_and_removes_wine(self):
        self.prepare(editions.STORE, self.wine)
        self.prepare(editions.DIRECT)
        self.assertFalse((self.output / "WindowsSupport/wine.zip").exists())
        self.assertEqual(json.loads((self.output / "WindowsSupport/packages.json").read_text()), self.pins)
        self.assertEqual(plistlib.loads((self.output / "PrivacyInfo.xcprivacy").read_bytes()), editions.privacy(editions.DIRECT, self.source))

    def test_direct_keeps_the_catalog_for_the_catalog_staging_phase(self):
        self.prepare(editions.DIRECT)
        self.assertEqual((self.output / "Demos/catalog.xml").read_text(), "old catalog")


if __name__ == "__main__":
    unittest.main()
