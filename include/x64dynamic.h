#ifndef __X64DYNAMIC_H__
#define __X64DYNAMIC_H__

#include "platform.h"
#include "cpu.h"

void* x64_translateEip(struct CPU* cpu, U32 ip);
void* x64_initCPU(struct CPU* cpu);
U32 x64_handleCmd(struct CPU* cpu, U32 cmd, U32 value);

#endif