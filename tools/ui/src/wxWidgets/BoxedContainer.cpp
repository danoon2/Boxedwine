#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/dir.h>
#include <wx/txtstrm.h>
#include "BoxedContainer.h"
#include <algorithm>
#include "GlobalSettings.h"

bool BoxedContainer::Load(const wxString& dirPath) {
    this->dirPath = dirPath;
    wxString iniFilePath = this->dirPath + wxFileName::GetPathSeparator() + "container.ini";
    wxFileConfig *config = new wxFileConfig("", "", iniFilePath);
    this->name = config->Read("Name", "");
    this->fileSystem = config->Read("FileSystem", "");
    bool result = this->name.Length()>0 && this->fileSystem.Length()>0;
    if (result) {
        this->LoadApps();
    }
    return result;
}

void BoxedContainer::LoadApps() {
    wxString filename;
    wxString appDirPath = GlobalSettings::GetAppFolder(this);
    wxString rootDirPath = GlobalSettings::GetRootFolder(this);
    wxDir dir(appDirPath);
    bool cont = dir.GetFirst(&filename, "*.ini", wxDIR_FILES);
    while(cont)
    {
        wxString iniFilePath = appDirPath+wxFileName::GetPathSeparator()+filename;
        BoxedApp* app = new BoxedApp();
        if (app->Load(this, rootDirPath, iniFilePath)) {
            this->apps.push_back(app);
        } else {
            delete app;
        }
        cont = dir.GetNext(&filename);
    }
}

void BoxedContainer::DeleteApp(BoxedApp* app) {
    std::vector<BoxedApp*>::iterator position = std::find(this->apps.begin(), this->apps.end(), app);
    if (position != this->apps.end()) {
        this->apps.erase(position);
    }
}

void BoxedContainer::Launch(const wxString& cmd, const wxString& path, bool showConsole) {
    if (this->currentProcess && this->currentProcess->isRunning) {
        wxString msg = "The container, "+this->name + ", is already running.  It is not safe to run two separate Boxedwine instances against the same directory.";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
        return;
    }
    if (this->currentProcess) {
        delete this->currentProcess;
    }

    wxString title = this->name + " Log";
    //this->logWindow = new BoxedLogDlg(NULL, title);

    this->currentProcess = new BoxedContainerProcess();

    wxString zip = GlobalSettings::GetFileSystemZip(this->fileSystem);
    wxString root = GlobalSettings::GetRootFolder(this);

    wxString launchCmd = "\""+GlobalSettings::exeFileLocation+"\" -root \""+root+"\" -zip \""+zip+"\" -w \""+path+"\" "+cmd;
    // wxEXEC_HIDE_CONSOLE causes /usr/bin/wine winecfg to not show a window 
    if (wxExecute(launchCmd, (showConsole?wxEXEC_HIDE_CONSOLE:wxEXEC_HIDE_CONSOLE)|wxEXEC_ASYNC, this->currentProcess)==0) {        
        wxString msg = "Failed to launch container, "+this->name + ", with the command, "+launchCmd+".  Most likely it is because the Boxedwine executable could not be found";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
        return;
    }
    this->currentProcess->isRunning = true;
}

void BoxedContainer::LaunchWine(const wxString& cmd, const wxString& path, bool showConsole) {
    Launch("/bin/wine "+cmd, path, showConsole);
}

void BoxedContainer::OnClose() {
    if (this->currentProcess && this->currentProcess->isRunning) {
        this->currentProcess->CloseOutput();
    }
}