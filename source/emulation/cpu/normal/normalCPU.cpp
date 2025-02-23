#include "boxedwine.h"

#include "../decoder.h"
#include "normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../x32/x32CPU.h"
#include "../armv7/armv7CPU.h"
#include "../armv8/armv8CPU.h"
#include "../../../util/ptrpool.h"

#ifdef _DEBUG
//#define START_OP(cpu, op) op->log(cpu)
#define START_OP(cpu, op)
#else
#define START_OP(cpu, op)
#endif

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#define NEXT() cpu->eip.u32+=op->len
#define NEXT_DONE() 
#define NEXT_BRANCH1() cpu->eip.u32+=op->len
#define NEXT_BRANCH2() cpu->eip.u32+=op->len
#else
#define NEXT() cpu->eip.u32+=op->len; if (op->next == nullptr) op->next = cpu->getNextOp(); op->next->pfn(cpu, op->next)
#define NEXT_DONE() cpu->nextOp = cpu->getNextOp();
#define NEXT_DONE_CALL() cpu->nextOp = cpu->getNextOp(true);

// if jmp back, then return so that we don't blow the stack
#define NEXT_BRANCH1()                                      \
cpu->eip.u32+=op->len;                                      \
if (!op->nextJump) {                                        \
    DecodedOp* nextOp = cpu->getNextOp();                       \
    if (((S32)op->imm) > 0) {                               \
        nextOp->pfn(cpu, nextOp);                                   \
    } else {                                                \
        cpu->nextOp = nextOp;                                   \
    }                                                       \
} else {                                                    \
    if (!(*(op->nextJump))) {                               \
        *(op->nextJump) = cpu->getNextOp();                 \
    }                                                       \
    if (((S32)op->imm) > 0) {                               \
        (*(op->nextJump))->pfn(cpu, *(op->nextJump));       \
    } else {                                                \
        cpu->nextOp = *(op->nextJump);                      \
    }                                                       \
}
#define NEXT_BRANCH1_CALL()                                      \
cpu->eip.u32+=op->len;                                      \
if (!op->nextJump) {                                        \
    DecodedOp* nextOp = cpu->getNextOp(true);                       \
    if (((S32)op->imm) > 0) {                               \
        nextOp->pfn(cpu, nextOp);                                   \
    } else {                                                \
        cpu->nextOp = nextOp;                                   \
    }                                                       \
} else {                                                    \
    if (!(*(op->nextJump))) {                               \
        *(op->nextJump) = cpu->getNextOp(true);                 \
    }                                                       \
    if (((S32)op->imm) > 0) {                               \
        (*(op->nextJump))->pfn(cpu, *(op->nextJump));       \
    } else {                                                \
        cpu->nextOp = *(op->nextJump);                      \
    }                                                       \
}
#define NEXT_BRANCH2() cpu->eip.u32+=op->len; if (!op->next) {op->next = cpu->getNextOp(); } op->next->pfn(cpu, op->next);
#endif

#include "instructions.h"
#include "normal_arith.h"
#include "normal_conditions.h"
#include "normal_incdec.h"
#include "normal_pushpop.h"
#include "normal_setcc.h"
#include "normal_strings.h"
#include "normal_strings_op.h"
#include "normal_shift.h"
#include "normal_shift_op.h"
#include "normal_xchg.h"
#include "normal_bit.h"
#include "normal_mmx.h"
#include "normal_sse.h"
#include "normal_sse2.h"
#include "normal_fpu.h"
#include "normal_other.h"
#include "normal_jump.h"
#include "normal_move.h"

static OpCallback normalOps[NUMBER_OF_OPS];
static U32 normalOpsInitialized;

void OPCALL normal_sidt(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);    
    U32 eaa = eaa(cpu, op);
    cpu->memory->writew(eaa, 1023); // limit
    cpu->memory->writed(eaa, 0); // base
#ifdef _DEBUG
    klog("sidt not implemented");
