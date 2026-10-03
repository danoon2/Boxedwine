# Validation — 2026-10-03

The x64 Release build and tests ran on the Windows development machine.

| Check | Result |
| --- | --- |
| `build.ps1 -BuildRuntime -Test` | Native emulator and WPF app built successfully |
| Dependency-free core regression suite | 14 checks passed |
| Complete suite with real Wine, catalog, network and emulator | **18 passed, 0 failed** |
| WPF visual/theme harness | Passed; 16 light/dark previews generated |
| `git diff --check` | Passed |

The complete suite validated Wine 11.0, filesystem 13, SHA-256
`8363a62eac5f95f942160b82b4e25706857a99da0b46d83f7dd4c604c5bf3818`.
It applied and queried Windows 98, GDI and GLX, restored the package defaults,
verified persisted pending flags were cleared, checked real Wine command output,
and started and gracefully stopped hidden Notepad. Bang! Bang! and NetSurf were
downloaded, verified and staged as portable/installer examples. All 34 pinned
catalog recipes parsed.

The regression suite includes path/ZIP rejection, CRC corruption, concurrent
library access, media-folder/MSI launches, cancellation cleanup, backup tampering,
trial isolation, package retention/reuse under a read lease, interrupted-copy
integrity, and backup ownership checks.

An earlier configuration run exposed an intermittent x64 emulator page fault in
BusyBox while resetting renderer settings. Background Wine configuration now uses
`-disableLinearMemory`; both subsequent configuration sequences completed. Setup
requires a zero runtime exit, a matching completion token and an independently
parsed query result. Failure keeps the settings pending and the app closed. This
is a launcher mitigation; the underlying emulator fault was not diagnosed here.

The visual harness tested library, demos, selected-app details, Add App, Settings,
App Settings, expanded Advanced options and Troubleshooting in both themes, plus
live theme changes on open windows. Previews are in `artifacts/previews` after
running the documented UI check command with that output directory.

Desktop app-control approval was requested but timed out. These checks used the
app's own WPF rendering/test harness and real emulator subprocesses. Manual desktop
interaction, screen readers, multiple monitor/DPI transitions, high contrast,
native picker behavior, ARM64, Mac-to-Windows backup exchange, and compatibility
of every demo remain unverified. No existing user library was used for testing.

## Launch feedback follow-up

- The core suite now has 15 checks, including complete-line window readiness,
  every pipe split, repeated markers, misleading output and long output lines.
- The WPF harness checks actual animated pixels in light and dark mode, plus
  launching-to-running, exit before a window, and stop before a window. Late
  readiness output cannot restore a cancelled launch banner. It renders 18 previews.
- Real Wine Notepad reports the same `Showing Window` signal as the Mac launcher.
  Immediate Stop after its first window can need the existing five-second forced
  cleanup. Sending both stdin quit and WM_CLOSE also produced an access violation;
  the launcher now uses WM_CLOSE only if the stdin quit command cannot be sent.
  Emulator shutdown timing itself has not been changed by this UI update.

## Product naming and library location follow-up

- The Windows default library is `%LOCALAPPDATA%\Boxedwine`. The existing local
  library was moved with its apps, Wine packages and settings; library metadata
  and settings checksums were verified unchanged.
- Help now links to the actual library folder in File Explorer. The WPF harness
  verifies the target for an overridden library and renders Help in both themes.
- The standalone-runtime build property was renamed consistently in the script
  and Visual Studio project; the full Windows build and 15 core checks passed.
- Mac naming and default-path references were updated too. Its library code can
  adopt a unique preview library by recognizing its metadata and lock, without
  hard-coding an obsolete folder name or merging existing libraries.
  All five library-access checks passed on the Mac in an isolated test directory;
  its installed app and library were not modified.

## Installer demo program selection follow-up

- After a normal installer exit, demos select the uniquely matching catalog
  executable automatically. A valid existing choice survives a reinstall.
  Missing/ambiguous matches, failed or stopped installers, and manual imports
  retain the program picker.
