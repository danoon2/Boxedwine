#include "boxedwine.h"
#include "../boxedwineui.h"

BaseView::BaseView(const std::string& viewName) : errorMsg(NULL), tabIndex(0), viewName(viewName), errorMsgOpen(false), tabChanged(true) {
    this->toolTipWidth = (float)ImGui::CalcTextSize("(?)").x + ImGui::GetStyle().ItemSpacing.x;
    this->extraVerticalSpacing = (float)GlobalSettings::scaleIntUI(5);
}

void BaseView::addTab(const std::string& name, int index, const std::function<void()>& drawTabIcon) {
    ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));
    ImGui::SameLine();    
    ImVec2 pos = ImGui::GetCursorPos();
    ImVec2 s = ImGui::CalcTextSize(name.c_str(), NULL, true);
    s.x = 0;
    s.y += this->extraVerticalSpacing * 2;
    std::string nameId = "##" + name;
    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_WindowBg) | 0xFF000000);
    if (ImGui::Selectable(nameId.c_str(), tabIndex == index, ImGuiSelectableFlags_AllowRightClick, s)) {
        if (this->saveChanges()) {
            tabIndex = index;
            tabChanged = true;
        }
    }
    ImGui::PopStyleColor();
    pos.y += this->extraVerticalSpacing;
    ImGui::SetCursorPos(pos);
    if (drawTabIcon) {
        drawTabIcon();
    }
    SAFE_IMGUI_TEXT(name.c_str());
    pos = ImGui::GetCursorPos();
    pos.y += this->extraVerticalSpacing;
    ImGui::SetCursorPos(pos);
}

void BaseView::run(const ImVec2& size) {
    ImVec2 leftSize = ImGui::GetContentRegionAvail();
    ImVec2 rightSize = leftSize;
    leftSize.x = leftSize.x / 4 - 2 * ImGui::GetStyle().FramePadding.x;
    leftSize.y = 0;
    rightSize.x = rightSize.x * 3 / 4 - 2 * ImGui::GetStyle().FramePadding.x;
    rightSize.y = 0;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_WindowBg) | 0xFF000000);
    ImGui::BeginChild(110, size, false, 0);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_TabUnfocusedActive) | 0xFF000000);
    ImGui::BeginChild(111, leftSize, false, 0);
    ImGui::PushFont(GlobalSettings::mediumFont);

    ImGui::Dummy(ImVec2(0.0f, this->extraVerticalSpacing));
    for (int i=0;i<(int)this->tabs.size();i++) {
        addTab(this->tabs[i].name, i, this->tabs[i].drawTabIcon);
    }
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::BeginChild(112, rightSize, false, 0);
    
    this->tabs[this->tabIndex].drawTab(tabChanged, this->tabs[this->tabIndex]);
    if (this->tabs[this->tabIndex].model) {
        this->tabs[this->tabIndex].model->draw();
    }    
    this->tabChanged = false;

    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    if (this->errorMsg) {
        runErrorMsg(!errorMsgOpen);
    }
}

void BaseView::runErrorMsg(bool open) {
    if (open) {
        this->errorMsgOpen = true;
    }
    ImGui::PushFont(GlobalSettings::mediumFont);
    if (!showMessageBox(this->viewName + "ErrorMsg", open, getTranslation(GENERIC_DLG_ERROR_TITLE), this->errorMsg)) {
        this->errorMsgOpen = false;
        this->errorMsg = NULL;
    }
    ImGui::PopFont();
}

void BaseView::addTab(const std::string& name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab, std::function<void()> drawTabIcon) {
    if (model) {
        model->doLayout();
    }
    this->tabs.push_back(BaseViewTab(name, model, drawTab, drawTabIcon));
}


std::shared_ptr<LayoutComboboxControl> BaseView::createWindowsVersionCombobox(const std::shared_ptr<LayoutSection>& section) {
    std::vector<ComboboxItem> windowsVersion;
    for (auto& win : BoxedwineData::getWinVersions()) {
        windowsVersion.push_back(ComboboxItem(win.szDescription));
    }
    std::shared_ptr<LayoutComboboxControl> result = section->addComboboxRow(CONTAINER_VIEW_WINDOWS_VERION_LABEL, CONTAINER_VIEW_WINDOWS_VERION_HELP, windowsVersion, BoxedwineData::getDefaultWindowsVersionIndex());
    result->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    return result;
}

std::shared_ptr<LayoutComboboxControl> BaseView::createWineVersionCombobox(const std::shared_ptr<LayoutSection>& section) {
    std::vector<ComboboxItem> wineVersions;
    for (auto& ver : GlobalSettings::getWineVersions()) {
        wineVersions.push_back(ComboboxItem(ver.name));
    }
    std::shared_ptr<LayoutComboboxControl> result = section->addComboboxRow(COMMON_WINE_VERSION_LABEL, COMMON_WINE_VERSION_HELP, wineVersions, 0);
    result->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    return result;
}