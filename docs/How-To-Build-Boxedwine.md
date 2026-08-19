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

## Windows Build

Install Visual Studio 2026 Community with the **Desktop development with C++** workload.

Open the solution file at: `project\msvc\BoxedWine\BoxedWine.sln`

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

## Emscripten Build

Follow the [official Emscripten SDK instructions](https://emscripten.org/docs/getting_started/downloads.html) to install and activate the toolchain. The commands below assume the SDK is installed at `~/emsdk`.

From the root of the Boxedwine checkout, initialize the Emscripten environment in the current shell session, then change to the Emscripten project directory and build:

```bash
source ~/emsdk/emsdk_env.sh
cd project/emscripten
make
```

Read [`buildFlags.txt`](../buildFlags.txt) in the source tree for configuration options.
