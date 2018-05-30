#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

bool stringHasEnding(std::string const &fullString, std::string const &ending);
bool stringStartsWith(std::string const &fullString, std::string const &start);
void stringSplit(std::vector<std::string>& results, const std::string& s, char seperator);
void stringReplaceAll(std::string& subject, const std::string& search, const std::string& replace);
bool stringContains(const std::string& str, const std::string& search);
std::string stringJoin(const std::vector<std::string>& v, const std::string& sep);

#endif