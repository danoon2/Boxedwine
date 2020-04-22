#ifndef __APP_FILE_H__
#define __APP_FILE_H__

class AppFile {
public:
    AppFile(const std::string& name, const std::string& installType, const std::string& iconPath, const std::string& filePath, U32 size, const std::string& exe, const std::string& exeOptions, const std::string& help);
    std::string name;
    std::string installType;
    std::string filePath;
    std::string iconPath;
    std::string localIconPath;
    std::string localFilePath;
    U32 size;
    std::string exe;
    std::string exeOptions;
    std::string help;
    void* iconTexture;

    void buildIconTexture();
    void install();

    bool operator<(const AppFile& rhs) const { return name < rhs.name; }
};

#endif