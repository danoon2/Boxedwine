#ifndef __BOXED_CONTAINER_H__
#define __BOXED_CONTAINER_H__

class BoxedContainer {
public:
    BoxedContainer() {}
    ~BoxedContainer();

    static BoxedContainer* createContainer(const std::string& dirPath, const std::string& name, const std::string& wineVersion);

    bool load(const std::string& dirPath);
    bool saveContainer();

    void reload();
    const std::vector<BoxedApp*>& getApps() {return this->apps;}
    void addApp(BoxedApp* app) {this->apps.push_back(app);}
    void deleteApp(BoxedApp* app);
    void deleteContainerFromFilesystem();
    void launch(); // will set StartUpArgs to use this container
    void launch(const std::vector<std::string>& args, const std::string& labelForWaitDlg); // will create new StartUpArgs and set it to use this container and the args
    bool doesWineVersionExist();

    BoxedApp* getAppByIniFile(const std::string& iniFile);

    const std::string& getName() {return this->name;}
    const std::string& getDir() {return this->dirPath;}
    const std::string& getWineVersion() {return this->wineVersion;}
    const std::string& getSize() {return this->cachedSize;}

    void setName(const std::string& name);

    std::string getNativePathForApp(const BoxedApp& app);

    void getNewApps(std::vector<BoxedApp>& apps, MountInfo* mount=NULL, const std::string& nativeDirectory="");
    void getWineApps(std::vector<BoxedApp>& apps);
    void updateCachedSize();

    void setWineVersion(const std::string& wineVersion) {this->wineVersion = wineVersion;}
    bool addNewMount(const MountInfo& mountInfo);
    void clearMounts() {this->mounts.clear();}
    const std::vector<MountInfo>& getMounts() {return this->mounts;}

    bool isGDI();
    void setGDI(bool gdi);
    std::string getWindowsVersion();
    void setWindowsVersion(const BoxedWinVersion& version);    
    std::string getLogPath();

private:
    void loadApps();
    void getNewDesktopLinkApps(std::vector<BoxedApp>& apps);
    void getNewExeApps(std::vector<BoxedApp>& apps, MountInfo* mount, std::string nativeDirectory);
    std::string getWindowsVersion2();

    std::vector<BoxedApp*> apps;
    std::string name;
    std::string wineVersion;
    std::string cachedSize;

    friend class GlobalSettings;
    std::string dirPath;
    std::vector<MountInfo> mounts;
};

#endif
