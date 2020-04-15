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

    this->leftColumnWidth = ImGui::CalcTextSize(this->wineVersionLabel);
    ImVec2 width = ImGui::CalcTextSize(this->addAppLabel);
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
}

bool ContainersView::saveChanges() {
    if (this->currentContainer) {
        if (strlen(this->containerName) == 0) {
            this->errorMsg = getTranslation(CONTAINER_VIEW_ERROR_BLANK_NAME);
        }
    }
    if (this->mountDriveComboboxData.currentSelectedIndex != 0 && strlen(this->mountLocation)==0) {
        this->errorMsg = getTranslation(CONTAINER_VIEW_ERROR_MISSING_MOUNT_LOCATION);
    } else if (this->mountDriveComboboxData.currentSelectedIndex == 0 && strlen(this->mountLocation) != 0) {
        this->errorMsg = getTranslation(CONTAINER_VIEW_ERROR_MISSING_MOUNT_DRIVE);
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
    }
    return this->errorMsg == NULL;
}

void ContainersView::runContainerView(BoxedContainer* container, bool buttonPressed, BaseViewTab& tab) {
    this->currentContainer = container;

    if (buttonPressed) {
        this->wineVersionComboboxData.currentSelectedIndex = stringIndexInVector(container->getWineVersion(), this->wineVersionComboboxData.data, 0);
        strncpy(this->containerName, container->getName().c_str(), sizeof(this->containerName));
        this->containerName[sizeof(this->containerName) - 1] = 0;

        strncpy(this->containerLocation, container->getDir().c_str(), sizeof(this->containerLocation));
        this->containerLocation[sizeof(this->containerLocation) - 1] = 0;

        container->updateCachedSize();

        if (!container->getMounts().size() || !container->getMounts()[0].wine || container->getMounts()[0].localPath.length()!=1) {
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
        new AppChooserDlg(container);
    }
    if (this->addAppHelp) {
        ImGui::SameLine();
        this->toolTip(this->addAppHelp);
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing*4));
    ImGui::Separator();
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
                    this->currentContainer = NULL;
                    this->currentContainerChanged = false;
                    container->deleteContainerFromFilesystem();
                    BoxedwineData::reloadContainers();
                    gotoView(VIEW_CONTAINERS);
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