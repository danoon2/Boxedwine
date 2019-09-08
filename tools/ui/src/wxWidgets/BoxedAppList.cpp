#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "BoxedApp.h"
#include "BoxedAppList.h"
#include "BoxedContainer.h"
#include "wxWidgetsApp.h"
#include "BoxedAppOptionsDlg.h"

static const int ID_ITEM_LAUNCH = 300;
static const int ID_ITEM_LAUNCH_AND_SHOW_LOG = 301;
static const int ID_ITEM_REMOVE = 302;
static const int ID_ITEM_OPTIONS = 303;

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
    menu.Append(ID_ITEM_OPTIONS, "&Options");
    menu.AppendSeparator();
    menu.Append(ID_ITEM_REMOVE, "&Remove Shortcut");

    int id = GetPopupMenuSelectionFromUser(menu);

    if (id == ID_ITEM_LAUNCH) {
        app->Launch(false);
    } else if (id == ID_ITEM_LAUNCH_AND_SHOW_LOG) {
        app->Launch(true);
    } else if (id == ID_ITEM_REMOVE) {
        wxMessageDialog dialog(this, "Are you sure you want to delete this short cut?  If you change your mind later you can add the short cut back but right clicking the contrainer in the container table.", "Confirm Delete", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_QUESTION);
        if (dialog.ShowModal()==wxID_YES) {
            app->Remove();
            this->mainFrame->ReloadAppList();
        }
    } else if (id == ID_ITEM_OPTIONS) {
        BoxedAppOptionsDialog *dlg = new BoxedAppOptionsDialog(this, app);
        dlg->Show(true);
        if (dlg->GetReturnCode()==wxID_OK) {
            this->mainFrame->LoadContainers();
            this->mainFrame->ReloadAppList();
            this->mainFrame->ReloadContainerList();
        }
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