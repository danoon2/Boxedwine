#include "boxedwine.h"
#include "../boxedwineui.h"

bool showMessageBox(bool open, const char* title, const char* msg) {    
    bool result = true;
    ImGui::PushID(msg);
    if (open) {
        ImGui::OpenPopup(title);
    }
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        SAFE_IMGUI_TEXT(msg);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { 
            ImGui::CloseCurrentPopup(); 
            result = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    return result;
}

std::string getReadableSize(U64 bytes) {
    if (bytes < 4096) {
        return std::to_string(bytes)+"  B";
    }
    bytes /= 1024;
    if (bytes < 4096) {
        return std::to_string(bytes)+" KB";
    }
    bytes /= 1024;
    if (bytes < 4096) {
        return std::to_string(bytes)+" MB";
    }
    bytes /= 1024;
    return std::to_string(bytes)+" GB";
}

void alignNextTextRightInColumn(const char* text) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x  - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
}

