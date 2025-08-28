/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#include "btCpu.h"
#include "btData.h"
#include "ksignal.h"
#include "knativethread.h"
#include "knativesystem.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"
#include "../../softmmu/soft_page.h"

#ifdef BOXEDWINE_BINARY_TRANSLATOR

typedef void (*StartCPU)();

static_assert(sizeof(DecodedOpPageCache) == sizeof(DecodedOp*) * K_PAGE_SIZE, "binary translator assumes sizeof(DecodedOpPageCache) ");

BtCPU::BtCPU(KMemory* memory) : NormalCPU(memory) {
    opCache = (DecodedOp***)&memory->data->opCache.pageData[0];
}

void BtCPU::reset() {
    NormalCPU::reset();
    opCache = (DecodedOp***)&memory->data->opCache.pageData[0];
}

void BtCPU::run() {
    while (true) {
        this->exitToStartThreadLoop = 0;
        StartCPU start = (StartCPU)this->init();
        start();
#ifdef __TEST
        return;
#else
        if (this->thread->process->terminated) {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
            this->memory->cleanup();
        }
        if (this->thread->terminating) {
            break;
        }

        if (this->inException) {
            this->inException = false;
        }
        if (this->memory->deleteOnNextLoop) {
            // a bit ugly
            // see CodePageData::addCode
            // that code uses a custom deleter that looks at the current thread->memory->data
            KMemoryData* data = this->memory->data;
            this->memory->data = this->memory->deleteOnNextLoop;
            delete this->memory->deleteOnNextLoop;
            this->memory->data = data;
            this->memory->deleteOnNextLoop = nullptr;
        }
#endif
    }
}

void* BtCPU::translateChunk(U32 ip) {
    BtData* firstPass = getData1();
    firstPass->ip = ip;
    firstPass->startOfDataIp = ip;
    firstPass->startOfOpIp = ip; // in case first op is dynamic (F-16 on x64 with 4k page option can trigger this)
    translateData(firstPass);

    BtData* secondPass = getData2();
    secondPass->ip = ip;
    secondPass->startOfDataIp = ip;
    secondPass->startOfOpIp = ip;
    secondPass->calculatedEipLen = firstPass->ip - firstPass->startOfDataIp;
    translateData(secondPass, firstPass);
    S32 failedJumpOpIndex = this->preLinkCheck(secondPass);

    if (failedJumpOpIndex == -1) {        
        void* result = secondPass->commit(memory);
        link(secondPass, result);
        secondPass->makeLive((U8*)result);
        return result;
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

        void* result = secondPass->commit(memory);

        link(secondPass, result);
        secondPass->makeLive((U8*)result);
        return result;
    }
}

void BtCPU::clearTranslatedChunk(DecodedOp* op) {
}

U64 BtCPU::reTranslateChunk() {
#ifndef __TEST
    // only one thread at a time can update the host code pages and related date like opToAddressPages
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
#endif
    DecodedOp* op = getNextOp();
    if (op->blockStart) {
        DecodedOp* chunk = op->blockStart;
        clearTranslatedChunk(chunk);
        translateEip(chunk->eip - seg[CS].address);
    }

    U64 result = (U64)op->pfnJitCode;
    if (result == 0) {
        result = (U64)this->translateEip(this->eip.u32);
    }
    if (result == 0) {
        kpanic("BtCPU::reTranslateChunk failed to translate code in exception");
    }
    return result;
}

static U8 fetchByte(void* p, U32* eip) {
    KMemory* memory = (KMemory*)p;
    return memory->readb((*eip)++);
}

void* BtCPU::translateEipInternal(U32 ip) {
    if (!this->isBig()) {
        ip = ip & 0xFFFF;
    }
    U32 address = this->seg[CS].address + ip;
    DecodedOp* op = getOp(address, 0);
    void* result = op->pfnJitCode;

    if (!result) {
        result = translateChunk(ip);
    }
    return result;
}

void* BtCPU::translateEip(U32 ip) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);

    void* result = translateEipInternal(ip);
    makePendingCodePagesReadOnly();
    return result;
}

void BtCPU::makePendingCodePagesReadOnly() {
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
            kpanic("BtCPU::startException failed to translate code in exception");
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
        kpanic("BtCPU::handleFpuException failed to translate code");
    }
    return result;
}

U64 BtCPU::handleAccessException(DecodedOp* op) {
#ifdef BOXEDWINE_4K_PAGE_SIZE
    if (op->exceptionCount < 0xff) {
        op->exceptionCount++;
    } else {        
        U32 eip = op->blockStart->eip;
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        memory->removeCodeBlock(op->blockStart, false);
    }
#endif
    try {
        op->pfn(this, op);
    } catch (...) {
        
    }
    fillFlags();
    return (U64)this->translateEip(this->eip.u32);
}

extern std::atomic<int> platformThreadCount;

void BtCPU::startThread() {
    KThread::setCurrentThread(thread);

    try {
        this->run();
    } catch (...) {
        
    }
#ifndef __TEST
    KProcessPtr process = thread->process;
    process->deleteThread(thread);
    KThread::setCurrentThread(nullptr);
    platformThreadCount--;
    if (platformThreadCount == 0) {
        KSystem::shutingDown = true;
        KNativeSystem::postQuit();
    }
#endif
}

// called from another thread
void BtCPU::wakeThreadIfWaiting() {
    const std::shared_ptr<BoxedWineCondition> cond = thread->waitingCond;

    // wait up the thread if it is waiting
    if (cond) {
        cond->lock();
        cond->signal();
        cond->unlock();
    }
}

S32 BtCPU::preLinkCheck(BtData* data) {
    for (S32 i = 0; i < (S32)data->todoJump.size(); i++) {
        U32 eip = this->seg[CS].address + data->todoJump[i].eip;
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
    return -1;
}

U64 BtCPU::handleMissingCode(U32 page, U32 offset) {
    this->eip.u32 = ((page << K_PAGE_SHIFT) | offset) - this->seg[CS].address;
    return (U64)this->translateEip(this->eip.u32);
}

void terminateOtherThread(const std::shared_ptr<KProcess>& process, U32 threadId) {    
    while (true) {
        KThread* thread = process->getThreadById(threadId);
        if (thread) {
            thread->terminating = true;
            ((BtCPU*)thread->cpu)->exitToStartThreadLoop = true;
            ((BtCPU*)thread->cpu)->wakeThreadIfWaiting();
        }
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

#include "../x64/x64CPU.h"

#if !defined(BOXEDWINE_X64)
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
        op->pfn(cpu, op);
    } catch (...) {
        
    }
    cpu->fillFlags();
    cpu->returnHostAddress = (U64)cpu->translateEip(cpu->eip.u32);
    if (deallocOp) {
        op->dealloc(true);
    }
}
#endif
#endif
