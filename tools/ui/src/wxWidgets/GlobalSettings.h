#ifndef __GLOBAL_SETTINGS_H__
#define __GLOBAL_SETTINGS_H__

class BoxedContainer;

class WineVersion {
public:
    WineVersion(const wxString& name, const wxString& filePath):name(name),filePath(filePath){}
    wxString name;
    wxString filePath;
};

class GlobalSettings {
public:
    static void InitWineVersions();
    static wxString GetFileFromWineName(const wxString& name);

    static wxString exeFileLocation;
    static wxString dataFolderLocation;

    static wxString GetContainerFolder();
    static wxString GetFileSystemFolder();    
    static wxString GetAppFolder(BoxedContainer* container);
    static wxString GetRootFolder(BoxedContainer* container);

    static int iconSize;
    static double scaleFactor;

    static double GetScaleFactor();
    static std::vector<WineVersion> wineVersions;

};

#endif