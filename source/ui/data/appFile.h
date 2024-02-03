#ifndef __APP_FILE_H__
#define __APP_FILE_H__

class BoxedTexture;
class BoxedContainer;
class BoxedApp;

class AppFile {
public:
    AppFile(BString name, BString installType, BString iconPath, BString filePath, U32 size, BString exe, BString exeOptions, BString help, BString optionsName, BString installOptions, BString installExe, const std::vector<BString>& args);
    BString name;
    BString optionsName;
    BString installType;
    BString filePath;
    BString iconPath;
    BString localIconPath;
    BString localFilePath;
    U32 size;
    BString exe;
    std::vector<BString> args;
    BString installExe;
    std::vector<BString> installOptions;
    std::vector<BString> exeOptions;
    BString help;

    std::shared_ptr<BoxedTexture> iconTexture;

    void buildIconTexture();
    void install(bool chooseShortCut=true, BoxedContainer* container=nullptr);    

    bool operator<(const AppFile& rhs) const { return name < rhs.name; }

private:
    static void runOptions(BoxedContainer* container, BoxedApp* app, const std::vector<BString>& options, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads);
    void install(bool chooseShortCut, BoxedContainer* container, std::list< std::function<bool() > >& runner, std::list<AppFile*>& downloads);
};

#endif