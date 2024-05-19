#ifndef __GLOBAL_SETTINGS_H__
#define __GLOBAL_SETTINGS_H__

#include "../../sdl/startupArgs.h"
#include "appFile.h"

class BoxedContainer;

//used http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=F015&mode=hex to convert 16-bit unicode value to utf-8

// home icon - F016
#define APP_LIST_ICON "\xEF\x80\x95"

// save icon - F0C7
#define INSTALL_ICON "\xEF\x83\x87" 
// download icon - F01A
#define INSTALL_DEMO_ICON "\xEF\x80\x9A"

#define INSTALL_COMPONENTS_ICON "\xEF\x80\x9A"

// archive icon - F187
#define CONTAINER_ICON "\xEF\x86\x87"

// cog icon - F013
#define OPTIONS_ICON "\xEF\x80\x93"
// wrench icon - F0AD
#define OPTIONS_GENERAL "\xEF\x82\xAD"
// desktop icon - F108
#define OPTION_DISPLAY_ICON "\xEF\x84\x88"
// linux icon - F17C
#define OPTION_WINE_ICON "\xEF\x85\xBC"

// remove_circle icon - F05C
#define DELETE_ICON "\xEF\x81\x9C"
// trash icon - F014
#define TRASH_ICON "\xEF\x80\x94"
// question icon - F059
#define QUESTION_ICON "\xEF\x81\x99"

// comment icon - F075
#define ABOUT_ICON "\xEF\x81\xB5"

class FileSystemZip {
public:
    FileSystemZip(BString name, BString wineName, BString fsVersion, BString filePath, BString filePathBackup, BString depend, U32 size=0):name(name), wineName(wineName), filePath(filePath), filePathBackup(filePathBackup), fsVersion(fsVersion), depend(depend), size(size)  {}
    std::shared_ptr<FileSystemZip> getMissingDependency() const;
    BString getLocalFilePath() const;
    BString getDependFilePath() const;

    bool hasWine() {return wineName.length() > 0;}
    bool hasWineTricks() {return wineTrickFonts.length() > 0 || wineTrickDlls.length() > 0;}

    BString name;
    BString wineName;
    BString filePath;
    BString filePathBackup;
    BString fsVersion;
    BString depend;
    BString wineTrickFonts;
    BString wineTrickDlls;
    std::vector<BString> tinyCorePackages;
    BString tinyCoreURL;
    U32 size;
    bool operator<(const FileSystemZip& rhs) const { 
        if (wineName.length() && rhs.wineName.length()) {
            return wineName < rhs.wineName;
        } else if (wineName.length()) {
            return false;
        } else if (rhs.wineName.length()) {
            return true;
        }
        return name < rhs.name; 
    }
};

struct ImFont;

class GlobalSettings {
public:
    static void init(int argc, const char **argv);
    
