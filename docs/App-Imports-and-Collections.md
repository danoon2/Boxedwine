# App imports and collections

The native Mac, Windows, and Linux launchers use **Add to Library**:

1. Choose **App or installer**, **Collection**, or **Restore a backup**.
2. Choose a file, folder, or direct download URL. Collections accept local or downloaded JSON. Restore uses an existing `.boxedwinebackup` package/folder.
3. Review the destination and action. A ZIP is extracted into temporary storage before review. Choose the app program or installer; supporting files are copied together. Compatibility settings are optional. Nothing runs during inspection.

App downloads support direct ZIP, EXE, and MSI links, up to 1 GiB. Collection JSON is limited to 1 MiB. Redirects cannot switch from HTTPS to HTTP. Downloads and extraction can be cancelled; failed or cancelled preparation removes temporary files. The Mac App Store build retains its existing restrictions on downloads and demo installation.

## Collection JSON

See [the example](../resources/examples/collection.json). The example URL, size, and checksum are placeholders; replace them with an actual package before importing for installation.

The format is shared by all three launchers:

- `schemaVersion`: `1`.
- `folder`: the proposed top-level collection folder, including `Demos` or an existing folder. The user can change this during review.
- `apps`: 1–256 entries using the existing demo recipe field names. All recipe values are **strings**, including sizes and boolean settings.

Required entry fields are `ID`, `Name`, `FileURL`, `FileSizeBytes`, `FileSHA256`, `InstallType`, `ShortcutExe`, and `WineVersion`. `Summary` and `Help` supply the listing text. `Icon` accepts an existing bundled PNG filename or a complete HTTP/HTTPS URL to a PNG (for example, `"Icon": "https://example.com/icons/game.png"`). Remote icons load in the background when saved listings are loaded, including after relaunch. Each image must be at most 1 MiB and 1024×1024 pixels. Downloads have a 20-second timeout, bounded redirects, no cookies or saved credentials, and no HTTPS-to-HTTP redirects. Missing, invalid, or unavailable images use the normal fallback and never prevent importing or using a collection.

Remote icons are cached under `CollectionIcons` beside `collections.json`, keyed by the complete URL, and reused offline. Change the icon URL (or add a version query such as `?v=2`) when publishing new artwork; a changed URL gets a new cached image. A failed download is retried on the next launch. Deleting a collection retains this shared image cache. Existing demo compatibility fields are supported, including Windows version, renderer, display size, CNC DDraw, Glide, and validated DLL overrides. Arbitrary command strings and legacy `Options` fields are not accepted.

`InstallType` is `Zip` for a ready-to-run ZIP or `Installer` for an EXE, MSI, or ZIP containing setup. Installer ZIPs also specify `InstallExe`, a relative path within the archive. `ShortcutExe` names the program to launch after installation. Packages require their exact byte size and SHA-256 checksum, including when hosted outside boxedwine.org.

## Reimporting and updating

**Keep each entry's `ID` stable.** IDs use lowercase letters, numbers, and hyphens, start with a letter or number, and contain at most 64 characters. Changing an app's name, download, checksum, description, or settings must keep its original ID.

Importing into the same destination folder merges by ID:

- New IDs add new listings.
- Existing IDs with changed fields replace their previous listing metadata.
- Identical entries are unchanged.
- Entries absent from the incoming file are retained.
- Duplicate IDs within an incoming file are rejected before anything is saved.

The review displays new, updated, and unchanged counts. Folder names match without regard to case. Each custom folder has its own entry namespace, so two folders can reuse an app ID. An update to a listing does not reinstall or modify an installed app or its saves. Using a new ID deliberately creates a separate listing.

Collections are saved snapshots in `collections.json` beside `library.json`. They appear immediately and survive relaunch, even offline or after the source JSON is moved. Reimport the file or URL to refresh a collection; it is not a background subscription.

Delete a custom collection from its folder menu or collection view. **Reset Demos** appears only when Demos has imported additions or overrides; it removes those custom listings and restores the bundled listings. Both actions affect catalog metadata only; installed apps, backups, and saves are kept. Library views such as All Apps and Recently Opened are not collection folders.
