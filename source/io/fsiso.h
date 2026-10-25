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

#ifndef __FSISO_H__
#define __FSISO_H__

#include "platformBoxedwine.h"

#define ISO_SECTOR_SIZE 2048

class DiscImage;

// The ISO 9660 filesystem on a disc image (an ISO, or a CUE sheet with its BIN files),
// mounted as virtual nodes
class FsIso : public std::enable_shared_from_this<FsIso> {
public:
    FsIso() = default;

    // With preparedRoot, validate/build a detached tree; the caller publishes it
    // only after validation succeeds, so a bad replacement cannot eject a disc.
    bool init(const BString& imagePath, const BString& mountPath, std::shared_ptr<FsNode>* preparedRoot = nullptr);
    // reads the disc's data sectors, see DiscImage::readData
    U32 readData(U64 byteOffset, U8* buf, U32 len);
    // reads whole raw sectors, such as CD audio, see DiscImage::readRaw
    bool readRaw(U32 sector, U32 count, U8* buf);
    // true for the mount point itself and everything below it
    bool containsPath(const BString& path) const;

    U64 totalSectors = 0; // size of the ISO 9660 filesystem
    BOXEDWINE_MUTEX readMutex;
    std::shared_ptr<DiscImage> image;

private:
    bool joliet = false;
    BString mountPath;

    bool scanDirectory(const BString& localPath, const std::shared_ptr<FsNode>& parentNode, U64 lba, U64 dataLength, std::vector<U64> ancestors);
    BString decodeFilename(const U8* nameData, U8 nameLen);
};

#endif
