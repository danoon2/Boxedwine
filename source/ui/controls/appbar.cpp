#include "boxedwine.h"
#include "../boxedwineui.h"

void drawAppBar(const std::vector<AppButton>& buttons, int selected, ImFont* font, bool extendedFont) {
    if (font) {
        ImGui::PushFont(font);
    }
    ImGui::BeginGroup();
    AppButton* selectedButton = NULL;

    for (int i=0;i<(int)buttons.size();i++) {
        if (selected == i) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
        }
        if (ImGui::Button(buttons[i].name.c_str())) {
            if (font) {
                ImGui::PopFont();
            }
            buttons[i].onSelect();
            if (font) {
                ImGui::PushFont(font);
            }
        }
        if (selected == i) {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
    }    
    ImGui::EndGroup();
    if (font) {
        ImGui::PopFont();
    }
}
