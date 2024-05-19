#ifndef __X64CODECHUNK_H__
#define __X64CODECHUNK_H__

#ifdef BOXEDWINE_X64

#include "../binaryTranslation/btCodeChunk.h"

class X64CodeChunk : public BtCodeChunk {
public:
	X64CodeChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) : BtCodeChunk(instructionCount, eipInstructionAddress, hostInstructionIndex, hostInstructionBuffer, hostInstructionBufferLen, eip, eipLen, dynamic) {}

	// from BtCodeChunk
	bool retranslateSingleInstruction(BtCPU* cpu, U8* address) override;
};

#endif

#endif