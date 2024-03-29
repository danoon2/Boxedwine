#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../io/fszip.h"

bool BoxedContainer::load(BString dirPath) {
    this->dirPath = dirPath;
    BString iniFilePath = this->dirPath ^ "container.ini";
    ConfigFile config(iniFilePath);
    this->name = config.readString(B("Name"), B(""));
    this->wineVersion = config.readString(B("WineVersion"), B(""));

    int i = 1;
    while (true) {
        BString mount = config.readString("Mount" + BString::valueOf(i), B(""));
        if (mount.length() == 0) {
            break;
        }
        std::vector<BString> parts;
        mount.split('|', parts);
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

BoxedContainer* BoxedContainer::createContainer(BString dirPath, BString name, BString wineVersion) {
    BoxedContainer* container = new BoxedContainer();
    container->name = name;
    container->wineVersion = wineVersion;
    container->dirPath = dirPath;
    if (!Fs::doesNativePathExist(dirPath)) {
        Fs::makeNativeDirs(dirPath);
    }
#ifdef __APPLE__
    // FBO's don't work with the Mac OpenGL code, so for DirectDraw games, instead of using OpenGL with FBO's which Wine does by default, tell Wine to take care of it in software
    container->setGDI(true);
#endif
    container->saveContainer();
    return container;
}

bool BoxedContainer::saveContainer() {
    BString iniFilePath = dirPath ^ "container.ini";
    ConfigFile config(iniFilePath);
    config.writeString(B("Name"), this->name);
    config.writeString(B("WineVersion"), this->wineVersion);
    for (int i = 0; i < (int)this->mounts.size(); i++) {
        config.writeString("Mount" + BString::valueOf(i + 1), this->mounts[i].localPath + "|" + this->mounts[i].nativePath);
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
    BString appDirPath = GlobalSettings::getAppFolder(this);

    this->apps.clear();

    Fs::iterateAllNativeFiles(appDirPath, true, false, [this] (BString filepath, bool isDir)->U32 {
        if (filepath.endsWith(".ini", true)) {
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

int BoxedContainer::getWineVersionAsNumber() {
    return getWineVersionAsNumber(this->wineVersion);
}

int BoxedContainer::getWineVersionAsNumber(BString wineVersion) {
    BString ver = wineVersion;
    ver = ver.substr(5);
    std::vector<BString> parts;
    ver.split('.', parts);
    if (parts.size() > 1) {
        BString major = parts[0];
        BString minor = parts[1];
        return atoi(major.c_str()) * 100 + atoi(minor.c_str());
    }
    return 0;
}

bool BoxedContainer::doesWineVersionExist() {
    return Fs::doesNativePathExist(GlobalSettings::getFileFromWineName(this->wineVersion));
}

void BoxedContainer::launch(const std::vector<BString>& args, BString labelForWaitDlg) {
    GlobalSettings::startUpArgs = StartUpArgs();
    GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    GlobalSettings::startUpArgs.setVsync(GlobalSettings::getDefaultVsync());
    GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
    this->launch();
    GlobalSettings::startUpArgs.addArgs(args);
    GlobalSettings::startUpArgs.readyToLaunch = true;

    runOnMainUI([labelForWaitDlg]() {
        new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, labelForWaitDlg));
        return false;
        });
}

void BoxedContainer::launch() {
    GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    GlobalSettings::startUpArgs.setVsync(GlobalSettings::getDefaultVsync());
    GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
    GlobalSettings::startUpArgs.addZip(GlobalSettings::getFileFromWineName(this->wineVersion));
    BString root = GlobalSettings::getRootFolder(this);

    if (!Fs::doesNativePathExist(root)) {
        Fs::makeNativeDirs(root);
    }
    GlobalSettings::startUpArgs.setRoot(root);
    GlobalSettings::startUpArgs.mountInfo = this->mounts;
    GlobalSettings::startUpArgs.logPath = getLogPath();
}

void BoxedContainer::getNewApps(std::vector<BoxedApp>& apps, MountInfo* mount, BString nativeDirectory) {
    //getNewDesktopLinkApps(apps);
    if (!mount && nativeDirectory.length()==0) {
        for (auto& m : this->mounts) {
            getNewExeApps(apps, &m, B(""));
        }
    }
    getNewExeApps(apps, mount, nativeDirectory);
}

bool compareApps(BoxedApp& a1, BoxedApp& a2)
{
    return (a1.getName() < a2.getName());
}

void BoxedContainer::getWineApps(std::vector<BoxedApp>& apps) {
    std::set<BString> wineApps;
    wineApps.insert(B("taskmgr.exe"));
    wineApps.insert(B("winecfg.exe"));
    wineApps.insert(B("clock.exe"));
    //wineApps.insert("winefile.exe");
    wineApps.insert(B("winemine.exe"));
    //wineApps.insert("cmd.exe");
    wineApps.insert(B("explorer.exe"));
    wineApps.insert(B("iexplore.exe"));
    //wineApps.insert("hh.exe");
    wineApps.insert(B("notepad.exe"));
    wineApps.insert(B("regedit.exe"));
    wineApps.insert(B("wordpad.exe"));
    //wineApps.insert("wmplayer.exe");
    FsZip::iterateFiles(GlobalSettings::getFileFromWineName(this->wineVersion), [this, &apps, &wineApps](BString fileName) {
        if (!fileName.endsWith('/')) {
            BString name = Fs::getFileNameFromPath(fileName);
            BString lname = name.toLowerCase();

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
                    app.path = "/"+Fs::getParentPath(fileName);
                    app.cmd = app.name;
                    apps.push_back(app);
                }
            }
        }
        });
    BoxedApp app;
    app.container = this;
    app.name = B("wineboot -u");
    app.path = B("/bin");
    app.cmd = B("wineboot");
    app.args.push_back(B("-u"));
    apps.push_back(app);
    std::sort(apps.begin(), apps.end(), compareApps);
}

void BoxedContainer::getNewExeApps(std::vector<BoxedApp>& apps, MountInfo* mount, BString nativeDirectory) {
    BString root;
    BString path;

    if (mount) {
        path = mount->nativePath;
        root = mount->nativePath;
    } else if (nativeDirectory.length()) {
        path = nativeDirectory;
        root = GlobalSettings::getRootFolder(this);
    } else {        
        root = GlobalSettings::getRootFolder(this);
        path = root ^ "home" ^ "username" ^ ".wine" ^ "drive_c";
    }
    BString winDir = B("drive_c") ^ "windows";
    Fs::iterateAllNativeFiles(path, true, true, [this, &apps, root, mount, winDir] (BString filepath, bool isDir)->U32 {
        if (filepath.endsWith(".exe", true) && !filepath.contains(winDir)) {
            BString localPath = filepath.substr(root.length());
            if (mount) {
                localPath = mount->getFullLocalPath() + localPath;
            }
            Fs::remoteNameToLocal(localPath);            
            BString name = Fs::getFileNameFromPath(localPath);

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
    BString root = GlobalSettings::getRootFolder(this);
    BString path = root ^ "home" ^ "username" ^ ".wine" ^ "drive_c" ^ "users" ^ "username" ^ "Start Menu";

    Fs::iterateAllNativeFiles(path, true, true, [this, &apps, root] (BString filepath, bool isDir)->U32 {
        if (filepath.endsWith(".lnk", true)) {
            BString localPath = filepath.substr(root.length());
            Fs::remoteNameToLocal(localPath);            
            BString name = Fs::getFileNameFromPath(localPath);

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

BString BoxedContainer::getNativePathForApp(const BoxedApp& app) {
    for (auto& mount : mounts) {
        BString path = mount.getFullLocalPath();
        if (app.path.startsWith(path)) {
            BString result = app.path;
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

BoxedApp* BoxedContainer::getAppByIniFile(BString iniFile) {
    for (auto& app : this->apps) {
        if (app->iniFilePath == iniFile) {
            return app;
        }
    }
    return NULL;
}

void BoxedContainer::setName(BString name) {
    this->name = name;
}

bool BoxedContainer::isGDI() {
    BoxedReg reg(this, false);
    BString value;

    return (reg.readKey("Software\\Wine\\Direct3D", "DirectDrawRenderer", value) && value == "gdi");
}

void BoxedContainer::setGDI(bool gdi) {
    BoxedReg reg(this, false);
    reg.writeKey("Software\\Wine\\Direct3D", "DirectDrawRenderer", (gdi ? "gdi" : "opengl"));
    reg.save();
}

BString BoxedContainer::getRenderer() {
    BoxedReg reg(this, false);
    BString value;

    reg.readKey("Software\\Wine\\Direct3D", "renderer", value);
    if (value.length() == 0) {
        value = B("gl");
    }
    return value;
}

void BoxedContainer::setRenderer(BString renderer) {
    BoxedReg reg(this, false);
    reg.writeKey("Software\\Wine\\Direct3D", "renderer", renderer.c_str());
    reg.save();
}

BString BoxedContainer::getMouseWarpOverride() {
    BoxedReg reg(this, false);
    BString value = B("enable");

    reg.readKey("Software\\Wine\\DirectInput", "MouseWarpOverride", value);
    return value;
}

void BoxedContainer::setMouseWarpOverride(BString value) {
    BoxedReg reg(this, false);
    reg.writeKey("Software\\Wine\\DirectInput", "MouseWarpOverride", value.c_str());
    reg.save();
}

static const char szKey9x[] = "Software\\Microsoft\\Windows\\CurrentVersion";
static const char szKeyNT[] = "Software\\Microsoft\\Windows NT\\CurrentVersion";
static const char szKeyProdNT[] = "System\\CurrentControlSet\\Control\\ProductOptions";
static const char szKeyWindNT[] = "System\\CurrentControlSet\\Control\\Windows";
static const char szKeyEnvNT[] = "System\\CurrentControlSet\\Control\\Session Manager\\Environment";

BString BoxedContainer::getWindowsVersion2() {
    BoxedReg reg(this, false);
    BString value;
    if (reg.readKey("Software\\Wine", "Version", value)) {
        return value;
    }
    return B("");
}

BString BoxedContainer::getWindowsVersion() {    
    BString userVer = getWindowsVersion2();
    if (userVer.length()) {
        for (auto& winVer : BoxedwineData::getWinVersions()) {
            if (winVer.szVersion == userVer) {
                return winVer.szDescription;
            }
        }
    }
    BoxedReg reg(this, true);    
    int platform = VER_PLATFORM_WIN32s, major = 0, minor = 0, build = 0;

    BString ver;    
    BString type;
    BString build_str;
    BString best;

    reg.readKey(szKeyNT, "CurrentVersion", ver);    

    if (ver.length()) {
        reg.readKey(szKeyNT, "CurrentBuildNumber", build_str);
        platform = VER_PLATFORM_WIN32_NT;
        build = build_str.toInt();
        reg.readKey(szKeyProdNT, "ProductType", type);
    } else {
        reg.readKey(szKey9x, "VersionNumber", ver);
        if (ver.length()) {
            platform = VER_PLATFORM_WIN32_WINDOWS;
        }
    }
    if (ver.length()) {
        if (!ver.contains('.')) {
            major = ver.toInt();
        } else {
            std::vector<BString> parts;
            ver.split('.', parts);
            if (parts.size() >= 2) {
                major = parts[0].toInt();
                minor = parts[1].toInt();
            } else {
                return B("");
            }
            if (parts.size() >= 3) {
                build = parts[2].toInt();
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
    
    BString build = BString::valueOf(version.dwBuildNumber);
    if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        BString ver = BString::valueOf(version.dwMajorVersion) + "." + BString::valueOf(version.dwMinorVersion);
        BString product = "Microsoft " + version.szDescription;

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
        BString ver = BString::valueOf(version.dwMajorVersion) + "." + BString::valueOf(version.dwMinorVersion) + "." + BString::valueOf(version.dwBuildNumber);
        BString product = "Microsoft " + version.szDescription;

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

BString BoxedContainer::getLogPath() {
    return this->dirPath ^ "lastLog.txt";    
}