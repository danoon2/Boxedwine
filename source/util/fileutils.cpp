#include "boxedwine.h"
#include "fileutils.h"

bool getline(FILE *fin, std::string& result) {    
    int c;
    result="";
    while (EOF != (c = getc(fin))) {
        if (c == '\n') {
            break;
        } else if (c == '\r') {
            continue;
        }
        result+=(char)c;
    }
    if (c == EOF) {
        return false;
    }
    return true;
}

bool readLinesFromFile(const std::string& filepath, std::vector<std::string>& lines) {
    FILE *fp = fopen(filepath.c_str(), "rb");
    if (fp) { 
        std::string line;
        while (getline(fp, line)) {
            lines.push_back(line);
        }
        fclose(fp);
        return true;
    }
    return false;
}

bool writeLinesToFile(const std::string& filepath, std::vector<std::string>& lines) {
    FILE *fp = fopen(filepath.c_str(), "wb");
    if (fp) { 
        for (auto& line : lines) {
            fwrite(line.c_str(), line.size(), 1, fp);
            fwrite("\r\n", 2, 1, fp);
        }
        fflush(fp);
        fclose(fp);
        return true;
    }
    return false;
}