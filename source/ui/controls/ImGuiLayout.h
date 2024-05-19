#ifndef __LAYOUT_MODEL_H__
#define __LAYOUT_MODEL_H__

enum class LayoutControlType {
	TextInput,
	Combobox,
	Checkbox,
	Label,
	Button,
	Seperator,
	Text,
	Custom
};

class ComboboxItem {
public:
	ComboboxItem(BString label, BString value) : label(label), strValue(value), intValue(0) {};
	ComboboxItem(BString label, int value) : label(label), intValue(value) {};
	ComboboxItem(BString label) : label(label), strValue(label), intValue(0) {};
	BString label;
	BString strValue;
	int intValue;
};

class ComboboxData {
public:
	BString dataForCombobox;
	std::vector<ComboboxItem> data;
	int currentSelectedIndex = 0;

	void dataChanged();
};

class LayoutRow;

class LayoutControl {
public:	
	LayoutControl(std::shared_ptr<LayoutRow> row, LayoutControlType type);

	void setReadOnly(bool readOnly) {this->readOnly = readOnly;}
	bool isReadOnly() {return this->readOnly;}

	void setWidth(int width) {this->width = width;}
	int getWidth() {return this->width;}

	void setRowHidden(bool hidden);

	void setFont(ImFont* font) {this->font = font;}
	ImFont* getFont() {return this->font;}

	void setHelpId(Msg helpId);

	virtual void draw(int width) = 0;	
	virtual int getRecommendedWidth() {return width;}

	std::function<void()> onChange;
private:
	std::weak_ptr<LayoutRow> row;

	int width;
	LayoutControlType type;
	bool readOnly;
	ImFont* font;
};

#define BROWSE_BUTTON_NONE 0
#define BROWSE_BUTTON_FILE 1
#define BROWSE_BUTTON_DIR 2

class LayoutTextInputControl : public LayoutControl {
public:
	LayoutTextInputControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::TextInput), onBrowseFinished(nullptr), numberOfLines(1), browseButtonType(BROWSE_BUTTON_NONE), browseButtonWidth(0.0f), browseButtonLabel(nullptr) {this->text[0]=0;}

	// from LayoutControl
	void draw(int width) override;

	void setText(BString text) { strncpy(this->text, text.c_str(), sizeof(this->text)); }
	BString getText() { return BString::copy(this->text); }

	void setBrowseFileButton(const std::vector<BString>& browseFileTypes) {this->browseButtonType = BROWSE_BUTTON_FILE; this->browseFileTypes = browseFileTypes;}
	void setBrowseDirButton() {this->browseButtonType = BROWSE_BUTTON_DIR;}
	void removeBrowseButton() {this->browseButtonType = BROWSE_BUTTON_NONE;}

	void setNumberOfLines(int numberOfLines) {this->numberOfLines = numberOfLines;}
	int getNumberOfLines() {return this->numberOfLines;}
	
	std::function<void()> onBrowseFinished;
private:
	int numberOfLines;
	int browseButtonType;
	float browseButtonWidth;
	const char* browseButtonLabel;
	std::vector<BString> browseFileTypes;
	char text[1024];
};

class LayoutTextControl : public LayoutControl {
public:
	LayoutTextControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Text) {}

	// LayoutControl
	void draw(int width) override;

	void setText(BString text) { this->text = text; }
	BString getText() { return this->text; }	
private:
	BString text;
};

class LayoutComboboxControl : public LayoutControl {
public:
	LayoutComboboxControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Combobox) {}

	// from LayoutControl
	void draw(int width) override;

	void setSelection(int selection) { this->options.currentSelectedIndex = selection;}
	int getSelection() {return this->options.currentSelectedIndex;}

	bool setSelectionByLabel(BString label);
	bool setSelectionStringValue(BString value);
	bool setSelectionIntValue(int value);
	int getSelectionIntValue() {return this->options.data[this->options.currentSelectedIndex].intValue;}
	BString getSelectionStringValue() { return this->options.data[this->options.currentSelectedIndex].strValue; }

	void setOptions(const std::vector<ComboboxItem>& options);	
private:
	ComboboxData options;
};

class LayoutCheckboxControl : public LayoutControl {
public:
	LayoutCheckboxControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Checkbox), checked(false) {}

	// from LayoutControl
	void draw(int width) override;

	void setCheck(bool b) { this->checked = b; }
	bool isChecked() { return this->checked; }	
private:
	bool checked;
};

