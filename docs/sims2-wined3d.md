# Sims 2 Double Deluxe: native OpenGL investigation

Local development work, November 8, 2026. Wine 11.0, native x64 Windows
Boxedwine, RTX 4070 SUPER, Sims2SP4.exe 1.8.0.80.

## Separate startup failures

1. The game requires a 60 Hz fullscreen mode. The XRandR change in this branch
   exposes 60 Hz alongside the host refresh rate without changing the monitor.
2. The game's texture-memory probe exhausts the guest address space when Wine
   advertises the host's 12 GiB. A per-game `VideoMemorySize="256"` bounds it.
3. The game switches to software vertex processing and calls
   `SetVertexShaderConstantF(0, ..., 1024)`. Stock Wine rejects this at 256.

`tools/buildWine/patches/wine11_d3d9_swvp_float_constants.patch` addresses the
third issue. Software and mixed devices retain 8,192 floating-point constants;
hardware devices retain their original limit. State blocks include the larger
bank. The legacy GLSL backend retains Wine's sparse uniform path for low
registers and reads extended registers through eight 16 KiB uniform blocks.
The low-register count reserves space for integer/boolean constants, clip
planes, and fixup uniforms. Relative reads resolve shader-local `def` values
in the shader, so switching programs does not alter application constants.
Hardware-device uniform caches and upload buffer sizes remain unchanged.

The review follow-up bounds state-block bitmap scans to the highest changed
register and tracks extended dirty ranges separately for each GL context.
The buffer is allocated once; subsequent draws upload only changed extended
registers. Vertex-stage buffer rebinding invalidates the cached SWVP bindings.
Buffer deletion runs in GL context cleanup after the owning context is current.

The expanded constant/state-block/readback probe passes 482 checks with the A8
override, including high-only updates, uniform/UBO boundaries, shader-local
constants, fixed-function transitions, and state-block application after draws.
The separate GL cleanup probe passes 17 checks with `csmt=0`; it protects a
buffer with the same name in an unrelated WGL share group during device release.
A fresh 20,000-draw low-register comparison measured warm-loop medians of
0.065740 seconds for the original Wine build and 0.066520 seconds for the fixed
build. These are microbenchmark timings, not Sims 2 frame-rate measurements.
Review logs and reproducers are under `tmp/sims2-review-20261108`.

This implements extended float constants using the host GPU. It is **not** a
CPU vertex-shader interpreter or complete D3D9 software vertex processing.
Extended integer/boolean registers, software-only shader draw validation, and
shader execution in `ProcessVertices` remain outside this patch. Extended GLSL
execution requires uniform buffer support and eight vertex-stage bindings;
the tested path is native OpenGL. WebGL, Wine's Vulkan backend, and other GPUs
have not been qualified.

## Black squares under Sims

This is a separate compatibility issue in the A8 shadow path. D9VK uses a
capability workaround for the same visible symptom:
https://github.com/misyltoad/d9vk/commit/58daddb31ccbcadb6a878104545e42526609ad77

Follow-up investigation compared native Windows D3D9 with the original Wine
package, rather than treating that workaround as proof of fault:

- The game creates 128x128 A8 shadow render targets. The captured final shadow
  draw uses no pixel shader, selects texture RGB and alpha, disables the next
  texture stage and alpha testing, and blends with `DESTCOLOR / ZERO`.
- Reproducing that combination on native Windows also produces black RGB.
  A8 contains only alpha; its missing RGB channels read as zero under the
  [D3D9 format specification](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dformat).
  Multiplying the floor color by those RGB values blacks out the entire quad,
  regardless of the shape stored in alpha. This supports a format/render-state
  mismatch in the game's selected path on this system.
- Wine also has independent A8 render-target defects. Clearing with
  ARGB `0x4070b0e0` stores `0x70` instead of alpha `0x40`; a fixed-function draw
  with `0x90c02050` stores `0xc0` instead of `0x90`. The OpenGL R8 backing texture
  receives red rather than alpha in these paths. The tested pixel-shader write
  and ordinary A8 texture sampling agree with Windows.

