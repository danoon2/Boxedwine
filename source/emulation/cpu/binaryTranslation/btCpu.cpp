#include "boxedwine.h"
#include "btCodeChunk.h"
#include "btCpu.h"
#include "btData.h"
#include "ksignal.h"
#include "knativethread.h"
#include "knativesystem.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR

typedef void (*StartCPU)();

void BtCPU::run() {
    while (true) {
        this->memOffset = this->thread->process->memory->id;
        this->exitToStartThreadLoop = 0;
        if (setjmp(this->runBlockJump) == 0) {
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

U64 BtCPU::reTranslateChunk() {
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
        kpanic("BtCPU::reTranslateChunk failed to translate code in exception");
    }
    return result;
}

U64 BtCPU::handleChangedUnpatchedCode(U64 rip) {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);
#endif

    unsigned char* hostAddress = (unsigned char*)rip;
    std::shared_ptr<BtCodeChunk> chunk = this->thread->memory->getCodeChunkContainingHostAddress(hostAddress);
    if (!chunk) {
        kpanic("BtCPU::handleChangedUnpatchedCode: could not find chunk");
    }
    U32 startOfEip = chunk->getEipThatContainsHostAddress(hostAddress, NULL, NULL);
    if (!chunk->isDynamicAware() || !chunk->retranslateSingleInstruction(this, hostAddress)) {
        chunk->releaseAndRetranslate();
    }
    U64 result = (U64)this->thread->memory->getExistingHostAddress(startOfEip);
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
        // free'd chunks are filled in with 0xcd, if this one is free'd, it is possible another thread replaced the chunk
        // while this thread jumped to it and this thread waited in the critical section at the top of this function.
        void* host = this->thread->memory->getExistingHostAddress(this->eip.u32 + this->seg[CS].address);
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

static U8 fetchByte(U32* eip) {
    return readb((*eip)++);
}

DecodedOp* BtCPU::getOp(U32 eip, bool existing) {
    if (this->isBig()) {
        eip += this->seg[CS].address;
    }
    else {
        eip = this->seg[CS].address + (eip & 0xFFFF);
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

void* BtCPU::translateEip(U32 ip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(this->thread->memory->executableMemoryMutex);

    void* result = translateEipInternal(ip);
    makePendingCodePagesReadOnly();
    return result;
}

void BtCPU::makePendingCodePagesReadOnly() {
    for (int i = 0; i < (int)this->pendingCodePages.size(); i++) {
        // the chunk could cross a page and be a mix of dynamic and non dynamic code
        if (this->thread->memory->dynamicCodePageUpdateCount[this->pendingCodePages[i]] != MAX_DYNAMIC_CODE_PAGE_COUNT) {
            this->thread->memory->makeCodePageReadOnly(this->pendingCodePages[i]);
        }
    }
    this->pendingCodePages.clear();
}

void BtCPU::markCodePageReadOnly(BtData* data) {
    U32 pageStart = this->thread->memory->getNativePage((data->startOfDataIp + this->seg[CS].address) >> K_PAGE_SHIFT);
    if (pageStart == 0) {
        return; // x64CPU::init()
    }
    U32 pageEnd = this->thread->memory->getNativePage((data->ip + this->seg[CS].address - 1) >> K_PAGE_SHIFT);
    S32 pageCount = pageEnd - pageStart + 1;

#ifndef __TEST
    for (int i = 0; i < pageCount; i++) {
        pendingCodePages.push_back(pageStart + i);
    }
#endif    
}

U64 BtCPU::startException(U64 address, bool readAddress, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) {
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
        if (result == 0) {
            kpanic("Armv8btCPU::startException failed to translate code in exception");
        }
        return result;
    }
    return 0;
}

U64 BtCPU::handleFpuException(int code, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo) {
    if (doSyncFrom) {
        doSyncFrom(NULL);
    }
    if (code == K_FPE_INTDIV) {
        this->prepareException(EXCEPTION_DIVIDE, 0);
    }
    else if (code == K_FPE_INTOVF) {
        this->prepareException(EXCEPTION_DIVIDE, 1);
    }
    else {
        this->prepareFpuException(code);
    }
    if (doSyncTo) {
        doSyncTo(NULL);
    }
    U64 result = (U64)this->translateEip(this->eip.u32);
    if (result == 0) {
        kpanic("Armv8btCPU::handleFpuException failed to translate code");
    }
    return result;
}

extern U32 platformThreadCount;

void BtCPU::startThread() {
    jmp_buf jmpBuf;
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

void terminateOtherThread(const std::shared_ptr<KProcess>& process, U32 threadId) {
    process->threadsCondition.lock();
    KThread* thread = process->getThreadById(threadId);
    if (thread) {
        thread->terminating = true;
        ((BtCPU*)thread->cpu)->exitToStartThreadLoop = true;
        ((BtCPU*)thread->cpu)->wakeThreadIfWaiting();
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
    ((BtCPU*)thread->cpu)->exitToStartThreadLoop = true;
}

void unscheduleThread(KThread* thread) {
}

#endif