class LayoutSeparatorControl : public LayoutControl {
public:
	LayoutSeparatorControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Seperator) {}

	// from LayoutControl
	void draw(int width) override;
};

class LayoutButtonControl : public LayoutControl {
public:
	LayoutButtonControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Button) {}

	// from LayoutControl
	void draw(int width) override;
	int getRecommendedWidth() override;

	void setLabel(BString label) { this->label = label; }
	BString getLabel() { return this->label; }	
private:
	BString label;
};

class LayoutCustomControl : public LayoutControl {
public:
	LayoutCustomControl(std::shared_ptr<LayoutRow> row, std::function<void()> onDraw) : LayoutControl(row, LayoutControlType::Seperator), onDraw(onDraw) {}

	// from LayoutControl
	void draw(int width) override {
		onDraw();
	}
	int getRecommendedWidth() override {return 0;}
private:
	std::function<void()> onDraw;
};

class LayoutRow : public std::enable_shared_from_this<LayoutRow> {
public:
	LayoutRow() : hidden(false), topMargin(GlobalSettings::extraVerticalSpacing) {}

	void draw(float toolTipWidth, float labelOffset, float valueOffset);
	void drawToolTip(BString help);

	std::shared_ptr<LayoutTextInputControl> addTextInput(BString initialValue = BString(), bool readOnly = false);
	std::shared_ptr<LayoutComboboxControl> addComboBox(const std::vector<ComboboxItem>& options, int selected=0);
	std::shared_ptr<LayoutComboboxControl> addComboBox();
	std::shared_ptr<LayoutSeparatorControl> addSeparator();
	std::shared_ptr<LayoutButtonControl> addButton(BString label);
	std::shared_ptr< LayoutCheckboxControl> addCheckbox(bool checked);
	std::shared_ptr<LayoutCustomControl> addCustomControl(std::function<void()> onDraw);
	std::shared_ptr<LayoutTextControl> addText(BString text);

	void setHidden(bool hidden) { this->hidden = hidden; }
	bool isHidden() { return this->hidden; }

	void setHelp(Msg helpId);
	void setTopMargin(float margin) {this->topMargin = margin;}
private:
	friend class LayoutSection;
	BString label;
	BString help;
	std::vector<std::shared_ptr<LayoutControl>> controls;
	bool hidden;
	float topMargin;
};

class LayoutSection {
public:
	LayoutSection() : indent(false), hidden(false), leftColumnWidth(0.0f), isLeftColumnWidthDirty(true) {}

	void draw(float toolTipWidth, float offset);

	std::shared_ptr<LayoutRow> addRow(Msg labelId, Msg helpId);

	// convenience functions
	std::shared_ptr<LayoutTextInputControl> addTextInputRow(Msg labelId, Msg helpId, BString initialValue=BString(), bool readOnly=false);
	std::shared_ptr<LayoutComboboxControl> addComboboxRow(Msg labelId, Msg helpId, const std::vector<ComboboxItem>& options, int selected = 0);
	std::shared_ptr<LayoutSeparatorControl> addSeparator();
	std::shared_ptr<LayoutButtonControl> addButton(Msg labelId, Msg helpId, BString buttonLabel);
	std::shared_ptr< LayoutCheckboxControl> addCheckbox(Msg labelId, Msg helpId, bool value);
	std::shared_ptr<LayoutTextControl>addText(Msg labelId, Msg helpId, BString text);

	void setTitle(BString title) {this->title = title;}
	BString getTitle() {return this->title;}
	void setIndent(bool b) { this->indent = b; }

	void setHidden(bool hidden) { this->hidden = hidden; }
	bool isHidden() { return this->hidden; }

private:
	friend class ImGuiLayout;
	void doLayout();

	BString title;
	bool indent;
	bool hidden;
	std::vector<std::shared_ptr<LayoutRow>> rows;
	float leftColumnWidth;
	bool isLeftColumnWidthDirty;
};

class ImGuiLayout {
public:
	ImGuiLayout() : leftColumnWidth(0.0f), isLeftColumnWidthDirty(true), toolTipWidth(0.0f) {}

	std::shared_ptr<LayoutSection> addSection(Msg titleId=Msg::NONE);

	void draw();
	void doLayout();
private:
	std::vector<std::shared_ptr<LayoutSection>> sections;
	float leftColumnWidth;
	bool isLeftColumnWidthDirty;
	float toolTipWidth;
};

#endif
