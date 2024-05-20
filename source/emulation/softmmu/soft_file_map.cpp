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
#include "boxedwine.h"

#include "soft_file_map.h"
#include "kmemory_soft.h"
#include "soft_ram.h"

#include "kprocess.h"

FilePage* FilePage::alloc(const std::shared_ptr<MappedFile>& mapped, U32 index) {
    return new FilePage(mapped, index);
}

// :TODO: what about sync'ing the writes back to the file?
void FilePage::ondemmandFile(U32 address) {
    KMemory* memory = KThread::currentThread()->memory;
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    U32 page = address >> K_PAGE_SHIFT;
    KRamPtr ram;

    if (mem->getPage(page) != this) {
        return;
    }

    if (1) {
        if (index < mapped->systemCacheEntry->dataSize) {
            ram = mapped->systemCacheEntry->data[this->index];
        }
    } 
    if (!ram) {
        ram = ramPageAlloc();
        U64 pos = this->mapped->file->getPos();
        this->mapped->file->seek(((U64)this->index) << K_PAGE_SHIFT);
        this->mapped->file->readNative(ram.get(), K_PAGE_SIZE);
        this->mapped->file->seek(pos);
        if (index < mapped->systemCacheEntry->dataSize) {
            mapped->systemCacheEntry->data[this->index] = ram;
        }
    }

    getMemData(memory)->setPageRam(ram, page, true);
}

U8 FilePage::readb(U32 address) {	
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmandFile(address);	
    return data->memory->readb(address);
}

void FilePage::writeb(U32 address, U8 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmandFile(address);	
    data->memory->writeb(address, value);
}

U16 FilePage::readw(U32 address) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmandFile(address);	
    return data->memory->readw(address);
}

void FilePage::writew(U32 address, U16 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmandFile(address);	
    data->memory->writew(address, value);
}

U32 FilePage::readd(U32 address) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmandFile(address);	
    return data->memory->readd(address);
}

void FilePage::writed(U32 address, U32 value) {
    KMemoryData* data = getMemData(KThread::currentThread()->memory);
    ondemmandFile(address);	
    data->memory->writed(address, value);
}

U8* FilePage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    if (makeReady && memory->canRead(address >> K_PAGE_SHIFT)) {
        KMemoryData* data = getMemData(memory);
        ondemmandFile(address);
        return data->getPage(address >> K_PAGE_SHIFT)->getReadPtr(memory, address, true);
    }
    return nullptr;
}

U8* FilePage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    if (makeReady && memory->canWrite(address >> K_PAGE_SHIFT)) {
        KMemoryData* data = getMemData(memory);
        ondemmandFile(address);
        return data->getPage(address >> K_PAGE_SHIFT)->getWritePtr(memory, address, len, true);
    }
    return nullptr;
}
