#ifndef __BOXED_APP_H__
#define __BOXED_APP_H__

class BoxedContainer;

class BoxedAppIcon {
public:
    BoxedAppIcon(const unsigned char* data, int width, int height);
    ~BoxedAppIcon();

    std::shared_ptr<BoxedTexture> texture;
    int getWidth() const {return width;}
    int getHeight() const {return height;}
private:
    int width;
    int height;
    const unsigned char* data;
};

class BoxedApp {
public:
    BoxedApp() : openGlType(OPENGL_TYPE_NOT_SET), bpp(32), fullScreen(FULLSCREEN_NOTSET), vsync(VSYNC_NOT_SET), dpiAware(false), showWindowImmediately(false), autoRefresh(false), scale(0), scaleQuality(0), cpuAffinity(0), pollRate(DEFAULT_POLL_RATE), skipFramesFPS(0), container(NULL) {}
    BoxedApp(BString name, BString path, BString cmd, BoxedContainer* container) : name(name), path(path), cmd(cmd), bpp(0), fullScreen(FULLSCREEN_NOTSET), vsync(VSYNC_NOT_SET), dpiAware(false), showWindowImmediately(false), autoRefresh(false), scale(0), scaleQuality(0), cpuAffinity(0), pollRate(DEFAULT_POLL_RATE), skipFramesFPS(0), container(container) {}
    
    bool load(BoxedContainer* container, BString iniFilepath);

    BString getName() {return this->name;}
    BString getPath() {return this->path;}
    BString getCmd() { return this->cmd;}
    BString getIniFilePath() {return this->iniFilePath;}

    void setName(BString name) {this->name = name;}
    void setArgs(const std::vector<BString>& args) {this->args = args;}
    void launch();
    void createAutomation();
    void runAutomation();
    bool hasAutomation();
    const BoxedAppIcon* getIconTexture(int iconSize=0);

    BoxedContainer* getContainer() {return this->container;}
    bool isLink() { return link.length()>0;}
    bool saveApp();
    void remove();

private:
    friend class BoxedContainer;
    friend class BoxedAppOptionsDialog;   
    friend class AppOptionsDlg;
    friend class ContainersView;
    friend class AppFile;

    BString name;
    BString path;
    std::unordered_map<int, BoxedAppIcon*> iconsBySize;
    BString iconPath;
    BString link;
    BString cmd;    
    std::vector<BString> args;
    U32 openGlType;

    // Boxedwine command line options
    BString resolution;
    int bpp;
    U32 fullScreen;
    U32 vsync;
    bool dpiAware;
    bool showWindowImmediately;
    bool autoRefresh;
    BString glExt;
    int scale;
    int scaleQuality;
    int cpuAffinity;
    int pollRate;
    int skipFramesFPS;
    
    BoxedContainer* container;
    BString iniFilePath;
};

#endif
