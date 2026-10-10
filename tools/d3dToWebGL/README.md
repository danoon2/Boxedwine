# Wine 11 DirectX-to-WebGL patch series

The current source series is pinned by
[`webgl-test-divergences-v43.json`](../wineTests/webgl-test-divergences-v43.json).
It contains 59 production patches and one separate, complete Wine graphics-test
adaptation. The [v43 inventory](../wineTests/webgl-patch-inventory-v43.json)
records every affected file, patch order, category and SHA-256 hash.

Versions 2 through 42 retain earlier build and test selections for reproducing historical
results. Their test patches are alternatives, not incremental layers: apply
only the test patch named by the selected manifest.

The published v11 filesystem is pinned to v41. Its September 19 download
matches the candidate used for the focused regression checks; see
[the validation and upload record](../wineTests/graphics-review-fixes-20260919.json).
The version 15 full and web filesystems select v43, adding RGB565 SSE2 upload
conversion, fog/projection constant caches and immediate map/unmap dispatch.
Their separately built PE32 `C:/webgl/wined3d.dll` is pinned by
[`webgl_filesystems_v15.json`](../buildWine/webgl_filesystems_v15.json).
Regular Wine carries the fog/projection and immediate-map counterparts through
`wine_builds.json`; RGB565 expansion is specific to the WebGL upload path.

The RGB565 upload optimization is
[`webgl-rgb565-upload-scaling-against-wine-11.0.patch`](webgl-rgb565-upload-scaling-against-wine-11.0.patch).
It is patch 56 in the v43 production series. It expands eight RGB565 pixels at a time
with SSE2, preserves exact rounded channel values, and uses scalar conversion
for remaining pixels and builds without SSE2. The compiled PE32 row function
passed all 65,536 input colours plus alignment, tail and row-guard checks in
Boxedwine's single-threaded Wasm JIT. Alternating local benchmarks measured
about 2.0–5.2 times the original conversion throughput for 32–4096-pixel rows;
these are CPU conversion timings, not MW3 frame-rate measurements.

The fog-uniform optimization is
[`webgl-fog-uniform-cache-against-wine-11.0.patch`](webgl-fog-uniform-cache-against-wine-11.0.patch).
It is patch 57 in the v43 production series. In WebGL
mode it remembers all seven fog floats per linked program and skips uploads
only when their bits match. New programs start with an invalid cache. Focused
helper checks cover every component, program switching and recreation, signed
zero, NaN payloads and native-mode behavior. MW3 rendered correctly in the
browser capture and the measured Uniform1f/Uniform4fv calls fell from about
five per draw to one. The observed frame rate rose from 7.59 to 9.56 swaps/s,
but draw counts per frame differed, so this is not a controlled speedup result.

The projection-constant optimization is
[`webgl-projection-constant-cache-against-wine-11.0.patch`](webgl-projection-constant-cache-against-wine-11.0.patch).
It is patch 58 in the v43 production series.
DirectDraw surface synchronization repeatedly invalidates the projection used
for pretransformed vertices. In WebGL mode this patch compares the complete
matrix before queuing a constant-buffer copy and shader invalidation. Both
normal and pretransformed projection updates share the cache; reset and full
state invalidation clear it, including DirectDraw device switches. Focused
helper checks cover first uploads, allocation retry, every component, exact
float bits, matrix changes, invalidation and native-mode behavior. MW3 rendered
correctly during stationary and active browser captures. With fog caching
already enabled, measured Uniform1f/Uniform4fv calls per draw fell by 96.6%
and 92.0%, respectively. Stationary presentation rose from 9.66 to 10.89
swaps/s in one pair of runs; diagnostic overhead and differences in workload
prevent treating this as a controlled whole-game speedup. The separate
constant dirty-range rounding issue is not changed by this patch.

