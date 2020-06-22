#ifndef __X32CPU_H__
#define __X32CPU_H__

#ifdef BOXEDWINE_DYNAMIC32
void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op);
#endif

#endif