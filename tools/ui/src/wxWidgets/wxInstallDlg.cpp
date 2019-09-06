#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/combobox.h>

#include "wxInstallDlg.h"
#include "GlobalSettings.h"

static const int ID_SETUP_LOCATION = 300;
static const int ID_SETUP_LOCATION_BROWSE = 301;

InstallDialog::InstallDialog(wxWindow* parent, const wxString& filePathToInstall, std::vector<BoxedContainer*>& containers) : wxDialog(parent, -1, "Install"), containers(containers) {
    wxFlexGridSizer* fSizer = new wxFlexGridSizer(3);    
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(fSizer);
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    fSizer->Add(new wxStaticText(this, -1, "Setup file location:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->setupFileLocationText = new wxTextCtrl(this, ID_SETUP_LOCATION, filePathToInstall, wxDefaultPosition, wxSize(300, -1));
    fSizer->Add(this->setupFileLocationText, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));
    fSizer->Add(new wxButton(this, ID_SETUP_LOCATION_BROWSE, "Browse" ), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    Connect(ID_SETUP_LOCATION_BROWSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(InstallDialog::OnBrowseExeButtonClicked));

    wxArrayString options;
    options.Add("*Create New Container (Recommended)");
    for (auto& container : this->containers) {
        options.Add(container->GetName());
    }
    this->containerComboBox = new wxComboBox(this, wxID_ANY, options[0], wxDefaultPosition, wxDefaultSize, options, wxCB_SORT|wxCB_READONLY);
    Connect(wxID_ANY, wxEVT_COMBOBOX, wxCommandEventHandler(InstallDialog::OnComboBoxUpdate));

    fSizer->Add(new wxStaticText(this, -1, "Container:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    fSizer->Add(this->containerComboBox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));
    fSizer->AddStretchSpacer();

    fSizer->AddStretchSpacer();
    
    wxFlexGridSizer* newContainerSizer = new wxFlexGridSizer(2);
    newContainerSizer->Add(new wxStaticText(this, -1, "Name:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->containerNameText = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(250, -1));
    newContainerSizer->Add(this->containerNameText, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));
    newContainerSizer->Add(new wxStaticText(this, -1, "Wine Version:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    wxArrayString wineVersions;
    for (auto& wineVersion : GlobalSettings::wineVersions) {
        wineVersions.Add(wineVersion.name);
    }
    this->wineVersionComboBox = new wxComboBox(this, wxID_ANY, (wineVersions.Count()?wineVersions[0]:""), wxDefaultPosition, wxDefaultSize, wineVersions, wxCB_SORT|wxCB_READONLY);
    newContainerSizer->Add(this->wineVersionComboBox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));
    newContainerSizer->Add(new wxStaticText(this, -1, "Run Wine Config:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->runWineConfigCheckBox = new wxCheckBox(this, -1, "(can change Win version to Win95, etc)");
    newContainerSizer->Add(this->runWineConfigCheckBox, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
    fSizer->Add(newContainerSizer, wxSizerFlags().Expand());
    fSizer->AddStretchSpacer();

    hbox->Add(new wxButton(this, wxID_OK, "OK" ));
    hbox->AddSpacer(10);
    hbox->Add(new wxButton(this, wxID_CANCEL, "Cancel" ));
   
    this->SetSizerAndFit(vbox);

    Centre();
    ShowModal();

    Destroy();
}


void InstallDialog::OnBrowseExeButtonClicked(wxCommandEvent& event) {
    wxFileDialog* openFileDialog = new wxFileDialog(this, "Setup executable", wxEmptyString, wxEmptyString, "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (openFileDialog->ShowModal() == wxID_OK){
        this->setupFileLocationText->SetLabelText(openFileDialog->GetPath());
    }
}

void InstallDialog::OnComboBoxUpdate(wxCommandEvent& event)
{
    if (event.GetSelection()==0) {
        this->containerNameText->Enable(true);
        this->wineVersionComboBox->Enable(true);
        this->runWineConfigCheckBox->Enable(true);
    } else {
        this->containerNameText->Enable(false);
        this->wineVersionComboBox->Enable(false);
        this->runWineConfigCheckBox->Enable(false);
    }
}