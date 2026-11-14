# Wine 11.0 filesystem revision 35

Built November 13, 2026. Revision 35 exposes 16-bit RGB565 display modes alongside
32-bit modes in the WebGL WineD3D renderer. The desktop remains 32-bit. This
allows the System Shock 2 demo to detect a supported mode and enter gameplay.

Boxedwine's XRandR emulation still supplies the resolutions. Wine's X11 driver
generates their color-depth variants; the WebGL WineD3D filter now accepts 16 and
32 bits. Paletted modes remain excluded from that renderer. GDI, the regular
Wine runtime, the seven other WebGL DLLs and the v33 base are unchanged.

## Artifacts

| Archive | Bytes | Immutable URL |
| --- | ---: | --- |
| Full | 217,015,557 | https://boxedwine.org/v2/35/TinyCore15Wine11.0.zip |
| Web | 96,835,517 | https://boxedwine.org/v2/35/TinyCore15Wine11.0-web.zip |

```text
cdd2ec5f2415b71d85910311c6f31d5e4880055fca504872ea18aa22236a06ae  TinyCore15Wine11.0.zip
db817c64a68e5be86743151dc0d2ca849e16bb6c28e916a7c52a308426dedf80  TinyCore15Wine11.0-web.zip
```

Local release artifacts and validation records are under
`C:\Boxedwine2\tmp\filesystem-v35-release`; game, catalogue and publication
records are under `C:\Boxedwine2\tmp\shock2-demo`.
Published November 13, 2026, preserving previous versioned downloads and retained
demo builds. Latest aliases use identical ZIP bytes, and the hosted legacy
catalogue remains `/v2/26R2/filesV2.xml`. Deployment used guarded input hashes,
atomic file replacement, a shared publication lock and backups of mutable files.
Fresh HTTP downloads verified both versioned archives, both latest aliases, the
mutable and retained demo roots, the game payload and catalogue against their
exact sizes and SHA-256 hashes. Demo downloads retain CORS access. All four
published launch paths resolve to the game payload. The hosted XML matches the
prepared catalogue. The local and public homepage cards, mobile layout, catalogue
search and public player launch pass without browser or HTTP errors; the player
opens at 640x480 with the console hidden and no top banner.

## Inputs and assembly

[`filesystem_wine11.json`](filesystem_wine11.json) selects revision 35 and
[`webgl_filesystems_v35.json`](webgl_filesystems_v35.json). The
[v53 manifest](../wineTests/webgl-test-divergences-v53.json) appends one production
patch to v52; all 77 patches apply to a clean checkout at Wine commit
`db11d0fe6a169c457e23d007e20404643d067aa8`. The existing Wine test adaptation is
unchanged. Historical source manifests and patch hashes are retained.

WineD3D was rebuilt incrementally from the verified v52 object tree, compiling
the final patched `directx.c` and relinking in an isolated directory. The final
DLL is 3,995,520 bytes, SHA-256
`34f2ebb53e580446be2f8123fedd8aa4defef5bb98086b5d3172a1a5f3c3488e`.
Its source comment clarifies the XRandR/Wine depth relationship. The release
archives and final DLL were retested after rebuilding; they do not reuse the
temporary experiment ZIP's revision-34 metadata.

Dedicated WSL `boxedwine-wine11-fs35-{main,web}` directories reuse the unchanged
regular Wine build after verifying its configuration, all 39 native patch
hashes, and 1,849/1,834 installed runtime files against revision 34. Only WineD3D
changes in the addon inventory. Both archives were assembled with fresh Wine
prefixes and the new patch documentation. The web ZIP remains below 100 MB and
passes the existing pruning and lossless archive optimization checks.

## Consumers and demos

- Shared Windows/macOS/Linux package metadata selects the full v35 URL, exact
  size/hash and filesystem revision. The legacy bundled and hosted XML also
  select revision 35. Mac App Store resources were prepared from the exact full
  archive. Existing installations retain their saved package pins.
- Jenkins AbiWord uses the final web URL/hash; both demo scripts and the demo
  generator select the v35 manifest. Native automation selects the full URL,
  size and hash through `filesystem.properties`. Its 256 MiB/GDI presets are
  unchanged. The automation and AbiWord application bundles are unchanged.
- Catalogue `26R2-catalog-16` contains 43 recipes, preserving all 42 previous
  recipes and adding System Shock 2 with its original icon. Its portable ZIP
  contains 37 unmodified files from the supplied `SHOCK2.EXE` demo's `shock/`
  directory. Launch `shock2.exe` with Wine 11.0, 640x480 and a 32-bit desktop.
  No GDI preset, registry overlay or game configuration edit is needed.
- `SystemShock2Demo.zip`: 72,624,183 bytes, SHA-256
  `9239da1d6a6dde26090789fee72fd306c8b492b3dead4c98fa1f05478ae26818`.
  Catalogue 16: 122,003 bytes, SHA-256
  `1b0c4e52ad5f1f81a5ba94373fcda7bd66dd3f22f25a19641a44513556ea5742`.
- Local and public demos use a new `filesystem__35/20261113` snapshot containing
  the existing four browser runtimes from `james/web_gdi/5`, paired with the v35
  web ZIP and newly computed graphics identities. Earlier build directories
  retain their original ZIPs. The public snapshot has 14 demos; the local site
  retains its additional Halo demo. The homepage at `/test/` uses ST JIT,
  features System Shock 2 and includes it in the searchable catalogue. AbiWord
  remains in the catalogue. The homepage root points to the retained snapshot.

## Validation

- Both archives pass CRC, PE/import, GL/cache and registry validation. Both pass
  writable Temp-directory and all eight URI checks; the full variant also
  passes Gecko HTML, JavaScript and layout checks.
- The expanded display-mode probe passes 1,228 checks at a 1280x720 starting
  resolution against each final archive, and 1,012 checks at 800x600 against the
  web archive. This tests both color depths, repeated alternating mode switches,
  backbuffer dimensions/formats, exact RGB565/32-bit pixels, enumeration and
  restoration of the original 32-bit desktop. All runs exit zero and complete
  Wine cleanup without harness errors. These first runs used build 3 ST JIT;
  the final web archive also passes the 1,228 checks with published build 5.
- System Shock 2 reaches the main menu and training, accepts forward movement
  and turning, returns to the menu and exits through its closing screens.
  Application exit and Wine cleanup are zero, followed by a 15-second stability
  observation. Both build 3 and the final selected build 5 pass with fresh
  in-memory roots and the exact release web ZIP.
- 104 builder, 31 web-tooling, 65 Jenkins-tooling and 56 Windows UI tests pass.
  The Linux parser accepts all 43 recipes and the 32-bit System Shock 2 settings.
  Java runner regressions pass with a 32 MiB heap, retaining registry presets.
- AbiWord matches all four reference captures and returns success exit 111 in
  ST, ST JIT, MT and MT JIT using the final web ZIP. Both single-threaded modes
  complete without browser errors. The threaded modes then report `ErrorEvent`
  during shutdown, consistent with the issue previously recorded against
  revision 34; they are playback passes, not clean-shutdown passes. Runtime
  hashes and per-mode evidence are recorded in `abiword-results.json`.

This is focused release qualification, not a full game playthrough or the full
Wine graphics matrix. System Shock 2 logs missing AVI decoder/GStreamer warnings;
intro/cutscene playback and audio quality are not qualified. Native functional
and Cinebench worker suites, ARM execution and signed Mac builds were not run.
