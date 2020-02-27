#include "boxedwine.h"
#include "../boxedwineui.h"

void drawListView(const std::string& listViewId, const std::vector<ListViewItem>& items, const ImVec2& size) {
    ImGui::BeginChild(1, size, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(3);
    for (auto& item : items) {
        drawListViewItem(item);
        ImGui::NextColumn();
    }    
    ImGui::Columns(1);
    ImGui::EndChild();
}