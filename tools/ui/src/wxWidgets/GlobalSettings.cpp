#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/fs_zip.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/txtstrm.h>

#include "BoxedContainer.h"
#include "GlobalSettings.h"

wxString GlobalSettings::exeFileLocation;
wxString GlobalSettings::dataFolderLocation;
std::vector<WineVersion> GlobalSettings::wineVersions;

wxString GlobalSettings::GetWineNameFromFile(const wxString& fileName) {
    for (auto& ver : GlobalSettings::wineVersions) {
        if (ver.fileName.CmpNoCase(fileName)==0) {
            return ver.name;
        }
    }
    return fileName;
}

void GlobalSettings::InitWineVersions() {
    static bool initFS;
    if (!initFS) {
        wxFileSystem::AddHandler(new wxZipFSHandler);
        initFS = true;
    }

    wxString filename;
    wxDir dir(GlobalSettings::GetFileSystemFolder());
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