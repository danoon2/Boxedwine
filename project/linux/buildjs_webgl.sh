#/bin/sh
emcc -Wall -Wno-unused-function -I../../include ../../source/sdl/*.c ../../platform/linux/*.c ../../source/emulation/*.c ../../source/io/*.c ../../source/kernel/*.c ../../source/kernel/devs/*.c ../../source/kernel/proc/*.c ../../source/kernel/loader/*.c ../../source/util/*.c ../../source/util/pbl/*.c ../../source/opengl/es/*.c ../../source/opengl/*.c -o boxedwine.html -O2 -DUNALIGNED_MEMORY=1 -DUSE_MMU -DBOXEDWINE_ES -s LEGACY_GL_EMULATION=1 -s USE_SDL=1 -DGLH=\<GL/gl.h\> --preload-file root@/root -s TOTAL_MEMORY=603979776
./packager.py
mv package.js boxedwine.js
gzip -9 boxedwine.data
mv boxedwine.data.gz boxedwine.data
