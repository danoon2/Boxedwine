#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../source/io/fszip.h"

BoxedReg::BoxedReg(BoxedContainer* container, bool system) {
	std::string root = GlobalSettings::getRootFolder(container);
	this->filePath = root + Fs::nativePathSeperator + "home" + Fs::nativePathSeperator + "username" + Fs::nativePathSeperator + ".wine" + Fs::nativePathSeperator + (system?"system.reg":"user.reg");
	if (!Fs::doesNativePathExist(this->filePath)) {
		FsZip::extractFileFromZip(GlobalSettings::getFileFromWineName(container->getWineVersion()), (system?"home/username/.wine/system.reg":"home/username/.wine/user.reg"), this->filePath);
	}
	readLinesFromFile(this->filePath, lines);
}

bool BoxedReg::readKey(const char* path, const char* key, std::string& value) {
    bool found = false;
    std::string section = "[";
    section += path;
    section += "]";
    stringReplaceAll(section, "\\", "\\\\");
    std::string searchKey = "\"";
    searchKey += key;
    searchKey += "\"=";

    for (int i = 0; i < (int)lines.size(); i++) {
        std::string& line = lines[i];
        if (!found) {
            if (stringStartsWith(line, section)) {
                found = true;
            }
        } else {
            if (stringStartsWith(line, "[")) {
                return false;
            }
            if (stringStartsWith(line, searchKey)) {
                value = line.substr(searchKey.length());
                stringTrim(value);
                stringReplaceAll(value, "\"", "");
                return true;
            }
        }
    }
    return false;
}

void BoxedReg::writeKeyDword(const char* path, const char* key, U32 value) {
    std::string v = "dword:";
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%08x", value);
    v += tmp;
    writeKey(path, key, v.c_str(), false);
}

void BoxedReg::writeKey(const char* path, const char* key, const char* value, bool useQuotesAroundValue) {
    bool found = false;
    std::string section = "[";    
    section += path;
    section += "]";
    stringReplaceAll(section, "\\", "\\\\");
    std::string searchKey = "\"";
    searchKey += key;
    searchKey += "\"=";

    for (int i = 0; i < (int)lines.size(); i++) {
        std::string& line = lines[i];
        if (!found) {
            if (stringStartsWith(line, section)) {
                found = true;
            }
        } else {
            if (stringStartsWith(line, "[")) {
                if (value) {
                    std::string p = "\"";
                    p += key;
                    p += "\"=";
                    if (useQuotesAroundValue) {
                        p += "\"";
                    }
                    p += value;
                    if (useQuotesAroundValue) {
                        p += "\"";
                    }
                    lines.insert(lines.begin() + i - 1, p);
                }
                return;
            }
            if (stringStartsWith(line, searchKey)) {
                if (value) {
                    line = "\"";
                    line += key;
                    line += "\"=";
                    if (useQuotesAroundValue) {
                        line += "\"";
                    }
                    line += value;
                    if (useQuotesAroundValue) {
                        line += "\"";
                    }
                } else {
                    lines.erase(lines.begin() + i);
                    i--;
                }
                return;
            }
        }
    }
    if (value) {
        std::string line = "\"";
        line = key;
        line += "\"=";
        if (useQuotesAroundValue) {
            line += "\"";
        }
        line += value;
        if (useQuotesAroundValue) {
            line += "\"";
        }
        lines.push_back("");
        lines.push_back(section);
        lines.push_back(line);    
    }
}

void BoxedReg::save() {
	writeLinesToFile(this->filePath, lines);
}