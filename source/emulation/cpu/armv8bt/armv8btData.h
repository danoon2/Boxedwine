#ifndef __ARMV8BT_DATA_H__
#define __ARMV8BT_DATA_H__

#ifdef BOXEDWINE_ARMV8BT

#include "arm8btFlags.h"
#include "../binaryTranslation/btData.h"

class Armv8btCPU;

class Armv8btData : public BtData {
public:
    Armv8btData(Armv8btCPU* cpu);    

    void resetForNewOp() override;

    Armv8btCPU* cpu;

    bool fpuTopRegSet;
    bool fpuOffsetRegSet;
    bool isFpuRegCached[8];
    void clearCachedFpuRegs();

    virtual void reset() override;
protected:
    virtual std::shared_ptr<BtCodeChunk> createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) override;
};
#endif
#endif
