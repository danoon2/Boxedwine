# Wine filesystem patch and release checklist

Use this checklist when shipping a Wine patch in the regular and web filesystems.
It covers the archives and their consumers; adding a patch to `wine_builds.json`
or installing a loose file in one application's root does not complete a release.
The [builder guide](README.md) explains the build internals and prerequisites.

Review the release plan before starting builds. Commands below are for the later
execution steps, run from the repository root in WSL unless noted otherwise.
The inventory was updated for filesystem 14 on October 4, 2026. Resolve the
currently selected profiles and pins again for each release.

## 1. Agree on the release scope

- [ ] Identify the patch, affected Wine versions, reproducer, and expected result.
- [ ] Choose the filesystem revision and publication URLs. For a new public
  release, use a new revision and URL so existing hash-pinned installations can
  still download their original bytes. An unpublished candidate can be rebuilt
  under its candidate revision.
- [ ] Keep the Wine version separate from the filesystem revision: a patch to
  Wine 11.0 still has `wineVersion.txt = 11.0`; the filesystem's `version.txt`,
  package `fileVersion`, and package `filesystemVersion` identify the new package.
- [ ] Include both `TinyCore15Wine11.0.zip` and `TinyCore15Wine11.0-web.zip`.
  They have different builds, sizes, hashes, and smoke tests.
- [ ] Decide which automation and UI releases move to the new filesystem.
  Record anything intentionally left on an older input, with a reason.
- [ ] Define the required game/probe, UI, browser, and native automation checks.
  Record the tested Boxedwine binary identities as well as the Wine ZIP identities.

For the SimTower cursor fix, the patch is to `server/queue.c` and applies to both
variants. It does not change the separate WebGL PE DLLs, so their patch-series
and DLL hashes should stay unchanged. The installed SimTower `wineserver` override
is a local workaround; release testing must use the candidate ZIP in a fresh root.

## 2. Update the reproducible inputs

| File | What to update |
| --- | --- |
| [`patches/`](patches/) and [`wine_builds.json`](wine_builds.json) | Add the patch and its version-bounded operation. Both variants derive their Wine build from this configuration. |
| [`filesystem_wine11.json`](filesystem_wine11.json) | Set `filesystem_version` and select the release's `webgl_config`. Change `wine_tag`/`wine_commit` only when changing upstream Wine. |
| [`changes_wine11.txt`](changes_wine11.txt) | Add the release entry; its first line must start with the configured `vN`. Preserve previous entries. |
| [`changes_wine11_web.txt`](changes_wine11_web.txt) | Update the web release entry to the same `vN`. It is prepended to the full history. Update size-savings claims only from new measurements. |
| Release's `webgl_filesystems_vN.json` | Start from the current config, such as [`webgl_filesystems_v14.json`](webgl_filesystems_v14.json), keeping older configs as historical pins. Update identity/date, `full-vN` and `web-vN` profiles, filenames and revision. Fill final archive sizes/hashes after packaging. |

