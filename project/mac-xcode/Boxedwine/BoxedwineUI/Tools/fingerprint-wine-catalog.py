#!/usr/bin/env python3
"""Record exact fingerprints of downloaded filesV2.xml Wine packages for a release."""
import hashlib
import json
from pathlib import Path
import sys
from urllib.parse import urlsplit, urlunsplit
import xml.etree.ElementTree as ET
import zipfile


def main():
    if len(sys.argv) != 4:
        raise SystemExit("Usage: fingerprint-wine-catalog.py filesV2.xml downloads output.json")
    xml, downloads, output = map(Path, sys.argv[1:])
    data = xml.read_bytes()
    if len(data) > 1024 * 1024 or b"<!DOCTYPE" in data or b"<!ENTITY" in data:
        raise SystemExit("Unsupported XML")
    root = ET.fromstring(data)
    if root.tag != "XML":
        raise SystemExit("Expected filesV2.xml")
    records = {}
    for wine in root.findall("Wine"):
        parts = urlsplit(wine.findtext("FileURL", "").strip())
        if parts.scheme not in ("http", "https") or parts.hostname not in ("boxedwine.org", "www.boxedwine.org") or parts.username or parts.password or parts.query or parts.fragment or parts.port not in (None, 443):
            raise SystemExit("Unsupported package URL")
        url = urlunsplit(parts._replace(scheme="https"))
        path = downloads / Path(parts.path).name
        with zipfile.ZipFile(path) as package:
            if any(package.getinfo(name).file_size > 1024 for name in ("wineVersion.txt", "version.txt")):
                raise SystemExit("Oversized package metadata")
            version = package.read("wineVersion.txt").decode().strip()
            filesystem = package.read("version.txt").decode().strip()
        if version != wine.findtext("WineVersion", "").strip() or url in records:
            raise SystemExit("Package version mismatch or duplicate Wine URL")
        with path.open("rb") as stream:
            digest = hashlib.sha256()
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
        records[url] = {"fileVersion": wine.findtext("FileVersion", "").strip(), "filesystemVersion": filesystem,
                        "bytes": path.stat().st_size, "sha256": digest.hexdigest()}
    if not records:
        raise SystemExit("No Wine entries")
    output.write_text(json.dumps(records, indent=2, sort_keys=True) + "\n")
    print(f"Recorded {len(records)} packages. Run BoxedwinePackageCheck --wine-downloads before shipping.")


if __name__ == "__main__":
    main()
