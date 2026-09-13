#!/bin/bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/../.." && pwd)
output=${1:-"$repo/tmp/sdl-callback-test"}
mkdir -p "$output"
cd "$repo"
sanitizer_flags=()
if [[ ${SANITIZE:-0} == 1 ]]; then
  sanitizer_flags=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"${CXX:-g++}" -std=c++20 -O1 -g -pthread -ffunction-sections -fdata-sections \
  "${sanitizer_flags[@]}" \
  -DBOXEDWINE_MULTI_THREADED -DBOXEDWINE_64 -DSDL2=1 \
  -I. -Iinclude -Ilib/asmjit -Ilib/glew/include -Ilib/imgui -Ilib/simde -Ilib/pugixml/src -Ilib/zlib \
  $(pkg-config --cflags sdl2) \
  tools/openglTest/SDLCallbackTest.cpp \
  platform/sdl/sdlcallback.cpp source/util/synchronization.cpp source/util/bstring.cpp \
  -Wl,--gc-sections $(pkg-config --libs sdl2) \
  -o "$output/SDLCallbackTest"
timeout 20s "$output/SDLCallbackTest"
