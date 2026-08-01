# Emscripten Build

There are 2 builds for the Web
- Single Threaded. Target Release
- Multi-Threaded. Target multiThreaded


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


## Running The Multi-Threaded OpenGL Bootstrap Regression

This browser-only target verifies that supported WebGL procedures are
available after SDL creates the window but before Wine creates its first GL
context:

```bash
make testMultiThreadedOpenGL
node server.mjs --root Build/TestMultiThreadedOpenGL --port 8001
```

Then open:

```text
http://127.0.0.1:8001/boxedwine.html?0&1&1
```

The one-test run must report `0 tests FAILED`.


