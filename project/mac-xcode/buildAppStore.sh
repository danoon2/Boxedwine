#!/bin/sh
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

usage() {
    echo 'Usage: buildAppStore.sh [--skip-dependencies] Wine-filesystem.zip Team-ID'
    echo 'Archives with the pinned Wine 11 package and automatic Apple Development signing.'
    echo 'Open the archive in Xcode Organizer to Validate App or distribute it.'
}
skip_dependencies=NO
if [ "${1:-}" = --help ]; then usage; exit 0; fi
if [ "${1:-}" = --skip-dependencies ]; then skip_dependencies=YES; shift; fi
if [ "$#" -ne 2 ]; then usage >&2; exit 2; fi
wine_zip=$(CDPATH= cd -- "$(dirname -- "$1")" && pwd)/$(basename -- "$1")
team_id=$2
case "$team_id" in
    ''|*[!A-Z0-9]*) echo 'error: Expected an Apple Developer Team ID.' >&2; exit 2 ;;
esac
if [ "${#team_id}" -ne 10 ]; then
    echo 'error: Expected a 10-character Apple Developer Team ID.' >&2
    exit 2
fi
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_dir"
support_dir="$project_dir/Boxedwine/BoxedwineUI/Resources/WindowsSupport"

# Fail before building if this is not the exact Wine 11 package pinned on this branch.
/usr/bin/python3 - "$support_dir" "$wine_zip" <<'PY'
import hashlib
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

try:
    support, wine = map(Path, sys.argv[1:])
    catalog = ET.parse(support / "filesV2.xml")
    entries = [e for e in catalog.findall("Wine") if e.findtext("WineVersion") == "11.0"]
    if len(entries) != 1:
        raise ValueError("Expected exactly one Wine 11.0 catalog entry")
    url = entries[0].findtext("FileURL").replace("http://", "https://", 1)
    pin = json.loads((support / "packages.json").read_text())[url]
    if wine.stat().st_size != pin["bytes"]:
        raise ValueError(f"Wine ZIP size differs from the pinned package: {url}")
    sha = hashlib.sha256()
    with wine.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    if sha.hexdigest() != pin["sha256"]:
        raise ValueError(f"Wine ZIP checksum differs from the pinned package: {url}")
    print(f"Verified pinned Wine 11.0: {pin['bytes']} bytes, SHA-256 {pin['sha256']}")
except (OSError, ValueError, KeyError, AttributeError, ET.ParseError) as error:
    sys.exit(f"error: {error}")
PY

if [ "$skip_dependencies" != YES ]; then /bin/sh fetchDepends.sh; fi
output_dir="$project_dir/bin/app-store"
mkdir -p "$output_dir"
archive_path="$output_dir/Boxedwine-$(date '+%Y%m%d-%H%M%S').xcarchive"

# Keep the Jenkins Release defaults; Store signing is scoped to this archive.
# Xcode Organizer re-signs for Store distribution during validation/export.
# The prebuilt SDL/hidapi and MoltenVK packages omit dSYMs, so Organizer can
# warn about their symbols. The UI and emulator dSYMs remain in the archive.
xcodebuild archive -project Boxedwine/Boxedwine.xcodeproj \
    -scheme BoxedwineUI -configuration Release \
    -destination 'generic/platform=macOS' \
    -derivedDataPath "$project_dir/bin/app-store-build" \
    -archivePath "$archive_path" -allowProvisioningUpdates \
    CODE_SIGN_STYLE=Automatic DEVELOPMENT_TEAM="$team_id" \
    'CODE_SIGN_IDENTITY=Apple Development' \
    BOXEDWINE_BUILD_VARIANT=BOXEDWINE_APP_STORE BOXEDWINE_REQUIRE_DEMO_CATALOG=NO BOXEDWINE_WINE_ZIP="$wine_zip"

app_path="$archive_path/Products/Applications/Boxedwine.app"
audit_path="$archive_path/bundle-audit.json"
/usr/bin/python3 Boxedwine/BoxedwineUI/Tools/audit-native-bundle.py "$app_path" \
    --configuration Release --app-store --json "$audit_path"
if ! cmp -s "$wine_zip" "$app_path/Contents/Resources/WindowsSupport/wine.zip"; then
    echo 'error: The archive does not contain the verified Wine package.' >&2
    exit 1
fi

/usr/bin/python3 - "$audit_path" "$team_id" <<'PY'
import json
import subprocess
import sys
from pathlib import Path

report = json.loads(Path(sys.argv[1]).read_text())
team = sys.argv[2]
for entry in report["images"]:
    path = Path(report["bundle"]) / entry["path"]
    result = subprocess.run(["/usr/bin/codesign", "-d", "--verbose=4", str(path)], capture_output=True, text=True)
    if result.returncode or f"TeamIdentifier={team}" not in result.stderr.splitlines():
        sys.exit(f"error: Archive code must be signed by team {team}: {path}")
print("Archive signing team verified for every embedded Mach-O image.")
PY

echo "Archive ready for Xcode Organizer: $archive_path"
