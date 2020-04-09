#include "boxedwine.h"
#include "../boxedwineui.h"

WaitDlg::WaitDlg(int title, const std::string& label) : BaseDlg(title, 400, 150), label(label) {
}

WaitDlg::WaitDlg(int title, const std::string& label, std::function<bool()> checkIfShouldContinue) : BaseDlg(title, 400, 150), label(label), checkIfShouldContinue(checkIfShouldContinue) {
}

void WaitDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width/2-ImGui::GetSpinnerWidth()/2);
    ImGui::Spinner("1");
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width/2-ImGui::CalcTextSize(this->label.c_str()).x/2);
    SAFE_IMGUI_TEXT(this->label.c_str());
    if (checkIfShouldContinue && !checkIfShouldContinue()) {
        this->done();
    }
}