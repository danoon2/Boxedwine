package org.boxedwine;

import org.apache.commons.io.FileUtils;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Vector;

/**
 * Created by James on 11/13/2021.
 */
public class DPkg {
    static void install(String name, boolean downloadOnly) throws Exception {
        HashSet<String> ignoreDependency = new HashSet<>();
        ignoreDependency.add("libdrm-intel1");
        ignoreDependency.add("libdrm-nouveau2");
        ignoreDependency.add("libdrm-amdgpu1");
        ignoreDependency.add("libdrm-radeon1");
        ignoreDependency.add("libgl1-mesa-dri");
        ignoreDependency.add("xserver-xorg-video-vmware");
        ignoreDependency.add("xserver-xorg-video-ati");
        ignoreDependency.add("xserver-xorg-video-radeon");
        ignoreDependency.add("xserver-xorg-video-all");
        ignoreDependency.add("xfonts-100dpi");
        ignoreDependency.add("xfonts-75dpi");
        DebianPackages.getPackage("libglx-mesa0").force = true; // needs libgl1-mesa-drv which is ignored
        DebianPackages.getPackage("xorg").force = true; // depends on libgl1-mesa-dri

        install(name, ignoreDependency, false, downloadOnly);
    }
    static void install(String name, HashSet<String> ignoreDependency, boolean force, boolean downloadOnly) throws Exception {
        if (ignoreDependency.contains(name)) {
            return;
        }
        ignoreDependency.add(name);
        HashMap<String, DebianPackage> depends = new HashMap<>();
        DebianPackage pkg = DebianPackages.getPackage(name);
        if (pkg == null) {
            System.out.println("Package not found: "+name);
            return;
        }
        pkg.getDepends(ignoreDependency, depends);

        if (!force) {
            for (DebianPackage depend : depends.values()) {
                if (downloadOnly) {
                    if (!depend.downloaded) {
                        install(depend.name, ignoreDependency, false, downloadOnly);
                    }
                } else {
                    if (!depend.installed) {
                        install(depend.name, ignoreDependency, false, downloadOnly);
                    }
                }
            }
        }
        File src = FileCache.getFile(pkg.url);
        File dst = new File(Settings.outputDir+File.separator+"var"+File.separator+"cache"+File.separator+"deb"+File.separator+src.getName());
        if (!dst.exists()) {
            FileUtils.copyFile(src, dst);
        }
        if (downloadOnly) {
            pkg.downloaded = true;
            return;
        }
        Vector<String> cmd = new Vector<>();
        if (force || pkg.force) {
            cmd.add("--force-all");
        }
        cmd.add("-i");
        cmd.add("/var/cache/deb/"+src.getName());
        run(cmd);
        pkg.installed = true;
    }
    static void run(Vector<String> cmds) throws Exception {
        try {
            ProcessBuilder builder = new ProcessBuilder(Settings.boxedwinePath, "-root", Settings.outputDir.getAbsolutePath(), "-uid", "0", "-env", "DEBIAN_FRONTEND=noninteractive", "/usr/bin/dpkg");
            builder.command().addAll(cmds);
            Process process = builder.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line = null;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                    if (line.contains("Errors were encountered") || line.contains("error processing") || line.contains("command not found")) {
                        throw new Exception(line);
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
