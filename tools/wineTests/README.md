# Wine 11 NTDLL and kernel32 tests

`runWineTests.py` builds the 64-bit Linux BoxedWine release and runs the
32-bit Wine 11 `ntdll_test.exe` and `kernel32_test.exe` groups. It exits
successfully only when every selected group completes within its
suite-specific failure ceiling.

Every generated BoxedWine command uses `-novideo` to prevent host test windows
from appearing during headless execution.

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
`make release` in `project/linux`, and executes all 26 NTDLL groups followed
by all 33 kernel32 groups.

## Inputs and cache

The versioned inputs are:

- BoxedWine Wine 11 filesystem:
  `https://boxedwine.org/v2/8/TinyCore15Wine11.0.zip`
- Wine 11 tests:
  `https://boxedwine.org/v2/1/wine_tests_v2.zip`

They are cached in
`${XDG_CACHE_HOME:-$HOME/.cache}/boxedwine/wineTests`. A nonempty cached file is
reused. Downloads use a `.part` file so an interrupted transfer does not
replace a cached artifact.

Each invocation creates a timestamped directory below `runs/`. It contains:

- `logs/<suite>/<group>.log`: combined BoxedWine stdout and stderr.
- `manifest.json`: suite name, parsed counts, ceilings, status, and failure
  reason.
- `roots/<suite>/<group>/`: retained guest state for a failed or timed-out
  group.
- `input/ntdll_test.exe` and `input/kernel32_test.exe`: the validated
  PE32/i386 tests extracted for that run.

Guest roots for successful groups are removed automatically.

## Options

Run one or more NTDLL groups by repeating `--group`:

```bash
python3 tools/wineTests/runWineTests.py --group file --group virtual
```

Run one or more kernel32 groups by repeating `--kernel32-group`:

```bash
python3 tools/wineTests/runWineTests.py \
    --kernel32-group sync \
    --kernel32-group virtual
```

When either selector is present, only explicitly selected groups run. The two
selectors may be combined. With no selectors, both complete suites run.

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

### NTDLL

| NTDLL group | Maximum failures |
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

Because some Wine groups occasionally stall during teardown, the all-suite
runner retries a timed-out group once with a fresh guest root. A successful
retry is reported as `ok after timeout retry`; both the original root/log and
the retry log remain available. A second timeout still fails the run.

### kernel32

| kernel32 group | Maximum failures |
| --- | ---: |
| `sync` | 1 |
| `loader` | 62 |
| `virtual` | 109 |
| Every other group | 0 |

These three nonzero results are reproduced by the same PE32 test under native
pure-i386 Wine 11. The `console` group is also accepted as skipped when it
exits cleanly after Wine reports exactly that `console.c:5869` cannot open
`HKCU\Console`; other missing summaries remain failures. Kernel32 runs set
`WINEDLLOVERRIDES=mscoree,mshtml=` so Mono and Gecko installer dialogs cannot
block unattended tests.

## Test archive

See [BUILD_TESTS.md](BUILD_TESTS.md) for the reusable Wine 11 test build
process and the verified commands that produce both supported PE32/i386
executables.

`wine_tests_v2.zip` is a flat archive containing:

- `ntdll_test.exe`: Wine 11 PE32/i386 test executable.
- `kernel32_test.exe`: Wine 11 PE32/i386 test executable.
- `COPYING.LIB`: Wine's LGPL license.
- `SHA256SUMS`: hashes for all three files above.

The prepared upload artifact is `tools/wineTests/wine_tests_v2.zip`. Its
SHA-256 is:

```text
925599a65331f0b811ddd16133b7ffa8906699ffaf242997a84e3d3a177538ee
```

Upload that file without repacking it to:

```text
https://boxedwine.org/v2/1/wine_tests_v2.zip
```

After uploading, verify the public file before relying on the default runner:

```bash
curl -fL https://boxedwine.org/v2/1/wine_tests_v2.zip -o /tmp/wine_tests_v2.zip
sha256sum /tmp/wine_tests_v2.zip
unzip -t /tmp/wine_tests_v2.zip
```

## Development checks

The unit tests use temporary files and fake subprocesses; they do not download
artifacts or build BoxedWine:

```bash
python3 -m unittest discover -s tools/wineTests/tests -v
python3 -m py_compile tools/wineTests/runWineTests.py
unzip -t tools/wineTests/wine_tests_v2.zip
```

If a real group fails, inspect its log and retained root under the run directory
printed at the end. Increase `--timeout` for a slow machine. Delete an invalid
or partial cached ZIP to make the runner download it again.
