# Boxedwine - How to Build

## Download the source

You can use git:

git clone https://github.com/danoon2/Boxedwine.git

or you can download a zip of the source from github

https://github.com/danoon2/Boxedwine/archive/refs/heads/master.zip

## Windows

Install Visual Studio 2022.  There is a free Community version that works which is what I use to do most of my development

https://visualstudio.microsoft.com/vs/community/

Once installed, you can open open project\msvc\BoxedWine\BoxedWine.sln

There are no dependencies.

## Mac

You need to install XCode 15 or later.  You can do this from the App Store on your Mac.  Boxedwine supports the old Intel Macs and the new M series.

https://developer.apple.com/xcode/

To install the dependencies, open a terminal and got to where you installed the source code.  In the folder project/mac-xcode, you need to run: 

sh fetchDepends.sh

This only needs to be done once, after you download the source

After that, in XCode you just need to open project/mac-xcode/Boxedwine.xcworkspace

## Linux

You need to have GCC 12 or highter.  This means running Debian 12 or Ubuntu 23 or higher

package you might need to install:

zlib1g-dev
libminizip-dev
libsdl2-dev
libssl-dev
libcurl4-openssl-dev

To build, in the terminal go to the source directory and in, project/linux, you need to type

make

