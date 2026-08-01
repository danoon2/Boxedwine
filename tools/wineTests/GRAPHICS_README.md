# Wine graphics tests in Emscripten and native Wine

`runWineTests.py` remains the single Wine test entry point and accepted-result
policy. Its `wineGraphicsBrowser.py` backend can run one Wine 11 DirectDraw,
D3D8, D3D9, D3DX9, or D3DXOF test group inside the single-threaded non-JIT
Emscripten build. The unified CLI exposes DirectDraw, D3D8, D3D9, D3DX9,
and D3DXOF. It can also run a stable comparison subset against a native
pure-i386 Wine 11 build. The browser backend creates a temporary flat app ZIP,
serves the selected versioned BoxedWine filesystem without copying it, and
launches a separate Chrome profile.

The runner does not modify the production `boxedwine.html` or
`boxedwine-shell.js`. Its local HTTP server injects a test-only observer into
the served HTML. The observer sends periodic copies of the BoxedWine output
text area to the runner and reports completion when the selected Wine group
prints its final test summary. A graceful `Boxedwine shutdown` is also
recognized, but is not required after an authoritative Wine summary.

## DirectDraw baseline

Start with Wine's small DirectDraw `refcount` group:

```powershell
python tools/wineTests/runWineTests.py `
  --ddraw-group refcount `
  --graphics-filesystem "$env:APPDATA\Boxedwine\FileSystems2\boxedwine.3.zip"
```

Repeat `--ddraw-group` to select more groups from the same suite. D3D8, D3D9,
D3DX9, and D3DXOF are available through `--d3d8-group`, `--d3d9-group`,
`--d3dx9-group`, and `--d3dxof-group`, respectively.

For example, run the D3D8 device group with:

```powershell
python tools/wineTests/runWineTests.py `
  --d3d8-group device `
  --graphics-filesystem "$env:APPDATA\Boxedwine\FileSystems2\boxedwine.3.zip"
```

By default the runner extracts the selected suite's executable from
`tools/wineTests/wine_tests_v6.zip`. Use `--graphics-test-executable PATH` to
override the bundled executable while developing a new Wine patch.

The defaults use:

- `project/emscripten/Deploy/Web/SingleThreaded`
- `%APPDATA%\Boxedwine\FileSystems2\boxedwine.3.zip`
- a visible isolated Chrome window
- a 1,200-second timeout

Pass `--graphics-headless` for Chrome's new headless mode. A visible run is the
primary baseline because it is closer to normal game launches.

Each invocation creates a timestamped directory below
`~/.cache/boxedwine/wineTests/runs/<timestamp>/graphics` containing:

- `manifest.json`: inputs, hashes, browser identity, result counts, and status
- `wine.log`: the complete BoxedWine and Wine output text
- `chrome.log`: Chrome process output
- `browser-payload.json`: the final browser snapshot and console tail
- `server.log`: local HTTP requests
- `input/<suite>-<group>.zip`: the exact generated app ZIP

The injected harness runs the Wine test through `/bin/sh` and attempts to stop
wineserver after the test exits. Some Emscripten Wine runs leave service
processes alive after printing the final summary, so the runner treats that
group-specific summary as authoritative and terminates only its isolated
Chrome process tree. A normal run passes only when Wine reports the exact
versioned test, todo, failure, skip, and accepted-failure identities in
`graphics-baseline-v1.json`, and the browser reports no error. The same values
are retained in the manifest.

The July 31, 2026 source-color-key checkpoint expands the `ddraw1` baseline to
19,640 assertions, 59 todo results, 0 failures, and 20 skips. Ordinary RGB
`OFFSCREENPLAIN` source-color-key blits use WineD3D's CPU fallback under WebGL,
including packed RGB formats and GPU-authoritative destinations. The test keeps
the surface result checks enabled for every A4R4G4B4 key value; only the later
A4R4G4B4 texture-sampling draw is classified as unimplemented emulation and
skipped. The final exact production-input run is `20260731-154330-658016`; candidate
single-threaded and `SingleThreadedJit` runs are `20260731-151954-785600` and
`20260731-152132-431391`.

