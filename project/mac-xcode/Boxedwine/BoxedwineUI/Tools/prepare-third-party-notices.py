#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build offline license resources from reviewed notices, without network access."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[5]
MANIFEST = Path(__file__).resolve().parents[1] / "Licensing/native-notices.json"
SOURCES = MANIFEST.with_name("native-dependencies.json")


def validate_inputs():
    records = json.loads(SOURCES.read_text())
    inputs = {
        "lib/mac/SDL2.framework/Versions/A/SDL2": records["sdl"]["frameworkSHA256"],
        "lib/mac/SDL2.framework/Versions/A/Frameworks/hidapi.framework/Versions/A/hidapi": records["sdl"]["hidapiSHA256"],
        "lib/mac/vulkan/lib/libMoltenVK.dylib": records["moltenvk"]["librarySHA256"],
    }
    for path, expected in inputs.items():
        actual = hashlib.sha256((ROOT / path).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError("Native dependency changed; update its source record and notices: " + path)
    return records


def document(root=ROOT, manifest=MANIFEST):
    source = json.loads(manifest.read_text())
    components = []
    identifiers = set()
    for component in source["components"]:
        if component["id"] in identifiers:
            raise ValueError("Duplicate third-party component: " + component["id"])
        identifiers.add(component["id"])
        parts = []
        for notice in component["notices"]:
            path = (root / notice["path"]).resolve()
            if root.resolve() not in path.parents:
                raise ValueError("Notice must be inside the source tree")
            text = path.read_text()
            if "lines" in notice:
                start, end = notice["lines"]
                lines = text.splitlines()
                if not 1 <= start <= end <= len(lines):
                    raise ValueError("Invalid notice line range: " + notice["path"])
                text = "\n".join(lines[start - 1:end]) + "\n"
            if not text.strip() or hashlib.sha256(text.encode()).hexdigest() != notice["sha256"]:
                raise ValueError("License text changed; review native-notices.json: " + notice["path"])
            parts.append(text.rstrip())
        # Tiny Core's official website is HTTP-only. This is a displayed link,
        # not a download endpoint; keep HTTPS required for other source links.
        source_url = component["sourceURL"]
        if not parts or not (source_url.startswith("https://") or
                             source_url == "http://www.tinycorelinux.net/"):
            raise ValueError("Component needs notices and a supported project/source URL")
        components.append({key: component[key] for key in
                           ("id", "name", "license", "sourceURL", "sourceDescription")})
        if "sourceLinkTitle" in component:
            components[-1]["sourceLinkTitle"] = component["sourceLinkTitle"]
        components[-1]["text"] = "\n\n".join(parts)
    if not components:
        raise ValueError("Third-party notices are empty")
    return {"schemaVersion": 1, "components": components}


def plain_text(data):
    return "Boxedwine — Third-Party Licenses\n\n" + "\n\n".join(
        f"{c['name']}\n{c['license']}\n{c['sourceURL']}\n{c['sourceDescription']}\n\n{c['text']}"
        for c in data["components"]) + "\n"


def stage(output):
    data = document()
    records = validate_inputs()
    output.mkdir(parents=True, exist_ok=True)
    (output / "notices.json").write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n")
    (output / "Third-Party-Licenses.txt").write_text(plain_text(data))
    (output / "sources.json").write_text(json.dumps(records, indent=2) + "\n")
    return data


def audit(output):
    expected = document()
    if json.loads((output / "notices.json").read_text()) != expected:
        raise ValueError("Bundled third-party notices differ from the reviewed source notices")
    if (output / "Third-Party-Licenses.txt").read_text() != plain_text(expected):
        raise ValueError("Bundled plain-text third-party licenses are incomplete or outdated")
    if json.loads((output / "sources.json").read_text()) != json.loads(SOURCES.read_text()):
        raise ValueError("Bundled native source records are incomplete or outdated")
    return {"components": len(expected["components"]), "manifestSHA256": hashlib.sha256(MANIFEST.read_bytes()).hexdigest()}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--audit", action="store_true")
    args = parser.parse_args()
    try:
        result = audit(args.output) if args.audit else stage(args.output)
    except (OSError, ValueError, KeyError) as error:
        parser.exit(1, f"error: Third-party licenses: {error}\n")
    print("Third-party license resources verified" if args.audit else
          f"Prepared notices for {len(result['components'])} components")
