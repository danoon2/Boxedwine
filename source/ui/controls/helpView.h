#ifndef __HELP_VIEW_H__
#define __HELP_VIEW_H__

class HelpView : public BaseView {
public:
	HelpView(std::string tab);

	virtual bool saveChanges();

private:
	void createAboutTab();
	void createHelpInstallTab();
	void createHelpTroubleshootingTab();
};

#endif