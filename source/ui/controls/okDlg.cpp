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