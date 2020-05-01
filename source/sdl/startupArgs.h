#ifndef __STARTUP_ARGS_H__
#define __STARTUP_ARGS_H__

#define UI_TYPE_UNSET 0
#define UI_TYPE_OPENGL 1
#define UI_TYPE_DX9 2

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
    StartUpArgs() : euidSet(false), nozip(false), pentiumLevel(4), rel_mouse_sensitivity(0), userId(UID), groupId(GID), effectiveUserId(UID), effectiveGroupId(GID), soundEnabled(true), videoEnabled(true), dpiAware(false), readyToLaunch(false), workingDirSet(false), resolutionSet(false), screenCx(800), screenCy(600), screenBpp(32), sdlFullScreen(false), sdlScaleX(100), sdlScaleY(100), sdlScaleQuality("0") {
        workingDir = "/home/username";        
    }
    bool parseStartupArgs(int argc, const char **argv);
    bool apply();
    bool shouldStartUI() {return this->args.size()==0;}

    void setWorkingDir(const std::string& path) {this->workingDir = path; this->workingDirSet=true;}
    void setResolution(const std::string& path);
    void setBpp(U32 bpp) {this->screenBpp = bpp;}
    void setFullscreen() {this->sdlFullScreen = true;}
    void setAllowedGlExtension(const std::string& glExt) {this->glExt = glExt;}
    void setScale(int scale) {this->sdlScaleX = scale; this->sdlScaleY = scale;}
    void setScaleQuality(const std::string& scaleQuality) {this->sdlScaleQuality = scaleQuality;}
    void addArg(const std::string& arg) {this->args.push_back(arg);}
    void addArgs(const std::vector<std::string>& args) {this->args.insert(this->args.end(), args.begin(), args.end());}
    void addZip(const std::string& zip) {this->zips.push_back(zip);}
    void setRoot(const std::string& root) {this->root = root;}
    std::vector<std::string> buildArgs();

    std::vector<MountInfo> mountInfo;    
    std::vector<std::string> envValues;
    std::vector<std::string> nonExecFileFullPaths;        
        
    bool euidSet;
    bool nozip;
        
    U32 pentiumLevel;

    U32 rel_mouse_sensitivity;        

    int userId;
    int groupId;
    int effectiveUserId;
    int effectiveGroupId;

    bool soundEnabled;
    bool videoEnabled;
    bool dpiAware;
    static U32 uiType;
    bool readyToLaunch;
    std::string showAppPickerForContainerDir;
    std::function<void()> runOnRestartUI;
    std::string logPath;
private:
    bool workingDirSet;
    bool resolutionSet;    

    std::string workingDir;
    U32 screenCx;
    U32 screenCy;
    U32 screenBpp;
    bool sdlFullScreen;
    std::string glExt;
    int sdlScaleX;
    int sdlScaleY;
    std::string sdlScaleQuality;
    std::vector<std::string> args;
    std::string root;
    std::vector<std::string> zips;

    void buildVirtualFileSystem();
    int parse_resolution(const char *resolutionString, U32 *width, U32 *height);
};

#endif