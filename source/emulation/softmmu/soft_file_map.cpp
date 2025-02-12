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

FilePage* FilePage::alloc(U32 key, U32 index) {
    return new FilePage(key, index);
}

// :TODO: what about sync'ing the writes back to the file?
void FilePage::ondemmandFile(U32 address) {
    KThread* thread = KThread::currentThread();
    KMemory* memory = thread->memory;
    KMemoryData* mem = getMemData(memory);
    U32 page = address >> K_PAGE_SHIFT;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->memory->mutex);
    if (mem->getPage(page) != this) {
        return;
    }
    MappedFilePtr mappedFile = thread->process->getMappedFile(this->key);
    U32 mappedOffset = (address - mappedFile->address) & 0xfffff000;
    U64 fileOffset = mappedOffset + mappedFile->offset;
    U32 fileOffsetPage = (U32)(fileOffset >> K_PAGE_SHIFT);
    RamPage ramPage;

    ramPage.value = 0;
    if (mappedFile->systemCacheEntry && fileOffsetPage < (U32)mappedFile->systemCacheEntry->data.size()) {
        ramPage = mappedFile->systemCacheEntry->data[fileOffsetPage];
        if (ramPage.value) {
            ramPageRetain(ramPage);
        }
    }

    if (!ramPage.value) {
        ramPage = ramPageAlloc();
        mappedFile->file->preadNative(ramPageGet(ramPage), fileOffset, K_PAGE_SIZE);
        if (mappedFile->systemCacheEntry && fileOffsetPage < (U32)mappedFile->systemCacheEntry->data.size()) {
            ramPageRetain(ramPage);
            ramPageMarkSystem(ramPage, true);
            mappedFile->systemCacheEntry->data[fileOffsetPage] = ramPage;
        }
    }

    getMemData(memory)->setPageRam(ramPage, page, true);
    ramPageRelease(ramPage); // setPageRam will retain
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

U8* FilePage::getRamPtr(KMemory* memory, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (force && ((memory->canRead(page) && !write) || ((memory->canWrite(page) && write)))) {
        KMemoryData* data = getMemData(memory);
        ondemmandFile((page << K_PAGE_SHIFT) + offset);
        return data->getPage(page)->getRamPtr(memory, page, write, force, offset, len);
    }
    return nullptr;
}