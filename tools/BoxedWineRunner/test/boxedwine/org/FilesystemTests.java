package boxedwine.org;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Properties;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;

class FilesystemTests {
    private static void check(boolean value, String message) {
        if (!value) throw new AssertionError(message);
    }

    private static Properties properties(Path source) throws IOException {
        Properties properties = new Properties();
        properties.setProperty("url", "https://example.invalid/full.zip");
        properties.setProperty("sha256", PrepareFilesystem.sha256(source));
        properties.setProperty("size", String.valueOf(Files.size(source)));
        return properties;
    }

    static void run(Path root) throws Exception {
        Path source = root.resolve("published.zip");
        String original = "WINE REGISTRY Version 2\n\n[Software\\\\Wine\\\\Direct3D] 123\n"
                + "#time=abc\n\"renderer\"=\"gl\"\n\"VideoMemorySize\"=\"64\"\n\"Keep\"=\"value\"\n\n"
                + "[Software\\\\Wine\\\\DirectInput]\n\"MouseWarpOverride\"=\"disable\"\n\n"
                + "[Software\\\\Other]\n\"renderer\"=\"untouched\"\n";
        try (ZipOutputStream zip = new ZipOutputStream(Files.newOutputStream(source))) {
            zip.putNextEntry(new ZipEntry("empty/"));
            zip.closeEntry();
            zip.putNextEntry(new ZipEntry(PrepareFilesystem.USER_REG));
            zip.write(original.getBytes(StandardCharsets.UTF_8));
            zip.closeEntry();
            zip.putNextEntry(new ZipEntry("opt/wine/bin/wineserver"));
            zip.write(new byte[] {1, 2, 3});
            zip.closeEntry();
        }
        PrepareFilesystem.Pin pin = new PrepareFilesystem.Pin(properties(source));
        Path cacheDirectory = root.resolve("cache");
        int[] requests = {0};
        PrepareFilesystem.Download download = (url, destination) -> {
            requests[0]++;
            Files.copy(source, destination, REPLACE_EXISTING);
        };
        Path cached = PrepareFilesystem.download(pin, cacheDirectory, download);
        check(requests[0] == 1, "did not download the pinned filesystem");
        PrepareFilesystem.download(pin, cacheDirectory, download);
        check(requests[0] == 1, "did not reuse the verified cache");
        byte[] corrupt = Files.readAllBytes(cached);
        corrupt[0] ^= 1;
        Files.write(cached, corrupt);
        PrepareFilesystem.download(pin, cacheDirectory, download);
        check(requests[0] == 2 && PrepareFilesystem.matches(cached, pin), "reused a corrupt cache");

        Files.delete(cached);
        try {
            PrepareFilesystem.download(pin, cacheDirectory, (url, destination) -> Files.write(destination, corrupt));
            throw new AssertionError("accepted a download with the wrong SHA");
        } catch (IOException expected) {
            check(!Files.exists(cached), "published a bad cache entry");
        }
        try (java.util.stream.Stream<Path> files = Files.list(cacheDirectory)) {
            check(!files.findAny().isPresent(), "left a partial download behind");
        }
        cached = PrepareFilesystem.download(pin, cacheDirectory, download);
        Path output = root.resolve("fs");
        PrepareFilesystem.stage(cached, output);
        check(Arrays.equals(Files.readAllBytes(source), Files.readAllBytes(output.resolve("fs.zip"))),
                "changed the published ZIP (including directory entries)");
        String registry = new String(Files.readAllBytes(output.resolve("user.reg")), StandardCharsets.UTF_8);
        check(registry.contains("\"renderer\"=\"gdi\"") && registry.contains("\"DirectDrawRenderer\"=\"gdi\"")
                && registry.contains("\"VideoMemorySize\"=\"256\""), "missing automation presets");
        check(!registry.contains("\"gl\""), "kept old renderer preset");
        check(registry.contains("\"MouseWarpOverride\"=\"disable\""), "changed a source mouse setting");
        check(registry.contains("\"renderer\"=\"untouched\"") && registry.contains("\"Keep\"=\"value\"")
                && registry.contains("#time=abc"), "lost unrelated registry content");
        check(registry.equals(PrepareFilesystem.automationRegistry(registry)), "presets are not idempotent");
        String missingSections = PrepareFilesystem.automationRegistry("WINE REGISTRY Version 2\r\n");
        check(missingSections.contains("[Software\\\\Wine\\\\Direct3D]\r\n"), "did not add the renderer section");
        check(!missingSections.contains("MouseWarpOverride"), "added an unnecessary mouse override");

        // No process is launched: scripts without Install/Play files leave the root in place.
        Path test = Files.createDirectory(root.resolve("registry-root"));
        Path files = Files.createDirectory(test.resolve("files"));
        Main.userRegistry = output.resolve("user.reg");
        try {
            Main.runTest("registry", files.toString(), test.toString(), new Main.Results());
            Path installed = test.resolve("root").resolve(PrepareFilesystem.USER_REG);
            check(Arrays.equals(Files.readAllBytes(installed), Files.readAllBytes(Main.userRegistry)), "did not seed test root");
            Files.write(installed, new byte[] {0});
            Main.runTest("registry", files.toString(), test.toString(), new Main.Results());
            check(Arrays.equals(Files.readAllBytes(installed), Files.readAllBytes(Main.userRegistry)), "did not restore presets on retry");
        } finally {
            Main.userRegistry = null;
        }

        Path badZip = root.resolve("missing-registry.zip");
        try (ZipOutputStream zip = new ZipOutputStream(Files.newOutputStream(badZip))) {
            zip.putNextEntry(new ZipEntry("empty/"));
            zip.closeEntry();
        }
        try {
            PrepareFilesystem.stage(badZip, output);
            throw new AssertionError("accepted a filesystem without user.reg");
        } catch (IOException expected) {
            check(!Files.exists(output.resolve("user.reg")), "kept stale presets after failed preparation");
        }
        Properties insecure = properties(source);
        insecure.setProperty("url", "http://example.invalid/full.zip");
        try {
            new PrepareFilesystem.Pin(insecure);
            throw new AssertionError("accepted an HTTP filesystem pin");
        } catch (IOException expected) {}
    }
}
