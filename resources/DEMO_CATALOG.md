# Shared native UI demo catalog

`demo-catalog.lock.json` pins the version, immutable HTTPS URL, exact byte count
and SHA-256 of a ZIP hosted on boxedwine.org. Commit this lock with changes to the
code that consumes it. Branches can pin different packages independently. There
is no checked-in catalog or artwork fallback and no lookup of a mutable “latest”
URL. An unchanged pin makes no network request when its verified ZIP is cached.

The package contains `catalog.xml` and its referenced PNGs in one flat directory.
The XML's `release` matches the lock's `version`; `schemaVersion` describes the
recipe format separately. The current pin, `26R2-catalog-45`, contains 56 recipes and icons,
including 3DMark2001 SE, 3DMark03 (2003), 3DMark05 (2004),
3DMark06 (2006), 3DMark Vantage (2008), SimTower Demo (1994), Max Payne Demo (2001),
SimCity 3000 Demo (1999), System Shock 2 Demo (1999), Pharaoh Demo (1999),
The Incredible Machine: Even More Contraptions Demo (2001),
Star Wars: Force Commander Demo (2000), Z: Steel Soldiers Demo (2001),
World of Goo Demo (2008), The Settlers IV Demo (2001), Tropico Demo (2001),
The Movies Demo (2006), MechCommander 2 Demo (2001),
Escape from Monkey Island Demo (2000), Links 2003 Demo (2002), and
Star Wars: Galactic Battlegrounds Trial (2001).
The 3DMark03, 3DMark05, 3DMark06, and Vantage details include their full-version
unlock keys. The new entries use application icons and select `3DMark06.exe`,
`3DMarkVantage.exe`, or `SIMTDEMO.EXE` after installation. SimTower uses the demo
archive `towerdmo.zip`, replacing the full-game archive from catalog 7 while
preserving its `sim-tower` ID. It launches `SIMTOWER/Setup.exe` with its accompanying
installer files, including the WinG libraries. Payload byte counts and SHA-256
hashes match the supplied local installers; the hosted SimTower demo also matches.
All demos now select Wine 11.0: Daytona USA and vkQuake no longer select Wine 9.0,
and vkQuake's obsolete Wine 10 mouse-button warning has been removed.
Java Solitaire and FreeCol were removed in `26R2-catalog-2`.
Catalog 9 corrects the 3DMark05 display year to 2004; its installer and recipe are unchanged.
Catalog 10 retains that correction and adds Vantage's CPU-Z/GPU-Z driver exclusions.
Catalog 11 updates Broken Sword's payload size and SHA-256 for the hosted demo ZIP
containing the quit fix: 119,336,877 bytes, SHA-256
`a3ccc4755169b089829cc0875baa1a1d976bff134e5f26240f23ee169b144d76`.
The hosted ZIP was downloaded and verified, including the patched WINSWORD.EXE.
Catalog 12 retains that payload update and explains Need for Speed: Hot Pursuit 2's
intro-movie crash and the `+nomovie` workaround. Catalog 13 moves that default into
the Need for Speed recipe's `<AppArguments>+nomovie</AppArguments>` field using
schema 9, removing the game-specific defaults from all three launchers.
CPU affinity 1 can help but does not reliably prevent the crash. Existing
installations retain their saved arguments.
Catalog 14 adds Max Payne as a portable ZIP with its original executable icon,
Wine 11.0, and a 1024x768 32-bit desktop. It launches `MaxPayneDemo.exe`; choose
**Play** in the game's settings window. Its payload is the supplied
`MaxPayneDemo.zip`, unchanged: 136,027,034 bytes, SHA-256
`1b5ac32661c3c061a93ce34874bf54569ac35e6056e9406d9ed891374d4f7461`.
The same hosted ZIP serves the Jenkins demos and local homepage prototype.
The legacy hosted and bundled UI catalogues also contain the new demo.

Catalog 14 is published at the pinned URL. A fresh download was verified against
the lock: 119,265 bytes, SHA-256
`ba2efc75e04851e5ca32d83d513035a476f2072b15fa18317a6d54d44559ccc1`.
The Windows and Linux parsers accepted all 41 recipes. Browser ST JIT testing
reached the tutorial and verified movement; native gameplay on each platform
has not been tested as part of this catalogue update.

Catalog 15 adds SimCity 3000 as a portable ZIP, preserving all 41 existing recipes.
It uses the executable's icon, Wine 11.0, GDI rendering, and a 1024x768 32-bit
desktop. The launchers select `sc3demo.exe` inside the archive's `SC3/` folder.
The supplied `Simcity3000Demo.zip` is unchanged: 123,939,643 bytes, SHA-256
`3d8f1e949a171487b4827d18c723c5ab37b7084cda81cd33f886a7b1496928be`.
The same public payload serves Jenkins, the local demo site, and boxedwine.org/test/.
The hosted and bundled legacy catalogues also include it.

