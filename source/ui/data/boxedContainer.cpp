#include "boxedwine.h"
#include "../boxedwineui.h"

bool BoxedContainer::load(const std::string& dirPath) {
    this->dirPath = dirPath;
    std::string iniFilePath = this->dirPath + Fs::nativePathSeperator + "container.ini";
    ConfigFile config(iniFilePath);
    this->name = config.readString("Name", "");
    this->wineVersion = config.readString("WineVersion", "");

    int i = 1;
    while (true) {
        std::string mount = config.readString("Mount" + std::to_string(i), "");
        if (mount.length() == 0) {
            break;
        }
        std::vector<std::string> parts;
        stringSplit(parts, mount, '|');
        if (parts.size() != 2) {
            kwarn("Failed to parse container %s mount command %s", this->name.c_str(), mount.c_str());
            break;
        }
        this->mounts.push_back(MountInfo(parts[0], parts[1], parts[0].length() == 1));
        i++;
    }
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
    for (int i = 0; i < (int)this->mounts.size(); i++) {
        config.writeString("Mount" + std::to_string(i + 1), this->mounts[i].localPath + "|" + this->mounts[i].nativePath);
    }
    config.saveChanges();
    return true;
}

void BoxedContainer::deleteContainerFromFilesystem() {
    Fs::deleteNativeDirAndAllFilesInDir(this->dirPath);
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

bool BoxedContainer::doesWineVersionExist() {
    return Fs::doesNativePathExist(GlobalSettings::getFileFromWineName(this->wineVersion));
}

void BoxedContainer::launch() {
    GlobalSettings::startUpArgs.addZip(GlobalSettings::getFileFromWineName(this->wineVersion));
    std::string root = GlobalSettings::getRootFolder(this);

    if (!Fs::doesNativePathExist(root)) {
        Fs::makeNativeDirs(root);
    }
    GlobalSettings::startUpArgs.setRoot(root);
    GlobalSettings::startUpArgs.mountInfo = this->mounts;
}

void BoxedContainer::getNewApps(std::vector<BoxedApp>& apps) {
    getNewDesktopLinkApps(apps);
    for (auto& mount : this->mounts) {
        getNewExeApps(apps, &mount);
    }
    getNewExeApps(apps, NULL);
}

void BoxedContainer::getNewExeApps(std::vector<BoxedApp>& apps, MountInfo* mount) {
    std::string root;
    std::string path;

    if (mount) {
        path = mount->nativePath;
        root = mount->nativePath;
    } else {        
        root = GlobalSettings::getRootFolder(this);
        path = root + Fs::nativePathSeperator + "home" + Fs::nativePathSeperator + "username" + Fs::nativePathSeperator + ".wine" + Fs::nativePathSeperator + "drive_c";
    }
    Fs::iterateAllNativeFiles(path, true, true, [this, &apps, root, mount] (const std::string& filepath, bool isDir)->U32 {
        if (stringHasEnding(filepath, ".exe", true) && !stringContains(filepath, "drive_c"+Fs::nativePathSeperator+"windows")) {            
            std::string localPath = filepath.substr(root.length());
            if (mount) {
                localPath = mount->getFullLocalPath() + localPath;
            }
            Fs::remoteNameToLocal(localPath);            
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
                app.path = Fs::getParentPath(localPath);
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
            Fs::remoteNameToLocal(localPath);            
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
                app.path = Fs::getParentPath(localPath);
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

std::string BoxedContainer::getNativePathForApp(const BoxedApp& app) {
    for (auto& mount : mounts) {
        std::string path = mount.getFullLocalPath();
        if (stringStartsWith(app.path, path)) {
            std::string result = app.path;
            return mount.nativePath + Fs::nativeFromLocal(app.path.substr(path.length())+"/"+app.cmd);
        }
    }
    return GlobalSettings::getRootFolder(this) + Fs::nativeFromLocal(app.path + "/" + app.cmd);
}

bool BoxedContainer::addNewMount(const MountInfo& mountInfo) {
    for (auto& mount : this->mounts) {
        if (mount.localPath == mountInfo.localPath) {
            return false;
        }
    }
    this->mounts.push_back(mountInfo);
    return true;
}

BoxedApp* BoxedContainer::getAppByIniFile(const std::string& iniFile) {
    for (auto& app : this->apps) {
        if (app->iniFilePath == iniFile) {
            return app;
        }
    }
    return NULL;
}

void BoxedContainer::setName(const std::string& name) {
    this->name = name;
}