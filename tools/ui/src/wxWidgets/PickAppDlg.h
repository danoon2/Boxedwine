#ifndef __PICKAPPDLG_H__
#define __PICKAPPDLG_H__

#include "BoxedContainer.h"

class wxListView;

class PickAppDialog : public wxDialog {
public:
    PickAppDialog(wxWindow* parent, BoxedContainer* container);

private:
    void OnDone(wxCommandEvent& event);    

    BoxedContainer* container;
    wxListView* listView;
    std::vector<BoxedApp> apps;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(PickAppDialog);
};

#endif