#include "boxedwine.h"
#include "../boxedwineui.h"

#include "../../util/fileutils.h"
#include "../../util/stringutil.h"

ConfigFile::ConfigFile(BString fileName) {
    std::vector<BString> lines;
    readLinesFromFile(fileName, lines);
    this->fileName = fileName;
    for (auto& line : lines) {
        std::vector<BString> parts;
        line.split('=', parts);
        if (parts.size() < 2) {
            kwarn("%s has a malformed key/value pair", fileName.c_str());
        } else {
            parts[0] = parts[0].trim();
            parts[1] = parts[1].trim();
            this->values.set(parts[0], parts[1]);
        }
    }
}

BString ConfigFile::readString(BString name, BString defaultValue) {
    BString value;
    if (this->values.get(name, value)) {
        return value;
    }
    return defaultValue;
}

bool ConfigFile::readBool(BString name, bool defaultValue) {
    BString value;
    if (this->values.get(name, value)) {
        return value!="0";
    }
    return defaultValue;
}

int ConfigFile::readInt(BString name, int defaultValue) {
    BString value;
    if (this->values.get(name, value)) {
        if (value.length()==0) {
            return 0;
        }
        return atoi(value.c_str());
    }
    return defaultValue;
}

void ConfigFile::writeString(BString name, BString value) {
    this->values.set(name, value);
}

void ConfigFile::writeBool(BString name, bool value) {
    this->values.set(name, B(value?"1":"0"));
}

void ConfigFile::writeInt(BString name, int value) {
    this->values.set(name, BString::valueOf(value));
}

bool ConfigFile::saveChanges() {
    std::vector<BString> lines;
    for (auto& n : this->values) {
        lines.push_back(n.key+"="+n.value);
    }
    return writeLinesToFile(this->fileName, lines);
}