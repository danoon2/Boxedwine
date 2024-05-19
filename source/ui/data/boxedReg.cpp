#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../source/io/fszip.h"

BoxedReg::BoxedReg(BoxedContainer* container, bool system) {
	BString root = GlobalSettings::getRootFolder(container);
	this->filePath = root ^ "home" ^ "username" ^ ".wine" ^ (system?"system.reg":"user.reg");
	if (!Fs::doesNativePathExist(this->filePath)) {
        std::shared_ptr<FileSystemZip> fs = container->getFileSystem().lock();
        if (fs) {
            FsZip::extractFileFromZip(fs->filePath, B(system ? "home/username/.wine/system.reg" : "home/username/.wine/user.reg"), Fs::getNativeParentPath(this->filePath));
        }
	}
	readLinesFromFile(this->filePath, lines);
}

bool BoxedReg::readKey(const char* path, const char* key, BString& value) {
    bool found = false;
    BString section = B("[");
    section += path;
    section += "]";
    section.replace("\\", "\\\\");
    BString searchKey = B("\"");
    searchKey += key;
    searchKey += "\"=";

    for (size_t i = 0; i < lines.size(); i++) {
        BString line = lines[i];
        if (!found) {
            if (line.startsWith(section)) {
                found = true;
            }
        } else {
            if (line.startsWith("[")) {
                return false;
            }
            if (line.startsWith(searchKey)) {
                value = line.substr(searchKey.length());
                value = value.trim();
                value = value.replace("\"", "");
                return true;
            }
        }
    }
    return false;
}

void BoxedReg::writeKeyDword(const char* path, const char* key, U32 value) {
    BString v = B("dword:");
    v.append(value, 16);
    writeKey(path, key, v.c_str(), false);
}

void BoxedReg::writeKey(const char* path, const char* key, const char* value, bool useQuotesAroundValue) {
    bool found = false;
    BString section = B("[");
    section += path;
    section += "]";
    section.replace("\\", "\\\\");
    BString searchKey = B("\"");
    searchKey += key;
    searchKey += "\"=";

    for (size_t i = 0; i < lines.size(); i++) {
        BString& line = lines[i];
        if (!found) {
            if (line.startsWith(section)) {
                found = true;
            }
        } else {
            if (line.startsWith("[")) {
                if (value) {
                    BString p = B("\"");
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
            if (line.startsWith(searchKey)) {
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
        BString line = B("\"");
        line += key;
        line += "\"=";
        if (useQuotesAroundValue) {
            line += "\"";
        }
        line += value;
        if (useQuotesAroundValue) {
            line += "\"";
        }        
        if (!found) {
            lines.push_back(B(""));
            lines.push_back(section);
        }
        lines.push_back(line);    
    }
}

void BoxedReg::save() {
	writeLinesToFile(this->filePath, lines);
}