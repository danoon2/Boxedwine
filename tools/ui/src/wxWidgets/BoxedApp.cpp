#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include "BoxedApp.h"
#include "BoxedContainer.h"
#include "GlobalSettings.h"

bool BoxedApp::Load(BoxedContainer* container, const wxString& iniFilePath) {
    this->container = container;
    this->iniFilePath = iniFilePath;
    wxFileConfig *config = new wxFileConfig("", "", iniFilePath);
    this->name = config->Read("Name", "");
    this->link = config->Read("Link", "");
    this->cmd = config->Read("Cmd", "");
    this->icon = config->Read("Icon", "");
    this->path = config->Read("Path", "");
    bool result = this->name.Length()>0 && this->path.Length()>0 && (this->cmd.Length()>0 || this->link.Length()>0);
    delete config;
    return result;
}

bool BoxedApp::SaveApp() {
    if (this->iniFilePath.Length()==0) {
        wxString appDir = GlobalSettings::GetAppFolder(this->container);
        if (!wxDirExists(appDir)) {
            if (!wxMkdir(appDir)) {
                return false;
            }
        }
        for (int i=0;i<10000;i++) {
            wxString id = wxString::Format(wxT("%i"),i);
            wxString iniPath = appDir + wxFileName::GetPathSeparator() + id + ".ini";
            if (!wxFileExists(iniPath)) {
                this->iniFilePath = iniPath;
                break;
            }
        }
    }
    wxFileConfig *config = new wxFileConfig("", "", iniFilePath);
    config->Write("Name", this->name);
    config->Write("Link", this->link);
    config->Write("Cmd", this->cmd);
    config->Write("Icon", this->icon);
    config->Write("Path", this->path);
    config->Flush();
    delete config;
    return true;
}

void BoxedApp::Launch(bool showConsole) {
    wxString launchCmd;
    if (this->link.Length()>0) {
        launchCmd = "C:\\windows\\command\\start.exe /Unix \""+this->link+"\"";
    } else {
        launchCmd = "\""+this->cmd+"\"";
    }
    this->container->LaunchWine(launchCmd, this->path, showConsole);
}

wxIcon* BoxedApp::CreateIcon(int size) {
    if (this->icon.Length()) {    
        wxString path = GlobalSettings::GetRootFolder(this->container)+wxFileName::GetPathSeparator()+"home"+wxFileName::GetPathSeparator()+"username"+wxFileName::GetPathSeparator()+".local"+wxFileName::GetPathSeparator()+"share"+wxFileName::GetPathSeparator()+"icons"+wxFileName::GetPathSeparator()+"hicolor"+wxFileName::GetPathSeparator();
        if (size==32) {
            path+="32x32";
        } else if (size==48) {
            path+="48x48";
        } else {
            path+="32x32";
        }
        path=path+wxFileName::GetPathSeparator()+"apps"+wxFileName::GetPathSeparator()+this->icon;
        if (!wxFileExists(path)) {
            return NULL;
        }
        wxIcon* result = new wxIcon(path, wxBITMAP_TYPE_PNG, size, size);
        if (result->GetWidth()) {
            return result;
        }
        delete result;
    } else if (this->cmd.Length()) {
        // :TODO: will this work on non windows platforms, what about 16-bit exe's
        wxString path = GlobalSettings::GetRootFolder(this->container)+wxFileName(this->path).GetFullPath()+wxFileName::GetPathSeparator()+this->cmd;
        wxIcon* result = new wxIcon(path, wxBITMAP_TYPE_ICO, size, size);
        if (result->GetWidth()) {
            return result;
        }
        delete result;
    }
    return NULL;
}