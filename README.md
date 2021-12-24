# Boxedwine
Boxedwine is an emulator that runs Windows applications.  It achieves this by running a 32-bit version of Wine, and emulating the Linux kernel and CPU.  It is written in C++ with SDL and is supported on multiple platforms.

Boxedwine is open source and released under the terms of the GNU General Public License v2 (GPL).

## Features

- Runs 16/32-bit Windows applications
- Runs on Windows, Mac, Linux, Raspberry Pi and Web
- Can run multiple version of Wine, from 1.8 to 5.0
- Supports OpenGL and DirectX games.


## Needs Work

- Networking does not work well
- Mac OpenGL does not work with frame buffers
- ARMv8 CPU core for Mac M1 and Raspberry Pi still has bugs
- Games after the year 2000 have limitted success at running


## Platforms that are tested

- Emscripten (WASM)
- Linux 64-bit
- Mac (Arm)
- Mac (Intel)
- Raspberry Pi 4 32-bit
- Raspberry Pi 4 64-bit
- Windows 32-bit
- Windows 64-bit


## Performance Test using MDK Perf with GDI backend on Wine

![This is an image](http://boxedwine.org/mdk.jpg)

Emscripten on Intel i7-6700K on Windows 10
- **27** Firefox 81
- **29** Chrome Version 96.0.4664.110 (Official Build) (64-bit)


Mac Mini M1
- **870** ARMv8 CPU Core
- **150** x64 CPU Core with Rosetta


iMac 2017 3.4GHz i5
- **245** x64 CPU Core


Raspberry Pi 4 64-bit
- **133** ARMv8 CPU Core


Raspberry Pi 4 32-bit
- **14** Normal CPU Core + JIT


Windows 10 on Intel i7-6700K
- **64** Normal CPU Core + JIT
- **985** x64 CPU Core


Repo is at: https://github.com/danoon2/Boxedwine
