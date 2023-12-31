#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__

#include "../common/cpu.h"

#ifndef CPU_OFFSET_TYPE
#define CPU_OFFSET_TYPE U32
#endif

#ifndef DYN_HOST_FN
#define DYN_HOST_FN(x) (void*)x
#endif

#ifndef DYN_LAZY_FLAG
#define DYN_LAZY_FLAG(x) x
#endif

#ifndef DYN_LAZY_FLAG_TYPE
#define DYN_LAZY_FLAG_TYPE const LazyFlags*
#endif

#ifndef CPU_OFFSET_OF_REG8
#define CPU_OFFSET_OF_REG8(x) (x>=4?offsetof(CPU, reg[x-4].h8):offsetof(CPU, reg[x].u8))
#endif

#ifndef CPU_OFFSET_OF_REG16
#define CPU_OFFSET_OF_REG16(x) offsetof(CPU, reg[x].u16)
#endif

#ifndef CPU_OFFSET_OF_REG32
#define CPU_OFFSET_OF_REG32(x) offsetof(CPU, reg[x].u32)
#endif

#ifndef CPU_OFFSET_OF_SEG_VALUE
#define CPU_OFFSET_OF_SEG_VALUE(x) offsetof(CPU, seg[x].value)
#endif

#ifndef CPU_OFFSET_OF_SEG_ADDRESS
#define CPU_OFFSET_OF_SEG_ADDRESS(x) offsetof(CPU, seg[x].address)
#endif

#ifndef CPU_OFFSET_TYPE
#define CPU_OFFSET_TYPE U32
#endif

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