#ifndef __ARMV8BT_DATA_H__
#define __ARMV8BT_DATA_H__

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btCPU.h"
#include "arm8btFlags.h"
#include "../binaryTranslation/btData.h"

class Armv8btData : public BtData {
public:
    Armv8btData(Armv8btCPU* cpu);    

    void resetForNewOp();
    std::shared_ptr<BtCodeChunk> commit(bool makeLive);

    DecodedOp* decodedOp;
    DecodedBlock* currentBlock;

    Armv8btCPU* cpu;

    bool fpuTopRegSet;
    bool fpuOffsetRegSet;
    bool isFpuRegCached[8];
    void clearCachedFpuRegs();
};
#endif
#endif