- [ ] If the patch also affects the separately built `C:/webgl` DLLs, update the
  ordered series under [`../d3dToWebGL/`](../d3dToWebGL/README.md), its versioned
  `../wineTests/webgl-test-divergences-v*.json`, the config's `patch_manifest`
  path/hash, and the rebuilt DLL sizes/hashes. Updating the main Wine ELF module
  alone does not update those PE DLLs. See the
  [Daytona example](../../docs/daytona-wine11-gdi.md#separate-webgl-pe-dll).
- [ ] If addons, the TinyCore base, or web pruning change, update their source
  pins, patches, notices/source archives, and [`web_runtime_policy.json`](web_runtime_policy.json)
  as applicable. A cursor-only patch does not require changing these inputs.

The two final ZIP hashes are output identities, not substitutes for checking
the source patches and individual DLLs. Do not change unrelated expected hashes
just to make validation pass.

## 3. Build and validate both candidates

After the checklist review, choose dedicated WSL-native work directories. This
example uses revision 14; replace it with the agreed revision before running:

```bash
release_revision=14
release_main="$HOME/boxedwine-wine11-fs${release_revision}-main"
release_web="$HOME/boxedwine-wine11-fs${release_revision}-web"
release_config="tools/buildWine/webgl_filesystems_v${release_revision}.json"
release_stage="tmp/filesystem-v${release_revision}-release"

python3 tools/buildWine/build_filesystem.py \
  --profile tools/buildWine/filesystem_wine11.json \
  --variant main --work-dir "$release_main" --jobs 12
python3 tools/buildWine/build_filesystem.py \
  --profile tools/buildWine/filesystem_wine11.json \
  --variant web --work-dir "$release_web" --jobs 12
```

- [ ] Keep the main and web work directories separate and off `/mnt/c`. Retain
  `--variant web` when retrying web phases. A changed Wine patch requires rebuilding
  Wine; `--phase assemble` alone can reuse stale binaries.
- [ ] Check each `filesystem-build-result.json`, `filesystem-inventory.json`,
  `filesystem-smoke.json`, and build log. Retain the web variant's
  `web-runtime-validation.json`. The main smoke test includes Gecko; web does not.
- [ ] Inspect each ZIP's `wineVersion.txt`, `version.txt`, `changes.txt`, and
  `filesystem-build.json`. Confirm the new patch hash is recorded and the actual
  affected binary is present. Preserve explicit empty directories, especially
  the writable Wine Temp directories, and ensure ZIP CRC validation passes.
- [ ] Ensure the web ZIP is below **100,000,000 bytes** and passes its pruning
  policy. Check the main/web inventories for unintended additions or removals.
- [ ] Run the reproducer and affected game against the packaged binaries in
  fresh Boxedwine roots, without loose overrides. Test both candidate variants;
  include the browser path when validating web behavior. Record any unrun checks.
- [ ] Run the relevant builder checks and patch-specific regressions. The
  builder's unit checks are available with:

```bash
python3 -m unittest discover -s tools/buildWine -p 'test_build*.py'
python3 -m unittest discover -s tools/buildWine -p 'test_web*.py'
```

Copy the exact tested ZIPs and their evidence into a release staging directory.
Once staged, treat those bytes as final: any repack changes the archive hash and
requires regenerating the pins and repeating the checks affected by the change.

## 4. Set final archive identities

- [ ] Read size and SHA-256 from each build result, independently hash the staged
  ZIPs, and enter those values in the release config's `full-vN` and `web-vN`
  profiles. Retain unchanged DLL/GL hashes and renderer expectations.
- [ ] Validate each final ZIP against its exact profile and write its sidecar:

```bash
python3 tools/buildWine/webgl_filesystem.py validate \
  --config "$release_config" --profile "full-v${release_revision}" \
  --filesystem "$release_stage/TinyCore15Wine11.0.zip" --write-sidecar
python3 tools/buildWine/webgl_filesystem.py validate \
  --config "$release_config" --profile "web-v${release_revision}" \
  --filesystem "$release_stage/TinyCore15Wine11.0-web.zip" --write-sidecar
```

Always supply `--config`; the validator's default is a historical configuration.
`validate-set` is also a historical v3/v10 comparison, not the validator for the
current main/web pair. Profile validation checks hashes, CRCs, DLLs, GL libraries,
links, cache, and registry expectations; inspect packaged version text separately.

## 5. Update native UI and catalog consumers

The shared native Wine catalog is under
[`project/mac-xcode/Boxedwine/BoxedwineUI/Resources/WindowsSupport`](../../project/mac-xcode/Boxedwine/BoxedwineUI/Resources/WindowsSupport).
Windows links those resources into its build; Linux stages `packages.json` from
the same location. Do not edit generated `bin/`, `Build/`, or `artifacts/` copies.

| File or consumer | Required review/update |
| --- | --- |
| [`packages.json`](../../project/mac-xcode/Boxedwine/BoxedwineUI/Resources/WindowsSupport/packages.json) | Set the final package URL key, exact `bytes`, `sha256`, `fileVersion`, and packaged `filesystemVersion`, using the **main ZIP**, not the web ZIP. |
| Mac, Windows, and Linux UI distributions | Rebuild/stage their resources when shipping the updated catalog. Test package selection, clean download, verification, setup, and launch against the candidate. Existing applications retain their pinned package records. |
| [`resources/demo-catalog.lock.json`](../../resources/demo-catalog.lock.json) | Conditional: update only if demo recipes/icons or their compatibility requirements change. This is a separate package from the Wine catalog; see the [catalog release procedure](../../resources/DEMO_CATALOG.md). |
| Release documentation and provenance | Update current version/download descriptions and new validation records. Review applicable licensing/source records when components change; historical inventories and earlier test results remain evidence of their original artifacts. |

- [ ] Verify the package URL and each fingerprint in `packages.json` against the
  final main ZIP. Check `bytes` and `sha256` independently, and confirm
  `fileVersion` and `filesystemVersion` match its `version.txt`.
- [ ] Keep `wineVersion.txt` at `11.0` for a patch-only release. Confirm the UI
  identifies the package as Wine 11.0 with the new filesystem revision.
- [ ] Check that each distributed UI contains the intended `packages.json`
  records and can download and validate those exact archives.

Platform-specific build and smoke commands are in the
[Windows UI guide](../../project/msvc/BoxedwineUI/README.md) and
[Linux UI guide](../../project/linux/ui/README.md). Record checks actually run;
do not rewrite old validation reports as though they tested the new ZIP.

## 6. Update web demo and automation consumers

| File or input | Required review/update |
| --- | --- |
| [`../jenkins/local-build-site.sh`](../jenkins/local-build-site.sh) and [`../jenkins/publish-build-site.sh`](../jenkins/publish-build-site.sh) | Advance the default `BOXEDWINE_ZIP_URL` to the **web ZIP** and `DEMO_ROOT_CONFIG` to the new versioned manifest. |
| [`../jenkins/build_site.py`](../jenkins/build_site.py) | Advance `DEFAULT_DEMO_ROOT_CONFIG`. ZIP filenames normally remain unchanged; update filename constants/migrations only if intentionally renaming them. |
| Site `demos/apps/demos.json` and downloaded roots | Check every referenced root against the selected manifest. Ensure the downloaded web ZIP is the final candidate. Existing full/old root selections have migration logic; validate the resulting selections. |
| [`../../Jenkinsfile`](../../Jenkinsfile), Emscripten AbiWord stage | Update `BOXEDWINE_AUTO_URL` and `BOXEDWINE_AUTO_SHA256` together to the final **web ZIP**. The downloaded local name is `boxedwine.zip`. |
| AbiWord overlay | Review `ABIWORD_AUTO_URL`/`ABIWORD_AUTO_SHA256` only if application scripts or screenshot references need changes. Requalify all four browser modes with the new filesystem even when the overlay stays byte-identical. |
| [`../BoxedWineRunner/filesystem.properties`](../BoxedWineRunner/filesystem.properties) | Update `url`, `sha256`, and `size` together to the final **full ZIP**. Jenkins distributes this pin with the runner to Linux x64/ARM64, Mac ARM, and Windows x64/ARM64. Each worker downloads/verifies the filesystem separately. |
| Native automation assets | `automation34.zip` contains no filesystem. Keep the asset bundle unchanged for filesystem-only Jenkins releases; update the checkout's pin instead. When publishing a successor bundle, use [`package_assets.py`](../BoxedWineRunner/package_assets.py) to preserve apps, scripts, captures, and performance scripts, refresh the bundled runner/launchers/pin, and verify retained content. Update every native download/extract filename in `Jenkinsfile` together. |

**Preserve the native automation presets.** The runner's `PrepareFilesystem`
reads `home/username/.wine/user.reg` from the pinned full ZIP and stages a separate
registry with these existing automation32 settings:

```text
[Software\\Wine\\Direct3D]
"VideoMemorySize"="256"
"DirectDrawRenderer"="gdi"
"renderer"="gdi"
```

The old bundle also explicitly selected `MouseWarpOverride=enable`, which is
Wine 11's default behavior. No extra mouse preset is needed; preserve the new
filesystem's input settings.

The full ZIP is staged byte-for-byte as `fs/fs.zip`; its published hash remains
valid. The runner's `-user-reg fs/user.reg` option seeds each writable test root
on every attempt. Other registry values come from the new candidate, not the old
bundle. Record the filesystem pin and runner commit with the results. Jenkins
supplies the emulator and rebuilds the [Java runner](../BoxedWineRunner/README.md)
from the checkout. Cache entries are addressed by SHA-256 and reverified on use;
failed downloads or mismatches stop the job rather than using the embedded ZIP.

- [ ] Run the Java runner regression tests before qualifying the new filesystem.
  Review any preset change explicitly. When first migrating from automation32's
  embedded web root to the full root, compare functional captures and Cinebench
  results even though the Wine version is unchanged.

- [ ] Qualify native functional and Cinebench automation on the intended workers.
  Keep previous results and explain performance-baseline changes. Review actual
  screenshot differences before updating references.
- [ ] Qualify AbiWord in ST, MT, ST JIT, and MT JIT. The current Jenkins workflow
  runs them serially on its shared worker.
- [ ] Validate demo roots before generating or publishing the site:

```bash
python3 tools/jenkins/build_site.py \
  --site-dir /path/to/staged-site --demo-source /path/to/staged-site/demos/apps \
  --demo-root-config "$release_config" --validate-demo-roots-only
```

The [Jenkins guide](../jenkins/instructions.md) documents local candidate URL and
manifest overrides. Preserve historical published builds and their retained ZIPs;
generate new `graphics-builds.json` identities for newly generated builds.

## 7. Review separately pinned test suites

- [ ] [`../wineTests/runWineTests.py`](../wineTests/runWineTests.py) has its own
  `FILESYSTEM_URL` and `FILESYSTEM_CACHE_NAME` (v10 at this audit). Decide whether
  that suite moves to the release. If it does, update both defaults, corresponding
  tests/docs, and use a fresh cache. A nonempty old cache can otherwise be reused
  even with `--filesystem-url` set to a new URL.
- [ ] Graphics qualification uses explicit filesystem, runtime, test-executable,
  divergence-manifest, and baseline identities. Use the
  [graphics test guide](../wineTests/GRAPHICS_README.md) and
  [graphics CI instructions](../jenkins/instructions.md#wine-graphics-ci).
  Prepare a new reviewed input set for the candidate rather than overwriting
  historical baseline files. Preserve expected counts unless reviewed evidence
  warrants a change.
- [ ] The optional graphics Jenkins job reads a worker JSON file selected by
  `BOXEDWINE_GRAPHICS_CONFIG`; changing repository pins does not update that
  external configuration. Update its inputs and baseline/test-archive hashes
  when that job is included in the release qualification.

## 8. Publish, verify consumers, and retain rollback inputs

- [ ] Finish candidate validation and retain the exact tested ZIPs, patch/source
  identities, inventories, sidecars, logs, and automation receipts.
- [ ] Publish the main/web ZIPs and any changed automation/catalog bundles to
  their agreed versioned URLs **before activating consumer pins that require
  them**. Publishing a build site is a separate step from uploading a filesystem.
- [ ] Download each newly hosted artifact into a fresh location and verify its
  exact byte count and SHA-256 against the staged artifact. Revalidate the roots.
- [ ] Activate the reviewed repository/UI/Jenkins pins and any separately hosted
  catalog or worker configuration. Rebuild the UI packages that embed new pins.
- [ ] Test a clean UI download and browser load, plus the supported existing-app
  path. Do not silently replace installed applications' hash-pinned Wine records.
- [ ] Retain previous published ZIPs, UI package records, automation bundles, and
  test inputs for rollback. Roll back the corresponding consumer pins together.
- [ ] Summarize shipped identities, platforms/modes actually tested, skipped checks,
  and whether a user must install/select a new package to receive the patch.

Before shipping, search tracked files for the previous hashes, URLs, and manifest
name. Classify each result as an active pin, an intentionally fixed test input,
or historical evidence; do not perform a global version/hash replacement:

```bash
git grep -n -F -e '<old-main-sha256>' -e '<old-web-sha256>' \
  -e 'v2/13/' -e 'webgl_filesystems_v13.json'
git grep -n -E 'automation[0-9]+\.zip|FILESYSTEM_URL|FILESYSTEM_CACHE_NAME|BOXEDWINE_AUTO_|ABIWORD_AUTO_|DEMO_ROOT_CONFIG'
git diff --check
```

Adjust the old revision and manifest in that search for the release being
replaced. Untracked staging receipts and bundles, public catalog files, and
worker-local graphics configurations require their own review.
