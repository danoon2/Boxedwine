#include "boxedwine.h"
#include "fileutils.h"

#include <string>

bool readLinesFromFile(BString filepath, std::vector<BString>& lines) {
    std::fstream file;

    file.open(filepath.c_str(), std::ios::in);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(BString::copy(line.c_str()));
        }
        file.close();
        return true;
    }
    return false;
}

bool writeLinesToFile(BString filepath, std::vector<BString>& lines) {
    std::fstream file;

    file.open(filepath.c_str(), std::ios::out);
    if (file.is_open()) {
        for (auto& line : lines) {
            file << line.c_str() << "\r\n";
        }
        return true;
    }
    return false;
}