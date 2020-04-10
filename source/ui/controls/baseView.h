#ifndef __BASE_VIEW_H__
#define __BASE_VIEW_H__

class BaseViewTab {
public:
	BaseViewTab(const std::string& name, std::function<void()> drawTab) : name(name), drawTab(drawTab) {}
	std::string name;
	std::function<void()> drawTab;
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
	void addTab(const std::string& name, std::function<void()> drawTab);
	void runErrorMsg(bool open);

	const char* errorMsg;
	int tabIndex;
private:
	void addTab(const std::string& name, int index);
		
	std::string viewName;
	std::vector<BaseViewTab> tabs;
};

#endif
