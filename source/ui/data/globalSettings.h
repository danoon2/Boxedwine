#ifndef __GLOBAL_SETTINGS_H__
#define __GLOBAL_SETTINGS_H__

#include "../../sdl/startupArgs.h"

class BoxedContainer;

class WineVersion {
public:
    WineVersion(const std::string& name, const std::string& filePath):name(name),filePath(filePath){}
    std::string name;
    std::string filePath;

    bool operator<(const WineVersion &rhs) const { return name < rhs.name; }
};

class GlobalSettings {
public:
    static void init(int argc, const char **argv);

    static void initWineVersions();
    static std::string getFileFromWineName(const std::string& name);    
    static std::string getContainerFolder();
    static std::string getFileSystemFolder();    
    static std::string getAppFolder(BoxedContainer* container);
    static std::string getRootFolder(BoxedContainer* container);    
    static double getScaleFactor();
    static std::string getExePath() {return GlobalSettings::exePath;}
    static const std::vector<WineVersion>& getWineVersions() {return GlobalSettings::wineVersions;}
    static std::string getDataFolder() {return GlobalSettings::dataFolderLocation;}
    static void setDataFolder(const std::string& location) {GlobalSettings::dataFolderLocation = location;}
    static void setTheme(const std::string& theme) {GlobalSettings::theme=theme;}
    static std::string getTheme() {return GlobalSettings::theme;}
    static void saveConfig();
    static void loadTheme();

    static StartUpArgs startUpArgs;
private:
    static void lookForFileSystems(const std::string& path);    

    static int iconSize;
    static double scaleFactor;
    static std::string dataFolderLocation;
    static std::vector<WineVersion> wineVersions;    
    static std::string exePath;
    static std::string theme;
    static std::string configFilePath;
};

#endif