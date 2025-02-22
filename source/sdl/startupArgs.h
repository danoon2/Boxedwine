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
    StartUpArgs() {
        workingDir = B("/home/username");
        sdlScaleQuality = B("0");
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
        
    bool euidSet = false;
    bool nozip = false;
        
    U32 pentiumLevel = 4;

    U32 rel_mouse_sensitivity = 0;        
    int pollRate = DEFAULT_POLL_RATE;

    int userId = UID;
    int groupId = GID;
    int effectiveUserId = UID;
    int effectiveGroupId = GID;

    bool soundEnabled = true;
    VideoOption videoOption = VIDEO_NORMAL;
    U32 vsync = VSYNC_DEFAULT;
    bool dpiAware = false;
    U32 skipFrameFPS = 0;
    static U32 uiType;
    bool readyToLaunch = false;
    U32 openGlType = OPENGL_TYPE_NOT_SET;
    bool ttyPrepend = false;
    BString showAppPickerForContainerDir;
    std::function<void()> runOnRestartUI;
    BString logPath;
    BString title;

    BString recordAutomation;
    BString runAutomation;

    BString ddrawOverridePath;
    bool enableDXVK = false;
    bool disableHideCursor = false;
    bool forceRelativeMouse = false;

private:
    bool workingDirSet = false;
    bool resolutionSet = false;

    BString workingDir;
    U32 screenCx = 800;
    U32 screenCy = 600;
    U32 screenBpp = 32;
    U32 sdlFullScreen = FULLSCREEN_NOTSET;
    BString glExt;
    int sdlScaleX = 100;
    int sdlScaleY = 100;
    BString sdlScaleQuality;
    std::vector<BString> args;
    BString root;
    std::vector<BString> zips;
    int cpuAffinity = 0;

    void buildVirtualFileSystem();
    int parse_resolution(const char *resolutionString, U32 *width, U32 *height);
};

#endif
