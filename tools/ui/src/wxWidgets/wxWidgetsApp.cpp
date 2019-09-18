
#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/stdpaths.h"
#include "wx/filename.h"
#include "wx/fileconf.h"
#include "wx/listctrl.h"
#include "wx/dir.h"
#include "wx/config.h"
#include "wx/dnd.h"
#include "wx/display.h"
#include "wx/imaglist.h"

#include "wxWidgetsApp.h"
#include "wxSettings.h"
#include "wxInstallDlg.h"
#include "wxHelp.h"
#include "BoxedContainer.h"
#include "BoxedApp.h"
#include "BoxedAppList.h"
#include "BoxedContainerList.h"
#include "GlobalSettings.h"
#include "wxUtils.h"

const int ID_TOOLBAR = 500;

const int ID_APPS = 200;
const int ID_CONTAINERS = 201;
const int ID_SETTINGS = 202;
const int ID_INSTALL = 203;
const int ID_HELP = 204;

DEFINE_EVENT_TYPE(wxEVT_LAUNCH_APP)

enum
{
    ID_Hello = 1
};
wxBEGIN_EVENT_TABLE(BoxedFrame, wxFrame)
    EVT_MENU(wxID_ANY, BoxedFrame::OnToolLeftClick)
    EVT_SIZE(BoxedFrame::OnSize)
    EVT_CLOSE(BoxedFrame::OnClose)
    EVT_COMMAND(wxID_ANY, wxEVT_LAUNCH_APP, BoxedFrame::OnCommand)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(BoxedWineApp);
bool BoxedWineApp::OnInit()
{
    wxString launchContainer;
    wxString launchCommand;

    for (int i=0;i<this->argc;i++) {
        if (this->argv[i]=="-container" && (i+1<this->argc)) {
            launchContainer = this->argv[i+1];
            i++;
        } else {
            launchCommand = argv[i];
        }
    }

    SetVendorName("Boxedwine");
    SetAppName("Boxedwine");
    wxLog::EnableLogging(false);

    BoxedFrame *frame = new BoxedFrame( "Boxedwine", wxDefaultPosition, wxSize(800, 600));
    if (launchContainer.Length() && launchCommand.Length()) {        
        for (auto& container : frame->containers) {
            if (container->GetName()==launchContainer) {                
                container->Launch(launchCommand, "", "/home/username", true, true);
                return false;
            }
        }
        wxString msg = "Did not find container: "+launchContainer+".  You should open settings for the container and update the OS integration";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
    }
    frame->Show( true );
    return true;
}

class BoxedInstallDrop : public wxFileDropTarget
{
public:
    BoxedInstallDrop(BoxedFrame *pOwner = NULL) { m_pOwner = pOwner; }

    virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) wxOVERRIDE;

private:
    BoxedFrame *m_pOwner;
};

bool BoxedInstallDrop::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    if (filenames.Count()) {
        wxCommandEvent evt(wxEVT_LAUNCH_APP, wxID_ANY);
		evt.SetString(filenames[0]);
		this->m_pOwner->GetEventHandler()->AddPendingEvent(evt);
    }
    return true;
}

BoxedFrame::BoxedFrame(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size), appPanel(NULL), appListView(NULL), containerPanel(NULL), containerListView(NULL)
{
    GlobalSettings::scaleFactor = this->FromDIP(10)/10;
    this->SetSize(wxSize(size.GetWidth()*GlobalSettings::GetScaleFactor(), size.GetHeight()*GlobalSettings::GetScaleFactor()));

#ifdef _WINDOWS
    HWND hWnd = this->GetHandle();
    HINSTANCE hInstance = wxGetInstance();
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(101));
    SetClassLongPtr(hWnd, GCLP_HICONSM, (LONG_PTR)hIcon);
#endif

    ::wxInitAllImageHandlers();    
    LoadConfig();    
    SetupToolBar();
    LoadContainers();
    SetupAppList();
    SetupContainerList();
    this->containerListView->Hide();
    this->SetDropTarget(new BoxedInstallDrop(this));
}

BoxedFrame::~BoxedFrame() {
    for (auto &container : this->containers) {
        delete container;
    }
}

