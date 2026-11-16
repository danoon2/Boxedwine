#!/bin/bash
set -e
cd "$(dirname "$0")"
bash build_egl_real_es_context_test.sh
for test in EGLWindowResizeTest EGLSwapIntervalTest; do
gcc -m32 -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -Werror \
    -Wl,-dynamic-linker,/lib/ld-linux.so.2 -Wl,-rpath,'$ORIGIN/lib' \
    -Wl,--allow-shlib-undefined -LWin32/Release/lib \
    "$test.c" -lEGL -l:libGL.so.1 -lX11 \
    -Wl,--no-as-needed -l:libdl.so.2 -lc \
    -o "Win32/Release/$test"
done
