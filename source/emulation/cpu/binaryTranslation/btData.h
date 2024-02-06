#ifndef __BTDATA_H__
#define __BTDATA_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

#include "btCpu.h"

class TodoJump {
public:
    TodoJump() = default;
    TodoJump(U32 eip, U32 bufferPos, U32 opIndex) : eip(eip), bufferPos(bufferPos), opIndex(opIndex) {}
    U32 eip = 0;
    U32 bufferPos = 0;
    U32 opIndex = 0;
};

class BtData {
public:
    BtData();
    virtual ~BtData();    
    std::shared_ptr<BtCodeChunk> commit(bool makeLive);

    U32 ip = 0;
    U32 startOfDataIp = 0;
    U32 startOfOpIp = 0;
    U32 calculatedEipLen = 0;
    bool done = false;
    U32* ipAddress = nullptr;
    U32* ipAddressBufferPos = nullptr;
    U32 ipAddressCount = 0;
    U32 ipAddressBufferSize = 0;
    U32 ipAddressBuffer[64] = { 0 };
    U32 ipAddressBufferPosBuffer[64] = { 0 };
    U8* buffer = nullptr;
    U32 bufferSize = 0;
    U32 bufferPos = 0;
    U8 bufferInternal[512] = { 0 };

    std::vector<TodoJump> todoJump;
    S32 stopAfterInstruction = -1;

    DecodedOp* currentOp = nullptr;
    DecodedBlock* currentBlock = nullptr;

    BtData* firstPass = nullptr;
    bool needLargeIfJmpReg = false;

    void mapAddress(U32 ip, U32 bufferPos);
    U8 calculateEipLen(U32 eip);

    void write8(U8 data);
    void write16(U16 data);
    void write32(U32 data);
    void write64(U64 data);
    void write64Buffer(U8* buffer, U64 value);
    void write32Buffer(U8* buffer, U32 value);
    void write16Buffer(U8* buffer, U16 value);

    virtual void jumpTo(U32 eip) = 0;
    virtual void resetForNewOp() = 0;
    virtual void translateInstruction() = 0;
    virtual void reset();
protected:
    virtual std::shared_ptr<BtCodeChunk> createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) = 0;
};

#endif
#endif