#endif
    NEXT();
}

void OPCALL onTestEnd(CPU* cpu, DecodedOp* op) {
    cpu->nextOp = op;
}

static void initNormalOps() {
    if (normalOpsInitialized)
        return;
    normalOpsInitialized = 1;   
    for (int i=0;i<InstructionCount;i++) {
        normalOps[i] = normal_invalid;
    }
#define INIT_CPU(e, f) normalOps[e] = normal_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#undef INIT_CPU    
    
    normalOps[SLDTReg] = nullptr;
    normalOps[SLDTE16] = nullptr;
    normalOps[STRReg] = nullptr;
    normalOps[STRE16] = nullptr;
    normalOps[LLDTR16] = nullptr;
    normalOps[LLDTE16] = nullptr;
    normalOps[LTRR16] = nullptr;
    normalOps[LTRE16] = nullptr;
    normalOps[VERRR16] = nullptr;
    normalOps[VERWR16] = nullptr;
    normalOps[SGDT] = nullptr;
    normalOps[SIDT] = normal_sidt;
    normalOps[LGDT] = nullptr;
    normalOps[LIDT] = nullptr;
    normalOps[SMSWRreg] = nullptr;
    normalOps[SMSW] = nullptr;
    normalOps[LMSWRreg] = nullptr;
    normalOps[LMSW] = nullptr;
    normalOps[INVLPG] = nullptr;
    normalOps[Callback] = onExitSignal;
    normalOps[TestEnd] = onTestEnd;
}

OpCallback NormalCPU::getFunctionForOp(DecodedOp* op) {
    return normalOps[op->inst];
}

NormalCPU::NormalCPU(KMemory* memory) : CPU(memory) {   
    initNormalOps();
#ifdef BOXEDWINE_DYNAMIC
    this->firstOp = firstDynamicOp;
#else
    this->firstOp = nullptr;
#endif
}

#ifdef BOXEDWINE_MULTI_THREADED
void OPCALL emptyInst(CPU* cpu, DecodedOp* op) {
}

DecodedOp emptyOp;

void OPCALL lockOp8(CPU* cpu, DecodedOp* op) {
    BOXEDWINE_CRITICAL_SECTION;
    normalOps[op->inst](cpu, op);
    /*
    START_OP(cpu, op);
    if (!cpu->tmpLockAddress) {
        cpu->tmpLockAddress = cpu->thread->process->alloc(cpu->thread, 4);
    }
    U32 address = eaa(cpu, op);
    DecodedOp lockedOp = *op;
    lockedOp.base = SEG_ZERO;
    lockedOp.rm = regZero;
    lockedOp.sibIndex = regZero;
    lockedOp.sibScale = 0;
    lockedOp.disp = cpu->tmpLockAddress;
    lockedOp.ea16 = 0;

    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    Reg savedRegs[8];
    U32 savedEip = cpu->eip.u32;

    for (int i = 0; i < 8; i++) {
        savedRegs[i] = cpu->reg[i];
    }

    lockedOp.next = &emptyOp;

    while (true) {
        U8 oldValue = cpu->memory->readb(address);
        cpu->memory->writeb(cpu->tmpLockAddress, oldValue);

        normalOps[op->inst](cpu, &lockedOp);

        U8 newValue = cpu->memory->readb(cpu->tmpLockAddress);

        if (mem.compare_exchange_weak(oldValue, newValue)) {
            break;
        }
        for (int i = 0; i < 8; i++) {
            cpu->reg[i] = savedRegs[i];
        }
    }
    cpu->eip.u32 = savedEip;
    NEXT();
    */
}

