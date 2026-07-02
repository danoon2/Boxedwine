/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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
#include "fsiso.h"
#include "fsisoopennode.h"
#include "fsfilenode.h"
#include "fs.h"
#include "kstat.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>

// ISO 9660 volume descriptor types
#define ISO_VD_PRIMARY       1
#define ISO_VD_SUPPLEMENTARY 2
#define ISO_VD_TERMINATOR    255

// Directory record flag bits
#define ISO_FLAG_DIRECTORY   0x02

// Joliet escape sequences start with 0x25 0x2F
#define JOLIET_LEVEL1_ESCAPE "\x25\x2F\x40"
#define JOLIET_LEVEL2_ESCAPE "\x25\x2F\x43"
#define JOLIET_LEVEL3_ESCAPE "\x25\x2F\x45"

static U32 readU32LE(const U8* p) {
    return (U32)p[0] | ((U32)p[1] << 8) | ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

static U64 readDirectoryTime(const U8* record) {
    struct tm tm = {};
    U8 month = record[19];
    U8 day = record[20];
    if (month < 1 || month > 12 || day < 1 || day > 31)
        return 0;
    tm.tm_year = record[18];
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = record[21];
    tm.tm_min = record[22];
    tm.tm_sec = record[23];
    tm.tm_isdst = -1;
    time_t seconds = mktime(&tm);
    if (seconds < 0)
        return 0;
    return (U64)seconds * 1000l;
}

// Convert Joliet UTF-16 BE filename to UTF-8
static BString utf16BEtoUtf8(const U8* data, U8 byteLen) {
    BString result;
    for (U32 i = 0; i + 1 < byteLen; i += 2) {
        U32 cp = ((U32)data[i] << 8) | data[i + 1];
        if (cp < 0x80) {
            result.append((char)cp);
        } else if (cp < 0x800) {
            result.append((char)(0xC0 | (cp >> 6)));
            result.append((char)(0x80 | (cp & 0x3F)));
        } else {
            result.append((char)(0xE0 | (cp >> 12)));
            result.append((char)(0x80 | ((cp >> 6) & 0x3F)));
            result.append((char)(0x80 | (cp & 0x3F)));
        }
    }
    return result;
}

FsIso::~FsIso() {
    if (file) {
        fclose(file);
        file = nullptr;
    }
}

U32 FsIso::readData(U64 byteOffset, U8* buf, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(readMutex);
#ifdef BOXEDWINE_MSVC
    if (_fseeki64(file, (S64)byteOffset, SEEK_SET) != 0)
        return 0;
#else
    if (fseeko(file, (off_t)byteOffset, SEEK_SET) != 0)
        return 0;
#endif
    return (U32)fread(buf, 1, len, file);
}

BString FsIso::decodeFilename(const U8* nameData, U8 nameLen) {
    BString name;
    if (joliet) {
        name = utf16BEtoUtf8(nameData, nameLen);
    } else {
        name = BString::copy((const char*)nameData, nameLen);
    }
    int semi = name.indexOf(';');
    if (semi >= 0)
        name = name.substr(0, semi);
    if (name.endsWith('.'))
        name = name.substr(0, name.length() - 1);
    return name;
}

void FsIso::scanDirectory(const BString& localPath, const std::shared_ptr<FsNode>& parentNode, U64 lba, U64 dataLength) {
    if (dataLength == 0 || dataLength > 16 * 1024 * 1024)
        return;

    U32 bufSize = (U32)(((dataLength + ISO_SECTOR_SIZE - 1) / ISO_SECTOR_SIZE) * ISO_SECTOR_SIZE);
    std::vector<U8> buf(bufSize);
    if (readData(lba * ISO_SECTOR_SIZE, buf.data(), bufSize) == 0)
        return;

    U64 offset = 0;
    while (offset < dataLength) {
        U8 recLen = buf[offset];
        if (recLen == 0) {
            // Padding at end of sector — skip to next sector boundary
            U64 nextSector = ((offset / ISO_SECTOR_SIZE) + 1) * ISO_SECTOR_SIZE;
            if (nextSector >= dataLength)
                break;
            offset = nextSector;
            continue;
        }
        if (offset + recLen > dataLength)
            break;

        U32 entryLba    = readU32LE(buf.data() + offset + 2);
        U32 entryLength = readU32LE(buf.data() + offset + 10);
        U8  flags       = buf[offset + 25];
        U8  nameLen     = buf[offset + 32];
        bool isDir      = (flags & ISO_FLAG_DIRECTORY) != 0;
        U64 lastModified = readDirectoryTime(buf.data() + offset);

        // Skip "." (0x00) and ".." (0x01) special entries
        if (nameLen == 1 && (buf[offset + 33] == 0x00 || buf[offset + 33] == 0x01)) {
            offset += recLen;
            continue;
        }

        BString name = decodeFilename(buf.data() + offset + 33, nameLen);
        if (name.length() == 0) {
            offset += recLen;
            continue;
        }

        BString childPath = localPath + "/" + name;

        if (isDir) {
            std::shared_ptr<FsNode> existing = Fs::getNodeFromLocalPath(B(""), childPath, false);
            if (!existing) {
                Fs::addVirtualDirectory(childPath,
                    K__S_IFDIR | K__S_IREAD | K__S_IEXEC | K__S_IRGRP | K__S_IXGRP | K__S_IROTH | K__S_IXOTH,
                    parentNode, lastModified);
                existing = Fs::getNodeFromLocalPath(B(""), childPath, false);
            }
            if (existing)
                scanDirectory(childPath, existing, entryLba, entryLength);
        } else {
            if (!Fs::getNodeFromLocalPath(B(""), childPath, false)) {
                U64 dataOffset = (U64)entryLba * ISO_SECTOR_SIZE;
                U64 fileLength = entryLength;
                std::shared_ptr<FsIso> isoRef = shared_from_this();
                Fs::addVirtualFile(childPath,
                    [dataOffset, fileLength, isoRef](const std::shared_ptr<FsNode>& node, U32 openFlags, U32 data) -> FsOpenNode* {
                        return new FsIsoOpenNode(node, openFlags, dataOffset, fileLength, isoRef);
                    },
                    K__S_IFREG | K__S_IREAD | K__S_IEXEC | K__S_IRGRP | K__S_IXGRP | K__S_IROTH | K__S_IXOTH,
                    0, parentNode, 0, fileLength, lastModified);
            }
        }

        offset += recLen;
    }
}

bool FsIso::init(const BString& isoPath, const BString& mountPath) {
    file = fopen(isoPath.c_str(), "rb");
    if (!file) {
        klog_fmt("FsIso: could not open ISO file: %s", isoPath.c_str());
        return false;
    }

    U8 sector[ISO_SECTOR_SIZE];

    // Read Primary Volume Descriptor at sector 16
    if (readData(16 * ISO_SECTOR_SIZE, sector, ISO_SECTOR_SIZE) != ISO_SECTOR_SIZE) {
        klog_fmt("FsIso: could not read PVD from: %s", isoPath.c_str());
        return false;
    }

    if (sector[0] != ISO_VD_PRIMARY || memcmp(sector + 1, "CD001", 5) != 0) {
        klog_fmt("FsIso: not a valid ISO 9660 image: %s", isoPath.c_str());
        return false;
    }

    totalSectors = readU32LE(sector + 80);

    // Root directory record starts at byte 156 in the PVD
    U32 pvdRootLba    = readU32LE(sector + 158);
    U32 pvdRootLength = readU32LE(sector + 166);

    U32 rootLba    = pvdRootLba;
    U32 rootLength = pvdRootLength;

    // Check subsequent sectors for a Joliet Supplementary Volume Descriptor
    for (U32 sectorNum = 17; sectorNum < 32; sectorNum++) {
        U8 svd[ISO_SECTOR_SIZE];
        if (readData((U64)sectorNum * ISO_SECTOR_SIZE, svd, ISO_SECTOR_SIZE) != ISO_SECTOR_SIZE)
            break;
        if (svd[0] == ISO_VD_TERMINATOR)
            break;
        if (svd[0] != ISO_VD_SUPPLEMENTARY)
            continue;
        if (memcmp(svd + 1, "CD001", 5) != 0)
            continue;

        // Bytes 88-90 contain the Joliet escape sequence
        bool isJoliet = (memcmp(svd + 88, JOLIET_LEVEL1_ESCAPE, 3) == 0 ||
                         memcmp(svd + 88, JOLIET_LEVEL2_ESCAPE, 3) == 0 ||
                         memcmp(svd + 88, JOLIET_LEVEL3_ESCAPE, 3) == 0);
        if (isJoliet) {
            rootLba    = readU32LE(svd + 158);
            rootLength = readU32LE(svd + 166);
            joliet = true;
            klog_fmt("FsIso: Joliet extensions found in %s", isoPath.c_str());
            break;
        }
    }

    // Build the virtual filesystem under mountPath
    Fs::makeLocalDirs(mountPath);
    std::shared_ptr<FsNode> mountNode = Fs::getNodeFromLocalPath(B(""), mountPath, true);
    if (!mountNode) {
        klog_fmt("FsIso: could not create mount point %s", mountPath.c_str());
        return false;
    }

    klog_fmt("FsIso: mounting %s at %s (%s, %llu sectors)",
             isoPath.c_str(), mountPath.c_str(),
             joliet ? "Joliet" : "ISO 9660",
             (unsigned long long)totalSectors);

    scanDirectory(mountPath, mountNode, rootLba, rootLength);
    return true;
}
