#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/combobox.h"

#include "BoxedContainerOptionsDlg.h"
#include "wxUtils.h"
#include "GlobalSettings.h"

static const int ID_INSTALL_REGISTRY = 300;
static const int ID_DELETE_REGISTRY = 301;

wxBEGIN_EVENT_TABLE(BoxedContainerOptionsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, BoxedContainerOptionsDialog::OnDone)
    EVT_BUTTON(ID_INSTALL_REGISTRY, BoxedContainerOptionsDialog::OnInstallRegistry)
    EVT_BUTTON(ID_DELETE_REGISTRY, BoxedContainerOptionsDialog::OnDeleteRegistry)
wxEND_EVENT_TABLE()

BoxedContainerOptionsDialog::BoxedContainerOptionsDialog(wxWindow* parent, BoxedContainer* container) : wxDialog(parent, -1, "Container Options"), container(container) {    
    wxFlexGridSizer* fSizer = new wxFlexGridSizer(2);    
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);    

    wxArrayString wineVersions;
    for (auto& wineVersion : GlobalSettings::wineVersions) {
        wineVersions.Add(wineVersion.name);
    }
    wineVersions.Sort(true);

    fSizer->Add(new wxStaticText(this, -1, "Wine Version:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->wineVersionCombobox = new wxComboBox(this, wxID_ANY, container->GetWineVersion(), wxDefaultPosition, wxDefaultSize, wineVersions, wxCB_READONLY);
    fSizer->Add(this->wineVersionCombobox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    vbox->Add(fSizer);

#ifdef _WINDOWS
    wxStaticBoxSizer *registryBox = new wxStaticBoxSizer(wxVERTICAL, this, "Windows Integration");    

    wxFlexGridSizer* fRegistrySizer = new wxFlexGridSizer(2);    
    registryBox->Add(new wxStaticText(this, -1, "Allows launching 16-bit programs directly in Windows.  For example,\r\nyou can double click a 16-bit program in Explorer and it will launch Boxedwine\r\nautomatically with the container and Wine version you selected.", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT).DoubleBorder());

    fRegistrySizer->Add(new wxStaticText(this, -1, "Add/Update registry entries for this container:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    fRegistrySizer->Add(new wxButton(this, ID_INSTALL_REGISTRY, "Add/Update" ), wxSizerFlags().DoubleBorder(wxRIGHT));
    fRegistrySizer->Add(new wxStaticText(this, -1, "Delete all registry entries:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    fRegistrySizer->Add(new wxButton(this, ID_DELETE_REGISTRY, "Delete" ), wxSizerFlags().DoubleBorder(wxRIGHT));
    registryBox->Add(fRegistrySizer);        

    vbox->AddSpacer(10);
    vbox->Add(registryBox, wxSizerFlags().DoubleBorder());
#endif

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

void BoxedContainerOptionsDialog::OnDone(wxCommandEvent& event) {
    this->container->wineVersion = this->wineVersionCombobox->GetValue();
    this->container->SaveContainer();
    event.Skip();
}

void BoxedContainerOptionsDialog::OnInstallRegistry(wxCommandEvent& event) {
#ifdef _WINDOWS
    updateWindowsIntegrationRegistry(this->container);
#endif
}

void BoxedContainerOptionsDialog::OnDeleteRegistry(wxCommandEvent& event) {
#ifdef _WINDOWS
    deleteWindowsIntegrationRegistry();
#endif
}