#ifndef __UIHELPER_H__
#define __UIHELPER_H__

class BoxedContainer;

bool showMessageBox(const std::string& id, bool open, const char* title, const char* msg);
bool showYesNoMessageBox(const std::string& id, bool open, const char* title, const char* msg, bool* yes);
std::string getReadableSize(U64 bytes);
void alignNextTextRightInColumn(const char* text);
void askToDownloadDefaultWine();

#define VIEW_APPS 0
#define VIEW_INSTALL 1
#define VIEW_CONTAINERS 2
#define VIEW_OPTIONS 3
void gotoView(int viewId, std::string tab="", std::string param1="");

class UIDisableStyle {
public:
	UIDisableStyle(bool disabled=true);
	~UIDisableStyle();

private:
	bool disabled;
};

#endif