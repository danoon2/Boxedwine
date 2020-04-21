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
std::vector<DemoFile> GlobalSettings::demos;
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
std::vector<std::string> GlobalSettings::availableResolutions;
std::string GlobalSettings::defaultResolution;
int GlobalSettings::defaultScale;
int GlobalSettings::screenCx;
int GlobalSettings::screenCy;
float GlobalSettings::extraVerticalSpacing;
float GlobalSettings::fontScale;
bool GlobalSettings::iconFontsLoaded;

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
    GlobalSettings::defaultResolution = config.readString("DefaultResolution", "1024x768");
    GlobalSettings::defaultScale = config.readInt("DefaultScale", 100);
    GlobalSettings::fontScale = (float)config.readInt("FontScale", 100) / 100.0f;

    if (!Fs::doesNativePathExist(configFilePath)) {
        saveConfig();
    }    
    GlobalSettings::startUp();

    SDL_DisplayMode dm;    
    availableResolutions.push_back("640x480");
    availableResolutions.push_back("800x600");
    availableResolutions.push_back("1024x768");
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        GlobalSettings::screenCx = dm.w;
        GlobalSettings::screenCy = dm.h;
        if (dm.w>=3072) {
            std::string res = std::to_string(dm.w / 3) + "x" + std::to_string(dm.h / 3);
            availableResolutions.push_back(res.c_str());
        }
        if (dm.w>=2048) {
            std::string res = std::to_string(dm.w/2) + "x" + std::to_string(dm.h/2);
            availableResolutions.push_back(res.c_str());
        }
        std::string res = std::to_string(dm.w) + "x" + std::to_string(dm.h);
        availableResolutions.push_back(res.c_str());
    }
    GlobalSettings::extraVerticalSpacing = (float)GlobalSettings::scaleIntUI(5);
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
    config.writeString("DefaultResolution", GlobalSettings::defaultResolution);
    config.writeInt("DefaultScale", GlobalSettings::defaultScale);
    config.writeInt("FontScale", (int)(GlobalSettings::fontScale*100));
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

