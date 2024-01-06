#ifndef __LIST_VIEW_ITEM_H__
#define __LIST_VIEW_ITEM_H__

#include <functional>

class ListViewItem {
public:
    ListViewItem(BString text, const BoxedAppIcon* icon, std::function<void(bool right)> onSelect) : text(text), icon(icon), onSelect(onSelect) {}
    BString text;
    const BoxedAppIcon* icon;
    std::function<void(bool right)> onSelect;
};

void drawListViewItem(const ListViewItem& item);

#endif