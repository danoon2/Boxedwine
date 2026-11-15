# Boxedwine testing notes

## Preserve original game code

The user generally does not authorize patching game executables to make them
work. Diagnose and fix compatibility in Boxedwine/Wine, verify original media,
and consider the game's intended older operating systems first. A crash on
modern native Windows does not establish that the original game was defective
or justify modifying it. Do not create, run, package, or publish game patches
without the user's explicit approval for that specific exception. The Rogue
Squadron diagnostic patches made on 2026-11-14 are not approved for further use.

## Rogue Squadron: no direct Windows launches

The user explicitly prohibited launching Rogue Squadron directly on the Windows
host on 2026-11-14 after its fullscreen test disrupted the desktop and forced
them to log out. Do not launch its original executable, launcher, or patched
diagnostic copies directly on Windows again. Further game testing must run
inside Boxedwine (desktop or browser), not as a host-native comparison.

## Windows native UI runtime

Build native UI previews with their native runtime:

```powershell
& project/msvc/BoxedwineUI/build.ps1 -BuildRuntime -Test -OutputDirectory C:\Boxedwine2\tmp\native-ui-preview
```

`-Emulator` must point to a runtime built with
`/p:BoxedwineStandaloneRuntime=true`, normally
`project/msvc/BoxedwineUI/artifacts/runtime/x64/Release/BoxedwineEngine.exe`.
Do not supply an ordinary command-line/recorder `BoxedWine.exe`, even if it has
the latest graphics fixes. Renaming that executable does not make it a native
runtime. Build the native target from the same updated sources instead.

The command-line/recorder build can return 1 after a normal no-playback shutdown.
The native runtime returns 0 and implements the launcher's stdin lifetime/quit
protocol. Native UIs deliberately require a successful installer exit before
automatically selecting the catalogue's `ShortcutExe`. Do not reinterpret all
exit-code-1 failures as success or weaken this check to accommodate a bad bundle.

In addition to the normal UI tests, verify a supplied runtime with an actual
Wine filesystem using the `--emulator`, `--wine`, and `--catalog` options of
`project/msvc/BoxedwineUI/Tests/Boxedwine.Tests.csproj`. Those checks exercise
Wine configuration, normal command exit and graceful shutdown over stdin.

Keep installer tests in isolated roots. Native demo packages must contain the
original installer; installed browser overlays are separate packages. Preserve
the user's existing library and selected programs when refreshing a preview.

See [the Windows UI guide](../project/msvc/BoxedwineUI/README.md),
[demo catalogue workflow](../resources/DEMO_CATALOG.md),
[Jenkins/local browser demo notes](../tools/jenkins/instructions.md), and
[Wine filesystem build notes](../tools/buildWine/README.md) for their commands
and conventions.
