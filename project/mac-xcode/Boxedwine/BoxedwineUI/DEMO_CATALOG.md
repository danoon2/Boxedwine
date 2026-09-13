# Bundled demo catalog

The native app embeds `Contents/Resources/Demos/catalog.xml` and its PNG icons from the versioned ZIP pinned by the shared `resources/demo-catalog.lock.json`. These assets are fetched and cached at build time, with no catalog fallback in Git. Local offline builds without the exact cache omit demos; Jenkins requires them. See [Shared catalog build and publishing](../../../../resources/DEMO_CATALOG.md). Opening Demos never fetches a catalog or artwork. A release always shows the same list; changing it requires a new app build. Download & Install fetches the chosen payload and, if needed, its recipe’s Wine version from the bundled filesV2.xml Wine list. Both downloads are verified against exact byte counts and SHA-256 fingerprints before preparing an independent library entry pinned to a shared Wine package. See [Wine catalog](WINE_CATALOG.md).

This keeps catalog maintenance in a data file, rather than Swift source. The original OpenGL UI and its release-specific remote XML remain unchanged. The native format retains familiar fields but adds required identity and integrity metadata; it is not a drop-in parser for the complete legacy catalog.

## Current selection

Initial shared release `26R2-catalog-1` includes **36 entries**: seventeen portable ZIPs and nineteen installers. Thirty-three use Wine 11.0 and three use Wine 9.0. See [Demo coverage](DEMO_COVERAGE.md) for each recipe, verification evidence and the two remaining entries.

