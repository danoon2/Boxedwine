# Shared native UI demo catalog

`demo-catalog.lock.json` pins the version, immutable HTTPS URL, exact byte count
and SHA-256 of a ZIP hosted on boxedwine.org. Commit this lock with changes to the
code that consumes it. Branches can pin different packages independently. There
is no checked-in catalog or artwork fallback and no lookup of a mutable “latest”
URL. An unchanged pin makes no network request when its verified ZIP is cached.

The package contains `catalog.xml` and its referenced PNGs in one flat directory.
The XML's `release` matches the lock's `version`; `schemaVersion` describes the
recipe format separately. The current pin, `26R2-catalog-2`, contains 34 recipes and icons. It removes
Java Solitaire and FreeCol from the original 36-entry `26R2-catalog-1`.
The new ZIP is prepared and cached locally; upload is pending. See the
[recipe schema](../project/mac-xcode/Boxedwine/BoxedwineUI/DEMO_CATALOG.md).

`tools/demo_catalog.py` uses Python 3's standard library and is shared build
infrastructure for future native frontends. The old OpenGL UI still uses its
existing XML. Other platforms must implement the recipe schema before using
this package; this change does not convert their UIs.

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
   releases. Give the XML a new `release`, for example `26R2-catalog-2`. Retain the
   schema version unless the recipe format changes. Follow the native schema's
   payload verification and compatibility testing procedure.
3. Validate recipes with `BoxedwinePackageCheck --catalog`, then pack the files
   and update the project pin:

   ```sh
   swift run --package-path project/mac-xcode/Boxedwine/BoxedwineUI \
     BoxedwinePackageCheck --catalog tmp/catalog-edit/catalog.xml
   python3 tools/demo_catalog.py pack --source tmp/catalog-edit \
     --version 26R2-catalog-2 \
     --url https://boxedwine.org/catalogs/26R2-catalog-2/catalog.zip \
     --output tmp/catalog-2.zip --lock resources/demo-catalog.lock.json
   ```

4. Upload the ZIP to the exact URL printed by the packer. Never overwrite an
   existing release URL. For testing before upload, `import --zip tmp/catalog-2.zip`
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
