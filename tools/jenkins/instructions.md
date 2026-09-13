# Local Build Site

`local-build-site.sh` builds a local copy of the Boxedwine web demo site so Emscripten builds can be tested in a browser before publishing anything.

The script mirrors the existing public build site, builds single-threaded, multi-threaded, single-threaded JIT, and multi-threaded JIT Emscripten outputs, places those outputs into the same layout used by the published site, regenerates the demo pages, and starts a local static server with the headers required by the multi-threaded builds.

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

Local builds now reuse each mode's compiled objects. The makefile records
compiler identity and compilation/link settings as separate dependencies.
Changed flags (including `WASM_PROFILING`) invalidate the affected build
configuration; changed headers and link inputs are tracked normally. Removing
a source or deleting a generated `.js`/`.wasm` also forces the necessary work.
The first build after introducing configuration tracking recompiles existing
objects that have no configuration record.

Use `--clean-build` (or `LOCAL_BUILD_SITE_CLEAN_BUILD=1`) for a full rebuild.
It cannot be combined with `--skip-build`. Build logs include `BUILD_TIMING`
records for download, root validation, each mode's compilation/link/copy,
and site generation. The link phase includes Emscripten's Wasm optimization;
these records do not yet split the compiler's internal optimizer/linker steps.

The underlying makefile supports `make jit BUILD_PHASE=compile` followed by
`make jit BUILD_PHASE=all` for the same separation. Ordinary `make jit` still
builds the complete output.

Dependency and workflow checks use fake tools and do not run an emulator or
GPU workload:

```bash
python3 project/emscripten/test_build_config.py -v
python3 tools/jenkins/test_local_build_site.py -v
```

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
--clean-build         Recompile from scratch instead of reusing objects
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

Newly generated demo builds retain `graphics-builds.json` beside their build
index. Every launch link includes a `buildid` that identifies the supplied
commit label, execution mode, all five runtime files, the root ZIP and its
WebGL `wined3d.dll`. The full SHA-256 values remain in that JSON file. An
unknown source commit or dirty state stays unknown; a commit label alone
does not establish the provenance of an existing runtime binary.

The generator verifies the launch files against those identities. Each new
build keeps its own root and application ZIPs, and the mode directories link
to those preserved copies where supported. Updating `demos/apps` for a later
build therefore cannot change the archives used by an earlier build created
with this generator. Existing historical sites are not rewritten or assigned
retroactive identities. The normal `--keep` pruning policy still applies.

## Wine graphics CI

`run_graphics_ci.py` runs the complete configured four-mode Wine graphics grid
against immutable inputs, then independently audits results, full browser logs,
cleanup and suite coverage. It preserves the first failed attempt and reports
unrun groups as failures. The wrapper does not build, download or publish inputs.

Create a JSON file on the test worker using these fields. Paths are absolute
or relative to the configuration file. Replace the example paths and hashes
with a reviewed input set; do not regenerate expected counts from CI output.

```json
{
  "schema_version": 1,
  "filesystem": "inputs/TinyCore15Wine11.0.zip",
  "tests_archive": "inputs/wine_tests_graphics.zip",
  "tests_archive_sha256": "REVIEWED_ARCHIVE_SHA256",
  "baseline": "inputs/expected.json",
  "baseline_sha256": "REVIEWED_BASELINE_SHA256",
  "divergences": "inputs/webgl-test-divergences.json",
  "builds": {
    "single-threaded-non-jit": "runtime/st",
    "single-threaded-jit": "runtime/st-jit",
    "multi-threaded-non-jit": "runtime/mt",
    "multi-threaded-jit": "runtime/mt-jit"
  },
  "timeout_seconds": 1800,
  "headless": true
}
```

The baseline already pins the root, divergence manifest and executable bytes.
The two additional hashes pin the complete test ZIP and the baseline itself.
The runner records every runtime file's hash before and after execution.
Keep the immutable build manifests identifying the source revision and local
changes alongside the runtime directories; a checkout revision alone does
not establish the provenance of an existing Wasm binary.

Run from the repository root with Python 3.10+ and installed desktop Chrome:

```powershell
python tools/jenkins/run_graphics_ci.py --config C:/ci/graphics.json --output tmp/graphics-preflight --prepare-only
python tools/jenkins/run_graphics_ci.py --config C:/ci/graphics.json --output tmp/graphics-full
```

Preflight returns zero when inputs validate, writes `state: prepared` with
`passed: false`, and produces no JUnit results or browser launch. The real run
returns zero only after both audits and the unchanged-input check pass. It
retains `inputs.json`, `command.json`, `status.json`, `artifact-audit.json`,
`coverage-audit.json`, `junit.xml` and each group's complete matrix artifacts.
Expected upstream failures still require their exact Wine exit status; they
are not general failure allowances.

The per-group timeout includes startup and fifteen seconds of cleanup
observation. A full run may take many hours on the interpreter. Stop-on-failure
does not erase partial group logs. Investigate a failure before deciding on a
new run; never merge duplicate successful retries over the original evidence.


`graphics.Jenkinsfile` is an opt-in Pipeline definition. Configure a dedicated
Windows GPU worker with label `boxedwine-graphics-windows` and one executor,
then point a Pipeline from SCM job to `tools/jenkins/graphics.Jenkinsfile`.
Set `BOXEDWINE_GRAPHICS_CONFIG` to the reviewed JSON file and optionally set
`BOXEDWINE_GRAPHICS_PYTHON` (default: `python`). The template schedules a weekly
Sunday run, serializes overlapping builds and archives JSON/log/XML artifacts
even after failure. Keep immutable inputs outside workspace cleanup.

Validate Chrome acceleration and the worker account in a manual job before
enabling its schedule. The template has not been deployed or validated on a
Jenkins worker by these local changes. Run the lightweight wrapper checks with
`python tools/jenkins/test_graphics_ci.py`; they use synthetic artifacts and
launch no browser, emulator or compiler.

## Application-only overlays

`prepare_demo_overlay.py SOURCE.zip OUTPUT.zip --include PATH --report REPORT.json`
creates a deterministic ZIP containing explicitly selected regular files under
`home/username/.wine/drive_c/`. Repeat `--include`; use a trailing slash to select
a directory. Both output paths must be new. The report records source and member
hashes and omitted paths. Packaging does not validate the application's startup.
