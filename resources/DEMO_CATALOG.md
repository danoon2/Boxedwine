# Shared native UI demo catalog

`demo-catalog.lock.json` pins the version, immutable HTTPS URL, exact byte count
and SHA-256 of a ZIP hosted on boxedwine.org. Commit this lock with changes to the
code that consumes it. Branches can pin different packages independently. There
is no checked-in catalog or artwork fallback and no lookup of a mutable “latest”
URL. An unchanged pin makes no network request when its verified ZIP is cached.

The package contains `catalog.xml` and its referenced PNGs in one flat directory.
The XML's `release` matches the lock's `version`; `schemaVersion` describes the
recipe format separately. The current pin, `26R2-catalog-10`, contains 40 recipes and icons,
including 3DMark2001 SE, 3DMark03 (2003), 3DMark05 (2004),
3DMark06 (2006), 3DMark Vantage (2008), and Sim Tower Demo (1994).
The 3DMark03, 3DMark05, 3DMark06, and Vantage details include their full-version
unlock keys. The new entries use application icons and select `3DMark06.exe`,
`3DMarkVantage.exe`, or `SIMTDEMO.EXE` after installation. SimTower uses the demo
archive `towerdmo.zip`, replacing the full-game archive from catalog 7 while
preserving its `sim-tower` ID. It launches `SIMTOWER/Setup.exe` with its accompanying
installer files, including the WinG libraries. Payload byte counts and SHA-256
hashes match the supplied local installers; the hosted SimTower demo also matches.
All demos now select Wine 11.0: Daytona USA and vkQuake no longer select Wine 9.0,
and vkQuake's obsolete Wine 10 mouse-button warning has been removed.
Java Solitaire and FreeCol were removed in `26R2-catalog-2`.
Catalog 9 corrects the 3DMark05 display year to 2004; its installer and recipe are unchanged.
Catalog 10 retains that correction and adds Vantage's CPU-Z/GPU-Z driver exclusions.
The new ZIP is prepared locally; upload it to the pinned URL and verify a fresh
download before merging the pin. See the
[recipe schema](../project/mac-xcode/Boxedwine/BoxedwineUI/DEMO_CATALOG.md).

The native launchers initialize newly installed Alice demos (ID `alice`) with
200% mouse sensitivity. This is saved as an editable per-app Boxedwine argument;
existing installations retain their saved settings.

Schema 8 adds optional `WineDllOverrides`, a nonempty single-line value of at most
1024 characters. All three native launchers seed it into the new app's editable
Boxedwine arguments as `-env` followed by `WINEDLLOVERRIDES=<value>`. The value is
passed literally, including a trailing `=` used to disable a DLL or driver. The
existing app metadata and backup formats already preserve these arguments.
Catalog updates do not change previously installed apps or their custom settings.

Vantage uses `<WineDllOverrides>cpuz136_x32.sys,cpuz136_x64.sys,GPUZ.sys=</WineDllOverrides>`
to avoid its GPU-Z hardware-scan exception. This only disables those probes in
Vantage's Wine environment; it does not replace any Wine module. Existing Vantage
installations can use the same `-env` argument in their app settings.

The native Mac, Windows, and Linux UIs consume this shared pin and recipe format.
Each UI sorts demos alphabetically by name, independently of their XML order,
using the same ordering as its installed-app list.
`tools/demo_catalog.py` uses Python 3's standard library to package and validate
the catalog. The old OpenGL UI still uses its existing XML, and the Emscripten
demo site has a separate catalog.

Windows builds always copy the extracted catalog into their output and publish
directories. Catalog ZIPs use fixed timestamps, so copying only newer files can
leave an older catalog embedded after the pin changes.

## Building and caching

Xcode's `BoxedwineUI` bundle phase runs the tool automatically. It verifies the
cached ZIP on every use, or downloads the exact URL with a 20-second total timeout.
Only verified downloads enter the cache. The native recipe validator then checks
the XML and icons before Xcode signs the app. Downloaded game payloads and Wine
filesystems are separate from this small catalog ZIP.

| Situation | Local Xcode build | Jenkins / `buildRelease.sh` |
| --- | --- | --- |
| Exact verified ZIP cached | Use it, without network | Use it, without network |
| Missing cache, download succeeds | Verify and embed | Verify and embed |
| Missing/corrupt cache, download fails | Warn; build without demos | Fail before archiving |
| Wrong version cached | Fetch pinned version, or omit demos | Fetch pinned version, or fail |
| Downloaded catalog has unsupported recipes | Fail native validation | Fail native validation |

