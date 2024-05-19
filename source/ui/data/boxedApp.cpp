#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../io/fszip.h"

bool BoxedApp::load(BoxedContainer* container, BString iniFilePath) {
    this->container = container;
    this->iniFilePath = iniFilePath;
    ConfigFile config(iniFilePath);

    this->name = config.readString(B("Name"), B(""));
    this->link = config.readString(B("Link"), B(""));
    this->cmd = config.readString(B("Cmd"), B(""));
    this->iconPath = config.readString(B("Icon"), B(""));
    this->path = config.readString(B("Path"), B(""));
    this->resolution = config.readString(B("Resolution"),B(""));
    this->bpp = config.readInt(B("BPP"),32);
    this->fullScreen = config.readInt(B("Fullscreen"),FULLSCREEN_NOTSET);
    this->vsync = config.readInt(B("VSync"), VSYNC_NOT_SET);
    this->dpiAware = config.readBool(B("DpiAware"), false);
    this->showWindowImmediately = config.readBool(B("ShowWindowImmediately"), false);
    this->autoRefresh = config.readBool(B("AutoRefresh"), false);
    this->glExt = config.readString(B("AllowedGlExt"),B(""));
    this->scale = config.readInt(B("Scale"),100);
    this->scaleQuality = config.readInt(B("ScaleQuality"),0);    
    this->cpuAffinity = config.readInt(B("CpuAffinity"), 0);
    this->pollRate = config.readInt(B("PollRate"), 0);
    this->skipFramesFPS = config.readInt(B("SkipFramesFPS"), 0);
    this->openGlType = config.readInt(B("OpenGL"), OPENGL_TYPE_NOT_SET);
    this->isWine = config.readBool(B("IsWine"), true);
    this->uid = config.readInt(B("uid"), -1);
    this->euid = config.readInt(B("euid"), -1);

    int i = 1;
    this->args.clear();
    while (true) {
        BString key = B("Arg");
        key += i;
        BString arg = config.readString(key, B(""));
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
        BString appDir = GlobalSettings::getAppFolder(this->container);
        if (!Fs::doesNativePathExist(appDir)) {
            if (!Fs::makeNativeDirs(appDir)) {
                return false;
            }
        }
        for (int i=0;i<10000;i++) {
            BString iniPath = appDir ^ BString::valueOf(i) + ".ini";
            if (!Fs::doesNativePathExist(iniPath)) {
                this->iniFilePath = iniPath;
                break;
            }
        }
    }
    ConfigFile config(iniFilePath);
    config.writeString(B("Name"), this->name);
    config.writeString(B("Link"), this->link);
    config.writeString(B("Cmd"), this->cmd);
    config.writeString(B("Icon"), this->iconPath);
    config.writeString(B("Path"), this->path);
    config.writeString(B("Resolution"),this->resolution);
    config.writeInt(B("BPP"),this->bpp);
    config.writeInt(B("Fullscreen"),this->fullScreen);
    config.writeInt(B("VSync"), this->vsync);
    config.writeBool(B("DpiAware"), this->dpiAware);
    config.writeBool(B("ShowWindowImmediately"), this->showWindowImmediately);
    config.writeBool(B("AutoRefresh"), this->autoRefresh);
    config.writeString(B("AllowedGlExt"),this->glExt);
    config.writeInt(B("Scale"),this->scale);
    config.writeInt(B("ScaleQuality"),this->scaleQuality);
    config.writeInt(B("CpuAffinity"),this->cpuAffinity);
    config.writeInt(B("PollRate"), this->pollRate);
    config.writeInt(B("SkipFramesFPS"), this->skipFramesFPS);
    config.writeInt(B("OpenGL"), this->openGlType);
    config.writeBool(B("IsWine"), this->isWine);
    config.writeInt(B("uid"), this->uid);
    config.writeInt(B("euid"), this->euid);

    BString key;
    for (int i = 0; i < (int)this->args.size(); i++) {
        key = "Arg";
        key += (i + 1);
        config.writeString(key, this->args[i]);
    }
    config.saveChanges();
    return true;
}

void BoxedApp::createAutomation() {
    BString path = GlobalSettings::getAutomationFolder(this->container);
    if (!Fs::doesNativePathExist(path)) {
        Fs::makeNativeDirs(path);
    }
    this->launch();
    GlobalSettings::startUpArgs.recordAutomation = GlobalSettings::getAutomationFolder(this->container);
}

void BoxedApp::runAutomation() {
    this->launch();
    GlobalSettings::startUpArgs.runAutomation = GlobalSettings::getAutomationFolder(this->container);
}

