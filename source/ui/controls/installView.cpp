#include "boxedwine.h"
#include "../boxedwineui.h"

#define INSTALL_TYPE_SETUP 0
#define INSTALL_TYPE_DIR 1
#define INSTALL_TYPE_MOUNT 2
#define INSTALL_TYPE_BLANK 3

InstallView::InstallView(BString initialFileOrDirPath, BString tab) : BaseView(B("InstallView")) {
    createInstallTab(initialFileOrDirPath);
    if (GlobalSettings::getDemos().size()) {
        createDemoTab();
    }
    if (tab.length()) {
        if (tab == "Demo") {
            this->tabIndex = 1;
        }
    }            
}

void InstallView::createDemoTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::NONE);

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += INSTALL_DEMO_ICON;
        name += " ";
    }
    name += getTranslation(Msg::INSTALLVIEW_DEMO_TITLE);

    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {
        runApps(GlobalSettings::getDemos());
        });
}

void InstallView::runApps(std::vector<AppFile>& apps) {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(c_getTranslation(Msg::INSTALLVIEW_DEMO_TITLE));
    ImGui::PopFont();

    ImVec2 size = ImGui::GetWindowContentRegionMax();
    size.y -= ImGui::GetCursorPosY() + GlobalSettings::scaleFloatUI(8.0f);

    ImGui::PushFont(GlobalSettings::mediumFont);
    //ImGui::BeginChildFrame(401, size);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    for (auto& app : apps) {
        ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));
        ImGui::SameLine();
        ImVec2 pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
        
        BString buttonLabel = B("Install");
        ImGui::PushID(&app);            
        if (ImGui::Button(buttonLabel.c_str())) {
            if (Fs::doesNativePathExist(app.localFilePath)) {
                app.install();
            } else {
                GlobalSettings::downloadFile(app.filePath, app.localFilePath, app.name, app.size, [&app](bool sucess) {
                    app.install();
                    });
            }
        }
        ImGui::PopID();
        ImGui::SameLine();
        
        //ImGui::SameLine(pos.x + this->wineButtonTotalColumnWidth);
        BString name = app.name;
        BString name2;
        if (!Fs::doesNativePathExist(app.localFilePath)) {
            name2 += "(";
            name2 += getTranslation(Msg::INSTALLVIEW_DEMO_DOWNLOAD_SIZE);
            name2 += app.size;
            name2 += " MB)";
        }
        if (app.iconTexture) {
            float pad = ImGui::GetStyle().FramePadding.y;
            ImGui::Image(app.iconTexture->getTexture(), ImVec2(ImGui::GetTextLineHeight()+pad*2, ImGui::GetTextLineHeight()+pad*2));
            ImGui::SameLine();
        }
        SAFE_IMGUI_TEXT(name.c_str());
        ImGui::SameLine();
        ImGui::PushFont(GlobalSettings::defaultFont);
        pos = ImGui::GetCursorPos();
        float y = pos.y;
        pos.y += GlobalSettings::mediumFont->FontSize / 2 - GlobalSettings::defaultFont->FontSize / 2;
        ImGui::SetCursorPos(pos);
        SAFE_IMGUI_TEXT(name2.c_str());
        ImGui::PopFont();        
        if (app.help.length()) {
            pos = ImGui::GetCursorPos();
            pos.y = y;
            ImGui::SetCursorPos(pos);
            ImGui::SameLine();
            this->drawToolTip(app.help);
        }
    }
    ImGui::PopFont();
    //ImGui::EndChildFrame();
}

