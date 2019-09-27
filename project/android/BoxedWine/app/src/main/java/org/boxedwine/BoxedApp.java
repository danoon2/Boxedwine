package org.boxedwine;

import java.io.File;
import java.util.Vector;

public class BoxedApp {
    BoxedApp() {
    }

    BoxedApp(String name, String path, String cmd, BoxedContainer container) {
        this.name = name;
        this.path = path;
        this.cmd = cmd;
        this.container = container;
    }

    BoxedApp(String name, String path, String link, String icon, BoxedContainer container) {
        this.name = name;
        this.path = path;
        this.link = link;
        this.icon = icon;
        this.container = container;
    }

    boolean load(BoxedContainer container, String iniFilePath) {
        this.container = container;
        this.iniFilePath = iniFilePath;
        BoxedFileConfig config = new BoxedFileConfig(iniFilePath);
        this.name = config.readString("Name", "");
        this.link = config.readString("Link", "");
        this.cmd = config.readString("Cmd", "");
        this.icon = config.readString("Icon", "");
        this.path = config.readString("Path", "");
        this.resolution = config.readString("Resolution","");
        this.bpp = config.readInt("BPP",32);
        this.fullScreen = config.readBool("Fullscreen",false);
        this.glExt = config.readString("AllowedGlExt","");
        this.scale = config.readInt("Scale",100);
        this.scaleQuality = config.readInt("ScaleQuality",0);
        return this.name.length()>0 && (this.cmd.length()>0 || this.link.length()>0);
    }

    String getName() {
        return this.name;
    }

    String getPath() {
        return this.path;
    }

    String getCmd() {
        return this.cmd;
    }

    void launch(BoxedWineUiActivity activity) {
        String launchCmd;
        Vector<String> boxedWineArgs = new Vector<>();
        Vector<String> appArgs = new Vector<String>();

        if (this.resolution!=null && this.resolution.length()!=0) {
            boxedWineArgs.addElement("-resolution");
            boxedWineArgs.addElement(this.resolution);
        }
        if (this.bpp!=0) {
            boxedWineArgs.addElement("-bpp");
            boxedWineArgs.addElement(String.valueOf(this.bpp));
        }
        if (this.fullScreen) {
            boxedWineArgs.addElement("-fullscreen ");
        }
        if (this.glExt!=null && this.glExt.length()!=0) {
            boxedWineArgs.addElement("-glext");
            boxedWineArgs.addElement("\""+this.glExt+"\"");
        }
        if (this.scale!=0) {
            boxedWineArgs.addElement("-scale");
            boxedWineArgs.addElement(String.valueOf(this.scale));
        }
        if (this.scaleQuality!=0) {
            boxedWineArgs.addElement("-scale_quality");
            boxedWineArgs.addElement(String.valueOf(this.scaleQuality));
        }
        if (this.link!=null && this.link.length()>0) {
            launchCmd = "C:\\windows\\command\\start.exe";
            appArgs.addElement("/Unix");
            appArgs.addElement(this.link);
        } else {
            launchCmd = this.cmd;
        }
        this.container.launchWine(activity, launchCmd, boxedWineArgs, this.path, appArgs);
    }

    String getIconPath(int size) {
        if (this.icon!=null && this.icon.length()!=0) {
            String root = GlobalSettings.getRootFolder(this.container)+File.separator+"home"+File.separator+"username"+File.separator+".local"+File.separator+"share"+File.separator+"icons"+File.separator+"hicolor"+File.separator;
            String path = root;
            if (size==32) {
                path+="32x32";
            } else if (size==48) {
                path+="48x48";
            } else {
                path+="32x32";
            }
            path=path+File.separator+"apps"+File.separator+this.icon;
            if (!new File(path).exists()) {
                if (size==32) {
                    return null;
                } else {
                    path=root+"32x32"+File.separator+"apps"+File.separator+this.icon;
                    if (!new File(path).exists()) {
                        return null;
                    }
                }
            }
            return path;
        } else if (this.cmd!=null && this.cmd.length()!=0) {
            // :TODO: extract from exe
        }
        return null;
    }

    BoxedContainer getContainer() {
        return this.container;
    }

    boolean isLink() {
        return link!=null && link.length()>0;
    }

    boolean saveApp() {
        if (this.iniFilePath==null || this.iniFilePath.length()==0) {
            File appDir = new File(GlobalSettings.getAppFolder(this.container));

            if (!appDir.exists()) {
                if (!appDir.mkdirs()) {
                    return false;
                }
            }
            for (int i=0;i<10000;i++) {
                String id = String.valueOf(i);
                String iniPath = appDir + File.separator + id + ".ini";
                if (!new File(iniPath).exists()) {
                    this.iniFilePath = iniPath;
                    break;
                }
            }
        }
        BoxedFileConfig config = new BoxedFileConfig(iniFilePath);
        config.writeString("Name", this.name);
        config.writeString("Link", this.link);
        config.writeString("Cmd", this.cmd);
        config.writeString("Icon", this.icon);
        config.writeString("Path", this.path);
        config.writeString("Resolution",this.resolution);
        config.writeInt("BPP",this.bpp);
        config.writeBool("Fullscreen",this.fullScreen);
        config.writeString("AllowedGlExt",this.glExt);
        config.writeInt("Scale",this.scale);
        config.writeInt("ScaleQuality",this.scaleQuality);

        config.save();
        return true;
    }

    void remove() {
        new File(this.iniFilePath).delete();
        this.container.deleteApp(this);
    }

    private String name;
    private String path;
    private String icon;
    private String link;
    private String cmd;

    // Boxedwine command line options
    private String resolution;
    private int bpp;
    private boolean fullScreen;
    private String glExt;
    private int scale;
    private int scaleQuality;

    BoxedContainer container;
    private String iniFilePath;
}
