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

#include "../decoder.h"
#include "normalCPU.h"
#include "../../softmmu/soft_code_page.h"
#include "../../softmmu/kmemory_soft.h"
#include "../armv7/armv7CPU.h"
#include "../armv8/armv8CPU.h"
#include "../../../util/ptrpool.h"

#ifdef BOXEDWINE_MULTI_THREADED
#ifdef _DEBUG
//#define START_OP(cpu, op) op->log(cpu)
#define START_OP(cpu, op)
#else
#define START_OP(cpu, op)
#endif
#else
#ifdef _DEBUG
#define START_OP(cpu, op) cpu->blockInstructionCount++; op->log(cpu)
 //#define START_OP(cpu, op)
#else
#define START_OP(cpu, op) cpu->blockInstructionCount++
#endif
#endif

#ifdef BOXEDWINE_DIRECT_NORMAL_DISPATCH
#define NEXT() cpu->eip.u32+=op->len; MUSTTAIL return normalDispatch(cpu, op->next);
#else
#define NEXT() cpu->eip.u32+=op->len; MUSTTAIL return op->next->pfn(cpu, op->next);
#endif
#define NEXT_DONE() cpu->nextOp = cpu->getNextOp();
#define NEXT_DONE_JUMP_OR_CALL() cpu->nextOp = cpu->getNextOp(OP_FLAG2_JUMP_TARGET);

static inline bool normalGetZF(CPU* cpu) {
    switch (cpu->lazyFlagType) {
    case FLAGS_NONE:
        return (cpu->flags & ZF) != 0;
    case FLAGS_ADD8:
    case FLAGS_OR8:
    case FLAGS_AND8:
    case FLAGS_SUB8:
    case FLAGS_XOR8:
    case FLAGS_INC8:
    case FLAGS_DEC8:
    case FLAGS_CMP8:
    case FLAGS_TEST8:
        return cpu->result.u8 == 0;
    case FLAGS_ADD16:
    case FLAGS_OR16:
    case FLAGS_AND16:
    case FLAGS_SUB16:
    case FLAGS_XOR16:
    case FLAGS_INC16:
    case FLAGS_DEC16:
    case FLAGS_CMP16:
    case FLAGS_TEST16:
        return cpu->result.u16 == 0;
    case FLAGS_ADD32:
    case FLAGS_OR32:
    case FLAGS_AND32:
    case FLAGS_SUB32:
    case FLAGS_XOR32:
    case FLAGS_INC32:
    case FLAGS_DEC32:
    case FLAGS_CMP32:
    case FLAGS_TEST32:
        return cpu->result.u32 == 0;
    default:
        return cpu->getZF();
    }
}

static inline bool normalGetCF(CPU* cpu) {
    switch (cpu->lazyFlagType) {
    case FLAGS_NONE:
        return (cpu->flags & CF) != 0;
    case FLAGS_ADD8:
        return cpu->result.u8 < cpu->dst.u8;
    case FLAGS_ADD16:
        return cpu->result.u16 < cpu->dst.u16;
    case FLAGS_ADD32:
        return cpu->result.u32 < cpu->dst.u32;
    case FLAGS_SUB8:
    case FLAGS_CMP8:
        return cpu->dst.u8 < cpu->src.u8;
    case FLAGS_SUB16:
    case FLAGS_CMP16:
        return cpu->dst.u16 < cpu->src.u16;
    case FLAGS_SUB32:
    case FLAGS_CMP32:
        return cpu->dst.u32 < cpu->src.u32;
    case FLAGS_OR8:
    case FLAGS_OR16:
    case FLAGS_OR32:
    case FLAGS_AND8:
    case FLAGS_AND16:
    case FLAGS_AND32:
    case FLAGS_XOR8:
    case FLAGS_XOR16:
    case FLAGS_XOR32:
    case FLAGS_TEST8:
    case FLAGS_TEST16:
    case FLAGS_TEST32:
        return false;
    case FLAGS_INC8:
    case FLAGS_INC16:
    case FLAGS_INC32:
    case FLAGS_DEC8:
    case FLAGS_DEC16:
    case FLAGS_DEC32:
        return cpu->oldCF != 0;
    default:
        return cpu->getCF();
    }
}

