#!/bin/bash
mkdir -p bin
gcc -std=c++17 -O2 \
  -Wall \
  -Wno-strict-aliasing \
  -Wno-unused-local-typedefs \
  -Wno-unused-variable \
  -Wno-unknown-pragmas \
  -Wno-unused-function \
  -Wno-unused-but-set-variable \
  -I../../include \
  ../../source/test/*.cpp \
  ../../source/sdl/*.cpp \
  ../../source/sdl/singleThreaded/*.cpp \
  ../../platform/sdl/*.cpp \
  ../../platform/linux/*.cpp \
  ../../source/emulation/cpu/*.cpp \
  ../../source/emulation/cpu/common/*.cpp \
  ../../source/emulation/cpu/normal/*.cpp \
  ../../source/emulation/softmmu/*.cpp \
  ../../source/io/*.cpp \
  ../../source/kernel/*.cpp \
  ../../source/kernel/devs/*.cpp \
  ../../source/kernel/proc/*.cpp \
  ../../source/kernel/sys/*.cpp \
  ../../source/kernel/loader/*.cpp \
  ../../source/util/*.cpp \
  ../../source/opengl/sdl/*.cpp \
  ../../source/opengl/*.cpp \
  -lm \
  -lz \
  -lminizip \
  -lGL \
  -lstdc++ \
  -lstdc++fs \
  -DBOXEDWINE_RECORDER \
  -DBOXEDWINE_ZLIB \
  -DBOXEDWINE_HAS_SETJMP \
  -DSDL2=1 \
  "-DGLH=<SDL_opengl.h>" \
  -DBOXEDWINE_OPENGL_SDL `sdl2-config --cflags --libs` \
  -D __TEST \
  -DSIMDE_SSE2_NO_NATIVE \
  -DBOXEDWINE_DISABLE_UI \
  -o bin/boxedwineTest
