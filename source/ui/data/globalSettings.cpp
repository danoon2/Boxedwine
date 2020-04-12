#include "boxedwine.h"
#include "../boxedwineui.h"

#include "../../io/fszip.h"
#include "../../util/networkutils.h"
#include "../../util/threadutils.h"

#include <sys/stat.h>

#include <SDL.h>

#define SLOW_FRAME_DELAY 1000
#define FAST_FRAME_DELAY 1

std::string GlobalSettings::dataFolderLocation;
std::vector<WineVersion> GlobalSettings::wineVersions;
std::vector<WineVersion> GlobalSettings::availableWineVersions;
int GlobalSettings::iconSize;
StartUpArgs GlobalSettings::startUpArgs;
std::string GlobalSettings::exePath;
std::string GlobalSettings::theme;
std::string GlobalSettings::configFilePath;
ImFont* GlobalSettings::largeFontBold;
ImFont* GlobalSettings::largeFont;
ImFont* GlobalSettings::mediumFont;
ImFont* GlobalSettings::defaultFont;
ImFont* GlobalSettings::sectionTitleFont;
U32 GlobalSettings::scale=1000;
bool GlobalSettings::filesListDownloading;
bool GlobalSettings::restartUI;
U32 GlobalSettings::frameDelayMillies = SLOW_FRAME_DELAY;
U32 GlobalSettings::fastFrameRateCount = 0;
U64 GlobalSettings::lastFrameDelayChange = 0;

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
    GlobalSettings::configFilePath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "boxedwine.ini";
    ConfigFile config(GlobalSettings::configFilePath);
    GlobalSettings::dataFolderLocation = config.readString("DataFolder", GlobalSettings::dataFolderLocation);
    stringReplaceAll(GlobalSettings::dataFolderLocation, "//", "/");
    stringReplaceAll(GlobalSettings::dataFolderLocation, "\\\\", "\\");
    GlobalSettings::theme = config.readString("Theme", "Dark");
    if (!Fs::doesNativePathExist(configFilePath)) {
        saveConfig();
    }    
    GlobalSettings::startUp();
}

void GlobalSettings::startUp() {
    GlobalSettings::initWineVersions();
    std::string containersPath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "Containers";
    if (!Fs::doesNativePathExist(containersPath) && GlobalSettings::wineVersions.size() > 0) {
        std::string defaultContainerPath = containersPath + Fs::nativePathSeperator + "Default";
        Fs::makeNativeDirs(defaultContainerPath);
        BoxedContainer* container = BoxedContainer::createContainer(defaultContainerPath, "Default", GlobalSettings::wineVersions[0].name);
        std::string s = Fs::nativePathSeperator;
        std::string nativePath = GlobalSettings::getRootFolder(container) + s + "home" + s + "username" + s + ".wine" + s + "drive_c" + s + "windows" + s + "system32";
        Fs::makeNativeDirs(nativePath);
        // so that icon detection works
        FsZip::extractFileFromZip(wineVersions[0].filePath, "home/username/.wine/drive_c/windows/system32/winemine.exe", nativePath);
        BoxedApp app("WineMine", "/home/username/.wine/drive_c/windows/system32", "winemine.exe", container);
        app.saveApp();
    }
    GlobalSettings::loadFileList();
    GlobalSettings::checkFileListForUpdate();
}