The map/unmap dispatch optimization is
[`webgl-direct-map-dispatch-against-wine-11.0.patch`](webgl-direct-map-dispatch-against-wine-11.0.patch).
It is patch 59 in the v43 production series. Immediate WebGL contexts call the resource map/unmap callbacks
directly after the existing upload-buffer and synchronization handling.
Other contexts retain command dispatch. The compiled PE32 code passed 77
focused checks per run under the single-threaded Wasm JIT, including nested
commands, failure returns, pitches, upload addresses and idle-wait ordering.
Six alternating benchmark processes measured about 0.45 microseconds less
dispatch overhead per NOOVERWRITE map/unmap pair (1.03 to 0.59 microseconds).
Resource callbacks were test substitutes, so this does not establish full
mapping cost or a game frame-rate gain. The user reported correct MW3 rendering
during the Chrome gameplay check.

Production patch replay and test-policy validation establish reproducibility,
not graphics conformance. The standalone probes in `tools/wineTests/tests/`
cover context transitions, copies, packed formats, capabilities, D3DX behavior,
mapped buffers and shader-failure recovery. The browser harness runs those
probes and Wine's graphics tests against the selected runtime and filesystem.

## Production patch order

The first ten layers provide the original WebGL implementation:

Apply these patches, in order, to the official Wine 11 source commit
`db11d0fe6a169c457e23d007e20404643d067aa8`:

1. `webgl-build-config-against-wine-11.0.patch`
   adds the reproducible PE32 build script and guide, defines the centralized
   WineD3D WebGL settings, reads `webgl` / `webgl_glsl_es` from
   `WINE_D3D_CONFIG`, and centralizes WineD3D profiling enablement, timing,
   buckets, and aggregate reporting.
2. `webgl-adapter-context-caps-against-wine-11.0.patch`
   contains OpenGL adapter initialization, extension/capability filtering,
   WebGL context state initialization, logical display-mode handling,
   synthetic browser-output behavior, and the desktop framebuffer-sRGB
   capability clamp used by the WebGL clear fallback.
3. `webgl-shader-generation-glsl-es-against-wine-11.0.patch`
   contains D3D8/D3DX shader-semantic handling, fixed-function fog shader
   generation, GLSL ES 3.00 source and interface generation, shader compile
   keys, and WebGL shader-stage capability limits.
4. `webgl-texture-formats-transfers-against-wine-11.0.patch`
   contains WebGL texture-format mapping and capability clamps, D3D9 managed
   texture and mipmap handling, texture storage/upload/download, compressed
   sysmem preservation, CPU format conversion, and framebuffer readback.
5. `webgl-blitter-batching-against-wine-11.0.patch`
   contains the GLSL blitter programs, WebGL palette handling, GPU blit
   routing, command-stream blits, batched shaded-quad submission, and batch
   flush rules.
6. `webgl-directdraw-runtime-presentation-against-wine-11.0.patch`
   contains centralized DirectDraw WebGL configuration detection, front-buffer
   batching and presentation, overlay composition and z-order, clipper state,
   GPU-aware blit paths, profiling, and legacy Direct3D 1-7 compatibility
   fixes implemented by `ddraw.dll`.
7. `webgl-d3dx9-assets-compatibility-against-wine-11.0.patch`
   contains D3DX9 animation-controller support, mesh loading and hierarchy
   handling, tangent/normal/strip generation, sprite/effect/font fixes, image
   metadata handling, and cross-version D3DX9 exports.
8. `webgl-d3dxof-parser-hardening-against-wine-11.0.patch`
   contains bounded template/object-name handling, reduced fixed subobject
   allocation, and parser/object-enumeration cleanup for DirectX `.x` files.
9. `webgl-wined3d-draw-state-query-against-wine-11.0.patch`
   contains buffer mapping and streaming, client-array draw submission,
   fixed-function state adaptation, WebGL query handling, clear fallbacks,
   CPU sRGB conversion for WebGL clears when `D3DRS_SRGBWRITEENABLE` is set,
   sampler state, and texture/renderbuffer resource synchronization.
