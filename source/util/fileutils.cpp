#include "boxedwine.h"
#include "fileutils.h"

#include <string>

bool readLinesFromFile(BString filepath, std::vector<BString>& lines) {
    BReadFile file;

    file.open(filepath);
    if (file.isOpen()) {
        BString line;
        while (file.readLine(line)) {
            lines.push_back(line);
        }
        file.close();
        return true;
    }
    return false;
}

bool writeLinesToFile(BString filepath, std::vector<BString>& lines) {
    BWriteFile file;

    file.createNew(filepath);
    if (file.isOpen()) {
        for (auto& line : lines) {
            file.write(line);
            file.write("\r\n");
        }
        return true;
    }
    return false;
}