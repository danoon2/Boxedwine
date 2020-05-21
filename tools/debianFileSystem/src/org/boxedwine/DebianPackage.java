package org.boxedwine;

import org.apache.commons.compress.archivers.ArchiveEntry;
import org.apache.commons.compress.archivers.ArchiveException;
import org.apache.commons.compress.archivers.ArchiveInputStream;
import org.apache.commons.compress.archivers.ArchiveStreamFactory;
import org.apache.commons.compress.archivers.ar.ArArchiveEntry;
import org.apache.commons.compress.archivers.ar.ArArchiveInputStream;
import org.apache.commons.compress.archivers.tar.TarArchiveEntry;
import org.apache.commons.compress.compressors.xz.XZCompressorInputStream;
import org.apache.commons.compress.utils.IOUtils;
import org.apache.commons.io.FileUtils;

import java.io.*;
import java.net.URL;
import java.util.*;

public class DebianPackage {
    public File getFile() {
        return FileCache.getFile(url);
    }

    private DebianPackage getDepend(DebianPackages pkgs, String depend, HashSet<String> prefered) {
        String[] parts = depend.split("\\|");
        String dep = depend;
        if (parts.length > 1) {
            int ii=0;
            dep = parts[0];
            for (String p : parts) {
                p = p.trim();
                if (prefered.contains(p)) {
                    dep = p;
                    break;
                }
            }
        }
        int pos = dep.indexOf("(");
        if (pos > 0) {
            dep = dep.substring(0, pos-1);
        }
        dep = dep.trim();
        DebianPackage result = pkgs.getPackage(dep);
        if (result == null) {
            int ii=0;
        }
        return result;
    }

    public void getDepends(DebianPackages pkgs, HashSet<String> ignore, HashSet<String> prefered, HashMap<String, DebianPackage> results) {
        if (preDepends != null) {
            for (String d : preDepends) {
                DebianPackage p = getDepend(pkgs, d, prefered);
                if (p != null && !results.containsKey(p.name) && !ignore.contains(p.name)) {
                    results.put(p.name, p);
                    p.getDepends(pkgs, ignore, prefered, results);
                }
            }
        }
        if (depends != null) {
            for (String d : depends) {
                DebianPackage p = getDepend(pkgs, d, prefered);
                if (p != null && !results.containsKey(p.name) && !ignore.contains(p.name)) {
                    results.put(p.name, p);
                    p.getDepends(pkgs, ignore, prefered, results);
                }
            }
        }
    }

    public void unpack() throws IOException, ArchiveException {
        final InputStream is = new FileInputStream(getFile());
        final ArArchiveInputStream debInputStream = (ArArchiveInputStream) new ArchiveStreamFactory().createArchiveInputStream("ar", is);
        ArArchiveEntry entry = null;
        while ((entry = (ArArchiveEntry)debInputStream.getNextEntry()) != null) {
            if (entry.getName().startsWith("data.")) {
                InputStream compressedInputStream = debInputStream;

                if (entry.getName().endsWith(".xz")) {
                    compressedInputStream = new XZCompressorInputStream(debInputStream);
                }
                final ArchiveInputStream dataInputStream = new ArchiveStreamFactory().createArchiveInputStream(new BufferedInputStream(compressedInputStream));
                TarArchiveEntry dataEntry = null;
                while ((dataEntry = (TarArchiveEntry)dataInputStream.getNextEntry()) != null) {
                    if (dataEntry.getName().equals("./")) {
                        continue;
                    }
                    if (dataEntry.getLinkName().length()!=0) {
                        final File outputFile = new File(Settings.outputDir, dataEntry.getName()+".link");
                        FileUtils.writeStringToFile(outputFile, dataEntry.getLinkName(), "UTF-8");
                        continue;
                    }
                    final File outputFile = new File(Settings.outputDir, dataEntry.getName());
                    if (dataEntry.getName().endsWith("/")) {
                        outputFile.mkdirs();
                        continue;
                    }
                    final OutputStream dataOutputFileStream = new FileOutputStream(outputFile);
                    IOUtils.copy(dataInputStream, dataOutputFileStream);
                    dataOutputFileStream.close();
                }
                dataInputStream.close();
                break;
            }
        }
        debInputStream.close();
    }

    public String name;
    public String version;
    public URL url;
    public Vector<String> preDepends;
    public Vector<String> depends;
}
