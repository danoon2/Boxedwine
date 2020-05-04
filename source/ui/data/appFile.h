#ifndef __APP_FILE_H__
#define __APP_FILE_H__

class BoxedTexture;
class BoxedContainer;
class BoxedApp;

class AppFile {
public:
    AppFile(const std::string& name, const std::string& installType, const std::string& iconPath, const std::string& filePath, U32 size, const std::string& exe, const std::string& exeOptions, const std::string& help, const std::string& optionsName, const std::string& installOptions, const std::string& installExe);
    std::string name;
    std::string optionsName;
    std::string installType;
    std::string filePath;
    std::string iconPath;
    std::string localIconPath;
    std::string localFilePath;
    U32 size;
    std::string exe;
    std::string installExe;
    std::vector<std::string> installOptions;
    std::vector<std::string> exeOptions;
    std::string help;

    std::shared_ptr<BoxedTexture> iconTexture;

    void buildIconTexture();
    void install(bool chooseShortCut=true, BoxedContainer* container=NULL);    

    bool operator<(const AppFile& rhs) const { return name < rhs.name; }

private:
    static void runOptions(BoxedContainer* container, BoxedApp* app, const std::vector<std::string>& options, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads);
    void install(bool chooseShortCut, BoxedContainer* container, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads);
};

#endif