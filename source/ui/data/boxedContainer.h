#ifndef __BOXED_CONTAINER_H__
#define __BOXED_CONTAINER_H__

class WaitDlg;

class BoxedContainer {
public:
    BoxedContainer() {}
    ~BoxedContainer();

    static BoxedContainer* createContainer(BString dirPath, BString name, std::shared_ptr<FileSystemZip> fileSystem);

    bool load(BString dirPath);
    bool saveContainer();

    void reload();
    const std::vector<BoxedApp*>& getApps() { return this->apps; }
    void addApp(BoxedApp* app) { this->apps.push_back(app); }
    void deleteApp(BoxedApp* app);
    void deleteContainerFromFilesystem();
    void launch(); // will set StartUpArgs to use this container
    void launch(const std::vector<BString>& args, BString labelForWaitDlg); // will create new StartUpArgs and set it to use this container and the args
    bool doesFileSystemExist();

    BoxedApp* getAppByIniFile(BString iniFile);

    BString getName() { return this->name; }
    BString getDir() { return this->dirPath; }
    std::weak_ptr<FileSystemZip> getFileSystem() {return this->fileSystem;}
    void setFileSystem(std::shared_ptr<FileSystemZip> fileSystem) { this->fileSystem = fileSystem; }
    BString getFileSystemName();
    int getWineVersionAsNumber(BString wineVersion);
    BString getSize() {return this->cachedSize;}

    void setName(BString name);

    BString getNativePathForApp(const BoxedApp& app);

    void getNewApps(std::vector<BoxedApp>& apps, MountInfo* mount= nullptr, BString nativeDirectory=BString());
    void findApps(std::vector<BoxedApp>& apps);
    void updateCachedSize();
    bool doesFileExist(const BString& localFilePath);

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

    void installTinyCorePackage(BString package);    
    
private:
    void getTinyCorePackages(BString package, std::vector<BString>& todo, std::vector<BString>& needsDownload);
    void installNextTinyCorePackage(WaitDlg* dlg, std::vector<BString> packages);
    void doInstallTinyCorePackage(const std::vector<BString>& todo);

    void loadApps();
    void getNewDesktopLinkApps(std::vector<BoxedApp>& apps);
    void getNewExeApps(std::vector<BoxedApp>& apps, MountInfo* mount, BString nativeDirectory);
    BString getWindowsVersion2();

    std::vector<BoxedApp*> apps;
    BString name;
    BString cachedSize;
    BString fileSystemZipName;

    friend class GlobalSettings;
    BString dirPath;
    std::vector<MountInfo> mounts;
    std::weak_ptr<FileSystemZip> fileSystem;
};

#endif
