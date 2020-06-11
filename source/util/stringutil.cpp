#include "boxedwine.h"

#include <locale>  
#include <cctype>
#include <string>
#include <sstream>

bool stringHasEnding(std::string const &fullString, std::string const &ending, bool ignoreCase) {
    if (fullString.length() >= ending.length()) {
        if (ignoreCase) {            
            std::string s = fullString;
            std::string e = ending;
            stringToLower(s);
            stringToLower(e);
            return (0 == s.compare (s.length() - e.length(), e.length(), e));
        }
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool stringStartsWith(std::string const &fullString, std::string const &start, bool ignoreCase) {
    if (fullString.length() >= start.length()) {
        if (ignoreCase) {            
            std::string f = fullString;
            std::string s = start;
            stringToLower(f);
            stringToLower(s);
            return (0 == f.compare (0, s.length(), s));
        }
        return equal(start.begin(), start.end(), fullString.begin());
    } else {
        return false;
    }
}

void stringSplit(std::vector<std::string>& results, const std::string& s, char seperator, int maxParts)
{
    if (s.length()) {
        std::string::size_type prev_pos = 0, pos = 0;

        while ((pos = s.find(seperator, pos)) != std::string::npos && (maxParts == -1 || ((int)results.size()) < maxParts - 1))
        {
            std::string substring(s.substr(prev_pos, pos - prev_pos));

            results.push_back(substring);
            prev_pos = ++pos;
        }
        results.push_back(s.substr(prev_pos, pos - prev_pos));
    }
}

void stringReplaceAll(std::string& subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

bool stringContains(const std::string& str, const std::string& search) {
    return strstr(str.c_str(),search.c_str())!=0;
}

bool stringContainsIgnoreCase(const std::string& str, const std::string& search) {
    std::locale loc;
    auto it = std::search(str.begin(), str.end(), search.begin(),   search.end(), [loc](char ch1, char ch2) { 
        return std::toupper(ch1, loc) == std::toupper(ch2, loc); 
    });
    return (it != str.end() );
}

bool vectorContainsIgnoreCase(const std::vector<std::string>& strs, const std::string& search) {
    for (auto& s : strs) {
        if (stringContainsIgnoreCase(s, search)) {
            return true;
        }
    }
    return false;
}

std::string stringJoin(const std::vector<std::string>& v, const std::string& sep) {
    std::string result;

   for (std::vector<std::string>::const_iterator p = v.begin(); p != v.end(); ++p) {
      result += *p;
      if (p != v.end() - 1)
        result += sep;
   }
   return result;
}

void stringToLower(std::string& s) {
    std::locale loc;
    for (auto& c : s) {
        c = std::tolower(c, loc);
    }
}

bool stringCaseInSensativeEquals(const std::string & str1, const std::string &str2) {
    std::locale loc;
	return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), [loc](const char & c1, const char & c2){
							return (c1 == c2 || std::toupper(c1, loc) == std::toupper(c2, loc));
								}));
}

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
void stringTrim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

int stringIndexInVector(const std::string& value, const std::vector<std::string> values, int returnIfNotFound) {
    auto it = std::find(values.begin(), values.end(), value);
    if (it == values.end()) {
        return returnIfNotFound;
    } else {
        return (int)std::distance(values.begin(), it);
    }
}

bool stringIsLessCaseInsensative(const std::string& a, const std::string& b) {
    for (size_t c = 0; c < a.size() && c < b.size(); c++) {
        if (std::tolower(a[c]) != std::tolower(b[c]))
            return (std::tolower(a[c]) < std::tolower(b[c]));
    }
    return a.size() < b.size();
}

std::string toHexString(int i) {
    std::string result;
    std::stringstream ss;
    ss << std::hex << i;
    ss >> result;
    return result;
}