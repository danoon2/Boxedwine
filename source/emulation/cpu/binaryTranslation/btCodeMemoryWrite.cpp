#include "boxedwine.h"
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "btCodeMemoryWrite.h"
#include "btCpu.h"
#include "../../hardmmu/hard_memory.h"

BtCodeMemoryWrite::BtCodeMemoryWrite(BtCPU* cpu, U32 address, U32 len) : cpu(cpu) {
    this->invalidateCode(address, len);
}

BtCodeMemoryWrite::BtCodeMemoryWrite(BtCPU* cpu) : cpu(cpu) {
}

void BtCodeMemoryWrite::invalidateStringWriteToDi(bool repeat, U32 size) {
    U32 addressStart;
    U32 addressLen;

    if (repeat) {
        addressLen = size * (cpu->isBig() ? ECX : CX);
    } else {
        addressLen = size;
    }

    if (cpu->df == 1) {
        addressStart = (this->cpu->isBig() ? EDI : DI) + cpu->seg[ES].address;
    } else {
        addressStart = (this->cpu->isBig() ? EDI : DI) + cpu->seg[ES].address + size - addressLen;
    }
    invalidateCode(addressStart, addressLen);
}

void BtCodeMemoryWrite::invalidateCode(U32 addressStart, U32 addressLen) {
    U32 pageStart = this->cpu->thread->memory->getNativePage(addressStart >> K_PAGE_SHIFT);
    U32 pageStop = this->cpu->thread->memory->getNativePage((addressStart + addressLen - 1) >> K_PAGE_SHIFT);

    for (U32 page = pageStart; page <= pageStop; page++) {
        if (cpu->thread->memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->thread->memory->executableMemoryMutex);
            this->cpu->thread->memory->clearHostCodeForWriting(pageStart, pageStop - pageStart + 1);
            return;
        }
    }    
}

#endif