void BoxedFrame::LoadContainers() {
    wxString filename;
    wxDir dir(GlobalSettings::GetContainerFolder());

     for (auto &container : this->containers) {
        delete container;
    }
    this->containers.clear();
    if (dir.IsOpened()) {
        bool cont = dir.GetFirst(&filename, wxEmptyString, wxDIR_DIRS);
       
        while(cont)
        {
            wxString dirPath = GlobalSettings::GetContainerFolder()+wxFileName::GetPathSeparator()+filename;
            BoxedContainer* container = new BoxedContainer();
            if (container->Load(dirPath)) {
                this->containers.push_back(container);
            } else {
                delete container;
            }
            cont = dir.GetNext(&filename);
        }
    }
}

void BoxedFrame::LoadConfig() {
    bool createDefault = false;

    if (!wxDirExists(wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator())) {
        wxMkdir(wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator());
        createDefault = true;    
    }
    wxString ini_filename = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + "boxedwine.ini";
    wxFileConfig *config = new wxFileConfig( "", "", ini_filename);
    config->SetRecordDefaults();
    wxString exe = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + wxFileName::GetPathSeparator() + "BoxedWine";
#ifdef _WINDOWS
    exe += ".exe";
#endif
    GlobalSettings::exeFileLocation = config->Read("Exe", exe);
    GlobalSettings::dataFolderLocation = config->Read("DataFolder", wxStandardPaths::Get().GetUserDataDir());
    int defaultIconSize = GlobalSettings::GetScaleFactor()>1.5?64:32;
    GlobalSettings::iconSize = config->Read("AppIconSize", defaultIconSize);
    if (GlobalSettings::iconSize!=16 || GlobalSettings::iconSize!=32 || GlobalSettings::iconSize!=64 || GlobalSettings::iconSize!=128) {
        GlobalSettings::iconSize = defaultIconSize;
    }
    config->Flush();
    delete config;

    GlobalSettings::InitWineVersions();

    if (createDefault && GlobalSettings::wineVersions.size()) {
        wxString containerFilePath = GlobalSettings::GetContainerFolder() + wxFileName::GetPathSeparator() + "Default";
        wxFileName(containerFilePath+wxFileName::GetPathSeparator()).Mkdir(511, wxPATH_MKDIR_FULL);

        BoxedContainer* container = BoxedContainer::CreateContainer(containerFilePath, "Default", GlobalSettings::wineVersions[0].name);
        
        BoxedApp app("WineMine", "/home/username/.wine/drive_c/windows/system32", "winemine.exe", container);
        app.SaveApp();

        // :TODO: unzip winemine.exe into that location so that icon works
        wxString sep = wxFileName::GetPathSeparator();
        wxString targetPath = containerFilePath + sep + "root";
        wxString targetFile = targetPath + sep + "home" + sep + "username" + sep + ".wine" + sep + "drive_c" + sep + "windows" + sep + "system32" + sep + "winemine.exe";
        wxString timestampeFile = targetPath + sep + "home" + sep + "username" + sep + ".wine" + sep + ".update-timestamp" ;

        // :TODO: not sure why if I don't do this, wine will detect a change and update the .wine directory
        wxExtractZipFile(GlobalSettings::wineVersions[0].filePath, targetPath, timestampeFile);

        wxExtractZipFile(GlobalSettings::wineVersions[0].filePath, targetPath, targetFile);
    }
}

void BoxedFrame::SaveConfig() {
    wxString ini_filename = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + "boxedwine.ini";
    wxFileConfig *config = new wxFileConfig( "", "", ini_filename);
    config->Write("Exe", GlobalSettings::exeFileLocation);
    config->Write("DataFolder", GlobalSettings::dataFolderLocation);
    config->Write("AppIconSize", GlobalSettings::iconSize);
    config->Flush();
    delete config;
}

bool compareBoxedApp(BoxedApp* a, BoxedApp* b) { 
    return a->GetName().CmpNoCase(b->GetName())<0; 
}

void BoxedFrame::SetupAppList() {
    this->appListView = new BoxedAppList(this, this, wxID_ANY, wxPoint(0,0), this->GetClientSize(), wxLC_ICON|wxLC_ALIGN_TOP);        
    ReloadAppList();
}

void BoxedFrame::ResizeAppList() {
    if (this->appListView) {
        this->appListView->SetSize(this->GetClientSize());
        this->appListView->Arrange();
    }
}

void BoxedFrame::SetupContainerList() {
    this->containerListView = new BoxedContainerList(this, this, wxID_ANY, wxPoint(0,0), this->GetClientSize(), wxLC_REPORT|wxLC_NO_SORT_HEADER);
    this->containerListView->SetFont(wxFontInfo(12));
    ReloadContainerList();
}

