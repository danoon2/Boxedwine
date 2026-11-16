#!/bin/bash
set -e
cd "$(dirname "$0")"
bash build_egl_real_es_context_test.sh
gcc -O2 -m32 -Wall -Wextra -Werror EGLCallingConventionTest.c \
    -LWin32/Release/lib -Wl,-rpath,'$ORIGIN/lib' \
    -lEGL -lGLESv2 -ldl -o Win32/Release/EGLCallingConventionTest
LD_LIBRARY_PATH="$PWD/Win32/Release/lib" Win32/Release/EGLCallingConventionTest
