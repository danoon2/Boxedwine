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