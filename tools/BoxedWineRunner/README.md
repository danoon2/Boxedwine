# BoxedWineRunner

Build and run the regression tests with Java 11 or newer:

```text
java tools/BoxedWineRunner/Build.java tools/BoxedWineRunner automation-runner/bin/BoxedWineRunner.jar --test
```

Run this command from the repository root. The Java installation must include
`jdk.compiler`; separate `javac` and `jar` commands are not needed in `PATH`.
Omit `--test` to only build the JAR. The runner uses Java 8 APIs and bytecode.

Jenkins builds and tests this JAR from the current checkout, then distributes it
to the native automation workers over the JAR bundled in `automation33.zip`.
It also stashes the recordings in `overrides/` and overlays them onto each
worker's extracted `automation/scripts/` directory before running tests.

The Caesar III installer override handles the additional Add Bookmarks page
shown when the full filesystem's Internet Explorer is detected. It verifies
that page, selects DO NOT Install Bookmarks, and continues with the original
Read Me and completion checks. For local runs from an existing bundle, copy
the contents of `overrides/` into its `automation/scripts/` directory as well.

## Native filesystem

Native automation downloads the **full** filesystem pinned by URL, byte count,
and SHA-256 in [filesystem.properties](filesystem.properties). Browser automation
has its own web filesystem pin in the root Jenkinsfile.

`automation33.zip` contains no filesystem. Jenkins runs this command inside
`automation/` after extracting the bundle and unstashing the runner and pin:

```text
java -cp bin/BoxedWineRunner.jar boxedwine.org.PrepareFilesystem filesystem.properties ../automation-filesystems fs
```

The preparer verifies cached downloads on every use, retries failed downloads,
and stages the exact published bytes as `fs/fs.zip`. Its checksum-addressed cache
lives outside `automation/`, so refreshing scripts does not discard the download.
A download or verification failure stops preparation; there is no fallback to
the bundled filesystem.

It also reads `home/username/.wine/user.reg` from the downloaded ZIP and writes
`fs/user.reg`, preserving other registry values while applying automation32's
existing settings:

```text
[Software\\Wine\\Direct3D]
"VideoMemorySize"="256"
"DirectDrawRenderer"="gdi"
"renderer"="gdi"
```

No mouse override is added. The old bundle's `MouseWarpOverride=enable` selects
[Wine 11's default behavior](https://github.com/wine-mirror/wine/blob/wine-11.0/dlls/dinput/mouse.c#L491-L507);
the preparer leaves the source's input settings alone.

Pass `-user-reg fs/user.reg` **before** the filesystem and script arguments for
both functional and Cinebench runs. The runner copies that registry into every
fresh writable test root, including retries. The downloaded ZIP stays unchanged.
For example, from `automation/`:

```text
java -jar bin/BoxedWineRunner.jar -user-reg fs/user.reg /absolute/path/to/automation/fs/fs.zip /absolute/path/to/automation/scripts /absolute/path/to/boxedwine -nosound -novideo
```

To advance native Wine, update the three fields in `filesystem.properties` from
the final full ZIP and qualify the automation workers. Follow the
[filesystem release checklist](../buildWine/RELEASING.md). Updating Wine no longer
requires repackaging the automation asset bundle for Jenkins. The bundled
`runAll.bat`, `runall.sh`, and Cinebench launchers prepare the full pinned ZIP
and pass `-user-reg` automatically. For manual runs, update the extracted
`filesystem.properties` when advancing Wine.

## Packaging automation assets

Use Python 3.11+ and the tested runner to create a new bundle:

```text
python3 tools/BoxedWineRunner/package_assets.py automation32.zip automation33.zip --runner automation-runner/bin/BoxedWineRunner.jar
```

The output must not already exist. The packager removes `automation/fs/`, embeds
the current runner, filesystem pin and launchers from `bundle/`, and verifies
all retained files byte-for-byte along with ZIP CRCs. Recordings and screenshots
in `overrides/` replace the corresponding script assets; app files and all other
assets stay unchanged. Earlier README/validation files are preserved
under `automation/provenance/`; the new validation record describes packaging.
It also writes SHA-256 and validation sidecars next to the archive.

Run the packaging and Unix launcher checks with
`python3 -m unittest discover -s tools/BoxedWineRunner -p test_package_assets.py`.

Publish the new archive before using its updated Jenkins URLs. Keep older
bundles available for jobs pinned to earlier revisions.

## Runner behavior and tests

Each child process has a 30-minute timeout, configurable with
`-Dboxedwine.runner.timeout.seconds=SECONDS` before `-jar`. Failed scripts get
three attempts. Worker exceptions and failed output readers make the runner
exit with a nonzero status so Jenkins can retry the job.

Captured output keeps the most recent 4,096 lines, up to 512 Ki characters,
with individual lines limited to 8,192 characters. A notice reports discarded
or truncated output. `-v` still streams output to the console. This prevents a
repeated native error from exhausting the Java heap while retaining the end of
the failure log.

The regression tests run with a 32 MiB heap and cover verified filesystem cache
reuse, corrupt downloads, registry presets, unchanged ZIP bytes, fresh test
roots, large output streams, oversized lines, reader failures, and runner failures.
Filesystem tests use local fixtures and make no network requests. On POSIX hosts they also
check successful execution, retries, worker exceptions, and timeout cleanup
using mock executables; they do not require Wine or a Boxedwine build.
