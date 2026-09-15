# Wine 11 DirectX-to-WebGL patch series

The current series is pinned by
[`webgl-test-divergences-v39.json`](../wineTests/webgl-test-divergences-v39.json).
It contains 49 production patches and one separate, complete Wine graphics-test
adaptation. The [v39 inventory](../wineTests/webgl-patch-inventory-v39.json)
records every affected file, patch order, category and SHA-256 hash.

Versions 2 through 38 retain earlier build and test selections for reproducing historical
results. Their test patches are alternatives, not incremental layers: apply
only the test patch named by the selected manifest.

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
  --manifest tools/wineTests/webgl-test-divergences-v38.json
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
manifest = json.loads((repo / "tools/wineTests/webgl-test-divergences-v38.json").read_text())
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

## MechWarrior 3 performance candidate

`webgl-lazy-depth-clear-against-wine-11.0.patch` removes the eager CPU depth
clear after DirectDraw has already cleared the GPU surface. The existing
texture-location tracking defers synchronization until a CPU reader needs it.
The candidate was tested on the September 14 SSE WebGL DLL filesystem with
the standalone full-surface-clear and color-readback-loop patches. It is not
part of the production manifest yet.

Two alternating gameplay comparisons measured about 8.7% higher throughput,
with one fewer readback per frame. The clean candidate passed 2,118 depth
readback checks and 3,014 stencil checks, plus scheduler wake/idle checks.
See [`docs/mechwarrior3-chrome-performance.md`](../../docs/mechwarrior3-chrome-performance.md)
for the experiment identities, limitations, and subsequent search results.
