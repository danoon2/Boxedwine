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
    ~BoxedContainer();

    static BoxedContainer* CreateContainer(const wxString& dirPath, const wxString& name, const wxString& wineVersion);

    bool Load(const wxString& dirPath);
    bool SaveContainer();

    void Reload();
    int GetAppAcount() {return this->apps.size();}
    BoxedApp* GetApp(int index) {if (index>=0 && index<(int)this->apps.size()) { return this->apps[index]; } else {return NULL;}}
    void AddApp(BoxedApp* app) {this->apps.push_back(app);}
    void DeleteApp(BoxedApp* app);
    void DeleteContainerFromFilesystem();
    void Launch(const wxString& cmd, const wxString& args, const wxString& path, bool showConsole, bool async=true);
    void LaunchWine(const wxString& cmd, const wxString& args, const wxString& path, bool showConsole, bool async=true);

    const wxString& GetName() {return this->name;}
    const wxString& GetDir() {return this->dirPath;}
    const wxString& GetWineVersion() {return this->wineVersion;}
    const std::vector<BoxedApp*>& GetApps() {return this->apps;}
    void GetNewApps(std::vector<BoxedApp>& apps);

    void OnClose();
private:
    void LoadApps();
    void GetDesktopApps(std::vector<BoxedApp>& apps);
    void GetExeApps(std::vector<BoxedApp>& apps);

    friend class GlobalSettings;
    friend class BoxedContainerOptionsDialog;
    std::vector<BoxedApp*> apps;
    wxString name;
    wxString wineVersion;
    wxString dirPath;
    BoxedContainerProcess* currentProcess;
};

#endif