void InstallView::createInstallTab(BString initialFileOrDirPath) {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::INSTALLVIEW_INSTALL_TITLE);

    // Initialize Install Type Control
    std::vector<ComboboxItem> installTypes;
    installTypes.push_back(ComboboxItem(getTranslation(Msg::INSTALLVIEW_TYPE_SETUP)));
    installTypes.push_back(ComboboxItem(getTranslation(Msg::INSTALLVIEW_TYPE_DIRECTORY)));
    installTypes.push_back(ComboboxItem(getTranslation(Msg::INSTALLVIEW_TYPE_MOUNT)));
    installTypes.push_back(ComboboxItem(getTranslation(Msg::INSTALLVIEW_TYPE_BLANK)));
    installTypeControl = section->addComboboxRow(Msg::INSTALLVIEW_INSTALL_TYPE_LABEL, Msg::INSTALLVIEW_INSTALL_TYPE_HELP, installTypes, 0);
    installTypeControl->onChange = [this]() {
        int type = installTypeControl->getSelection();
        locationControl->setRowHidden(type == INSTALL_TYPE_BLANK);
        containerControl->setReadOnly(type == INSTALL_TYPE_BLANK);
        if (type == INSTALL_TYPE_SETUP) {
            std::vector<BString> types;
            types.push_back(B("*.exe"));
#ifndef BOXEDWINE_MSVC
            types.push_back(B("*.EXE"));
#endif
            locationControl->setBrowseFileButton(types);
            locationControl->setHelpId(Msg::INSTALLVIEW_TYPE_SETUP_HELP);
        } else if (type == INSTALL_TYPE_DIR) {
            locationControl->setBrowseDirButton();
            locationControl->setHelpId(Msg::INSTALLVIEW_TYPE_DIR_HELP);
        } else if (type == INSTALL_TYPE_MOUNT) {
            locationControl->setBrowseDirButton();
            locationControl->setHelpId(Msg::INSTALLVIEW_TYPE_MOUNT_HELP);
        } else {
            containerControl->setSelection(0);
        }
    };

    // Initialize File/Dir Location Control
    locationControl = section->addTextInputRow(Msg::INSTALLVIEW_SETUP_FILE_LOCATION_LABEL, Msg::INSTALLVIEW_TYPE_SETUP_HELP);
    locationControl->onBrowseFinished = [this]() {
        if (containerNameControl->getText().length()==0) {
            setContainerName();
        }
    };

    // Initialize Container Control
    std::vector<ComboboxItem> containers;
    containers.push_back(ComboboxItem(getTranslation(Msg::INSTALLVIEW_NEW_CONTAINER)));
    for (auto& container : BoxedwineData::getContainers()) {
        containers.push_back(ComboboxItem(container->getName()));
    }
    containerControl = section->addComboboxRow(Msg::INSTALLVIEW_CONTAINER_LABEL, Msg::INSTALLVIEW_CONTAINER_HELP, containers, 0);

    // Sub Section For New Container
    containerSection = model->addSection();
    containerSection->setIndent(true);
    containerControl->onChange = [this]() {
        containerSection->setHidden(containerControl->getSelection() != 0);
    };

    // Initialize Container Name Control
    containerNameControl = containerSection->addTextInputRow(Msg::INSTALLVIEW_CONTAINER_NAME_LABEL, Msg::INSTALLVIEW_CONTAINER_NAME_HELP);

    // Initialize FileSystem Version Control
    fileSystemVersionControl = createFileSystemVersionCombobox(containerSection);
    fileSystemVersionControl->onChange = [this]() {
        setWindowsVersionDefault();        
    };
    // Initialize Windows Version Control
    windowsVersionControl = createWindowsVersionCombobox(containerSection);
    setWindowsVersionDefault();

    section = model->addSection();
    section->addSeparator();
    std::shared_ptr<LayoutButtonControl> buttonControl = section->addButton(Msg::NONE, Msg::NONE, getTranslation(Msg::INSTALLVIEW_INSTALL_BUTTON_LABEL));
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
        installTypeControl->onChange();
        locationControl->setText(initialFileOrDirPath);
        setContainerName();
    }
    installTypeControl->onChange();

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += INSTALL_ICON;
        name += " ";
    }
    name += getTranslation(Msg::INSTALLVIEW_INSTALL_TITLE);
    addTab(name, name, model, [this](bool buttonPressed, BaseViewTab& tab) {
        
        });
}