std::string GlobalSettings::getDemoFolder() {
    return GlobalSettings::dataFolderLocation + Fs::nativePathSeperator + "DemoCache";
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

float GlobalSettings::scaleFloatUIAndFont(float value) {
    return value * scale / SCALE_DENOMINATOR * fontScale;
}

void GlobalSettings::setScale(U32 scale) {
    GlobalSettings::scale = scale;
    UiSettings::ICON_SIZE = GlobalSettings::scaleIntUI(48);
    GlobalSettings::extraVerticalSpacing = (float)GlobalSettings::scaleIntUI(5);
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
    GlobalSettings::demos.clear();
    for (U32 i = 1; i < 1000; i++) {
        std::string name = config.readString("DemoName" + std::to_string(i), "");
        std::string icon = config.readString("DemoIcon" + std::to_string(i), "");
        std::string file = config.readString("DemoFile" + std::to_string(i), "");
        std::string exe = config.readString("DemoExe" + std::to_string(i), "");
        std::string exeOptions = config.readString("DemoExeOptions" + std::to_string(i), "");
        U32 fileSize = config.readInt("DemoFileSize" + std::to_string(i), 0);        
        if (name.length() && file.length()) {
            GlobalSettings::demos.push_back(DemoFile(name, icon, file, fileSize, exe, exeOptions));
        } else {
            break;
        }
    }
    runOnMainUI([]() { // might not have an OpenGL context while starting up
        for (auto& demo : GlobalSettings::getDemos()) {
            demo.buildIconTexture();
        }
        return false;
        });
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
        ::downloadFile("http://www.boxedwine.org/files.ini", fileLocation, [](U64 bytesCompleted) {
            }, NULL, errorMsg);
        GlobalSettings::loadFileList();
        GlobalSettings::filesListDownloading = false;
        runInBackgroundThread([]() {
            std::string errorMsg;

            for (auto& demo : GlobalSettings::getDemos()) {
                if (demo.iconPath.length()) {
                    size_t pos = demo.iconPath.rfind("/");
                    if (pos == std::string::npos) {
                        return; // :TODO: error msg?
                    }
                    if (!Fs::doesNativePathExist(GlobalSettings::getDemoFolder())) {
                        Fs::makeNativeDirs(GlobalSettings::getDemoFolder());
                    }
                    if (!Fs::doesNativePathExist(demo.localIconPath)) {
                        ::downloadFile(demo.iconPath, demo.localIconPath, [](U64 bytesCompleted) {
                            }, NULL, errorMsg);
                        runOnMainUI([&demo]() {
                            demo.buildIconTexture();
                            return false;
                            });
                    }
                }
            }
            });
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
    std::string awesomeFontPath = fontsPath + Fs::nativePathSeperator + "fontawesome-webfont.ttf";
    if (!Fs::doesNativePathExist(awesomeFontPath) && wineVersions.size() > 0) {
        FsZip::extractFileFromZip(wineVersions[0].filePath, "usr/share/fonts/truetype/fontawesome-webfont.ttf", fontsPath);
    }

    ImGuiIO& io = ImGui::GetIO();
    // first font added will be default
    if (Fs::doesNativePathExist(sansFontsPath) && !Fs::isNativePathDirectory(sansFontsPath)) {
        defaultFont = io.Fonts->AddFontFromFileTTF(sansFontsPath.c_str(), floor(scaleFloatUI(15.0f * GlobalSettings::fontScale)));
        if (Fs::doesNativePathExist(awesomeFontPath)) {
            static const ImWchar icons_ranges[] = { 0xf000, 0xf2b4, 0 }; // will not be copied by AddFont* so keep in scope.
            ImFontConfig config;
            config.MergeMode = true;
            if (io.Fonts->AddFontFromFileTTF(awesomeFontPath.c_str(), floor(scaleFloatUI(15.0f * GlobalSettings::fontScale)), &config, icons_ranges)) {
                GlobalSettings::iconFontsLoaded = true;
            }
        }
        mediumFont = io.Fonts->AddFontFromFileTTF(sansFontsPath.c_str(), floor(scaleFloatUI(20.0f * GlobalSettings::fontScale)));
        if (Fs::doesNativePathExist(awesomeFontPath)) {
            static const ImWchar icons_ranges[] = { 0xf000, 0xf2b4, 0 }; // will not be copied by AddFont* so keep in scope.
            ImFontConfig config;
            config.MergeMode = true;
            if (io.Fonts->AddFontFromFileTTF(awesomeFontPath.c_str(), floor(scaleFloatUI(20.0f * GlobalSettings::fontScale)), &config, icons_ranges)) {
                GlobalSettings::iconFontsLoaded = true;
            }
        }
        largeFont = io.Fonts->AddFontFromFileTTF(sansFontsPath.c_str(), floor(scaleFloatUI(25.0f * GlobalSettings::fontScale)));
        if (Fs::doesNativePathExist(awesomeFontPath)) {
            static const ImWchar icons_ranges[] = { 0xf000, 0xf2b4, 0 }; // will not be copied by AddFont* so keep in scope.
            ImFontConfig config;
            config.MergeMode = true;
            if (io.Fonts->AddFontFromFileTTF(awesomeFontPath.c_str(), floor(scaleFloatUI(25.0f * GlobalSettings::fontScale)), &config, icons_ranges)) {
                GlobalSettings::iconFontsLoaded = true;
            }
        }
    }

    if (Fs::doesNativePathExist(sansBoldFontsPath) && !Fs::isNativePathDirectory(sansBoldFontsPath)) {
        largeFontBold = io.Fonts->AddFontFromFileTTF(sansBoldFontsPath.c_str(), scaleFloatUI(24.0f * GlobalSettings::fontScale));
        if (Fs::doesNativePathExist(awesomeFontPath)) {
            static const ImWchar icons_ranges[] = { 0xf000, 0xf2b4, 0 }; // will not be copied by AddFont* so keep in scope.
            ImFontConfig config;
            config.MergeMode = true;
            if (io.Fonts->AddFontFromFileTTF(awesomeFontPath.c_str(), floor(scaleFloatUI(25.0f * GlobalSettings::fontScale)), &config, icons_ranges)) {
                GlobalSettings::iconFontsLoaded = true;
            }
        }
    }    
}

void GlobalSettings::downloadFile(const std::string& url, const std::string& filePath, const std::string& name, U32 sizeMB, std::function<void(bool)> onCompleted) {
    runOnMainUI([url, filePath, name, sizeMB, onCompleted]() {
        std::string parentPath = Fs::getNativeParentPath(filePath);
        if (!Fs::doesNativePathExist(parentPath)) {
            Fs::makeNativeDirs(parentPath);
        }
        new DownloadDlg(DOWNLOADDLG_TITLE, getTranslationWithFormat(DOWNLOADDLG_LABEL, true, name), url, filePath, [onCompleted](bool success) {
            runOnMainUI([success, onCompleted]() {
                GlobalSettings::reloadWineVersions();
                if (!GlobalSettings::defaultFont) {
                    GlobalSettings::restartUI = true;
                }
                onCompleted(success);
                return false;
                });
            }, ((U64)sizeMB) * 1024 * 1024);
        return false;
        });
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

void GlobalSettings::setTheme(const std::string& theme) { 
    GlobalSettings::theme = theme; 
    loadTheme();
}

void GlobalSettings::setFontScale(float scale) {
    GlobalSettings::fontScale = scale;
}

std::string GlobalSettings::createUniqueContainerPath(const std::string& name) {
    std::srand((U32)std::time(nullptr));
    while (true) {
        int r = std::rand();
        char tmp[10];
        SDL_itoa(r, tmp, 16);
        std::string result = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + name + "-"+tmp;
        if (!Fs::doesNativePathExist(result)) {
            return result;
        }
    }
}

DemoFile::DemoFile(const std::string& name, const std::string& iconPath, const std::string& filePath, U32 size, const std::string& exe, const std::string& exeOptions) : name(name), filePath(filePath), iconPath(iconPath), size(size), exe(exe), exeOptions(exeOptions), installed(false), iconTexture(NULL) {
    if (iconPath.length()) {
        size_t pos = iconPath.rfind("/");
        if (pos != std::string::npos) {
            localIconPath = GlobalSettings::getDemoFolder() + Fs::nativePathSeperator + iconPath.substr(pos + 1);                        
        }
    }
    if (filePath.length()) {
        size_t pos = filePath.rfind("/");
        if (pos != std::string::npos) {
            localFilePath = GlobalSettings::getDemoFolder() + Fs::nativePathSeperator + filePath.substr(pos + 1);
        }
    }
}

void DemoFile::buildIconTexture() {
    if (Fs::doesNativePathExist(localIconPath)) {
        int w = 0, h = 0;
        this->iconTexture = LoadTextureFromFile(localIconPath.c_str(), &w, &h);
    }
}

std::string DemoFile::getContainerNamePrefix() {
    std::string containerName = this->name;
    stringReplaceAll(containerName, " ", "_");
    return containerName;
}

void DemoFile::install() {

    GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
    GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
    GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());    
    std::string containerFilePath = GlobalSettings::createUniqueContainerPath(this->getContainerNamePrefix());
    BoxedContainer* container = BoxedContainer::createContainer(containerFilePath, this->name, GlobalSettings::getWineVersions()[0].name);
    BoxedwineData::addContainer(container);
    container->launch();
    GlobalSettings::startUpArgs.addArg(localFilePath);
    GlobalSettings::startUpArgs.readyToLaunch = true;
    container->saveContainer();

    runOnMainUI([this]() {
        new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, name.c_str()));
        return false;
        });

    std::string containerDir = container->getDir();
    std::string cmd = this->exe;
    std::string exeOptions = this->exeOptions;
    std::string appName = this->name;
    GlobalSettings::startUpArgs.runOnRestartUI = [appName, cmd, exeOptions, containerDir]() {
        runOnMainUI([appName, cmd, exeOptions, containerDir]() {
            BoxedContainer* container = BoxedwineData::getContainerByDir(containerDir);
            if (container) {
                std::vector<BoxedApp> items;
                container->getNewApps(items);
                for (auto& app : items) {
                    if (app.getCmd() == cmd) {
                        if (exeOptions.length()) {
                            std::vector<std::string> parts;
                            stringSplit(parts, exeOptions, ',');
                            for (auto& s : parts) {
                                stringTrim(s);
                                if (s == "GDI") {
                                    container->setGDI(true);
                                }
                            }
                            container->saveContainer();
                        }
                        app.setName(appName);
                        app.saveApp();
                        app.getContainer()->reload();
                        GlobalSettings::reloadApps();
                        return false;;
                    }
                }
                new AppChooserDlg(items, [container](BoxedApp* app) {
                    gotoView(VIEW_CONTAINERS, container->getDir(), app->getIniFilePath());
                    });
            }
            return false;
            });
    };
}
