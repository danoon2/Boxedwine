#ifndef __APP_OPTIONS_H__
#define __App_OPTIONS_H__

class AppOptionsDlg : public BaseDlg {
public:
	AppOptionsDlg(BoxedApp* app);

protected:
    virtual void run();
    virtual void onOk(bool buttonClicked);

private:
	BoxedApp* app;

    const char* nameLabel;
    const char* nameHelp;
    const char* resolutionLabel;
    const char* resolutionHelp;
    const char* bppLabel;
    const char* bppHelp;
    const char* fullscreenLabel;
    const char* fullscreenHelp;
    const char* scaleLabel;
    const char* scaleHelp;
    const char* scaleQualityLabel;
    const char* scaleQualityHelp;
    const char* glExtenstionsLabel;
    const char* glExtenstionsSetButtonLabel;
    const char* glExtensionsHelp;

    ImVec2 leftColumnWidth;
    ComboboxData resolutionComboboxData;
    ComboboxData bppComboboxData;
    ComboboxData scaleComboboxData;
    ComboboxData scaleQualityComboboxData;

    char appName[256];
    char glExt[2048];

    const char* errorMsg;
    std::string errorMsgString;
};

#endif