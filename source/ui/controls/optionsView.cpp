#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

#ifdef BOXEDWINE_OPENGL_OSMESA
bool isMesaOpenglAvailable();
#endif

OptionsView::OptionsView(BString tab) : BaseView(B("OptionsView")) {
    if (tab.length()) {
        if (tab == "Wine") {
            this->tabIndex = 2;
        } if (tab == "Display") {
            this->tabIndex = 1;
        }

    }
    this->wineTitle = c_getTranslation(Msg::OPTIONSVIEW_TITLE_FILESYSTEM);

    createGeneralTab();
    createThemeTab();
    
    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += OPTION_WINE_ICON;
        name += " ";
    }
    name += this->wineTitle;
    addTab(name, name, NULL, [this](bool buttonPressed, BaseViewTab& tab) {
        this->runWineOptions();
        });    

    loadFileSystemVersions();
}

void OptionsView::createThemeTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::OPTIONSVIEW_TITLE_DISPLAY);
    
    std::vector<ComboboxItem> themes;
    themes.push_back(ComboboxItem(getTranslation(Msg::OPTIONSVIEW_THEME_DARK), B("Dark")));
    themes.push_back(ComboboxItem(getTranslation(Msg::OPTIONSVIEW_THEME_LIGHT), B("Light")));
    themes.push_back(ComboboxItem(getTranslation(Msg::OPTIONSVIEW_THEME_CLASSIC), B("Classic")));

    themeControl = section->addComboboxRow(Msg::OPTIONSVIEW_THEME_LABEL, Msg::OPTIONSVIEW_THEME_HELP, themes);
    themeControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));

    themeControl->setSelectionStringValue(GlobalSettings::getTheme());
    themeControl->onChange = [this]() {
        GlobalSettings::setTheme(this->themeControl->getSelectionStringValue());
        GlobalSettings::saveConfig();
    };

    std::vector<ComboboxItem> fontScales;
    fontScales.push_back(ComboboxItem(B("50%"), 50));
    fontScales.push_back(ComboboxItem(B("75%"), 75));
    fontScales.push_back(ComboboxItem(B("100%"), 100));
    fontScales.push_back(ComboboxItem(B("125%"), 125));
    fontScales.push_back(ComboboxItem(B("150%"), 150));
    fontScales.push_back(ComboboxItem(B("200%"), 200));
    std::shared_ptr<LayoutComboboxControl> fontScale = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_FONT_SCALE_LABEL, Msg::OPTIONSVIEW_DEFAULT_FONT_SCALE_HELP, fontScales);
    fontScale->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    fontScale->setSelectionIntValue((int)(GlobalSettings::fontScale * 100+.5f));
    fontScale->onChange = [fontScale]() {
        runAfterFrame([fontScale]() {
            GlobalSettings::setFontScale(fontScale->getSelectionIntValue() / 100.0f);
            GlobalSettings::saveConfig();
            GlobalSettings::restartUI = true;
            GlobalSettings::startUpArgs.runOnRestartUI = []() {
                gotoView(VIEW_OPTIONS, B("Display"));
            };
            return false;
        });
    };

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += OPTION_DISPLAY_ICON;
        name += " ";
    }
    name += getTranslation(Msg::OPTIONSVIEW_TITLE_DISPLAY);
    addTab(name, name, model, [](bool buttonPressed, BaseViewTab& tab) {
        });
}

