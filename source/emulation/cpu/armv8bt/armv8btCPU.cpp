#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCPU.h"
#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../../hardmmu/hard_memory.h"
#include "../normal/normalCPU.h"
#include "ksignal.h"
#include "knativethread.h"
#include "knativesystem.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "armv8btCodeChunk.h"

#undef u8

CPU* CPU::allocCPU() {
    return new Armv8btCPU();
}

Armv8btCPU::Armv8btCPU() : exitToStartThreadLoop(0), regPage(0), regOffset(0) {
    sseConstants[vMaxInt32PlusOneAsDouble- vFirstSseConstant].pd.f64[0] = 2147483648.0;
    sseConstants[vMaxInt32PlusOneAsDouble - vFirstSseConstant].pd.f64[1] = 2147483648.0;
    sseConstants[vMinInt32MinusOneAsDouble - vFirstSseConstant].pd.f64[0] = -2147483649.0;
    sseConstants[vMinInt32MinusOneAsDouble - vFirstSseConstant].pd.f64[1] = -2147483649.0;

    sseConstants[vMaxInt32PlusOneAsFloat - vFirstSseConstant].ps.f32[0] = 2147483648.0;
    sseConstants[vMaxInt32PlusOneAsFloat - vFirstSseConstant].ps.f32[1] = 2147483648.0;
    sseConstants[vMaxInt32PlusOneAsFloat - vFirstSseConstant].ps.f32[2] = 2147483648.0;
    sseConstants[vMaxInt32PlusOneAsFloat - vFirstSseConstant].ps.f32[3] = 2147483648.0;
    sseConstants[vMinInt32MinusOneAsFloat - vFirstSseConstant].ps.f32[0] = -2147483649.0;
    sseConstants[vMinInt32MinusOneAsFloat - vFirstSseConstant].ps.f32[1] = -2147483649.0;
    sseConstants[vMinInt32MinusOneAsFloat - vFirstSseConstant].ps.f32[2] = -2147483649.0;
    sseConstants[vMinInt32MinusOneAsFloat - vFirstSseConstant].ps.f32[3] = -2147483649.0;

    sseConstants[vInt32BitMask - vFirstSseConstant].ps.u32[0] = 1;
    sseConstants[vInt32BitMask - vFirstSseConstant].ps.u32[1] = 2;
    sseConstants[vInt32BitMask - vFirstSseConstant].ps.u32[2] = 4;
    sseConstants[vInt32BitMask - vFirstSseConstant].ps.u32[3] = 8;

    sseConstants[vByte8BitMask - vFirstSseConstant].pi.u8[0] = 1;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[1] = 2;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[2] = 4;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[3] = 8;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[4] = 16;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[5] = 32;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[6] = 64;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[7] = 128;
    sseConstants[vByte8BitMask - vFirstSseConstant].pi.u8[8] = 1;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[9] = 2;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[10] = 4;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[11] = 8;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[12] = 16;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[13] = 32;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[14] = 64;
    sseConstants[vByte8BitMask - vFirstSseConstant].ps.u8[15] = 128;
}

typedef void (*StartCPU)();

void Armv8btCPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
}

void Armv8btCPU::run() {
    while (true) {
        this->memOffset = this->thread->process->memory->id;
		this->exitToStartThreadLoop = 0;
        if (setjmp(this->runBlockJump)==0) {
            StartCPU start = (StartCPU)this->init();
            start();
#ifdef __TEST
            return;
#endif
        }		
		if (this->thread->terminating) {
			break;
		}
		if (this->exitToStartThreadLoop) {
			Memory* previousMemory = this->thread->process->previousMemory;
			if (previousMemory && previousMemory->getRefCount() == 1) {
				// :TODO: this seem like a bad dependency that memory will access KThread::currentThread()
				Memory* currentMemory = this->thread->process->memory;
				this->thread->process->memory = previousMemory;
				this->thread->memory = previousMemory;
				delete previousMemory;
				this->thread->process->memory = currentMemory;
				this->thread->memory = currentMemory;
			}
			else {
				previousMemory->decRefCount();
			}
			this->thread->process->previousMemory = NULL;
		}
        if (this->inException) {
            this->inException = false;
        }		
    }
}

