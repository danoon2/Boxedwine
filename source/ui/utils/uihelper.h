#ifndef __UIHELPER_H__
#define __UIHELPER_H__

class BoxedContainer;

bool showMessageBox(bool open, const char* title, const char* msg);
std::string getReadableSize(U64 bytes);
void alignNextTextRightInColumn(const char* text);

#endif