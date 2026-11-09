# Wine 11.0 filesystem revision 29

Built November 8, 2026. Local candidates; not uploaded. Revision 29 backports
upstream ASF-reader fixes and narrows the local delivery workaround. The v28
base/source archives and the v51 WebGL DLLs remain unchanged. Movie encoding
remains unsupported.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v29-release`.

| File | Bytes | Upload path |
| --- | ---: | --- |
| `TinyCore15Wine11.0.zip` | 217,092,523 | `/v2/29/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 96,909,056 | `/v2/29/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v29.xml` | | `/v2/26R2/filesV2.xml` |

```text
bc383edb8cf58b4895645754e02e2c16404850cd59afd408ace06e4c7f09341e  TinyCore15Wine11.0.zip
407d92cce8d04d7962bf71ba235a1e443b68cc2e39da90c3ad7c0fbf21821dd3  TinyCore15Wine11.0-web.zip
```

Upload and verify both versioned ZIPs before activating consumers. Copy those
exact bytes to the same filenames under `/v2/latest/`, then upload the hosted
catalog. Its other Wine versions and demo entries are preserved. The shared
native catalog used by Windows, macOS, and Linux, Jenkins, the native runner,
and the demo-site defaults all select v29. UI distributions were not rebuilt
in this follow-up; rebuild their bundled catalogs when shipping.

Do not replace the published v28 archives. No new base or media source upload
is needed. Continue using `/v2/28/TinyCore15WineBase.zip` and
`/v2/28/TinyCore15WineBase-media-sources.zip`, with the hashes recorded in
[RELEASE-v28.md](RELEASE-v28.md). The media source bundle remains available
alongside the runtime downloads for source/license compliance.

## Source changes and rationale

The Wine base is still `wine-11.0`, commit
`db11d0fe6a169c457e23d007e20404643d067aa8`, targeting GLib 2.78.
Both variants apply these upstream patches in this order, including their tests:

1. [e33a8cc4](https://github.com/wine-mirror/wine/commit/e33a8cc4ba7d2e9a0d38202968b44ab5d5960615): protect seeking against unconnected pins.
2. [bbbd1d8a](https://github.com/wine-mirror/wine/commit/bbbd1d8a34548d679da341a1deaa378e1f58670e): release WMReader before destroying its pins and resources.
3. [a0a82c47](https://github.com/wine-mirror/wine/commit/a0a82c471b09a0c0b8b5049bc2a696286422d492): do not start a stopped reader when seeking.

The upstream-only build returns `VFW_S_STATE_INTERMEDIATE` from the five-second
Pause/GetState check: the graph cannot finish audio/video preroll. Therefore
`wine11_qasf_reader_delivery_after_upstream.patch` remains necessary. It creates
bounded delivery queues only for outputs whose `ReceiveCanBlock` is not
`S_FALSE`; nonblocking outputs retain WMReader's shared callback thread.
It flushes all connected outputs, releases pending samples, joins delivery
workers before freeing pins, drains samples before EOS, and preserves segment
positions/timestamps across seeking. The upstream stopped-reader guard and
early WMReader release remain in place.

The old `wine11_qasf_reader_delivery.patch` is retained only as historical v28
source. It is no longer selected by active builds or included in v29 ZIPs.
The ASF writer availability stub is unchanged and still rejects encoding.

Both configured Wine trees were rebuilt and installed before packaging. Only
`opt/wine/lib/wine/i386-unix/qasf.dll.so` changed among the Wine binaries; its
SHA-256 is `1e75af54787ba784bbcc92e099f4fa6963b9e3acefb9282623d02d4a3f39de22`
in both variants. Addon inputs and file inventories were verified unchanged.
The recorded reader patch series applies cleanly to pristine Wine 11.0 and
exactly reproduces the built reader and test sources.

## Validation

- Both archives pass CRC, PE/import, GL/cache, registry and manifest validation.
  Main has 7,146 entries; web has 4,669 and remains below 100,000,000 bytes.
- Both packaged URI/Temp smoke tests pass; main also passes Gecko
  HTML/JavaScript/layout checks.
- Fresh roots for each final ZIP pass the writer contract, full WMV decode
  (429 audio/video samples through EOF), and DirectShow graph regressions.
  These cover stopped seeking, completed preroll, Run, EOS, Stop/restart,
  seeking during playback, and cleanup. Every probe reports zero failures.
- Each final ZIP passes Wine's ASF-reader suite: 272 checks, 25 marked todo,
  zero failures and zero skipped. This includes upstream unconnected-seek and
  running-reader destruction cases, plus blocking/nonblocking delivery threads.
- Builder tests: 103 passed. Web tooling tests: 31 passed.
- The Movies launches with the final main ZIP, passes startup/media checks,
  and reaches the animated main menu. The user then took control of the movie
  player. No native Microsoft media DLLs or loose Wine-module overrides exist
  in either final test root. Game files were mounted from their existing
  location, not copied; the original installed prefix was not modified.

The Windows x64 engine used for the final probes and game was:
`C:\Boxedwine2\tmp\movies-writer-stub-20261108\media-engine\BoxedwineEngine.exe`,
SHA-256 `2f305611dd6ac56590c8e35aebcaa306cc46786c645dd51509c3b6aca1fb3179`.
It contains the FPU reset and pthread floating-point inheritance fixes required
by the v28 media stack. This follow-up does not change that engine binary.

Web-package tests above ran in the native Windows engine. Browser execution,
macOS/Linux game execution, visual game startup with the web ZIP, full gameplay,
saving/loading, and export handling were not qualified in this follow-up.
The main-package tutorial check from the earlier loose-module experiment is
historical evidence, not a v29 final-ZIP result. See [RELEASING.md](RELEASING.md)
for broader platform, UI, and automation release qualification.

Exact ZIPs, validation sidecars, input hashes, inventory comparisons, and logs
are staged in `tmp/filesystem-v29-release`. Local diagnostic logs and test roots
are under `tmp/movies-writer-stub-20261108`; these generated files and the private
game registry export are not source-control inputs.
