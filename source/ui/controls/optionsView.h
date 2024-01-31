#ifndef __OPTIONS_VIEW_H__
#define __OPTIONS_VIEW_H__

#include <map>

class OptionsViewWineVersion {
public:
	OptionsViewWineVersion() = default;
	WineVersion* currentVersion = nullptr;
	WineVersion* availableVersion = nullptr;
	BString name;
	BString size;
};

class OptionsView : public BaseView {
public:
	OptionsView(BString tab);

	virtual bool saveChanges();

private:
	void createGeneralTab();
	void createThemeTab();

	void runWineOptions();	
	void download(WineVersion* version);
	void loadWineVersions();

	const char* wineTitle;
	float leftColumnWidthWine;
	float rightColumnWidth;	
	float wineButtonTotalColumnWidth;
	float wineButtonFirstColumnWidth;
	std::map<BString, OptionsViewWineVersion, std::greater<BString>> wineVersions;

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