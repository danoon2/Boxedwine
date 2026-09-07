# Local Build Site

`local-build-site.sh` builds a local copy of the Boxedwine web demo site so Emscripten builds can be tested in a browser before publishing anything.

The script mirrors the existing public build site, builds fresh single-threaded, multi-threaded, single-threaded JIT, and multi-threaded JIT Emscripten outputs, places those outputs into the same layout used by the published site, regenerates the demo pages, and starts a local static server with the headers required by the multi-threaded builds.

## What It Does

1. Syncs an existing build site into a local folder.
   - If `BUILD_SITE_REMOTE` is set, it uses `rsync` from that remote.
   - Otherwise it falls back to `wget` from `https://boxedwine.org/builds/`.
2. Hydrates demo assets for local use.
   - Demo app ZIPs and `demos.json` are refreshed when the public files are newer; unchanged local files are retained.
   - One filesystem, `TinyCore15Wine11.0.zip`, is downloaded from `http://boxedwine.org/v2/11/TinyCore15Wine11.0.zip`.
   - A demo selects GDI with `"directDrawRenderer": "gdi"` in `demos.json`. The generated launcher sets the registry before running the executable or existing batch file. Other demos retain Wine's default renderer without a registry command.
   - Each app ZIP, or overlay ZIP list when no app ZIP is present, gets its own persistent Wine root and D: drive in IndexedDB. Games on the same origin keep separate registries, settings, and saves; the same game shares its storage across all four runtime modes and subsequent builds.
   - Existing `boxedwine.3.zip` and `boxedwine.gdi.3.zip` selections are migrated to the v11 root when it is available; GDI selections become a launch setting. Old published ZIPs and build pages remain available.
   - A demo can set an app-specific Wine compatibility version with `"windowsVersion": "win98"`. The generated launcher sets that version before starting the executable, without changing other demos.
3. Validates every filesystem root referenced by `demos.json` against
   `tools/buildWine/webgl_filesystems_v11.json`.
   - The patch series, declared filename, archive size and SHA-256, full ZIP CRC, PE32 DLLs and
     imports, GL SONAME links, `ld.so.cache`, and renderer registry settings must all match.
   - Validation runs before the four Emscripten builds, so a missing, stale, or damaged public
     root stops the local workflow before the expensive compilation step.
4. Builds the Emscripten web targets from `project/emscripten`.
   - `make release`
   - `make multiThreaded`
   - `make jit`
   - `make multiThreadedJit`
5. Copies the web build outputs into the existing demo runner layout:
   - `project/emscripten/Deploy/Web/SingleThreaded`
   - `project/emscripten/Deploy/Web/MultiThreaded`
   - `project/emscripten/Deploy/Web/SingleThreadedJit`
   - `project/emscripten/Deploy/Web/MultiThreadedJit`
6. Runs `tools/jenkins/build_site.py` against the local site directory.
7. Starts `project/emscripten/server.mjs` with:
   - `Cross-Origin-Embedder-Policy: require-corp`
   - `Cross-Origin-Opener-Policy: same-origin`
   - `Cross-Origin-Resource-Policy: same-origin`

The local workflow does not upload or publish anything.

## Prerequisites

Run this from WSL. The default Emscripten SDK path is:

```bash
/home/james/emsdk
```

The script expects these tools to be available in WSL:

```bash
bash
make
node
python3
rsync
wget
```

## Basic Usage

From PowerShell:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh'
```

When it finishes, open:

```text
http://127.0.0.1:8000/
http://127.0.0.1:8000/demos/
```

## Faster Local Iteration

Reuse the already mirrored site and already built Emscripten outputs:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh --skip-sync --skip-build'
```

`--skip-sync` deliberately bypasses the public demo ZIP and `demos.json` refresh.

Game storage starts fresh when switching from the older shared `/root` and
`/d_drive` databases. Those old databases are preserved but are not imported into
the new game databases. Desktop/upload launches without an app or overlay ZIP
still use the original storage. `storage=memory` disables persistence for a launch.

The launcher derives storage names from archive names, so renaming an archive
starts a separate game profile. Changing the Wine filesystem ZIP or runtime mode
does not reset a game's profile.

Run the launcher storage tests with:

```bash
node --test project/emscripten/boxedwine-shell.test.cjs
```

