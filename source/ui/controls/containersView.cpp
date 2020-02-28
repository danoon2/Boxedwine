#include "boxedwine.h"
#include "../boxedwineui.h"

void drawContainersView(const ImVec2& size) {
    ImGui::BeginChild(1, size, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(3, "containercolumns");
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Wine Version"); ImGui::NextColumn();
    alignNextTextRightInColumn("Size");
    ImGui::Text("Size"); ImGui::NextColumn();    
    ImGui::Separator();
    ImGui::Columns(3);
    for (auto& item : BoxedwineData::getContainers()) {
        ImGui::Text(item->getName().c_str()); ImGui::NextColumn();
        ImGui::Text(item->getWineVersion().c_str()); ImGui::NextColumn();
        alignNextTextRightInColumn(item->getSize().c_str());
        ImGui::Text(item->getSize().c_str()); ImGui::NextColumn();        
    }    
    ImGui::Columns(1);
    ImGui::EndChild();
}