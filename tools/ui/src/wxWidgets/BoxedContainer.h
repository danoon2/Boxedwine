#ifndef __BOXED_CONTAINER_H__
#define __BOXED_CONTAINER_H__
#include <wx/process.h>
#include "BoxedApp.h"

class BoxedContainerProcess : public wxProcess {
public:
    BoxedContainerProcess() : isRunning(false) {}
    virtual void OnTerminate(int pid, int status) {this->isRunning = false;}
    bool isRunning;
};

class BoxedContainer {
public:
    BoxedContainer() : currentProcess(NULL) {}

    bool Load(const wxString& dirPath);
    int GetAppAcount() {return this->apps.size();}
    BoxedApp* GetApp(int index) {if (index>=0 && index<(int)this->apps.size()) { return this->apps[index]; } else {return NULL;}}
    void AddApp(BoxedApp* app) {this->apps.push_back(app);}
    void DeleteApp(BoxedApp* app);
    void Launch(const wxString& cmd, const wxString& path, bool showConsole);
    void LaunchWine(const wxString& cmd, const wxString& path, bool showConsole);

    const wxString& GetName() {return this->name;}
    const wxString& GetDir() {return this->dirPath;}
    const wxString& GetFileSystemName() {return this->fileSystem;}
    const std::vector<BoxedApp*>& GetApps() {return this->apps;}

    void OnClose();
private:
    void LoadApps();

    friend class GlobalSettings;
    std::vector<BoxedApp*> apps;
    wxString name;
    wxString fileSystem;
    wxString dirPath;
    BoxedContainerProcess* currentProcess;
};

#endif