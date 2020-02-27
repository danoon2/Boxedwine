#include "boxedwine.h"
#include "../boxedwineui.h"
#include "menubar.h"

void uiShowAppMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About", "CTRL+A")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}