    static void reloadWineVersions();
    static BString getFileFromFileSystemName(BString name);
    static BString getFileFromWineVersion(BString name);
    static std::shared_ptr<FileSystemZip> getAvailableFileSystemFromName(BString name, bool mustHaveWine = false);
    static std::shared_ptr<FileSystemZip> getInstalledFileSystemFromName(BString name, bool mustHaveWine = false);
    static BString getContainerFolder();
    static BString getFileSystemFolder();
    static BString getAppFolder(BoxedContainer* container);
    static BString getAutomationFolder(BoxedContainer* container);
    static BString getRootFolder(BoxedContainer* container);
    static BString getDemoFolder();
    static BString getExePath() {return GlobalSettings::exePath;}
    static BString getExeFilePath() { return GlobalSettings::exeFilePath; }
    static std::vector<std::shared_ptr<FileSystemZip>> getWineVersions();
    static std::vector<std::shared_ptr<FileSystemZip>> getAvailableWineVersions();
    static const std::vector<std::shared_ptr<FileSystemZip>>& getFileSystemVersions() {return GlobalSettings::fileSystemVersions;}
    static const std::vector<std::shared_ptr<FileSystemZip>>& getAvailableFileSystemVersions() { return GlobalSettings::availableFileSystemVersions; }
    static BString getDataFolder() {return GlobalSettings::dataFolderLocation;}
    static void setDataFolder(BString location) {GlobalSettings::dataFolderLocation = location;}
    static void setTheme(BString theme);
    static BString getTheme() {return GlobalSettings::theme;}
    static void saveConfig();
    static void saveScreenSize(int x, int y, int cx, int cy);
    static void loadTheme();
    static void reloadApps();
    static void loadFonts();
    static void setScale(U32 scale);
    static U32 scaleIntUI(U32 value);
    static float scaleFloatUI(float value);
    static float scaleFloatUIAndFont(float value);
    static void downloadFileSystem(const std::shared_ptr<FileSystemZip>& version, std::function<void(bool)> onCompleted);
    static bool isFilesListDownloading();
    static void startUp();
    static U32 getFrameDelayMillies();
    static void useFastFrameRate(bool useFast);
    static void updateLastFrameDelayChange();
    static const std::vector<BString>& getAvailableResolutions() { return GlobalSettings::availableResolutions;}
    static const int getDefaultScale() { return GlobalSettings::defaultScale; }
    static const int getDefaultVsync() { return GlobalSettings::defaultVsync; }
    static const BString getDefaultResolution() { return GlobalSettings::defaultResolution; }
    static U32 getDefaultOpenGL() { return GlobalSettings::defaultOpenGL; }
    static int getScreenCx() { return GlobalSettings::screenCx; }
    static int getScreenCy() { return GlobalSettings::screenCy; }
    static void setFontScale(float scale);
    static bool hasIconsFont() {return GlobalSettings::iconFontsLoaded;}
    static std::vector<AppFile>& getDemos() {return GlobalSettings::demos;}
    static std::vector<AppFile>& getComponents() { return GlobalSettings::components; }
    static AppFile* getComponentByOptionName(BString name);
    static void downloadFile(BString url, BString filePath, BString name, U32 sizeMB, std::function<void(bool)> onCompleted);
    static BString createUniqueContainerPath(BString name);
    static bool isDpiAware() {return GlobalSettings::scale != 100;}
    static int getPreviousScreenWidth() {return GlobalSettings::lastScreenCx;}
    static int getPreviousScreenHeight() { return GlobalSettings::lastScreenCy;}
    static int getPreviousScreenX() { return GlobalSettings::lastScreenX; }
    static int getPreviousScreenY() { return GlobalSettings::lastScreenY; }
    static void clearData();
    static bool isAutomationEnabled() {return GlobalSettings::enabledAutomation;}

    static StartUpArgs startUpArgs;

    static ImFont* largeFontBold;
    static ImFont* largeFont;
    static ImFont* mediumFont;
    static ImFont* defaultFont;
    static ImFont* sectionTitleFont;
    static bool restartUI;
    static bool reinit;
    static std::function<void()> keepUIRunning;
    static float extraVerticalSpacing;
private:    
    static void initWineVersions();
    static void lookForFileSystems(BString path);    
    static void loadFileLists();
    static void updateFileList(BString fileLocation);
    static bool checkFileListForUpdate();

    static int iconSize;
    static U32 scale;
    static BString dataFolderLocation;
    static std::vector<std::shared_ptr<FileSystemZip>> fileSystemVersions;
    static BString exePath;
    static BString exeFilePath;
    static BString theme;
    static BString configFilePath;

    friend class OptionsView;
    friend class FileSystemZip;
    static std::vector<std::shared_ptr<FileSystemZip>> availableFileSystemVersions;
    static std::vector<std::shared_ptr<FileSystemZip>> availableFileSystemDependencies;
    static std::vector<AppFile> demos;
    static std::vector<AppFile> components;
    static bool filesListDownloading;
    static U32 frameDelayMillies; // decrease if we are animating, else this can be pretty large
    static U32 fastFrameRateCount;
    static U64 lastFrameDelayChange;
    static std::vector<BString> availableResolutions;
    static BString defaultResolution;
    static int defaultScale;
    static int defaultVsync;
    static int screenCx;
    static int screenCy;
    static float fontScale;
    static bool iconFontsLoaded;
    static std::vector<BString> fileUrls;
    static int lastScreenCx;
    static int lastScreenCy;
    static int lastScreenX;
    static int lastScreenY;
    static U32 defaultOpenGL;
    static bool enabledAutomation;
};

#endif