void BoxedFrame::ResizeContainerList() {
    if (this->containerListView) {
        this->containerListView->SetSize(this->GetClientSize());
        this->containerListView->Arrange();
    }
}

bool compareBoxedContainer(BoxedContainer* a, BoxedContainer* b) { 
    return a->GetName().CmpNoCase(b->GetName())<0; 
}

void BoxedFrame::ReloadContainerList() {
    if (!this->containerListView) {
        return;
    }
    std::vector<BoxedContainer*> containers;
    for (auto &container : this->containers) {
        containers.push_back(container);
    }
    std::sort(containers.begin(), containers.end(), compareBoxedContainer);

    this->containerListView->ClearAll();
    this->containerListView->AppendColumn("Name");
    this->containerListView->SetColumnWidth(0, 200);
    this->containerListView->AppendColumn("Size");
    this->containerListView->SetColumnWidth(1, 100);
    this->containerListView->AppendColumn("Wine Version");
    this->containerListView->SetColumnWidth(2, 125);
    wxString containerWithIntegration = getContainerNameAssociatedWithIntegration();
    if (containerWithIntegration.Length()) {
        this->containerListView->AppendColumn("Integrated?");
        this->containerListView->SetColumnWidth(2, 125);
    }
    int index = 0;
    for (auto& container : containers) {
        long id = this->containerListView->InsertItem(index, container->GetName());
        this->containerListView->SetItemPtrData(id, (wxUIntPtr)container);
        wxULongLong size = wxDir::GetTotalSize(container->GetDir());
        this->containerListView->SetItem(id, 1, wxFileName::GetHumanReadableSize(size));
        this->containerListView->SetItem(id, 2, container->GetWineVersion());
        if (containerWithIntegration.Length()) {
            if (containerWithIntegration == container->GetName()) {
                this->containerListView->SetItem(id, 3, "YES");
            } else {
                this->containerListView->SetItem(id, 3, "");
            }
        }
        index++;
    }

    for (int col=0;col<this->containerListView->GetColumnCount();col++) {
        this->containerListView->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER );
        int wh = this->containerListView->GetColumnWidth(col);
        this->containerListView->SetColumnWidth(col, wxLIST_AUTOSIZE );
        int wc = this->containerListView->GetColumnWidth(col);
        if (wh > wc) {
            this->containerListView->SetColumnWidth(col, wh*1.2);
        } else {
            this->containerListView->SetColumnWidth(col, wc*1.2);
        }
        
    }
}

void BoxedFrame::ReloadAppList() {
    if (!this->appListView) {
        return;
    }
    std::vector<BoxedApp*> apps;
    for (auto &container : this->containers) {
        for (auto &app : container->GetApps()) {
            apps.push_back(app);
        }
    }
    std::sort(apps.begin(), apps.end(), compareBoxedApp);

    wxImageList* imageList = new wxImageList(GlobalSettings::iconSize, GlobalSettings::iconSize);
    int* imageIndex = new int[apps.size()];
    int index = 0;
    int imageCount=0;
    for (auto& app : apps) {
        wxIcon* icon = app->CreateIcon(GlobalSettings::iconSize);
        if (icon) {
            imageList->Add(*icon);
            imageIndex[index++] = imageCount;
            imageCount++;
        } else {
            imageIndex[index++] = -1;
        }
        delete icon;
    }
    this->appListView->ClearAll();
    this->appListView->AssignImageList(imageList, wxIMAGE_LIST_NORMAL);

    index = 0;
    for (auto& app : apps) {
        long id = this->appListView->InsertItem(index, app->GetName(), imageIndex[index]);
        this->appListView->SetItemPtrData(id, (wxUIntPtr)app);
        index++;
    }
    delete[] imageIndex;
}

