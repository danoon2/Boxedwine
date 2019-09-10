#ifndef __BOXEDCONTAINEROPTIONS_H__
#define __BOXEDCONTAINEROPTIONS_H__

#include "BoxedContainer.h"

class wxListView;

class BoxedContainerOptionsDialog : public wxDialog {
public:
    BoxedContainerOptionsDialog(wxWindow* parent, BoxedContainer* container);

private:
    void OnDone(wxCommandEvent& event);    

    BoxedContainer* container;
    wxComboBox* wineVersionCombobox;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(BoxedContainerOptionsDialog);
};

#endif