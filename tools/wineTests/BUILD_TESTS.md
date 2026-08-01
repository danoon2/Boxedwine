# Building Wine 11 test executables

This guide describes the reusable process for building Wine 11 test suites as
32-bit Windows PE executables on an x86-64 Ubuntu or Debian host.

BoxedWine currently supports Wine's NTDLL, kernel32, and advapi32 test suites,
the ws2_32 AFD group, and DirectDraw, D3D8, D3D9, D3DX9, and D3DXOF browser
groups.
The public native-test `wine_tests_v4.zip` archive contains `ntdll_test.exe`,
`kernel32_test.exe`, `ws2_32_test.exe`, and `advapi32_test.exe`. The prepared
`wine_tests_v6.zip` extends that flat bundle with the patched Wine 11
`d3d8_test.exe`, `d3d9_test.exe`, `d3dx9_43_test.exe`, `d3dxof_test.exe`,
and `ddraw_test.exe` used by the Emscripten graphics runner.

Use a native Linux filesystem for the Wine source and build directories. A
WSL path below `/home` or `/tmp` is substantially faster than building below
`/mnt/c`.

## Install prerequisites

On Ubuntu or Debian:

```bash
sudo apt update
sudo apt install build-essential git flex bison gcc-multilib g++-multilib \
    gcc-mingw-w64-i686 file
```

The important pieces are a compiler that supports `-m32` and the
`i686-w64-mingw32` cross compiler. Confirm both before configuring Wine:

```bash
printf 'int main(void) { return 0; }\n' | gcc -m32 -x c - -o /tmp/wine-i386-check
i686-w64-mingw32-gcc --version
file /tmp/wine-i386-check
```

The last command should identify `/tmp/wine-i386-check` as a 32-bit ELF
executable.

## Fetch the Wine 11 source

Choose an empty working directory and clone the official Wine repository at
the `wine-11.0` tag:

```bash
WORK_DIR="$PWD/wine11-tests"
git clone --branch wine-11.0 --depth 1 \
    https://gitlab.winehq.org/wine/wine.git "$WORK_DIR/source"
```

The tag used for the current BoxedWine test archive resolves to this commit:

```text
db11d0fe6a169c457e23d007e20404643d067aa8
```

Verify the checkout:

```bash
git -C "$WORK_DIR/source" describe --tags --always
git -C "$WORK_DIR/source" rev-parse HEAD
```

The first command should print `wine-11.0`. The second should print the commit
above.

## Configure the 32-bit build

Keep the build output separate from the source tree:

```bash
mkdir "$WORK_DIR/build-i386"
cd "$WORK_DIR/build-i386"
../source/configure --enable-archs=i386 --without-x --without-freetype
```

`--enable-archs=i386` makes the requested Windows output architecture
explicit. Disabling X and FreeType keeps this test-only build from requiring
the optional graphics and font development packages. Configure warnings about
other optional libraries are expected when only building this test.

To confirm the saved configuration later:

```bash
./config.status --config
```

It should include `--enable-archs=i386`.

## Build the test executables

### NTDLL

From the build directory:

```bash
make -j"$(nproc)" dlls/ntdll/tests/i386-windows/ntdll_test.exe
```

This target builds the required Wine tools and import libraries, then links
all 26 NTDLL test groups into one executable. A complete Wine build is not
required.

The output is:

```text
$WORK_DIR/build-i386/dlls/ntdll/tests/i386-windows/ntdll_test.exe
```

### kernel32

From the same build directory:

```bash
make -j"$(nproc)" dlls/kernel32/tests/i386-windows/kernel32_test.exe
```

This links all 33 kernel32 test groups into:

```text
$WORK_DIR/build-i386/dlls/kernel32/tests/i386-windows/kernel32_test.exe
```

### ws2_32

From the same build directory:

```bash
make -j"$(nproc)" dlls/ws2_32/tests/i386-windows/ws2_32_test.exe
```

This links the Wine socket tests into:

```text
$WORK_DIR/build-i386/dlls/ws2_32/tests/i386-windows/ws2_32_test.exe
```

The BoxedWine runner currently selects only the `afd` group from this
executable.

### advapi32

From the same build directory:

```bash
make -j"$(nproc)" dlls/advapi32/tests/i386-windows/advapi32_test.exe
```

This links all 12 supported advapi32 groups into:

```text
$WORK_DIR/build-i386/dlls/advapi32/tests/i386-windows/advapi32_test.exe
```

### Additional Wine test suites

Wine DLL test targets generally follow this layout:

```text
dlls/<module>/tests/i386-windows/<module>_test.exe
```

For example, another module would be built with its complete
architecture-specific target:

```bash
make -j"$(nproc)" dlls/<module>/tests/i386-windows/<module>_test.exe
```

Confirm the exact target in the generated `Makefile`; not every Wine test
suite is required to use the same name. Building another executable does not
automatically add it to BoxedWine's test runner.

## Verify the output architecture

Do not use an x86-64 or PE32+ test binary with BoxedWine. Verify all supported
artifacts:

```bash
NTDLL_TEST_EXE="$WORK_DIR/build-i386/dlls/ntdll/tests/i386-windows/ntdll_test.exe"
KERNEL32_TEST_EXE="$WORK_DIR/build-i386/dlls/kernel32/tests/i386-windows/kernel32_test.exe"
WS2_32_TEST_EXE="$WORK_DIR/build-i386/dlls/ws2_32/tests/i386-windows/ws2_32_test.exe"
ADVAPI32_TEST_EXE="$WORK_DIR/build-i386/dlls/advapi32/tests/i386-windows/advapi32_test.exe"
file "$NTDLL_TEST_EXE" "$KERNEL32_TEST_EXE" "$WS2_32_TEST_EXE" "$ADVAPI32_TEST_EXE"
i686-w64-mingw32-objdump -f "$NTDLL_TEST_EXE" | sed -n '1,6p'
i686-w64-mingw32-objdump -f "$KERNEL32_TEST_EXE" | sed -n '1,6p'
i686-w64-mingw32-objdump -f "$WS2_32_TEST_EXE" | sed -n '1,6p'
i686-w64-mingw32-objdump -f "$ADVAPI32_TEST_EXE" | sed -n '1,6p'
```

`file` must report output equivalent to:

```text
PE32 executable (console) Intel 80386, for MS Windows
```

The file size and SHA-256 hash can vary between builds because PE timestamps
and build paths may be embedded in the executable. Validate the Wine tag and
PE32/i386 architecture instead of expecting the hash of a previous build.

## Copy into a BoxedWine checkout

Set `BOXEDWINE_DIR` to the Linux or WSL path of the BoxedWine checkout:

```bash
BOXEDWINE_DIR=/mnt/c/Boxedwine2
cp "$NTDLL_TEST_EXE" "$BOXEDWINE_DIR/tools/wineTests/ntdll_test.exe"
cp "$KERNEL32_TEST_EXE" "$BOXEDWINE_DIR/tools/wineTests/kernel32_test.exe"
cp "$WS2_32_TEST_EXE" "$BOXEDWINE_DIR/tools/wineTests/ws2_32_test.exe"
cp "$ADVAPI32_TEST_EXE" "$BOXEDWINE_DIR/tools/wineTests/advapi32_test.exe"
```

The build/configuration WebGL patch adds `build-boxedwine-webgl-dlls.sh` to its
Wine source tree. Apply the build/configuration, adapter/context/capability,
shader/GLSL ES, texture-transfer, GLSL blitter/batching, DirectDraw
runtime/presentation, D3DX9 assets/compatibility, D3DXOF parser hardening,
WineD3D draw/state/query, D3D8/D3D9 compatibility/diagnostics, and test-only
patches in order:

```bash
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-build-config-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-adapter-context-caps-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-shader-generation-glsl-es-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-texture-formats-transfers-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-blitter-batching-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-directdraw-runtime-presentation-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3dx9-assets-compatibility-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3dxof-parser-hardening-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-wined3d-draw-state-query-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch
git apply /path/to/Boxedwine/tools/d3dToWebGL/webgl-tests-against-wine-11.0.patch
```

Then run the generated script. It stages the graphics test executables in
`boxedwine-webgl-wine-build/boxedwine-webgl-tests/`. The v6 bundle uses that
directory's `d3d8_test.exe`, `d3d9_test.exe`, `d3dx9_43_test.exe`,
`d3dxof_test.exe`, and `ddraw_test.exe`.

