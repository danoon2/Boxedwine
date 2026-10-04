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

For the native Windows frontend, see [Native Windows UI](project/msvc/BoxedwineUI/README.md).
It uses WPF and .NET 10 with Windows light/dark themes. Run
`project\msvc\BoxedwineUI\build.ps1 -BuildRuntime -Test` in PowerShell to build its
separate emulator and UI. The existing Windows solution above remains available.

Jenkins packages the framework-dependent native UI in `Deploy/Win64` and
`Deploy/WinARM64`, and retains the existing frontend in `Deploy/Win32`. All three
folders go into the existing combined build ZIP. The native UI requires an installed
[.NET 10 Desktop Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/10.0)
matching its architecture, and offers a download prompt if it is missing.
Windows packaging downloads a pinned .NET 10 SDK into the workspace on first use;
workers need access to Microsoft's .NET downloads and NuGet.org for publishing. Run
`tools\jenkins\build-windows.ps1 -Platform x64` (or `ARM64` / `Win32`) to reproduce
a target's packaging locally. See the UI README's Jenkins release section for
the folder layout and the separate command-line executable used by automation.

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

The emulator’s Debug configuration uses `CLANG_CXX_STANDARD_LIBRARY_HARDENING=extensive`. Xcode 26.5 otherwise enables debug-mode checks that walk the whole JIT address map on each insertion or removal, making Wine startup appear stuck. Extensive mode retains standard-library safety checks without those internal invariant scans; emulator assertions, debug symbols, and unoptimized source-level debugging remain enabled.

The native `Boxedwine` target also prepares checksum-pinned SDL 2.32.10 and public-API MoltenVK builds before linking. It downloads only when the required dependency/cache is absent. SDL 2.32.10 fixes the hidden-window Metal renderer shutdown crash encountered during background Wine configuration with Xcode Metal validation enabled. See [SDL build inputs and verification](tools/sdl/README.md) and [MoltenVK build inputs and verification](tools/moltenvk/README.md).

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

### Jenkins unit tests

The MT JIT unit-test binary is built once and stashed, then run as 16 independent
batches on the `emscripten` worker pool. Each free worker takes another batch,
including after finishing the ST or ST JIT suite. No executor is held while the
batches wait for workers. Each batch uses a separate workspace and Firefox
profile, has a 15-minute execution timeout, and archives its own console log.
The two Linux workers and Mac M4 should each have the `emscripten` label and keep
one Jenkins executor. Concurrent MT JIT browser runs caused memory pressure and
timeouts during Mac validation.

Unit-test workers require Emscripten **5.0.7**. The environment helper rejects a
different active version rather than silently using a newer SDK. On Linux, it
uses `~/emsdk` and `/usr/bin/firefox`. The Mac M4 uses the shared installation at
`/Users/Shared/BoxedwineCI/emsdk` and the normal Firefox installation at
`/Applications/Firefox.app` (verified with **157.0**; Linux uses **156.0.1**).
Its SDK uses Node **22.16.0** and bundled Python **3.13.3**.
Each Mac account keeps its writable SDK cache in
`~/Library/Caches/Boxedwine/emscripten-5.0.7`, so Jenkins can use the shared tools
without write access to the SSH account's home. `BOXEDWINE_EMSDK_ROOT`,
`BOXEDWINE_FIREFOX`, and `EM_CACHE` can override the paths.

From the repository root, use `source tools/jenkins/emscripten-unit-test-env.sh`
to load the same environment as Jenkins. The helper is included in the MT JIT
stash so batches can move between Linux and macOS without checking out or
rebuilding the source. Web packaging and AbiWord browser automation use
`emscripten && linux64`; the packaging assets in `/var/www/buildfiles` are only
available on the Linux x64 worker.

The test runner assigns entries to batches by index modulo batch count, so every
test runs exactly once and expensive instruction families are spread across
batches. New tests are included automatically. Full test thoroughness is retained;
the pipeline does not pass `-fast`.

To reproduce the first MT JIT batch after `make testMultiThreadedJit`:

