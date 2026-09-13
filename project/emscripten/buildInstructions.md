# Emscripten Build

The four browser targets are `release`, `jit`, `multiThreaded`, and
`multiThreadedJit`. Each target retains its own object directory.

The configuration dependency records compiler paths/versions, make flags,
`EMCC_CFLAGS`, and `SOURCE_DATE_EPOCH`. Changing or removing either environment
setting recompiles and relinks; unchanged settings preserve the existing
objects and outputs. Keep a fixed `SOURCE_DATE_EPOCH` when comparing cold and
incremental builds, since the executable embeds `__DATE__` and `__TIME__`.


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

## WASM JIT cache compatibility

The current cache version is `v6`. Record new cache ZIPs with this build;
`v5` modules embed an older CPU layout and are rejected. Keep the cache version
in `boxedwine-shell.js`, `jitWasmCodeGen.cpp`, and the offline cache pipeline
in sync whenever generated modules become incompatible with the runtime.

Run `node testJitCacheVersion.mjs` to check old-cache rejection, current-cache
imports in ST/MT loaders, and flat-to-grouped pipeline round trips. The test
uses `binaryen_js.js` from this directory, or accepts its path as an argument.

## Reusing offline optimizer output

The offline pipeline can retain Binaryen's per-module output between runs:

```text
node boxedwine-wasm-jit-cache-pipeline.mjs --optimizer-cache-dir ../../tmp/jit-optimizer-cache input.zip output.zip
```

This cache is separate from the guest JIT cache version and runtime compatibility
checks. Both flat and grouped pipelines can reuse it. Keys include the input WASM
bytes, optimizer JS and assets requested through `locateFile`, pipeline/cache
implementation, pass order and settings, and Node version. Use a self-contained
Binaryen bundle such as the supplied `binaryen_js.js`; arbitrary wrapper modules
with additional imports are not covered by this dependency tracking.

Each entry stores a hash of its output. Damaged entries are recomputed; writes
publish a complete record atomically. The cache reports hits, misses, and invalid
entries. Omit `--optimizer-cache-dir` for an uncached control. The pipeline also
reports optimizer loading/identity time and per-module optimization/cache time
as `OPTIMIZER_TIMING` records. These are offline build timings, not game frame times.
ZIP manifests still contain generation timestamps, so compare WASM payloads and
manifest content excluding `generatedAt` when validating cached output.

Lightweight cache and integration checks use a fake optimizer:

```text
node testOptimizerCache.mjs
node testOptimizerPipeline.mjs
```

Validate the actual bundled optimizer separately from browser/compiler work:

```text
node validateOptimizerCache.mjs ../../tmp/optimizer-validation
```

The output directory must be new. An optional second argument selects a
self-contained Binaryen bundle. The validator keeps every input, log, output
and timing sample. It checks cold misses and warm hits, compares flat/grouped
WASM and manifests (excluding generation timestamps), and executes all 64
synthetic memory-reading exports after each of eleven pipeline runs. The
uncached control must change the input bytes, so a no-op optimizer cannot pass.

## Separating linker and optimizer build time

Emscripten's `EMPROFILE=1` writes structured profiling events under
`$TMPDIR/emscripten_toolchain_profiler_logs`. Use a new temporary directory
for each compile/link phase, retain the command's actual exit code and logs,
and summarize the completed trace:

```text
python3 summarizeBuildProfile.py /path/to/phase-temp/emscripten_toolchain_profiler_logs --build-exit-code 0 --output new-profile-report.json
```

Replace the example zero with the observed exit code. The report fails for
failed builds, malformed or incomplete traces, clock reversals and duplicate
process records. It preserves source hashes and refuses to overwrite a report.
Emscripten may leave a process status unrecorded at normal `atexit`; the
separately captured build status remains required.

The `link`, `binaryen` and nested `wasm_opt` blocks distinguish linker work
from WebAssembly optimization. Timings are inclusive: do not add `wasm_opt`
to its `binaryen` parent. `busy_seconds` merges overlapping intervals across
compiler processes; `process_seconds` sums durations and may exceed wall time.
Neither measures CPU self time. Retain the outer phase's wall-clock measurement
to include work outside the profiled blocks. Profiling can perturb build time;
compare emitted output hashes with the ordinary build before reusing its
runtime validation.

## Profiling the WASM JIT

Build a clean checkout with `make multiThreadedJit WASM_PROFILING=1`
(or `make jit WASM_PROFILING=1`). Keep the usual optimization flags. This
preserves Emscripten function names and adds names to generated guest blocks,
including functions remapped into runtime module groups. Switching this option
rebuilds objects through the makefile's compiler/configuration dependencies.

Generated names contain the guest process, mapped file basename, file offset,
linear x86 instruction address, and block instruction count. An unmapped block
is labeled `Unknown`; its file offset is not meaningful. The file offset is
not a PE RVA. Names add metadata bytes and can affect module batching, so use
this build to locate hotspots and validate speed changes with a normal build.

Capture short samples during sustained rendering in Chrome's Performance
panel, with CPU sampling enabled and no CPU throttling. Include pthread worker
tracks. Profile startup separately, and measure benchmark scores with DevTools
closed. The optional `BOXEDWINE_WASM_JIT_PROFILE` timing/counter instrumentation
is independent and is not enabled by `WASM_PROFILING=1`.

For logging-based diagnosis without a debugger connection, use another clean
checkout and build with:

```
make multiThreadedJit WASM_PROFILING=1 'GCC_EXTRA_FLAGS=-DBOXEDWINE_WASM_JIT_PROFILE -DBOXEDWINE_WASM_JIT_NAMES'
```

The counter macro changes `CPU` layout, so every object must be rebuilt. It
logs cumulative `[WASM JIT profile]` records about every five seconds. Compare
deltas between records from the same phase of the workload. Most helper counts
and timings are estimates from one-in-1024 sampling. Timings include nested
calls, clock overhead and worker descheduling; they are not additive CPU
self-time percentages. Some chaining fields apply only to the single-threaded
dispatcher, and `instantiate` does not cover deferred MT group installation.
Use counter builds to narrow a hypothesis, then confirm it with a short worker
CPU profile and a normal build benchmark.

Before interpreting a render profile, check
`Module.getWasmJitMtRuntimeBatchStats()` in the browser console. Installation
failures can leave hot blocks interpreted and distort the apparent balance
between guest code and emulator helpers. Worker installation errors are
forwarded through Emscripten's `printErr` handler.

For a focused regression check of heap addresses above 2 GiB, run
`node testJitHighMemory.mjs Build/MultiThreadedJit/boxedwine.js` after building
the non-test MT JIT target. This executes the emitted production group
installer with real WASM modules and memory, stubbing broker bookkeeping.
It checks compilation, table reservation, output slots, and execution with
both low and high addresses. The test reserves just over 2 GiB of shared
linear address space but touches only a few pages.


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


