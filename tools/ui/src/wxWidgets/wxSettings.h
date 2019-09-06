#ifndef __WXSETTINGS_H__
#define __WXSETTINGS_H__

class SettingsDialog : public wxDialog
{
public:
    SettingsDialog(wxWindow* parent, const wxString& exeFileLocation, const wxString& dataFolderLocation);

    wxTextCtrl* exeFileLocationText;
    wxTextCtrl* dataFolderLocationText;
private:
    void OnBrowseExeButtonClicked(wxCommandEvent& event);
    void OnBrowseDataButtonClicked(wxCommandEvent& event);    
};

#endif