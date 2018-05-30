#!/bin/bash
gcc -Wall -I../../include ../../source/emulation/*.c ../../source/io/*.c ../../source/kernel/*.c ../../source/kernel/devs/*.c ../../source/kernel/loader/*.c ../../source/util/*.c ../../source/test/*.c ../../platform/linux/*.c -o test -lm -D __TEST
