#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/combobox.h"
#include "wx/display.h"

#include "BoxedAppOptionsDlg.h"
#include "GlobalSettings.h"

wxBEGIN_EVENT_TABLE(BoxedAppOptionsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, BoxedAppOptionsDialog::OnDone)
wxEND_EVENT_TABLE()

BoxedAppOptionsDialog::BoxedAppOptionsDialog(wxWindow* parent, BoxedApp* app) : wxDialog(parent, -1, "App Options"), app(app) {    
    wxFlexGridSizer* fSizer = new wxFlexGridSizer(2);    
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    vbox->Add(fSizer);
    vbox->AddSpacer(20);
    vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    fSizer->Add(new wxStaticText(this, -1, "Shortcut Name:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->shortCutNameText = new wxTextCtrl(this, -1, app->GetName(), wxDefaultPosition, wxSize(300*GlobalSettings::GetScaleFactor(), -1));
    fSizer->Add(this->shortCutNameText, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    wxArrayString options;
    int width = wxDisplay().GetGeometry().GetWidth()/GlobalSettings::GetScaleFactor();
    int height = wxDisplay().GetGeometry().GetHeight()/GlobalSettings::GetScaleFactor();
    wxString defaultResolutionOption;
    options.Add("640x480");
    options.Add("800x600");
    defaultResolutionOption = options[1];
    if (width>1024 && height>768) {
        options.Add("1024x768");
        defaultResolutionOption = options[2];
    }
    if (width>1280 && height>1024) {
        options.Add("1280x1024");
    }
    if (width>1920 && height>1080) {
        options.Add("1920x1080");
    }
    fSizer->Add(new wxStaticText(this, -1, "Emulated Screen Resolution:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->screenResolutionCombobox = new wxComboBox(this, wxID_ANY, (app->resolution.Length()>0?app->resolution:defaultResolutionOption), wxDefaultPosition, wxDefaultSize, options, wxCB_READONLY);
    fSizer->Add(this->screenResolutionCombobox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    wxArrayString bppOptions;
    bppOptions.Add("8-bit (256 colors)");
    bppOptions.Add("16-bit");
    bppOptions.Add("32-bit (default)");
    wxString defaultOption;
    if (app->bpp==8) {
        defaultOption = bppOptions[0];
    } else if (app->bpp==16) {
        defaultOption = bppOptions[1];
    } else {
        defaultOption = bppOptions[2];
    }
    fSizer->Add(new wxStaticText(this, -1, "Emulated Screen Bits Per Pixel (only if game requires a specific bpp):", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->screenBppCombobox = new wxComboBox(this, wxID_ANY, defaultOption, wxDefaultPosition, wxDefaultSize, bppOptions, wxCB_READONLY);
    fSizer->Add(this->screenBppCombobox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    fSizer->Add(new wxStaticText(this, -1, "Full Screen:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->fullScreenCheckBox = new wxCheckBox(this, -1, "(Does not apply to OpenGL)");
    this->fullScreenCheckBox->SetValue(app->fullScreen);
    fSizer->Add(this->fullScreenCheckBox, wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));

    wxArrayString scaleOptions;
    wxString scaleDefaultOption;
    scaleOptions.Add("1/2x");
    scaleOptions.Add("1x (default)");
    scaleOptions.Add("1.5x");
    scaleOptions.Add("2x");
    scaleOptions.Add("3x");

    if (app->scale == 50) {
        scaleDefaultOption = scaleOptions[0];
    } else if (app->scale == 100) {
        scaleDefaultOption = scaleOptions[1];
    } else if (app->scale == 150) {
        scaleDefaultOption = scaleOptions[2];
    } else if (app->scale == 200) {
        scaleDefaultOption = scaleOptions[3];
    } else if (app->scale == 300) {
        scaleDefaultOption = scaleOptions[4];
    } else {
        scaleDefaultOption = scaleOptions[1];
    }
    fSizer->Add(new wxStaticText(this, -1, "Scale (useful for small resolution games):", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->scaleCombobox = new wxComboBox(this, wxID_ANY, scaleDefaultOption, wxDefaultPosition, wxDefaultSize, scaleOptions, wxCB_READONLY);
    fSizer->Add(this->scaleCombobox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    wxArrayString qualityOptions;
    qualityOptions.Add("Nearest pixel sampling (default)");
    qualityOptions.Add("Linear filtering");
    if (app->scaleQuality<0 || app->scaleQuality>1) {
        app->scaleQuality = 0;
    }
    fSizer->Add(new wxStaticText(this, -1, "Scale Quality:", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->scaleQualityCombobox = new wxComboBox(this, wxID_ANY, qualityOptions[app->scaleQuality], wxDefaultPosition, wxDefaultSize, qualityOptions, wxCB_READONLY);
    fSizer->Add(this->scaleQualityCombobox, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    fSizer->Add(new wxStaticText(this, -1, "Allowed OpenGL Extensions (rarely need to set):", wxDefaultPosition, wxDefaultSize), wxSizerFlags().Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).DoubleBorder());
    this->glExtText = new wxTextCtrl(this, -1, app->glExt, wxDefaultPosition, wxSize(300*GlobalSettings::GetScaleFactor(), -1));
    this->glExtText->SetHint("If empty, then all extension allowed");
    fSizer->Add(this->glExtText, wxSizerFlags().Expand().Align(wxALIGN_CENTER_VERTICAL).DoubleBorder(wxRIGHT));

    hbox->Add(new wxButton(this, wxID_OK, "OK" ));
    hbox->AddSpacer(10);
    hbox->Add(new wxButton(this, wxID_CANCEL, "Cancel" ));
   
    this->SetSizerAndFit(vbox);

    Centre();
    ShowModal();

    Destroy();
}    

void BoxedAppOptionsDialog::OnDone(wxCommandEvent& event) {
    if (this->shortCutNameText->GetValue().Length()==0) {
        wxString msg = "Shortcut name may not be blank.";
        wxMessageDialog *dlg = new wxMessageDialog(NULL, msg, "Error", wxOK | wxICON_ERROR);
        dlg->ShowModal();
        return;
    }
    this->app->name = this->shortCutNameText->GetValue();
    this->app->resolution = this->screenResolutionCombobox->GetValue();
    if (this->screenBppCombobox->GetSelection()==0) {
        this->app->bpp = 8;
    } else if (this->screenBppCombobox->GetSelection()==1) {
        this->app->bpp = 16;
    } else {
        this->app->bpp = 32;
    }
    this->app->fullScreen = this->fullScreenCheckBox->GetValue();
    if (this->scaleCombobox->GetSelection()==0) {
        this->app->scale = 50;
    } else if (this->scaleCombobox->GetSelection()==1) {
        this->app->scale = 100;
    } else if (this->scaleCombobox->GetSelection()==2) {
        this->app->scale = 150;
    } else if (this->scaleCombobox->GetSelection()==3) {
        this->app->scale = 200;
    } else if (this->scaleCombobox->GetSelection()==4) {
        this->app->scale = 300;
    }
    this->app->scaleQuality = this->scaleQualityCombobox->GetSelection();
    this->app->glExt = this->glExtText->GetValue();
    this->app->SaveApp();
    event.Skip();
}
