# Wine 11.0 filesystem revision 30

Built November 9, 2026. Local candidates; not uploaded. Revision 30 fixes the
periodic pauses in The Movies' WMV intros by letting DirectShow renderers
schedule presentation. Wine remains 11.0. The v28 media base/source archives
and v51 WebGL DLLs are unchanged. Movie encoding remains unsupported.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v30-release`.

| File | Bytes | Upload path on boxedwine.org |
| --- | ---: | --- |
| `TinyCore15Wine11.0.zip` | 217,095,684 | `/v2/30/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 96,910,727 | `/v2/30/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v30.xml` | | `/v2/26R2/filesV2.xml` |

```text
2e1693a9348e89cead788effdf6a5201a8dded3cb32deb1bdff0b7722e2a3b60  TinyCore15Wine11.0.zip
4a3ff6df91496e0331887191e04a7dd86eedef2844854be889aeb4e2ccd5f885  TinyCore15Wine11.0-web.zip
```

Upload and verify both versioned ZIPs first. Copy those exact bytes to the same
filenames under `/v2/latest/`, then deploy `filesV2.hosted-v30.xml` as the hosted
legacy catalog. This catalog was fetched from the server on November 9; only
its Wine 11 entry was updated, preserving other Wine versions and demo entries.
If the hosted catalog changes before upload, merge that entry into the newer
catalog instead of overwriting those intervening changes.

The shared Windows/macOS/Linux native catalog, Jenkins browser automation,
the native automation runner, and demo-site defaults now select revision 30.
Their URLs become usable after the upload. Existing applications keep their
saved package pins; this release does not migrate installed applications.

No new base or media source upload is needed. Continue using
`/v2/28/TinyCore15WineBase.zip` and
`/v2/28/TinyCore15WineBase-media-sources.zip`, with the identities in
[RELEASE-v28.md](RELEASE-v28.md). Previous archives and release receipts retain
their identities. The AbiWord overlay and native `automation36.zip` asset
bundle are unchanged; the latter contains no filesystem.

## Build inputs and packaged change

The source is still `wine-11.0`, commit
`db11d0fe6a169c457e23d007e20404643d067aa8`, targeting GLib 2.78.
`wine11_qasf_renderer_clock.patch` follows the reader backports and bounded
delivery patch from revision 29 in `wine_builds.json`, for both variants.
Its SHA-256 is
`965d33da7d0b8b11daa6a42f902b654781a1f15990ce684e9f0bda120b5083e5`.

The ASF reader requests samples ahead through WMReader's user-provided clock.
Downstream renderers retain graph-clock scheduling, and the existing allocators
and bounded delivery queues limit buffering. This prevents a future audio
timestamp from holding up earlier video samples on the reader's shared callback
thread. The reader also acknowledges `OnTime` notifications successfully.
Blocking and nonblocking unclocked-sink tests cover the read-ahead contract.

Main and web Wine trees were configured, rebuilt and installed in separate
`/home/james/boxedwine-wine11-fs30-{main,web}` directories. Unchanged addon inputs
and inventories were verified before reusing their overlays. Both prefixes were
regenerated and the web ZIP was losslessly recompressed by the normal builder.
The recorded reader patch series applies cleanly to pristine Wine 11.0 and
exactly reproduces both built reader and test sources.

Only `opt/wine/lib/wine/i386-unix/qasf.dll.so` changed among the Wine binaries
relative to v29. Its SHA-256 is
`5042ca571df7fa585a49b5f209893aa77b7448b69d118dc134b572b281e415e2`
in both variants. Other archive changes are release metadata, generated prefix
registries/timestamps, and the included patch documentation. No files were
removed. WebGL DLLs, GL libraries and the media base remain byte-identical.

## Completed validation

- Both final ZIPs pass CRC, PE/import, GL/cache, registry and exact manifest
  validation. Main contains 7,147 entries; web contains 4,670 and is below the
  100,000,000-byte web limit. Packaged versions are filesystem 30 and Wine 11.0.
- Both packaged Temp/URI smoke tests pass; main also passes Gecko HTML,
  JavaScript and layout checks.
- Both final ZIPs, in fresh roots with no loose reader override or native
  Microsoft media DLLs, pass 314 ASF-reader checks each: 63 known todos, zero
  failures and zero skips. The new read-ahead checks fail with the v29 reader.
- Both final ZIPs pass the writer contract, decode all 429 audio/video samples
  of the game's Activision intro through EOF, and pass both DirectShow graph
  playbacks. These cover stopped and active seeking, preroll, pause/resume,
  EOS, restart and clean shutdown.
- The Movies plays its startup videos and reaches its menu/quit screen using
  the final main ZIP in a fresh root, then exits with code 0. Game files were
  mounted from the existing installation without copying them. The test root
  has no loose Wine module or native Microsoft media DLL overrides.
  The user also confirmed the existing installation works after switching its
  package pin to v30 and removing the loose experimental reader.
- Builder tests: 103 passed. Web tooling: 31 passed. Jenkins tooling: 63 passed.
  Windows UI: 53 passed. Linux UI backend: 80 passed. Mac distribution-resource
  tests: 6 passed. Java runner regression tests pass with a 32 MiB heap.
- Windows UI rebuilt successfully at `windows-ui\Boxedwine.exe`, with the
  revision 30 shared catalog. Its existing engine was copied into the staged
  distribution; this filesystem release does not rebuild the engine.
  The normal `project\msvc\BoxedwineUI\bin\Release\net10.0-windows` output was
  also rebuilt and its bundled catalog verified against the shared source.
- Mac App Store resources staged successfully at `mac-app-store-resources`,
  including verification and bundling of the exact final main ZIP.
- Linux imports the final main ZIP, matches it to the new shared catalog, and
  creates a built-in Notepad application. Its catalog is staged at
  `linux-resources/WindowsSupport/packages.json`.
- Native `PrepareFilesystem` verifies the final main ZIP from a populated local
  cache and stages the unchanged ZIP plus the separate automation registry.
  Its 256 MiB/GDI presets remain unchanged. Demo-site root validation accepts
  the final web ZIP using the new default manifest.

The native Windows engine used for packaged media tests is
`project\msvc\BoxedwineUI\bin\Release\net10.0-windows\Runtime\BoxedwineEngine.exe`,
SHA-256 `ca3d2b4481884669c3e6fd5b0b096cc187b166c6772005a24d470e7fdfdc8ec1`.
The builder's Linux engine is `project/linux/Build/Release/boxedwine`, SHA-256
`fea449930c2e5142234fdae3a5f9e45aa7903e992300e6d1ccdc2de9601ede44`.

Browser execution (including the four AbiWord modes), full native CI suites,
macOS/Linux game execution, full gameplay/save/load/export, and clean downloads
from the new public URLs were not qualified in this release. Web media tests
above used the native Windows engine. macOS application binaries and Linux UI
binaries were not rebuilt; their shared catalog source is updated. These limits
do not turn the earlier v29 or loose-module tests into v30 final-ZIP evidence.

Exact ZIPs, SHA256SUMS, validation sidecars, build logs, inventories and test roots
are retained in `tmp/filesystem-v30-release`. The earlier pacing comparison and
native Windows reference test remain in `tmp/movies-pacing-20261109` and are
described in [the probe guide](probes/movies-writer-stub/README.md).
