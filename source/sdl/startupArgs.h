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
    MountInfo(const std::string& localPath, const std::string& nativePath, bool wine) : localPath(localPath), nativePath(nativePath), wine(wine){}

    std::string getFullLocalPath() {
        if (this->wine) {
            return "/mnt/drive_" + this->localPath;
        }
        return this->localPath;
    }

    std::string localPath;
    std::string nativePath;
    bool wine;
};

class StartUpArgs {
public:
    StartUpArgs() : euidSet(false), nozip(false), pentiumLevel(4), rel_mouse_sensitivity(0), pollRate(DEFAULT_POLL_RATE), userId(UID), groupId(GID), effectiveUserId(UID), effectiveGroupId(GID), soundEnabled(true), videoEnabled(true), vsync(VSYNC_DEFAULT), dpiAware(false), showWindowImmediately(false), skipFrameFPS(0), readyToLaunch(false), openGlType(OPENGL_TYPE_NOT_SET), ttyPrepend(false), workingDirSet(false), resolutionSet(false), screenCx(800), screenCy(600), screenBpp(32), sdlFullScreen(FULLSCREEN_NOTSET), sdlScaleX(100), sdlScaleY(100), sdlScaleQuality("0"), cpuAffinity(0) {
        workingDir = "/home/username";        
    }
    bool loadDefaultResource(const char* app);
    bool parseStartupArgs(int argc, const char **argv);
    bool apply();
    bool shouldStartUI() {return this->args.size()==0;}

    void setWorkingDir(const std::string& path) {this->workingDir = path; this->workingDirSet=true;}
    void setResolution(const std::string& path);
    void setBpp(U32 bpp) {this->screenBpp = bpp;}
    void setFullscreen(U32 fullScreen) {this->sdlFullScreen = fullScreen;}
    void setAllowedGlExtension(const std::string& glExt) {this->glExt = glExt;}
    void setScale(int scale) {this->sdlScaleX = scale; this->sdlScaleY = scale;}
    void setVsync(int vsync) { this->vsync = vsync; }
    void setScaleQuality(const std::string& scaleQuality) {this->sdlScaleQuality = scaleQuality;}
    void addArg(const std::string& arg) {this->args.push_back(arg);}
    void addArgs(const std::vector<std::string>& args) {this->args.insert(this->args.end(), args.begin(), args.end());}
    void addZip(const std::string& zip) {this->zips.push_back(zip);}
    void setRoot(const std::string& root) {this->root = root;}
    void setCpuAffinity(int affinity) {this->cpuAffinity = affinity;}

    std::vector<std::string> buildArgs();

    std::vector<MountInfo> mountInfo;    
    std::vector<std::string> envValues;
    std::vector<std::string> nonExecFileFullPaths;        
        
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
    bool showWindowImmediately;
    U32 skipFrameFPS;
    static U32 uiType;
    bool readyToLaunch;
    U32 openGlType;
    bool ttyPrepend;
    std::string showAppPickerForContainerDir;
    std::function<void()> runOnRestartUI;
    std::string logPath;
    std::string title;

    std::string recordAutomation;
    std::string runAutomation;

private:
    bool workingDirSet;
    bool resolutionSet;    

    std::string workingDir;
    U32 screenCx;
    U32 screenCy;
    U32 screenBpp;
    U32 sdlFullScreen;
    std::string glExt;
    int sdlScaleX;
    int sdlScaleY;
    std::string sdlScaleQuality;
    std::vector<std::string> args;
    std::string root;
    std::vector<std::string> zips;
    int cpuAffinity;

    void buildVirtualFileSystem();
    int parse_resolution(const char *resolutionString, U32 *width, U32 *height);
};

#endif
