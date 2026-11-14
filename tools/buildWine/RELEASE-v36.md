# Wine 11.0 filesystem revision 36

Built November 14, 2026. Revision 36 implements DirectPlay peer capabilities and
corrects the absent-lobby-connection result used by MechCommander 2 startup.
It also includes the tested WineD3D multisample texture upload fix for Z: Steel
Soldiers intro and exit screens. Neither fix changes game executables.

| Archive | Bytes | SHA-256 |
| --- | ---: | --- |
| TinyCore15Wine11.0.zip | 217021245 | `78b0dbb1ef9ed4783de1b9c9fb19edc518933356866317dd44ad136d2d342ea5` |
| TinyCore15Wine11.0-web.zip | 96841362 | `0932559a416a11d7ed38954156ee3d4daac62cf27ba607310a43cefcc0f8e936` |

Versioned destination: `https://boxedwine.org/v2/36/`. Release files and receipts
are under `tmp/filesystem-v36-release`.

## Build provenance

Both variants use clean checkouts of the pinned Wine 11.0 commit and the full
current patch stack. Before reusing the v35 objects and installed files, the
build verified all 39 previous patch hashes, the build configuration, and
1,849 full / 1,834 web installed runtime files against the previous archives.
All tracked source files were compared: only `dpnet/peer.c`,
`dpnet/lobbiedapp.c`, and `wined3d/texture_gl.c` changed. These files were compiled
against each variant's existing configuration and relinked into isolated output
directories. No previous build or published archive was modified.

Both final archives have newly initialized prefixes and current patch/build
metadata. The eight WebGL PE DLLs, v53 graphics patch series and 16/32-bit display
mode behavior remain unchanged. The web ZIP remains below 100 MB.

## Validation

- Both archives pass CRC, PE/import, GL/cache and registry checks.
- Both pass writable Temp and URI conversion tests; the full variant also passes
  Gecko HTML, JavaScript and layout tests.
- The DirectPlay probe matches all 40 native Windows reference lines with both
  final archives, using fresh roots without patched-DLL overlays.
- The D3D8 multisample upload probe passes all 12 cases against the final full
  archive: zero failures, zero skips. Native Windows also passed all 12 cases;
  the original Wine runtime failed 16 pixel checks.

See `probes/dpnet-startup.md` and `probes/d3d8-msaa-upload.md` for scope and
reproduction. These checks do not claim DirectPlay multiplayer support.

## Publication and consumers

Published November 14, 2026. Both versioned archives, their `/v2/latest/`
aliases, sidecars and checksums were verified over fresh HTTPS downloads.
Catalogue 41 adds MechCommander 2's original installer to the Windows, macOS
and Linux native UIs. Package metadata, BoxedWineRunner, Jenkins and the demo
site defaults select revision 36.

The local and published `filesystem__36/20261114` demo snapshots retain the
existing four engines and use new filesystem/build identities. All homepage
entries now select this snapshot, with MechCommander 2 added as the 24th browser
demo. Its campaign was verified by the user in external Chrome, including
visible controls and completion of the first objective. The final overlay also
reaches the game menu without the hardware detector's machine-specific data.
Automated input in the in-app browser did not reliably progress past the intro.

The native UI's 55 tests and the demo site's 39 build/publish tests pass. The
Wine builder's 104 tests pass. Linux UI resources and the macOS distribution
resources were prepared and checked; macOS/Linux gameplay was not retested.

Publication receipts are in `tmp/mechcommander2-demo/native-publish` and
`tmp/mechcommander2-demo/publish`. The web deployment verified all 52 uploaded
files against the tested hashes and preserved previous immutable snapshots.
The Windows UI was reopened with MechCommander 2 uninstalled and selected.
