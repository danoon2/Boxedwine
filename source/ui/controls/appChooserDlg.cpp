#include "boxedwine.h"
#include "../boxedwineui.h"
#include <thread>

AppChooserDlg::AppChooserDlg(std::vector<BoxedApp>& items, std::vector<BoxedApp>& wineApps, std::function<void(BoxedApp)> onSelected, bool saveApp, BaseDlg* parent) : BaseDlg(Msg::APPCHOOSER_DLG_TITLE, 600, 400, nullptr, parent), items(items), wineApps(wineApps), onSelected(onSelected), labelId(Msg::APPCHOOSER_DLG_CHOOSE_APP_LABEL), saveApp(saveApp) {
}

AppChooserDlg::AppChooserDlg(std::vector<BoxedApp>& items, std::function<void(BoxedApp)> onSelected, bool saveApp, BaseDlg* parent, Msg titleId) : BaseDlg(titleId, 600, 400, nullptr, parent), items(items), onSelected(onSelected), labelId(Msg::APPCHOOSER_DLG_CHOOSE_APP_LABEL), saveApp(saveApp) {
}

void AppChooserDlg::drawItems(std::vector<BoxedApp>& apps, int startingIndex) {
    for (int i = 0; i < (int)apps.size(); i++) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::PushID(i+startingIndex);
        if (ImGui::Selectable("", false, 0, ImVec2(ImGui::GetColumnWidth(), ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y))) {
            if (this->saveApp) {
                apps[i].saveApp();
                apps[i].getContainer()->reload();
                GlobalSettings::reloadApps();
            }
            std::function<void(BoxedApp)> onSelected = this->onSelected;
            BoxedApp app = apps[i];

            runOnMainUI([app, onSelected]() {
                // don't hold on to this, it will be deleted before this runs
                if (onSelected) {
                    onSelected(app);
                }
                return false;
                });
            this->done();
        }
        ImGui::PopID();
        ImGui::SetCursorPos(pos);
        const BoxedAppIcon* icon = apps[i].getIconTexture(16);
        if (icon) {
            ImGui::Image(icon->texture->getTexture(), ImVec2(GlobalSettings::scaleFloatUI(16.0f), GlobalSettings::scaleFloatUI(16.0f)));
            ImGui::SameLine();
        } else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + GlobalSettings::scaleFloatUI(16.0f) + ImGui::GetStyle().ItemSpacing.x);
        }
        SAFE_IMGUI_TEXT(apps[i].getName().c_str());
        ImGui::NextColumn();
    }    
}

void AppChooserDlg::run() {
    ImVec2 pos = ImGui::GetCursorPos();
    pos.y += this->extraVerticalSpacing*2;
    pos.x += getOuterFramePadding();
    ImGui::SetCursorPos(pos);
    if (this->items.size()==0) {
        SAFE_IMGUI_TEXT(c_getTranslation(Msg::APPCHOOSER_DLG_NO_APPS_LABEL));
    } else {
        SAFE_IMGUI_TEXT(c_getTranslation(this->labelId));
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    pos = ImGui::GetCursorPos();
    pos.x += getOuterFramePadding();
    ImGui::SetCursorPos(pos);
    ImGui::BeginChildFrame(1, ImVec2(-1 - getOuterFramePadding(), this->height - ImGui::GetCursorPosY() - getReservedHeightForButtons()), 0);
    
    ImGui::Columns(3);
    drawItems(this->items, 0);
    ImGui::Columns(1);
    if (this->wineApps.size()) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing * 4));
        SAFE_IMGUI_TEXT(c_getTranslation(Msg::APPCHOOSER_DLG_WINE_APPS_LABEL));
        ImGui::Separator();
        ImGui::Columns(3);
        drawItems(this->wineApps, (int)this->items.size());
        ImGui::Columns(1);
    }
    ImGui::EndChildFrame();
    
    
    this->addCancelButton();
}

void AppChooserDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->done();            
    }
}
