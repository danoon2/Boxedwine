#include "boxedwine.h"
#include "../boxedwineui.h"

#define INSTALL_TYPE_SETUP 0
#define INSTALL_TYPE_DIR 1
#define INSTALL_TYPE_MOUNT 2
#define INSTALL_TYPE_BLANK 3

InstallView::InstallView(const std::string& initialFileOrDirPath, std::string tab) : BaseView("InstallView") {
    createInstallTab(initialFileOrDirPath);
    createDemoTab();
    if (tab.length()) {
        if (tab == "Demo") {
            this->tabIndex = 1;
        }
    }            
}

void InstallView::createDemoTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(INSTALLVIEW_DEMO_TITLE);

    addTab(getTranslation(INSTALLVIEW_DEMO_TITLE), model, [this](bool buttonPressed, BaseViewTab& tab) {
        
        });
}

void InstallView::createInstallTab(const std::string& initialFileOrDirPath) {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(INSTALLVIEW_INSTALL_TITLE);

    // Initialize Install Type Control
    std::vector<ComboboxItem> installTypes;
    installTypes.push_back(ComboboxItem(getTranslation(INSTALLVIEW_TYPE_SETUP)));
    installTypes.push_back(ComboboxItem(getTranslation(INSTALLVIEW_TYPE_DIRECTORY)));
    installTypes.push_back(ComboboxItem(getTranslation(INSTALLVIEW_TYPE_MOUNT)));
    installTypes.push_back(ComboboxItem(getTranslation(INSTALLVIEW_TYPE_BLANK)));
    installTypeControl = section->addComboboxRow(INSTALLVIEW_INSTALL_TYPE_LABEL, INSTALLVIEW_INSTALL_TYPE_HELP, installTypes, 0);
    installTypeControl->onChange = [this]() {
        int type = installTypeControl->getSelection();
        locationControl->setRowHidden(type == INSTALL_TYPE_BLANK);
        containerControl->setReadOnly(type == INSTALL_TYPE_BLANK);
        if (type == INSTALL_TYPE_SETUP) {
            std::vector<std::string> types;
            types.push_back("*.exe");
#ifndef BOXEDWINE_MSVC
            types.push_back("*.EXE");
#endif
            locationControl->setBrowseFileButton(types);
            locationControl->setHelpId(INSTALLVIEW_TYPE_SETUP_HELP);
        } else if (type == INSTALL_TYPE_DIR) {
            locationControl->setBrowseDirButton();
            locationControl->setHelpId(INSTALLVIEW_TYPE_DIR_HELP);
        } else if (type == INSTALL_TYPE_MOUNT) {
            locationControl->setBrowseDirButton();
            locationControl->setHelpId(INSTALLVIEW_TYPE_MOUNT_HELP);
        } else {
            containerControl->setSelection(0);
        }
    };

    // Initialize File/Dir Location Control
    locationControl = section->addTextInputRow(INSTALLVIEW_SETUP_FILE_LOCATION_LABEL, INSTALLVIEW_TYPE_SETUP_HELP);

    // Initialize Container Control
    std::vector<ComboboxItem> containers;
    containers.push_back(ComboboxItem(getTranslation(INSTALLVIEW_NEW_CONTAINER)));
    for (auto& container : BoxedwineData::getContainers()) {
        containers.push_back(ComboboxItem(container->getName()));
    }
    containerControl = section->addComboboxRow(INSTALLVIEW_CONTAINER_LABEL, INSTALLVIEW_CONTAINER_HELP, containers, 0);    

    // Sub Section For New Container
    containerSection = model->addSection();
    containerSection->setIndent(true);
    containerControl->onChange = [this]() {
        containerSection->setHidden(containerControl->getSelection() != 0);
    };

    // Initialize Container Name Control
    containerNameControl = containerSection->addTextInputRow(INSTALLVIEW_CONTAINER_NAME_LABEL, INSTALLVIEW_CONTAINER_NAME_HELP);

    // Initialize Wine Version Control
    std::vector<ComboboxItem> wineVersions;
    for (auto& ver : GlobalSettings::getWineVersions()) {
        wineVersions.push_back(ComboboxItem(ver.name));
    }
    wineVersionControl = containerSection->addComboboxRow(COMMON_WINE_VERSION_LABEL, COMMON_WINE_VERSION_HELP, wineVersions, 0);
    wineVersionControl->setWidth(GlobalSettings::scaleIntUI(150));

    // Initialize Windows Version Control
    std::vector<ComboboxItem> windowsVersion;
    for (auto& win : BoxedwineData::getWinVersions()) {
        windowsVersion.push_back(ComboboxItem(win.szDescription));
    }
    windowsVersionControl = containerSection->addComboboxRow(CONTAINER_VIEW_WINDOWS_VERION_LABEL, CONTAINER_VIEW_WINDOWS_VERION_HELP, windowsVersion, BoxedwineData::getDefaultWindowsVersionIndex());    
    windowsVersionControl->setWidth(GlobalSettings::scaleIntUI(150));

    section = model->addSection();
    section->addSeparator();
    std::shared_ptr<LayoutButtonControl> buttonControl = section->addButton(0, 0, getTranslation(INSTALLVIEW_INSTALL_BUTTON_LABEL));
    buttonControl->setFont(GlobalSettings::largeFont);
    buttonControl->onChange = [this]() {
        onInstall();
    };

    if (initialFileOrDirPath.length()) {
        if (Fs::doesNativePathExist(initialFileOrDirPath) && Fs::isNativePathDirectory(initialFileOrDirPath)) {
            installTypeControl->setSelection(INSTALL_TYPE_DIR);
        } else {
            installTypeControl->setSelection(INSTALL_TYPE_SETUP);
        }
        locationControl->setText(initialFileOrDirPath);
    }
    installTypeControl->onChange();

    addTab(getTranslation(INSTALLVIEW_INSTALL_TITLE), model, [this](bool buttonPressed, BaseViewTab& tab) {
        
        });
}

