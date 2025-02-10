#include "boxedwine.h"

#include "soft_code_page.h"
#include "soft_ram.h"
#include "kmemory_soft.h"
#include "../cpu/normal/normalCPU.h"
    
CodePage* CodePage::alloc(RamPage page, U32 address) {
    return new CodePage(page, address);
}

CodePage::CodePage(RamPage page, U32 address) : RWPage(page, address) {
}

CodePage::~CodePage() {
}

void CodePage::close() {
    KThread::currentThread()->memory->removeCodeBlock(address, K_PAGE_SIZE);
    RWPage::close();
}

void CodePage::copyOnWrite() {
    if (!KThread::currentThread()->memory->mapShared(address >> K_PAGE_SHIFT) && ramPageUseCount(page) > 1) {
        RamPage ram = ramPageAlloc();
        ::memcpy(ramPageGet(ram), ramPageGet(page), K_PAGE_SIZE);
        page = ram;
        getMemData(KThread::currentThread()->memory)->setPage(address >> K_PAGE_SHIFT, this);
        ramPageRelease(ram); // setPage will retain
    }
}

void CodePage::writeb(U32 address, U8 value) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    if (value!=this->readb(address)) {
        KMemory* memory = KThread::currentThread()->memory;
        memory->removeCodeBlock(address, 1);
        copyOnWrite();
        RWPage::writeb(address, value);
    }
}

void CodePage::writew(U32 address, U16 value) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U16 currentValue = this->readw(address);
    if (value != currentValue) {
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
        KMemory* memory = KThread::currentThread()->memory;
        memory->removeCodeBlock(startAddress, len);
        copyOnWrite();
        RWPage::writew(address, value);
    }
}

void CodePage::writed(U32 address, U32 value) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mutex);
    U32 currentValue = this->readd(address);
    if (value!= currentValue) {
        U32 startAddress = 0;
        U32 endAddress = 0;

        if ((U8)currentValue != (U8)value) {
            startAddress = address;
            endAddress = address;
        }
        if ((currentValue & 0xFF00) != (value & 0xFF00)) {
            if (!startAddress) {
                startAddress = address + 1;
            }
            endAddress = address + 1;
        }
        if ((currentValue & 0xFF0000) != (value & 0xFF0000)) {
            if (!startAddress) {
                startAddress = address + 2;
            }
            endAddress = address + 2;
        }
        if ((currentValue & 0xFF000000) != (value & 0xFF000000)) {
            if (!startAddress) {
                startAddress = address + 3;
            }
            endAddress = address + 3;
        }
        KMemory* memory = KThread::currentThread()->memory;
        memory->removeCodeBlock(startAddress, endAddress - startAddress + 1);
        copyOnWrite();
        RWPage::writed(address, value);
    }
}

U8* CodePage::getReadPtr(KMemory* memory, U32 address, bool makeReady) {
    if (memory->canRead(address >> K_PAGE_SHIFT)) {
        return this->ram;
    }
    return nullptr;
}

U8* CodePage::getWritePtr(KMemory* memory, U32 address, U32 len, bool makeReady) {
    if (memory->canWrite(address >> K_PAGE_SHIFT) && makeReady) {
        KMemory* memory = KThread::currentThread()->memory;
        memory->removeCodeBlock(address, len);
        return this->ram;
    }
    return nullptr;
}
