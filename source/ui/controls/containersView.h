/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __CONTAINERS_VIEW_H__
#define __CONTAINERS_VIEW_H__

class ContainersView : public BaseView {
public:
	ContainersView(BString tab, BString app);

    // from BaseView
	bool saveChanges() override;
    
private:
    void setCurrentApp(BoxedApp* app);
    void setCurrentContainer(BoxedContainer* container);
    void rebuildShortcutsCombobox();
    void showAppSection(bool show);
    void deleteContainer(BoxedContainer* container);
    void winetricks(const std::shared_ptr<FileSystemZip>& winetricks, BString verb);

    BoxedContainer* currentContainer;
    bool currentContainerChanged;
    bool currentContainerMountChanged;

    BoxedApp* currentApp;
    bool currentAppChanged;
    BString gotoApp;

    std::shared_ptr<LayoutSection> section;
    std::shared_ptr<LayoutTextInputControl> containerNameControl;
    std::shared_ptr<LayoutComboboxControl> containerFileSystemControl;
    std::shared_ptr<LayoutComboboxControl> containerWindowsVersionControl;
    std::shared_ptr<LayoutCheckboxControl> containerGdiControl;
    std::shared_ptr<LayoutComboboxControl> containerRendererControl;

#ifdef _DEBUG
    std::shared_ptr<LayoutTextInputControl> videoMemorySizeControl;
#endif

    std::shared_ptr<LayoutComboboxControl> containerMouseWarpControl;
    std::shared_ptr<LayoutComboboxControl> containerMountDriveControl;
    std::shared_ptr<LayoutTextInputControl> containerMountPathControl;
    std::shared_ptr<LayoutTextInputControl> containerLocationControl;
    std::shared_ptr<LayoutComboboxControl> componentsControl;
    std::shared_ptr<LayoutComboboxControl> fontsControl;
    std::shared_ptr<LayoutComboboxControl> dllsControl;
    std::shared_ptr<LayoutComboboxControl> packagesControl;

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
    std::shared_ptr<LayoutCheckboxControl> appDdrawOverrideControl;
    std::shared_ptr<LayoutCheckboxControl> appEnableDXVKControl;
    std::shared_ptr<LayoutCheckboxControl> appDisableHideCursorControl;
    std::shared_ptr<LayoutCheckboxControl> appForceRelativeMouseControl;
    std::shared_ptr<LayoutTextInputControl> appPollRateControl;
    std::shared_ptr<LayoutTextInputControl> appSkipFramesControl;
#ifdef BOXEDWINE_MULTI_THREADED
    std::shared_ptr<LayoutComboboxControl> appCpuAffinityControl;
#endif
    std::shared_ptr<LayoutComboboxControl> appScaleControl;
    std::shared_ptr<LayoutComboboxControl> appScaleQualityControl;
    std::shared_ptr<LayoutComboboxControl> appOpenGlControl;
    std::shared_ptr<LayoutTextInputControl> appGlExControl;
#ifdef _DEBUG
    // experimental, currently patch for file system not available for wine 9 or later
    std::shared_ptr<LayoutCheckboxControl> appDirectDrawAutoRefreshControl;
#endif
};

#endif
