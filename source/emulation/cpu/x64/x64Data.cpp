#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Data.h"
#include "x64CodeChunk.h"
#include "x64CPU.h"

X64Data::X64Data(x64CPU* cpu) : cpu(cpu) {
    this->resetForNewOp();
}

U8 X64Data::fetch8() {
    U32 address = 0;

    if (this->cpu->isBig()) {
        address = this->ip + this->cpu->seg[CS].address;
        this->ip++;
    } else {
        address = (this->ip & 0xFFFF) + this->cpu->seg[CS].address;    
        this->ip++;
        this->ip &= 0xFFFF;
    }
    return cpu->memory->readb(address);
}

U16 X64Data::fetch16() {
    U16 result = this->fetch8();
    result |= ((U16)this->fetch8()) << 8;
    return result;
}

U32 X64Data::fetch32() {
    U32 result = this->fetch16();
    result |= ((U32)this->fetch16()) << 16;
    return result;
}

U64 X64Data::fetch64() {
    U64 result = this->fetch32();
    result |= ((U64)this->fetch32()) << 32;
    return result;
}

void X64Data::resetForNewOp() {
    this->ds = DS;
    this->ss = SS;
    this->rex = 0;
    this->repNotZeroPrefix = false;
    this->repZeroPrefix = false;
    this->addressPrefix = false;
    this->operandPrefix = false;
    this->multiBytePrefix = false;
    this->lockPrefix = false;
    this->startOfOpIp = 0;
    this->op = 0;
    this->rm = 0;
    this->has_rm = false;
    this->sib = 0;
    this->has_sib = false;
    this->dispSize = 0;
    this->disp = 0;
    this->imm = 0;
    this->immSize = 0;

    if (this->cpu->isBig()) {
        this->baseOp = 0x200;
        this->ea16 = false;
    } else {
        this->baseOp = 0;
        this->ea16 = true;
    } 
    this->startOfOpIp = this->ip;
    this->skipWriteOp = false;
    this->isG8bitWritten = false;
    this->flagsWrittenToStringFlags = false;
}

std::shared_ptr<BtCodeChunk> X64Data::createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) {
    return std::make_shared<X64CodeChunk>(instructionCount, eipInstructionAddress, hostInstructionIndex, hostInstructionBuffer, hostInstructionBufferLen, eip, eipLen, dynamic);
}

#endif