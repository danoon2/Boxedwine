# Native Linux UI

This frontend uses Python 3, GTK 4.14+, and libadwaita 1.5+. It runs the C++
Boxedwine emulator as a child process. All frontend sources and build integration
live in `project/linux`; the Mac icon assets, release pins, and demo catalog
validator remain shared repository resources.

## Supported baseline

Native packages: Debian 13+, Ubuntu 24.04+, Linux Mint 22+, or Fedora 40+ with
GTK 4.14 and libadwaita 1.5. Use a currently maintained Fedora release. The source
supports x86-64 and AArch64; compile the engine on the target architecture.
Python 3.12+ is the tested baseline (the backend uses Python 3.10-compatible syntax).
Debian 12's normal packages do not meet the GTK/libadwaita baseline. A newer
isolated runtime, such as a future Flatpak package, is needed there.

Install the UI dependencies on Debian/Ubuntu/Mint:

```sh
sudo apt install python3-gi gir1.2-gtk-4.0 gir1.2-adw-1
```

On Fedora:

```sh
sudo dnf install python3-gobject gtk4 libadwaita
```

The emulator also needs the existing Linux build dependencies from
`project/linux/buildInstructions.txt` (C++20 compiler, SDL2, OpenGL, zlib,
curl, and OpenSSL development packages).

## Build and run

From `project/linux`:

```sh
make native-ui JOBS=8
./Build/NativeUI/boxedwine-ui
```

The existing `release` and `test` targets keep their existing behavior.
`native-runtime` builds `Build/Native/boxedwine-engine` with the legacy UI
disabled and the native-launcher stdin protocol enabled. `native-ui` stages a
relocatable frontend, engine in `Runtime/boxedwine-engine`, resources, desktop
entry, and licenses in
`Build/NativeUI`. Staging does not need GTK development headers or network access.
If the exact pinned demo catalog is in the shared build cache, it is bundled.
Otherwise the frontend downloads and verifies it on first use of Demos.

Jenkins packages both architectures in the combined build archive:

```text
Linux/
  x64/
    boxedwine-ui
    Runtime/boxedwine-engine
    CommandLine/boxedwine
    ui/                     # Python frontend and shared resources
    org.boxedwine.Boxedwine.desktop
    README.md
  arm64/
    ...                     # Same layout, built on the ARM64 worker
```

`x64` is Linux's `x86_64`; `arm64` is also called `aarch64`. The UI locates
`Runtime/boxedwine-engine` relative to its own launcher, regardless of the working
directory or where the archive is extracted. `--emulator` and a saved custom
engine still take precedence. `CommandLine/boxedwine` is the independent legacy
command-line/automation build; the UI uses its separate native runtime for the
launch/stop protocol. Keep the entire architecture folder together.

To include the standalone build when staging locally:

```sh
make release
make native-runtime JOBS=8
python3 ui/build.py --console Build/Release/boxedwine
```

Run directly from the source tree after `make native-runtime`:

```sh
./boxedwine-ui
./boxedwine-ui --library /tmp/boxedwine-test-library --emulator /path/to/boxedwine-engine
```

Optional installation (no installation happens during the normal build):

```sh
python3 ui/build.py --prefix "$HOME/.local"
# Distribution packaging example:
python3 ui/build.py --prefix /usr --destdir /tmp/boxedwine-package
```

Put the prefix's `bin` directory on PATH for desktop launchers. A package should
declare its GTK, libadwaita, Python GObject, SDL2, OpenGL, curl, OpenSSL and zlib
runtime dependencies. This is a native source/build integration, not yet a
signed Debian/RPM/Flatpak distribution package.

## Behavior and data

The default library is `$XDG_DATA_HOME/boxedwine`, or
`~/.local/share/boxedwine`. `--library` selects a separate library. The engine can
be selected with `--emulator` or in Settings. A file lock prevents concurrent
Linux frontends from editing the same library. Reopening a library activates its
existing window; `--open APP_ID` also opens that saved app through the running
launcher.

The launcher registers its bundled icon even when run without installation.
Launched apps use their library icon (custom, demo, built-in, or extracted EXE)
on X11. Matching hidden desktop entries supply the identity/icon on Wayland;
they are stored in `$XDG_DATA_HOME/applications` and point to icons under the
library's `LinuxDesktop` directory. Apps without an icon use the Boxedwine icon.
Entries refresh on launch and do not replace manually installed desktop entries.

Preparation verifies Wine and applies any pending compatibility settings. Wine
checksum results are cached while the package remains unchanged. A progress
dialog appears only after 300 ms, waits for libadwaita's initial presentation
frames before closing, and finishes closing before the launch callback runs.