void OPCALL lockOp16(CPU* cpu, DecodedOp* op) {
    BOXEDWINE_CRITICAL_SECTION;
    normalOps[op->inst](cpu, op);
    /*
    START_OP(cpu, op);
    if (!cpu->tmpLockAddress) {
        cpu->tmpLockAddress = cpu->thread->process->alloc(cpu->thread, 4);
    }
    U32 address = eaa(cpu, op);
    DecodedOp lockedOp = *op;
    lockedOp.base = SEG_ZERO;
    lockedOp.rm = regZero;
    lockedOp.sibIndex = regZero;
    lockedOp.sibScale = 0;
    lockedOp.disp = cpu->tmpLockAddress;
    lockedOp.ea16 = 0;

    LockData16* p= (LockData16*)cpu->memory->getRamPtr(address, 2, true);
    auto iptr = reinterpret_cast<std::uintptr_t>(p);
    if (iptr % 2) {
        BOXEDWINE_CRITICAL_SECTION;
        normalOps[op->inst](cpu, op);
        return;
    }
    std::atomic_ref<U16> mem(p->data);
    Reg savedRegs[8];
    U32 savedEip = cpu->eip.u32;

    for (int i = 0; i < 8; i++) {
        savedRegs[i] = cpu->reg[i];
    }

    lockedOp.next = &emptyOp;

    while (true) {
        U16 oldValue = cpu->memory->readw(address);
        cpu->memory->writew(cpu->tmpLockAddress, oldValue);

        normalOps[op->inst](cpu, &lockedOp);

        U16 newValue = cpu->memory->readw(cpu->tmpLockAddress);

        if (mem.compare_exchange_weak(oldValue, newValue)) {
            break;
        }
        for (int i = 0; i < 8; i++) {
            cpu->reg[i] = savedRegs[i];
        }
    }
    cpu->eip.u32 = savedEip;
    NEXT();
    */
}

void OPCALL lockOp32(CPU* cpu, DecodedOp* op) {
    // :TODO: not sure why std::atomic_ref didn't work for me, it passes unit tests, but I see problems, especially in high contention areas
    // like the audio thread where pthread_mutex will use cmpxchge32r32 with futex a lot.  The symptom that cmpxchge32r32 didn't work correctly
    // is that 2 threads end up waiting on the same value for a futex

    // this is pretty heavy to do a global lock for this instruction and the rest of the instructions in this block, but most blocks are small
    // and so far this hasn't been a problem
    BOXEDWINE_CRITICAL_SECTION;
    normalOps[op->inst](cpu, op);
    /*
    START_OP(cpu, op);
    if (!cpu->tmpLockAddress) {
        cpu->tmpLockAddress = cpu->thread->process->alloc(cpu->thread, 4);
    }
    U32 address = eaa(cpu, op);
    DecodedOp lockedOp = *op;
    lockedOp.base = SEG_ZERO;
    lockedOp.rm = regZero;
    lockedOp.sibIndex = regZero;
    lockedOp.sibScale = 0;
    lockedOp.disp = cpu->tmpLockAddress;
    lockedOp.ea16 = 0;

    LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
    auto iptr = reinterpret_cast<std::uintptr_t>(p);
    if (iptr % 4) {
        BOXEDWINE_CRITICAL_SECTION;
        normalOps[op->inst](cpu, op);
        return;
    }
    std::atomic_ref<U32> mem(p->data);
    Reg savedRegs[8];
    U32 savedEip = cpu->eip.u32;

    for (int i = 0; i < 8; i++) {
        savedRegs[i] = cpu->reg[i];
    }

    lockedOp.next = &emptyOp;
    
    while (true) {
        U32 oldValue = cpu->memory->readd(address);   
        cpu->memory->writed(cpu->tmpLockAddress, oldValue);

        normalOps[op->inst](cpu, &lockedOp);

        U32 newValue = cpu->memory->readd(cpu->tmpLockAddress);

        if (mem.compare_exchange_weak(oldValue, newValue)) {
            break;
        }
        for (int i = 0; i < 8; i++) {
            cpu->reg[i] = savedRegs[i];
        }        
    }
    cpu->eip.u32 = savedEip;
    NEXT();
    */
}

