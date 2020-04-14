#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

#define INSTALL_TYPE_SETUP 0
#define INSTALL_TYPE_DIR 1
#define INSTALL_TYPE_MOUNT 2
#define INSTALL_TYPE_BLANK 3

InstallView::InstallView(const std::string& initialFileOrDirPath, const char* startingTab) : BaseView("InstallView") {
    if (startingTab) {
        if (!strcmp(startingTab, "Demo")) {
            this->tabIndex = 1;
        }
    }    

    this->lastInstallType = INSTALL_TYPE_SETUP;
    this->installLabelText = getTranslation(INSTALLVIEW_INSTALL_TYPE_LABEL);
    this->containerLabelText = getTranslation(INSTALLVIEW_CONTAINER_LABEL);
    this->locationLabelText = getTranslation(INSTALLVIEW_SETUP_FILE_LOCATION_LABEL);
    this->browseButtonText = getTranslation(GENERIC_BROWSE_BUTTON);
    this->installTypeHelp = getTranslation(INSTALLVIEW_INSTALL_TYPE_HELP, false);
    this->containerHelp = getTranslation(INSTALLVIEW_CONTAINER_HELP, false);
    this->containerNameHelp = getTranslation(INSTALLVIEW_CONTAINER_NAME_HELP, false);
    this->wineVersionHelp = getTranslation(COMMON_WINE_VERSION_HELP, false);
    this->wineConfigHelp = getTranslation(INSTALLVIEW_RUN_WINE_CONFIG_HELP, false);

    this->leftColumnWidth = ImGui::CalcTextSize(this->installLabelText);
    this->rightColumnWidth = ImGui::CalcTextSize(this->browseButtonText);
    this->rightColumnWidth.x += ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;

    if (this->locationLabelText && this->locationLabelText[0]) {
        ImVec2 locationLabelWidth = ImGui::CalcTextSize(this->locationLabelText);
        if (locationLabelWidth.x > this->leftColumnWidth.x) {
            this->leftColumnWidth = locationLabelWidth;
        }
    }
    locationLabelText = getTranslation(INSTALLVIEW_DIRECTORY_LABEL);
    if (this->locationLabelText && this->locationLabelText[0]) {
        ImVec2 locationLabelWidth = ImGui::CalcTextSize(this->locationLabelText);
        if (locationLabelWidth.x > this->leftColumnWidth.x) {
            this->leftColumnWidth = locationLabelWidth;
        }
    }

    ImVec2 containerLabelWidth = ImGui::CalcTextSize(containerLabelText);
    if (containerLabelWidth.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = containerLabelWidth;
    }

    this->leftColumnWidth.x += ImGui::GetStyle().ItemSpacing.x;

    this->containerName[0] = 0;
    this->locationBuffer[0] = 0;

    this->installTypeComboboxData.data.push_back(getTranslation(INSTALLVIEW_TYPE_SETUP));
    this->installTypeComboboxData.data.push_back(getTranslation(INSTALLVIEW_TYPE_DIRECTORY));
    this->installTypeComboboxData.data.push_back(getTranslation(INSTALLVIEW_TYPE_MOUNT));
    this->installTypeComboboxData.data.push_back(getTranslation(INSTALLVIEW_TYPE_BLANK));
    this->installTypeComboboxData.dataChanged();

    this->containerComboboxData.data.push_back("Create New Container (Recommended)");
    for (auto& container : BoxedwineData::getContainers()) {
        this->containerComboboxData.data.push_back(container->getName());
    }
    this->containerComboboxData.dataChanged();
    this->containerComboboxData.currentSelectedIndex = 0;

    for (auto& ver : GlobalSettings::getWineVersions()) {
        this->wineVersionComboboxData.data.push_back(ver.name);
    }
    this->wineVersionComboboxData.dataChanged();
    this->wineVersionComboboxData.currentSelectedIndex = 0;

    this->runWineConfig = false;    

    if (initialFileOrDirPath.length()) {
        if (Fs::doesNativePathExist(initialFileOrDirPath) && Fs::isNativePathDirectory(initialFileOrDirPath)) {
            this->installTypeComboboxData.currentSelectedIndex = INSTALL_TYPE_DIR;
            this->lastInstallType = INSTALL_TYPE_DIR;
        } else {
            this->installTypeComboboxData.currentSelectedIndex = INSTALL_TYPE_SETUP;
            this->lastInstallType = INSTALL_TYPE_SETUP;
        }
        strncpy(this->locationBuffer, initialFileOrDirPath.c_str(), sizeof(this->locationBuffer));
        this->locationBuffer[sizeof(this->locationBuffer) - 1] = 0;
    }

    this->installTitle = getTranslation(INSTALLVIEW_INSTALL_TITLE);
    this->demoTitle = getTranslation(INSTALLVIEW_DEMO_TITLE);

    addTab(this->installTitle, [this]() {
        this->runInstallView();
        });
    addTab(this->demoTitle, [this]() {
        this->runDemoView();
        });
}

