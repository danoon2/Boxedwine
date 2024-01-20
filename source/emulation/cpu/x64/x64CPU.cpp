#include "boxedwine.h"

#ifdef BOXEDWINE_X64
#include "x64Ops.h"
#include "x64CPU.h"
#include "x64Asm.h"
#include "x64CodeChunk.h"
#include "../../softmmu/kmemory_soft.h"
#include "../../hardmmu/kmemory_hard.h"
#include "../normal/normalCPU.h"

CPU* CPU::allocCPU(KMemory* memory) {
    return new x64CPU(memory);
}

// hard to guage the benifit, seems like 1% to 3% with quake 2 and quake 3
bool x64CPU::hasBMI2 = true;
bool x64Intialized = false;

x64CPU::x64CPU(KMemory* memory) : BtCPU(memory) {
    if (!x64Intialized) {
        x64Intialized = true;
        x64CPU::hasBMI2 = platformHasBMI2();
    }
    largeAddressJumpInstruction = 0xCE24FF43;
    pageJumpInstruction = 0x0A8B4566;
    pageOffsetJumpInstruction = 0xCA148B4F;
}

void x64CPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
    this->negSegAddress[index] = (U32)(-((S32)(this->seg[index].address)));
}

void x64CPU::restart() {
#ifdef BOXEDWINE_64BIT_MMU
    KMemoryData* mem = getMemData(memory);
	this->memOffset = mem->id;
	this->negMemOffset = (U64)(-(S64)this->memOffset);
#endif
	for (int i = 0; i < 6; i++) {
		this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
	}
	this->exitToStartThreadLoop = true;
}

