#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/config.h"
#include "wx/fileconf.h"
#include "wx/dir.h"
#include "wx/txtstrm.h"
#include "BoxedContainer.h"
#include <algorithm>
#include "GlobalSettings.h"
#include "wxUtils.h"

bool BoxedContainer::Load(const wxString& dirPath) {
    this->dirPath = dirPath;
    wxString iniFilePath = this->dirPath + wxFileName::GetPathSeparator() + "container.ini";
    wxFileConfig *config = new wxFileConfig("", "", iniFilePath);
    this->name = config->Read("Name", "");
    this->wineVersion = config->Read("WineVersion", "");
    delete config;
    bool result = this->name.Length()>0 && this->wineVersion.Length()>0;
    if (result) {
        this->LoadApps();
    }
    return result;
}

BoxedContainer::~BoxedContainer() {
    for (auto& app : this->apps) {
        delete app;
    }
}

BoxedContainer* BoxedContainer::CreateContainer(const wxString& dirPath, const wxString& name, const wxString& wineVersion) {
    wxString ini_filename = dirPath + wxFileName::GetPathSeparator() + "container.ini";
    wxFileConfig *config = new wxFileConfig( "", "", ini_filename);
    config->Write("Name", name);
    config->Write("WineVersion", wineVersion);
    config->Flush();
    delete config;
    BoxedContainer* container = new BoxedContainer();
    container->name = name;
    container->wineVersion = wineVersion;
    container->dirPath = dirPath;
    return container;
}

bool BoxedContainer::SaveContainer() {
    wxString iniFilePath = dirPath + wxFileName::GetPathSeparator() + "container.ini";
    wxFileConfig *config = new wxFileConfig("", "", iniFilePath);
    config->Write("Name", this->name);
    config->Write("WineVersion", this->wineVersion);
    config->Flush();
    delete config;
    return true;
}

void BoxedContainer::DeleteContainerFromFilesystem() {
    wxFileName::Rmdir(this->dirPath, wxPATH_RMDIR_RECURSIVE);
}

void BoxedContainer::Reload() {
    this->LoadApps();
}

void BoxedContainer::LoadApps() {
    wxString filename;
    wxString appDirPath = GlobalSettings::GetAppFolder(this);
    wxDir dir(appDirPath);
    this->apps.clear();
    if (dir.IsOpened()) {
        bool cont = dir.GetFirst(&filename, "*.ini", wxDIR_FILES);
        while(cont)
        {
            wxString iniFilePath = appDirPath+wxFileName::GetPathSeparator()+filename;
            BoxedApp* app = new BoxedApp();
            if (app->Load(this, iniFilePath)) {
                this->apps.push_back(app);
            } else {
                delete app;
            }
            cont = dir.GetNext(&filename);
        }
    }
}

void BoxedContainer::DeleteApp(BoxedApp* app) {
    std::vector<BoxedApp*>::iterator position = std::find(this->apps.begin(), this->apps.end(), app);
    if (position != this->apps.end()) {
        this->apps.erase(position);
    }
}

void BoxedContainer::Launch(const wxString& cmd, const wxString& args, const wxString& path, bool showConsole, bool async) {
    if (this->currentProcess && this->currentProcess->isRunning) {
        wxString msg = "The container, "+this->name + ", is already running.  It is not safe to run two separate Boxedwine instances against the same directory.";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
        return;
    }
    if (this->currentProcess) {
        delete this->currentProcess;
    }

    this->currentProcess = new BoxedContainerProcess();
    if (async) {
        this->currentProcess->isRunning = true;
    }
    wxString zip = GlobalSettings::GetFileFromWineName(this->wineVersion);
    wxString root = GlobalSettings::GetRootFolder(this);

    if (!wxDirExists(root)) {
        wxMkdir(root);
    }
    wxString launchCmd = "\""+GlobalSettings::exeFileLocation+"\" -showStartupWindow -root \""+root+"\" -zip \""+zip+"\" -w \""+path+"\" "+args;
    if (!launchCmd.EndsWith(" ")) {
        launchCmd=launchCmd+" ";
    }
    launchCmd+=cmd;

    // wxEXEC_HIDE_CONSOLE causes /bin/wine winecfg to not show a window 
    long result = wxExecute(launchCmd, (showConsole?0:wxEXEC_HIDE_CONSOLE)|(async?wxEXEC_ASYNC:wxEXEC_SYNC), this->currentProcess);
    if ((async && result==0) || (!async && result==-1)) {        
        this->currentProcess->isRunning = false;
        wxString msg = "Failed to launch container, "+this->name + ", with the command, "+launchCmd+".  Most likely it is because the Boxedwine executable could not be found";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
        return;
    }    
}

