#include "boxedwine.h"
#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCodeChunk.h"
#include "armv8btAsm.h"

void Armv8CodeChunk::makeLive() {
    BtCodeChunk::makeLive();
    __builtin___clear_cache((char*)this->hostAddress, ((char*)this->hostAddress) + this->hostLen);
}

bool Armv8CodeChunk::retranslateSingleInstruction(BtCPU* btCPU, void* address) {
    Armv8btCPU* cpu = (Armv8btCPU*)btCPU;
    void* startofHostInstruction;
    U32 index;
    U32 eip = this->getEipThatContainsHostAddress(address, &startofHostInstruction, &index) - cpu->seg[CS].address;
    Armv8btAsm data(cpu);
    data.ip = eip;
    data.startOfDataIp = eip;
    data.dynamic = this->dynamic;
    cpu->translateInstruction(&data, NULL);
    U32 eipLen = data.ip - data.startOfOpIp;
    U32 hostLen = data.bufferPos;
    if (eipLen == this->emulatedInstructionLen[index] && hostLen == this->hostInstructionLen[index]) {
        memcpy(startofHostInstruction, data.buffer, hostLen);
        return true;
    }
    return false;
}

#endif
