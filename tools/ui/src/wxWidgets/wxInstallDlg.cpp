#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/combobox.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/dir.h>

#include "wxInstallDlg.h"
#include "GlobalSettings.h"
#include "PickAppDlg.h"

static const int ID_SETUP_LOCATION = 300;
static const int ID_SETUP_LOCATION_BROWSE = 301;

wxBEGIN_EVENT_TABLE(InstallDialog, wxDialog)
    EVT_BUTTON(wxID_OK, InstallDialog::OnDone)
wxEND_EVENT_TABLE()

InstallDialog::InstallDialog(wxWindow* parent, const wxString& filePathToInstall, std::vector<BoxedContainer*>& containers) : wxDialog(parent, -1, "Install"), containers(containers) {
    wxFlexGridSizer* fSizer = new wxFlexGridSizer(3);    
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(fSizer);
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    fSizer->Add(new wxStaticText(this, -1, "Setup file location:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->setupFileLocationText = new wxTextCtrl(this, ID_SETUP_LOCATION, filePathToInstall, wxDefaultPosition, wxSize(300*GlobalSettings::GetScaleFactor(), -1));
    this->setupFileLocationText->SetHint("Optional: If blank, will create empty container");
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
    this->containerNameText = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(250*GlobalSettings::GetScaleFactor(), -1));
    this->containerNameText->SetHint("Must be valid file system name");
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

void InstallDialog::OnDone(wxCommandEvent& event) {
    BoxedContainer* container = NULL;

    wxString filePath = this->setupFileLocationText->GetValue();
    if (filePath.Length()!=0 && !wxFileExists(filePath)) {
        wxString msg = "The setup file was entered, but it does not exist";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
        return;
    }

    if (this->containerComboBox->GetSelection()!=0) {
        container = this->containers[this->containerComboBox->GetSelection()-1];
    } else {
        wxString containerFileName = this->containerNameText->GetValue();
        if (containerFileName.Length()==0) {
            wxString msg = "You chose to create a new container, please enter a name for that container";
            wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
            dlg->ShowModal();
            return;
        }
        wxString containerFilePath = GlobalSettings::GetContainerFolder() + wxFileName::GetPathSeparator() + containerFileName;
        if (wxDirExists(containerFilePath)) {
            if (wxDir::GetTotalSize(containerFilePath)!=0) {
                wxString msg = "You chose to create a new container, but a folder with the name you entered already exists: "+containerFileName;
                wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
                dlg->ShowModal();
                return;
            }            
        } else if (!wxMkdir(containerFilePath)) {
            wxString msg = "Failed to create a direcotry for the new container: "+ wxSysErrorMsgStr(wxSysErrorCode());
            wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
            dlg->ShowModal();
            return;
        }    
        wxString wineName = this->wineVersionComboBox->GetValue();
        wxString fileSystem;
        for (auto& wineVersion : GlobalSettings::wineVersions) {
            if (wineVersion.name == wineName) {
                fileSystem = wineVersion.fileName;
                break;
            }
        }
        container = BoxedContainer::CreateContainer(containerFilePath, containerFileName, fileSystem);
        if (this->runWineConfigCheckBox->IsChecked()) {
            container->LaunchWine("winecfg", "", "/home/username", false, false);
        }
        this->containers.push_back(container);
    }     
    if (filePath.Length()) {
        container->Launch(filePath, "", "/home/username", true, false);
        PickAppDialog *dlg = new PickAppDialog(this, container);
        dlg->Show(true);
        if (dlg->GetReturnCode()==wxID_OK) {
        }
    }
    event.Skip();
}