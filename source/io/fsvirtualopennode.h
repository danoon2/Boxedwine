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

#ifndef __FSVIRTUALOPENNODE_H__
#define __FSVIRTUALOPENNODE_H__

#include "fsopennode.h"
#include "kerror.h"

class FsVirtualOpenNode: public FsOpenNode {
public:
    FsVirtualOpenNode(std::shared_ptr<FsNode> node, U32 flags) : FsOpenNode(node, flags) {};

    // From FsOpenNode
    S64 length() override {return 0;}
    bool setLength(S64 length) override {return true;}
    S64 getFilePointer() override {return 0;}
    S64 seek(S64 pos) override {return 0;}
    U32 map(KThread* thread, U32 address, U32 len, S32 prot, S32 flags, U64 off) override {return 0;}
    bool canMap() override {return false;}
    U32 ioctl(KThread* thread, U32 request) override {return -K_ENODEV;	}
    void setAsync(bool isAsync) override {if (isAsync) kdebug("FsVirtualOpenNode::setAsync not implemented");}
    bool isAsync() override {return false;}
    void waitForEvents(BOXEDWINE_CONDITION& parentCondition, U32 events) override { kdebug("FsVirtualOpenNode::waitForEvents not implemented");}
    bool isWriteReady() override {return true;}
    bool isReadReady() override {return true;}
    U32 readNative(U8* buffer, U32 len) override = 0;
    U32 writeNative(U8* buffer, U32 len) override = 0;
    void close() override {}
    void reopen() override {}
    bool isOpen() override {return true;}
};

#endif