#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/filename.h"
#include "wx/fs_zip.h"
#include "wx/zipstrm.h"
#include "wx/wfstream.h"
#include "wx/dir.h"
#include "wx/txtstrm.h"
#include "wx/stdpaths.h"

#include "BoxedContainer.h"
#include "GlobalSettings.h"

wxString GlobalSettings::exeFileLocation;
wxString GlobalSettings::dataFolderLocation;
std::vector<WineVersion> GlobalSettings::wineVersions;
int GlobalSettings::iconSize;
double GlobalSettings::scaleFactor;

wxString GlobalSettings::GetFileFromWineName(const wxString& name) {
    for (auto& ver : GlobalSettings::wineVersions) {
        if (ver.name.CmpNoCase(name)==0) {
            return ver.fileName;
        }
    }
    if (GlobalSettings::wineVersions.size()) {
        return GlobalSettings::wineVersions[0].fileName;
    }
    return "";
}

void lookForFileSystems(const wxString& path) {
    wxString filename;
    wxDir dir(path);
    if (dir.IsOpened()) {
        bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
        while(cont)
        {
            if (filename.Lower().EndsWith(".zip")) {
                wxString zipPath = GlobalSettings::GetFileSystemFolder()+wxFileName::GetPathSeparator()+filename+"#zip:wineVersion.txt";

                wxFileSystem fs;
                wxFSFile *zip = fs.OpenFile(zipPath);
                if(zip!=NULL)
                {
                    wxInputStream *in = zip->GetStream();
                    if ( in != NULL )
                    {
                        wxTextInputStream text(*in);
                        wxString wineName = text.ReadLine();
                        GlobalSettings::wineVersions.push_back(WineVersion(wineName, filename));
                    }
                    delete zip;
                }
            }
            cont = dir.GetNext(&filename);
        }
    }
}

void GlobalSettings::InitWineVersions() {
    static bool initFS;
    if (!initFS) {
        wxFileSystem::AddHandler(new wxZipFSHandler);
        initFS = true;
    }

    lookForFileSystems(GlobalSettings::GetFileSystemFolder());
    lookForFileSystems(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath());
    lookForFileSystems(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + wxFileName::GetPathSeparator() + ".." +  wxFileName::GetPathSeparator() + "FileSystems");
}

wxString GlobalSettings::GetContainerFolder() {
    return GlobalSettings::dataFolderLocation + wxFileName::GetPathSeparator() + "Containers";
}

wxString GlobalSettings::GetFileSystemFolder() {
    return GlobalSettings::dataFolderLocation + wxFileName::GetPathSeparator() + "FileSystems";
}

wxString GlobalSettings::GetFileSystemZip(const wxString& zipName) {
    return GlobalSettings::dataFolderLocation + wxFileName::GetPathSeparator() + "FileSystems" + wxFileName::GetPathSeparator() + zipName;
}

wxString GlobalSettings::GetRootFolder(BoxedContainer* container) {
    return container->dirPath + wxFileName::GetPathSeparator() + "root";
}

wxString GlobalSettings::GetAppFolder(BoxedContainer* container) {
    return container->dirPath + wxFileName::GetPathSeparator() + "apps";
}

double GlobalSettings::GetScaleFactor() {    
    return scaleFactor;
}
