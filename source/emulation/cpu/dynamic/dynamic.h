#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicData {
public:
    DynamicData() : cpu(NULL), skipToOp(NULL), block(NULL), skipEipUpdateLen(0), done(false), currentLazyFlags(NULL) {}
    CPU* cpu;
    DecodedOp* skipToOp;
    DecodedBlock* block;
    U32 skipEipUpdateLen;
    bool done;
    const LazyFlags* currentLazyFlags;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif