#include "boxedwine.h"
#include "../boxedwineui.h"

#include "baseDlg.h"

std::list<BaseDlg*> BaseDlg::activeDialogs;

void BaseDlg::runDialogs() {
    BaseDlg* toDelete = NULL;
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
    int len = 1;
    
    if (dataForCombobox) {
        delete[] dataForCombobox;
    }
    for (auto& s : data) {
        len+=((int)s.length()+1);
    }
    dataForCombobox = new char[len];
    len = 0;
    for (auto& s : data) {
        strcpy(dataForCombobox+len, s.c_str());
        len+=(int)s.length()+1;
    }
    dataForCombobox[len]=0;
}

BaseDlg::BaseDlg(int title, int width, int height, BaseDlg* parent) : width(width), height(height), toolTipWidth(28), extraVerticalSpacing(5), isDone(false), parent(parent), child(NULL) {
    this->title = getTranslation(title);
    ImGui::OpenPopup(this->title);
    if (parent) {
        parent->child = this;
    } else {
        BaseDlg::activeDialogs.push_back(this);
    }
}

void BaseDlg::runIfVisible() {
    if (ImGui::BeginPopupModal(this->title, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse/* | ImGuiWindowFlags_NoMove*/))
    {
        ImGui::SetWindowSize(ImVec2((float)width, (float)height));        
        this->run();
        if (this->child) {
            this->child->runIfVisible();
        }
        ImGui::EndPopup();
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
    float buttonArea = ImGui::CalcTextSize(getTranslation(GENERIC_DLG_OK)).x+ImGui::CalcTextSize(getTranslation(GENERIC_DLG_CANCEL)).x;
    ImGui::SetCursorPos(ImVec2((float)(this->width-buttonArea-35),(float)(this->height-32)));

    this->onOk(ImGui::Button(getTranslation(GENERIC_DLG_OK)));
    ImGui::SameLine();
    if (ImGui::Button(getTranslation(GENERIC_DLG_CANCEL))) {
        this->done();
    }
}

void BaseDlg::addCancelButton() {
    float buttonArea = ImGui::CalcTextSize(getTranslation(GENERIC_DLG_CANCEL)).x;
    ImGui::SetCursorPos(ImVec2((float)(this->width-buttonArea-18),(float)(this->height-32)));

    if (ImGui::Button(getTranslation(GENERIC_DLG_CANCEL))) {
        this->done();
    }
}

void BaseDlg::onOk(bool buttonClicked) {
    if (buttonClicked) {
        this->done();
    }
}