void Armv8btCPU::restart() {
	this->memOffset = this->thread->process->memory->id;
	this->exitToStartThreadLoop = true;
}

DecodedBlock* Armv8btCPU::getNextBlock() {
    return NULL;
}

void* Armv8btCPU::init() {
    Armv8btAsm data(this);
    void* result;
    Memory* memory = this->thread->memory;
    Armv8btCPU* cpu = this;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->executableMemoryMutex);
    this->eipToHostInstructionAddressSpaceMapping = this->thread->memory->eipToHostInstructionAddressSpaceMapping;

	data.saveNativeState();

    data.writeToRegFromValue(xCPU, (U64)this);
    data.writeToRegFromValue(xMem, cpu->memOffset);

    if (KSystem::useLargeAddressSpace) {
        data.writeToRegFromValue(xLargeAddress, (U64)cpu->eipToHostInstructionAddressSpaceMapping);
    }

    data.writeToRegFromValue(xES, (U32)cpu->seg[ES].address);    
    data.writeToRegFromValue(xCS, (U32)cpu->seg[CS].address);
    data.writeToRegFromValue(xSS, (U32)cpu->seg[SS].address);
    data.writeToRegFromValue(xDS, (U32)cpu->seg[DS].address);
    data.writeToRegFromValue(xFS, (U32)cpu->seg[FS].address);
    data.writeToRegFromValue(xGS, (U32)cpu->seg[GS].address);

    data.writeToRegFromValue(xStackMask, (U32)cpu->stackMask);

    data.setNativeFlags(this->flags, FMASK_TEST|DF|ID);

    data.writeToRegFromValue(xEAX, EAX);
    data.writeToRegFromValue(xECX, ECX);
    data.writeToRegFromValue(xEDX, EDX);
    data.writeToRegFromValue(xEBX, EBX);
    data.writeToRegFromValue(xESP, ESP);
    data.writeToRegFromValue(xEBP, EBP);
    data.writeToRegFromValue(xESI, ESI);
    data.writeToRegFromValue(xEDI, EDI);        
    
    data.calculatedEipLen = 1; // will force the long x64 chunk jump
    data.doJmp(false);
    std::shared_ptr<BtCodeChunk> chunk = data.commit(true);
    result = chunk->getHostAddress();
    link(&data, chunk);
    this->pendingCodePages.clear();    
    this->eipToHostInstructionPages = this->thread->memory->eipToHostInstructionPages;

    if (!this->thread->process->returnToLoopAddress) {
        Armv8btAsm returnData(this);
        returnData.restoreNativeState();
        returnData.addReturn();
        std::shared_ptr<BtCodeChunk> chunk2 = returnData.commit(true);
        this->thread->process->returnToLoopAddress = chunk2->getHostAddress();
    }
    this->returnToLoopAddress = this->thread->process->returnToLoopAddress;

    if (!this->thread->process->reTranslateChunkAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddress = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddress = this->thread->process->reTranslateChunkAddress;
    if (!this->thread->process->reTranslateChunkAddressFromR9) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddressFromR9 = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddressFromR9 = this->thread->process->reTranslateChunkAddressFromR9;
#ifdef BOXEDWINE_X64_DEBUG_NO_EXCEPTIONS
    if (!this->thread->process->jmpAndTranslateIfNecessaryToR9) {
        Armv8btAsm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessaryToR9 = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessaryToR9 = this->thread->process->jmpAndTranslateIfNecessaryToR9;
#endif
#ifdef BOXEDWINE_POSIX
    if (!this->thread->process->runSignalAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRunSignal();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->runSignalAddress = chunk3->getHostAddress();
    }
#endif
    return result;
}

std::shared_ptr<BtCodeChunk> Armv8btCPU::translateChunk(U32 ip) {
    return translateChunk(NULL, ip);
}

std::shared_ptr<BtCodeChunk> Armv8btCPU::translateChunk(Armv8btAsm* parent, U32 ip) {
    Armv8btAsm data1(this);
    data1.ip = ip;
    data1.startOfDataIp = ip;       
    data1.parent = parent;
    translateData(&data1);

    Armv8btAsm data(this);
    data.ip = ip;
    data.startOfDataIp = ip;  
    data.calculatedEipLen = data1.ip - data1.startOfDataIp;
    data.parent = parent;
    translateData(&data, &data1);        
    S32 failedJumpOpIndex = this->preLinkCheck(&data);

    if (failedJumpOpIndex==-1) {
        std::shared_ptr<BtCodeChunk> chunk = data.commit(false);
        link(&data, chunk);
        return chunk;
    } else {
        Armv8btAsm data2(this);
        data2.ip = ip;
        data2.startOfDataIp = ip;       
        data2.parent = parent;
        data2.stopAfterInstruction = failedJumpOpIndex;
        translateData(&data2);

        Armv8btAsm data3(this);
        data3.ip = ip;
        data3.startOfDataIp = ip;  
        data3.calculatedEipLen = data2.ip - data2.startOfDataIp;
        data3.parent = parent;
        data3.stopAfterInstruction = failedJumpOpIndex;
        translateData(&data3, &data2);

        std::shared_ptr<BtCodeChunk> chunk = data3.commit(false);
        link(&data3, chunk);
        return chunk;
    }    
}

void* Armv8btCPU::translateEipInternal(Armv8btAsm* parent, U32 ip) {
    if (!this->isBig()) {
        ip = ip & 0xFFFF;
    }
    U32 address = this->seg[CS].address+ip;
    void* result = this->thread->memory->getExistingHostAddress(address);

    if (!result) {
        std::shared_ptr<BtCodeChunk> chunk = this->translateChunk(parent, ip);
        result = chunk->getHostAddress();
        chunk->makeLive();
    }
    return result;
}

#ifdef __TEST
void Armv8btCPU::addReturnFromTest() {
    Armv8btAsm data(this);
    data.addReturnFromTest();
    data.commit(true);
}
#endif

S32 Armv8btCPU::preLinkCheck(Armv8btAsm* data) {
    for (S32 i=0;i<(S32)data->todoJump.size();i++) {
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
            if (!found && !this->translateEip(data->todoJump[i].eip)) {
                return data->todoJump[i].opIndex;
            }
        }
    }
    return -1;
}