10. `webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch`
   contains D3D8 system-memory stream iteration cleanup and explicit D3D9
   initialization, device, and output-table failure diagnostics.

Production patches must not modify files below a Wine `dlls/*/tests/`
directory.

## Patch audit map

Each patch has one review boundary and at least one existing test that can
prove its behavior independently of a game:

| Patch | WebGL limitation or responsibility | Smallest current proof |
| --- | --- | --- |
| `build-config` | WebGL must be an explicit runtime mode while desktop Wine keeps its normal defaults; PE32 DLL builds must be reproducible. | Build-script PE/import validation plus the native stable groups with WebGL disabled. |
| `adapter-context-caps` | Browser contexts do not expose desktop WGL initialization, display modes, or the full desktop GL extension set. | D3D8 `device`, D3D9 `d3d9ex` / `device`, and the BoxedWine OpenGL context tests. |
| `shader-generation-glsl-es` | WebGL2 accepts GLSL ES 3.00 and has different fixed-function, fog, interface, and shader-limit rules. | D3D8 `visual`, D3D9 `stateblock` / `visual`, ShadowMap, and PostProcess. |
| `texture-formats-transfers` | WebGL2 restricts texture formats, storage, upload/download, compressed data, and framebuffer readback. | DirectDraw `ddraw7`, D3D9 `visual`, and D3DX9 `surface`, `texture`, and `volume`. |
| `blitter-batching` | WebGL2 requires shader/FBO blits and benefits from batching palette and shaded-quad work without changing ordering. | DirectDraw `dsurface`, `ddraw7`, and `visual`. |
| `directdraw-runtime-presentation` | Browser presentation has no native DirectDraw front buffer, overlay plane, or legacy D3D 1-7 presentation path. | DirectDraw `refcount` followed by `d3d`, `ddraw7`, and `visual`; Tomb Raider 3 and VideoDD cover application behavior. |
| `d3dx9-assets-compatibility` | D3DX9 helpers needed by browser games were incomplete, especially animation, mesh, effect, font, sprite, and image paths. | D3DX9 `core`, `effect`, `mesh`, `surface`, `texture`, and `xfile`. |
| `d3dxof-parser-hardening` | Real-world `.x` files exceed old fixed parser/object assumptions and require safe cleanup. | D3DXOF `d3dxof` plus D3DX9 `xfile` and `mesh`. |
| `wined3d-draw-state-query` | WebGL2 buffer mapping, client arrays, query behavior, clear state, and resource synchronization differ from desktop GL. | D3D8/D3D9 `stateblock`, `device`, and `visual`, plus the BoxedWine OpenGL microtests. |
| `d3d8-d3d9-compatibility-diagnostics` | D3D8 system-memory streams need safe iteration, while D3D9 initialization failures need enough context to diagnose browser-only failures. | D3D8 `device` and D3D9 `device` / `d3d9ex`. |
| `tests` | Wine tests must distinguish intentional WebGL limits from missing emulation and defects without weakening production code. | `webglTestDivergences.py` plus every exact graphics baseline group. |

The July 30, 2026 audit found no unused WebGL helper. The retained feature
marker strings and the controls below are intentional diagnostics or A/B
fallbacks, not demo defaults.


The manifest appends these corrections after production patch 10, in order:

