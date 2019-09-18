#include "boxedwine.h"

#include <locale>  

bool stringHasEnding(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool stringStartsWith(std::string const &fullString, std::string const &start) {
    return fullString.size() >= start.size() && equal(start.begin(), start.end(), fullString.begin());
}

void stringSplit(std::vector<std::string>& results, const std::string& s, char seperator)
{
    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        results.push_back(substring);
        prev_pos = ++pos;
    }
    results.push_back(s.substr(prev_pos, pos-prev_pos));
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