## Supported suites

```text
ddraw:    d3d, ddraw1, ddraw2, ddraw4, ddraw7, ddrawmodes, dsurface,
          refcount, visual
d3d8:     device, stateblock, visual
d3d9:     d3d9ex, device, stateblock, visual
d3dx9_43: asm, core, effect, line, math, mesh, shader, surface, texture,
          volume, xfile
d3dxof:   d3dxof
```

The browser backend executes each selected group separately. This keeps every
browser, Wine prefix, timeout, and artifact set isolated. Exposing other
Emscripten modes should follow after the single-threaded non-JIT baselines are
reliable.

## Exact result baseline

`graphics-baseline-v1.json` covers every supported DirectDraw, D3D8, D3D9,
D3DX9, and D3DXOF group. It records exact assertion, todo, failure, and skip
counts plus accepted Wine source locations. The default runner rejects any
count change, including reduced assertion coverage, added skips, or a known
failure unexpectedly disappearing. That makes improvements visible for review
instead of silently changing what the suite accepts.

Before Chrome starts, the runner also verifies the selected root ZIP,
`boxedwine.wasm`, and suite test executable against the SHA-256 values stored
with the baseline. This prevents exact result expectations from being applied
silently to different inputs.

For an exploratory run against a newly built test executable or runtime, bypass
the exact policy explicitly:

```powershell
python tools/wineTests/runWineTests.py `
  --d3d9-group stateblock `
  --no-graphics-baseline
```

After reviewing the run's complete manifest and failure records, update the
versioned JSON deliberately and rerun without `--no-graphics-baseline`.
`--graphics-baseline PATH` can select a candidate baseline without replacing
the default file. The run manifest records the applied path, schema, baseline
ID, and baseline-file SHA-256.

## WebGL test-divergence inventory

The v6 browser executables include WebGL-specific changes to nine Wine test
source files. Those changes are physically isolated in
`webgl-tests-against-wine-11.0.patch`; none of the
build/configuration, adapter/context/capability, shader/GLSL ES,
texture-transfer, GLSL blitter/batching, DirectDraw runtime/presentation,
D3DX9 assets/compatibility, D3DXOF parser-hardening, WineD3D
draw/state/query, or D3D8/D3D9 compatibility/diagnostics production patches
contains Wine test files.
`webgl-test-divergences-v2.json` pins the ordered patch series and the official
Wine 11 source commit, then classifies all 45 unique added
`skip()` rules (69 calls) and all 14 unique added `todo_wine_if()` rules
(44 calls) into exactly one of:

- `intentional_webgl_limit`
- `unimplemented_emulation`
- `defect_to_fix`

Validate the inventory directly with:

```bash
python3 tools/wineTests/webglTestDivergences.py
```

The browser runner performs the same validation before launch and
`graphics-baseline-v1.json` pins the divergence-manifest SHA-256. It rejects a
test hunk in any production patch, a non-test hunk in the test patch, an
overlapping file, or a new/unclassified skip or todo before execution. The run
manifest records every production patch ID, category, path, hash, and file
count plus the test patch and divergence policy identity.

The first exact browser proof through the fully named production patch series
completed the D3D8 `stateblock` group's 9,283 assertions with no todo results,
failures, or skips:

```text
C:\Users\james\.cache\boxedwine\wineTests\runs\20260730-093624-044892
```

## Native pure-i386 Wine comparison

On Linux or WSL, add `--native-wine-root` to run the selected v6 PE32
executable directly in a native 32-bit Wine build instead of Chrome:

```bash
python3 tools/wineTests/runWineTests.py \
  --d3d9-group stateblock \
  --native-wine-root \
    /home/james/Boxedwine-native-buildwine/tools/buildWine/wine-git
```

