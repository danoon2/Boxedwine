#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/imgui_internal.h"

namespace ImGui {
void uiSpinner(const char* label, const float indicator_radius, const ImVec4& main_color, const ImVec4& backdrop_color, const int circle_count, const float speed) {
	ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = indicator_radius / 10.0f;
    const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f,
                                pos.y + indicator_radius * 2.0f));
    ItemSize(bb, ImGui::GetStyle().FramePadding.y);
    if (!ItemAdd(bb, id)) {
        return;
    }
    const float t = g.Time;
    const auto degree_offset = 2.0f * IM_PI / circle_count;
    for (int i = 0; i < circle_count; ++i) {
        const auto x = indicator_radius * std::sin(degree_offset * i);
        const auto y = indicator_radius * std::cos(degree_offset * i);
        const auto growth = std::max(0.0f, std::sin(t * speed - i * degree_offset));
        ImVec4 color;
        color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
        color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
        color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
        color.w = 1.0f;
        window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x,
                                                 pos.y + indicator_radius - y),
                                                 circle_radius + growth * circle_radius,
                                                 GetColorU32(color));
    }
}

float GetSpinnerWidth() {
    return GetCurrentContext()->Font->FontSize*2.0f;
}

void Spinner(const char* label)
{
	uiSpinner( label, GetCurrentContext()->Font->FontSize, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered), ImGui::GetStyleColorVec4(ImGuiCol_WindowBg), 12, 5.0f);
}
} // namespace ImGui