void Armv8btCPU::writeJumpAmount(Armv8btAsm* data, U32 pos, U32 toLocation, U8* offset) {
    S32 amount = (S32)(toLocation) >> 2;
    if (offset[pos + 3] == 0x14) {
        if (amount > 0xFFFFFF) {
            kpanic("Armv8btCPU::writeJumpAmount in large jump not supported: %d", amount);
        }
        offset[pos] = (U8)amount;
        offset[pos + 1] = (U8)(amount >> 8);
        offset[pos + 2] = (U8)(amount >> 16);
        offset[pos + 3] |= (U8)((amount >> 24) & 3);
    } else if (offset[pos + 3] == 0x34 || offset[pos + 3] == 0x35) {
        if (amount >= 0x40000 || amount <= -0x40000) {
            kpanic("Armv8btCPU::writeJumpAmount in large jump not supported: %d", amount);
        }
        offset[pos] |= (U8)(amount << 5);
        offset[pos + 1] = (U8)(amount >> 3);
        offset[pos + 2] = (U8)(amount >> 11);
    } else {
        kpanic("Armv8btCPU::writeJumpAmount unknown jump: %d", offset[pos + 3]);
    }
}

void Armv8btCPU::link(Armv8btAsm* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk) {
    U32 i;
    if (!fromChunk) {
        kpanic("Armv8btCPU::link fromChunk missing");
    }
    for (i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;        
        U8* offset = (U8*)fromChunk->getHostAddress()+offsetIntoChunk+data->todoJump[i].bufferPos;
        U8 size = data->todoJump[i].offsetSize;
        U8* host = NULL;

        if (size==4 && (host = (U8*)fromChunk->getHostFromEip(eip))) {
            writeJumpAmount(data, data->todoJump[i].bufferPos, (U32)(host - offset), (U8*)fromChunk->getHostAddress() + offsetIntoChunk);
        } else if (size==4) {
            U8* toHostAddress = (U8*)this->thread->memory->getExistingHostAddress(eip);

            if (!toHostAddress) {
                U8 op = 0xce;
                U32 hostIndex = 0;
                std::shared_ptr<BtCodeChunk> chunk = std::make_shared<Armv8CodeChunk>(1, &eip, &hostIndex, &op, 1, eip-this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();            
            }
            std::shared_ptr<BtCodeChunk> toChunk = this->thread->memory->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("Armv8btCPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, true);
            writeJumpAmount(data, data->todoJump[i].bufferPos, (U32)(toHostAddress - offset), (U8*)fromChunk->getHostAddress() + offsetIntoChunk);
        } else if (size==8 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)this->thread->memory->getExistingHostAddress(eip);

            if (!toHostAddress) {
                Armv8btAsm returnData(this);
                returnData.startOfOpIp = eip - this->seg[CS].address;
                returnData.callRetranslateChunk();
                U32 hostIndex = 0;
                std::shared_ptr<BtCodeChunk> chunk = std::make_shared<Armv8CodeChunk>(1, &eip, &hostIndex, returnData.buffer, returnData.bufferPos, eip - this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();
            }
            std::shared_ptr<BtCodeChunk> toChunk = this->thread->memory->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("Armv8btCPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, false);
            data->write64Buffer(offset, (U64)&(link->toHostInstruction));
        } else {
            kpanic("Armv8btCPU::link unexpected patch size");
        }
    }
    if (data->todoJump.size()) {

    }
    markCodePageReadOnly(data);
}

void Armv8btCPU::markCodePageReadOnly(Armv8btAsm* data) {
    U32 pageStart = (data->startOfDataIp+this->seg[CS].address) >> K_PAGE_SHIFT;
    U32 pageEnd = (data->startOfDataIp+this->seg[CS].address) >> K_PAGE_SHIFT;
    S32 pageCount = pageEnd-pageStart+1;

#ifndef __TEST
    for (int i=0;i<pageCount;i++) {        
        pendingCodePages.push_back(pageStart+i);        
    }
#endif    
}

void Armv8btCPU::makePendingCodePagesReadOnly() {
    for (int i=0;i<(int)this->pendingCodePages.size();i++) {
        // the chunk could cross a page and be a mix of dynamic and non dynamic code
        if (this->thread->memory->dynamicCodePageUpdateCount[this->pendingCodePages[i]]!=MAX_DYNAMIC_CODE_PAGE_COUNT) {
            ::makeCodePageReadOnly(this->thread->memory, this->pendingCodePages[i]);
        }
    }
    this->pendingCodePages.clear();
}

void* Armv8btCPU::translateEip(U32 ip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);

    void* result = translateEipInternal(NULL, ip);
    makePendingCodePagesReadOnly();
    return result;
}

static U8 fetchByte(U32* eip) {
    return readb((*eip)++);
}

void Armv8btCPU::translateInstruction(Armv8btAsm* data, Armv8btAsm* firstPass) {
    data->startOfOpIp = data->ip;    
    data->ip += data->decodedOp->len;   
#ifdef _DEBUG
    //data->logOp(data->startOfOpIp);
    // just makes debugging the asm output easier
#ifndef __TEST
    data->loadConst(14, data->startOfOpIp);
    //data->writeMem32ValueOffset(xTmp5, xCPU, CPU_OFFSET_EIP);
#endif
#endif
    if (data->dynamic) {
        data->addDynamicCheck(false);
    } else {
#ifdef _DEBUG
        //data->addDynamicCheck(true);
#endif
    }   
    armv8btEncoder[data->decodedOp->inst](data);
}

void Armv8btCPU::translateData(Armv8btAsm* data, Armv8btAsm* firstPass) {
    Memory* memory = data->cpu->thread->memory;

    U32 codePage = (data->ip+data->cpu->seg[CS].address) >> K_PAGE_SHIFT;
    if (this->thread->memory->dynamicCodePageUpdateCount[codePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
    DecodedBlock block;
    data->currentBlock = &block;
    decodeBlock(fetchByte, data->startOfDataIp + this->seg[CS].address, this->isBig(), 0, 0, 0, &block);
    DecodedOp* op = block.op;
    while (op) {  
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
        if (firstPass && firstPass->fpuTopRegSet && !data->fpuOffsetRegSet) {
            data->getFpuTopReg();
        }
        U32 page = address >> K_PAGE_SHIFT;
        data->decodedOp = op;
        translateInstruction(data, firstPass);
        op = op->next;
        if (data->done) {
            break;
        }
        if (data->stopAfterInstruction!=-1 && (int)data->ipAddressCount==data->stopAfterInstruction) {
            break;
        }
        for (int i = 0; i < xNumberOfTmpRegs; i++) {
            if (data->tmpRegInUse[i]) {
                kpanic("op(%x) leaked tmp reg", data->decodedOp->originalOp);
            }
        }        
        data->resetForNewOp();
        if (!op) {
            block.op->dealloc(true);
            decodeBlock(fetchByte, data->startOfOpIp + this->seg[CS].address, this->isBig(), 0, 0, 0, &block);
            op = block.op;
        }
    }     
    block.op->dealloc(true);
    data->currentBlock = NULL;
}

DecodedOp* Armv8btCPU::getOp(U32 eip, bool existing) {
    if (this->isBig()) {
        eip+=this->seg[CS].address;
    } else {
        eip=this->seg[CS].address + (eip & 0xFFFF);
    }        
    if (!existing || this->thread->memory->getExistingHostAddress(eip)) {
        THREAD_LOCAL static DecodedBlock* block;
        if (!block) {
            block = new DecodedBlock();
        }
        decodeBlock(fetchByte, eip, this->isBig(), 4, 64, 1, block);
        return block->op;
    }
    return NULL;
}

U64 Armv8btCPU::handleCodePatch(U64 rip, U32 address) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
#endif
    // get the emulated eip of the op that corresponds to the host address where the exception happened
    std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress((void*)rip);
    this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)rip, NULL, NULL)-this->seg[CS].address;

    // get the emulated op that caused the write
    DecodedOp* op = this->getOp(this->eip.u32, true);
    if (op) {             
        // change permission of the page so that we can write to it
        U32 len = instructionInfo[op->inst].writeMemWidth/8;
        Armv8btAsmCodeMemoryWrite w(this);        
        static DecodedBlock b;
        DecodedBlock::currentBlock = &b;
        b.next1 = &b;
        b.next2 = &b;
        // do the write
        op->pfn = NormalCPU::getFunctionForOp(op);
        op->next = DecodedOp::alloc();
        op->next->inst = Done;
        op->next->pfn = NormalCPU::getFunctionForOp(op->next);

        if (this->flags & DF) {
            this->df = -1;
        } else {
            this->df = 1;
        }
        // (Examples: diablo 1 will trigger this in the middle of the Stosd when creating a new game)
        if (op->inst==Stosb || op->inst==Stosw || op->inst==Stosd ||
            op->inst==Scasb || op->inst==Scasw || op->inst==Scasd || 
            op->inst==Movsb || op->inst==Movsw || op->inst==Movsd) {
            w.invalidateStringWriteToDi(op->repNotZero || op->repZero, instructionInfo[op->inst].writeMemWidth/8);
        } else {
            w.invalidateCode(address, len);
        }  
        FILE* f = (FILE*)this->logFile;
        this->logFile = NULL;       
        op->pfn(this, op);   
        this->logFile = f;        

        // eip was ajusted after running this instruction                        
        U32 a = this->getEipAddress();
        if (!this->thread->memory->getExistingHostAddress(a)) {
            this->translateEip(this->eip.u32);
        }
        U64 result = (U64)this->thread->memory->getExistingHostAddress(a);
        if (result==0) {
            kpanic("Armv8btCPU::handleCodePatch failed to translate code");
        }
        op->dealloc(true);
        return result;
    } else {                        
        kpanic("Threw an exception from a host location that doesn't map to an emulated instruction");
    }
    return 0;
}

U64 Armv8btCPU::handleChangedUnpatchedCode(U64 rip) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
#endif
                        
    unsigned char* hostAddress = (unsigned char*)rip;
    std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress(hostAddress);
    if (!chunk) {
        kpanic("Armv8btCPU::handleChangedUnpatchedCode: could not find chunk");
    }
    U32 startOfEip = chunk->getEipThatContainsHostAddress(hostAddress, NULL, NULL);
    if (!chunk->isDynamicAware() || !chunk->retranslateSingleInstruction(this, hostAddress)) {        
        chunk->releaseAndRetranslate();   
    }
    U64 result = (U64)this->thread->memory->getExistingHostAddress(startOfEip);
    if (result==0) {
        result = (U64)this->translateEip(startOfEip-this->seg[CS].address);
    }
    if (result==0) {
        kpanic("Armv8btCPU::handleChangedUnpatchedCode failed to translate code in exception");
    }
    return result;
}

