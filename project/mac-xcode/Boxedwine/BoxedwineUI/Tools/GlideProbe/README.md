# Glide loading and rendering diagnostic

This developer-only program tests a Windows Glide wrapper inside a disposable Wine prefix. It is not included in the native app. It declares only the small ABI surface it exercises and does not use 3dfx SDK headers.

Build with an i686 Windows compiler, for example LLVM-MinGW:

```sh
i686-w64-mingw32-clang -O2 -static glide-probe.c -o glide-probe.exe
```

Put the program in a writable guest directory. Put the wrapper either next to it or in the disposable prefix's `C:\windows\system32`; the latter tests preinstalled loading. For OpenGlide9x, a local `OpenGLid.ini` containing `InitFullScreen=0` and `NoSplash=1` requests a window and disables the optional splash. The probe supplies an owned Win32 window and requests windowed mode through psVoodoo's optional `setConfig` export.

```text
glide-probe.exe glide2x.dll load-only
glide-probe.exe glide3x.dll load-only
glide-probe.exe glide2x.dll
glide-probe.exe glide3x.dll
glide-probe.exe glide2x.dll render 24000
```

Loading mode must print `PASS_LOAD_ONLY` and return zero. Rendering mode opens a 640×480 context, clears/swaps blue, green and red fields for about twelve seconds, then closes the context and library. It must visibly show those fields and print `PASS frames=…` with exit zero. A positive context or frame count alone does not prove correct rendering. Missing exports and failed loads/contexts return nonzero. A hang or modal error needs an external timeout.

The optional fourth argument sets the duration in milliseconds. In the September 2026 psVoodoo runs, a failed guest context still resulted in a shell exit code of zero. Check the diagnostic's explicit result markers and visible output; do not rely on the shell exit code alone.

`d3d9-probe.c` is a separate control program that clears/presents through D3D9 without loading any Glide wrapper:

```sh
i686-w64-mingw32-clang -O2 -static d3d9-probe.c -ld3d9 -o d3d9-probe.exe
```

Run `d3d9-probe.exe` with the same Wine/backend configuration. The optional `state-block` mode captures and restores all D3D9 state each frame, reproducing the WineD3D bit-scan hang fixed in `Jit::bsStartFlags`. An optional second argument sets the duration in milliseconds, for example `d3d9-probe.exe state-block 45000`. `D3D9_API_PASS` means the calls succeeded; check the colored window as well. For the isolated Wine 11 Vulkan comparison, the guest environment was `WINEDLLOVERRIDES=d3d9=b WINE_D3D_CONFIG=renderer=vulkan`. Do not combine that with Boxedwine's `-dxvk` switch, which installs and selects a different D3D9 implementation.

This tests neither triangle/texture correctness nor game compatibility. See [the prototype findings](../../GLIDE_SUPPORT.md) for observed results and the remaining runtime blockers. Never use the regular app library or overwrite a published Wine package for this diagnostic.


## Framebuffer color regression

`lfb-probe.c` exercises direct RGB565 framebuffer writes through the public Glide 2 ABI:

```sh
i686-w64-mingw32-clang -O2 -static lfb-probe.c -o lfb-probe.exe
lfb-probe.exe
lfb-probe.exe benchmark
```

Put the tested `glide2x.dll` next to the executable in a disposable prefix, using the same WineD3D OpenGL and filesystem-11 setup. It writes all 65,536 RGB565 values, performs partial writes of cyan, magenta, and white, changes the guest alpha-test/blend state, then reads pixels back. It checks the complete palette, background, and four-pixel border. Success is the explicit `LFB_COLORS_PASS mismatches=0` marker; the host's exit status alone does not establish success. An optional argument measures 120 additional small writes. This is a focused pixel/copy benchmark, not a game frame-rate measurement.

The test catches psVoodoo's old use of cyan as a marker for unwritten pixels and its dependence on the game's alpha-test state. It does not test Glide's optional LFB pixel pipeline, alternate requested LFB formats, stereo, or the final host presentation image. Game screenshots remain necessary to check that last step.

## Clip ordering regression

`clip-probe.c` queues a full-screen red rectangle, then changes the clip rectangle to exclude the first two rows before reading the back buffer. Earlier geometry must retain its original clip. The old psVoodoo build loses exactly 1,280 pixels; nGlide 2.10 and the patched wrapper print `CLIP_ORDER_PASS mismatches=0`.

```sh
i686-w64-mingw32-clang -O2 -static clip-probe.c -o clip-probe.exe
```

## Texture detail regression

`detail-probe.c` draws 48 panels through the texture-alpha/blending path. It varies texture gradients, detail bias/scale/maximum, alpha inversion, texture alpha, and transitions between local/zero/detail modes. It compares RGB565 readback against Glide's documented blend equation, allowing three aggregate channel steps for quantization. The required success marker is `DETAIL_BLEND_PASS mismatches=0`.

```sh
i686-w64-mingw32-clang -O2 -static detail-probe.c -o detail-probe.exe
detail-probe.exe
detail-probe.exe reference
```

`reference` records pixels from another wrapper without asserting that it implements the equation. In the nGlide 2.10 control run, the opaque texture produced a roughly one-half blend across the tested gradients and detail controls. That is useful comparison evidence, not the expected result for this regression. This test uses constant texture colors and affine gradients; it does not establish hardware-exact LOD quantization, perspective interpolation, mipmap selection, or all texture-combiner modes.