The runner requires an X display, verifies that `wine`, `wine-preloader`, and
`wineserver` are ELF32/i386, and creates a fresh `WINEARCH=win32` prefix for
every group. It always asks wineserver to stop and waits for shutdown.
Successful prefixes are removed; failed and incomplete prefixes are retained
with the log.

Both in-source and out-of-tree Wine builds are supported. For an out-of-tree
build, the runner reads `srcdir` from the generated `Makefile` and records the
commit and dirty state from that source checkout while hashing runtime files
from the selected build directory.

`native-graphics-baseline-v1.json` is enabled by default in native mode. It
pins the Wine version and source commit, the native loader/server and relevant
DirectX/OpenGL runtime binaries, and all five v6 graphics-test executables.
The exact stable subset is:

```text
ddraw:     refcount
d3d8:      stateblock
d3d9:      stateblock
d3dx9_43:  core, line, math
d3dxof:    d3dxof
```

Those seven groups pass with the same assertion/todo/skip counts as WebGL,
except native D3DX9 `math` has 0 failures while WebGL has the separately
accepted `math.c:1557` failure. Exact native proof artifacts are:

- D3DX9 `core`, `line`, and `math`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184114-684499`
- DirectDraw `refcount`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184141-481489`
- D3D8 `stateblock`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184153-283257`
- D3D9 `stateblock`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184205-040643`
- D3DXOF:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184217-574844`

The patched v6 executables take their native path when the WebGL runtime is
not detected, so the same binary is compared on both sides. Under WSLg,
display-mode-changing D3D8 `device` and `visual` runs do not reach their final
Wine summaries: WSLg exposes a fixed 1920x1080 mode and reports that the
32-bit DRI3 device is unavailable. They are deliberately excluded from the
exact native baseline rather than recorded as conformance results. Other
groups remain available for investigation with
`--no-native-graphics-baseline`.

A clean official Wine 11 source checkout at commit
`db11d0fe6a169c457e23d007e20404643d067aa8` was also used to build unmodified
PE32/i386 graphics tests. Running those executables with
`--graphics-test-executable` produced an exact count match with the adapted v6
executables for all seven stable native groups above. The clean-test proof
artifacts are:

- DirectDraw `refcount`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184756-556595`
- D3D8 `stateblock`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184809-147273`
- D3D9 `stateblock`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184820-835794`
- D3DX9 `core`, `line`, and `math`:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184835-481538`
- D3DXOF:
  `/home/james/.cache/boxedwine/wineTests/runs/20260729-184903-133859`

Their executable hashes and build location are versioned in
`webgl-test-divergences-v2.json`. This proves that the selected native
comparison groups do not depend on the WebGL-only test branches; browser
groups that exercise those branches still use the classified adapted tests.

### Patched Wine WebGL-off audit

On July 30, 2026 the complete production patch series was built as native
pure-i386 Wine at:

```text
/home/james/webgl/boxedwine-webgl-wine-build/wine-native32-webgl-off
```

With `WINE_D3D_CONFIG=webgl=0,webgl_glsl_es=0`, all seven stable groups matched
the pinned upstream counts exactly:

| Group | Assertions | Todo | Failures | Skipped | Artifact |
| --- | ---: | ---: | ---: | ---: | --- |
| DirectDraw `refcount` | 96 | 10 | 0 | 0 | `20260730-193847-867225` |
| D3D8 `stateblock` | 9,283 | 0 | 0 | 0 | `20260730-193900-791853` |
| D3D9 `stateblock` | 14,738 | 0 | 0 | 0 | `20260730-193914-527956` |
| D3DX9 `core` | 1,485 | 99 | 0 | 0 | `20260730-194510-532953` |
| D3DX9 `line` | 23 | 0 | 0 | 0 | `20260730-193927-532582` |
| D3DX9 `math` | 32,509 | 0 | 0 | 0 | `20260730-193927-532582` |
| D3DXOF `d3dxof` | 192 | 0 | 0 | 0 | `20260730-194003-119040` |

The artifact IDs above are directories below
`/home/james/.cache/boxedwine/wineTests/runs/`. The temporary native build uses
the 32-bit cross-architecture pkg-config metadata so D3DX font creation is
covered by the `core` group.

The audit then changed `wined3d_settings.webgl` and
`wined3d_settings.webgl_glsl_es` to default to false. With
`WINE_D3D_CONFIG` completely unset, DirectDraw `refcount`, D3D8 `stateblock`,
and D3D9 `stateblock` again passed with the exact counts above:

- DirectDraw: `20260730-194612-311594`
- D3D8: `20260730-194623-533377`
- D3D9: `20260730-194635-460075`

The Emscripten launcher continues to set
`WINE_D3D_CONFIG=webgl=1,webgl_glsl_es=1`. A temporary copy of
`boxedwine.3.zip` containing the rebuilt DLLs passed the browser DirectDraw
`refcount` group with 96 assertions, 10 todo results, and no failure or skip:

```text
/home/james/.cache/boxedwine/wineTests/runs/20260730-195319-148286
```

After that temporary-root proof, the rebuilt PE32/i386 DLLs were promoted to
the production v3/v10 full filesystems in place. `ddraw.dll` is 867,870 bytes
with SHA-256
`de922af65811c0a339eb16f318e7f71b58f73f2f421446b56e4bde603b6e236b`;
`wined3d.dll` is 3,982,469 bytes with SHA-256
`40e06c9d00981e687645073f6cd8a0eaace8c42e5e5ed0891768f44154947725`.
The real promoted `boxedwine.3.zip` then passed the exact browser baseline for
DirectDraw `refcount` with the same 96 assertions, 10 todo results, and no
failure or skip:

```text
/home/james/.cache/boxedwine/wineTests/runs/20260730-201206-240999
```

## D3D8 baseline

All three D3D8 groups passed in visible Chrome on July 29, 2026 with
`boxedwine.3.zip` and the packaged `wine_tests_v6.zip`:

| Group | Assertions | Todo | Failures | Skipped | Duration |
| --- | ---: | ---: | ---: | ---: | ---: |
| `device` | 54,926 | 36 | 0 | 13 | 84.954 s |
| `stateblock` | 9,283 | 0 | 0 | 0 | 12.937 s |
| `visual` | 20,712 | 17 | 0 | 15 | 536.954 s |

No D3D8 failures are accepted by the runner. The test executable is PE32/i386
with SHA-256
`c132f99ff3bd8547d10412a14aed127233ff001218ba5a6f81c702287d81f81f`.

## D3D9 baseline

All four D3D9 groups passed in visible Chrome on July 29, 2026 with
`boxedwine.3.zip` SHA-256
`5bec857d928fbf3c9d570a61d91c2f4983e283d3f11ca986031d08e2aee8b699`,
the packaged `wine_tests_v6.zip`, and the single-threaded non-JIT
`boxedwine.wasm` SHA-256
`24f582ca16e04b8cbdc2fee22d5e87bdf4ce2b8aa0a631f65aaebbb58c3074b5`:

| Group | Assertions | Todo | Failures | Skipped | Duration | Wasm heap |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `stateblock` | 14,738 | 0 | 0 | 0 | 14.000 s | 536,870,912 |
| `device` | 55,282 | 127 | 0 | 23 | 59.063 s | 637,599,744 |
| `d3d9ex` | 2,104 | 77 | 0 | 45 | 37.063 s | 536,870,912 |
| `visual` | 25,350 | 40 | 0 | 50 | 385.015 s | 739,573,760 |

The first current-root `visual` run intermittently read black after
`Present()` in `test_vshader_float16()`. Chrome reported that
`glReadPixels()` ran with `GL_NONE`, even though WineD3D had just selected
`GL_COLOR_ATTACHMENT0`. Instrumentation showed Wine's expected and actual
framebuffer/read-buffer state were both correct immediately before BoxedWine
yielded to the single-threaded browser loop. Reasserting the state in WineD3D
therefore could not make the two separate guest GL calls atomic.

BoxedWine now remembers the guest's latest `glReadBuffer()` selection and
replays it inside the same Emscripten host callback that calls
`glReadPixels()`. The stale runtime reproduced 2 failures in 5,700 assertions
over 100 focused iterations; the rebuilt runtime completed the same stress
test with 0 failures. The complete official visual group then passed with no
`GL_NONE` readback error. This is a BoxedWine runtime fix, so the Wine patch,
`boxedwine.3.zip`, and the v10 Wine filesystems do not need new DLL payloads.

Run artifacts:

- `stateblock`, `device`, and `d3d9ex`:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-161747-083033`
- official `visual`:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-161106-545428`
- focused 100-iteration proof:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-160755-585731`
- stale-runtime A/B control:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-160508-472056`

The permanent `tools/openglTest` regression is
`readbuffer-yield-replay`, run by `tools/openglTest/runBrowserTest.py`. It
forces the underlying WebGL2 read buffer to `NONE` while the single-threaded
guest is yielded, then checks the guest readback numerically. The current
runtime passes:
`C:\Users\james\.cache\boxedwine\openglTests\runs\20260729-170838-962640-cbc37e`.
The pre-fix `20260729020709/st` runtime fails with
`GL_INVALID_OPERATION (1282)`:
`C:\Users\james\.cache\boxedwine\openglTests\runs\20260729-170801-930070`.

No D3D9 failures are accepted by the runner. The July 29 packaged PE32/i386
`d3d9_test.exe` had SHA-256
`7811a1ad1330e84f997ca9897c08103f2aab9d5767205286ccd938e0e00d9c3a`.
The rebuilt native x64 BoxedWine fast suite passed all 756 tests, and the
Linux-hosted Wine runner suite passed all 109 tests after this runtime change.

### D3D9 sRGB clear correction

On July 30, 2026 the WebGL WineD3D path gained deterministic sRGB conversion
for render-target clears. WebGL still reports `D3DUSAGE_QUERY_SRGBWRITE` as
unsupported because it cannot expose desktop framebuffer-sRGB control
generally. WineD3D now also clamps `ARB_FRAMEBUFFER_SRGB` from its WebGL GL
capabilities and CPU-converts clear colours whenever
`D3DRS_SRGBWRITEENABLE` is set. Desktop WineD3D keeps its existing path.

That allowed the broad WebGL skip in D3D9 `clear_test()` to be removed. The
focused clear-only executable passed 261 assertions with 0 failures, and the
complete unskipped loose visual executable passed 25,406 assertions with
40 todo results, 0 failures, and 49 skips:

- focused proof:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260730-101551-867702`
- complete loose-executable proof:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260730-101628-582791`

The finalized in-place inputs are:

- `boxedwine.3.zip` and `TinyCore15Wine11.0-v10-candidate.zip`
  (157,980,580 bytes):
  `2fad01442a3ce237dbb4a4725522b4c5ac686b4504f1770f1e10946688e6b636`
- `boxedwine.gdi.3.zip` (157,980,700 bytes):
  `5f765beb539e7cf21008392acfd78fd74f7d7603e54ea8dbd3550094ff92318d`
- single-threaded non-JIT `boxedwine.wasm`:
  `005c8fd96d33c11fab8680b2502f699b848623f1f99d00ba674d1512e963a225`
- packaged `ddraw.dll`:
  `59046d81fe044150d1e834604860f5f7f8ffc5c9224efc90ce8f40da044d3160`
- packaged `wined3d.dll`:
  `3b2b29f5275d1b40626f2783cffa1e0a142099cb4daa335b81ffb78deb487910`
- packaged `ddraw_test.exe`:
  `094a89f7799812a33c5741221ccb0cf02455f4b2f31e30bce5272ce3caae30cf`
- `wine_tests_v6.zip` (11,990,501 bytes):
  `eba177bb5c742cdb26a0ddce99e16d71a35ac85ec277759459b0c231ce9c155f`
- packaged `d3d9_test.exe`:
  `1d42eb50ada90f7d2e4b9bb145c2a36ce5caac5730be9c1115e9d766b0d656b6`

The exact packaged checkpoint passed `stateblock`, `device`, and `d3d9ex`.
Its first visual attempt recorded one intermittent black back-buffer read in
`test_flip()` at `visual.c:22153`; the exact isolated visual rerun passed all
25,406 assertions. The baseline remains at zero accepted failures and no
retry exception was added:

- combined D3D9 artifact:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260730-103900-127214`
- exact isolated visual pass:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260730-105756-283695`
- exact native D3D9 `stateblock` pass with the rebuilt executable:
  `/home/james/.cache/boxedwine/wineTests/runs/20260730-111138-051897`

### D3D9 vPos fragment-coordinate shader correction

On July 31, 2026 Wine's GLSL ES fragment-coordinate path was corrected so
`test_fragment_coords()` no longer needs a WebGL skip. WineD3D generated
`vpos.w = 1 / vpos.w;`; WebGL2's GLSL ES compiler rejects the mixed integer
and floating-point division. Generating `1.0 / vpos.w` instead produces a
valid shader. A direct WebGL2 compiler probe confirmed the original shader
failed and the corrected shader compiled, and the test's four expected pixels
then matched without a WebGL Y-origin adjustment.

The focused 57-assertion fragment-coordinate test passes with no todo,
failure, or skip in all three comparison modes:

- single-threaded non-JIT:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-170414-255282`
- single-threaded JIT:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-170414-261186`
- native Wine:
  `/home/james/.cache/boxedwine/wineTests/runs/20260731-164135-001859`

The final clean Wine patch-series build is packaged in place without a
version bump. Its exact inputs are:

- `boxedwine.3.zip` and `TinyCore15Wine11.0-v10-candidate.zip`
  (157,980,576 bytes):
  `e38234f93e85b1714c54f87ec3246a8275683b091219a8a4651ea7e3acd16b79`
- `boxedwine.gdi.3.zip` (157,980,696 bytes):
  `fe18720cd7b85c84764ef950e14e643ff3e0a58e6174fd3039ef8d0831670919`
- packaged `wined3d.dll`:
  `e82ce3d3f2b572722f42f8f9ce26fc8edbc55c107ca7908e8ff947f64b1018f6`
- packaged `d3d9_test.exe`:
  `15dae3b495e38d496ccb7858f734fce33811d84364404da8093c3d1a8f507916`
- `wine_tests_v6.zip` (11,990,014 bytes):
  `0a9e9b8c33fe33648fefeaa9408939121a60fe6ea1d7e06c4b4838ad5f288487`
- WebGL divergence manifest:
  `884b32dee777967c060aa94613e3ea6668826a34eba2f3c5493c3df26b6a6f8b`

The exact packaged D3D9 `visual` group passed 25,459 assertions with 40 todo
results, 0 failures, and 48 skips in 910.063 seconds:

```text
C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-172027-042093
```

An otherwise clean attempt reached the runner's former 900-second limit just
before the Wine summary. Because the previous exact pass had already taken
867.360 seconds and this pass needed 910.063 seconds, the default graphics
timeout is now 1,200 seconds. The incomplete timeout artifact is retained at
`C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-170512-006839`.

The same final archives also passed the exact DirectDraw `ddraw1` baseline:
19,640 assertions, 59 todo results, 0 failures, and 20 skips in 172.500
seconds. This confirms that the DirectDraw and color-key sources restored in
the clean patch-series build remain present:

```text
C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-173601-212796
```

### D3D9 lockable-depth capability correction

On July 31, 2026 the WebGL profile stopped advertising
`D3DFMT_D16_LOCKABLE`. WineD3D's current WebGL path does not have a complete
GPU-to-CPU depth download after clears and draws, so the format previously
returned stale CPU data from `LockRect()`. Standard non-lockable depth formats
remain supported. The test-only WebGL skip was removed; Wine's existing
unsupported-format path now records `D3DFMT_D16_LOCKABLE is not supported.`

The focused clear-and-lock test passes in both Emscripten modes with 18
assertions, 0 todo results, 0 failures, and 1 upstream skip:

- single-threaded non-JIT:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-181137-311835`
- single-threaded JIT:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-183527-924106`

The final single-threaded non-JIT exact `visual` baseline passed 25,431
assertions with 40 todo results, 0 failures, and 48 skips in 869.125 seconds:

```text
C:\Users\james\.cache\boxedwine\wineTests\runs\20260731-183617-722284
```

The 28-assertion reduction from the prior baseline is entirely below Wine's
lockable-depth capability check. The current checkpoint artifacts are:

- `boxedwine.3.zip` and `TinyCore15Wine11.0-v10-candidate.zip`
  (157,980,607 bytes):
  `a6c2a2a0d902f2725b95f4ae58dd2a1a21acc00aae74d8ab82fa551cbc67f8d1`
- `boxedwine.gdi.3.zip` (157,980,727 bytes):
  `641f8135265c00efae5f2ec40ebfed6dd404148d23feaab969b64e95dbb63ce5`
- packaged `wined3d.dll`:
  `4b8ba863aae0f936d06eaed835b62cd0ad5eb8fb49297526d6317c66e11f6b4f`
- packaged `d3d9_test.exe`:
  `8f690c501950a3cd8f6e25ff5920b3eefab229390e41a5b4ecb86ce8bca133bc`
- `wine_tests_v6.zip` (11,989,896 bytes):
  `4fab1b0fb27a15a8f1a136acaa70b1d5bfeaa32addee2252372e72a60828185e`
- WebGL divergence manifest:
  `d45deefa5a520395176ff21f7345c01bd4bc8623e8d85307175c6cc11973d540`

## D3DX9 baseline

Run the complete validated D3DX9 set with:

```powershell
python tools/wineTests/runWineTests.py `
  --d3dx9-group asm `
  --d3dx9-group core `
  --d3dx9-group effect `
  --d3dx9-group line `
  --d3dx9-group math `
  --d3dx9-group mesh `
  --d3dx9-group shader `
  --d3dx9-group surface `
  --d3dx9-group texture `
  --d3dx9-group volume `
  --d3dx9-group xfile `
  --graphics-filesystem "$env:APPDATA\Boxedwine\FileSystems2\boxedwine.3.zip"
