#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Data.h"
#include "x64CodeChunk.h"
#include "../../hardmmu/hard_memory.h"

X64Data::X64Data(x64CPU* cpu) : cpu(cpu) {
    this->ipAddress = this->ipAddressBuffer;
    this->ipAddressBufferPos = this->ipAddressBufferPosBuffer;
    this->ipAddressCount = 0;
    this->ipAddressBufferSize = sizeof(this->ipAddressBuffer)/sizeof(this->ipAddressBuffer[0]);
    this->resetForNewOp();

    this->buffer = this->bufferInternal;
    this->bufferSize = sizeof(this->bufferInternal);
    this->bufferPos = 0;
    this->done = false;
    this->ip = 0;
    this->startOfDataIp = 0;
    this->startOfOpIp = 0;
    this->calculatedEipLen = 0;
    this->stopAfterInstruction = -1;
    this->dynamic = false;
}

X64Data::~X64Data() {
    if (this->buffer!=this->bufferInternal) {
        delete[] this->buffer;
    }
    if (this->ipAddress!=this->ipAddressBuffer) {
        delete[] this->ipAddress;
    }
    if (this->ipAddressBufferPos!=this->ipAddressBufferPosBuffer) {
        delete[] this->ipAddressBufferPos;
    }
}

U8 X64Data::fetch8() {
    U32 address;

    if (this->cpu->isBig()) {
        address = this->ip + this->cpu->seg[CS].address;
        this->ip++;
    } else {
        address = (this->ip & 0xFFFF) + this->cpu->seg[CS].address;    
        this->ip++;
        this->ip &= 0xFFFF;
    }
    return readb(address);
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
}

U8 X64Data::calculateEipLen(U32 eip) {    
    for (U32 i=0;i<this->ipAddressCount;i++) {
        if (this->ipAddress[i]==eip) {
            if (i==this->ipAddressCount-1) {
                return this->ip-this->ipAddress[i];
            }
            return this->ipAddress[i+1]-this->ipAddress[i];
        }
    }
    return 0;
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

X64CodeChunk* X64Data::commit(bool makeLive) {
    X64CodeChunk* chunk = X64CodeChunk::allocChunk(this->ipAddressCount, this->ipAddress, this->ipAddressBufferPos, this->buffer, this->bufferPos, this->startOfDataIp, this->ip-this->startOfDataIp, this->dynamic);
    if (makeLive) {
        chunk->makeLive();
    }
    return chunk;
}

#endif