void BoxedContainer::LaunchWine(const wxString& cmd, const wxString& args, const wxString& path, bool showConsole, bool async) {
    Launch("/bin/wine "+cmd+"", args, path, showConsole, async);
}

void BoxedContainer::OnClose() {
    if (this->currentProcess && this->currentProcess->isRunning) {
        this->currentProcess->CloseOutput();
    }
}

void BoxedContainer::GetNewApps(std::vector<BoxedApp>& apps) {
    GetDesktopApps(apps); 
    GetExeApps(apps);
}

void BoxedContainer::GetExeApps(std::vector<BoxedApp>& apps) {
    wxString root = GlobalSettings::GetRootFolder(this);
    wxString path = root + wxFileName::GetPathSeparator() + "home" + wxFileName::GetPathSeparator() + "username" + wxFileName::GetPathSeparator() + ".wine" + wxFileName::GetPathSeparator() + "drive_c";
    wxArrayString results;

    wxDir::GetAllFiles(path, &results, "*.exe");
    wxDir::GetAllFiles(path, &results, "*.EXE");
    wxDir::GetAllFiles(path, &results, "*.Exe");

    results = wxUtilRemoveDuplicates(results);
    for (int i=0;i<results.Count();i++) {
        BoxedApp app;
        wxFileName fileName(results[i].SubString(root.Length(), results[i].Length()));

        app.container = this;
        app.name = fileName.GetFullName();
        app.path = fileName.GetPath(0, wxPATH_UNIX);
        app.cmd = app.name;

        bool found = false;
        for (auto& a : this->apps) {
            if (a->cmd==app.cmd) {
                found = true;
                break;
            }
        }
        if (!found) {
            apps.push_back(app);
        }
    }
}

void BoxedContainer::GetDesktopApps(std::vector<BoxedApp>& apps) {
    wxString path = this->dirPath + wxFileName::GetPathSeparator() + "root" + wxFileName::GetPathSeparator() + "home" + wxFileName::GetPathSeparator() + "username" + wxFileName::GetPathSeparator() + ".local" + wxFileName::GetPathSeparator() + "share" + wxFileName::GetPathSeparator() + "applications" + wxFileName::GetPathSeparator() + "wine";
    wxArrayString results;

    wxDir::GetAllFiles(path, &results, "*.desktop");
    for (int i=0;i<results.Count();i++) {
        wxFileConfig *config = new wxFileConfig("", "", results[i]);
        BoxedApp app;
        app.container = this;
        app.name = config->Read("Desktop Entry/Name", "");
        app.path = config->Read("Desktop Entry/Path", "");
        app.link = config->Read("Desktop Entry/Exec", "");
        if (app.link.Length()==0) {
            continue;
        } else {
            int pos = app.link.Find("/Unix");
            if (pos>0) {
                app.link = app.link.SubString(pos+6, app.link.Length());
                app.link.Replace("\\", "");
            }
        }
        app.icon = config->Read("Desktop Entry/Icon", "");
        if (app.icon.Length()) {
            app.icon+=".png";
        }
        delete config;
        bool found = false;
        for (auto& a : this->apps) {
            if (a->link==app.link) {
                found = true;
                break;
            }
        }
        if (!found) {
            apps.push_back(app);
        }
    }
}
