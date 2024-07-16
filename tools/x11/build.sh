#!/bin/bash
gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 X11.c
gcc -Wl,-soname,libX11.so.6.4.0 -shared -m32 -o libX11.so.6.4.0 X11.o

gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 X11ext.c
gcc -Wl,-soname,libXext.so.6 -shared -m32 -o libXext.so.6 X11ext.o