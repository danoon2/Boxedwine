# Getting Started

This guide takes you from nothing to a Windows game running from the command
line. It uses Windows as the example; the Boxedwine command line is the same
on Linux and macOS once you have an executable
(see [How to Build Boxedwine](How-To-Build-Boxedwine.md)).

## 1. Get Boxedwine

### Prebuilt binaries

Download the package for your platform from the
[releases page](https://github.com/danoon2/Boxedwine/releases), then launch
the app. If it is the first time running on your system it will download the
latest Wine/Linux file system (Wine 11.0 for release 26R1). The UI has demos
in the Install tab to try, or you can install your own apps and games,
including by dragging installers or directories into the UI.

The Windows package (`Boxedwine26R1Windows-x86-x64-Arm64.zip` for 26R1) has
one folder per architecture:

| Path | Use |
|---|---|
| `Win64\BoxedWine.exe` | 64-bit x86 Windows |
| `Win64\Boxedwine_console.exe` | The same, with a console window for its output |
| `Win32\BoxedWine.exe`, `Win32\Boxedwine_console.exe` | 32-bit x86 Windows |
| `WinARM64\BoxedWine.exe` | Windows on ARM64 |

Use the `_console` executable when running from a terminal, so that log
output shows up there. Every command below works with the prebuilt
executables; skip to [Run a Wine program](#3-run-a-wine-program) if you are
not building from source.

### Build from source

Open `project\msvc\BoxedWine\BoxedWine.sln` in Visual Studio 2026 and build
`Release|x64`, or build from a Developer Command Prompt:

```bat
cd project\msvc\BoxedWine
msbuild BoxedWine.sln -p:Configuration=Release -p:Platform=x64 -m
```

The executable is `project\msvc\BoxedWine\x64\Release\BoxedWine.exe`.

The projects use the v145 toolset from Visual Studio 2026. With only Visual
Studio 2022 (or its Build Tools) installed, the build stops with
`MSB8020: The build tools for v145 ... cannot be found`. Pass the 2022 toolset
instead:

```bat
msbuild BoxedWine.sln -p:Configuration=Release -p:Platform=x64 -p:PlatformToolset=v143 -m
```

## 2. Get a Wine file system

Boxedwine runs a 32-bit Linux build of Wine, which comes as a ZIP file system.
Launching `BoxedWine.exe` without arguments opens the UI, which offers to
download one into `%APPDATA%\Boxedwine\FileSystems2`. To get it without the UI,
download it directly:

```sh
curl -L -o TinyCore15Wine11.0.zip https://boxedwine.org/v2/11/TinyCore15Wine11.0.zip
```

Use the file system that matches your build. The current source expects the
`v2/11` file system. With an older one (such as `v2/10`), every Wine program
stops at startup with an `Error` dialog saying
`Vulkan not compiled into Boxedwine`, and the Boxedwine window stays white
until the dialog is closed.

## 3. Run a Wine program

```bat
BoxedWine.exe -zip TinyCore15Wine11.0.zip -root C:\bw\root /bin/wine winemine
```

- `-zip` is the read-only file system. `-root` is a host directory where
  everything written goes, including the Wine prefix
  (`/home/username/.wine`). Use a separate root per game to keep them apart.
- The first run in a new root creates the Wine prefix, which takes a few
  seconds, and Wine asks whether to install Wine Mono. Add
  `-env WINEDLLOVERRIDES=mscoree,mshtml=` to skip that dialog, which matters
  for unattended runs.
- The guest program and its arguments come last. `/bin/wine` is a guest path.

## 4. Run a game

Put the game's files in a host directory and mount it as a Wine drive with
`-mount_drive <host directory> <drive letter>`. A game that reads its CD needs
the CD's files mounted at the drive letter it was installed with. Then start
the game from its own directory with `-w`:

```bat
BoxedWine.exe -zip TinyCore15Wine11.0.zip -root C:\bw\root-ei ^
  -mount_drive C:\games\ei-cd d ^
  -mount_drive "C:\games\Enemy Infestation" e ^
  -w /home/username/.wine/dosdevices/e: ^
  -env WINEDLLOVERRIDES=mscoree,mshtml= ^
  /bin/wine Ei.exe
```

`-mount_drive` takes a directory. To use a CD image, extract it first (7-Zip
opens ISO files). A raw `.bin` image with 2352-byte sectors has to be
converted to a 2048-byte ISO before 7-Zip can read it: keep bytes 16 to 2063
of each sector for mode 1 discs, and 24 to 2071 for mode 2 (CD-XA) discs.
Games installed into a DOSBox hard disk image can be copied out the same way,
since 7-Zip reads FAT partitions.

To change Wine settings for a root, run Wine's own tools in it, for example
the DirectDraw renderer:

```bat
BoxedWine.exe -zip TinyCore15Wine11.0.zip -root C:\bw\root-ei ^
  /bin/wine reg add "HKCU\Software\Wine\Direct3D" /v renderer /d gdi /f
```

A mounted drive is a fixed disk to Wine. For a game that checks the drive
type of its CD, mark the drive as a CD-ROM in the same root:

```bat
BoxedWine.exe -zip TinyCore15Wine11.0.zip -root C:\bw\root-mm ^
  /bin/wine reg add "HKLM\Software\Wine\Drives" /v e: /d cdrom /f
```

Boxedwine does not emulate a CD-ROM device behind a mounted directory, so
CD audio and reading the disc's table of contents fail. A game that checks
its CD through MCI CD audio (for example Magic & Mayhem, which opens
`cdaudio` and asks for the track count) still reports that the CD is
missing.

## 5. Logs and pitfalls

- `-log <file>` copies the console output to a file. Wine's own debug
  channels work as usual, for example `-env WINEDEBUG=+loaddll,+seh`.
  Output still buffered when the process is killed is lost, so end the
  game normally when you need the end of the log.
- Fatal Boxedwine errors show a modal `Error` dialog and the window stops
  responding until it is closed. A run that hangs with a white window is
  usually waiting on one of these.
- `-novideo` hides the window on Windows. The program keeps running with
  sound, but there is nothing to see or capture.
- In Git Bash, guest paths such as `/bin/wine` are rewritten into Windows
  paths (`C:/Program Files/Git/usr/bin/wine`) and Boxedwine reports
  `Could not find`. Set `MSYS_NO_PATHCONV=1` for the command, or start
  Boxedwine from `cmd` or PowerShell.

For games that do not run, see
[Troubleshooting Games/Apps](Troubleshooting-Games-Apps.md) and
[Developer Debugging](Developer-Debugging.md).