- All 18 offline core checks passed, including metadata preservation, matching
  with mixed case and spaces, exclusion of Wine system programs/setup media,
  missing/duplicate matches, and unsuccessful installer exits.
- The WPF harness exercised installer completion and deferred selection while
  the owner was disabled in both themes: an exact match was saved without any
  dialog, and duplicate matches opened the picker. These checks used isolated
  libraries and a controlled runtime subprocess, not the user's installed apps.
- The normal Release output was rebuilt with zero warnings and errors.

## Wine launch checksum follow-up

- Launches with a stored package identity (including the library default) check
  the ZIP's size and SHA-256 without opening or decompressing its entries.
  Successful checks are cached by package path, size, modification time and
  expected identity for the launcher session, so apps sharing Wine reuse them.
- Import validation remains available. Older per-app snapshots without a stored
  checksum use the existing full validation on their first launch in a session.
- All 20 offline checks passed, plus the real Wine ZIP check (21 total). New
  checks cover opaque-byte checksum verification, same-size corruption, cached
  verification, changed expectations/files, cancellation and older snapshots.
- One local Wine 11.0 measurement: full validation 2104 ms, SHA-256 launch check
  142 ms, unchanged shared cache 0.2 ms. The hash measurement followed the full
  read and therefore benefited from the OS file cache; timings vary by storage.

## Jenkins Windows packaging follow-up

- Ran the new Jenkins packaging helper locally for Win32, x64 and ARM64. All
  three completed, including both native engine variants for each 64-bit target
  and self-contained .NET 10.0.10/WPF publishes for win-x64 and win-arm64.
- The core suite passed all 20 checks during both UI packaging runs. These runs
  used the local x64 SDK; ARM64 was cross-built, not executed on this machine.
- Package checks verified launcher, engine, console and bundled runtime PE
  architectures, GUI/console subsystems, self-contained runtime configuration,
  and catalog/license presence. An intentionally mislabeled x64-as-ARM64 package
  was rejected, and a stale ARM64 output sentinel was removed before packaging.
- The published x64 UI assemblies passed the existing WPF harness under a
  self-contained test host, including both themes and runtime lifecycle checks.
- The Jenkins archive shell block passed syntax validation and produced the
  local combined Windows ZIP with Win32, Win64 and WinARM64 folders. No installed
  user library was used, and no Jenkins job or deployment was triggered.
- Native ARM64 execution and the full recorder/performance pipeline remain for
  the Jenkins workers. Both Windows workers need the .NET 10 SDK and NuGet.org
  access for their first runtime-pack restore.

## Windows OpenGL fullscreen follow-up

- Ported the behavior from Mac commit `509f6a754`: preserve the guest framebuffer
  size, scale presentation to the fullscreen window, and use the same viewport
  for mouse positions, relative movement and cursor recentering. The viewport
  math is now shared by Mac and Windows.
- Windows uses a WGL pbuffer and a GPU framebuffer blit, preserving guest front,
  back, depth and stencil buffers. Borderless fullscreen stays on the desktop
  during the GDI/OpenGL focus handoff.
- `tools/test_windows_opengl_fullscreen.ps1` passed on the local NVIDIA driver:
  actual fullscreen pixels, aspect/stretch modes, legacy/core contexts, larger
  guest buffers, resize without rebinding, single-buffered presentation, FBO and
  GL state restoration, offscreen rebinding, and the shared cursor/layout tests.
- An isolated Wine 11/Alice demo reached its fullscreen menu. The user confirmed
  that it looked correct and the mouse worked well. No installed game data was
  changed by the test.
- x64 and ARM64 native engines built successfully. The GPU probe also cross-built
  for ARM64; executing it on ARM64 hardware remains unverified. Updated engines
  are copied into the normal x64 UI and both 64-bit deployment folders.

## Installed .NET Desktop Runtime packaging follow-up

- Supersedes the earlier self-contained packaging: x64 and ARM64 now publish
  framework-dependent launchers requiring .NET 10 Desktop Runtime. The combined
  archive still contains Win32, Win64 and WinARM64. Native engine and console
  builds are retained, including the Windows OpenGL fullscreen changes above.
