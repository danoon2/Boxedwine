#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
configuration=Debug
if [ "${1:-}" = --configuration ]; then
    configuration=${2:-}
    [ "$#" -ge 2 ] || { echo "--configuration requires Debug, Sandbox, or Release" >&2; exit 2; }
    shift 2
fi
case "$configuration" in
    Debug|Sandbox|Release) ;;
    *) echo "--configuration requires Debug, Sandbox, or Release" >&2; exit 2 ;;
esac
if [ "${1:-}" = --help ]; then
    echo 'Usage: buildNative.sh [--configuration Debug|Sandbox|Release] [Wine-filesystem.zip|""] [Xcode build settings…]'
    echo 'Builds a local preview, then audits the resulting bundle. No upload or distribution signing.'
    exit 0
fi
wine_zip=${1:-}
if [ "$#" -gt 0 ]; then shift; fi
if [ -n "$wine_zip" ]; then
    wine_zip=$(CDPATH= cd -- "$(dirname -- "$wine_zip")" && pwd)/$(basename -- "$wine_zip")
fi

# Keep the output location/configuration fixed so the post-build audit always
# inspects the app just built. Xcode build-setting overrides remain available.
for setting in "$@"; do
    case "$setting" in
        -*|CONFIGURATION=*|CONFIGURATION_BUILD_DIR=*|TARGET_BUILD_DIR=*|BUILT_PRODUCTS_DIR=*|SYMROOT=*|BUILD_DIR=*|PRODUCT_NAME=*|WRAPPER_NAME=*)
            echo "Unsupported output/action override: $setting. Use Xcode directly and run audit-native-bundle.py on its output." >&2
            exit 2 ;;
        *=*) ;;
        *) echo "Expected an Xcode build setting (KEY=value): $setting" >&2; exit 2 ;;
    esac
done

if [ ! -d "$project_dir/../../lib/mac/SDL2.framework" ]; then
    echo "Install the Mac dependencies described in BUILD.md first." >&2
    exit 1
fi

xcodebuild build \
    -project "$project_dir/Boxedwine/Boxedwine.xcodeproj" \
    -scheme BoxedwineUI -configuration "$configuration" \
    -destination 'platform=macOS' \
    -derivedDataPath "$project_dir/bin/native-build" \
    "BOXEDWINE_WINE_ZIP=$wine_zip" \
    "$@"

/usr/bin/python3 "$project_dir/Boxedwine/BoxedwineUI/Tools/audit-native-bundle.py" \
    "$project_dir/bin/native-build/Build/Products/$configuration/Boxedwine.app" \
    --configuration "$configuration" \
    --json "$project_dir/bin/native-build/native-audit-$configuration.json"
