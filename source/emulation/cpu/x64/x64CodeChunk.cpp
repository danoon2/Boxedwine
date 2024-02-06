#include "boxedwine.h"
#ifdef BOXEDWINE_X64
#include "x64CodeChunk.h"
#include "x64Asm.h"
#include "x64CPU.h"

bool X64CodeChunk::retranslateSingleInstruction(BtCPU* btCPU, U8* address) {
    x64CPU* cpu = (x64CPU*)btCPU;
    U8* startofHostInstruction;
    U32 index;
    U32 eip = this->getEipThatContainsHostAddress(address, &startofHostInstruction, &index) - cpu->seg[CS].address;
    X64Asm data(cpu);
    data.ip = eip;
    data.startOfDataIp = eip;
    data.translateInstruction();
    U32 eipLen = data.ip - data.startOfOpIp;
    U32 hostLen = data.bufferPos;
    if (eipLen == this->emulatedInstructionLen[index] && hostLen == this->hostInstructionLen[index]) {
        Platform::writeCodeToMemory(startofHostInstruction, hostLen, [startofHostInstruction, &data, hostLen]() {
            memcpy(startofHostInstruction, data.buffer, hostLen);
            });
        return true;
    }
    return false;
}

#endif
