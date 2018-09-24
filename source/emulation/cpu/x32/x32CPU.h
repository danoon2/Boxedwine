#ifndef __X32CPU_H__
#define __X32CPU_H__

#ifdef BOXEDWINE_DYNAMIC32
void OPCALL firstX32Op(CPU* cpu, DecodedOp* op);
#endif

#endif