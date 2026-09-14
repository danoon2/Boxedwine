#!/bin/sh
set -eu

# SDL's vendored framework contains a nested hidapi framework. Sign inside out;
# Xcode's Code Sign On Copy only signs the enclosing SDL framework.
framework_dir="$TARGET_BUILD_DIR/$FRAMEWORKS_FOLDER_PATH"
/bin/sh "$SRCROOT/../remove-mesa-libraries.sh"
# Former helper names must not survive in an incremental application bundle.
rm -rf "$TARGET_BUILD_DIR/$CONTENTS_FOLDER_PATH/Helpers/BoxedwineRuntime.app" \
    "$TARGET_BUILD_DIR/$CONTENTS_FOLDER_PATH/Helpers/Boxedwine.app"
# Older builds used Boxedwine.app for the emulator. The launcher now occupies
# that output path, with a distinct executable so debugger attachment still works.
rm -f "$TARGET_BUILD_DIR/$CONTENTS_FOLDER_PATH/MacOS/Boxedwine"
sdl_framework="$framework_dir/SDL2.framework"
hidapi_framework="$sdl_framework/Versions/A/Frameworks/hidapi.framework"
if [ "${CODE_SIGNING_ALLOWED:-YES}" != NO ] && [ -d "$hidapi_framework" ]; then
    /usr/bin/codesign --force --sign "${EXPANDED_CODE_SIGN_IDENTITY:--}" --options runtime "$hidapi_framework"
    /usr/bin/codesign --force --sign "${EXPANDED_CODE_SIGN_IDENTITY:--}" --options runtime "$sdl_framework"
fi

resource_dir="$TARGET_BUILD_DIR/$UNLOCALIZED_RESOURCES_FOLDER_PATH"
mkdir -p "$resource_dir"
/usr/bin/ditto "$SRCROOT/../../../license.txt" "$resource_dir/Boxedwine-LICENSE.txt"
package_check() {
    CLANG_MODULE_CACHE_PATH="$TARGET_TEMP_DIR/PackageCheckModules" \
    SWIFTPM_MODULECACHE_OVERRIDE="$TARGET_TEMP_DIR/PackageCheckModules" \
    /usr/bin/xcrun swift run --package-path "$SRCROOT/BoxedwineUI" \
        --scratch-path "$TARGET_TEMP_DIR/PackageCheck" --disable-sandbox \
        BoxedwinePackageCheck "$@"
}
set --
if [ "${BOXEDWINE_REQUIRE_DEMO_CATALOG:-NO}" = YES ]; then set -- "$@" --required; fi
if [ "${BOXEDWINE_CATALOG_OFFLINE:-NO}" = YES ]; then set -- "$@" --offline; fi
/usr/bin/python3 "$SRCROOT/../../../tools/demo_catalog.py" stage --output "$resource_dir/Demos" "$@"
if [ -f "$resource_dir/Demos/catalog.xml" ]; then
    package_check --catalog "$resource_dir/Demos/catalog.xml"
fi
package_check --wine-catalog "$SRCROOT/BoxedwineUI/Resources/WindowsSupport"
/usr/bin/ditto "$SRCROOT/BoxedwineUI/Resources/WindowsSupport" "$resource_dir/WindowsSupport"
/usr/bin/ditto "$SRCROOT/BoxedwineUI/Resources/AppIcons" "$resource_dir/AppIcons"

# A distributor must audit the exact Wine package and provide its corresponding
# sources before shipping. Never download or choose a user's package implicitly.
if [ -n "${BOXEDWINE_WINE_ZIP:-}" ]; then
    if [ ! -f "$BOXEDWINE_WINE_ZIP" ]; then
        echo "error: BOXEDWINE_WINE_ZIP does not name an existing file" >&2
        exit 1
    fi
    package_check "$BOXEDWINE_WINE_ZIP"
    mkdir -p "$resource_dir/WindowsSupport"
    /usr/bin/ditto "$BOXEDWINE_WINE_ZIP" "$resource_dir/WindowsSupport/wine.zip"
else
    # Avoid accidentally retaining a package from an earlier incremental build.
    rm -f "$resource_dir/WindowsSupport/wine.zip"
fi

# Declared as a build-phase output so Xcode re-signs the containing app after
# either the nested framework signature or the optional Wine package changes.
printf 'Native bundle resources prepared.\n' > "$resource_dir/BundlePreparation.txt"