U64 Armv8btCPU::reTranslateChunk() {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
#endif
    std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingEip(this->eip.u32 + this->seg[CS].address);
    if (chunk) {
        chunk->releaseAndRetranslate();
    }    

    U64 result = (U64)this->thread->memory->getExistingHostAddress(this->eip.u32 + this->seg[CS].address);
    if (result == 0) {
        result = (U64)this->translateEip(this->eip.u32);
    }
    if (result == 0) {
        kpanic("Armv8btCPU::reTranslateChunk failed to translate code in exception");
    }
    return result;
}

U64 Armv8btCPU::handleMissingCode(U64 regPage, U64 regOffset, U32 inst) {
    U32 page = (U32)regPage;
    U32 offset = (U32)regOffset;

    this->eip.u32 = ((page << K_PAGE_SHIFT) | offset) - this->seg[CS].address;
    return (U64)this->translateEip(this->eip.u32);  
}

U64 Armv8btCPU::handleIllegalInstruction(U64 rip) {    
    if (*((U8*)rip)==0xce) {            
        return this->handleChangedUnpatchedCode(rip);
    } 
    if (*((U8*)rip)==0xcd) { 
        // free'd chunks are filled in with 0xcd, if this one is free'd, it is possible another thread replaced the chunk
        // while this thread jumped to it and this thread waited in the critical section at the top of this function.
        void* host = this->thread->memory->getExistingHostAddress(this->eip.u32+this->seg[CS].address);
        if (host) {
            return (U64)host;
        } else {
            kpanic("Armv8btCPU::handleIllegalInstruction tried to run code in a free'd chunk");
        }
    }
    return 0;
}

