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
    BoxedApp() : bpp(32), fullScreen(false), dpiAware(false), scale(0), scaleQuality(0), cpuAffinity(0), container(NULL) {}
    BoxedApp(const std::string& name, const std::string& path, const std::string& cmd, BoxedContainer* container) : name(name), path(path), cmd(cmd), bpp(0), fullScreen(false), dpiAware(false), scale(0), scaleQuality(0), cpuAffinity(0), container(container) {}
    
    bool load(BoxedContainer* container, const std::string& iniFilepath);

    const std::string& getName() {return this->name;}
    const std::string& getPath() {return this->path;}
    const std::string& getCmd() { return this->cmd;}
    const std::string& getIniFilePath() {return this->iniFilePath;}

    void setName(const std::string& name) {this->name = name;}

    void launch();
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

    std::string name;
    std::string path;
    std::unordered_map<int, BoxedAppIcon*> iconsBySize;
    std::string iconPath;
    std::string link;
    std::string cmd;    
    std::vector<std::string> args;

    // Boxedwine command line options
    std::string resolution;
    int bpp;
    bool fullScreen;
    bool dpiAware;
    std::string glExt;
    int scale;
    int scaleQuality;
    int cpuAffinity;

    BoxedContainer* container;
    std::string iniFilePath;
};

#endif
