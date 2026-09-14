# Bundled Glide support research

Updated September 12, 2026. The user has added the tested psVoodoo DLL, including the Descent 3 texture-memory query fix and the F-16 framebuffer, clipping, terrain-detail, and camera-turn shadow fixes, to the Wine 11 filesystem. The native demo catalog references that refreshed package; no vendor inquiry has been sent.

**Current result:** Motorhead renders its title and races through psVoodoo with three Boxedwine fixes. A player run exited normally; a rebuilt run cycled through attract scenes for over five minutes without the earlier overlap-copy fault. The user confirmed that the game ran well and the keyboard controls worked. The final playtest also exited normally. Detailed results and reproduction instructions follow the historical experiments below.

**Catalog integration:** release `26R2-native-preview-5` includes Descent 3, Motorhead, and F-16. The current Wine 11 package is 167,016,911 bytes, SHA-256 `a3367c4e977dbbe0295ce3b4b102bf7b0baa1278ea121cefe6065d561b960452`. It contains the final F-16-tested `glide2x.dll` (904,192 bytes, SHA-256 `55aee80ac08478f8d965cede9e07b011fa046ec02f84ebc18e78b1abaf44a939`), including the shadow fix from psVoodoo commit `e83947c2f4d800831db0643ad589fba1c98b2457`. The September 12 refresh changed only that Glide DLL. The September 13 refresh changes only CNC DDraw; the Glide DLL stays identical. The native helper includes the matching filesystem-11 graphics ABI. The experiments below describe the package and overlays available at each stage.

The earlier September 12 Debug rebuild passed the package/catalog validators and native bundle audit. Its bundled ZIP and the shared library ZIP both match the new fingerprint. At the user's request, sixteen local Wine 11 app pins were migrated with a library backup; other settings, Wine versions, removed apps, and local DLL overrides were preserved. Motorhead launched from the regular UI using the new shared ZIP with no local Glide override, visibly rendered textured racing and the HUD, and shut down through Stop App. Evidence is in `tmp/native-ui-test/wine-refresh-20260912/`.

**Current direction:** retain WineD3D's **OpenGL** backend. The user deferred Vulkan work and confirmed that the recent breaking change requires **filesystem version 11**. Use the unmodified catalog Wine 11.0 ZIP with `version.txt = 11`, together with a matching runtime; a Wine version name alone does not identify the filesystem ABI. Earlier Vulkan results below are archived experiments, not the next implementation steps.

**Release scope:** Glide 3 support is deferred by the user's decision. Keep psVoodoo for Glide 2 games and use Direct3D for Diablo II. Adding a Glide 3 wrapper is not a Mac release requirement. Run Another Program → Choose File remains available for users who want to install additional components themselves. The user installed nGlide for the F-16 rendering comparison; this does not establish broader game compatibility or redistribution permission.

## nGlide

