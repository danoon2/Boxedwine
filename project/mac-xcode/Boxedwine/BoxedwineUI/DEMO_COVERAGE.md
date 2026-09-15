# Native demo coverage

Catalog release: `26R2-catalog-2` (published and verified by fresh download). This release includes **34 of the 38** entries in the original [26R2 catalog](https://www.boxedwine.org/v2/26R2/filesV2.xml). Java Solitaire and FreeCol are excluded following removal of dedicated Java support. Solitaire DotNet still needs Mono; 3DMark’s legacy `Debug` token still needs clarification.

## Included entries

The original 32 actual payloads passed the native import pipeline: exact size and SHA-256, ZIP extraction/CRC/path checks where applicable, expected program or setup path, the assigned private validated Wine package (9.0, 10.0 or 11.0), and library save/reload. These checks used isolated temporary libraries and executed no guest programs. Successful import does not establish application compatibility.

| Entry | Recipe | Wine | Native settings |
| --- | --- | --- | --- |
| Abiword (2015) | Portable ZIP | 11.0 | Default |
| Age of Empires demo (1997) | Single EXE installer | 11.0 | Windows XP, GDI |
| American McGee's Alice (2000) | Portable ZIP | 11.0 | Native OpenGL through Wine GLX (`UseEGL=false`) |
| Bang! Bang! shareware (1990) | Portable ZIP | 11.0 | Default |
| BassTour Professional shareware (1998) | Installer ZIP → `SETUP.EXE` | 11.0 | Default |
| Broken Sword demo (1996) | Portable ZIP | 11.0 | Default |
| Caesar 3 demo (1998) | Single EXE installer | 11.0 | GDI, Game resolution 800x600, Setup resolution 800x600, Keep cursor visible |
| Castle of the Winds (1992) | Portable ZIP | 11.0 | Default |
| Cinebench 11.5 (2010) | Portable ZIP | 11.0 | Native OpenGL |
| Cities In Motion demo (2011) | Installer ZIP → `cim-demo-1-0-21.exe` | 11.0 | Default |
| Daytona USA demo (1996) | Single EXE installer | 9.0 | GDI, Game resolution 640x480 |
| Deadlock demo (1996) | Installer ZIP → `SETUP.EXE` | 11.0 | Default |
| Descent 3 demo 2 (1998) | Installer ZIP → `Setup.exe` | 11.0 | Preinstalled psVoodoo, WineD3D OpenGL |
| Diablo II Shareware (2000) | Single EXE installer | 11.0 | Default |
| Drowned God demo (1996) | Portable ZIP | 11.0 | Windows 98, Color depth 16 |
| F-16 Multirole Fighter demo (1998) | Installer ZIP → `SETUP.EXE` | 11.0 | Preinstalled psVoodoo, WineD3D OpenGL |
| F-22 Raptor demo (1997) | Single EXE installer | 11.0 | Default |
| Fire Fight demo (1996) | Single EXE installer | 11.0 | Default |
| Full Tilt! Pinball Demo (1995) | Portable ZIP | 11.0 | Default |
| Half-Life: Uplink demo (1999) | Single EXE installer | 11.0 | GDI menus, native OpenGL through GLX; choose OpenGL in the game's Video modes |
| MDK Performance (1999) | Portable ZIP | 11.0 | Game resolution 640x480, CNC DDraw with automatic rendering, uncapped benchmark timing |
| Mech Warrior 3 demo (1999) | Single EXE installer | 11.0 | Default |
| Motorhead 3Dfx demo (1998) | Single EXE installer | 11.0 | Windows 98 before setup, preinstalled psVoodoo, WineD3D OpenGL, 640×480 game |
| Need for Speed: Hot Pursuit 2 demo (2002) | Single EXE installer | 11.0 | Default |
| NetSurf 3.10 (2020) | Single EXE installer | 11.0 | Default |
| Nitemare 3D (1994) | Installer ZIP → `N3WUNR.EXE` | 11.0 | Default |
| Norse By Norse West (1997) | Portable ZIP | 11.0 | 32-bit desktop, CNC DDraw, private 320×240×16 game profile |
| RollerCoaster Tycoon demo (1999) | Single EXE installer | 11.0 | Default |
| Spherejongg (1995) | Portable ZIP | 11.0 | Default |
| Thief demo (1998) | Portable ZIP | 11.0 | Windows XP, CPU count 1, Native OpenGL |
| Tomb Raider 3 demo (1998) | Single EXE installer | 11.0 | Windows 98 |
| vkQuake (1996) | Portable ZIP | 9.0 | Game resolution 1280x720, Relative mouse |
| WinRoids (1994) | Portable ZIP | 11.0 | Default |
| WinTrek (1992) | Portable ZIP | 11.0 | Default |

Payload URLs, byte counts and hashes are recorded in `catalog.xml` inside the [shared pinned catalog ZIP](../../../../resources/DEMO_CATALOG.md). All 36 icons are bundled. Cities in Motion uses a 48×48 PNG exported from the largest icon in its installed executable, preserving the original alpha channel; its legacy online icon was unavailable. No artwork is fetched at runtime. Existing Pinball and NetSurf IDs are unchanged.

Fire Fight, MDK Performance, and Norse By Norse West now target Wine 11.0 for user testing. Their Wine 10 requirements and Fire Fight’s old installer-hang warning were removed. Fire Fight remains pending retesting; MDK and Norse rechecks are below. The earlier Wine 10 observations below remain historical results. Existing installed copies retain their saved Wine packages and can be tested through **Try Another Wine Version…**.

### MDK Performance Wine 11 recheck (2026-09-12)

The regular installed copy reproduced a black screen with CNC DDraw’s automatic renderer under Wine’s default backend and explicit GLX. Its private `renderer=gdi` setting restored the benchmark. That workaround was retained until the September 13 filesystem refresh supplied the corrected CNC DLL.

Tracing the exact bundled CNC DDraw 6.9 revision (`a902db06e9830a9feafda69da05c766a81722b9b`) identified a wrapper loader bug: `glGetIntegerv` was requested through `wglGetProcAddress`, which returns NULL for that OpenGL 1.1 export under Wine. CNC then calls legacy `glGetString(GL_EXTENSIONS)` inside its core context, receives `GL_INVALID_ENUM`, and falls back to GDI while Boxedwine still displays its empty OpenGL window. Loading `glGetIntegerv` directly from `opengl32.dll` fixes the OpenGL path.

The patched debug DLL passed CNC’s texture and shader checks and rendered MDK on the original runtime. A release DLL then rendered the existing MDK installation through the regular native UI with `renderer=auto`; Escape exited with code 0. No emulator OpenGL change was retained. The shared Wine ZIP is unchanged, and only MDK received a private DLL plus its restored automatic renderer setting. Other apps’ metadata is unchanged. The Debug build and bundle audit passed.

The patch and repeatable release build are in `tools/cnc-ddraw/`; evidence and the DLL, source ZIP and MIT license are in `tmp/native-ui-test/mdk-opengl/release/`. Once the DLL is incorporated into the Wine filesystem, refresh its package size/checksum and remove the catalog’s MDK GDI override. Existing MDK’s private DLL should then be removed if it matches the new shared one, so future filesystem updates can supply CNC DDraw normally.

MDK’s recipe now includes `CNCDDrawUncapped=true`, which installs `maxfps=0`, `vsync=false`, and `maxgameticks=-1`. Disabling only `maxfps` left CNC’s default 60 Hz flip/vblank timing active. In the user’s reinstalled GDI copy, disabling that second limit raised the benchmark score from about 241 to 488; the user confirmed the improvement. These are benchmark scores, not measured FPS. The setting is private to this demo, with no change to other demos or the shared Wine ZIP. All 215 Swift tests in 25 suites passed, covering catalog compatibility, INI preservation and ambiguity rejection, missing-file cleanup, recovery, backups and Wine test copies.

### CNC filesystem refresh (2026-09-13)

The downloaded filesystem now contains the exact tested release CNC DLL. Catalog release `26R2-native-preview-8` removes MDK’s GDI override and retains its uncapped timing. Earlier private-DLL observations above describe the original investigation. The regular installed MDK benchmark rendered with the refreshed shared ZIP and automatic renderer, retained uncapped timing, and exited with code 0. Norse’s menu also rendered without the software-rendering banner using the shared DLL and its working private INI; Stop App completed with code 0.

Norse’s existing installation rendered its intro and menu through OpenGL using the patched DLL, `-bpp 32`, and a private `[NORSE95]` section with `fake_mode=320x240x16`. The uploaded filesystem includes the DLL but not that INI section. Its fresh-install recipe therefore remains unchanged pending a coordinated INI/32-bit update. The working installed copy keeps its private INI and saved 32-bit argument. Full Norse gameplay still needs user testing.

### Norse without CNC (2026-09-13)

The user confirmed the regular installed Norse demo works well with CNC disabled,
`-bpp 16`, and Wine's default renderer/backend. No explicit GDI renderer override
was applied. At 32-bit color, the game instead displays a requirement for 256
colors or 16-bit color. A temporary black window and Wine's OpenGL initialization
errors during the 16-bit run did not establish a rendering failure; the user
confirmed success afterward. The installed copy retains CNC disabled and 16-bit
color. Its old private CNC INI is unused. The bundled demo recipe remains unchanged
by this trial. Evidence: `tmp/native-ui-test/norse-without-cnc/`.

### Norse scaling recipe (2026-09-13)

The user preferred CNC's scaling after confirming that Wine's default renderer works but produces a small window. CNC and the tested 32-bit desktop are restored for the installed copy. Catalog release `26R2-native-preview-9` now uses `BitsPerPixel=32`, `CNCDDraw=true`, and `CNCDDrawFakeMode=320x240x16`. New installations generate their own `[NORSE95]` section in a private `C:/ddraw/ddraw.ini`; game-specific overrides do not need another shared filesystem upload. The DLL remains shared. The earlier pending-INI notes above describe the previous approach and are superseded by this recipe. All 221 tests in 26 suites passed, including private INI preservation, backup/recovery/Wine-copy persistence, and schema compatibility. An offline fresh import of the actual Norse ZIP with the current Wine 11 ZIP passed. The rebuilt native app passed its bundle audit, and the restored regular installation displayed the scaled demo scene.

## Glide additions

Both payloads passed the actual native import pipeline using the refreshed Wine 11 filesystem: checksum/size, complete ZIP extraction for Descent, private installer media, shared Wine pinning, and save/reload. The Wine download is 167,059,472 bytes and contains the exact `glide2x.dll` tested with Motorhead. Total downloads without a local matching Wine package are 205,190,211 bytes for Descent and 177,756,005 bytes for Motorhead; once Wine is available only the game payload is downloaded.

Motorhead’s earlier isolated playtest rendered the game and the user confirmed the controls worked. It used the same DLL in a private overlay with a matching filesystem-11 runtime. This is not a playtest of the refreshed ZIP through the native helper: the earlier test preceded the native helper’s graphics update. A new isolated smoke check with the rebuilt native helper rendered Motorhead’s textured race using the preinstalled DLL; normal Motorhead shutdown through that helper still needs checking. Descent 3 has not yet passed a psVoodoo gameplay check. See [Glide support](GLIDE_SUPPORT.md).

## Runtime checks

### Half-Life: Uplink filesystem-11 recheck (2026-09-12)

The regular installed demo reproduced a black gameplay screen while its menus remained visible. It used Wine 9.0 filesystem 5 and the game's Software renderer. Current Boxedwine removes the incompatible guest `libGL.so.1` from filesystem versions below 10; Wine logged that OpenGL support was disabled. A Wine 11.0 filesystem-11 test copy made through the regular native UI could load OpenGL, but the menus were black with both Wine's default backend and explicit GLX.

Setting Wine's `DirectDrawRenderer` and `renderer` to `gdi` restored the menus. Selecting **Configuration → Video → Video modes → OpenGL**, with the Default driver at 640×480, then rendered the opening text, HUD, and textured game scene through native OpenGL/GLX. The user confirmed the game looks right and responds normally. This was a short gameplay smoke check, not a full demo playthrough. No emulator rendering code changed for this configuration fix.

The recipe now selects Wine 11.0, seeds GDI and GLX for new installations, and explains the in-game OpenGL selection. It does not silently change the game's own saved renderer choice. Existing installations retain their pinned Wine package; use **Try Another Wine Version…** to preserve the original while testing an upgrade.

A follow-up comparison copied the current filesystem-11 GL/EGL/GLES bridge libraries into a private Wine 9 package. Only those files and `version.txt` changed; raising the marker to 11 allowed the current runtime to retain `libGL.so.1`. This was a diagnostic package, not a complete Wine 9 filesystem upgrade. Both Wine versions used the same native runtime, identical bridge bytes, GLX, copies of the same installed game, and no GDI renderer override. Both reported Apple M4 / OpenGL 2.1 / GLX 1.3.

| Matched check | Wine 9 with current GL libraries | Wine 11 with current GL libraries |
| --- | --- | --- |
| Main menu | Background and some choices visible; other choices missing | Black |
| Difficulty menu (N) | Correctly drawn | Correctly drawn |
| Game's Software renderer | HUD visible over black scene after the opening fade | HUD visible over black scene after the opening fade |
| Game's OpenGL renderer, Default driver | Textured opening scene and HUD visible | Opening text, textured scene and HUD visible |

These checks show that the Software-renderer failure is shared by both Wine versions and that the copied libraries support native OpenGL gameplay under Wine 9. The menu behavior differs, but Wine 9 also has artifacts when acceleration is available. The working hypothesis is Boxedwine's incomplete GDI/OpenGL composition and drawable presentation, exposed differently by the two Wine versions. The comparison does not identify the exact failing API or establish an upstream Wine regression; it was not repeated in native Linux Wine. The Wine 11 GDI-menu/OpenGL-gameplay recipe remains the tested configuration.

### Alice filesystem-11 recheck (2026-09-11)

Alice's OpenGL/3dfx startup error was reproduced in an isolated copy with the current native helper and the Wine 11.0 filesystem-11 ZIP (`743b6d30…fd9d8e9e`). `opengl32.dll` loaded, but Alice inspected only 255 pixel formats. A Win32 probe found 1,440 formats through EGL, with the first suitable hardware double-buffered window format at 724. With Wine's `HKCU\Software\Wine\X11 Driver\UseEGL` set to `N`, the same helper exposed 312 GLX formats and the first suitable format was 13. Alice selected format 17, created its Apple M4 OpenGL 2.1 context, played the intro, loaded `d_gvillage`, and exited normally. The developer confirmed the test worked perfectly.

The bundled Alice recipe now seeds `UseEGL=false` in its private Windows environment, and Advanced settings displays the chosen Wine OpenGL backend. The optional recipe field leaves other demos at Wine's default and is retained by backups, recovery and Wine test copies. This is an Alice compatibility setting; it does not fix the excessive EGL format enumeration or establish the cause of a gray screen in a separate master build. No emulator OpenGL code was changed for this fix.

### Other checks

Historical Java experiments ran Java Solitaire and FreeCol through Java 8/17. Dedicated Java support and both demo recipes were removed September 14; the earlier runtime results are preserved in Git history.

Pinball and NetSurf were already checked through installation, launch and normal close in the signed native preview. The runtime checks for the earlier expansion are recorded below; they are short smoke checks, not extended gameplay or broad compatibility certification.

These four new entries were downloaded and run through a separately identified copy of the signed, sandboxed Debug app, with an isolated test library and private Wine 11.0 for each entry:

| Entry | Observed result |
| --- | --- |
| Abiword | Rendered a blank document and accepted keyboard input. |
| Bang! Bang! | Rendered the game field, cannons and wind display. |
| Castle of the Winds | Reached the title screen, character creation and spell-selection dialogs. |
| Nitemare 3D | Completed its 16-bit installer, returned to the native chooser with `NITE3W.EXE` selected, saved to the library, and launched to its title window. It displayed a warning recommending 256-color or fullscreen mode at the default desktop settings. |

All four runtime sessions stopped through the native launcher with exit code 0; Nitemare's installer also exited with code 0. These checks do not establish gameplay, audio quality, or complete application compatibility. The other eleven additions have passed actual import verification but still need manual runtime checks. The native catalog's count, search, and Cities in Motion fallback icon were also visually checked. The developer's regular four-app library and all 133 regular-file hashes stayed unchanged.

## Current recipe expansion runtime checks

The fifteen additions all passed actual import checks with their assigned Wine versions. In a separately identified copy of the signed, sandboxed Debug app:

| Entry | Observed result |
| --- | --- |
| Drowned God | Downloaded, launched with 16-bit color, and rendered its game scene. Its private registry retained Windows 98 after Wine startup. |
| MDK Performance | Cancelling the Wine picker added nothing. Selecting Wine 11 was rejected before download; choosing Wine 10 then imported a matching private copy without changing the library default. The performance-test screen rendered at 640×480 with the CNC DDraw command. It displayed a software-rendering warning; no performance conclusion is drawn. |
| Thief | Downloaded, reached its main menu and rendered the training scene. Its private registry retained Windows XP and its launch command used CPU affinity 1 with the native default GL backend. |

All three stopped through the launcher with exit code 0. Their private Wine hashes matched the selected packages; the isolated library used format 5 and had no pending operation records. The regular four-app library and all 133 regular-file hashes remained unchanged. These are short runtime checks, not complete gameplay or compatibility tests. The other twelve additions, including their installers, still need manual runtime coverage. All 102 Swift tests and the Debug/Release bundle audits passed; each bundle contained eighteen Mach-O images.

### Drowned God filesystem-11 recheck (2026-09-11)

Using an isolated copy of the reported app's root with the current native filesystem-11 diagnostic helper and the published Wine 11.0 ZIP (`743b6d30…fd9d8e9e`), a click on the intro's final image entered the first room; another click moved forward within the room. Escape played the exit animation and the runtime exited with code 0. The developer subsequently confirmed that the intro continues automatically after roughly 20 seconds, with the same behavior in the master build's old UI. The temporary click-to-continue Help note was removed. The bundled recipe already supplies Windows 98 and `-bpp 16`; adding the same color-depth option manually is unnecessary. The isolated check did not change the regular app's root or settings and does not establish full gameplay compatibility.

## Automatic installed-demo program selection (2026-09-11)

The refreshed native Debug app was copied to a separately identified, signed, sandboxed preview with a fresh test library. The original Age of Empires payload passed its catalog size/hash check, then installed through the native launcher with Wine 11.0 filesystem 11 and the catalog's Windows XP/GDI settings. After the installer's normal exit (code 0), the library showed Ready to open without a program chooser. The saved executable was `home/username/.wine/drive_c/Program Files/Microsoft Games/Age of Empires Trial/empires.exe`; the catalog name and settings were retained. This check covers installation and automatic selection, not gameplay. The regular native library was not used for this installation.

All 23 focused demo/import/selection tests passed, including unique case-insensitive matches, missing and duplicate filenames, stopped/failed/signalled installers, preservation of a valid user choice on reinstall, manual-import fallback, and saving/reloading the selection. The Debug build and eighteen-image bundle audit passed. No catalog or persistence schema change was needed.

## Remaining entries

These entries are deliberately absent from the native catalog until their nonempty recipe options have supported native equivalents. None of those options are silently discarded. Windows-version and renderer settings can affect both installation and launch.

| Entry | Missing recipe support |
| --- | --- |
| 3D Mark 2001 SE (2002) | Run: `Debug` |
| Solitaire DotNet 2.0 | Install: `mono` |

The `Debug` token on 3DMark needs clarification: the current legacy option dispatcher has no explicit handler for it. Caesar 3’s former prose in `InstallOptions` is now an explicit 800×600 setup resolution, with the explanation retained in Help. Its GDI environment is seeded before setup, and its game-only cursor setting is used on launch.

Solitaire DotNet still requires Mono. Java Solitaire and FreeCol are no longer included.

Descent 3 and Motorhead now use the preinstalled psVoodoo DLL in the refreshed Wine 11 filesystem. [Glide support](GLIDE_SUPPORT.md) records the tested DLL, Motorhead results and the native-runtime integration and its verification limits.

## Reproduce payload verification

With copies of the exact payload files in one directory, retaining the URL basenames:

```sh
swift run --package-path project/mac-xcode/Boxedwine/BoxedwineUI \
  BoxedwinePackageCheck --demo-downloads \
  tmp/catalog-resources/catalog.xml \
  /absolute/path/to/downloads /absolute/path/to/directory-with-Wine-9-10-11-ZIPs
```

The last argument accepts a single Wine ZIP or a directory of Wine ZIPs. Each ZIP is fully validated; versions are read from the package, not inferred from its filename. Each demo uses only its exact assigned version.

This command is offline. It uses the same import implementation as the native app, writes each entry to a new temporary library, verifies persistence, then removes that temporary library. It does not read or alter the normal native library. It needs room for the largest expanded demo plus its copied download and Wine package.

The original catalog and icons remain third-party distribution materials; their presence on boxedwine.org is source provenance, not a determination of redistribution rights. The App Store questions in [Release preparation](RELEASE.md) remain open.

## Motorhead installer filename regression (2026-09-11)

The native demo importer renamed the standalone download `motordemo_3dfx.exe` to `setup.exe`. With the same Wine 11/filesystem 11 package, Windows 98 prefix, mount, resolution and graphics overrides, that name produced `LOCAL_GetBlock not enough space in local heap` and a Win16 installer crash after extraction. The failure also reproduced with the earlier isolated runtime, and removing the graphics environment overrides did not fix it. Preserving the original filename reached Setup Complete and installed `motor.exe` with SHA-256 `a3f57f88c9fc2789de288bbee6cfc799cc545d6296819e72a0689da9edeca09b`, matching the working playtest. The initial standalone diagnostic was bounded and eventually stopped by its timeout; it does not establish normal runtime shutdown.

Standalone EXE/MSI demo imports now keep the original URL filename, validated as one safe path component. Regression cases cover Motorhead, a name with spaces and an uppercase EXE extension, and MSI launch arguments. All 182 native core tests and the Debug build with its eighteen-image bundle audit passed. Reproduction logs, argument lists and the pre-repair library backup are in `tmp/native-ui-test/motorhead-install/`.

The final retry through the rebuilt native UI completed installation in the user's existing Motorhead entry after a backed-up filename repair. Setup reached its completion page, its shortcut-folder window was closed, and the helper then exited normally with code 0. The launcher selected `motor.exe` automatically and displayed Ready to open. `native-install-success.log` and `native-result.json` record this result. No additional gameplay test was performed for this importer-only fix.
