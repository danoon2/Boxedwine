#include "boxedwine.h"
#include "../boxedwineui.h"

#include "../../io/fszip.h"
#include "../../util/networkutils.h"
#include "../../util/threadutils.h"
#include "../../../lib/pugixml/src/pugixml.hpp"
#include "knativesystem.h"
#include "ksystem.h"
#include "crc.h"

#include <sys/stat.h>

#define SLOW_FRAME_DELAY 1000
#define FAST_FRAME_DELAY 1

BString GlobalSettings::dataFolderLocation;
std::vector<WineVersion> GlobalSettings::wineVersions;
std::vector<WineVersion> GlobalSettings::winetricksVersions;
std::vector<WineVersion> GlobalSettings::availableWineVersions;
std::vector<WineVersion> GlobalSettings::availableWinetricksVersions;
std::vector<WineVersion> GlobalSettings::availableWineDependencies;
std::vector<AppFile> GlobalSettings::demos;
std::vector<AppFile> GlobalSettings::components;
int GlobalSettings::iconSize;
StartUpArgs GlobalSettings::startUpArgs;
BString GlobalSettings::exePath;
BString GlobalSettings::exeFilePath;
BString GlobalSettings::theme;
BString GlobalSettings::configFilePath;
ImFont* GlobalSettings::largeFontBold;
ImFont* GlobalSettings::largeFont;
ImFont* GlobalSettings::mediumFont;
ImFont* GlobalSettings::defaultFont;
ImFont* GlobalSettings::sectionTitleFont;
U32 GlobalSettings::scale=1000;
bool GlobalSettings::filesListDownloading;
bool GlobalSettings::restartUI;
bool GlobalSettings::reinit;
U32 GlobalSettings::frameDelayMillies = SLOW_FRAME_DELAY;
U32 GlobalSettings::fastFrameRateCount = 0;
U64 GlobalSettings::lastFrameDelayChange = 0;
std::vector<BString> GlobalSettings::availableResolutions;
BString GlobalSettings::defaultResolution;
int GlobalSettings::defaultScale;
int GlobalSettings::defaultVsync;
int GlobalSettings::screenCx;
int GlobalSettings::screenCy;
float GlobalSettings::extraVerticalSpacing;
float GlobalSettings::fontScale;
bool GlobalSettings::iconFontsLoaded;
std::vector<BString> GlobalSettings::fileUrls;
int GlobalSettings::lastScreenCx;
int GlobalSettings::lastScreenCy;
int GlobalSettings::lastScreenX;
int GlobalSettings::lastScreenY;
U32 GlobalSettings::defaultOpenGL = OPENGL_TYPE_SDL;
bool GlobalSettings::enabledAutomation = false;

