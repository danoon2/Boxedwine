#ifndef __CONTAINERS_VIEW_H__
#define __CONTAINERS_VIEW_H__

class ContainersView : public BaseView {
public:
	ContainersView(std::string tab, std::string app);

	virtual bool saveChanges();
    
private:
    void setCurrentApp(BoxedApp* app);
    void setCurrentContainer(BoxedContainer* container);
    void rebuildShortcutsCombobox();
    void showAppSection(bool show);
    void deleteContainer(BoxedContainer* container);
    void winetricks(const WineVersion& winetricks, const std::string& verb);

    BoxedContainer* currentContainer;
    bool currentContainerChanged;
    bool currentContainerMountChanged;

    BoxedApp* currentApp;
    bool currentAppChanged;
    std::string gotoApp;

    std::shared_ptr<LayoutSection> section;
    std::shared_ptr<LayoutTextInputControl> containerNameControl;
    std::shared_ptr<LayoutComboboxControl> containerWineVersionControl;
    std::shared_ptr<LayoutComboboxControl> containerWindowsVersionControl;
    std::shared_ptr<LayoutCheckboxControl> containerGdiControl;
    std::shared_ptr<LayoutComboboxControl> containerRendererControl;
    std::shared_ptr<LayoutComboboxControl> containerMouseWarpControl;
    std::shared_ptr<LayoutComboboxControl> containerMountDriveControl;
    std::shared_ptr<LayoutTextInputControl> containerMountPathControl;
    std::shared_ptr<LayoutTextInputControl> containerLocationControl;
    std::shared_ptr<LayoutComboboxControl> componentsControl;
    std::shared_ptr<LayoutComboboxControl> fontsControl;
    std::shared_ptr<LayoutComboboxControl> dllsControl;

    std::shared_ptr<LayoutButtonControl> containerNewShortcutButtonControl;

    std::shared_ptr<LayoutSection> appSection;
    std::shared_ptr<LayoutComboboxControl> appPickerControl;
    std::shared_ptr<LayoutTextInputControl> appNameControl;
    std::shared_ptr<LayoutTextInputControl> appPathControl;
    std::shared_ptr<LayoutTextInputControl> appArgumentsControl;
    std::shared_ptr<LayoutComboboxControl> appResolutionControl;
    std::shared_ptr<LayoutComboboxControl> appBppControl;
    std::shared_ptr<LayoutComboboxControl> appFullScreenControl;
    std::shared_ptr<LayoutComboboxControl> appVSyncControl;
    std::shared_ptr<LayoutCheckboxControl> appDpiAwareControl;
    std::shared_ptr<LayoutTextInputControl> appPollRateControl;
    std::shared_ptr<LayoutTextInputControl> appSkipFramesControl;
#ifdef BOXEDWINE_MULTI_THREADED
    std::shared_ptr<LayoutComboboxControl> appCpuAffinityControl;
#endif
    std::shared_ptr<LayoutComboboxControl> appScaleControl;
    std::shared_ptr<LayoutComboboxControl> appScaleQualityControl;
    std::shared_ptr<LayoutComboboxControl> appOpenGlControl;
    std::shared_ptr<LayoutTextInputControl> appGlExControl;
    std::shared_ptr<LayoutCheckboxControl> appShowWindowImmediatelyControl;
};

#endif
