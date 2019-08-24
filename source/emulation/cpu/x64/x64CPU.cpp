#include "boxedwine.h"

#ifdef BOXEDWINE_X64
#include "x64Ops.h"
#include "x64CPU.h"
#include "x64Asm.h"
#include "../../hardmmu/hard_memory.h"
#include "x64CodeChunk.h"
#include "../normal/normalCPU.h"

x64CPU::x64CPU() : nativeHandle(0), jmpBuf(NULL), endCond("x64CPU::endcond"), inException(false), restarting(false) {
}

typedef void (*StartCPU)();

void x64CPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
    this->negSegAddress[index] = (U32)(-((S32)(this->seg[index].address)));
}

void x64CPU::run() {    
    while (true) {
        this->memOffset = this->thread->process->memory->id;
        this->negMemOffset = (U64)(-(S64)this->memOffset);
        for (int i=0;i<6;i++) {
            this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
        }
		this->restarting = false;
        if (setjmp(this->runBlockJump)==0) {
            StartCPU start = (StartCPU)this->init();
            start();
#ifdef __TEST
            return;
#endif
        }
    }
}

void x64CPU::restart() {
	this->memOffset = this->thread->process->memory->id;
	this->negMemOffset = (U64)(-(S64)this->memOffset);
	for (int i = 0; i < 6; i++) {
		this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
	}
	this->restarting = true;
    longjmp(this->runBlockJump, 1);
}

DecodedBlock* x64CPU::getNextBlock() {
    return NULL;
}

void* x64CPU::init() {
    X64Asm data(this);
    void* result;
    Memory* memory = this->thread->memory;
    x64CPU* cpu = this;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->executableMemoryMutex);

    data.pushNativeFlags();
    data.writeToRegFromValue(HOST_CPU, true, (U64)this, 8);
    data.writeToRegFromValue(HOST_MEM, true, cpu->memOffset, 8);

    data.writeToRegFromValue(HOST_SS, true, (U32)cpu->seg[SS].address, 4);
    data.writeToRegFromValue(HOST_DS, true, (U32)cpu->seg[DS].address, 4);

    data.setNativeFlags(this->flags, FMASK_TEST|DF);

    data.writeToRegFromValue(0, false, EAX, 4);
    data.writeToRegFromValue(1, false, ECX, 4);
    data.writeToRegFromValue(2, false, EDX, 4);
    data.writeToRegFromValue(3, false, EBX, 4);
    data.writeToRegFromValue(HOST_ESP, true, ESP, 4);
    data.writeToRegFromValue(5, false, EBP, 4);
    data.writeToRegFromValue(6, false, ESI, 4);
    data.writeToRegFromValue(7, false, EDI, 4);        
    
    data.calculatedEipLen = 1; // will force the long x64 chunk jump
    data.doJmp();
    X64CodeChunk* chunk = data.commit(true);
    result = chunk->getHostAddress();
    link(&data, chunk);
    this->pendingCodePages.clear();
    this->eipToHostInstruction = this->thread->memory->eipToHostInstruction;
    return result;
}

X64CodeChunk* x64CPU::translateChunk(X64Asm* parent, U32 ip) {
    X64Asm data1(this);
    data1.ip = ip;
    data1.startOfDataIp = ip;       
    data1.parent = parent;
    translateData(&data1);

    X64Asm data(this);
    data.ip = ip;
    data.startOfDataIp = ip;  
    data.calculatedEipLen = data1.ip - data1.startOfDataIp;
    data.parent = parent;
    translateData(&data, &data1);        
    S32 failedJumpOpIndex = this->preLinkCheck(&data);

    if (failedJumpOpIndex==-1) {
        X64CodeChunk* chunk = data.commit(false);
        link(&data, chunk);
        return chunk;
    } else {
        X64Asm data2(this);
        data2.ip = ip;
        data2.startOfDataIp = ip;       
        data2.parent = parent;
        data2.stopAfterInstruction = failedJumpOpIndex;
        translateData(&data2);

        X64Asm data3(this);
        data3.ip = ip;
        data3.startOfDataIp = ip;  
        data3.calculatedEipLen = data2.ip - data2.startOfDataIp;
        data3.parent = parent;
        data3.stopAfterInstruction = failedJumpOpIndex;
        translateData(&data3, &data2);

        X64CodeChunk* chunk = data3.commit(false);
        link(&data3, chunk);
        return chunk;
    }    
}

