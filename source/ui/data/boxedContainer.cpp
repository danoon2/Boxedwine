#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../io/fszip.h"

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
    if (!Fs::doesNativePathExist(dirPath)) {
        Fs::makeNativeDirs(dirPath);
    }
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

void BoxedContainer::launch(const std::vector<std::string>& args, const std::string& labelForWaitDlg) {
    GlobalSettings::startUpArgs = StartUpArgs();
    GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
    this->launch();
    GlobalSettings::startUpArgs.addArgs(args);
    GlobalSettings::startUpArgs.readyToLaunch = true;

    runOnMainUI([labelForWaitDlg]() {
        new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, labelForWaitDlg.c_str()));
        return false;
        });
}

void BoxedContainer::launch() {
    GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
    GlobalSettings::startUpArgs.addZip(GlobalSettings::getFileFromWineName(this->wineVersion));
    std::string root = GlobalSettings::getRootFolder(this);

    if (!Fs::doesNativePathExist(root)) {
        Fs::makeNativeDirs(root);
    }
    GlobalSettings::startUpArgs.setRoot(root);
    GlobalSettings::startUpArgs.mountInfo = this->mounts;
    GlobalSettings::startUpArgs.logPath = getLogPath();
}

void BoxedContainer::getNewApps(std::vector<BoxedApp>& apps, MountInfo* mount, const std::string& nativeDirectory) {
    //getNewDesktopLinkApps(apps);
    if (!mount && nativeDirectory.length()==0) {
        for (auto& m : this->mounts) {
            getNewExeApps(apps, &m, "");
        }
    }
    getNewExeApps(apps, mount, nativeDirectory);
}

bool compareApps(BoxedApp& a1, BoxedApp& a2)
{
    return (a1.getName() < a2.getName());
}

void BoxedContainer::getWineApps(std::vector<BoxedApp>& apps) {
    std::set<std::string> wineApps;
    wineApps.insert("taskmgr.exe");
    wineApps.insert("winecfg.exe");
    wineApps.insert("clock.exe");
    //wineApps.insert("winefile.exe");
    wineApps.insert("winemine.exe");
    //wineApps.insert("cmd.exe");
    wineApps.insert("explorer.exe");
    wineApps.insert("iexplore.exe");
    //wineApps.insert("hh.exe");
    wineApps.insert("notepad.exe");
    wineApps.insert("regedit.exe");
    wineApps.insert("wordpad.exe");
    //wineApps.insert("wmplayer.exe");
    FsZip::iterateFiles(GlobalSettings::getFileFromWineName(this->wineVersion), [this, &apps, &wineApps](const std::string& fileName) {
        if (fileName[fileName.length() - 1] != '/') {
            std::string name = Fs::getFileNameFromPath(fileName);
            std::string lname = name;
            stringToLower(lname);
            if (wineApps.count(lname)) {
                bool found = false;
                for (auto& a : apps) {
                    if (a.cmd == name) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    BoxedApp app;
                    app.container = this;
                    app.name = name;
                    app.path = Fs::getParentPath(fileName);
                    app.cmd = app.name;
                    apps.push_back(app);
                }
            }
        }
        });
    std::sort(apps.begin(), apps.end(), compareApps);
}