`tools/wineTests/tests/d3d9_a8_shadow_probe.c` preserves the readback test.
Native Windows passes 180 checks; the original pinned Wine package fails five
pixel comparisons. The earlier patched Wine build reproduces the same clear
and fixed-function failures. Fixing those writes alone cannot change the
final shadow draw's multiplication by zero RGB. A general A8 implementation
fix still needs separate work and coverage for blending and channel masks.

A temporary build rejecting only A8 queries with `WINED3D_BIND_RENDER_TARGET`
also removed the rectangles under Cassandra and Mortimer at the Goth piano.
The trace retained ordinary A8 textures and showed the shadow targets changing
to `B8G8R8A8_UNORM`; the game log still reported High shadows. Thus hiding all
A8 textures is broader than necessary for this tested installation. That
narrower override was initially experimental. The patch now uses that narrower
render-target check, while the logging remains experimental. No general A8
rendering fix is included.

This does not establish how the game's original supported GPUs or older
drivers behaved. A capability change could expose a path that was rarely used
on those systems; that historical explanation remains unverified. Captured
states and outputs are in `tmp/sims2-swvp-20261108/a8-*`.

`wine11_wined3d_disable_a8.patch` adds the DWORD `DisableA8`. When enabled,
WineD3D's format-capability query returns `D3DERR_NOTAVAILABLE` for A8 render
targets, including queries for render-target blending. Ordinary A8 textures
and other formats retain normal support. The option does not change the game
shaders or lower the shadow quality setting.

When `DisableA8` is absent, the default follows the emulated Windows version:

| Emulated version | A8 render-target default |
| --- | --- |
| Before Vista (Windows major version below 6, including 98, Me, 2000, XP and Server 2003) | Hidden (`DisableA8=1`) |
| Vista and later | Normal support (`DisableA8=0`) |

WineD3D reads the process's Windows version at initialization, including Wine's
per-executable version setting. Restart the game after changing the version.
Explicit settings take precedence: environment, then application registry,
then global registry, then the version-based default. DWORD `0` restores
normal support even on XP; DWORD `1` hides the targets even on Vista or later.
No registry value is written for the automatic default, so changing versions
does not leave behind a stale setting. If the version query fails, normal
format support is retained.

This is a Boxedwine compatibility policy, not an assertion that pre-Vista GPUs
lacked A8 render targets. Older applications using those targets correctly may
need the explicit `0` override. The policy does not repair Wine's separate A8
clear/fixed-function defects.

For this executable, the saved prefix settings are:

```reg
[HKEY_CURRENT_USER\Software\Wine\AppDefaults\Sims2SP4.exe\DllOverrides]
"d3d9"="builtin"

[HKEY_CURRENT_USER\Software\Wine\AppDefaults\Sims2SP4.exe\Direct3D]
"VideoMemorySize"="256"
"DisableA8"=dword:00000001
```

For a temporary test, override the existing native DLL selection using
`WINEDLLOVERRIDES=d3d9=b` and set `WINE_D3D_CONFIG=DisableA8=1`.

## Filesystem video-memory default

The filesystem builders now seed the global `VideoMemorySize` string to `512`
for new prefixes. Both regular and web variants use `video_memory_size_mb` in
`tools/buildWine/filesystem_wine11.json`; existing global settings and per-app
overrides are preserved. The staged v26 ZIPs and installed application prefixes
have not been rewritten. This is pending the next filesystem release.

On November 8, 2026, a D3D9 probe using a prefix seeded by the new builder code
reported exactly 536,870,912 available bytes (512 MiB). Adding a 256 MiB
AppDefaults override for that executable returned 268,435,456 bytes, confirming
precedence. The 103 build tests and 31 web builder tests passed. A Sims 2 launch
with the temporary `WINE_D3D_CONFIG=VideoMemorySize=512` setting reached
neighborhood selection; the game's own texture-memory estimate was 536 MB.
That run was stopped after the startup check; the installed game's saved
256 MiB override was preserved. Probe output and logs are in the same local
artifact directory as the earlier investigation (`memory512.txt`,
`memory-app256.txt`, `game-memory512-*`, and `memory-default-*-tests.log`).

