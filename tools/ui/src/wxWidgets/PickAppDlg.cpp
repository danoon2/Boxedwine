#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "PickAppDlg.h"
#include "GlobalSettings.h"

wxBEGIN_EVENT_TABLE(PickAppDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PickAppDialog::OnDone)
wxEND_EVENT_TABLE()

PickAppDialog::PickAppDialog(wxWindow* parent, BoxedContainer* container) : wxDialog(parent, -1, "Create Shortcut"), container(container) {    
    container->GetNewApps(apps);

    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    int size = GlobalSettings::GetScaleFactor()*400;
    vbox->Add(new wxStaticText(this, -1, "Please select a file to use for the app short cut.", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT).DoubleBorder());
    this->listView = new wxListView(this, wxID_ANY, wxPoint(0,0), wxSize(size, size), wxLC_REPORT|wxLC_NO_HEADER|wxBORDER_SUNKEN);

    wxImageList* imageList = new wxImageList(GlobalSettings::iconSize, GlobalSettings::iconSize);
    int* imageIndex = new int[apps.size()+1];
    imageIndex[0]=-1; // Browse image
    int index = 1;
    int imageCount=0;
    for (auto& app : apps) {
        wxIcon* icon = app.CreateIcon(GlobalSettings::iconSize);
        if (icon) {
            imageList->Add(*icon);
            imageIndex[index++] = imageCount;
            imageCount++;
        } else {
            imageIndex[index++] = -1;
        }
        delete icon;
    }
    this->listView->AssignImageList(imageList, wxIMAGE_LIST_SMALL);

    this->listView->AppendColumn("Name");
    this->listView->SetColumnWidth(0, this->listView->GetClientSize().GetWidth());
    
    index = 1;
    this->listView->InsertItem(index, "Browse", -1);

    for (auto& app : apps) {
        long id = this->listView->InsertItem(index, (app.IsLink()?app.GetName()+".lnk":app.GetName()), imageIndex[index]);
        index++;
    }
    delete[] imageIndex;
    this->listView->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

    vbox->Add(this->listView, wxSizerFlags().DoubleBorder());
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    hbox->Add(new wxButton(this, wxID_OK, "OK" ));
    hbox->AddSpacer(10);
    hbox->Add(new wxButton(this, wxID_CANCEL, "Cancel" ));
   
    this->SetSizerAndFit(vbox);

    Centre();
    ShowModal();

    Destroy();
}

void PickAppDialog::OnDone(wxCommandEvent& event) {
    long index = this->listView->GetFirstSelected();
    if (index==0) {
    } else {
        this->apps[index-1].SaveApp();
        event.Skip();
    }
}