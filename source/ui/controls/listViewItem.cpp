#include "boxedwine.h"
#include "../boxedwineui.h"
#include "listViewItem.h"

void drawIcon(const ListViewItem& item) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(ImGui::GetColumnWidth()/2-(float)item.iconWidth/2));
    if (item.iconHeight==UiSettings::ICON_SIZE)  {
        ImGui::Image(item.icon, ImVec2((float)item.iconWidth, (float)item.iconHeight));
    } else {
        int offset = UiSettings::ICON_SIZE/2-item.iconWidth/2;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(float)offset);
        ImGui::Image(item.icon, ImVec2((float)item.iconWidth, (float)item.iconHeight));
        offset+=item.iconHeight;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(float)(UiSettings::ICON_SIZE-offset));
    }
}

void drawListViewItem(const ListViewItem& item) {
    float width = ImGui::GetColumnWidth();
    ImVec2 startPos = ImGui::GetCursorPos();
    
    ImVec2 textSize = ImGui::CalcTextSize(item.text.c_str());
    if (textSize.x>width) {
        ImVec2 fullTextSize = ImGui::CalcTextSize(item.text.c_str(), NULL, false, width);
        ImFont* font = ImGui::GetFont();
        const char* start = item.text.c_str();
        const char* end = start + item.text.length();
        const char* text = start;
        const char* p = font->CalcWordWrapPositionA(font->Scale, text, text+item.text.length(), width);

        ImGui::PushID(start);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(float)(width/2-fullTextSize.x/2));
        int lineCount = (int)(fullTextSize.y/textSize.y+0.5);
        if (lineCount>UiSettings::MAX_NUMBER_OF_LINES_FOR_APP_LIST_VIEW_TEXT) {
            lineCount = UiSettings::MAX_NUMBER_OF_LINES_FOR_APP_LIST_VIEW_TEXT;
        }
        fullTextSize.y=ImGui::GetStyle().ItemSpacing.y*(lineCount-1)+lineCount*textSize.y;
        ImVec2 fullItemSize = fullTextSize;
        if (item.icon) {            
            fullItemSize.y+=UiSettings::ICON_SIZE+ImGui::GetStyle().ItemSpacing.y;
        }
        ImGui::Selectable("", false, 0, fullItemSize);
        if (item.icon) {
            ImGui::SetCursorPos(startPos);
            drawIcon(item);
        } else {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(float)UiSettings::ICON_SIZE+ImGui::GetStyle().ItemSpacing.y);
        }   
        ImGui::PopID();
        int i=0;
        while (p<end && i<UiSettings::MAX_NUMBER_OF_LINES_FOR_APP_LIST_VIEW_TEXT) {
            i++;
            std::string line = std::string(text, p);
            stringTrim(line);
            textSize = ImGui::CalcTextSize(line.c_str());
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
            SAFE_IMGUI_TEXT(line.c_str());
            text = p;
            p = font->CalcWordWrapPositionA(font->Scale, text, text+item.text.length(), width);
        }
        if (text<end) {
            std::string line;
            if (p==end) {
                line = std::string(text);
            } else {
                line = std::string(text, p-3);
                line+="...";
            }
            stringTrim(line);
            textSize = ImGui::CalcTextSize(line.c_str());
            ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
            SAFE_IMGUI_TEXT(line.c_str());
        }
    } else {
        ImVec2 fullItemSize = textSize;
        if (item.icon) {            
            fullItemSize.y+=UiSettings::ICON_SIZE+ImGui::GetStyle().ItemSpacing.y;
        }
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
        ImGui::Selectable("", false, 0, fullItemSize);
        if (item.icon) {
            ImGui::SetCursorPos(startPos);
            drawIcon(item);
        } else {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(float)UiSettings::ICON_SIZE+ImGui::GetStyle().ItemSpacing.y);
        }       
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+(width/2-textSize.x/2));
        SAFE_IMGUI_TEXT(item.text.c_str());
    }
}