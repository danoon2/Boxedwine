#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/combobox.h>

#include "BoxedContainerOptionsDlg.h"
#include "GlobalSettings.h"

wxBEGIN_EVENT_TABLE(BoxedContainerOptionsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, BoxedContainerOptionsDialog::OnDone)
wxEND_EVENT_TABLE()

BoxedContainerOptionsDialog::BoxedContainerOptionsDialog(wxWindow* parent, BoxedContainer* container) : wxDialog(parent, -1, "Container Options"), container(container) {    
    wxFlexGridSizer* fSizer = new wxFlexGridSizer(2);    
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(fSizer);
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    wxArrayString wineVersions;
    for (auto& wineVersion : GlobalSettings::wineVersions) {
        wineVersions.Add(wineVersion.name);
    }
    wineVersions.Sort(true);

    fSizer->Add(new wxStaticText(this, -1, "Wine Version:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->wineVersionCombobox = new wxComboBox(this, wxID_ANY, container->GetWineVersion(), wxDefaultPosition, wxDefaultSize, wineVersions, wxCB_READONLY);
    fSizer->Add(this->wineVersionCombobox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));
    
    hbox->Add(new wxButton(this, wxID_OK, "OK" ));
    hbox->AddSpacer(10);
    hbox->Add(new wxButton(this, wxID_CANCEL, "Cancel" ));
   
    this->SetSizerAndFit(vbox);

    Centre();
    ShowModal();

    Destroy();
}

void BoxedContainerOptionsDialog::OnDone(wxCommandEvent& event) {
    this->container->wineVersion = this->wineVersionCombobox->GetValue();
    this->container->SaveContainer();
    event.Skip();
}