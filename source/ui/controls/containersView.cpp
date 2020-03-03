#include "boxedwine.h"
#include "../boxedwineui.h"

void drawContainersView(const ImVec2& size) {
    ImGui::BeginChild(1, size, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(3, "containercolumns");
    SAFE_IMGUI_TEXT("Name"); ImGui::NextColumn();
    SAFE_IMGUI_TEXT("Wine Version"); ImGui::NextColumn();
    alignNextTextRightInColumn("Size");
    SAFE_IMGUI_TEXT("Size"); ImGui::NextColumn();    
    ImGui::Separator();
    ImGui::Columns(3);
    BoxedContainer* containerToOpen = NULL;
    for (auto& item : BoxedwineData::getContainers()) {
        // item->getName() is guaranteed to be unique
        if (ImGui::Selectable(item->getName().c_str())) {
            containerToOpen = item;
        }
        ImGui::NextColumn();
        SAFE_IMGUI_TEXT(item->getWineVersion().c_str()); ImGui::NextColumn();
        alignNextTextRightInColumn(item->getSize().c_str());
        SAFE_IMGUI_TEXT(item->getSize().c_str()); ImGui::NextColumn();        
    }    
    ImGui::Columns(1);
    ImGui::EndChild();
    if (containerToOpen) {
        new ContainerOptionsDlg(containerToOpen);
    }
}