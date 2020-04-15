#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

OptionsView::OptionsView(const char* startingTab) : BaseView("OptionsView"), lastThemeSelectionIndex(-1) {
    if (startingTab) {
        if (!strcmp(startingTab, "Wine")) {
            this->tabIndex = 2;
        } if (!strcmp(startingTab, "Display")) {
            this->tabIndex = 1;
        }

    }
    this->generalTitle = getTranslation(OPTIONSVIEW_TITLE_GENERAL);
    this->displayTitle = getTranslation(OPTIONSVIEW_TITLE_DISPLAY);
    this->wineTitle = getTranslation(OPTIONSVIEW_TITLE_WINE_VERSISONS);

    this->saveFolderLabel = getTranslation(OPTIONSVIEW_SAVE_FOLDER_LABEL);
    this->saveFolderHelp = getTranslation(OPTIONSVIEW_SAVE_FOLDER_HELP, false);
    this->themeLabel = getTranslation(OPTIONSVIEW_THEME_LABEL);
    this->themeHelp = getTranslation(OPTIONSVIEW_THEME_HELP, false);
    this->browseButtonText = getTranslation(GENERIC_BROWSE_BUTTON);
    
    this->leftColumnWidthGeneral = ImGui::CalcTextSize(this->saveFolderLabel).x;
    this->leftColumnWidthGeneral += ImGui::GetStyle().ItemSpacing.x;

    this->leftColumnWidthDisplay = ImGui::CalcTextSize(this->themeLabel).x;
    this->leftColumnWidthDisplay += ImGui::GetStyle().ItemSpacing.x;

    this->rightColumnWidth = ImGui::CalcTextSize(this->browseButtonText).x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
    
    strcpy(this->saveFolderLocationBuffer, GlobalSettings::getDataFolder().c_str());

    this->themeComboboxData.data.push_back(getTranslation(OPTIONSVIEW_THEME_DARK));
    this->themeComboboxData.data.push_back(getTranslation(OPTIONSVIEW_THEME_LIGHT));
    this->themeComboboxData.data.push_back(getTranslation(OPTIONSVIEW_THEME_CLASSIC));
    this->themeComboboxData.dataChanged();
    if (GlobalSettings::getTheme()=="Light") {
        this->themeComboboxData.currentSelectedIndex = 1;
    } else if (GlobalSettings::getTheme()=="Classic") {
        this->themeComboboxData.currentSelectedIndex = 2;
    } else {
        this->themeComboboxData.currentSelectedIndex = 0;
    }

    addTab(this->generalTitle, [this](bool buttonPressed, BaseViewTab& tab) {
        this->runGeneralOptions();
        });
    addTab(this->displayTitle, [this](bool buttonPressed, BaseViewTab& tab) {
        if (this->lastThemeSelectionIndex != this->themeComboboxData.currentSelectedIndex) {
            if (this->themeComboboxData.currentSelectedIndex == 0) {
                ImGui::StyleColorsDark();
                ImGui::GetStyle().FrameBorderSize = 0.0f;
            } else if (this->themeComboboxData.currentSelectedIndex == 1) {
                ImGui::StyleColorsLight();
                ImGui::GetStyle().FrameBorderSize = 1.0f;
            } else {
                ImGui::StyleColorsClassic();
                ImGui::GetStyle().FrameBorderSize = 0.0f;
            }
            this->lastThemeSelectionIndex = this->themeComboboxData.currentSelectedIndex;
        }
        this->runDisplayOptions();
        });
    addTab(this->wineTitle, [this](bool buttonPressed, BaseViewTab& tab) {
        this->runWineOptions();
        });    

    loadWineVersions();
}

