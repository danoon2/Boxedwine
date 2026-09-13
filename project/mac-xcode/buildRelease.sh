#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_dir"
case "${1:-}" in
    "") [ "$#" -eq 0 ] ;;
    --skip-dependencies) [ "$#" -eq 1 ] ;;
    *) echo "Usage: buildRelease.sh [--skip-dependencies]" >&2; exit 2 ;;
esac

mkdir -p bin
# Fail early if this branch's exact catalog cannot be fetched or reused. The
# bundle phase verifies the same pin again before signing the app.
catalog_offline=
if [ "${BOXEDWINE_CATALOG_OFFLINE:-NO}" = YES ]; then catalog_offline=--offline; fi
/usr/bin/python3 ../../tools/demo_catalog.py stage --required \
    --output "$project_dir/bin/release-demo-catalog" ${catalog_offline:+"$catalog_offline"}
rm -rf bin/Boxedwine.app bin/native-release.xcarchive
if [ "${1:-}" != --skip-dependencies ]; then
    /bin/sh fetchDepends.sh
fi

# Jenkins ships the catalog and downloads Wine on demand. The explicit empty
# setting also prevents a worker's environment from bundling a local Wine ZIP.
unset BOXEDWINE_WINE_ZIP
xcodebuild archive -workspace Boxedwine.xcworkspace \
    -configuration Release -scheme BoxedwineUI \
    -destination 'generic/platform=macOS' \
    -archivePath "$project_dir/bin/native-release.xcarchive" \
    BOXEDWINE_WINE_ZIP= BOXEDWINE_REQUIRE_DEMO_CATALOG=YES

/usr/bin/ditto "bin/native-release.xcarchive/Products/Applications/Boxedwine.app" bin/Boxedwine.app
if [ -e bin/Boxedwine.app/Contents/Resources/WindowsSupport/wine.zip ]; then
    echo "error: The Jenkins app must not contain a Wine filesystem ZIP." >&2
    exit 1
fi
/usr/bin/python3 Boxedwine/BoxedwineUI/Tools/audit-native-bundle.py bin/Boxedwine.app \
    --configuration Release --require-demo-catalog --json bin/native-build-audit.json
