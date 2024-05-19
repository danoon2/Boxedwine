#ifndef __ARMV8BTCODECHUNK_H__
#define __ARMV8BTCODECHUNK_H__

#ifdef BOXEDWINE_ARMV8BT

#include "../binaryTranslation/btCodeChunk.h"

class Armv8CodeChunk : public BtCodeChunk {
public:
	Armv8CodeChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) : BtCodeChunk(instructionCount, eipInstructionAddress, hostInstructionIndex, hostInstructionBuffer, hostInstructionBufferLen, eip, eipLen, dynamic) {}
	virtual bool retranslateSingleInstruction(BtCPU* cpu, U8* address) override;
	virtual void clearInstructionCache(U8* hostAddress, U32 len) override;
};

#endif

#endif