11. `webgl-graphics-correctness-against-wine-11.0.patch`
12. `webgl-clip-plane-state-against-wine-11.0.patch`
13. `webgl-zero-clip-capability-against-wine-11.0.patch`
14. `webgl-depth-range-against-wine-11.0.patch`
15. `webgl-depth-bias-against-wine-11.0.patch`
16. `webgl-depth-copy-against-wine-11.0.patch`
17. `webgl-depth-readback-against-wine-11.0.patch`
18. `webgl-stencil-clear-against-wine-11.0.patch`
19. `webgl-point-size-capability-against-wine-11.0.patch`
20. `webgl-point-sprite-origin-against-wine-11.0.patch`
21. `webgl-context-state-against-wine-11.0.patch`
22. `webgl-presentation-state-against-wine-11.0.patch`
23. `webgl-self-blit-against-wine-11.0.patch`
24. `webgl-alpha-test-outputs-against-wine-11.0.patch`
25. `webgl-context-backend-lifecycle-against-wine-11.0.patch`
26. `webgl-multisample-color-copy-against-wine-11.0.patch`
27. `webgl-shared-context-owner-against-wine-11.0.patch`
28. `webgl-d3dxof-object-limit-against-wine-11.0.patch`
29. `webgl-format-storage-against-wine-11.0.patch`
30. `webgl-rgb10-transfers-against-wine-11.0.patch`
31. `webgl-format-sample-policy-against-wine-11.0.patch`
32. `webgl-float-capabilities-against-wine-11.0.patch`
33. `webgl-ffp-normal-texgen-against-wine-11.0.patch`
34. `webgl-vertex-fog-against-wine-11.0.patch`
35. `webgl-d3dx-tangent-frame-against-wine-11.0.patch`
36. `webgl-d3dx-state-lifecycle-against-wine-11.0.patch`
37. `webgl-d3dx-frame-sphere-against-wine-11.0.patch`
38. `webgl-generated-texcoords-against-wine-11.0.patch`
39. `webgl-ffp-failure-recovery-against-wine-11.0.patch`
40. `webgl-mapped-buffer-uploads-against-wine-11.0.patch`
41. `webgl-glsl-program-failure-against-wine-11.0.patch`
42. `webgl-flat-shading-indices-against-wine-11.0.patch`
43. `webgl-nonindexed-instance-streams-against-wine-11.0.patch`
44. `webgl-legacy-specular-power-against-wine-11.0.patch`
45. `webgl-blitter-failure-against-wine-11.0.patch`
46. `webgl-sample-mask-against-wine-11.0.patch`
47. `webgl-p8-copy-against-wine-11.0.patch`
48. `webgl-frontbuffer-immediate-against-wine-11.0.patch`
49. `webgl-sse-build-against-wine-11.0.patch`
    matches the main Wine build's `-msse2 -march=pentium4 -mfpmath=sse` flags
    while retaining the existing PE32 build configuration and optimization level.
50. `webgl-full-surface-clear-against-wine-11.0.patch`
    avoids reading old pixels before a complete CPU surface clear.
51. `webgl-color-readback-loops-against-wine-11.0.patch`
    uses dedicated conversion loops for common color readback formats.
52. `webgl-lazy-depth-clear-against-wine-11.0.patch`
    defers CPU depth synchronization after the GPU depth clear.
53. `webgl-indexed-draw-boundaries-against-wine-11.0.patch`
    preserves index 65535 for lists as well as strips and fans, including flat
    shading. Negative base vertices are baked into the indices so attribute
    pointers stay within their buffers, including indexed user-pointer draws.
54. `webgl-ddraw-rgb10-masks-against-wine-11.0.patch`
    fixes both DirectDraw B10G10R10A2 mappings: RGB occupy bits 20..29,
    10..19 and 0..9, while alpha occupies bits 30..31.
55. `webgl-gdi-owndc-sysmem-against-wine-11.0.patch`
    keeps the system-memory allocation alive while persistent GDI DCs reference
    it. The independent `tools/buildWine/probes/ddraw-flip.c` regression faults
    after two flips with the old PE DLL when `renderer=gdi` is selected. This
    patch carries the main Wine runtime's lifetime fix into the WebGL build;
    it does not change Wine test policy.

## Runtime controls

- `WINE_D3D_CONFIG=webgl=1,webgl_glsl_es=1` enables the WebGL path. The
  Emscripten launcher sets it explicitly; native Wine leaves it disabled by default.
- `BOXEDWINE_WEBGL_PROFILE=1` enables WineD3D and DirectDraw timing counters.
- `BOXEDWINE_WEBGL_BLTFAST_LOCKS=1` restores the older DirectDraw `BltFast`
  lock path for comparison.
