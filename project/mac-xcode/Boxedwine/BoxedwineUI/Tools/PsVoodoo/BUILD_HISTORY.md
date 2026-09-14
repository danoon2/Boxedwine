# Historical upstream patch build

These notes describe the pre-fork build. The current build uses the pinned GitHub fork; see [README.md](README.md). `build-upstream-snapshot.py` preserves the old build recipe and its patches for reproducibility.

# psVoodoo build experiment

This builds the Windows Glide 2 wrapper for disposable Wine prefixes. It is not part of the app build, a release package, or a cleared third-party dependency. Compatibility results and the outstanding legacy SDK notices are recorded in [GLIDE_SUPPORT.md](../../GLIDE_SUPPORT.md).

The current target is **WineD3D OpenGL with filesystem version 11 and a matching Boxedwine runtime**. Use the default build first. The optional Vulkan experiments below are deferred.

Clone the official repository and select the inspected version:

```sh
git clone https://git.code.sf.net/p/psvoodoo/psVoodoo psvoodoo-source
git -C psvoodoo-source checkout 8ebfc3c45f8f067af852720f9857d4641bf64252
```

From this directory, provide an LLVM-MinGW bin directory and a **new scratch output directory**:

```sh
python3 build.py --source /path/to/psvoodoo-source \
  --toolchain /path/to/llvm-mingw/bin --output /path/to/new-build
```

The verified toolchain used here was the official `20260908` UCRT macOS universal archive, SHA-256 `d1dc5d1ecf3a3ced5ed5544c72f1acd0c8e84eb3024d520ecc6b143eec62a149`. The script rejects a different source revision or tracked modifications, copies sources to the output directory, and applies these local adjustments by default:

- Make the read-only chroma-key parameter a const reference, declare `sprintf`, and normalize include spelling.
- Use MinGW's D3D9Ex declarations and Windows resource constants in place of redundant compatibility declarations and MFC's resource include. The MFC configuration application is not built.
- Accept legacy narrowing conversions that the original compiler accepted, and statically link compiler runtime support.
- Export the bare, MinGW stdcall and Microsoft stdcall spellings. Link D3DX calls to `d3dx9_43.dll`, already provided by Wine in the tested filesystems.
- Implement `guTexMemQueryAvail` using the remaining bytes in the wrapper's single 16 MiB TMU. Its upstream stub returns zero, causing Descent 3 to allocate an empty texture cache and dereference its `0xDEADBEEF` sentinel. The query subtracts existing utility texture allocations, clamps exhausted memory to zero, and reports zero for unsupported TMUs or an unopened context. `build.json` records `texture_memory_query: true`.

- Apply `lfb-colors.patch`: snapshot the target for RGB565 LFB writes, compare the resulting pixels, and upload changed pixels with a separate alpha mask. Cyan and magenta remain ordinary colors, and untouched 3D pixels keep their original color precision and resolution. The cached readback and a direct pixel conversion avoid the slower D3DX/GDI conversion route. Failed initial readback returns a failed lock instead of copying uninitialized pixels. `build.json` records `lfb_color_mask` and the patch hash. The [framebuffer regression](../GlideProbe/README.md#framebuffer-color-regression) verifies the palette and partial updates.

- Apply `clip-window.patch`: flush queued triangles before updating the scissor rectangle, so an application's later clip change cannot cut earlier geometry. The [clip regression](../GlideProbe/README.md#clip-ordering-regression) reproduces F-16's two-row sky defect.
- Apply `texture-detail.patch`: implement single-TMU detail-weighted alpha and texture inversion used by F-16's terrain. Extended shaders compute the detail factor from texture-coordinate gradients and the game's bias, scale, and maximum. Shader model 2.x retains the existing fog pipeline; other modes retain their original shaders. Devices without the required shader capabilities keep reporting the mode unsupported. The shader-cache key includes the added states, and constant color uploads correctly specify one register rather than reading four registers from a four-float array. Patch hashes and feature flags are recorded in `build.json`. The [detail regression](../GlideProbe/README.md#texture-detail-regression) covers mode and parameter changes.

Every output includes the exact patched sources, build log, build commands, compiler identification, DLL hash, and PE import/export inspection. The original Git checkout is untouched. Builds are reproducible from inputs and scripts, but byte-for-byte determinism has not been established.

Two opt-in diagnostic changes can be combined:

```sh
python3 build.py --source /path/to/psvoodoo-source \
  --toolchain /path/to/llvm-mingw/bin --output /path/to/new-experiment \
  --gamma-logexp --d3d9-only
```

`--gamma-logexp` lowers the presentation shader's `pow` instructions to `log`, multiply and `exp`, retaining the gamma exponent. This bypasses an unsupported opcode in the tested Wine Vulkan shader translator. Numerical edge cases, especially zero and non-default gamma, need validation before shipping.

`--d3d9-only` bypasses D3D9Ex creation and exercises the wrapper's existing single-back-buffer D3D9 fallback. This avoids the unsupported flip/swapchain warnings observed in the tested Wine backends; the OpenGL game experiments use this option with the original gamma shader. It is an experimental build choice, not an automatic runtime capability decision.

`--trace-present` adds bounded first-frame tracing of presentation calls and state-block Apply/Release, and writes `trace-calls.json`. Use it only in diagnostic builds.

Use the [owned diagnostic](../GlideProbe/README.md) before installing games. The wrapper can be copied to a **disposable** prefix's `C:\windows\system32` before installation or hardware detection. For a reproduction explicitly requested in an existing app, use that app's own DLL override, preserve any previous override, and record how to undo it. Keep shared and released filesystem ZIPs unchanged during the test. No Microsoft redistributables, nGlide DLLs, or 3dfx splash binaries are part of this build.


## Prepared Motorhead experiment

After preparing the matching runtime and installing the demo as recorded in [GLIDE_SUPPORT.md](../../GLIDE_SUPPORT.md), run from the repository:

```sh
python3 project/mac-xcode/Boxedwine/BoxedwineUI/Tools/PsVoodoo/run-motorhead.py --trace-exceptions
```

This requires the local research artifacts; it does not download or install dependencies. It checks filesystem version 11, creates a separate `root-motorhead-play11` on first use, and saves a new timestamped log for each launch. It leaves the regular native app library untouched. There is no default time limit; optional `--timeout SECONDS` limits an unattended test. Ctrl-C terminates the isolated runtime if in-game exit does not complete. Do not run two instances against that test prefix.

Visible 3D racing and camera input are established. One player run exited normally. The recorded overlap-copy fault has a regression test and fix, and the rebuilt runtime cycled attract scenes for over five minutes without that fault. The user confirmed that Motorhead ran well and the keyboard controls worked; that playtest exited normally. The isolated Motorhead compatibility goal is complete for this configuration. Production integration and redistribution review remain separate work.
