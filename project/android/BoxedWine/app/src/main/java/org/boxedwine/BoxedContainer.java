package org.boxedwine;

import android.content.Intent;

import java.io.File;
import java.util.Vector;

public class BoxedContainer {
    BoxedContainer() {
        this.apps = new Vector<>();
    }
    static BoxedContainer createContainer(String dirPath, String name, String wineVersion) {
        BoxedContainer container = new BoxedContainer();
        container.name = name;
        container.wineVersion = wineVersion;
        container.dirPath = dirPath;
        container.saveContainer();

        String targetPath = dirPath + File.separator + "root";
        String timestampeFile = "home" + File.separator + "username" + File.separator + ".wine" + File.separator + ".update-timestamp";

        // :TODO: not sure why if I don't do this, wine will detect a change and update the .wine directory
        BoxedUtils.extractFileFromZip(new File(GlobalSettings.wineVersions.elementAt(0).filePath), timestampeFile, new File(targetPath + File.separator + timestampeFile));
        return container;
    }

    boolean load(String dirPath) {
        this.dirPath = dirPath;
        String iniFilePath = this.dirPath + File.separator + "container.ini";
        BoxedFileConfig config = new BoxedFileConfig(iniFilePath);
        this.name = config.readString("Name", "");
        this.wineVersion = config.readString("WineVersion", "");
        boolean result = this.name.length()>0 && this.wineVersion.length()>0;
        if (result) {
            this.loadApps();
        }
        return result;
    }

    boolean saveContainer() {
        String iniFilePath = this.dirPath + File.separator + "container.ini";
        BoxedFileConfig config = new BoxedFileConfig(iniFilePath);
        config.writeString("Name", this.name);
        config.writeString("WineVersion", this.wineVersion);
        config.save();
        return true;
    }

    void reload() {
        this.loadApps();
    }

    int getAppCount() {
        return this.apps.size();
    }

    BoxedApp getApp(int index) {
        if (index>=0 && index<this.apps.size()) {
            return this.apps.elementAt(index);
        } else {
            return null;
        }
    }

    void addApp(BoxedApp app) {
        this.apps.addElement(app);
    }

    void deleteApp(BoxedApp app) {
        this.apps.remove(app);
    }

    void deleteContainerFromFilesystem() {
        BoxedUtils.deleteRecursive(new File(this.dirPath));
    }

    void launch(BoxedWineUiActivity activity, String cmd, Vector<String> boxedWineArgs, String path, Vector<String> appArgs) {
        if (path!=null) {
            boxedWineArgs.addElement("-w");
            boxedWineArgs.addElement(path);
        }

        String root = GlobalSettings.getRootFolder(this);
        File rootFile = new File(root);
        if (!rootFile.exists()) {
            rootFile.mkdirs();
        }
        boxedWineArgs.addElement("-root");
        boxedWineArgs.addElement(root);

        boxedWineArgs.addElement("-zip");
        boxedWineArgs.addElement(GlobalSettings.getFileFromWineName(this.wineVersion));

        boxedWineArgs.addElement(cmd);
        boxedWineArgs.addAll(appArgs);
        Intent myIntent = new Intent(activity, BoxedWineActivity.class);
        myIntent.putExtra("args", boxedWineArgs.toArray(new String[boxedWineArgs.size()]));
        activity.startActivity(myIntent);
    }

    void launchWine(BoxedWineUiActivity activity, String cmd, Vector<String> boxedWineArgs, String path, Vector<String> appArgs) {
        appArgs.insertElementAt(cmd, 0);
        launch(activity, "/bin/wine", boxedWineArgs, path, appArgs);
    }

    String getName() {
        return this.name;
    }

    String getDir() {
        return this.dirPath;
    }

    String getWineVersion() {
        return this.wineVersion;
    }

    Vector<BoxedApp> getApps() {
        return this.apps;
    }

    void getNewApps(Vector<BoxedApp> apps) {
        getDesktopApps(apps);
        getExeApps(apps);
    }

    private void loadApps() {
        File appDirPath = new File(GlobalSettings.getAppFolder(this));

        this.apps.removeAllElements();
        if (appDirPath.isDirectory()) {
            for (File child : appDirPath.listFiles()) {
                if (child.getAbsolutePath().toLowerCase().endsWith(".ini")) {
                    String iniFilePath = child.getAbsolutePath();
                    BoxedApp app = new BoxedApp();
                    if (app.load(this, iniFilePath)) {
                        this.apps.addElement(app);
                    }
                }
            }
        }
    }

    private boolean doesAppExist(BoxedApp app) {
        for (BoxedApp a : this.apps) {
            String fullPath1 = a.getPath()+File.separator+a.getCmd();
            String fullPath2 = app.getPath()+File.separator+app.getCmd();
            if (fullPath1.equalsIgnoreCase(fullPath2)) {
                return true;
            }
        }
        return false;
    }

    private void getDesktopApps(Vector<BoxedApp> apps) {
        String path = this.dirPath + File.separator + "root" + File.separator + "home" + File.separator + "username" + File.separator + ".local" + File.separator + "share" + File.separator + "applications" + File.separator + "wine";
        Vector<File> results = new Vector<>();

        BoxedUtils.getAllFileRecursivlyThatHaveExt(new File(path), ".desktop", results);

        for (File file : results) {
            BoxedFileConfig config = new BoxedFileConfig(file.getAbsolutePath());

            String icon = config.readString("Desktop Entry/Icon", "");
            if (icon.length()>0) {
                icon+=".png";
            }
            String link = config.readString("Desktop Entry/Exec", "");
            if (link.length()==0) {
                continue;
            }
            int pos = link.indexOf("/Unix");
            if (pos>0) {
                link = link.substring(pos+6);
                link.replace("\\", "");
            }
            BoxedApp app = new BoxedApp(config.readString("Desktop Entry/Name", ""), config.readString("Desktop Entry/Path", ""), link, icon, this);

            if (!doesAppExist(app)) {
                apps.addElement(app);
            }
        }
    }

    private void getExeApps(Vector<BoxedApp> apps) {
        String root = GlobalSettings.getRootFolder(this);
        String path = root + File.separator + "home" + File.separator + "username" + File.separator + ".wine" + File.separator + "drive_c";
        Vector<File> results = new Vector<>();

        BoxedUtils.getAllFileRecursivlyThatHaveExt(new File(path), ".exe", results);

        for (File file : results) {
            BoxedApp app = new BoxedApp(file.getName(), file.getParentFile().getAbsolutePath(), file.getName(), this);

            if (!doesAppExist(app)) {
                apps.addElement(app);
            }
        }
    }

    private Vector<BoxedApp> apps;
    private String name;
    private String wineVersion;
    private String dirPath;
    private BoxedContainerProcess currentProcess;
}
