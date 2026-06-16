# Emscripten Build

There are 4 builds for the Web
- Single Threaded. Target Release
- Multi-Threaded. Target multiThreaded
- Experimental release-style WasmFS/OPFS storage. Target releaseOpfs
- Experimental multi-threaded WasmFS/OPFS storage. Target multiThreadedOpfs


## Build

From `project/emscripten`:

type make and the name of the target. See contents of ./Build/<Target> folder for output


## Running Single Threaded Build
- runs in main browser thread

python3 -m http.server <port number>


## Running Multi-Threaded Build
- runs using Emscripten -pthread -sPROXY_TO_PTHREAD=1

browser cross-origin isolation headers required for `SharedArrayBuffer`

node server.mjs <port number>

alternatively make sure your web server returns COEP, COOP headers


## Running WasmFS/OPFS Build
- build with `make releaseOpfs`
- or build the Boxedwine multi-threaded variant with `make multiThreadedOpfs`
- run with `node server.mjs <port number>`
- use `?storage=opfs` or leave `storage` unset for this build

The OPFS targets use Emscripten WasmFS with `PROXY_TO_PTHREAD` and mount OPFS
from native startup before Boxedwine opens `/root`. This keeps OPFS on the
worker/sync-access-handle path. WasmFS is not generally available from
JavaScript `Module.preRun`, though `FS.createDataFile` can still queue the zip
and payload files used by `boxedwine-shell.js`.
