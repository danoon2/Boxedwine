# Wine 11.0 filesystem revision 34

Built November 12, 2026. Staged locally; not uploaded. Revision 34 updates the
separate WebGL WineD3D to enumerate the emulated display's 32-bit modes instead
of returning only the current mode. The matching Boxedwine engine advertises
640x480, 800x600, 1024x768 and the starting resolution without duplicates.
Wine remains 11.0. The v33 base and regular Wine runtime are unchanged.

## Artifacts and publication

Local directory: `C:\Boxedwine2\tmp\filesystem-v34-release`.

| File | Bytes | Upload path on boxedwine.org |
| --- | ---: | --- |
| `TinyCore15Wine11.0.zip` | 217,014,268 | `/v2/34/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 96,834,692 | `/v2/34/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v34.xml` | | `/v2/26R2/filesV2.xml` |

```text
fe217c209682703edde486d2ef93eca8c8d89610a27a1d038c42b8c71ab164fe  TinyCore15Wine11.0.zip
f4a44763f6de3ce24f1f80aec13062cdfacf5d4d16a870be37ef05b236c7f1db  TinyCore15Wine11.0-web.zip
```

Publish and verify the versioned ZIPs before activating the updated consumers.
Mirror both Wine archives under `/v2/latest/` using identical bytes. Deploy the
hosted catalog after the downloads are available. Only its Wine 11 entry was
changed; all other entries were preserved byte-for-byte from the catalog fetched
November 12. That hosted entry still selected filesystem 11 when fetched.
Merge the updated entry if the live catalog changes before publication.

Publish a rebuilt browser engine and refresh
`/builds/demos/apps/TinyCore15Wine11.0-web.zip` through the updated Jenkins
publisher. This is also the CORS-enabled root used by the separate homepage
prototype. That prototype bundles its own build-133 engine: refresh its
`boxedwine.js` and `boxedwine.wasm` to a build containing the XRandR change when
deploying the new filesystem there. Changing the ZIP cannot update a bundled
engine. The prototype and public build site were not republished by this release.

The unchanged base stays at `/v2/33/TinyCore15WineBase.zip`; its public URL
returned HTTP 200. Both new `/v2/34/` Wine URLs returned 404 at preparation time.
Public download/hash verification remains pending. Earlier releases retain
their original URLs, archive hashes and validation records.

## Inputs and assembly

[`filesystem_wine11.json`](filesystem_wine11.json) selects revision 34 and
[`webgl_filesystems_v34.json`](webgl_filesystems_v34.json), with the
[v52 source manifest](../wineTests/webgl-test-divergences-v52.json): 76
production patches and one separate Wine test adaptation. The new patch is
included in each archive's `usr/local/share/doc/wine/patches/` directory.
The build/validation and Jenkins demo defaults now select the v34 manifest.

WineD3D was rebuilt from the v51 build with the display-mode patch applied.
A separate clean Wine checkout accepted the complete v52 series. All eight
PE32/i386 WebGL DLLs passed size, hash and import validation; the seven other
DLLs retain their revision-33 identities. The new `wined3d.dll` is 3,995,520
bytes, SHA-256
`f06e4456908735c6374a1356c37d5961e8717beecc5fc635b0ec75ad6144179b`.

Dedicated `/home/james/boxedwine-wine11-fs34-{main,web}` directories reused the
unchanged v33 Wine install trees after checking build configuration, all 39
regular Wine patch hashes, and 1,849/1,834 packaged runtime files respectively.
The addon inventory was checked before and after replacement; only WineD3D
changed. Both variants were assembled with fresh prefixes and smoke-tested.
The regular Wine binaries did not require recompilation.

The final archives remove no files or directories compared with v33. Changes
are WineD3D, release metadata and regenerated registries; the new patch document
is added. The web archive used the existing lossless `advzip` optimization;
every member's contents and permissions were checked before and after.

## Consumers and validation

- Shared Windows/macOS/Linux `packages.json` and `filesV2.xml` select the full
  v34 ZIP with its exact size, hash and package revision. Existing applications
  retain their saved package pins.
- Windows UI was rebuilt into `windows-ui/`; Linux UI was staged into
  `linux-ui/`; Mac App Store resources were prepared in
  `mac-app-store-resources/`. All catalogs match the final full ZIP. Linux
  package import and Jenkins demo-root validation passed.
- Jenkins AbiWord selects the final web URL/hash; both demo build scripts and
  `build_site.py` select the v34 web manifest. Native automation's
  `filesystem.properties` selects the final full URL/size/hash. Cached ZIP
  verification and preparation passed without changing the ZIP bytes; the
  separate registry retains the 256 MiB/GDI automation presets.
- `automation36.zip`, the AbiWord overlay and demo catalog are unchanged.
  Jenkins supplies the checkout's current runner and filesystem pin after
  extracting the asset bundle. Manual users of an older extracted bundle need
  the updated `filesystem.properties`. Historical graphics baselines and
  worker-local `BOXEDWINE_GRAPHICS_CONFIG` inputs remain separately qualified.
- Windows and WSL archive hashes agree. Both exact release profiles pass CRC,
  PE/import, GL/cache and registry validation: 7,137 full and 4,661 web ZIP
  entries. Both pass Temp-directory and all eight file-URI checks; full also
  passes Gecko HTML, JavaScript and layout checks.
- The focused display-mode probe passed against each final archive in ST JIT
  at a 1280x720 starting resolution: 426 checks per archive, zero failures,
  process exit zero and successful wineserver cleanup. This covers repeated
  switches, enumeration, backbuffer dimensions, rendered pixels and restoration.
  The earlier candidate also passed 366 checks starting at 800x600 to exercise
  duplicate removal. See [the probe](../wineTests/tests/webgl_display_modes_probe.md).
- 104 builder, 31 web-tooling, 63 Jenkins-tooling, 55 Windows-UI, 83 Linux-UI,
  6 Mac distribution-resource and 34 shared-launcher tests passed. Java runner
  regressions passed with a 32 MiB heap.
- All four browser automation targets were rebuilt. AbiWord matched all four
  reference captures and returned success code 111 in ST, MT, ST JIT and MT JIT
  against the final web ZIP. The threaded modes then logged worker
  `memory access out of bounds` errors during shutdown. The MT run against v33
  with the same engine reproduced this after successful playback. This is a
  runtime cleanup qualification issue, not evidence of a v34-specific failure;
  it is not a clean shutdown pass.

Browser records, runtime file hashes, native engine identity, source-reuse
receipts, inventories, consumer audit and validator sidecars are retained in
the staging directory. Browser tests used the in-app Chromium browser and
in-memory guest roots; they did not invoke a remote Jenkins job.

Native functional/Cinebench worker runs, signed Mac builds, ARM execution and
the full Wine graphics matrix were not run. Public upload, latest aliases,
hosted-catalog activation and public download verification are still pending.