U32 dynamicCodeExceptionCount;

#define JMP_PAGE_EXCEPTION 0xf86f7929
#define JMP_OFFSET_EXCEPTION 0x38400131

U64 Armv8btCPU::handleAccessException(U64 ip, U64 address, bool readAddress) {
    U32 inst = *((U32*)ip);

    if (inst == 0xf8400149) { // ldur x9, [x9]
        return (U64) this->translateEip(this->destEip - this->seg[CS].address);
    } else if (inst == JMP_OFFSET_EXCEPTION || inst == JMP_PAGE_EXCEPTION) {
        return this->handleMissingCode(this->regPage, this->regOffset, inst);
    } else if (inst==0xcdcdcdcd) {
        // this thread was waiting on the critical section and the thread that was currently in this handler removed the code we were running
        void* host = this->thread->memory->getExistingHostAddress(this->eip.u32+this->seg[CS].address);
        if (host) {
            return (U64)host;
        } else {
            U64 result = (U64)this->translateEip(this->eip.u32); 
            if (!result) {
                kpanic("Armv8btCPU::handleAccessException tried to run code in a free'd chunk");
            }
            return result;
        }
    } else {          
        // check if the emulated memory caused the exception
        if ((address & 0xFFFFFFFF00000000l) == this->thread->memory->id) {                
            U32 emulatedAddress = (U32)address;
            
            // check if emulated memory that caused the exception is a page that has code
            if (this->thread->memory->nativeFlags[emulatedAddress>>K_PAGE_SHIFT] & NATIVE_FLAG_CODEPAGE_READONLY) {                    
                dynamicCodeExceptionCount++;                    
                return this->handleCodePatch(ip, emulatedAddress);                    
            }
        }   
#ifdef _DEBUG
        //void* fromHost = this->thread->memory->getExistingHostAddress(this->fromEip);
        //std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress((void*)rip);
#endif
        // this can be exercised with Wine 5.0 and CC95 demo installer, it is triggered in strlen as it tries to grow the stack
        this->thread->seg_mapper((U32)address, readAddress, !readAddress, false);
        U64 result = (U64)this->translateEip(this->eip.u32); 
        if (result==0) {
            kpanic("Armv8btCPU::handleAccessException failed to translate code");
        }
        return result;
    }
}

