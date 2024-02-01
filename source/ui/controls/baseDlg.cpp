#include "boxedwine.h"
#include "../boxedwineui.h"

#include "baseDlg.h"

std::list<BaseDlg*> BaseDlg::activeDialogs;

void BaseDlg::runDialogs() {
    BaseDlg* toDelete = nullptr;
    for (auto& dlg : BaseDlg::activeDialogs) {
        if (!dlg->isDone) {
            dlg->runIfVisible();
        }
        if (dlg->isDone) {
            toDelete = dlg;
        }
    }
    if (toDelete) {
        BaseDlg::activeDialogs.remove(toDelete);
        delete toDelete;        
    }
}

void BaseDlg::stopAllDialogs() {
    while (BaseDlg::activeDialogs.size()) {
        BaseDlg* dlg = BaseDlg::activeDialogs.back();
        BaseDlg::activeDialogs.pop_back();
        delete dlg;        
    }
}

void ComboboxData::dataChanged() {
    dataForCombobox.clear();
    for (auto& s : data) {
        if (dataForCombobox.length() == 0) {
            dataForCombobox.append(s.label);
        } else {
            dataForCombobox.appendAfterNull(s.label);
        }
    }
    dataForCombobox.appendAfterNull("");
}

BaseDlg::BaseDlg(Msg title, int width, int height, ImFont* font, BaseDlg* parent) : isDone(false), child(nullptr), font(font) {
    this->width = GlobalSettings::scaleIntUI(width);
    this->height = GlobalSettings::scaleIntUI(height);
    this->toolTipWidth = (float)GlobalSettings::scaleIntUI(28);
    this->extraVerticalSpacing = (float)GlobalSettings::scaleIntUI(5);
    this->title = c_getTranslation(title);
    ImGui::OpenPopup(this->title);
    if (parent) {
        parent->child = this;
    } else {
        BaseDlg::activeDialogs.push_back(this);
    }
    if (!this->font) {
        this->font = GlobalSettings::defaultFont;
    }
}

void BaseDlg::runIfVisible() {
    if (this->font) {
        ImGui::PushFont(this->font);
    }
    if (ImGui::BeginPopupModal(this->title, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse/* | ImGuiWindowFlags_NoMove*/))
    {
        ImGui::SetWindowSize(ImVec2((float)width, (float)height));        
        this->run();
        if (this->child) {
            this->child->runIfVisible();
        }
        ImGui::EndPopup();
    }
    if (this->font) {
        ImGui::PopFont();
    }
}

void BaseDlg::done() {
    ImGui::CloseCurrentPopup();
    this->isDone = true;
}

void BaseDlg::toolTip(const char* desc) {
    SAFE_IMGUI_TEXT_DISABLED("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void BaseDlg::addOkAndCancelButtons() {
    float buttonArea = ImGui::CalcTextSize(c_getTranslation(Msg::GENERIC_DLG_OK)).x+ImGui::CalcTextSize(c_getTranslation(Msg::GENERIC_DLG_CANCEL)).x;
    ImGui::SetCursorPos(ImVec2((float)(this->width-buttonArea-35),(float)(this->height-32)));

    this->onOk(ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_OK)));
    ImGui::SameLine();
    if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_CANCEL))) {
        this->done();
    }
}

float BaseDlg::getReservedHeightForButtons() {
    return ImGui::GetStyle().ItemSpacing.y * 4 + ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetTextLineHeight();
}

float BaseDlg::getButtonRowY() {
    return this->height - (ImGui::GetStyle().ItemSpacing.y * 2 + ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetTextLineHeight());
}

float BaseDlg::getOuterFramePadding() {
    return ImGui::GetStyle().ItemSpacing.x;
}

void BaseDlg::addCancelButton() {
    float buttonArea = ImGui::CalcTextSize(c_getTranslation(Msg::GENERIC_DLG_CANCEL)).x+ImGui::GetStyle().FramePadding.x*2;
    ImGui::SetCursorPos(ImVec2((float)(this->width - buttonArea - getOuterFramePadding()), getButtonRowY()));

    if (ImGui::Button(c_getTranslation(Msg::GENERIC_DLG_CANCEL))) {
        this->done();
    }
}

void BaseDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->done();
    }
}
