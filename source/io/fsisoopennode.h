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

#ifndef __FSISOOPENNODE_H__
#define __FSISOOPENNODE_H__

#include "fsopennode.h"

class FsIso;

class FsIsoOpenNode : public FsOpenNode {
public:
    FsIsoOpenNode(const std::shared_ptr<FsNode>& node, U32 flags, U64 dataOffset, U64 fileLength, const std::shared_ptr<FsIso>& iso);

    S64  length() override;
    bool setLength(S64 length) override { return false; }
    S64  getFilePointer() override;
    S64  seek(S64 pos) override;
    U32  map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override { return 0; }
    bool canMap() override { return false; }
    U32  ioctl(KThread* thread, U32 request) override;
    void setAsync(bool isAsync) override {}
    bool isAsync() override { return false; }
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override {}
    bool isWriteReady() override { return false; }
    bool isReadReady() override { return true; }
    U32  readNative(U8* buffer, U32 len) override;
    U32  writeNative(U8* buffer, U32 len) override { return 0; }
    void close() override {}
    void reopen() override { pos = 0; }
    bool isOpen() override { return true; }
    BOXEDWINE_MUTEX* getReadMutex() override;

private:
    std::shared_ptr<FsIso> iso;
    U64 dataOffset;
    U64 fileLength;
    U64 pos = 0;
};

#endif