```

The July 29, 2026 visible-Chrome baseline uses `boxedwine.3.zip` SHA-256
`5bec857d928fbf3c9d570a61d91c2f4983e283d3f11ca986031d08e2aee8b699`:

| Group | Assertions | Todo | Failures | Skipped | Duration |
| --- | ---: | ---: | ---: | ---: | ---: |
| `asm` | 40 | 3 | 0 | 0 | 13.032 s |
| `core` | 1,485 | 99 | 0 | 0 | 19.047 s |
| `effect` | 234,068 | 60 | 0 | 0 | 17.047 s |
| `line` | 23 | 0 | 0 | 0 | 13.063 s |
| `math` | 32,509 | 0 | 1 accepted | 0 | 14.079 s |
| `mesh` | 132,301 | 9 | 0 | 0 | 26.016 s |
| `shader` | 5,734,482 | 0 | 0 | 0 | 21.578 s |
| `surface` | 3,396 | 136 | 0 | 1 | 22.062 s |
| `texture` | 461,276 | 56 | 0 | 0 | 26.078 s |
| `volume` | 263,237 | 23 | 0 | 0 | 16.094 s |
| `xfile` | 42 | 0 | 0 | 0 | 13.047 s |

The `math` result is accepted only when its single failure occurs at exactly
`math.c:1557`; it reproduces with the same PE32 Wine D3DX DLL on Windows. The
previously accepted `math.c:4559` RGB565 coefficient discrepancy no longer
reproduces with the current DLL/runtime and was removed from the policy. Any
different identity or failure count fails the run.

The first `surface` run exposed a one-ULP `D3DFMT_CxV8U8` conversion failure
at `surface.c:3514`. Wine sets the x87 precision control to single precision
when creating the D3D9 device, but BoxedWine's double-backed interpreter and
JIT arithmetic previously retained a double-precision intermediate. BoxedWine
now rounds x87 add, subtract, multiply, divide, and square-root results when
the control word requests single precision. The exact surface assertion also
passes when the same PE32 test and Wine D3DX DLL run directly on Windows.

The first `effect` run found five BoxedWine x87 defects: `FRNDINT` converted
large integral results through `S64`, and interpreter/JIT `FCHS` lost the sign
of zero. Preserving the rounded double directly and flipping the x87 sign bit
returns the group to zero failures. The native x64 fast suite passes all 756
tests after the fix.

The first `mesh` run stalled while Wine handled an unsupported animated X-file.
Its error cleanup released the caller's untouched sentinel animation-controller
pointer. Initializing that output to `NULL` before any fallible work removes
the hang. Restoring Wine's existing `D3DXMESHOPT_COMPACT` and
`D3DXMESHOPT_ATTRSORT` implementations also removed 131 follow-on mesh
failures; no mesh failure is accepted.

Current run artifacts:

- `core`, `shader`, `surface`, and `texture`:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-150449-967534`
- final accepted-policy `math` proof:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-151215-448477`
- `asm`, `line`, `volume`, and `xfile`:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-150325-138130`
- `effect`:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-150237-155518`
- `mesh`:
  `C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-150159-973431`

The packaged `d3dx9_43_test.exe` is PE32/i386 with SHA-256
`a2c3dfa3c1c0aabeaa5d0a9961ad16f2066574522041caf120046b8a225e96a8`.
The packaged WebGL `d3dx9_36.dll` and `d3dx9_43.dll` hashes are
`832b447b03d7ee8c8a9cd01871dea7286eefcd4f7b7e3337cf9f3f15bc83d8a3`
and
`4efb7a5554e3fb938b09a6367f414fe223014c883b26352a38daae183415d436`,
respectively.

## D3DXOF baseline

Run Wine's D3DXOF group with:

```powershell
python tools/wineTests/runWineTests.py `
  --d3dxof-group d3dxof `
  --graphics-filesystem "$env:APPDATA\Boxedwine\FileSystems2\boxedwine.3.zip"
```

The July 29, 2026 visible-Chrome baseline completed 192 assertions with
0 todo results, 0 failures, and 0 skips in 8.922 seconds. No D3DXOF failures
are accepted by the runner.

The packaged `d3dxof_test.exe` is PE32/i386 with SHA-256
`2327cf9140c0b2e891fd93ae1eb9a352631072edaf8cad50eb2881fb01989811`.
Run artifacts are retained at:
`C:\Users\james\.cache\boxedwine\wineTests\runs\20260729-135543-617471`.

## Development checks

```powershell
python -m unittest tools/wineTests/tests/test_run_wine_graphics_tests.py -v
python -m unittest tools/wineTests/tests/test_run_wine_graphics_integration.py -v
python -m py_compile tools/wineTests/wineGraphicsBrowser.py
```
