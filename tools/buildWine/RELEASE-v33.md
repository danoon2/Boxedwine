# Wine 11.0 filesystem revision 33

Built November 11, 2026. Staged locally; not uploaded. Revision 33 adds
GStreamer's WAV parser to the base, full and web filesystems. This fixes the
RalliSport Challenge main-menu stall caused by failed PCM WAV music playback.
Wine remains 11.0; existing Wine and WebGL binaries are unchanged.

## Artifacts and publication

Local directory: `C:\Boxedwine2\tmp\filesystem-v33-release`.

| File | Bytes | Upload path on boxedwine.org |
| --- | ---: | --- |
| `TinyCore15WineBase.zip` | 71,975,004 | `/v2/33/TinyCore15WineBase.zip` |
| `TinyCore15Wine11.0.zip` | 217,013,434 | `/v2/33/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 96,833,775 | `/v2/33/TinyCore15Wine11.0-web.zip` |
| `TinyCore15WineBase-wavparse-sources.zip` | 3,912,956 | `/v2/33/TinyCore15WineBase-wavparse-sources.zip` |
| `filesV2.hosted-v33.xml` | | `/v2/26R2/filesV2.xml` |

```text
ce55a695a5addbf8abab82d98e5c46d5787b517c570e6ac49deae1643cdf20bd  TinyCore15WineBase.zip
5aa015fb7a1b36ec991af670d837101bd5626ec5a1173dd628225c4d92185280  TinyCore15Wine11.0.zip
b6ea4e78031e3c430b3aceecd86fc5c089c538f401003c17ce2ca20b99c00032  TinyCore15Wine11.0-web.zip
52ab9c97a93fdbb6f204264f483c1fe1533a488e9af7c597dbdf6fbfa0ae5173  TinyCore15WineBase-wavparse-sources.zip
```

Upload and verify the versioned ZIPs first. Mirror the two Wine archives under
`/v2/latest/` using identical bytes, then deploy the hosted catalog and activate
the new UI/Jenkins pins. The hosted catalog was fetched November 11; only its
Wine 11 entry changed. Other Wine versions and demos were preserved byte-for-byte.
Merge that entry if the hosted catalog changes before publication.

The shared Windows/macOS/Linux catalog, native automation runner, Jenkins browser
automation and demo-site defaults select revision 33. Existing applications keep
their saved package pins. The AbiWord overlay, `automation36.zip` and demo catalog
are unchanged. Revision 32 work directories already existed for a separate
CyberSpeed change; this release uses the checked-out v31 inputs and leaves those
directories intact.

## Inputs and packaging

[`build_wav_base.py`](build_wav_base.py) verifies the published, trimmed v31 base
(`e64a97dffa5962f3278ac43f3160382636ca95585dcf13d3f39fb63236d11fe7`)
and extracts `usr/local/lib/gstreamer-1.0/libgstwavparse.so` from the pinned
TinyCore 15 `gst-plugins-good.tcz`. The parser is 46,972 bytes, SHA-256
`11b7bcda096572abdae48f31d0f7aeff5d9d64c0a409a17beb7e969afaa27e0d`.

All pre-existing base members and Unix permissions are retained; only
`recreateInstructions.txt` changes. Additions are the parser, its license,
builder and provenance. The source ZIP contains GStreamer 1.16.2 source,
both builder scripts and the base receipt. Embedded builder sources match the
checkout. The gamma library and guest loader cache retain their v31 identities.
The web pruning policy retains the same removals and preserves the new parser.

Rebuild the base in WSL with:

```bash
python3 tools/buildWine/build_wav_base.py \
  --work-dir "$HOME/boxedwine-base-v33-new" \
  --base /path/to/v31/TinyCore15WineBase.zip
```

Wine remains at `db11d0fe6a169c457e23d007e20404643d067aa8` (`wine-11.0`).
The full and web v31 install trees were copied to dedicated
`/home/james/boxedwine-wine11-fs33-{main,web}` directories after verifying all
39 patch hashes, build configuration and addon inventories. Respectively 1,849
and 1,834 Wine runtime files matched their prior archives. Both filesystems were
reassembled against the new base with fresh prefixes. No Wine recompilation was
needed for this plugin addition. The v51 WebGL DLLs retain their hashes.

The final full ZIP was also compared directly with the installed, published v31
ZIP (`1e57a3bfe620fafdc95abdeec6a7aaaf37ba64c8c798640f939524c32a12040a`).
No members were removed and all runtime bytes/permissions are unchanged.
Apart from the parser, additions are documentation and provenance restored by
normal assembly. Changed files are release metadata and generated registries.
The web archive uses the existing lossless `advzip` pass, with member contents
verified before and after compression, and stays below 100,000,000 bytes.

## Validation

- Windows and WSL SHA-256 results agree for all four ZIPs. CRC checks pass.
  Full and web pass exact manifest, DLL/import, library/cache, registry and
  version checks, with 7,136 and 4,660 ZIP entries respectively. The new manifest
  also verifies the exact WAV parser bytes.
- Both Wine archives pass Temp-directory and all eight file-URI checks. Full
  also passes Gecko HTML, JavaScript and layout checks; web omits Gecko.
- Fresh roots using both final Wine archives pass [`wav-playback.c`](probes/wav-playback.c):
  DirectShow renders the generated PCM WAV, reports 2,500,000 duration units and
  reaches `EC_COMPLETE`. The v31 control fails at `RenderFile` with `0x80040218`.
  The base archive passes `gst-launch-1.0 filesrc ! wavparse ! fakesink` through EOS.
- Chrome headless with the existing multithreaded JIT Emscripten runtime passes
  the same WAV probe against the exact final web ZIP: one test, zero failures,
  process exit status zero. Runtime/input hashes and output are retained under
  `browser-wav-mtjit/` in the staging directory.
- 104 builder, 31 web-tooling, 63 Jenkins-tooling, 54 Windows-UI, 80 Linux-UI and
  6 Mac distribution-resource tests pass. Java runner regressions pass with a
  32 MiB heap.
- Windows UI was rebuilt into `windows-ui/`; Linux UI staged into `linux-ui/`;
  Mac App Store resources staged into `mac-app-store-resources/`. Their catalogs
  match the final full ZIP. Linux package import and demo web-root validation pass.
- Native automation cache verification and filesystem preparation pass. The
  staged `fs.zip` remains byte-identical, with the existing 256 MiB/GDI presets
  in a separate `user.reg`.

The native probes used Linux Boxedwine SHA-256
`fea449930c2e5142234fdae3a5f9e45aa7903e992300e6d1ccdc2de9601ede44`.
The WAV probe executable SHA-256 is
`059c8ac3007d1e20ce425a4038c378fffac7ccbfac25b1a1c29dda7d32bf4425`.
The tested browser runtime's WASM SHA-256 is
`b128ba2c5e12ea3df1d3e9aad84aa41cdfc3ef87f54d3facbee83d1f0af1a3e9`;
its source commit was not inferred from the current checkout.

Probe runs used `GST_REGISTRY_FORK=no` to avoid the standalone GStreamer plugin
scanner; the first base-only scan without it timed out. The original native
Wine probe and the installed game fix also worked without that environment
override. Existing warnings for unavailable optional GStreamer sinks did not
prevent WAV playback.

The earlier installed-game check restored menu interaction and a race using the
same parser as a loose override. The final archives were tested with fresh roots
and the standalone probe; the game was not reinstalled against v33. Full AbiWord
qualification in all four modes, native functional/Cinebench worker runs, signed
Mac builds and public-download verification were not run for this release.
These are distinct from the completed packaging and focused regression checks.