- Library, recent apps, search, demo catalog, built-in Notepad and Minesweeper.
  Single-click a card for details; double-click an app to open it or a demo to
  install it. Installed demos open their existing copy; removed demos reveal
  their entry in Removed Apps.
- Installer file, installer folder, portable folder, and file-drop import.
- Choose an installed program after setup; run another program in the same
  Windows environment, including an external EXE/MSI after folder-access consent.
  Run Another Program includes Wine Configuration, Registry Editor, Command
  Prompt, File Manager, Add/Remove Programs, Wine Internet Explorer (blank page),
  and Notepad. These tools use the selected app’s root and pinned Wine package;
  they never replace its saved executable or inherit its program arguments.
- Native app settings for name, program, display size, full screen, Windows
  version, icon, Wine renderer, Wine GLX/EGL interface, and arguments. There is
  no Windows OpenGL implementation selector (Mesa/LLVMpipe/Zink/D3D12).
- Verified Wine downloads, offline import of the pinned release, and shared
  content-addressed package storage. App packages are pinned independently.
- Mac/Windows version-13 library metadata and version-8 `.boxedwinebackup`
  folders with manifests, file hashes, and embedded Wine. Restoring a backup
  creates a new app identity. Use backups to transfer apps across hosts; do not
  share a live library with a different platform's frontend.
- Removed Apps, restore, permanent deletion, optional immediate deletion,
  journaled file-copy recovery, storage details, and unused-package cleanup.
- Troubleshooting, installer rerun, bounded/rotated logs and export, help,
  keyboard shortcuts, and license information.

After an auxiliary program exits, the launcher reads the environment’s global
Windows version, DirectDraw/Direct3D renderer values, and Wine GLX/EGL setting
back into App Settings. Readback never applies settings. Unrepresentable registry
values appear as “Custom (set in Wine)” and are preserved until the user explicitly
chooses a supported setting. Explicit UI changes still waiting to be applied are
kept. Failed/interrupted readback is retried before launching or opening App
Settings again; the retry marker persists if the launcher closes with a tool open.
The readback log is `Logs/configuration.log` inside the app directory.

Windows version and Wine graphics changes remain pending until guest commands
confirm the requested result. A failed preparation never silently launches with
unconfirmed settings. App launches preserve argument boundaries, never use a
host shell, and stop through the engine's `quit` protocol, with process-group
termination as a fallback. Output is drained even when the 4 MB log limit is
reached. Host symlinks and archive traversal are rejected during imports and
backups; Boxedwine guest `.link` files remain ordinary data files.

The engine and Wine are compatibility tools, not a security sandbox for hostile
Windows software. External programs receive writable access to the selected
host directory. Downloads are restricted to checksum-pinned boxedwine.org URLs.

## Validation

```sh
make test-ui
python3 ui/tests/gui_smoke.py --output /tmp/boxedwine-ui-smoke
python3 ui/tests/launch_ui_smoke.py  # Progress lifecycle, icons, and desktop activation
python3 ui/tests/pointer_smoke.py  # X11 + libXtst; uses actual pointer clicks
python3 ui/tests/runtime_smoke.py --wine /path/to/TinyCore15Wine11.0.zip
python3 ui/tests/wine_tools_smoke.py --wine /path/to/TinyCore15Wine11.0.zip
python3 ui/tests/wine_tools_ui_smoke.py
```

The backend tests use temporary libraries and synthetic guest filesystems, with
no network or display required. The GUI smoke test uses the installed native
GTK stack and a display, creates its own temporary library, visits key screens,
and captures screenshots on X11 when ImageMagick is available. For a real launch,
use a separate test library, set up the pinned Wine package, and run Notepad or
Minesweeper. Test Wayland and AArch64 on those environments before distributing
architecture-specific binaries. Packaging tests also check the relocated archive layout,
engine lookup, executable permissions, and staged prefix installation.

The Linux-only native control helper uses descriptor reads for stdin: glibc's
blocking `fgets` otherwise holds a stream lock that can prevent a naturally
exiting engine from finishing. The small integration hooks in
`source/sdl/main.cpp` select this helper and a successful native-runtime exit
status; legacy Linux launches and other platforms retain their existing paths.

Validated on Linux Mint 22.1 x86-64 / X11 with GTK 4.14.5, libadwaita 1.5.0,
and Python 3.12: 30 backend/packaging tests; 16 native screen/interaction checks; real Wine
11.0 V13 configuration, default reset, Notepad and Minesweeper startup/stop;
AbiWord download/import and launch/stop through the staged UI; launcher and guest
X11 window icons, per-app window identity, progress-dialog timing/cancellation/
error handling, and desktop activation of an existing launcher; Win16 portable
and installer-recipe imports; PE/NE icon extraction; and a staged `/usr`
installation with a validated desktop entry. Fedora, Debian, Wayland, and
AArch64 have not been exercised on this host.
