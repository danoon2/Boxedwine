#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

class DynamicData {
public:
    CPU* cpu;
};

typedef void (*pfnDynamicOp)(DynamicData* data, DecodedOp* op);

#endif