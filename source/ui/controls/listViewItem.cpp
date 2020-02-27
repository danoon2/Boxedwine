#include "boxedwine.h"
#include "../boxedwineui.h"
#include "listViewItem.h"

void drawListViewItem(const ListViewItem& item) {
    if (item.icon) {
        ImGui::Image(item.icon, ImVec2((float)item.iconWidth, (float)item.iconHeight));
    }
    ImGui::Selectable(item.text.c_str());
}