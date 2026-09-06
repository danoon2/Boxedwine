#!/bin/bash
set -e

cd "$(dirname "$0")"

# Like libGL, the direct int99 stubs must preserve the caller's stack frame.
gcc -O2 -c -Wall -Werror -Wno-return-type -Wno-array-parameter -fpic -fno-stack-protector -m32 -march=i586 gl.c
ld -m elf_i386 -shared -soname libGLESv2.so.2 -o libGLESv2.so.2 gl.o

gcc -O2 -c -Wall -Werror -Wno-return-type -Wno-array-parameter -fpic -fno-stack-protector -m32 -march=i586 egl.c
ld -m elf_i386 -shared -soname libEGL.so.1 -o libEGL.so.1 egl.o

ln -sf libGLESv2.so.2 libGLESv2.so
ln -sf libGLESv2.so.2 libGLESv1_CM.so.1
ln -sf libGLESv2.so.2 libGLESv1_CM.so
ln -sf libGLESv2.so.2 libgles.so
ln -sf libEGL.so.1 libEGL.so
