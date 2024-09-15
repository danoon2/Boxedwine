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

void FilePage::onDemand(KMemory* memory, MemInfo& info, U32 address) {
    KThread* thread = KThread::currentThread();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
    U32 page = address >> K_PAGE_SHIFT;
    MappedFilePtr mappedFile = thread->process->getMappedFile(info.ramPageIndex);
    U32 mappedOffset = (address - mappedFile->address) & 0xfffff000;
    U64 fileOffset = mappedOffset + mappedFile->offset;
    U32 fileOffsetPage = (U32)(fileOffset >> K_PAGE_SHIFT);
    RamPage ramPage = 0;

    if (info.type != (U32)PageType::File_Page) {
        return;
    }    
    U32 pageType = (U32)PageType::RAM_Page;

    if (mappedFile->systemCacheEntry && fileOffsetPage < (U32)mappedFile->systemCacheEntry->ramPages.size()) {
        ramPage = mappedFile->systemCacheEntry->ramPages[fileOffsetPage];
        if (ramPage) {
            ramPageRetain(ramPage);
            pageType = (U32)PageType::Copy_On_Write_Page;
        }
    }

    if (!ramPage) {
        ramPage = ramPageAlloc();
        mappedFile->file->preadNative(ramPageGet(ramPage), fileOffset, K_PAGE_SIZE);
        if (mappedFile->systemCacheEntry && fileOffsetPage < (U32)mappedFile->systemCacheEntry->ramPages.size()) {
            ramPageRetain(ramPage);
            mappedFile->systemCacheEntry->ramPages[fileOffsetPage] = ramPage;
            pageType = (U32)PageType::Copy_On_Write_Page;
        }
    }
    info.type = (U32)pageType;
    info.ramPageIndex = ramPage;    
    info.updatePermissionCache();
}

U8 FilePage::readb(MemInfo& info, U32 address) {
    KMemory* memory = KThread::currentThread()->memory;
    onDemand(memory, info, address);
    return memory->readb(address);
}

void FilePage::writeb(MemInfo& info, U32 address, U8 value) {
    KMemory* memory = KThread::currentThread()->memory;
    onDemand(memory, info, address);
    memory->writeb(address, value);
}

U16 FilePage::readw(MemInfo& info, U32 address) {
    KMemory* memory = KThread::currentThread()->memory;
    onDemand(memory, info, address);
    return memory->readw(address);
}

void FilePage::writew(MemInfo& info, U32 address, U16 value) {
    KMemory* memory = KThread::currentThread()->memory;
    onDemand(memory, info, address);
    memory->writew(address, value);
}

U32 FilePage::readd(MemInfo& info, U32 address) {
    KMemory* memory = KThread::currentThread()->memory;
    onDemand(memory, info, address);
    return memory->readd(address);
}

void FilePage::writed(MemInfo& info, U32 address, U32 value) {
    KMemory* memory = KThread::currentThread()->memory;
    onDemand(memory, info, address);
    memory->writed(address, value);
}

U8* FilePage::getReadPtr(KMemory* memory, MemInfo& info, U32 address, bool makeReady) {
    if (makeReady && memory->canRead(address >> K_PAGE_SHIFT)) {
        KMemoryData* data = getMemData(memory);
        onDemand(memory, info, address);
        return data->getPage(address >> K_PAGE_SHIFT)->getReadPtr(memory, info, address, true);
    }
    return nullptr;
}

U8* FilePage::getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady) {
    if (makeReady && memory->canWrite(address >> K_PAGE_SHIFT)) {
        KMemoryData* data = getMemData(memory);
        onDemand(memory, info, address);
        return data->getPage(address >> K_PAGE_SHIFT)->getWritePtr(memory, info, address, len, true);
    }
    return nullptr;
}
