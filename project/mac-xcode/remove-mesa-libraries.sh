#!/bin/sh
set -eu

# Xcode can leave files from removed Copy Files entries in incremental products.
# Restrict cleanup to the known retired libraries in the app being built.
framework_dir="$TARGET_BUILD_DIR/$FRAMEWORKS_FOLDER_PATH"
case "$framework_dir" in
    /*.app/Contents/Frameworks) ;;
    *) echo "error: Unexpected app Frameworks directory: $framework_dir" >&2; exit 1 ;;
esac
for library in \
    libOSMesa.8.dylib libglapi.0.dylib libLLVM.dylib \
    libffi.8.dylib libedit.0.dylib libncurses.6.dylib \
    libz.1.dylib libzstd.1.dylib libxml2.2.dylib liblzma.5.dylib \
    libiconv.2.dylib libicuuc.76.dylib libicudata.76.dylib \
    libc++.1.dylib libc++abi.1.dylib; do
    rm -f "$framework_dir/$library"
done

# The old automation target copied these four files beside its executable.
for library in libOSMesa.8.dylib libglapi.0.dylib libc++.1.dylib libc++abi.1.dylib; do
    rm -f "${framework_dir%/Frameworks}/MacOS/$library"
done
