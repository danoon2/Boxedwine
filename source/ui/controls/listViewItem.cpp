#include "boxedwine.h"
#include "../boxedwineui.h"
#include "listViewItem.h"

void drawIcon(const ListViewItem& item) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(ImGui::GetColumnWidth()/2-(float)UiSettings::ICON_SIZE /2));
    ImGui::Image(item.icon->texture->getTexture(), ImVec2((float)UiSettings::ICON_SIZE, (float)UiSettings::ICON_SIZE));
}

const char* getTextThatFits(const char* p, float width) {
    const char* result = p + 1;
    while (ImGui::CalcTextSize(p, result, false, 9999.0f).x<width) {
        result++;
    }
    result--;
    return result;
}

void drawListViewItem(const ListViewItem& item) {
    float width = ImGui::GetColumnWidth()-GlobalSettings::scaleFloatUI(4.0f);
    ImVec2 startPos = ImGui::GetCursorPos();
    const float iconVertGap = GlobalSettings::scaleFloatUI(5);

    ImVec2 textSize = ImGui::CalcTextSize(item.text.c_str());
    if (textSize.x>width) {
        ImVec2 fullTextSize = ImGui::CalcTextSize(item.text.c_str(), nullptr, false, width);
        ImFont* font = ImGui::GetFont();
        const char* start = item.text.c_str();
        const char* end = start + item.text.length();
        const char* text = start;
        const char* p = font->CalcWordWrapPositionA(font->Scale, text, text+item.text.length(), width);

        ImGui::PushID(start);
        //ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(float)(width/2-fullTextSize.x/2));
        int lineCount = (int)(fullTextSize.y/textSize.y+0.5);
        if (lineCount>UiSettings::MAX_NUMBER_OF_LINES_FOR_APP_LIST_VIEW_TEXT) {
            lineCount = UiSettings::MAX_NUMBER_OF_LINES_FOR_APP_LIST_VIEW_TEXT;
        }
        fullTextSize.y=ImGui::GetStyle().ItemSpacing.y*(lineCount-1)+lineCount*textSize.y;
        ImVec2 fullItemSize = fullTextSize;         
        fullItemSize.x = width;
        fullItemSize.y += UiSettings::ICON_SIZE + iconVertGap * 3;
        if (ImGui::Selectable("", false, ImGuiSelectableFlags_AllowRightClick, fullItemSize)) {
            item.onSelect(ImGui::IsMouseReleased(ImGuiMouseButton_Right));
        }
        if (item.icon) {
            ImGui::SetCursorPosX(startPos.x);
            ImGui::SetCursorPosY(startPos.y+ iconVertGap);
            drawIcon(item);
        }
        ImGui::SetCursorPosY(startPos.y + (float)UiSettings::ICON_SIZE + iconVertGap * 2);
        ImGui::PopID();
        int i=0;
        while (p<end && i<UiSettings::MAX_NUMBER_OF_LINES_FOR_APP_LIST_VIEW_TEXT-1) {
            i++;
            BString line = BString::copy(text, (int)(p - text)).trim();
            textSize = ImGui::CalcTextSize(line.c_str());
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
            SAFE_IMGUI_TEXT(line.c_str());
            text = p;
            while(text[0]==' ') {
                text++;
            }
            p = font->CalcWordWrapPositionA(font->Scale, text, text+item.text.length(), width);
            if (p == text) { // didn't find a break;
                p = getTextThatFits(p, width);
            }
        }
        if (text<end) {
            BString line;
            if (p == end) {
                line = BString::copy(text);
            } else {
                float w = width - ImGui::CalcTextSize("...").x;
                line = BString::copy(text, (int)(getTextThatFits(text, w) - text));
                line = line + "...";
            }
            line = line.trim();
            textSize = ImGui::CalcTextSize(line.c_str());
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
            SAFE_IMGUI_TEXT(line.c_str());
        }
    } else {
        ImVec2 fullItemSize = textSize;
        fullItemSize.x = width;
        fullItemSize.y+=UiSettings::ICON_SIZE+ iconVertGap * 3;
        //ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
        if (ImGui::Selectable("", false, ImGuiSelectableFlags_AllowRightClick, fullItemSize)) {
            item.onSelect(ImGui::IsMouseReleased(ImGuiMouseButton_Right));
        }        
        if (item.icon) {
            ImGui::SetCursorPosX(startPos.x);
            ImGui::SetCursorPosY(startPos.y + iconVertGap);
            drawIcon(item);
        }
        ImGui::SetCursorPosY(startPos.y + (float)UiSettings::ICON_SIZE + iconVertGap*2);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
        SAFE_IMGUI_TEXT(item.text.c_str());
    }
}
