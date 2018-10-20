#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicData {
public:
    DynamicData() : cpu(NULL), skipToOp(NULL), block(NULL), done(false), currentLazyFlags(NULL) {}
    CPU* cpu;
    DecodedOp* skipToOp;
    DecodedBlock* block;
    bool done;
    const LazyFlags* currentLazyFlags;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif