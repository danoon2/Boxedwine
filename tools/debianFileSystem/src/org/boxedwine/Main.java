package org.boxedwine;

import org.apache.commons.io.FileUtils;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.HashSet;

public class Main {
    static boolean includeX11 = false;
    static String release = "bullseye";

    public static void add(String name, HashMap<String, DebianPackage> depends, HashSet<String> ignored) {
        DebianPackages.getPackage(name).getDepends(ignored, depends);
        depends.put(name, DebianPackages.getPackage(name));
    }
    public static void main(String[] args) {
        Settings.fileCachePath = new File("c:\\debianCache");
        Settings.outputDir = new File("c:\\debianCache\\out");
        Settings.extraFiles = new File("C:\\boxedwine\\tools\\debianFileSystem\\fs");
        Settings.boxedwinePath = "C:\\Boxedwine\\project\\msvc\\BoxedWine\\x64\\Release\\Boxedwine"; // find it in the OS path

        DebianPackages.instance = new DebianPackages(release);
        DebianPackages.instance.prefered = new HashSet<>();
        HashSet<String> ignored = new HashSet<>();
        DebianPackages.instance.prefered.add("fonts-liberation");

        if (!includeX11) {
            // for now, lets ignore this, otherwise we will have to pull in Mesa
            ignored.add("ocl-icd-libopencl1");
            ignored.add("libopencl1");

            // Boxedwine doesn't support this
            ignored.add("libpusle0");
            ignored.add("libasound2-data");
            ignored.add("libvorbisenc2");

            // only useful for install with deb
            ignored.add("debconf");

            ignored.add("libncurses6");

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
            ignored.add("adwaita-icon-theme");
            ignored.add("ahicolor-icon-theme");
            ignored.add("libgl1");
            ignored.add("libwayland-client0");
            ignored.add("libwayland-cursor0");
            ignored.add("libwayland-egl1");
            ignored.add("libxcursor1");
            ignored.add("libxinerama1");
            ignored.add("libxkbcommon0");
            ignored.add("libxss1");
            ignored.add("libxxf86vm1");
        }

        HashMap<String, DebianPackage> depends = new HashMap<>();
        add("curl", depends, ignored);
        add("libnss-mdns", depends, ignored);
        add("binutils", depends, ignored);
        add("cabextract", depends, ignored);
        add("p7zip", depends, ignored);
        add("unzip", depends, ignored);
        //add("unrar", depends, ignored);
        add("xz-utils", depends, ignored);
        add("dash", depends, ignored);
        add("sed", depends, ignored);
        add("grep", depends, ignored);
        add("aria2", depends, ignored);
        add("gawk", depends, ignored);
        add("coreutils", depends, ignored);
        add("xdg-utils", depends, ignored);
        add("zenity", depends, ignored);
        add("openssl", depends, ignored);
        add("winbind", depends, ignored);
        add("gstreamer1.0-libav", depends, ignored);
        add("gstreamer1.0-plugins-bad", depends, ignored);
        add("gstreamer1.0-plugins-ugly", depends, ignored);
        add("fonts-wine", depends, ignored);
        //add("libosmesa6", depends, ignored);
        add("libsdl2-2.0-0", depends, ignored);
        add("ca-certificates", depends, ignored);
        add("findutils", depends, ignored); //arial font in winetricks needs this
        add("libsdl2-2.0-0", depends, ignored);
        add("gnutls-bin", depends, ignored);
        add("libsdl2-2.0-0", depends, ignored);
        add("apt", depends, ignored);
        add("wget", depends, ignored);
        add("git", depends, ignored);

        DebianPackages.getPackage("wine32-preloader").getDepends(ignored, depends);
        depends.put("libc-bin", DebianPackages.getPackage("libc-bin"));
        DebianPackages.getPackage("libc-bin").getDepends(ignored, depends);
        depends.put("fontconfig", DebianPackages.getPackage("fontconfig"));
        DebianPackages.getPackage("fontconfig").getDepends(ignored, depends);

        depends.remove("wine32-preloader");
        depends.remove("wine32");
        depends.remove("libwine");
        if (true) {
            depends.put("dpg", DebianPackages.getPackage("dpkg"));
            DebianPackages.getPackage("dpkg").getDepends(ignored, depends);
            depends.put("dash", DebianPackages.getPackage("dash"));
            DebianPackages.getPackage("dash").getDepends(ignored, depends);
            depends.put("diffutils", DebianPackages.getPackage("diffutils"));
            DebianPackages.getPackage("diffutils").getDepends(ignored, depends);

            // required by libc dpkg install
            depends.put("sed", DebianPackages.getPackage("sed"));
            DebianPackages.getPackage("sed").getDepends(ignored, depends);
            depends.put("gawk", DebianPackages.getPackage("gawk"));
            DebianPackages.getPackage("gawk").getDepends(ignored, depends);
            depends.put("bash", DebianPackages.getPackage("bash")); // circular dependency with menu
            DebianPackages.getPackage("bash").getDepends(ignored, depends);

            depends.put("libbz2-1.0", DebianPackages.getPackage("libbz2-1.0")); // circular dependency with menu
            DebianPackages.getPackage("libbz2-1.0").getDepends(ignored, depends);
        }
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
            File wineserverLink = new File(Settings.outputDir + File.separator + "bin" + File.separator + "wineserver.link");
            FileUtils.writeStringToFile(wineserverLink, "/opt/wine/bin/wineserver", "UTF-8");
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
            if (includeX11) {
                new File(Settings.outputDir+"/lib/libGL.so.1").delete();
            }
            ProcessBuilder builder = new ProcessBuilder(Settings.boxedwinePath, "-root", Settings.outputDir.getAbsolutePath(), "-uid", "0", "/sbin/ldconfig");
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

            if (true) {
                for (int i=0;i<2;i++) {
                    boolean downloadOnly = i==0;
                    HashSet<String> ignoreDependency = new HashSet<>();
                    touch("/var/lib/dpkg/status");
                    new File(Settings.outputDir + "/var/log/").mkdirs();
                    new File(Settings.outputDir + "/var/run/").mkdirs();
                    DPkg.install("dpkg", ignoreDependency, true, downloadOnly);
                    DPkg.install("libbz2-1.0", ignoreDependency, true, downloadOnly); // requires libc
                    // bullseye
                    // DPkg.install("libgcc1", ignoreDependency, true, downloadOnly); // requires libc
                    DPkg.install("gawk", ignoreDependency, true, downloadOnly); // requires libc
                    //DPkg.install("libbz2-1.0", ignoreDependency, false);
                    DPkg.install("libc6", ignoreDependency, true, downloadOnly);
                    DPkg.install("libcrypt1", ignoreDependency, true, downloadOnly);

                    // now install cleanly
                    DPkg.install("libc6", downloadOnly);
                    DPkg.install("gawk", downloadOnly);
                    DPkg.install("libbz2-1.0", downloadOnly);
                    DPkg.install("dpkg", downloadOnly);
                    // required by some installs but not listed as dependency
                    DPkg.install("perl", downloadOnly);
                    DPkg.install("libterm-readline-gnu-perl", downloadOnly);
                    DPkg.install("init-system-helpers", downloadOnly); // need for update-rc.d
                    DPkg.install("menu", downloadOnly); // bash requires menu and menu requires base
                    DPkg.install("grep", downloadOnly);
                    DPkg.install("bash", downloadOnly);
                    DPkg.install("util-linux", downloadOnly); // needed for getopt
                    DPkg.install("xz-utils", downloadOnly);
                    DPkg.install("apt", downloadOnly);
                    DPkg.install("apt-utils", downloadOnly);
                    DPkg.install("wget", downloadOnly);

                    // don't want x11 dependencies
                    DPkg.install("git", ignoreDependency, true, downloadOnly);

                    File arch = new File(Settings.outputDir + "/var/lib/dpkg/arch");
                    FileUtils.writeStringToFile(arch, "i386", "UTF-8");

                    File sources = new File(Settings.outputDir + "/etc/apt/sources.list");
                    FileUtils.writeStringToFile(sources, "deb http://deb.debian.org/debian/ "+release+" main contrib non-free\n" +
                            "deb http://deb.debian.org/debian/ "+release+"-updates main contrib non-free\n" +
                            "deb http://deb.debian.org/debian "+release+"-proposed-updates main contrib non-free\n" +
                            "deb http://deb.debian.org/debian-security/ "+release+"-security main contrib non-free\n" +
                            "deb http://deb.debian.org/debian/ "+release+"-backports main contrib non-free\n", "UTF-8");
                    File machineid = new File(Settings.outputDir + "/etc/machine-id");
                    FileUtils.writeStringToFile(arch, "b20ee23b73e04574bb24427aaf336ebd\n", "UTF-8");

                    File nsswitch = new File(Settings.outputDir + "/etc/nsswitch.conf");
                    FileUtils.writeStringToFile(arch, "# /etc/nsswitch.conf\n" +
                                                        "#\n" +
                                                        "# Example configuration of GNU Name Service Switch functionality.\n"+
                                                        "# If you have the `glibc-doc-reference' and `info' packages installed, try:\n"+
                                                        "# `info libc \"Name Service Switch\"' for information about this file.\n"+
                                                        "\n"+
                                                        "passwd:         files systemd\n"+
                                                        "group:          files systemd\n"+
                                                        "shadow:         files\n"+
                                                        "gshadow:        files\n"+
                                                        "\n"+
                                                        "hosts:          files mdns4_minimal [NOTFOUND=return] dns\n"+
                                                        "networks:       files\n"+
                                                        "\n"+
                                                        "protocols:      db files\n"+
                                                        "services:       db files\n"+
                                                        "ethers:         db files\n"+
                                                        "rpc:            db files\n"+
                                                        "\n"+
                                                        "netgroup:       nis\n", "UTF-8");

                    // now ready to install system
                    if (includeX11) {
                        DebianPackages.getPackage("xserver-xorg-core").force = true; // depends on udev, which crashes Boxedwine
                        DPkg.install("xserver-xorg-video-fbdev", downloadOnly); // since xserver-xorg-video-all is ignored in DPkg.install
                        DPkg.install("xorg", downloadOnly);
                        DPkg.install("xserver-xorg-input-evdev", downloadOnly);


                        //DPkg.install("gnome", downloadOnly);

                        DPkg.install("icewm", downloadOnly);
                        DPkg.install("xdemineur", downloadOnly);
                        DPkg.install("xfe", downloadOnly); // file manager

                        new File(Settings.outputDir + "/sys/class/devices/virtual/graphics/fb0").mkdirs();
                        new File(Settings.outputDir + "/sys/class/graphics").mkdirs();
                        new File(Settings.outputDir + "/home/username/.icewm").mkdirs();
                        File fb0Link = new File(Settings.outputDir + "/sys/class/graphics/fb0.link");
                        FileUtils.writeStringToFile(fb0Link, "../../devices/virtual/graphics/fb0", "UTF-8");
                        File xinitrc = new File(Settings.outputDir + "/home/username/.xinitrc");
                        FileUtils.writeStringToFile(xinitrc, "icewm", "UTF-8");
                        File xorgConf = new File(Settings.outputDir + "/etc/X11/xorg.conf");
                        FileUtils.writeStringToFile(xorgConf, Main.xConf, "UTF-8");

                        File menu = new File(Settings.outputDir + "/home/username/.icewm/menu");
                        FileUtils.writeStringToFile(menu, "menufile Games folder games", "UTF-8");
                        File games = new File(Settings.outputDir + "/home/username/.icewm/games");
                        FileUtils.writeStringToFile(games, "prog XDemineur /usr/share/pixmaps/xdemineur-icon.xpm /usr/games/xdemineur", "UTF-8");
                        File programs = new File(Settings.outputDir + "/home/username/.icewm/programs");
                        FileUtils.writeStringToFile(programs, "prog \"File Manager (Xfe)\" xfe xfe", "UTF-8");
                    }
                    // :TODO: add extra packages here
                    // for example

                }
                if (!includeX11) {
                    FileUtils.deleteDirectory(new File(Settings.outputDir+"/home"));
                }
            }
            FileUtils.deleteDirectory(new File(Settings.outputDir+"/usr/share/doc"));
            FileUtils.deleteDirectory(new File(Settings.outputDir+"/usr/share/man"));
            //FileUtils.deleteDirectory(new File(Settings.outputDir+"/var/cache/deb"));
            //FileUtils.deleteDirectory(new File(Settings.outputDir+"/usr/share/locale"));
            FileUtils.deleteDirectory(new File(Settings.outputDir+"/usr/share/help"));
            FileUtils.deleteDirectory(new File(Settings.outputDir+"/usr/share/sounds"));
            FileUtils.deleteDirectory(new File(Settings.outputDir+"/usr/lib/i386-linux-gnu/dri"));
            // using 7zip created a 3% smaller zip
            //ZipUtil.createZip(Settings.outputDir, Settings.finishedZip);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    static void touch(String path) throws Exception {
        File file = new File(Settings.outputDir + path);
        FileUtils.writeStringToFile(file, "", "UTF-8");
    }

