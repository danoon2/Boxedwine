#ifndef __STARTUP_ARGS_H__
#define __STARTUP_ARGS_H__

class MountInfo {
public:
    MountInfo(const std::string& localPath, const std::string& nativePath, bool wine) : localPath(localPath), nativePath(nativePath), wine(wine){}

    std::string localPath;
    std::string nativePath;
    bool wine;
};

class StartUpArgs {
public:
    StartUpArgs() : workingDirSet(false), resolutionSet(false), euidSet(false), nozip(false), sdlFullScreen(false), screenCx(800), screenCy(600), screenBpp(32), pentiumLevel(4), rel_mouse_sensitivity(0), sdlScaleX(100), sdlScaleY(100), sdlScaleQuality("0"), userId(UID), groupId(GID), effectiveUserId(UID), effectiveGroupId(GID), soundEnabled(true), videoEnabled(true), showStartingWindow(false) {
        workingDir = "/home/username";
    }
    bool parseStartupArgs(int argc, const char **argv);
    bool apply();

    std::string root;
    std::string workingDir;
    std::vector<MountInfo> mountInfo;
    std::vector<std::string> zips;
    std::vector<std::string> envValues;
    std::vector<std::string> nonExecFileFullPaths;
    std::string glExt;
    std::vector<std::string> args;

    bool workingDirSet;
    bool resolutionSet;
    bool euidSet;
    bool nozip;
    
    bool sdlFullScreen;
    U32 screenCx;
    U32 screenCy;
    U32 screenBpp;
    U32 pentiumLevel;

    U32 rel_mouse_sensitivity;
    int sdlScaleX;
    int sdlScaleY;
    std::string sdlScaleQuality;

    int userId = UID;
    int groupId = GID;
    int effectiveUserId = UID;
    int effectiveGroupId = GID;

    bool soundEnabled;
    bool videoEnabled;
    bool showStartingWindow;

private:
    void buildVirtualFileSystem();
    int parse_resolution(const char *resolutionString, U32 *width, U32 *height);
};

#endif