import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import javax.tools.JavaCompiler;
import javax.tools.ToolProvider;

// Run with Java 11+: java tools/BoxedWineRunner/Build.java <project-dir> <output.jar> [--test]
// The runner itself remains compatible with Java 8. No javac/jar PATH entries are needed.
class Build {
    private static void compile(JavaCompiler compiler, Path source, Path output, String classpath) throws IOException {
        Files.createDirectories(output);
        // Headless JRE packages can include jdk.compiler without ct.sym (--release).
        List<String> options = new ArrayList<>(Arrays.asList("-source", "8", "-target", "8", "-Xlint:-options", "-d", output.toString()));
        if (classpath != null) options.addAll(Arrays.asList("-classpath", classpath));
        try (Stream<Path> files = Files.walk(source)) {
            files.filter(p -> p.toString().endsWith(".java")).sorted().forEach(p -> options.add(p.toString()));
        }
        if (compiler.run(null, null, null, options.toArray(new String[0])) != 0) {
            throw new IOException("Runner compilation failed");
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 2 || args.length > 3 || (args.length == 3 && !args[2].equals("--test"))) {
            throw new IllegalArgumentException("Usage: Build.java <project-dir> <output.jar> [--test]");
        }
        JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
        if (compiler == null) throw new IllegalStateException("Java's jdk.compiler module is required");
        Path project = Paths.get(args[0]).toAbsolutePath();
        Path output = Paths.get(args[1]).toAbsolutePath();
        Files.createDirectories(output.getParent());
        Path temp = Files.createTempDirectory(output.getParent(), "runner-build-");
        try {
            Path classes = temp.resolve("classes");
            compile(compiler, project.resolve("src"), classes, null);
            Manifest manifest = new Manifest();
            manifest.getMainAttributes().putValue("Manifest-Version", "1.0");
            manifest.getMainAttributes().putValue("Main-Class", "boxedwine.org.Main");
            try (OutputStream file = Files.newOutputStream(output);
                 JarOutputStream jar = new JarOutputStream(file, manifest);
                 Stream<Path> files = Files.walk(classes)) {
                for (Path path : files.filter(Files::isRegularFile).sorted().collect(Collectors.toList())) {
                    JarEntry entry = new JarEntry(classes.relativize(path).toString().replace('\\', '/'));
                    entry.setTime(0);
                    jar.putNextEntry(entry);
                    Files.copy(path, jar);
                    jar.closeEntry();
                }
            }
            if (args.length == 3) {
                Path tests = temp.resolve("tests");
                compile(compiler, project.resolve("test"), tests, classes.toString());
                String java = Paths.get(System.getProperty("java.home"), "bin", "java").toString();
                String classpath = classes + System.getProperty("path.separator") + tests;
                int rc = new ProcessBuilder(java, "-Xmx32m", "-cp", classpath,
                        "boxedwine.org.RunnerTests", output.toString()).inheritIO().start().waitFor();
                if (rc != 0) throw new IOException("Runner regression tests failed: " + rc);
            }
            System.out.println("Built " + output);
        } finally {
            try (Stream<Path> files = Files.walk(temp)) {
                for (Path path : files.sorted(Comparator.reverseOrder()).collect(Collectors.toList())) Files.delete(path);
            }
        }
    }
}