void OptionsView::loadWineVersions() {
    this->wineButtonTotalColumnWidth = 0;
    this->wineButtonFirstColumnWidth = 0;
    this->wineVersions.clear();

    for (WineVersion& wine : GlobalSettings::availableWineVersions) {
        OptionsViewWineVersion v;
        v.availableVersion = &wine;
        v.name = wine.name;
        this->wineVersions[v.name] = v;
    }

    for (WineVersion& wine : GlobalSettings::wineVersions) {
        if (wine.size == 0) {
            wine.size = (U32)(Fs::getNativeFileSize(wine.filePath) / 1024 / 1024);
        }
        if (this->wineVersions.count(wine.name)) {
            OptionsViewWineVersion& v = this->wineVersions[wine.name];
            v.currentVersion = &wine;
        } else {
            OptionsViewWineVersion v;
            v.availableVersion = &wine;
            v.name = wine.name;
            this->wineVersions[v.name] = v;
        }
    }
    float leftColumn = 0;
    float rightColumn = 0;

    ImGui::PushFont(GlobalSettings::mediumFont);
    for (auto& wine : this->wineVersions) {
        float width = 0.0f;
        if (wine.second.availableVersion && !wine.second.currentVersion) {
            width += ImGui::CalcTextSize(getTranslation(OPTIONSVIEW_WINE_VERSION_INSTALL)).x;            
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > leftColumn) {
                leftColumn = width;
            }
        } else if (wine.second.availableVersion && wine.second.currentVersion && wine.second.currentVersion->fsVersion != wine.second.availableVersion->fsVersion) {
            width += ImGui::CalcTextSize(getTranslation(OPTIONSVIEW_WINE_VERSION_UPDATE)).x;
            width += ImGui::GetStyle().FramePadding.x * 2;
            if (width > leftColumn) {
                leftColumn = width;
            }
        }
        if (wine.second.currentVersion) {
            width = ImGui::CalcTextSize(getTranslation(OPTIONSVIEW_WINE_VERSION_DELETE)).x;
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

void OptionsView::runDisplayOptions() {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->displayTitle);
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->themeLabel);
    ImGui::SameLine(this->leftColumnWidthDisplay);
    ImGui::PushItemWidth(-1 - (this->themeHelp ? this->toolTipWidth : 0));
    ImGui::Combo("##ThemeCombo", &this->themeComboboxData.currentSelectedIndex, this->themeComboboxData.dataForCombobox);
    ImGui::PopItemWidth();
    if (this->themeHelp) {
        ImGui::SameLine();
        toolTip(this->themeHelp);
    }
}

void OptionsView::runGeneralOptions() {    
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->generalTitle);
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->saveFolderLabel);
    ImGui::SameLine(this->leftColumnWidthGeneral);
    ImGui::PushItemWidth(-this->rightColumnWidth - (saveFolderHelp ? this->toolTipWidth : 0));
    ImGui::InputText("##LocationID", this->saveFolderLocationBuffer, sizeof(this->saveFolderLocationBuffer));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(this->browseButtonText)) {
        const char* result = tfd::selectFolderDialog(getTranslation(GENERIC_OPEN_FOLDER_TITLE), this->saveFolderLocationBuffer);
        if (result) {
            strcpy(this->saveFolderLocationBuffer, result);
        }
    }
    if (this->saveFolderHelp) {
        ImGui::SameLine();
        this->toolTip(this->saveFolderHelp);
    }
}

