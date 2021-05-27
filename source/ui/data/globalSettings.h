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

class WineVersion {
public:
    WineVersion(const std::string& name, const std::string& fsVersion, const std::string& filePath, const std::string& filePathBackup, const std::string& depend, U32 size=0):name(name), filePath(filePath), filePathBackup(filePathBackup), fsVersion(fsVersion), depend(depend), size(size)  {}
    WineVersion* getMissingDependency() const;
    std::string getLocalFilePath() const;
    std::string getDependFilePath() const;

    std::string name;
    std::string filePath;
    std::string filePathBackup;
    std::string fsVersion;
    std::string depend;
    U32 size;
    bool operator<(const WineVersion& rhs) const { return name < rhs.name; }
};

struct ImFont;

class GlobalSettings {
public:
    static void init(int argc, const char **argv);
    
    static void reloadWineVersions();
    static std::string getFileFromWineName(const std::string& name);    
    static WineVersion* getAvailableWineFromName(const std::string& name);
    static std::string getContainerFolder();
    static std::string getFileSystemFolder();    
    static std::string getAppFolder(BoxedContainer* container);
    static std::string getRootFolder(BoxedContainer* container);    
    static std::string getDemoFolder();
    static std::string getExePath() {return GlobalSettings::exePath;}
    static std::string getExeFilePath() { return GlobalSettings::exeFilePath; }
    static std::string getMesaExeFilePath() { return GlobalSettings::mesaFilePath; }
    static const std::vector<WineVersion>& getWineVersions() {return GlobalSettings::wineVersions;}
    static const std::vector<WineVersion>& getAvailableWineVersions() { return GlobalSettings::availableWineVersions; }
    static std::string getDataFolder() {return GlobalSettings::dataFolderLocation;}
    static void setDataFolder(const std::string& location) {GlobalSettings::dataFolderLocation = location;}
    static void setTheme(const std::string& theme);
    static std::string getTheme() {return GlobalSettings::theme;}
    static void saveConfig();
    static void saveScreenSize(int x, int y, int cx, int cy);
    static void loadTheme();
    static void reloadApps();
    static void loadFonts();
    static void setScale(U32 scale);
    static U32 scaleIntUI(U32 value);
    static float scaleFloatUI(float value);
    static float scaleFloatUIAndFont(float value);
    static void downloadWine(const WineVersion& version, std::function<void(bool)> onCompleted);
    static bool isFilesListDownloading();
    static void startUp();
    static U32 getFrameDelayMillies();
    static void useFastFrameRate(bool useFast);
    static void updateLastFrameDelayChange();
    static const std::vector<std::string>& getAvailableResolutions() { return GlobalSettings::availableResolutions;}
    static const int getDefaultScale() { return GlobalSettings::defaultScale; }
    static const int getDefaultVsync() { return GlobalSettings::defaultVsync; }
    static const std::string& getDefaultResolution() { return GlobalSettings::defaultResolution; }
    static U32 getDefaultOpenGL() { return GlobalSettings::defaultOpenGL; }
    static int getScreenCx() { return GlobalSettings::screenCx; }
    static int getScreenCy() { return GlobalSettings::screenCy; }
    static void setFontScale(float scale);
    static bool hasIconsFont() {return GlobalSettings::iconFontsLoaded;}
    static std::vector<AppFile>& getDemos() {return GlobalSettings::demos;}
    static std::vector<AppFile>& getComponents() { return GlobalSettings::components; }
    static AppFile* getComponentByOptionName(const std::string& name);
    static void downloadFile(const std::string& url, const std::string& filePath, const std::string& name, U32 sizeMB, std::function<void(bool)> onCompleted);
    static std::string createUniqueContainerPath(const std::string& name);
    static bool isDpiAware() {return GlobalSettings::scale != 100;}
    static int getPreviousScreenWidth() {return GlobalSettings::lastScreenCx;}
    static int getPreviousScreenHeight() { return GlobalSettings::lastScreenCy;}
    static int getPreviousScreenX() { return GlobalSettings::lastScreenX; }
    static int getPreviousScreenY() { return GlobalSettings::lastScreenY; }
    static void clearData();

    static StartUpArgs startUpArgs;

    static ImFont* largeFontBold;
    static ImFont* largeFont;
    static ImFont* mediumFont;
    static ImFont* defaultFont;
    static ImFont* sectionTitleFont;
    static bool restartUI;
    static bool reinit;
    static float extraVerticalSpacing;
private:    
    static void initWineVersions();
    static void lookForFileSystems(const std::string& path);    
    static void loadFileLists();
    static void updateFileList(const std::string& fileLocation);
    static bool checkFileListForUpdate();

    static int iconSize;
    static U32 scale;
    static std::string dataFolderLocation;
    static std::vector<WineVersion> wineVersions;
    static std::string exePath;
    static std::string exeFilePath;
    static std::string mesaFilePath;
    static std::string theme;
    static std::string configFilePath;

    friend class OptionsView;
    friend class WineVersion;
    static std::vector<WineVersion> availableWineVersions;
    static std::vector<WineVersion> availableWineDependencies;
    static std::vector<AppFile> demos;
    static std::vector<AppFile> components;
    static bool filesListDownloading;
    static U32 frameDelayMillies; // decrease if we are animating, else this can be pretty large
    static U32 fastFrameRateCount;
    static U64 lastFrameDelayChange;
    static std::vector<std::string> availableResolutions;
    static std::string defaultResolution;
    static int defaultScale;
    static int defaultVsync;
    static int screenCx;
    static int screenCy;
    static float fontScale;
    static bool iconFontsLoaded;
    static std::vector<std::string> fileUrls;
    static int lastScreenCx;
    static int lastScreenCy;
    static int lastScreenX;
    static int lastScreenY;
    static U32 defaultOpenGL;
};

#endif
