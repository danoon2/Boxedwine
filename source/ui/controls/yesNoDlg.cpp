#include "boxedwine.h"
#include "../boxedwineui.h"

YesNoDlg::YesNoDlg(Msg title, BString label, std::function<void(bool)> onCompleted) : BaseDlg(title, 400, 150, GlobalSettings::mediumFont), label(label), onCompleted(onCompleted) {
}

void YesNoDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));    
    ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));
    ImGui::SameLine();
    ImGui::PushTextWrapPos(this->width - this->extraVerticalSpacing*2);
    SAFE_IMGUI_TEXT(this->label.c_str());
    ImGui::PopTextWrapPos();

    
    ImGui::SetCursorPosY(this->height - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y);
    ImGui::Separator();    
    ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.y);
    if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_YES), ImVec2(GlobalSettings::scaleFloatUIAndFont(120.0f), 0))) {
        this->onCompleted(true);
        this->done();
    }
    ImGui::SameLine();
    if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_NO), ImVec2(GlobalSettings::scaleFloatUIAndFont(120.0f), 0))) {
        this->onCompleted(false);
        this->done();
    }
}