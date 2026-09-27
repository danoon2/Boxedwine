package boxedwine.org;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class RunnerTests {
    private static void check(boolean value, String message) {
        if (!value) throw new AssertionError(message);
    }

    private static void checkOutputLimits(int lineLength) {
        // Generate 128 MiB without storing it. This test runs with a 32 MiB heap.
        InputStream input = new InputStream() {
            long remaining = 128L * 1024 * 1024;
            int suffix;
            final byte[] tail = "\nscript: success\r\nlast line".getBytes(StandardCharsets.UTF_8);
            public int read() {
                if (remaining > 0) {
                    remaining--;
                    return lineLength != 0 && remaining % lineLength == 0 ? '\n' : 'x';
                }
                return suffix < tail.length ? tail[suffix++] : -1;
            }
            public int read(byte[] b, int off, int len) {
                int count = 0;
                while (count < len) {
                    int value = read();
                    if (value == -1) break;
                    b[off + count++] = (byte)value;
                }
                return count == 0 ? -1 : count;
            }
        };
        StreamGobbler gobbler = new StreamGobbler(input, "flood");
        gobbler.run();
        check(gobbler.failure == null, "flood failed: " + gobbler.failure);
        check(gobbler.scriptFinished, "lost success marker after output flood");
        Vector<String> lines = gobbler.getLines();
        check(lines.firstElement().startsWith("Output limited:"), "missing truncation notice");
        check(lines.lastElement().equals("last line"), "lost unterminated final line");
        check(lines.size() <= StreamGobbler.MAX_LINES + 1, "unbounded line count");
        int chars = 0;
        for (String line : lines) chars += line.length() + 1;
        check(chars <= StreamGobbler.MAX_CHARS + 200, "unbounded text storage");
    }

    private static void checkLineEndings() {
        byte[] text = "one\rtwo\r\n\nscript: success\nlast".getBytes(StandardCharsets.UTF_8);
        StreamGobbler gobbler = new StreamGobbler(new ByteArrayInputStream(text), "endings");
        gobbler.run();
        check(gobbler.getLines().equals(new Vector<>(Arrays.asList("one", "two", "", "script: success", "last"))), "line ending handling");
        check(gobbler.scriptFinished, "success marker missing");
        StreamGobbler prefix = new StreamGobbler(new ByteArrayInputStream("script: success extra".getBytes(StandardCharsets.UTF_8)), "prefix");
        prefix.run();
        check(!prefix.scriptFinished, "accepted a partial success marker");
    }

    private static void checkReaderFailures() {
        for (Throwable failure : Arrays.asList(new IOException("injected read failure"),
                new IllegalStateException("injected runtime failure"), new OutOfMemoryError("injected OOM"))) {
            InputStream input = new InputStream() {
                public int read() throws IOException {
                    if (failure instanceof IOException) throw (IOException)failure;
                    if (failure instanceof RuntimeException) throw (RuntimeException)failure;
                    throw (Error)failure;
                }
            };
            StreamGobbler gobbler = new StreamGobbler(input, "broken");
            gobbler.run();
            check(gobbler.failure == failure, "reader failure was hidden");
        }
    }

    private static Path fixture(Path root, String name) throws IOException {
        Path dir = Files.createDirectory(root.resolve(name));
        Path app = Files.createDirectories(dir.resolve("scripts/app/files")).getParent();
        Files.write(app.resolve("Play.txt"), Arrays.asList("/files", "fake.exe"), StandardCharsets.UTF_8);
        return dir;
    }

    private static String run(Path fixture, Path jar, Path executable, int expected) throws Exception {
        Path log = fixture.resolve("runner.log");
        String java = Paths.get(System.getProperty("java.home"), "bin", "java").toString();
        Process process = new ProcessBuilder(java, "-Xmx32m", "-Dboxedwine.runner.timeout.seconds=1", "-jar", jar.toString(),
                fixture.resolve("unused.zip").toString(), fixture.resolve("scripts").toString(), executable.toString())
                .redirectErrorStream(true).redirectOutput(log.toFile()).start();
        try {
            check(process.waitFor(20, TimeUnit.SECONDS), "runner hung: " + fixture);
            String output = new String(Files.readAllBytes(log), StandardCharsets.UTF_8);
            check(process.exitValue() == expected, "wrong exit code " + process.exitValue() + ": " + output);
            return output;
        } finally {
            if (process.isAlive()) process.destroyForcibly();
        }
    }

    private static void checkProcesses(Path root, Path jar) throws Exception {
        Path missing = fixture(root, "missing-executable");
        String output = run(missing, jar, missing.resolve("nonexistent"), 1);
        check(output.contains("FAILED app"), "missing executable was not reported");
        check(output.split("RETRY  app", -1).length == 3, "expected three attempts");
        if (File.separatorChar != '/') return;

        // An unchecked Files.copy failure previously disappeared in an ignored Future.
        Path broken = fixture(root, "worker-exception");
        Files.createSymbolicLink(broken.resolve("scripts/app/files/broken-link"), Paths.get("does-not-exist"));
        output = run(broken, jar, Paths.get("/bin/true"), 1);
        check(output.contains("Automation worker failed:"), "worker exception was swallowed");

        Path ok = fixture(root, "success");
        Path executable = root.resolve("fake-boxedwine");
        Files.write(executable, Arrays.asList("#!/bin/sh", "echo 'script: success'", "exit 111"), StandardCharsets.UTF_8);
        check(executable.toFile().setExecutable(true), "cannot make fake child executable");
        output = run(ok, jar, executable, 0);
        check(output.contains("OK     app"), "successful child failed");

        Path retry = fixture(root, "retries");
        Path count = retry.resolve("attempts");
        Files.write(executable, Arrays.asList("#!/bin/sh", "echo attempt >> '" + count + "'", "exit 1"), StandardCharsets.UTF_8);
        output = run(retry, jar, executable, 1);
        check(Files.readAllLines(count).size() == 3, "child was not run three times");

        Path timeout = fixture(root, "timeout");
        Path childPids = timeout.resolve("children");
        Files.write(executable, Arrays.asList("#!/bin/sh", "echo $$ >> '" + childPids + "'", "exec sleep 60"), StandardCharsets.UTF_8);
        output = run(timeout, jar, executable, 1);
        check(output.contains("after 1 seconds"), "child timeout did not fire");
        for (String pid : Files.readAllLines(childPids)) {
            Process probe = new ProcessBuilder("kill", "-0", pid).redirectErrorStream(true).start();
            check(probe.waitFor() != 0, "timed-out child is still alive: " + pid);
        }
    }

    public static void main(String[] args) throws Exception {
        checkOutputLimits(100);
        checkOutputLimits(4096);
        checkOutputLimits(0);
        checkLineEndings();
        checkReaderFailures();
        Path root = Files.createTempDirectory("boxedwine-runner-test-");
        try {
            checkProcesses(root, Paths.get(args[0]).toAbsolutePath());
        } finally {
            try (Stream<Path> paths = Files.walk(root)) {
                List<Path> files = paths.sorted(Comparator.reverseOrder()).collect(Collectors.toList());
                for (Path file : files) Files.delete(file);
            }
        }
        System.out.println("Runner regression tests passed (32 MiB heap)");
    }
}
