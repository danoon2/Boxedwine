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

Armv8btCPU::Armv8btCPU() : regPage(0), regOffset(0) {
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

void* Armv8btCPU::translateEipInternal(U32 ip) {
    return translateEipInternal(NULL, ip);
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
    markCodePageReadOnly(data);
}

static U8 fetchByte(U32* eip) {
    return readb((*eip)++);
}

void Armv8btCPU::translateInstruction(Armv8btAsm* data, Armv8btAsm* firstPass) {
    data->startOfOpIp = data->ip;
    data->useSingleMemOffset = !data->cpu->thread->memory->doesInstructionNeedMemoryOffset(data->ip);
    data->ip += data->decodedOp->len;
#ifdef _DEBUG
    if (this->logFile) {
        data->logOp(data->startOfOpIp);
    }
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
    if (memory->dynamicCodePageUpdateCount[codePage]==MAX_DYNAMIC_CODE_PAGE_COUNT) {
        data->dynamic = true;
    }
    DecodedBlock block;
    data->currentBlock = &block;
    decodeBlock(fetchByte, data->startOfDataIp + this->seg[CS].address, this->isBig(), 0, 0, 0, &block);
    DecodedOp* op = block.op;
    while (op) {  
        U32 address = data->cpu->seg[CS].address+data->ip;
        void* hostAddress = memory->getExistingHostAddress(address);
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
                    if (memory->dynamicCodePageUpdateCount[codePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
                        // continue to cross from my dynamic page into another dynamic page
                    } else {
                        // we will continue to emit code that will self check for modified code, even though the page we spill into is not dynamic
                    }
                } else {
                    if (memory->dynamicCodePageUpdateCount[codePage] == MAX_DYNAMIC_CODE_PAGE_COUNT) {
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

U64 Armv8btCPU::getIpFromEip() {
    U32 a = this->getEipAddress();
    U64 result = (U64)this->thread->memory->getExistingHostAddress(a);
    if (!result) {
        this->translateEip(this->eip.u32);
        result = (U64)this->thread->memory->getExistingHostAddress(a);
    }
    if (result == 0) {
        kpanic("Armv8btCPU::getIpFromEip failed to translate code");
    }
    return result;
}

bool Armv8btCPU::isStringOp(DecodedOp* op) {
    if (op->inst == Lodsb || op->inst == Lodsw || op->inst == Lodsd) {
        return true;
    }
    // uses di (Examples: diablo 1 will trigger this in the middle of the Stosd when creating a new game)
    else if (op->inst == Stosb || op->inst == Stosw || op->inst == Stosd ||
        op->inst == Scasb || op->inst == Scasw || op->inst == Scasd) {
        return true;
    }
    // uses si and di
    else if (op->inst == Movsb || op->inst == Movsw || op->inst == Movsd ||
        op->inst == Cmpsb || op->inst == Cmpsw || op->inst == Cmpsd) {

        return true;
    }
    return false;
}

U64 Armv8btCPU::handleCodePatch(U64 rip, U32 address) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
#endif
    Memory* memory = this->thread->memory;
    U32 nativePage = memory->getNativePage(address >> K_PAGE_SHIFT);
    // get the emulated eip of the op that corresponds to the host address where the exception happened
    std::shared_ptr<BtCodeChunk> chunk = memory->getCodeChunkContainingHostAddress((void*)rip);
    this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)rip, NULL, NULL)-this->seg[CS].address;

    // make sure it wasn't changed before we got the executableMemoryMutex lock
    if (!(memory->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        return getIpFromEip();
    }
    
    // get the emulated op that caused the write
    DecodedOp* op = this->getOp(this->eip.u32, true);
    if (op) {             
        if (this->flags & DF) {
            this->df = -1;
        } else {
            this->df = 1;
        }
        U32 addressStart = address;
        U32 len = instructionInfo[op->inst].writeMemWidth / 8;

        // Fix string will set EDI and ESI back to their correct values so we can re-enter this instruction
        if (isStringOp(op)) {
            if (op->repNotZero || op->repZero) {
                len = len * (isBig() ? THIS_ECX : THIS_CX);
            }

            if (this->df == 1) {
                addressStart = (this->isBig() ? THIS_EDI : THIS_DI) + this->seg[ES].address;
            } else {
                addressStart = (this->isBig() ? THIS_EDI : THIS_DI) + this->seg[ES].address + (instructionInfo[op->inst].writeMemWidth / 8) - len;
            }
        }
        U32 startPage = addressStart >> K_PAGE_SHIFT;
        U32 endPage = (addressStart + len - 1) >> K_PAGE_SHIFT;
        memory->clearHostCodeForWriting(memory->getNativePage(startPage), memory->getNativePage(endPage - startPage) + 1);
        op->dealloc(true);
        return getIpFromEip();
    } else {                        
        kpanic("Threw an exception from a host location that doesn't map to an emulated instruction");
    }
    return 0;
}

U64 Armv8btCPU::handleMissingCode(U64 regPage, U64 regOffset, U32 inst) {
    U32 page = (U32)regPage;
    U32 offset = (U32)regOffset;

    this->eip.u32 = ((page << K_PAGE_SHIFT) | offset) - this->seg[CS].address;
    return (U64)this->translateEip(this->eip.u32);  
}

U32 dynamicCodeExceptionCount;

#define JMP_PAGE_EXCEPTION 0xf86f7929
#define JMP_OFFSET_EXCEPTION 0x38400131

U64 Armv8btCPU::handleAccessException(U64 ip, U64 address, bool readAddress) {
    if ((address & 0xFFFFFFFF00000000l) == this->thread->memory->id) {
        U32 emulatedAddress = (U32)address;
        U32 page = emulatedAddress >> K_PAGE_SHIFT;
        Memory* m = this->thread->memory;
        U32 flags = m->flags[page];
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);

        std::shared_ptr<BtCodeChunk> chunk = m->getCodeChunkContainingHostAddress((void*)ip);
        if (chunk) {
            this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)ip, NULL, NULL) - this->seg[CS].address;
            // if the page we are trying to access needs a special memory offset and this instruction isn't flagged to looked at that special memory offset, then flag it
            if ((flags & PAGE_MAPPED_HOST) && (((flags & PAGE_READ) && readAddress) || ((flags & PAGE_WRITE) && !readAddress))) {
                m->setNeedsMemoryOffset(getEipAddress());
                chunk->releaseAndRetranslate();
                return getIpFromEip();
            } else {
                U32 nativeFlags = this->thread->memory->nativeFlags[this->thread->memory->getNativePage(page)];
                if (!readAddress && (flags & PAGE_WRITE) && !(nativeFlags & NATIVE_FLAG_CODEPAGE_READONLY)) {
                    // :TODO: this is a hack, why is it necessary
                    m->updatePagePermission(page, 1);
                    return 0;
                }
            }
        }
    }
    
    U32 inst = *((U32*)ip);

    if (inst == 0xf8400149 && thread->memory->isValidReadAddress(this->destEip, 1)) { // ldur x9, [x9]
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
            U32 page = emulatedAddress >> K_PAGE_SHIFT;
            Memory* m = this->thread->memory;
            U32 nativeFlags = m->nativeFlags[m->getNativePage(page)];
            U32 flags = m->flags[page];
            // check if emulated memory that caused the exception is a page that has code
            if ((flags & PAGE_MAPPED_HOST) || (flags & (PAGE_READ | PAGE_WRITE)) != (nativeFlags & (PAGE_READ | PAGE_WRITE))) {
                dynamicCodeExceptionCount++;                    
                return this->handleCodePatch(ip, emulatedAddress);                    
            }
        }   
#ifdef _DEBUG
        //void* fromHost = this->thread->memory->getExistingHostAddress(this->fromEip);
        //std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress((void*)rip);
#endif
        
        U32 emulatedAddress = (U32)address;
        U32 page = emulatedAddress >> K_PAGE_SHIFT;        
        Memory* m = this->thread->memory;
        U32 nativeFlags = m->nativeFlags[m->getNativePage(page)];
        U32 flags = m->flags[page];
        if ((address & 0xFFFFFFFF00000000l) == this->thread->memory->id) {
            // check if emulated memory that caused the exception is a page that has code
            if ((flags & PAGE_MAPPED_HOST) || (flags & (PAGE_READ | PAGE_WRITE)) != (nativeFlags & (PAGE_READ | PAGE_WRITE))) {
                dynamicCodeExceptionCount++;
                return this->handleCodePatch(ip, emulatedAddress);
            }
        }
        // this can be exercised with Wine 5.0 and CC95 demo installer, it is triggered in strlen as it tries to grow the stack
        this->thread->seg_mapper((U32)address, readAddress, !readAddress, false);
        U64 result = (U64)this->translateEip(this->eip.u32); 
        if (result==0) {
            kpanic("Armv8btCPU::handleAccessException failed to translate code");
        }
        return result;
    }
}

#endif
