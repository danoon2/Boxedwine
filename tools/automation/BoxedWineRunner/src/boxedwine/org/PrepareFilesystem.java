package boxedwine.org;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;

/** Fetch the published native filesystem and stage its automation registry separately. */
public class PrepareFilesystem {
    static final String USER_REG = "home/username/.wine/user.reg";

    static class Pin {
        final URL url;
        final String sha256;
        final long size;

        Pin(Properties properties) throws IOException {
            url = new URL(properties.getProperty("url", ""));
            sha256 = properties.getProperty("sha256", "");
            try {
                size = Long.parseLong(properties.getProperty("size", ""));
            } catch (NumberFormatException e) {
                throw new IOException("Missing or invalid filesystem size", e);
            }
            if (!url.getProtocol().equals("https") || !sha256.matches("[0-9a-f]{64}") || size <= 0) {
                throw new IOException("Filesystem pin requires HTTPS, a SHA-256, and a positive byte count");
            }
        }
    }

    interface Download {
        void copy(URL url, Path destination) throws IOException;
    }

    static String sha256(Path path) throws IOException {
        MessageDigest digest;
        try {
            digest = MessageDigest.getInstance("SHA-256");
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalStateException(e);
        }
        try (InputStream input = Files.newInputStream(path)) {
            byte[] buffer = new byte[65536];
            int count;
            while ((count = input.read(buffer)) != -1) digest.update(buffer, 0, count);
        }
        StringBuilder hex = new StringBuilder();
        for (byte value : digest.digest()) hex.append(String.format("%02x", value & 255));
        return hex.toString();
    }

    static boolean matches(Path path, Pin pin) throws IOException {
        return Files.isRegularFile(path) && Files.size(path) == pin.size && sha256(path).equals(pin.sha256);
    }

    static Path download(Pin pin, Path cacheDirectory, Download download) throws IOException {
        Files.createDirectories(cacheDirectory);
        Path cached = cacheDirectory.resolve(pin.sha256 + ".zip");
        if (matches(cached, pin)) return cached;
        IOException failure = null;
        for (int attempt = 1; attempt <= 3; attempt++) {
            Path partial = Files.createTempFile(cacheDirectory, "filesystem-", ".part");
            try {
                System.out.println("Downloading " + pin.url + " (attempt " + attempt + ")");
                download.copy(pin.url, partial);
                if (!matches(partial, pin)) throw new IOException("Filesystem size or SHA-256 mismatch: " + pin.url);
                Files.move(partial, cached, REPLACE_EXISTING);
                return cached;
            } catch (IOException e) {
                failure = e;
                System.err.println(e.getMessage());
            } finally {
                Files.deleteIfExists(partial);
            }
        }
        throw new IOException("Could not download the pinned filesystem", failure);
    }

    static String setValues(String registry, String section, Map<String, String> values) {
        String newline = registry.contains("\r\n") ? "\r\n" : "\n";
        StringBuilder result = new StringBuilder();
        boolean inside = false;
        boolean found = false;
        String[] lines = registry.split("\\r?\\n", -1);
        for (int i = 0; i < lines.length; i++) {
            String line = lines[i];
            if (i == lines.length - 1 && line.isEmpty()) break;
            if (line.startsWith("[")) {
                inside = line.startsWith("[" + section + "]");
                if (inside) {
                    result.append(line).append(newline);
                    for (Map.Entry<String, String> value : values.entrySet()) {
                        result.append('"').append(value.getKey()).append("\"=\"")
                                .append(value.getValue()).append('"').append(newline);
                    }
                    found = true;
                    continue;
                }
            }
            boolean replaced = false;
            if (inside) {
                for (String key : values.keySet()) {
                    if (line.startsWith("\"" + key + "\"=")) replaced = true;
                }
            }
            if (!replaced) result.append(line).append(newline);
        }
        if (!found) {
            result.append(newline).append('[').append(section).append(']').append(newline);
            for (Map.Entry<String, String> value : values.entrySet()) {
                result.append('"').append(value.getKey()).append("\"=\"")
                        .append(value.getValue()).append('"').append(newline);
            }
        }
        return result.toString();
    }

    static String automationRegistry(String registry) throws IOException {
        if (!registry.startsWith("WINE REGISTRY Version 2")) throw new IOException("Invalid Wine user.reg");
        Map<String, String> values = new LinkedHashMap<>();
        values.put("VideoMemorySize", "256");
        values.put("DirectDrawRenderer", "gdi");
        values.put("renderer", "gdi");
        return setValues(registry, "Software\\\\Wine\\\\Direct3D", values);
    }

    static void stage(Path cached, Path outputDirectory) throws IOException {
        Files.createDirectories(outputDirectory);
        Path registry = outputDirectory.resolve("user.reg");
        // A failed preparation must not leave a usable registry from a previous pin.
        Files.deleteIfExists(registry);
        try (ZipFile zip = new ZipFile(cached.toFile())) {
            ZipEntry entry = zip.getEntry(USER_REG);
            if (entry == null || entry.isDirectory() || entry.getSize() < 0 || entry.getSize() > 4 * 1024 * 1024) {
                throw new IOException("Missing or oversized " + USER_REG + " in " + cached);
            }
            byte[] bytes = new byte[(int)entry.getSize()];
            try (InputStream input = zip.getInputStream(entry)) {
                int offset = 0;
                while (offset < bytes.length) {
                    int count = input.read(bytes, offset, bytes.length - offset);
                    if (count < 0) throw new IOException("Truncated " + USER_REG);
                    offset += count;
                }
            }
            String prepared = automationRegistry(new String(bytes, StandardCharsets.UTF_8));
            Files.copy(cached, outputDirectory.resolve("fs.zip"), REPLACE_EXISTING);
            Files.write(registry, prepared.getBytes(StandardCharsets.UTF_8));
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 3) throw new IllegalArgumentException(
                "Usage: PrepareFilesystem <filesystem.properties> <cache-directory> <output-directory>");
        Properties properties = new Properties();
        try (InputStream input = Files.newInputStream(Paths.get(args[0]))) {
            properties.load(input);
        }
        Pin pin = new Pin(properties);
        Path cached = download(pin, Paths.get(args[1]), (url, destination) -> {
            URLConnection connection = url.openConnection();
            connection.setConnectTimeout(30000);
            connection.setReadTimeout(60000);
            try (InputStream input = connection.getInputStream()) {
                Files.copy(input, destination, REPLACE_EXISTING);
            }
        });
        stage(cached, Paths.get(args[2]));
        System.out.println("Native filesystem: " + pin.url + " size=" + pin.size + " sha256=" + pin.sha256);
        System.out.println("ZIP unchanged; automation user.reg staged separately in " + args[2]);
    }
}
