package org.boxedwine;

import org.apache.commons.io.FileUtils;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.HashSet;

public class Main {
    public static void main(String[] args) {
        Settings.fileCachePath = new File("c:\\debianCache");
        Settings.outputDir = new File("c:\\debianCache\\out");
        Settings.extraFiles = new File("C:\\boxedwine-source\\tools\\debianFileSystem\\fs");
        Settings.finishedZip = new File("c:\\debianCache\\debian10.zip");
        Settings.boxedwinePath = "boxedwine"; // find it in the OS path
        Settings.winePath = "c:\\debianCache\\wine-5.0.zip";

        DebianPackages packages = new DebianPackages("buster");
        HashSet<String> prefered = new HashSet<>();
        HashSet<String> ignored = new HashSet<>();
        prefered.add("fonts-liberation");

        // for now, lets ignore this, otherwise we will have to pull in Mesa
        ignored.add("ocl-icd-libopencl1");
        ignored.add("libopencl1");

        ignored.add("libncurses6");

        // only useful for install with deb
        ignored.add("debconf");

        // Boxedwine doesn't support this
        ignored.add("libpusle0");
        ignored.add("libasound2-data");
        ignored.add("libvorbisenc2");

        // definitely don't need X11 related stuff
        ignored.add("libxext6");
        ignored.add("libx11-xcb1");
        ignored.add("libx11-6");
        ignored.add("x11-common");
        ignored.add("libpcap0.8");
        ignored.add("libxau6");
        ignored.add("libxi6");
        ignored.add("libice6");

        ignored.add("iso-codes"); // this a big one and only required by libgstreamer-plugins-base1.0-0
        ignored.add("libicu63");

        HashMap<String, DebianPackage> depends = new HashMap<>();
        packages.getPackage("wine32-preloader").getDepends(packages, ignored, prefered, depends);
        depends.put("libc-bin", packages.getPackage("libc-bin"));
        packages.getPackage("libc-bin").getDepends(packages, ignored, prefered, depends);
        depends.put("fontconfig", packages.getPackage("fontconfig"));
        packages.getPackage("fontconfig").getDepends(packages, ignored, prefered, depends);

        depends.remove("wine32-preloader");
        depends.remove("wine32");
        depends.remove("libwine");
        if (Settings.outputDir.exists()) {
            try {
                FileUtils.deleteDirectory(Settings.outputDir);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        if (!Settings.outputDir.exists()) {
            Settings.outputDir.mkdirs();
        }
        for (String n : depends.keySet()) {

            System.out.println(n);
            depends.get(n).getFile();
            try {
                depends.get(n).unpack();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        try {
            File wineLink = new File(Settings.outputDir + File.separator + "bin" + File.separator + "wine.link");
            FileUtils.writeStringToFile(wineLink, "/opt/wine/bin/wine", "UTF-8");
            File ldConfig = new File(Settings.outputDir + File.separator + "etc" + File.separator + "ld.so.conf.d" + File.separator + "wine.conf");
            FileUtils.writeStringToFile(ldConfig, "/opt/wine/lib", "UTF-8");
            File resolv = new File(Settings.outputDir + File.separator + "etc" + File.separator + "resolv.conf");
            FileUtils.writeStringToFile(resolv, "nameserver 8.8.8.8", "UTF-8");
            File group = new File(Settings.outputDir + File.separator + "etc" + File.separator + "group");
            FileUtils.writeStringToFile(group, "root:x:0:\nadm:x:4:\nmail:x:8:\nshadow:x:42:\nutmp:x:43:\nstaff:x:50:\nutempter:x:100:\nwinbindd_priv:x:101:\ndebian-tor:x:141:\nmessagebus:x:108:\nnogroup:x:65534:\nsystemd-timesync:x:129:\nsystemd-network:x:130:\nsystemd-resolve:x:131:", "UTF-8");
            File gshadow = new File(Settings.outputDir + File.separator + "etc" + File.separator + "gshadow");
            FileUtils.writeStringToFile(gshadow, "root:*::\nmail:*::\nshadow:*::\nutmp:*::\nstaff:*::\nutempter:*::\nwinbindd_priv:*::\nmessagebus:*::\nnogroup:*::\nsystemd-timesync:!::\nsystemd-network:!::\nsystemd-resolve:!::\nadm:x::\ndebian-tor:x::", "UTF-8");
            File passwd = new File(Settings.outputDir + File.separator + "etc" + File.separator + "passwd");
            FileUtils.writeStringToFile(passwd, "root:x:0:0:root:/root:/bin/bash\nusername:x:1:2:root:/home/username:/bin/bash\nmessagebus:x:102:108::/var/run/dbus:/bin/false\nsystemd-timesync:x:122:129:systemd Time Synchronization,,,:/run/systemd:/bin/false\nsystemd-network:x:123:130:systemd Network Management,,,:/run/systemd/netif:/bin/false\nsystemd-resolve:x:124:131:systemd Resolver,,,:/run/systemd/resolve:/bin/false\n", "UTF-8");
            File shadow = new File(Settings.outputDir + File.separator + "etc" + File.separator + "shadow");
            FileUtils.writeStringToFile(shadow, "root:*:17594:0:99999:7:::\nmessagebus:*:16767:0:99999:7:::\nsystemd-timesync:*:17163:0:99999:7:::\nsystemd-network:*:17163:0:99999:7:::\nsystemd-resolve:*:17163:0:99999:7:::\nusername:x:18312::::::", "UTF-8");
            new File(Settings.outputDir + File.separator + "run" + File.separator + "user" + File.separator + "1").mkdirs();
            //let the wine file system contain /home/username
            //new File(Settings.outputDir + File.separator + "home" + File.separator + "username").mkdirs();
            new File(Settings.outputDir + File.separator + "root").mkdirs();
            new File(Settings.outputDir + File.separator + "tmp").mkdirs();
            FileUtils.copyDirectory(Settings.extraFiles, Settings.outputDir);

            ProcessBuilder builder = new ProcessBuilder(Settings.boxedwinePath, "-root", Settings.outputDir.getAbsolutePath(), "-zip", Settings.winePath, "-uid", "0", "/sbin/ldconfig");
            Process process = builder.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line = null;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            builder = new ProcessBuilder(Settings.boxedwinePath, "-root", Settings.outputDir.getAbsolutePath(), "-uid", "0", "/usr/bin/fc-cache");
            process = builder.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line = null;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            // using 7zip created a 3% smaller zip
            //ZipUtil.createZip(Settings.outputDir, Settings.finishedZip);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
