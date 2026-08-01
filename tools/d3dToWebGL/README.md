# Wine 11 DirectX-to-WebGL patch series

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
11. `webgl-tests-against-wine-11.0.patch`
   contains only the WebGL-specific Wine graphics-test adaptations.

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

## Runtime controls

- `WINE_D3D_CONFIG=webgl=1,webgl_glsl_es=1` enables the production WebGL path.
  The Emscripten launcher sets this explicitly; an unconfigured native Wine
  run leaves both settings disabled.
- `BOXEDWINE_WEBGL_PROFILE=1` enables WineD3D and DirectDraw timing counters.
- `BOXEDWINE_WEBGL_BLTFAST_LOCKS=1` restores the older DirectDraw
  `BltFast` lock path for A/B diagnosis.
- `BOXEDWINE_WEBGL_FRONTBUFFER_PRESENT_MIN_RECTS=N` overrides the diagnostic
  dirty-rectangle threshold. Unset or zero uses the normal time/count
  heuristic.
- `BOXEDWINE_WEBGL_DISABLE_BATCH_BLITS=1` disables GLSL blit batches for A/B
  diagnosis.
- `BOXEDWINE_WEBGL_DISABLE_MULTISOURCE_BATCH_BLITS=1` retains batching but
  prevents a batch from spanning multiple source textures.

All `BOXEDWINE_WEBGL_*` controls are off by default. They are retained because
they isolate presentation and batching regressions without requiring another
DLL build.

Later layers intentionally modify different hunks in some of the same files.
Check and apply each patch before moving to the next one:

```bash
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-build-config-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-build-config-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-adapter-context-caps-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-adapter-context-caps-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-shader-generation-glsl-es-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-shader-generation-glsl-es-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-texture-formats-transfers-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-texture-formats-transfers-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-blitter-batching-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-blitter-batching-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-directdraw-runtime-presentation-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-directdraw-runtime-presentation-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3dx9-assets-compatibility-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3dx9-assets-compatibility-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3dxof-parser-hardening-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3dxof-parser-hardening-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-wined3d-draw-state-query-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-wined3d-draw-state-query-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch
git apply --check /path/to/Boxedwine/tools/d3dToWebGL/webgl-tests-against-wine-11.0.patch
git apply         /path/to/Boxedwine/tools/d3dToWebGL/webgl-tests-against-wine-11.0.patch
```

`tools/wineTests/webgl-test-divergences-v2.json` pins the ordered patch hashes and
classifies every added `skip()` and `todo_wine_if()` in the test-only patch.
Validate the boundary and classifications from the Boxedwine checkout:

```bash
python3 tools/wineTests/webglTestDivergences.py
```

The validator rejects production test hunks, non-test files in the test patch,
production/test overlap, unclassified test policies, and patch hash changes.

For the clean pinned build, deterministic PE32 output validation, filesystem
packaging, and v3/v10 ZIP validator, use
`tools/buildWine/webgl_filesystem.py` as documented in
`tools/buildWine/README.md`.
