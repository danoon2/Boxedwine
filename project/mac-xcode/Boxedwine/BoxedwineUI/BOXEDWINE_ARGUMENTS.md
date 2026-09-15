# Advanced Boxedwine arguments

App Settings → Advanced has separate **Boxedwine arguments** and **App arguments**. Each line is one argument; an option's value belongs on the next line. Blank lines in Boxedwine arguments are ignored, CRLF is accepted, and spaces within values stay intact without shell expansion or surrounding quotes. Supported Options provides the current list and an example.

For demos with typed launch options, Advanced also shows a selectable **Demo launch arguments** block above the editable Boxedwine arguments. It uses the same `DemoSettings.launchArguments` function as the actual launch, exposing color depth, CPU affinity, the DirectDraw override path, and cursor options for both existing and newly imported demos. The path follows the selected program; before a program is selected it displays `(program folder)`. The label explains that these defaults apply to app launches and that editable Boxedwine arguments can override matching values. Glide environment presets already appear in the editable field. Viewing or saving settings does not copy game-only options into installer launches or alter saved demo defaults.

All three editors share an adaptive text-input background, rounded separator border, internal padding and keyboard-focus outline. Their colors come from AppKit's semantic colors. Native visual checks covered empty and populated fields, typing after clicking the field padding, focus moving between editors, and scrolling with Save/Cancel visible in both dark and light appearances. Light mode was forced only in a temporary test bundle; the regular app follows the system appearance. Both styled Debug and Release builds passed packaging checks. Argument parsing and storage are unchanged by this presentation change.

The Name field uses the same inset border and focus outline. This keeps its right edge aligned with the argument editors and prevents its focus ring from being clipped at the scroll area's top and right edges. Dark and light checks covered collapsed and expanded settings; editing, undo and focus transfer were also checked in the isolated app.

```
-nosound
-cpuAffinity
1
-env
WINEDEBUG=-all
```

Boxedwine arguments apply to app and installer launches. They follow typed demo launch options so matching scalar values can override those defaults. Notepad uses the same rule. App arguments remain after the executable. The launcher's separate Windows-version configuration commands keep their controlled settings.

```
Boxedwine [managed settings] [demo options] [Boxedwine arguments] \
  [managed mounts and working directory] /bin/wine program.exe [app arguments]
```

`Core/BoxedwineArguments.swift` checks the supported native subset against the parser in `source/sdl/startupArgs.cpp`. It checks option arity and values before a command can reach the runtime: the runtime otherwise treats the first unknown token as the program, and a missing value can consume the managed `/bin/wine` boundary. Validation runs in the editor, on library save/load, on recovery and backup import, and when building a launch request. Invalid input is shown inline and disables Save.

Supported flags include sound, CPU model, cursor, DPI, read caching and linear-memory controls. Supported values cover CPU count, color depth, vsync, scale and scaling quality, mouse sensitivity, polling, frame skipping, DXVK, OpenGL extensions, and repeatable guest environment variables. Values have explicit bounds; total input is limited to 256 arguments and 64 KiB. The in-app list is generated from the same definitions as validation.

Mac builds use native OpenGL. Older library, backup, and recovery metadata containing `-opengl osmesa` remains readable; launch construction removes that obsolete pair, and saving edited settings removes it from the saved arguments. Other options and environment values are preserved. The Mac runtime also treats a legacy command-line request for `osmesa` as native OpenGL, so old launch commands cannot try to load a removed library.

Root directories, Wine ZIPs, mounts, working directories, titles, logging destinations, display size/fullscreen and launcher lifecycle options remain managed. Arbitrary host OpenGL libraries and unsupported/build-specific switches are not accepted in this preview field. Clearing the field restores the ordinary launch defaults.

The optional `boxedwineArguments` app property preserves old library decoding. Nonempty settings require library format 9, recovery format 6 and backup format 5 so older launchers cannot silently discard required options. Existing apps without these settings keep their prior format unless their library was already upgraded. Settings travel with removed/restored apps, Wine test copies, recovery and backups.

## Verification

The demo-argument visibility update passed seven focused existing tests covering the bundled catalog, app/installer argument placement, parsing, persistence and backups, plus the Debug build and eighteen-image packaging audit. A native dark-mode check of the existing Drowned God app showed its inherited `-bpp` / `16` above the editable arguments. No app settings were saved during this check.

The default suite passed 153 executed tests (155 total; two integrations remain opt-in). New checks cover missing/invalid values, unsupported and managed switches, literal spaces/shell characters in environment values, demo override ordering, independent program arguments, Notepad and installer launch placement, Java argument separation, legacy loading, rejected saves preserving the original document, and recovery/backup format enforcement and round trips. Debug and Release passed the eighteen-image packaging audits.

In the isolated signed, sandboxed Debug app, entering `-cpuAffinity` without a value displayed an inline error and disabled Save; `-root` did the same with a managed-option explanation. Supported Options exposed the generated reference. The expanded form scrolled to all three argument fields while Save and Cancel remained visible. Saving `-nosound`, `-cpuAffinity`/`1`, and `-env`/`LABEL=two words` persisted format 9 and launched the Java 8 fixture. The runtime reported CPU affinity 1, while the Swing window and log showed the original VM property and three original app arguments with spaces preserved. It exited with code 0. The regular library's 151 file hashes and app metadata were unchanged; only the selected isolated fixture's settings, launch log and guest registry files changed among existing files. These checks do not establish compatibility of every supported flag with every game.

September 14: dedicated Java support was removed. Advanced now contains only the Boxedwine and app argument fields; the Java fixture results above describe earlier builds.
