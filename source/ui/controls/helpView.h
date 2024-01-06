#ifndef __HELP_VIEW_H__
#define __HELP_VIEW_H__

class HelpView : public BaseView {
public:
	HelpView(BString tab);

	virtual bool saveChanges();

private:
	void createAboutTab();
	void createHelpInstallTab();
	void createHelpTroubleshootingTab();
};

#endif