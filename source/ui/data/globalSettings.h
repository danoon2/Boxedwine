/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GLOBAL_SETTINGS_H__
#define __GLOBAL_SETTINGS_H__

#include "../../sdl/startupArgs.h"
#include "appFile.h"

class BoxedContainer;

#define OPENGL_TYPE_DEFAULT 0
#define OPENGL_TYPE_NATIVE 1
#define OPENGL_TYPE_LLVM_PIPE 2
#define OPENGL_TYPE_ON_D3D12 3
#define OPENGL_TYPE_ON_VULKAN 4

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
    FileSystemZip(BString name, BString wineName, BString fsVersion, BString filePath, BString filePathBackup, BString depend, U32 size = 0);
    std::shared_ptr<FileSystemZip> getMissingDependency() const;
    BString getLocalFilePath() const;
    BString getDependFilePath() const;

    bool hasWine() {return wineMajorVersion > 0;}
    bool hasWineTricks();

    BString name;
    int wineVersion = 0; // 6.0 will equal 600, 6.1 will equal 601
    int wineMajorVersion = 0;
    int wineMinorVersion = 0;
    BString filePath;
    BString filePathBackup;
    BString fsVersion;
    BString depend;
    BString wineTrickFonts;
    BString wineTrickDlls;
    std::vector<BString> tinyCorePackages;
    BString tinyCoreURL;
    BString tinyCoreBackupURL;
    BString dist;
    U32 size;
    bool operator<(const FileSystemZip& rhs) const { 
        if (wineMajorVersion && rhs.wineMajorVersion) {
            if (wineMajorVersion != rhs.wineMajorVersion) {
                return wineMajorVersion < rhs.wineMajorVersion;
            }
            return wineMinorVersion < rhs.wineMinorVersion;
        } else if (wineMajorVersion) {
            return false;
        } else if (rhs.wineMajorVersion) {
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
    static BString getFileFromWineVersion(int wineVersion);
    static std::shared_ptr<FileSystemZip> getAvailableFileSystemFromName(BString name, bool mustHaveWine = false);
    static std::shared_ptr<FileSystemZip> getInstalledFileSystemFromName(BString name, bool mustHaveWine = false);
    static BString getContainerFolder();
    static BString getFileSystemFolder();
    static BString getAlternativeOpenGlFolder();
    static BString getAppFolder(BoxedContainer* container);
    static BString getAutomationFolder(BoxedContainer* container);
    static BString getRootFolder(BoxedContainer* container);
    static BString getDemoFolder();
    static BString getArgsPath(); // used by Mac
    static BString getExePath() {return GlobalSettings::exePath;}
    static BString getExeFilePath() { return GlobalSettings::exeFilePath; }
    static std::vector<std::shared_ptr<FileSystemZip>> getWineVersions();
    static std::vector<std::shared_ptr<FileSystemZip>> getAvailableWineVersions();
    static const std::vector<std::shared_ptr<FileSystemZip>>& getFileSystemVersions() {return GlobalSettings::fileSystemVersions;}
    static const std::vector<std::shared_ptr<FileSystemZip>>& getAvailableFileSystemVersions() { return GlobalSettings::availableFileSystemVersions; }
    static BString getDataFolder() {return GlobalSettings::dataFolderLocation;}
    static BString getCacheFolder() { return GlobalSettings::dataFolderLocation.stringByApppendingPath("cache"); }
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
    static std::vector<AppFilePtr>& getDemos() {return GlobalSettings::demos;}
    static std::vector<AppFilePtr>& getComponents() { return GlobalSettings::components; }
    static AppFilePtr getComponentByOptionName(BString name);
    static void downloadFile(BString url, BString filePath, BString name, U32 sizeMB, std::function<void(bool)> onCompleted);
    static void downloadOpenGL(std::function<void(bool)> onCompleted);
    static BString createUniqueContainerPath(BString name);
    static bool isDpiAware() {return GlobalSettings::scale != 100;}
    static int getPreviousScreenWidth() {return GlobalSettings::lastScreenCx;}
    static int getPreviousScreenHeight() { return GlobalSettings::lastScreenCy;}
    static int getPreviousScreenX() { return GlobalSettings::lastScreenX; }
    static int getPreviousScreenY() { return GlobalSettings::lastScreenY; }
    static bool isAutomationEnabled() {return GlobalSettings::enabledAutomation;}
    static bool isAlternativeOpenGlDownloaded();
    static BString alternativeOpenGlLocation();
    static BString alternativeOpenGlUrl();
    static U32 alternativeOpenGlDownloadSizeMB();
    static bool cacheReads() { return GlobalSettings::enabledCachedReadFiles; }
    static void setOpenGlTypeOnStartupArgs(U32 type);

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
    static std::vector<AppFilePtr> demos;
    static std::vector<AppFilePtr> components;
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
    static bool enabledCachedReadFiles;
};

#endif
