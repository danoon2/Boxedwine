#include "boxedwine.h"
#include "../boxedwineui.h"

bool BoxedApp::load(BoxedContainer* container, const std::string& iniFilePath) {
    this->container = container;
    this->iniFilePath = iniFilePath;
    ConfigFile config(iniFilePath);

    this->name = config.readString("Name", "");
    this->link = config.readString("Link", "");
    this->cmd = config.readString("Cmd", "");
    this->icon = config.readString("Icon", "");
    this->path = config.readString("Path", "");
    this->resolution = config.readString("Resolution","");
    this->bpp = config.readInt("BPP",32);
    this->fullScreen = config.readBool("Fullscreen",false);
    this->glExt = config.readString("AllowedGlExt","");
    this->scale = config.readInt("Scale",100);
    int defaultScaleQuality = 0;
    this->scaleQuality = config.readInt("ScaleQuality",defaultScaleQuality);

    return this->name.length()>0 && (this->cmd.length()>0 || this->link.length()>0);
}

bool BoxedApp::saveApp() {
    if (this->iniFilePath.length()==0) {
        std::string appDir = GlobalSettings::getAppFolder(this->container);
        if (!Fs::doesNativePathExist(appDir)) {
            if (!Fs::makeNativeDirs(appDir)) {
                return false;
            }
        }
        for (int i=0;i<10000;i++) {
            std::string iniPath = appDir + Fs::nativePathSeperator + std::to_string(i) + ".ini";
            if (!Fs::doesNativePathExist(iniPath)) {
                this->iniFilePath = iniPath;
                break;
            }
        }
    }
    ConfigFile config(iniFilePath);
    config.writeString("Name", this->name);
    config.writeString("Link", this->link);
    config.writeString("Cmd", this->cmd);
    config.writeString("Icon", this->icon);
    config.writeString("Path", this->path);
    config.writeString("Resolution",this->resolution);
    config.writeInt("BPP",this->bpp);
    config.writeBool("Fullscreen",this->fullScreen);
    config.writeString("AllowedGlExt",this->glExt);
    config.writeInt("Scale",this->scale);
    config.writeInt("ScaleQuality",this->scaleQuality);

    config.saveChanges();
    return true;
}

void BoxedApp::launch() {
    std::string launchCmd;
    std::string args;

    GlobalSettings::startUpArgs = StartUpArgs();

    if (this->resolution.length()) {
        GlobalSettings::startUpArgs.setResolution(this->resolution);
    }
    if (this->bpp) {
        GlobalSettings::startUpArgs.setBpp(this->bpp);
    }
    if (this->fullScreen) {
        GlobalSettings::startUpArgs.setFullscreen();
    }
    if (this->glExt.length()) {
        GlobalSettings::startUpArgs.setAllowedGlExtension(this->glExt);
    }
    if (this->scale) {
        GlobalSettings::startUpArgs.setScale(this->scale);
    }
    if (this->scaleQuality) {
        GlobalSettings::startUpArgs.setScaleQuality(std::to_string(this->scaleQuality));
    }
    GlobalSettings::startUpArgs.addArg("/bin/wine");
    if (this->link.length()>0) {        
        GlobalSettings::startUpArgs.addArg("C:\\windows\\command\\start.exe");
        GlobalSettings::startUpArgs.addArg(this->link);
    } else {
        GlobalSettings::startUpArgs.addArg(this->cmd);
        for (U32 i=0;i<this->args.size();i++) {
            GlobalSettings::startUpArgs.addArg(this->args[i]);
        }
    }
    GlobalSettings::startUpArgs.setWorkingDir(this->path);

    this->container->launch();
}

std::string BoxedApp::getIcon() {
    if (!this->icon.length()) {
        // wrestool -x --output=. -t14 ~/.wine/drive_c/CATZ.FIR/CATZ.EXE
        // icotool -x CATZ.EXE_14_1.ico
        return "";
    }
    return GlobalSettings::getAppFolder(this->container) + Fs::nativePathSeperator + this->icon;
}

void BoxedApp::remove() {
    Fs::deleteNativeFile(this->iniFilePath);
    this->container->deleteApp(this);
    delete this;
}
