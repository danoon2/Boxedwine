#ifndef __CONTAINERS_VIEW_H__
#define __CONTAINERS_VIEW_H__

class ContainersView : public BaseView {
public:
	ContainersView(const char* startingTab);

	virtual bool saveChanges();

private:
	void runContainerView(BoxedContainer* container, bool buttonPressed, BaseViewTab& tab);
    void setCurrentApp(BoxedApp* app);
    void setCurrentContainer(BoxedContainer* container);
    void rebuildShortcutsCombobox();

    const char* containerNameLabel;
    const char* containerLocationLabel;
    const char* containerLocationSizeLabel;
    const char* containerLocationOpenLabel;
    const char* containerDeleteButtonLabel;
    const char* containerDeleteButtonHelp;
    const char* containerMountFolderLabel;
    const char* containerMountFolderHelp;
    const char* browseButtonText;    
    float containerLocationOpenLabelButtonWidth;
    float browseButtonWidth;
    float deleteShortcutButtonWidth;

    char containerName[256];
    char containerLocation[1024];
    char mountLocation[1024];
    const char* wineVersionLabel;
    const char* wineVersionHelp;
    const char* addAppLabel;
    const char* addAppHelp;
    const char* addAppButtonLabel;
    ImVec2 leftColumnWidth;
    ImVec2 innerColumnWidth;    
    ComboboxData wineVersionComboboxData;
    ComboboxData mountDriveComboboxData;

    ComboboxData shortcutsComboboxData;
    const char* shortcutListLabel;
    const char* deleteShortcutLabel;
    const char* shortcutHelp;
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
    const char* pathLabel;
    const char* pathHelp;

    ComboboxData resolutionComboboxData;
    ComboboxData bppComboboxData;
    ComboboxData scaleComboboxData;
    ComboboxData scaleQualityComboboxData;

    char appName[256];
    char glExt[2048];
    char path[1024];
    bool fullScreen;

    BoxedContainer* currentContainer;
    bool currentContainerChanged;
    bool currentContainerMountChanged;

    BoxedApp* currentApp;
    bool currentAppChanged;
};

#endif