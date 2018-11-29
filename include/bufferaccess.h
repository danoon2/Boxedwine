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
    BufferAccess(const BoxedPtr<FsNode>& node, U32 flags, const std::string& buffer);
    virtual S64  length();
    virtual bool setLength(S64 length);
    virtual S64  getFilePointer();
    virtual S64  seek(S64 pos);	
    virtual U32  map(U32 address, U32 len, S32 prot, S32 flags, U64 off);
    virtual bool canMap();
    virtual U32  ioctl(U32 request);	
    virtual void setAsync(bool isAsync);
    virtual bool isAsync();
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events);
    virtual bool isWriteReady();
    virtual bool isReadReady();
    virtual U32 readNative(U8* buffer, U32 len);
    virtual U32 writeNative(U8* buffer, U32 len);
    virtual void close();
    virtual void reopen();
    virtual bool isOpen();

    std::string buffer;
private:
    S32 pos;
};

void makeBufferAccess(struct FsOpenNodeFunc* nodeAccess);

#endif