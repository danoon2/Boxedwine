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

#ifndef __SOFT_MEMORY_H__
#define __SOFT_MEMORY_H__

#ifndef BOXEDWINE_64BIT_MMU

#include "platform.h"

class KProcess;
class Page;

class CPU;
class DecodedOp;

typedef void (OPCALL *OpCallback)(CPU* cpu, DecodedOp* op);

class Memory {
public:   
    Memory(KProcess* process);
    ~Memory();

    void log_pf(KThread* thread, U32 address);
    void clone(Memory* from);
    void reset();
    void reset(U32 page, U32 pageCount);

    void map(U32 startPage, const std::vector<U8*>& pages, U32 permissions);
    U32 mapNativeMemory(void* buf, U32 len);

    bool findFirstAvailablePage(U32 startingPage, U32 pageCount, U32* result, bool canBeMapped);
    void protectPage(U32 i, U32 permissions);
    void allocPages(U32 page, U32 pageCount, U8 permissions, U32 fd, U64 offset, const BoxedPtr<MappedFile>& mappedFile);
    bool isValidReadAddress(U32 address, U32 len);
    bool isValidWriteAddress(U32 address, U32 len);
    bool isPageAllocated(U32 page);

    KProcess* process;
    Page* mmu[NUMBER_OF_PAGES];    
#ifdef LOG_OPS
    U32 log;
#endif

private:
    static U8* callbackRam;
    static U32 callbackRamPos;
    void addCallback(OpCallback func);
    U8* nativeAddressStart;
};

#endif
#endif