void GlobalSettings::init(int argc, const char **argv) {
    GlobalSettings::largeFontBold = NULL;
    GlobalSettings::largeFont = NULL;
    GlobalSettings::mediumFont = NULL;
    GlobalSettings::defaultFont = NULL;
    GlobalSettings::sectionTitleFont = NULL;
    GlobalSettings::iconFontsLoaded = false;
    GlobalSettings::availableWineVersions.clear();
    GlobalSettings::availableWinetricksVersions.clear();
    GlobalSettings::demos.clear();
    GlobalSettings::wineVersions.clear();
    GlobalSettings::winetricksVersions.clear();
    GlobalSettings::components.clear();

    GlobalSettings::dataFolderLocation = KNativeSystem::getLocalDirectory();
    if (Fs::nativePathSeperator.length()==0) {
        Fs::nativePathSeperator = GlobalSettings::dataFolderLocation.charAt(GlobalSettings::dataFolderLocation.length()-1);
    }
    GlobalSettings::dataFolderLocation = GlobalSettings::dataFolderLocation.substr(0, GlobalSettings::dataFolderLocation.length()-1);
    GlobalSettings::exePath = Fs::getNativeParentPath(BString::copy(argv[0]));
    GlobalSettings::exeFilePath = BString::copy(argv[0]);
    if (!Fs::doesNativePathExist(GlobalSettings::dataFolderLocation)) {
        Fs::makeNativeDirs(GlobalSettings::dataFolderLocation);
    }
    GlobalSettings::configFilePath = GlobalSettings::dataFolderLocation ^ "boxedwine.ini";
    ConfigFile config(GlobalSettings::configFilePath);
    GlobalSettings::dataFolderLocation = config.readString(B("DataFolder"), GlobalSettings::dataFolderLocation);
    GlobalSettings::dataFolderLocation = GlobalSettings::dataFolderLocation.replace("//", "/");
    GlobalSettings::dataFolderLocation = GlobalSettings::dataFolderLocation.replace("\\\\", "\\");
    GlobalSettings::theme = config.readString(B("Theme"), B("Dark"));
    GlobalSettings::defaultResolution = config.readString(B("DefaultResolution"), B("1024x768"));
    GlobalSettings::defaultScale = config.readInt(B("DefaultScale"), 100);
    GlobalSettings::defaultVsync = config.readInt(B("DefaultVsync"), VSYNC_DEFAULT);
    GlobalSettings::fontScale = (float)config.readInt(B("FontScale"), 100) / 100.0f;
    GlobalSettings::fileUrls.push_back(config.readString(B("FilesURL"), B("http://www.boxedwine.org/v/" BOXEDWINE_VERSION_STR "/files.xml")));
    for (int i = 1; i < 10; i++) {
        BString name = "FilesURL" + BString::valueOf(i + 1);
        BString url = config.readString(name, B(""));
        if (url.length()) {
            GlobalSettings::fileUrls.push_back(url);
        }
    }
    GlobalSettings::lastScreenCx = config.readInt(B("WindowWidth"), 0);
    GlobalSettings::lastScreenCy = config.readInt(B("WindowHeight"), 0);
    GlobalSettings::lastScreenX = config.readInt(B("WindowX"), 0);
    GlobalSettings::lastScreenY = config.readInt(B("WindowY"), 0);
    GlobalSettings::defaultOpenGL = config.readInt(B("OpenGL"), OPENGL_TYPE_SDL);
    GlobalSettings::enabledAutomation = config.readBool(B("EnableAutomation"), false);

    if (!Fs::doesNativePathExist(configFilePath)) {
        saveConfig();
    }    
    GlobalSettings::startUp();

    U32 width = 0;
    U32 height = 0;
    availableResolutions.push_back(B("640x480"));
    availableResolutions.push_back(B("800x600"));
    availableResolutions.push_back(B("1024x768"));
    if (KNativeSystem::getScreenDimensions(&width, &height)) {
        GlobalSettings::screenCx = width;
        GlobalSettings::screenCy = height;
        if (width>=3072) {
            BString res = BString::valueOf(width / 3) + "x" + BString::valueOf(height / 3);
            availableResolutions.push_back(res);
        }
        if (width >=2048) {
            BString res = BString::valueOf(width /2) + "x" + BString::valueOf(height /2);
            availableResolutions.push_back(res);
        }
        BString res = BString::valueOf(width) + "x" + BString::valueOf(height);
        availableResolutions.push_back(res);
    }
    GlobalSettings::extraVerticalSpacing = (float)GlobalSettings::scaleIntUI(5);
}

void GlobalSettings::startUp() {
    GlobalSettings::initWineVersions();
    BString containersPath = GlobalSettings::dataFolderLocation ^ B("Containers");
    if (!Fs::doesNativePathExist(containersPath) && GlobalSettings::wineVersions.size() > 0) {
        BString defaultContainerPath = containersPath ^ B("Default");
        Fs::makeNativeDirs(defaultContainerPath);
        BoxedContainer* container = BoxedContainer::createContainer(defaultContainerPath, B("Default"), GlobalSettings::wineVersions[0].name);
        BString nativePath = GlobalSettings::getRootFolder(container) ^ "home" ^ "username" ^ ".wine" ^ "drive_c" ^ "windows" ^ "system32";
        Fs::makeNativeDirs(nativePath);
        BoxedApp app(B("WineMine"), B("/opt/wine/lib/wine/fakedlls"), B("winemine.exe"), container);
        app.saveApp();
    }
    GlobalSettings::loadFileLists();
    GlobalSettings::checkFileListForUpdate();
}

