#include "boxedwine.h"
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#include "btCodeMemoryWrite.h"
#include "btCpu.h"
#include "../../hardmmu/hard_memory.h"

BtCodeMemoryWrite::BtCodeMemoryWrite(BtCPU* cpu, U32 address, U32 len) : count(0), cpu(cpu) {
    this->invalidateCode(address, len);
}

BtCodeMemoryWrite::BtCodeMemoryWrite(BtCPU* cpu) : count(0), cpu(cpu) {
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
    U32 pageStart = addressStart >> K_PAGE_SHIFT;
    U32 pageStop = (addressStart + addressLen) >> K_PAGE_SHIFT;

    for (U32 page = pageStart; page <= pageStop; page++) {
        if (cpu->thread->memory->nativeFlags[page] & NATIVE_FLAG_CODEPAGE_READONLY) {
            if (count >= CLEAR_BUFFER_SIZE) {
                kpanic("invalidateCode CLEAR_BUFFER_SIZE is not large enough");
            }
            buffer[count] = page;
            count++;
            ::clearCodePageReadOnly(cpu->thread->memory, page);
        }
    }
    if (count) {
        this->cpu->thread->memory->invalideHostCode(addressStart, addressLen);
    }
}

BtCodeMemoryWrite::~BtCodeMemoryWrite() {
    this->restoreCodePageReadOnly();
}

void BtCodeMemoryWrite::restoreCodePageReadOnly() {
    for (U32 i = 0; i < this->count; i++) {
        if (cpu->thread->memory->dynamicCodePageUpdateCount[buffer[i]] != MAX_DYNAMIC_CODE_PAGE_COUNT) {
            ::makeCodePageReadOnly(cpu->thread->memory, buffer[i]);
        }
    }
    this->count = 0;
}
#endif