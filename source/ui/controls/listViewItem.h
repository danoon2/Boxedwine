#ifndef __LIST_VIEW_ITEM_H__
#define __LIST_VIEW_ITEM_H__

#include <string>
#include <functional>

class ListViewItem {
public:
    ListViewItem(const std::string& text, const BoxedAppIcon* icon, std::function<void(bool right)> onSelect) : text(text), icon(icon), onSelect(onSelect) {}
    std::string text;
    const BoxedAppIcon* icon;
    std::function<void(bool right)> onSelect;
};

void drawListViewItem(const ListViewItem& item);

#endif