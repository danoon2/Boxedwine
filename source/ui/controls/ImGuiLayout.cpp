#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../../lib/imgui/addon/imguitinyfiledialogs.h"

void ImGuiLayout::draw() {
	if (toolTipWidth < 1.0f) {
		toolTipWidth = (float)ImGui::CalcTextSize("(?)").x + ImGui::GetStyle().ItemSpacing.x;
	}
	for (auto& section : this->sections) {
		if (!section->isHidden()) {
			section->draw(this->toolTipWidth, this->leftColumnWidth);
		}
	}
}

void LayoutSection::draw(float toolTipWidth, float offset) {
	if (this->title.length()) {
		ImGui::PushFont(GlobalSettings::largeFont);
		ImGui::Dummy(ImVec2(0.0f, GlobalSettings::extraVerticalSpacing));
		SAFE_IMGUI_TEXT(this->title.c_str());
		ImGui::PopFont();
	}
	float labelOffset = 0.0f;
	float valueOffset = offset;
	if (this->indent) {
		labelOffset += offset;
		valueOffset += this->leftColumnWidth;
	}
	for (auto& row : this->rows) {		
		if (!row->isHidden()) {
			row->draw(toolTipWidth, labelOffset, valueOffset);
		}
	}
}

class FontHelper {
public:
	FontHelper(ImFont* font) : font(font) {
		if (font) {
			ImGui::PushFont(font);
		}
	}
	~FontHelper() {
		if (font) {
			ImGui::PopFont();
		}
	}
	ImFont* font;
};

void LayoutRow::draw(float toolTipWidth, float labelOffset, float valueOffset) {	
	ImGui::Dummy(ImVec2(0.0f, GlobalSettings::extraVerticalSpacing));
	if (this->label.length()) {
		ImGui::AlignTextToFramePadding();
		if (labelOffset > 1.0f) {
			SAFE_IMGUI_TEXT("");
			ImGui::SameLine(labelOffset);
		}	
		SAFE_IMGUI_TEXT(this->label.c_str());
		ImGui::SameLine(valueOffset);
	}
	if (controls.size()) {
		auto end = controls.end() - 1;
		for (auto iter = controls.begin(); iter != end; ++iter) {
			FontHelper f((*iter)->getFont());
			(*iter)->draw(-1);
		}
		FontHelper f(controls.back()->getFont());
		controls.back()->draw(-1 - (this->help.length() ? (int)toolTipWidth :0));
	}
	if (this->help.length()) {
		ImGui::SameLine();
		this->drawToolTip(this->help);
	}
}

void LayoutRow::drawToolTip(const std::string& help) {
	ImGui::AlignTextToFramePadding();
	SAFE_IMGUI_TEXT_DISABLED("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GlobalSettings::scaleFloatUI(8.0f), GlobalSettings::scaleFloatUI(8.0f)));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, GlobalSettings::scaleFloatUI(7.0f));
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(help.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
		ImGui::PopStyleVar(2);
	}
}

void LayoutComboboxControl::draw(int width) {
	UIDisableStyle disabled(this->isReadOnly());
	if (this->getWidth()) {
		ImGui::PushItemWidth((float)this->getWidth());
	} else {
		ImGui::PushItemWidth((float)width);
	}
	ImGui::PushID(this);
	if (ImGui::Combo("##Combo", &this->options.currentSelectedIndex, this->options.dataForCombobox) && this->onChange) {
		this->onChange();
	}
	ImGui::PopID();
	ImGui::PopItemWidth();
}

void LayoutTextInputControl::draw(int width) {
	bool browseButton = this->browseButtonType != BROWSE_BUTTON_NONE;

	if (browseButton && !this->browseButtonLabel) {
		this->browseButtonLabel = getTranslation(GENERIC_BROWSE_BUTTON);
		this->browseButtonWidth = ImGui::CalcTextSize(this->browseButtonLabel).x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
	}
	if (this->isReadOnly()) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_TextDisabled));
	}
	if (this->getWidth()) {
		width = this->getWidth();
	} else if (browseButton) {
		width -= (int)this->browseButtonWidth;
	}
	ImGui::PushItemWidth((float)width);
	ImGui::PushID(this);
	if (ImGui::InputText("##Text", this->text, sizeof(this->text), (this->isReadOnly()?ImGuiInputTextFlags_ReadOnly:0)) && this->onChange) {
		this->onChange();
	}
	ImGui::PopID();
	ImGui::PopItemWidth();
	if (this->isReadOnly()) {
		ImGui::PopStyleColor();
	}
	
	if (browseButton) {
		ImGui::SameLine();
		if (ImGui::Button(this->browseButtonLabel)) {
			if (this->browseButtonType == BROWSE_BUTTON_FILE) {
				char** types = (char**)alloca(sizeof(char*) * this->browseFileTypes.size());
				for (int i=0;i<(int)this->browseFileTypes.size();i++) {
					types[i] = (char*)alloca(this->browseFileTypes[i].length() + 1);
					strcpy(types[i], this->browseFileTypes[i].c_str());
				}
				const char* result = tfd::openFileDialog(getTranslation(INSTALLVIEW_OPEN_SETUP_FILE_TITLE), this->text, 1, types, NULL, 0);
				if (result) {
					strcpy(this->text, result);
					if (this->onChange) {
						this->onChange();
					}
				}
			} else {
				const char* result = tfd::selectFolderDialog(getTranslation(GENERIC_OPEN_FOLDER_TITLE), this->text);
				if (result) {
					strcpy(this->text, result);
					if (this->onChange) {
						this->onChange();
					}
				}
			}
		}
	}
}

