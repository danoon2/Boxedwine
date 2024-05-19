#include "boxedwine.h"
#include "../boxedwineui.h"

BaseView::BaseView(BString viewName) : tabIndex(0), viewName(viewName), errorMsgOpen(false), tabChanged(true) {
    this->toolTipWidth = (float)ImGui::CalcTextSize("(?)").x + ImGui::GetStyle().ItemSpacing.x;
    this->extraVerticalSpacing = (float)GlobalSettings::scaleIntUI(5);
}

void BaseView::addTab(const BaseViewTab& tab, int index) {
    ImGui::Dummy(ImVec2(this->extraVerticalSpacing, 0.0f));
    ImGui::SameLine();    
    ImVec2 pos = ImGui::GetCursorPos();
    ImVec2 s = ImGui::CalcTextSize(tab.name.c_str(), nullptr, true);
    s.x = 0;
    s.y += this->extraVerticalSpacing * 2;
    BString nameId = "##" + tab. name;
    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_WindowBg) | 0xFF000000);
    ImGui::PushID(tab.id.c_str());
    if (ImGui::Selectable(nameId.c_str(), tabIndex == index, ImGuiSelectableFlags_AllowRightClick, s)) {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            if (tab.onRightClick && this->saveChanges()) {
                tab.onRightClick();
            }
        } else {
            if (this->saveChanges()) {
                tabIndex = index;
                tabChanged = true;
            }
        }
    }
    ImGui::PopID();
    ImGui::PopStyleColor();
    pos.y += this->extraVerticalSpacing;
    ImGui::SetCursorPos(pos);
    if (tab.drawTabIcon) {
        tab.drawTabIcon();
    }
    SAFE_IMGUI_TEXT(tab.name.c_str());
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
        addTab(this->tabs[i], i);
    }
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::BeginChild(112, rightSize, false, 0);
    
    if (this->tabIndex < (int)this->tabs.size()) {
        this->tabs[this->tabIndex].drawTab(tabChanged, this->tabs[this->tabIndex]);
        if (this->tabs[this->tabIndex].model) {
            this->tabs[this->tabIndex].model->draw();
        }
    }
    this->tabChanged = false;

    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    if (!this->errorMsg.isEmpty()) {
        runErrorMsg(!errorMsgOpen);
    }
}

void BaseView::runErrorMsg(bool open) {
    if (open) {
        this->errorMsgOpen = true;
    }
    ImGui::PushFont(GlobalSettings::mediumFont);
    if (!showMessageBox(this->viewName + "ErrorMsg", open, c_getTranslation(Msg::GENERIC_DLG_ERROR_TITLE), this->errorMsg.c_str())) {
        this->errorMsgOpen = false;
        this->errorMsg = B("");
    }
    ImGui::PopFont();
}

void BaseView::addTab(BString id, BString name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab, std::function<void()> drawTabIcon, std::function<void()> onRightClick) {
    if (model) {
        model->doLayout();
    }
    this->tabs.push_back(BaseViewTab(id, name, model, drawTab, drawTabIcon, onRightClick));
}


std::shared_ptr<LayoutComboboxControl> BaseView::createWindowsVersionCombobox(const std::shared_ptr<LayoutSection>& section) {
    std::vector<ComboboxItem> windowsVersion;
    for (auto& win : BoxedwineData::getWinVersions()) {
        windowsVersion.push_back(ComboboxItem(win.szDescription));
    }
    std::shared_ptr<LayoutComboboxControl> result = section->addComboboxRow(Msg::CONTAINER_VIEW_WINDOWS_VERION_LABEL, Msg::CONTAINER_VIEW_WINDOWS_VERION_HELP, windowsVersion, BoxedwineData::getDefaultWindowsVersionIndex());
    result->setWidth((int)GlobalSettings::scaleFloatUIAndFont(150));
    return result;
}

std::shared_ptr<LayoutComboboxControl> BaseView::createFileSystemVersionCombobox(const std::shared_ptr<LayoutSection>& section) {
    std::vector<ComboboxItem> wineVersions;
    for (auto& ver : GlobalSettings::getFileSystemVersions()) {
        wineVersions.push_back(ComboboxItem(ver->name));
    }
    std::shared_ptr<LayoutComboboxControl> result = section->addComboboxRow(Msg::COMMON_FILESYSTEM_VERSION_LABEL, Msg::COMMON_FILESYSTEM_VERSION_HELP, wineVersions, 0);
    result->setWidth((int)GlobalSettings::scaleFloatUIAndFont(300));
    return result;
}

void BaseView::drawToolTip(BString help) {
    ImGui::AlignTextToFramePadding();
    if (GlobalSettings::hasIconsFont()) {
        SAFE_IMGUI_TEXT_DISABLED(QUESTION_ICON);
    } else {
        SAFE_IMGUI_TEXT_DISABLED("(?)");
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GlobalSettings::scaleFloatUI(8.0f), GlobalSettings::scaleFloatUI(8.0f)));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, GlobalSettings::scaleFloatUI(7.0f));
        ImGui::BeginTooltip();
        float width = ImGui::GetFontSize() * 35.0f;
        if (width > ImGui::GetIO().DisplaySize.x - GlobalSettings::scaleFloatUI(10.0f)) {
            width = ImGui::GetIO().DisplaySize.x - GlobalSettings::scaleFloatUI(10.0f);
        }
        ImGui::PushTextWrapPos(width);
        ImGui::TextUnformatted(help.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
        ImGui::PopStyleVar(2);
    }
}