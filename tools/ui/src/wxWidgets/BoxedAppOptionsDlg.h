#ifndef __BOXEDAPPOPTIONS_H__
#define __BOXEDAPPOPTIONS_H__

#include "BoxedApp.h"

class wxListView;

class BoxedAppOptionsDialog : public wxDialog {
public:
    BoxedAppOptionsDialog(wxWindow* parent, BoxedApp* app);

private:
    void OnDone(wxCommandEvent& event);    

    BoxedApp* app;
    wxTextCtrl* shortCutNameText;
    wxTextCtrl* glExtText;
    wxComboBox* screenResolutionCombobox;
    wxComboBox* screenBppCombobox;
    wxComboBox* scaleCombobox;
    wxComboBox* scaleQualityCombobox;
    wxCheckBox* fullScreenCheckBox;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(BoxedAppOptionsDialog);
};

#endif