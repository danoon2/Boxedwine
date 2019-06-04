#include "boxedwine.h"
#ifdef BOXEDWINE_X64
#include "x64CodeChunk.h"
#include "x64Asm.h"

X64CodeChunkLink* X64CodeChunkLink::alloc() {
    return new X64CodeChunkLink();
}

void X64CodeChunkLink::dealloc() {
    this->linkFrom.remove();
    this->linkTo.remove();
    delete this;
}

X64CodeChunk* X64CodeChunk::allocChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen) {
    X64CodeChunk* result = new X64CodeChunk();
    CPU* cpu = KThread::currentThread()->cpu;
    result->next = NULL;
    result->prev = NULL;
    result->instructionCount = instructionCount;
    result->emulatedAddress = eip+cpu->seg[CS].address;
    result->emulatedLen = eipLen;
    result->hostAddress = cpu->thread->memory->allocateExcutableMemory(hostInstructionBufferLen+instructionCount*sizeof(U32)+instructionCount*sizeof(U8)+4, &result->hostAddressSize); // +4 for a guard
    result->hostLen = hostInstructionBufferLen;
    result->emulatedInstructionLen = (U8*)result->hostAddress+result->hostAddressSize-instructionCount*sizeof(U8)-instructionCount*sizeof(U32);
    result->hostInstructionLen = (U32*)((U8*)result->hostAddress+result->hostAddressSize-instructionCount*sizeof(U32));// should be aligned to 4 byte boundry
    memset(result->hostAddress, 0xce, result->hostAddressSize);

    if (instructionCount) {
        for (U32 i=0;i<instructionCount;i++) {
            if (i==instructionCount-1) {
                result->emulatedInstructionLen[i] = eipLen-(eipInstructionAddress[i]-result->emulatedAddress);
                result->hostInstructionLen[i] = hostInstructionBufferLen-hostInstructionIndex[i];
            } else {
                result->emulatedInstructionLen[i] = eipInstructionAddress[i+1]-eipInstructionAddress[i];
                result->hostInstructionLen[i] = hostInstructionIndex[i+1]-hostInstructionIndex[i];
            }
            if (result->emulatedInstructionLen[i]>K_MAX_X86_OP_LEN) {
                kpanic("X64CodeChunk::allocChunk emulatedInstructionLen sanity check failed");
            }           
        }        
    }
    memcpy(result->hostAddress, hostInstructionBuffer, hostInstructionBufferLen);
    return result;
}

void X64CodeChunk::makeLive() {
    CPU* cpu = KThread::currentThread()->cpu;
    U32 eip = this->emulatedAddress;
    U8* host = (U8*)this->hostAddress;

    for (U32 i=0;i<instructionCount;i++) {        
        U32 page = eip >> K_PAGE_SHIFT;
        U32 offset = eip & K_PAGE_MASK;

        void** table = cpu->thread->memory->eipToHostInstruction[page];
        if (!table) {
            table = new void*[K_PAGE_SIZE];
            memset(table, 0, sizeof(void*)*K_PAGE_SIZE);
            cpu->thread->memory->eipToHostInstruction[page] = table;
        }
        if (table[offset]) {
            kpanic("X64CodeChunk::allocChunk eip already mapped");
        }
        table[offset] = host;
        eip+=this->emulatedInstructionLen[i];
        host+=this->hostInstructionLen[i];
    }        
    cpu->thread->memory->addCodeChunk(this);    
}

void X64CodeChunk::detachFromHost() {
    U32 eip = this->emulatedAddress;
    CPU* cpu = KThread::currentThread()->cpu;
    for (U32 i=0;i<this->instructionCount;i++) {
        if (cpu->thread->memory->eipToHostInstruction[eip >> K_PAGE_SHIFT]) { // might span multiple pages and the other pages are already deleted
            cpu->thread->memory->eipToHostInstruction[eip >> K_PAGE_SHIFT][eip & K_PAGE_MASK] = NULL;
        }
        eip+=this->emulatedInstructionLen[i];
    }
    cpu->thread->memory->removeCodeChunk(this);
    this->linksTo.for_each([] (KListNode<X64CodeChunkLink*>* link) {
        link->data->dealloc();
    });
}

void X64CodeChunk::dealloc() {        
    this->detachFromHost();    
    this->internalDealloc();
}

void X64CodeChunk::internalDealloc() {    
    KThread::currentThread()->memory->freeExcutableMemory(this->hostAddress, this->hostAddressSize);
#ifdef _DEBUG
    // tag it so that we can see where it came from when debugging
    U32* p = (U32*)this->hostAddress;
    p++;
    p++;
    *p = this->emulatedAddress;
#endif
    delete this;
}

U32 X64CodeChunk::getEipThatContainsHostAddress(void* address, void** startOfHostInstruction) {
    if (this->containsHostAddress(address)) {
        U8* p = (U8*)this->hostAddress;
        U32 result = this->emulatedAddress;

        for (unsigned int i=0;i<this->instructionCount;i++) {
            U32 len = this->hostInstructionLen[i];
            if (address>=p && address<p+len) {
                if (startOfHostInstruction) {
                    *startOfHostInstruction = p;
                }
                return result;
            }
            p+=len;
            result+=this->emulatedInstructionLen[i];
        }
    }
    return 0;
}

