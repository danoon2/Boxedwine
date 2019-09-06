#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "BoxedApp.h"
#include "BoxedAppList.h"
#include "BoxedContainer.h"

static const int ID_ITEM_LAUNCH = 300;
static const int ID_ITEM_LAUNCH_AND_SHOW_LOG = 301;

void BoxedAppList::OnItemRightClick(wxListEvent& event) 
{
    BoxedApp* app = (BoxedApp*)this->GetItemData(event.GetIndex());
    ShowContextMenu(app);
}

void BoxedAppList::OnContextMenu(wxContextMenuEvent& event)
{
    //ShowContextMenu();
}

void BoxedAppList::ShowContextMenu(BoxedApp* app) 
{
    wxMenu menu;

    menu.Append(ID_ITEM_LAUNCH, "&Launch");
    menu.Append(ID_ITEM_LAUNCH_AND_SHOW_LOG, "Launch And &Show Console");
    int id = GetPopupMenuSelectionFromUser(menu);

    if (id == ID_ITEM_LAUNCH) {
        app->Launch(false);
    } else if (id == ID_ITEM_LAUNCH_AND_SHOW_LOG) {
        app->Launch(true);
    }
}

void BoxedAppList::OnActivated(wxListEvent& event) 
{
    BoxedApp* app = (BoxedApp*)this->GetItemData(event.GetIndex());
    app->Launch(false);
}

wxBEGIN_EVENT_TABLE(BoxedAppList, wxListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, BoxedAppList::OnActivated)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, BoxedAppList::OnItemRightClick)
wxEND_EVENT_TABLE()