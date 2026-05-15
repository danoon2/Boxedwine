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
buffer writes, client pointer getters, texture object array APIs,
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

Run the EGL ES context test directly as a Linux guest program:

```powershell
project\msvc\BoxedWine\Release\BoxedWine.exe `
  -root "C:\Boxedwine\tools\openglTest\Win32\Release" `
  -zip "C:\Users\james\AppData\Roaming\Boxedwine\FileSystems2\TinyCore15Wine11.0.zip" `
  /EGLRealESContextTest
```