When a local build omits demos, its Demos screen says they aren't included.
Installed apps and manual installation remain available. Rebuild with a network
connection to include the catalog. The output directory is replaced on each build,
so a previous branch's list cannot survive an offline version change. There is no
runtime catalog/artwork update: the signed app always uses its embedded list.

Default cache directories:

- macOS: `~/Library/Caches/BoxedwineBuild/demo-catalogs`
- Linux: `$XDG_CACHE_HOME/BoxedwineBuild/demo-catalogs` (default `~/.cache`)
- Windows: `%LOCALAPPDATA%/BoxedwineBuild/demo-catalogs`

ZIPs are named by SHA-256. Concurrent builds download into separate temporary
files and publish atomically. Set `BOXEDWINE_CATALOG_CACHE` to share/persist a cache
on a build worker. Jenkins may use its cache offline, but must have the exact pin.
Deleting the cache is safe; the next build downloads it again.

Xcode build settings `BOXEDWINE_REQUIRE_DEMO_CATALOG=YES` and
`BOXEDWINE_CATALOG_OFFLINE=YES` require the package and disable network respectively.
`buildRelease.sh` always requires it, independently of the environment. Direct CLI
equivalents are `--required` and `--offline`. `stage --output` **replaces that entire
directory**; use a dedicated build-resource directory, never your editing source.

```sh
python3 tools/demo_catalog.py stage --required --output tmp/catalog-resources
python3 tools/demo_catalog.py audit --output tmp/catalog-resources
```

Staging adds `catalog-package.json` to the output, recording the pin and file
hashes. It is a generated build receipt, not part of the uploaded ZIP. The Mac
bundle audit checks this receipt and every asset; release/distribution audits also
require that the catalog is present. ZIP validation rejects links, paths,
duplicates, unexpected files, malformed XML and missing icons, with size limits.

## Publishing a catalog update

1. Resolve the current package using `stage`, then copy the resulting files into
   an editing directory outside Git (for example `tmp/catalog-edit`). The packer
   ignores the generated receipt. Keep a backed-up authoring copy or retrieve the
   immutable published ZIP to edit it later.
2. Edit `catalog.xml` and its PNGs. Preserve demo IDs for the same app across
   releases. Give the XML a new `release`, for example `26R2-catalog-8`. Retain the
   schema version unless the recipe format changes. Follow the native schema's
   payload verification and compatibility testing procedure.
3. Validate recipes with `BoxedwinePackageCheck --catalog`, then pack the files
   and update the project pin:

   ```sh
   swift run --package-path project/mac-xcode/Boxedwine/BoxedwineUI \
     BoxedwinePackageCheck --catalog tmp/catalog-edit/catalog.xml
   python3 tools/demo_catalog.py pack --source tmp/catalog-edit \
     --version 26R2-catalog-8 \
     --url https://boxedwine.org/catalogs/26R2-catalog-8/catalog.zip \
     --output tmp/catalog-8.zip --lock resources/demo-catalog.lock.json
   ```

4. Upload the ZIP to the exact URL printed by the packer. Never overwrite an
   existing release URL. For testing before upload, `import --zip tmp/catalog-8.zip`
   verifies the ZIP against the project lock and seeds only your build cache.
5. Verify an actual download using a fresh cache, test the catalog, and build:

   ```sh
   python3 tools/demo_catalog.py stage --required --cache tmp/fresh-catalog-cache \
     --output tmp/catalog-resources
   BOXEDWINE_TEST_DEMO_CATALOG="$PWD/tmp/catalog-resources" \
     swift test --package-path project/mac-xcode/Boxedwine/BoxedwineUI
   sh project/mac-xcode/buildNative.sh
   ```

6. Commit the updated lock and any schema/code changes. Keep all released ZIPs
   available so older branches and tags can rebuild. Upload before merging the
   pin so a clean Jenkins worker can build immediately.

## Tests

`python3 tools/test_demo_catalog.py` runs without network or release assets. The
Swift unit suite uses synthetic recipe fixtures and works on a fresh offline
checkout. Setting `BOXEDWINE_TEST_DEMO_CATALOG` additionally checks the staged
release's recipes against the Wine catalog and decodes every icon. Supplying a
bad path fails those checks rather than silently skipping them.
