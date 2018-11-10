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

#ifdef BOXEDWINE_DEFAULT_MMU

inline U8 readb(U32 address) {
    int index = address >> 12;
#ifdef LOG_OPS
    KThread* thread = KThread::currentThread();
    Memory* memory = thread->memory;
    U8 result = thread->memory->mmu[index]->readb(address);;
    if (memory->log && thread->cpu->log)
        fprintf(logFile, "readb %X @%X\n", result, address);
    return result;
#else
    if (Memory::currentMMUReadPtr[index])
        return Memory::currentMMUReadPtr[index][address & 0xFFF];
    return Memory::currentMMU[index]->readb(address);
#endif
}

inline void writeb(U32 address, U8 value) {
    int index = address >> 12;
#ifdef LOG_OPS
    if (memory->log && thread->cpu->log)
        fprintf(logFile, "writeb %X @%X\n", value, address);
#endif
    if (Memory::currentMMUWritePtr[index])
        Memory::currentMMUWritePtr[index][address & 0xFFF] = value;
    else
        Memory::currentMMU[index]->writeb(address, value);
}

inline U16 readw(U32 address) {
#ifdef LOG_OPS
    U16 result;
    KThread* thread = KThread::currentThread();
    Memory* memory = thread->memory;

    if((address & 0xFFF) < 0xFFF) {
        result = thread->memory->mmu[index]->readw(address);
    } else {
        result = readb(address) | (readb(address+1) << 8);
    }
    if (memory->log && thread->cpu->log)
        fprintf(logFile, "readw %X @%X\n", result, address);
    return result;
#else
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (Memory::currentMMUReadPtr[index])
            return *(U16*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
#endif
        return Memory::currentMMU[index]->readw(address);
    }
    return readb(address) | (readb(address+1) << 8);
#endif
}

inline void writew(U32 address, U16 value) {
#ifdef LOG_OPS
    KThread* thread = KThread::currentThread();
    Memory* memory = thread->memory;
    if (memory->log && thread->cpu->log)
        fprintf(logFile, "writew %X @%X\n", value, address);
#endif
    if ((address & 0xFFF) < 0xFFF) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (Memory::currentMMUWritePtr[index])
            *(U16*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            Memory::currentMMU[index]->writew(address, value);
    } else {
        writeb(address, (U8)value);
        writeb(address+1, (U8)(value >> 8));
    }
}

inline U32 readd(U32 address) {
#ifdef LOG_OPS
    U32 result;
    KThread* thread = KThread::currentThread();
    Memory* memory = thread->memory;

    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
        result = thread->memory->mmu[index]->readd(address);
    } else {
        result = readb(address) | (readb(address+1) << 8) | (readb(address+2) << 16) | (readb(address+3) << 24);
    }
    if (memory->log && thread->cpu->log)
        fprintf(logFile, "readd %X @%X\n", result, address);
    return result;
#else
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (Memory::currentMMUReadPtr[index])
            return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
#endif
        return Memory::currentMMU[index]->readd(address);
    } else {
        return readb(address) | (readb(address+1) << 8) | (readb(address+2) << 16) | (readb(address+3) << 24);
    }
#endif
}

inline void writed(U32 address, U32 value) {
#ifdef LOG_OPS
    KThread* thread = KThread::currentThread();
    Memory* memory = thread->memory;
    if (memory->log && thread->cpu->log)
        fprintf(logFile, "writed %X @%X\n", value, address);
#endif    
    if ((address & 0xFFF) < 0xFFD) {
        int index = address >> 12;
#ifndef UNALIGNED_MEMORY
        if (Memory::currentMMUWritePtr[index])
            *(U32*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
        else
#endif
            Memory::currentMMU[index]->writed(address, value);		
    } else {
        writeb(address, value);
        writeb(address+1, value >> 8);
        writeb(address+2, value >> 16);
        writeb(address+3, value >> 24);
    }
}

inline U64 readq(U32 address) {
#ifndef UNALIGNED_MEMORY
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        if (Memory::currentMMUWritePtr[index]) {
            return *(U64*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
        }
    }
#endif
    return readd(address) | ((U64)readd(address + 4) << 32);
}

inline void writeq(U32 address, U64 value) {
#ifndef UNALIGNED_MEMORY
    if ((address & 0xFFF) < 0xFF9) {
        int index = address >> 12;
        if (Memory::currentMMUWritePtr[index]) {
            *(U64*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
            return;
        }
    }
#endif
    writed(address, (U32)value); writed(address + 4, (U32)(value >> 32));
}
#endif
#endif