void* x64CPU::translateEipInternal(X64Asm* parent, U32 ip) {
    if (!this->big) {
        ip = ip & 0xFFFF;
    }
    U32 address = this->seg[CS].address+ip;
    void* result = this->thread->memory->getExistingHostAddress(address);

    if (!result) {
        X64CodeChunk* chunk = this->translateChunk(parent, ip);
        result = chunk->getHostAddress();
        chunk->makeLive();
    }
    return result;
}

#ifdef __TEST
void x64CPU::addReturnFromTest() {
    X64Asm data(this);
    data.addReturnFromTest();
    data.commit(true);
}
#endif

S32 x64CPU::preLinkCheck(X64Asm* data) {
    for (S32 i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;        
        U8 size = data->todoJump[i].offsetSize;

        if (size==4 && data->todoJump[i].sameChunk) {
            bool found = false;

            for (U32 ip=0;ip<data->ipAddressCount;ip++) {
                if (data->ipAddress[ip] == eip) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return data->todoJump[i].opIndex;
            }
        }
    }
    return -1;
}

void x64CPU::link(X64Asm* data, X64CodeChunk* fromChunk, U32 offsetIntoChunk) {
    U32 i;
    if (!fromChunk) {
        kpanic("x64CPU::link fromChunk missing");
    }
    for (i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;        
        U8* offset = (U8*)fromChunk->getHostAddress()+offsetIntoChunk+data->todoJump[i].bufferPos;
        U8 size = data->todoJump[i].offsetSize;

        if (size==4 && data->todoJump[i].sameChunk) {
            U8* host = (U8*)fromChunk->getHostFromEip(eip);
            if (!host) {
                kpanic("x64CPU::link can not link into the middle of an instruction");
            }
            data->write32Buffer(offset, (U32)(host - offset - 4));            
        } else if (size==4 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)this->thread->memory->getExistingHostAddress(eip);

            if (!toHostAddress) {
                U8 op = 0xce;
                U32 hostIndex = 0;
                X64CodeChunk* chunk = X64CodeChunk::allocChunk(1, &eip, &hostIndex, &op, 1, eip-this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();            
            }
            X64CodeChunk* toChunk = this->thread->memory->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("x64CPU::link to chunk missing");
            }
            X64CodeChunkLink* link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, true);
            data->write32Buffer(offset, (U32)(toHostAddress - offset - 4));            
        } else if (size==8 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)this->thread->memory->getExistingHostAddress(eip);

            if (!toHostAddress) {
                U8 op = 0xce;
                U32 hostIndex = 0;
                X64CodeChunk* chunk = X64CodeChunk::allocChunk(1, &eip, &hostIndex, &op, 1, eip-this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();            
            }
            X64CodeChunk* toChunk = this->thread->memory->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("x64CPU::link to chunk missing");
            }
            X64CodeChunkLink* link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, false);
            data->write64Buffer(offset, (U64)&link->toHostInstruction);
        } else {
            kpanic("x64CPU::link unexpected patch size");
        }
    }
    markCodePageReadOnly(data);
}

void x64CPU::markCodePageReadOnly(X64Asm* data) {
    U32 pageStart = (data->startOfDataIp+this->seg[CS].address) >> K_PAGE_SHIFT;
    U32 pageEnd = (data->startOfDataIp+this->seg[CS].address) >> K_PAGE_SHIFT;
    S32 pageCount = pageEnd-pageStart+1;

#ifndef __TEST
    for (int i=0;i<pageCount;i++) {        
        pendingCodePages.push_back(pageStart+i);        
    }
#endif    
}

