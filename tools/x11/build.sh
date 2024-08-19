#!/bin/bash
gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 X11.c
gcc -Wl,-soname,libX11.so.6.4.0 -shared -m32 -o libX11.so.6.4.0 X11.o

gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 X11ext.c
gcc -Wl,-soname,libXext.so.6 -shared -m32 -o libXext.so.6 X11ext.o

gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 Xinerama.c
gcc -Wl,-soname,libXinerama.so.1 -shared -m32 -o libXinerama.so.1 Xinerama.o

gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 Xrandr.c
gcc -Wl,-soname,libXrandr.so.2 -shared -m32 -o libXrandr.so.2 Xrandr.o

gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 Xrenderer.c
gcc -Wl,-soname,libXrender.so.1 -shared -m32 -o libXrender.so.1 Xrenderer.o

gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 Xinput2.c
gcc -Wl,-soname,libXi.so.6.1.0 -shared -m32 -o libXi.so.6.1.0 Xinput2.o
