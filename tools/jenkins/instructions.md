# Local Build Site

`local-build-site.sh` builds a local copy of the Boxedwine web demo site so Emscripten builds can be tested in a browser before publishing anything.

The script mirrors the existing public build site, builds fresh single-threaded, multi-threaded, single-threaded JIT, and multi-threaded JIT Emscripten outputs, places those outputs into the same layout used by the published site, regenerates the demo pages, and starts a local static server with the headers required by the multi-threaded builds.

## What It Does

1. Syncs an existing build site into a local folder.
   - If `BUILD_SITE_REMOTE` is set, it uses `rsync` from that remote.
   - Otherwise it falls back to `wget` from `https://boxedwine.org/builds/`.
2. Hydrates demo assets for local use.
   - Demo app ZIPs and `demos.json` are refreshed when the public files are newer; unchanged local files are retained.
   - `boxedwine.zip` is always downloaded from `http://boxedwine.org/v2/demos/boxedwine.1.zip`.
3. Builds the Emscripten web targets from `project/emscripten`.
   - `make release`
   - `make multiThreaded`
   - `make jit`
   - `make multiThreadedJit`
4. Copies the web build outputs into the existing demo runner layout:
   - `project/emscripten/Deploy/Web/SingleThreaded`
   - `project/emscripten/Deploy/Web/MultiThreaded`
   - `project/emscripten/Deploy/Web/SingleThreadedJit`
   - `project/emscripten/Deploy/Web/MultiThreadedJit`
5. Runs `tools/jenkins/build_site.py` against the local site directory.
6. Starts `project/emscripten/server.mjs` with:
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

For consistency with the local workflow, the publish script also downloads:

```text
http://boxedwine.org/v2/demos/boxedwine.1.zip
```

and writes it as:

```text
demos/apps/boxedwine.zip
```
