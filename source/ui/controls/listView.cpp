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

void drawListView(BString listViewId, const std::vector<ListViewItem>& items, const ImVec2& size) {
    ImGui::SetCursorPosX(0);    
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_WindowBg) | 0xFF000000);
    ImGui::BeginChild(1, size, false, 0);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + GlobalSettings::scaleFloatUI(5.0f));
    int maxImageWidth = GlobalSettings::scaleIntUI(125);
    for (auto& item : items) {
        if (item.icon && item.icon->getWidth() > maxImageWidth) {
            maxImageWidth = item.icon->getWidth();
        }
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::Columns((int)(ImGui::GetWindowWidth()/maxImageWidth), nullptr, false);
    for (auto& item : items) {
        ImGui::Dummy(ImVec2(0.0f, GlobalSettings::scaleFloatUI(10.0f)));
        ImGui::PushID(&item);
        drawListViewItem(item);
        ImGui::PopID();
        ImGui::NextColumn();        
    }    
    ImGui::Columns(1);
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}