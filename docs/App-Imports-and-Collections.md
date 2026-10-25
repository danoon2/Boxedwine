# App imports and collections

The native Mac, Windows, and Linux launchers use **Add to Library**:

1. Choose **App or installer**, **Collection**, or **Restore a backup**.
2. Choose a file, folder, or direct download URL. Collections accept local or downloaded JSON. Restore uses an existing `.boxedwinebackup` package/folder.
3. Review the destination and action. A ZIP is extracted into temporary storage before review. Choose the app program or installer; supporting files are copied together. Compatibility settings are optional. Nothing runs during inspection.

App downloads support direct ZIP, EXE, and MSI links, up to 1 GiB. Collection JSON is limited to 1 MiB. Redirects cannot switch from HTTPS to HTTP. Downloads and extraction can be cancelled; failed or cancelled preparation removes temporary files. The Mac App Store build supports collection imports from files and URLs, remote icons, refresh, and installing apps from imported listings using its included Wine. It does not bundle a demo catalog or download additional Wine packages; direct app/installer URL imports remain unavailable there.

Invalid links, failed downloads, and invalid source files show an error inside the source form. The entered URL stays available to edit and retry; editing the URL or starting another attempt clears the previous error. Cancelling a download also returns to the source form without discarding the URL.

## Collection JSON

See [the example](../resources/examples/collection.json). The example URL, size, and checksum are placeholders; replace them with an actual package before importing for installation.

The format is shared by all three launchers:

- `schemaVersion`: `1`.
- `folder`: the proposed top-level collection folder, including `Demos` or an existing folder. The user can change this during review.
- `apps`: 1–256 entries using the existing demo recipe field names. All recipe values are **strings**, including sizes and boolean settings.

Required entry fields are `ID`, `Name`, `FileURL`, `FileSizeBytes`, `FileSHA256`, `InstallType`, and `ShortcutExe`. `WineVersion` is optional: when omitted, installation selects the highest numeric Wine version in the app's current Wine catalog, downloading its package if needed. For example, a collection without this field will select Wine 12 when a future app release offers it, without editing or reimporting the JSON. The library default does not override this choice. An explicit value such as `"WineVersion": "11.0"` requires that exact version; empty strings and `null` are invalid. The omission stays in saved collection metadata, while each installed app remains pinned to the package actually used for its installation. Existing installed apps do not automatically switch Wine versions.

`Summary` and `Help` supply the listing text. `Icon` accepts an existing bundled PNG filename or a complete HTTP/HTTPS URL to a PNG (for example, `"Icon": "https://example.com/icons/game.png"`). Remote icons load in the background when saved listings are loaded, including after relaunch. Each image must be at most 1 MiB and 1024×1024 pixels. Downloads have a 20-second timeout, bounded redirects, no cookies or saved credentials, and no HTTPS-to-HTTP redirects. Missing, invalid, or unavailable images use the normal fallback and never prevent importing or using a collection.

Remote icons are cached under `CollectionIcons` beside `collections.json`, keyed by the complete URL, and reused offline. Change the icon URL (or add a version query such as `?v=2`) when publishing new artwork; a changed URL gets a new cached image. A failed download is retried on the next launch. Deleting a collection retains this shared image cache. Existing demo compatibility fields are supported, including Windows version, renderer, display size, CNC DDraw, Glide, and validated DLL overrides. Shell commands and legacy `Options` fields are not accepted.

`AppArguments` optionally supplies the Windows app's launch arguments as a string with one argument per line, matching App Settings. For Quake II's timed demo, use `"AppArguments": "+timedemo\n1\n+map\ndemo1.dm2"`. Spaces within an argument are preserved; do not add shell quotes. LF and CRLF separators are accepted, and blank lines represent empty arguments. An omitted or empty string supplies no arguments. The limits are 256 arguments, 8192 UTF-8 bytes per argument, and 65536 bytes total; NUL and bare carriage returns are rejected. These values seed the editable app settings on first installation and are passed literally after the Windows executable, never to the installer or emulator. Refreshing a listing does not overwrite an installed app's arguments.

