#ifndef __WXINSTALLDLG_H__
#define __WXINSTALLDLG_H__

#include "BoxedContainer.h"

class InstallDialog : public wxDialog {
public:
    InstallDialog(wxWindow* parent, const wxString& filePathToInstall, std::vector<BoxedContainer*>& containers);

private:    
    void OnBrowseExeButtonClicked(wxCommandEvent& event);
    void OnComboBoxUpdate(wxCommandEvent& event);
    void OnDone(wxCommandEvent& event);

    std::vector<BoxedContainer*>& containers;
    wxTextCtrl* setupFileLocationText;
    wxTextCtrl* containerNameText;
    wxComboBox* containerComboBox;
    wxComboBox* wineVersionComboBox;
    wxCheckBox* runWineConfigCheckBox;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(InstallDialog);
};

#endif