Catalog 15 is 120,394 bytes, SHA-256
`700c59d2a53304029a9b7fa03e5c82bc0fda97ca02eb6a98fe9efd17ea5cd24d`.
Windows catalogue tests and the Linux parser accepted all 42 recipes. Browser
ST JIT testing reached the interactive tutorial, verified simulation and mouse
input, and observed successful application exit and Wine cleanup. Native gameplay
on each platform has not been tested as part of this catalogue update.

Catalog 16 adds System Shock 2, preserving all 42 previous recipes. It uses the
original game icon and a portable ZIP of 37 unmodified game files extracted from
the supplied `SHOCK2.EXE`: 72,624,183 bytes, SHA-256
`9239da1d6a6dde26090789fee72fd306c8b492b3dead4c98fa1f05478ae26818`. It launches `shock2.exe` with Wine 11.0 and a 640x480 32-bit
desktop. The game selects 16-bit rendering; WebGL needs filesystem 35 or later.
No GDI preset, registry overlay, or modified game configuration is required.

Catalog 16 is 122,003 bytes, SHA-256
`1b0c4e52ad5f1f81a5ba94373fcda7bd66dd3f22f25a19641a44513556ea5742`. The same payload serves the Jenkins demos and homepage.
The hosted and bundled legacy catalogues also include it.

Catalog 18 adds Pharaoh's three-mission demo with its original installer files
and application icon. `PharaohDemoSetup.zip` is 77,327,746 bytes, SHA-256
`b3daff52ea8557a29cec2464eaa20c80c9b7b32e0d8edb766129aa7a31172195`.
The recipe runs `SETUP.EXE`, selects `PharaohDemo.exe` after setup, and uses
Wine 11.0 with GDI, a visible cursor and an 800x600 desktop. The native UIs do
not receive a preinstalled game or Wine prefix. Choose to restart later when
setup finishes. The legacy catalogue includes the same installer.

