#ifndef __BASE_VIEW_H__
#define __BASE_VIEW_H__

class BaseViewTab {
public:
	BaseViewTab(BString id, BString name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab, std::function<void()> drawTabIcon, std::function<void()> onRightClick) : name(name), drawTab(drawTab), model(model), drawTabIcon(drawTabIcon), onRightClick(onRightClick), id(id) {}
	BString name;
	std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab;
	std::shared_ptr<ImGuiLayout> model;
	std::function<void()> drawTabIcon;
	std::function<void()> onRightClick;
	BString id;
};

class BaseView {
public:
	BaseView(BString viewName);
        virtual ~BaseView() {}

	virtual bool saveChanges()=0;
	void run(const ImVec2& size);

protected:
	float toolTipWidth;
	float extraVerticalSpacing;

	void addTab(BString id, BString name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab, std::function<void()> drawTabIcon=nullptr, std::function<void()> onRightClick = nullptr);
	void runErrorMsg(bool open);
	int getTabCount() {return (int)tabs.size();}
	void setTabName(int index, BString name) { tabs[index].name = name; }
	void drawToolTip(BString help);

	std::shared_ptr<LayoutComboboxControl> createWindowsVersionCombobox(const std::shared_ptr<LayoutSection>& section);
	std::shared_ptr<LayoutComboboxControl> createFileSystemVersionCombobox(const std::shared_ptr<LayoutSection>& section);

	BString errorMsg;
	BString errorMsgString;
	int tabIndex;
private:
	void addTab(const BaseViewTab& tab, int index);
		
	BString viewName;
	std::vector<BaseViewTab> tabs;
	bool errorMsgOpen;
	bool tabChanged;
};

#endif
