#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicData {
public:
    CPU* cpu = nullptr;
    DecodedOp* skipToOp = nullptr;
    DecodedOp* firstOp = nullptr;
    DecodedOp* currentOp = nullptr;

    U32 skipEipUpdateLen = 0;
    bool done = false;
    const LazyFlags* currentLazyFlags = nullptr;;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif