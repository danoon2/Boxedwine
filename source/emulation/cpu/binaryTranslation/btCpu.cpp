#include "boxedwine.h"
#include "btCodeChunk.h"
#include "btCpu.h"
#include "btData.h"
#include "ksignal.h"
#include "knativethread.h"
#include "knativesystem.h"
#include "../../hardmmu/kmemory_hard.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR

typedef void (*StartCPU)();

void BtCPU::run() {
    while (true) {
#ifdef BOXEDWINE_64BIT_MMU
        this->memOffset = getMemData(memory)->id;
#endif
        this->exitToStartThreadLoop = 0;
        StartCPU start = (StartCPU)this->init();
        start();
#ifdef __TEST
        return;
#else
        if (this->thread->process->terminated) {
            this->memory->cleanup();
        }
        if (this->thread->terminating) {
            break;
        }

        KMemoryData* mem = getMemData(memory);
        mem->clearDelayedReset();

        if (this->inException) {
            this->inException = false;
        }
#endif
    }
}

std::shared_ptr<BtCodeChunk> BtCPU::translateChunk(U32 ip) {
    BtData* firstPass = getData1();
    firstPass->ip = ip;
    firstPass->startOfDataIp = ip;
    translateData(firstPass);

    BtData* secondPass = getData2();
    secondPass->ip = ip;
    secondPass->startOfDataIp = ip;
    secondPass->calculatedEipLen = firstPass->ip - firstPass->startOfDataIp;
    translateData(secondPass, firstPass);
    S32 failedJumpOpIndex = this->preLinkCheck(secondPass);

    if (failedJumpOpIndex == -1) {
        std::shared_ptr<BtCodeChunk> chunk = secondPass->commit(false);
        link(secondPass, chunk);
        return chunk;
    }
    else {
        firstPass->reset();
        firstPass->ip = ip;
        firstPass->startOfDataIp = ip;
        firstPass->stopAfterInstruction = failedJumpOpIndex;
        translateData(firstPass);

        secondPass->reset();
        secondPass->ip = ip;
        secondPass->startOfDataIp = ip;
        secondPass->calculatedEipLen = firstPass->ip - firstPass->startOfDataIp;
        secondPass->stopAfterInstruction = failedJumpOpIndex;
        translateData(secondPass, firstPass);

        std::shared_ptr<BtCodeChunk> chunk = secondPass->commit(false);
        link(secondPass, chunk);
        return chunk;
    }
}

U64 BtCPU::reTranslateChunk() {
    KMemoryData* mem = getMemData(memory);
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
#endif
    std::shared_ptr<BtCodeChunk> chunk = mem->getCodeChunkContainingEip(this->eip.u32 + this->seg[CS].address);
    if (chunk) {
        chunk->releaseAndRetranslate();
    }

    U64 result = (U64)mem->getExistingHostAddress(this->eip.u32 + this->seg[CS].address);
    if (result == 0) {
        result = (U64)this->translateEip(this->eip.u32);
    }
    if (result == 0) {
        kpanic("BtCPU::reTranslateChunk failed to translate code in exception");
    }
    return result;
}

U64 BtCPU::handleChangedUnpatchedCode(U64 rip) {
    KMemoryData* mem = getMemData(memory);
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
#endif

    unsigned char* hostAddress = (unsigned char*)rip;
    std::shared_ptr<BtCodeChunk> chunk = mem->getCodeChunkContainingHostAddress(hostAddress);
    if (!chunk) {
        kpanic("BtCPU::handleChangedUnpatchedCode: could not find chunk");
    }
    U32 startOfEip = chunk->getEipThatContainsHostAddress(hostAddress, NULL, NULL);
    if (!chunk->isDynamicAware() || !chunk->retranslateSingleInstruction(this, hostAddress)) {
        chunk->releaseAndRetranslate();
    }
    U64 result = (U64)mem->getExistingHostAddress(startOfEip);
    if (result == 0) {
        result = (U64)this->translateEip(startOfEip - this->seg[CS].address);
    }
    if (result == 0) {
        kpanic("BtCPU::handleChangedUnpatchedCode failed to translate code in exception");
    }
    return result;
}