void GlobalSettings::saveConfig() {
    ConfigFile config(GlobalSettings::configFilePath);
    config.writeString("Version", "1");
    config.writeString("DataFolder", GlobalSettings::dataFolderLocation);
    config.writeString("Theme", GlobalSettings::theme);
    config.saveChanges();
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
            std::string wineVersion;
            if (FsZip::readFileFromZip(filepath, "wineVersion.txt", wineVersion) && wineVersion.length() && !GlobalSettings::getFileFromWineName(wineVersion).length()) {
                std::string fsVersion;
                FsZip::readFileFromZip(filepath, "version.txt", fsVersion);
                GlobalSettings::wineVersions.push_back(WineVersion(wineVersion, fsVersion, filepath));
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

void GlobalSettings::reloadWineVersions() {
    GlobalSettings::wineVersions.clear();
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

U32 GlobalSettings::scaleIntUI(U32 value) {
    return value * scale / SCALE_DENOMINATOR;
}

float GlobalSettings::scaleFloatUI(float value) {
    return value * scale / SCALE_DENOMINATOR;
}

void GlobalSettings::setScale(U32 scale) {
    GlobalSettings::scale = scale;
    UiSettings::ICON_SIZE = GlobalSettings::scaleIntUI(48);
}

void GlobalSettings::loadTheme() {
    if (GlobalSettings::theme == "Classic") {        
        ImGui::StyleColorsClassic();
        ImGui::GetStyle().FrameBorderSize = 0.0f;
    } else if (GlobalSettings::theme == "Light") {
        ImGui::StyleColorsLight();
        ImGui::GetStyle().FrameBorderSize = 1.0f;
    } else {
        ImGui::StyleColorsDark();        
        ImGui::GetStyle().FrameBorderSize = 0.0f;
    }
}

void loadApps();
void GlobalSettings::reloadApps() {
    loadApps();
}

void GlobalSettings::loadFileList() {
    std::string filesConfigPath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "files.ini";
    ConfigFile config(filesConfigPath);
    GlobalSettings::availableWineVersions.clear();
    for (U32 i = 1; i < 1000; i++) {
        std::string name = config.readString("WineFsName" + std::to_string(i), "");
        std::string ver = config.readString("WineFsVer" + std::to_string(i), "");
        std::string file = config.readString("WineFsFile" + std::to_string(i), "");
        U32 fileSize = config.readInt("WineFsFileSize" + std::to_string(i), 0);
        std::string changes = config.readString("WineFsChangeMsg" + std::to_string(i), "");
        if (name.length() && ver.length() && file.length()) {
            GlobalSettings::availableWineVersions.push_back(WineVersion(name, ver, file, fileSize, changes));
        } else {
            break;
        }
    }
}

bool GlobalSettings::checkFileListForUpdate() {
    std::string filesConfigPath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "files.ini";

    PLATFORM_STAT_STRUCT buf;
    if (PLATFORM_STAT(filesConfigPath.c_str(), &buf) == 0) {
        U64 t = (U64)buf.st_mtime; // time as seconds
        U64 currentTime = KSystem::getSystemTimeAsMicroSeconds() / 1000000l;
        if (t + 24 * 60 * 60 < currentTime) {
            updateFileList(filesConfigPath);
            return true;
        }
    } else {
        updateFileList(filesConfigPath);
        return true;
    }    
    return false;
}

bool GlobalSettings::isFilesListDownloading() {
    return GlobalSettings::filesListDownloading;
}

void GlobalSettings::updateFileList(const std::string& fileLocation) {
    runInBackgroundThread([fileLocation]() {
        std::string errorMsg;
        GlobalSettings::filesListDownloading = true;
        downloadFile("http://www.boxedwine.org/files.ini", fileLocation, [](U32 percentDone) {
            }, NULL, errorMsg);
        GlobalSettings::loadFileList();
        GlobalSettings::filesListDownloading = false;
    }       
    );
}

void GlobalSettings::loadFonts() {
    std::string fontsPath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "Fonts";
    if (!Fs::doesNativePathExist(fontsPath)) {
        Fs::makeNativeDirs(fontsPath);
    }
    std::string sansBoldFontsPath = fontsPath + Fs::nativePathSeperator + "LiberationSans-Bold.ttf";
    if (!Fs::doesNativePathExist(sansBoldFontsPath) && wineVersions.size() > 0) {
        FsZip::extractFileFromZip(wineVersions[0].filePath, "usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf", fontsPath);
    }
    std::string sansFontsPath = fontsPath + Fs::nativePathSeperator + "LiberationSans-Regular.ttf";
    if (!Fs::doesNativePathExist(sansFontsPath) && wineVersions.size() > 0) {
        FsZip::extractFileFromZip(wineVersions[0].filePath, "usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", fontsPath);
    }

    ImGuiIO& io = ImGui::GetIO();
    // first font added will be default
    if (Fs::doesNativePathExist(sansFontsPath) && !Fs::isNativePathDirectory(sansFontsPath)) {
        defaultFont = io.Fonts->AddFontFromFileTTF(sansFontsPath.c_str(), scaleFloatUI(15.0f));
        mediumFont = io.Fonts->AddFontFromFileTTF(sansFontsPath.c_str(), scaleFloatUI(20.0f));
        largeFont = io.Fonts->AddFontFromFileTTF(sansFontsPath.c_str(), scaleFloatUI(25.0f));
    }

    if (Fs::doesNativePathExist(sansBoldFontsPath) && !Fs::isNativePathDirectory(sansBoldFontsPath)) {
        largeFontBold = io.Fonts->AddFontFromFileTTF(sansBoldFontsPath.c_str(), scaleFloatUI(24.0f));
    }    
}

void GlobalSettings::downloadWine(const WineVersion& version, std::function<void(bool)> onCompleted) {
    runOnMainUI([&version, onCompleted]() {
        size_t pos = version.filePath.rfind("/");
        if (pos == std::string::npos) {
            return false; // :TODO: error msg?
        }
        std::string fileName = version.filePath.substr(pos + 1);
        std::string filePath = GlobalSettings::getFileSystemFolder() + Fs::nativePathSeperator + fileName;
        if (!Fs::doesNativePathExist(GlobalSettings::getFileSystemFolder())) {
            Fs::makeNativeDirs(GlobalSettings::getFileSystemFolder());
        }
        new DownloadDlg(DOWNLOADDLG_TITLE, getTranslationWithFormat(DOWNLOADDLG_LABEL, true, version.name), version.filePath, filePath, [onCompleted](bool success) {
            runOnMainUI([success, onCompleted]() {
                GlobalSettings::reloadWineVersions();
                if (!GlobalSettings::defaultFont) {
                    GlobalSettings::restartUI = true;
                }
                onCompleted(success);
                return false;
                });
            }, ((U64)(version.size))*1024*1024);
        return false;
        });
}

void GlobalSettings::useFastFrameRate(bool useFast) {
    if (useFast) {
        if (GlobalSettings::fastFrameRateCount == 0) {
            GlobalSettings::frameDelayMillies = FAST_FRAME_DELAY;
        }
        GlobalSettings::fastFrameRateCount++;
    } else {
        GlobalSettings::fastFrameRateCount--;
        if (GlobalSettings::fastFrameRateCount == 0) {
            GlobalSettings::frameDelayMillies = SLOW_FRAME_DELAY;
        }
    }
    GlobalSettings::lastFrameDelayChange = KSystem::getSystemTimeAsMicroSeconds();
}

void GlobalSettings::updateLastFrameDelayChange() {
    GlobalSettings::lastFrameDelayChange = KSystem::getSystemTimeAsMicroSeconds();
}

U32 GlobalSettings::getFrameDelayMillies() { 
    if (lastFrameDelayChange + 1000000l < KSystem::getSystemTimeAsMicroSeconds()) {
        return GlobalSettings::frameDelayMillies;
    }
    return FAST_FRAME_DELAY;
}