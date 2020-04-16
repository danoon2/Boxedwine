#include "boxedwine.h"
#include "../boxedwineui.h"
#include <thread>

AppChooserDlg::AppChooserDlg(BoxedContainer* container, std::function<void(BoxedApp*)> onSelected, BaseDlg* parent) : BaseDlg(APPCHOOSER_DLG_TITLE, 600, 400, NULL, parent), onSelected(onSelected) {
    container->getNewApps(this->items);
}

void AppChooserDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    if (this->items.size()==0) {
        SAFE_IMGUI_TEXT(getTranslation(APPCHOOSER_DLG_NO_APPS_LABEL));
    } else {
        SAFE_IMGUI_TEXT(getTranslation(APPCHOOSER_DLG_CHOOSE_APP_LABEL));
    }
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));

    ImGui::BeginChildFrame(1, ImVec2(-1, this->height-ImGui::GetCursorPosY()-42), ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(3);
    for (int i = 0; i < (int)this->items.size(); i++) {
        if (i<3) {
            ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        }
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::PushID(i);
        if (ImGui::Selectable("", false, 0, ImVec2(ImGui::GetColumnWidth(), 16.0f))) {
            this->items[i].saveApp();
            this->items[i].getContainer()->reload();
            GlobalSettings::reloadApps();
            BoxedApp* app = this->items[i].getContainer()->getAppByIniFile(this->items[i].getIniFilePath());
            if (app) {    
                std::function<void(BoxedApp*)> onSelected = this->onSelected;

                runOnMainUI([app, onSelected]() {
                    // don't hold on to this, it will be deleted before this runs
                    if (onSelected) {
                        onSelected(app);
                    } else {
                        new AppOptionsDlg(app);
                    }
                    return false;
                    });
            }
            this->done();
        }
        ImGui::PopID();
        ImGui::SetCursorPos(pos);
        const BoxedAppIcon* icon = this->items[i].getIconTexture(16);
        if (icon) {
            ImGui::Image(icon->texture, ImVec2(16.0f, 16.0f));
            ImGui::SameLine();            
        } else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+16.0f);
        }
        SAFE_IMGUI_TEXT(this->items[i].getName().c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::EndChildFrame();

    this->addCancelButton();
}

void AppChooserDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->done();            
    }
}