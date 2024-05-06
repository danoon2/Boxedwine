/*
 *  Copyright (C) 2016  The BoxedWine Team
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

#ifndef __BUFFERACCESS_H__
#define __BUFFERACCESS_H__

class BufferAccess : public FsOpenNode {
public:
    BufferAccess(const std::shared_ptr<FsNode>& node, U32 flags, const BString& buffer);
    BufferAccess(const std::shared_ptr<FsNode>& node, U32 flags, BString* buffer);

    // from FsOpenNode
    S64  length() override;
    bool setLength(S64 length) override;
    S64  getFilePointer() override;
    S64  seek(S64 pos) override;
    U32  map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override;
    bool canMap() override;
    U32  ioctl(KThread* thread, U32 request) override;
    void setAsync(bool isAsync) override;
    bool isAsync() override;
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override;
    bool isWriteReady() override;
    bool isReadReady() override;
    U32 readNative(U8* buffer, U32 len) override;
    U32 writeNative(U8* buffer, U32 len) override;
    void close() override;
    void reopen() override;
    bool isOpen() override;
    
private:
    BString* buffer;
    BString storage;
    S32 pos;
};

void makeBufferAccess(struct FsOpenNodeFunc* nodeAccess);

#endif