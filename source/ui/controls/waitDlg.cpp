/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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