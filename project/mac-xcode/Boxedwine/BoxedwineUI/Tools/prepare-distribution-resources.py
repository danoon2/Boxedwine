#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Prepare edition-specific resources. This tool never uses the network."""
import argparse
import hashlib
import json
import plistlib
import shutil
import xml.etree.ElementTree as ET
from pathlib import Path

SOURCE = Path(__file__).resolve().parents[1] / "Resources"
DIRECT = "BOXEDWINE_DIRECT"
STORE = "BOXEDWINE_APP_STORE"


def store_catalog(source=SOURCE):
    catalog = ET.parse(source / "WindowsSupport/filesV2.xml")
    entries = [e for e in catalog.findall("Wine") if e.findtext("WineVersion") == "11.0"]
    if len(entries) != 1:
        raise ValueError("Expected exactly one pinned Wine 11.0 package")
    entry = entries[0]
    url = entry.findtext("FileURL").replace("http://", "https://", 1)
    pin = json.loads((source / "WindowsSupport/packages.json").read_text())[url]
    if pin["filesystemVersion"] != "11":
        raise ValueError("The Store edition requires filesystem version 11")
    root = ET.Element("XML")
    root.append(entry)
    return ET.tostring(root, encoding="utf-8", xml_declaration=True), {url: pin}


def verify_store_wine(wine, source=SOURCE):
    _, pins = store_catalog(source)
    pin = next(iter(pins.values()))
    if wine is None or not wine.is_file() or wine.stat().st_size != pin["bytes"]:
        raise ValueError("App Store builds require the exact pinned Wine ZIP; set BOXEDWINE_WINE_ZIP")
    sha = hashlib.sha256()
    with wine.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    if sha.hexdigest() != pin["sha256"]:
        raise ValueError("The included Wine ZIP checksum differs from the project pin")


def privacy(variant, source=SOURCE):
    result = plistlib.loads((source / "PrivacyInfo.xcprivacy").read_bytes())
    if variant == STORE:
        # The Store launcher has no downloader, analytics, accounts, or telemetry.
        result["NSPrivacyCollectedDataTypes"] = []
    return result


def prepare(output, variant, wine=None, source=SOURCE):
    if variant not in (DIRECT, STORE):
        raise ValueError("Unknown Boxedwine build variant: " + variant)
    if variant == STORE:
        verify_store_wine(wine, source)
    output.mkdir(parents=True, exist_ok=True)
    support = output / "WindowsSupport"
    # Removing this directory also prevents stale downloads/pins surviving when
    # a developer switches editions in one DerivedData directory.
    if support.exists():
        shutil.rmtree(support)
    shutil.copytree(source / "WindowsSupport", support)
    if variant == STORE:
        xml, pins = store_catalog(source)
        (support / "filesV2.xml").write_bytes(xml)
        (support / "packages.json").write_text(json.dumps(pins, indent=2) + "\n")
        demos = output / "Demos"
        if demos.exists():
            shutil.rmtree(demos)
    if wine is not None:
        shutil.copyfile(wine, support / "wine.zip")
    (output / "PrivacyInfo.xcprivacy").write_bytes(plistlib.dumps(privacy(variant, source)))
    (output / "Distribution.plist").write_bytes(plistlib.dumps({"variant": variant}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--variant", choices=(DIRECT, STORE), required=True)
    parser.add_argument("--wine", type=Path)
    args = parser.parse_args()
    try:
        prepare(args.output, args.variant, args.wine)
    except (OSError, ValueError, KeyError, ET.ParseError) as error:
        parser.exit(1, "error: " + str(error) + "\n")
