#/bin/sh
emcc -Wall -Wno-unused-function -I../../include ../../source/sdl/*.cpp ../../platform/linux/*.cpp ../../source/emulation/cpu/*.cpp ../../source/emulation/cpu/common/*.cpp ../../source/emulation/cpu/normal/*.cpp ../../source/emulation/softmmu/*.cpp ../../source/io/*.cpp ../../source/kernel/*.cpp ../../source/kernel/devs/*.cpp ../../source/kernel/proc/*.cpp ../../source/kernel/loader/*.cpp ../../source/util/*.cpp ../../source/opengl/sdl/*.cpp ../../source/opengl/*.cpp -o boxedwine.html -O2 -DUNALIGNED_MEMORY=1 -DUSE_MMU -DBOXEDWINE_ES -s LEGACY_GL_EMULATION=1 -s USE_SDL=1 -DGLH=\<GL/gl.h\> --preload-file root@/root -s TOTAL_MEMORY=603979776
./packager.py
mv package.js boxedwine.js
gzip -9 boxedwine.data
mv boxedwine.data.gz boxedwine.data
