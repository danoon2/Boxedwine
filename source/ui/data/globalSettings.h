#ifndef __GLOBAL_SETTINGS_H__
#define __GLOBAL_SETTINGS_H__

#include "../../sdl/startupArgs.h"

class BoxedContainer;

class WineVersion {
public:
    WineVersion(const std::string& name, const std::string& fsVersion, const std::string& filePath, U32 size=0, const std::string& changes=""):name(name), filePath(filePath), fsVersion(fsVersion), size(size), changes(changes)  {}
    std::string name;
    std::string filePath;
    std::string fsVersion;
    U32 size;
    std::string changes;
    bool operator<(const WineVersion &rhs) const { return name < rhs.name; }
};

struct ImFont;

class GlobalSettings {
public:
    static void init(int argc, const char **argv);
    
    static void reloadWineVersions();
    static std::string getFileFromWineName(const std::string& name);    
    static std::string getContainerFolder();
    static std::string getFileSystemFolder();    
    static std::string getAppFolder(BoxedContainer* container);
    static std::string getRootFolder(BoxedContainer* container);    
    static double getScaleFactor();
    static std::string getExePath() {return GlobalSettings::exePath;}
    static const std::vector<WineVersion>& getWineVersions() {return GlobalSettings::wineVersions;}
    static const std::vector<WineVersion>& getAvailableWineVersions() { return GlobalSettings::availableWineVersions; }
    static std::string getDataFolder() {return GlobalSettings::dataFolderLocation;}
    static void setDataFolder(const std::string& location) {GlobalSettings::dataFolderLocation = location;}
    static void setTheme(const std::string& theme);
    static std::string getTheme() {return GlobalSettings::theme;}
    static void saveConfig();
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
    static const std::string& getDefaultResolution() { return GlobalSettings::defaultResolution; }
    static int getScreenCx() { return GlobalSettings::screenCx; }
    static int getScreenCy() { return GlobalSettings::screenCy; }
    static void setFontScale(float scale);

    static StartUpArgs startUpArgs;

    static ImFont* largeFontBold;
    static ImFont* largeFont;
    static ImFont* mediumFont;
    static ImFont* defaultFont;
    static ImFont* sectionTitleFont;
    static bool restartUI;    
    static float extraVerticalSpacing;
private:    
    static void initWineVersions();
    static void lookForFileSystems(const std::string& path);    
    static void loadFileList();
    static void updateFileList(const std::string& fileLocation);
    static bool checkFileListForUpdate();

    static int iconSize;
    static U32 scale;
    static std::string dataFolderLocation;
    static std::vector<WineVersion> wineVersions;
    static std::string exePath;
    static std::string theme;
    static std::string configFilePath;

    friend class OptionsView;
    static std::vector<WineVersion> availableWineVersions;
    static bool filesListDownloading;
    static U32 frameDelayMillies; // decrease if we are animating, else this can be pretty large
    static U32 fastFrameRateCount;
    static U64 lastFrameDelayChange;
    static std::vector<std::string> availableResolutions;
    static std::string defaultResolution;
    static int defaultScale;
    static int screenCx;
    static int screenCy;
    static float fontScale;
};

#endif
