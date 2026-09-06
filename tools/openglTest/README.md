# OpenGL Marshal Test

Small Win32/WGL OpenGL test harness for exercising API shapes that are easy to
mis-marshal in BoxedWine.

Coverage currently includes shader string arrays, uniform getters, uniform
matrix shapes, 1D/2D/3D/array texture upload/readback, compressed texture
readback, DSA/EXT texture readback, texture subimage upload/readback/copy paths,
texture parameter getters, texture parameter vector inputs, fixed-function
texenv/texgen/light/material getters, fixed-function texenv/texgen vector
inputs, fixed-function light/material vector inputs, pixel/evaluator map getters,
pixel map vector inputs,
evaluator map vector inputs, evaluator coordinate vector inputs, framebuffer texture readback,
pixel pack/unpack state, selection/feedback buffer copy-back, clip-plane and
polygon-stipple copy-back, fog vector inputs, fog and pixel-transfer state
getters, pixel buffer object pack/unpack paths, buffer subdata readback, mapped
buffer writes, buffer allocation/orphaning/growth, client pointer getters,
texture object array APIs,
fixed-function current color/index/edge vector inputs, fixed-function matrix
vector inputs, fixed-function texcoord/raster-position vector inputs,
immediate-mode vertex vector inputs, rect vector inputs, interleaved client
arrays, `glArrayElement`, `glReadPixels`,
`glReadnPixels`, page-boundary draw-pixels/bitmap paths, page-boundary client
arrays, draw-range/base-vertex-elements, multi-draw-arrays, and
multi-draw-elements pointer arrays. A small Linux guest EGL test also verifies
that BoxedWine can create and bind a real OpenGL ES pbuffer context through the
guest `libEGL.so.1` stub, then run GL clear/readback and shader-source compile
calls on that ES context. It also links an ES2 shader program, uploads vertex
data through a VBO, draws into the pbuffer, uploads/samples a texture, and
verifies the rendered pixels.

## Build

From the repository root:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe' `
  'tools\openglTest\OpenGLMarshalTest.sln' `
  /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=v143
```

You can also open `OpenGLMarshalTest.sln` directly in Visual Studio.

The Linux guest EGL test can be built from WSL or another shell with 32-bit gcc
support:

```bash
cd tools/openglTest
bash build_egl_real_es_context_test.sh
```

This rebuilds and stages `libEGL.so.1`, `libGLESv2.so.2`, and `libGL.so.1`
beside the test. It needs 32-bit gcc plus EGL/GLES2 and desktop GL headers.
The test calls the actual libraries, including `eglGetProcAddress`, and repeats
EGL/GL callbacks past the JIT warmup threshold.

Check the guest calling convention natively in WSL, without a graphics driver:

```bash
bash tools/openglTest/run_egl_calling_convention_test.sh
```

This intercepts `int99` in the built 32-bit libraries and checks callback
indices, argument slots, return values, extension wrappers, 64-bit timeouts,
float arguments, and EGL/GLX proc-address lookup. Expected result:
`PASS EGL/GLES calling convention (20 callbacks)`.

## Run

```powershell
tools\openglTest\x64\Debug\OpenGLMarshalTest.exe
```

Useful options:

```text
--list              List tests
--test <name>       Run one test
--log <path>        Write a log file
--quiet             Do not write test output to stdout
```

Tests report `PASS`, `FAIL`, or `SKIP`. Extension-dependent paths skip cleanly
when the host OpenGL driver does not expose the required functions.

When running inside Wine/BoxedWine, use `--quiet --log opengl-test.log` if the
console path emits cursor-control escape sequences.

## Emscripten browser regressions

`wgl-context-lifecycle` creates a secondary WGL context, switches between it
and the primary context, reports whether GL state is isolated or shared,
deletes the secondary context, and verifies the primary context remains
usable. Emscripten currently uses one underlying WebGL context for multiple
Wine WGL handles, so browser runs report the shared state model.

`wgl-context-thread-switch` releases the primary WGL context from the main
guest thread, makes it current on a Win32 worker thread, changes GL state,
releases it, and makes it current on the original thread again. This covers
the BoxedWine guest-thread transition. Single-threaded builds must pass.
Pthread builds execute the attempted migration and report a targeted skip
when Emscripten rejects moving a direct OffscreenCanvas context away from its
owning host pthread; other failure stages still fail.

