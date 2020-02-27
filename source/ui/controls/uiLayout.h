#ifndef __UI_LAYOUT_H__
#define __UI_LAYOUT_H__

#include <string>
#include <vector>
#include <functional>

enum UiLayoutFieldType {
    UiText,
    UiButton,
    UiCombobox,
    UiFileLocation,
    UiDirectoryLocation,
    UiCheckbox
};

enum UiLayoutHorizontalAlignment {
    UiLeft,
    UiCenter,
    UiRight,
};

class UiLayoutCell {
public:
    std::string label;
    std::string value;
    std::string comboOptions;
    int widthOverride;
    UiLayoutFieldType fieldType;

    std::function<void()> onChanged;
};

class UiLayoutRow {
public:
    std::vector<UiLayoutCell> cells;
};

class UiLayout  {
public:
    std::vector<UiLayoutRow> rows;
    std::string title;
};
#endif