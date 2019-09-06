#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "BoxedApp.h"
#include "BoxedContainerList.h"
#include "BoxedContainer.h"

static const int ID_ITEM_SETTINGS = 300;
static const int ID_ITEM_LAUNCH_REGEDIT = 301;
static const int ID_ITEM_LAUNCH_EXPLORER = 302;
static const int ID_ITEM_LAUNCH_WINECFG = 303;

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

    menu.Append(ID_ITEM_SETTINGS, "&Settings");
    menu.AppendSeparator();
    wxMenu* programsMenu = new wxMenu();
    menu.AppendSubMenu(programsMenu, "&Programs");
    programsMenu->Append(ID_ITEM_LAUNCH_WINECFG, "Wine Config");
    programsMenu->Append(ID_ITEM_LAUNCH_EXPLORER, "Explorer");
    programsMenu->Append(ID_ITEM_LAUNCH_REGEDIT, "Regedit");

    int id = GetPopupMenuSelectionFromUser(menu);

    if (id == ID_ITEM_SETTINGS) {

    } else if (id == ID_ITEM_LAUNCH_WINECFG) {
        container->LaunchWine("winecfg", "/home/username", true);
    } else if (id == ID_ITEM_LAUNCH_EXPLORER) {
        container->LaunchWine("explorer", "/home/username", true);
    } else if (id == ID_ITEM_LAUNCH_REGEDIT) {
        container->LaunchWine("regedit", "/home/username", true);
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