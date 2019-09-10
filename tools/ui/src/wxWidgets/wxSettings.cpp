#include "wx/wx.h"
#include "wxSettings.h"
#include "GlobalSettings.h"

const int ID_EXE_LOCATION = 300;
const int ID_EXE_LOCATION_BROWSE = 301;
const int ID_DATA_LOCATION = 302;
const int ID_DATA_LOCATION_BROWSE = 303;

SettingsDialog::SettingsDialog(wxWindow* parent, const wxString& exeFileLocation, const wxString& dataFolderLocation) : wxDialog(parent, -1, "Settings") {
    wxFlexGridSizer* fSizer = new wxFlexGridSizer(3);    
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(fSizer);
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    fSizer->Add(new wxStaticText(this, -1, "Boxedwine file location:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->exeFileLocationText = new wxTextCtrl(this, ID_EXE_LOCATION, exeFileLocation, wxDefaultPosition, wxSize(300*GlobalSettings::GetScaleFactor(), -1));
    fSizer->Add(this->exeFileLocationText, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));
    fSizer->Add(new wxButton(this, ID_EXE_LOCATION_BROWSE, "Browse" ), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    Connect(ID_EXE_LOCATION_BROWSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SettingsDialog::OnBrowseExeButtonClicked));

    fSizer->Add(new wxStaticText(this, -1, "Save folder location:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->dataFolderLocationText = new wxTextCtrl(this, ID_DATA_LOCATION, dataFolderLocation, wxDefaultPosition, wxSize(300*GlobalSettings::GetScaleFactor(), -1));
    fSizer->Add(this->dataFolderLocationText, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL));
    fSizer->Add(new wxButton(this, ID_DATA_LOCATION_BROWSE, "Browse" ), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    Connect(ID_DATA_LOCATION_BROWSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SettingsDialog::OnBrowseDataButtonClicked));

    hbox->Add(new wxButton(this, wxID_OK, "OK" ));
    hbox->AddSpacer(10);
    hbox->Add(new wxButton(this, wxID_CANCEL, "Cancel" ));
   
    this->SetSizerAndFit(vbox);

    Centre();
    ShowModal();

    Destroy();
}

void SettingsDialog::OnBrowseExeButtonClicked(wxCommandEvent& event) {
    wxFileDialog* openFileDialog = new wxFileDialog(this, "Boxedwine executable", wxEmptyString, wxEmptyString, "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (openFileDialog->ShowModal() == wxID_OK){
        this->exeFileLocationText->SetLabelText(openFileDialog->GetPath());
    }
}

void SettingsDialog::OnBrowseDataButtonClicked(wxCommandEvent& event) {
    wxDirDialog* openFileDialog = new wxDirDialog(this);

    if (openFileDialog->ShowModal() == wxID_OK){
        this->dataFolderLocationText->SetLabelText(openFileDialog->GetPath());
    }
}