void InstallView::setWindowsVersionDefault() {
    BString ver = fileSystemVersionControl->getSelectionStringValue();
    std::shared_ptr<FileSystemZip> fileSystem = GlobalSettings::getAvailableFileSystemFromName(ver);

    if (fileSystem) {
        windowsVersionControl->setRowHidden(!fileSystem->hasWine());
    }
    if (ver.startsWith("Wine ")) {
        ver = ver.substr(5);
        std::vector<BString> parts;
        ver.split('.', parts);
        if (parts.size() > 1) {
            BString major = parts[0];
            BString minor = parts[1];
            if (major < "2" || (major == "2" && minor < "2")) {
                windowsVersionControl->setSelectionStringValue(B("Windows XP"));
            } else {
                windowsVersionControl->setSelectionStringValue(B("Windows 7"));
            }
        }
    }
}

bool InstallView::saveChanges() {
    return true;
}

void InstallView::setContainerName() {
    int installType = installTypeControl->getSelection();
    BString location = locationControl->getText();
    BString dirName;
    BString name;

    if (installType == INSTALL_TYPE_SETUP) {
        BString fileName = Fs::getFileNameFromNativePath(location);
        if (fileName.contains("setup", true)) {
            dirName = Fs::getNativeParentPath(location);
        } else {
            std::vector<BString> parts;
            fileName.split('.', parts);
            if (parts.size()) {
                name = parts[0];
            }
        }
    } else {
        dirName = location;
    }
    if (dirName.length()) {
        dirName = Fs::trimTrailingSlash(dirName);
        name = Fs::getFileNameFromNativePath(dirName);        
    }
    containerNameControl->setText(name);
}

