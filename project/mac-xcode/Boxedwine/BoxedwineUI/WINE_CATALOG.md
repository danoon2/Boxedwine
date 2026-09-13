# Wine versions from filesV2.xml

The native picker reads the `Wine` entries in `Resources/WindowsSupport/filesV2.xml`, an unchanged snapshot of [the 26R2 release list](https://www.boxedwine.org/v2/26R2/filesV2.xml). Its seven names and package URLs come from that XML. Demos and other entries are ignored by the Wine parser. The list is bundled with each app release, just like the demo catalog; opening a picker never contacts a server.

Add App, Try Another Wine Version, and the library-default chooser use this list. Demos resolve the Wine version required by their recipe through the same list. Choosing files or creating a test copy starts a cancellable download only if the exact package is unavailable locally. Legacy HTTP URLs are upgraded to HTTPS; downloads and redirects stay on boxedwine.org. There is no ZIP-file picker in these flows.

Try Notepad also works in builds without an included Wine ZIP or an imported library default. It reuses the release's default catalog package if available, otherwise displays its size and offers Download & Open. Successful setup pins Notepad to the verified shared package and opens it. Cancel leaves no Notepad entry; subsequent tries open the existing entry using its saved Wine. A configured, valid library-default package retains the original one-click Notepad behavior.

September 13 Notepad download verification: Debug and Release builds without Wine passed their packaging audits, and the 229-test Swift suite passed. In an empty development library, cancelling the size prompt created no app or Wine package. A separate sandboxed Release copy downloaded the actual catalog Wine 11 package, passed its exact size and SHA-256 checks, and opened a rendered Notepad window automatically. Trying Notepad again opened the same entry with the unchanged saved package. The regular library was not used. The unoptimized Debug emulator's cold Wine startup remained slow and was stopped; successful guest-window checks used Release. Evidence: `tmp/native-ui-test/notepad-download/`.

Before continuing, each chooser states whether the selected Wine is available on this Mac or requires a download, with its size. Uninstalled demos show the total download size and a demo/Wine breakdown when Wine is required. Already installed or removed demos show their library state. Sizes describe transferred package bytes, not installed disk use.

Availability checks use the same ZIP, metadata, size and SHA-256 validation as the import resolver. They run off the UI thread and share results across the pickers and demo rows. Import actions stay disabled while a selected package is being checked. Results are cached for unchanged file snapshots and refreshed when the library changes, an operation finishes, a chooser appears, or the app becomes active. Opening these screens makes no network request. If a package disappears or changes after “No Wine download needed” was shown, importing fails with a retry message before any download starts; the next attempt displays the revised size.

`packages.json` supplements the XML with exact byte counts, SHA-256 fingerprints, and the package’s internal filesystem version. It cannot add versions absent from the XML. `FileVersion` identifies the published package revision and is checked against the supplement. It is not always the same as `version.txt` inside the ZIP: Wine 5.0, 4.1, and 3.1 have XML revision 2 and internal filesystem version 7. Keep these separate. The 26R2 Wine 11.0 entry uses revision 11; the older preview’s bundled revision 7 is not reused as that package.

Local candidates come from the shared Wine library, bundled/default Wine, and older private packages saved with active or removed apps. Metadata, exact size, and SHA-256 must match before reuse. Otherwise Boxedwine downloads the catalog build. Downloads receive the same checks and full ZIP validation before any installer or `winecfg` process starts. The result is stored once in `WinePackages/<sha256>.zip`; each app records its exact package reference, which remains pinned after library-default changes or deletion of another app. See [Shared Wine storage](SHARED_WINE.md). An invalid download fails without launching the installer.

Downloaded files are staged under the app sandbox’s temporary directory. Normal completion, cancellation, and failure remove the staging directory. An abrupt process/machine failure during download may leave a temporary file for OS cleanup; it does not create a library entry. Once the app copy begins, the existing import journal, cleanup, backup, and restart-recovery rules apply. The managed shared Wine library provides persistent reuse. Packages are retained while referenced by active/removed apps, the imported default, or ready recovery records; unfinished or unreadable work defers collection.

To update a release:

1. Replace `filesV2.xml` with the release’s published XML.
2. Download each listed Wine package from its official URL into a working directory.
3. Generate the supplement with `python3 Tools/fingerprint-wine-catalog.py Resources/WindowsSupport/filesV2.xml /path/to/downloads Resources/WindowsSupport/packages.json`.
4. Run `BoxedwinePackageCheck --wine-downloads Resources/WindowsSupport /path/to/downloads` to verify every package, and review the resulting XML/fingerprint changes.
5. Build and audit Debug/Release. The normal build also validates the bundled XML/supplement pair.

Existing library entries are preserved. New imports use the catalog-selected package; backups retain their saved Wine. The current build keeps the preview’s previously bundled Wine package for older entries that follow the library default.


The filesV2.xml Wine picker passed 124 executed Swift tests (one real-runtime integration test is opt-in and skipped by default). All seven actual release ZIPs passed the package validator, exact-size checks and SHA-256 checks; the fingerprint-generation tool reproduced the bundled supplement. Debug and Release builds passed their 18-image bundle audits. In an isolated sandboxed native app, Add App showed all seven versions, downloaded Wine 6.0 directly, and saved its exact catalog ZIP. Cancellation reached Windows configuration after the download had completed; the pending choice survived and retry ran the Windows-98-required installer successfully. A Wine 10.0 trial reused a matching library package, created an independent saved copy, and preserved all original Wine 6 app files. Settings displayed the catalog chooser, and cancelling retained the existing default. The regular library’s 150 file hashes and its previously bundled Wine default were unchanged. These checks do not establish game compatibility for every offered Wine version.


The download-disclosure update passed 125 executed Swift tests (126 total, with the real-runtime opt-in test skipped). Regression cases verify that removing, corrupting, or replacing a previously available package with a same-version/same-size build cannot start an undisclosed download or import. Final Debug and Release builds passed their eighteen-image bundle audits. Native UI checks covered missing Wine 11 (166.7 MB), available Wine 6/10, Add App, Wine test copies, the library-default chooser and cancellation, demo totals with required Wine, and Fire Fight’s demo-only 8.5 MB download. The test-copy explanation wraps without clipping. No downloads or installs were needed for these UI checks. All 150 regular-library file hashes and 128 isolated-library file hashes remained unchanged. The updated Debug build was reopened at Add App.

## September 10 Glide package refresh

The publisher replaced the Wine 11 ZIP at the existing catalog URL. Downloaded it again and regenerated `packages.json`: **167,059,472 bytes**, SHA-256 `743b6d30482d7b2b1040b0210fbb2d3b57b2e2c1a0531ed99ed2ff68fd9d8e9e`, Wine 11.0, filesystem 11, catalog FileVersion 11. The other six fingerprints are unchanged; `filesV2.xml` is byte-identical to the current 26R2 source snapshot. Its rounded FileSizeMB is not used for native download estimates.

The archive contains `home/username/.wine/drive_c/windows/system32/glide2x.dll` (896,512 bytes, SHA-256 `4f4d1cf329a5d7a223d9a7ef299ecfc0503f6751d82f9d796a383bc030fa1d26`), matching the tested psVoodoo build. New catalog imports require the updated package; existing apps retain their exact saved Wine. Previews with the previous fingerprint will reject fresh downloads from this changed URL. Use a new URL/revision for subsequent published package changes.

The current Debug build now includes this filesystem-11 package as `WindowsSupport/wine.zip`, and the native helper includes the matching graphics interface. Settings was switched to the included package and verified as Wine 11.0 / filesystem 11. Existing apps pinned to older packages continue to use their saved versions. For subsequent development builds, pass the updated ZIP to `buildNative.sh`; do not reuse the older filesystem-7 test fixture. See [native integration checks](GLIDE_SUPPORT.md#native-filesystem-11-integration).

## September 11 Descent 3 package refresh

Downloaded the publisher's refreshed Wine 11 ZIP from the same URL. Its exact size is **167,059,509 bytes**, SHA-256 `fccc6fc9fe5294eaf9a8b0dca5461c40d577b5e801cfc2cb513c4a15cbfaa715`; Wine 11.0, filesystem 11, and catalog FileVersion 11 are unchanged. The only changed archive entry is `glide2x.dll`, byte-identical to the psVoodoo build with the Descent 3 texture-memory query fix. Regenerating `packages.json` changed only Wine 11's size and checksum; the other six packages and bundled XML are unchanged.

All seven catalog downloads passed `BoxedwinePackageCheck --wine-downloads`. The Debug app was rebuilt with the updated ZIP and passed the eighteen-image bundle audit. At the user's explicit request, the fourteen active library entries referencing the previous Wine 11 build were moved to the new shared package identity. The Wine 9 entry, other app settings, and inactive older references were preserved. The library still selects the included package by default. Descent's matching temporary DLL override was backed up and removed. This was a local development migration, not a change to the product's pinning behavior. Package verification, metadata backups, and migration results are recorded in `tmp/native-ui-test/wine-refresh-20260911/`.


## September 12 F-16 shadow package refresh

The updated Wine 11 filesystem is **167,063,262 bytes**, SHA-256
`de809143d4481f5e6a0dfa1dca4e044fb8f401c0988a172d1c32ffd27649fc69`. Wine version, filesystem ABI, and catalog FileVersion remain
11.0 / 11 / 11. The only changed entry is glide2x.dll, matching the tested
shadow fix from psVoodoo commit `e83947c2f4d800831db0643ad589fba1c98b2457`.
The other six Wine package fingerprints and the bundled XML are unchanged.

The Debug app includes this ZIP and passed the native package/catalog checks and
five-image bundle audit. Its included default and sixteen local Wine 11 app pins
now use the new package. The previous ZIP is retained, and a library backup
precedes the pin migration. This explicit development-library update does not
change the product's per-app pinning behavior. Evidence is in
tmp/native-ui-test/wine-refresh-shadow-20260912/.

### September 13 CNC DDraw refresh

The current Wine 11 filesystem is 167,016,911 bytes, SHA-256
`a3367c4e977dbbe0295ce3b4b102bf7b0baa1278ea121cefe6065d561b960452`. Wine, filesystem, and catalog file versions remain 11.0/11/11. Only `C:/ddraw/ddraw.dll` changed from the September 12 shadow-fix package. It matches the tested CNC OpenGL loader fix (343,552 bytes, SHA-256 `318406915f4d11b08321949026626253554d7ae780d9aa3541cbfe5dcedbddf2`). The psVoodoo DLL is unchanged. The Norse INI profile is not in this upload; see [Demo coverage](DEMO_COVERAGE.md).

The rebuilt Debug app passed the five-image bundle audit and all 216 Swift tests. At the user’s request, 28 local Wine 11 apps now reference this shared package; the two Wine 9 apps are unchanged. Previous packages and the library metadata backup were retained. MDK rendered using its automatic renderer and uncapped settings, then exited with code 0. Norse rendered its menu with the shared DLL and existing private INI, then stopped through the native UI with exit code 0. Its identical temporary DLL was backed up and removed. Evidence: `tmp/native-ui-test/wine-refresh-cnc-20260913/`.
