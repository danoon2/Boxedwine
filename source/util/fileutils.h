#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

bool readLinesFromFile(BString filepath, std::vector<BString>& lines);
bool writeLinesToFile(BString filepath, std::vector<BString>& lines);

#endif