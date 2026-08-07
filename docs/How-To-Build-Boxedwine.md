# How to build

- Clone the repository

```
git clone https://github.com/danoon2/Boxedwine
```

or you can download a zip of the repo by visiting https://github.com/danoon2/Boxedwine in your browser, then tapping the green button in the upper right called “Code”.  That will launch a popup and at the bottom of the popup is an entry for “Download ZIP”.  Select that and after it downloads, unzip it on your computer.  This is the full Boxedwine source code and you can build it using the instructions below.

(TODO: move to README) To use Boxedwine, you need a file system.  The easiest way to get a file system is to launch Boxedwine that you built with no command line arguments.  If you haven't downloaded a file system before it will ask you if you want to.

You can see the filesystem locations below:

- Windows location: `C:\Users\username\AppData\Roaming\Boxedwine\FileSystems2`
- Linux location: `~/.local/share/Boxedwine/FileSystems2`

With just the filesystem and an executable you should be able to test launching the Wine Notepad with this command line:

```
boxedwine -root . -zip TinyCore15Wine6.0.zip /bin/wine notepad
```

## Windows Build

Currently I use Visual Studio 2022 community edition.

Open the solution file at: `project\msvc\BoxedWine\BoxedWine.sln`

## Linux Build

### Dependencies

- Debian/Ubuntu

```
sudo apt install build-essential zlib1g-dev, libminizip-dev, libsdl2-dev libssl-dev
```

Change the current directory to: `project/linux` :

```
project/linux
```

And run either:

```
make multiThreaded
```

or 

```
make release
```

* The `multiThreaded` target is to compile with CPU binary translation for x86_64 or ARMv8 systems
* The `release` target is for normal CPU emulation (TODO: explain what it is, the Linux kernel virtualization with KVM enabled or disabled?)


## MacOS Build

Install XCode from the App store

Go to the directory at `project\mac-xcode` in Finder and open the `Boxedwine.xcworkspace` file

The Boxedwine target uses the binary translator for x64 or Armv8 depending on your system.

## Emscripten Build

Follow the instructions on Emscripten and/or WebAssembly website to setup the toolchain, make sure to initialize the Emscripten environment in current shell session using the following command:

```
source ./emsdk_env.sh
```

Change the current directory to: `project/emscripten` and run `make`

```
cd project/emscripten
```

```
make
```

Read the `buildFlags.txt` file in source code tree to find the configuration options.
