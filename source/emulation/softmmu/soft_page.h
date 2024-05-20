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

class KThread;
class KMemory;

class Page {
public:
    enum class Type {
        Invalid_Page,
        RW_Page,
        RO_Page,
        WO_Page,
        NO_Page,
        File_Page,
        Code_Page,
        Copy_On_Write_Page,
        Native_Page,
        Frame_Buffer_Page
    };
    virtual ~Page() {};

    virtual U8 readb(U32 address)=0;
    virtual void writeb(U32 address, U8 value)=0;
    virtual U16 readw(U32 address)=0;
    virtual void writew(U32 address, U16 value)=0;
    virtual U32 readd(U32 address)=0;
    virtual void writed(U32 address, U32 value)=0;

    // these two take memory argument so that they won't call KThread::current thread, this makes them safe to call from the audio thread
    virtual U8* getReadPtr(KMemory* memory, U32 address, bool makeReady = false)=0; // might have permission, but may not ready
    virtual U8* getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady = false)=0; // might have permission, but may not be ready

    virtual bool inRam()=0;
    virtual void close() = 0;    
    virtual Type getType() = 0;    
};

#endif
