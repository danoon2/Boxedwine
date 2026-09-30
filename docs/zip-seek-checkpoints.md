# Compressed ZIP seeking

Boxedwine keeps a bounded, in-memory DEFLATE seek index in each `FsZip`. ZIP
members remain compressed in the original archive; no filesystem cache or ZIP
repacking is required.

## Why this is needed

Wine Gecko 2.47.4's `xul.dll` is about 68.4 MiB uncompressed. During PE relocation,
Wine alternates between relocation records near offset 54 MiB and pages near the
start of the DLL. The old ZIP reader restarted inflation on backward seeks and
entry switches, and discarded all intervening output on forward seeks.

The September 28, 2026 profile found one XUL handle, mostly 4 KiB reads, and
22.09 GB of discarded inflation in the first 90 seconds. A dedicated inflater
for that handle would still have incurred about 86% of that work. Caching only
requested pages would also miss the problem: nearly all returned bytes were
unique, while discarded bytes were repeatedly inflated.

## Implementation

* Save a checkpoint at each 1 MiB boundary crossed during reads or skips.
* Use zlib `inflateCopy` to retain the inflater, its dictionary, unread compressed
  input, MiniZip's remaining-byte counts, and accumulated CRC. Checkpoints work
  inside DEFLATE blocks and preserve CRC validation when reading to EOF.
* Before seeking, choose the furthest usable position at or before the target:
  either the live stream or a saved checkpoint. Inflate only the remaining gap.
* Share the index across handles and entries, under the archive's existing read
  mutex. Evict the least recently used checkpoint when reaching 128 entries.
  This caps retained checkpoint state at approximately 7 MiB per archive with
  the current zlib and MiniZip buffer sizes. Archives allocate checkpoints lazily.
* Stored ZIP members continue using direct file reads. Unsupported snapshot
  formats continue through MiniZip without an index.

The first visit to a distant, unindexed offset still needs a sequential scan.
Eviction can also require rescanning. This is a bounded seek optimization, not a
guarantee that arbitrary access to every archive is as fast as an extracted file.

The MiniZip snapshot API is local to Boxedwine. Snapshots belong to one open
archive, and must be freed before that archive closes. Its saved directory offset
comes from `pos_in_central_dir`: `unzGetOffset64` returns zero after
`unzSetOffset64` because of MiniZip's `num_file` sentinel.

The accompanying fixes check read/open/seek errors and premature EOF, preserve
the file position on failed reads and negative seeks, allow seeks beyond EOF,
track closed handles, reopen stored-file descriptors, close them on destruction,
and remove incomplete filesystem copies while releasing both handles.

## Measurements and validation

Same revision-13 TinyCore15Wine11.0 ZIP, fresh Wine roots, local HTML/JavaScript
completion callback, Linux release build under WSL:

| Reader / storage | Gecko probe completion |
| --- | ---: |
| Previous reader, deflated ZIP | Did not finish within 90 seconds |
| Previous reader, XUL and omni.ja stored | 11.7 seconds |
| Checkpoint reader, original deflated ZIP | 12.8 and 13.9 seconds |
| Checkpoint reader, `-cacheReads` | 8.9 seconds |

The stored-entry variant increased the ZIP by 45,431,063 bytes. Checkpoints add
zero bytes to the ZIP, and neither XUL nor omni.ja appeared in the probe roots.

Native Windows Cinebench R11.5, original executable, a fresh root, and a load
callback appended to an isolated copy of its EULA HTML: the compressed ZIP with
checkpoints loaded the EULA in 28.3 seconds and reported 5,179 text characters.
The `-cacheReads` comparison took 35.2 seconds in that run. These are startup
measurements, not rendering benchmark scores; the EULA was not accepted.

Validation includes the Linux filesystem/mapping regression slice (85 tests),
new compressed/stored random-access and handle-lifetime tests, concurrent handles,
checkpoint eviction, malformed DEFLATE and premature EOF, failed extraction
cleanup, Linux and Windows release builds, ASan/UBSan snapshot checks with the
real XUL member, and Emscripten compilation plus a Node-run snapshot/CRC test.
The complete browser application was not tested under Emscripten in this run.

Local measurements and diagnostic harnesses are under
`tmp/gecko-zip-20260928/`; the repository regression lives in
`source/test/fs/testHardLinks.cpp` (`testZipRandomAccess`).
