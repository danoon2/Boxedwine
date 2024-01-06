#ifndef __BOXED_CONTAINER_H__
#define __BOXED_CONTAINER_H__

class BoxedContainer {
public:
    BoxedContainer() {}
    ~BoxedContainer();

    static BoxedContainer* createContainer(BString dirPath, BString name, BString wineVersion);

    bool load(BString dirPath);
    bool saveContainer();

    void reload();
    const std::vector<BoxedApp*>& getApps() {return this->apps;}
    void addApp(BoxedApp* app) {this->apps.push_back(app);}
    void deleteApp(BoxedApp* app);
    void deleteContainerFromFilesystem();
    void launch(); // will set StartUpArgs to use this container
    void launch(const std::vector<BString>& args, BString labelForWaitDlg); // will create new StartUpArgs and set it to use this container and the args
    bool doesWineVersionExist();

    BoxedApp* getAppByIniFile(BString iniFile);

    BString getName() {return this->name;}
    BString getDir() {return this->dirPath;}
    BString getWineVersion() {return this->wineVersion;}
    int getWineVersionAsNumber();
    int getWineVersionAsNumber(BString wineVersion);
    BString getSize() {return this->cachedSize;}

    void setName(BString name);

    BString getNativePathForApp(const BoxedApp& app);

    void getNewApps(std::vector<BoxedApp>& apps, MountInfo* mount=NULL, BString nativeDirectory=BString());
    void getWineApps(std::vector<BoxedApp>& apps);
    void updateCachedSize();

    void setWineVersion(BString wineVersion) {this->wineVersion = wineVersion;}
    bool addNewMount(const MountInfo& mountInfo);
    void clearMounts() {this->mounts.clear();}
    const std::vector<MountInfo>& getMounts() {return this->mounts;}

    bool isGDI();
    void setGDI(bool gdi);
    BString getRenderer();
    void setRenderer(BString renderer);
    BString getMouseWarpOverride();
    void setMouseWarpOverride(BString value);
    BString getWindowsVersion();
    void setWindowsVersion(const BoxedWinVersion& version);    
    BString getLogPath();

private:
    void loadApps();
    void getNewDesktopLinkApps(std::vector<BoxedApp>& apps);
    void getNewExeApps(std::vector<BoxedApp>& apps, MountInfo* mount, BString nativeDirectory);
    BString getWindowsVersion2();

    std::vector<BoxedApp*> apps;
    BString name;
    BString wineVersion;
    BString cachedSize;

    friend class GlobalSettings;
    BString dirPath;
    std::vector<MountInfo> mounts;
};

#endif