bool InstallView::saveChanges() {
    return true;
}

void InstallView::onInstall() {
    std::string location = locationControl->getText();
    int containerIndex = containerControl->getSelection();
    std::string containerName = containerNameControl->getText();
    int wineVersionIndex = wineVersionControl->getSelection();
    int windowsVersionIndex = windowsVersionControl->getSelection();
    int installType = installTypeControl->getSelection();

    if (installType == INSTALL_TYPE_SETUP) {
        if (!location.length()) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_SETUP_FILE_MISSING);
        } else if (!Fs::doesNativePathExist(location)) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_SETUP_FILE_NOT_FOUND);
        }
    } else if (installType == INSTALL_TYPE_DIR || installType == INSTALL_TYPE_MOUNT) {
        Fs::trimTrailingSlash(location);
        if (!location.length()) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_DIR_MISSING);
        } else if (!Fs::doesNativePathExist(location) || !Fs::isNativePathDirectory(location)) {
            this->errorMsg = getTranslation(INSTALLVIEW_ERROR_DIR_NOT_FOUND);
        }
    }

    BoxedContainer* container = NULL;

    if (!this->errorMsg) {
        if (containerIndex != 0) {
            container = BoxedwineData::getContainers()[containerIndex - 1];
        } else {
            if (containerName.length() == 0) {
                this->errorMsg = getTranslation(INSTALLVIEW_ERROR_CONTAINER_NAME_MISSING);
            } else {
                std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + containerName;
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
        GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
        GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
        bool containerCreated = false;
        if (!container) {
            std::string containerFilePath = GlobalSettings::getContainerFolder() + Fs::nativePathSeperator + containerName;
            container = BoxedContainer::createContainer(containerFilePath, containerName, GlobalSettings::getWineVersions()[wineVersionIndex].name);
            container->setWindowsVersion(BoxedwineData::getWinVersions()[windowsVersionIndex]);
            containerCreated = true;
        }
        container->launch(); // fill out startUpArgs specific to a container        

        if (installType == INSTALL_TYPE_SETUP) {
            GlobalSettings::startUpArgs.addArg(location);
            GlobalSettings::startUpArgs.readyToLaunch = true;
            GlobalSettings::startUpArgs.showAppPickerForContainer = container->getName();
        } else if (installType == INSTALL_TYPE_DIR) {
            std::filesystem::path dest(GlobalSettings::getRootFolder(container));
            dest = dest / "home" / "username" / ".wine" / "drive_c" / Fs::getFileNameFromNativePath(location);

            if (!std::filesystem::exists(dest)) {
                std::error_code ec;
                if (!std::filesystem::create_directories(dest, ec)) {
                    this->errorMsgString = getTranslationWithFormat(INSTALLVIEW_ERROR_FILESYSTEM_FAIL_TO_CREATE_DIRS, true, ec.message());
                    this->errorMsg = this->errorMsgString.c_str();
                }
            }
            if (!this->errorMsg) {
                std::error_code ec;
                std::filesystem::copy(location, dest, std::filesystem::copy_options::recursive, ec);
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
                        new AppChooserDlg(container, nullptr);
                        return false;
                        });
                }
            }
        } else if (installType == INSTALL_TYPE_MOUNT) {
            if (!container->addNewMount(MountInfo("t", location, true))) {
                this->errorMsg = getTranslation(INSTALLVIEW_ERROR_FAILED_TO_MOUNT);
            } else {
                container->saveContainer();
                if (GlobalSettings::startUpArgs.readyToLaunch) {
                    GlobalSettings::startUpArgs.showAppPickerForContainer = container->getName();
                } else {
                    runOnMainUI([container]() {
                        new AppChooserDlg(container, nullptr);
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
            static std::string name;
            name = Fs::getFileNameFromNativePath(location);
            runOnMainUI([]() {
                new WaitDlg(WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(WAITDLG_LAUNCH_APP_LABEL, true, name.c_str()));
                return false;
                });
        }
    }
}