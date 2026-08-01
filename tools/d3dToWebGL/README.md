# Wine 11 DirectX-to-WebGL patch series

Apply these patches, in order, to the official Wine 11 source commit
`db11d0fe6a169c457e23d007e20404643d067aa8`:

1. `webgl-build-config-against-wine-11.0.patch`
   adds the reproducible PE32 build script and guide, defines the centralized
   WineD3D WebGL settings, and reads `webgl` / `webgl_glsl_es` from
   `WINE_D3D_CONFIG`.
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
   contains DirectDraw front-buffer batching and presentation, overlay
   composition and z-order, clipper state, GPU-aware blit paths, profiling,
   and legacy Direct3D 1-7 compatibility fixes implemented by `ddraw.dll`.
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
