# Wine 11 NTDLL tests

`runWineTests.py` builds the 64-bit Linux BoxedWine release and runs the
32-bit Wine 11 `ntdll_test.exe` groups. It exits successfully only when every
group completes within its failure ceiling.

NTDLL is currently the only Wine test suite supported by this runner and its
published test archive. The build process in [BUILD_TESTS.md](BUILD_TESTS.md)
also applies to other Wine test executables, but each new suite must be added
explicitly to the runner, archive, and expected-result rules.

## Linux prerequisites

On Ubuntu or Debian, install the BoxedWine build dependencies and Python 3:

```bash
sudo apt update
sudo apt install build-essential libsdl2-dev libssl-dev libminizip-dev \
    libcurl4-openssl-dev python3 unzip
```

The runner supports Linux on an `x86_64` host. Run it from any directory in a
BoxedWine checkout:

```bash
python3 tools/wineTests/runWineTests.py
```

The normal command downloads its inputs when they are absent, runs
`make release` in `project/linux`, and executes all 26 groups sequentially.

## Inputs and cache

The versioned inputs are:

- BoxedWine Wine 11 filesystem:
  `https://boxedwine.org/v2/8/TinyCore15Wine11.0.zip`
- Wine 11 tests:
  `http://boxedwine.org/v2/1/wine_tests_v1.zip`

They are cached in
`${XDG_CACHE_HOME:-$HOME/.cache}/boxedwine/wineTests`. A nonempty cached file is
reused. Downloads use a `.part` file so an interrupted transfer does not
replace a cached artifact.

Each invocation creates a timestamped directory below `runs/`. It contains:

- `logs/<group>.log`: combined BoxedWine stdout and stderr.
- `manifest.json`: parsed counts, ceilings, status, and failure reason.
- `roots/<group>/`: retained guest state for a failed or timed-out group.
- `input/ntdll_test.exe`: the PE32/i386 test extracted for that run.

Guest roots for successful groups are removed automatically.

## Options

Run one or more groups by repeating `--group`:

```bash
python3 tools/wineTests/runWineTests.py --group file --group virtual
```

Use an existing Linux BoxedWine executable and skip the build:

```bash
python3 tools/wineTests/runWineTests.py \
    --boxedwine project/linux/Build/Release/boxedwine
```

Other useful overrides are:

```text
--cache-dir PATH       Download and run cache directory
--filesystem-url URL   BoxedWine Wine 11 filesystem ZIP
--tests-url URL        Wine 11 test ZIP
--timeout SECONDS      Per-group timeout (default: 180)
```

Use `python3 tools/wineTests/runWineTests.py --help` for the complete option
list. A local ZIP can be supplied with a `file:///...` URL.

## Expected failures

The thresholds are ceilings: fewer failures still pass, but one more fails the
run.

| Group | Maximum failures |
| --- | ---: |
| `file` | 9 |
| `threadpool` | 1 only for the timer-merging TODO success described below |
| `virtual` | 7 |
| `wow64` | 3 |
| Every other group | 0 |

The `threadpool` group also passes with zero failures. Its one-failure result is
accepted only when Wine reports the timing-dependent unexpected TODO success at
`threadpool.c:1622` (`expected that timers are merged`) and reports no ordinary
`Test failed:` records. An unrelated single `threadpool` failure still fails the
run.

A timeout, malformed archive, missing summary, missing `Boxedwine shutdown`,
failed guest-root cleanup, or failed release build also fails the run. The
`wow64` command runs `/opt/wine/bin/wineserver -k` in the same guest shell and
requires an explicit cleanup-complete marker before it passes.

## Test archive

See [BUILD_TESTS.md](BUILD_TESTS.md) for the reusable Wine 11 test build
process and the verified commands that produce the currently supported
PE32/i386 `ntdll_test.exe`.

`wine_tests_v1.zip` is a flat archive containing:

- `ntdll_test.exe`: Wine 11 PE32/i386 test executable.
- `COPYING.LIB`: Wine's LGPL license.
- `SHA256SUMS`: hashes for those two files.

The prepared upload artifact is `tools/wineTests/wine_tests_v1.zip`. Its
SHA-256 is:

```text
c01d79500d08b72a59e0e505cfae548974859b87ab2ecc608f4d118364e7f3ce
```

Upload that file without repacking it to:

```text
http://boxedwine.org/v2/1/wine_tests_v1.zip
```

After uploading, verify the public file before relying on the default runner:

```bash
curl -fL http://boxedwine.org/v2/1/wine_tests_v1.zip -o /tmp/wine_tests_v1.zip
sha256sum /tmp/wine_tests_v1.zip
unzip -t /tmp/wine_tests_v1.zip
```

## Development checks

The unit tests use temporary files and fake subprocesses; they do not download
artifacts or build BoxedWine:

```bash
python3 -m unittest discover -s tools/wineTests/tests -v
python3 -m py_compile tools/wineTests/runWineTests.py
unzip -t tools/wineTests/wine_tests_v1.zip
```

If a real group fails, inspect its log and retained root under the run directory
printed at the end. Increase `--timeout` for a slow machine. Delete an invalid
or partial cached ZIP to make the runner download it again.
