#How to build

The code is hosted at GitHub and can be seen here

https://github.com/danoon2/Boxedwine

You can clone this repo locally,

git clone https://github.com/danoon2/Boxedwine

or you can download a zip of the repo by visiting https://github.com/danoon2/Boxedwine in your browser, then tapping the green button in the upper right called “Code”.  That will launch a popup and at the bottom of the popup is an entry for “Download ZIP”.  Select that and after it downloads, unzip it on your computer.  This is the full Boxedwine source code and you can build it using the instructions below.

To use Boxedwine, you need a file system.  The easiest way to get a file system is to launch Boxedwine that you built with no command line arguments.  If you haven't downloaded a file system before it will ask you if you want to.

On Windows, Boxedwine will store the file system here:

C:\Users\username\AppData\Roaming\Boxedwine\FileSystems2

On Linux

~/.local/share/Boxedwine/FileSystems2

With just the filesystem and an executable you should be able to test launching notepad with this command line:

boxedwine -root . -zip TinyCore15Wine6.0.zip /bin/wine notepad

##Windows Build

Currently I use Visual Studio 2022 community edition.

Open solution file project\msvc\BoxedWine\BoxedWine.sln

##Linux Build

dependencies: on Ubuntu this might be zlib1g-dev, libminizip-dev, libsdl2-dev and libssl-dev

change directory to project/linux and run either

make multiThreaded

or 

make release

* the multiThreaded target is for x64 or Armv8 systems to compile the binary translator emulation. 
* the release target is for the normal CPU emulation


##MacOSX Build

Install XCode from the App store

change to the directory project\mac-xcode in finder and open Boxedwine.xcworkspace

The Boxedwine target uses the binary translator for x64 or Armv8 depending on your system.

##Emscripten Build

follow instructions on Emscripten and/or WebAssembly website for setup

make sure to initialize the emscripten environment: source ./emsdk_env.sh

change directory to project/emscripten and make

read buildFlags.txt in source code tree for configuration options