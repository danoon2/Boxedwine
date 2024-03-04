#ifndef __BOXED_APP_H__
#define __BOXED_APP_H__

class BoxedContainer;

class BoxedAppIcon {
public:
    BoxedAppIcon(std::shared_ptr<U8[]> data, int width, int height);
    ~BoxedAppIcon();

    std::shared_ptr<BoxedTexture> texture;
    int getWidth() const {return width;}
    int getHeight() const {return height;}
private:
    int width;
    int height;
    std::shared_ptr<U8[]> data;
};

class BoxedApp {
public:
    BoxedApp() = default;
    BoxedApp(BString name, BString path, BString cmd, BoxedContainer* container) : name(name), path(path), cmd(cmd) {}
    
    bool load(BoxedContainer* container, BString iniFilepath);

    BString getName() const {return this->name;}
    BString getPath() const {return this->path;}
    BString getCmd() const { return this->cmd;}
    BString getIniFilePath() const {return this->iniFilePath;}

    void setName(BString name) {this->name = name;}
    void setArgs(const std::vector<BString>& args) {this->args = args;}
    void launch();
    void createAutomation();
    void runAutomation();
    bool hasAutomation();
    const BoxedAppIcon* getIconTexture(int iconSize=0);

    BoxedContainer* getContainer() {return this->container;}
    bool isLink() { return link.length()>0; }
    bool usingWine() { return this->isWine; }
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
    BHashTable<int, BoxedAppIcon*> iconsBySize;
    BString iconPath;
    BString link;
    BString cmd;    
    std::vector<BString> args;
    U32 openGlType = OPENGL_TYPE_NOT_SET;

    // Boxedwine command line options
    BString resolution;
    int bpp = 32;
    U32 fullScreen = FULLSCREEN_NOTSET;
    U32 vsync = VSYNC_NOT_SET;
    bool dpiAware = false;
    bool showWindowImmediately = false;
    bool autoRefresh = false;
    BString glExt;
    int scale = 0;
    int scaleQuality = 0;
    int cpuAffinity = 0;
    int pollRate = DEFAULT_POLL_RATE;
    int skipFramesFPS = 0;
    int uid = -1;
    int euid = -1;
    bool isWine = true;

    BoxedContainer* container = nullptr;
    BString iniFilePath;
};

#endif
