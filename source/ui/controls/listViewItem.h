#ifndef __LIST_VIEW_ITEM_H__
#define __LIST_VIEW_ITEM_H__

#include <string>
#include <functional>

class ListViewItem {
public:
    ListViewItem(const std::string& text, void* icon, int iconWidth, int iconHeight, std::function<void(bool right)> onSelect) : text(text), icon(icon), iconWidth(iconWidth), iconHeight(iconHeight), onSelect(onSelect) {}
    std::string text;
    void* icon;
    int iconWidth;
    int iconHeight;
    std::function<void(bool right)> onSelect;
};

void drawListViewItem(const ListViewItem& item);

#endif