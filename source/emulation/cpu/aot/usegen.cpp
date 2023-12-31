#include "boxedwine.h"
#include "usegen.h"
#include "gen.h"
#include "crc.h"

#ifdef BOXEDWINE_USE_GENERATE_SOURCE

static U32 count;

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    if (DecodedBlock::currentBlock->runCount == 0) {
#else
    if (DecodedBlock::currentBlock->runCount == 50) {
#endif
        U32 len = DecodedBlock::currentBlock->bytes;
        unsigned char* bytes = new unsigned char[len];
        memcopyToNative(DecodedBlock::currentBlock->address, bytes, len);
        U32 crc = crc32b(bytes, len);

        OpCallback pfn = getCompiledFunction(cpu, crc, bytes, len, cpu->getEipAddress());
        if (pfn) {
            op->pfn = pfn;
            count++;
        } else {
            DecodedOp* firstOp = op;
            DecodedBlock::currentBlock->op = op->next;
            op = op->next;
            firstOp->dealloc(false);
        }  
        op->pfn(cpu, op);
    } else {
        op->next->pfn(cpu, op->next);
    }
}

#endif