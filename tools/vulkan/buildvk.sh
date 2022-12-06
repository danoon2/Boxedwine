#!/bin/bash
gcc -c -Wall -Werror -Wno-return-type -fpic -m32 -march=i586 vk.c
gcc -Wl,-soname,libvulkan.so.1 -shared -m32 -o libvulkan.so.1 vk.o
