#ifndef __GLOBAL_SETTINGS_H__
#define __GLOBAL_SETTINGS_H__

class BoxedContainer;

class WineVersion {
public:
    WineVersion(const wxString& name, const wxString& fileName):name(name),fileName(fileName){}
    wxString name;
    wxString fileName;
};

class GlobalSettings {
public:
    static void InitWineVersions();
    static wxString GetFileFromWineName(const wxString& name);

    static wxString exeFileLocation;
    static wxString dataFolderLocation;

    static wxString GetContainerFolder();
    static wxString GetFileSystemFolder();    
    static wxString GetFileSystemZip(const wxString& zipName);
    static wxString GetAppFolder(BoxedContainer* container);
    static wxString GetRootFolder(BoxedContainer* container);

    static int iconSize;
    static double scaleFactor;

    static double GetScaleFactor();
    static std::vector<WineVersion> wineVersions;

};

#endif