U32 X64CodeChunk::getStartOfInstructionByEip(U32 eip, U8** host, U32* index) {
    if (this->containsEip(eip)) {
        U32 result = this->emulatedAddress;
        U8* hostResult = (U8*)this->hostAddress;

        for (unsigned int i=0;i<this->instructionCount;i++) {
            U32 len = this->emulatedInstructionLen[i];
            if (eip>=result && eip<result+len) {
                if (index) {
                    *index = i;
                }
                if (host) {
                    *host = hostResult;
                }
                return result;
            }
            result+=len;
            hostResult+=this->hostInstructionLen[i];
        }
    }
    return 0;
}

X64CodeChunkLink* X64CodeChunk::addLinkFrom(X64CodeChunk* from, U32 toEip, void* toHostInstruction, void* fromHostOffset) {
    if (from==this) {
        kpanic("X64CodeChunk::addLinkFrom can not link to itself");
    }
    X64CodeChunkLink* link = X64CodeChunkLink::alloc();

    link->toEip = toEip;
    link->toHostInstruction = toHostInstruction;
    link->fromHostOffset = fromHostOffset;

    from->linksTo.addToBack(&link->linkTo);
    this->linksFrom.addToBack(&link->linkFrom);
    return link;
}

void X64CodeChunk::deallocAndRetranslate() {     
    // remove this chunk and its mappings from being used (since it is about to be replaced)
    detachFromHost(); 

    x64CPU* cpu = (x64CPU*)KThread::currentThread()->cpu;
    X64Asm data1(cpu);
    data1.ip = this->emulatedAddress-cpu->seg[CS].address;
    data1.startOfDataIp = data1.ip;       
    cpu->translateData(&data1);
    
    X64Asm data(cpu);
    data.ip = this->emulatedAddress-cpu->seg[CS].address;
    data.startOfDataIp = data.ip;       
    data.calculatedEipLen = data1.ip-data1.startOfDataIp;
    cpu->translateData(&data);

    X64CodeChunk* chunk = data.commit(false);
    cpu->link(&data, chunk);    
    cpu->makePendingCodePagesReadOnly();    

    this->linksFrom.for_each([chunk, cpu] (KListNode<X64CodeChunkLink*>* link) {        
        U64 destHost = (U64)chunk->getHostFromEip(link->data->toEip);
        if (destHost==0) {
            kpanic("X64CodeChunk::deallocAndRetranslate failed to relink");
        }

        link->data->linkFrom.remove();
        chunk->linksFrom.addToBack(&link->data->linkFrom);
        
        ATOMIC_WRITE64((U64*)&link->data->toHostInstruction, destHost);
    });
    chunk->makeLive();

    this->internalDealloc(); // don't call dealloc() because the new chunk occupies the memory cache and we don't want to mess with it
}

bool X64CodeChunk::hasLinkTo(void* hostAddress) {
    bool found = false;

    this->linksFrom.for_each([&found, hostAddress] (KListNode<X64CodeChunkLink*>* link) {
        if (link->data->toHostInstruction==hostAddress) {
            found = true;
        }
    });
    this->linksTo.for_each([&found, hostAddress] (KListNode<X64CodeChunkLink*>* link) {
        if (link->data->toHostInstruction==hostAddress) {
            found = true;
        }
    });
    return found;
}

bool X64CodeChunk::hasLinkToEip(U32 eip) {
    bool found = false;

    this->linksFrom.for_each([&found, eip] (KListNode<X64CodeChunkLink*>* link) {
        if (link->data->toEip==eip) {
            found = true;
        }
    });
    this->linksTo.for_each([&found, eip] (KListNode<X64CodeChunkLink*>* link) {
        if (link->data->toEip==eip) {
            found = true;
        }
    });
    return found;
}

void X64CodeChunk::invalidateStartingAt(U32 eipAddress) {    
    U32 eipIndex = 0;
    U8* host = NULL;
    U32 eip = this->getStartOfInstructionByEip(eipAddress, &host, &eipIndex);
    U32 remainingLen = this->hostLen - (U32)(host - (U8*)this->hostAddress);
    memset(host, 0xce, remainingLen);
}    

void X64CodeChunk::removeFromList() {
    if (this->prev) {
        this->prev->next = this->next;
    }
    if (this->next) {
        this->next->prev = this->prev;
    }
}

bool X64CodeChunk::containsEip(U32 eip, U32 len) {
    // do we begin in this chunk?
    if (this->containsEip(eip)) {
        return true;
    }
    // do we end in this chunk?
    if (this->containsEip(eip+len-1)) {
        return true;
    }
    // we we span this chunk
    if (eip < this->emulatedAddress && eip+len>this->emulatedAddress+this->emulatedLen) {
        return true;
    }
    return false;
}
#endif