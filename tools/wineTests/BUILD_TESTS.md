# Building Wine 11 test executables

This guide describes the reusable process for building Wine 11 test suites as
32-bit Windows PE executables on an x86-64 Ubuntu or Debian host.

BoxedWine currently supports only Wine's NTDLL test suite. The current
`runWineTests.py` runner and `wine_tests_v1.zip` archive therefore expect
`ntdll_test.exe`. The same Wine configuration, architecture verification, and
targeted Make workflow can build other Wine test suites as BoxedWine support
is added. Each additional suite will also need corresponding runner,
packaging, and expected-result configuration.

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

## Build a test executable

### Currently supported: NTDLL

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

### Future Wine test suites

Wine DLL test targets generally follow this layout:

```text
dlls/<module>/tests/i386-windows/<module>_test.exe
```

For example, a future supported module would be built with its complete
architecture-specific target:

```bash
make -j"$(nproc)" dlls/<module>/tests/i386-windows/<module>_test.exe
```

Confirm the exact target in the generated `Makefile`; not every Wine test
suite is required to use the same name. Building another executable does not
automatically add it to BoxedWine's test runner.

## Verify the output architecture

Do not use an x86-64 or PE32+ test binary with BoxedWine. The following example
verifies the currently supported NTDLL artifact:

```bash
TEST_EXE="$WORK_DIR/build-i386/dlls/ntdll/tests/i386-windows/ntdll_test.exe"
file "$TEST_EXE"
i686-w64-mingw32-objdump -f "$TEST_EXE" | sed -n '1,6p'
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
cp "$TEST_EXE" "$BOXEDWINE_DIR/tools/wineTests/ntdll_test.exe"
```

When rebuilding `wine_tests_v1.zip`, also copy `COPYING.LIB` from the same
Wine source checkout and regenerate `SHA256SUMS`. The archive layout and
runner verification commands are documented in `README.md`.

For a future test suite, update `runWineTests.py`, its tests, the published
archive layout, and the expected-result rules before treating the new
executable as supported.

## Rebuild after changing Wine test sources

After editing a test below `dlls/<module>/tests`, rerun that suite's complete
Make target. For the currently supported NTDLL suite:

```bash
cd "$WORK_DIR/build-i386"
make -j"$(nproc)" dlls/ntdll/tests/i386-windows/ntdll_test.exe
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
- `PE32+` or `x86-64` output: remove the build directory and configure again
  with `--enable-archs=i386`.
