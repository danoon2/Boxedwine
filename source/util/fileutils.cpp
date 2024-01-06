#include "boxedwine.h"
#include "fileutils.h"

bool getline(FILE *fin, BString& result) {
    int c;
    result.removeAll();
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

bool readLinesFromFile(BString filepath, std::vector<BString>& lines) {
    FILE *fp = fopen(filepath.c_str(), "rb");
    if (fp) { 
        BString line;
        while (getline(fp, line)) {
            lines.push_back(line);
        }
        if (line.length()) {
            lines.push_back(line);
        }
        fclose(fp);
        return true;
    }
    return false;
}

bool writeLinesToFile(BString filepath, std::vector<BString>& lines) {
    FILE *fp = fopen(filepath.c_str(), "wb");
    if (fp) { 
        for (auto& line : lines) {
            fwrite(line.c_str(), line.length(), 1, fp);
            fwrite("\r\n", 2, 1, fp);
        }
        fflush(fp);
        fclose(fp);
        return true;
    }
    return false;
}