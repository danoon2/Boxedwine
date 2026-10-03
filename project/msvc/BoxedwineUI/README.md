# Native Windows UI

A WPF desktop frontend for Boxedwine, using the same workflows and saved-app
format as the native Mac frontend. It uses Windows controls, native window chrome,
file/folder pickers, keyboard navigation, drag and drop, and .NET's Fluent theme.
Settings supports the Windows appearance setting, Light, and Dark, with live
changes across open windows. System high-contrast colors are respected.

## Build and run

Install the .NET 10 SDK and Visual Studio's C++ desktop build tools. The complete
x64 build was verified with Visual Studio 18 Community. The UI has no NuGet package
dependencies. Running a framework-dependent build requires the .NET 10 **Desktop**
Runtime, which is also included with the SDK on this development machine.

From the repository root, in PowerShell:

```powershell
./project/msvc/BoxedwineUI/build.ps1 -BuildRuntime -Test
./project/msvc/BoxedwineUI/bin/Release/net10.0-windows/Boxedwine.exe
```

`build.ps1` verifies the demo catalog against `resources/demo-catalog.lock.json`.
An already verified download works offline. `-SkipCatalog` skips catalog preparation;
if no catalog was previously built, the Demos page offers Download Catalog.
Wine itself is downloaded when needed.

The current native UI release offers only Wine 11.0 V13. With a single catalog
entry, Add App shows the package as text, built-in apps select it automatically,
and version selection, ZIP import and trial-copy controls and guidance are hidden.
Settings still offers setup of the supported package when needed. Existing apps
retain their pinned package records. The shared native catalog lives in the Mac
UI's `Resources/WindowsSupport/filesV2.xml` and `packages.json`; add a validated
future package to both files to restore multiple-version controls in both UIs.

The native runtime is built separately as `BoxedwineEngine.exe`, with intermediate
files in `artifacts/runtime-obj` and output in `artifacts/runtime`. The script copies
it into the UI's `Runtime` folder. The existing emulator solution/configurations
retain their existing default frontend. Native mode disables frontend initialization
and uses a stdin quit command and parent lifetime tracking. Normal native shutdown
returns zero instead of the legacy recorder's no-playback status.

For UI development, open `BoxedwineUI.sln` and set **BoxedwineUI** as the startup
project. Build the runtime once with the script. `-Configuration Debug` builds debug
versions; `-Platform ARM64` selects the emulator's ARM64 configuration. ARM64
packages have been cross-built and checked; execution on ARM64 hardware remains
unverified here. An existing native runtime can be
supplied with `-Emulator C:\path\BoxedwineEngine.exe`.

## Jenkins release packages

Jenkins keeps the existing combined `build-<number>.zip` and its Windows folders:

```text
Win32/Boxedwine.exe                    Existing 32-bit frontend
Win32/Boxedwine_console.exe            Existing command-line build
Win64/Boxedwine.exe                    Native x64 Windows UI
Win64/Runtime/BoxedwineEngine.exe       UI-owned x64 emulator
Win64/Boxedwine_console.exe            Command-line/recorder build
WinARM64/Boxedwine.exe                  Native ARM64 Windows UI
WinARM64/Runtime/BoxedwineEngine.exe    UI-owned ARM64 emulator
WinARM64/Boxedwine_console.exe         Command-line/recorder build
```

The two new UI folders include the catalog, icons, licenses and native emulator.
They require an installed [.NET 10 Desktop Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/10.0):
Windows x64 for Win64, or Windows Arm64 for WinARM64. If it is missing, the native
`Boxedwine.exe` launcher displays Microsoft's download prompt before any managed
code runs. Install the matching Desktop Runtime and reopen Boxedwine. The SDK is
not required; the plain .NET Runtime alone does not include WPF. A matching
installation is shared with other apps instead of bundled in each download.

Extract the archive and run the appropriate `Boxedwine.exe`; keep each complete
folder together. The `Runtime` folder contains the Boxedwine emulator, not .NET.
Wine is downloaded on demand. The Win32 frontend and console builds do not need
.NET. The existing Linux, Mac and Web outputs remain in the combined archive.

