#include "boxedwine.h"

#include "soft_code_page.h"
#include "kmemory_soft.h"

void CodePage::copyOnWrite(MemInfo& info, U32 page) {        
    if (!(info.flags & PAGE_SHARED) && ramPageUseCount(info.ramPageIndex) > 1) {
        if (info.type != (U32)PageType::Code_Page) {
            return;
        }

        KMemory* memory = KThread::currentThread()->memory;
        KMemoryData* mem = getMemData(memory);
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);

        RamPage ram = ramPageAlloc();
        memcpy(ramPageGet(ram), ramPageGet(info.ramPageIndex), K_PAGE_SIZE);
        ramPageRelease(info.ramPageIndex);
        info.ramPageIndex = ram;
    }
}

void CodePage::writeb(MemInfo& info, U32 address, U8 value) {
    if (!(info.flags & PAGE_WRITE)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    if (value!=this->readb(info, address)) {
        KMemory* memory = KThread::currentThread()->memory;
        memory->removeCodeBlock(address, 1);
        copyOnWrite(info, address >> K_PAGE_SHIFT);        
        ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK] = value;
    }
}

void CodePage::writew(MemInfo& info, U32 address, U16 value) {
    if (!(info.flags & PAGE_WRITE)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    if (value!=this->readw(info, address)) {
        KMemory* memory = KThread::currentThread()->memory;
        if (readb(info, address) != (U8)value) {
            memory->removeCodeBlock(address, 1);
        }
        if (readb(info, address+1) != (U8)(value >> 8)) {
            memory->removeCodeBlock(address+1, 1);
        }
        copyOnWrite(info, address >> K_PAGE_SHIFT);
#ifdef UNALIGNED_MEMORY
        U8* ram = getRamPageAtIndex(info.ramPageIndex);
        U32 offset = address & K_PAGE_MASK;
        ram[offset] = (U8)value;
        ram[offset + 1] = (U8)(value >> 8);
#else
        * (U16*)(&ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK]) = value;
#endif
    }
}

void CodePage::writed(MemInfo& info, U32 address, U32 value) {
    if (!(info.flags & PAGE_WRITE)) {
        KThread::currentThread()->seg_access(address, false, true);
    }
    if (value!=this->readd(info, address)) {
        KMemory* memory = KThread::currentThread()->memory;
        if (readb(info, address) != (U8)value) {
            memory->removeCodeBlock(address, 1);
        }
        if (readb(info, address + 1) != (U8)(value >> 8)) {
            memory->removeCodeBlock(address + 1, 1);
        }
        if (readb(info, address + 2) != (U8)(value >> 16)) {
            memory->removeCodeBlock(address + 2, 1);
        }
        if (readb(info, address + 3) != (U8)(value >> 24)) {
            memory->removeCodeBlock(address + 3, 1);
        }
        copyOnWrite(info, address >> K_PAGE_SHIFT);
#ifdef UNALIGNED_MEMORY
        U8* ram = getRamPageAtIndex(info.ramPageIndex);
        U32 offset = address & K_PAGE_MASK;
        this->ram[offset] = (U8)value;
        this->ram[offset + 1] = (U8)(value >> 8);
        this->ram[offset + 2] = (U8)(value >> 16);
        this->ram[offset + 3] = (U8)(value >> 24);
#else
        * (U32*)(&ramPageGet(info.ramPageIndex)[address & K_PAGE_MASK]) = value;
#endif
    }
}

U8* CodePage::getWritePtr(KMemory* memory, MemInfo& info, U32 address, U32 len, bool makeReady) {
    if (memory->canWrite(address >> K_PAGE_SHIFT) && makeReady) {
        memory->removeCodeBlock(address & 0xFFFFF000, K_PAGE_SIZE);
        copyOnWrite(info, address >> K_PAGE_SHIFT);
        return ramPageGet(info.ramPageIndex);
    }
    return nullptr;
}