U64 BtCPU::handleIllegalInstruction(U64 rip) {
    if (*((U8*)rip) == 0xce) {
        return this->handleChangedUnpatchedCode(rip);
    }
    if (*((U8*)rip) == 0xcd) {
        KMemoryData* mem = getMemData(memory);
        // free'd chunks are filled in with 0xcd, if this one is free'd, it is possible another thread replaced the chunk
        // while this thread jumped to it and this thread waited in the critical section at the top of this function.
        void* host = mem->getExistingHostAddress(this->eip.u32 + this->seg[CS].address);
        if (host) {
            return (U64)host;
        }
        else {
            kpanic("BtCPU::handleIllegalInstruction tried to run code in a free'd chunk");
        }
    }
    kpanic("BtCPU::handleIllegalInstruction instruction %X not handled", *(U32*)rip);
    return 0;
}

static U8 fetchByte(void* p, U32* eip) {
    KMemory* memory = (KMemory*)p;
    return memory->readb((*eip)++);
}

DecodedOp* BtCPU::getOp(U32 eip, bool existing) {
    KMemoryData* mem = getMemData(memory);

    if (this->isBig()) {
        eip += this->seg[CS].address;
    }
    else {
        eip = this->seg[CS].address + (eip & 0xFFFF);
    }
    if (!existing || mem->getExistingHostAddress(eip)) {
        thread_local static DecodedBlock* block = new DecodedBlock();
        decodeBlock(fetchByte, memory, eip, this->isBig(), 4, 64, 1, block);
        return block->op;
    }
    return NULL;
}

void* BtCPU::translateEipInternal(U32 ip) {
    if (!this->isBig()) {
        ip = ip & 0xFFFF;
    }
    U32 address = this->seg[CS].address + ip;
    KMemoryData* mem = getMemData(memory);
    void* result = mem->getExistingHostAddress(address);

    if (!result) {
        std::shared_ptr<BtCodeChunk> chunk = this->translateChunk(ip);
        result = chunk->getHostAddress();
        chunk->makeLive();
    }
    return result;
}

void* BtCPU::translateEip(U32 ip) {
    KMemoryData* mem = getMemData(memory);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);

    void* result = translateEipInternal(ip);
    makePendingCodePagesReadOnly();
    return result;
}

void BtCPU::makePendingCodePagesReadOnly() {
#ifdef BOXEDWINE_64BIT_MMU 
    KMemoryData* mem = getMemData(memory);
    for (int i = 0; i < (int)this->pendingCodePages.size(); i++) {
        // the chunk could cross a page and be a mix of dynamic and non dynamic code
        if (mem->dynamicCodePageUpdateCount[this->pendingCodePages[i]] != MAX_DYNAMIC_CODE_PAGE_COUNT) {
            mem->makeCodePageReadOnly(this->pendingCodePages[i]);
        }
    }
#endif
    this->pendingCodePages.clear();
}

U64 BtCPU::startException(U64 address, bool readAddress) {
    if (this->thread->terminating) {
        return (U64)this->returnToLoopAddress;
    }
    if (this->inException) {
        this->thread->seg_mapper((U32)address, readAddress, !readAddress);
        U64 result = (U64)this->translateEip(this->eip.u32);
        if (result == 0) {
            kpanic("Armv8btCPU::startException failed to translate code in exception");
        }
        return result;
    }
    return 0;
}

U64 BtCPU::handleFpuException(int code) {
    if (code == K_FPE_INTDIV) {
        this->prepareException(EXCEPTION_DIVIDE, 0);
    }
    else if (code == K_FPE_INTOVF) {
        this->prepareException(EXCEPTION_DIVIDE, 1);
    }
    else {
        this->prepareFpuException(code);
    }
    U64 result = (U64)this->translateEip(this->eip.u32);
    if (result == 0) {
        kpanic("Armv8btCPU::handleFpuException failed to translate code");
    }
    return result;
}