void BoxedFrame::SetupToolBar()
{
    wxToolBarBase *toolBar = CreateToolBar(wxTB_NOICONS | wxTB_FLAT | wxTB_TEXT | wxTB_HORZ_LAYOUT | wxTB_TOP, ID_TOOLBAR);

    toolBar->SetFont(wxFontInfo(20));
    toolBar->AddTool(ID_APPS, "   Apps   ", wxNullBitmap, "Shortcuts you created in a container", wxITEM_CHECK);
    toolBar->AddSeparator();
    toolBar->AddTool(ID_INSTALL, "   Install   ", wxNullBitmap, "Install a new application/game into a new or existing container", wxITEM_NORMAL);
    toolBar->AddSeparator();
    toolBar->AddTool(ID_CONTAINERS, "   Containers   ", wxNullBitmap, "On Linux, this is a separate Wine folder", wxITEM_CHECK);
    toolBar->AddSeparator();
    toolBar->AddTool(ID_SETTINGS, "   Settings   ", wxNullBitmap, "You can customize how Boxedwine is run", wxITEM_NORMAL);
    toolBar->AddSeparator();
    toolBar->AddTool(ID_HELP, "   Help   ", wxNullBitmap, "How to use the app", wxITEM_NORMAL);
    toolBar->Realize();
    toolBar->ToggleTool(ID_APPS, true);
}

void BoxedFrame::OnToolLeftClick(wxCommandEvent& event)
{
    if (event.GetId() == ID_APPS)
    {
        GetToolBar()->ToggleTool(ID_APPS, true);
        GetToolBar()->ToggleTool(ID_CONTAINERS, false);
        this->containerListView->Hide();
        this->appListView->Show();
    } 
    else if (event.GetId() == ID_CONTAINERS) 
    {
        GetToolBar()->ToggleTool(ID_APPS, false);
        GetToolBar()->ToggleTool(ID_CONTAINERS, true);
        this->containerListView->Show();
        this->appListView->Hide();
    }
    else if (event.GetId() == ID_SETTINGS) 
    {
        bool apps = GetToolBar()->GetToolState(ID_APPS);
        bool containers = GetToolBar()->GetToolState(ID_CONTAINERS);
        GetToolBar()->ToggleTool(ID_APPS, false);
        GetToolBar()->ToggleTool(ID_CONTAINERS, false);

        SettingsDialog *dlg = new SettingsDialog(this, GlobalSettings::exeFileLocation, GlobalSettings::dataFolderLocation);
        dlg->Show(true);
        if (dlg->GetReturnCode()==wxID_OK) {
            GlobalSettings::exeFileLocation = dlg->exeFileLocationText->GetValue();
            GlobalSettings::dataFolderLocation = dlg->dataFolderLocationText->GetValue();
            SaveConfig();
        }
        GetToolBar()->ToggleTool(ID_APPS, apps);
        GetToolBar()->ToggleTool(ID_CONTAINERS, containers);
    } 
    else if (event.GetId() == ID_INSTALL) 
    {
        InstallApp("");
    } 
    else if (event.GetId() == ID_HELP) {
        bool apps = GetToolBar()->GetToolState(ID_APPS);
        bool containers = GetToolBar()->GetToolState(ID_CONTAINERS);
        GetToolBar()->ToggleTool(ID_APPS, false);
        GetToolBar()->ToggleTool(ID_CONTAINERS, false);

        HelpDialog *dlg = new HelpDialog(this);
        dlg->Show(true);
        GetToolBar()->ToggleTool(ID_APPS, apps);
        GetToolBar()->ToggleTool(ID_CONTAINERS, containers);
    }
}

void BoxedFrame::OnSize(wxSizeEvent& event) {
    ResizeAppList();    
    ResizeContainerList();
    event.Skip();
}

void BoxedFrame::OnClose(wxCloseEvent& event) {
    for (auto &container : this->containers) {
        container->OnClose();
    }
    event.Skip();
}

void BoxedFrame::InstallApp(const wxString& filePath) {
    bool apps = GetToolBar()->GetToolState(ID_APPS);
    bool containers = GetToolBar()->GetToolState(ID_CONTAINERS);
    GetToolBar()->ToggleTool(ID_APPS, false);
    GetToolBar()->ToggleTool(ID_CONTAINERS, false);

    InstallDialog *dlg = new InstallDialog(this, filePath, this->containers);
    dlg->Show(true);
    if (dlg->GetReturnCode()==wxID_OK) {
        this->LoadContainers();
        this->ReloadAppList();
        this->ReloadContainerList();
    }
    GetToolBar()->ToggleTool(ID_APPS, apps);
    GetToolBar()->ToggleTool(ID_CONTAINERS, containers);
}

void BoxedFrame::OnCommand(wxCommandEvent& evt) {
    if (evt.GetEventType()==wxEVT_LAUNCH_APP) {
        this->InstallApp(evt.GetString());
    }
}
