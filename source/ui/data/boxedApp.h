#ifndef __BOXED_APP_H__
#define __BOXED_APP_H__

class BoxedContainer;

class BoxedApp {
public:
    BoxedApp() : bpp(0), fullScreen(false), scale(0), scaleQuality(0), container(NULL) {}
    BoxedApp(const std::string& name, const std::string& path, const std::string& cmd, BoxedContainer* container) : name(name), path(path), cmd(cmd), bpp(0), fullScreen(false), scale(0), scaleQuality(0), container(container) {}
    
    bool load(BoxedContainer* container, const std::string& iniFilepath);

    const std::string& getName() {return this->name;}
    const std::string& getPath() {return this->path;}

    void launch();
    std::string getIcon();
    BoxedContainer* getContainer() {return this->container;}
    bool isLink() { return link.length()>0;}
    bool saveApp();
    void remove();

private:
    friend class BoxedContainer;
    friend class BoxedAppOptionsDialog;
    
    std::string name;
    std::string path;
    std::string icon;
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
