#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "../../hardmmu/hard_memory.h"
#include "armv8btCodeChunk.h"

Armv8btData::Armv8btData(Armv8btCPU* cpu) : cpu(cpu) {
    this->resetForNewOp();

    this->currentBlock = NULL;
    this->fpuTopRegSet = false;
    this->fpuOffsetRegSet = false;
    this->clearCachedFpuRegs();
}

void Armv8btData::clearCachedFpuRegs() {
    memset(this->isFpuRegCached, 0, sizeof(isFpuRegCached));
}

void Armv8btData::resetForNewOp() {
    this->startOfOpIp = this->ip;
}

std::shared_ptr<BtCodeChunk> Armv8btData::commit(bool makeLive) {
    std::shared_ptr<BtCodeChunk> chunk = std::make_shared<Armv8CodeChunk>(this->ipAddressCount, this->ipAddress, this->ipAddressBufferPos, this->buffer, this->bufferPos, this->startOfDataIp, this->ip-this->startOfDataIp, this->dynamic);
    if (makeLive) {
        chunk->makeLive();
    }
    return chunk;
}

#endif