    static String xConf = "Section \"ServerLayout\"\n" +
            "        Identifier     \"X.org Configured\"\n" +
            "        Screen      0  \"Screen0\" 0 0\n" +
            "        InputDevice    \"Mouse0\" \"CorePointer\"\n" +
            "        InputDevice    \"Keyboard0\" \"CoreKeyboard\"\n" +
            "EndSection\n" +
            "\n" +
            "Section \"InputDevice\"\n" +
            "        Identifier  \"Keyboard0\"\n" +
            "        Driver      \"evdev\"\n" +
            "\tOption      \"Device\" \"/dev/input/event4\"\n" +
            "EndSection\n" +
            "\n" +
            "Section \"InputDevice\"\n" +
            "        Identifier  \"Mouse0\"\n" +
            "        Driver      \"evdev\"\n" +
            "        Option      \"Protocol\" \"auto\"\n" +
            "        Option      \"Device\" \"/dev/input/event3\"\n" +
            "        Option      \"ZAxisMapping\" \"4 5 6 7\"\n" +
            "\tOption \"AccelerationNumerator\" \"1\"\n" +
            "\tOption \"AccelerationDenominator\" \"1\"\n" +
            "\tOption \"AccelerationThreshold\" \"0\"\n" +
            "EndSection\n" +
            "\n" +
            "Section \"Monitor\"\n" +
            "        Identifier   \"Monitor0\"\n" +
            "        VendorName   \"Monitor Vendor\"\n" +
            "        ModelName    \"Monitor Model\"\n" +
            "EndSection\n" +
            "\n" +
            "Section \"Device\"\n" +
            "        Identifier \"Card0\"\n" +
            "        Driver \"fbdev\"\n" +
            "        Option \"fbdev\" \"/dev/fb0\"\n" +
            "#    Option \"ShadowFB\" \"false\"\n" +
            "EndSection\n" +
            "\n" +
            "Section \"Screen\"\n" +
            "        Identifier \"Screen0\"\n" +
            "        Device     \"Card0\"\n" +
            "        Monitor    \"Monitor0\"\n" +
            "        SubSection \"Display\"\n" +
            "                Viewport   0 0\n" +
            "                Depth     1\n" +
            "        EndSubSection\n" +
            "        SubSection \"Display\"\n" +
            "                Viewport   0 0\n" +
            "                Depth     4\n" +
            "        EndSubSection\n" +
            "        SubSection \"Display\"\n" +
            "                Viewport   0 0\n" +
            "                Depth     8\n" +
            "        EndSubSection\n" +
            "        SubSection \"Display\"\n" +
            "                Viewport   0 0\n" +
            "                Depth     15\n" +
            "        EndSubSection\n" +
            "        SubSection \"Display\"\n" +
            "                Viewport   0 0\n" +
            "                Depth     16\n" +
            "        EndSubSection\n" +
            "        SubSection \"Display\"\n" +
            "                Viewport   0 0\n" +
            "                Depth     24\n" +
            "        EndSubSection\n" +
            "EndSection";
}
