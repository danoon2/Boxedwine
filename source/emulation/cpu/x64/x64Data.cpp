#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Data.h"

X64Data::X64Data(x64CPU* cpu) : cpu(cpu) {
    if (!cpu->thread->memory->x64Mem) {
       cpu->thread->memory->x64Mem = (U8*)allocExecutable64kBlock(cpu->thread->memory);
    }

    this->ipAddress = this->ipAddressBuffer;
    this->ipAddressBufferPos = this->ipAddressBufferPosBuffer;
    this->ipAddressCount = 0;
    this->ipAddressBufferSize = sizeof(this->ipAddressBuffer)/sizeof(this->ipAddressBuffer[0]);
    this->resetForNewOp();

    this->buffer = this->bufferInternal;
    this->bufferSize = sizeof(this->bufferInternal);
    this->bufferPos = 0;
    this->done = false;
}

X64Data::~X64Data() {
    if (this->buffer!=this->bufferInternal) {
        delete[] this->buffer;
    }
}

U8 X64Data::fetch8() {
    U32 address;

    if (this->cpu->big)
        address = this->ip + this->cpu->seg[CS].address;
    else
        address = (this->ip & 0xFFFF) + this->cpu->seg[CS].address;
    this->ip++;
    return readb(address);
}

U16 X64Data::fetch16() {
    U32 address;

    if (this->cpu->big)
        address = this->ip + this->cpu->seg[CS].address;
    else
        address = (this->ip & 0xFFFF) + this->cpu->seg[CS].address;
    this->ip+=2;
    return readw(address);
}

U32 X64Data::fetch32() {
    U32 address;

    if (this->cpu->big)
        address = this->ip + this->cpu->seg[CS].address;
    else
        address = (this->ip & 0xFFFF) + this->cpu->seg[CS].address;
    this->ip+=4;
    return readd(address);
}

void X64Data::write8(U8 data) {
    if (this->bufferPos>=this->bufferSize) {
        U8* b = new U8[this->bufferSize*2];
        memcpy(b, this->buffer, this->bufferSize);
        if (this->buffer!=this->bufferInternal) {
            delete[] this->buffer;
        }
        this->buffer = b;
        this->bufferSize*=2;
    }
    this->buffer[this->bufferPos++] = data;
}

void X64Data::write16(U16 data) {
    this->write8((U8)data);
    this->write8((U8)(data>>8));
}

void X64Data::write32(U32 data) {
    this->write8((U8)data);
    this->write8((U8)(data>>8));
    this->write8((U8)(data>>16));
    this->write8((U8)(data>>24));
}

void X64Data::write64(U64 data) {
    this->write8((U8)data);
    this->write8((U8)(data>>8));
    this->write8((U8)(data>>16));
    this->write8((U8)(data>>24));
    this->write8((U8)(data>>32));
    this->write8((U8)(data>>40));
    this->write8((U8)(data>>48));
    this->write8((U8)(data>>56));
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

    if (this->cpu->big) {
        this->baseOp = 0x200;
        this->ea16 = false;
    } else {
        this->baseOp = 0;
        this->ea16 = true;
    } 
    this->startOfOpIp = this->ip;
    this->skipWriteOp = false;
    this->isG8bitWritten = false;
}

void X64Data::mapAddress(U32 ip, U32 bufferPos) {
    if (this->ipAddressCount>=this->ipAddressBufferSize) {
        U32* ipAddressOld = this->ipAddress;
        U32* ipAddressBufferPosOld = this->ipAddressBufferPos;

        this->ipAddress = new U32[this->ipAddressBufferSize*2];
        this->ipAddressBufferPos = new U32[this->ipAddressBufferSize*2];

        memcpy(this->ipAddress, ipAddressOld, sizeof(U32)*this->ipAddressBufferSize);
        memcpy(this->ipAddressBufferPos, ipAddressBufferPosOld, sizeof(U32)*this->ipAddressBufferSize);

        this->ipAddressBufferSize*=2;
        if (ipAddressOld!=this->ipAddressBuffer) {
            delete[] ipAddressOld;
        }
        if (ipAddressBufferPosOld!=this->ipAddressBufferPosBuffer) {
            delete[] ipAddressBufferPosOld;
        }
    }
    this->ipAddress[this->ipAddressCount] = ip;
    this->ipAddressBufferPos[this->ipAddressCount++] = bufferPos;        
}

void* X64Data::commit() {
    if (cpu->thread->memory->x64AvailableMem<this->bufferPos) {
        allocExecutable64kBlock(cpu->thread->memory);
    }
    void* result = cpu->thread->memory->x64Mem+cpu->thread->memory->x64MemPos;
    memcpy(result, this->buffer, this->bufferPos);
    cpu->thread->memory->x64MemPos+=this->bufferPos;
    cpu->thread->memory->x64AvailableMem-=this->bufferPos;
    return result;
}

#endif