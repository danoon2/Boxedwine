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

#ifndef __MEMORY_H__
#define __MEMORY_H__

class KFile;

class MappedFileCache : public BoxedPtrBase {
public:
    MappedFileCache(const std::string& name) : name(name) {}
    virtual ~MappedFileCache();
    const std::string name;
    BoxedPtr<KFile> file;
    U8** data;
};

#define PAGE_SIZE 4096
#define PAGE_MASK 0xFFF
#define PAGE_SHIFT 12
#define NUMBER_OF_PAGES 0x100000
#define ROUND_UP_TO_PAGE(x) ((x + 0xFFF) & 0xFFFFF000)

class Memory;
class KProcess;
class KThread;
class MappedFile;

U8 readb(U32 address);
void writeb(U32 address, U8 value);
U16 readw(U32 address);
void writew(U32 address, U16 value);
U32 readd(U32 address);
void writed(U32 address, U32 value);

INLINE U64 readq(U32 address) {return readd(address) | ((U64)readd(address + 4) << 32);}
INLINE void writeq(U32 address, U64 value) {writed(address, (U32)value); writed(address + 4, (U32)(value >> 32));}

void zeroMemory(U32 address, int len);
void readMemory(U8* data, U32 address, int len);
void writeMemory(U32 address, U8* data, int len);

U8* getPhysicalAddress(U32 address);

char* getNativeString(U32 address, char* buffer, U32 cbBuffer);
char* getNativeStringW(U32 address, char* buffer, U32 cbBuffer);
void writeNativeString(U32 address, const char* str);
U32 writeNativeString2(U32 address, const char* str, U32 len);
void writeNativeStringW(U32 address, const char* str);
U32 getNativeStringLen(U32 address);

void memcopyFromNative(U32 address, const char* p, U32 len);
void memcopyToNative(U32 address, char* p, U32 len);

#include "../source/emulation/softmmu/soft_memory.h"
#include "../source/emulation/softmmu/soft_page.h"
#endif
