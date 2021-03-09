#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../io/fszip.h"

bool BoxedApp::load(BoxedContainer* container, const std::string& iniFilePath) {
    this->container = container;
    this->iniFilePath = iniFilePath;
    ConfigFile config(iniFilePath);

    this->name = config.readString("Name", "");
    this->link = config.readString("Link", "");
    this->cmd = config.readString("Cmd", "");
    this->iconPath = config.readString("Icon", "");
    this->path = config.readString("Path", "");
    this->resolution = config.readString("Resolution","");
    this->bpp = config.readInt("BPP",32);
    this->fullScreen = config.readInt("Fullscreen",FULLSCREEN_NOTSET);
    this->vsync = config.readInt("VSync", VSYNC_NOT_SET);
    this->dpiAware = config.readBool("DpiAware", false);
    this->showWindowImmediately = config.readBool("ShowWindowImmediately", false);
    this->glExt = config.readString("AllowedGlExt","");
    this->scale = config.readInt("Scale",100);
    this->scaleQuality = config.readInt("ScaleQuality",0);    
    this->cpuAffinity = config.readInt("CpuAffinity", 0);
    this->pollRate = config.readInt("PollRate", -1);
    int i = 1;
    this->args.clear();
    while (true) {
        std::string key = "Arg";
        key += i;
        std::string arg = config.readString(key, "");
        if (arg.length() == 0) {
            break;
        }
        this->args.push_back(arg);
        i++;
    }
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
    config.writeString("Icon", this->iconPath);
    config.writeString("Path", this->path);
    config.writeString("Resolution",this->resolution);
    config.writeInt("BPP",this->bpp);
    config.writeInt("Fullscreen",this->fullScreen);
    config.writeInt("VSync", this->vsync);
    config.writeBool("DpiAware", this->dpiAware);
    config.writeBool("ShowWindowImmediately", this->showWindowImmediately);
    config.writeString("AllowedGlExt",this->glExt);
    config.writeInt("Scale",this->scale);
    config.writeInt("ScaleQuality",this->scaleQuality);
    config.writeInt("CpuAffinity",this->cpuAffinity);
    config.writeInt("PollRate", this->pollRate);

    for (int i = 0; i < (int)this->args.size(); i++) {
        std::string key = "Arg";
        key += (i + 1);
        config.writeString(key, this->args[i]);
    }
    config.saveChanges();
    return true;
}

void BoxedApp::launch() {
    std::string launchCmd;
    std::string args;

    GlobalSettings::startUpArgs = StartUpArgs();

    // set container defaults first, then override with app settings
    this->container->launch();

    if (this->resolution.length()) {
        GlobalSettings::startUpArgs.setResolution(this->resolution);
    } else {
        GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
    }
    if (this->bpp) {
        GlobalSettings::startUpArgs.setBpp(this->bpp);
    }
    if (this->fullScreen != FULLSCREEN_NOTSET) {
        GlobalSettings::startUpArgs.setFullscreen(this->fullScreen);
    }
    if (GlobalSettings::isDpiAware() && this->dpiAware) {
        GlobalSettings::startUpArgs.dpiAware = true;
    }
    if (this->showWindowImmediately) {
        GlobalSettings::startUpArgs.showWindowImmediately = true;
    }
    if (this->cpuAffinity) {
        GlobalSettings::startUpArgs.setCpuAffinity(this->cpuAffinity);
    }
    if (this->glExt.length()) {
        GlobalSettings::startUpArgs.setAllowedGlExtension(this->glExt);
    }
    if (this->scale) {
        GlobalSettings::startUpArgs.setScale(this->scale);
    } else {
        GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    }
    if (this->vsync == VSYNC_NOT_SET) {
        GlobalSettings::startUpArgs.setVsync(GlobalSettings::getDefaultVsync());
    } else {
        GlobalSettings::startUpArgs.setVsync(this->vsync);
    }
    if (this->scaleQuality) {
        GlobalSettings::startUpArgs.setScaleQuality(std::to_string(this->scaleQuality));
    }
    if (this->pollRate >= 0) {
        GlobalSettings::startUpArgs.pollRate = this->pollRate;
    }
    GlobalSettings::startUpArgs.title = this->name;

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
    GlobalSettings::startUpArgs.readyToLaunch = true;
}

BoxedAppIcon::BoxedAppIcon(const unsigned char* data, int width, int height) : width(width), height(height), data(data) {
    this->texture = std::make_shared<BoxedTexture>([this]() {
        return MakeRGBATexture(this->data, this->width, this->height);
        });
}

BoxedAppIcon::~BoxedAppIcon() {
    if (data) {
        delete[] data;
    }
}

const BoxedAppIcon* BoxedApp::getIconTexture(int iconSize) {
    if (iconSize==0) {
        iconSize = UiSettings::ICON_SIZE;
    }
    if (!this->iconsBySize.count(iconSize)) {
        int width = 0;
        int height = 0;
        const unsigned char* data = NULL;
        std::string nativeExePath = this->container->getNativePathForApp(*this);

        if (Fs::doesNativePathExist(nativeExePath)) {
            data = extractIconFromExe(this->container->getNativePathForApp(*this), iconSize, &width, &height);
        } else {
            std::string nativeDir = this->container->getDir() + Fs::nativePathSeperator + "tmp";
            FsZip::extractFileFromZip(GlobalSettings::getFileFromWineName(container->getWineVersion()), this->path.substr(1) + "/" + this->cmd, nativeDir);
            std::string nativePath = nativeDir + Fs::nativePathSeperator + this->cmd;
            data = extractIconFromExe(nativePath, iconSize, &width, &height);
            Fs::deleteNativeFile(nativePath);
        }
        if (data) {
            this->iconsBySize[iconSize] = new BoxedAppIcon(data, width, height);
        } else {
            this->iconsBySize[iconSize] = NULL;
        }
    }
    if (this->iconsBySize.count(iconSize)) {
        return this->iconsBySize[iconSize];
    }
    return NULL;
}

void BoxedApp::remove() {
    Fs::deleteNativeFile(this->iniFilePath);
    this->container->deleteApp(this);
    delete this;
}
