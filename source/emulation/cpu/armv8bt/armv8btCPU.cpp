#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCPU.h"
#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../../hardmmu/kmemory_hard.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "armv8btCodeChunk.h"
#include "../binaryTranslation/btCodeMemoryWrite.h"

#undef u8

CPU* CPU::allocCPU(KMemory* memory) {
    return new Armv8btCPU(memory);
}

Armv8btCPU::Armv8btCPU(KMemory* memory) : BtCPU(memory) {
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

std::shared_ptr<BtData> Armv8btCPU::createData() {
    return std::make_shared<Armv8btAsm>(this);
}

void Armv8btCPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
}

void Armv8btCPU::restart() {
#ifdef BOXEDWINE_64BIT_MMU
    KMemoryData* mem = getMemData(memory);
	this->memOffset = mem->id;
#endif
	this->exitToStartThreadLoop = true;
}

void* Armv8btCPU::init() {
    Armv8btAsm data(this);
    void* result;
    KMemoryData* mem = getMemData(memory);
    Armv8btCPU* cpu = this;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
#ifdef BOXEDWINE_64BIT_MMU
    this->eipToHostInstructionAddressSpaceMapping = mem->eipToHostInstructionAddressSpaceMapping;
    this->memOffsets = mem->memOffsets;
#endif
	data.saveNativeState();

    data.writeToRegFromValue(xCPU, (U64)this);
#ifdef BOXEDWINE_64BIT_MMU
    data.writeToRegFromValue(xMem, cpu->memOffset);

    if (KSystem::useLargeAddressSpace) {
        data.writeToRegFromValue(xLargeAddress, (U64)cpu->eipToHostInstructionAddressSpaceMapping);
    }
#endif
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
#ifdef BOXEDWINE_64BIT_MMU
    if (!this->thread->process->reTranslateChunkAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddress = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddress = this->thread->process->reTranslateChunkAddress;
    if (!this->thread->process->reTranslateChunkAddressFromReg) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddressFromReg = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddressFromReg = this->thread->process->reTranslateChunkAddressFromReg;
#endif
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        Armv8btAsm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessary = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
#endif
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

void Armv8btCPU::writeJumpAmount(const std::shared_ptr<BtData>& data, U32 pos, U32 toLocation, U8* offset) {
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

void Armv8btCPU::link(const std::shared_ptr<BtData>& data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk) {
    U32 i;
    KMemoryData* mem = getMemData(memory);

    if (!fromChunk) {
        kpanic("Armv8btCPU::link fromChunk missing");
    }
    for (i=0;i<data->todoJump.size();i++) {
        U32 eip = this->seg[CS].address+data->todoJump[i].eip;        
        U8* offset = (U8*)fromChunk->getHostAddress()+offsetIntoChunk+data->todoJump[i].bufferPos;
        U8 size = data->todoJump[i].offsetSize;
        U8* host = NULL;

        if (size==4 && (host = (U8*)fromChunk->getHostFromEip(eip))) {
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
        } else if (size==4) {
            U8* toHostAddress = (U8*)mem->getExistingHostAddress(eip);

            if (!toHostAddress) {
                U8 op = 0xce;
                U32 hostIndex = 0;
                std::shared_ptr<BtCodeChunk> chunk = std::make_shared<Armv8CodeChunk>(1, &eip, &hostIndex, &op, 1, eip-this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();            
            }
            std::shared_ptr<BtCodeChunk> toChunk = mem->getCodeChunkContainingHostAddress(toHostAddress);
            if (!toChunk) {
                kpanic("Armv8btCPU::link to chunk missing");
            }
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, false);
            writeJumpAmount(data, data->todoJump[i].bufferPos, (U32)(toHostAddress - offset), (U8*)fromChunk->getHostAddress() + offsetIntoChunk);
        } else if (size==8 && !data->todoJump[i].sameChunk) {
            U8* toHostAddress = (U8*)mem->getExistingHostAddress(eip);

            if (!toHostAddress) {
                Armv8btAsm returnData(this);
                returnData.startOfOpIp = eip - this->seg[CS].address;
                returnData.callRetranslateChunk();
                U32 hostIndex = 0;
                std::shared_ptr<BtCodeChunk> chunk = std::make_shared<Armv8CodeChunk>(1, &eip, &hostIndex, returnData.buffer, returnData.bufferPos, eip - this->seg[CS].address, 1, false);
                chunk->makeLive();
                toHostAddress = (U8*)chunk->getHostAddress();
            }
            std::shared_ptr<BtCodeChunk> toChunk = mem->getCodeChunkContainingHostAddress(toHostAddress);
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
#ifdef BOXEDWINE_64BIT_MMU
    markCodePageReadOnly(data.get());
#endif
}

void Armv8btCPU::translateData(const std::shared_ptr<BtData>& data, const std::shared_ptr<BtData>& firstPass) {
    KMemoryData* mem = getMemData(memory);
#ifdef BOXEDWINE_64BIT_MMU    
    U32 codePage = (data->ip+ this->seg[CS].address) >> K_PAGE_SHIFT;
    U32 nativePage = mem->getNativePage(codePage);
    if (mem->dynamicCodePageUpdateCount[nativePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
#endif
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
#ifdef BOXEDWINE_64BIT_MMU
        if (firstPass) {
            U32 nextEipLen = firstPass->calculateEipLen(data->ip + this->seg[CS].address);
            U32 page = (data->ip+ this->seg[CS].address+nextEipLen) >> K_PAGE_SHIFT;

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
        if (firstPass) {
            // :TODO: find a way without a cast
            std::shared_ptr<Armv8btAsm> a = std::dynamic_pointer_cast<Armv8btAsm>(firstPass);
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
#ifdef BOXEDWINE_64BIT_MMU
    data->currentBlock->dealloc(false);
    data->currentBlock = NULL;
#endif    
}

#endif