void OptionsView::createGeneralTab() {
    std::shared_ptr<ImGuiLayout> model = std::make_shared<ImGuiLayout>();
    std::shared_ptr<LayoutSection> section = model->addSection(Msg::OPTIONSVIEW_TITLE_GENERAL);

    saveLocationControl = section->addTextInputRow(Msg::OPTIONSVIEW_SAVE_FOLDER_LABEL, Msg::OPTIONSVIEW_SAVE_FOLDER_HELP, GlobalSettings::getDataFolder());
    saveLocationControl->setBrowseDirButton();
    // :TODO: track when we loose focus and save

    std::vector<ComboboxItem> resolutions;
    for (auto& res : GlobalSettings::getAvailableResolutions()) {
        resolutions.push_back(ComboboxItem(res));
    }
    resolutionControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_RESOLUTION_LABEL, Msg::OPTIONSVIEW_DEFAULT_RESOLUTION_HELP, resolutions);
    resolutionControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    resolutionControl->setSelectionByLabel(GlobalSettings::getDefaultResolution());
    resolutionControl->onChange = [this]() {
        GlobalSettings::defaultResolution = GlobalSettings::availableResolutions[this->resolutionControl->getSelection()];
        GlobalSettings::saveConfig();
    };

    std::vector<ComboboxItem> vsync;
    vsync.push_back(ComboboxItem(B("Disabled"), VSYNC_DISABLED));
    vsync.push_back(ComboboxItem(B("Enabled"), VSYNC_ENABLED));
    vsync.push_back(ComboboxItem(B("Adaptive"), VSYNC_ADAPTIVE));
    vsyncControl = section->addComboboxRow(Msg::OPTIONS_VIEW_VSYNC_LABEL, Msg::OPTIONS_VIEW_VSYNC_HELP, vsync, GlobalSettings::defaultVsync);
    vsyncControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    vsyncControl->onChange = [this]() {
        GlobalSettings::defaultVsync = this->vsyncControl->getSelectionIntValue();
        GlobalSettings::saveConfig();
    };

    std::vector<ComboboxItem> scales;
    scales.push_back(ComboboxItem(B("1/2x"), 50));
    scales.push_back(ComboboxItem(B("1x"), 100));
    scales.push_back(ComboboxItem(B("2x"), 200));
    scales.push_back(ComboboxItem(B("3x"), 300));
    scaleControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_SCALE_LABEL, Msg::OPTIONSVIEW_DEFAULT_SCALE_HELP, scales, GlobalSettings::getDefaultScale()/100);
    scaleControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    scaleControl->onChange = [this]() {
        GlobalSettings::defaultScale = this->scaleControl->getSelectionIntValue();
        GlobalSettings::saveConfig();
    };

#if defined(BOXEDWINE_OPENGL_OSMESA) && defined(BOXEDWINE_OPENGL_SDL)
    if (isMesaOpenglAvailable()) {
        std::vector<ComboboxItem> glOptions;
        glOptions.push_back(ComboboxItem(B("Native"), OPENGL_TYPE_SDL));
        glOptions.push_back(ComboboxItem(B("Mesa - OpenGL in Software"), OPENGL_TYPE_OSMESA));
        openGlControl = section->addComboboxRow(Msg::OPTIONSVIEW_DEFAULT_OPENGL_LABEL, Msg::OPTIONSVIEW_DEFAULT_OPENGL_HELP, glOptions, GlobalSettings::defaultOpenGL);
        openGlControl->setWidth((int)GlobalSettings::scaleFloatUIAndFont(250));
        openGlControl->setSelectionIntValue(GlobalSettings::defaultOpenGL);
        openGlControl->onChange = [this]() {
            GlobalSettings::defaultOpenGL = this->openGlControl->getSelectionIntValue();
            GlobalSettings::saveConfig();
        };
    }
#endif

#ifdef BOXEDWINE_RECORDER
    automationControl = section->addCheckbox(Msg::OPTIONSVIEW_ENABLE_AUTOMATION_LABEL, Msg::OPTIONSVIEW_ENABLE_AUTOMATION_HELP, GlobalSettings::enabledAutomation);
    automationControl->onChange = [this]() {
        GlobalSettings::enabledAutomation = this->automationControl->isChecked();
        GlobalSettings::saveConfig();
    };
#endif

    std::shared_ptr<LayoutSection> bottomSection = model->addSection();
    bottomSection->addSeparator();
    BString deleteLabel;
    if (GlobalSettings::hasIconsFont()) {
        deleteLabel += TRASH_ICON;
        deleteLabel += " ";
    }
    deleteLabel += getTranslation(Msg::OPTIONS_VIEW_DELETE_ALL_BUTTON_LABEL);
    std::shared_ptr<LayoutButtonControl> deleteAllContainersButton = bottomSection->addButton(Msg::NONE, Msg::OPTIONS_VIEW_DELETE_ALL_BUTTON_HELP, deleteLabel);
    deleteAllContainersButton->onChange = [this]() {
        runOnMainUI([this]() {
            new YesNoDlg(Msg::GENERIC_DLG_CONFIRM_TITLE, getTranslation(Msg::OPTIONS_VIEW_DELETE_ALL_CONFIRM), [this](bool yes) {
                if (yes) {
                    runOnMainUI([]() {
                        Fs::deleteNativeDirAndAllFilesInDir(GlobalSettings::getDataFolder());
                        GlobalSettings::restartUI = true;
                        GlobalSettings::reinit = true;                      
                        return false;
                        });
                }
                });
            return false;
            });
    };

    BString name;
    if (GlobalSettings::hasIconsFont()) {
        name += OPTIONS_GENERAL;
        name += " ";
    }
    name += getTranslation(Msg::OPTIONSVIEW_TITLE_GENERAL);
    addTab(name, name, model, [](bool buttonPressed, BaseViewTab& tab) {

        });
}