void GlobalSettings::saveScreenSize(int x, int y, int cx, int cy) {
    GlobalSettings::lastScreenCx = cx;
    GlobalSettings::lastScreenCy = cy;
    GlobalSettings::lastScreenX = x;
    GlobalSettings::lastScreenY = y;
    GlobalSettings::saveConfig();
}

void GlobalSettings::saveConfig() {
    ConfigFile config(GlobalSettings::configFilePath);
    config.writeString(B("Version"), B("1"));
    config.writeString(B("DataFolder"), GlobalSettings::dataFolderLocation);
    config.writeString(B("Theme"), GlobalSettings::theme);
    config.writeString(B("DefaultResolution"), GlobalSettings::defaultResolution);
    config.writeInt(B("DefaultScale"), GlobalSettings::defaultScale);
    config.writeInt(B("DefaultVsync"), GlobalSettings::defaultVsync);
    config.writeInt(B("FontScale"), (int)(GlobalSettings::fontScale*100));
    // don't save this so that it can be updated each release, but if someone wants to override it by entering a value they can
    //config.writeString("FilesURL", GlobalSettings::filesUrl);
    config.writeInt(B("WindowWidth"), GlobalSettings::lastScreenCx);
    config.writeInt(B("WindowHeight"), GlobalSettings::lastScreenCy);
    config.writeInt(B("WindowX"), GlobalSettings::lastScreenX);
    config.writeInt(B("WindowY"), GlobalSettings::lastScreenY);
    config.writeInt(B("OpenGL"), GlobalSettings::defaultOpenGL);
    config.writeBool(B("EnableAutomation"), GlobalSettings::enabledAutomation);
    config.saveChanges();
}

BString GlobalSettings::getFileFromWineName(BString name) {
    for (auto& ver : GlobalSettings::wineVersions) {
        if (ver.name.compareTo(name, true) == 0) {
            return ver.filePath;
        }
    }
    return B("");
}

WineVersion* GlobalSettings::getInstalledWineFromName(BString name) {
    for (auto& ver : GlobalSettings::wineVersions) {
        if (ver.name.compareTo(name, true) == 0) {
            return &ver;
        }
    }
    return NULL;
}

WineVersion* GlobalSettings::getAvailableWineFromName(BString name) {
    for (auto& ver : GlobalSettings::availableWineVersions) {
        if (ver.name.compareTo(name, true) == 0) {
            return &ver;
        }
    }
    return NULL;
}

void GlobalSettings::lookForFileSystems(BString path) {
    Fs::iterateAllNativeFiles(path, true, false, [] (BString filepath, bool isDir)->U32 {
        if (filepath.endsWith(".zip", true)) {
            BString wineVersion;
            if (FsZip::readFileFromZip(filepath, B("wineVersion.txt"), wineVersion) && wineVersion.length() && !GlobalSettings::getFileFromWineName(wineVersion).length()) {
                BString fsVersion;
                BString depend;
                FsZip::readFileFromZip(filepath, B("version.txt"), fsVersion);
                fsVersion = fsVersion.trim();
                FsZip::readFileFromZip(filepath, B("depends.txt"), depend);
                depend = depend.trim();
                GlobalSettings::wineVersions.push_back(WineVersion(wineVersion, fsVersion, filepath, B(""), depend));
            }
            BString winetricksVersion;
            if (FsZip::readFileFromZip(filepath, B("winetricksVersion.txt"), winetricksVersion) && winetricksVersion.length()) {
                BString fsVersion;
                BString depend;
                BString dlls;
                BString fonts;
                FsZip::readFileFromZip(filepath, B("version.txt"), fsVersion);
                fsVersion = fsVersion.trim();
                FsZip::readFileFromZip(filepath, B("depends.txt"), depend);
                depend = depend.trim();
                FsZip::readFileFromZip(filepath, B("dlls.txt"), dlls);
                FsZip::readFileFromZip(filepath, B("fonts.txt"), fonts);
                WineVersion v = WineVersion(winetricksVersion, fsVersion, filepath, B(""), depend);
                v.data = fonts;
                v.data2 = dlls;
                GlobalSettings::winetricksVersions.push_back(v);
            }
        }        
        return 0;
    });
}

