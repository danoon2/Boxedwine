#!/usr/bin/env python3
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Generate Xcode's input Info.plist with the emulator's public version.

Both Mac targets use this before ProcessInfoPlistFile. Xcode expands the bundle
settings below, including the separate CURRENT_PROJECT_VERSION build number.
The original C++ definition remains the single source of the public version.
"""

import argparse
from pathlib import Path
import plistlib
import re
import sys


def display_version(header):
    definitions = re.findall(r'^\s*#\s*define[ \t]+BOXEDWINE_VERSION_DISPLAY[ \t]+([^\r\n]+)', header, re.MULTILINE)
    if len(definitions) != 1:
        raise ValueError("Expected one BOXEDWINE_VERSION_DISPLAY definition in boxedwine.h")
    match = re.fullmatch(r'"([0-9]+\.[0-9]+\.[0-9]+)"\s*(?://.*|/\*.*\*/\s*)?', definitions[0])
    if not match:
        raise ValueError("BOXEDWINE_VERSION_DISPLAY must be a quoted major.minor.patch version")
    return match[1]


def prepare(header, output, template=None):
    version = display_version(header.read_text())
    extra = plistlib.loads(template.read_bytes()) if template else {}
    if not isinstance(extra, dict):
        raise ValueError("Info.plist template must contain a dictionary")
    info = {
        "CFBundleDevelopmentRegion": "$(DEVELOPMENT_LANGUAGE)",
        "CFBundleDisplayName": "$(INFOPLIST_KEY_CFBundleDisplayName)",
        "CFBundleExecutable": "$(EXECUTABLE_NAME)",
        "CFBundleIdentifier": "$(PRODUCT_BUNDLE_IDENTIFIER)",
        "CFBundleInfoDictionaryVersion": "6.0",
        "CFBundleName": "$(PRODUCT_NAME)",
        "CFBundlePackageType": "$(PRODUCT_BUNDLE_PACKAGE_TYPE)",
        "CFBundleVersion": "$(CURRENT_PROJECT_VERSION)",
        "LSMinimumSystemVersion": "$(MACOSX_DEPLOYMENT_TARGET)",
    }
    info.update(extra)
    info["CFBundleShortVersionString"] = version
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(plistlib.dumps(info, sort_keys=False))
    return version


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--template", type=Path, help="optional UI-specific bundle declarations")
    args = parser.parse_args()
    try:
        version = prepare(args.header, args.output, args.template)
    except (OSError, ValueError, plistlib.InvalidFileException) as error:
        print(f"error: Could not prepare Boxedwine version metadata: {error}", file=sys.stderr)
        return 1
    print(f"Boxedwine app version: {version} (from {args.header})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
