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

#ifndef __PAGE_H__
#define __PAGE_H__

#include "platform.h"
#include "soft_ram.h"
#include "pageType.h"

class KMemory;

#include "meminfostruct.h"

class Page {
public:    
    virtual U8 readb(MemInfo& info, U32 address)=0;
    virtual void writeb(MemInfo& info, U32 address, U8 value)=0;
    virtual U16 readw(MemInfo& info, U32 address)=0;
    virtual void writew(MemInfo& info, U32 address, U16 value)=0;
    virtual U32 readd(MemInfo& info, U32 address)=0;
    virtual void writed(MemInfo& info, U32 address, U32 value)=0;

    virtual void onDemand(KMemory* memory, MemInfo& info, U32 address) = 0;

    // these two take memory argument so that they won't call KThread::current thread, this makes them safe to call from the audio thread
    virtual U8* getReadPtr(KMemory* memory, MemInfo& info, U32 address, bool makeReady = false)=0; // might have permission, but may not ready
    virtual U8* getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady = false)=0; // might have permission, but may not be ready   
};

#endif
