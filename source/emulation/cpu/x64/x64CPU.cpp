#include "boxedwine.h"

#ifdef BOXEDWINE_X64
#include "x64Ops.h"
#include "x64CPU.h"
#include "x64Asm.h"
#include "../../hardmmu/hard_memory.h"
#include "x64CodeChunk.h"

x64CPU::x64CPU() : nativeHandle(0), jmpBuf(NULL), endCond("x64CPU::endcond"), inException(false) {
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
        if (setjmp(this->runBlockJump)==0) {
            StartCPU start = (StartCPU)this->init();
#ifdef __TEST
            addReturnFromTest();
#endif
            start();
#ifdef __TEST
            return;
#endif
        }
    }
}

void x64CPU::restart() {
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
    
    void* existingHostAddress = cpu->thread->memory->getExistingHostAddress(cpu->eip.u32);
    data.jumpTo(this->eip.u32);
    result = data.commit();
    link(&data, result);  
    this->eipToHostInstruction = this->thread->memory->eipToHostInstruction;
    return result;
}

void* x64CPU::translateEipInternal(X64Asm* parent, U32 ip) {
    if (!this->big) {
        ip = ip & 0xFFFF;
    }
    U32 address = this->seg[CS].address+ip;
    void* result = this->thread->memory->getExistingHostAddress(address);

    if (!result) {
        X64Asm data(this);
        data.ip = ip;
        data.startOfDataIp = ip;       
        data.parent = parent;
        translateData(&data);
        result = data.commit();
        link(&data, result);        
    }
    return result;
}

#ifdef __TEST
void x64CPU::addReturnFromTest() {
    X64Asm data(this);
    data.addReturnFromTest();
    data.commit();
}
#endif

void x64CPU::link(X64Asm* data, void* address) {
    U32 i;

    for (i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;
        void* hostAddress = this->thread->memory->getExistingHostAddress(eip);
        if (!hostAddress) {
            X64Asm* p = data->parent;
            bool found = false;

            while (p && !found) {
                U32 i;
                for (i=0;i<p->ipAddressCount;i++) {
                    if (p->ipAddress[i]==eip) {
                        p->todoJump.push_back(X64Data::TodoJump(eip, p->ipAddressBufferPos[i], 4));                        
                        found = true;
                        break;
                    }
                }
                p = p->parent;
            }
            if (found) {
                continue;
            }
            if (!data->cpu->thread->memory->isValidReadAddress(eip, 1)) {
                // hope that the program will fill this in later
                continue;
            }
            if (readb(eip) || readb(eip+1) || readb(eip+2) || readb(eip+3)) {
                hostAddress = translateEipInternal(data, data->todoJump[i].eip);
            } else {
                int ii=0;
            }
        }

        U8* translatedOffset = (U8*)translateEipInternal(data, data->todoJump[i].eip);
        U8* offset = (U8*)address+data->todoJump[i].bufferPos;
        U8 size = data->todoJump[i].offsetSize;

        if (size==4) {
            data->write32Buffer(offset, (U32)(translatedOffset - offset - 4));

            X64CodeChunk* fromChunk = this->thread->memory->getCodeChunkContainingHostAddress(offset);
            X64CodeChunk* toChunk = this->thread->memory->getCodeChunkContainingHostAddress(translatedOffset);
            if (fromChunk != toChunk && fromChunk && toChunk) {
                toChunk->addLinkFrom(fromChunk, eip, translatedOffset, offset);
            }
        } else {
            kpanic("x64CPU only supports 4 byte jumps so that we can patch it easily later");
        }
    }
    markCodePageReadOnly(data);
}

void x64CPU::markCodePageReadOnly(X64Asm* data) {
    S32 pageCount = (data->ip-data->startOfDataIp+K_PAGE_MASK) >> K_PAGE_SHIFT;
    U32 pageStart = (data->ip+this->seg[CS].address) >> K_PAGE_SHIFT;
#ifndef __TEST
    for (int i=0;i<pageCount;i++) {
        pendingCodePages.push_back(pageStart+i);        
    }
#endif
}

void x64CPU::makePendingCodePagesReadOnly() {
    for (int i=0;i<this->pendingCodePages.size();i++) {
        ::makeCodePageReadOnly(this->thread->memory, this->pendingCodePages[i]);
    }
    this->pendingCodePages.clear();
}

void* x64CPU::translateEip(U32 ip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
    void* result = translateEipInternal(NULL, ip);
    makePendingCodePagesReadOnly();
    return result;
}

void x64CPU::translateInstruction(X64Asm* data) {
//
#ifdef _DEBUG
    //data->logOp(data->ip);
    // just makes debugging the asm output easier
    data->writeToMemFromValue(data->ip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
#endif
    if (data->ip==0x54C562) {
        int ii=0;
    }
    data->startOfOpIp = data->ip;        
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

void x64CPU::translateData(X64Asm* data) {
    Memory* memory = data->cpu->thread->memory;

    while (1) {  
        U32 address = data->cpu->seg[CS].address+data->ip;
        void* hostAddress = this->thread->memory->getExistingHostAddress(address);
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
        data->mapAddress(address, data->bufferPos);
        translateInstruction(data);
        if (data->done) {
            break;
        }
#ifdef __TEST
        break;
#endif
        data->resetForNewOp();
    }     
}

static U8 fetchByte(U32 *eip) {
    return readb((*eip)++);
}

DecodedOp* x64CPU::getExistingOp(U32 eip) {
    if (this->big) {
        eip+=this->seg[CS].address;
    } else {
        eip=this->seg[CS].address + (eip & 0xFFFF);
    }        
    if (this->eipToHostInstruction[eip >> K_PAGE_SHIFT] && this->eipToHostInstruction[eip >> K_PAGE_SHIFT][eip & K_PAGE_MASK]) {
        static DecodedBlock* block;
        if (!block) {
            block = new DecodedBlock();
        }
        decodeBlock(fetchByte, eip, this->big, 4, 64, 1, block);
        return block->op;
    }
    return NULL;
}

#endif