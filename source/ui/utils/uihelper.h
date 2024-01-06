#ifndef __UIHELPER_H__
#define __UIHELPER_H__

class BoxedContainer;

bool showMessageBox(BString id, bool open, const char* title, const char* msg);
bool showYesNoMessageBox(BString id, bool open, const char* title, const char* msg, bool* yes);
BString getReadableSize(U64 bytes);
void alignNextTextRightInColumn(const char* text);
void askToDownloadDefaultWine();

#define VIEW_APPS 0
#define VIEW_INSTALL 1
#define VIEW_CONTAINERS 2
#define VIEW_OPTIONS 3
#define VIEW_HELP 4

void gotoView(int viewId, BString tab=BString(), BString param1=BString());

class UIDisableStyle {
public:
	UIDisableStyle(bool disabled=true);
	~UIDisableStyle();

private:
	bool disabled;
};

#endif