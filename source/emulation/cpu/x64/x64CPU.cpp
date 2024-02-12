#include "boxedwine.h"

#ifdef BOXEDWINE_X64
#include "x64Ops.h"
#include "x64CPU.h"
#include "x64Asm.h"
#include "x64CodeChunk.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"

CPU* CPU::allocCPU(KMemory* memory) {
    return new x64CPU(memory);
}

// hard to guage the benifit, seems like 1% to 3% with quake 2 and quake 3
bool x64CPU::hasBMI2 = true;
bool x64Intialized = false;

x64CPU::x64CPU(KMemory* memory) : BtCPU(memory), data1(this), data2(this) {
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
	for (int i = 0; i < 6; i++) {
		this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
	}
	this->exitToStartThreadLoop = true;
}

void* x64CPU::init() {
    X64Asm data(this);
    KMemoryData* mem = getMemData(memory);
    x64CPU* cpu = this;

    for (int i = 0; i < 6; i++) {
        this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
    }

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);

	// will push 15 regs, since it is odd, it will balance rip being pushed on the stack and give use a 16-byte alignment
	data.saveNativeState(); // also sets HOST_CPU

    //data.writeToRegFromValue(HOST_CPU, true, (U64)this, 8);
    KMemoryData* memData = getMemData(memory);
    data.writeToRegFromValue(HOST_MEM_READ, true, (U64)memData->mmuReadPtrAdjusted, 8);
    data.writeToRegFromValue(HOST_MEM_WRITE, true, (U64)memData->mmuWritePtrAdjusted, 8);
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
    void* result = chunk->getHostAddress();
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
    if (!this->thread->process->doSingleOpAddress) {
        X64Asm translateData(this);
        translateData.createCodeForDoSingleOp();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->doSingleOpAddress = chunk3->getHostAddress();
    }
    this->doSingleOpAddress = this->thread->process->doSingleOpAddress;
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        X64Asm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary(true);
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessary = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
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

void x64CPU::link(BtData* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk) {
    if (!fromChunk) {
        kpanic("x64CPU::link fromChunk missing");
    }
    for (auto& todoJump : data->todoJump) {
        U32 eip = seg[CS].address + todoJump.eip;
        U8* offset = (U8*)fromChunk->getHostAddress()+offsetIntoChunk+todoJump.bufferPos;

        U8* host = (U8*)fromChunk->getHostFromEip(eip);
        if (!host) {
            kpanic("x64CPU::link can not link into the middle of an instruction");
        }
        data->write32Buffer(offset, (U32)(host - offset - 4));            
    }
}

void x64CPU::translateData(BtData* data, BtData* firstPass) {
    KMemoryData* mem = getMemData(memory);
  
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
                std::shared_ptr<BtCodeChunk> chunk = memory->findCodeBlockContaining(address, 1);
                if (chunk) {
                    data->currentBlock = chunk->block;
                    if (data->currentBlock) {
                        data->currentOp = data->currentBlock->getOp(address);
                        if (!data->currentOp) {
                            // winfish seems to jump into the middle of an instruction which changes it from cmp to mov
                            data->currentBlock = nullptr;
                        }
                    } else {
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
                    data->currentBlock->dealloc(false);
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

void common_runSingleOp(x64CPU* cpu) {
    U32 address = cpu->eip.u32;

    if (cpu->isBig()) {
        address += cpu->seg[CS].address;
    } else {
        address = cpu->seg[CS].address + (address & 0xFFFF);
    }
    DecodedOp* op = cpu->currentSingleOp;
    bool deallocOp = false;
    bool dynamic = cpu->arg5 != 0;
    if (dynamic) {
        op = NormalCPU::decodeSingleOp(cpu, address);
        deallocOp = true;
    } else if (!op) {
        kpanic("common_runSingleOp oops");
    }
    if (op->inst >= Fxsave && op->inst <= ShufpdXmmE128) {
        for (U32 i = 0; i < 8; i++) {
            cpu->xmm[i].pd.u64[0] = cpu->fpuState.xmm[i].low;
            cpu->xmm[i].pd.u64[1] = cpu->fpuState.xmm[i].high;
        }
    }
    if (!cpu->thread->process->emulateFPU && op->inst >= FADD_ST0_STj && op->inst <= FISTP_QWORD_INTEGER) {
        U16 controlWord = cpu->fpuState.fcw;
        U16 statusWord = cpu->fpuState.fsw;
        U8 tag = ((x64CPU*)cpu)->fpuState.ftw;
        cpu->fpu.SetCW(controlWord);
        cpu->fpu.SetSW(statusWord);
        cpu->fpu.SetTagFromAbridged(tag);

        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            if (!(tag & (1 << i))) {
                //cpu->fpu.setReg(i, 0.0);
            } else {
                double d = cpu->fpu.FLD80(cpu->fpuState.st_mm[index].low, (S16)cpu->fpuState.st_mm[index].high);
                cpu->fpu.setReg(i, d);
            }
        }
    } else {
        for (U32 i = 0; i < 8; i++) {
            cpu->reg_mmx[i].q = cpu->fpuState.st_mm[i].low;
        }
    }
    bool inSignal = cpu->thread->inSignal;
    if (op->inst >= Movsb && op->inst <= Scasd) {
        cpu->flags &= ~(SF | AF | ZF | PF | CF | OF);
        cpu->flags |= ((cpu->stringFlags >> 8) & 0xff) | ((cpu->stringFlags & 1) << 11);    
    }
    try {
        if (!op->lock) {
            op->pfn(cpu, op);
        } else {
            BOXEDWINE_CRITICAL_SECTION;
            op->pfn(cpu, op);
        }
    } catch (...) {
        int ii = 0;
    }
    if (inSignal != (cpu->thread->inSignal!=0)) {
        // :TODO: move this threads context read/write
        if (inSignal) {
            memcpy(&cpu->fpuState, &cpu->originalFpuState, sizeof(cpu->fpuState));
        } else {
            memcpy(&cpu->originalFpuState, &cpu->fpuState, sizeof(cpu->fpuState));
        }
    }
    if (!cpu->thread->process->emulateFPU && op->inst >= FADD_ST0_STj && op->inst <= FISTP_QWORD_INTEGER) {
        cpu->fpuState.fcw = cpu->fpu.CW();
        cpu->fpuState.fsw = cpu->fpu.SW();
        cpu->fpuState.ftw = cpu->fpu.GetAbridgedTag();
        U8 tag = cpu->fpuState.ftw;
        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            if (!(tag & (1 << i))) {
                //memset(&((x64CPU*)cpu)->fpuState.st_mm[i].st[0], 0, 10);
            } else {
                cpu->fpu.ST80(i, (U64*)&cpu->fpuState.st_mm[index].low, (U64*)&cpu->fpuState.st_mm[index].high);
            }
        }
    } else {
        for (U32 i = 0; i < 8; i++) {
            cpu->fpuState.st_mm[i].low = cpu->reg_mmx[i].q;
        }
    }
    if (op->inst >= Fxsave && op->inst <= ShufpdXmmE128) {
        for (U32 i = 0; i < 8; i++) {
            cpu->fpuState.xmm[i].low = cpu->xmm[i].pd.u64[0];
            cpu->fpuState.xmm[i].high = cpu->xmm[i].pd.u64[1];
        }
    }
    cpu->fillFlags();
    if (deallocOp) {
        op->dealloc(true);
    }
    DecodedBlock::currentBlock = nullptr;
}
#endif