void GlobalSettings::initWineVersions() {
    GlobalSettings::lookForFileSystems(GlobalSettings::getFileSystemFolder());
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath);
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath ^ "FileSystems");
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath ^ ".." ^ "FileSystems");
    std::sort(GlobalSettings::wineVersions.rbegin(), GlobalSettings::wineVersions.rend());
}

void GlobalSettings::reloadWineVersions() {
    GlobalSettings::wineVersions.clear();
    GlobalSettings::lookForFileSystems(GlobalSettings::getFileSystemFolder());
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath);
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath ^ "FileSystems");
    GlobalSettings::lookForFileSystems(GlobalSettings::exePath ^ ".." ^ "FileSystems");
    std::sort(GlobalSettings::wineVersions.rbegin(), GlobalSettings::wineVersions.rend());
}

BString GlobalSettings::getContainerFolder() {
    return GlobalSettings::dataFolderLocation ^ "Containers";
}

BString GlobalSettings::getFileSystemFolder() {
    return GlobalSettings::dataFolderLocation ^ "FileSystems";
}

BString GlobalSettings::getDemoFolder() {
    return GlobalSettings::dataFolderLocation ^ "DemoCache";
}

BString GlobalSettings::getRootFolder(BoxedContainer* container) {
    return container->dirPath ^ "root";
}

BString GlobalSettings::getAppFolder(BoxedContainer* container) {
    return container->dirPath ^ "apps";
}

