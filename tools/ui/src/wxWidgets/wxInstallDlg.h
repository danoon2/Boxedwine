#ifndef __WXINSTALLDLG_H__
#define __WXINSTALLDLG_H__

#include "BoxedContainer.h"

enum InstallType {
    INSTALL_TYPE_FILE,
    INSTALL_TYPE_DIR,
    INSTALL_TYPE_BLANK
};
class InstallDialog : public wxDialog {
public:
    InstallDialog(wxWindow* parent, const wxString& filePathToInstall, std::vector<BoxedContainer*>& containers);

private:    
    void OnBrowseExeButtonClicked(wxCommandEvent& event);
    void OnContainerComboBoxUpdate(wxCommandEvent& event);
    void OnInstallTypeComboBoxUpdate(wxCommandEvent& event);
    void OnDone(wxCommandEvent& event);
    InstallType GetInstallType();

    std::vector<BoxedContainer*>& containers;
    wxTextCtrl* setupFileLocationText;
    wxTextCtrl* containerNameText;
    wxComboBox* containerComboBox;
    wxComboBox* wineVersionComboBox;
    wxCheckBox* runWineConfigCheckBox;
    wxComboBox* installationTypeComboBox;
    wxStaticText* installInstructionText;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(InstallDialog);
};

#endif