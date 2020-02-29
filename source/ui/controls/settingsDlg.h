#ifndef __SETTINGS_DLG_H__
#define __SETTINGS_DLG_H__

class SettingsDlg : public BaseDlg {
public:
    SettingsDlg();

protected:
    virtual void run();
    virtual void onOk(bool buttonClicked);
    virtual void done();

private:
    const char* saveFolderLabel;
    const char* saveFolderHelp;
    const char* browseButtonText;
    const char* themeLabel;
    const char* themeHelp;
    ComboboxData themeComboboxData;
    char saveFolderLocationBuffer[1024];
    ImVec2 leftColumnWidth;
    ImVec2 rightColumnWidth;
    const char* errorMsg;
};

#endif