void* x64CPU::init() {
    X64Asm data(this);
    void* result;
    KMemoryData* mem = getMemData(memory);
    x64CPU* cpu = this;

#ifdef BOXEDWINE_64BIT_MMU
    this->memOffsets = mem->memOffsets;
    this->negMemOffset = (U64)(-(S64)this->memOffset);
#endif
    for (int i = 0; i < 6; i++) {
        this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
    }

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
#ifdef BOXEDWINE_64BIT_MMU
    this->eipToHostInstructionAddressSpaceMapping = mem->eipToHostInstructionAddressSpaceMapping;
    this->memOffsets = mem->memOffsets;
#endif

	// will push 15 regs, since it is odd, it will balance rip being pushed on the stack and give use a 16-byte alignment
	data.saveNativeState(); // also sets HOST_CPU

    //data.writeToRegFromValue(HOST_CPU, true, (U64)this, 8);
#ifdef BOXEDWINE_64BIT_MMU
    data.writeToRegFromValue(HOST_MEM, true, cpu->memOffset, 8);
    if (KSystem::useLargeAddressSpace) {
        data.writeToRegFromValue(HOST_LARGE_ADDRESS_SPACE_MAPPING, true, (U64)cpu->eipToHostInstructionAddressSpaceMapping, 8);
    } else {
        data.writeToRegFromValue(HOST_SMALL_ADDRESS_SPACE_SS, true, (U32)cpu->seg[SS].address, 4);
    }
#else 
    KMemoryData* memData = getMemData(memory);
    data.writeToRegFromValue(HOST_MEM_READ, true, (U64)memData->mmuReadPtrAdjusted, 8);
    data.writeToRegFromValue(HOST_MEM_WRITE, true, (U64)memData->mmuWritePtrAdjusted, 8);
#endif
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
    data.doJmp(false);
    std::shared_ptr<BtCodeChunk> chunk = data.commit(true);
    result = chunk->getHostAddress();
    //link(&data, chunk);
    this->pendingCodePages.clear();    
    this->eipToHostInstructionPages = mem->eipToHostInstructionPages;

    if (!this->thread->process->returnToLoopAddress) {
        X64Asm returnData(this);
        returnData.restoreNativeState();
        returnData.write8(0xfc); // cld
        returnData.write8(0xc3); // retn
        std::shared_ptr<BtCodeChunk> chunk2 = returnData.commit(true);
        this->thread->process->returnToLoopAddress = chunk2->getHostAddress();
    }
    this->returnToLoopAddress = this->thread->process->returnToLoopAddress;

    if (!this->thread->process->reTranslateChunkAddress) {
        X64Asm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddress = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddress = this->thread->process->reTranslateChunkAddress;
    if (!this->thread->process->syncToHostAddress) {
        X64Asm translateData(this);
        translateData.createCodeForSyncToHost();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->syncToHostAddress = chunk3->getHostAddress();
    }
    this->syncToHostAddress = this->thread->process->syncToHostAddress;
    if (!this->thread->process->syncFromHostAddress) {
        X64Asm translateData(this);
        translateData.createCodeForSyncFromHost();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->syncFromHostAddress = chunk3->getHostAddress();
    }
    this->syncFromHostAddress = this->thread->process->syncFromHostAddress;    
#ifdef BOXEDWINE_64BIT_MMU
    if (!this->thread->process->reTranslateChunkAddressFromReg) {
        X64Asm translateData(this);
        translateData.createCodeForRetranslateChunk(true);
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddressFromReg = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddressFromReg = this->thread->process->reTranslateChunkAddressFromReg;
#else
    if (!this->thread->process->doSingleOpAddress) {
        X64Asm translateData(this);
        translateData.createCodeForDoSingleOp();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->doSingleOpAddress = chunk3->getHostAddress();
    }
    this->doSingleOpAddress = this->thread->process->doSingleOpAddress;
#endif
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        X64Asm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary(true);
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessary = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
#endif
#ifdef BOXEDWINE_POSIX
    if (!this->thread->process->runSignalAddress) {
        X64Asm translateData(this);
        translateData.createCodeForRunSignal();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->runSignalAddress = chunk3->getHostAddress();
    }
#endif
    return result;
}

std::shared_ptr<BtData> x64CPU::createData() {
    return std::make_shared<X64Asm>(this);
}

#ifdef __TEST
void x64CPU::postTestRun() {
    for (int i = 0; i < 8; i++) {
        reg_mmx[i].q = fpuState.st_mm[i].low;
    }
    for (int i = 0; i < 8; i++) {
        xmm[i].pi.u64[0] = fpuState.xmm[i].low;
        xmm[i].pi.u64[1] = fpuState.xmm[i].high;
    }
}

void x64CPU::addReturnFromTest() {
    X64Asm data(this);
    data.addReturnFromTest();
    data.commit(true);
}
#endif

void x64CPU::link(const std::shared_ptr<BtData>& data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk) {
    U32 i;
    KMemoryData* mem = getMemData(memory);

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
            U8* toHostAddress = (U8*)mem->getExistingHostAddress(eip);

            if (!toHostAddress) {
                U8 op = 0xce;
                U32 hostIndex = 0;
                std::shared_ptr<X64CodeChunk> chunk = std::make_shared<X64CodeChunk>(1, &eip, &hostIndex, &op, 1, eip-this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();            
            }
            std::shared_ptr<BtCodeChunk> toChunk = mem->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("x64CPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, true);
            data->write32Buffer(offset, (U32)(toHostAddress - offset - 4));            
        } else if (size==8 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)mem->getExistingHostAddress(eip);

            if (!toHostAddress) {
                X64Asm returnData(this);
                returnData.startOfOpIp = eip - this->seg[CS].address;
                returnData.callRetranslateChunk();
                U32 hostIndex = 0;
                std::shared_ptr<X64CodeChunk> chunk = std::make_shared<X64CodeChunk>(1, &eip, &hostIndex, returnData.buffer, returnData.bufferPos, eip - this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();
            }
            std::shared_ptr<BtCodeChunk> toChunk = mem->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("x64CPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, false);
            data->write64Buffer(offset, (U64)&(link->toHostInstruction));
        } else {
            kpanic("x64CPU::link unexpected patch size");
        }
    }
#ifdef BOXEDWINE_64BIT_MMU
    markCodePageReadOnly(data.get());
#endif
}

void x64CPU::translateData(const std::shared_ptr<BtData>& data, const std::shared_ptr<BtData>& firstPass) {
    KMemoryData* mem = getMemData(memory);
#ifdef BOXEDWINE_64BIT_MMU
    U32 codePage = (data->ip+this->seg[CS].address) >> K_PAGE_SHIFT;    
    U32 nativePage = mem->getNativePage(codePage);
    if (mem->dynamicCodePageUpdateCount[nativePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
#endif    
    data->firstPass = firstPass;
    data->currentOp = nullptr;
    data->currentBlock = nullptr;
    DecodedOp* prevOp = nullptr;

    while (1) {  
        U32 address = this->seg[CS].address+data->ip;
        void* hostAddress = mem->getExistingHostAddress(address);
        if (!data->currentOp) {
            U32 address = this->seg[CS].address + data->ip;
            DecodedBlock* prev = data->currentBlock;
            if (prev) {
                data->currentBlock = nullptr; // don't chain to an existing block
            } else {
                std::shared_ptr<BtCodeChunk> chunk = memory->getCodeBlock(address);
                if (chunk) {
                    data->currentBlock = chunk->block;
                    if (!data->currentBlock) {
                        int ii = 0;
                    }
                }
            }
            if (!data->currentBlock) {
                data->currentBlock = NormalCPU::getBlockForInspectionButNotUsed(this, address, big);
                if (prev) {
                    prev->bytes += data->currentBlock->bytes;
                    prevOp->next = data->currentBlock->op;
                    data->currentBlock->op = nullptr;
                    delete data->currentBlock;
                    data->currentBlock = prev;
                }
            }
            data->currentOp = data->currentBlock->getOp(address);
            if (!data->currentOp) {
                int ii = 0;
            }
        }
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
#ifdef BOXEDWINE_64BIT_MMU
        if (firstPass) {
            U32 nextEipLen = firstPass->calculateEipLen(data->ip+this->seg[CS].address);
            U32 page = (data->ip+this->seg[CS].address+nextEipLen) >> K_PAGE_SHIFT;

            if (page!=codePage) {
                codePage = page;
                nativePage = mem->getNativePage(codePage);
                if (data->dynamic) {                    
                    if (mem->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // continue to cross from my dynamic page into another dynamic page
                    } else {
                        // we will continue to emit code that will self check for modified code, even though the page we spill into is not dynamic
                    }
                } else {
                    if (mem->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // we crossed a page boundry from a non dynamic page to a dynamic page
                        data->dynamic = true; // the instructions from this point on will do their own check
                    } else {
                        // continue to cross from one non dynamic page into another non dynamic page
                    }
                }
            }
        }
#endif
        data->mapAddress(address, data->bufferPos);
        data->translateInstruction();
        if (data->done || data->currentOp->inst == Invalid) {
            break;
        }
        if (data->stopAfterInstruction!=-1 && (int)data->ipAddressCount==data->stopAfterInstruction) {
            break;
        }
        data->resetForNewOp();
        prevOp = data->currentOp;
        data->currentOp = data->currentOp->next;
    }  
}

#ifdef BOXEDWINE_64BIT_MMU
// for string instruction, we modify (add memory offset and segment) rdi and rsi so that the native string instruction can be used, this code will revert it back to the original values
bool x64CPU::handleStringOp(DecodedOp* op) {
    if (op->inst == Lodsb || op->inst == Lodsw || op->inst == Lodsd) {
        THIS_ESI = (U32)(this->exceptionRSI - this->memOffset);
        if (this->thread->process->hasSetSeg[op->base]) {
            THIS_ESI -= this->seg[op->base].address;
        }
        return true;
    }
    // uses di (Examples: diablo 1 will trigger this in the middle of the Stosd when creating a new game)
    else if (op->inst == Stosb || op->inst == Stosw || op->inst == Stosd ||
        op->inst == Scasb || op->inst == Scasw || op->inst == Scasd) {
        THIS_EDI = (U32)(this->exceptionRDI - this->memOffset);
        if (this->thread->process->hasSetSeg[ES]) {
            THIS_EDI -= this->seg[ES].address;
        }
        return true;
    }
    // uses si and di
    else if (op->inst == Movsb || op->inst == Movsw || op->inst == Movsd ||
        op->inst == Cmpsb || op->inst == Cmpsw || op->inst == Cmpsd) {
        THIS_ESI = (U32)(this->exceptionRSI - this->memOffset);
        THIS_EDI = (U32)(this->exceptionRDI - this->memOffset);
        if (this->thread->process->hasSetSeg[ES]) {
            THIS_EDI -= this->seg[ES].address;
        }
        if (this->thread->process->hasSetSeg[op->base]) {
            THIS_ESI -= this->seg[op->base].address;
        }
        return true;
    }
    return false;
}
#endif

#ifndef BOXEDWINE_64BIT_MMU
void common_runSingleOp(x64CPU* cpu) {
    U32 address = cpu->eip.u32;

    if (cpu->isBig()) {
        address += cpu->seg[CS].address;
    } else {
        address = cpu->seg[CS].address + (address & 0xFFFF);
    }
    CodeBlock block = cpu->memory->findCodeBlockContaining(address, 0);
    if (!block) {
        int ii = 0;
    }
    DecodedBlock::currentBlock = block->block;
    if (!DecodedBlock::currentBlock) {
        void* host = getMemData(cpu->memory)->getExistingHostAddress(address);
        DecodedBlock::currentBlock = NormalCPU::getBlockForInspectionButNotUsed(cpu, address, cpu->isBig());
        //cpu->memory->addExtraCodeBlock(address, DecodedBlock::currentBlock);
    }
    DecodedOp* op = DecodedBlock::currentBlock->getOp(address);
    bool deallocBlock = false;
    if (!op) {
        // interesting rare case where the code skipped over the 1 byte lock prefix and the address wasn't ready (on demand, mapped file)
        DecodedBlock::currentBlock = NormalCPU::getBlockForInspectionButNotUsed(cpu, address, cpu->isBig());
        op = DecodedBlock::currentBlock->getOp(address);
        deallocBlock = true;
    }

    for (U32 i = 0; i < 8; i++) {
        cpu->xmm[i].pd.u64[0] = cpu->fpuState.xmm[i].low;
        cpu->xmm[i].pd.u64[1] = cpu->fpuState.xmm[i].high;
    }

    U8 tag = ((x64CPU*)cpu)->fpuState.ftw;
    bool mmx = false;

    U16 controlWord = cpu->fpuState.fcw;
    U16 statusWord = cpu->fpuState.fsw;
    cpu->fpu.SetCW(controlWord);
    cpu->fpu.SetSW(statusWord);
    cpu->fpu.SetTagFromAbridged(tag);

    if (tag) {        
        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            if (!(tag & (1 << i))) {
                //cpu->fpu.setReg(i, 0.0);
            } else {
                if ((cpu->fpuState.st_mm[index].high & 0xFFFFFF00) == 0xFFFFFF00) {
                    mmx = true;
                    break;
                }
            }
        }

        if (mmx) {
            for (U32 i = 0; i < 8; i++) {
                cpu->reg_mmx[i].q = cpu->fpuState.st_mm[i].low;
            }
        } else {
            cpu->resetMMX();
            if (!cpu->thread->process->emulateFPU) {
                for (U32 i = 0; i < 8; i++) {
                    U32 index = (i - cpu->fpu.GetTop()) & 7;
                    if (!(tag & (1 << i))) {
                        //cpu->fpu.setReg(i, 0.0);
                    } else {
                        double d = cpu->fpu.FLD80(cpu->fpuState.st_mm[index].low, (S16)cpu->fpuState.st_mm[index].high);
                        cpu->fpu.setReg(i, d);
                    }
                }
            }
        }
    }
    bool inSignal = cpu->thread->inSignal;

    op->pfn(cpu, op);

    if (inSignal != (cpu->thread->inSignal!=0)) {
        // :TODO: move this threads context read/write
        if (inSignal) {
            memcpy(&cpu->fpuState, &cpu->originalFpuState, sizeof(cpu->fpuState));
        } else {
            memcpy(&cpu->originalFpuState, &cpu->fpuState, sizeof(cpu->fpuState));
        }
    }
    cpu->fpuState.fcw = cpu->fpu.CW();
    cpu->fpuState.fsw = cpu->fpu.SW();
    cpu->fpuState.ftw = cpu->fpu.GetAbridgedTag();
    if (cpu->fpuState.ftw) {
        if (cpu->isMMXinUse()) {
            for (U32 i = 0; i < 8; i++) {
                cpu->fpuState.st_mm[i].low = cpu->reg_mmx[i].q;
            }
        } else if (!cpu->thread->process->emulateFPU) {
            U8 tag = cpu->fpuState.ftw;
            for (U32 i = 0; i < 8; i++) {
                U32 index = (i - cpu->fpu.GetTop()) & 7;
                if (!(tag & (1 << i))) {
                    //memset(&((x64CPU*)cpu)->fpuState.st_mm[i].st[0], 0, 10);
                } else {
                    cpu->fpu.ST80(i, (U64*)&cpu->fpuState.st_mm[index].low, (U64*)&cpu->fpuState.st_mm[index].high);
                }
            }
        }
    }

    for (U32 i = 0; i < 8; i++) {
        cpu->fpuState.xmm[i].low = cpu->xmm[i].pd.u64[0];
        cpu->fpuState.xmm[i].high = cpu->xmm[i].pd.u64[1];
    }

    cpu->fillFlags();
    if (deallocBlock) {
        DecodedBlock::currentBlock->dealloc(false);
    }
    DecodedBlock::currentBlock = nullptr;
}
#endif
#endif
