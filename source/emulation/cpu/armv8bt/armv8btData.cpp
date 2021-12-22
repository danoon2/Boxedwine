#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "../../hardmmu/hard_memory.h"
#include "armv8btCodeChunk.h"

Armv8btData::Armv8btData(Armv8btCPU* cpu) : cpu(cpu) {
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
    this->currentBlock = NULL;
    this->fpuTopRegSet = false;
    this->fpuOffsetRegSet = false;
    this->clearCachedFpuRegs();
}

Armv8btData::~Armv8btData() {
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

void Armv8btData::clearCachedFpuRegs() {
    memset(this->isFpuRegCached, 0, sizeof(isFpuRegCached));
}

void Armv8btData::write8(U8 data) {
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

void Armv8btData::write16(U16 data) {
    this->write8((U8)data);
    this->write8((U8)(data>>8));
}

void Armv8btData::write32(U32 data) {
    this->write8((U8)data);
    this->write8((U8)(data>>8));
    this->write8((U8)(data>>16));
    this->write8((U8)(data>>24));
}

void Armv8btData::write64(U64 data) {
    this->write8((U8)data);
    this->write8((U8)(data>>8));
    this->write8((U8)(data>>16));
    this->write8((U8)(data>>24));
    this->write8((U8)(data>>32));
    this->write8((U8)(data>>40));
    this->write8((U8)(data>>48));
    this->write8((U8)(data>>56));
}

void Armv8btData::resetForNewOp() {
    this->startOfOpIp = this->ip;
}

U8 Armv8btData::calculateEipLen(U32 eip) {    
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

void Armv8btData::mapAddress(U32 ip, U32 bufferPos) {
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

std::shared_ptr<BtCodeChunk> Armv8btData::commit(bool makeLive) {
    std::shared_ptr<BtCodeChunk> chunk = std::make_shared<Armv8CodeChunk>(this->ipAddressCount, this->ipAddress, this->ipAddressBufferPos, this->buffer, this->bufferPos, this->startOfDataIp, this->ip-this->startOfDataIp, this->dynamic);
    if (makeLive) {
        chunk->makeLive();
    }
    return chunk;
}

#endif