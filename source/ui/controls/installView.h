#ifndef __INSTALL_VIEW_H__
#define __INSTALL_VIEW_H__

class InstallView : public BaseView {
public:
	InstallView(const std::string& initialFileOrDirPath, const char* startingTab=NULL);

	virtual bool saveChanges();

private:
    void runInstallView();
    void runDemoView();
    void onInstall();

    const char* installTitle;
    const char* demoTitle;

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
};

#endif