The `windows` and `windowsARM64` workers need the .NET 10 SDK on `PATH`, Visual
Studio C++ tools for their targets, and access to NuGet.org for architecture-specific
apphost packs. MSBuild/editbin are discovered from `PATH` or Visual Studio;
the packaging helper also accepts explicit `-MSBuildPath` and `-EditbinPath`.

Run the same packaging steps locally from the repository root:

```powershell
./tools/jenkins/build-windows.ps1 -Platform Win32
./tools/jenkins/build-windows.ps1 -Platform x64
./tools/jenkins/build-windows.ps1 -Platform ARM64
```

Each invocation cleans only its own folder beneath `project/msvc/Deploy`, builds
the target, and checks executable architecture, console/GUI subsystem, the .NET 10
Desktop Runtime requirement, absence of bundled runtime DLLs, and catalog presence
before Jenkins stashes it. The 64-bit targets also
run the core tests. Jenkins automation invokes `Boxedwine_console.exe`, preserving
the recorder's command-line, stdin and exit-code behavior independently of the UI.

For a framework-dependent UI release outside the Jenkins layout:

```powershell
./project/msvc/BoxedwineUI/build.ps1 -BuildRuntime -Publish -Platform x64
./project/msvc/BoxedwineUI/build.ps1 -BuildRuntime -Publish -Platform ARM64
```

Outputs default to `artifacts/publish/win-x64/Release` and
`artifacts/publish/win-arm64/Release` beneath this project; `-OutputDirectory`
overrides the destination. Publishing rejects folders containing an old bundled
.NET runtime: remove that previous publish output or select a clean folder.
`NuGet.Publish.Config` enables apphost-pack restores only for publishing; ordinary
development builds still use the offline config.

The launcher first checks its saved emulator preference, then its bundled
`Runtime/BoxedwineEngine.exe`, then known development outputs. Settings can select
another emulator executable. Prefer the native runtime built by this script;
older emulator binaries may be incompatible with current Wine filesystems.

For an isolated development library:

```powershell
./project/msvc/BoxedwineUI/bin/Release/net10.0-windows/Boxedwine.exe --library C:\Boxedwine\tmp\my-test-library --theme Dark
```

`--emulator` overrides the emulator path for that launch. The library can also be
selected through `BOXEDWINE_LIBRARY_DIRECTORY`. The normal library is
`%LOCALAPPDATA%\Boxedwine`. The library location in Help opens that folder in
File Explorer, or the selected library folder when an override is used.

## Workflows

| Mac workflow | Windows UI |
| --- | --- |
| App library, recent apps, search | Library sidebar, searchable app cards, details pane |
| Add app | Single installer, complete installer folder, or portable app folder; file drop supported |
| Choose program after setup | Program picker, with setup/maintenance tools marked; queued if another operation is active |
| Demos | Same pinned catalog, icons, 34 recipes, Wine versions, typed display/compatibility options |
| Notepad and Minesweeper | Sidebar and Help menu; each has a private Windows environment |
| App settings | Name, program, window size, full screen, Windows version, custom icon, renderer/backend and arguments |
| Run another program | Installed executable or external EXE/MSI; external folder is mounted directly after confirmation |
| Wine versions | Download/import/verify default support; package pinned per app; independent trial copies |
| Backup and restore | `.boxedwinebackup` folders with manifest, file hashes, app settings and embedded Wine snapshot |
| Remove/restore/delete | Removed Apps, permanent deletion, delete all, optional immediate deletion policy |
| Unfinished work | Journaled app imports, restores, trials and backup exports; checked before completing or discarding |
| Troubleshooting | Program chooser, rerun installer, Wine trial, live logs, export logs, storage/files |
| Help and licenses | In-app help, keyboard shortcuts, GPL and Wine icon license text |

