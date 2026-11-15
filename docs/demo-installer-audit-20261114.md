# Native demo source audit — November 14, 2026

Audited the 16 native UI entries added or replaced during the recent demo work.
The reference files are the user's downloads, primarily `~/Downloads/demo/`.
Each published package was downloaded, verified against catalog 43's exact size
and SHA-256, then compared against its source. System Shock 2 was corrected in
catalog 44 and verified after publication. Browser packages are intentionally
prepared separately and are not substitutes for these native installer recipes.

| Native UI demo | Original source under Downloads | Verification |
| --- | --- | --- |
| SimTower | `towerdmo.zip` | Published ZIP is byte-identical; runs `SIMTOWER/Setup.exe`. |
| Max Payne | `MaxPayneDemo.zip` | Published ZIP is byte-identical; runs the supplied portable demo. |
| SimCity 3000 | `Simcity3000Demo.zip` | Published ZIP is byte-identical; runs the supplied portable demo. |
| System Shock 2 | `SHOCK2.EXE` | **Corrected:** all 61 original setup files are preserved; runs `setup.exe`. |
| Pharaoh | `demo/PHARAOH/` | All 331 files match; runs `SETUP.EXE`. |
| Alone in the Dark | `demo/Alone/` | All 1,519 files match; runs `SETUP.EXE`. |
| Incredible Machine: Even More Contraptions | `demo/Contraptions/` | All 11 files match; runs `Setup.exe`. |
| Force Commander | `demo/ForceCom/FocomDemo.exe` | All 68 extracted files match, plus the original bundled setup engine described below. |
| Z: Steel Soldiers | `demo/SteelSoldiers/` | All 10 files match; runs `Setup.exe`. |
| World of Goo | `demo/Demos.WorldOfGoo.exe` | The installer inside the ZIP is byte-identical. |
| Settlers IV | `demo/SettlersIV/` | Both files match; runs `thesettlers4_demo.exe`. |
| Tropico | `demo/Tropico/` | All 9 files match; runs `Setup.exe`. |
| The Movies | `demo/The_Movies_Demo.exe` | All 130 extracted files match; runs the original `setup.exe`. |
| MechCommander 2 | `demo/MechCommander2/` | Both files match; runs `MC2Trial.exe`. |
| Escape from Monkey Island | `demo/MonkeyIsland/` | Both files match; runs `monkeycomplete.exe`. |
| Links 2003 | `demo/Links2003/` | The original `2k3demo.exe` matches. |

Every file in the audited directory sources and every setup file inside the
audited self-extractors matched. None of these twelve `Downloads/demo` packages
contains a substituted installed-game payload. Rogue Squadron remains skipped
and is not in the catalogue. Existing catalogue entries predating this batch,
and local experiments absent from the native catalogue, are outside this audit.

## Extracted installer details

Force Commander keeps all files from the self-extractor, plus an additional
copy of its own 32-bit InstallShield engine named `INSTALL/SETUP32.EXE`. The
original `SETUP.EXE` was run in an isolated Boxedwine root to unpack
`_INS0432._MP`. Its SHA-256 matched the published `SETUP32.EXE` exactly:
`12a2929a615924e212490f4ef3a1cbb36ecebddafea0f358991a0fc57b7cab84`.
This is the original engine, not a replacement installer or game patch.
The separately downloaded `ForceCom/Readme.rtf` is not part of that ZIP; the
original self-extractor's `INSTALL/README.TXT` is retained.

The Movies uses the original setup media extracted from its outer self-extractor.
All 130 file paths and file hashes matched the supplied EXE's extracted contents.
The catalogue launches setup; it does not launch `MoviesDemo.exe` before setup.

System Shock 2's old native entry incorrectly reused the prepared browser ZIP.
That bypassed setup and omitted the empty `current/` directory. Catalog 44 uses
the 61 original setup files extracted from `SHOCK2.EXE` (source SHA-256
`57c711da4401841fff935889404a14c970844b723a6febdd950e1d13ec9459a3`).
The normal `setup.exe` works with Wine configured as Windows 98, installs into
`C:\Sshock2 Demo`, creates `current/`, and exits successfully. The game then
renders its menu. The optional bundled Indeo codec installer previously reported
“File not found” because Wine substituted its builtin DLL when reading the
original codec's version resources. Boxedwine now defaults to
`WINEDLLOVERRIDES=ir50_32=n,b`, preserving explicit per-app environment choices.
The unmodified installer completes on released filesystem 36 with this engine
change; all six installed codec files match the originals. No Wine resource
patch or Wine `--prefer-native` build change is required.
The DLL-load probe confirms Intel Indeo 5.06 is used after installation, Wine's
builtin is used when Intel's DLL is absent, and an explicit
`-env WINEDLLOVERRIDES=ir50_32=b` still selects the builtin after installation.
As with other `WINEDLLOVERRIDES` settings, the environment takes precedence over
Wine's registry DLL overrides.

Movie playback also needs the missing GStreamer AVI parser. In an isolated
test, adding `usr/local/lib/gstreamer-1.0/libgstavi.so` from the already pinned
TinyCore `gst-plugins-good.tcz` makes both the opening logos and the main menu's
Intro movie display using native Intel Indeo. Revision 37 packages this parser
in the base, full and web ZIPs, now published under `/v2/37/` and verified through
fresh HTTPS downloads. Only those three ZIPs were uploaded for this release.
The plugin is 84,744 bytes, SHA-256
`e9fa031bfdaba66055db8d9275c5e932ef70c192d4926ebdc9e059dcb556337b`.
The isolated tests and logs are in `tmp/indeo-codec-20261114/`; `boxedwine-root`
uses the released filesystem, while `avi-root` additionally contains the parser.
The startup DLL-override regression passed, as did all 56 Windows UI/catalogue
checks. The standalone runtime built successfully; installer, codec probes,
and game shutdown returned zero. The broader real-runtime settings smoke test
applied the initial settings but exceeded its three-minute cancellation limit
while resetting them; this test has not passed in this run. Its isolated root
and log are under `tmp/indeo-runtime-tests/07377086220744a8aac2400d98d42968/`.
The browser ZIP only adds the empty directory, preserving all 37 original game
files unchanged.

## Scope of validation

This was an archive-content and launch-recipe audit, not a fresh playthrough of
every game. The shared catalogue used by Windows, macOS and Linux now pins
`26R2-catalog-44`; its other 54 recipes and all 55 icons are unchanged. Windows
catalogue tests passed (56 checks), and Linux parsed all 55 recipes. The legacy
26R2 catalogue's System Shock 2 recipe was corrected too. The browser-site tests
passed (20 checks). The proposed `hidden` manifest flag and its test were
subsequently removed at the user's request; demo discovery scans the source ZIPs.

The corrected native package, browser package, catalogue, homepage manifest,
Jenkins demo manifest and generated pages were downloaded after publication and
verified by SHA-256. All four browser runtime package URLs were checked. The
old System Shock 2 ZIP remains available unchanged for old catalogue pins.

A ZIP-mount comparison used the browser's `/home/username/files` location in
the native engine: the old package lacked `current/` and could not perform the
test write; the corrected package exposed the directory and successfully wrote
and read a temporary `CAMPAIGN.probe`. This checks the shared filesystem path,
not a new browser gameplay session.
