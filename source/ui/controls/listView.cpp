#include "boxedwine.h"
#include "../boxedwineui.h"

void drawListView(const std::string& listViewId, const std::vector<ListViewItem>& items, const ImVec2& size) {
    ImGui::BeginChild(1, size, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    int maxImageWidth = 125;
    for (auto& item : items) {
        if (item.iconWidth>maxImageWidth) {
            maxImageWidth = item.iconWidth;
        }
    }
    ImGui::Columns((int)(ImGui::GetWindowHeight()/maxImageWidth), NULL, false);
    for (auto& item : items) {
        ImGui::Dummy(ImVec2(0.0f, 10));
        drawListViewItem(item);
        ImGui::NextColumn();        
    }    
    ImGui::Columns(1);
    ImGui::EndChild();
}