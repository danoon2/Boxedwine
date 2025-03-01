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

#ifndef __FSVIRTUALNODE_H__
#define __FSVIRTUALNODE_H__

#include "fsnode.h"
#include "fs.h"

class FsVirtualNode : public FsNode {
public:
    FsVirtualNode(U32 id, U32 rdev, BString path, std::function<FsOpenNode* (const std::shared_ptr<FsNode>& node, U32 flags, U32 data)> openFunc, U32 mode, std::shared_ptr<FsNode> parent, U32 openData=0) : FsNode(Type::Virtual, id, rdev, path, B(""), B(""), false, parent), mode(mode), openData(openData), openFunc(openFunc) {}

    // from FsNode
    U32 rename(BString path) override; //return 0 if success, else errno
    bool remove() override;
    U64 lastModified() override;
    U64 length() override;
    FsOpenNode* open(U32 flags) override;
    U32 getType(bool checkForLink) override;
    U32 getMode() override;
    U32 removeDir() override;
    U32 setTimes(U64 lastAccessTime, U32 lastAccessTimeNano, U64 lastModifiedTime, U32 lastModifiedTimeNano) override;

    BString data;
private:
    const U32 mode;
    const U32 openData;
    std::function<FsOpenNode* (const std::shared_ptr<FsNode>& node, U32 flags, U32 data)> openFunc;
};

#endif