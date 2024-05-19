#ifndef __OPTIONS_VIEW_H__
#define __OPTIONS_VIEW_H__

#include <map>

class OptionsViewWineVersion {
public:
	OptionsViewWineVersion() = default;
	std::shared_ptr<FileSystemZip> currentVersion;
	std::shared_ptr<FileSystemZip> availableVersion;
	BString name;
	BString size;
};

class OptionsView : public BaseView {
public:
	OptionsView(BString tab);

	// from BaseView
	bool saveChanges() override;

private:
	void createGeneralTab();
	void createThemeTab();

	void runWineOptions();	
	void download(const std::shared_ptr<FileSystemZip>& version);
	void loadFileSystemVersions();

	const char* wineTitle;
	float wineButtonTotalColumnWidth = 0.0f;
	float wineButtonFirstColumnWidth = 0.0f;
	std::map<BString, OptionsViewWineVersion, std::greater<BString>> fileSystemVersions;

	// General
	std::shared_ptr<LayoutTextInputControl> saveLocationControl;
	std::shared_ptr<LayoutComboboxControl> resolutionControl;
	std::shared_ptr<LayoutComboboxControl> vsyncControl;
	std::shared_ptr<LayoutComboboxControl> scaleControl;
	std::shared_ptr<LayoutComboboxControl> openGlControl;
	std::shared_ptr<LayoutCheckboxControl> automationControl;

	// Display
	std::shared_ptr<LayoutComboboxControl> themeControl;
};

#endif