# Getting Started

This guide takes you from nothing to a Windows game running from the command
line. It uses Windows as the example; the Boxedwine command line is the same
on Linux and macOS.

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
output shows up there.

### Build from source

To build Boxedwine yourself, follow
[How to Build Boxedwine](How-To-Build-Boxedwine.md). It also covers getting
the file system that matches a source build.

## 2. Get a Wine file system

Boxedwine runs a 32-bit Linux build of Wine, which comes as a ZIP file system.
The first launch without arguments opens the UI, which downloads one into
`%APPDATA%\Boxedwine\FileSystems2` (`~/.local/share/Boxedwine/FileSystems2`
on Linux). The commands below pass that ZIP to `-zip` by its full path,
since Boxedwine opens the path as given and does not look in
`FileSystems2` for it.

## 3. Run a Wine program

```bat
BoxedWine.exe -zip "%APPDATA%\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" -root C:\bw\root /bin/wine winemine
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

`-mount_drive <host directory> <drive letter>` makes a host directory a Wine
drive. A game usually needs two of them:

- The installed game: the directory with the game's EXE, as its installer
  left it.
- The game's CD: the files from the CD, for games that read data from it
  while running. Mount them at the drive letter the game was installed from,
  since games usually store that path in their settings.

This starts `Game.exe` with the installed game on E: and the CD's files on
D:, from the game's own directory (`-w`):

```bat
BoxedWine.exe -zip "%APPDATA%\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" -root C:\bw\root-mygame ^
  -mount_drive C:\games\MyGame e ^
  -mount_drive C:\games\MyGame-cd d ^
  -w /home/username/.wine/dosdevices/e: ^
  -env WINEDLLOVERRIDES=mscoree,mshtml= ^
  /bin/wine Game.exe
```

`-mount_drive` takes a directory, not an image file. To use a CD image, extract it first (7-Zip
opens ISO files). A raw `.bin` image with 2352-byte sectors has to be
converted to a 2048-byte ISO before 7-Zip can read it: keep bytes 16 to 2063
of each sector for mode 1 discs, and 24 to 2071 for mode 2 form 1 (CD-XA)
discs. Mode 2 form 2 sectors hold 2324 bytes, so a disc with form 2 sectors
(video or audio tracks stored as data) needs a track-aware extraction tool.
Games installed into a DOSBox hard disk image can be copied out the same way,
since 7-Zip reads FAT partitions.

To change Wine settings for a root, run Wine's own tools in it, for example
the DirectDraw renderer:

```bat
BoxedWine.exe -zip "%APPDATA%\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" -root C:\bw\root-mygame ^
  /bin/wine reg add "HKCU\Software\Wine\Direct3D" /v renderer /d gdi /f
```

A mounted drive is a fixed disk to Wine. For a game that checks the drive
type of its CD, mark the drive as a CD-ROM in the same root:

```bat
BoxedWine.exe -zip "%APPDATA%\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" -root C:\bw\root-mygame ^
  /bin/wine reg add "HKLM\Software\Wine\Drives" /v d: /d cdrom /f
```

Boxedwine does not emulate a CD-ROM device behind a mounted directory, so
CD audio and reading the disc's table of contents fail. A game that checks
for its CD through MCI CD audio (opening `cdaudio` and asking for the track
count) still reports that the CD is missing.

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