The entries and artwork come from the existing [26R2 catalog](https://www.boxedwine.org/v2/26R2/filesV2.xml). Exact payload sizes and fingerprints are recorded in the bundled XML. Thirty-five PNG icons are bundled unchanged from the legacy URLs; Cities in Motion adds a PNG extracted from the installed demo executable because the legacy icon URL was unavailable. It uses the largest embedded representation (48×48) with its original alpha channel, bringing the bundled total to 36. This records their source, not a determination of redistribution rights.

The current expansion translates fifteen legacy recipes with known native equivalents. Mac-specific legacy options override generic options when present. Windows versions are applied and verified with winecfg before setup; GDI values seed the private environment during import; launch-only display, cursor, CPU and graphics options are kept off the installer command. Caesar 3 starts both setup and game at 800×600, translating its former explanatory prose explicitly. Its GDI setting is prepared before setup too. Unknown legacy strings are never executed or silently discarded.

Solitaire DotNet still needs Mono. 3DMark’s unexplained `Debug` option remains unresolved. Java Solitaire and FreeCol use [automatic Java setup](JAVA_SUPPORT.md), with tested Java 8/17 choices and FreeCol’s VM memory option. Catalog schema 3 adds typed `JavaVersion` and `JavaArguments` fields for these JAR recipes.

Descent 3 demo 2, Motorhead, and F-16 Multirole Fighter use schema 4’s `Glide` field, whose only supported value is `psVoodoo`. All three select Wine 11.0. Import verifies filesystem 11 and a preinstalled `C:/windows/system32/glide2x.dll` before fetching the demo, then saves `WINEDLLOVERRIDES=d3d9=b` and `WINE_D3D_CONFIG=renderer=gl` as ordinary Advanced Boxedwine arguments. These apply to setup and the game and remain editable. The DLL stays in the shared Wine ZIP; no separate Glide installer or per-app DLL copy is needed. Motorhead also applies Windows 98 through winecfg before setup and starts the game at 640×480. Descent’s help explains how to select 3dfx during video detection. F-16 explicitly disables the old GDI-only recipe setting: it detects the installed Glide wrapper, which needs WineD3D 3D support. Existing installations can use the same two Advanced environment options without reinstalling or replacing the DLL.

The September 12 Wine 11 refresh is 167,063,262 bytes, SHA-256 `de809143d4481f5e6a0dfa1dca4e044fb8f401c0988a172d1c32ffd27649fc69`. Its preinstalled psVoodoo DLL includes the Descent 3 texture-memory query fix and the F-16 framebuffer, clipping, terrain-detail, and camera-turn shadow fixes. The packaged DLL is byte-identical to the final F-16-tested build. Catalog updates preserve installed apps' exact pins; this development library's sixteen apps using the previous Wine 11 build were explicitly migrated at the user's request. Schema 1–3 remain readable; schema 4 retains their fields. Glide presets reuse library/backup/recovery formats 9/5/6 through the existing Advanced argument support.

The September 13 refresh is 167,016,911 bytes, SHA-256 `a3367c4e977dbbe0295ce3b4b102bf7b0baa1278ea121cefe6065d561b960452`. Only `C:/ddraw/ddraw.dll` changed: it now includes the CNC OpenGL loader fix. Catalog release 8 removes MDK’s GDI override while retaining uncapped timing. The shared download intentionally remains unchanged for game-specific settings. Catalog release 9 uses a 32-bit desktop for Norse and seeds `[NORSE95]` / `fake_mode=320x240x16` in its private `C:/ddraw/ddraw.ini` during installation. The installed development copy has its tested CNC configuration restored.

**Runtime integration:** the native helper now has the filesystem-11 graphics ABI, and the Debug app includes the refreshed Wine 11 package. Its helper rendered Motorhead’s race in an isolated smoke check using the preinstalled DLL; the full earlier user playtest used the separate runtime. Descent 3's 3dfx startup fix was confirmed by the user in the regular native UI. Normal shutdown through the new helper and extended stability remain separate checks. See [Glide support](GLIDE_SUPPORT.md#native-filesystem-11-integration) for evidence and limits.

## Editing a release

1. Copy the resolved catalog into an editing directory outside Git, then edit `catalog.xml` and place each icon alongside it. Use the [shared packaging tool](../../../../resources/DEMO_CATALOG.md#publishing-a-catalog-update) to publish the ZIP and update the project lock. Update the root `release` label for a new catalog release; preserve IDs for the same demo across releases. A different application/version that should coexist needs a different ID.
2. Download the exact proposed payload for inspection. Record its size in bytes and lowercase SHA-256 (`wc -c < payload.zip` and `shasum -a 256 payload.zip`). Keep published payload URLs immutable too: changed bytes at an old URL make old releases reject the download.
3. Test download, installation, program choice, launch and normal close with the specified Wine package. Do not infer compatibility from an installer's exit status. Check help text, artwork and redistribution materials.
4. Run the catalog validator and Swift tests, then build the native app. The native bundle preparation step fetches or reuses the pinned ZIP, then validates the XML and icons before signing the app.

```sh
swift run --package-path project/mac-xcode/Boxedwine/BoxedwineUI \
  BoxedwinePackageCheck --catalog tmp/catalog-edit/catalog.xml
BOXEDWINE_TEST_DEMO_CATALOG="$PWD/tmp/catalog-edit" \
  swift test --package-path project/mac-xcode/Boxedwine/BoxedwineUI
```

The current schema is `<XML schemaVersion="7" release="26R2-catalog-1">` containing one or more `<Demo>` entries. It accepts UTF-8 XML, at most 1 MiB and 256 entries, with no document type or external entities. Fields cannot be duplicated, and unknown fields/attributes are rejected.

| Field | Meaning |
| --- | --- |
| `ID` | Required stable ID: 1–64 lowercase letters, digits or hyphens; starts with a letter or digit. Unique within the catalog. |
| `Name` | Required display name, up to 256 UTF-8 bytes. |
| `Summary` | Optional short description shown on the card. |
| `Help` | Optional plain text in About this demo. Actual newlines and legacy literal `\n`/`\t` escapes are supported. |
| `Icon` | Optional local PNG basename, including `.png`; omit or leave empty for the native fallback symbol. No URL or directory path. Build validation checks supplied PNG signatures and the 1 MiB size limit. |
| `FileURL` | Required HTTPS URL on boxedwine.org or www.boxedwine.org, without embedded credentials or a fragment. Only the default/443 port is accepted. |
| `FileSizeBytes` | Required exact positive byte count, up to 1 GiB. Not the legacy rounded `Size` field. |
| `FileSHA256` | Required 64-character lowercase hexadecimal SHA-256 of the complete download. |
| `InstallType` | Required `Zip` for a portable ZIP, or `Installer` for a standalone EXE/MSI or a ZIP containing setup media. |
| `InstallExe` | Required only for an installer ZIP: relative EXE/MSI path within the extracted media. Omit for other recipes. |
| `ShortcutExe` | Required program basename (EXE, or JAR for a Java recipe). Portable imports require exactly one match. After a normal installer exit, a unique case-insensitive match is selected and saved automatically; missing or ambiguous matches open the chooser. |
| `WineVersion` | Required exact Wine version string from the validated support ZIP. |

Schema 2 adds these optional typed fields. Schema 1 remains readable but cannot contain them. Duplicate fields, invalid enums/numbers, empty typed values, and unsupported values fail validation.

| Field | Meaning |
| --- | --- |
| `WindowsVersion` | `win98` or `winxp`; saves a pending per-app choice, applied and verified with winecfg before the first installer or app launch. |
| `GDIRenderer` | `true` or `false`; seeds both DirectDrawRenderer and renderer registry values. |
| `Resolution` | Initial game resolution as `WIDTHxHEIGHT`, with each dimension 320–8192. The user can change it later in App Settings. |
| `InstallResolution` | Setup resolution, independent of the game; defaults to 1024×768 for configured recipes. |
| `BitsPerPixel` | Game color depth: 8, 16 or 32. |
| `CPUCount` | Runtime `-cpuAffinity` value, 1–64. |
| `NativeOpenGL` | Only `true` is supported: uses the native runtime’s default GL backend without an OSMesa override. |
| `CNCDDraw` | Boolean; enables the runtime’s CNC DDraw override in the selected program’s directory. Its DLL must be present in the assigned Wine package. |
| `DisableHideCursor` | Boolean; keeps the game cursor visible. |
| `ForceRelativeMouse` | Boolean; enables relative mouse input for the game. |

`Options`, `InstallOptions`, `Options_Mac` and `InstallOptions_Mac` must still be empty. Native fields are finite settings, not forwarded command strings. No shell or arbitrary scripts are used.

Schema 5 adds `CNCDDrawRenderer` (`gdi`, `opengl`, or `direct3d9`), requiring `CNCDDraw=true`. Import copies the selected Wine package’s `C:/ddraw/ddraw.ini` into the new app’s private root and changes only the global `renderer` setting, retaining other settings and compatibility sections. Missing or ambiguous settings fail the import with normal cleanup. This is CNC DDraw’s presentation backend, independent of the Wine renderer picker. MDK now uses the shared Wine filesystem’s automatic renderer with the corrected CNC DDraw loader; see the MDK recheck in [Demo coverage](DEMO_COVERAGE.md). The INI travels as an ordinary app file through backup, recovery, removal and Wine test copies; no new library metadata format or launch argument is needed. Schemas 1–4 remain readable and do not accept this new field.

Schema 6 adds `CNCDDrawUncapped` (`true` or `false`), requiring `CNCDDraw=true`. When true, import seeds `maxfps=0`, `vsync=false`, and `maxgameticks=-1` in the app’s private INI. Both timing limits must be disabled: CNC’s default `maxgameticks=0` still emulates 60 Hz vertical blank even with `maxfps=0`. Only MDK Performance uses this option. Omitted or false leaves the package’s timing settings alone, and renderer selection stays independent. Duplicate targeted INI keys reject the import. Existing app files are not rewritten. Schemas 1–5 remain readable and reject this new field.

Schema 7 adds `CNCDDrawFakeMode` (`WIDTHxHEIGHTxBPP`), requiring `CNCDDraw=true` and an EXE shortcut. Dimensions must be positive and at most 8192; color depth must be 8, 16, or 32. The importer copies the package's default `C:/ddraw/ddraw.ini` into the app's private files and sets `fake_mode` only in the shortcut's executable section (basename without `.exe`). Existing settings, line endings, other game sections, and the shared Wine ZIP remain intact. Duplicate target sections or keys reject the import. Norse uses `320x240x16` with `BitsPerPixel=32` to retain CNC's scaling and OpenGL presentation. These are ordinary app files, so backup, restore, recovery, and Wine test copies retain them without a library format change. Existing installed apps are not rewritten by a catalog update. Schemas 1–6 remain readable and reject this new field.

## Installation and persistence

The app resolves the recipe’s Wine version through the bundled filesV2.xml list. It reuses a local package only when its exact size, SHA-256, and metadata match the catalog; otherwise Download & Install downloads the required Wine automatically. Both the Wine package and demo payload are verified before setup. Each demo records an exact shared Wine reference without changing the library default. Identical packages occupy storage once; Windows files and saves remain separate. See [Shared Wine storage](SHARED_WINE.md).

Before Download & Install is enabled, the card shows the total transfer size, adding the required Wine package when it is missing and showing a breakdown. If an exact local package is available, only the demo size is counted and the card says no Wine download is needed. A removed or changed local package cannot silently trigger a Wine download after that notice: the operation stops and asks the user to retry after reviewing the updated size. Installed and removed demos show their library state instead of a new-download estimate.

Downloads use an ephemeral URLSession without shared cookies, credentials or cache. At most three redirects are accepted, all within the same allowed HTTPS host set. Transfers have size and timeout bounds and support cancellation even while awaiting data. Incomplete or changed downloads never become installed apps. Checksums pin the release's bytes; they do not establish software safety or compatibility.

Portable ZIPs extract into the new app's private `C:/App`. Installer ZIPs retain their complete media tree and relative setup path; raw EXE/MSI downloads are saved as private setup media under their original URL filenames. Renaming a self-extractor to `setup.exe` can break its nested installer (Motorhead's Win16 setup reports an environment allocation error). Filenames must be single safe path components. ZIP extraction checks entry counts, decompressed sizes and CRCs, and rejects traversal, links, special files, encryption, unsupported compression and conflicting names. Limits are 100,000 entries, 2 GiB expanded total, 512 MiB per file and 128 directory levels. Only stored and Deflate entries are supported.

An installer opens after the new entry commits. After a normal runtime exit, the launcher scans the app's Windows files for the catalog's `ShortcutExe`, selects a unique case-insensitive filename match, and saves it without showing the chooser. It retains the demo's library name and all other settings. Re-running setup preserves an existing valid program choice. A missing or ambiguous match, a stopped installer, or an abnormal runtime exit opens the chooser; Run Installer Again remains available. The scan excludes Wine system programs, staged setup media, and symbolic links. A normal runtime exit and a matching file do not prove complete installation or game compatibility. Manual imports and explicit Choose Program actions retain the chooser. Selecting a demo program does not launch it automatically. Portable demos remain ready to open after import.

The existing import journal covers transfer, extraction and the Wine copy. Normal cancellation removes only the uncommitted new app; interrupted work is surfaced through Unfinished Work. Completed entries record their demo ID, catalog release, payload hash, suggested executable and any typed settings. Configured entries require library format 5, backup format 2, and completed-operation format 2; a general Windows-version preference requires formats 6/3/3. Earlier launchers reject newer formats instead of dropping settings. Older formats remain readable. New demo Windows-version choices use the shared per-app field and pending flag; older recipes prefill App Settings without rewriting metadata. Only GDI settings use registry templates during import. Windows-version choices are applied and verified by a hidden winecfg runtime before the first installer or app launch, and after later changes in App Settings; ordinary launches preserve later app/winecfg edits. See [Windows compatibility](WINDOWS_COMPATIBILITY.md). Unrelated registry values remain intact. Backups and Wine test copies preserve that provenance. An active matching ID shows Show in Library; a removed one shows View in Removed Apps. App updates do not reinstall, overwrite or automatically upgrade existing entries.

## Distribution

This is a local preview feature. A bundled list does not settle App Store rules for downloaded guest software, redistribution permissions, or the separate Wine/support-package questions. The draft App Review inquiry in [Release preparation](RELEASE.md) explicitly includes the catalog. No catalog has been published and no review inquiry has been sent by this implementation.
