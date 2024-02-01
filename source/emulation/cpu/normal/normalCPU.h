#ifndef __NORMAL_CPU_H__
#define __NORMAL_CPU_H__

#include "../common/cpu.h"

class NormalCPU : public CPU {
public:
    NormalCPU(KMemory* memory);

    static void clearCache();

    // from CPU
    void run() override;
    DecodedBlock* getNextBlock() override;

    static OpCallback getFunctionForOp(DecodedOp* op);

    static DecodedBlock* getBlockForInspectionButNotUsed(CPU* cpu, U32 address, bool big);
    static DecodedOp* decodeSingleOp(CPU* cpu, U32 address);

    OpCallback firstOp;
};

#endif