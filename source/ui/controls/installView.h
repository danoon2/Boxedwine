#ifndef __INSTALL_VIEW_H__
#define __INSTALL_VIEW_H__

class InstallView : public BaseView {
public:
	InstallView(const std::string& initialFileOrDirPath, std::string tab="");

	virtual bool saveChanges();

private:
    void createInstallTab(const std::string& initialFileOrDirPath);
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
    std::shared_ptr<LayoutComboboxControl> wineVersionControl;
    std::shared_ptr<LayoutComboboxControl> windowsVersionControl;
};

#endif