`webgl-context-loss-restore` renders to fresh GL resources, asks the browser
harness to force `WEBGL_lose_context`, requires both browser loss and restore
events, verifies guest clear and texture-creation calls fail safely while the
context remains lost, then creates fresh resources and verifies a pixel after
restoration in single-threaded builds. The browser waits for the guest's lost-
context probe before requesting restoration, and the runner enforces the event
ordering. The lost-state probe permits `glGenTextures()` to reserve a nonzero
name, but requires `glIsTexture()` to remain false and `glGetError()` to report
context loss. Pthread builds report a targeted skip because the
direct OffscreenCanvas context's owning guest pthread does not return to the
browser event loop while it is running, so it cannot receive loss/restoration
events in time. Supporting that case requires a cooperative host-thread design.

`readbuffer-yield-replay` is a browser-only regression for the
single-threaded yield between guest `glReadBuffer()` and `glReadPixels()`.
The guest renders a known FBO color, selects `GL_COLOR_ATTACHMENT0`, prints an
arm marker, and sleeps. The browser harness then changes the underlying WebGL2
read buffer to `NONE`. The resumed guest read must still return the expected
pixel because BoxedWine replays its remembered guest read-buffer selection in
the same host callback as `glReadPixels()`.

`framebuffer-read-draw-switch-orientation` creates two texture-backed FBOs,
binds different objects to `GL_DRAW_FRAMEBUFFER` and `GL_READ_FRAMEBUFFER`,
and switches them in both directions. Asymmetric quadrant clears verify that
draws and reads continue to use the selected object and that lower-left guest
coordinates keep their orientation. Browser runs reject a skipped test.

`buffer-lifecycle-growth` uploads an array buffer, applies a partial update,
orphans it with `bufferData(NULL)`, updates the orphan, and grows it with a new
upload. Native OpenGL verifies the defined payload ranges. Browser runs also
require one exact 256-byte zero-filled safety tail after every allocation and
reject a skipped test. BoxedWine supplies Wine's desktop
`glGetBufferSubData[ARB]` entry points through WebGL 2 `getBufferSubData()` so
the guest can verify the actual bytes. The page wrapper owns padding for ST and
ST-JIT (including both Emscripten typed-array overloads); the C++ marshaller
owns it for worker-hosted MT and MT-JIT contexts.

`element-buffer-client-array-max-index` combines a page-boundary client vertex
array with an element-array buffer. It verifies
nonzero element-buffer offsets, unsigned byte/short/int index widths, a partial
index-buffer update, and sizing client uploads from the maximum referenced
vertex rather than the draw's index count. Browser runs reject a skipped test.

`dynamic-buffer-map-sync` exercises discard-style mapped vertex and element
buffers, a non-overlapping explicit vertex-buffer update, and split explicit
element-buffer flushes. It verifies both rendered output and synchronization of
mapped indices with the client-array maximum-index shadow. Browser runs reject
a skipped test.

`texture-level-update-mipmap-row-pitch` allocates a nonzero mip level, updates
one texel from a padded source row with unpack skips, and verifies through FBO
readback that the update preserves the level size and untouched texels. It then
generates and checks the complete mip chain. Browser runs reject a skipped test.

`compressed-texture-capabilities` compares the advertised S3TC extension with
the compressed-format list, rejects an invalid format, and, when S3TC is
available, uploads, partially updates, and samples known DXT1, DXT3, and DXT5
blocks. When S3TC is unavailable it instead verifies that a DXT1 upload is
rejected. Browser runs reject a skipped test.

Build the Win32 Release target, then run:

```powershell
python tools\openglTest\runBrowserTest.py --headless
```

Select a context regression and Emscripten target with:

```powershell
python tools\openglTest\runBrowserTest.py --headless `
  --test wgl-context-lifecycle --build-mode st
python tools\openglTest\runBrowserTest.py --headless `
  --test wgl-context-thread-switch --build-mode mt
```

