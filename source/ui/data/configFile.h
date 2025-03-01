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

#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

class ConfigFile {
public:
    ConfigFile(BString fileName);

    BString readString(BString name, BString defaultValue);
    bool readBool(BString name, bool defaultValue);
    int readInt(BString name, int defaultValue);

    void writeString(BString name, BString value);
    void writeBool(BString name, bool value);
    void writeInt(BString name, int value);

    bool saveChanges();

private:
    BHashTable<BString,BString> values;
    BString fileName;
};
#endif