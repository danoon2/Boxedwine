#ifndef __BASE_VIEW_H__
#define __BASE_VIEW_H__

class BaseViewTab {
public:
	BaseViewTab(const std::string& name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab) : name(name), drawTab(drawTab), model(model) {}
	std::string name;
	std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab;
	std::shared_ptr<ImGuiLayout> model;
};

class BaseView {
public:
	BaseView(const std::string& viewName);
        virtual ~BaseView() {}

	virtual bool saveChanges()=0;
	void run(const ImVec2& size);

protected:
	float toolTipWidth;
	float extraVerticalSpacing;

	void toolTip(const char* desc);
	void addTab(const std::string& name, const std::shared_ptr<ImGuiLayout>& model, std::function<void(bool buttonPressed, BaseViewTab& tab)> drawTab);
	void runErrorMsg(bool open);
	int getTabCount() {return (int)tabs.size();}

	const char* errorMsg;
	std::string errorMsgString;
	int tabIndex;
private:
	void addTab(const std::string& name, int index);
		
	std::string viewName;
	std::vector<BaseViewTab> tabs;
	bool errorMsgOpen;
	bool tabChanged;
};

#endif
