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
        std::string name = " "+buttons[i].name + " ";
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
