#include "boxedwine.h"
#include "../boxedwineui.h"

#include "../../io/fszip.h"

#include <SDL.h>

std::string GlobalSettings::dataFolderLocation;
std::vector<WineVersion> GlobalSettings::wineVersions;
int GlobalSettings::iconSize;
double GlobalSettings::scaleFactor;
StartUpArgs GlobalSettings::startUpArgs;
std::string GlobalSettings::exePath;

void GlobalSettings::init(int argc, const char **argv) {
    GlobalSettings::dataFolderLocation = SDL_GetPrefPath("", "Boxedwine");
    if (Fs::nativePathSeperator.length()==0) {
        Fs::nativePathSeperator = GlobalSettings::dataFolderLocation[GlobalSettings::dataFolderLocation.length()-1];
    }
    GlobalSettings::dataFolderLocation = GlobalSettings::dataFolderLocation.substr(0, GlobalSettings::dataFolderLocation.length()-1);
    GlobalSettings::exePath = Fs::getNativeParentPath(argv[0]);
    if (!Fs::doesNativePathExist(GlobalSettings::dataFolderLocation)) {
        Fs::makeNativeDirs(GlobalSettings::dataFolderLocation);
    }
    std::string configFilePath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "boxedwine.ini";
    ConfigFile config(configFilePath);
    GlobalSettings::dataFolderLocation = config.readString("DataFolder", GlobalSettings::dataFolderLocation);
    if (!Fs::doesNativePathExist(configFilePath)) {
        config.writeString("Version", "1");
        config.writeString("DataFolder", GlobalSettings::dataFolderLocation);
        config.saveChanges();
    }

    GlobalSettings::initWineVersions();
    std::string defaultContainerPath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "Containers" + Fs::nativePathSeperator + "Default";
    if (!Fs::doesNativePathExist(defaultContainerPath) && GlobalSettings::wineVersions.size()>0) {
        Fs::makeNativeDirs(defaultContainerPath);
        BoxedContainer* container = BoxedContainer::createContainer(defaultContainerPath, "Default", GlobalSettings::wineVersions[0].name);
        BoxedApp app("WineMine", "/home/username/.wine/drive_c/windows/system32", "winemine.exe", container);
        app.saveApp();
    }
}

std::string GlobalSettings::getFileFromWineName(const std::string& name) {
    for (auto& ver : GlobalSettings::wineVersions) {
        if (stringCaseInSensativeEquals(ver.name, name)) {
            return ver.filePath;
        }
    }
    return "";
}

void GlobalSettings::lookForFileSystems(const std::string& path) {
    Fs::iterateAllNativeFiles(path, true, false, [] (const std::string& filepath, bool isDir)->U32 {
        if (stringHasEnding(filepath, ".zip", true)) {
            std::string ver;
            if (FsZip::readFileFromZip(filepath, "wineVersion.txt", ver) && ver.length() && !GlobalSettings::getFileFromWineName(ver).length()) {
                GlobalSettings::wineVersions.push_back(WineVersion(ver, filepath));
            }
        }
        return 0;
    });
}

void GlobalSettings::initWineVersions() {
    GlobalSettings::lookForFileSystems(GlobalSettings::getFileSystemFolder());
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath);
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath + Fs::nativePathSeperator + "FileSystems");
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath + Fs::nativePathSeperator + ".." + Fs::nativePathSeperator + "FileSystems");
    std::sort(GlobalSettings::wineVersions.rbegin(), GlobalSettings::wineVersions.rend());
}

std::string GlobalSettings::getContainerFolder() {
    return GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "Containers";
}

std::string GlobalSettings::getFileSystemFolder() {
    return GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "FileSystems";
}

std::string GlobalSettings::getRootFolder(BoxedContainer* container) {
    return container->dirPath + Fs::nativePathSeperator + "root";
}

std::string GlobalSettings::getAppFolder(BoxedContainer* container) {
    return container->dirPath + Fs::nativePathSeperator + "apps";
}

double GlobalSettings::getScaleFactor() {    
    return scaleFactor;
}
