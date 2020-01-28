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

#ifndef __HARD_MEMORY_H__
#define __HARD_MEMORY_H__

#ifdef BOXEDWINE_64BIT_MMU

#define NATIVE_FLAG_COMMITTED 0x01
#define NATIVE_FLAG_CODEPAGE_READONLY 0x02

INLINE void* getNativeAddress(Memory* memory, U32 address) {
#ifdef _DEBUG
    if (!memory->isPageAllocated(address >> K_PAGE_SHIFT)) {
        memory->log_pf(KThread::currentThread(), KThread::currentThread()->cpu->eip.u32);
        kpanic("bad memory access");
    }
#endif
    return (void*)(address + memory->id);
}

INLINE U32 getHostAddress(KThread* thread, void* address) {
    return (U32)(size_t)address; // size_t because of xcode
}

void reserveNativeMemory(Memory* memory);
void releaseNativeMemory(Memory* memory);
void allocNativeMemory(Memory* memory, U32 page, U32 pageCount, U32 flags);
void freeNativeMemory(Memory* memory, U32 page, U32 pageCount);
void makeCodePageReadOnly(Memory* memory, U32 page);
bool clearCodePageReadOnly(Memory* memory, U32 page);

#endif
#endif