void x64CPU::makePendingCodePagesReadOnly() {
    for (int i=0;i<this->pendingCodePages.size();i++) {
        // the chunk could cross a page and be a mix of dynamic and non dynamic code
        if (this->thread->memory->dynamicCodePageUpdateCount[this->pendingCodePages[i]]!=MAX_DYNAMIC_CODE_PAGE_COUNT) {
            ::makeCodePageReadOnly(this->thread->memory, this->pendingCodePages[i]);
        }
    }
    this->pendingCodePages.clear();
}

void* x64CPU::translateEip(U32 ip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);

    void* result = translateEipInternal(NULL, ip);
    makePendingCodePagesReadOnly();
    return result;
}

void x64CPU::translateInstruction(X64Asm* data, X64Asm* firstPass) {
    data->startOfOpIp = data->ip;        
#ifdef _DEBUG
    //data->logOp(data->ip);
    // just makes debugging the asm output easier
#ifndef __TEST
    data->writeToMemFromValue(data->ip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
#endif
#endif    
    if (data->dynamic) {
        data->addDynamicCheck(false);
    } else {
#ifdef _DEBUG
        data->addDynamicCheck(true);
#endif
    }    
    while (1) {  
        data->op = data->fetch8();            
        data->inst = data->baseOp + data->op;        
        if (!x64Decoder[data->inst](data)) {                
            break;
        }            
    }
    if (data->tmp1InUse || data->tmp2InUse || data->tmp3InUse) {
        kpanic("x64: instruction %X did not release tmp register", data->inst);
    }
}

void x64CPU::translateData(X64Asm* data, X64Asm* firstPass) {
    Memory* memory = data->cpu->thread->memory;

    U32 codePage = (data->ip+data->cpu->seg[CS].address) >> K_PAGE_SHIFT;
    if (this->thread->memory->dynamicCodePageUpdateCount[codePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
    while (1) {  
        U32 address = data->cpu->seg[CS].address+data->ip;
        void* hostAddress = this->thread->memory->getExistingHostAddress(address);
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
        if (firstPass) {
            U32 nextEipLen = firstPass->calculateEipLen(data->ip+data->cpu->seg[CS].address);
            U32 page = (data->ip+data->cpu->seg[CS].address+nextEipLen) >> K_PAGE_SHIFT;

            if (page!=codePage) {
                codePage = page;
                if (data->dynamic) {                    
                    if (this->thread->memory->dynamicCodePageUpdateCount[codePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // continue to cross from my dynamic page into another dynamic page
                    } else {
                        // we will continue to emit code that will self check for modified code, even though the page we spill into is not dynamic
                    }
                } else {
                    if (this->thread->memory->dynamicCodePageUpdateCount[codePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // we crossed a page boundry from a non dynamic page to a dynamic page
                        data->dynamic = true; // the instructions from this point on will do their own check
                    } else {
                        // continue to cross from one non dynamic page into another non dynamic page
                    }
                }
            }
        }
        data->mapAddress(address, data->bufferPos);
        U32 page = address >> K_PAGE_SHIFT;
        translateInstruction(data, firstPass);
        if (data->done) {
            break;
        }
        if (data->stopAfterInstruction!=-1 && data->ipAddressCount==data->stopAfterInstruction) {
            break;
        }
        data->resetForNewOp();
    }     
}

static U8 fetchByte(U32 *eip) {
    return readb((*eip)++);
}

DecodedOp* x64CPU::getOp(U32 eip, bool existing) {
    if (this->big) {
        eip+=this->seg[CS].address;
    } else {
        eip=this->seg[CS].address + (eip & 0xFFFF);
    }        
    if (!existing || (this->eipToHostInstruction[eip >> K_PAGE_SHIFT] && this->eipToHostInstruction[eip >> K_PAGE_SHIFT][eip & K_PAGE_MASK])) {
        THREAD_LOCAL static DecodedBlock* block;
        if (!block) {
            block = new DecodedBlock();
        }
        decodeBlock(fetchByte, eip, this->big, 4, 64, 1, block);
        return block->op;
    }
    return NULL;
}

#endif