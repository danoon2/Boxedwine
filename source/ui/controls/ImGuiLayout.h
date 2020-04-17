#ifndef __LAYOUT_MODEL_H__
#define __LAYOUT_MODEL_H__

enum class LayoutControlType {
	TextInput,
	TextInputMultiLine,
	Combobox,
	Checkbox,
	Label,
	Button,
	Seperator
};

class ComboboxData {
public:
	ComboboxData() : dataForCombobox(0), currentSelectedIndex(0) {}
	char* dataForCombobox;
	std::vector<std::string> data;
	int currentSelectedIndex;

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

	void setHelpId(int helpId);

	virtual void draw(int width) = 0;	

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
	LayoutTextInputControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::TextInput), browseButtonType(BROWSE_BUTTON_NONE), browseButtonWidth(0.0f), browseButtonLabel(NULL) {this->text[0]=0;}

	void setText(const std::string& text) { strncpy(this->text, text.c_str(), sizeof(this->text)); }
	std::string getText() {return this->text;}

	void setBrowseFileButton(char const* const* const browseFileTypes) {this->browseButtonType = BROWSE_BUTTON_FILE; this->browseFileTypes = browseFileTypes;}
	void setBrowseDirButton() {this->browseButtonType = BROWSE_BUTTON_DIR;}
	void removeBrowseButton() {this->browseButtonType = BROWSE_BUTTON_NONE;}

	virtual void draw(int width);
private:
	int browseButtonType;
	bool hasDirBrowseButton;
	float browseButtonWidth;
	const char* browseButtonLabel;
	char const* const* browseFileTypes;
	char text[1024];
};

class LayoutComboboxControl : public LayoutControl {
public:
	LayoutComboboxControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Combobox) {}

	void setSelection(int selection) { this->options.currentSelectedIndex = selection; if (this->onChange) {onChange();} }
	int getSelection() {return this->options.currentSelectedIndex;}

	void setOptions(const std::vector<std::string>& options);

	virtual void draw(int width);
private:
	ComboboxData options;
};

class LayoutSeparatorControl : public LayoutControl {
public:
	LayoutSeparatorControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Seperator) {}

	virtual void draw(int width);
};

class LayoutButtonControl : public LayoutControl {
public:
	LayoutButtonControl(std::shared_ptr<LayoutRow> row) : LayoutControl(row, LayoutControlType::Button) {}

	void setLabel(const std::string& label) { this->label = label; }
	const std::string& getLabel() { return this->label; }

	virtual void draw(int width);
private:
	std::string label;
};

class LayoutRow : public std::enable_shared_from_this<LayoutRow> {
public:
	LayoutRow() : hidden(false) {}

	void draw(float toolTipWidth, float labelOffset, float valueOffset);
	void drawToolTip(const std::string& help);

	std::shared_ptr<LayoutTextInputControl> addTextInput(const std::string& initialValue = "", bool readOnly = false);
	std::shared_ptr<LayoutComboboxControl> addComboBox(const std::vector<std::string>& options, int selected=0);
	std::shared_ptr<LayoutSeparatorControl> addSeparator();
	std::shared_ptr<LayoutButtonControl> addButton(const std::string& label);

	void setHidden(bool hidden) { this->hidden = hidden; }
	bool isHidden() { return this->hidden; }

	void setHelp(int helpId);
private:
	friend class LayoutSection;
	std::string label;
	std::string help;
	std::vector<std::shared_ptr<LayoutControl>> controls;
	bool hidden;
};

class LayoutSection {
public:
	LayoutSection() : indent(false), hidden(false), leftColumnWidth(0.0f), isLeftColumnWidthDirty(true) {}

	void draw(float toolTipWidth, float offset);

	std::shared_ptr<LayoutRow> addRow(int labelId, int helpId);

	// convenience functions
	std::shared_ptr<LayoutTextInputControl> addTextInputRow(int labelId, int helpId, const std::string& initialValue="", bool readOnly=false);
	std::shared_ptr<LayoutComboboxControl> addComboboxRow(int labelId, int helpId, const std::vector<std::string>& options, int selected = 0);
	std::shared_ptr<LayoutSeparatorControl> addSeparator();
	std::shared_ptr<LayoutButtonControl> addButton(int labelId, int helpId, const std::string& buttonLabel);

	void setTitle(const std::string& title) {this->title = title;}
	const std::string& getTitle() {return this->title;}
	void setIndent(bool b) { this->indent = b; }

	void setHidden(bool hidden) { this->hidden = hidden; }
	bool isHidden() { return this->hidden; }

private:
	friend class ImGuiLayout;
	void doLayout();

	std::string title;
	bool indent;
	bool hidden;
	std::vector<std::shared_ptr<LayoutRow>> rows;
	float leftColumnWidth;
	bool isLeftColumnWidthDirty;
};

class ImGuiLayout {
public:
	ImGuiLayout() : leftColumnWidth(0.0f), isLeftColumnWidthDirty(true), toolTipWidth(0.0f) {}

	std::shared_ptr<LayoutSection> addSection(int titleId=0);

	void draw();
	void doLayout();
private:
	std::vector<std::shared_ptr<LayoutSection>> sections;
	float leftColumnWidth;
	bool isLeftColumnWidthDirty;
	float toolTipWidth;
};

#endif