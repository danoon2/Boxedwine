#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/config.h"
#include "wx/fileconf.h"
#include "wx/icon.h"
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
    this->resolution = config->Read("Resolution","");
    this->bpp = config->Read("BPP",32);
    this->fullScreen = config->ReadBool("Fullscreen",false);
    this->glExt = config->Read("AllowedGlExt","");
    this->scale = config->Read("Scale",100);
    int defaultScaleQuality = 0;
    this->scaleQuality = config->Read("ScaleQuality",defaultScaleQuality);

    bool result = this->name.Length()>0 && (this->cmd.Length()>0 || this->link.Length()>0);
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
    config->Write("Resolution",this->resolution);
    config->Write("BPP",this->bpp);
    config->Write("Fullscreen",this->fullScreen);
    config->Write("AllowedGlExt",this->glExt);
    config->Write("Scale",this->scale);
    config->Write("ScaleQuality",this->scaleQuality);

    config->Flush();
    delete config;
    return true;
}

void BoxedApp::Launch(bool showConsole) {
    wxString launchCmd;
    wxString args;

    if (this->resolution.Length()) {
        args+="-resolution "+this->resolution+" ";
    }
    if (this->bpp) {
        args+="-bpp "+wxString::Format("%d", this->bpp)+" ";
    }
    if (this->fullScreen) {
        args+="-fullscreen ";
    }
    if (this->glExt.Length()) {
        args+="-glext \""+this->glExt+"\" ";
    }
    if (this->scale) {
        args+="-scale "+wxString::Format("%d", this->scale)+" ";
    }
    if (this->scaleQuality) {
        args+="-scale_quality "+wxString::Format("%d", this->scaleQuality)+" ";
    }
    if (this->link.Length()>0) {
        launchCmd = "C:\\windows\\command\\start.exe /Unix \""+this->link+"\"";
    } else {
        launchCmd = "\""+this->cmd+"\"";
    }
    this->container->LaunchWine(launchCmd, args, this->path, showConsole);
}

wxIcon* BoxedApp::CreateIcon(int size) {
    if (this->icon.Length()) {    
        wxString root = GlobalSettings::GetRootFolder(this->container)+wxFileName::GetPathSeparator()+"home"+wxFileName::GetPathSeparator()+"username"+wxFileName::GetPathSeparator()+".local"+wxFileName::GetPathSeparator()+"share"+wxFileName::GetPathSeparator()+"icons"+wxFileName::GetPathSeparator()+"hicolor"+wxFileName::GetPathSeparator();
        wxString path = root;
        if (size==32) {
            path+="32x32";
        } else if (size==48) {
            path+="48x48";
        } else {
            path+="32x32";
        }
        path=path+wxFileName::GetPathSeparator()+"apps"+wxFileName::GetPathSeparator()+this->icon;
        if (!wxFileExists(path)) {
            if (size==32) {
                return NULL;
            } else {
                path=root+"32x32"+wxFileName::GetPathSeparator()+"apps"+wxFileName::GetPathSeparator()+this->icon;
                if (!wxFileExists(path)) {
                    return NULL;
                }
            }            
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
        if (result->IsOk() && result->GetWidth()) {
            return result;
        }
        delete result;
    }
    return NULL;
}

void BoxedApp::Remove() {
    wxRemoveFile(this->iniFilePath);
    this->container->DeleteApp(this);
    delete this;
}