The [official product page](https://www.zeus-software.com/downloads/nglide) identifies nGlide 2.10 as freeware. It documents Glide 2.11 (`glide.dll`), 2.60 (`glide2x.dll`) and 3.10 (`glide3x.dll`) support, translated to Direct3D or Vulkan. It does not provide an explicit grant for redistributing extracted DLLs in another product.

The [official installer](https://www.zeus-software.com/files/nglide/nGlide210_setup.exe) and Boxedwine's existing HTTPS mirror were downloaded for inspection, without executing either installer. Both are 3,301,587 bytes with SHA-256 `3cfcd03a923386c36685a772d24797fb78762cfbe63fe5676756091cf27da7a4`. The embedded `nglide_readme.txt`, section 9, states: “nGlide is the property of Zeus Software.” It separately assigns ownership of the three `3DfxSpl*.dll` splash plugins to 3Dfx Interactive. It contains no express redistribution permission.

Conclusion: permission for a preinstalled filesystem distribution is **unconfirmed**, not demonstrably prohibited. Neither freeware labeling nor existing bundles establishes permission for Boxedwine's proposed distribution. Obtain written permission covering extracted files, the filesystem package, and free and monetized releases before relying on this route. The splash plugins need explicit coverage or exclusion. This research does not establish permission for mirroring the original installer either.

The user supplied the text of [forum post 1001, nGlide Starter](https://www.zeus-software.com/forum/viewtopic.php?p=1001#p1001), posted by `3dfxer`. Its permission to distribute “this program” appears scoped to **nGlide Starter (`nglstart`)**, the separate command-line utility that changes and restores nGlide settings. The excerpt explicitly describes that utility's dependency on nGlide 1.04. It does not establish a redistribution grant for the nGlide wrapper DLLs. The live post could not be retrieved during this check; this interpretation is based on the user-provided excerpt, not a verified reading of the full thread.

Draft inquiry to `info@zeus-software.com`, the address in the readme:

> I maintain Boxedwine, an open-source emulator that runs Windows software through Wine. May we redistribute nGlide 2.10 as unmodified, preinstalled DLLs inside our Wine filesystem package, so users do not need to run its installer? We would retain your readme and attribution. Distribution would include free GitHub/boxedwine.org builds and a proposed Mac App Store build with free compatibility testing and possible optional paid support or advertising. Please specify which files we may include, required notices or conditions, and whether the separate 3Dfx splash plugins are covered or should be omitted. Would users be allowed to redistribute that filesystem with Boxedwine under the same permission?

## Open-source candidates

**OpenGLide:** the [maintained source mirror](https://github.com/voyageur/openglide) contains an [LGPL 2.1 license](https://github.com/voyageur/openglide/blob/master/LICENSE). It translates Glide to OpenGL and has a Windows `Glide2x.def` export definition. Its base API coverage is narrower than nGlide's three DLLs.

**OpenGlide9x:** [JHRobotics' fork](https://github.com/JHRobotics/openglide9x) explicitly adds Glide 3 support, threading, rendering fixes and incomplete additional texture-unit support. Its Makefile builds 32-bit Windows `glide2x.dll` and `glide3x.dll` with MinGW or MSVC. Its `LICENSE` is byte-identical to the upstream LGPL 2.1 text. Inspected revision: `b8ac1a32c98f9f8e8616aeffcf4b0af163b59b8f`.

LGPL 2.1 provides a route to distributing built libraries, including in paid products, with its source, notice, modification and applicable replacement/relinking requirements. It does not require users to run an installer. Supply the exact corresponding sources, patches and build scripts with the release's source materials; preserve the required notices and recipients' rights. This is distinct from approval of an App Store submission.

There is a concrete provenance issue to resolve before treating either tree as cleared for shipping: legacy `sdk2_*.h` files still carry proprietary 3Dfx notices. OpenGlide9x's [`gbanner.cpp`](https://github.com/JHRobotics/openglide9x/blob/b8ac1a32c98f9f8e8616aeffcf4b0af163b59b8f/gbanner.cpp) invokes the separate 3dfx Glide license. Trace the applicable grants and include their required notices, or replace/exclude those portions using properly licensed equivalents. Do not infer that a top-level LGPL file relicenses every third-party file. Also inventory the pthread9x dependency and other files used by the selected build. External proprietary splash DLLs are optional in the inspected loader and should not be copied from nGlide without permission.

## Initial OpenGL direction

The initial experiment evaluated OpenGlide9x, building the Windows DLLs alone, not the full SoftGPU driver stack. The inspected loader uses Windows OpenGL, making Wine's existing OpenGL path a plausible fit; this is an architectural inference, not a compatibility result. The user's subsequent preference for a Direct3D path moved the active evaluation to psVoodoo, below.

After resolving the source notices and testing, package the DLLs and default configuration in a new, versioned Wine filesystem and arrange automatic placement/loading before a game's installer or hardware detection runs. Existing published filesystem URLs must remain immutable. Test Descent 3 and Motorhead installation, device detection, rendering, input, fullscreen transitions and shutdown against the existing nGlide behavior. OpenGlide9x has no demonstrated Boxedwine compatibility yet and is not established as a complete nGlide replacement.

Local research downloads and the extracted readme are under `tmp/glide-redistribution-research/`; they are not app resources or release payloads.

## OpenGlide9x prototype results

Built both i686 Windows DLLs from revision `b8ac1a32c98f9f8e8616aeffcf4b0af163b59b8f` with LLVM-MinGW `20260908`, UCRT, using the upstream Makefile. No wrapper source changes. The Wine-targeted build omitted `HAVE_CRTEX`, `crtfix.o` and the pthread9x dependency; these supply compatibility with real Windows 9x. Static C++ runtime linking still leaves Windows/UCRT imports. No nGlide or 3dfx splash DLLs were included. `gbanner.cpp` is not in this Makefile's source list.

The toolchain archive SHA-256 was verified against its official GitHub release asset: `d1dc5d1ecf3a3ced5ed5544c72f1acd0c8e84eb3024d520ecc6b143eec62a149`. The generated `prototype/source/config.mk`, exact sources, build log, DLLs and runtime harness remain under `tmp/glide-redistribution-research/`. Make overrides:

```sh
make -j4 all DEPS_EXTRA= \
  'GLIDE_LIBS=-static -lgdi32 -ladvapi32 -luser32' \
  'DEFS=-DWIN32 -DCPPDLL -DHAVE_MMX -DNEW_FOG -DNO_TMU0_LIMIT' \
  GLIDE2_DEF=glide2x.def GLIDE3_DEF=glide3x.def
```

The owned [Glide diagnostic](Tools/GlideProbe/glide-probe.c) uses dynamic exports without including a vendor SDK. Both DLLs loaded and unloaded successfully under catalog Wine 11 / filesystem 11: both guest exit codes and the Boxedwine host exit code were zero. This is a **DLL loading result only**. The separate render attempt reached Glide 2 initialization and its version query, but did not produce a frame:

- The older Wine 11 / filesystem 7 and Wine 10 / filesystem 5 packages lost their old `libGL.so.1` through this runtime's existing `fileSystemVersion < 10` handling. Wine reported OpenGL disabled; Wine 10 displayed a pixel-format error.
- Catalog Wine 11 / filesystem 11 invoked unsupported OpenGL callback `3099`. A byte-level search found that direct interrupt call in its `lib/libEGL.so.1`, not its `libGL.so.1`. This checkout has GLX callbacks but no corresponding EGL callback implementation.
- Masking EGL with an invalid file only in the disposable overlay forced GLX loading, but startup then stalled after GLX symbol resolution. A process sample was saved as `prototype/glx-startup.sample.txt`. The cause of that second stall is unresolved; it does not establish a wrapper defect.

The stalled processes were stopped. Descent 3 and Motorhead downloads were inspected, but neither game was installed or tested because the prerequisite graphics probe failed. No game compatibility or performance claim follows from this experiment. Regular app data, published ZIPs, runtime source, and product build settings were not changed for the prototype.

The original open-source Glide tree contains later headers with an express [3dfx license notice](https://github.com/sezero/glide/blob/master/glide2x/sst1/glide/src/glide.h) and the [3dfx license text](https://github.com/sezero/glide/blob/master/glide3x/COPYING). These provide a concrete provenance/replacement avenue; they do not by themselves prove that every older header in a wrapper is covered. No license notices have been rewritten or removed.

## Direct3D candidates and backend choice

The user's WebGL and Vulkan goals favor evaluating a Direct3D implementation before further OpenGL-wrapper integration:

| Candidate | Output | Source and coverage |
| --- | --- | --- |
| [psVoodoo](https://psvoodoo.sourceforge.net/) | D3D9, optional D3D9Ex in current source | LGPL notices; Windows `Glide2x.dll`; documented partial API coverage. The website advertises 0.13, but the official Git repository HEAD inspected here is `8ebfc3c45f8f067af852720f9857d4641bf64252`, version 0.18, dated August 25, 2026. |
| [dgVoodoo 1.x](https://github.com/dege-diosg/dgVoodoo) | Legacy DX7/DX9 | Author explicitly licenses source under LGPL 2.1. Windows Glide 2.11 and Glide 2.x modules; no Glide 3 module listed. Archived, substantial assembly and older build tools. |
| [dgVoodoo 2](https://github.com/dege-diosg/dgVoodoo2) | D3D11/12 | Separate implementation; the 1.x source license does not cover it. Its public repository is not the wrapper implementation. Do not treat it as the open-source bundling solution. |
| [nGlide](https://www.zeus-software.com/downloads/nglide) | Direct3D or Vulkan | Covers Glide 2.11, 2.60 and 3.10; extracted-file redistribution remains unconfirmed as above. |

psVoodoo is the active experimental candidate, not yet a selected dependency. Its official source was cloned from `https://git.code.sf.net/p/psvoodoo/psVoodoo` and inspected locally. `Device.cpp` calls `Direct3DCreate9`, optionally tries D3D9Ex, and uses software vertex processing. `DevCombineStateShader.cpp` emits `ps_1_1` shader text through `D3DXAssembleShader`; the current presentation/gamma shader in `RenderBuffer2D.cpp` requires `ps_2_0`. Other code uses D3DX surface operations. This source also retains old proprietary notices in `sdk2_*.h`; its individual LGPL notices and SourceForge metadata do not close that provenance question.

The intended Vulkan route is Glide → D3D9 → [DXVK](https://github.com/doitsujin/dxvk) → Boxedwine's Vulkan backend. The browser route would use WineD3D and Boxedwine's browser graphics translation. D3D9 does not directly produce WebGL, and WebGL cannot consume desktop OpenGL fixed-function calls unchanged. Validate the selected WineD3D configuration, shader translation, render-target formats, texture operations and framebuffer reads in a browser. D3D9 is a useful shared input API, not a guarantee of compatibility or speed. Both open-source Direct3D candidates leave Glide 3 coverage to investigate separately.

## psVoodoo prototype results

Built the 32-bit `glide2x.dll` from official revision `8ebfc3c45f8f067af852720f9857d4641bf64252` (0.18). The developer-only [build script and instructions](Tools/PsVoodoo/README.md) preserve the source revision, local portability adjustments, compiler identity, commands, resulting imports/exports and DLL hash. The wrapper is 896,512 bytes in these builds. The separate MFC settings application is not required or built.

Imports include `d3d9.dll`, `d3dx9_43.dll`, Windows system DLLs and UCRT API sets. The tested Wine 10/11 packages already supply D3DX9; no Microsoft DirectX installer or proprietary helper DLL was added. MinGW, bare and Microsoft-style stdcall export names are available. Loading the wrapper from the disposable prefix's `C:\windows\system32` succeeded, followed by `grGlideInit` and the compatibility version string `Glide 2.45`. This establishes that preinstalled placement works technically, not that the library is ready to distribute.

Tests used an isolated copy of the Release runtime, Apple M4 / macOS 26.4.1, and separate writable prefixes. The catalog Wine 11 filesystem was version 11, SHA-256 `8e8d4e8f1506376feac607bb391f2f383d21b53e42a4736c908007465304a964`; Wine 10 was filesystem 5, SHA-256 `f0ed13eaf0c11bc95b229e2a747f04167c4e63445dc274d122758bd7e84b5572`. None of the published ZIPs, regular app library, runtime source or production graphics dependencies was changed.

| Configuration | Observed result |
| --- | --- |
| Wine 11 + packaged DXVK 2.5.2 | Wrapper loads; context fails. DXVK rejects the Apple M4 Vulkan 1.2 adapter because this DXVK requires Vulkan 1.3. |
| Wine 11 + official DXVK 1.10.3 D3D9 | Adapter enumeration succeeds; Vulkan device creation fails with `VK_ERROR_FEATURE_NOT_PRESENT`. MoltenVK identifies the fifth and 39th feature flags, corresponding to `geometryShader` and `shaderCullDistance`. Context remains zero. |
| Wine 10 + WineD3D OpenGL + current `libGL` in the private overlay, with explicit `LD_LIBRARY_PATH` | Repeated unsupported interrupt callback `3139`; stopped. This is a guest/host graphics bridge compatibility problem to investigate, not evidence of psVoodoo game compatibility. |
| Wine 11 + WineD3D Vulkan, baseline psVoodoo | Context opens, but the window is black. Wine's SPIR-V compiler rejects `pow` in the presentation/gamma shader; it also reports unsupported flip/swapchain behavior. |
| Same, with experimental `--gamma-logexp` | The reported shader opcode failure disappears; the window remains black with the D3D9Ex swapchain path. Stopped after timeout. |
| Same, with `--gamma-logexp --d3d9-only` | A blue test frame is visibly rendered. Color cycling and normal completion do not occur; the process times out. **First-frame result only.** |
| Same two changes, `WINE_D3D_CONFIG=renderer=vulkan,csmt=0`, instrumented probe | Context opens. `SWAP_END 0` and `SWAP_END 1` appear, followed by `SWAP_BEGIN 2` with no matching return. Disabling Wine's command thread does not resolve the third-swap stall. |
| D3D9 control without a Glide wrapper, Wine 11 + WineD3D Vulkan | A colored frame is visibly rendered, `D3D9_API_PASS frames=683`, guest shell exit zero, normal Boxedwine shutdown and host exit zero. This control only exercises clear/present, not psVoodoo's textured presentation or a game. |

The DXVK 1.10.3 archive came from the [official release](https://github.com/doitsujin/dxvk/releases/tag/v1.10.3), with downloaded archive SHA-256 `8d1a3c912761b450c879f98478ae64f6f6639e40ce6848170a0f6b8596fd53c6`. Only its x32 D3D9 DLL was used in a private overlay. The [DXVK-macOS maintainer's release notes](https://github.com/Gcenx/DXVK-macOS/releases) explicitly restrict that fork to D3D10/11 and remove D3D9 from the repack, so it was not substituted as a presumed D3D9 fix.

The experimental shader change expresses gamma through log/multiply/exp instead of `pow`; it does not deliberately disable gamma. Gamma edge cases and numerical equivalence remain untested. The D3D9-only change selects the existing fallback rather than changing the app's production backend. Neither is a shipping compatibility policy.

The test harness and raw/ANSI-cleaned logs are under `tmp/glide-redistribution-research/prototype/`, with names matching the tests (`ps-dxvk11`, `ps-dxvk110`, `ps-wined3d10`, `ps-winevulkan11`, `ps-logexp11`, `ps-d3d9only11`, `ps-csmt0-11`, and `d3d9-winevulkan11`). `ps-csmt0-11.sample.txt` contains a host thread sample during the third-swap stall. Baseline and experimental source/build inventories are in adjacent `psvoodoo-repro-*` directories. Guest `$?` misleadingly reported zero after a failed context in the DXVK runs; the probe's own markers and visible results take precedence over that shell status.

**Milestone at that stage (see subsequent results below):** test baseline psVoodoo through WineD3D OpenGL using filesystem 11 and its matching runtime, then require visible color cycling and a clean shutdown. Add triangle/texture and gamma checks, followed by Motorhead and Descent 3 installation, hardware detection and gameplay. Vulkan and browser/WebGL work are deferred. At that stage no game had been tested with psVoodoo, and no Glide 3 replacement had been established. Resolve the legacy header provenance and provide the applicable full license/source materials before packaging a new, immutable filesystem release.

### Filesystem/runtime mismatch found

The active `master` checkout is `296ff0fa`; its GL callback table ends at 2922 entries. The catalog filesystem 11 `libGL.so.1` invokes callback 3139 from `glXGetProcAddressARB`, and its EGL library invokes callback 3099. Matching implementations exist in local remote-tracking branch `origin/james/webgl` (head `b44fb6ca`), including `kGlProcAddressAvailable` and EGL dispatch. The version 11 manifest is also on that branch. Consequently, earlier unsupported-callback results tested an incompatible runtime/filesystem combination and must not be treated as psVoodoo failures.

The matching runtime built successfully from an archived copy of that branch under `tmp/glide-redistribution-research/runtime-fs11/`, without merging or replacing the native UI work. The isolated Release build targets arm64/macOS 15, with a separate diagnostic bundle identity. The runner `prototype/run-fs11-opengl.py` verifies `version.txt = 11` and rejects `--dxvk`. The new baseline prefix has no substituted `libGL`, masked EGL files or Vulkan-specific wrapper patches.

With `WINEDLLOVERRIDES=d3d9=b WINE_D3D_CONFIG=renderer=gl`:

- Baseline psVoodoo opens an OpenGL context (`CONTEXT 1`), with no unknown callback errors. The probe returns from its first two `grBufferSwap` calls, then stalls at `SWAP_BEGIN 2`. The inspected window was black. It required external termination.
- A separate build using only `--d3d9-only` retains the original gamma shader and removes the D3D9Ex swap-mode warning, but still stalls at the third swap. It also required termination. This does not establish that D3D9Ex caused the stall.
- The independent D3D9 control reports `D3D9_API_PASS frames=679`, guest shell exit zero, and `Boxedwine shutdown`; the host executable exits with status 1. Correct visible color cycling was not established in that control run, so this is an API-completion result rather than a full visual pass.

Logs are `ps-opengl11`, `ps-opengl-d3d9-11`, and `d3d9-opengl11` under `prototype/`, with ANSI-cleaned copies. The machine-readable summary is `psvoodoo-opengl-fs11-results.json`. At that stage the third-swap issue was unresolved and no demo had been installed with this wrapper.

During the archived project's first build, its legacy scheme pre-action ran `fetchDepends.sh`, removed the shared `lib/mac` directory, then failed to download. The directory was restored from the official `/v/depend/mac/4/maclib.zip` archive (SHA-256 `060d52bacf869e99111cbf16a98087acc2f3d0ee55013a84088e1b08845040d1`). SDL's header matches the existing native app byte-for-byte; SDL, OSMesa and MoltenVK binary UUIDs match that app's dependencies. The scratch scheme's dependency-download actions are now disabled and its Mac dependencies are private copies. The existing native app bundle and regular app library were not replaced.


### Motorhead investigation: CPU hang fixed

The third-swap stall was reproduced without psVoodoo by adding a captured D3D9 state block to the independent D3D9 control. Its fourth `Apply` hung. The matching runtime's JIT map and host sample placed the loop in WineD3D's stream-frequency state restoration, repeatedly executing `bsf` followed by `cmove`.

`Jit::bsStartFlags` incorrectly omitted the runtime lazy-flags reset when the preceding generated instruction had left the compiler hint at `FLAGS_NONE`. A jump/return can enter that instruction with different runtime flags. This is expressly disallowed by the existing `currentLazyFlags` contract. The fix always emits `storeLazyFlagType(FLAGS_NONE)` when a scan's flags are needed. It applies to both BSF and BSR, 16/32-bit register and memory forms.

`source/test/cpu/testBit.cpp` now re-enters the second compiled scan with deliberately opposite lazy ZF, testing both zero and nonzero operands. All 16 added cases fail before the fix and pass after it. The selected 90-test range (start 600, one thread, fast operand selection) passes after the fix. Isolated build logs: `runtime-fs11/test-bsf-before.log` and `test-bsf-after.log`.

After the fix, the D3D9 state-block control completed 673 frames. psVoodoo with only `--d3d9-only` completed 1,635 frames over 30 seconds, progressed through all color phases, and exited normally. Both guests reported exit zero and Boxedwine shutdown; the host exits 1. The inspected psVoodoo window remained black, so these are API-completion results, not a visual rendering pass. Logs: `prototype/d3d9-state-fixed11.log` and `ps-fixed11.log`.

Motorhead's original 3DFX demo installer completed in `prototype/root-motorhead11`, with Windows 98 selected first using `winecfg -v win98`. Installed executable: `C:\Program Files\Digital Illusions\Motorhead Playable 3DFX Demo\motor.exe`. It loads psVoodoo from `system32`, reacts to Return and begins generating game shaders, but its window is also black. The current investigation is the native OpenGL presentation path. The regular native app, user library and catalog ZIP are unchanged. The CPU fix is in the main working tree and matching isolated runtime source; the filesystem-11 branch has not been merged.


### Native single-buffer presentation fixed

Framebuffer diagnostics showed that the D3D9 control produced the expected colors in its read framebuffer and the native default front buffer. The context correctly reported single buffering, but the visible Cocoa window remained black. Refreshing the drawable did not resolve this. Explicit `glFlush` before native presentation did: the D3D9 control visibly rendered green and completed 2,536 frames over 45 seconds.

`platform/mac/macOpenGL.mm` now submits queued drawing with `glFlush()` before `NSOpenGLContext::flushBuffer`. The latter alone did not present this single-buffered path. The fix is in the main working tree and isolated filesystem-11 runtime. Temporary SDL swap tracing, refresh and flush switches were removed from the final runtime source. With that final build, psVoodoo visibly cycles colors (green then red inspected), retaining the original gamma shader and the `--d3d9-only` build option.


### Motorhead reaches visible 3D racing

With both fixes, the original Motorhead Playable 3DFX Demo displays its title, countdown, textured track, cars and HUD using catalog Wine 11.0 / filesystem 11, Windows 98, psVoodoo's D3D9 fallback and WineD3D OpenGL. No Vulkan or gamma-shader workaround is active. The final color probe completed 2,444 frames in 45 seconds with guest exit zero and normal Boxedwine shutdown (host exit 1).

At this stage sustained driving control and clean game exit still needed confirmation; the later user playtest below resolves both checks. A fresh run started a player race, remained stationary at the start line, and changed from the cockpit to the external camera after repeated F2 input, establishing that camera input reaches the game. Brief synthetic Up events did not accelerate the car. The title enters attract mode after a short countdown; movement in a screenshot therefore does not prove user control. CUA supplies very brief keypresses, and the car motion seen after those events may have been attract playback. Escape reached a promotional screen, but that screen did not close in automated attempts. The same guest yield loop appears during normal rendering, so it is not sufficient evidence of a shutdown deadlock.

One extended run (`prototype/motorhead-stack11.log`) raised a guest write page fault at `00439415`, targeting `01D17000`. Its cause is unresolved. A fresh run captures Wine exception diagnostics. Audio output was initialized in the log but has not been audibly checked. These limits prevent calling the demo fully playable yet.

The developer launcher [run-motorhead.py](Tools/PsVoodoo/run-motorhead.py) reuses only a separate prepared test prefix, keeps timestamped logs and has no default time limit. Run it with Python from the repository; `--trace-exceptions` enables Wine exception logging. Arrow keys steer/accelerate/brake, F1/F2/F3 change camera, and Escape exits according to the bundled demo readme. Ctrl-C in the launcher terminal stops this isolated runtime. The main checkout's native UI is not yet running the filesystem-11 branch; this launcher uses the matching archived runtime.

Reproduction inputs and hashes are recorded in `tmp/glide-redistribution-research/motorhead-fs11-test.json`; `runtime-fs11/motorhead-runtime-fixes.patch` contains the three runtime fixes and the regression tests. The final runtime has no temporary stack or framebuffer tracing hooks. No game/wrapper binaries were added to app resources or a published filesystem.


### Profiled overlap-copy crash fixed

The fault at Motorhead address `00439415` is a `repne movsd` copying twelve dwords with destination one dword above source. This intentionally propagates the first value through the overlapping range. In `Jit::movsr`, the 64-bit copy implementation's overlap loops mistakenly tested the constant source/destination distance on every iteration. ECX continued decrementing through zero until the loop reached an inaccessible page. Both directions now test the remaining count inside the overlap guard.

The bug was hidden by the normal test profile: the vector implementation already terminates correctly. After more than ten observations of copies averaging eight through fifteen elements, `JitSSE::movsr` selects the 64-bit fallback. The new flat-address regression explicitly exercises both profiles, both repeat prefixes and directions, byte/word/dword elements, identical pointers and overlapping/boundary separations, with counts 0, 1, 12 and 17. The broken implementation faults after ECX underflow; after the fix all 384 combinations pass. The surrounding 40-test range (start 409, one thread, fast mode) passes. Logs: `runtime-fs11/test-movs-before.log` and `test-movs-after.log`.

The previous player-race run completed without external termination after 241 seconds: `MOTORHEAD_EXIT=0`, `Boxedwine shutdown`, host exit 1 (`prototype/motorhead-runs/20260910-173810-386453.log`). The promotional screen is therefore not established as a permanent hang. All three fixes are in the working tree and isolated runtime, with compiler diagnostics removed. Scene-transition retesting used the rebuilt runtime; physical keyboard control was subsequently confirmed in the user playtest below.

The rebuilt runtime was visually checked after more than five minutes of attract-mode cycling, returned to the title, and accepted Return to start a player race. Its log (`prototype/motorhead-runs/20260910-175502-657147.log`) recorded eight audio initializations and no unhandled page fault at that checkpoint. This is evidence that repeated scene initialization passes the earlier fault site, not a long-term stability guarantee.


### Motorhead goal completed: user playtest

On September 10, 2026, the user tested the rebuilt runtime and reported: “it ran well and the keys worked.” This confirms the sustained keyboard input check that brief CUA keypresses could not establish. The same run ended without external termination after 51.97 seconds, with `MOTORHEAD_EXIT=0` and `Boxedwine shutdown` (the legacy host executable returned 1). Log: `prototype/motorhead-runs/20260910-182849-445011.log`; launcher result metadata is the adjacent `.json` file.

Validated configuration: Motorhead Playable 3DFX Demo, psVoodoo 0.18 with the existing D3D9 fallback and original gamma shader, WineD3D OpenGL, catalog Wine 11.0 / filesystem ABI 11, Windows 98, and the matching `b44fb6ca` runtime plus the three fixes above. Runtime SHA-256: `b96aa8f7daba4ce35e084bb51da0acc4aecd8f5cc998c7f215ba4e21bf27db09`. The 90-test bit-scan range and 40-test string/related range passed, including the added regressions.

The isolated Motorhead compatibility goal is complete. Production native-UI integration, redistribution provenance and other games remain separate work; this result applies to this tested configuration.

## Native filesystem-11 integration

The native Debug app now bundles the refreshed Wine 11.0/filesystem-11 ZIP. Settings was switched from the older imported package to **Use Included Package**, then visibly reported Wine 11.0, filesystem 11, included with Boxedwine, and successful package validation. Existing apps with exact Wine references retain those references.

The graphics bridge was ported from `origin/james/webgl` at `b44fb6ca4653bbf219714182e7b546db380df639`: the extended callback table, proc-address discovery, EGL dispatch and drawable state, GL marshaling, SDL/macOS swap behavior, and corrected XVisualInfo layout. Existing GL/GLX callback numbers are retained. This was a targeted source integration, not a branch merge. The local CPU fixes, native hidden-window behavior, stdin helper controls, and single-buffered `glFlush` fix were preserved. The new EGL source is discovered by Xcode’s synchronized source group and was also registered in both explicit MSVC projects. Browser/Vulkan work and the branch’s unrelated CPU, filesystem, build-site and Wine patch changes were not imported. The SDL window friend declaration needed by the shared backend was retained.

Validation used the newly built native helper in a standalone diagnostic copy, with separate roots and the exact published ZIP. The Wine configuration integration passed for both filesystem 11 and the earlier filesystem 7: discover the default, set and verify Windows 98, set and verify XP, restore the default, and exit normally without showing a configuration window. The filesystem-11 psVoodoo probe reported `PASS frames=1342` and host exit zero. Its color output was not visually inspected. Motorhead visibly rendered textured track, cars and HUD; Escape returned from an attract race to the title. This was a bounded smoke check, not another full user gameplay test. It continued running and was force-stopped by the harness at the three-minute timeout after the cooperative stop deadline; normal Motorhead shutdown through the native helper remains to be checked. Descent 3 gameplay remains unverified.

A D3D9 check using filesystem 7 failed to create a context because `/lib/libGL.so.1` was hidden by the existing `StartUpArgs::apply()` policy for filesystems below 10. That policy already exists in `HEAD` and was unchanged here. The successful older-filesystem winecfg check establishes configuration compatibility, not old-filesystem OpenGL rendering.

The Debug native build and eighteen-image bundle audit passed. Local build/test evidence and the pre-integration source snapshots are in `tmp/native-ui-test/fs11-integration/`. Test roots are separate from the regular library. This check does not establish Windows/Linux/browser build or rendering coverage.

## Descent 3 demo 2: Glide startup crash, September 11, 2026

The user reproduced the crash from the regular native library and confirmed that the launcher was configured for 3dfx. The saved registry also had `PreferredRenderer=4` and `RenderingDeviceName="Voodoo Graphics Family"`. The access violation occurred at `main.exe` address `0051e5ac`, reading through `EBP=DEADBEEF`.

Disassembly identifies the failing pointer as a texture-cache allocation. Initialization at `0051b720` calls `guTexMemQueryAvail` through the dynamically resolved function pointer at `01304b20`. psVoodoo 0.18 leaves that export as a stub returning zero. Descent derives its cache size from this value; its zero-byte allocator returns the `0xDEADBEEF` sentinel, which rendering later dereferences. The released Descent source's `mem/mem.cpp` also documents that sentinel for zero-byte allocations.

The [build script](Tools/PsVoodoo/build.py) now implements the query using `TEXMEMSIZE - m_freemem`, where the upstream member name actually denotes the next allocation offset. The result is clamped to zero when exhausted or when an unsupported TMU is queried. The export dispatches to this method when a context exists. No allocator, rendering, gamma-shader, or emulator changes were needed for this fix.

The patched D3D9-only build keeps the original gamma shader and is 896,512 bytes, SHA-256 `22a05a7fc5b94aa62aff52ac9a4a61afc7249b543ed08b511731988767579f08`. The prior DLL in the shared Wine ZIP is `4f4d1cf329a5d7a223d9a7ef299ecfc0503f6751d82f9d796a383bc030fa1d26`; comparing their generated sources confirms changes only in `UTMU.cpp`, `UTMU.h`, and `entry.cpp`. The build succeeded and both bare and Microsoft-style query exports were verified.

Per the user's request, testing used the existing app `C35B0B5E-257F-47C1-BB0B-7AC130FFC91E`, launched through the regular native UI and its original launcher. Wine 11.0/filesystem 11, WineD3D OpenGL, and the game's 3dfx selection were retained. The new DLL is a per-app override at `root/home/username/.wine/drive_c/windows/system32/glide2x.dll`; there was no prior override. Removing it restores the shared package's original DLL. The shared ZIP and catalog were not changed.

After pressing Play, the game passed the previous crash site and visibly rendered the pilot selector, main menu, and Chapter 1 briefing. psVoodoo's log confirms Glide texture formats and generated shaders. The native host window title "OpenGL Window" identifies Wine's backend; the game remains configured for Glide. Brief synthetic input did not reliably leave the briefing screen during automation.

The user subsequently tested the same installation and confirmed: "that fixed it, seems like the mouse issue was because it was using relative mouse." This confirms the reported 3dfx crash is resolved in the user playtest. Relative mouse mode is the user's likely explanation for the automated pointer targeting difficulty; no mouse-handling code was changed. Normal shutdown and extended stability were not separately reported. That playtest used a per-app DLL override; the subsequent package refresh below replaces it.

Evidence is under `tmp/native-ui-test/descent-glide/`: `confirmed-regular-crash/`, `texture-query-menu-success/`, `texture-memory-query.patch`, `texture-query-override.json`, and the `psvoodoo-texture-query/` build with exact sources and metadata.

### Published filesystem refresh

The user uploaded the fixed DLL in the existing Wine 11 ZIP and requested a local replacement. The refreshed download is 167,059,509 bytes, SHA-256 `fccc6fc9fe5294eaf9a8b0dca5461c40d577b5e801cfc2cb513c4a15cbfaa715`. Wine and filesystem metadata remain 11.0/11. All ZIP CRCs passed; `glide2x.dll` is the only entry with changed content and matches the tested DLL byte for byte. The fingerprint supplement and Debug bundled ZIP now use this download. The fourteen active apps pinned to the previous Wine 11 build were migrated to the new content identity with a metadata backup. The Wine 9 app was preserved. Descent's redundant DLL override was backed up and removed, so its next launch uses the shared package's copy. All seven catalog downloads passed the native validator, and the rebuilt Debug app passed its eighteen-image bundle audit. Evidence and rollback copies are in `tmp/native-ui-test/wine-refresh-20260911/`.

## Diablo II: Glide 3 renderer deferred

The installed Diablo II Shareware video test reaches its Glide detection stage but does not offer 3dfx. Its own `D2260911.txt` log repeatedly records `Missing glide3x.dll. Assuming no 3dfx boards present!`. Inspection of `D2VidTst.exe` confirms that it loads `glide3x.dll` dynamically. The game's `D2Glide.dll` also imports that DLL, including Glide 3 APIs such as `grGet`, `grGetString`, `grVertexLayout`, `grDrawVertexArray`, and `grDrawVertexArrayContiguous`.

The refreshed Wine 11 ZIP contains only `glide2x.dll` from psVoodoo. This is a missing API implementation, not evidence of a Wine 11 detection regression or a failure in Run Another Program. Adding the existing Glide demo preset alone cannot supply Glide 3. A compatible Glide 3 implementation would be needed to test Diablo II's 3dfx renderer. The user reports that installing nGlide in the old Windows UI exposes the 3dfx option, but has chosen to use Direct3D and defer Glide 3 support. No Diablo settings, runtime binaries, or catalog recipes were changed for this diagnosis or scope decision. Log evidence is saved in `tmp/native-ui-test/diablo-glide3/`.


## F-16 Quick Mission crash, September 12, 2026

Reproduced in the user's existing F-16 Multirole Fighter demo through the regular native UI: Quick Mission (`Q`), Accept (`A`), Accept (`A`). The game detected the preinstalled psVoodoo DLL, but the saved demo recipe had seeded Wine's `renderer=gdi`. The user confirmed GDI had been a testing setting. The baseline log reports `Disabling 3D support`, followed by an unhandled null read at `7BB99F26`. The crash debugger was force-stopped through the native UI after its normal stop did not complete.

Applying the existing Glide preset in this app's Advanced Boxedwine arguments enabled WineD3D OpenGL:

```text
-env
WINEDLLOVERRIDES=d3d9=b
-env
WINE_D3D_CONFIG=renderer=gl
```

The identical Q → A → A sequence then reached the runway with textured aircraft, buildings, terrain, and sky. psVoodoo logged Glide texture formats and generated shaders, establishing that this exercised the 3dfx renderer. The native window title `OpenGL Window` refers to WineD3D's host backend. No DLL, Wine ZIP, emulator, game executable, or game-renderer setting was changed. The app's old registry values are overridden by the explicit runtime environment; all options remain visible and editable in Advanced.

Escape → Y → 2 returned from the mission statistics to the briefing without a guest fault. The runtime subsequently exited with code 0. This is a mission entry/exit smoke test, not an extended flight test. WineD3D still reports texture-location warnings, and psVoodoo reports unsupported optional calls; this does not establish complete rendering fidelity.

The bundled F-16 recipe now selects `Glide=psVoodoo`, `GDIRenderer=false`, and native OpenGL, using the same filesystem-11 prerequisite and Advanced presets as Motorhead and Descent 3. This applies to future imports; existing apps retain their saved settings. The regular F-16 installation was updated through App Settings, without reinstalling. Evidence and the before-settings snapshot are in `tmp/native-ui-test/f16-glide/`.

The 19 existing demo tests passed, and the rebuilt native Debug app passed its five-image bundle audit with the corrected F-16 recipe.


## F-16 cyan text and framebuffer writes, September 12, 2026

The regular F-16 installation displayed black text shadows without their cyan lettering in the mission view and statistics. psVoodoo's 32-bit framebuffer access emulated a write-only RGB565 buffer by filling it with `0x07ff`, then discarding that cyan color when copying it back. F-16 uses the same cyan for its text. A temporary magenta marker restored the cyan letters, establishing the collision, but reserved another legitimate color and exposed the marker in a narrow undrawn strip at the top of the scene.

`Tools/PsVoodoo/lfb-colors.patch`, applied by the build script, replaces the color marker with a target snapshot and a separate alpha mask. CPU conversion supplies the game's RGB565 buffer. After the game writes, only changed pixels are uploaded as opaque pixels; unchanged pixels have zero alpha. The copy explicitly sets alpha test, blending, and RGB write state and restores the prior state afterward. Untouched 3D pixels retain their original color precision and resolution. Readback surfaces and upload textures are cached, the upload has one mip level, and failed initial access returns a failed Glide lock. The initial whole-frame D3DX conversion experiment was discarded because it quantized the entire 3D scene and was substantially slower.

The clean D3D9-only DLL retains the original gamma shader and the Descent 3 texture-memory query fix. It is 899,072 bytes, SHA-256 `fde1d52062206f5d4274cb1c86ab8f8f2737cc466f68209ec65c6e20034b0576`. Only `BufAccess32Write.cpp`, `BufAccess32Write.h`, and `LFBColor.cpp` differ from the preceding portable source build. No emulator, Wine, demo recipe, or app UI changes were required for this rendering fix.

The owned `Tools/GlideProbe/lfb-probe.c` regression exercises all 65,536 RGB565 colors, partial cyan/magenta/white writes, inherited guest alpha-test and blending state, untouched background, and the four-pixel frame border. The original DLL fails; the new DLL passes with zero mismatches. A 120-update microbenchmark took roughly 4–5 ms per update with the optimized copy, compared with about 26 ms for the discarded full-copy/D3DX prototype on this Apple M4 configuration. These are copy timings, not game frame rates.

The existing F-16 app was launched through the regular native UI with Q → A → A. The runway rendered, the Abort Mission prompt and Mission Statistics lettering were readable in cyan, and the magenta marker was gone. Initial source and presentation captures located a remaining two-row dark edge in the 3D framebuffer. A presentation alignment experiment did not fix it and was discarded. The initial explanation attributed that edge to game geometry; the subsequent nGlide comparison below identified a wrapper clipping-order bug instead.

The text-fix DLL was installed only as the existing F-16 app's override (`62F2092C-54C7-4724-B862-DAAABD30D5AE/root/home/username/.wine/drive_c/windows/system32/glide2x.dll`). No override existed before this work; removing it restores the shared Wine package's DLL. The shared Wine ZIP and catalog fingerprint remain unchanged. All temporary pixel captures were copied into the workspace and removed from the game's directory, and no tracing remains in the clean DLL. Exact patched source, compiler commands, logs, diagnostic comparisons, and `verification.json` are under `tmp/native-ui-test/f16-text/`. Extended flying and other games with this new DLL have not been playtested. The existing limitations around alternate LFB formats, the optional LFB pixel pipeline, and stereo remain.

### F-16 comparison with nGlide: clipping and terrain (2026-09-12)

The user installed nGlide 2.10 in the existing F-16 app (`62F2092C-54C7-4724-B862-DAAABD30D5AE`). Its system32 `glide2x.dll` is 1,630,208 bytes, SHA-256 `7cbd095872e821b54cd6fa03f76aa22073271567175069c53ebb2e73b0299aab`. The regular native UI was used for both wrappers, with the same installation, Wine 11/filesystem 11, WineD3D OpenGL arguments, Quick Mission, and Q → A → A sequence. nGlide's three Glide DLLs and the app's settings were backed up before testing. The psVoodoo comparison uses an app-local `glide2x.dll`, preserving the user's nGlide installation in system32.

Two visible differences were identified and addressed:

- **Top edge:** `grClipWindow` changed D3D scissor state while earlier triangles remained queued. Those triangles could be rendered using the next draw's clipping rectangle. Flushing before changing the clip fixes the ordering; no game-specific coordinates or sky geometry changes are needed. The owned regression loses exactly 640 × 2 pixels with the old wrapper and none with either nGlide or the patched wrapper. The runway view's colored/dark top bar disappeared in the patched run.
- **Ground detail:** F-16 uses texture alpha `SCALE_MINUS_LOCAL_ADD_LOCAL`, factor `ONE_MINUS_DETAIL_FACTOR`, with inversion. psVoodoo previously treated it as unsupported and discarded the corresponding triangles. The game sets detail bias 15, scale 4, maximum 0.8. The patch implements this alpha mode and its complementary factor/inversion combinations, including changes to the detail parameters and texture-coordinate scale. The ground texture beside the runway is now visible. The original shader path remains in use for ordinary combine modes.

The detail factor follows the [3dfx Glide reference](https://www.gamers.org/dEngine/xf3D/glide/glideref.htm): derive LOD from texture gradients and apply the configured bias, scale, and maximum before texture-alpha combination. The gradient implementation uses [Direct3D shader model 2.x](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx9-graphics-reference-asm-ps-2-x). This is floating-point emulation, not bit-exact reproduction of Voodoo's internal fixed-point calculation. In the focused nGlide control, the opaque texture produced an approximately one-half blend across the tested LOD/control values, so the patched result is deliberately validated against the documented equation rather than forced to match those reference pixels.

The aircraft, runway, cockpit, HUD, and cyan Abort Mission text were visually checked. No further obvious difference was identified in those views beyond shading/blending differences; an extended flight comparison and complete renderer fidelity are not established. General multi-TMU combining, all mipmap controls, and the previously documented LFB limitations remain outside this patch.

The final D3D9-only DLL keeps the original gamma shader: **904,192 bytes**, SHA-256 **`10661ca8bfb1fc288bcf07024aae35ba88f27d939d63df0f3dedcd9e394e7ee3`**. A clean build through `Tools/PsVoodoo/build.py` passed `CLIP_ORDER_PASS mismatches=0`, `DETAIL_BLEND_PASS mismatches=0` (48 panels), and `LFB_COLORS_PASS mismatches=0` (all 65,536 RGB565 colors plus partial writes). The terrain patch also fixes the constant-color upload's register count and avoids dereferencing null shader-assembly results.

Build sources, commands, DLLs, comparison logs, and verification records are in `tmp/native-ui-test/f16-nglide-compare/`; the framebuffer regression log remains in `tmp/native-ui-test/f16-text/compare-final-lfb.log`. The final DLL is installed only in the existing game's `C:\Program Files\NovaLogic\F-16 Demo\glide2x.dll`. Removing that app-local DLL switches this installation back to the user's nGlide. No shared Wine ZIP, catalog checksum, game executable, or Boxedwine emulator code was changed for this comparison.


### Source maintenance moved to GitHub (2026-09-12)

The full upstream main-branch history (232 commits) is preserved in [danoon2/psVoodoo](https://github.com/danoon2/psVoodoo). The Boxedwine fork adds separate commits for LLVM-MinGW support, the utility texture-memory query, framebuffer colors, clip ordering, detail-alpha blending, and project/test documentation. The SourceForge repository remains configured as `upstream` in the local checkout.

The initial fork migration pinned `Tools/PsVoodoo/build.py` to commit `aaa990e330554008980b9248277fcfd8f014bddc` and delegates to that checkout's build tool. The earlier patch build is retained as `build-upstream-snapshot.py` for historical reproducibility. Future renderer changes belong in the fork.

The fork build has identical executable/data/resource sections to the F-16-tested DLL; only its generated `.buildid` section differs. This migration leaves the already supplied DLL, shared filesystem ZIP, and catalog unchanged. Migration build and verification records are under `tmp/psvoodoo-fork-migration/`.


### F-16 shadow during camera movement (2026-09-12)

The current installed F-16 demo (`441E554D-244B-4C8E-959A-D9D2A205E41D`)
was compared through the regular native UI while preserving its user-installed
nGlide in system32. The shadow intermittently disappeared during the camera turn.
A psVoodoo triangle/depth trace identified W buffering with LEQUAL, runway bias
-128, and shadow bias -1024. CPU evaluation found 4,815 failed shadow samples in
one captured frame, while neighboring frames did not have that runway failure.

`/Users/james/psVoodoo/Depth.cpp` now scales the reciprocal-W depth offset with
depth instead of adding a constant. The relative scale is calibrated from nGlide
2.10; the fork's test notes document its approximation and limits. The linear-Z
path is unchanged. Biased in-range vertices remain within the near depth plane.
The fork changes and new `tests/depth-probe.c` are intentionally uncommitted;
no push or Boxedwine fork-revision pin update was performed.

The synthetic 72-panel depth regression checks 648 samples across distances,
slopes, bias signs, and bias transitions. The previous DLL fails 192 samples;
nGlide and the corrected DLL pass all samples. Clipping, texture detail, and
framebuffer-color regressions also pass. Re-evaluating the captured failing
frame's runway region with the corrected depth calculation found no failed
shadow samples.

The fixed DLL is 904,192 bytes, SHA-256
`55aee80ac08478f8d965cede9e07b011fa046ec02f84ebc18e78b1abaf44a939`,
from `tmp/native-ui-test/f16-shadow/candidate/glide2x.dll`. Its source snapshot,
commands, hashes, calibration logs, fork patch, and results are recorded in that
work directory. It is installed only beside F16.exe in the existing demo;
removing this app-local DLL restores the preserved nGlide. The shared filesystem
ZIP and checksum have not changed. The clean DLL contains no shadow tracing or
capture instrumentation. Q/A/A reached the runway with the shadow visible;
the user confirmed the corrected shadow looks good after testing the camera turn.


### Packaged shadow fix refresh (2026-09-12)

The user uploaded the tested shadow-fix DLL and requested a package refresh.
The downloaded ZIP passed all CRCs and the native package/catalog validators;
its Wine version is 11.0 and filesystem version is 11. Only glide2x.dll changed,
and it matches the user-tested build byte for byte. The current package identity
is recorded at the top of this document and in Resources/WindowsSupport/packages.json.

The Debug app was rebuilt with this ZIP and passed its five-image bundle audit.
The bundled and managed shared ZIPs match the catalog hash. With a backup of
library.json, sixteen active Wine 11 apps were moved to this package; the Wine 9
app, removed apps, settings, and local DLL overrides were preserved. In particular,
F-16 retains its tested app-local psVoodoo DLL and the user's system32 nGlide.
The previous shared ZIP remains available. Boxedwine's fork build helper now pins
psVoodoo commit e83947c2f4d800831db0643ad589fba1c98b2457, which is pushed to GitHub.
Verification, migration records, and build logs are in
tmp/native-ui-test/wine-refresh-shadow-20260912/.

The refreshed Debug UI launched Motorhead using the new shared ZIP with no local
Glide override. Textured racing and the HUD rendered, then Stop App closed it.
The native UI remains open for user testing.
