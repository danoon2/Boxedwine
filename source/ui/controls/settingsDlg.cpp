#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

SettingsDlg::SettingsDlg() : BaseDlg(SETTINGS_DLG_TITLE, 600, 300), errorMsg(NULL) {
    this->saveFolderLabel = getTranslation(SETUPDLG_SAVE_FOLDER_LABEL);
    this->saveFolderHelp = getTranslation(SETUPDLG_SAVE_FOLDER_HELP, false);
    this->themeLabel = getTranslation(SETUPDLG_THEME_LABEL);
    this->themeHelp = getTranslation(SETUPDLG_THEME_HELP, false);
    this->browseButtonText = getTranslation(GENERIC_BROWSE_BUTTON);
    this->leftColumnWidth = ImGui::CalcTextSize(this->saveFolderLabel);

    ImVec2 labelSize = ImGui::CalcTextSize(this->saveFolderLabel);
    if (labelSize.x>this->leftColumnWidth.x) {
        leftColumnWidth.x = labelSize.x;
    }
    this->leftColumnWidth.x+=COLUMN_PADDING;
    this->rightColumnWidth = ImGui::CalcTextSize(this->browseButtonText);
    this->rightColumnWidth.x+=8+COLUMN_PADDING; // more space for button;
    strcpy(this->saveFolderLocationBuffer, GlobalSettings::getDataFolder().c_str());

    this->themeComboboxData.data.push_back(getTranslation(SETUPDLG_THEME_DARK));
    this->themeComboboxData.data.push_back(getTranslation(SETUPDLG_THEME_LIGHT));
    this->themeComboboxData.data.push_back(getTranslation(SETUPDLG_THEME_CLASSIC));
    this->themeComboboxData.dataChanged();
    if (GlobalSettings::getTheme()=="Light") {
        this->themeComboboxData.currentSelectedIndex = 1;
    } else if (GlobalSettings::getTheme()=="Classic") {
        this->themeComboboxData.currentSelectedIndex = 2;
    } else {
        this->themeComboboxData.currentSelectedIndex = 0;
    }
}

void SettingsDlg::run() {
    if (this->themeComboboxData.currentSelectedIndex==0) {
        ImGui::StyleColorsDark();
        ImGui::GetStyle().FrameBorderSize = 0.0f;
    } else if (this->themeComboboxData.currentSelectedIndex==1) {
        ImGui::StyleColorsLight();
        ImGui::GetStyle().FrameBorderSize = 1.0f;
    } else {
        ImGui::StyleColorsClassic();
        ImGui::GetStyle().FrameBorderSize = 0.0f;
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));    
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->saveFolderLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-this->rightColumnWidth.x-(saveFolderHelp?this->toolTipWidth:0));
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

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing/2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->themeLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1-(this->themeHelp?this->toolTipWidth:0));
    ImGui::Combo("##ThemeCombo", &this->themeComboboxData.currentSelectedIndex, this->themeComboboxData.dataForCombobox);
    ImGui::PopItemWidth();        
    if (this->themeHelp) {
        ImGui::SameLine();
        toolTip(this->themeHelp);
    }
    this->addOkAndCancelButtons();
}

void SettingsDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->errorMsg = NULL;
        if (!Fs::doesNativePathExist(this->saveFolderLocationBuffer)) {
            this->errorMsg = getTranslation(SETUPDLG_ERROR_DATA_DIR_NOT_FOUND);
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
    }

    if (this->errorMsg) {        
        if (!showMessageBox(buttonClicked, getTranslation(GENERIC_DLG_ERROR_TITLE), this->errorMsg)) {
            this->errorMsg = NULL;
        }
    }
    if (buttonClicked && !this->errorMsg) {          
        this->done();
    }
}

void SettingsDlg::done() {
    GlobalSettings::loadTheme();
    BaseDlg::done();
}