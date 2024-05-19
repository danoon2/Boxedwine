#ifndef __READ_ICONS_H__
#define __READ_ICONS_H__

std::shared_ptr<U8[]> extractIconFromExe(BString nativeExePath, int size, int* width, int* height);

#endif