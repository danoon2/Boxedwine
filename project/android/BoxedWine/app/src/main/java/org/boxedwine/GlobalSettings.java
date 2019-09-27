package org.boxedwine;

import java.io.File;
import java.util.Vector;

public class GlobalSettings {
    static private void lookForFileSystems(File dir) {
        if (dir.exists()) {
            for (File f : dir.listFiles()) {
                if (f.getName().toLowerCase().endsWith(".zip")) {
                    byte[] data = BoxedUtils.readFileFromZip(f, "wineVersion.txt");
                    if (data!=null) {
                        String wineName = new String(data);
                        boolean found = false;

                        for (WineVersion v : GlobalSettings.wineVersions){
                            if (v.name == wineName) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            GlobalSettings.wineVersions.addElement(new WineVersion(wineName, f.getAbsolutePath()));
                        }
                    }
                }
            }
        }
    }

    static void initWineVersions() {
        wineVersions = new Vector<>();
        lookForFileSystems(new File(GlobalSettings.getFileSystemFolder()));
    }

    static String getFileFromWineName(String name) {
        for (WineVersion ver : GlobalSettings.wineVersions) {
            if (ver.name.equalsIgnoreCase(name)) {
                return ver.filePath;
            }
        }
        if (GlobalSettings.wineVersions.size()!=0) {
            return GlobalSettings.wineVersions.elementAt(0).filePath;
        }
        return null;
    }

    static String dataFolderLocation;

    static String getContainerFolder() {
        return GlobalSettings.dataFolderLocation + File.separator + "Containers";
    }

    static String getFileSystemFolder() {
        return GlobalSettings.dataFolderLocation + File.separator + "FileSystems";
    }

    static String getAppFolder(BoxedContainer container) {
        return container.getDir() + File.separator + "apps";
    }

    static String getRootFolder(BoxedContainer container) {
        return container.getDir() + File.separator + "root";
    }

    static Vector<WineVersion> wineVersions;
}
