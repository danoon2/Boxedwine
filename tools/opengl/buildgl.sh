#!/bin/bash
gcc -O2 -c -Wall -Werror -Wno-return-type -Wno-array-parameter -fpic -m32 -march=i586 gl.c
gcc -Wl,-soname,libGL.so.1 -shared -m32 -o libGL.so.1 gl.o
