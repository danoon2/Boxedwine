#ifndef __WXWIDGETSAPP_H__
#define __WXWIDGETSAPP_H__
#include "BoxedContainer.h"

class BoxedWineApp: public wxApp
{
public:
    virtual bool OnInit();
};
class BoxedFrame: public wxFrame
{
public:
    BoxedFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    void InstallApp(const wxString& filePath);

    void ReloadAppList();
    void ReloadContainerList(); 
private:
    wxDECLARE_EVENT_TABLE();

    void SetupToolBar();
    void OnToolLeftClick(wxCommandEvent& event);
    void LoadConfig();
    void SaveConfig();
    void SetupAppList();
    void SetupContainerList();
    void LoadContainers();    
    void OnSize(wxSizeEvent& event);
    void OnClose(wxCloseEvent& event);    
    void ResizeAppList();
    void ResizeContainerList();       
    void OnCommand(wxCommandEvent& evt);

    wxPanel* appPanel;
    wxListView* appListView;

    wxPanel* containerPanel;
    wxListView* containerListView;

    std::vector<BoxedContainer*> containers;
};

#endif