The catalogue package is 122,804 bytes, SHA-256
`4d6d1835a7637a5c1b9722c9ccefcd6466b1b6072c31a8aeeab96360fc404d3e`.
All 56 Windows catalogue tests pass, and the Windows and Linux parsers accept
44 recipes. Native Linux setup and browser ST JIT gameplay were checked; the
Windows UI is prepared for a fresh user installation test. Browser packaging
and validation are described in the [Jenkins notes](../tools/jenkins/instructions.md#pharaoh-demo).

Catalog 19 selects Windows 98 for Pharaoh. The demo skips sound initialization
on NT-based Windows versions; Windows 98 restores audio in native and browser
tests. The installer and browser overlay are unchanged. The package is 122,813
bytes, SHA-256 `93e2c87a41d57199a1630b0415601d5341b2387cd02cedabcb3de2c84a1cbfcf`.
The shared [troubleshooting tips](TROUBLESHOOTING.md) record this workaround for
existing installations and other older games with no sound.

Catalog 20 adds Alone in the Dark: The New Nightmare Demo (2001) to the native
UIs only. It uses OpenGL and is deliberately excluded from the browser demos,
Jenkins demo pages and homepage. `AloneInTheDarkDemoSetup.zip` contains all 1,519
original files from the supplied `Alone` folder: 86,692,170 bytes, SHA-256
`eb1161a807f5131fda95c04a1d3f164c0ae6544fe6869c158c0d24d49239867e`.
The recipe runs `SETUP.EXE` with an 800x600 desktop, selects `launch.exe` after
installation and opens the game launcher at 640x480 with native OpenGL. Choose
**Play** to start. Decline DirectX 8, dismiss the GLSetup notice, and clear the
Readme and Play checkboxes before finishing setup. The payload is an installer,
not a preinstalled game. All 44 earlier recipes are unchanged.

The package is 123,302 bytes, SHA-256
`57a25e596c9b3da242635b8037e8d5ecc19c6bbe851a86f29c9782277aee4a83`.
The legacy catalogue and hosted installer/icon were updated too. Fresh downloads
matched their hashes. Windows catalogue tests passed (56), and the Linux parser
accepted all 45 recipes. An isolated native Linux installation retained all 1,499
game files unchanged and reached the game launcher. Its headless OpenGL test could
not create a matching GLX visual. A Windows automation run produced stereo PCM
audio, but initially drew the intro in a small corner. The native OpenGL fix now
initializes the viewport and scissor rectangle from the first guest drawable,
instead of the temporary host window. Windows testing verified the intro and
menu at 640x480. The regression test passes on native Windows and the updated
UI runtime, including preservation of application state on later context binds.
Full gameplay remains untested.

Catalog 21 corrects Alone in the Dark's blank icon by selecting the original
256-color image from `alone.ico`. All 45 recipes and other icons are unchanged.
The package is 124,633 bytes, SHA-256
`ad416e305f031fa376e29c832370c0d98119b7bc775a15b5bd7a83fbc47f4f89`.
The legacy hosted icon was corrected too, and both published files were verified.

Catalog 22 adds The Incredible Machine: Even More Contraptions Demo (2001),
preserving all 45 earlier recipes. The native UIs run the original `Setup.exe`
and select `EvenMore Demo.exe` after installation. The installer ZIP contains
the 11 supplied setup files unchanged: 19,234,536 bytes, SHA-256
`b9a1d7ce1c873252934128ded1a6aaaad8a1cd96138101bb82bf17468f30fbd5`.
The executable's icon and an 800x600, 16-bit GDI preset are included. Keep the
default installation folder; no separate DirectX installation is needed.

The catalogue package is 127,630 bytes, SHA-256
`737df06ede73231bd40d2deb2f3105d450159b0a9a15eb0340bf4330399ced50`.
Fresh downloads matched the pin, and all 56 Windows catalogue tests passed.
An isolated native Linux setup completed; the current Windows engine displayed
the intro and generated stereo PCM audio. Browser ST JIT testing reached the
first puzzle, placed a part and ran the simulation with active audio. Mac
gameplay has not been tested. The browser uses a separate portable package of
seven pristine installed files; see the [Jenkins notes](../tools/jenkins/instructions.md#even-more-contraptions-demo).

Catalog 23 updates the About text for Castle of the Winds, Alone in the Dark:
The New Nightmare and Even More Contraptions. Castle's obsolete Wine 7 drawing
note is removed. Alone and Contraptions have game descriptions and original PC
requirements in place of installation walkthroughs. Their short summaries also
describe the games. All 46 recipes retain their payloads, icons and launch settings.
The package is 127,747 bytes, SHA-256
`fb316de1cf5c5d84bdffac0978c105bf3f53e9471b1bb1c49852838040d42852`.
The same descriptions are used in the legacy catalogue.

Requirements come from the original demo readmes: Alone's May 23, 2001 notes
and Contraptions' April 10, 2001 notes, both included in their hosted installer
ZIPs. Alone's game description is also supported by the
[publisher's game page](https://store.steampowered.com/app/259190/Alone_in_the_Dark_The_New_Nightmare/).
The old readme, rather than the later Steam release, supplies its period requirements.

Catalog 24 removes the setup, resolution, renderer and cursor configuration
sentence from Caesar III's About text. The original system requirements remain.
All 46 recipes retain their launch settings, payloads and icons. The package is
127,696 bytes, SHA-256
`e97c745fdc7a7ee6839ccc7ac5e40f552c17b1bb2a67bcbf8b33eb7612a41b5c`.

Catalog 25 fixes the garbled apostrophe in Descent 3's About text, using a plain
ASCII apostrophe. The compatibility advice and all launch settings are unchanged.
The package is 127,684 bytes, SHA-256
`d619b97158e5a842d539c0dc87076368d999d81638d920899d5ea6ada0e667cc`.

Catalog 26 removes 3dfx from Diablo II's listed rendering options, leaving
DirectDraw and Direct3D. The legacy description matches. Launch settings,
payloads and icons are unchanged. The package is 127,683 bytes, SHA-256
`a679c9687561dde7e497ee0dd77d747a45550e08ac732c731657ed25568d6d14`.

Catalog 27 replaces Half-Life: Uplink's setup and implementation comments with
a short history of the standalone mini-campaign and a "Graphics: OpenGL" note.
Its summary now describes the game, and the legacy About text matches.
The history is sourced from [Valve's 25th-anniversary page](https://www.half-life.com/en/halflife/25th).
Launch settings, payloads and icons are unchanged. The package is 127,640 bytes,
SHA-256 `7f39f83f9b973d14d6dcb31722824b2448b08e9bcbec543851bd1af3adc6d580`.

Catalog 28 removes MechWarrior 3's obsolete Wine 9/10 compatibility paragraph
from both catalogues, following confirmation that the game works with Wine 11.
Its original requirements and Wine 11.0 selection remain. The package is
127,601 bytes, SHA-256
`bcd341e6f687c0d53fee0ce61679ab9a7bb91fca191d472a0c5abb273bc9cf13`.

Catalog 29 replaces Pharaoh's installation walkthrough with a game description,
details of its three demo missions and original requirements from the supplied
1999 demo readme. A brief Windows 98 sound compatibility note remains. Its
summary no longer describes setup, and the legacy About text matches.
All launch settings, payloads and icons remain unchanged. The package is
127,656 bytes, SHA-256
`0e3391694bb2f063e93615c71501d53a39a91d44bf0c5a1c90d98b4fa666bf66`.

Catalog 30 replaces SimTower's setup notes with its skyscraper-management
gameplay, elevator planning and guided, time-limited demo. Its summary describes
the game, and the legacy About text matches. The developer, publisher and year
are documented in the [Centre for Computing History collection](https://www.computinghistory.org.uk/det/69279/SimTower/).
Launch settings, payloads and icons remain unchanged. The package is 127,787 bytes,
SHA-256 `b291b7c1e4d8e6cf3a66ea2efcec5f4666dcba8b3e1b0a213a4cbbd99deea09d`.

Catalog 31 replaces System Shock 2's launch and WebGL implementation notes with
game background and demo contents. Its Dark Engine connection to Thief is
documented in [Irrational's retrospective](https://irrationalgames.ghoststorygames.com/insider/what-might-have-been/).
The original demo's `README.WRI` describes the training area, Med/Sci deck and
pre-generated Navy character. The legacy About text matches; the summary,
launch settings, payloads and icons remain unchanged. The package is 127,940 bytes,
SHA-256 `ef0397ccd78813bb1bcb78ea78b657d2424e9dc2ee340c517859ee9780b5320c`.

Catalog 32 fills Cities in Motion's empty About field with game background and
original Windows requirements, and updates its summary. Sources are the
[publisher's 2011 announcement](https://www.prnewswire.com/news-releases/transport-simulator-cities-in-motion-released-today-116650389.html)
and [official Steam listing](https://store.steampowered.com/app/73010/Cities_in_Motion/).
Max Payne now has a game overview based on [Remedy's description](https://www.remedygames.com/games/max-payne)
and [Rockstar's game page](https://www.rockstargames.com/games/maxpayne); its
"Choose Play" instructions are removed from About and Summary. Tomb Raider 3's
"Works with Wine 10" comment is removed, retaining its original requirements.
All three legacy About entries match. Launch settings, payloads and icons remain
unchanged. The package is 128,335 bytes, SHA-256
`4fa0073ceb8da1cc77e42c566bcdc8a8ce42049e9510363fb9190341be02951c`.

Catalog 33 adds Max Payne's original minimum and recommended requirements from
`help/html/intro.htm` inside the supplied demo ZIP. Its game description and
Direct3D 8 note remain, and the legacy About text matches. No launch settings,
payloads or icons changed. The package is 128,444 bytes, SHA-256
`b80ff74f1926613534e97b28a249e12397eb6a84bc1b9dbefd9eb3dac6afc20f`.

Catalog 34 adds historical requirements to Motorhead, SimCity 3000 and System
Shock 2. Motorhead's `demo1.txt`, extracted from the verified `motordemo_3dfx.exe`,
lists the actual 3Dfx demo's Pentium 133, 24 MB RAM and accelerator requirements.
These differ from published retail requirements and should not be interchanged.
SimCity 3000 uses the original full-game requirements recorded with the
[Maxis retail edition](https://openlibrary.org/works/OL16997921W/SimCity_3000?v=3).
System Shock 2 uses the [1999 packaging record](https://elisoftware.org/wiki/System_Shock_2_%28PC%2C_CD-ROM%29_Electronic_Arts_-_1999_USA%2C_Canada_Release),
with its 4 MB graphics and DirectX 6 requirements confirmed in the original demo's
`README.WRI`. Both retail lists are explicitly labelled as full-game requirements.
All three legacy About entries match. No other recipe fields or artwork changed;
Max Payne's requirements from catalog 33 are retained. The package is 128,648 bytes,
SHA-256 `46eb072f0095b84d7812b963416aa875bb09683f511fe24d9874d35a21c18db0`.

Catalog 35 adds Star Wars: Force Commander Demo (2000), preserving all 46 earlier
recipes and icons. The About text describes the game and lists requirements from
the original March 2000 demo readme. The recipe uses Wine 11.0, an 800x600 32-bit
desktop and the original game icon. `ForceCommanderDemoInstaller.zip` contains
the original 68 installer files plus its bundled 32-bit InstallShield engine,
unchanged, named `INSTALL/SETUP32.EXE`. The original 16-bit setup launcher stalled
at 100% during testing; launching the bundled 32-bit engine completed setup.
The native UIs run that installer and select `focom.exe` afterward. The payload
contains no preinstalled game or Wine prefix. It is 81,610,386 bytes, SHA-256
`21dd032e4e6663dacfa0f5d55d747271aaa82496ba94fecf29400280d296c803`.

The catalogue package is 129,591 bytes, SHA-256
`08241f80dfc5bde366adb8fd7055afcb67aa2b1309bcc1a231356acaa0222a34`.
All 56 Windows catalogue tests pass, and the Windows and Linux parsers accept
47 recipes. An isolated Windows installation completed and reached the 3D
briefing with nonzero stereo PCM audio. Browser ST JIT testing reached the
Assault on Ruul battlefield, moved the camera and selected units. A full mission
and native Mac/Linux gameplay have not been tested. Browser packaging is
documented in the [Jenkins notes](../tools/jenkins/instructions.md#force-commander-demo).

Catalog 36 adds Z: Steel Soldiers Demo (2001), preserving all 47 existing recipes
and icons. The original setup files are packaged as `SteelSoldiersDemoSetup.zip`
(58,434,428 bytes, SHA-256
`4a0668a43002cbf3c6fd559fdbca1d0e1b4c418db44f287206ab41a6dffa6de6`).
The recipe launches `Setup.exe`, selects `demo.exe` after installation, and uses
Wine 11.0 with an 800x600, 32-bit desktop. Native UIs receive the original
installer, without preinstalled game data. Its About text includes the original
demo requirements and notes the unavailable separate graphics setup utility.

The published package is 130,881 bytes, SHA-256
`b3749c7c3381ea9939ae7c10e7588ff42660c6d203015e12a9d145545f142824`.
A fresh public download was verified and staged. All 56 Windows tests pass;
the Windows and Linux parsers accept 48 recipes. Native Windows setup and
mission rendering were tested, with nonzero stereo PCM audio. Browser ST JIT
testing verified mission loading, selection, movement and camera controls.
Native Mac/Linux gameplay and a complete mission were not tested. Browser
packaging is documented in the
[Jenkins notes](../tools/jenkins/instructions.md#steel-soldiers-demo).

Catalog 37 adds World of Goo Demo (2008), preserving all 48 existing recipes and
icons. Its original `Demos.WorldOfGoo.exe` installer is packaged unchanged in
`WorldOfGooDemoSetup.zip` (32,801,046 bytes, SHA-256
`e65bae7079fc7c53abca9e9217b9fad29a137acd6195492b0509b86b270fe2e0`).
The native UIs launch that installer and select `WorldOfGoo.exe` afterward, with
Wine 11.0 and an 800x600, 32-bit desktop. About describes 2D Boy's puzzle game and
lists the original demo requirements from its bundled `readme.html`.

The catalogue package is 132,227 bytes, SHA-256
`5356459ed9be58b821c9d4b38d1c2311d74734d37d34463d9b40b5427e541ed8`.
The published package was downloaded into a fresh cache and audited successfully.
The Windows and Linux parsers accept all 49 recipes; all 56 Windows tests pass.
Native Windows setup and interactive puzzle rendering were tested, with nonzero
stereo PCM audio. Browser ST JIT testing reached the first puzzle and connected
goo balls. A complete playthrough and native Mac/Linux gameplay were not tested.
Browser packaging is documented in the
[Jenkins notes](../tools/jenkins/instructions.md#world-of-goo-demo).

Catalog 40 adds The Movies Demo (2006), preserving all 51 existing recipes and
icons. `TheMoviesDemoSetup.zip` contains the original setup media extracted from
the supplied self-extracting demo. Native UIs run `setup.exe`, then select
`MoviesDemo.exe`; this is an installer package, not a preinstalled game.
The original outer EXE fails before setup with `c0000018`, while the extracted
installer completes. The recipe uses Wine 11.0 and an 800x600, 32-bit starting
desktop, with the original icon and the demo readme's system requirements.
The user confirmed native gameplay. The installer ZIP is 299,295,796 bytes,
SHA-256 `b2a49aa41711433b97e668d22e3c84e8e7ce38aa139b2893622253138f0dfe9f`.
The browser demo uses a separate installed overlay; Chrome reached the studio
tutorial and the user confirmed mouse capture works. See the
[The Movies packaging notes](../tools/jenkins/instructions.md#the-movies-demo).

Catalog 39 adds Tropico Demo (2001), preserving all 50 existing recipes and icons.
`TropicoDemoSetup.zip` contains the nine original setup files unchanged; native
UIs run `Setup.exe` and select `TropicoD.EXE` after installation. The recipe uses
Wine 11.0 and an 800x600, 32-bit starting desktop with no game-specific cursor
override. About includes the demo's 50-year scenario and original requirements.
The package is 134,854 bytes, SHA-256
`905b130ed79a7861bfab7260a5a8bfcb53399c27a3deee9404121bc3210734e9`.
All 56 Windows catalogue tests pass and both parsers accept 51 recipes.
See the [Tropico packaging notes](../tools/jenkins/instructions.md#tropico-demo)
for the distinct installed browser overlay and cursor fix.

Catalog 38 adds The Settlers IV Demo (2001), preserving all 49 existing recipes
and icons. `SettlersIVDemoSetup.zip` contains the supplied original installer and
readme unchanged (99,893,156 bytes, SHA-256
`753dd49c6fd643c33adc8844401f96f2c2922faae5e4ad70335005dccd268296`).
The native UIs run `thesettlers4_demo.exe` and select `S4.exe` afterward, using
Wine 11.0 and an 800x600, 32-bit desktop. Its About text describes settlement
building, the two tutorials and single-player map, and the original requirements
from the supplied `readme.txt`. The original executable icon is included.

The catalogue package is 134,020 bytes, SHA-256
`e5fa099d08884191fb2ececd08ee5becb23d0e4eb5406814904005986b404d13`.
Native Windows setup and the first tutorial were tested with nonzero stereo PCM
audio. Browser ST JIT testing reached the same tutorial, advanced instructions
with the keyboard, and exercised camera movement and Home to return to the
settlement. A complete mission, multiplayer and native Mac/Linux gameplay have
not been tested. Browser packaging is documented in the
[Jenkins notes](../tools/jenkins/instructions.md#the-settlers-iv-demo).

All 56 Windows catalogue tests pass, and both Windows and Linux parsers accept
the 50 recipes. The Windows UI rebuild completed without warnings. A fresh
download of catalog 38 passed its pinned hash and audit; all 56 newly published
files matched the tested local hashes. The shared catalogue, legacy catalogue,
Jenkins manifest, local demo site and homepage now include the demo.

The native launchers initialize newly installed Alice demos (ID `alice`) with
200% mouse sensitivity. This is saved as an editable per-app Boxedwine argument;
existing installations retain their saved settings.

Schema 8 adds optional `WineDllOverrides`, a nonempty single-line value of at most
1024 characters. All three native launchers seed it into the new app's editable
Boxedwine arguments as `-env` followed by `WINEDLLOVERRIDES=<value>`. The value is
passed literally, including a trailing `=` used to disable a DLL or driver. The
existing app metadata and backup formats already preserve these arguments.
Catalog updates do not change previously installed apps or their custom settings.

Schema 9 adds optional `AppArguments`, using the same format and validation as
[custom collections](../docs/App-Imports-and-Collections.md): one literal Windows
application argument per line, with no shell parsing or quote removal. Spaces,
backslashes (including literal `\n`), and empty lines are preserved. Use actual LF
or CRLF line breaks in XML, or `&#10;`; XML entities are decoded normally. Avoid
indentation or surrounding newlines inside this field unless they belong in the
arguments. An omitted or empty field supplies no arguments. There may be at most
256 arguments, 8192 UTF-8 bytes per argument, and 65536 bytes total; NUL and bare
carriage returns are rejected after XML's standard newline normalization.
These defaults seed editable app settings on first installation and are passed
after the Windows executable, never to its installer or the emulator. Older
schemas remain supported, but `AppArguments` requires schema 9.

```xml
<AppArguments>+nomovie</AppArguments>
```

Vantage uses `<WineDllOverrides>cpuz136_x32.sys,cpuz136_x64.sys,GPUZ.sys=</WineDllOverrides>`
to avoid its GPU-Z hardware-scan exception. This only disables those probes in
Vantage's Wine environment; it does not replace any Wine module. Existing Vantage
installations can use the same `-env` argument in their app settings.

The native Mac, Windows, and Linux UIs consume this shared pin and recipe format.
Each UI sorts demos alphabetically by name, independently of their XML order,
using the same ordering as its installed-app list.
`project/shared/demo_catalog.py` uses Python 3's standard library to package and validate
the catalog. The old OpenGL UI still uses its existing XML, and the Emscripten
demo site has a separate catalog.

Windows builds always copy the extracted catalog into their output and publish
directories. Catalog ZIPs use fixed timestamps, so copying only newer files can
leave an older catalog embedded after the pin changes.

## Optional disc and legacy Java support

Schema 10 enables `InstallType=Disc` with `CDROMImages` for pinned catalogues,
using the existing collection-disc installation, backup and mount paths.
The ZIP must contain the listed ISO paths; `InstallExe` is omitted so the
original disc's autorun chooses the installer. Existing collection recipes
continue to work without changing their schema. Download trust and checksum
requirements are unchanged.

Schema 10 also supports the installation-only boolean `LegacyJavaFonts`.
Java 1.3 opens Windows font filenames directly, bypassing Wine's substitutions.
This option copies the Wine package's Liberation Sans, Mono and Serif families
to Arial, Courier New and Times New Roman filenames in the new app's Windows
Fonts directory, together with Wine's Symbol and Wingdings fonts. The font
bytes are unchanged and no host OS fonts are used. These capabilities do not
change the current catalogue or add a demo.

## Building and caching

Xcode's `BoxedwineUI` bundle phase runs the tool automatically. It verifies the
cached ZIP on every use, or downloads the exact URL with a 20-second total timeout.
Only verified downloads enter the cache. The native recipe validator then checks
the XML and icons before Xcode signs the app. Downloaded game payloads and Wine
filesystems are separate from this small catalog ZIP.

| Situation | Local Xcode build | Jenkins / `buildRelease.sh` |
| --- | --- | --- |
| Exact verified ZIP cached | Use it, without network | Use it, without network |
| Missing cache, download succeeds | Verify and embed | Verify and embed |
| Missing/corrupt cache, download fails | Warn; build without demos | Fail before archiving |
| Wrong version cached | Fetch pinned version, or omit demos | Fetch pinned version, or fail |
| Downloaded catalog has unsupported recipes | Fail native validation | Fail native validation |

When a local build omits demos, its Demos screen says they aren't included.
Installed apps and manual installation remain available. Rebuild with a network
connection to include the catalog. The output directory is replaced on each build,
so a previous branch's list cannot survive an offline version change. There is no
runtime catalog/artwork update: the signed app always uses its embedded list.

Default cache directories:

- macOS: `~/Library/Caches/BoxedwineBuild/demo-catalogs`
- Linux: `$XDG_CACHE_HOME/BoxedwineBuild/demo-catalogs` (default `~/.cache`)
- Windows: `%LOCALAPPDATA%/BoxedwineBuild/demo-catalogs`

ZIPs are named by SHA-256. Concurrent builds download into separate temporary
files and publish atomically. Set `BOXEDWINE_CATALOG_CACHE` to share/persist a cache
on a build worker. Jenkins may use its cache offline, but must have the exact pin.
Deleting the cache is safe; the next build downloads it again.

Xcode build settings `BOXEDWINE_REQUIRE_DEMO_CATALOG=YES` and
`BOXEDWINE_CATALOG_OFFLINE=YES` require the package and disable network respectively.
`buildRelease.sh` always requires it, independently of the environment. Direct CLI
equivalents are `--required` and `--offline`. `stage --output` **replaces that entire
directory**; use a dedicated build-resource directory, never your editing source.

```sh
python3 project/shared/demo_catalog.py stage --required --output tmp/catalog-resources
python3 project/shared/demo_catalog.py audit --output tmp/catalog-resources
```

Staging adds `catalog-package.json` to the output, recording the pin and file
hashes. It is a generated build receipt, not part of the uploaded ZIP. The Mac
bundle audit checks this receipt and every asset; release/distribution audits also
require that the catalog is present. ZIP validation rejects links, paths,
duplicates, unexpected files, malformed XML and missing icons, with size limits.

## Writing About descriptions

`Help` is shown as "About this app" in the native UI. Use it for game background,
what the demo includes, and clearly labelled original system requirements.
Add compatibility notes only for known problems that affect using the app in
the native UI. Keep setup transcripts, developer/testing instructions and
browser availability out of this text; record implementation and test details
in developer documentation. `Summary` should briefly describe the game or app.

## Publishing a catalog update

1. Resolve the current package using `stage`, then copy the resulting files into
   an editing directory outside Git (for example `tmp/catalog-edit`). The packer
   ignores the generated receipt. Keep a backed-up authoring copy or retrieve the
   immutable published ZIP to edit it later.
2. Edit `catalog.xml` and its PNGs. Preserve demo IDs for the same app across
   releases. Give the XML a new `release`, for example `26R2-catalog-8`. Retain the
   schema version unless the recipe format changes. Follow the native schema's
   payload verification and compatibility testing procedure.
3. Validate recipes with `BoxedwinePackageCheck --catalog`, then pack the files
   and update the project pin:

   ```sh
   swift run --package-path project/mac-xcode/Boxedwine/BoxedwineUI \
     BoxedwinePackageCheck --catalog tmp/catalog-edit/catalog.xml
   python3 project/shared/demo_catalog.py pack --source tmp/catalog-edit \
     --version 26R2-catalog-8 \
     --url https://boxedwine.org/catalogs/26R2-catalog-8/catalog.zip \
     --output tmp/catalog-8.zip --lock resources/demo-catalog.lock.json
   ```

4. Upload the ZIP to the exact URL printed by the packer. Never overwrite an
   existing release URL. For testing before upload, `import --zip tmp/catalog-8.zip`
   verifies the ZIP against the project lock and seeds only your build cache.
5. Verify an actual download using a fresh cache, test the catalog, and build:

   ```sh
   python3 project/shared/demo_catalog.py stage --required --cache tmp/fresh-catalog-cache \
     --output tmp/catalog-resources
   BOXEDWINE_TEST_DEMO_CATALOG="$PWD/tmp/catalog-resources" \
     swift test --package-path project/mac-xcode/Boxedwine/BoxedwineUI
   sh project/mac-xcode/buildNative.sh
   ```

6. Commit the updated lock and any schema/code changes. Keep all released ZIPs
   available so older branches and tags can rebuild. Upload before merging the
   pin so a clean Jenkins worker can build immediately.

## Tests

`python3 tools/testing/test_demo_catalog.py` runs without network or release assets. The
Swift unit suite uses synthetic recipe fixtures and works on a fresh offline
checkout. Setting `BOXEDWINE_TEST_DEMO_CATALOG` additionally checks the staged
release's recipes against the Wine catalog and decodes every icon. Supplying a
bad path fails those checks rather than silently skipping them.


Catalog 17 retains all 43 recipes. It updates the existing `sim-tower` entry's
name and instructions, and sets setup and the game to 800x600. Native UIs keep
using the verified `towerdmo.zip` installer, which installs WinG and sound DLLs;
the browser demo uses a separate installed overlay. The legacy `filesV2.xml`
catalogue now includes the same installer and `SIMTDEMO.EXE` shortcut.


Catalog 42 adds Escape from Monkey Island Demo (2000), preserving all 53 previous recipes and icons. The installer ZIP contains the supplied `monkeycomplete.exe` and `README.TXT` unchanged (122,177,820 bytes; SHA-256 `3872ac1276e0649e680af6c072bd8cef40f05b4398274ed6d330826753d3cacf`). Native UIs run the original installer and select `Monkey4.exe` with `-cd`, the same argument supplied by the original launcher. The legacy UI selects `MnkyDemo.exe`. About includes the original requirements and the demo's keyboard/gamepad controls and disabled saves.

The package is 138,217 bytes, SHA-256 `14d1346ca3c3f46c84aaebad79031844635d26328f419d5f65e245830ff5288e`, with 54 recipes and icons. Windows setup and the first scene were tested on Wine 11.0 filesystem 36. All 55 Windows UI tests passed. Browser ST JIT testing exercised movement, conversation and dialogue selection with nonzero stereo audio. Mac/Linux gameplay and a full demo playthrough have not been tested. See the [packaging notes](../tools/jenkins/instructions.md#escape-from-monkey-island-demo).


Catalog 43 adds Links 2003 Demo (2002), preserving all 54 previous recipes and
icons. The installer ZIP contains only the original `2k3demo.exe`, unchanged:
170,237,642 bytes, SHA-256
`0f31ea2640fa77b0cd225cbb69a55721b8f0be4bd6537f77682f578a2150003f`.
Native UIs run this installer and select `LinksMMIII.exe` when it exits
successfully. The recipe uses Wine 11.0, 800x600 at 32-bit, and
`WineDllOverrides=wmvcore=`. The legacy UI catalogue contains the same recipe.
About describes the demo's Skeleton Coast course, Sergio Garcia, Real Time
Swing, and the original full-game requirements.

The catalogue is 139,670 bytes, SHA-256
`3001da626c663732166c90d1bd103b4bff95d87cdb411b52a45c2262f3470993`.
The Windows UI tests pass (55), and Windows and Linux parse all 55 recipes.
The user confirmed native new-game play and ball-hit sounds, then created a
fresh browser player and started the swing tutorial with spoken instructions.
Filesystem 36 and game executables are unchanged. Mac/Linux gameplay, online
play and a full nine-hole round have not been tested. See the
[packaging notes](../tools/jenkins/instructions.md#links-2003-demo).

Additional catalogue-43 runtime verification: the regular 55-test UI suite passed.
The extended suite with filesystem 36 and the actual bundled runtime passed 57
checks, but resetting Wine settings exceeded the test's shared three-minute
deadline on a fresh rerun. An initial Mesa file-access failure did not recur.
The runtime matches the successful Links 2003 gameplay test byte for byte;
the settings-reset timeout remains unresolved.

Catalog 44 corrects System Shock 2's native recipe to run the original installer.
`SystemShock2DemoSetup.zip` contains all 61 unchanged files extracted from the
supplied `SHOCK2.EXE`, including `setup.exe`, its installation data and bundled
codec installer. It is 74,644,764 bytes, SHA-256
`dd1d456089919ead60b371533f1b4ee3bfcfbdcafbffcd36d55985c9380811eb`.
The recipe selects Windows 98 (setup rejects NT), starts setup at 800x600, and
selects `shock2.exe` afterward at 640x480, 32-bit. No installed Wine prefix or
modified game file is supplied to the native UIs.

The previous portable package omitted the `current/` directory created by setup.
The game crashed writing `current/CAMPAIGN` at the training-to-Med/Sci transition.
Restoring that empty directory fixed the user's repeatable crash. Browser demos
now use the separate corrected `SystemShock2Demo-r2.zip`; the original download
remains available for older catalogue pins.

Catalog 44 is 139,689 bytes, SHA-256
`f2a03c37c0dbd94c14acda8cdffb87db9ae24b8cf21305f74a1934d47ca92077`.
The other 54 recipes and all icons are unchanged. Windows tests passed (56), and
the Linux parser accepted all 55 recipes. The original setup completed with exit
code 0 and its installed game rendered the menu. Its optional Indeo codec setup
initially reported “File not found”. The subsequent Boxedwine startup change
prefers the installed Indeo codec with Wine's builtin as fallback, allowing
setup to complete. Filesystem 37 adds the AVI parser needed for movie playback;
the installed codec and parser were verified with the opening logos and Intro
movie in an isolated installation. Native
Mac/Linux installation and a new browser playthrough were not retested here.
Published files were downloaded and SHA-256 verified. See the
[native demo source audit](../docs/demo-installer-audit-20261114.md).

Catalog 45 adds Star Wars: Galactic Battlegrounds Trial (2001). The native UIs
run the original `SWGB_Trial.exe` unchanged (73,387,227 bytes, SHA-256
`c46e9a45f2d0c7bf75e8b3fca4fae9e54765af9cf110d7613071badc93d4d3d2`)
and select `battlegrounds_trial.exe` after installation. Its recipe uses Wine
11.0, GDI rendering and a 1024x768, 32-bit desktop. GDI correctly draws the
Windows controls mixed with DirectDraw, including the New Player dialog.
The About text and original requirements come from the trial's readme; its
icon comes from the original launcher. No installed prefix is supplied to the UIs.

The package is 141,117 bytes, SHA-256
`7df4edc73db148738bf2b313c6fd615bf640834b3a2e83c1b7ef2f3540bb827b`.
All 55 earlier recipes and icons are unchanged. The shared catalogue tests
passed (15), and the Windows UI tests passed (57). The original installer
completed in an isolated Windows-hosted Boxedwine root, and the user confirmed
gameplay with GDI. Native macOS/Linux gameplay and multiplayer are untested.
The browser uses a separate installed overlay described in the
[Jenkins notes](../tools/jenkins/instructions.md#galactic-battlegrounds-trial).
The installer, icon, immutable catalogue ZIP and legacy catalogue were published
and verified by SHA-256; their public HTTPS URLs were checked as well.
