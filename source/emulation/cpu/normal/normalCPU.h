#ifndef __NORMAL_CPU_H__
#define __NORMAL_CPU_H__

#include "../common/cpu.h"

class NormalCPU : public CPU, public DecodeBlockCallback {
public:
    NormalCPU(KMemory* memory);

    // from CPU
    void run() override;
    DecodedOp* getNextOp() override;

    static OpCallback getFunctionForOp(DecodedOp* op);

    OpCallback firstOp;

    // from DecodeBlockCallback
    U8 fetchByte(U32* eip) override;
    bool shouldContinue(U32 eip) override;
    DecodedOp** getOpLocation(U32 eip) override;
};

#endif