bool BoxedApp::hasAutomation() {
#ifdef BOXEDWINE_RECORDER
    BString path = GlobalSettings::getAutomationFolder(this->container) ^ RECORDER_SCRIPT;
    return Fs::doesNativePathExist(path);
#else
    return false;
#endif
}

void BoxedApp::launch() {
    BString launchCmd;
    BString args;

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
    if (this->autoRefresh) {
        GlobalSettings::startUpArgs.envValues.push_back(B("BOXED_DD_AUTOREFRESH=1"));
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
    if (this->vsync == (U32)VSYNC_NOT_SET) {
        GlobalSettings::startUpArgs.setVsync(GlobalSettings::getDefaultVsync());
    } else {
        GlobalSettings::startUpArgs.setVsync(this->vsync);
    }
    if (this->scaleQuality) {
        GlobalSettings::startUpArgs.setScaleQuality(BString::valueOf(this->scaleQuality));
    }
    if (this->pollRate >= 0) {
        GlobalSettings::startUpArgs.pollRate = this->pollRate;
    }
    if (this->skipFramesFPS) {
        GlobalSettings::startUpArgs.skipFrameFPS = this->skipFramesFPS;
    }
    if (this->uid != -1) {
        GlobalSettings::startUpArgs.userId = this->uid;
    }
    if (this->euid != -1) {
        GlobalSettings::startUpArgs.effectiveUserId = this->euid;
    }
    GlobalSettings::startUpArgs.title = this->name;

    if (isWine) {
        GlobalSettings::startUpArgs.addArg(B("/bin/wine"));
        if (this->link.length() > 0) {
            GlobalSettings::startUpArgs.addArg(B("C:\\windows\\command\\start.exe"));
            GlobalSettings::startUpArgs.addArg(this->link);
        } else {
            GlobalSettings::startUpArgs.addArg(this->cmd);
            for (U32 i = 0; i < this->args.size(); i++) {
                GlobalSettings::startUpArgs.addArg(this->args[i]);
            }
        }
    } else {
        GlobalSettings::startUpArgs.addArg(this->cmd);
        for (U32 i = 0; i < this->args.size(); i++) {
            GlobalSettings::startUpArgs.addArg(this->args[i]);
        }
    }
    GlobalSettings::startUpArgs.setWorkingDir(this->path);    
    GlobalSettings::startUpArgs.openGlType = this->openGlType;
    if (GlobalSettings::startUpArgs.openGlType == OPENGL_TYPE_NOT_SET) {
        GlobalSettings::startUpArgs.openGlType = GlobalSettings::getDefaultOpenGL();
    }
    GlobalSettings::startUpArgs.readyToLaunch = true;
}

BoxedAppIcon::BoxedAppIcon(std::shared_ptr<U8[]> data, int width, int height) : width(width), height(height), data(data) {
    this->texture = std::make_shared<BoxedTexture>([this]() {
        return MakeRGBATexture(this->data.get(), this->width, this->height);
        });
}

BoxedAppIcon::~BoxedAppIcon() {
}

const BoxedAppIcon* BoxedApp::getIconTexture(int iconSize) {
    if (iconSize==0) {
        iconSize = UiSettings::ICON_SIZE;
    }
    if (!this->iconsBySize.contains(iconSize)) {
        int width = 0;
        int height = 0;
        std::shared_ptr<U8[]> data;
        BString nativeExePath = this->container->getNativePathForApp(*this);

        if (Fs::doesNativePathExist(nativeExePath)) {
            data = extractIconFromExe(this->container->getNativePathForApp(*this), iconSize, &width, &height);
        } else {
            BString nativeDir = this->container->getDir() ^ "tmp";
            std::shared_ptr<FileSystemZip> fs = container->getFileSystem().lock();
            if (fs) {
                FsZip::extractFileFromZip(fs->filePath, this->path.substr(1) + "/" + this->cmd, nativeDir);
                BString nativePath = nativeDir ^ this->cmd;
                data = extractIconFromExe(nativePath, iconSize, &width, &height);
                Fs::deleteNativeFile(nativePath);
            }
        }
        if (!data) {
            if (Fs::doesNativePathExist(this->iconPath)) {
                data.reset(LoadImageFromFile(this->iconPath.c_str(), &width, &height));
            }
        }
        if (data) {
            this->iconsBySize.set(iconSize, new BoxedAppIcon(data, width, height));
        } else {
            this->iconsBySize.set(iconSize, nullptr);
        }
    }
    return this->iconsBySize[iconSize];
}

void BoxedApp::remove() {
    Fs::deleteNativeFile(this->iniFilePath);
    this->container->deleteApp(this);
    delete this;
}
