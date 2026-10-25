#!/bin/sh
# Open Watcom 1.9 Linux host. WATCOM points at an extracted installation.
set -eu
: "${WATCOM:?Set WATCOM to the Open Watcom installation}"
probe_sources=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
probe_output=${1:?Usage: build_copyrect16.sh output-directory}
mkdir -p "$probe_output"
cd "$probe_output"
export PATH="$WATCOM/binl:$PATH"
export INCLUDE="$WATCOM/h:$WATCOM/h/win"
wasm -q -fo=copyrect16-start.obj "$probe_sources/copyrect16-start.asm"
wcc -q -bt=windows -ml -zw -zl -s -fo=copyrect16.obj "$probe_sources/copyrect16.c"
wlink system windows option quiet option nodefaultlibs option map \
    option version=3.10 name RECT16.EXE \
    file copyrect16-start.obj,copyrect16.obj \
    library "$WATCOM/lib286/win/windows.lib"
