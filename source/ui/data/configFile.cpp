#include "boxedwine.h"
#include "../boxedwineui.h"

#include "../../util/fileutils.h"
#include "../../util/stringutil.h"

ConfigFile::ConfigFile(const std::string& fileName) {
    std::vector<std::string> lines;
    readLinesFromFile(fileName, lines);
    this->fileName = fileName;
    for (auto& line : lines) {
        std::vector<std::string> parts;
        stringSplit(parts, line, '=', 2);
        if (parts.size()!=2) {
            kwarn("%s has a malformed key/value pair", fileName.c_str());
        } else {
            stringTrim(parts[0]);
            stringTrim(parts[1]);
            this->values[parts[0]] = parts[1];
        }
    }
}

std::string ConfigFile::readString(const std::string& name, const std::string& defaultValue) {
    if (this->values.count(name)) {
        return this->values[name];
    }
    return defaultValue;
}

bool ConfigFile::readBool(const std::string& name, bool defaultValue) {
    if (this->values.count(name)) {
        return this->values[name]!="0";
    }
    return defaultValue;
}

int ConfigFile::readInt(const std::string& name, int defaultValue) {
    if (this->values.count(name)) {
        std::string& value = this->values[name];
        if (value.length()==0) {
            return 0;
        }
        return atoi(value.c_str());
    }
    return defaultValue;
}

void ConfigFile::writeString(const std::string& name, const std::string& value) {
    this->values[name] = value;
}

void ConfigFile::writeBool(const std::string& name, bool value) {
    this->values[name] = value?"1":"0";
}

void ConfigFile::writeInt(const std::string& name, int value) {
    this->values[name] = std::to_string(value);
}

bool ConfigFile::saveChanges() {
    std::vector<std::string> lines;
    for (auto& n : this->values) {
        lines.push_back(n.first+"="+n.second);
    }
    return writeLinesToFile(this->fileName, lines);
}