`InstallType` is `Zip` for a ready-to-run ZIP, `Installer` for an EXE, MSI, or ZIP containing setup, or `Disc` for a ZIP containing installation ISOs. Installer ZIPs also specify `InstallExe`, a relative path within the archive. `ShortcutExe` names the program to launch after installation. Packages require their exact byte size and SHA-256 checksum, including when hosted outside boxedwine.org.

### Disc collections

All three native launchers support `InstallType: "Disc"` in custom collections. Set `FileURL` to the ZIP and `CDROMImages` to the ordered ISO paths inside it, one per line. For example, these fields describe a ZIP with two images at its root:

```json
"InstallType": "Disc",
"CDROMImages": "Robot City - Disc 1.iso\nRobot City - Disc 2.iso",
"ShortcutExe": "ROBOTCTY.EXE",
"Resolution": "640x480",
"InstallResolution": "640x480",
"BitsPerPixel": "8"
```

Disc 1 is inserted when installation starts. The engine tries its autorun command, falling back to Wine Explorer when no usable autorun command is present. Do not set `InstallExe` for a disc recipe. After installation, the launcher selects the installed `ShortcutExe`; normal app launches mount the discs without running setup again.

The list accepts LF or CRLF separators and 1–256 distinct relative `.iso` paths (case-insensitive duplicate detection). Use `/` between folders and match ZIP entry names exactly. Absolute paths, `..`, blank lines, and CUE files are not accepted in collection recipes. Every listed ISO must exist after extraction or the import is discarded. The existing collection schema stays at version 1; these optional fields require a launcher that supports disc collections.

The extracted images are stored in the app's `Discs` folder, with relative paths in its settings. They travel with app backups, moves, and Wine trial copies. External images selected through General settings remain external references. Users can reorder or replace images in **App Settings → General**, switch discs in the runtime status bar, or cycle through them with **Ctrl+Shift+F12**.

## Reimporting and updating

**Keep each entry's `ID` stable.** IDs use lowercase letters, numbers, and hyphens, start with a letter or number, and contain at most 64 characters. Changing an app's name, download, checksum, description, or settings must keep its original ID.

Importing into the same destination folder merges by ID:

- New IDs add new listings.
- Existing IDs with changed fields replace their previous listing metadata.
- Identical entries are unchanged.
- Entries absent from the incoming file are retained.
- Duplicate IDs within an incoming file are rejected before anything is saved.

The review displays new, updated, and unchanged counts. Folder names match without regard to case. Each custom folder has its own entry namespace, so two folders can reuse an app ID. An update to a listing does not reinstall or modify an installed app or its saves. Using a new ID deliberately creates a separate listing.

Collections are saved snapshots in `collections.json` beside `library.json`. They appear immediately and survive relaunch, even offline or after the source JSON is moved. Each folder can remember up to 32 source URLs in its optional `sources` array; repeated imports of the same URL do not add another source. Local file imports retain any URLs already associated with the destination.

**Refresh Collection** appears in the collection header (and the Mac/Windows folder menu) when a folder has saved source URLs. It downloads every source, validates them, and reviews the combined new/updated/unchanged counts before **Apply Changes** saves anything. If no listings changed, it reports that the collection is up to date. Failed or cancelled downloads leave the entire collection unchanged; cancelling the review also saves nothing. A review is rejected if the destination changed or was deleted while refreshing.

Refresh merges by ID into the original destination folder, even if a source's top-level folder name changes. Missing entries are retained and installed apps/saves are untouched. Sources are applied in the order they were first added; when several sources define the same ID, the last source in that order wins. Changed artwork should still use a new icon URL/version as described above. Refresh is manual, not a background subscription.

Collections saved by older builds and local-file imports have no source URLs. Import a collection from its URL once to enable Refresh; local files can continue using the import flow. Deleting a collection or resetting Demos also removes its remembered source URLs.

Delete a custom collection from its folder menu or collection view. **Reset Demos** appears only when Demos has imported additions or overrides; it removes those custom listings and restores the bundled listings. Both actions affect catalog metadata only; installed apps, backups, and saves are kept. Library views such as All Apps and Recently Opened are not collection folders.