The Mac App Store tip/purchase interface is platform-specific and is not included.
Keyboard shortcuts follow Windows conventions and are listed in Help and menus.

## Data and process behavior

- Library JSON uses Mac-compatible field names, UUIDs, base64 PNGs, and Swift's
  2001 date epoch; versions 1–13 are accepted and writes use version 13.
- Apps have private `Applications/<UUID>/root` trees. Identical Wine ZIPs share
  `WinePackages/<sha256>.zip`. Removed apps and unfinished copies retain their Wine.
- The backup format matches the Mac format through version 8. Restoring creates a
  new app identity and imports the Wine snapshot into shared storage.
- Native host symlinks/junctions and names that cannot be represented on Windows
  are rejected. Guest `.link` files are preserved. Consequently, not every possible
  Mac backup is transferable; cross-machine backup exchange has not been tested.
- Metadata writes are atomic, a second launcher cannot open the same Windows
  library, and an unreadable library is never replaced with an empty one. Recovery
  journals and launcher preferences are Windows-specific.
- File work and downloads run in the background with progress and cancellation.
  A completed copy is inventoried before it becomes visible in the library.
  Interrupted exports retain an ownership marker and are listed in Unfinished Work.
- Windows-version, OpenGL backend and renderer changes run through Wine itself,
  then independently query the result before clearing pending settings or launching
  the app. Resetting the Windows version reads the chosen package's actual default.
  Background configuration uses `-disableLinearMemory` after an intermittent x64
  emulator fault was observed in the Linux shell during repeated Wine commands.
  App launches retain their normal memory settings. A nonzero setup exit or missing
  completion proof always keeps the requested settings pending.
- Processes use argument arrays, redirected/drained output, bounded rotating logs,
  and a Windows job object. Stop sends the native quit command, then kills the
  process tree if necessary after five seconds. Quitting waits for file cleanup.
- Launching apps show a banner with an animated Windows progress bar and a Stop
  button. Cards and details say “Launching…” until the emulator reports its first
  visible window, using the same readiness signal as the Mac UI. Exiting or
  stopping during startup clears the banner too.

See [architecture and sharing assessment](ARCHITECTURE.md) for the core boundary
and a proposed route to sharing actual implementation with macOS.

## Verification

The dependency-free tests exercise metadata, package structure/hash/CRC, paths,
ZIP extraction, import/cancellation, MSI and launch argument boundaries, backups,
Wine trials, removed apps, package retention, recovery, and Wine query parsing.

```powershell
cd project/msvc/BoxedwineUI
dotnet restore BoxedwineUI.sln --configfile NuGet.Config
dotnet run --project Tests/Boxedwine.Tests.csproj -c Release --no-restore -- C:\Boxedwine\tmp\native-ui-tests
dotnet run --project Tests/UI/Boxedwine.UIChecks.csproj -c Release --no-restore -- C:\Boxedwine\tmp\native-ui-previews
```

The UI harness renders the app's own WPF tree and opens/closes its own dialogs;
it checks live theme switching and produces 16 preview images. It does not automate
or capture other desktop applications. Interactive desktop access was requested
during development, but its app-specific approval timed out; manual keyboard,
accessibility, native file picker and full-screen interaction checks remain useful.

Optional integration switches for the core test runner:

```powershell
dotnet run --project Tests/Boxedwine.Tests.csproj -c Release --no-restore -- C:\Boxedwine\tmp\native-ui-tests --wine C:\path\TinyCore15Wine11.0.zip --catalog Assets/Catalog/catalog.xml --network --emulator bin/Release/net10.0-windows/Runtime/BoxedwineEngine.exe
```

`--network` downloads and verifies Bang! Bang! and NetSurf into a disposable library.
`--emulator` checks Wine configuration and defaults, runs a Wine command, and starts
and gracefully stops hidden Notepad. The current Wine 11/filesystem 13 pin was used
for real-runtime validation. All 34 recipes parse; this is not a gameplay/installer
compatibility test of all 34 demos. x64 was exercised; ARM64 was not.
