#ifndef __CONTAINERS_VIEW_H__
#define __CONTAINERS_VIEW_H__

class ContainersView : public BaseView {
public:
	ContainersView(const char* startingTab);

	virtual bool saveChanges();

private:
	void runContainerView(BoxedContainer* container, bool buttonPressed, BaseViewTab& tab);

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

    char containerName[256];
    char containerLocation[1024];
    char mountLocation[1024];
    const char* wineVersionLabel;
    const char* wineVersionHelp;
    const char* addAppLabel;
    const char* addAppHelp;
    const char* addAppButtonLabel;
    ImVec2 leftColumnWidth;
    ComboboxData wineVersionComboboxData;
    ComboboxData mountDriveComboboxData;

    BoxedContainer* currentContainer;
    bool currentContainerChanged;
    bool currentContainerMountChanged;
};

#endif