`--build-mode` accepts `st`, `mt`, `st-jit`, or `mt-jit`. The runner defaults
to the original `readbuffer-yield-replay` test on the single-threaded non-JIT
build, `boxedwine.3.zip`, and
`tools\openglTest\Win32\Release\OpenGLMarshalTest.exe`. Override these with
`--build-dir`, `--filesystem`, or `--test-executable`. Run artifacts are
written under
`~/.cache/boxedwine/openglTests/runs/<timestamp>` and include the manifest,
OpenGL output, browser payload, Chrome log, HTTP log, and generated app ZIP.
The parser also requires the browser mutation marker, so a missing or broken
mutation cannot produce a false pass.

Run the EGL ES context test directly as a Linux guest program:

```powershell
New-Item -ItemType Directory -Force tmp\egl-validation\root | Out-Null
project\msvc\BoxedWine\x64\Release\BoxedWine.exe `
  -root "$PWD\tmp\egl-validation\root" `
  -zip "$env:APPDATA\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" `
  -mount "$PWD\tools\openglTest\Win32\Release" /egl-test `
  -env LD_LIBRARY_PATH=/egl-test/lib `
  /egl-test/EGLRealESContextTest
```

Expected result: `PASS real ES pbuffer context, VBO draw, and texture sample`.
Set the host environment variable `BOXEDWINE_GL_INT99_SLOW=1` to repeat through
the generic native JIT interrupt handler; remove it afterward.

After building the desired Emscripten target, run the same EGL guest test in
headless Chrome:

```powershell
python tools\openglTest\runEGLBrowserTest.py --build-mode st
```

`--build-mode` accepts `st`, `mt`, `st-jit`, and `mt-jit`. Use `--filesystem`
to select another guest filesystem ZIP. Artifacts go under `tmp/egl-validation`
by default, with a separate browser profile per run. The guest test checks
core GL function lookup before context creation (matching Wine startup),
initial vertex-array state, and that creating a secondary context preserves the
current context and its GL state, in addition to rendering and ABI checks.

The native desktop GL resize regression covers an EGL/X11 window shrinking and
growing before its first presentation, then resizing again while visible:

```powershell
wsl -e bash -lc 'cd /mnt/c/Boxedwine2/tools/openglTest && bash build_egl_window_resize_test.sh'
project\msvc\BoxedWine\x64\Release\BoxedWine.exe `
  -root "$PWD\tmp\egl-validation\root" `
  -zip "$env:APPDATA\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" `
  -mount "$PWD\tools\openglTest\Win32\Release" /egl-test `
  -env LD_LIBRARY_PATH=/egl-test/lib `
  /egl-test/EGLWindowResizeTest
```

It measures rendered sample counts to check the actual native framebuffer size,
checks `eglQuerySurface`, and verifies that resizing preserves the GL viewport.
Expected: `PASS EGL window resize before and after first presentation`.
This test requires desktop GL occlusion queries and is not a WebGL test.

The same build script also builds `EGLSwapIntervalTest`. Run it with the same
native command, replacing `/egl-test/EGLWindowResizeTest` with
`/egl-test/EGLSwapIntervalTest`. It measures actual swap durations, checks the
default interval of one, disabling VSync, clamping to the advertised range,
independent surface state across context switches, errors, and unpaced pbuffers.
Expected: `PASS EGL swap interval pacing, clamping, surface state, and errors`.

Native SDL EGL windows honor the guest's swap interval. Synchronized windows
also default to a 60 FPS compatibility limit, since some old games advance
animation once per frame even on a 240 Hz host display. Interval zero disables
both native VSync and this limit. The host environment variable
`BOXEDWINE_EGL_VSYNC_FPS` overrides the limit (1–1000), or `0` uses only the host's
VSync. The physical monitor mode and the reported XRandR refresh rate are unchanged.
This pacing applies to native EGL windows; browser presentation is unchanged.

Set host `BOXEDWINE_EGL_SWAP_LOG=1` to log measured FPS every two seconds.
For a timing regression on a 60 Hz host, set host `BOXEDWINE_EGL_VSYNC_FPS=20`
and add guest `-env EGL_TEST_VSYNC_FPS=20` to the command above. The lower limit
lets the test distinguish VSync on from off even when the desktop compositor
limits unsynchronized swaps. Run timing tests without other heavy workloads.
