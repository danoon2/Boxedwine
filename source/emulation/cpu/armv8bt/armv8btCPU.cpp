#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCPU.h"
#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "armv8btCodeChunk.h"

#undef u8

CPU* CPU::allocCPU(KMemory* memory) {
    return new Armv8btCPU(memory);
}

Armv8btCPU::Armv8btCPU(KMemory* memory) : BtCPU(memory), data1(this), data2(this) {
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE].pd.f64[0] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE].pd.f64[1] = 2147483648.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE].pd.f64[0] = -2147483649.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE].pd.f64[1] = -2147483649.0;

    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[0] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[1] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[2] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[3] = 2147483648.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[0] = -2147483649.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[1] = -2147483649.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[2] = -2147483649.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[3] = -2147483649.0;

    sseConstants[SSE_INT32_BIT_MASK].ps.u32[0] = 1;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[1] = 2;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[2] = 4;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[3] = 8;

    sseConstants[SSE_BYTE8_BIT_MASK].pi.u8[0] = 1;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[1] = 2;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[2] = 4;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[3] = 8;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[4] = 16;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[5] = 32;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[6] = 64;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[7] = 128;
    sseConstants[SSE_BYTE8_BIT_MASK].pi.u8[8] = 1;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[9] = 2;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[10] = 4;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[11] = 8;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[12] = 16;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[13] = 32;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[14] = 64;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[15] = 128;

    largeAddressJumpInstruction = 0xf8400149;
    pageJumpInstruction = 0xf86f7929;
    pageOffsetJumpInstruction = 0x38400131;
}

void Armv8btCPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
}

void Armv8btCPU::restart() {
	this->exitToStartThreadLoop = true;
}

void* Armv8btCPU::init() {
    Armv8btAsm data(this);
    void* result;
    KMemoryData* mem = getMemData(memory);
    Armv8btCPU* cpu = this;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
	data.saveNativeState();

    data.writeToRegFromValue(xCPU, (U64)this);
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
    
#ifdef xMemRead
    KMemoryData* memData = getMemData(memory);
    data.writeToRegFromValue(xMemRead, (U64)memData->mmuReadPtrAdjusted);
    data.writeToRegFromValue(xMemWrite, (U64)memData->mmuWritePtrAdjusted);
#endif

    data.calculatedEipLen = 1; // will force the long x64 chunk jump
    data.doJmp(false);
    std::shared_ptr<BtCodeChunk> chunk = data.commit(true);
    result = chunk->getHostAddress();
    //link(&data, chunk);
    this->pendingCodePages.clear();    
    this->eipToHostInstructionPages = mem->eipToHostInstructionPages;

    if (!this->thread->process->returnToLoopAddress) {
        Armv8btAsm returnData(this);
        returnData.restoreNativeState();
        returnData.addReturn();
        std::shared_ptr<BtCodeChunk> chunk2 = returnData.commit(true);
        this->thread->process->returnToLoopAddress = chunk2->getHostAddress();
    }
    this->returnToLoopAddress = this->thread->process->returnToLoopAddress;
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        Armv8btAsm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessary = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
    if (!this->thread->process->syncToHostAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForSyncToHost();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->syncToHostAddress = chunk3->getHostAddress();
    }
    this->syncToHostAddress = this->thread->process->syncToHostAddress;
    if (!this->thread->process->syncFromHostAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForSyncFromHost();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->syncFromHostAddress = chunk3->getHostAddress();
    }
    this->syncFromHostAddress = this->thread->process->syncFromHostAddress;
    if (!this->thread->process->doSingleOpAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForDoSingleOp();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->doSingleOpAddress = chunk3->getHostAddress();
    }
    this->doSingleOpAddress = this->thread->process->doSingleOpAddress;
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

#ifdef __TEST
void Armv8btCPU::addReturnFromTest() {
    Armv8btAsm data(this);
    data.addReturnFromTest();
    data.commit(true);
}
#endif

void Armv8btCPU::writeJumpAmount(BtData* data, U32 pos, U32 toLocation, U8* offset) {
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

void Armv8btCPU::link(BtData* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk) {
    if (!fromChunk) {
        kpanic("Armv8btCPU::link fromChunk missing");
    }
    for (U32 i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;
        U8* offset = (U8*)fromChunk->getHostAddress()+offsetIntoChunk+data->todoJump[i].bufferPos;
        U8* host = (U8*)fromChunk->getHostFromEip(eip);

        if (!host) {
            kpanic("Armv8btCPU::link can not link into the middle of an instruction");
        }
#ifdef BOXEDWINE_MAC_JIT
        if (__builtin_available(macOS 11.0, *)) {
            pthread_jit_write_protect_np(false);
        }
#endif
        writeJumpAmount(data, data->todoJump[i].bufferPos, (U32)(host - offset), (U8*)fromChunk->getHostAddress() + offsetIntoChunk);
#ifdef BOXEDWINE_MAC_JIT
        if (__builtin_available(macOS 11.0, *)) {
            pthread_jit_write_protect_np(true);
        }
#endif
    }
}

void Armv8btCPU::translateData(BtData* data, BtData* firstPass) {
    KMemoryData* mem = getMemData(memory);
    data->currentOp = nullptr;
    data->currentBlock = nullptr;
    DecodedOp* prevOp = nullptr;
    while (1) {
        U32 address = this->seg[CS].address+data->ip;
        void* hostAddress = mem->getExistingHostAddress(address);
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
        if (!data->currentOp) {            
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
        data->mapAddress(address, data->bufferPos);
        // add a mapping so that if they skip the 1 byte lock they will get mapped to the lock version anyway, libc seems to want to skip the lock sometimes in order to improve performance by a tiny bit
        if (data->currentOp->lock) {
            data->mapAddress(address+1, data->bufferPos);
        }
        if (firstPass) {
            // :TODO: find a way without a cast
            Armv8btAsm* a = (Armv8btAsm*)(firstPass);
            if (a->fpuTopRegSet && !a->fpuOffsetRegSet) {
                a->getFpuTopReg();
            }
        }
        data->translateInstruction();
        if (data->done) {
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

#endif
