# BoxedWineRunner

Build and run the regression tests with Java 11 or newer:

```text
java tools/BoxedWineRunner/Build.java tools/BoxedWineRunner automation-runner/bin/BoxedWineRunner.jar --test
```

Run this command from the repository root. The Java installation must include
`jdk.compiler`; separate `javac` and `jar` commands are not needed in `PATH`.
Omit `--test` to only build the JAR. The runner uses Java 8 APIs and bytecode.

Jenkins builds and tests this JAR from the current checkout, then distributes it
to the native automation workers over the JAR bundled in `automation32.zip`.

Each child process has a 30-minute timeout, configurable with
`-Dboxedwine.runner.timeout.seconds=SECONDS` before `-jar`. Failed scripts get
three attempts. Worker exceptions and failed output readers make the runner
exit with a nonzero status so Jenkins can retry the job.

Captured output keeps the most recent 4,096 lines, up to 512 Ki characters,
with individual lines limited to 8,192 characters. A notice reports discarded
or truncated output. `-v` still streams output to the console. This prevents a
repeated native error from exhausting the Java heap while retaining the end of
the failure log.

The regression tests run with a 32 MiB heap and cover large output streams,
oversized lines, reader failures, and runner failures. On POSIX hosts they also
check successful execution, retries, worker exceptions, and timeout cleanup
using mock executables; they do not require Wine or a Boxedwine build.