void OptionsView::loadFileSystemVersions() {
    this->wineButtonTotalColumnWidth = 0;
    this->wineButtonFirstColumnWidth = 0;
    this->fileSystemVersions.clear();

    for (auto& fileSystem : GlobalSettings::availableFileSystemVersions) {
        OptionsViewWineVersion v;
        v.availableVersion = fileSystem;
        v.name = fileSystem->name;

        U64 size = fileSystem->size;
        std::shared_ptr<FileSystemZip> dep = fileSystem->getMissingDependency();
        if (dep) {
            size += dep->size;
        }
        v.size = BString::valueOf(size);
        this->fileSystemVersions[v.name] = v;
    }

    for (auto& fileSystem : GlobalSettings::fileSystemVersions) {
        if (fileSystem->size == 0) {
            fileSystem->size = (U32)(Fs::getNativeFileSize(fileSystem->filePath) / 1024 / 1024);
        }
        if (this->fileSystemVersions.count(fileSystem->name)) {
            OptionsViewWineVersion& v = this->fileSystemVersions[fileSystem->name];
            v.currentVersion = fileSystem;
        } else {
            OptionsViewWineVersion v;
            v.availableVersion = fileSystem;
            v.name = fileSystem->name;
            
            U64 size = fileSystem->size;
            std::shared_ptr<FileSystemZip> dep = fileSystem->getMissingDependency();
            if (dep) {
                size += dep->size;
            }
            v.size = BString::valueOf(size);
            this->fileSystemVersions[v.name] = v;
        }
    }
    float leftColumn = 0;
    float rightColumn = 0;

    ImGui::PushFont(GlobalSettings::mediumFont);
    for (auto& wine : this->fileSystemVersions) {
        float width = 0.0f;
        if (wine.second.availableVersion && !wine.second.currentVersion) {
            width += ImGui::CalcTextSize(c_getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_INSTALL)).x;            
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > leftColumn) {
                leftColumn = width;
            }
        } else if (wine.second.availableVersion && wine.second.currentVersion && wine.second.currentVersion->fsVersion != wine.second.availableVersion->fsVersion) {
            width += ImGui::CalcTextSize(c_getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPDATE)).x;
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > leftColumn) {
                leftColumn = width;
            }
        }
        if (wine.second.currentVersion) {
            width = ImGui::CalcTextSize(c_getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_DELETE)).x;
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > rightColumn) {
                rightColumn = width;
            }
        } 
        if (width > 0) {
            width += ImGui::GetStyle().ItemSpacing.x;
        }
        if (width > this->wineButtonTotalColumnWidth) {
            this->wineButtonTotalColumnWidth = width;
        }
    }
    if (leftColumn > 0 && rightColumn > 0) {
        this->wineButtonFirstColumnWidth = leftColumn + ImGui::GetStyle().ItemSpacing.x;
        this->wineButtonTotalColumnWidth = this->wineButtonFirstColumnWidth + rightColumn + ImGui::GetStyle().ItemSpacing.x;
    } else if (leftColumn > 0) {
        this->wineButtonTotalColumnWidth = leftColumn + ImGui::GetStyle().ItemSpacing.x;
    } else if (rightColumn > 0) {
        this->wineButtonTotalColumnWidth = rightColumn + ImGui::GetStyle().ItemSpacing.x;
    }
    ImGui::PopFont();
}

