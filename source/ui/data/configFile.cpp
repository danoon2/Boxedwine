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
            kwarn_fmt("%s has a malformed key/value pair", fileName.c_str());
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