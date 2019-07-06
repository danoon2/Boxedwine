#ifndef __X64CODECHUNK_H__
#define __X64CODECHUNK_H__

#ifdef BOXEDWINE_X64
class x64CPU;

class X64CodeChunkLink {
public:
    static X64CodeChunkLink* alloc();

    X64CodeChunkLink() : linkTo(this), linkFrom(this) {}
    void dealloc();    

    // will address to the middle of the instruction
    void* fromHostOffset;

    // will point to the start of the instruction
    U32 toEip;
    void* toHostInstruction;
    bool direct;

    KListNode<X64CodeChunkLink*> linkTo;
    KListNode<X64CodeChunkLink*> linkFrom;
};

class X64CodeChunk {
public:
    static X64CodeChunk* allocChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic);

    void dealloc();    
    void deallocAndRetranslate();
    void invalidateStartingAt(U32 eipAddress);
    void makeLive();

    U32 getEipThatContainsHostAddress(void* hostAddress, void** startOfHostInstruction, U32* index);

    void* getHostAddress() {return this->hostAddress;}
    U32 getHostAddressLen() {return this->hostLen;}

    bool containsHostAddress(void* hostAddress) {return hostAddress>=this->hostAddress && hostAddress<=(U8*)this->hostAddress+this->hostLen;}
    bool containsEip(U32 eip) {return eip>=this->emulatedAddress && eip<this->emulatedAddress+this->emulatedLen;}
    bool containsEip(U32 eip, U32 len);
    X64CodeChunk* getNextChunkInHostPage() {return this->nextHost;}
    X64CodeChunk* getNextChunkInEmulationPage() {return this->nextEmulation;}

    void addNextHostChunk(X64CodeChunk* n) {this->nextHost = n; if (n) n->prevHost=this;}
    void addNextEmulationChunk(X64CodeChunk* n) {this->nextEmulation = n; if (n) n->prevEmulation=this;}
    void removeFromList();

    X64CodeChunkLink* addLinkFrom(X64CodeChunk* from, U32 toEip, void* toHostInstruction, void* fromHostOffset, bool direct);
    bool hasLinkTo(void* hostAddress);
    bool hasLinkToEip(U32 eip);

    void* getHostFromEip(U32 eip) {U8* result=NULL; if (this->getStartOfInstructionByEip(eip, &result, NULL)==eip) {return result;} else {return 0;}}
    U32 getEip() {return emulatedAddress;}
    U32 getEipLen() {return emulatedLen;}    
    bool isDynamicAware() {return this->dynamic;}
    U32 getStartOfInstructionByEip(U32 eip, U8** hostAddress, U32* index);

    bool retranslateSingleInstruction(x64CPU* cpu, void* address);
private:
    void detachFromHost();
    void internalDealloc();    

    U32 emulatedAddress;
    U32 emulatedLen;
    U8* emulatedInstructionLen; // must be 15 or less per op

    void* hostAddress;
    U32 hostAddressSize;
    U32 hostLen;
    U32* hostInstructionLen;

    U32 instructionCount;
    
    // double linked list for chunks in the page
    X64CodeChunk* nextHost;
    X64CodeChunk* prevHost;
    X64CodeChunk* nextEmulation;
    X64CodeChunk* prevEmulation;

    KList<X64CodeChunkLink*> linksTo;
    KList<X64CodeChunkLink*> linksFrom;

    bool dynamic; // will include a check of the original vs current code bytes to make sure it is still valid at a per instruction level
};

#endif

#endif