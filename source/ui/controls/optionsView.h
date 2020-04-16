#ifndef __OPTIONS_VIEW_H__
#define __OPTIONS_VIEW_H__

#include <map>

class OptionsViewWineVersion {
public:
	OptionsViewWineVersion() : currentVersion(NULL), availableVersion(NULL) {}
	WineVersion* currentVersion;
	WineVersion* availableVersion;
	std::string name;
};

class OptionsView : public BaseView {
public:
	OptionsView(const char* startingTab);

	virtual bool saveChanges();

private:
	void runDisplayOptions();
	void runGeneralOptions();
	void runWineOptions();	
	void download(WineVersion* version);
	void loadWineVersions();

	const char* saveFolderLabel;
	const char* saveFolderHelp;
	const char* browseButtonText;
	const char* themeLabel;
	const char* themeHelp;
	const char* generalTitle;
	const char* displayTitle;
	const char* wineTitle;
	const char* defaultResolutionLabel;
	const char* defaultResolutionHelp;
	const char* defaultScaleLabel;
	const char* defaultScaleHelp;
	ComboboxData themeComboboxData;
	ComboboxData resolutionComboboxData;
	ComboboxData scaleComboboxData;
	char saveFolderLocationBuffer[1024];
	float leftColumnWidthGeneral;
	float leftColumnWidthDisplay;
	float leftColumnWidthWine;
	float rightColumnWidth;	
	int lastThemeSelectionIndex;
	float wineButtonTotalColumnWidth;
	float wineButtonFirstColumnWidth;
	std::map<std::string, OptionsViewWineVersion, std::greater<std::string>> wineVersions;
};

#endif