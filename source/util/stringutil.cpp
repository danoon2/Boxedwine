#include "boxedwine.h"

#include <locale>  
#include <cctype>
#include <string>
#include <sstream>
#include <string>
#include <charconv>

bool vectorContainsIgnoreCase(const std::vector<BString>& strs, BString search) {
    for (auto& s : strs) {
        if (s.contains(search, true)) {
            return true;
        }
    }
    return false;
}

int stringIndexInVector(const std::string& value, const std::vector<std::string> values, int returnIfNotFound) {
    auto it = std::find(values.begin(), values.end(), value);
    if (it == values.end()) {
        return returnIfNotFound;
    } else {
        return (int)std::distance(values.begin(), it);
    }
}