void BoxedContainer::getNewExeApps(std::vector<BoxedApp>& apps, MountInfo* mount, std::string nativeDirectory) {
    std::string root;
    std::string path;

    if (mount) {
        path = mount->nativePath;
        root = mount->nativePath;
    } else if (nativeDirectory.length()) {
        path = nativeDirectory;
        root = GlobalSettings::getRootFolder(this);
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

bool BoxedContainer::isGDI() {
    BoxedReg reg(this, false);
    std::string value;

    return (reg.readKey("Software\\Wine\\Direct3D", "DirectDrawRenderer", value) && value == "gdi");
}

void BoxedContainer::setGDI(bool gdi) {
    BoxedReg reg(this, false);
    reg.writeKey("Software\\Wine\\Direct3D", "DirectDrawRenderer", (gdi ? "gdi" : "opengl"));
    reg.save();
}

static const char szKey9x[] = "Software\\Microsoft\\Windows\\CurrentVersion";
static const char szKeyNT[] = "Software\\Microsoft\\Windows NT\\CurrentVersion";
static const char szKeyProdNT[] = "System\\CurrentControlSet\\Control\\ProductOptions";
static const char szKeyWindNT[] = "System\\CurrentControlSet\\Control\\Windows";
static const char szKeyEnvNT[] = "System\\CurrentControlSet\\Control\\Session Manager\\Environment";

std::string BoxedContainer::getWindowsVersion2() {
    BoxedReg reg(this, false);
    std::string value;
    if (reg.readKey("Software\\Wine", "Version", value)) {
        return value;
    }
    return "";
}

std::string BoxedContainer::getWindowsVersion() {    
    std::string userVer = getWindowsVersion2();
    if (userVer.length()) {
        for (auto& winVer : BoxedwineData::getWinVersions()) {
            if (winVer.szVersion == userVer) {
                return winVer.szDescription;
            }
        }
    }
    BoxedReg reg(this, true);    
    int platform = VER_PLATFORM_WIN32s, major = 0, minor = 0, build = 0;

    std::string ver;    
    std::string type;
    std::string build_str;
    std::string best;

    reg.readKey(szKeyNT, "CurrentVersion", ver);    

    if (ver.length()) {
        reg.readKey(szKeyNT, "CurrentBuildNumber", build_str);
        platform = VER_PLATFORM_WIN32_NT;
        build = std::stoi(build_str);
        reg.readKey(szKeyProdNT, "ProductType", type);
    } else {
        reg.readKey(szKey9x, "VersionNumber", ver);
        if (ver.length()) {
            platform = VER_PLATFORM_WIN32_WINDOWS;
        }
    }
    if (ver.length()) {
        if (!stringContains(ver, ".")) {
            major = std::stoi(ver);
        } else {
            std::vector<std::string> parts;
            stringSplit(parts, ver, '.');
            if (parts.size() >= 2) {
                major = std::stoi(parts[0]);
                minor = std::stoi(parts[1]);
            } else {
                return "";
            }
            if (parts.size() >= 3) {
                build = std::stoi(parts[2]);
            }
        }
    }
    for (auto& winVer : BoxedwineData::getWinVersions()) {
        if ((int)winVer.dwPlatformId != platform) continue;
        if ((int)winVer.dwMajorVersion != major) continue;
        if (type.length() && winVer.szProductType != type) continue;
        best = winVer.szDescription;
        if (((int)winVer.dwMinorVersion == minor) && ((int)winVer.dwBuildNumber == build)) {
            return winVer.szDescription;
        }
    }
    return best;
}

void BoxedContainer::setWindowsVersion(const BoxedWinVersion& version) {
    BoxedReg userReg(this, false);
    BoxedReg systemReg(this, true);
    
    std::string build = std::to_string(version.dwBuildNumber);
    if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        std::string ver = std::to_string(version.dwMajorVersion) + "." + std::to_string(version.dwMinorVersion);
        std::string product = "Microsoft " + version.szDescription;

        systemReg.writeKey(szKeyNT, "CurrentVersion", ver.c_str());
        systemReg.writeKey(szKeyNT, "CSDVersion", version.szCSDVersion.c_str());
        systemReg.writeKey(szKeyNT, "CurrentBuild", build.c_str());
        systemReg.writeKey(szKeyNT, "CurrentBuildNumber", build.c_str());
        systemReg.writeKey(szKeyNT, "ProductName", product.c_str());
        systemReg.writeKey(szKeyProdNT, "ProductType", version.szProductType.c_str());
        systemReg.writeKeyDword(szKeyWindNT, "CSDVersion", (U32)((version.wServicePackMajor << 8) | version.wServicePackMajor));
        systemReg.writeKey(szKeyEnvNT, "OS", "Windows_NT");

        systemReg.writeKey(szKey9x, "VersionNumber", NULL);
        systemReg.writeKey(szKey9x, "SubVersionNumber", NULL);
        systemReg.writeKey(szKey9x, "ProductName", NULL);
        userReg.writeKey("Software\\Wine", "Version", NULL);
    } else if (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        std::string ver = std::to_string(version.dwMajorVersion) + "." + std::to_string(version.dwMinorVersion) + "." + std::to_string(version.dwBuildNumber);
        std::string product = "Microsoft " + version.szDescription;

        systemReg.writeKey(szKeyNT, "CurrentVersion", NULL);
        systemReg.writeKey(szKeyNT, "CSDVersion", NULL);
        systemReg.writeKey(szKeyNT, "CurrentBuild", NULL);
        systemReg.writeKey(szKeyNT, "CurrentBuildNumber", NULL);
        systemReg.writeKey(szKeyNT, "ProductName", NULL);
        systemReg.writeKey(szKeyProdNT, "ProductType", NULL);
        systemReg.writeKey(szKeyWindNT, "CSDVersion", NULL);
        systemReg.writeKey(szKeyEnvNT, "OS", NULL);

        systemReg.writeKey(szKey9x, "VersionNumber", ver.c_str());
        systemReg.writeKey(szKey9x, "SubVersionNumber", version.szCSDVersion.c_str());
        systemReg.writeKey(szKey9x, "ProductName", product.c_str());
        userReg.writeKey("Software\\Wine", "Version", NULL);
    } else {
        systemReg.writeKey(szKeyNT, "CurrentVersion", NULL);
        systemReg.writeKey(szKeyNT, "CSDVersion", NULL);
        systemReg.writeKey(szKeyNT, "CurrentBuild", NULL);
        systemReg.writeKey(szKeyNT, "CurrentBuildNumber", NULL);
        systemReg.writeKey(szKeyNT, "ProductName", NULL);
        systemReg.writeKey(szKeyProdNT, "ProductType", NULL);
        systemReg.writeKey(szKeyWindNT, "CSDVersion", NULL);
        systemReg.writeKey(szKeyEnvNT, "OS", NULL);

        systemReg.writeKey(szKey9x, "VersionNumber", NULL);
        systemReg.writeKey(szKey9x, "SubVersionNumber", NULL);
        systemReg.writeKey(szKey9x, "ProductName", NULL);

        userReg.writeKey("Software\\Wine", "Version", version.szVersion.c_str());
    }
    userReg.save();
    systemReg.save();
}

std::string BoxedContainer::getLogPath() {
    return this->dirPath + Fs::nativePathSeperator + "lastLog.txt";    
}