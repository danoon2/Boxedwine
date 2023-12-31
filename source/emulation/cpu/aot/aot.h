#ifndef __AOT_H__
#define __AOT_H__

#ifdef BOXEDWINE_GENERATE_SOURCE
void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op);
void writeSource(const std::string path);
#endif

#endif