U64 Armv8btCPU::handleFpuException(int code, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) {
    if (doSyncFrom) {
        doSyncFrom(NULL);
    }
    if (code == K_FPE_INTDIV) {
        this->prepareException(EXCEPTION_DIVIDE, 0);
    } else if (code == K_FPE_INTOVF) {
        this->prepareException(EXCEPTION_DIVIDE, 1);
    } else {
        this->prepareFpuException(code);
    }
    if (doSyncTo) {
        doSyncTo(NULL);
    }
    U64 result = (U64)this->translateEip(this->eip.u32); 
    if (result==0) {
        kpanic("Armv8btCPU::handleFpuException failed to translate code");
    }
    return result;
}


U64 Armv8btCPU::startException(U64 address, bool readAddress, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) {
    if (this->thread->terminating) {
        return (U64)this->returnToLoopAddress;                
    }
    if (this->inException) {
        if (doSyncFrom) {
            doSyncFrom(NULL);
        }
        this->thread->seg_mapper((U32)address, readAddress, !readAddress);
        if (doSyncTo) {
            doSyncTo(NULL);
        }
        U64 result = (U64)this->translateEip(this->eip.u32); 
        if (result==0) {
            kpanic("Armv8btCPU::startException failed to translate code in exception");
        }
        return result;
    } 
    return 0;
}