static inline bool normalGetNLE(CPU* cpu) {
    switch (cpu->lazyFlagType) {
    case FLAGS_NONE:
        return !(cpu->flags & ZF) && (((cpu->flags & SF) != 0) == ((cpu->flags & OF) != 0));
    case FLAGS_CMP8:
    case FLAGS_SUB8: {
        U8 result = cpu->result.u8;
        U8 overflow = (cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ result) & 0x80;
        return result != 0 && ((result & 0x80) != 0) == (overflow != 0);
    }
    case FLAGS_CMP16:
    case FLAGS_SUB16: {
        U16 result = cpu->result.u16;
        U16 overflow = (cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ result) & 0x8000;
        return result != 0 && ((result & 0x8000) != 0) == (overflow != 0);
    }
    case FLAGS_CMP32:
    case FLAGS_SUB32: {
        U32 result = cpu->result.u32;
        U32 overflow = (cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ result) & 0x80000000;
        return result != 0 && ((result & 0x80000000) != 0) == (overflow != 0);
    }
    case FLAGS_OR8:
    case FLAGS_AND8:
    case FLAGS_XOR8:
    case FLAGS_TEST8:
        return cpu->result.u8 != 0 && (cpu->result.u8 & 0x80) == 0;
    case FLAGS_OR16:
    case FLAGS_AND16:
    case FLAGS_XOR16:
    case FLAGS_TEST16:
        return cpu->result.u16 != 0 && (cpu->result.u16 & 0x8000) == 0;
    case FLAGS_OR32:
    case FLAGS_AND32:
    case FLAGS_XOR32:
    case FLAGS_TEST32:
        return cpu->result.u32 != 0 && (cpu->result.u32 & 0x80000000) == 0;
    default:
        return !cpu->getZF() && cpu->getSF() == cpu->getOF();
    }
}

// if jmp back, then return so that we don't blow the stack
#define NEXT_BRANCH1()                                      \
    cpu->eip.u32+=op->len;                                  \
    if (!(*(op->data.nextJump))) {                          \
        *(op->data.nextJump) = cpu->getNextOp(OP_FLAG2_JUMP_TARGET);            \
    }                                                       \
    cpu->nextOp = *(op->data.nextJump);

#define NEXT_BRANCH2() cpu->eip.u32+=op->len; if (!op->next) {op->next = cpu->getNextOp(); } cpu->nextOp = op->next;

#ifdef BOXEDWINE_DIRECT_NORMAL_DISPATCH
static void OPCALL normalDispatch(CPU* cpu, DecodedOp* op);
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

#include "normal_lock.h"

static OpCallback normalOps[NUMBER_OF_OPS];
static U32 normalOpsInitialized;

void OPCALL normal_sidt(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);    
    U32 eaa = eaa(cpu, op);
    cpu->memory->writew(eaa, 1023); // limit
    cpu->memory->writed(eaa + 2, 0); // base
#ifdef _DEBUG
    klog("sidt not implemented");
#endif
    NEXT();
}

void OPCALL onTestEnd(CPU* cpu, DecodedOp* op) {
    cpu->nextOp = op;
}

#ifdef BOXEDWINE_DIRECT_NORMAL_DISPATCH
static void OPCALL normalDispatch(CPU* cpu, DecodedOp* op) {
    switch (op->inst) {
#undef INIT_CPU
#define INIT_CPU(e, f) case e: MUSTTAIL return normal_##f(cpu, op);
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#undef INIT_CPU
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) case e##_Lock: MUSTTAIL return normal_##f##_lock(cpu, op);
#include "../common/cpu_init_lock.h"
#undef INIT_CPU_LOCK
#endif
    case SIDT: MUSTTAIL return normal_sidt(cpu, op);
    case Callback: MUSTTAIL return onExitSignal(cpu, op);
    case TestEnd: MUSTTAIL return onTestEnd(cpu, op);
    default: MUSTTAIL return op->pfn(cpu, op);
    }
}
#endif

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
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) normalOps[e##_Lock] = normal_##f##_lock;
#include "../common/cpu_init_lock.h"
#undef INIT_CPU_LOCK
#endif
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

#if defined(BOXEDWINE_JIT)
void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op);
#endif

