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
    result->hostAddress = cpu->thread->memory->allocateExcutableMemory(hostInstructionBufferLen+instructionCount*sizeof(U32)+instructionCount*sizeof(U8), &result->hostAddressSize); 
    result->hostLen = hostInstructionBufferLen;
    result->emulatedInstructionLen = (U8*)result->hostAddress+result->hostAddressSize-instructionCount*sizeof(U8)-instructionCount*sizeof(U32);
    result->hostInstructionLen = (U32*)((U8*)result->hostAddress+result->hostAddressSize-instructionCount*sizeof(U32));// should be aligned to 4 byte boundry
    memset(result->hostAddress, 0xce, hostInstructionBufferLen);

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
            U32 eip = eipInstructionAddress[i];
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
            table[offset] = (U8*)result->hostAddress + hostInstructionIndex[i];
        }        
    }
    memcpy(result->hostAddress, hostInstructionBuffer, hostInstructionBufferLen);
    cpu->thread->memory->addCodeChunk(result);    
    return result;
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

void X64CodeChunk::addLinkFrom(X64CodeChunk* from, U32 toEip, void* toHostInstruction, void* fromHostOffset) {
    X64CodeChunkLink* link = X64CodeChunkLink::alloc();

    link->toEip = toEip;
    link->toHostInstruction = toHostInstruction;
    link->fromHostOffset = fromHostOffset;

    from->linksTo.addToBack(&link->linkTo);
    this->linksFrom.addToBack(&link->linkFrom);
}

void X64CodeChunk::deallocAndRetranslate() {     
    // remove this chunk and its mappings from being used (since it is about to be replaced)
    detachFromHost(); 

    x64CPU* cpu = (x64CPU*)KThread::currentThread()->cpu;
    X64Asm data(cpu);
    data.ip = this->emulatedAddress-cpu->seg[CS].address;
    data.startOfDataIp = data.ip;       
    cpu->translateData(&data);

    void* result = data.commit();
    X64CodeChunk* chunk = cpu->thread->memory->getCodeChunkContainingHostAddress(result);
    cpu->link(&data, result);    
    cpu->makePendingCodePagesReadOnly();
    this->linksFrom.for_each([chunk, cpu] (KListNode<X64CodeChunkLink*>* link) {
        U32 hostOpLen = (U32)link->data->toHostInstruction - (U32)link->data->fromHostOffset - *((S32*)link->data->fromHostOffset)+1;
        link->data->linkFrom.remove();
        chunk->linksFrom.addToBack(&link->data->linkFrom);
        U64 destHost = (U64)cpu->thread->memory->getExistingHostAddress(link->data->toEip);
        U32 fromInstructionIndex;        
        X64CodeChunk* fromChunk = cpu->thread->memory->getCodeChunkContainingHostAddress(link->data->fromHostOffset);
        void* srcHostInstruction = NULL;
        U32 srcEip = fromChunk->getEipThatContainsHostAddress(link->data->fromHostOffset, &srcHostInstruction);
        fromChunk->getStartOfInstructionByEip(srcEip, NULL, &fromInstructionIndex);
        U64 srcHost = (U64)srcHostInstruction;   
        U64 endOfJump = (U64)link->data->fromHostOffset - srcHost + 4;

        *((U32*)link->data->fromHostOffset) = (U32)(destHost-srcHost-endOfJump);
        link->data->toHostInstruction = (void*)destHost;
    });

    // :TODO: retranslate entire chunk
    // :TODO: if it doesn't fit then get new address
    // :TODO: 
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

// if the patch doesn't change the size of the instructions then the patch will happen now, otherwise
// an invalid instruction will be caught later and the code rewritten in the above function, updateStartingAtHostAddress
void X64CodeChunk::patch(U32 eipAddress, U32 len) {    
    U32 eipIndex = 0;
    U8* host = NULL;
    U32 eip = this->getStartOfInstructionByEip(eipAddress, &host, &eipIndex);
    S32 todo = len + (eipAddress - eip);
    x64CPU* cpu = (x64CPU*)KThread::currentThread()->cpu;

    while (todo>0) {
        X64Asm data(cpu);                                
        data.ip = eip;
        data.startOfDataIp = eip;
        cpu->translateInstruction(&data);

        U32 newInstructionLen = data.ip - data.startOfDataIp;
        U32 oldInstructionLen = this->emulatedInstructionLen[eipIndex];

        if (newInstructionLen == oldInstructionLen) {
            U32 oldHostInstructionLen = this->hostInstructionLen[eipIndex];
            U32 newHostInstructionLen = data.bufferPos;

            if (newHostInstructionLen<=oldHostInstructionLen) {
                // this will happen if the code is changed but it doesn't change the length of the instruction, mainly this will apply to offsets in the instruction
                                
                memcpy(host, data.buffer, data.bufferPos);
                for (unsigned int i=data.bufferPos;i<oldHostInstructionLen;i++) {
                    host[i]=(U8)0x90; // nop
                }

                // if there is a link from this old instruction, then remove it, the cpu->link code below will re-create it
                this->linksTo.for_each([host,oldHostInstructionLen] (KListNode<X64CodeChunkLink*>* link) {
                    if (link->data->fromHostOffset>=host && link->data->fromHostOffset<(U8*)host+oldHostInstructionLen) {
                        link->data->dealloc();
                    }
                });

                // in case the new instruction is a call/jump
                cpu->link(&data, (void*)host);

                todo-=newInstructionLen;
                eip+=newInstructionLen;
                host+=oldHostInstructionLen;
                continue;
            }
        }
        U32 remainingLen = this->hostLen - (U32)(host - (U8*)this->hostAddress);
        memset(host, 0xce, remainingLen);
        break;
    }
    cpu->makePendingCodePagesReadOnly();
}    

void X64CodeChunk::removeFromList() {
    if (this->prev) {
        this->prev->next = this->next;
    }
    if (this->next) {
        this->next->prev = this->prev;
    }
}
#endif