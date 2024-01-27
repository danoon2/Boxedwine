#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "armv8btCodeChunk.h"

Armv8btData::Armv8btData(Armv8btCPU* cpu) : cpu(cpu) {
    this->resetForNewOp();

    this->fpuTopRegSet = false;
    this->fpuOffsetRegSet = false;
    this->clearCachedFpuRegs();
}

void Armv8btData::clearCachedFpuRegs() {
    memset(this->isFpuRegCached, 0, sizeof(isFpuRegCached));
}

void Armv8btData::reset() {
    BtData::reset();
    clearCachedFpuRegs();
    this->fpuTopRegSet = false;
    this->fpuOffsetRegSet = false;
}

void Armv8btData::resetForNewOp() {
    this->startOfOpIp = this->ip;
}

std::shared_ptr<BtCodeChunk> Armv8btData::createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) {
    return std::make_shared<Armv8CodeChunk>(instructionCount, eipInstructionAddress, hostInstructionIndex, hostInstructionBuffer, hostInstructionBufferLen, eip, eipLen, dynamic);
}

#endif