NormalCPU::NormalCPU(KMemory* memory) : CPU(memory) {   
    initNormalOps();
#ifdef BOXEDWINE_JIT
    this->firstOp = firstDynamicOp;
    memcpy(this->calculateCF, memory->process->calculateCF, sizeof(calculateCF));
#else
    this->firstOp = nullptr;
#endif
}

OpCallback NormalCPU::getFunctionForOp(DecodedOp* op) {
    return normalOps[op->inst];
}

bool NormalCPU::isValidExecutableAddress(U32 address) {
    return (memory->getPageFlags(address >> K_PAGE_SHIFT) & PAGE_EXEC) != 0;
}

DecodedOp* NormalCPU::getOp(U32 startIp, U32 jumpTargetFlags) {
    if (!this->thread->process) // exit was called, don't need to pre-cache the next block
        return nullptr;

    DecodedOp* op = memory->getDecodedOp(startIp);

    if (!op) {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
        op = memory->getDecodedOp(startIp);
        if (!op) {
            U32 opCount = 0;
            U32 eipLen = 0;
            U32 address = startIp;

            op = decodeBlock(this, startIp, this->isBig(), opCount, eipLen);
            if (!op) {
                return nullptr;
            }
            DecodedOp* nextOp = op;

            while (nextOp) {
                if (!nextOp->pfn) {
                    nextOp->pfn = getFunctionForOp(nextOp);
                }
                if (memory->isAddressDynamic(address, nextOp->len)) {
                    nextOp->flags |= OP_FLAG_NO_JIT;
#ifdef BOXEDWINE_JIT
                    nextOp->runCount = JIT_RUN_COUNT + 1;
#endif
                }
                address += nextOp->len;
                nextOp = nextOp->next;
            }
            this->thread->memory->addCode_nolock(startIp, eipLen, op, opCount);            
        }
    }
#ifdef BOXEDWINE_JIT
    op->flags2 |= jumpTargetFlags;
    if (jumpTargetFlags && (op->flags2 & OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE)) {
        kpanic("JUMP_TARGET_ASSUMED_FALSE");
    }
#endif
    return op;
}

void NormalCPU::run() {
#ifdef BOXEDWINE_DIRECT_NORMAL_DISPATCH
    if (!nextOp) {
        if (thread->terminating) {
            return;
        }
        nextOp = getNextOp();
        if (!nextOp) {
            thread->seg_mapper(getEipAddress(), true, false, false);
            nextOp = getNextOp();
            if (!nextOp) {
                kpanic_fmt("Failed to get op for thread %d of process %d at address %x", thread->id, thread->process->id, getEipAddress());
            }
        }
    }
    normalDispatch(this, nextOp);
#else
#ifdef BOXEDWINE_JIT
    if (nextOp->runCount <= JIT_RUN_COUNT) {
        firstOp(this, nextOp);
    } else {
        nextOp->pfn(this, nextOp);
#if !defined(BOXEDWINE_MULTI_THREADED)
        this->blockInstructionCount += nextOp->blockOpCount;
#endif
    }
    if (!nextOp && !thread->terminating) {
        nextOp = getNextOp();
        if (!nextOp) {
            thread->seg_mapper(getEipAddress(), true, false, false);
            nextOp = getNextOp();
            if (!nextOp) {
                kpanic_fmt("Failed to get op for thread %d of process %d at address %x", thread->id, thread->process->id, getEipAddress());
            }
        }
    }
#else
    nextOp->pfn(this, nextOp);
#endif
#endif
}