void InstallView::runInstallView() {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->installTitle);
    ImGui::PopFont();

    if (installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
        locationLabelText = getTranslation(INSTALLVIEW_SETUP_FILE_LOCATION_LABEL);
    } else if (installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_DIR || installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_MOUNT) {
        locationLabelText = getTranslation(INSTALLVIEW_DIRECTORY_LABEL);
    }

    if (this->lastInstallType != this->installTypeComboboxData.currentSelectedIndex) {
        if (this->lastInstallType == INSTALL_TYPE_SETUP || this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP || this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_BLANK) {
            this->locationBuffer[0] = 0;
        }
        if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_BLANK) {
            this->containerComboboxData.currentSelectedIndex = 0;
        }
        this->lastInstallType = this->installTypeComboboxData.currentSelectedIndex;        
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->installLabelText);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1 - (this->installTypeHelp ? this->toolTipWidth : 0));
    ImGui::Combo("##InstallTypeCombo", &this->installTypeComboboxData.currentSelectedIndex, this->installTypeComboboxData.dataForCombobox);
    ImGui::PopItemWidth();
    if (this->installTypeHelp) {
        ImGui::SameLine();
        toolTip(this->installTypeHelp);
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    if (this->lastInstallType != INSTALL_TYPE_BLANK && this->locationLabelText && this->locationLabelText[0]) {
        const char* locationHelp = NULL;
        int locationHelpId = 0;

        if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
            locationHelpId = INSTALLVIEW_TYPE_SETUP_HELP;
        } else if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_DIR) {
            locationHelpId = INSTALLVIEW_TYPE_DIR_HELP;
        } else if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_MOUNT) {
            locationHelpId = INSTALLVIEW_TYPE_MOUNT_HELP;
        }
        if (locationHelpId) {
            locationHelp = getTranslation(locationHelpId);
            if (locationHelp && locationHelp[0] == 0) {
                locationHelp = NULL;
            }
        }
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(this->locationLabelText);
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::PushItemWidth(-this->rightColumnWidth.x - (locationHelp ? this->toolTipWidth : 0));
        ImGui::InputText("##LocationID", this->locationBuffer, sizeof(this->locationBuffer));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button(this->browseButtonText)) {
            if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
                const char* types[] = { "*.exe" };
                const char* result = tfd::openFileDialog(getTranslation(INSTALLVIEW_OPEN_SETUP_FILE_TITLE), this->locationBuffer, 1, types, NULL, 0);
                if (result) {
                    strcpy(this->locationBuffer, result);
                }
            } else {
                const char* result = tfd::selectFolderDialog(getTranslation(GENERIC_OPEN_FOLDER_TITLE), this->locationBuffer);
                if (result) {
                    strcpy(this->locationBuffer, result);
                }
            }
        }
        if (locationHelp) {
            ImGui::SameLine();
            this->toolTip(getTranslation(locationHelpId));
        }
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    }

    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->containerLabelText);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1 - (this->containerHelp ? this->toolTipWidth : 0));
    
    if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_BLANK) {
        UIDisableStyle disabled;
        ImGui::Combo("##ContainerCombo", &this->containerComboboxData.currentSelectedIndex, this->containerComboboxData.dataForCombobox);
    } else {
        ImGui::Combo("##ContainerCombo", &this->containerComboboxData.currentSelectedIndex, this->containerComboboxData.dataForCombobox);
    }
    ImGui::SameLine();
    ImGui::PopItemWidth();
    if (this->containerHelp) {
        ImGui::SameLine();
        this->toolTip(this->containerHelp);
    }
    if (this->containerComboboxData.currentSelectedIndex == 0) { // create new container
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(getTranslation(INSTALLVIEW_CONTAINER_NAME_LABEL));
        ImGui::SameLine();
        ImGui::PushItemWidth(-1 - (this->containerNameHelp ? this->toolTipWidth : 0));
        ImGui::InputText("##ContainerName", this->containerName, sizeof(this->containerName));
        if (containerNameHelp) {
            ImGui::SameLine();
            this->toolTip(containerNameHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(this->leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(getTranslation(COMMON_WINE_VERSION_LABEL));
        ImGui::SameLine();
        ImGui::PushItemWidth(-1 - (this->wineVersionHelp ? this->toolTipWidth : 0));
        ImGui::Combo("##WineCombo", &this->wineVersionComboboxData.currentSelectedIndex, this->wineVersionComboboxData.dataForCombobox);
        ImGui::PopItemWidth();
        if (this->wineVersionHelp) {
            ImGui::SameLine();
            this->toolTip(wineVersionHelp);
        }

        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        SAFE_IMGUI_TEXT("");
        ImGui::SameLine(leftColumnWidth.x);
        ImGui::AlignTextToFramePadding();
        SAFE_IMGUI_TEXT(getTranslation(INSTALLVIEW_CONTAINER_RUN_WINE_CONFIG_LABEL));
        ImGui::SameLine();
        ImGui::Checkbox("##WineConfigCheckbox", &runWineConfig);
        if (wineConfigHelp) {
            ImGui::SameLine();
            this->toolTip(wineConfigHelp);
        }
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing*3));
    ImGui::PushFont(GlobalSettings::largeFont);
    if (ImGui::Button(getTranslation(INSTALLVIEW_INSTALL_BUTTON_LABEL))) {
        onInstall();
    }
    ImGui::PopFont();
}

void InstallView::runDemoView() {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->demoTitle);
    ImGui::PopFont();
}

