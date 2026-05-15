#!/bin/bash
set -e

gcc -c -Wall -Werror -Wno-return-type -Wno-array-parameter -fpic -fno-stack-protector -m32 -march=i586 gl.c
ld -m elf_i386 -shared -soname libGL.so.1 -o libGL.so.1 gl.o
