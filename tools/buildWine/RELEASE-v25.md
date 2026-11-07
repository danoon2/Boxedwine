# Wine 11.0 filesystem revision 25

Built November 7, 2026. Staged locally; upload and consumer deployment are pending.
Both variants rebuild Wine 11.0 with the upstream DirectSound FIR/SSE backport.
The main and web DirectSound DLLs exactly match the DLL tested in the Halo beach
comparison. The separate v49 WebGL DLLs retain their revision 24 identities.

## Artifacts and upload destinations

Local directory: `C:\Boxedwine2\tmp\filesystem-v25-release`.

| Local artifact | Bytes | Upload under the boxedwine.org document root |
| --- | ---: | --- |
| `TinyCore15Wine11.0.zip` | 215,210,963 | `/v2/25/TinyCore15Wine11.0.zip` |
| `TinyCore15Wine11.0-web.zip` | 99,626,437 | `/v2/25/TinyCore15Wine11.0-web.zip` |
| `filesV2.hosted-v25.xml` | | `/v2/26R2/filesV2.xml` |

The versioned URLs after upload are
[full](https://boxedwine.org/v2/25/TinyCore15Wine11.0.zip) and
[web](https://boxedwine.org/v2/25/TinyCore15Wine11.0-web.zip).
Exact hashes are pinned in [the release manifest](webgl_filesystems_v25.json)
and the staging directory's `SHA256SUMS.txt` / `release-manifest.json`.

## Completed

- [x] Add the version-bounded DirectSound backport and include its source in both ZIPs.
- [x] Advance filesystem profile, both changelogs, validator default, and versioned manifest to 25.
- [x] Fresh main and web Wine builds; fresh prefixes and filesystem assembly.
- [x] Validate ZIP hashes/CRCs, version metadata, source patch, DLLs, registry and Temp directories; web is below 100,000,000 bytes.
- [x] Compare revision 24 inventories: no missing files/directories; only the new patch document was added. Record regenerated files and changed DLLs.
- [x] Both native URI/Temp smoke tests; full Gecko HTML, JavaScript and layout smoke.
- [x] DirectSound integration: 413 assertions on native main and web, full browser ST JIT, and web ST/MT with and without JIT; zero failures in all seven runs.
- [x] Builder (101), web tooling (31), site (20), local site (7), publish site (12), Linux UI (76), and updated UI catalog/core (37) tests; Java runner regression tests.
- [x] Shared Mac/Windows/Linux `packages.json` and `filesV2.xml` main ZIP pins.
- [x] Mac App Store and Direct resource staging; Linux package validation against the final main ZIP.
- [x] Hosted legacy catalog staged, preserving other Wine versions and demos. Its previous Wine 11 entry still pointed at revision 11.
- [x] Jenkins AbiWord web URL/hash; native `filesystem.properties` URL/hash/size.
- [x] Demo-site build/publish defaults and manifest; 11 demos generated in all four modes with the final web root.
- [x] Preserve automation36, AbiWord overlay, demo catalog lock, older manifests and historical test inputs. No separate WebGL rebuild was needed.

## Upload and activation checklist

- [ ] Upload both ZIPs to `/v2/25/`, preserving the exact staged bytes.
- [ ] Copy those same bytes to `/v2/latest/TinyCore15Wine11.0.zip` and `/v2/latest/TinyCore15Wine11.0-web.zip` (the native Wine-test runner uses the latest full alias).
- [ ] Upload the staged `filesV2.hosted-v25.xml` as `/v2/26R2/filesV2.xml`. Do not upload the smaller bundled native catalog in its place.
- [ ] Run `verify-published.ps1` from the staging directory to download and verify the versioned ZIPs, both latest aliases and hosted XML.
- [ ] Commit/merge and activate repository/UI/Jenkins pins after the downloads are available. Rebuild desired UI distributions and publish the demo site separately.
- [ ] Qualify native functional/Cinebench workers and AbiWord ST/MT/ST JIT/MT JIT with the new inputs. Existing asset and reference bundles remain unchanged.
- [ ] Rebuild/install the desired UI distributions and test clean downloads plus the existing-app path. Prepared resource directories are not finished UI installers.
- [ ] If the optional graphics Jenkins job is used, update and qualify its external worker JSON; repository pins do not update that file.
- [ ] Retain earlier versioned ZIPs and package records for rollback.

The staging directory retains build logs, inventories, comparisons, sidecars,
audio probe source/executable/results, browser runtime identities, UI resource
staging, and `validation-summary.json`. Manual Halo listening on these final
ZIPs has not been repeated; the earlier listening tests used the identical DLL.
The new integration probe tests playback APIs/cursors, rates, pitch and looping,
not sample-by-sample waveform quality. Native audio checks use SDL's dummy sink.

Follow [RELEASING.md](RELEASING.md) for the complete consumer inventory and
reproduction commands. No upload, commit or push was performed for this release.
