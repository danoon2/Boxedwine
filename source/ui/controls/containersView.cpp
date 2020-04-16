#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

ContainersView::ContainersView(const char* startingTab) : BaseView("ContainersView"), currentContainer(NULL), currentContainerChanged(false), currentContainerMountChanged(false) {
    for (auto& item : BoxedwineData::getContainers()) {
        addTab(item->getName(), [this, item](bool buttonPressed, BaseViewTab& tab) {
            this->runContainerView(item, buttonPressed, tab);
            });
    }

    this->wineVersionLabel = getTranslation(COMMON_WINE_VERSION_LABEL);
    this->wineVersionHelp = getTranslation(COMMON_WINE_VERSION_HELP, false);
    this->addAppLabel = getTranslation(CONTAINER_OPTIONS_DLG_ADD_APP_LABEL);
    this->addAppHelp = getTranslation(CONTAINER_OPTIONS_DLG_ADD_APP_HELP, false);
    this->addAppButtonLabel = getTranslation(CONTAINER_OPTIONS_DLG_ADD_APP_BUTTON_LABEL);
    this->containerNameLabel = getTranslation(CONTAINER_VIEW_CONTAINER_NAME_LABEL);
    this->containerLocationLabel = getTranslation(CONTAINER_VIEW_CONTAINER_LOCATION_LABEL);
    this->containerLocationOpenLabel = getTranslation(GENERIC_OPEN_BUTTON);
    this->containerLocationSizeLabel = getTranslation(CONTAINER_VIEW_CONTAINER_LOCATION_SIZE_LABEL);
    this->containerDeleteButtonLabel = getTranslation(CONTAINER_VIEW_DELETE_BUTTON_LABEL);
    this->containerDeleteButtonHelp = getTranslation(CONTAINER_VIEW_DELETE_BUTTON_HELP);
    this->containerMountFolderLabel = getTranslation(CONTAINER_VIEW_MOUNT_DIR_LABEL);
    this->containerMountFolderHelp = getTranslation(CONTAINER_VIEW_MOUNT_DIR_HELP);
    this->browseButtonText = getTranslation(GENERIC_BROWSE_BUTTON);

    this->shortcutListLabel = getTranslation(CONTAINER_VIEW_SHORTCUT_LIST_LABEL);
    this->deleteShortcutLabel = getTranslation(CONTAINER_VIEW_DELETE_SHORTCUT);
    this->shortcutHelp = getTranslation(CONTAINER_VIEW_SHORTCUT_LIST_HELP);
    this->nameLabel = getTranslation(CONTAINER_VIEW_NAME_LABEL);
    this->nameHelp = getTranslation(CONTAINER_VIEW_NAME_HELP);
    this->resolutionLabel = getTranslation(CONTAINER_VIEW_RESOLUTION_LABEL);
    this->resolutionHelp = getTranslation(CONTAINER_VIEW_RESOLUTION_HELP);
    this->bppLabel = getTranslation(CONTAINER_VIEW_BPP_LABEL);
    this->bppHelp = getTranslation(CONTAINER_VIEW_BPP_HELP);
    this->fullscreenLabel = getTranslation(CONTAINER_VIEW_FULL_SCREEN_LABEL);
    this->fullscreenHelp = getTranslation(CONTAINER_VIEW_FULL_SCREEN_HELP);
    this->scaleLabel = getTranslation(CONTAINER_VIEW_SCALE_LABEL);
    this->scaleHelp = getTranslation(CONTAINER_VIEW_SCALE_HELP);
    this->scaleQualityLabel = getTranslation(CONTAINER_VIEW_SCALE_QUALITY_LABEL);
    this->scaleQualityHelp = getTranslation(CONTAINER_VIEW_SCALE_QUALITY_HELP);
    this->glExtenstionsLabel = getTranslation(CONTAINER_VIEW_GL_EXT_LABEL);
    this->glExtenstionsSetButtonLabel = getTranslation(CONTAINER_VIEW_GL_EXT_SET_BUTTON_LABEL);
    this->glExtensionsHelp = getTranslation(CONTAINER_VIEW_GL_EXT_HELP);    
    this->pathLabel = getTranslation(CONTAINER_VIEW_SHORTCUT_PATH_LABEL);
    this->pathHelp = getTranslation(CONTAINER_VIEW_SHORTCUT_PATH_HELP);

    this->leftColumnWidth = ImGui::CalcTextSize(this->shortcutListLabel);
    ImVec2 width = ImGui::CalcTextSize(this->wineVersionLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->addAppLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->containerNameLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->containerLocationLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->containerLocationSizeLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->containerMountFolderLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    this->leftColumnWidth.x += ImGui::GetStyle().ItemSpacing.x;

    this->innerColumnWidth = ImGui::CalcTextSize(this->nameLabel);
    width = ImGui::CalcTextSize(this->resolutionLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->bppLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->fullscreenLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->scaleLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->scaleQualityLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->glExtenstionsLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->pathLabel);
    if (width.x > this->innerColumnWidth.x) {
        this->innerColumnWidth = width;
    }
    this->innerColumnWidth.x += ImGui::GetStyle().ItemSpacing.x + this->leftColumnWidth.x;

    this->resolutionComboboxData.data.push_back(getTranslation(GENERIC_DEFAULT));
    for (auto& res : GlobalSettings::getAvailableResolutions()) {
        this->resolutionComboboxData.data.push_back(res);
    }
    this->resolutionComboboxData.dataChanged();

    this->bppComboboxData.data.push_back("32-bit (default)");
    this->bppComboboxData.data.push_back("16-bit");
    this->bppComboboxData.data.push_back("8-bit (256 colors)");
    this->bppComboboxData.dataChanged();

    this->scaleComboboxData.data.push_back(getTranslation(GENERIC_DEFAULT));
    this->scaleComboboxData.data.push_back("1/2x");
    this->scaleComboboxData.data.push_back("1x");
    this->scaleComboboxData.data.push_back("2x");
    this->scaleComboboxData.data.push_back("3x");
    this->scaleComboboxData.dataChanged();

    this->scaleQualityComboboxData.data.push_back("Nearest Pixel Sampling (default)");
    this->scaleQualityComboboxData.data.push_back("Linear Filtering");
    this->scaleQualityComboboxData.dataChanged();

    this->containerLocationOpenLabelButtonWidth = ImGui::CalcTextSize(this->containerLocationOpenLabel).x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
    this->browseButtonWidth = ImGui::CalcTextSize(this->browseButtonText).x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;

    this->wineVersionComboboxData.data.clear();
    for (auto& ver : GlobalSettings::getWineVersions()) {
        this->wineVersionComboboxData.data.push_back(ver.name);
    }
    this->wineVersionComboboxData.dataChanged();    
    this->containerName[0] = 0;

    this->mountDriveComboboxData.data.clear();
    this->mountDriveComboboxData.data.push_back(" ");
    for (int i = 3; i < 26; i++) {
        this->mountDriveComboboxData.data.push_back(std::string(1, (char)('A' + i))+":");
    }
    this->mountDriveComboboxData.dataChanged();
    this->mountLocation[0] = 0;     

    this->deleteShortcutButtonWidth = ImGui::CalcTextSize(this->deleteShortcutLabel).x + +ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
}

bool ContainersView::saveChanges() {
    if (this->currentContainer && strlen(this->containerName) == 0) {
        this->errorMsg = getTranslation(CONTAINER_VIEW_ERROR_BLANK_NAME);
    } else if (this->mountDriveComboboxData.currentSelectedIndex != 0 && strlen(this->mountLocation)==0) {
        this->errorMsg = getTranslation(CONTAINER_VIEW_ERROR_MISSING_MOUNT_LOCATION);
    } else if (this->mountDriveComboboxData.currentSelectedIndex == 0 && strlen(this->mountLocation) != 0) {
        this->errorMsg = getTranslation(CONTAINER_VIEW_ERROR_MISSING_MOUNT_DRIVE);
    } else if (this->currentApp && !this->appName[0]) {
        this->errorMsg = getTranslation(CONTAINER_VIEW_NAME_REQUIRED);
    }
    if (!this->errorMsg) {
        if (this->currentContainer && this->currentContainerChanged) {
            if (this->currentContainerMountChanged) {
                this->currentContainer->clearMounts();
                if (this->mountDriveComboboxData.currentSelectedIndex != 0) {
                    std::string drive(1, (char)('c' + this->mountDriveComboboxData.currentSelectedIndex));
                    this->currentContainer->addNewMount(MountInfo(drive, this->mountLocation, true));
                }
            }
            this->currentContainer->saveContainer();
            this->currentContainerChanged = false;            
        }
        if (this->currentApp && this->currentAppChanged) {
            std::string ext = this->glExt;
            stringReplaceAll(ext, "\n", " ");
            stringReplaceAll(ext, "  ", " ");
            stringReplaceAll(ext, "  ", " ");
            this->currentApp->glExt = ext;
            this->currentApp->name = this->appName;
            if (this->resolutionComboboxData.currentSelectedIndex == 0) {
                this->currentApp->resolution = "";
            } else {
                this->currentApp->resolution = this->resolutionComboboxData.data[this->resolutionComboboxData.currentSelectedIndex];
            }
            if (this->bppComboboxData.currentSelectedIndex == 0) {
                this->currentApp->bpp = 32;
            } else if (this->bppComboboxData.currentSelectedIndex == 1) {
                this->currentApp->bpp = 16;
            } else if (this->bppComboboxData.currentSelectedIndex == 2) {
                this->currentApp->bpp = 8;
            }
            if (this->scaleComboboxData.currentSelectedIndex == 0) {
                this->currentApp->scale = 0;
            } else if (this->scaleComboboxData.currentSelectedIndex == 1) {
                this->currentApp->scale = 50;
            } else if (this->scaleComboboxData.currentSelectedIndex == 2) {
                this->currentApp->scale = 100;
            } else if (this->scaleComboboxData.currentSelectedIndex == 3) {
                this->currentApp->scale = 200;
            } else if (this->scaleComboboxData.currentSelectedIndex == 4) {
                this->currentApp->scale = 300;
            }
            this->currentApp->scaleQuality = this->scaleQualityComboboxData.currentSelectedIndex;
            this->currentApp->fullScreen = this->fullScreen;
            this->currentApp->saveApp();
            this->currentAppChanged = false;
            GlobalSettings::reloadApps();
        }
    }
    return this->errorMsg == NULL;
}

void ContainersView::setCurrentApp(BoxedApp* app) {
    this->currentApp = app;

    strncpy(this->appName, app->name.c_str(), sizeof(this->appName));
    this->appName[sizeof(this->appName) - 1] = 0;

    strncpy(this->glExt, app->glExt.c_str(), sizeof(this->glExt));
    this->glExt[sizeof(this->glExt) - 1] = 0;

    if (app->link.length()) {
        strncpy(this->path, app->link.c_str(), sizeof(this->path));
        this->path[sizeof(this->path) - 1] = 0;
    } else {
        strncpy(this->path, app->path.c_str(), sizeof(this->path));
        strncat(this->path, "/", sizeof(this->path));
        strncat(this->path, app->cmd.c_str(), sizeof(this->path));
        this->path[sizeof(this->path) - 1] = 0;
    }

    this->resolutionComboboxData.currentSelectedIndex = stringIndexInVector(app->resolution, this->resolutionComboboxData.data, 0);

    if (app->bpp == 16) {
        this->bppComboboxData.currentSelectedIndex = 1;
    } else if (app->bpp == 8) {
        this->bppComboboxData.currentSelectedIndex = 2;
    } else {
        this->bppComboboxData.currentSelectedIndex = 0;
    }

    if (app->fullScreen) {
        this->scaleComboboxData.currentSelectedIndex = 0;
    } else if (app->scale == 50) {
        this->scaleComboboxData.currentSelectedIndex = 1;
    } else if (app->scale == 100) {
        this->scaleComboboxData.currentSelectedIndex = 2;
    } else if (app->scale == 200) {
        this->scaleComboboxData.currentSelectedIndex = 3;
    } else if (app->scale == 300) {
        this->scaleComboboxData.currentSelectedIndex = 4;
    } else {
        this->scaleComboboxData.currentSelectedIndex = 0;
    }

    if (app->scaleQuality == 1) {
        this->scaleQualityComboboxData.currentSelectedIndex = 1;
    } else {
        this->scaleQualityComboboxData.currentSelectedIndex = 0;
    }

    this->fullScreen = app->fullScreen;
}

void ContainersView::rebuildShortcutsCombobox() {
    this->shortcutsComboboxData.data.clear();
    for (auto& app : this->currentContainer->getApps()) {
        this->shortcutsComboboxData.data.push_back(app->getName());
    }
    this->shortcutsComboboxData.dataChanged();
}

void ContainersView::setCurrentContainer(BoxedContainer* container) {
    this->currentContainer = container;
    this->wineVersionComboboxData.currentSelectedIndex = stringIndexInVector(container->getWineVersion(), this->wineVersionComboboxData.data, 0);
    strncpy(this->containerName, container->getName().c_str(), sizeof(this->containerName));
    this->containerName[sizeof(this->containerName) - 1] = 0;

    strncpy(this->containerLocation, container->getDir().c_str(), sizeof(this->containerLocation));
    this->containerLocation[sizeof(this->containerLocation) - 1] = 0;

    container->updateCachedSize();

    if (!container->getMounts().size() || !container->getMounts()[0].wine || container->getMounts()[0].localPath.length() != 1) {
        this->mountDriveComboboxData.currentSelectedIndex = 0;
        this->mountLocation[0] = 0;
    } else {
        const MountInfo& mount = container->getMounts()[0];
        std::string lowerCase = mount.localPath;
        stringToLower(lowerCase);
        this->mountDriveComboboxData.currentSelectedIndex = (int)(mount.localPath.at(0) - 'a' - 2);
        strncpy(this->mountLocation, mount.nativePath.c_str(), sizeof(this->mountLocation));
        this->mountLocation[sizeof(this->mountLocation) - 1] = 0;
    }
    if (this->currentContainer->getApps().size()) {
        setCurrentApp(this->currentContainer->getApps()[0]);
        rebuildShortcutsCombobox();
    } else {
        this->currentApp = NULL;
    }
}

void ContainersView::runContainerView(BoxedContainer* container, bool buttonPressed, BaseViewTab& tab) {    
    if (buttonPressed) {
        setCurrentContainer(container);
    }
    if (!this->currentApp && this->currentContainer && this->currentContainer->getApps().size() > 0) {
        setCurrentApp(this->currentContainer->getApps()[0]); // can happen if the user has not apps in the container then adds the first one
        rebuildShortcutsCombobox();
    }
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(container->getName().c_str());
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->containerNameLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##ContainerName", this->containerName, sizeof(this->containerName))) {
        container->setName(this->containerName);
        tab.name = this->containerName;
        this->currentContainerChanged = true;
    }
    ImGui::PopItemWidth();
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));    

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->wineVersionLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(GlobalSettings::scaleFloatUI(100.0f));
    //ImGui::PushItemWidth(-1-(this->wineVersionHelp?this->toolTipWidth:0));
    if (ImGui::Combo("##WineCombo", &this->wineVersionComboboxData.currentSelectedIndex, this->wineVersionComboboxData.dataForCombobox)) {
        this->currentContainerChanged = true;
    }
    ImGui::PopItemWidth();
    if (this->wineVersionHelp) {
        ImGui::SameLine();
        this->toolTip(wineVersionHelp);
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->containerMountFolderLabel);
    ImGui::SameLine(this->leftColumnWidth.x);

    float driveComboWidth = GlobalSettings::scaleFloatUI(50.0f);
    ImGui::PushItemWidth(driveComboWidth);
    //ImGui::PushItemWidth(-1-(this->wineVersionHelp?this->toolTipWidth:0));
    if (ImGui::Combo("##MountDriveCombo", &this->mountDriveComboboxData.currentSelectedIndex, this->mountDriveComboboxData.dataForCombobox)) {
        this->currentContainerChanged = true;
        this->currentContainerMountChanged = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushItemWidth(-this->browseButtonWidth - (this->containerMountFolderHelp ? this->toolTipWidth : 0));
    if (ImGui::InputText("##MountLocationID", this->mountLocation, sizeof(this->mountLocation))) {
        this->currentContainerMountChanged = true;
        this->currentContainerChanged = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(this->browseButtonText)) {
        const char* result = tfd::selectFolderDialog(getTranslation(GENERIC_OPEN_FOLDER_TITLE), this->containerMountFolderHelp);
        if (result) {
            strcpy(this->mountLocation, result);
            this->currentContainerMountChanged = true;
            this->currentContainerChanged = true;
        }
    }    
    if (this->containerMountFolderHelp) {
        ImGui::SameLine();
        this->toolTip(this->containerMountFolderHelp);
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->containerLocationLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
    ImGui::PushItemWidth(Platform::supportsOpenFileLocation() ? -this->containerLocationOpenLabelButtonWidth : -1);
    ImGui::InputText("##ContainerLocation", this->containerLocation, sizeof(this->containerLocation), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
    if (Platform::supportsOpenFileLocation()) {
        ImGui::SameLine();
        if (ImGui::Button(this->containerLocationOpenLabel)) {
            Platform::openFileLocation(container->getDir());
        }
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->containerLocationSizeLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    SAFE_IMGUI_TEXT(container->getSize().c_str());
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));    

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->addAppLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    if (ImGui::Button(this->addAppButtonLabel)) {
        runOnMainUI([container, this] {
            new AppChooserDlg(container, [this, container](BoxedApp* app) {
                this->setCurrentApp(app);
                rebuildShortcutsCombobox();
                this->shortcutsComboboxData.currentSelectedIndex = vectorIndexOf(container->getApps(), app);
                if (this->shortcutsComboboxData.currentSelectedIndex < 0) {
                    this->shortcutsComboboxData.currentSelectedIndex = 0;
                }
                });
            return false;
            });
    }
    if (this->addAppHelp) {
        ImGui::SameLine();
        this->toolTip(this->addAppHelp);
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::Separator();

    if (this->currentApp) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));        

        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->shortcutListLabel);
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::PushItemWidth(-1 - (this->shortcutHelp ? this->toolTipWidth : 0) - this->deleteShortcutButtonWidth);
        if (ImGui::Combo("##ShortcutsCombo", &this->shortcutsComboboxData.currentSelectedIndex, this->shortcutsComboboxData.dataForCombobox)) {
            this->setCurrentApp(this->currentContainer->getApps()[this->shortcutsComboboxData.currentSelectedIndex]);
        }
        ImGui::PopItemWidth();        
        ImGui::SameLine();
        if (ImGui::Button(this->deleteShortcutLabel)) {
            std::string label = getTranslationWithFormat(CONTAINER_VIEW_DELETE_SHORTCUT_CONFIRMATION, true, this->currentApp->getName());
            runOnMainUI([label, container, this]() {
                new YesNoDlg(GENERIC_DLG_CONFIRM_TITLE, label, [container, this](bool yes) {
                    if (yes) {
                        runOnMainUI([this, container]() {
                            this->currentAppChanged = false;
                            this->currentApp->remove();                            
                            if (this->currentContainer->getApps().size()) {
                                this->setCurrentApp(this->currentContainer->getApps()[0]);
                            } else {
                                this->currentApp = NULL;
                            }
                            rebuildShortcutsCombobox();
                            GlobalSettings::reloadApps();
                            return false;
                            });
                    }
                    });
                return false;
                });
        }
        if (this->shortcutHelp) {
            ImGui::SameLine();
            this->toolTip(this->shortcutHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));        
        if (this->currentApp->getIconTexture()) {
            ImVec2 pos = ImGui::GetCursorPos();
            ImGui::Image(this->currentApp->getIconTexture()->texture, ImVec2((float)UiSettings::ICON_SIZE, (float)UiSettings::ICON_SIZE));
            ImGui::SetCursorPos(pos);
        }
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->nameLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        ImGui::PushItemWidth((this->nameHelp ? -this->toolTipWidth : 0));
        if (ImGui::InputText("##appNameID", this->appName, sizeof(this->appName))) {
            this->currentAppChanged = true;
            this->currentApp->name = this->appName;
            rebuildShortcutsCombobox();
        }
        ImGui::PopItemWidth();
        if (this->nameHelp) {
            ImGui::SameLine();
            this->toolTip(this->nameHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->pathLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        ImGui::PushItemWidth(-1 - (this->pathHelp ? this->toolTipWidth : 0));
        ImGui::InputText("##Path", this->path, sizeof(this->path), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();
        ImGui::PopStyleColor();
        if (this->pathHelp) {
            ImGui::SameLine();
            toolTip(this->pathHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));        
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->resolutionLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        ImGui::PushItemWidth(-1 - (this->resolutionHelp ? this->toolTipWidth : 0));
        if (ImGui::Combo("##AppResolutionCombo", &this->resolutionComboboxData.currentSelectedIndex, this->resolutionComboboxData.dataForCombobox)) {
            this->currentAppChanged = true;
        }
        ImGui::PopItemWidth();
        if (this->resolutionHelp) {
            ImGui::SameLine();
            toolTip(this->resolutionHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));        
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->bppLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        ImGui::PushItemWidth(-1 - (this->bppHelp ? this->toolTipWidth : 0));
        if (ImGui::Combo("##AppBppCombo", &this->bppComboboxData.currentSelectedIndex, this->bppComboboxData.dataForCombobox)) {
            this->currentAppChanged = true;
        }
        ImGui::PopItemWidth();
        if (this->bppHelp) {
            ImGui::SameLine();
            toolTip(this->bppHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->fullscreenLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        if (ImGui::Checkbox("##FullscreenCheckbox", &this->fullScreen)) {
            this->currentAppChanged = true;
            this->scaleComboboxData.currentSelectedIndex = 0;
        }
        if (this->fullscreenHelp) {
            ImGui::SameLine();
            this->toolTip(fullscreenHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));        
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->scaleLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        ImGui::PushItemWidth(-1 - (this->scaleHelp ? this->toolTipWidth : 0));
        {
            UIDisableStyle disabled(this->fullScreen);
            if (ImGui::Combo("##AppScaleCombo", &this->scaleComboboxData.currentSelectedIndex, this->scaleComboboxData.dataForCombobox)) {
                this->currentAppChanged = true;
            }
        }
        ImGui::PopItemWidth();
        if (this->scaleHelp) {
            ImGui::SameLine();
            toolTip(this->scaleHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));        
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->scaleQualityLabel);
        ImGui::SameLine(this->innerColumnWidth.x);
        ImGui::PushItemWidth(-1 - (this->scaleQualityHelp ? this->toolTipWidth : 0));        
        if (ImGui::Combo("##AppScaleQualityCombo", &this->scaleQualityComboboxData.currentSelectedIndex, this->scaleQualityComboboxData.dataForCombobox)) {
            this->currentAppChanged = true;
        }
        ImGui::PopItemWidth();
        if (this->scaleQualityHelp) {
            ImGui::SameLine();
            toolTip(this->scaleQualityHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        ImGui::BeginGroup();        
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->glExtenstionsLabel);
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        if (ImGui::Button(this->glExtenstionsSetButtonLabel)) {
            strcpy(this->glExt, "GL_EXT_multi_draw_arrays GL_ARB_vertex_program\nGL_ARB_fragment_program GL_ARB_multitexture\nGL_EXT_secondary_color GL_EXT_texture_lod_bias\nGL_NV_texture_env_combine4 GL_ATI_texture_env_combine3\nGL_EXT_texture_filter_anisotropic GL_ARB_texture_env_combine\nGL_EXT_texture_env_combine GL_EXT_texture_compression_s3tc\nGL_ARB_texture_compression GL_EXT_paletted_texture");
            this->currentAppChanged = true;
        }
        ImGui::EndGroup();
        ImGui::SameLine(this->innerColumnWidth.x);
        if (ImGui::InputTextMultiline("##appGlExtID", this->glExt, sizeof(this->glExt), ImVec2((this->glExtensionsHelp ? -this->toolTipWidth : 0), ImGui::GetTextLineHeight() * 4))) {
            this->currentAppChanged = true;
        }
        ImGui::SameLine();
        if (this->glExtensionsHelp) {
            ImGui::SameLine();
            this->toolTip(this->glExtensionsHelp);
        }
        ImGui::Separator();
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    if (ImGui::Button(this->containerDeleteButtonLabel)) {
        std::string label;
        if (!container->getApps().size()) {
            label = getTranslationWithFormat(CONTAINER_VIEW_DELETE_CONFIRMATION, true, container->getName());
        } else {
            label = "";

            for (auto& app : container->getApps()) {
                if (label.length() != 0) {
                    label += ", ";
                }
                label += app->getName();
            }
            label = getTranslationWithFormat(CONTAINER_VIEW_DELETE_CONFIRMATION_WITH_APPS, true, container->getName(), label);
        }
        runOnMainUI([label, container, this]() {
            new YesNoDlg(GENERIC_DLG_CONFIRM_TITLE, label, [container, this](bool yes) {
                if (yes) {
                    runOnMainUI([this, container]() {
                        this->currentContainer = NULL;
                        this->currentApp = NULL;
                        this->currentContainerChanged = false;
                        container->deleteContainerFromFilesystem();
                        BoxedwineData::reloadContainers();
                        gotoView(VIEW_CONTAINERS);
                        return false;
                        });
                }
                });
            return false;
            });
    }
    if (this->containerDeleteButtonHelp) {
        ImGui::SameLine();
        this->toolTip(this->containerDeleteButtonHelp);
    }
}