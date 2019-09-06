#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include "BoxedApp.h"
#include "BoxedContainer.h"

bool BoxedApp::Load(BoxedContainer* container, const wxString& root, const wxString& iniFilePath) {
    this->root = root;
    this->container = container;
    wxFileConfig *config = new wxFileConfig("", "", iniFilePath);
    this->name = config->Read("Name", "");
    this->link = config->Read("Link", "");
    this->cmd = config->Read("Cmd", "");
    this->icon = config->Read("Icon", "");
    this->path = config->Read("Path", "");
    return this->name.Length()>0 && this->path.Length()>0 && this->icon.Length()>0 && (this->cmd.Length()>0 || this->link.Length()>0);
}

void BoxedApp::Launch(bool showConsole) {
    wxString launchCmd;
    if (this->link.Length()>0) {
        launchCmd = "C:\\windows\\command\\start.exe /Unix \""+this->link+"\"";
    } else {
        launchCmd = this->cmd;
    }
    this->container->LaunchWine(launchCmd, this->path, showConsole);
}

wxIcon* BoxedApp::CreateIcon(int size) {
    wxString path = this->root+wxFileName::GetPathSeparator()+"home"+wxFileName::GetPathSeparator()+"username"+wxFileName::GetPathSeparator()+".local"+wxFileName::GetPathSeparator()+"share"+wxFileName::GetPathSeparator()+"icons"+wxFileName::GetPathSeparator()+"hicolor"+wxFileName::GetPathSeparator();
    if (size==32) {
        path+="32x32";
    } else if (size==48) {
        path+="48x48";
    } else {
        path+="32x32";
    }
    path=path+wxFileName::GetPathSeparator()+"apps"+wxFileName::GetPathSeparator()+this->icon;
    wxIcon* result = new wxIcon(path, wxBITMAP_TYPE_PNG, size, size);
    if (result->GetWidth()) {
        return result;
    }
    delete result;
    return NULL;
}