bool InstallView::saveChanges() {
    return true;
}

void InstallView::onInstall() {
    if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
        if (strlen(this->locationBuffer) == 0) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_SETUP_FILE_MISSING);
        } else if (!Fs::doesNativePathExist(this->locationBuffer)) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_SETUP_FILE_NOT_FOUND);
        }
    } else if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_DIR || this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_MOUNT) {
        Fs::trimTrailingSlash(this->locationBuffer);
        if (strlen(this->locationBuffer) == 0) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_DIR_MISSING);
        } else if (!Fs::doesNativePathExist(this->locationBuffer) || !Fs::isNativePathDirectory(this->locationBuffer)) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_DIR_NOT_FOUND);
        }
    }

    BoxedContainer* container = NULL;

    if (!this->errorMsg) {
        if (this->containerComboboxData.currentSelectedIndex != 0) {
            container = BoxedwineData::getContainers()[this->containerComboboxData.currentSelectedIndex - 1];
        } else {
            if (this->containerName[0] == 0) {
                this->errorMsg = getTranslation(INSTALLVIEW_ERROR_CONTAINER_NAME_MISSING);
            } else {
                std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + this->containerName;
                if (Fs::doesNativePathExist(containerFilePath)) {
                    if (!Fs::isNativeDirectoryEmpty(containerFilePath)) {
                        this->errorMsgString = getTranslationWithFormat(INSTALLVIEW_ERROR_CONTAINER_ALREADY_EXISTS, true, containerFilePath.c_str());
                        this->errorMsg = this->errorMsgString.c_str();
                    }
                } else if (!Fs::makeNativeDirs(containerFilePath)) {
                    this->errorMsgString = getTranslationWithFormat(INSTALLVIEW_ERROR_FAILED_TO_CREATE_CONTAINER_DIR, true, strerror(errno));
                    this->errorMsg = this->errorMsgString.c_str();
                }
            }
        }
    }
    if (!this->errorMsg) {
        GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
        bool containerCreated = false;
        if (!container) {
            std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + this->containerName;
            container = BoxedContainer::createContainer(containerFilePath, this->containerName, GlobalSettings::getWineVersions()[wineVersionComboboxData.currentSelectedIndex].name);
            containerCreated = true;
        }
        container->launch(); // fill out startUpArgs specific to a container        
        if (this->runWineConfig) {
            GlobalSettings::startUpArgs.setRunWineConfigFirst(true);
            GlobalSettings::startUpArgs.readyToLaunch = true;
        }

        if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_SETUP) {
            GlobalSettings::startUpArgs.addArg(locationBuffer);
            GlobalSettings::startUpArgs.readyToLaunch = true;
            GlobalSettings::startUpArgs.showAppPickerForContainer = container->getName();
        } else if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_DIR) {
            std::filesystem::path dest(GlobalSettings::getRootFolder(container));
            dest = dest / "home" / "username" / ".wine" / "drive_c" / Fs::getFileNameFromNativePath(this->locationBuffer);

            if (!std::filesystem::exists(dest)) {
                std::error_code ec;
                if (!std::filesystem::create_directories(dest, ec)) {
                    this->errorMsgString = getTranslationWithFormat(INSTALLVIEW_ERROR_FILESYSTEM_FAIL_TO_CREATE_DIRS, true, ec.message());
                    this->errorMsg = this->errorMsgString.c_str();
                }
            }
            if (!this->errorMsg) {
                std::error_code ec;
                std::filesystem::copy(this->locationBuffer, dest, std::filesystem::copy_options::recursive, ec);
                if (ec) {
                    this->errorMsgString = getTranslationWithFormat(INSTALLVIEW_ERROR_FILESYSTEM_COPY_DIRECTORY, true, ec.message());
                    this->errorMsg = this->errorMsgString.c_str();
                }
            }
            if (!this->errorMsg) {
                if (GlobalSettings::startUpArgs.readyToLaunch) {
                    GlobalSettings::startUpArgs.showAppPickerForContainer = container->getName();
                } else {
                    runOnMainUI([container]() {
                        new AppChooserDlg(container);
                        return false;
                        });
                }
            }
        } else if (this->installTypeComboboxData.currentSelectedIndex == INSTALL_TYPE_MOUNT) {
            if (!container->addNewMount(MountInfo("t", this->locationBuffer, true))) {
                this->errorMsg = getTranslation(INSTALLVIEW_ERROR_FAILED_TO_MOUNT);
            } else {
                container->saveContainer();
                if (GlobalSettings::startUpArgs.readyToLaunch) {
                    GlobalSettings::startUpArgs.showAppPickerForContainer = container->getName();
                } else {
                    runOnMainUI([container]() {
                        new AppChooserDlg(container);
                        return false;
                        });
                }
            }
        }
        if (containerCreated) {
            if (this->errorMsg) {
                container->deleteContainerFromFilesystem();
                delete container;
            } else {
                BoxedwineData::addContainer(container);
            }
        }
        if (!this->errorMsg && GlobalSettings::startUpArgs.readyToLaunch) {
            static std::string name = Fs::getFileNameFromNativePath(locationBuffer);
            runOnMainUI([]() {
                new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, name.c_str()));
                return false;
                });
        }
    }
}