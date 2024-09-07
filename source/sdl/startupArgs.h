#ifndef __STARTUP_ARGS_H__
#define __STARTUP_ARGS_H__

#define UI_TYPE_UNSET 0
#define UI_TYPE_OPENGL 1
#define UI_TYPE_DX9 2

#define FULLSCREEN_NOTSET 0
#define FULLSCREEN_STRETCH 1
#define FULLSCREEN_ASPECT 2

#define VSYNC_NOT_SET -1
#define VSYNC_DISABLED 0
#define VSYNC_ENABLED 1
#define VSYNC_ADAPTIVE 2
#define VSYNC_DEFAULT VSYNC_DISABLED

class MountInfo {
public:
    MountInfo(BString localPath, BString nativePath, bool wine) : localPath(localPath), nativePath(nativePath), wine(wine){}

    BString getFullLocalPath() {
        if (this->wine) {
            return "/mnt/drive_" + this->localPath;
        }
        return this->localPath;
    }

    BString localPath;
    BString nativePath;
    bool wine;
};

class StartUpArgs {
public:
    StartUpArgs() : euidSet(false), nozip(false), pentiumLevel(4), rel_mouse_sensitivity(0), pollRate(DEFAULT_POLL_RATE), userId(UID), groupId(GID), effectiveUserId(UID), effectiveGroupId(GID), soundEnabled(true), videoEnabled(true), vsync(VSYNC_DEFAULT), dpiAware(false), skipFrameFPS(0), readyToLaunch(false), openGlType(OPENGL_TYPE_NOT_SET), ttyPrepend(false), workingDirSet(false), resolutionSet(false), screenCx(800), screenCy(600), screenBpp(32), sdlFullScreen(FULLSCREEN_NOTSET), sdlScaleX(100), sdlScaleY(100), sdlScaleQuality(B("0")), cpuAffinity(0) {
        workingDir = B("/home/username");
    }
    bool loadDefaultResource(const char* app);
    bool parseStartupArgs(int argc, const char **argv);
    bool apply();
    bool shouldStartUI() {return this->args.size()==0;}

    void setWorkingDir(BString path) {this->workingDir = path; this->workingDirSet=true;}
    void setResolution(BString path);
    void setBpp(U32 bpp) {this->screenBpp = bpp;}
    void setFullscreen(U32 fullScreen) {this->sdlFullScreen = fullScreen;}
    void setAllowedGlExtension(BString glExt) {this->glExt = glExt;}
    void setScale(int scale) {this->sdlScaleX = scale; this->sdlScaleY = scale;}
    void setVsync(int vsync) { this->vsync = vsync; }
    void setScaleQuality(BString scaleQuality) {this->sdlScaleQuality = scaleQuality;}
    void addArg(BString arg) {this->args.push_back(arg);}
    void addArgs(const std::vector<BString>& args) {this->args.insert(this->args.end(), args.begin(), args.end());}
    void addZip(BString zip) {this->zips.push_back(zip);}
    void setRoot(BString root) {this->root = root;}
    void setCpuAffinity(int affinity) {this->cpuAffinity = affinity;}

    std::vector<BString> buildArgs();

    std::vector<MountInfo> mountInfo;    
    std::vector<BString> envValues;
    std::vector<BString> nonExecFileFullPaths;
        
    bool euidSet;
    bool nozip;
        
    U32 pentiumLevel;

    U32 rel_mouse_sensitivity;        
    int pollRate;

    int userId;
    int groupId;
    int effectiveUserId;
    int effectiveGroupId;

    bool soundEnabled;
    bool videoEnabled;
    U32 vsync;
    bool dpiAware;
    U32 skipFrameFPS;
    static U32 uiType;
    bool readyToLaunch;
    U32 openGlType;
    bool ttyPrepend;
    BString showAppPickerForContainerDir;
    std::function<void()> runOnRestartUI;
    BString logPath;
    BString title;

    BString recordAutomation;
    BString runAutomation;

private:
    bool workingDirSet;
    bool resolutionSet;    

    BString workingDir;
    U32 screenCx;
    U32 screenCy;
    U32 screenBpp;
    U32 sdlFullScreen;
    BString glExt;
    int sdlScaleX;
    int sdlScaleY;
    BString sdlScaleQuality;
    std::vector<BString> args;
    BString root;
    std::vector<BString> zips;
    int cpuAffinity;

    void buildVirtualFileSystem();
    int parse_resolution(const char *resolutionString, U32 *width, U32 *height);
};

#endif
