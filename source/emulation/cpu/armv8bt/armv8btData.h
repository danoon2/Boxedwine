#ifndef __ARMV8BT_DATA_H__
#define __ARMV8BT_DATA_H__

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btCPU.h"
#include "arm8btFlags.h"

class TodoJump {
public:
    TodoJump() : eip(0), bufferPos(0), offsetSize(0), sameChunk(true) {}
    TodoJump(U32 eip, U32 bufferPos, U8 offsetSize, bool sameChunk, U32 opIndex) : eip(eip), bufferPos(bufferPos), offsetSize(offsetSize), sameChunk(sameChunk), opIndex(opIndex) {}
    U32 eip;
    U32 bufferPos;
    U8 offsetSize;
    bool sameChunk;
    U32 opIndex;
};

class Armv8btData {
public:
    Armv8btData(Armv8btCPU* cpu);
    virtual ~Armv8btData();
    
    void mapAddress(U32 ip, U32 bufferPos);

    void write8(U8 data);
    void write16(U16 data);
    void write32(U32 data);
    void write64(U64 data);

    void resetForNewOp();
    std::shared_ptr<BtCodeChunk> commit(bool makeLive);

    DecodedOp* decodedOp;
    DecodedBlock* currentBlock;

    U32 ip;
    U32 startOfDataIp;
    U32 startOfOpIp;
    U32 calculatedEipLen;
    bool done;
    bool usesSSEConstant[vSseConstantCount];    

    Armv8btCPU* cpu;

    std::vector<TodoJump> todoJump;
    S32 stopAfterInstruction;

    U8 calculateEipLen(U32 eip);

    U32* ipAddress;
    U32* ipAddressBufferPos;
    U32 ipAddressCount;
    U32 ipAddressBufferSize;
    U32 ipAddressBuffer[64];
    U32 ipAddressBufferPosBuffer[64];
    U8* buffer;
    U32 bufferSize;
    U32 bufferPos;
    U8 bufferInternal[256];
    bool dynamic;

    bool fpuTopRegSet;
    bool fpuOffsetRegSet;
    bool isFpuRegCached[8];
    void clearCachedFpuRegs();
};
#endif
#endif
