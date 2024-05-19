#include "boxedwine.h"
#include "../boxedwineui.h"
#include "knativesystem.h"

bool showMessageBox(BString id, bool open, const char* title, const char* msg) {
    bool result = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GlobalSettings::scaleFloatUI(8.0f), GlobalSettings::scaleFloatUI(8.0f)));
    ImGui::PushID(id.c_str());
    if (open) {
        ImGui::OpenPopup(title);
    }
    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        SAFE_IMGUI_TEXT(msg);
        ImGui::Separator();

        if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_OK), ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup(); 
            result = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PopStyleVar();
    return result;
}

bool showYesNoMessageBox(BString id, bool open, const char* title, const char* msg, bool* yes) {
    bool result = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GlobalSettings::scaleFloatUI(8.0f), GlobalSettings::scaleFloatUI(8.0f)));
    ImGui::PushID(id.c_str());
    if (open) {
        ImGui::OpenPopup(title);
    }
    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        SAFE_IMGUI_TEXT(msg);
        ImGui::Separator();

        if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_YES), ImVec2(GlobalSettings::scaleFloatUIAndFont(120.0f), 0))) {
            ImGui::CloseCurrentPopup();
            result = false;
            *yes = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_NO), ImVec2(GlobalSettings::scaleFloatUIAndFont(120.0f), 0))) {
            ImGui::CloseCurrentPopup();
            result = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PopStyleVar();
    return result;
}

BString getReadableSize(U64 bytes) {
    if (bytes < 4096) {
        return BString::valueOf(bytes)+"  B";
    }
    bytes /= 1024;
    if (bytes < 4096) {
        return BString::valueOf(bytes)+" KB";
    }
    bytes /= 1024;
    if (bytes < 4096) {
        return BString::valueOf(bytes)+" MB";
    }
    bytes /= 1024;
    return BString::valueOf(bytes)+" GB";
}

void alignNextTextRightInColumn(const char* text) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x  - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
}

void askToDownloadDefaultWine() {
    Msg labelId = Msg::ERROR_NO_WINE;
#ifdef BOXEDWINE_HIGHDPI
    U32 scale = KNativeSystem::getDpiScale();
    if (scale >= 1500 && !GlobalSettings::defaultFont) {
        labelId = Msg::ERROR_NO_WINE_HIGH_DPI;
    }
#endif
    new YesNoDlg(Msg::GENERIC_DLG_ERROR_TITLE, getTranslation(labelId), [](bool yes) {
        if (yes) {
            GlobalSettings::downloadFileSystem(GlobalSettings::getAvailableWineVersions().front(), [](bool success) {
                });
        } else {
            gotoView(VIEW_OPTIONS, B("Wine"));
        }
        });
}

#include "../../../lib/imgui/imgui_internal.h"
namespace ImGui {
    void PushItemFlag(ImGuiItemFlags option, bool enabled);
    void PopItemFlag();
}

UIDisableStyle::UIDisableStyle(bool disable) : disabled(disable) {
    if (disable) {
        ImGui::PushItemFlag(1 << 2, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
}
UIDisableStyle::~UIDisableStyle() {
    if (disabled) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
}