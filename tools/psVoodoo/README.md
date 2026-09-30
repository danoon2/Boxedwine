# psVoodoo cockpit depth fix

`patches/0001-stabilize-saturated-w-depth.patch` applies to psVoodoo commit
`67fcb0a0eb1be9c5f77d04250ad715b2f481754b`. It includes the fix, a standalone
Glide regression, and a saturated-depth option for the detail regression.
The patch preserves the upstream source line endings.

F-16's moving cockpit submits triangles beyond the W-buffer near limit.
psVoodoo's constant vertex-depth clamp can acquire different rasterized depths
after WineD3D's XYZRHW conversion, making panels occlude later instrument draws.
The fix writes saturated near depth explicitly from a cached pixel shader while
keeping vertices inside the clip volume. Ordinary depth handling is unchanged.
Shader-model-1 devices retain the existing path.

Validation on 2026-09-29:

- The original three-triangle reproduction loses all 6,998 overlay samples on
  WineD3D/OpenGL. Writing vertex Z=1 instead fails on native Windows, so that
  approach was rejected.
- The final Glide regression passes 64 cases and 23,312 samples on both native
  Windows D3D9 and Boxedwine/Wine 11 OpenGL. It exercises textured/untextured
  drawing, LEQUAL/LESS, subpixel movement, and shader-cache transitions.
- Existing W-bias, clip-order, detail-texture and framebuffer regressions pass.
  The saturated detail-shader variant also passes.
- James confirmed the moving F1 cockpit looks good; the earlier F2 HUD fix is
  still present in Boxedwine.

The version 13 filesystem profile references this patch in
[`../buildWine/filesystem_wine11.json`](../buildWine/filesystem_wine11.json).
[`../buildWine/build_filesystem.py`](../buildWine/build_filesystem.py) applies it
to a private snapshot of the pinned upstream tree before calling upstream's
builder. The original checkout is unchanged. The build cache tracks the base
revision, patch contents, and toolchain pin. The packaged source archive contains
the exact patched sources, including the regression tests; `build.json` records
their hashes and identifies the snapshot separately from the upstream base.

For a standalone checkout, use `git apply --check PATCH` then `git apply PATCH`.
Upstream's `tools/build.py` requires committed source, so commit the patch in
that checkout before using that builder.

Local validation artifacts are under `tmp/f16-rendering-20260929`:
`glide2x-cockpit-shader-depth.dll`, `cockpit-fix-manifest.json`,
`near-depth-native-fixed.log`, `probe-near-depth-final.log`, and
`shader-depth-regressions.log`. The DLL SHA-256 is
`e009b30240016be41ad9db8768f3949ba3528bbd4bf05e766c67c1de1df123eb`.
The filesystem builder's DLL differs only in PE build timestamps and has SHA-256
`345c793abf12f1b3fb4c5cee311a168b9d1d767622118f98d21324ab1ea8e430`.
The version 13 packaging comparison and ZIP-based regression results are under
`tmp/filesystem-v13-psvoodoo-20260929`.
