#ifndef __OPTIONS_VIEW_H__
#define __OPTIONS_VIEW_H__

#include <map>

class OptionsViewWineVersion {
public:
	OptionsViewWineVersion() : currentVersion(NULL), availableVersion(NULL) {}
	WineVersion* currentVersion;
	WineVersion* availableVersion;
	std::string name;
	std::string size;
};

class OptionsView : public BaseView {
public:
	OptionsView(std::string tab);

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
	std::map<std::string, OptionsViewWineVersion, std::greater<std::string>> wineVersions;

	// General
	std::shared_ptr<LayoutTextInputControl> saveLocationControl;
	std::shared_ptr<LayoutComboboxControl> resolutionControl;
	std::shared_ptr<LayoutComboboxControl> vsyncControl;
	std::shared_ptr<LayoutComboboxControl> scaleControl;

	// Display
	std::shared_ptr<LayoutComboboxControl> themeControl;
};

#endif