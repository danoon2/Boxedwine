# Boxedwine

Boxedwine is an emulator that runs Windows applications.  It achieves this by running a 32-bit version of Wine, and emulating the Linux kernel and CPU.  It is written in C++ with SDL and is supported on multiple platforms.

Boxedwine is open source and released under the terms of the GNU General Public License v2 (GPL).

## Features

- Runs 16/32-bit Windows programs
- Works on Windows, MacOSX, Linux, Raspberry Pi and Web
- Works on x86-64, i686, ARMv7 and ARMv8
- Can run multiple versions of Wine, from 1.8 to 5.0
- Tools and games using OpenGL or Direct3D are supported

## TODOs

- Networking does not work well
- Mac OpenGL does not work with frame buffers
- ARMv8 CPU core for Mac M1 and Raspberry Pi still has bugs
- Games after the year 2000 have limitted success at running


## Platforms being tested

- Emscripten (WASM)
- Linux 64-bit
- MacOSX (ARM)
- MacOSX (Intel)
- Raspberry Pi 4 32-bit
- Raspberry Pi 4 64-bit
- Windows 32-bit
- Windows 64-bit


## Performance Test using MDK Perf with GDI backend on Wine

![This is an image](http://boxedwine.org/mdk.jpg)

### Emscripten on Intel i7-6700K on Windows 10

- **27** Firefox 81
- **29** Chrome Version 96.0.4664.110 (Official Build) (64-bit)


### Mac Mini M1

- **870** ARMv8 CPU Core
- **150** x64 CPU Core with Rosetta


### iMac 2017 3.4GHz i5

- **245** x64 CPU Core


### Raspberry Pi 4 64-bit

- **133** ARMv8 CPU Core


### Raspberry Pi 4 32-bit

- **14** Normal CPU Core + JIT


### Windows 10 on Intel i7-6700K

- **48** Normal CPU Core
- **64** Normal CPU Core + JIT
- **985** x64 CPU Core

## Documentation

- [Upcoming Features](docs/Roadmap-Features.md)
- [Troubleshooting Games/Apps](docs/Troubleshooting-Games-Apps.md)
- [Developer Debugging](docs/Developer-Debugging.md)
- [How To Build Wine](docs/How-To-Build-Boxedwine.md)
- [How To Build OSMesa on MacOSX](docs/How-To-Build-OSMesa-on-Mac.md)
- [CPU Emulation](docs/CPUemulation.md)