U32 dynamicCodeExceptionCount;

#ifdef BOXEDWINE_64BIT_MMU
void BtCPU::markCodePageReadOnly(BtData* data) {
    KMemoryData* mem = getMemData(memory);
    U32 pageStart = mem->getNativePage((data->startOfDataIp + this->seg[CS].address) >> K_PAGE_SHIFT);
    if (pageStart == 0) {
        return; // x64CPU::init()
    }
    U32 pageEnd = mem->getNativePage((data->ip + this->seg[CS].address - 1) >> K_PAGE_SHIFT);
    S32 pageCount = pageEnd - pageStart + 1;

    for (int i = 0; i < pageCount; i++) {
        pendingCodePages.push_back(pageStart + i);
    }
}

U64 BtCPU::handleAccessException(U64 ip, U64 address, bool readAddress) {
    KMemoryData* mem = getMemData(memory);

    if ((address & 0xFFFFFFFF00000000l) == mem->id) {
        U32 emulatedAddress = (U32)address;
        U32 page = emulatedAddress >> K_PAGE_SHIFT;
        U32 flags = mem->flags[page];
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);

        // do we need to dynamicly grow the stack?
        if (page >= this->thread->stackPageStart && page < this->thread->stackPageStart + this->thread->stackPageCount) {
            U32 startPage = mem->getEmulatedPage(mem->getNativePage(page - INITIAL_STACK_PAGES)); // stack grows down
            U32 endPage = this->thread->stackPageStart + this->thread->stackPageCount - this->thread->stackPageSize;
            U32 pageCount = endPage - startPage;
            mem->allocPages(thread, startPage, pageCount, PAGE_READ | PAGE_WRITE | PAGE_MAPPED, 0, 0, 0);
            this->thread->stackPageSize += pageCount;
            return 0;
        }
        std::shared_ptr<BtCodeChunk> chunk = mem->getCodeChunkContainingHostAddress((void*)ip);
        if (chunk) {
            this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)ip, NULL, NULL) - this->seg[CS].address;
            // if the page we are trying to access needs a special memory offset and this instruction isn't flagged to looked at that special memory offset, then flag it
            if (flags & PAGE_MAPPED_HOST && (((flags & PAGE_READ) && readAddress) || ((flags & PAGE_WRITE) && !readAddress))) {
                mem->setNeedsMemoryOffset(getEipAddress());
                DecodedOp* op = this->getOp(this->eip.u32, true);
                handleStringOp(op); // if we were in the middle of a string op, then reset RSI and RDI so that we can re-enter the same op
                chunk->releaseAndRetranslate();
                op->dealloc(true);
                return getIpFromEip();
            }
            else {
                if (!readAddress && (flags & PAGE_WRITE) && !(mem->nativeFlags[mem->getNativePage(page)] & NATIVE_FLAG_CODEPAGE_READONLY)) {
                    // :TODO: this is a hack, why is it necessary
                    mem->updatePagePermission(page, 1);
                    return 0;
                }
                else {
                    int ii = 0;
                }
            }
        }
    }

    U32 inst = *((U32*)ip);

    if (inst == largeAddressJumpInstruction && mem->isValidReadAddress((U32)this->destEip, 1)) { // useLargeAddressSpace = true
        return (U64)this->translateEip((U32)this->destEip - this->seg[CS].address);
    }
    else if ((inst == pageJumpInstruction || inst == pageOffsetJumpInstruction) && (this->regPage || this->regOffset)) { // if these constants change, update handleMissingCode too     
        return this->handleMissingCode((U32)this->regPage, (U32)this->regOffset); // useLargeAddressSpace = false
    }
    else if (inst == 0xcdcdcdcd) {
        // this thread was waiting on the critical section and the thread that was currently in this handler removed the code we were running
        void* host = mem->getExistingHostAddress(this->eip.u32 + this->seg[CS].address);
        if (host) {
            return (U64)host;
        }
        else {
            U64 result = (U64)this->translateEip(this->eip.u32);
            if (!result) {
                kpanic("x64CPU::handleAccessException tried to run code in a free'd chunk");
            }
            return result;
        }
    }
    else {
        // check if the emulated memory caused the exception
        if ((address & 0xFFFFFFFF00000000l) == mem->id) {
            U32 emulatedAddress = (U32)address;
            U32 page = emulatedAddress >> K_PAGE_SHIFT;
            // check if emulated memory that caused the exception is a page that has code
            if (mem->nativeFlags[mem->getNativePage(page)] & NATIVE_FLAG_CODEPAGE_READONLY) {
                dynamicCodeExceptionCount++;
                return this->handleCodePatch(ip, emulatedAddress);
            }
            U32 nativeFlags = mem->nativeFlags[mem->getNativePage(page)];
            if ((flags & PAGE_MAPPED_HOST) || (flags & (PAGE_READ | PAGE_WRITE)) != (nativeFlags & (PAGE_READ | PAGE_WRITE))) {
                int ii=0;
            }
        }

        // this can be exercised with Wine 5.0 and CC95 demo installer, it is triggered in strlen as it tries to grow the stack
        this->thread->seg_mapper((U32)address, readAddress, !readAddress, false);
        U64 result = (U64)this->translateEip(this->eip.u32);
        if (result == 0) {
            kpanic("x64CPU::handleAccessException failed to translate code");
        }
        return result;
    }
}

