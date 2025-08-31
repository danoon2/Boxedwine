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

#ifdef BOXEDWINE_ZLIB

#ifndef __FSZIP_H__
#define __FSZIP_H__

#include "platformBoxedwine.h"

#undef OF
#define STRICTUNZIP
extern "C"
{
#include "../../lib/zlib/contrib/minizip/unzip.h"
}
#undef OF

class fsZipInfo {
public:
    fsZipInfo() = default;
    BString filename;
    BString link;
    bool isLink = false;
    bool isDirectory = false;
    U64 length = 0;
    U64 lastModified = 0;
    U64 offset = 0;
};

class FsZip : public std::enable_shared_from_this<FsZip> {
public:
    FsZip() = default;
    ~FsZip();
    bool init(BString zipPath, BString mount);
    unzFile zipfile = nullptr;

    U64 lastZipOffset = 0xFFFFFFFFFFFFFFFFl;
    U64 lastZipFileOffset = 0;

    BOXEDWINE_MUTEX readMutex;

    void setupZipRead(U64 zipOffset, U64 zipFileOffset);
    void remove(BString localPath);

    static bool readFileFromZip(BString zipFile, BString file, BString& result);
    static bool extractFileFromZip(BString zipFile, BString file, BString path);
    static BString unzip(BString zipFile, BString path, std::function<void(U32, BString)> percentDone);
    static bool iterateFiles(BString zipFile, std::function<void(BString)> it);
    static bool doesFileExist(BString zipFile, BString file);
private:
    BString deleteFilePath;
};
#endif
#endif
