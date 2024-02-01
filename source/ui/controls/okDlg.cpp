#include "boxedwine.h"
#include "../boxedwineui.h"

OkDlg::OkDlg(Msg title, BString label, std::function<void()> onDone, int width, int height) : BaseDlg(title, width, height, GlobalSettings::mediumFont), label(label), onDone(onDone) {
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
    if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_OK), ImVec2(GlobalSettings::scaleFloatUIAndFont(120.0f), 0))) {
        if (this->onDone) {
            this->onDone();
        }
        this->done();
    }
}