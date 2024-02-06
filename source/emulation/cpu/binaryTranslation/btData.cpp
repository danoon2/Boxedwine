#include "boxedwine.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR

#include "btData.h"
#include "btCodeChunk.h"

BtData::BtData() {
    this->ipAddress = this->ipAddressBuffer;
    this->ipAddressBufferPos = this->ipAddressBufferPosBuffer;
    this->ipAddressBufferSize = sizeof(this->ipAddressBuffer) / sizeof(this->ipAddressBuffer[0]);

    this->buffer = this->bufferInternal;
    this->bufferSize = sizeof(this->bufferInternal);    
}

BtData::~BtData() {
    if (this->buffer != this->bufferInternal) {
        delete[] this->buffer;
    }
    if (this->ipAddress != this->ipAddressBuffer) {
        delete[] this->ipAddress;
    }
    if (this->ipAddressBufferPos != this->ipAddressBufferPosBuffer) {
        delete[] this->ipAddressBufferPos;
    }
}

void BtData::reset() {
    this->ipAddressCount = 0;
    this->bufferPos = 0;
    this->done = false;
    this->ip = 0;
    this->startOfDataIp = 0;
    this->startOfOpIp = 0;
    this->calculatedEipLen = 0;
    this->stopAfterInstruction = -1;
    this->currentOp = nullptr;
    this->needLargeIfJmpReg = false;
    todoJump.clear();
}

void BtData::write8(U8 data) {
    if (this->bufferPos >= this->bufferSize) {
        U8* b = new U8[this->bufferSize * 2];
        memcpy(b, this->buffer, this->bufferSize);
        if (this->buffer != this->bufferInternal) {
            delete[] this->buffer;
        }
        this->buffer = b;
        this->bufferSize *= 2;
    }
    this->buffer[this->bufferPos++] = data;
}

void BtData::write16(U16 data) {
    this->write8((U8)data);
    this->write8((U8)(data >> 8));
}

void BtData::write32(U32 data) {
    this->write8((U8)data);
    this->write8((U8)(data >> 8));
    this->write8((U8)(data >> 16));
    this->write8((U8)(data >> 24));
}

void BtData::write64(U64 data) {
    this->write8((U8)data);
    this->write8((U8)(data >> 8));
    this->write8((U8)(data >> 16));
    this->write8((U8)(data >> 24));
    this->write8((U8)(data >> 32));
    this->write8((U8)(data >> 40));
    this->write8((U8)(data >> 48));
    this->write8((U8)(data >> 56));
}

void BtData::write64Buffer(U8* buffer, U64 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
    buffer[4] = (U8)(value >> 32);
    buffer[5] = (U8)(value >> 40);
    buffer[6] = (U8)(value >> 48);
    buffer[7] = (U8)(value >> 56);
}

void BtData::write32Buffer(U8* buffer, U32 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
}

void BtData::write16Buffer(U8* buffer, U16 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);

}

U8 BtData::calculateEipLen(U32 eip) {
    for (U32 i = 0; i < this->ipAddressCount; i++) {
        if (this->ipAddress[i] == eip) {
            if (i == this->ipAddressCount - 1) {
                return this->ip - this->ipAddress[i];
            }
            return this->ipAddress[i + 1] - this->ipAddress[i];
        }
    }
    return 0;
}

void BtData::mapAddress(U32 ip, U32 bufferPos) {
    if (this->ipAddressCount >= this->ipAddressBufferSize) {
        U32* ipAddressOld = this->ipAddress;
        U32* ipAddressBufferPosOld = this->ipAddressBufferPos;

        this->ipAddress = new U32[this->ipAddressBufferSize * 2];
        this->ipAddressBufferPos = new U32[this->ipAddressBufferSize * 2];

        memcpy(this->ipAddress, ipAddressOld, sizeof(U32) * this->ipAddressBufferSize);
        memcpy(this->ipAddressBufferPos, ipAddressBufferPosOld, sizeof(U32) * this->ipAddressBufferSize);

        this->ipAddressBufferSize *= 2;
        if (ipAddressOld != this->ipAddressBuffer) {
            delete[] ipAddressOld;
        }
        if (ipAddressBufferPosOld != this->ipAddressBufferPosBuffer) {
            delete[] ipAddressBufferPosOld;
        }
    }
    this->ipAddress[this->ipAddressCount] = ip;
    this->ipAddressBufferPos[this->ipAddressCount++] = bufferPos;
}

std::shared_ptr<BtCodeChunk> BtData::commit(bool makeLive) {
    std::shared_ptr<BtCodeChunk> chunk = createChunk(this->ipAddressCount, this->ipAddress, this->ipAddressBufferPos, this->buffer, this->bufferPos, this->startOfDataIp, this->ip - this->startOfDataIp, false);
    chunk->block = this->currentBlock;
    if (makeLive) {
        chunk->makeLive();
    }
    return chunk;
}

#endif