void OptionsView::runWineOptions() {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->wineTitle);
    ImGui::PopFont();

    ImVec2 size = ImGui::GetWindowContentRegionMax();
    size.y -= ImGui::GetCursorPosY() + GlobalSettings::scaleFloatUI(8.0f);

    ImGui::PushFont(GlobalSettings::mediumFont);
    //ImGui::BeginChildFrame(401, size);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    for (auto& wine : this->fileSystemVersions) {
        ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));        
        ImGui::SameLine();
        ImVec2 pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
        if (wine.second.availableVersion && !wine.second.currentVersion) {
            BString buttonLabel = getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_INSTALL);
            buttonLabel += "##";
            buttonLabel += wine.first;
            if (ImGui::Button(buttonLabel.c_str())) {
                download(wine.second.availableVersion);
            }
            ImGui::SameLine();
        } else if (wine.second.availableVersion && wine.second.currentVersion && wine.second.currentVersion->fsVersion!=wine.second.availableVersion->fsVersion){
            BString buttonLabel = getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPDATE);
            buttonLabel += "##";
            buttonLabel += wine.first;
            if (ImGui::Button(buttonLabel.c_str())) {
                download(wine.second.availableVersion);
            }
            ImGui::SameLine();
        }
        if (wine.second.currentVersion) {
            ImGui::SameLine(pos.x + this->wineButtonFirstColumnWidth);
            bool buttonPressed = false;
            BString buttonLabel = getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_DELETE);
            buttonLabel += "##";
            buttonLabel += wine.first;
            if (ImGui::Button(buttonLabel.c_str())) {
                buttonPressed = true;
            }
            bool yes = false;
            BString label = getTranslationWithFormat(Msg::OPTIONSVIEW_WINE_VERSION_DELETE_CONFIRM_LABEL, true, wine.first);
            if (!showYesNoMessageBox("confirm"+wine.first, buttonPressed, c_getTranslation(Msg::GENERIC_DLG_CONFIRM_TITLE), label.c_str(), &yes)) {
                if (yes) {
                    Fs::deleteNativeFile(wine.second.currentVersion->filePath);
                    // run later, we don't want to change versions while we are iterating them
                    runOnMainUI([this]()->bool {
                        GlobalSettings::reloadWineVersions();
                        this->loadFileSystemVersions();
                        return false;
                        });
                }
            }
            ImGui::SameLine();
        }     
        ImGui::SameLine(pos.x+this->wineButtonTotalColumnWidth);
        BString name = wine.first;
        BString name2;
        OptionsViewWineVersion& version = wine.second;

        if (version.currentVersion) {
            if (version.availableVersion && version.availableVersion->fsVersion==version.currentVersion->fsVersion) {
                name2 += "   ";
                name2 += getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPTODATE);
                name2 += version.size;
                name2 += " MB";
            } else {
                name2 += "   ";
                name2 += getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_UPDATE_AVAILABLE);
                name2 += version.size;
                name2 += " MB";
            }
        } else if (version.availableVersion) {
            name2 += "   ";
            name2 += getTranslation(Msg::OPTIONSVIEW_WINE_VERSION_NOT_INSTALLED);
            name2 += version.size;
            name2 += " MB";
        }
        SAFE_IMGUI_TEXT(name.c_str());
        ImGui::SameLine();
        ImGui::PushFont(GlobalSettings::defaultFont);
        pos = ImGui::GetCursorPos();
        if (GlobalSettings::mediumFont) {
            pos.y += GlobalSettings::mediumFont->FontSize / 2 - GlobalSettings::defaultFont->FontSize / 2;
        }
        ImGui::SetCursorPos(pos);
        SAFE_IMGUI_TEXT(name2.c_str());
        ImGui::PopFont();
        pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
    }
    ImGui::PopFont();
    //ImGui::EndChildFrame();
}

void OptionsView::download(const std::shared_ptr<FileSystemZip>& version) {
    GlobalSettings::downloadFileSystem(version, [this](bool success) {
        if (success) {
            runOnMainUI([this]()->bool {        
                GlobalSettings::reloadWineVersions();
                this->loadFileSystemVersions();
                return false;
                });
        }
        });
}

bool OptionsView::saveChanges() {
    this->errorMsg = B("");
    if (!Fs::doesNativePathExist(this->saveLocationControl->getText())) {
        this->errorMsg = getTranslation(Msg::OPTIONSVIEW_ERROR_DATA_DIR_NOT_FOUND);
    }
    if (this->errorMsg.isEmpty()) {
        GlobalSettings::setDataFolder(this->saveLocationControl->getText());
        GlobalSettings::saveConfig();
    }

    if (!this->errorMsg.isEmpty()) {
        runErrorMsg(true);
    }
    return this->errorMsg.isEmpty();
}