void LayoutSeparatorControl::draw(int width) {
	ImGui::Separator();
}

void LayoutButtonControl::draw(int width) {
	if (ImGui::Button(this->label.c_str()) && this->onChange) {
		this->onChange();
	}
}

std::shared_ptr<LayoutSection> ImGuiLayout::addSection(int titleId) {
	std::shared_ptr<LayoutSection> section = std::make_shared<LayoutSection>();
	if (titleId) {
		section->setTitle(getTranslation(titleId));
	}
	this->sections.push_back(section);
	return section;
}

void ImGuiLayout::doLayout() {
	leftColumnWidth = 0.0f;
	for (auto& section : sections) {
		section->doLayout();
		if (!section->indent) {
			if (section->leftColumnWidth > leftColumnWidth) {
				leftColumnWidth = section->leftColumnWidth;
			}
		}
	}
}

void LayoutSection::doLayout() {
	leftColumnWidth = 0.0f;
	for (auto& row : rows) {
		if (row->label.length()) {
			float width = ImGui::CalcTextSize(row->label.c_str()).x;
			if (width > leftColumnWidth) {
				leftColumnWidth = width;
			}
		}
	}
	leftColumnWidth += ImGui::GetStyle().ItemSpacing.x;
}

std::shared_ptr<LayoutRow> LayoutSection::addRow(int labelId, int helpId) {
	std::shared_ptr<LayoutRow> row = std::make_shared<LayoutRow>();
	if (labelId) {
		row->label = getTranslation(labelId);
	}
	if (helpId) {
		const char* p = getTranslation(helpId, false);
		if (p) {
			row->help = p;
		}
	}
	this->rows.push_back(row);
	return row;
}

std::shared_ptr<LayoutTextInputControl> LayoutSection::addTextInputRow(int labelId, int helpId, const std::string& initialValue, bool readOnly) {
	std::shared_ptr<LayoutRow> row = this->addRow(labelId, helpId);
	return row->addTextInput(initialValue, readOnly);
}

std::shared_ptr<LayoutComboboxControl> LayoutSection::addComboboxRow(int labelId, int helpId, const std::vector<ComboboxItem>& options, int selected) {
	std::shared_ptr<LayoutRow> row = this->addRow(labelId, helpId);
	return row->addComboBox(options, selected);
}

std::shared_ptr<LayoutSeparatorControl> LayoutSection::addSeparator() {
	std::shared_ptr<LayoutRow> row = this->addRow(0, 0);
	return row->addSeparator();
}

std::shared_ptr<LayoutButtonControl> LayoutSection::addButton(int labelId, int helpId, const std::string& buttonLabel) {
	std::shared_ptr<LayoutRow> row = this->addRow(labelId, helpId);
	return row->addButton(buttonLabel);
}

std::shared_ptr<LayoutTextInputControl> LayoutRow::addTextInput(const std::string& initialValue, bool readOnly) {
	std::shared_ptr<LayoutTextInputControl> control = std::make_shared<LayoutTextInputControl>(shared_from_this());
	if (readOnly) {
		control->setReadOnly(true);
	}
	if (initialValue.length()) {
		control->setText(initialValue);
	}
	this->controls.push_back(control);
	return control;
}

void LayoutRow::setHelp(int helpId) {
	this->help = getTranslation(helpId, false);
}

std::shared_ptr<LayoutComboboxControl> LayoutRow::addComboBox(const std::vector<ComboboxItem>& options, int selected) {
	std::shared_ptr<LayoutComboboxControl> control = std::make_shared<LayoutComboboxControl>(shared_from_this());
	control->setOptions(options);
	if (selected) {
		control->setSelection(selected);
	}
	this->controls.push_back(control);
	return control;
}

std::shared_ptr<LayoutSeparatorControl> LayoutRow::addSeparator() {
	std::shared_ptr<LayoutSeparatorControl> control = std::make_shared<LayoutSeparatorControl>(shared_from_this());
	this->controls.push_back(control);
	return control;
}

std::shared_ptr<LayoutButtonControl> LayoutRow::addButton(const std::string& label) {
	std::shared_ptr<LayoutButtonControl> control = std::make_shared<LayoutButtonControl>(shared_from_this());
	if (label.length()) {
		control->setLabel(label);
	}
	this->controls.push_back(control);
	return control;
}

void LayoutComboboxControl::setOptions(const std::vector<ComboboxItem>& options) {
	this->options.data = options;
	this->options.dataChanged();
}

bool LayoutComboboxControl::setSelectionByLabel(const std::string& label) {
	for (int i = 0; i < this->options.data.size(); i++) {
		if (this->options.data[i].label == label) {
			this->setSelection(i);
			return true;
		}
	}
	return false;
}

bool LayoutComboboxControl::setSelectionStringValue(const std::string& value) {
	for (int i = 0; i < this->options.data.size(); i++) {
		if (this->options.data[i].strValue == value) {
			this->setSelection(i);
			return true;
		}
	}
	return false;
}

LayoutControl::LayoutControl(std::shared_ptr<LayoutRow> row, LayoutControlType type) : row(row), width(0), type(type), readOnly(false), font(NULL) {
}

void LayoutControl::setRowHidden(bool hidden) {
	std::shared_ptr<LayoutRow> row = this->row.lock();
	if (row) {
		row->setHidden(hidden);
	}
}
void LayoutControl::setHelpId(int helpId) {
	std::shared_ptr<LayoutRow> r = this->row.lock();
	if (r) {
		r->setHelp(helpId);
	}
}