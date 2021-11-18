package org.boxedwine;

// http://ftp.debian.org/debian/dists/buster/main/binary-i386/Packages.gz
// http://ftp.debian.org/debian/dists/buster/main/binary-all/Packages.gz

// format
    /*
    Package: 0ad-data
    Version: 0.0.17-1
    Installed-Size: 1422289
    Maintainer: Debian Games Team <pkg-games-devel@lists.alioth.debian.org>
    Architecture: all
    Pre-Depends: dpkg (>= 1.15.6~)
    Suggests: 0ad
    Description: Real-time strategy game of ancient warfare (data files)
    Homepage: http://play0ad.com/
    Description-md5: 26581e685027d5ae84824362a4ba59ee
    Tag: role::app-data
    Section: games
    Priority: optional
    Filename: pool/main/0/0ad-data/0ad-data_0.0.17-1_all.deb
    Size: 566073422
    MD5sum: b82e30c2927ed595cabbe8000ebb93b0
    SHA1: 08e4a649d2f2f7d9ae5ec4835fcd44ff58b0d3c2
    SHA256: 84ee024e2f19f0ffd732b419da2e594e57f615f45341926a3fb427746b7dc82b
    */

import java.io.*;
import java.net.URL;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Vector;
import java.util.zip.GZIPInputStream;

public class DebianPackages {
    private String name;
    private HashMap<String, DebianPackage> packages;
    private String debUrl = "http://ftp.us.debian.org/debian/";

    public HashSet<String> prefered;

    public static DebianPackages instance;

    public static DebianPackage getPackage(String name) {
        return instance.packages.get(name);
    }

    public DebianPackages(String debName) {
        this.name = debName;
        this.packages = new HashMap<>();
        loadPackages(getX86BinaryPackageURL());
    }

    URL getX86BinaryPackageURL() {
        try {
            return new URL(debUrl + "dists/" + name + "/main/binary-i386/Packages.gz");
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    Vector<String> parseDepends(String line) {
        Vector<String> result = new Vector<>();
        String[] parts = line.split(",");
        for (String part : parts) {
            result.add(part.trim());
        }
        return result;
    }

    void loadPackages(URL url) {
        File file = FileCache.getFile(url);
        if (file != null && file.exists()) {
            try {
                InputStream fileStream = new FileInputStream(file);
                InputStream gzipStream = new GZIPInputStream(fileStream);
                Reader decoder = new InputStreamReader(gzipStream, "UTF-8");
                BufferedReader buffered = new BufferedReader(decoder);

                String line;
                DebianPackage pkg = null;

                while ((line = buffered.readLine()) != null) {
                    if (line.startsWith("Package: ")) {
                        pkg = new DebianPackage();
                        pkg.name = line.substring(9).trim();
                        packages.put(pkg.name, pkg);
                    } else if (line.startsWith("Version: ")) {
                        pkg.version = line.substring(9).trim();
                    } else if (line.startsWith("Filename: ")) {
                        pkg.url = new URL(this.debUrl + line.substring(10).trim());
                    } else if (line.startsWith("Pre-Depends: ")) {
                        pkg.preDepends = parseDepends(line.substring(13).trim());
                    } else if (line.startsWith("Depends: ")) {
                        pkg.depends = parseDepends(line.substring(9).trim());
                    }
                }
            } catch (Exception e) {

            }
        }
    }
}