void OptionsView::runWineOptions() {
    ImGui::PushFont(GlobalSettings::largeFont);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    SAFE_IMGUI_TEXT(this->wineTitle);
    ImGui::PopFont();

    ImVec2 size = ImGui::GetWindowContentRegionMax();
    size.y -= ImGui::GetCursorPosY() + GlobalSettings::scaleFloatUI(8.0f);

    ImGui::PushFont(GlobalSettings::mediumFont);
    ImGui::BeginChildFrame(401, size);
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    for (auto& wine : this->wineVersions) {
        ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));        
        ImGui::SameLine();
        ImVec2 pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
        if (wine.second.availableVersion && !wine.second.currentVersion) {
            std::string buttonLabel = getTranslation(OPTIONSVIEW_WINE_VERSION_INSTALL);
            buttonLabel += "##" + wine.first;
            if (ImGui::Button(buttonLabel.c_str())) {
                download(wine.second.availableVersion);
            }
            ImGui::SameLine();
        } else if (wine.second.availableVersion && wine.second.currentVersion && wine.second.currentVersion->fsVersion!=wine.second.availableVersion->fsVersion){
            std::string buttonLabel = getTranslation(OPTIONSVIEW_WINE_VERSION_UPDATE);
            buttonLabel += "##" + wine.first;
            if (ImGui::Button(buttonLabel.c_str())) {
                download(wine.second.availableVersion);
            }
            ImGui::SameLine();
        }
        if (wine.second.currentVersion) {
            ImGui::SameLine(pos.x + this->wineButtonFirstColumnWidth);
            bool buttonPressed = false;
            std::string buttonLabel = getTranslation(OPTIONSVIEW_WINE_VERSION_DELETE);
            buttonLabel += "##" + wine.first;
            if (ImGui::Button(buttonLabel.c_str())) {
                buttonPressed = true;
            }
            bool yes = false;
            std::string label = getTranslationWithFormat(OPTIONSVIEW_WINE_VERSION_DELETE_CONFIRM_LABEL, true, wine.first);
            if (!showYesNoMessageBox("confirm"+wine.first, buttonPressed, getTranslation(GENERIC_DLG_CONFIRM_TITLE), label.c_str(), &yes)) {
                if (yes) {
                    Fs::deleteNativeFile(wine.second.currentVersion->filePath);
                    // run later, we don't want to change versions while we are iterating them
                    runOnMainUI([this]()->bool {
                        GlobalSettings::reloadWineVersions();
                        this->loadWineVersions();
                        return false;
                        });
                }
            }
            ImGui::SameLine();
        }     
        ImGui::SameLine(pos.x+this->wineButtonTotalColumnWidth);
        std::string name = wine.first;
        std::string name2;
        OptionsViewWineVersion& version = wine.second;
        if (version.currentVersion) {
            if (version.availableVersion && version.availableVersion->fsVersion==version.currentVersion->fsVersion) {
                name2 += "   ";
                name2 += getTranslation(OPTIONSVIEW_WINE_VERSION_UPTODATE);
                name2 += std::to_string(version.availableVersion->size);
                name2 += " MB";
            } else {
                name2 += "   ";
                name2 += getTranslation(OPTIONSVIEW_WINE_VERSION_UPDATE_AVAILABLE);
                name2 += std::to_string(version.currentVersion->size);
                name2 += " MB";
            }
        } else if (version.availableVersion) {
            name2 += "   ";
            name2 += getTranslation(OPTIONSVIEW_WINE_VERSION_NOT_INSTALLED);
            name2 += std::to_string(version.availableVersion->size);
            name2 += " MB";
        }
        SAFE_IMGUI_TEXT(name.c_str());
        ImGui::SameLine();
        ImGui::PushFont(GlobalSettings::defaultFont);
        pos = ImGui::GetCursorPos();
        pos.y += GlobalSettings::mediumFont->FontSize / 2 - GlobalSettings::defaultFont->FontSize / 2;
        ImGui::SetCursorPos(pos);
        SAFE_IMGUI_TEXT(name2.c_str());
        ImGui::PopFont();
        pos = ImGui::GetCursorPos();
        pos.y += this->extraVerticalSpacing;
        ImGui::SetCursorPos(pos);
    }
    ImGui::PopFont();
    ImGui::EndChildFrame();
}

void OptionsView::download(WineVersion* version) {
    GlobalSettings::downloadWine(*version, [this](bool success) {
        if (success) {
            runOnMainUI([this]()->bool {                
                this->loadWineVersions();
                return false;
                });
        }
        });
}

bool OptionsView::saveChanges() {
    this->errorMsg = NULL;
    if (!Fs::doesNativePathExist(this->saveFolderLocationBuffer)) {
        this->errorMsg = getTranslation(OPTIONSVIEW_ERROR_DATA_DIR_NOT_FOUND);
    }
    if (!this->errorMsg) {
        GlobalSettings::setDataFolder(this->saveFolderLocationBuffer);
        std::string theme;
        if (this->themeComboboxData.currentSelectedIndex==1) {
            theme = "Light";
        } else if (this->themeComboboxData.currentSelectedIndex==2) {
            theme = "Classic";
        } else {
            theme = "Dark";
        }
        GlobalSettings::setTheme(theme);
        GlobalSettings::saveConfig();
    }

    if (this->errorMsg) {
        runErrorMsg(true);
    }
    return this->errorMsg == NULL;
}