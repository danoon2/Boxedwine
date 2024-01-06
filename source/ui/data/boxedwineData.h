#ifndef __BOXEDWINE_DATA_H__
#define __BOXEDWINE_DATA_H__

#define VER_PLATFORM_WIN32s                     0
#define VER_PLATFORM_WIN32_WINDOWS              1
#define VER_PLATFORM_WIN32_NT                   2

class BoxedContainer;

class BoxedWinVersion {
public:
    BoxedWinVersion(BString ver, BString desc, U32 major, U32 minor, U32 build, U32 id, BString csd, U16 majorPack, U16 minorPack, BString product);

    BString szVersion;
    BString szDescription;
    U32       dwMajorVersion;
    U32       dwMinorVersion;
    U32       dwBuildNumber;
    U32       dwPlatformId;
    BString szCSDVersion;
    U16        wServicePackMajor;
    U16        wServicePackMinor;
    BString szProductType;
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