BString GlobalSettings::getAutomationFolder(BoxedContainer* container) {
    return container->dirPath ^ "automation";
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

void GlobalSettings::loadFileLists() {
    BOXEDWINE_CRITICAL_SECTION;

    GlobalSettings::availableWineVersions.clear();
    GlobalSettings::availableWinetricksVersions.clear();
    GlobalSettings::availableWineDependencies.clear();
    GlobalSettings::demos.clear();
    GlobalSettings::components.clear();

    for (auto& url : GlobalSettings::fileUrls) {
        BString name = Fs::getFileNameFromPath(url);
        BString filesConfigPath = GlobalSettings::dataFolderLocation ^ name;
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filesConfigPath.c_str());
        if (!result) {
            return;
        }
        pugi::xml_node node = doc.child("XML");        
        for (pugi::xml_node wine : node.children("Wine")) {
            BString name = BString::copy(wine.child("Name").text().as_string());
            BString ver = BString::copy(wine.child("FileVersion").text().as_string());
            BString file = BString::copy(wine.child("FileURL").text().as_string());
            BString file2 = BString::copy(wine.child("FileURL2").text().as_string());
            BString depend = BString::copy(wine.child("Depend").text().as_string());
            int fileSize = wine.child("FileSizeMB").text().as_int();

            if (name.length() && ver.length() && file.length()) {
                GlobalSettings::availableWineVersions.push_back(WineVersion(name, ver, file, file2, depend, fileSize));
            }
            else {
                break;
            }
        }    
        for (pugi::xml_node wine : node.children("Winetricks")) {
            BString name = BString::copy(wine.child("Name").text().as_string());
            BString ver = BString::copy(wine.child("FileVersion").text().as_string());
            BString file = BString::copy(wine.child("FileURL").text().as_string());
            int fileSize = wine.child("FileSizeMB").text().as_int();

            if (name.length() && ver.length() && file.length()) {
                GlobalSettings::availableWinetricksVersions.push_back(WineVersion(name, ver, file, B(""), B(""), fileSize));
            }
            else {
                break;
            }
        }
        for (pugi::xml_node wine : node.children("Dependency")) {
            BString name = BString::copy(wine.child("Name").text().as_string());
            BString ver = BString::copy(wine.child("FileVersion").text().as_string());
            BString file = BString::copy(wine.child("FileURL").text().as_string());
            BString depend = BString::copy(wine.child("Depend").text().as_string());
            int fileSize = wine.child("FileSizeMB").text().as_int();

            if (name.length() && ver.length() && file.length()) {
                GlobalSettings::availableWineDependencies.push_back(WineVersion(name, ver, file, B(""), depend, fileSize));
            } else {
                break;
            }
        }        
        for (pugi::xml_node demo : node.children("Demo")) {
            BString name = BString::copy(demo.child("Name").text().as_string());
            BString installType = BString::copy(demo.child("InstallType").text().as_string());
            BString icon = BString::copy(demo.child("IconURL").text().as_string());
            int fileSize = demo.child("FileSizeMB").text().as_int();
            BString file = BString::copy(demo.child("FileURL").text().as_string());
            BString exe = BString::copy(demo.child("ShortcutExe").text().as_string());
            BString installExe = BString::copy(demo.child("InstallExe").text().as_string());
            BString help = BString::copy(demo.child("Help").text().as_string());
            BString options = BString::copy(demo.child("Options").text().as_string());
            BString installOptions = BString::copy(demo.child("InstallOptions").text().as_string());
            std::vector<BString> args;

            help = help.replace("\\n", "\n");
            help = help.replace("\\t", "    ");
            for (int i = 1; i < 100; i++) {
                BString name = "Arg" + BString::valueOf(i);
                if (!demo.child(name.c_str()).empty()) {
                    args.push_back(BString::copy(demo.child(name.c_str()).text().as_string()));
                } else {
                    break;
                }
            }
            if (name.length() && file.length()) {
                GlobalSettings::demos.push_back(AppFile(name, installType, icon, file, fileSize, exe, options, help, B(""), installOptions, installExe, args));
            } else {
                break;
            }
        }        
        for (pugi::xml_node component : node.children("Component")) {
            BString name = BString::copy(component.child("Name").text().as_string());
            BString optionsName = BString::copy(component.child("OptionsName").text().as_string());
            BString installType = BString::copy(component.child("InstallType").text().as_string());
            BString icon = BString::copy(component.child("IconURL").text().as_string());
            int fileSize = component.child("FileSizeMB").text().as_int();
            BString file = BString::copy(component.child("FileURL").text().as_string());
            BString exe = BString::copy(component.child("ShortcutExe").text().as_string());
            BString help = BString::copy(component.child("Help").text().as_string());
            BString options = BString::copy(component.child("Options").text().as_string());
            BString installOptions = BString::copy(component.child("InstallOptions").text().as_string());
            std::vector<BString> args;

            if (name.length() && file.length()) {
                GlobalSettings::components.push_back(AppFile(name, installType, icon, file, fileSize, exe, options, help, optionsName, installOptions, B(""), args));
            } else {
                break;
            }
        }
    }
    runOnMainUI([]() { // might not have an OpenGL context while starting up
        for (auto& demo : GlobalSettings::getDemos()) {
            demo.buildIconTexture();
        }
        for (auto& component : GlobalSettings::getComponents()) {
            component.buildIconTexture();
        }
        return false;
        });
}

AppFile* GlobalSettings::getComponentByOptionName(BString name) {
    for (auto& app : GlobalSettings::components) {
        if (app.optionsName == name) {
            return &app;
        }
    }
    return NULL;
}

