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

void drawAppBar(const std::vector<AppButton>& buttons, int selected, ImFont* font) {
    if (font) {
        ImGui::PushFont(font);
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
    ImGui::BeginGroup();   
    int selectedButton = -1;   
    float buttonHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y + GlobalSettings::scaleFloatUI(10.0f);
    for (int i=0;i<(int)buttons.size();i++) {
        if (selected == i) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
        }        
        BString name = " "+buttons[i].name + " ";
        if (ImGui::Button(name.c_str(), ImVec2(0.0, buttonHeight))) {
            selectedButton = i; // don't call onSelect here because the styles were changed
        }        
        ImGui::PopStyleColor();
        ImGui::SameLine();
    }    
    ImGui::EndGroup();
    ImGui::PopStyleVar(2);
    if (font) {
        ImGui::PopFont();
    }
    if (selectedButton>=0) {
        buttons[selectedButton].onSelect();
    }
}
