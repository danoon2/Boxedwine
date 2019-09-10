#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "BoxedApp.h"
#include "BoxedContainerList.h"
#include "BoxedContainer.h"
#include "PickAppDlg.h"
#include "BoxedContainerOptionsDlg.h"

#include "wxWidgetsApp.h"

static const int ID_ITEM_SETTINGS = 300;
static const int ID_ITEM_LAUNCH_REGEDIT = 301;
static const int ID_ITEM_LAUNCH_EXPLORER = 302;
static const int ID_ITEM_LAUNCH_WINECFG = 303;
static const int ID_ITEM_ADD_APP = 304;
static const int ID_ITEM_REMOVE = 305;

void BoxedContainerList::OnItemRightClick(wxListEvent& event) 
{
    BoxedContainer* container = (BoxedContainer*)this->GetItemData(event.GetIndex());
    ShowContextMenu(container);
}

void BoxedContainerList::OnContextMenu(wxContextMenuEvent& event) 
{
    //ShowContextMenu();
}

void BoxedContainerList::ShowContextMenu(BoxedContainer* container) 
{
    wxMenu menu;

    menu.Append(ID_ITEM_ADD_APP, "&Create App Shortcut");
    menu.Append(ID_ITEM_SETTINGS, "&Settings");
    menu.AppendSeparator();
    wxMenu* programsMenu = new wxMenu();
    menu.AppendSubMenu(programsMenu, "Run &Program");
    programsMenu->Append(ID_ITEM_LAUNCH_WINECFG, "&Wine Config (Set Win Ver, etc)");
    programsMenu->Append(ID_ITEM_LAUNCH_EXPLORER, "&Explorer");
    programsMenu->Append(ID_ITEM_LAUNCH_REGEDIT, "&Regedit");
    menu.AppendSeparator();
    menu.Append(ID_ITEM_REMOVE, "&Delete Container");

    int id = GetPopupMenuSelectionFromUser(menu);

    if (id == ID_ITEM_SETTINGS) {
        BoxedContainerOptionsDialog *dlg = new BoxedContainerOptionsDialog(this, container);
        if (dlg->GetReturnCode()==wxID_OK) {
            this->mainFrame->ReloadContainerList();
        }
    } else if (id == ID_ITEM_ADD_APP) {
        PickAppDialog *dlg = new PickAppDialog(this, container);
        if (dlg->GetReturnCode()==wxID_OK) {
            container->Reload();
            this->mainFrame->ReloadAppList();
        }
    } else if (id == ID_ITEM_LAUNCH_WINECFG) {
        container->LaunchWine("winecfg", "", "/home/username", true);
    } else if (id == ID_ITEM_LAUNCH_EXPLORER) {
        container->LaunchWine("explorer", "", "/home/username", true);
    } else if (id == ID_ITEM_LAUNCH_REGEDIT) {
        container->LaunchWine("regedit", "", "/home/username", true);
    } else if (id == ID_ITEM_REMOVE) {
        wxMessageDialog dialog(this, "Are you sure you want to delete this container?  This will be permanant and if you change your mind you will have to reinstall any apps that were in this container.", "Confirm Delete", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_QUESTION);
        if (dialog.ShowModal()==wxID_YES) {
            container->DeleteContainerFromFilesystem();
            this->mainFrame->LoadContainers();
            this->mainFrame->ReloadAppList();
            this->mainFrame->ReloadContainerList();
        }
    }
}

void BoxedContainerList::OnActivated(wxListEvent& event) 
{
}

void BoxedContainerList::OnColClick(wxListEvent& event)
{
    int col = event.GetColumn();
}

wxBEGIN_EVENT_TABLE(BoxedContainerList, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, BoxedContainerList::OnActivated)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, BoxedContainerList::OnItemRightClick)
    EVT_LIST_COL_CLICK(wxID_ANY, BoxedContainerList::OnColClick)
wxEND_EVENT_TABLE()