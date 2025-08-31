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
#include "../x32/x32CPU.h"
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

#ifdef BOXEDWINE_BINARY_TRANSLATOR
#define NEXT() cpu->eip.u32+=op->len
#define NEXT_DONE() 
#define NEXT_BRANCH1() cpu->eip.u32+=op->len
#define NEXT_BRANCH2() cpu->eip.u32+=op->len
#define NEXT_DONE_CALL()
#define NEXT_BRANCH1_CALL() cpu->eip.u32+=op->len
#else
#define NEXT() cpu->eip.u32+=op->len; op->next->pfn(cpu, op->next);
#define NEXT_DONE() cpu->nextOp = cpu->getNextOp();
#define NEXT_DONE_CALL() cpu->nextOp = cpu->getNextOp(0);

// if jmp back, then return so that we don't blow the stack
#define NEXT_BRANCH1()                                      \
    cpu->eip.u32+=op->len;                                  \
    if (!(*(op->data.nextJump))) {                          \
        *(op->data.nextJump) = cpu->getNextOp();            \
    }                                                       \
    cpu->nextOp = *(op->data.nextJump);

#define NEXT_BRANCH1_CALL()                                      \
    cpu->eip.u32+=op->len;                                  \
    if (!(*(op->data.nextJump))) {                          \
        *(op->data.nextJump) = cpu->getNextOp(0);            \
    }                                                       \
    cpu->nextOp = *(op->data.nextJump);

#define NEXT_BRANCH2() cpu->eip.u32+=op->len; if (!op->next) {op->next = cpu->getNextOp(); } cpu->nextOp = op->next;
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

NormalCPU::NormalCPU(KMemory* memory) : CPU(memory) {   
    initNormalOps();
#ifdef BOXEDWINE_DYNAMIC
    this->firstOp = firstDynamicOp;
#else
    this->firstOp = nullptr;
#endif
}

OpCallback NormalCPU::getFunctionForOp(DecodedOp* op) {
    return normalOps[op->inst];
}

bool NormalCPU::isValidReadAddress(U32 address) {
    return memory->canRead(address >> K_PAGE_SHIFT);
}

DecodedOp* NormalCPU::decodeOneOp(U32 eip) {
    U32 opCount = 0;
    U32 eipLen;
    DecodedOp* result = decodeBlock(this, eip, this->isBig(), opCount, eipLen);
    result->pfn = getFunctionForOp(result);
    return result;
}

DecodedOp* NormalCPU::getOp(U32 startIp, U32 flags) {
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

            DecodedOp* nextOp = op;

            while (nextOp) {
                if (!nextOp->pfn) {
                    nextOp->pfn = getFunctionForOp(nextOp);
                }
                if (memory->isAddressDynamic(address, nextOp->len)) {
                    nextOp->flags |= OP_FLAG_NO_JIT;
                    nextOp->runCount = JIT_RUN_COUNT + 1;
                }
                address += nextOp->len;
                nextOp = nextOp->next;
            }
            this->thread->memory->addCode_nolock(startIp, eipLen, op, opCount);            
        }
    }
    op->flags |= flags;
    return op;
}

void NormalCPU::run() {
#ifdef BOXEDWINE_DYNAMIC
    if (nextOp->runCount <= JIT_RUN_COUNT) {
        firstOp(this, nextOp);
    } else {
        nextOp->pfn(this, nextOp);
#if !defined(BOXEDWINE_MULTI_THREADED)
        this->blockInstructionCount += nextOp->blockOpCount;
#endif
    }
#else
    nextOp->pfn(this, nextOp);
#endif
}
