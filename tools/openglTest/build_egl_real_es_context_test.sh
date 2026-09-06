#!/bin/bash
set -e

cd "$(dirname "$0")"
bash ../opengles/buildgles.sh
(cd ../opengl && bash buildgl.sh)
mkdir -p Win32/Release/lib
cp ../opengles/libEGL.so.1 Win32/Release/lib/libEGL.so.1
cp ../opengles/libGLESv2.so.2 Win32/Release/lib/libGLESv2.so.2
cp ../opengl/libGL.so.1 Win32/Release/lib/libGL.so.1
ln -sf libEGL.so.1 Win32/Release/lib/libEGL.so
ln -sf libGLESv2.so.2 Win32/Release/lib/libGLESv2.so

gcc -m32 -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -Werror \
    -Wl,-dynamic-linker,/lib/ld-linux.so.2 \
    -Wl,-rpath,'$ORIGIN/lib' \
    -Wl,--allow-shlib-undefined \
    -LWin32/Release/lib \
    EGLRealESContextTest.c -lEGL -lGLESv2 \
    -Wl,--no-as-needed -l:libdl.so.2 -lc \
    -o Win32/Release/EGLRealESContextTest
