#!/bin/sh
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

if [ "$#" -ne 2 ] || [ -z "$2" ]; then
    echo 'Usage: signNative.sh /path/to/Boxedwine.app "Developer ID Application: …"' >&2
    echo 'Use identity "-" only for a local ad-hoc signing check; it cannot be notarized.' >&2
    exit 2
fi
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
app=$(CDPATH= cd -- "$1" && pwd)
identity=$2
native_dir="$project_dir/Boxedwine/BoxedwineUI"

# Validate the layout before signing. In particular, reject broken/escaping
# symlinks and obsolete helpers rather than signing files outside this app.
/usr/bin/python3 "$native_dir/Tools/audit-native-bundle.py" "$app" --configuration Release --skip-signatures --require-demo-catalog
timestamp=--timestamp
if [ "$identity" = - ]; then timestamp=--timestamp=none; fi
sign() {
    /usr/bin/codesign --force --sign "$identity" --options runtime "$timestamp" "$@"
}

# Each executable gets its own entitlements. Libraries need no app entitlements.
# Do not use --deep when signing: it would apply the same options to nested code.
frameworks="$app/Contents/Frameworks"
sign "$frameworks/SDL2.framework/Versions/A/Frameworks/hidapi.framework"
sign "$frameworks/SDL2.framework"
sign "$frameworks/libMoltenVK.dylib"
sign --entitlements "$native_dir/Boxedwine.entitlements" "$app/Contents/Helpers/BoxedwineEngine.app"
sign --entitlements "$native_dir/BoxedwineUI.entitlements" "$app"

if [ "$identity" = - ]; then
    /usr/bin/python3 "$native_dir/Tools/audit-native-bundle.py" "$app" --configuration Release --require-demo-catalog \
        --json "${app%/*}/native-signing-audit.json"
else
    /usr/bin/python3 "$native_dir/Tools/audit-native-bundle.py" "$app" --configuration Release --distribution \
        --json "${app%/*}/native-signing-audit.json"
fi
