#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "ksignal.h"
#include "../../source/emulation/hardmmu/kmemory_hard.h"
#include "../../source/emulation/cpu/armv8bt/armv8btCPU.h"
#include "../../source/emulation/cpu/armv8bt/armv8btAsm.h"
#include "../../source/emulation/cpu/binaryTranslation/btCodeChunk.h"

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


void syncFromException(Armv8btCPU* cpu, ucontext_t* context) {
    EAX = (U32)context->CONTEXT_REG(xEAX);
    ECX = (U32)context->CONTEXT_REG(xECX);
    EDX = (U32)context->CONTEXT_REG(xEDX);
    EBX = (U32)context->CONTEXT_REG(xEBX);
    ESP = (U32)context->CONTEXT_REG(xESP);
    EBP = (U32)context->CONTEXT_REG(xEBP);
    ESI = (U32)context->CONTEXT_REG(xESI);
    EDI = (U32)context->CONTEXT_REG(xEDI);

#ifdef _DEBUG
    for (int i = 0; i < 32; i++) {
        cpu->exceptionRegs[i] = context->CONTEXT_REG(i);
    }
#endif
    cpu->flags = (U32)context->CONTEXT_REG(xFLAGS);
    cpu->lazyFlags = FLAGS_NONE;

#ifdef __MACH__
    for (int i = 0; i < 8; i++) {
        memcpy(&cpu->xmm[i], &context->uc_mcontext->__ns.__v[xXMM0 + i], 16);
        cpu->reg_mmx[i].q = (U64)context->uc_mcontext->__ns.__v[vMMX0 + i];
    }
#else
    struct fpsimd_context* fc = getSimdContext(&context->uc_mcontext);

    for (int i = 0; i < 8; i++) {
        memcpy(&cpu->xmm[i], &fc->vregs[xXMM0 + i], 16);
        cpu->reg_mmx[i].q = (U64)fc->vregs[vMMX0 + i];
    }
#endif
}

class InException {
public:
    InException(BtCPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    BtCPU* cpu;
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

    BtCPU* cpu = (BtCPU*)currentThread->cpu;
    if (cpu != (BtCPU*)context->CONTEXT_REG(xCPU)) {
        return;
    }
    Armv8btCPU* armCpu = (Armv8btCPU*)cpu;

    syncFromException(armCpu, context);

    cpu->exceptionReadAddress = true;
    
    cpu->exceptionAddress = (U64)info->si_addr;
    cpu->exceptionSigNo = info->si_signo;
    cpu->exceptionSigCode = info->si_code;
    armCpu->exceptionIp = context->CONTEXT_PC;

#ifdef BOXEDWINE_64BIT_MMU
    KMemoryData* mem = getMemData(currentThread->memory);
    if ((cpu->exceptionSigNo == SIGBUS || cpu->exceptionSigNo == SIGSEGV) && mem->isAddressExecutable((void*)cpu->exceptionIp)) {
        U32 insn = *(U32*)(cpu->exceptionIp);
        
        bool isWrite = false;
        if (!Aarch64GetESR(context, &isWrite)) {
            // is_write logic from QEMU (GPL2)
            isWrite = (  (insn & 0xbfff0000) == 0x0c000000   /* C3.3.1 */
                       || (insn & 0xbfe00000) == 0x0c800000   /* C3.3.2 */
                       || (insn & 0xbfdf0000) == 0x0d000000   /* C3.3.3 */
                       || (insn & 0xbfc00000) == 0x0d800000   /* C3.3.4 */
                       || (insn & 0x3f400000) == 0x08000000   /* C3.3.6 */
                       || (insn & 0x3bc00000) == 0x39000000   /* C3.3.13 */
                       || (insn & 0x3fc00000) == 0x3d800000   /* ... 128bit */
                       /* Ingore bits 10, 11 & 21, controlling indexing.  */
                       || (insn & 0x3bc00000) == 0x38000000   /* C3.3.8-12 */
                       || (insn & 0x3fe00000) == 0x3c800000   /* ... 128bit */
                       /* Ignore bits 23 & 24, controlling indexing.  */
                       || (insn & 0x3a400000) == 0x28000000); /* C3.3.7,14-16 */
        }
        cpu->exceptionReadAddress = !isWrite;
    }
#endif
    if (armCpu->exceptionIp == 0) {
        kpanic("oops jumps to 0");
    }
#ifdef BOXEDWINE_64BIT_MMU
    if (mem->isAddressExecutable((void*)armCpu->exceptionIp)) {
        unsigned char* hostAddress = (unsigned char*)armCpu->exceptionIp;
        std::shared_ptr<BtCodeChunk> chunk = mem->getCodeChunkContainingHostAddress(hostAddress);
        if (chunk && chunk->getEipLen()) { // during start up eip is already set
            cpu->eip.u32 = chunk->getEipThatContainsHostAddress(hostAddress, NULL, NULL) - cpu->seg[CS].address;
        }
    }
#endif
    context->CONTEXT_PC = (U64)cpu->thread->process->runSignalAddress;
}

int getFPUCode(int code) {
    switch (code) {
    case FPE_INTDIV: return K_FPE_INTDIV;
    case FPE_INTOVF: return K_FPE_INTOVF;
    case FPE_FLTDIV: return K_FPE_FLTDIV;
    case FPE_FLTOVF: return K_FPE_FLTOVF;
    case FPE_FLTUND: return K_FPE_FLTUND;
    case FPE_FLTRES: return K_FPE_FLTRES;
    case FPE_FLTINV: return K_FPE_FLTINV;
    default: klog("getFPUCode unhandled code %d", code); return 0;
    }
}

void signalHandler() {
    BOXEDWINE_CRITICAL_SECTION;
    KThread* currentThread = KThread::currentThread();
    Armv8btCPU* cpu = (Armv8btCPU*)currentThread->cpu;
#ifdef BOXEDWINE_64BIT_MMU
    KMemoryData* mem = getMemData(currentThread->memory);
#endif
    U64 result = cpu->startException(cpu->exceptionAddress, cpu->exceptionReadAddress);
    if (result) {
        cpu->returnHostAddress = result;
        return;
    }
    InException e(cpu);
    if (cpu->exceptionSigNo == SIGFPE) {
        int code = getFPUCode(cpu->exceptionSigCode);
        cpu->returnHostAddress = cpu->handleFpuException(code);
        return;
    }
    kpanic("unhandled exception %d", cpu->exceptionSigNo);
}

#endif