extern U32 platformThreadCount;

void Armv8btCPU::startThread() {
    jmp_buf jmpBuf;
    U32 threadId = thread->id;
    U32 processId = thread->process->id;

    KThread::setCurrentThread(thread);       

    // :TODO: hopefully this will eventually go away.  For now this prevents a signal from being generated which isn't handled yet
    KNativeThread::sleep(50);   

    if (!setjmp(jmpBuf)) {
        this->jmpBuf = &jmpBuf;
        this->run();
    }
    std::shared_ptr<KProcess> process = thread->process;
	process->deleteThread(thread);

    platformThreadCount--;
    if (platformThreadCount==0) {
        KSystem::shutingDown = true;
        KNativeSystem::postQuit();
    }
}

// called from another thread
void Armv8btCPU::wakeThreadIfWaiting() {
    BoxedWineCondition* cond = thread->waitingCond;

	// wait up the thread if it is waiting
    if (cond) {
        cond->lock();
        cond->signal();
        cond->unlock();
    }    
}

void addTimer(KTimer* timer) {
    kwarn("addTimer not implemented yet");
}

void removeTimer(KTimer* timer) {
    if (timer->active)
        kpanic("removeTimer not implemented yet");
}

void terminateOtherThread(const std::shared_ptr<KProcess>&  process, U32 threadId) {
	process->threadsCondition.lock();
	KThread* thread = process->getThreadById(threadId);
	if (thread) {
		thread->terminating = true;
		((Armv8btCPU*)thread->cpu)->exitToStartThreadLoop = true;
		((Armv8btCPU*)thread->cpu)->wakeThreadIfWaiting();
	}
	process->threadsCondition.unlock();

	while (true) {
		BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(process->threadsCondition);		
		if (!process->getThreadById(threadId)) {
			break;
		}
		BOXEDWINE_CONDITION_WAIT_TIMEOUT(process->threadsCondition, 1000);
	}
}

void terminateCurrentThread(KThread* thread) {
	thread->terminating = true;
	((Armv8btCPU*)thread->cpu)->exitToStartThreadLoop = true;
}

void unscheduleThread(KThread* thread) {
}

#endif
