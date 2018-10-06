#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicData {
public:
    DynamicData() : cpu(NULL), skipToOp(NULL), block(NULL), done(false) {}
    CPU* cpu;
    DecodedOp* skipToOp;
    DecodedBlock* block;
    bool done;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif