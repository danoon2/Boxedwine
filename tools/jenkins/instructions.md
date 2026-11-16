# Native automation filesystem

The root `Jenkinsfile` downloads the full native automation filesystem separately
from the apps and scripts in `automation34.zip`. Its URL, size, and SHA-256 are
pinned in [`../automation/BoxedWineRunner/filesystem.properties`](../automation/BoxedWineRunner/filesystem.properties).
The bundle contains no filesystem. Every native worker verifies the direct
download and applies the existing GDI renderer settings to each
writable test root. The downloaded ZIP is unchanged. See the
[runner instructions](../automation/BoxedWineRunner/README.md#native-filesystem) for local use
and the [release checklist](../buildWine/RELEASING.md) for advancing the pin.

Browser AbiWord automation keeps its separate web filesystem URL and SHA-256 in
`Jenkinsfile`; changing the native pin does not change browser automation.
Recordings and screenshot references are packaged in the downloaded automation
assets. Update those archives when recordings change; Jenkins does not patch
individual recordings or generate additional recording overlays.

# Local Build Site

`local-build-site.sh` builds a local copy of the Boxedwine web demo site so Emscripten builds can be tested in a browser before publishing anything.

The script mirrors the existing public build site, builds single-threaded, multi-threaded, single-threaded JIT, and multi-threaded JIT Emscripten outputs, places those outputs into the same layout used by the published site, regenerates the demo pages, and starts a local static server with the headers required by the multi-threaded builds.

## What It Does

1. Syncs an existing build site into a local folder.
   - If `BUILD_SITE_REMOTE` is set, it uses `rsync` from that remote.
   - Otherwise it falls back to `wget` from `https://boxedwine.org/builds/`.
2. Hydrates demo assets for local use.
   - Demo app ZIPs and `demos.json` are refreshed when the public files are newer; unchanged local files are retained.
   - The web v39 filesystem, `TinyCore15Wine11.0-web.zip` (size and SHA-256 pinned in the release manifest), is downloaded from `https://boxedwine.org/v2/39/TinyCore15Wine11.0-web.zip`.
   - A demo selects GDI with `"directDrawRenderer": "gdi"` in `demos.json`. The generated launcher sets the registry before running the executable or existing batch file. Other demos retain Wine's default renderer without a registry command.
   - Each app ZIP, or overlay ZIP list when no app ZIP is present, gets its own persistent Wine root and D: drive in IndexedDB. Games on the same origin keep separate registries, settings, and saves; the same game shares its storage across all four runtime modes and subsequent builds.
   - Existing full `TinyCore15Wine11.0.zip`, `boxedwine.3.zip`, and `boxedwine.gdi.3.zip` selections are migrated to the web v35 root when it is available; GDI selections become a launch setting. Old published ZIPs and build pages remain available.
   - A demo can set an app-specific Wine compatibility version with `"windowsVersion": "win98"`. The generated launcher sets that version before starting the executable, without changing other demos.
3. Validates every filesystem root referenced by `demos.json` against
   `tools/buildWine/webgl_filesystems.json`.
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
BUILD_SITE_REMOTE          remote build site (SSH host:path for publishing)
BUILD_SITE_SSH_KEY         SSH key used with BUILD_SITE_REMOTE
BUILD_SITE_MIRROR_URL      fallback public mirror URL
LOCAL_BUILD_SITE_DIR       local generated site directory
BUILD_SITE_DEMOS_SOURCE    local-build-site.sh source directory for demo ZIPs and demos.json
BOXEDWINE_ZIP_URL          local filesystem download URL; optional versioned URL override for publishing
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
`tools/buildWine/webgl_filesystems.json`. If the public downloads are older than the pinned
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

`publish-build-site.sh` is the Jenkins-side publishing workflow. It reads a small
metadata snapshot from `BUILD_SITE_REMOTE`, generates the new build pages locally,
and uploads the new runners, artifacts, logs and pages. It does not mirror old
build payloads or download/copy demo and filesystem ZIPs for each build.

The publishing host needs Python 3 as well as SSH/rsync. The standalone
`publish_site_state.py` helper runs over SSH stdin, without installing a script
on the host. It reads branch/build JSON, `demos/apps/demos.json`, ZIP directory
entries (for app/overlay detection and executable fallback), and screenshot
hashes. Archive payloads remain on the server. Failed builds skip demo inventory.

New demo launch links use the existing shared `demos/apps/<name>.zip` archives
and images. The filesystem URL defaults to the version and filename pinned in
`tools/buildWine/webgl_filesystems.json`, currently
`https://boxedwine.org/v2/39/TinyCore15Wine11.0-web.zip`. Each build records that
specific URL; updating the release pin affects subsequent builds only. To keep
an older demo payload available, publish a changed demo under a new filename
and update `demos.json`; overwriting a shared filename affects builds linking to it.

Publication uploads a partial site and never uses `rsync --delete`. Under the
existing publication lock, it records which branch/build directories were
retired by pruning, uploads successfully, then removes only those directories.
Shared assets and retained historical builds are preserved. `BUILD_SITE_KEEP`
defaults to five retained builds per branch. Existing per-build archive copies
are not migrated; they disappear through normal build retention.

The root `Jenkinsfile` packages artifacts only after a successful build. Its
`post/always` handler still publishes after an earlier test or compile failure,
or a packaging failure, even when no build stashes exist. Failed, unstable, and
aborted builds show an **Error Log** link instead of ZIP downloads or demos.
Historical failures with no saved log show **Log unavailable**.

The handler captures the console output available at publication time using
Token Macro's `tm('${BUILD_LOG, maxLines=2147483647, escapeHtml=false}')` and passes
the UTF-8 file as `BUILD_SITE_LOG`. Token Macro is already a dependency of the
email-ext plugin used by this Pipeline; no additional API credential or Groovy
script approval is needed. The snapshot includes packaging errors, but cannot
include later publishing or notification output. Failed publications require an
existing `BUILD_SITE_LOG` and skip artifact copying, demo downloads, and demo
validation, preserving the previous successful demos.

These changes apply to new runs using the updated Jenkinsfile. Previously missed
builds need their console logs and original build metadata passed through the
publisher separately; updating the Jenkinsfile does not backfill them.

Regression checks (the shell tests require POSIX bash and rsync):

```bash
python3 tools/jenkins/test_build_site.py -v
python3 tools/jenkins/test_publish_build_site.py -v
python3 tools/jenkins/test_publish_site_state.py -v
groovy tools/jenkins/test_publish_pipeline.groovy
```

The Groovy check executes the actual Jenkinsfile's `post/always` code with mocked
Pipeline steps. It covers early failures, missing stashes, ZIP/archive failures,
successful builds, and a publishing worker without a checkout. It makes no
network connections and does not start a Jenkins build.

The local development script still downloads and validates the filesystem for
offline/local demo testing:

```text
https://boxedwine.org/v2/39/TinyCore15Wine11.0-web.zip
```

and save it as:

```text
demos/apps/TinyCore15Wine11.0-web.zip
```

The Jenkins publisher uses the pinned manifest's archive and WineD3D identities,
without opening or revalidating the released filesystem on each build. Release
archives must be validated when published and kept unchanged at their versioned
URLs. Locally supplied runtime files are hashed and their copies are checked.

`BOXEDWINE_ZIP_URL` can override the download source for local validation. For
Jenkins it changes only the launch URL and must retain the pinned version directory
and filename. `BUILD_SITE_DEMO_ROOT_CONFIG` selects a different manifest.

Newly generated demo builds retain `graphics-builds.json` beside their build
index. Every launch link includes a `buildid` that identifies the supplied
commit label, execution mode, all five runtime files, the root ZIP and its
WebGL `wined3d.dll`. The full SHA-256 values remain in that JSON file. An
unknown source commit or dirty state stays unknown; a commit label alone
does not establish the provenance of an existing runtime binary.

For published demos, `graphics-builds.json` records `filesystemSource.url` and
`verification: release-manifest` to distinguish the pinned release metadata from
an archive examined locally. The local development generator continues to
validate and preserve its own root/application ZIPs. Existing historical sites
are not rewritten or assigned retroactive identities.

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
  "filesystem": "inputs/TinyCore15Wine11.0-web.zip",
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

### Age of Empires Trial

`prepare_aoe_demo.py` repackages the public trial from
`https://www.boxedwine.org/boxedwine/apps/aoe.zip`. It checks the source SHA-256
before packaging, retains all 128 game files, and adds only a launcher and the
trial's installation/DirectPlay registry entries. It does not copy a user's
Wine registry, settings, or saves. Both output paths must be new.

```bash
mkdir -p tmp/aoe-demo
wget -O tmp/aoe-demo/source.zip https://www.boxedwine.org/boxedwine/apps/aoe.zip
python3 tools/jenkins/prepare_aoe_demo.py tmp/aoe-demo/source.zip \
    tmp/aoe-demo/aoe-overlay.zip --report tmp/aoe-demo/package.json
python3 -m unittest discover -s tools/jenkins -p test_prepare_aoe_demo.py
```

Copy `aoe-overlay.zip` into `demos/apps/` and merge its entry from
`demos.example.json` into `demos/apps/demos.json`. Add a gameplay thumbnail at
`demos/apps/images/aoe-overlay.png`. Use the same files for the local site and
the configured `BUILD_SITE_REMOTE`; the next local refresh picks up the published
assets, and the next successful Jenkins publish includes all four runtime modes.
Updating the example manifest alone does not deploy assets.

Keep `registry=gdi,winxp`, `resolution=1024x768`, and `bpp=32`. The shared GDI
preset also disables the WebGL WineD3D profile, which AoE's paletted DirectDraw
surfaces need. The desktop remains 32-bit. The overlay launcher imports the
game's registry entries and sets its installed C: working directory before
starting `empires.exe`; launching the bare public ZIP from D: omits that setup.
Use the web v34 filesystem and a runtime containing the shared GDI preset fix.

The 2026-11-13 smoke test used the published `james/web_gdi` build 3 ST JIT
runtime and v34 web filesystem with fresh in-memory storage. It reached the
main menu, campaign selection, and a rendered **Bronze Age Art of War** scenario.
All four generated mode links and their published archives were checked; the
interactive gameplay check covered ST JIT.
The exit check did not observe the Wine process finishing within 25 seconds
after leaving the scenario; the test browser was closed explicitly. This smoke
test verifies startup and rendered gameplay, not clean application shutdown.

### Max Payne Demo

`MaxPayneDemo.zip` is a ready-to-run application archive. Use the supplied ZIP
unchanged (136,027,034 bytes, SHA-256
`1b5ac32661c3c061a93ce34874bf54569ac35e6056e9406d9ed891374d4f7461`).
It includes `MaxPayneDemo.exe`, the game's data archives, and its runtime DLLs.
It needs neither an installation overlay nor the GDI preset.

Place it at `demos/apps/MaxPayneDemo.zip`, merge the matching entry from
`demos.example.json` into `demos/apps/demos.json`, and place its gameplay
thumbnail at `demos/apps/images/MaxPayneDemo.png`. Use the same assets for local
generation and Jenkins publication. Launch with `app=MaxPayneDemo.zip`,
`p=MaxPayneDemo.exe`, `resolution=1024x768`, and `bpp=32`, using the v34 web
filesystem. The game first opens its settings window; choose **Play**.

The native UI catalogues use the same ZIP with `InstallType=Zip` and
`ShortcutExe=MaxPayneDemo.exe`. Their source URL is
`https://boxedwine.org/builds/demos/apps/MaxPayneDemo.zip`.

The 2026-11-13 ST JIT smoke test with published `james/web_gdi` build 3 and
the v34 web filesystem reached the tutorial and accepted movement input.
All four published mode links resolve to the verified archive. Interactive
gameplay testing covered ST JIT.

### SimCity 3000 Demo

`Simcity3000Demo.zip` is the supplied ready-to-run archive, unchanged:
123,939,643 bytes, SHA-256
`3d8f1e949a171487b4827d18c723c5ab37b7084cda81cd33f886a7b1496928be`.
It contains the demo and its DLLs/data under `SC3/`; no installer or installation
registry overlay is added.

Place it at `demos/apps/Simcity3000Demo.zip`, merge its entry from
`demos.example.json` into `demos/apps/demos.json`, and copy the gameplay thumbnail
to `demos/apps/images/Simcity3000Demo.png`. The same files serve local generation
and Jenkins publication. Use `app=Simcity3000Demo.zip`, `p=SC3/sc3demo.exe`,
`registry=gdi`, `resolution=1024x768`, and `bpp=32` with the v34 web filesystem.
The shared GDI preset disables the WebGL WineD3D profile. The game selects
800x600 when opening its interactive tutorial, demonstrating a mode switch
from the initial desktop. Close the tutorial and use the Run Simulation arrow
to explore the demonstration city.

The 2026-11-13 ST JIT check used published `james/web_gdi` build 3 with fresh
in-memory storage. It verified the tutorial, mouse controls, advancing dates,
population growth, and rendered city development. Exit and Wine cleanup both
returned zero; cleanup exceeded the first 30-second observation window and was
confirmed by a subsequent observation. Other browser modes and native gameplay
on each platform were not exercised by this smoke test.

Native UI catalogue 15 uses the same archive with `InstallType=Zip`,
`ShortcutExe=sc3demo.exe`, Wine 11.0, and GDI rendering. The launchers locate the
executable inside `SC3/`. The hosted legacy catalogue and homepage demo catalogue
also reference this archive.

### 3DMark2001 SE

Install the cached `3DMark2001SE.exe` (SE build 330) through native Boxedwine into
a fresh writable root, using the current full Wine filesystem. Keep the default
`C:\Program Files\MadOnion.com\3DMark2001 SE` destination. Allow the installer's
texture conversion to finish, select **Purchase Later** for the free edition,
disable **View Readme**, and finish setup before packaging.

From WSL, package that writable root:

```bash
python3 tools/jenkins/prepare_3dmark2001_demo.py \
    tmp/3dmark2001-demo/installed-root \
    tmp/3dmark2001-demo/3dmark2001se.zip \
    --report tmp/3dmark2001-demo/package.json
python3 -m unittest discover -s tools/jenkins -p test_prepare_3dmark2001_demo.py
```

The helper retains the installed application, converted textures, help and local
runtime DLLs. It adds `C:\3dmark2001se.bat` to set the working directory before
launching. Wine/Linux libraries, global registry hives, installer caches, logs
and saved benchmark results are excluded. Output ZIP and report paths must be
new; retain the report's file hashes with the ZIP.

Copy `3dmark2001se.zip` into the site's `demos/apps/` and merge only its entry
from `demos.example.json` into that directory's `demos.json`, preserving the
other demos. The generator detects it as an overlay, like `c3.zip`.
Keep `urlParams.resolution` at `1024x768` and `urlParams.bpp` at `32` for both
local and build demo links. 3DMark's default benchmark mode is 1024x768x32,
and the WebGL backend advertises only the current desktop mode. Starting at
640x480 makes 3DMark fall back to 640x480x16, even with a 32-bit desktop.
Place the demo thumbnail at `demos/apps/images/3dmark2001se.png` as well.
Validate startup and a rendered scene with the web filesystem and a fresh game
profile before publishing. Updating `demos.example.json` alone does not deploy assets.

For Jenkins, put the same ZIP and merged manifest in the configured
`BUILD_SITE_REMOTE` site's `demos/apps/`, or supply a complete demo asset directory
via `BUILD_SITE_DEMOS_SOURCE`. The next successful publish generates all four
launch modes and preserves the ZIP with the build. Local regeneration can reuse
the existing runtimes with `BUILD_RESULT=SUCCESS tools/jenkins/local-build-site.sh
--skip-sync --skip-build --no-server`; `SUCCESS` is required to generate demo links.

The 2026-10-30 smoke test with the v23 web filesystem and the existing MT JIT
runtime reached the main menu without importing registry hives. The cinematic
**Demo** reported that render-to-texture support was unavailable. **Benchmark**
reached the Car Chase loading screen, then remained black during the smoke test;
a rendered scene and complete benchmark run have not been validated.

### 3DMark03

`3dmark03.zip` contains the 284 original application files and eight original
SystemInfo files extracted from the UI sample's `3DMark03_v360_1901.exe`.
The installer SHA-256 is
`86d7f73747944c553e47e6ab5a74138e8bbca07fab8216ae70a61ac7f9a1c468`.
The browser package is 179,131,181 bytes, SHA-256
`bf683016cded343b3dc18243f326a35ffab9e1468348e705ca1f52f29be08c71`.
It contains no global Wine registry, saves, installer cache, or modified
application binaries. The native UI sample still uses the original installer.

The launcher `c:/3dmark03.bat` registers the bundled 32-bit `FMSI.dll` and
`FMSIX.dll` before starting `3DMark03.exe` from its application directory.
These are the original SystemInfo components supplied separately inside the
installer; omitting them leaves startup waiting at the splash screen. No
graphics override, application patch, or filesystem update is required.
The starting desktop is 1024x768 at 32-bit with the existing v37 web filesystem.

Set `urlParams.extract` to `"1"` for this demo. The shared browser launcher
extracts the complete application archive before starting Wine and does not
keep the demo ZIP mounted. The Wine root ZIP stays compressed. In particular,
this avoids repeatedly inflating the 449 MiB `3DMark03.dat` when Mother Nature
seeks through it. No change to the distributed demo ZIP is needed.

The shared Jenkins assets are `demos/apps/3dmark03.zip`, its entry in
`demos/apps/demos.json`, and `demos/apps/images/3dmark03.png`. The publisher picks
them up on the next successful build, using the shared archive and versioned
filesystem URLs. Do not copy them into existing published build directories.
The local demo index exposes all four runtime choices. The homepage catalogue
adds a Benchmarks entry using ST JIT and the same archive; its four featured
homepage games are unchanged.

Browser ST JIT testing verified startup, mouse input, animated Wings of Fury
rendering, returning to the menu after stopping the benchmark, and application
exit with Wine cleanup. The thumbnail is captured from the browser test.
With `extract=1`, a fresh local Chrome test verified all 293 extracted members,
the original DAT checksum, and a complete Mother Nature run returning to the
results dialog. Its first terrain draw took 67 seconds, compared with 256
seconds in the earlier compressed-ZIP baseline on the same machine. These
times exclude download and Wine startup. A full benchmark run, the remaining
scenes, and the other three browser runtime modes have not been validated.

## Disc demo media

The shared browser shell accepts `cdrom=e:filename.iso` and `cdromBase`.
For Jenkins demos, `cdromBase=../../../../apps/` resolves the disc from the
shared `demos/apps/` directory. The local generator copies ISO media into that
directory when its input directory differs, without making per-build copies.
Publication uses existing server assets; adding a disc is a separate upload.

## System Shock 2 demo

`SystemShock2Demo-r2.zip` is a portable archive of the 37 original game files from
the supplied `SHOCK2.EXE` demo's `shock/` directory. It launches `shock2.exe` at
`resolution=640x480&bpp=32`, with the normal WebGL renderer and no GDI preset or
registry overlay. The game chooses a 16-bit mode; keep the desktop at 32-bit.
Use filesystem 35 or newer for WebGL's 16-bit mode enumeration. Choose New Game,
a difficulty, and continue through the introduction to enter training.

The local and published filesystem-35 snapshots retain the existing four
browser runtimes from `james/web_gdi/5`. Each has a new graphics identity pinned
to the final v35 web ZIP. Older build pages retain their original root ZIPs.
The homepage uses the same filesystem and ST JIT only. Catalog 16 adds the same
portable payload and the original game's icon to the native UIs.

The November 14 correction adds the empty `current/` directory created by the
original installer. Without it, leaving training crashes at `shock2.exe+0x15b80c`
when saving `current/CAMPAIGN`. All 37 game files remain byte-for-byte identical
to the source download. The revised browser ZIP is 72,624,275 bytes, SHA-256
`53f49d1e8dd746216e8374a38b9dc8f0d37cade594561521aa0cfe564b38b9da`.
The user confirmed the previously failing transition works with the directory.

Catalog 44 replaces the native portable recipe with the original setup files in
`SystemShock2DemoSetup.zip`; native UIs launch `setup.exe` with Windows 98 selected
and select `shock2.exe` after installation. Browser overlays must not be reused
as native installer packages. See the [source audit](../../docs/demo-installer-audit-20261114.md).

The local and published `system_shock2__demo/20261114` snapshots reuse the runtime
and filesystem bytes from `links2003__demo/20261114`. Homepage ST JIT and the
current Jenkins/local demo lists select the revised browser ZIP. Demo discovery
scans ZIP files in the active source folder; it has no per-entry visibility flag.
Keep superseded packages outside that folder to avoid duplicate cards. The local
obsolete ZIP has been moved to test artifacts. The hosted original remains
unchanged and needs separate cleanup before publishing with the simplified
discovery code. All previously published files and four runtime ZIP links were
checked; a new browser playthrough was not repeated for that package correction.


## SimTower demo

Use the original `towerdmo.zip` demo installer (SHA-256
`409b3c1bb7614216a2708c687f9664f75539107419ba0b5c5654d70e7d410bb2`).
Install to its default `C:\SIMTDEMO` directory in a fresh Wine root, then package
only the demo and the WinG/sound support files installed into Windows:

```sh
python3 tools/jenkins/prepare_simtower_demo.py /path/to/installed/root \
  /path/to/demos/simtower-demo-overlay.zip --source /path/to/towerdmo.zip \
  --report /path/to/simtower-package.json
```

The packer pins every original file, uses deterministic ZIP metadata, and omits
Wine's global registries, caches, shortcuts, and saved towers. The browser entry
launches `c:/simtower-demo.bat` at 800x600 with a 32-bit desktop. The overlay is
needed for the WinG and sound DLLs under `windows/system`; no shared filesystem
update is needed. Follow the on-screen instructions to build a lobby and rooms.
This is a time-limited interactive tour, not the full game.

Native UI catalogues use the original installer so it can install those DLLs
in the application's Wine prefix. Keep the existing `sim-tower` catalogue ID.

Wasm JIT builds need the Win16 fixes for narrow immediate values and signal
delivery from interpreter-backed segment instructions. The November 13, 2026
`simtower__demo/20261113` snapshot includes them. Verified the ST JIT demo by
placing and expanding a lobby, building offices, advancing the tutorial, and
exiting through File > Exit with Wine cleanup observed. The installer also
completed and the interactive tour ran with the native Linux engine.

## Even More Contraptions demo

The Incredible Machine: Even More Contraptions Demo (2001) is distributed as
`EvenMoreContraptionsDemoSetup.zip` for the native UI catalogues. This ZIP retains
all 11 original installer files from the supplied `Downloads/demo/Contraptions`
directory. Its SHA-256 is
`b9a1d7ce1c873252934128ded1a6aaaad8a1cd96138101bb82bf17468f30fbd5`
and its size is 19,234,536 bytes. Run `Setup.exe`, keep the default destination,
and select `EvenMore Demo.exe` after setup. The catalogue uses the original
executable icon, Wine 11.0, GDI rendering and an 800x600, 16-bit desktop.

The preparation tool used for this release verifies both the original installer and the complete installed game
tree. It packages seven unchanged files, including the empty original INI, with
deterministic ZIP metadata. No Wine registry, system DLLs, installer cache or
user saves are included; no overlay or registry launcher is needed. The browser
ZIP is 17,452,540 bytes, SHA-256
`98a0d1876db90453b0c3f9f17effad605384ac5edc7e7d890da25068596b26ac`.

The browser entry launches `EvenMore Demo.exe` with `registry=gdi`,
`resolution=800x600` and `bpp=16`. Click through the opening demo screens and
choose **Play Contraptions**. A 32-bit starting desktop produced a blank screen
in this test; the 16-bit preset displayed correctly. ST JIT testing reached the
Mice & Cheese tutorial, placed cheese, ran/stopped the simulation, returned to
the menu and observed a running audio context with nonzero stereo samples.
Clean application exit was not confirmed during that smoke test.

The local and published `contraptions__demo/20261113` snapshots retain the
existing SimTower-tested runtime bytes, filesystem 35 and graphics provenance.
Other engine modes retain their links but were not retested for this demo.
The homepage uses the same package with ST JIT and an actual gameplay thumbnail.
All 48 new or changed published files were downloaded and SHA-256 verified.

## Force Commander demo

The supplied `ForceCom/FocomDemo.exe` is the original 2000 demo self-extractor,
SHA-256 `18a90e3856e907d570174e95727dc2894a2c09057dfac86c299d5bf010fd2b56`.
Its original 16-bit InstallShield launcher stalled at 100% in the tested native
Windows and Linux environments. The bundled 32-bit setup engine, unpacked by
setup as `_INS0432._MP`, completed installation when copied unchanged to
`INSTALL/SETUP32.EXE` beside the original installer files. Its SHA-256 is
`12a2929a615924e212490f4ef3a1cbb36ecebddafea0f358991a0fc57b7cab84`.
The native UI installer ZIP preserves all 68 extracted files plus that engine.
It installs normally; native UIs do not receive a preinstalled game.

Install into an isolated prefix and retain the 62 pristine installed files under
`C:\Program Files\LucasArts\Force Commander Demo`, excluding `Install/DeIsL1.isu`.
Keep the original `Resource/Options/FoCom.opt`; exclude user settings and saves.
The preparation tool used for this release checks the self-extractor, all original installer file hashes, the
32-bit setup engine and the complete pristine game tree. Both ZIPs have
deterministic metadata. The native `ForceCommanderDemoInstaller.zip` is
81,610,386 bytes, SHA-256
`21dd032e4e6663dacfa0f5d55d747271aaa82496ba94fecf29400280d296c803`.
The browser `force-commander-demo-overlay.zip` is 80,798,787 bytes, SHA-256
`3954b25c518d36d9e8bd9987ae7c8ca256b231a85169f505ee8b388285b955c1`.
It contains the 62 game files and a launcher importing only Force Commander's
installation registry entries. No global Wine registries, system DLLs, installer
caches or user saves are included.

The browser runs `c:/force-commander-demo.bat` with an 800x600, 32-bit desktop and
normal Direct3D rendering. Choose **Single Player**, then **Proceed** after the
briefing; Escape skips the briefing and dismisses mission objectives. The game
uses relative mouse input; use the player's **Capture mouse** control when needed.
ST JIT testing reached Assault on Ruul, moved the camera and selected units.
Native Windows testing reached the briefing and produced stereo PCM audio.
A complete mission and the other browser engine modes were not tested.

The local and published `force_commander__demo/20261113` snapshots retain the
SimTower-tested runtime bytes, filesystem 35 and graphics provenance. Jenkins
mirrors the published `demos/apps` payload and manifest on subsequent builds.
The homepage uses ST JIT, the same overlay and an actual battlefield thumbnail.
No engine or filesystem change is required for this demo.
All 53 new or changed public files were downloaded and SHA-256 verified after
publication. The generated homepage grid and all three theme previews include
the new entry; both Jenkins/local indexes have the four expected runtime links.

## Steel Soldiers demo

The native `SteelSoldiersDemoSetup.zip` preserves the original files from the
supplied `SteelSoldiers` installer directory. It is 58,434,428 bytes, SHA-256
`4a0668a43002cbf3c6fd559fdbca1d0e1b4c418db44f287206ab41a6dffa6de6`.
Run `Setup.exe` in a fresh Wine 11.0 prefix. Native catalogues use this installer
and select `Demo/demo.exe` after installation, without a preinstalled prefix.

The preparation tool used for this release verifies the installer hash and all 1,344 installed files, rejects
modified game data and user saves, and adds a working-directory launcher. No
global Wine registry, system DLLs, installer cache or game-specific registry
entries are needed. The deterministic browser overlay is 55,945,555 bytes,
SHA-256 `ef0a9be2e576733c3dd63cdb6c395904c6297c9e891ff5ada07815e1fdfca856`.

The browser runs `c:/steel-soldiers-demo.bat` with an 800x600, 32-bit starting
desktop. Choose **Single Player**, then **A Surprise Party**. Escape skips the
opening camera tour; Home centers the camera on your base. The game renders
with Direct3D. Its separate `zsetup.exe` utility requires the absent MFC42.DLL;
the game continues with default graphics settings. This limitation is noted in
the native catalogue description.

Native Windows testing completed setup, reached the mission, selected a unit
and confirmed nonzero stereo PCM audio. ST JIT testing reached the mission,
selected units, moved a jeep, issued the scatter command and changed the camera
zoom. A complete mission and the other browser engine modes were not tested.
The native renderer emits repeated WineD3D light-state warnings; the browser
test did not emit those warnings. Retain this distinction when investigating
native rendering or performance.

The local and published `steel_soldiers__demo/20261113` snapshots retain the
existing runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the
published payload and manifest. The homepage uses ST JIT with a gameplay
thumbnail. No engine or filesystem change is required for this demo.

## World of Goo demo

The native `WorldOfGooDemoSetup.zip` contains the supplied original
`Demos.WorldOfGoo.exe`, unchanged. The ZIP is 32,801,046 bytes, SHA-256
`e65bae7079fc7c53abca9e9217b9fad29a137acd6195492b0509b86b270fe2e0`.
Run the original installer in a fresh Wine 11.0 prefix. Native catalogues use
this installer and select `WorldOfGoo.exe` after installation.

The preparation tool used for this release verifies the installer and all 1,208 installed game files, rejects
modified files, and adds a working-directory launcher. The package excludes
Wine's global registry, installer caches and user-created saves. No additional
registry entries or system DLLs are required. The deterministic browser overlay
is 34,025,542 bytes, SHA-256
`997027fe99a60a28be11994f0113173d255f7dd40118322ee53220eee33f1ae2`.

The browser runs `c:/world-of-goo-demo.bat` with an 800x600, 32-bit desktop.
Create a profile, select Chapter 1, then drag goo balls into connected structures.
This original 2008 build uses Direct3D. Native Windows testing completed setup,
reached interactive puzzles, and confirmed nonzero stereo PCM audio. Browser
ST JIT testing reached the first puzzle and connected goo balls. A complete
playthrough, native Mac/Linux gameplay and the other browser modes were not
tested.

The local and published `world_of_goo__demo/20261113` snapshots retain the existing
runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the published
payload and manifest. The homepage uses ST JIT and a gameplay screenshot.
No engine or filesystem change is required for World of Goo.

## The Settlers IV demo

The native `SettlersIVDemoSetup.zip` contains the supplied `thesettlers4_demo.exe`
and `readme.txt`, unchanged. It is 99,893,156 bytes, SHA-256
`753dd49c6fd643c33adc8844401f96f2c2922faae5e4ad70335005dccd268296`.
The original installer completed in a fresh Wine 11.0 prefix; optional Acrobat
and Blue Byte Game Channel were skipped. Native catalogues run that installer
and select `S4.exe` afterward, without a preinstalled game or Wine prefix.

The preparation tool used for this release verifies the source installer hash and all 86 retained game files.
It excludes generated game settings, runtime logs, user saves and the uninstall
log. It adds a working-directory launcher and only the game's installation key,
`HKLM\Software\BlueByte\Settlers4-Demo` (path, program, language and version).
Wine's global registries are excluded. The deterministic browser overlay is
86,768,767 bytes, SHA-256
`5c22061eaf98045bb97e0b3f67c6e1d055dd573d83b2c59a493462f707e84805`.

The browser runs `c:/settlers4-demo.bat` with an 800x600, 32-bit starting desktop.
The game uses DirectDraw menus and Direct3D landscape rendering with its default
settings. Click through the two opening screens, then choose **Tutorial** and
**Tutorial 1**. Space advances the instructions; right-drag moves the camera and
Home returns to the settlement. No GDI preset or extra DLL override is required.

Native Windows testing reached the first tutorial and produced nonzero stereo
PCM audio. ST JIT testing verified tutorial loading, animated settlers, keyboard
input and camera movement. A complete mission, multiplayer, native Mac/Linux
gameplay and the other browser modes were not tested.

The local and published `settlers4__demo/20261113` snapshots retain the existing
runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the published
payload and manifest. The homepage uses ST JIT with a gameplay screenshot.
No engine or filesystem change is required for this demo.

## The Movies demo

Native UI catalogues use `TheMoviesDemoSetup.zip`, containing the 130 original
setup-media files extracted from the supplied `The_Movies_Demo.exe`. The outer
self-extractor fails with `c0000018` in the tested native build; running the
unchanged extracted `setup.exe` completes installation. The native package is
299,295,796 bytes, SHA-256
`b2a49aa41711433b97e668d22e3c84e8e7ce38aa139b2893622253138f0dfe9f`.
The original EXE is 299,440,640 bytes, SHA-256
`c3f0e8bf5b369495092be11714b28bace464f8dfa6c9d067f8898771719c7860`.
Native UIs launch `setup.exe`, then select `MoviesDemo.exe`; they do not receive
a preinstalled game. The recipe uses Wine 11.0 and an 800x600, 32-bit desktop.
The game chooses its own resolution after hardware detection.

Native testing reached the studio tutorial, with nonzero stereo PCM audio, and
the user independently started a game successfully. The application's original
icon and the supplied demo readme's requirements are included in catalogue 40.

The preparation tool used for this release preserves all 123 original game files and adds a launcher plus only
the game's installation path, version and language registry values. It excludes
Wine's global registry and installer files. The reproduced overlay is
288,652,228 bytes, SHA-256
`25468f60c20c41f53d5800eb4555ae41ddbefa88e9acd2b369f44f552546a8cb`.
Browser ST JIT reaches the studio tutorial with filesystem 35. The user verified
mouse capture in Chrome; gameplay and camera movement were observed there.
Choose Game, then Tutorial, and capture the mouse for the game controls. The
in-app browser could render the menu but its mouse capture did not work in this
test. This was not reproduced in Chrome. Browser audio PCM, a full tutorial, and
other browser modes have not been tested.

The local and published `the-movies__demo/20261113` snapshots retain the existing
runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the overlay
and manifest. The homepage uses ST JIT and a screenshot of the studio tutorial.
No engine or filesystem change is needed for this demo.

## Tropico demo

Native UI catalogues use `TropicoDemoSetup.zip`, containing the nine supplied
setup files unchanged (180,848,170 bytes, SHA-256
`110bc8a7caf63ec4074403e34f429423a9fc75acd2238e1f96cf11d05a1e2fe0`).
They run `Setup.exe`, then select `TropicoD.EXE`. They do not contain an installed
game or Wine prefix. The original installer completed in an isolated Wine 11.0
prefix. The user confirmed gameplay and the corrected native cursor.

The preparation tool used for this release verifies all 499 original game files and adds only a working-directory
launcher. It does not import Wine's registry or add system DLLs. Changed settings
or user saves fail its pristine-tree check. The reproducible overlay is
177,740,335 bytes, SHA-256
`7ce0cae800790efa15fa7fdcb11a5e42168734740fa932dbd7f9d9d484332e6a`.
The browser runs `c:/tropico-demo.bat` from an 800x600, 32-bit starting desktop.
Choose Tutorial after the introduction. The tutorial continues as a 50-year game.

Native gameplay and audio were checked; ST JIT reaches the tutorial with animated
citizens and working mouse input. Other browser modes and native Mac/Linux
gameplay have not been tested. The local/published `tropico__demo/20261113`
snapshots retain the existing runtimes, filesystem 35 and graphics provenance.
The browser shell uses its existing visible host pointer; it does not yet carry
the native cursor fix. The homepage adds the ST JIT demo and a gameplay image.

The native missing cursor required a shared engine fix: advertise depth-1 pixmap
storage to Wine and render monochrome cursor pixels and their transparency mask.
No Tropico preset or Wine filesystem change is needed. See
[the cursor regression notes](../fs_passthrough/x11/tests/README.md#monochrome-cursors).

## MechCommander 2 demo

The native catalogues use the original `MC2Trial.exe` installer and `Readme.rtf`
from `https://boxedwine.org/demos/MechCommander2DemoSetup.zip` (163,257,393 bytes,
SHA-256 `40b0b831c88926733231414dc1707a585a3255ddc545fc341cd8693f967245fe`).
Catalogue 41 selects Wine 11.0, an 800x600 32-bit desktop and the demo's bundled
`dbghelp` / `imagehlp` DLLs. It installs normally into a new application root.

The browser overlay requires filesystem 36's shared DirectPlay startup fixes.
The preparation tool used for this release checks the pinned installer and pristine 261-file game tree, preserves
the original fonts, and adds installation registry entries and a launcher. It
uses the demo's unmodified `minprefs1.cfg` for both graphics preference files,
matching the preset selected by the browser hardware detector. This avoids
rerunning that first-launch benchmark in the preinstalled browser demo. It does
not include the detector's machine measurements, user saves, global Wine
registries, accepted-license state or experimental Wine DLLs.
The reproducible overlay is 168,076,096 bytes, SHA-256
`b952c7994c92e23c2df5766ac5acab894f510169607cabcfc1b08d3f5ecc69b6`.

The entry runs `c:/mechcommander2-demo.bat`. Native single-player gameplay was
tested, and the user confirmed the ST JIT browser campaign works in external
Chrome. Its HUD, unit controls and completed first objective were captured.
The automated in-app browser stalled during the introduction; that result was
not treated as proof of a game bug. Multiplayer and native macOS/Linux gameplay
have not been verified. See the DirectPlay probe notes for the fix's scope.

The `filesystem__36/20261114` local and published snapshots retain all four
existing SimTower-tested engines with new filesystem/build identities. Jenkins
mirrors the published overlay and manifest on subsequent builds. The homepage
uses only ST JIT and keeps the console collapsed.

## Pharaoh demo

The native UI payload `https://boxedwine.org/demos/PharaohDemoSetup.zip` contains
the original files from the supplied `PHARAOH` demo installer folder, unchanged.
It is 77,327,746 bytes, SHA-256
`b3daff52ea8557a29cec2464eaa20c80c9b7b32e0d8edb766129aa7a31172195`.
Run `SETUP.EXE` in a fresh Wine root and keep the default
`C:\SIERRA\PharaohDemo` destination. Skip bookmarks and choose to restart later.
The preparation tool used for this release checks the installer hash and the complete pristine game-file tree.
It preserves the 307 installed demo files, including the supplied tutorial saves,
and adds a launcher with only the game-specific installation registry entries.
It excludes Wine's global registries, installer caches and uninstall log, and
rejects modified files or added user saves. The deterministic overlay is
60,649,304 bytes, SHA-256
`5c6d75f1791922336a31f286001813dd11e393281060f64840c7b0b1b0a62645`.

The browser entry launches `c:/pharaoh-demo.bat` with `registry=gdi,win98` at
800x600 and a 32-bit desktop. Windows 98 compatibility is required for audio:
this demo skips sound initialization when Windows reports an NT platform.
Native PCM capture and the browser AudioWorklet output verified sound with this
setting. Native catalogue 19 also selects Windows 98.
The local and published `pharaoh__demo/2026111303` snapshots retain
the previously tested SimTower runtime and filesystem 35; no engine or filesystem
change is needed. Jenkins mirrors the published `demos/apps` payload and manifest
on subsequent builds. Native catalogues use the installer, not this overlay.

Validated setup with the native Linux engine, then the browser ST JIT build by
entering the first mission, moving around the map, placing houses and observing
settlers move in. Exit and Wine cleanup completed after clicking through the
demo's two closing promotional screens. The homepage uses this ST JIT runtime;
Jenkins exposes the usual four runtime choices.


### Escape from Monkey Island demo

`monkey-island-demo-overlay.zip` contains 48 pristine installed game files and a minimal installation registry/launcher. The preparation tool used for this release pins both original installer ZIP and installed tree; it excludes the machine-specific `Install/DeIsL1.isu` uninstaller log, global Wine state, installer caches and filesystem metadata. Do not use a game directory modified during play.

The native package remains the original installer, not the overlay. The browser BAT runs `Monkey4.exe -cd` from its installed directory. A process trace of the original `MnkyDemo.exe` launcher confirmed this argument; omitting it produced a blank blue screen. Direct3D is the default, so no OpenGL override or game-specific Wine replacement is needed. Filesystem 36 is unchanged.

Overlay: 118,724,126 bytes, SHA-256 `085966301623c444a7039a0696693801edfb4e119e31d4328556a576e8686a17`. Native setup completed in a fresh prefix, and the first scene rendered with sound. ST JIT testing reached the same scene, moved Guybrush, opened a conversation and selected a dialogue line. AudioWorklet reported nonzero stereo output. This original demo has no mouse control or save/load support.


### Links 2003 demo

`links2003-demo-overlay.zip` contains 110 pristine installed game files, seven
original player/MOP files from setup, and the installation registry and launcher.
The package was built from a fresh installation and the original `MOP` and
`Players` setup directories, with installer, game and support files verified
by checksum. It excludes global Wine state, installer caches, filesystem
metadata and player-created saves. `Sergio.PIN` is installed as `Sergio Garcia.PIN`
with its original bytes. The native UI package remains the original installer,
not the installed browser overlay.

The browser launcher imports the game registry on first launch and runs
`LinksMMIII.exe`. Both browser and native recipes disable Wine's `wmvcore` for
this game. Without that setting, Wine 11.0 filesystem 36 fails during startup in
the original `WMA.dll` at offset 0x1004 after unsupported media-reader interface
requests. No game executable or shared Wine filesystem was patched.

Overlay: 176,328,407 bytes, SHA-256
`ac4c556ceb0d1995241aa6378150245f2a1399332e66b512563764f22ba2178f`.
Packaging verification reproduced the tested overlay byte for byte. Native
new-game play and ball-hit sounds were confirmed by the user. A fresh ST JIT
browser test on filesystem 36 reached the swing tutorial with spoken
instructions. Other music/commentary, online play, and all nine holes have not
been comprehensively tested. The website thumbnail shows Sergio Garcia at the
Skeleton Coast tee, captured from native Boxedwine on 2026-11-14. Jenkins, local
demos and the homepage use the same gameplay image.

### Galactic Battlegrounds trial

`galactic-battlegrounds-trial-overlay.zip` contains all 184 pristine installed
files from the original `SWGB_Trial.exe`, including the empty `Game/SaveGame/`
directory. File hashes were checked against a snapshot taken before the first
game launch. The package adds only a BAT launcher and the game's installation
and DirectPlay registry entries. It excludes global Wine state, installer caches
and player-created profiles or saves. The native UIs use the original installer.

Overlay: 69,984,742 bytes, SHA-256
`dad7dad7edbe70b08dcaa762404e7e01b48183c8588c6e9c4e7c3648e7266a1d`.
The browser starts `c:/galactic-battlegrounds-trial.bat` at 1024x768, 32-bit, with
`registry=gdi`. That preset sets both Wine renderer keys and disables WebGL;
setting only `DirectDrawRenderer` is insufficient with Wine 11. GDI also fixes
the New Player dialog's mixed Windows controls and DirectDraw rendering.

ST JIT testing on filesystem 39 created a player, entered the Darth Vader
campaign's Incursion on Yavin IV mission, selected troops and moved them.
AudioWorklet output contained nonzero audio with no queue failures. The user
also confirmed native gameplay. Multiplayer and native macOS/Linux gameplay
are untested. The web thumbnail is the native gameplay scene selected by the
user on November 15, 2026. Jenkins, the local demo site and the homepage demo
catalogue use that image. No engine or filesystem change is required.
