#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

bool stringHasEnding(std::string const &fullString, std::string const &ending, bool ignoreCase=false);
bool stringStartsWith(std::string const &fullString, std::string const &start, bool ignoreCase=false);
void stringSplit(std::vector<std::string>& results, const std::string& s, char seperator, int maxParts=-1);
void stringReplaceAll(std::string& subject, const std::string& search, const std::string& replace);
bool stringContains(const std::string& str, const std::string& search);
bool stringContainsIgnoreCase(const std::string& str, const std::string& search);
bool vectorContainsIgnoreCase(const std::vector<std::string>& strs, const std::string& search);
std::string stringJoin(const std::vector<std::string>& v, const std::string& sep);
void stringToLower(std::string& s);
bool stringCaseInSensativeEquals(const std::string & str1, const std::string &str2);
void stringTrim(std::string &s);
int stringIndexInVector(const std::string& value, const std::vector<std::string> values, int returnIfNotFound=-1);
#endif