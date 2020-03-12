#ifndef __INSTALL_DLG_H__
#define __INSTALL_DLG_H__

class InstallDlg : public BaseDlg {
public:
    InstallDlg();
    InstallDlg(const std::string& initialFileOrDirPath);

protected:
    virtual void run();
    virtual void onOk(bool buttonClicked);

private:    
    const char* installLabelText;
    const char* containerLabelText;
    const char* locationLabelText;
    const char* browseButtonText;
    const char* installTypeHelp;
    const char* containerHelp;
    const char* containerNameHelp;
    const char* wineVersionHelp;
    const char* wineConfigHelp;

    ImVec2 leftColumnWidth;
    ImVec2 rightColumnWidth;

    char locationBuffer[1024];
    int lastInstallType;
    char containerName[256];
    bool runWineConfig;
    ComboboxData installTypeComboboxData;
    ComboboxData containerComboboxData;
    ComboboxData wineVersionComboboxData;

    const char* errorMsg;
    std::string errorMsgString;
};

#endif