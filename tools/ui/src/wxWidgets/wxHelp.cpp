#include <wx/wx.h>
#include <wx/hyperlink.h>

#include "wxHelp.h"

HelpDialog::HelpDialog(wxWindow* parent) : wxDialog(parent, -1, "Help") {
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
    hbox2->Add(new wxStaticText(this, -1, "More information at ", wxDefaultPosition, wxDefaultSize), wxSizerFlags().DoubleBorder(wxLEFT|wxTOP));
    hbox2->Add(new wxHyperlinkCtrl(this, -1, "http://boxedwine.org", "http://boxedwine.org"), wxSizerFlags().DoubleBorder(wxTOP));
    vbox->Add(hbox2);
    vbox->Add(new wxStaticText(this, -1, "Install:\r\nYou can tap the button or drag the app you want to install onto Boxedwine.\r\n\r\nApps:\r\nYou can see more options, like launch and show console, on the apps by right-clicking them\r\n\r\nContainers:\r\nYou can see more options, like edit and delete, on the containers by right-clicking them", wxDefaultPosition, wxDefaultSize), wxSizerFlags().DoubleBorder());
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    hbox->Add(new wxButton(this, wxID_OK, "OK" ));   
    this->SetSizerAndFit(vbox);

    Centre();
    ShowModal();

    Destroy();
}