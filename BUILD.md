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

The native Mac UI needs Xcode with Swift 6 and a macOS 15 or newer SDK (validated with Xcode 26.5). The current native app targets Apple Silicon and macOS 15 or later.

https://developer.apple.com/xcode/

To install the dependencies, open a terminal and got to where you installed the source code.  In the folder project/mac-xcode, you need to run: 

sh fetchDepends.sh

This only needs to be done once, after you download the source

After that, in XCode you just need to open project/mac-xcode/Boxedwine.xcworkspace

For the experimental SwiftUI launcher, see [Native macOS preview](project/mac-xcode/Boxedwine/BoxedwineUI/README.md). Its `BoxedwineUI` scheme and build script build the native UI and the `Boxedwine` emulator with the old Mac UI disabled, currently targeting Apple Silicon and macOS 15+. All Mac targets use native OpenGL and omit software Mesa and its dependency libraries; the downloaded dependency archive may still contain these unused build inputs. Debug enables unsandboxed debugging; the `Boxedwine` scheme waits to attach to a launched emulator, and `BoxedwineUI-Sandbox` tests sandbox behavior. Debug, Sandbox, and Release builds include a bundle audit; [release preparation](project/mac-xcode/Boxedwine/BoxedwineUI/RELEASE.md) records the remaining distribution work.

Jenkins uses `project/mac-xcode/buildRelease.sh` to archive the native UI with its embedded emulator as `bin/Boxedwine.app`, without a Wine filesystem. It then uses `signNative.sh` with the existing Developer ID identity, audits every embedded binary, and notarizes and staples the app before packaging `Deploy/Mac/Boxedwine.zip`. See [Jenkins Mac builds](project/mac-xcode/Boxedwine/BoxedwineUI/RELEASE.md#jenkins-mac-builds) for worker requirements and local verification.

The native demo catalog and icons are downloaded at build time using the shared pin in `resources/demo-catalog.lock.json`. An exact verified cache works offline. Local Xcode builds warn and omit demos if the pinned package is unavailable; Jenkins fails instead. No catalog fallback is stored in Git. See [Shared demo catalog](resources/DEMO_CATALOG.md) for cache settings and publishing updates.

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

## Emscripten

Follow instructions for installing Emscripten on: https://emscripten.org/docs/getting_started/downloads.html

To build, in the terminal go to the source directory and in, project/emscripten, you need to type

make release