- `BOXEDWINE_WEBGL_FRONTBUFFER_PRESENT_MIN_RECTS=N` overrides the diagnostic
  dirty-rectangle threshold. Unset or empty selects one rectangle, so the last
  update is presented even when the application stops drawing. Explicit zero
  retains the old time/count heuristic, which requires another graphics call.
- `BOXEDWINE_WEBGL_DISABLE_BATCH_BLITS=1` disables GLSL blit batches.
- `BOXEDWINE_WEBGL_DISABLE_MULTISOURCE_BATCH_BLITS=1` retains batching but
  prevents a batch from spanning multiple source textures.

Leave the diagnostic overrides unset for normal operation.

## Validation and application

Run the policy validator from the Boxedwine checkout before applying patches:

```bash
python3 tools/wineTests/webglTestDivergences.py \
  --manifest tools/wineTests/webgl-test-divergences-v43.json
```

The validator checks patch hashes, forbids production changes to Wine test
files, rejects non-test files in the complete test adaptation, and requires
every added skip or TODO policy to have a classification. Both Git-format and
plain unified production diffs participate in those checks. The optional
`--inventory-output /tmp/webgl-patch-inventory.json` writes the categorized map.

Use a separate, clean Wine checkout at
`db11d0fe6a169c457e23d007e20404643d067aa8`. From the Boxedwine checkout, apply
the production list in manifest order:

```bash
python3 - /path/to/clean/wine-11.0 <<'PY'
import json
from pathlib import Path
import subprocess
import sys

repo = Path.cwd()
source = Path(sys.argv[1]).resolve()
manifest = json.loads((repo / "tools/wineTests/webgl-test-divergences-v43.json").read_text())
head = subprocess.check_output(["git", "-C", str(source), "rev-parse", "HEAD"], text=True).strip()
if head != manifest["wine_source_commit"]:
    raise SystemExit("Wine checkout is not at the pinned source commit")
if subprocess.check_output(["git", "-C", str(source), "status", "--porcelain"]):
    raise SystemExit("Use a clean Wine checkout")
for entry in manifest["production_patches"]:
    patch = str(repo / entry["path"])
    subprocess.run(["git", "-C", str(source), "apply", "--check", patch], check=True)
    subprocess.run(["git", "-C", str(source), "apply", patch], check=True)
PY
```

For test builds, apply only
`webgl-tests-p8-copy-against-wine-11.0.patch` after the production series.
The failure-injection patches under `tools/wineTests/tests/` are separate
diagnostic builds and must not enter production DLLs. Their control variants
target the earlier series identified in their filenames.

The Wine 11 Explorer startup-timeout patch under `tools/buildWine/patches/`
is a separate base-filesystem fix selected by `wine_builds.json`.
For deterministic PE32 output validation and filesystem packaging, see the
existing [`tools/buildWine/README.md`](../buildWine/README.md).

## MechWarrior 3 performance updates

`webgl-lazy-depth-clear-against-wine-11.0.patch` removes the eager CPU depth
clear after DirectDraw has already cleared the GPU surface. The existing
texture-location tracking defers synchronization until a CPU reader needs it.
The clean build was tested on the September 14 SSE WebGL DLL filesystem with
the full-surface-clear and color-readback-loop patches. All three are included
in v40 and the September 15 v11 upload package.

Two alternating gameplay comparisons measured about 8.7% higher throughput,
with one fewer readback per frame. The clean candidate passed 2,118 depth
readback checks and 3,014 stencil checks, plus scheduler wake/idle checks.
See [`docs/mechwarrior3-chrome-performance.md`](../../docs/mechwarrior3-chrome-performance.md)
for the experiment identities, limitations, and subsequent search results.
The full Wine graphics matrix remains qualified on v39; v40 adds the targeted
checks above and packaging validation without changing Wine test policy.
