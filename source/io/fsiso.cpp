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
#include "discimage.h"
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

U32 FsIso::readData(U64 byteOffset, U8* buf, U32 len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(readMutex);
    return image->readData(byteOffset, buf, len);
}

bool FsIso::readRaw(U32 sector, U32 count, U8* buf) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(readMutex);
    return image->readRaw(sector, count, buf);
}

bool FsIso::containsPath(const BString& path) const {
    if (mountPath.isEmpty() || !path.startsWith(mountPath))
        return false;
    return path.length() == mountPath.length() || path.charAt(mountPath.length()) == '/';
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

bool FsIso::scanDirectory(const BString& localPath, const std::shared_ptr<FsNode>& parentNode, U64 lba, U64 dataLength, std::vector<U64> ancestors) {
    if (dataLength == 0 || dataLength > 16 * 1024 * 1024 ||
            ancestors.size() >= 64 || std::find(ancestors.begin(), ancestors.end(), lba) != ancestors.end())
        return false;
    ancestors.push_back(lba);

    U32 bufSize = (U32)(((dataLength + ISO_SECTOR_SIZE - 1) / ISO_SECTOR_SIZE) * ISO_SECTOR_SIZE);
    std::vector<U8> buf(bufSize);
    if (readData(lba * ISO_SECTOR_SIZE, buf.data(), bufSize) < dataLength)
        return false;

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
        // A record contains 33 fixed bytes and at least one filename byte, and
        // must fit within both the directory extent and its logical sector.
        if (recLen < 34 || offset + recLen > dataLength ||
                recLen > ISO_SECTOR_SIZE - offset % ISO_SECTOR_SIZE)
            return false;

        U8 nameLen = buf[offset + 32];
        if (!nameLen || nameLen > recLen - 33)
            return false;

        U32 entryLba    = readU32LE(buf.data() + offset + 2);
        U32 entryLength = readU32LE(buf.data() + offset + 10);
        U8  flags       = buf[offset + 25];
        bool isDir      = (flags & ISO_FLAG_DIRECTORY) != 0;
        U64 lastModified = readDirectoryTime(buf.data() + offset);

        // Skip "." (0x00) and ".." (0x01) special entries
        if (nameLen == 1 && (buf[offset + 33] == 0x00 || buf[offset + 33] == 0x01)) {
            offset += recLen;
            continue;
        }

        BString name = decodeFilename(buf.data() + offset + 33, nameLen);
        if (name == B(".") || name == B("..") || name.indexOf('/') >= 0)
            return false;
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
            if (!existing || !scanDirectory(childPath, existing, entryLba, entryLength, ancestors))
                return false;
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
    return true;
}

bool FsIso::init(const BString& imagePath, const BString& mountPath) {
    image = DiscImage::open(imagePath);
    if (!image) {
        return false;
    }
    std::shared_ptr<FsNode> mountNode = Fs::getNodeFromLocalPath(B(""), mountPath, true);
    if (!mountNode) {
        klog_fmt("FsIso: mount point %s does not exist", mountPath.c_str());
        return false;
    }
    this->mountPath = mountPath;

    // An audio CD has no filesystem, but goes in the drive for its table of contents
    const DiscTrack* dataTrack = image->firstDataTrack();
    if (!dataTrack) {
        totalSectors = image->leadOut;
        klog_fmt("FsIso: mounting %s at %s (audio CD, %d tracks)", imagePath.c_str(), mountPath.c_str(), (U32)image->tracks.size());
        return true;
    }
    // ISO 9660 sector numbers count from the start of the disc, the volume descriptors
    // from the start of the data track
    U64 base = dataTrack->start;

    U8 sector[ISO_SECTOR_SIZE];

    // Read Primary Volume Descriptor at sector 16
    if (readData((base + 16) * ISO_SECTOR_SIZE, sector, ISO_SECTOR_SIZE) != ISO_SECTOR_SIZE) {
        klog_fmt("FsIso: could not read PVD from: %s", imagePath.c_str());
        return false;
    }

    if (sector[0] != ISO_VD_PRIMARY || memcmp(sector + 1, "CD001", 5) != 0) {
        klog_fmt("FsIso: not a valid ISO 9660 image: %s", imagePath.c_str());
        return false;
    }

    totalSectors = readU32LE(sector + 80);

    // Root directory record starts at byte 156 in the PVD
    U32 pvdRootLba    = readU32LE(sector + 158);
    U32 pvdRootLength = readU32LE(sector + 166);

    U32 rootLba    = pvdRootLba;
    U32 rootLength = pvdRootLength;

    // Check subsequent sectors for a Joliet Supplementary Volume Descriptor
    for (U64 sectorNum = base + 17; sectorNum < base + 32; sectorNum++) {
        U8 svd[ISO_SECTOR_SIZE];
        if (readData(sectorNum * ISO_SECTOR_SIZE, svd, ISO_SECTOR_SIZE) != ISO_SECTOR_SIZE)
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
            klog_fmt("FsIso: Joliet extensions found in %s", imagePath.c_str());
            break;
        }
    }

    klog_fmt("FsIso: mounting %s at %s (%s, %llu sectors)",
             imagePath.c_str(), mountPath.c_str(),
             joliet ? "Joliet" : "ISO 9660",
             (unsigned long long)totalSectors);

    if (!scanDirectory(mountPath, mountNode, rootLba, rootLength, {})) {
        // A failed insert must not leave a partial disc visible at the mount point.
        std::vector<std::shared_ptr<FsNode>> children;
        mountNode->getAllChildren(children);
        for (const auto& child : children)
            child->removeNodeFromParent();
        klog_fmt("FsIso: invalid directory in %s", imagePath.c_str());
        return false;
    }
    return true;
}