512 MiB is a compatibility default, not a 32-bit architectural limit.
[The 3DMark Vantage whitepaper](https://s3.amazonaws.com/download-aws.futuremark.com/3DMark_Vantage_Whitepaper.pdf#page=15)
lists 256 MB for Performance and 512 MB for the higher presets; it does not
establish a maximum requirement for all 32-bit applications.

## Focused regression

Build `tools/wineTests/tests/d3d9_swvp_constants_probe.c`:

```sh
i686-w64-mingw32-gcc -O2 -Wall d3d9_swvp_constants_probe.c -ld3d9 -lgdi32 -o swvp.exe
```

The optional first argument is a Windows output filename. Passing
`disable-a8` as the second argument additionally checks that A8 render targets
are hidden while ordinary A8 textures remain available. Configure the version
or override externally; the test does not change registry keys.

The test checks hardware/software/mixed caps, the 1,024/8,192-register upload
range, overflow rejection, Get/Set, mixed-device mode switches, recorded and
explicitly captured state blocks, and pixel readback from constants 256,
1,023, and 8,191. It also checks a shader-local constant through relative
addressing. Native Windows omits extended constants in a state block's initial
implicit capture, so the test explicitly calls Capture before comparing them.

Results on the development machine:

| Build | Checks | Failures |
| --- | ---: | ---: |
| Native Windows D3D9 | 207 | 0 |
| Original Wine 11 filesystem | 207 | 64 |
| Patched WineD3D, DisableA8 enabled | 210 | 0 |

The version-dependent, render-target-only build also passed all 210 checks.
The separate `d3d9_a8_caps_probe.c` covered 16 configurations (160 checks):
98, Me, 2000, XP, Server 2003, Vista, 7 and 10 defaults; both directions of
global, application and environment overrides; and application-specific
Windows versions overriding the prefix version. All final cases passed on
the native OpenGL host. The initial Windows 10 version assertion used
`GetVersionEx`, which reports 6.2 for this unmanifested executable; the corrected
probe uses `RtlGetVersion`, and the Windows 10 rerun passed all ten checks.
The capability checks themselves passed in both runs. Final results are in
`tmp/sims2-swvp-20261108/a8-version-final-results.json`, alongside the individual
logs and `a8-version-swvp-result.txt`. This does not qualify WebGL or a packaged
filesystem release.

The patched OpenGL game reached Pleasantview and the Goth household; Cassandra
and Mortimer rendered and animated. The first household run revealed the black
shadow rectangles. With `DisableA8=1`, the rectangles were absent in the same
household, including under Cassandra beside Mortimer's piano. The game log
still reported `simShadows=true`, `optionShadows=3`, and `Shadows=3` (High).
The separate fullscreen-mode probe passed 80 checks with the final modules.
This is a household smoke test, not a full playthrough.

After saving the per-game registry settings, launching through the normal
Boxedwine UI also reached neighborhood selection with native OpenGL and no
temporary environment overrides. That smoke test ended at the menu.

Artifacts are under `tmp/sims2-swvp-20261108`; the earlier fullscreen-mode and
DXVK investigation is under `tmp/sims2-dx9-20261108`. The isolated Wine build
is `/home/james/boxedwine-sims2-swvp/wine-git` in WSL, copied from the existing
revision-26 build with its Boxedwine patches. The installed application uses
development overrides in `root/opt/wine/lib/wine/i386-unix/` for `d3d9.dll.so`
and `wined3d.dll.so`. Its pinned package ZIP has not been rewritten. Installed
initial binary hashes are in `tmp/sims2-swvp-20261108/installed-hashes.json`;
the newer version-dependent A8 build is installed locally and recorded in
`installed-a8-version-hashes.json`. The saved per-game `DisableA8=1` and
`VideoMemorySize=256` settings remain in place. The prefix registry backup is
`user-before-swvp.reg` in the same directory.

The earlier DXVK `d3d9.dll` and `dxvk.conf` remain beside the game executable
for rollback. Setting the per-game `d3d9` override back to `native` selects
that path again. The current `builtin` setting selects patched WineD3D.

Both patches are selected for Wine 11.0 in `wine_builds.json`. These local DLL
tests do not qualify a filesystem release. Follow `tools/buildWine/RELEASING.md`
before publishing a new revision, including rebuilding all dependent Wine
modules and qualifying both packaged variants.
