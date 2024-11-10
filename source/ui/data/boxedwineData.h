#ifndef __BOXEDWINE_DATA_H__
#define __BOXEDWINE_DATA_H__

#define VER_PLATFORM_WIN32s                     0
#define VER_PLATFORM_WIN32_WINDOWS              1
#define VER_PLATFORM_WIN32_NT                   2

class BoxedContainer;

class BoxedWinVersion {
public:
    BoxedWinVersion(const char* ver, const char* desc, const char* currentVersion, U32 major, U32 minor, U32 build, U32 id, const char* csd, U16 majorPack, U16 minorPack, const char* product);

    const char* szVersion;
    const char* szDescription;
    const char* szCurrentVersion;
    U32       dwMajorVersion;
    U32       dwMinorVersion;
    U32       dwBuildNumber;
    U32       dwPlatformId;
    const char* szCSDVersion;
    U16        wServicePackMajor;
    U16        wServicePackMinor;
    const char* szProductType;
};

class BoxedwineData {
public:
    static void init(int argc, const char **argv);
    static void startApp();
    static void loadUI();

    static const std::vector<BoxedContainer*> getContainers() {return BoxedwineData::containers;}
    static void addContainer(BoxedContainer* container);
    static BoxedContainer* getContainerByDir(BString dir);
    static void reloadContainers();
    static const std::vector<BoxedWinVersion>& getWinVersions() { return BoxedwineData::winVersions; }
    static int getDefaultWindowsVersionIndex() {return 4;} // windows 7
    static BoxedWinVersion* getWinVersionFromName(BString name);
private:
    static void sortContainers();
    static void loadContainers();
    static std::vector<BoxedContainer*> containers;
    static std::vector<BoxedWinVersion> winVersions;
};
#endif