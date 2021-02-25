#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "ksignal.h"
#include "../../source/emulation/cpu/armv8bt/armv8btCPU.h"
#include "../../source/emulation/cpu/armv8bt/armv8btAsm.h"
#include "../../source/emulation/cpu/binaryTranslation/btCodeChunk.h"

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

void syncFromException(Armv8btCPU* cpu, ucontext_t* context) {
    mcontext_t* c = &context->uc_mcontext;

    EAX = (U32)c->regs[xEAX];
    ECX = (U32)c->regs[xECX];
    EDX = (U32)c->regs[xEDX];
    EBX = (U32)c->regs[xEBX];
    ESP = (U32)c->regs[xESP];
    EBP = (U32)c->regs[xEBP];
    ESI = (U32)c->regs[xESI];
    EDI = (U32)c->regs[xEDI];
    cpu->regPage = c->regs[xPage];
    cpu->regOffset = c->regs[xOffset];

#ifdef _DEBUG
    for (int i = 0; i < 32; i++) {
        cpu->exceptionRegs[i] = c->regs[i];
    }
#endif
    cpu->destEip = (U32)((c->regs[xBranchLargeAddressOffset] - (U64)cpu->eipToHostInstructionAddressSpaceMapping) >> 3);
    cpu->flags = (U32)c->regs[xFLAGS];
    cpu->lazyFlags = FLAGS_NONE;

    struct fpsimd_context* fc = getSimdContext(c);

    memcpy(&cpu->xmm[0], &fc->vregs[xXMM0], 16);
    memcpy(&cpu->xmm[1], &fc->vregs[xXMM1], 16);
    memcpy(&cpu->xmm[2], &fc->vregs[xXMM2], 16);
    memcpy(&cpu->xmm[3], &fc->vregs[xXMM3], 16);
    memcpy(&cpu->xmm[4], &fc->vregs[xXMM4], 16);
    memcpy(&cpu->xmm[5], &fc->vregs[xXMM5], 16);
    memcpy(&cpu->xmm[6], &fc->vregs[xXMM6], 16);
    memcpy(&cpu->xmm[7], &fc->vregs[xXMM7], 16);

    for (int i = 0; i < 8; i++) {
        cpu->reg_mmx[i].q = (U64)fc->vregs[vMMX0 + i];
    }
}

class InException {
public:
    InException(BtCPU* cpu) : cpu(cpu) { this->cpu->inException = true; }
    ~InException() { this->cpu->inException = false; }
    BtCPU* cpu;
};

U32 exceptionCount;

// this will quickly store the info then exit to signalHandler() to perform the logic there
void platformHandler(int sig, siginfo_t* info, void* vcontext) {
    exceptionCount++;
    KThread* currentThread = KThread::currentThread();
    if (!currentThread) {
        return;
    }
    ucontext_t* context = (ucontext_t*)vcontext;
    mcontext_t* mc = &context->uc_mcontext;

    BtCPU* cpu = (BtCPU*)currentThread->cpu;
    if (cpu != (BtCPU*)mc->regs[xCPU]) {
        return;
    }
    Armv8btCPU* armCpu = (Armv8btCPU*)cpu;

    syncFromException(armCpu, context);

    // :TODO:
    // cpu->exceptionReadAddress = mc->error_code == 0;
    cpu->exceptionAddress = (U64)info->si_addr;
    cpu->exceptionSigNo = info->si_signo;
    cpu->exceptionSigCode = info->si_code;
    armCpu->exceptionIp = mc->pc;

    if (armCpu->exceptionIp == 0) {
        kpanic("oops jumps to 0");
    }
    if ((armCpu->exceptionIp & 0xFFFFFFFF00000000l) == (U64)cpu->thread->memory->executableMemoryId) {
        unsigned char* hostAddress = (unsigned char*)armCpu->exceptionIp;
        std::shared_ptr<BtCodeChunk> chunk = cpu->thread->memory->getCodeChunkContainingHostAddress(hostAddress);
        if (chunk && chunk->getEipLen()) { // during start up eip is already set
            cpu->eip.u32 = chunk->getEipThatContainsHostAddress(hostAddress, NULL, NULL) - cpu->seg[CS].address;
        }
    }
    mc->pc = (U64)cpu->thread->process->runSignalAddress;
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

    U64 result = cpu->startException(cpu->exceptionAddress, cpu->exceptionReadAddress, NULL, NULL);
    if (result) {
        cpu->returnHostAddress = result;
        return;
    }
    InException e(cpu);
    if (cpu->exceptionSigNo == SIGILL || cpu->exceptionSigNo == SIGTRAP) {
        //if (cpu->exceptionRSP & 0xf) {
        //    kpanic("seh_filter: bad stack alignment");
        //}
        U64 rip = cpu->handleIllegalInstruction(cpu->exceptionIp);
        if (rip) {
            cpu->returnHostAddress = rip;
            return;
        }
        cpu->translateEip(cpu->eip.u32);
        cpu->returnHostAddress = cpu->exceptionIp;
        return;
    } else if (cpu->exceptionSigNo == SIGBUS && cpu->exceptionSigCode == BUS_ADRALN) {
        // :TODO: figure out how AC got set, I've only seen this while op logging
        cpu->flags &= ~AC;
        cpu->returnHostAddress = cpu->exceptionIp;
        return;
    } else if ((cpu->exceptionSigNo == SIGBUS || cpu->exceptionSigNo == SIGSEGV) && ((cpu->exceptionIp & 0xFFFFFFFF00000000l) == (U64)cpu->thread->memory->executableMemoryId)) {
        U64 rip = cpu->handleAccessException(cpu->exceptionIp, cpu->exceptionAddress, cpu->exceptionReadAddress);
        if (rip) {
            cpu->returnHostAddress = rip;
            return;
        }
        cpu->returnHostAddress = cpu->exceptionIp;
        return;
    } else if (cpu->exceptionSigNo == SIGFPE) {
        int code = getFPUCode(cpu->exceptionSigCode);
        cpu->returnHostAddress = cpu->handleFpuException(code, NULL, NULL);
        return;
    } else if (*((U8*)cpu->exceptionIp) == 0xce || *((U8*)cpu->exceptionIp) == 0xcd) {
        U64 rip = cpu->handleIllegalInstruction(cpu->exceptionIp);
        if (rip) {
            cpu->returnHostAddress = rip;
            return;
        }
        cpu->returnHostAddress = cpu->exceptionIp;
        return;
    }
    kpanic("unhandled exception %d", cpu->exceptionSigNo);
}

#endif