#!/bin/sh
set -eu
cd "$(dirname "$0")"
cc=${CC:-gcc}
"$cc" -c -O2 -Wall -Wextra -Werror -fPIC -m32 -march=i586 vk.c
"$cc" -Wl,-soname,libvulkan.so.1 -Wl,-z,noexecstack -shared -m32 -o libvulkan.so.1 vk.o -ldl
