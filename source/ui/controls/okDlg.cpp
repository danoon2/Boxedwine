#include "boxedwine.h"
#include "../boxedwineui.h"

OkDlg::OkDlg(int title, const std::string& label, std::function<void()> onDone) : BaseDlg(title, 400, 150, GlobalSettings::mediumFont), label(label), onDone(onDone) {
}

void OkDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));
    ImGui::SameLine();
    ImGui::PushTextWrapPos(this->width - this->extraVerticalSpacing * 2);
    SAFE_IMGUI_TEXT(this->label.c_str());
    ImGui::PopTextWrapPos();


    ImGui::SetCursorPosY(this->height - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y);
    ImGui::Separator();
    ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.y);
    if (ImGui::Button(getTranslation(GENERIC_DLG_OK), ImVec2(GlobalSettings::scaleFloatUI(120.0f), 0))) {
        this->onDone();
        this->done();
    }
}