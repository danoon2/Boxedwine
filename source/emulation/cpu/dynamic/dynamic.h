#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicJump {
public:
    DynamicJump() = default;
    DynamicJump(U32 eip, U32 bufferPos) : eip(eip), bufferPos(bufferPos) {}
    U32 eip = 0;
    U32 bufferPos = 0;
};

class DynamicData {
public:
    CPU* cpu = nullptr;
    DecodedOp* skipToOp = nullptr;
    DecodedOp* firstOp = nullptr;
    DecodedOp* currentOp = nullptr;

    bool done = false;
    const LazyFlags* currentLazyFlags = nullptr;
    std::vector<DynamicJump> jumps;
    BHashTable<U32, U32> eipToBufferPos;
    U32 currentEip = 0;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif