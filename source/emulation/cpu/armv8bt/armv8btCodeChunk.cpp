#include "boxedwine.h"
#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCodeChunk.h"
#include "armv8btAsm.h"
#include "armv8btCPU.h"

bool Armv8CodeChunk::retranslateSingleInstruction(BtCPU* btCPU, U8* address) {
    Armv8btCPU* cpu = (Armv8btCPU*)btCPU;
    U8* startofHostInstruction;
    U32 index;
    U32 eip = this->getEipThatContainsHostAddress(address, &startofHostInstruction, &index) - cpu->seg[CS].address;
    Armv8btAsm data(cpu);
    data.ip = eip;
    data.startOfDataIp = eip;
    data.translateInstruction();
    U32 eipLen = data.ip - data.startOfOpIp;
    U32 hostLen = data.bufferPos;
    if (eipLen == this->emulatedInstructionLen[index]) {
        if (hostLen == this->hostInstructionLen[index]) {
            memcpy(startofHostInstruction, data.buffer, hostLen);
            clearInstructionCache((U8*)startofHostInstruction, hostLen);
            return true;
        } else if (hostLen < this->hostInstructionLen[index]) {
            while (data.bufferPos < this->hostInstructionLen[index]) {
                data.mov32(0, 0);
            }
            memcpy(startofHostInstruction, data.buffer, hostLen);
            clearInstructionCache((U8*)startofHostInstruction, hostLen);
            return true;
        }
    }
    return false;
}

void Armv8CodeChunk::clearInstructionCache(U8* hostAddress, U32 len) {
    __builtin___clear_cache((char*)hostAddress, ((char*)hostAddress) + len);
}

#endif
