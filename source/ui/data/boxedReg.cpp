/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "../boxedwineui.h"
#include "../../source/io/fszip.h"

BoxedReg::BoxedReg(BoxedContainer* container, bool system) {
	BString root = GlobalSettings::getRootFolder(container);
    BString sep = BString::pathSeparator();
	this->filePath = root.stringByApppendingPath("home") + sep + "username" + sep + ".wine" + sep + (system?"system.reg":"user.reg");
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