When rebuilding `wine_tests_v6.zip`, also copy `COPYING.LIB` from the same
Wine source checkout and regenerate `SHA256SUMS` for all nine executables and
the license. The archive layout and runner verification commands are
documented in `README.md`.

## Build clean Wine 11 graphics comparison tests

Keep a second, unmodified source/build pair at the same official Wine 11 commit
to distinguish Wine's original test behavior from WebGL-adapted expectations.
With the `WORK_DIR`, checkout, and `build-i386` configuration described above:

```bash
make -C "$WORK_DIR/build-i386" -j"$(nproc)" \
    dlls/ddraw/tests/i386-windows/ddraw_test.exe \
    dlls/d3d8/tests/i386-windows/d3d8_test.exe \
    dlls/d3d9/tests/i386-windows/d3d9_test.exe \
    dlls/d3dx9_43/tests/i386-windows/d3dx9_43_test.exe \
    dlls/d3dxof/tests/i386-windows/d3dxof_test.exe
```

Verify that all five outputs are PE32/i386, then select one without changing
the packaged v6 bundle:

```bash
python3 tools/wineTests/runWineTests.py \
    --d3d9-group stateblock \
    --native-wine-root /path/to/native-pure-i386-wine-build \
    --graphics-test-executable \
      "$WORK_DIR/build-i386/dlls/d3d9/tests/i386-windows/d3d9_test.exe" \
    --no-native-graphics-baseline
```

The current clean official build hashes and the stable comparison artifacts
are recorded in `webgl-test-divergences-v2.json`. Use
`--no-native-graphics-baseline` for this comparison because the native exact
baseline deliberately pins the packaged v6 executables.

For an additional test suite, update `runWineTests.py`, its tests, the published
archive layout, and the expected-result rules before treating the new
executable as supported.

## Rebuild after changing Wine test sources

After editing a test below `dlls/<module>/tests`, rerun that suite's complete
Make target. For the supported suites:

```bash
cd "$WORK_DIR/build-i386"
make -j"$(nproc)" dlls/ntdll/tests/i386-windows/ntdll_test.exe
make -j"$(nproc)" dlls/kernel32/tests/i386-windows/kernel32_test.exe
make -j"$(nproc)" dlls/ws2_32/tests/i386-windows/ws2_32_test.exe
make -j"$(nproc)" dlls/advapi32/tests/i386-windows/advapi32_test.exe
```

The patched WebGL Wine build uses non-architecture-prefixed graphics targets.
Either run the staging script above or rebuild an individual graphics test with:

```bash
make -C /home/james/webgl/boxedwine-webgl-wine-build/wine-win32 \
    -j"$(nproc)" dlls/d3d8/tests/d3d8_test.exe

make -C /home/james/webgl/boxedwine-webgl-wine-build/wine-win32 \
    -j"$(nproc)" dlls/d3d9/tests/d3d9_test.exe

make -C /home/james/webgl/boxedwine-webgl-wine-build/wine-win32 \
    -j"$(nproc)" dlls/d3dx9_43/tests/d3dx9_43_test.exe

make -C /home/james/webgl/boxedwine-webgl-wine-build/wine-win32 \
    -j"$(nproc)" dlls/d3dxof/tests/d3dxof_test.exe
```

Use a new source and build directory when changing Wine versions. This avoids
mixing generated files or import libraries from different Wine releases.

## Common failures

- `Cannot build a 32-bit program`: install `gcc-multilib`, `g++-multilib`,
  and their 32-bit libc development dependencies.
- `i686-w64-mingw32-gcc: command not found`: install
  `gcc-mingw-w64-i686`.
- `No rule to make target dlls/ntdll/tests/ntdll_test.exe`: use the complete
  architecture-specific target ending in
  `i386-windows/ntdll_test.exe`.
- `No rule to make target dlls/kernel32/tests/kernel32_test.exe`: use the
  complete architecture-specific target ending in
  `i386-windows/kernel32_test.exe`.
- `No rule to make target dlls/ws2_32/tests/ws2_32_test.exe`: use the complete
  architecture-specific target ending in `i386-windows/ws2_32_test.exe`.
- `No rule to make target dlls/advapi32/tests/advapi32_test.exe`: use the
  complete architecture-specific target ending in
  `i386-windows/advapi32_test.exe`.
- `PE32+` or `x86-64` output: remove the build directory and configure again
  with `--enable-archs=i386`.
