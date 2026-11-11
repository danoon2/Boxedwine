# Wine 11.0 filesystem revision 31

Built November 10, 2026. Staged locally; not uploaded. Revision 31 fixes builtin
DLL aliases used by the Sims 3 launcher and adds the XF86VidMode gamma bridge
to a new base filesystem. Wine remains 11.0. Fullscreen gamma correction needs
the matching Windows OpenGL engine; other backends and older engines report
the extension as unavailable.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v31-release`.

| File | Bytes | Upload path on boxedwine.org |
| --- | ---: | --- |
| `TinyCore15WineBase.zip` | 71,943,620 | `/v2/31/TinyCore15WineBase.zip` |
| `TinyCore15Wine11.0.zip` | 216,619,829 | `/v2/31/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 96,465,405 | `/v2/31/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v31.xml` | | `/v2/26R2/filesV2.xml` |

```text
e64a97dffa5962f3278ac43f3160382636ca95585dcf13d3f39fb63236d11fe7  TinyCore15WineBase.zip
1e57a3bfe620fafdc95abdeec6a7aaaf37ba64c8c798640f939524c32a12040a  TinyCore15Wine11.0.zip
90abfb658c816165e6c17b605f134b4e8db660414aef189ff7536ef17e965c06  TinyCore15Wine11.0-web.zip
```

Upload and verify all three versioned ZIPs first. Mirror the two Wine archives
under `/v2/latest/`, using exactly the same bytes. Then deploy the hosted catalog
and activate the new UI/Jenkins pins. The hosted catalog was fetched November 10;
only its Wine 11 entry changed, preserving other versions and demo entries.
Merge that entry if the hosted catalog changes before upload.

The shared Windows/macOS/Linux native catalog, native automation runner,
Jenkins browser automation and demo-site defaults now select revision 31.
Existing applications retain their saved package pins. The AbiWord overlay and
`automation36.zip` asset bundle are unchanged; the latter contains no filesystem.
Previous versioned archives and receipts remain unchanged.

## Manual archive update

The archives were manually updated on November 10 after initial validation.
The updated hashes and sizes above describe those exact files. The update removed
25 base, 103 full and 102 web entries under `usr/local/share/doc/`; all surviving
file contents and permissions are unchanged. CRC, manifest and pin checks were
repeated. Runtime probe results below refer to the original archives, whose
runtime payloads are byte-identical. Original build records and embedded base
hashes retain their build-time identities; current download pins use the new hashes.

## Base and Wine inputs

`build_x11_base.py` verifies the v28 media base, builds the 32-bit gamma bridge,
installs it alongside the existing X11 libraries at
`/usr/local/lib/libXxf86vm.so.1`, and runs the guest's `ldconfig`.
Before manual trimming, no base files were removed compared with v28. Only `etc/ld.so.cache` and
`recreateInstructions.txt` changed; additions are the library and its provenance,
sources and GPL license. The library SHA-256 is
`3eeee9f7566a082f5915b38d28fbb103782e783417faa45e1200f33d2ad6211a`.
The existing X11 and media libraries are byte-identical. The media source bundle
remains at `/v2/28/TinyCore15WineBase-media-sources.zip`. The base builder
includes gamma sources under `/usr/local/share/doc/boxedwine-xf86vm`; this
directory was removed in the manual archive update.
The web pruning policy was audited against this base delta and keeps the same
removal list, with the new base hash.

Wine remains at `db11d0fe6a169c457e23d007e20404643d067aa8` (`wine-11.0`).
Both release trees were copied into dedicated
`/home/james/boxedwine-wine11-fs31-{main,web}` directories, configured, rebuilt
and installed with the loader object explicitly rebuilt. All 39 selected patches
were applied to pristine Wine sources and verified against both trees. Unchanged
addon profiles and file inventories were verified before reuse. Both archives
were then repackaged against the new base with freshly initialized prefixes.
The v51 WebGL DLLs retain their earlier hashes.

The only changed Wine binary is `opt/wine/lib/wine/i386-unix/ntdll.dll.so`:
`f5edf24641e18042e034e5b917e69b9c92184dc71653546b0136fdb97eb68608`.
The normal builder's existing pruning policy also removes
`winemenubuilder.exe.so`, which the earlier incremental install had retained.
Other differences are the base additions/cache, patch documentation, release
metadata and generated prefix registries/timestamps. The final changelog was
updated to describe the base release; all other ZIP members and their compressed
sizes were verified unchanged by that update.

## Completed validation

- All three archives pass CRC checks and independent Windows/WSL hashes.
  Full and web pass exact manifest, DLL/import, GL/library-cache, registry and
  version checks: 7,052 and 4,576 entries respectively. Web is below 100,000,000 bytes.
- Both original Wine ZIPs passed Temp/URI checks; full also passes Gecko HTML,
  JavaScript and layout checks.
- Fresh roots using the original archives, without loose Wine/X11 overrides, pass
  26 builtin CRT checks and 54 loader lifetime/alias checks per variant.
- Both original archives passed the gamma identity, nonlinear round trip, explicit
  restore and child-exit cleanup probe with zero failures.
- 104 builder tests, 31 web tooling tests, 63 Jenkins tooling tests, 54 Windows
  UI tests, 80 Linux UI tests and 6 Mac distribution-resource tests pass.
  Java runner regression tests pass with a 32 MiB heap.
- Windows UI rebuilt into `windows-ui/`; its bundled catalog matches the final
  main archive. Mac App Store resources staged and verified. Linux package import
  and builtin-app setup pass. Demo-root validation selects the final web ZIP.
- Native runner cache verification and filesystem preparation pass, retaining
  the exact full ZIP and the separate 256 MiB/GDI automation registry presets.
  Hosted catalog preservation and all active URL/hash pins were checked.

Windows probe engine SHA-256:
`64d2b4c78d5f932f3b99b9f8ff041907d0117f606ca1fa7da9d2e03708f4fb66`.
Linux prefix/smoke engine SHA-256:
`fea449930c2e5142234fdae3a5f9e45aa7903e992300e6d1ccdc2de9601ede44`.

The earlier Sims 3 fullscreen/native comparison used the same loader and gamma
library binaries and was confirmed by the user. The complete game was not rerun
with the final v31 ZIPs. Browser AbiWord's four modes, complete native CI suites,
macOS/Linux GUI game runs and public downloads were not tested in this release
pass. macOS/Linux application binaries were not rebuilt; their catalog sources
and the staged Mac resources were updated. Upload remains the user's next step.

Exact archives, manifests, `SHA256SUMS.txt`, inventories, build logs, source-series
verification and probe results are retained in the staging directory.
`release-validation.json` records the tested identities and qualification limits.
