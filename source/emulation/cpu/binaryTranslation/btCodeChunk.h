#ifndef __BT_CODE_CHUNK_H__
#define __BT_CODE_CHUNK_H__

#ifdef BOXEDWINE_BINARY_TRANSLATOR

class BtCPU;
class DecodedBlock;

class BtCodeChunk{
public:
    BtCodeChunk(U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic);

    void release(KMemory* memory);
    void releaseAndRetranslate();
    void makeLive(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex);

    void* getHostAddress() { return this->hostAddress; }
    U32 getHostAddressLen() { return this->hostLen; }

    bool containsHostAddress(U8* hostAddress) { return hostAddress >= this->hostAddress && hostAddress < this->hostAddress + this->hostLen; }
    bool containsEip(U32 eip) { return eip >= this->emulatedAddress && eip < this->emulatedAddress + this->emulatedLen; }
    bool containsEip(U32 eip, U32 len);

    U32 getEip() { return emulatedAddress; }
    U32 getEipLen() { return emulatedLen; }
    
    bool locked = false; // just a way to prevent release while transitioning to new memory setup during execv
protected:
    void detachFromHost(KMemory* memory);
    void internalDealloc();
    virtual void clearInstructionCache(U8* hostAddress, U32 len);

    U32 emulatedAddress;
    U32 emulatedLen;    

    U8* hostAddress;
    U32 hostLen;
    U32 hostMemoryLen;
};

#endif

#endif