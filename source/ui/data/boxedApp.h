#ifndef __BOXED_APP_H__
#define __BOXED_APP_H__

class BoxedContainer;

class BoxedAppIcon {
public:
    BoxedAppIcon() : width(0), height(0), texture(NULL) {}
    BoxedAppIcon(void* texture, int width, int height) : width(width), height(height), texture(texture) {}

    int width;
    int height;
    void* texture;
};

class BoxedApp {
public:
    BoxedApp() : bpp(0), fullScreen(false), scale(0), scaleQuality(0), container(NULL) {}
    BoxedApp(const std::string& name, const std::string& path, const std::string& cmd, BoxedContainer* container) : name(name), path(path), cmd(cmd), bpp(0), fullScreen(false), scale(0), scaleQuality(0), container(container) {}
    
    bool load(BoxedContainer* container, const std::string& iniFilepath);

    const std::string& getName() {return this->name;}
    const std::string& getPath() {return this->path;}

    void launch();
    const BoxedAppIcon* getIconTexture(int iconSize=0);

    BoxedContainer* getContainer() {return this->container;}
    bool isLink() { return link.length()>0;}
    bool saveApp();
    void remove();

private:
    friend class BoxedContainer;
    friend class BoxedAppOptionsDialog;
    
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
    std::string glExt;
    int scale;
    int scaleQuality;

    BoxedContainer* container;
    std::string iniFilePath;
};

#endif
