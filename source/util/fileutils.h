#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

bool readLinesFromFile(const std::string& filepath, std::vector<std::string>& lines);
bool writeLinesToFile(const std::string& filepath, std::vector<std::string>& lines);

#endif