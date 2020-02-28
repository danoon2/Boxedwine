#ifndef __UIHELPER_H__
#define __UIHELPER_H__

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
bool showMessageBox(bool open, const char* title, const char* msg);
std::string getReadableSize(U64 bytes);
void alignNextTextRightInColumn(const char* text);

#endif