void InstallView::onInstall() {
    BString location = locationControl->getText();
    int containerIndex = containerControl->getSelection();
    BString containerName = containerNameControl->getText();
    int fileSystemVersionIndex = fileSystemVersionControl->getSelection();
    int windowsVersionIndex = windowsVersionControl->getSelection();
    int installType = installTypeControl->getSelection();

    if (installType == INSTALL_TYPE_SETUP) {
        if (!location.length()) {
            this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_SETUP_FILE_MISSING);
        } else if (!Fs::doesNativePathExist(location)) {
            this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_SETUP_FILE_NOT_FOUND);
        } else if (Fs::isNativePathDirectory(location)) {
            this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_SETUP_FILE_NOT_FOUND);
        }
    } else if (installType == INSTALL_TYPE_DIR || installType == INSTALL_TYPE_MOUNT) {
        location = Fs::trimTrailingSlash(location);
        if (!location.length()) {
            this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_DIR_MISSING);
        } else if (!Fs::doesNativePathExist(location) || !Fs::isNativePathDirectory(location)) {
            this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_DIR_NOT_FOUND);
        }
    }

    BoxedContainer* container = nullptr;

    if (this->errorMsg.isEmpty()) {
        if (containerIndex != 0) {
            container = BoxedwineData::getContainers()[containerIndex - 1];
        } else if (containerName.length() == 0) {
            this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_CONTAINER_NAME_MISSING);
        }
    }
    if (this->errorMsg.isEmpty()) {
        GlobalSettings::startUpArgs = StartUpArgs(); // reset parameters
        GlobalSettings::startUpArgs.setScale(GlobalSettings::getDefaultScale());
        GlobalSettings::startUpArgs.setVsync(GlobalSettings::getDefaultVsync());
        GlobalSettings::startUpArgs.setResolution(GlobalSettings::getDefaultResolution());
        bool containerCreated = false;
        if (!container) {
            BString containerFilePath = GlobalSettings::createUniqueContainerPath(containerName);
            if (!Fs::makeNativeDirs(containerFilePath)) {
                this->errorMsgString = getTranslationWithFormat(Msg::INSTALLVIEW_ERROR_FAILED_TO_CREATE_CONTAINER_DIR, true, BString::copy(strerror(errno)));
                this->errorMsg = this->errorMsgString;
                return;
            }
            std::shared_ptr<FileSystemZip> fs = GlobalSettings::getFileSystemVersions()[fileSystemVersionIndex];
            container = BoxedContainer::createContainer(containerFilePath, containerName, fs);
            container->setWindowsVersion(BoxedwineData::getWinVersions()[windowsVersionIndex]);
            containerCreated = true;
        }
        container->launch(); // fill out startUpArgs specific to a container        

        if (installType == INSTALL_TYPE_SETUP) {
            GlobalSettings::startUpArgs.addArg(location);
            GlobalSettings::startUpArgs.readyToLaunch = true;
            GlobalSettings::startUpArgs.showAppPickerForContainerDir = container->getDir();
        } else if (installType == INSTALL_TYPE_DIR) {
            std::filesystem::path dest(GlobalSettings::getRootFolder(container).c_str());
            dest = dest / "home" / "username" / ".wine" / "drive_c" / Fs::getFileNameFromNativePath(location).c_str();

            if (!std::filesystem::exists(dest)) {
                std::error_code ec;
                if (!std::filesystem::create_directories(dest, ec)) {
                    this->errorMsgString = getTranslationWithFormat(Msg::INSTALLVIEW_ERROR_FILESYSTEM_FAIL_TO_CREATE_DIRS, true, BString::copy(ec.message().c_str()));
                    this->errorMsg = this->errorMsgString;
                }
            }
            if (this->errorMsg.isEmpty()) {
                std::error_code ec;
                std::filesystem::copy(location.c_str(), dest.c_str(), std::filesystem::copy_options::recursive, ec);
                if (ec) {
                    this->errorMsgString = getTranslationWithFormat(Msg::INSTALLVIEW_ERROR_FILESYSTEM_COPY_DIRECTORY, true, BString::copy(ec.message().c_str()));
                    this->errorMsg = this->errorMsgString;
                }
            }
            if (this->errorMsg.isEmpty()) {
                if (GlobalSettings::startUpArgs.readyToLaunch) {
                    GlobalSettings::startUpArgs.showAppPickerForContainerDir = container->getDir();
                } else {
                    runOnMainUI([container, dest]() {
                        std::vector<BoxedApp> items;
                        container->getNewApps(items, nullptr, BString::copy(dest.string().c_str()));
                        new AppChooserDlg(items, [container](BoxedApp app) {
                            gotoView(VIEW_CONTAINERS, container->getDir(), app.getIniFilePath());
                            });
                        return false;
                        });
                }
            }
        } else if (installType == INSTALL_TYPE_MOUNT) {
            if (!container->addNewMount(MountInfo(B("t"), location, true))) {
                this->errorMsg = getTranslation(Msg::INSTALLVIEW_ERROR_FAILED_TO_MOUNT);
            } else {
                container->saveContainer();
                if (GlobalSettings::startUpArgs.readyToLaunch) {
                    GlobalSettings::startUpArgs.showAppPickerForContainerDir = container->getDir();
                } else {
                    runOnMainUI([container, location]() {
                        MountInfo mount(B("t"), location, true);
                        std::vector<BoxedApp> items;
                        container->getNewApps(items, &mount);
                        new AppChooserDlg(items, [container](BoxedApp app) {
                            gotoView(VIEW_CONTAINERS, container->getDir(), app.getIniFilePath());
                            });
                        return false;
                        });
                }
            }
        } else if (this->errorMsg.isEmpty()) {
            runOnMainUI([container]() {
                gotoView(VIEW_CONTAINERS, container->getDir());
                return false;
                });
        }
        if (containerCreated) {
            if (!this->errorMsg.isEmpty()) {
                container->deleteContainerFromFilesystem();
                delete container;
            } else {
                BoxedwineData::addContainer(container);
            }
        }

        if (this->errorMsg.isEmpty() && GlobalSettings::startUpArgs.readyToLaunch) {
            static BString name;
            name = Fs::getFileNameFromNativePath(location);
            runOnMainUI([]() {
                new WaitDlg(Msg::WAITDLG_LAUNCH_APP_TITLE, getTranslationWithFormat(Msg::WAITDLG_LAUNCH_APP_LABEL, true, name));
                return false;
                });
        }
    }
}