```sh
cd project/emscripten/Build/TestMultiThreadedJit
emrun --browser=firefox 'boxedwine.html?-shard&0&16'
```

`-shard index count` uses a zero-based index. Add `&-list` to print the selected
test indexes and names without executing them. The Windows and Linux test
executables accept the same arguments. Existing positional `start count threads`
arguments still work and, if combined with `-shard`, select the range to split.

### Mac App Store edition

`project/mac-xcode/buildAppStore.sh [--skip-dependencies] /absolute/path/TinyCore15Wine11.0.zip TEAMID`
archives the native UI with `BOXEDWINE_BUILD_VARIANT=BOXEDWINE_APP_STORE`.
This becomes a Swift compilation condition: the launcher has no demo installer,
URLSession downloader, demo browser, or Wine download/version-switching UI.
Wine 11 and Wine's Minesweeper are included; adding apps requires no download.
The supplied Wine ZIP must match the exact branch pin in `Resources/WindowsSupport`.
The bundle phase skips fetching the demo catalog and strips any catalog left by
an earlier build. It includes only Wine 11's pin and a privacy manifest with no
collected-data declarations. Windows programs still have network access.

To test this edition locally with ad-hoc signing, run:

```sh
./project/mac-xcode/buildNative.sh --configuration Release /absolute/path/TinyCore15Wine11.0.zip BOXEDWINE_BUILD_VARIANT=BOXEDWINE_APP_STORE
```

Debug, Sandbox, and ordinary Release builds default to `BOXEDWINE_DIRECT`.
`buildRelease.sh` explicitly selects that variant for Jenkins, keeps the complete
Wine list and demo catalog, and omits the Wine ZIP so users download it as needed.
`audit-native-bundle.py --app-store` verifies the Store edition's bundled package,
catalog absence, privacy declarations, and absence of URLSession imports in the
launcher. The bundle preparation step removes quarantine attributes from the
assembled app before signing; the Store audit rejects any that remain, including
on nested resources and directories. The public privacy policy source is
`project/mac-xcode/privacy.html`;
its updated edition-specific wording must be uploaded when changing distribution.

### Optional App Store tips

The Store edition includes **Boxedwine → Support Boxedwine…**. Tips use StoreKit 2
consumable purchases and never unlock features. The direct/Jenkins edition does
not include the purchase code or support window. Product names and prices come
from Apple; missing products or an unavailable connection leave the app usable.
There is no developer payment server or stored purchase history. The transaction
listener starts with the library and finishes only verified tip transactions,
including interrupted purchases and later Ask to Buy approvals.

Configure these consumables in App Store Connect, with United States as the
base region and Apple's comparable prices elsewhere:

| Product ID | Display name | US price |
| --- | --- | --- |
| `org.boxedwine.app.tip.small` | Small Tip | $2.99 |
| `org.boxedwine.app.tip.medium` | Medium Tip | $4.99 |
| `org.boxedwine.app.tip.large` | Large Tip | $9.99 |

Description: **Optional one-time tip. All features remain free.** Add a screenshot
of the support window for review. The first purchases must be submitted with an
app version; they also require an active Paid Apps agreement and the account's
banking/tax setup. The app remains free in Pricing and Availability.

Run purchase-state tests with:

```sh
swift test --package-path project/mac-xcode/Boxedwine/BoxedwineUI -Xswiftc -DBOXEDWINE_APP_STORE --filter TipStoreTests
```

For local StoreKit testing, build the Store edition and choose
`BoxedwineUI/Tests/Fixtures/BoxedwineTips.storekit` under the Xcode scheme's
**Run → Options → StoreKit Configuration**. Run from Xcode to activate Apple's
local test environment. The fixture is not bundled in distribution builds.
Check a successful tip, a second tip of the same amount, cancellation, declined
payment, Ask to Buy approval, and an interrupted purchase. Switch the scheme's
StoreKit Configuration back to **None** for App Store sandbox/TestFlight testing.
No restore button is needed for consumable tips because there is no entitlement.
