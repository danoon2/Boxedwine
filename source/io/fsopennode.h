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

#ifndef __FSOPENNODE_H__
#define __FSOPENNODE_H__

#include "platform.h"
#include "kthread.h"

class FsOpenNode {
public:
    FsOpenNode(std::shared_ptr<FsNode> node, U32 flags);

    U32 read(KThread* thread, U32 address, U32 len); // will call into readNative
    U32 write(KThread* thread, U32 address, U32 len); // will call into writeNative

    U32 getDirectoryEntryCount();
    std::shared_ptr<FsNode> getDirectoryEntry(U32 index, BString& name);

    virtual ~FsOpenNode();

    virtual S64  length()=0;
    virtual bool setLength(S64 length)=0;
    virtual S64  getFilePointer()=0;
    virtual S64  seek(S64 pos)=0;	    
    virtual U32  map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off)=0;
    virtual bool canMap()=0;
    virtual U32  ioctl(KThread* thread, U32 request)=0;	
    virtual void setAsync(bool isAsync)=0;
    virtual bool isAsync()=0;
    virtual void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events)=0;
    virtual bool isWriteReady()=0;
    virtual bool isReadReady()=0;    
    virtual U32 readNative(U8* buffer, U32 len)=0;
    virtual U32 writeNative(U8* buffer, U32 len)=0;
    virtual void close()=0;
    virtual void reopen()=0;
    virtual bool isOpen()=0;
    virtual BOXEDWINE_MUTEX* getReadMutex() {return nullptr;}

    std::shared_ptr<FsNode> const node;
    const U32 flags;     
    BString openedPath; // when call fchdir, we should set the current directory to what was passed in, not what was linked to

private:
    std::vector<std::shared_ptr<FsNode> > dirEntries;
    void loadDirEntries();
    U32 internalRead(KThread* thread, U32 address, U32 len);

    friend FsNode;
    KListNode<FsOpenNode*> listNode;
};

#endif
