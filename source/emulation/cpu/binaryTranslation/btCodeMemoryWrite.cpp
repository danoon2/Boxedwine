#include "boxedwine.h"
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "btCodeMemoryWrite.h"
#include "btCpu.h"
#include "../../hardmmu/kmemory_hard.h"

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
    KMemoryData* mem = getMemData(cpu->memory);
    U32 pageStart = mem->getNativePage(addressStart >> K_PAGE_SHIFT);
    U32 pageStop = mem->getNativePage((addressStart + addressLen - 1) >> K_PAGE_SHIFT);

    for (U32 page = pageStart; page <= pageStop; page++) {
        if (mem->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
            mem->clearHostCodeForWriting(pageStart, pageStop - pageStart + 1);
            return;
        }
    }    
}

#endif
