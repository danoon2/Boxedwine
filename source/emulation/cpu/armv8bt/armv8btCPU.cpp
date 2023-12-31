#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCPU.h"
#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../../hardmmu/hard_memory.h"
#include "../normal/normalCPU.h"
#include "../binaryTranslation/btCodeChunk.h"
#include "armv8btCodeChunk.h"
#include "../binaryTranslation/btCodeMemoryWrite.h"

#undef u8

CPU* CPU::allocCPU() {
    return new Armv8btCPU();
}

Armv8btCPU::Armv8btCPU() {
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
	this->memOffset = this->thread->process->memory->id;
	this->exitToStartThreadLoop = true;
}

void* Armv8btCPU::init() {
    Armv8btAsm data(this);
    void* result;
    Memory* memory = this->thread->memory;
    Armv8btCPU* cpu = this;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->executableMemoryMutex);
    this->eipToHostInstructionAddressSpaceMapping = this->thread->memory->eipToHostInstructionAddressSpaceMapping;
    this->memOffsets = memory->memOffsets;

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
    //link(&data, chunk);
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
    if (!this->thread->process->reTranslateChunkAddressFromReg) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRetranslateChunk();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->reTranslateChunkAddressFromReg = chunk3->getHostAddress();
    }
    this->reTranslateChunkAddressFromReg = this->thread->process->reTranslateChunkAddressFromReg;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        Armv8btAsm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary();
        std::shared_ptr<BtCodeChunk> chunk3 = translateData.commit(true);
        this->thread->process->jmpAndTranslateIfNecessary = chunk3->getHostAddress();
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
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
            std::shared_ptr<BtCodeChunkLink> link = toChunk->addLinkFrom(fromChunk, eip, toHostAddress, offset, false);
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
    markCodePageReadOnly(data.get());
}

static U8 fetchByte(U32* eip) {
    return readb((*eip)++);
}

void Armv8btCPU::translateData(const std::shared_ptr<BtData>& data, const std::shared_ptr<BtData>& firstPass) {
    Memory* memory = this->thread->memory;

    U32 codePage = (data->ip+ this->seg[CS].address) >> K_PAGE_SHIFT;
    U32 nativePage = this->thread->memory->getNativePage(codePage);
    if (memory->dynamicCodePageUpdateCount[nativePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
    DecodedBlock block;
    data->currentBlock = &block;
    decodeBlock(fetchByte, data->startOfDataIp + this->seg[CS].address, this->isBig(), 0, 0, 0, &block);
    DecodedOp* op = block.op;
    while (op) {  
        U32 address = this->seg[CS].address+data->ip;
        void* hostAddress = memory->getExistingHostAddress(address);
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
        if (firstPass) {
            U32 nextEipLen = firstPass->calculateEipLen(data->ip + this->seg[CS].address);
            U32 page = (data->ip+ this->seg[CS].address+nextEipLen) >> K_PAGE_SHIFT;

            if (page!=codePage) {
                codePage = page;
                nativePage = this->thread->memory->getNativePage(codePage);
                if (data->dynamic) {                    
                    if (memory->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // continue to cross from my dynamic page into another dynamic page
                    } else {
                        // we will continue to emit code that will self check for modified code, even though the page we spill into is not dynamic
                    }
                } else {
                    if (memory->dynamicCodePageUpdateCount[nativePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // we crossed a page boundry from a non dynamic page to a dynamic page
                        data->dynamic = true; // the instructions from this point on will do their own check
                    } else {
                        // continue to cross from one non dynamic page into another non dynamic page
                    }
                }
            }
        }
        data->mapAddress(address, data->bufferPos);        
        if (firstPass) {
            // :TODO: find a way without a cast
            std::shared_ptr<Armv8btAsm> a = std::dynamic_pointer_cast<Armv8btAsm>(firstPass);
            if (a->fpuTopRegSet && !a->fpuOffsetRegSet) {
                a->getFpuTopReg();
            }
        }
        data->decodedOp = op;
        data->translateInstruction();
        op = op->next;
        if (data->done) {
            break;
        }
        if (data->stopAfterInstruction!=-1 && (int)data->ipAddressCount==data->stopAfterInstruction) {
            break;
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

#endif