Generate the site without starting a server:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh --skip-sync --skip-build --no-server'
```

Preview what would run without changing files:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh --dry-run'
```

## Common Options

```text
--site-dir DIR        Local website directory
--mirror-url URL      Public site mirror URL when BUILD_SITE_REMOTE is unset
--branch NAME         Branch name recorded in the generated site
--build-number NUM    Build number recorded in the generated site
--emsdk DIR           Emscripten SDK directory
--buildfiles-dir DIR  Extra web files copied like Jenkins
--demo-root-config FILE
                      Pinned filesystem manifest used to validate demo roots
--host HOST           Local server host
--port PORT           Local server port
--skip-sync           Reuse the existing local website directory
--skip-build          Reuse existing Emscripten Deploy/Web outputs
--no-server           Generate the site but do not start a server
--dry-run             Print the workflow without changing files
```

## Useful Environment Variables

```text
BUILD_SITE_REMOTE          rsync source for the existing build site
BUILD_SITE_SSH_KEY         SSH key used with BUILD_SITE_REMOTE
BUILD_SITE_MIRROR_URL      fallback public mirror URL
LOCAL_BUILD_SITE_DIR       local generated site directory
BUILD_SITE_DEMOS_SOURCE    source directory for demo zips and demos.json
BOXEDWINE_ZIP_URL          standard demo filesystem URL
BUILD_SITE_DEMO_ROOT_CONFIG
                           pinned filesystem manifest used to validate demo roots
BUILD_SITE_SINGLE_THREADED_DIR
BUILD_SITE_MULTI_THREADED_DIR
BUILD_SITE_SINGLE_THREADED_JIT_DIR
BUILD_SITE_MULTI_THREADED_JIT_DIR
                           override Emscripten web runner directories
BUILD_SITE_PUBLIC_URL      public URL written into generated links
BUILD_SITE_KEEP            number of builds to keep per branch
EMSDK_DIR                  Emscripten SDK directory
HOST                       local server bind host
PORT                       local server port
```

## Output Locations

```text
project/linux/Deploy/LocalBuildSite/
project/emscripten/Deploy/Web/SingleThreaded/
project/emscripten/Deploy/Web/MultiThreaded/
project/emscripten/Deploy/Web/SingleThreadedJit/
project/emscripten/Deploy/Web/MultiThreadedJit/
```

The generated local site and server log/pid files are local runtime artifacts and should not be checked in.

## Validate Demo Roots Only

To verify the current downloaded roots and `demos.json` without building Emscripten or changing
the generated site:

```powershell
wsl python3 tools/jenkins/build_site.py `
    --site-dir project/linux/Deploy/LocalBuildSite `
    --demo-source project/linux/Deploy/LocalBuildSite/demos/apps `
    --validate-demo-roots-only
```

This command requires each referenced root filename to have an exact profile in
`tools/buildWine/webgl_filesystems_v11.json`. If the public downloads are older than the pinned
profiles, upload the current roots first; do not bypass the validation.

## Stopping The Local Server

If the script was started by the helper workflow and wrote a pid file:

```powershell
wsl bash -lc 'kill $(cat /mnt/c/BoxedwineGPT/tools/jenkins/local-server.pid)'
```

Or find the process manually:

```powershell
wsl bash -lc 'ps -ef | grep "project/emscripten/server.mjs" | grep -v grep'
```

## Jenkins Publish Script

`publish-build-site.sh` is the Jenkins-side publishing workflow. It syncs the remote site down, updates the build/demo pages, and syncs the result back up to `BUILD_SITE_REMOTE`.

Both scripts download the same filesystem:

```text
http://boxedwine.org/v2/11/TinyCore15Wine11.0.zip
```

and save it as:

```text
demos/apps/TinyCore15Wine11.0.zip
```

The publish workflow passes the same pinned manifest to `build_site.py`. Root validation finishes
before the generated site is changed and before `rsync` uploads it, so an invalid filesystem cannot
be published through this script.

`BOXEDWINE_ZIP_URL` can override the download source for local validation before
uploading a release. `BUILD_SITE_DEMO_ROOT_CONFIG` selects a different manifest.
The GDI ZIP download is no longer needed. Upload the exact validated v11 archive
before running either script against the default URL.
