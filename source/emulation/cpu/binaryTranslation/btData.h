#ifdef BOXEDWINE_BINARY_TRANSLATOR

#include "btCpu.h"

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

class BtData {
public:
    BtData();
    virtual ~BtData();    
    std::shared_ptr<BtCodeChunk> commit(bool makeLive);

    U32 ip;
    U32 startOfDataIp;
    U32 startOfOpIp;
    U32 calculatedEipLen;
    bool done;
    bool dynamic;
    bool useSingleMemOffset;

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

    std::vector<TodoJump> todoJump;
    S32 stopAfterInstruction;

    DecodedOp* decodedOp;
    DecodedBlock* currentBlock;

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
protected:
    virtual std::shared_ptr<BtCodeChunk> createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) = 0;
};

#endif