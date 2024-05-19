#include "boxedwine.h"
#include "../boxedwineui.h"

WaitDlg::WaitDlg(Msg title, BString label) : BaseDlg(title, 400, 150), label(label) {
    GlobalSettings::useFastFrameRate(true);
}

WaitDlg::WaitDlg(Msg title, BString label, std::function<bool()> checkIfShouldContinue) : BaseDlg(title, 400, 150), label(label), checkIfShouldContinue(checkIfShouldContinue) {
    GlobalSettings::useFastFrameRate(true);
}

WaitDlg::~WaitDlg() {
    GlobalSettings::useFastFrameRate(false);
    if (this->onDone) {
        this->onDone();
    }
}

void WaitDlg::run() {
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width/2-ImGui::GetSpinnerWidth()/2);
    ImGui::Spinner("1");
    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    ImGui::SetCursorPosX(this->width/2-ImGui::CalcTextSize(this->label.c_str()).x/2);
    SAFE_IMGUI_TEXT(this->label.c_str());
    if (this->subLabels.size() > 0) {
        ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
        ImGui::Separator();
        for (auto& line : subLabels) {
            ImGui::SetCursorPosX((float)GlobalSettings::scaleIntUI(5));
            SAFE_IMGUI_TEXT(line.c_str());
        }
    }
    if (checkIfShouldContinue && !checkIfShouldContinue()) {
        this->done();
    }
}

void WaitDlg::addSubLabel(BString subLabel, int max) {
    while ((int)subLabels.size() > max) {
        subLabels.pop_front();
    }
    this->subLabels.push_back(subLabel);
    if (this->subLabels.size() > 2) {
        this->height = GlobalSettings::scaleIntUI(150) + (int)((this->subLabels.size() - 1) * ImGui::GetFontSize());
    } else {
        this->height = GlobalSettings::scaleIntUI(150);
    }
}