void OPCALL lockCmpXchg8b(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    common_cmpxchg8b_lock(cpu, address);
    NEXT();
}

void OPCALL lockCmpXchg32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    common_cmpxchge32r32_lock(cpu, address, op->reg);
    NEXT();
}

void OPCALL lockCmpXchg16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    common_cmpxchge16r16_lock(cpu, address, op->reg);
    NEXT();
}

void OPCALL lockCmpXchg8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    common_cmpxchge8r8_lock(cpu, address, op->reg);
    NEXT();
}
#endif

bool setNormalFunction(DecodedOp* op) {
    bool usesLock = false;
#ifdef BOXEDWINE_MULTI_THREADED
    // abiword has a lock test reg, value
    if (op->lock && instructionInfo[op->inst].writeMemWidth != 0 && instructionInfo[op->inst].readMemWidth != 0) {
        emptyOp.next = nullptr;
        emptyOp.pfn = emptyInst;

        usesLock = true;
        if (op->inst == CmpXchgE8R8) {
            op->pfn = lockCmpXchg8;
        } else if (op->inst == CmpXchgE16R16) {
            op->pfn = lockCmpXchg16;
        } else if (op->inst == CmpXchgE32R32) {
            op->pfn = lockCmpXchg32;
        } else if (op->inst == CmpXchg8b) {
            op->pfn = lockCmpXchg8b;
        } else if (instructionInfo[op->inst].readMemWidth == 8) {
            op->pfn = lockOp8;
        } else if (instructionInfo[op->inst].readMemWidth == 16) {
            op->pfn = lockOp16;
        } else if (instructionInfo[op->inst].readMemWidth == 32) {
            op->pfn = lockOp32;
        } else {
            kpanic("Unexepected memory width for locked instruction: %d", instructionInfo[op->inst].readMemWidth);
        }
    } else
#endif
        if (!op->pfn) { // callback will be set by decoder
            op->pfn = normalOps[op->inst];
        }
    return usesLock;
}

bool NormalCPU::shouldContinue(U32 eip) {
    return this->memory->getDecodedOp(eip) == nullptr;
}

DecodedOp* NormalCPU::getDecodedOp(U32 eip) {
    return memory->getDecodedOp(eip);
}

DecodedOp** NormalCPU::getOpLocation(U32 eip) {
    return this->memory->getDecodedOpLocation(eip);
}

U8 NormalCPU::fetchByte(U32* eip) {
    if (*eip - this->seg[CS].address == 0xFFFF && !this->isBig()) {
        kpanic("eip wrapped around.");
    }
    return this->memory->readb((*eip)++);
}

DecodedOp* NormalCPU::getNextOp(bool callTarget) {
    if (!this->thread->process) // exit was called, don't need to pre-cache the next block
        return nullptr;

    U32 startIp = getEipAddress();
    DecodedOp* op = memory->getDecodedOp(startIp);

    if (!op) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex); // lock across all emulated processes because the DecodedOpCache is global
        op = memory->getDecodedOp(startIp);
        if (!op) {
            U32 opCount = 0;
            U32 eipLen = 0;
            op = decodeBlock(this, startIp, this->isBig(), opCount, eipLen);

            DecodedOp* nextOp = op;
            while (nextOp) {
                setNormalFunction(nextOp);
                nextOp = nextOp->next;
            }
            if (callTarget && firstOp && op->next != nullptr) {
                DecodedOp* jitOp = DecodedOp::alloc();
                jitOp->pfn = firstOp;
                jitOp->inst = JIT;
                jitOp->next = op;
                jitOp->len = 0; // needs to be 0 so that code that loops won't count it as part of the address running total
                op = jitOp;
            }
            this->thread->memory->addCode_nolock(startIp, eipLen, op, true);
        }
    }
    return op;
}

void NormalCPU::run() {
    if (nextOp->inst == InstructionCount) {
        nextOp = getNextOp();
    }
    nextOp->pfn(this, nextOp);
}