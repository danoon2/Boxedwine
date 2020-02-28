#include "boxedwine.h"
#include "../boxedwineui.h"

bool BoxedContainer::load(const std::string& dirPath) {
    this->dirPath = dirPath;
    std::string iniFilePath = this->dirPath + Fs::nativePathSeperator + "container.ini";
    ConfigFile config(iniFilePath);
    this->name = config.readString("Name", "");
    this->wineVersion = config.readString("WineVersion", "");

    bool result = this->name.length()>0 && this->wineVersion.length()>0;
    if (result) {
        this->loadApps();
    }
    return result;
}

BoxedContainer::~BoxedContainer() {
    for (auto& app : this->apps) {
        delete app;
    }
}

BoxedContainer* BoxedContainer::createContainer(const std::string& dirPath, const std::string& name, const std::string& wineVersion) {
    BoxedContainer* container = new BoxedContainer();
    container->name = name;
    container->wineVersion = wineVersion;
    container->dirPath = dirPath;
    container->saveContainer();
    return container;
}

bool BoxedContainer::saveContainer() {
    std::string iniFilePath = dirPath + Fs::nativePathSeperator + "container.ini";
    ConfigFile config(iniFilePath);
    config.writeString("Name", this->name);
    config.writeString("WineVersion", this->wineVersion);
    config.saveChanges();
    return true;
}

void BoxedContainer::deleteContainerFromFilesystem() {
    Fs::deleteNativeDirAndAllFilesInDir(this->dirPath, true);
}

void BoxedContainer::reload() {
    this->loadApps();
}

void BoxedContainer::loadApps() {
    std::string appDirPath = GlobalSettings::getAppFolder(this);

    this->apps.clear();

    Fs::iterateAllNativeFiles(appDirPath, true, false, [this] (const std::string& filepath, bool isDir)->U32 {
        if (stringHasEnding(filepath, ".ini", true)) {            
            BoxedApp* app = new BoxedApp();
            if (app->load(this, filepath)) {
                this->apps.push_back(app);
            } else {
                delete app;
            }
        }
        return 0;
    });
}

void BoxedContainer::deleteApp(BoxedApp* app) {
    std::vector<BoxedApp*>::iterator position = std::find(this->apps.begin(), this->apps.end(), app);
    if (position != this->apps.end()) {
        this->apps.erase(position);
    }
}

void BoxedContainer::launch() {
    GlobalSettings::startUpArgs.addZip(GlobalSettings::getFileFromWineName(this->wineVersion));
    std::string root = GlobalSettings::getRootFolder(this);

    if (!Fs::doesNativePathExist(root)) {
        Fs::makeNativeDirs(root);
    }
    GlobalSettings::startUpArgs.setRoot(root);
}

void BoxedContainer::getNewApps(std::vector<BoxedApp>& apps) {
    getNewDesktopLinkApps(apps); 
    getNewExeApps(apps);
}

void BoxedContainer::getNewExeApps(std::vector<BoxedApp>& apps) {
    std::string root = GlobalSettings::getRootFolder(this);
    std::string path = root + Fs::nativePathSeperator + "home" + Fs::nativePathSeperator + "username" + Fs::nativePathSeperator + ".wine" + Fs::nativePathSeperator + "drive_c";

    Fs::iterateAllNativeFiles(path, true, true, [this, &apps, root] (const std::string& filepath, bool isDir)->U32 {
        if (stringHasEnding(filepath, ".exe", true)) {            
            std::string localPath = filepath.substr(root.length());
            Fs::localNameToRemote(localPath);            
            std::string name = Fs::getFileNameFromPath(localPath);

            bool found = false;
            for (auto& a : this->apps) {
                if (a->cmd==name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                BoxedApp app;
                app.container = this;
                app.name = name;
                app.path = localPath;
                app.cmd = app.name;
                apps.push_back(app);
            }
        }
        return 0;
    });
}

void BoxedContainer::getNewDesktopLinkApps(std::vector<BoxedApp>& apps) {
    std::string root = GlobalSettings::getRootFolder(this);
    std::string path = root + Fs::nativePathSeperator + "home" + Fs::nativePathSeperator + "username" + Fs::nativePathSeperator + ".wine" + Fs::nativePathSeperator + "drive_c" + Fs::nativePathSeperator + "users" + Fs::nativePathSeperator + "username" + Fs::nativePathSeperator + "Start Menu";

    Fs::iterateAllNativeFiles(path, true, true, [this, &apps, root] (const std::string& filepath, bool isDir)->U32 {
        if (stringHasEnding(filepath, ".lnk", true)) {            
            std::string localPath = filepath.substr(root.length());
            Fs::localNameToRemote(localPath);            
            std::string name = Fs::getFileNameFromPath(localPath);

            // :TODO: parse link to get name
            bool found = false;
            for (auto& a : this->apps) {
                if (a->link==name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                BoxedApp app;
                app.container = this;
                app.name = name;
                app.path = localPath;
                app.link = app.name;
                apps.push_back(app);
            }
        }
        return 0;
    });
}

void BoxedContainer::updateCachedSize() {
    this->cachedSize = getReadableSize(Fs::getNativeDirectorySize(this->getDir(), true));
}