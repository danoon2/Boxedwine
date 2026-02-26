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

#if defined(BOXEDWINE_JIT_ARMV8) && defined(BOXEDWINE_HOST_EXCEPTIONS)
#include "ksignal.h"
#include "../../source/emulation/softmmu/kmemory_soft.h"
#include "../../source/emulation/cpu/normal/normalCPU.h"
#ifdef __MACH__
#define __USE_GNU
#define _XOPEN_SOURCE
#include <ucontext.h>
#include <signal.h>
#include <pthread.h>
#define CONTEXT_REG(x) uc_mcontext->__ss.__x[x]
#define CONTEXT_PC uc_mcontext->__ss.__pc
#else
#define CONTEXT_REG(x) uc_mcontext.regs[x]
#define CONTEXT_PC uc_mcontext.pc
#include <ucontext.h>
#include <signal.h>
#include <pthread.h>
struct fpsimd_context* getSimdContext(mcontext_t* mc) {
    struct _aarch64_ctx* head = (struct _aarch64_ctx*) &mc->__reserved;
    size_t offset = 0;
    
    while (1) {
        int err = 0;

        head = (struct _aarch64_ctx*) &mc->__reserved[offset];

        U32 magic =  head->magic;
        U32 size = head->size;
        
        switch (magic) {
            case 0:
                return NULL;
            case FPSIMD_MAGIC:
                return (struct fpsimd_context*)head;
            case ESR_MAGIC:
                break; // ignore
            default:
                return NULL;
        }
        if (size < sizeof(*head)) {
            return NULL;
        }
        if (size > sizeof(mc->__reserved) - (sizeof(*head) + offset)) {
            return NULL;
        }
        offset += size;
    }
}
#endif

extern U8 regCache[8];
extern U8 xmmCache[8];

void syncFromException(CPU* cpu, ucontext_t* context) {
    for (int i = 0; i < 8; i++) {
        cpu->reg[i].u32 = (U32)context->CONTEXT_REG(regCache[i]);
    }

    cpu->flags = (U32)context->CONTEXT_REG(8);
    cpu->lazyFlagType = (LazyFlagType)context->CONTEXT_REG(11);
    cpu->src.u32 = (U32)context->CONTEXT_REG(16);
    cpu->dst.u32 = (U32)context->CONTEXT_REG(10);
    cpu->result.u32 = (U32)context->CONTEXT_REG(15);

#ifdef __MACH__
    for (int i = 0; i < 8; i++) {
        memcpy(&cpu->xmm[i], &context->uc_mcontext->__ns.__v[xmmCache[i]], 16);
    }
#else
    struct fpsimd_context* fc = getSimdContext(&context->uc_mcontext);

    for (int i = 0; i < 8; i++) {
        memcpy(&cpu->xmm[i], &fc->vregs[xmmCache[i]], 16);
    }
#endif
}

class InException {
public:
    InException(CPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    CPU* cpu;
};

U32 exceptionCount;

#ifdef __MACH__

#ifndef ESR_ELx_WNR
#define ESR_ELx_WNR        ((U32)(1) << 6)
#endif

static bool Aarch64GetESR(ucontext_t *ucontext, bool* isWrite) {
    __darwin_mcontext64* machineContext = ucontext->uc_mcontext;
    U64 esr = machineContext->__es.__esr;
    *isWrite = (esr & ESR_ELx_WNR) != 0;
    return true;
}
#else
static bool Aarch64GetESR(ucontext_t *ucontext, bool* isWrite) {
    U8 *aux = ucontext->uc_mcontext.__reserved;
    while (true) {
        _aarch64_ctx *ctx = (_aarch64_ctx *)aux;
        if (ctx->size == 0) break;
        if (ctx->magic == ESR_MAGIC) {
            U64 esr = ((struct esr_context*)ctx)->esr;
            U64 ESR_ELx_WNR = 1U << 6;
            *isWrite = (esr & ESR_ELx_WNR) != 0;
            return true;
        }
        aux += ctx->size;
    }
    return false;
}
#endif

// this will quickly store the info then exit to signalHandler() to perform the logic there
void platformHandler(int sig, siginfo_t* info, void* vcontext) {
    exceptionCount++;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return;
    }
    ucontext_t* context = (ucontext_t*)vcontext;

    CPU* cpu = (CPU*)currentThread->cpu;
    if (cpu != (CPU*)context->CONTEXT_REG(19)) {
        return;
    }

    syncFromException(cpu, context);

    bool isWrite = false;
    if (Aarch64GetESR((ucontext_t*)context, &isWrite)) {
        cpu->exceptionReadAddress = !isWrite;
    } else {
        cpu->exceptionReadAddress = true;
    }
    cpu->exceptionAddress = (U64)info->si_addr;
    cpu->exceptionSigNo = info->si_signo;
    cpu->exceptionSigCode = info->si_code;
    cpu->exceptionIp = context->CONTEXT_PC;

    context->CONTEXT_PC = (U64)cpu->thread->process->signalHandler;
}

void signalHandler(CPU* cpu) {
    void* result = cpu->startException(cpu->exceptionAddress, cpu->exceptionReadAddress);
    if (result) {
        cpu->returnHostAddress = result;
        return;
    }
    InException e(cpu);

    if (cpu->exceptionSigNo == SIGSEGV || cpu->exceptionSigNo == SIGBUS) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex); // jitCache needs this
        U32 eip = 0;
        if (!getMemData(cpu->memory)->findOpFromJitAddress((U8*)cpu->exceptionIp, eip)) {
            klog("probably about to crash, could not find emulation instruction that caused exception");
            cpu->returnHostAddress = cpu->thread->process->blockExitNoSync;
        } else {
            cpu->eip.u32 = eip;
            DecodedOp* op = cpu->getNextOp();
            void* result = cpu->handleAccessException(op);
            if (cpu->nextOp->pfnJitCode) {
                cpu->returnHostAddress = cpu->nextOp->pfnJitCode;
            } else {
                cpu->returnHostAddress = result;
            }
        }
        return;
    }
    kpanic_fmt("unhandled exception %d", cpu->exceptionSigNo);
}

#endif
