# Boxedwine
Boxedwine is an emulator that runs Windows applications.  It achieves this by running an unmodified 32-bit version of Wine, and emulating the Linux kernel and CPU.  It is written in C++ with SDL and is supported on multiple platforms.

Boxedwine is open source and released under the terms of the GNU General Public License v2 (GPL).

**Features**

-Runs 16-bit Windows applications
-Runs 32-bit Windows applications
-Runs in a browser with Emscripten (wasm and asm.js)
-Runs on Windows, Mac and Linux
-Can run multiple version of Wine, from 1.8 to 5.0


**Platforms that are tested**

-Emscripten (WASM)
-Linux 64-bit
-Mac (Arm)
-Mac (Intel)
-Raspberry Pi 4 32-bit
-Raspberry Pi 4 64-bit
-Windows 32-bit
-Windows 64-bit


**Performance Test using MDK Perf with GDI backend on Wine**

![This is an image](http://boxedwine.org/mdk.jpg)

Emscripten on Intel i7-6700K on Windows 10
- **27** Firefox 81
- **29** Chrome Version 96.0.4664.110 (Official Build) (64-bit)

Mac Mini M1
- **870** ARMv8 CPU Core
- **150** x64 CPU Core with Rosetta

Raspberry Pi 4 64-bit
- **133** ARMv8 CPU Core

Raspberry Pi 4 32-bit
- **14** Normal CPU Core + JIT

Windows 10 on Intel i7-6700K
- **64** Normal CPU Core + JIT
- **985** x64 CPU Core

Repo is at: https://github.com/danoon2/Boxedwine
