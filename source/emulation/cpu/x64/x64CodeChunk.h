#ifndef __X64CODECHUNK_H__
#define __X64CODECHUNK_H__

#ifdef BOXEDWINE_X64
class x64CPU;
class X64CodeChunk;

class X64CodeChunkLink {
public:
    X64CodeChunkLink(void* fromHostOffset, U32 toEip, void* toHostInstruction, bool direct) : fromHostOffset(fromHostOffset), toEip(toEip), toHostInstruction(toHostInstruction), direct(direct) {}
    // will point to an address in the middle of the instruction
    void* fromHostOffset;

    // will point to the start of the instruction
    U32 toEip;
    void* toHostInstruction;
    bool direct;
};

class X64CodeChunk {
public:
    static X64CodeChunk* allocChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic);

    void dealloc(Memory* memory);
    void deallocAndRetranslate();
    void invalidateStartingAt(U32 eipAddress);
    void makeLive();

    U32 getEipThatContainsHostAddress(void* hostAddress, void** startOfHostInstruction, U32* index);

    void* getHostAddress() {return this->hostAddress;}
    U32 getHostAddressLen() {return this->hostLen;}

    bool containsHostAddress(void* hostAddress) {return hostAddress>=this->hostAddress && hostAddress<(U8*)this->hostAddress+this->hostLen;}
    bool containsEip(U32 eip) {return eip>=this->emulatedAddress && eip<this->emulatedAddress+this->emulatedLen;}
    bool containsEip(U32 eip, U32 len);

    std::shared_ptr<X64CodeChunkLink> addLinkFrom(X64CodeChunk* from, U32 toEip, void* toHostInstruction, void* fromHostOffset, bool direct);

    void* getHostFromEip(U32 eip) {U8* result=NULL; if (this->getStartOfInstructionByEip(eip, &result, NULL)==eip) {return result;} else {return 0;}}
    U32 getEip() {return emulatedAddress;}
    U32 getEipLen() {return emulatedLen;}    
    bool isDynamicAware() {return this->dynamic;}
    U32 getStartOfInstructionByEip(U32 eip, U8** hostAddress, U32* index);

    bool retranslateSingleInstruction(x64CPU* cpu, void* address);
private:
    void detachFromHost(Memory* memory);
    void internalDealloc();    

    U32 emulatedAddress;
    U32 emulatedLen;
    U8* emulatedInstructionLen; // must be 15 or less per op

    void* hostAddress;
    U32 hostAddressSize;
    U32 hostLen;
    U32* hostInstructionLen;

    U32 instructionCount;
    
    std::list<std::shared_ptr<X64CodeChunkLink>> linksTo;
    std::list<std::shared_ptr<X64CodeChunkLink>> linksFrom;

    bool dynamic; // will include a check of the original vs current code bytes to make sure it is still valid at a per instruction level
};

#endif

#endif