# How to build

- Clone the repository

```bash
git clone https://github.com/danoon2/Boxedwine
cd Boxedwine
```

or you can download a zip of the repo by visiting https://github.com/danoon2/Boxedwine in your browser, then tapping the green button in the upper right called “Code”.  That will launch a popup and at the bottom of the popup is an entry for “Download ZIP”.  Select that and after it downloads, unzip it on your computer. Change into the extracted repository directory before continuing. This is the full Boxedwine source code and you can build it using the instructions below.

To use Boxedwine, you need a file system. The easiest way to get one is to launch the Boxedwine executable that you built without command-line arguments. If you have not downloaded a file system before, Boxedwine will ask whether you want to download one.

You can see the filesystem locations below:

- Windows location: `C:\Users\username\AppData\Roaming\Boxedwine\FileSystems2`
- Linux location: `~/.local/share/Boxedwine/FileSystems2`

With the executable and downloaded file system, you can test launching Wine Notepad from the command line. Replace the ZIP path if you downloaded a different file system:

```bash
boxedwine -root . -zip TinyCore15Wine11.0.zip /bin/wine notepad
```

You can also download the file system directly. A source build needs the file system it was made for; the current source uses:

```bash
curl -L -o TinyCore15Wine11.0.zip https://boxedwine.org/v2/11/TinyCore15Wine11.0.zip
```

With an older file system (such as `v2/10`), every Wine program stops at startup with an `Error` dialog saying `Vulkan not compiled into Boxedwine`, and the Boxedwine window stays white until the dialog is closed.

## Windows Build

Install Visual Studio 2026 Community with the **Desktop development with C++** workload.

Open the solution file at: `project\msvc\BoxedWine\BoxedWine.sln`

To build from a Developer Command Prompt instead:

```bat
cd project\msvc\BoxedWine
msbuild BoxedWine.sln -p:Configuration=Release -p:Platform=x64 -m
```

The executable is `project\msvc\BoxedWine\x64\Release\BoxedWine.exe`. Release builds use the Windows subsystem, so the executable writes nothing to a terminal it is started from. To get the console executable the release packages ship, copy it and change the copy's subsystem, as `tools/jenkins/build-windows.ps1` does:

```bat
copy x64\Release\BoxedWine.exe x64\Release\Boxedwine_console.exe
editbin /subsystem:console x64\Release\Boxedwine_console.exe
```

The projects use the v145 toolset from Visual Studio 2026. With only Visual Studio 2022 (or its Build Tools) installed, the build stops with `MSB8020: The build tools for v145 ... cannot be found`. Pass the 2022 toolset instead:

```bat
msbuild BoxedWine.sln -p:Configuration=Release -p:Platform=x64 -p:PlatformToolset=v143 -m
```

## Linux Build

### Dependencies

- Debian/Ubuntu

```bash
sudo apt install build-essential libcurl4-openssl-dev libgl-dev libsdl2-dev libssl-dev zlib1g-dev
```

From the root of the Boxedwine checkout, change to the Linux project directory and build the release target:

```bash
cd project/linux
make release
```

The Makefile selects the appropriate JIT and multithreading options for supported host architectures. The resulting executable is `project/linux/Build/Release/boxedwine`.

## macOS Build

Install Xcode from the App Store.

Go to the directory at `project/mac-xcode` in Finder and open the `Boxedwine.xcworkspace` file

The Boxedwine target uses the binary translator for x64 or Armv8 depending on your system.

The native macOS UI uses its bundled engine at
`Boxedwine.app/Contents/Helpers/BoxedwineEngine.app/Contents/MacOS/Boxedwine`.
The engine location is not a saved preference. For debugging a different build,
launch a new UI process with an explicit override:

```bash
open -n /path/to/Boxedwine.app --args --emulator /path/to/BoxedwineEngine.app/Contents/MacOS/Boxedwine
```

The override applies to app launches and Wine configuration for that process
only. An invalid path produces an error without falling back to another engine;
launch logs record the selected engine path.

## Emscripten Build

Follow the [official Emscripten SDK instructions](https://emscripten.org/docs/getting_started/downloads.html) to install and activate the toolchain. The commands below assume the SDK is installed at `~/emsdk`.

From the root of the Boxedwine checkout, initialize the Emscripten environment in the current shell session, then change to the Emscripten project directory and build:

```bash
source ~/emsdk/emsdk_env.sh
cd project/emscripten
make
```

Read [`buildFlags.txt`](../buildFlags.txt) in the source tree for configuration options.
