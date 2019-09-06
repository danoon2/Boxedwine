#ifndef __WXINSTALLDLG_H__
#define __WXINSTALLDLG_H__

#include "BoxedContainer.h"

class InstallDialog : public wxDialog {
public:
    InstallDialog(wxWindow* parent, const wxString& filePathToInstall, std::vector<BoxedContainer*>& containers);

private:    
    void OnBrowseExeButtonClicked(wxCommandEvent& event);
    void OnComboBoxUpdate(wxCommandEvent& event);

    std::vector<BoxedContainer*>& containers;
    wxTextCtrl* setupFileLocationText;
    wxTextCtrl* containerNameText;
    wxComboBox* containerComboBox;
    wxComboBox* wineVersionComboBox;
    wxCheckBox* runWineConfigCheckBox;
};

#endif