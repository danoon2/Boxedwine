package org.boxedwine;

import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream;
import org.apache.commons.compress.utils.IOUtils;

import java.io.*;

public class ZipUtil {
    public static void createZip(File dir, File zipFile) {
        try {
            ZipArchiveOutputStream out = new ZipArchiveOutputStream(zipFile);
            out.setLevel(9);
            compressDirectoryToZipfile(dir.getAbsolutePath(), dir.getAbsolutePath(), out);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void compressDirectoryToZipfile(String rootDir, String sourceDir, ZipArchiveOutputStream out) throws IOException {
        File[] files = new File(sourceDir).listFiles();
        assert files != null;
        for (File file : files) {
            if (file.isDirectory()) {
                compressDirectoryToZipfile(rootDir, sourceDir + File.separator + file.getName(), out);
            } else {
                ZipArchiveEntry entry = new ZipArchiveEntry(file.getAbsolutePath().substring(rootDir.length() + 1));
                out.putArchiveEntry(entry);
                try (InputStream in = new BufferedInputStream(new FileInputStream(sourceDir + File.separator + file.getName()))) {
                    IOUtils.copy(in, out);
                }
                out.closeArchiveEntry();
            }
        }
    }
}
