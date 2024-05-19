#ifndef __INSTALL_VIEW_H__
#define __INSTALL_VIEW_H__

class InstallView : public BaseView {
public:
	InstallView(BString initialFileOrDirPath, BString tab=BString());

    // from BaseView
	bool saveChanges() override;

private:
    void createInstallTab(BString initialFileOrDirPath);
    void createDemoTab();
    void runApps(std::vector<AppFile>& apps);
    void setContainerName();
    void setWindowsVersionDefault();

    void onInstall();   

    std::shared_ptr<LayoutComboboxControl> installTypeControl;
    std::shared_ptr<LayoutTextInputControl> locationControl;
    std::shared_ptr<LayoutComboboxControl> containerControl;
    std::shared_ptr<LayoutSection> containerSection;
    std::shared_ptr<LayoutTextInputControl> containerNameControl;
    std::shared_ptr<LayoutComboboxControl> fileSystemVersionControl;
    std::shared_ptr<LayoutComboboxControl> windowsVersionControl;
};

#endif