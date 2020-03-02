#include "boxedwine.h"
#include "../boxedwineui.h"

ContainerOptionsDlg::ContainerOptionsDlg(BoxedContainer* container) : BaseDlg(CONTAINER_OPTIONS_DLG_TITLE, 600, 300), container(container) {
    this->wineVersionLabel = getTranslation(COMMON_WINE_VERSION_LABEL);
    this->wineVersionHelp = getTranslation(COMMON_WINE_VERSION_HELP, false);
    this->addAppLabel = getTranslation(CONTAINER_OPTIONS_DLG_ADD_APP_LABEL);
    this->addAppHelp = getTranslation(CONTAINER_OPTIONS_DLG_ADD_APP_HELP, false);
    this->addAppButtonLabel = getTranslation(CONTAINER_OPTIONS_DLG_ADD_APP_BUTTON_LABEL);

    this->leftColumnWidth = ImGui::CalcTextSize(this->wineVersionLabel);
    ImVec2 width = ImGui::CalcTextSize(this->addAppLabel);
    if (width.x>this->leftColumnWidth.x) {
        this->leftColumnWidth = width;
    }
    this->leftColumnWidth.x+=COLUMN_PADDING;

    this->wineVersionComboboxData.data.clear();
    for (auto& ver : GlobalSettings::getWineVersions()) {
        this->wineVersionComboboxData.data.push_back(ver.name);
    }
    this->wineVersionComboboxData.dataChanged();
    this->wineVersionComboboxData.currentSelectedIndex=stringIndexInVector(container->getWineVersion(), this->wineVersionComboboxData.data, 0);

}

void ContainerOptionsDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->wineVersionLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    ImGui::PushItemWidth(100);
    //ImGui::PushItemWidth(-1-(this->wineVersionHelp?this->toolTipWidth:0));
    ImGui::Combo("##WineCombo", &this->wineVersionComboboxData.currentSelectedIndex, this->wineVersionComboboxData.dataForCombobox);
    ImGui::PopItemWidth();        
    if (this->wineVersionHelp) {
        ImGui::SameLine();
        this->toolTip(wineVersionHelp);
    }

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    
    ImGui::AlignTextToFramePadding();
    SAFE_IMGUI_TEXT(this->addAppLabel);
    ImGui::SameLine(this->leftColumnWidth.x);
    if (ImGui::Button(this->addAppButtonLabel)) {
        new AppChooserDlg(this->container, this);
    }            
    if (this->addAppHelp) {
        ImGui::SameLine();
        this->toolTip(this->addAppHelp);
    }

    this->addOkAndCancelButtons();
}

void ContainerOptionsDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->container->setWineVersion(this->wineVersionComboboxData.data[this->wineVersionComboboxData.currentSelectedIndex]);
        this->container->saveContainer();
        this->done();
    }
}