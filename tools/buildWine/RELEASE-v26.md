# Wine 11.0 filesystem revision 26

Built November 7, 2026 on `feat/webgl-perf-staging`. Staged locally; upload and
consumer deployment are pending. Both archives include all eight WebGL DLLs
rebuilt from the v50 patch series, including constant-upload deduplication,
fewer vertex-array disables, and the 256 KiB streaming buffer.

The regular Wine build configuration and every selected patch hash match the
validated revision 25 builds. Those binaries, including the DirectSound SSE
backport, were reused and repackaged with this branch's explicit directory
permissions. Addons, prefixes, and final archives were assembled again.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v26-release`.

| Local artifact | Bytes | Upload under the boxedwine.org document root |
| --- | ---: | --- |
| `TinyCore15Wine11.0.zip` | 215,215,003 | `/v2/26/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 99,629,744 | `/v2/26/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v26.xml` | | `/v2/26R2/filesV2.xml` |

The versioned URLs after upload are
[full](https://boxedwine.org/v2/26/TinyCore15Wine11.0.zip) and
[web](https://boxedwine.org/v2/26/TinyCore15Wine11.0-web.zip).
Exact hashes are pinned in [the release manifest](webgl_filesystems_v26.json)
and the staging directory's `SHA256SUMS.txt` / `release-manifest.json`.

## Completed

- [x] Clean WebGL source preparation at Wine 11.0 commit `db11d0fe6a169c457e23d007e20404643d067aa8`; apply all 73 production patches and the selected test adaptation.
- [x] Rebuild all eight WebGL DLLs with the local MinGW GCC 13 toolchain. Seven exactly reproduce revision 25; only `wined3d.dll` changes. Record the old candidate pins and new identities in `webgl-build-comparison.json`.
- [x] Verify cached main/web Wine source configuration, selected patch hashes and package hashes; regenerate Wine ZIP directory metadata from their build installations.
- [x] Fresh addon builds, prefix initialization, full Gecko installation, and both final filesystem archives.
- [x] Validate exact ZIP profiles, CRCs, version/changelog, patch sources, all DLLs/imports, GL links/cache, registry defaults, Temp directories and traversable Wine directories. The web ZIP remains below 100,000,000 bytes.
- [x] Compare revision 25 inventories: no missing files/directories; the three new WebGL patch documents are the only added files. All 1,849 main and 1,834 web regular Wine files are byte-identical to revision 25.
- [x] Both native URI/Temp smoke tests; full Gecko HTML, JavaScript and layout smoke.
- [x] Packaged WebGL rendering probe: 23,502 assertions per run, zero failures/skips in five runs (full ST JIT; web ST/MT with and without JIT). Covers repeated constants, dynamic vertex buffers, 76 indexed draw/readback cases and 5,120 signed-texture pixels.
- [x] Builder (101), web tooling (31), site (20), local site (7), publish site (12), Linux UI (76), and updated UI catalog/core (37) tests; Java runner regression tests.
- [x] Shared Mac/Windows/Linux `packages.json` and `filesV2.xml` pins; Mac App Store and Direct resource preparation; Linux validation of the final main ZIP.
- [x] Hosted legacy catalog staged, changing only Wine 11's URL/revision/size and preserving other versions and demos.
- [x] Jenkins AbiWord web URL/hash; native runner URL/hash/size; demo-site build/publish defaults and manifest. Generate 11 demos in all four runtime modes with the final web root.
- [x] Retain automation36, AbiWord overlay, demo catalog lock, older release manifests and historical test inputs.

## Upload and activation checklist

- [ ] Upload both ZIPs to `/v2/26/`, preserving the exact staged bytes.
- [ ] Copy those same bytes to `/v2/latest/TinyCore15Wine11.0.zip` and `/v2/latest/TinyCore15Wine11.0-web.zip`.
- [ ] Upload `filesV2.hosted-v26.xml` as `/v2/26R2/filesV2.xml`. Do not replace the hosted catalog with the smaller bundled native catalog.
- [ ] Run `verify-published.ps1` from the staging directory to download and verify the versioned ZIPs, latest aliases and hosted XML.
- [ ] Commit/merge and activate repository/UI/Jenkins pins after the downloads are available. Publish the demo site separately.
- [ ] Qualify native functional/Cinebench workers and AbiWord ST/MT/ST JIT/MT JIT with the new inputs.
- [ ] Rebuild/install desired UI distributions and test clean downloads plus existing applications. Prepared resource directories are not finished installers.
- [ ] If the optional graphics Jenkins job is used, update and qualify its external worker JSON; repository pins do not update that file.
- [ ] Retain earlier versioned ZIPs and package records for rollback.

The staging directory retains build logs, inventories, comparisons, sidecars,
probe source/executable/results, runtime identities, UI resource staging, and
`validation-summary.json`. Browser/native runtime binaries were reused, and
their identities are recorded without attributing them to this source commit.
Manual MW3/3DMark performance and Halo listening runs were not repeated on these
final archives. The DirectSound DLL is byte-identical to the Halo-tested DLL;
revision 25's audio integration results remain historical evidence.

Follow [RELEASING.md](RELEASING.md) for the complete consumer inventory and
reproduction commands. No upload, commit or push was performed for this release.