bool GlobalSettings::checkFileListForUpdate() {
    BString filesConfigPath = GlobalSettings::dataFolderLocation + Fs::nativePathSeperator;
    BString first = GlobalSettings::dataFolderLocation ^ Fs::getFileNameFromPath(GlobalSettings::fileUrls[0]);

    PLATFORM_STAT_STRUCT buf;
    if (PLATFORM_STAT(first.c_str(), &buf) == 0) {
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

static std::vector<WineVersion> upgradeAvailable;

void doUpgrade() {
    if (upgradeAvailable.size()) {
        GlobalSettings::downloadWine(upgradeAvailable.back(), [](bool onSuccess) {
            if (onSuccess) {
                upgradeAvailable.pop_back();
                doUpgrade();
            }
            });        
    } else {
        runOnMainUI([]()->bool {
            GlobalSettings::reloadWineVersions();
            return false;
            });
    }
}

void GlobalSettings::updateFileList(BString fileLocation) {
    runInBackgroundThread([fileLocation]() {
        bool changed = false;
        GlobalSettings::filesListDownloading = true;

        for (auto& url : GlobalSettings::fileUrls) {
            BString name = Fs::getFileNameFromPath(url);
            BString path = fileLocation + name;
            BString tmpPath = fileLocation + "tmp.xml";
            unsigned int oldcrc = crc32File(path);
            BString errorMsg;
            GlobalSettings::filesListDownloading = true;
            if (::downloadFile(url, tmpPath, [](U64 bytesCompleted) {
                }, NULL, errorMsg)) {
                if (Fs::doesNativePathExist(path)) {
                    Fs::deleteNativeFile(path);
                }
                ::rename(tmpPath.c_str(), path.c_str());
                unsigned int newcrc = crc32File(path);
                if (newcrc != oldcrc) {
                    changed = true;
                }
            }
            

        }        
        runOnMainUI([changed]() {
            GlobalSettings::loadFileLists();
            GlobalSettings::filesListDownloading = false;
            runInBackgroundThread([changed]() {
                BString errorMsg;

                for (auto& demo : GlobalSettings::getDemos()) {
                    if (demo.iconPath.length()) {
                        int pos = demo.iconPath.lastIndexOf('/');
                        if (pos == -1) {
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
                if (changed) {
                    upgradeAvailable.clear();
                    BString wineLabel;
                    for (auto& ver : GlobalSettings::wineVersions) {
                        for (auto& avail : GlobalSettings::availableWineVersions) {
                            if (ver.name == avail.name && ver.fsVersion != avail.fsVersion) {
                                upgradeAvailable.push_back(avail);
                                if (wineLabel.length()) {
                                    wineLabel += ", ";
                                }
                                wineLabel += ver.name;
                            }
                        }
                    }
                    if (upgradeAvailable.size()) {
                        runOnMainUI([wineLabel]() {
                            new YesNoDlg(WINE_UPGRADE_AVAILABLE_TITLE, getTranslationWithFormat(WINE_UPGRADE_AVAILABLE_LABEL, false, wineLabel), [](bool yes) {
                                if (yes) {
                                    runInBackgroundThread([]() {
                                        doUpgrade();
                                        });
                                }
                                });
                            return false;
                        });
                    }
                }
            });
            return false;
        });
    }       
    );
}

void GlobalSettings::loadFonts() {
    BString fontsPath = GlobalSettings::dataFolderLocation ^ "Fonts";
    if (!Fs::doesNativePathExist(fontsPath)) {
        Fs::makeNativeDirs(fontsPath);
    }
    BString zipFilePath;

    if (wineVersions.size()) {
        zipFilePath = wineVersions[0].getDependFilePath();

        if (!zipFilePath.length() || !Fs::doesNativePathExist(zipFilePath)) {
            zipFilePath = wineVersions[0].filePath;
        }
        if (!Fs::doesNativePathExist(zipFilePath)) {
            zipFilePath = B("");
        }
    }
    BString sansBoldFontsPath = fontsPath ^ "LiberationSans-Bold.ttf";
    if (!Fs::doesNativePathExist(sansBoldFontsPath) && zipFilePath.length()) {
        FsZip::extractFileFromZip(zipFilePath, B("usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"), fontsPath);
    }
    BString sansFontsPath = fontsPath ^ "LiberationSans-Regular.ttf";
    if (!Fs::doesNativePathExist(sansFontsPath) && zipFilePath.length()) {
        FsZip::extractFileFromZip(zipFilePath, B("usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"), fontsPath);
    }
    BString awesomeFontPath = fontsPath ^ "fontawesome-webfont.ttf";
    if (!Fs::doesNativePathExist(awesomeFontPath) && zipFilePath.length()) {
        FsZip::extractFileFromZip(zipFilePath, B("usr/share/fonts/truetype/fontawesome-webfont.ttf"), fontsPath);
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

void GlobalSettings::downloadFile(BString url, BString filePath, BString name, U32 sizeMB, std::function<void(bool)> onCompleted) {
    runOnMainUI([url, filePath, name, sizeMB, onCompleted]() {
        BString parentPath = Fs::getNativeParentPath(filePath);
        if (!Fs::doesNativePathExist(parentPath)) {
            Fs::makeNativeDirs(parentPath);
        }
        std::vector<DownloadItem> items;
        items.push_back(DownloadItem(getTranslationWithFormat(DOWNLOADDLG_LABEL, true, name), url, B(""), filePath, ((U64)sizeMB) * 1024 * 1024));
        new DownloadDlg(DOWNLOADDLG_TITLE, items, [onCompleted](bool success) {
            runOnMainUI([success, onCompleted]() {
                GlobalSettings::reloadWineVersions();
                if (!GlobalSettings::defaultFont) {
                    GlobalSettings::restartUI = true;
                }
                onCompleted(success);
                return false;
                });
            });
        return false;
        });
}

void GlobalSettings::downloadWine(const WineVersion& version, std::function<void(bool)> onCompleted) {
    runOnMainUI([&version, onCompleted]() {
        BString filePath = version.getLocalFilePath();
        if (!Fs::doesNativePathExist(GlobalSettings::getFileSystemFolder())) {
            Fs::makeNativeDirs(GlobalSettings::getFileSystemFolder());
        }
        std::vector<DownloadItem> items;
        items.push_back(DownloadItem(getTranslationWithFormat(DOWNLOADDLG_LABEL, true, version.name), version.filePath, version.filePathBackup, filePath, ((U64)(version.size)) * 1024 * 1024));
        WineVersion* depend = version.getMissingDependency();
        if (depend) {
            items.push_back(DownloadItem(getTranslationWithFormat(DOWNLOADDLG_LABEL, true, depend->name), depend->filePath, depend->filePathBackup, depend->getLocalFilePath(), ((U64)(depend->size)) * 1024 * 1024));
        }
        new DownloadDlg(DOWNLOADDLG_TITLE, items, [onCompleted](bool success) {
            runOnMainUI([success, onCompleted]() {
                GlobalSettings::reloadWineVersions();
                if (!GlobalSettings::defaultFont) {
                    GlobalSettings::restartUI = true;
                }
                if (onCompleted) {
                    onCompleted(success);
                }
                return false;
                });
            });
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

void GlobalSettings::setTheme(BString theme) { 
    GlobalSettings::theme = theme; 
    loadTheme();
}

void GlobalSettings::setFontScale(float scale) {
    GlobalSettings::fontScale = scale;
}

BString GlobalSettings::createUniqueContainerPath(BString name) {
    BString containerName = name;
    containerName = containerName.replace(' ', '_');
    containerName = containerName.replace(":", "");

    std::srand((U32)std::time(nullptr));
    while (true) {
        BString tmp = BString::valueOf(std::rand(), 16);
        BString result = GlobalSettings::getContainerFolder() ^ containerName + "-"+tmp;
        if (!Fs::doesNativePathExist(result)) {
            return result;
        }
    }
}

BString WineVersion::getDependFilePath() const {
    if (this->depend.length()) {
        BString result = GlobalSettings::getFileSystemFolder() ^ depend;
        if (!Fs::doesNativePathExist(result)) {
            BString parentPath = Fs::getNativeParentPath(this->filePath);
            result = parentPath ^ depend;
        }
        if (Fs::doesNativePathExist(result)) {
            return result;
        }
        return depend;
    }
    return B("");
}

WineVersion* WineVersion::getMissingDependency() const {
    BString dependPath = GlobalSettings::getFileSystemFolder() ^ depend;
    for (auto& w : GlobalSettings::availableWineDependencies) {
        if (w.name == this->depend) {
            BString version;
            if (FsZip::readFileFromZip(dependPath, B("version.txt"), version)) {
                if (version != w.fsVersion) {
                    return &w;
                }
            }
        }
    }
    if (this->depend.length()) {
        if (!Fs::doesNativePathExist(this->depend)) {            
            if (!Fs::doesNativePathExist(dependPath)) {
                for (auto& w : GlobalSettings::availableWineDependencies) {
                    if (w.name == this->depend) {
                        return &w;
                    }
                }
            }
        }
    }
    return NULL;
}

BString WineVersion::getLocalFilePath() const {
    int pos = this->filePath.lastIndexOf('/');
    if (pos == -1) {
        return B(""); // :TODO: error msg?
    }
    BString fileName = this->filePath.substr(pos + 1);
    return GlobalSettings::getFileSystemFolder() ^ fileName;
}
