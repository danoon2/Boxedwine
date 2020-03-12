#include "boxedwine.h"
#include "../boxedwineui.h"

AppOptionsDlg::AppOptionsDlg(BoxedApp* app) : BaseDlg(APP_OPTIONS_DLG_TITLE, 600, 300), app(app), errorMsg(NULL) {
    this->nameLabel = getTranslation(APP_OPTIONS_DLG_NAME_LABEL);
    this->nameHelp = getTranslation(APP_OPTIONS_DLG_NAME_HELP);
    this->resolutionLabel = getTranslation(APP_OPTIONS_DLG_RESOLUTION_LABEL);
    this->resolutionHelp = getTranslation(APP_OPTIONS_DLG_RESOLUTION_HELP);
    this->bppLabel = getTranslation(APP_OPTIONS_DLG_BPP_LABEL);
    this->bppHelp = getTranslation(APP_OPTIONS_DLG_BPP_HELP);
    this->fullscreenLabel = getTranslation(APP_OPTIONS_DLG_FULL_SCREEN_LABEL);
    this->fullscreenHelp = getTranslation(APP_OPTIONS_DLG_FULL_SCREEN_HELP);
    this->scaleLabel = getTranslation(APP_OPTIONS_DLG_SCALE_LABEL);
    this->scaleHelp = getTranslation(APP_OPTIONS_DLG_SCALE_HELP);
    this->scaleQualityLabel = getTranslation(APP_OPTIONS_DLG_SCALE_QUALITY_LABEL);
    this->scaleQualityHelp = getTranslation(APP_OPTIONS_DLG_SCALE_QUALITY_HELP);
    this->glExtenstionsLabel = getTranslation(APP_OPTIONS_DLG_GL_EXT_LABEL);
    this->glExtenstionsSetButtonLabel = getTranslation(APP_OPTIONS_DLG_GL_EXT_SET_BUTTON_LABEL);
    this->glExtensionsHelp = getTranslation(APP_OPTIONS_DLG_GL_EXT_HELP);


    this->leftColumnWidth = ImGui::CalcTextSize(this->nameLabel);
    ImVec2 width = ImGui::CalcTextSize(this->resolutionLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->bppLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->fullscreenLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->scaleLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->scaleQualityLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    width = ImGui::CalcTextSize(this->glExtenstionsLabel);
    if (width.x > this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    this->leftColumnWidth.x += COLUMN_PADDING;

    this->resolutionComboboxData.data.push_back(getTranslation(APP_OPTIONS_DLG_DEFAULT_RESOLUTION_LABEL));
    this->resolutionComboboxData.data.push_back("640x480");
    this->resolutionComboboxData.data.push_back("800x600");
    this->resolutionComboboxData.data.push_back("1024x768");
    this->resolutionComboboxData.dataChanged();
    this->resolutionComboboxData.currentSelectedIndex = stringIndexInVector(app->resolution, this->resolutionComboboxData.data, 0);

    this->bppComboboxData.data.push_back("32-bit (default)");
    this->bppComboboxData.data.push_back("16-bit");
    this->bppComboboxData.data.push_back("8-bit (256 colors)");
    this->bppComboboxData.dataChanged();
    if (app->bpp == 16) {
        this->bppComboboxData.currentSelectedIndex = 1;
    } else if (app->bpp == 8) {
        this->bppComboboxData.currentSelectedIndex = 2;
    } else {
        this->bppComboboxData.currentSelectedIndex = 0;
    }

    this->scaleComboboxData.data.push_back("1/2x");
    this->scaleComboboxData.data.push_back("1x (default)");
    this->scaleComboboxData.data.push_back("2x");
    this->scaleComboboxData.data.push_back("3x");
    this->scaleComboboxData.dataChanged();
    if (app->scale == 50) {
        this->scaleComboboxData.currentSelectedIndex = 0;
    } else if (app->bpp == 200) {
        this->scaleComboboxData.currentSelectedIndex = 2;
    } else if (app->bpp == 300) {
        this->scaleComboboxData.currentSelectedIndex = 3;
    } else {
        this->scaleComboboxData.currentSelectedIndex = 1;
    }

    this->scaleQualityComboboxData.data.push_back("Nearest Pixel Sampling (default)");
    this->scaleQualityComboboxData.data.push_back("Linear Filtering");
    this->scaleQualityComboboxData.dataChanged();
    if (app->scaleQuality == 1) {
        this->scaleQualityComboboxData.currentSelectedIndex = 1;
    } else {
        this->scaleQualityComboboxData.currentSelectedIndex = 0;
    }

    strncpy(this->appName, this->app->name.c_str(), sizeof(this->appName));
    this->appName[sizeof(this->appName) - 1] = 0;

    strncpy(this->glExt, this->app->glExt.c_str(), sizeof(this->glExt));
    this->glExt[sizeof(this->glExt) - 1] = 0;
}

void AppOptionsDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->nameLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth((this->nameHelp ? -this->toolTipWidth : 0));
    ImGui::InputText("##appNameID", this->appName, sizeof(this->appName));
    ImGui::PopItemWidth();
    if (this->nameHelp) {
        ImGui::SameLine();
        this->toolTip(this->nameHelp);
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->resolutionLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1 - (this->resolutionHelp ? this->toolTipWidth : 0));
    ImGui::Combo("##AppResolutionCombo", &this->resolutionComboboxData.currentSelectedIndex, this->resolutionComboboxData.dataForCombobox);
    ImGui::PopItemWidth();
    if (this->resolutionHelp) {
        ImGui::SameLine();
        toolTip(this->resolutionHelp);
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->bppLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1 - (this->bppHelp ? this->toolTipWidth : 0));
    ImGui::Combo("##AppBppCombo", &this->bppComboboxData.currentSelectedIndex, this->bppComboboxData.dataForCombobox);
    ImGui::PopItemWidth();
    if (this->bppHelp) {
        ImGui::SameLine();
        toolTip(this->bppHelp);
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->scaleLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1 - (this->scaleHelp ? this->toolTipWidth : 0));
    ImGui::Combo("##AppScaleCombo", &this->scaleComboboxData.currentSelectedIndex, this->scaleComboboxData.dataForCombobox);
    ImGui::PopItemWidth();
    if (this->scaleHelp) {
        ImGui::SameLine();
        toolTip(this->scaleHelp);
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing / 2));
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->scaleQualityLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(-1 - (this->scaleQualityHelp ? this->toolTipWidth : 0));
    ImGui::Combo("##AppScaleQualityCombo", &this->scaleQualityComboboxData.currentSelectedIndex, this->scaleQualityComboboxData.dataForCombobox);
    ImGui::PopItemWidth();
    if (this->scaleQualityHelp) {
        ImGui::SameLine();
        toolTip(this->scaleQualityHelp);
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->glExtenstionsLabel);
    if (ImGui::Button(this->glExtenstionsSetButtonLabel)) {
        strcpy(this->glExt, "GL_EXT_multi_draw_arrays GL_ARB_vertex_program\nGL_ARB_fragment_program GL_ARB_multitexture\nGL_EXT_secondary_color GL_EXT_texture_lod_bias\nGL_NV_texture_env_combine4 GL_ATI_texture_env_combine3\nGL_EXT_texture_filter_anisotropic GL_ARB_texture_env_combine\nGL_EXT_texture_env_combine GL_EXT_texture_compression_s3tc\nGL_ARB_texture_compression GL_EXT_paletted_texture");
    }
    ImGui::EndGroup();
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::InputTextMultiline("##appGlExtID", this->glExt, sizeof(this->glExt), ImVec2((this->glExtensionsHelp ? -this->toolTipWidth : 0), ImGui::GetTextLineHeight() * 4));
    ImGui::SameLine();    
    if (this->glExtensionsHelp) {
        ImGui::SameLine();        
        this->toolTip(this->glExtensionsHelp);
    }

    this->addOkAndCancelButtons();
}

void AppOptionsDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        if (!this->appName[0]) {
            this->errorMsg = getTranslation(APP_OPTIONS_DLG_NAME_REQUIRED);
        }
    }
    if (this->errorMsg) {
        if (!showMessageBox(buttonClicked, getTranslation(GENERIC_DLG_ERROR_TITLE), this->errorMsg)) {
            this->errorMsg = NULL;
        }
    }
    if (buttonClicked && !this->errorMsg) {
        std::string ext = this->glExt;
        stringReplaceAll(ext, "\n", " ");
        stringReplaceAll(ext, "  ", " ");
        stringReplaceAll(ext, "  ", " ");
        this->app->glExt = ext;
        this->app->name = this->appName;
        if (this->resolutionComboboxData.currentSelectedIndex == 0) {
            this->app->resolution = "";
        } else {
            this->app->resolution = this->resolutionComboboxData.data[this->resolutionComboboxData.currentSelectedIndex];
        }        
        if (this->bppComboboxData.currentSelectedIndex == 0) {
            this->app->bpp = 32;
        } else if (this->bppComboboxData.currentSelectedIndex == 1) {
            this->app->bpp = 16;
        } else if (this->bppComboboxData.currentSelectedIndex == 2) {
            this->app->bpp = 8;
        }
        if (this->scaleComboboxData.currentSelectedIndex == 0) {
            this->app->scale = 50;
        } else if (this->scaleComboboxData.currentSelectedIndex == 1) {
            this->app->scale = 100;
        } else if (this->scaleComboboxData.currentSelectedIndex == 2) {
            this->app->scale = 200;
        } else if (this->scaleComboboxData.currentSelectedIndex == 3) {
            this->app->scale = 300;
        }
        this->app->scaleQuality = this->scaleQualityComboboxData.currentSelectedIndex;
        this->app->saveApp();
        GlobalSettings::reloadApps();
        this->done();
    }
}