- Both Jenkins packaging helpers completed locally. All 20 core checks passed
  in each run. Package validation passed for all three architectures, including
  PE architecture/subsystem, .NET 10/WPF requirements, resources and licenses.
- Negative checks rejected a stale hostfxr.dll in a package, publishing over an
  old self-contained output, and a runtime configuration targeting .NET 9.
  Jenkins continues to clean each target's output before publishing.
- The existing WPF harness passed against the published x64 assemblies using
  the installed runtime: both themes, launch readiness, early exit, startup
  cancellation, and installer program selection. Tests used isolated libraries.
- Launched the actual published Boxedwine.exe with DOTNET_ROOT_X64 pointing to
  an isolated hostfxr-only folder without shared frameworks. Verified the native
  Windows missing-runtime dialog and its "Download it now" button, then closed
  it without downloading. The installed .NET runtime was not changed. Package
  READMEs link to Microsoft's .NET 10 page and specify Desktop Runtime and the
  architecture to choose.
- Created project/msvc/Deploy/build-windows-ui-framework-dependent.zip: 113
  files, 20,686,805 bytes, compared with 141,724,722 bytes previously (85.4%
  smaller). Verified all three launchers, both engines and installation READMEs
  are present and bundled .NET runtime DLLs are absent. The older ZIP was open
  in 7-Zip and could not be replaced, so the new archive has a distinct name.
- ARM64 was cross-built and inspected, not executed. No Jenkins job or external
  deployment was triggered.

## Optional ANGLE DLL removal

- Jenkins packaging and the native UI build script exclude libEGL.dll and
  libGLESv2.dll. The UI script also removes older copies from its Runtime output.
  Package validation rejects either DLL at the package root or in Runtime.
- Removed both DLLs from Deploy/Win64 and the current framework-dependent ZIP.
  All three deployment folders passed package validation. The ZIP now contains
  111 files and is 17,148,791 bytes; every entry's SHA-256 matches its deployed
  file. All edited PowerShell scripts passed syntax checks.
- A clean x64 packaging attempt encountered the user's running Boxedwine.exe
  and its locked assembly. Restored the partially cleaned folder from the
  previously verified ZIP before removing the two optional DLLs. The running
  instance was left open. No application code changed or runtime tests reran.

## Wine 11 V13 release selection

- The shared native XML/JSON release catalog contains only the checksum-pinned
  Wine 11.0 V13 package. Windows and Mac hide Wine selection and trial-copy
  controls and their help/troubleshooting guidance when the catalog has one
  entry. Windows also hides arbitrary Wine ZIP import in that mode. Setup of
  the supported package remains available, and existing app package records
  are preserved. Adding another supported catalog entry restores the controls.
- Windows core checks passed (20). The WPF harness passed in light and dark
  themes, including single-package controls, exclusion of a legacy imported
  default from new selections, and restoration of pickers/trials/help with a
  synthetic second catalog entry. Existing launch/installer checks also passed.
- Built and published x64 and ARM64, updated both deployment folders, and
  refreshed build-windows-ui-framework-dependent.zip. Both packaged catalogs
  were checked for the sole Wine 11 V13 entry; package checks passed and no
  bundled .NET or optional ANGLE DLLs were reintroduced. ARM64 is cross-built.
- After explicit approval to copy sources to the Mac, an isolated temporary
  checkout passed all 225 Swift tests in 26 suites, the native SwiftUI type
  check, and compilation/linking of the Mac UI executable. Local edition/resource
  packaging checks passed (4). The user's Mac
  checkout and installed apps were not modified.

- Label follow-up: app screens, Wine choices and app download notices show
  Wine 11.0 without V13; Settings retains the filesystem revision. Windows
  light/dark checks verified both labels, x64/ARM64 publishes and the ZIP were
  refreshed, and the Mac UI compiled/linked with its 9 catalog tests passing.
