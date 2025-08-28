/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#include "soft_code_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"
#include "soft_mmu.h"


void CodePage::onDemmand(MMU* mmu, U32 pageIndex) {
    KThread* thread = KThread::currentThread();
    if (!thread->memory->mapShared(pageIndex) && ramPageUseCount((RamPage)mmu->ramIndex) > 1) {
        RamPage ram = ramPageAlloc();
        ::memcpy(ramPageGet(ram), ramPageGet((RamPage)mmu->ramIndex), K_PAGE_SIZE);
        mmu->setPage(thread->memory, pageIndex, PageType::Code, ram);
        getMemData(thread->memory)->onPageChanged(pageIndex);
    }
}

void CodePage::writeb(MMU* mmu, U32 address, U8 value) {
    if (!RWPage::canWriteRam(mmu)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    if (value != this->readb(mmu, address)) {
        KThread* thread = KThread::currentThread();
        KMemory* memory = thread->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        U8 currentValue = this->readb(mmu, address);
        if (currentValue == value) {
            return;
        }
        bool currentJitRemoved = memory->removeCode(address, 1, true);
        onDemmand(mmu, address >> K_PAGE_SHIFT);
        Page::getRWPage()->writeb(mmu, address, value);
        if (currentJitRemoved) {
            thread->cpu->nextOp = thread->cpu->getNextOp()->next;
        }
    }
}

void CodePage::writew(MMU* mmu, U32 address, U16 value) {
    if (!RWPage::canWriteRam(mmu)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    U16 currentValue = this->readw(mmu, address);
    if (value != currentValue) {
        KThread* thread = KThread::currentThread();
        KMemory* memory = thread->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        currentValue = this->readw(mmu, address);
        if (value == currentValue) {
            return;
        }
        U32 startAddress = 0;
        U32 len = 0;
        if ((U8)currentValue != (U8)value) {
            startAddress = address;
            len++;
        }
        if ((U8)(currentValue >> 8) != (U8)(value >> 8)) {
            if (!startAddress) {
                startAddress = address + 1;
            }
            len++;
        }
        bool currentJitRemoved = memory->removeCode(startAddress, len, true);
        onDemmand(mmu, address >> K_PAGE_SHIFT);
        RWPage::writew(mmu, address, value);
        if (currentJitRemoved) {
            thread->cpu->nextOp = thread->cpu->getNextOp()->next;
        }
    }
}

void CodePage::writed(MMU* mmu, U32 address, U32 value) {
    if (!RWPage::canWriteRam(mmu)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    U32 currentValue = this->readd(mmu, address);
    if (value != currentValue) {
        KThread* thread = KThread::currentThread();
        KMemory* memory = thread->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        currentValue = this->readd(mmu, address);
        if (value == currentValue) {
            return;
        }
        U32 startAddress = 0;
        U32 endAddress = 0;
        if ((U8)currentValue != (U8)value) {
            startAddress = address;
            endAddress = address;
        }
        if ((U8)(currentValue >> 8) != (U8)(value >> 8)) {
            if (!startAddress) {
                startAddress = address + 1;
            }
            endAddress = address + 1;
        }
        if ((U8)(currentValue >> 16) != (U8)(value >> 16)) {
            if (!startAddress) {
                startAddress = address + 2;
            }
            endAddress = address + 2;
        }
        if ((U8)(currentValue >> 24) != (U8)(value >> 24)) {
            if (!startAddress) {
                startAddress = address + 3;
            }
            endAddress = address + 3;
        }
        bool currentJitRemoved = memory->removeCode(startAddress, endAddress - startAddress + 1, true);
        onDemmand(mmu, address >> K_PAGE_SHIFT);
        RWPage::writed(mmu, address, value);
        if (currentJitRemoved) {
            thread->cpu->nextOp = thread->cpu->getNextOp()->next;
        }
    }
}

bool CodePage::canWriteRam(MMU* mmu) {
    return false;
}

U8* CodePage::getRamPtr(MMU* mmu, U32 page, bool write, bool force, U32 offset, U32 len) {
    if (!write) {
        return Page::getRWPage()->getRamPtr(mmu, page, write, force, offset, len);
    }
    if (RWPage::canWriteRam(mmu) && force) {
        if (!len && !offset) {
            len = K_PAGE_SIZE;
        }
        KThread* thread = KThread::currentThread();
        KMemory* memory = thread->memory;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        bool currentJitRemoved = memory->removeCode((page << K_PAGE_SHIFT) + offset, len, true);
        onDemmand(mmu, page);
        if (currentJitRemoved) {
            thread->cpu->nextOp = thread->cpu->getNextOp()->next;
        }
        return Page::getRWPage()->getRamPtr(mmu, page, write, force, offset, len);
    }
    return nullptr;
}