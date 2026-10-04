#!/bin/bash
# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail

architecture="$1"
revision="$2"
jobs="$3"
cd /tmp/boxedwine/project/linux

if [ "$(dpkg --print-architecture)" != "$architecture" ]; then
    echo "Container architecture does not match requested $architecture build." >&2
    exit 1
fi

# The input archive excludes host objects, libraries and staged executables.
# Both executables must be linked against the container's Ubuntu libraries.
make release JOBS="$jobs"
make native-runtime JOBS="$jobs"
make test-ui
python3 ui/build.py --console Build/Release/boxedwine
python3 package_deb.py --architecture "$architecture" --revision "$revision" --output /output
cp -a Build/NativeUI /output/portable
