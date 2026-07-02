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

class FsIso : public std::enable_shared_from_this<FsIso> {
public:
    FsIso() = default;
    ~FsIso();

    bool init(const BString& isoPath, const BString& mountPath);
    U32 readData(U64 byteOffset, U8* buf, U32 len);

    U64 totalSectors = 0;
    BOXEDWINE_MUTEX readMutex;

private:
    FILE* file = nullptr;
    bool joliet = false;

    void scanDirectory(const BString& localPath, const std::shared_ptr<FsNode>& parentNode, U64 lba, U64 dataLength);
    BString decodeFilename(const U8* nameData, U8 nameLen);
};

#endif