// if the page we are writing to has code that we have cached, then it will be marked NATIVE_FLAG_CODEPAGE_READONLY
//
// 1) This function will clear the page of all cached code
// 2) Mark all the old cached code with "0xce" so that if the program tries to run it again, it will re-compile it
// 3) NATIVE_FLAG_CODEPAGE_READONLY will be removed
U64 BtCPU::handleCodePatch(U64 rip, U32 address) {
    KMemoryData* mem = getMemData(memory);
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(mem->executableMemoryMutex);
#endif
    U32 nativePage = mem->getNativePage(address >> K_PAGE_SHIFT);
    // get the emulated eip of the op that corresponds to the host address where the exception happened
    std::shared_ptr<BtCodeChunk> chunk = mem->getCodeChunkContainingHostAddress((void*)rip);

    this->eip.u32 = chunk->getEipThatContainsHostAddress((void*)rip, NULL, NULL) - this->seg[CS].address;

    // make sure it wasn't changed before we got the executableMemoryMutex lock
    if (!(mem->nativeFlags[nativePage] & NATIVE_FLAG_CODEPAGE_READONLY)) {
        return getIpFromEip();
    }

    // get the emulated op that caused the write
    DecodedOp* op = this->getOp(this->eip.u32, true);
    if (op) {
        U32 addressStart = address;
        U32 len = instructionInfo[op->inst].writeMemWidth / 8;

        // Fix string will set EDI and ESI back to their correct values so we can re-enter this instruction
        if (handleStringOp(op)) {
            if (op->repNotZero || op->repZero) {
                len = len * (isBig() ? THIS_ECX : THIS_CX);
            }

            if (this->getDirection() == 1) {
                addressStart = (this->isBig() ? THIS_EDI : THIS_DI) + this->seg[ES].address;
            } else {
                addressStart = (this->isBig() ? THIS_EDI : THIS_DI) + this->seg[ES].address + (instructionInfo[op->inst].writeMemWidth / 8) - len;
            }
        }
        U32 startPage = addressStart >> K_PAGE_SHIFT;
        U32 endPage = (addressStart + len - 1) >> K_PAGE_SHIFT;
        mem->clearHostCodeForWriting(mem->getNativePage(startPage), mem->getNativePage(endPage - startPage) + 1);
        op->dealloc(true);
        return getIpFromEip();
    } else {
        kpanic("Threw an exception from a host location that doesn't map to an emulated instruction");
    }
    return 0;
}

bool BtCPU::handleStringOp(DecodedOp* op) {
    return op->isStringOp();
}

