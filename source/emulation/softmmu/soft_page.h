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

class Memory;
class KThread;

#define PAGE_READ 0x01
#define PAGE_WRITE 0x02
#define PAGE_EXEC 0x04
#define PAGE_SHARED 0x08
#define PAGE_MAPPED 0x20
#define PAGE_PERMISSION_MASK 0x07

#define GET_PAGE_PERMISSIONS(flags) (flags & PAGE_PERMISSION_MASK)

class Page {
public:
    enum Type {
        Invalid_Page,
        On_Demand_Page,
        RW_Page,
        RO_Page,
        WO_Page,
        NO_Page,
        File_Page,
        Code_Page,
        Copy_On_Write_Page,
        Frame_Buffer,
        Native_Page
    };
    Page(Type type, U32 flags) : flags(flags), type(type) {}
    virtual ~Page() {};

    virtual U8 readb(U32 address)=0;
    virtual void writeb(U32 address, U8 value)=0;
    virtual U16 readw(U32 address)=0;
    virtual void writew(U32 address, U16 value)=0;
    virtual U32 readd(U32 address)=0;
    virtual void writed(U32 address, U32 value)=0;
    virtual U8* getCurrentReadPtr()=0; // might have permission, but may not ready
    virtual U8* getCurrentWritePtr()=0; // might have permission, but may not be ready
    virtual U8* getReadAddress(U32 address, U32 len)=0; // if has permission, will make ready 
    virtual U8* getWriteAddress(U32 address, U32 len)=0; // if has permission, will make ready
    virtual U8* getReadWriteAddress(U32 address, U32 len)=0; // if has permission, will make ready
    virtual bool inRam()=0;
    virtual void close() = 0;

    bool canRead() {return (this->flags & PAGE_READ)!=0;}
    bool canWrite() {return (this->flags & PAGE_WRITE)!=0;}
    bool canExec() {return (this->flags & PAGE_EXEC)!=0;}
    bool mapShared() {return (this->flags & PAGE_SHARED)!=0;}

    U8 flags;
    const Type type;
};

#endif
