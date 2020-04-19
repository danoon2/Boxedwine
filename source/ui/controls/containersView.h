#ifndef __CONTAINERS_VIEW_H__
#define __CONTAINERS_VIEW_H__

class ContainersView : public BaseView {
public:
	ContainersView(std::string tab);

	virtual bool saveChanges();
    
private:
    void setCurrentApp(BoxedApp* app);
    void setCurrentContainer(BoxedContainer* container);
    void rebuildShortcutsCombobox();

    BoxedContainer* currentContainer;
    bool currentContainerChanged;
    bool currentContainerMountChanged;

    BoxedApp* currentApp;
    bool currentAppChanged;


    std::shared_ptr<LayoutSection> section;
    std::shared_ptr<LayoutTextInputControl> containerNameControl;
    std::shared_ptr<LayoutComboboxControl> containerWineVersionControl;
    std::shared_ptr<LayoutComboboxControl> containerWindowsVersionControl;
    std::shared_ptr<LayoutCheckboxControl> containerGdiControl;
    std::shared_ptr<LayoutComboboxControl> containerMountDriveControl;
    std::shared_ptr<LayoutTextInputControl> containerMountPathControl;
    std::shared_ptr<LayoutTextInputControl> containerLocationControl;
    std::shared_ptr<LayoutButtonControl> containerWineCfgButtonControl;
    std::shared_ptr<LayoutButtonControl> containerRegeditButtonControl;

    std::shared_ptr<LayoutButtonControl> containerNewShortcutButtonControl;

    std::shared_ptr<LayoutSection> appSection;
    std::shared_ptr<LayoutComboboxControl> appPickerControl;
    std::shared_ptr<LayoutTextInputControl> appNameControl;
    std::shared_ptr<LayoutTextInputControl> appPathControl;
    std::shared_ptr<LayoutTextInputControl> appArgumentsControl;
    std::shared_ptr<LayoutComboboxControl> appResolutionControl;
    std::shared_ptr<LayoutComboboxControl> appBppControl;
    std::shared_ptr<LayoutCheckboxControl> appFullScreenControl;
    std::shared_ptr<LayoutComboboxControl> appScaleControl;
    std::shared_ptr<LayoutComboboxControl> appScaleQualityControl;
    std::shared_ptr<LayoutTextInputControl> appGlExControl;
};

#endif