#endif

extern std::atomic<int> platformThreadCount;

void BtCPU::startThread() {
    KThread::setCurrentThread(thread);

    // :TODO: hopefully this will eventually go away.  For now this prevents a signal from being generated which isn't handled yet
    KNativeThread::sleep(50);

    try {
        this->run();
    } catch (...) {
        int ii = 0;
    }
    std::shared_ptr<KProcess> process = thread->process;
    process->deleteThread(thread);

    platformThreadCount--;
    if (platformThreadCount == 0) {
        KSystem::shutingDown = true;
        KNativeSystem::postQuit();
    }
}

// called from another thread
void BtCPU::wakeThreadIfWaiting() {
    BoxedWineCondition* cond = thread->waitingCond;

    // wait up the thread if it is waiting
    if (cond) {
        cond->lock();
        cond->signal();
        cond->unlock();
    }
}

DecodedBlock* BtCPU::getNextBlock() {
    return NULL;
}

S32 BtCPU::preLinkCheck(BtData* data) {
    for (S32 i = 0; i < (S32)data->todoJump.size(); i++) {
        U32 eip = this->seg[CS].address + data->todoJump[i].eip;
        U8 size = data->todoJump[i].offsetSize;

        if (size == 4 && data->todoJump[i].sameChunk) {
            bool found = false;

            for (U32 ip = 0; ip < data->ipAddressCount; ip++) {
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

U64 BtCPU::getIpFromEip() {
    U32 a = this->getEipAddress();
    KMemoryData* mem = getMemData(memory);

    U64 result = (U64)mem->getExistingHostAddress(a);
    if (!result) {
        this->translateEip(this->eip.u32);
        result = (U64)mem->getExistingHostAddress(a);
    }
    if (result == 0) {
        kpanic("BtCPU::getIpFromEip failed to translate code");
    }
    return result;
}

U64 BtCPU::handleMissingCode(U32 page, U32 offset) {
    this->eip.u32 = ((page << K_PAGE_SHIFT) | offset) - this->seg[CS].address;
    return (U64)this->translateEip(this->eip.u32);
}

void terminateOtherThread(const std::shared_ptr<KProcess>& process, U32 threadId) {
    KThread* thread = process->getThreadById(threadId);        
    if (thread) {
        thread->terminating = true;
        ((BtCPU*)thread->cpu)->exitToStartThreadLoop = true;
        ((BtCPU*)thread->cpu)->wakeThreadIfWaiting();
    }

    while (true) {
        BOXEDWINE_CRITICAL_SECTION_WITH_CONDITION(process->threadRemovedCondition);
        if (!process->getThreadById(threadId)) {
            break;
        }
        BOXEDWINE_CONDITION_WAIT_TIMEOUT(process->threadRemovedCondition, 1000);
    }
}

void terminateCurrentThread(KThread* thread) {
    thread->terminating = true;
    ((BtCPU*)thread->cpu)->exitToStartThreadLoop = true;
}

void unscheduleThread(KThread* thread) {
}

static void OPCALL emptyOp(CPU* cpu, DecodedOp* op) {
}

#include "../x64/x64CPU.h"

#if !defined(BOXEDWINE_64BIT_MMU) && !defined(BOXEDWINE_X64)
void common_runSingleOp(BtCPU* cpu) {           
    U32 address = cpu->eip.u32;

    if (cpu->isBig()) {
        address += cpu->seg[CS].address;
    } else {
        address = cpu->seg[CS].address + (address & 0xFFFF);
    }

    DecodedOp* op = cpu->currentSingleOp;
    bool deallocOp = false;
    if (!op) {
        op = NormalCPU::decodeSingleOp(cpu, address);
        deallocOp = true;
    } else if (!op) {
        kpanic("common_runSingleOp oops");
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
    cpu->fillFlags();
    cpu->returnHostAddress = (U64)cpu->translateEip(cpu->eip.u32);
    if (deallocOp) {
        op->dealloc(true);
    }
}
#endif
#endif
