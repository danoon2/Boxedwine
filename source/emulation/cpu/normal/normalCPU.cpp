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
#define NEXT() cpu->eip.u32+=op->len; op->next->pfn(cpu, op->next)
#define NEXT_DONE() cpu->nextBlock = cpu->getNextBlock();
#define NEXT_BRANCH1() cpu->eip.u32+=op->len; if (!cpu->currentBlock->next1) {cpu->currentBlock->next1 = cpu->getNextBlock(); cpu->currentBlock->next1->addReferenceFrom(cpu->currentBlock);} cpu->nextBlock = cpu->currentBlock->next1
#define NEXT_BRANCH2() cpu->eip.u32+=op->len; if (!cpu->currentBlock->next2) {cpu->currentBlock->next2 = cpu->getNextBlock(); cpu->currentBlock->next2->addReferenceFrom(cpu->currentBlock);} cpu->nextBlock = cpu->currentBlock->next2
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
}

OpCallback NormalCPU::getFunctionForOp(DecodedOp* op) {
    initNormalOps();
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

U8 fetchByte(void* data, U32 *eip) {
    CPU* cpu = (CPU*)data;
    if (*eip - cpu->seg[CS].address == 0xFFFF && !cpu->isBig()) {
        kpanic("eip wrapped around.");
    }
    return cpu->memory->readb((*eip)++);
}

class NormalBlock : public DecodedBlock {
public:
    static NormalBlock* alloc();
    static void clearCache();

    NormalBlock();

    // from DecodedBlock
    void dealloc(bool delayed) override;
    void run(CPU* cpu) override;

    void reset();
};

NormalBlock::NormalBlock() {
    this->reset();
}

void NormalBlock::run(CPU* cpu) {
#ifdef _DEBUG
    if (this->op== nullptr || this->op->pfn== nullptr) {
        kpanic("NormalBlock::run is about to crash");
    }
#endif  
    this->op->pfn(cpu, this->op);
    this->runCount++;
    cpu->blockInstructionCount+=this->opCount;
}

static PtrPool<NormalBlock> freeBlocks;

void NormalBlock::reset() {
    this->op = nullptr;
    this->bytes = 0;
    this->opCount = 0;
    this->runCount = 0;
    this->next1 = nullptr;
    this->next2 = nullptr;
    this->referencedFrom = nullptr;
}

void NormalBlock::clearCache() {
    freeBlocks.deleteAll();
}

NormalBlock* NormalBlock::alloc() {
    return freeBlocks.get();
}

void NormalBlock::dealloc(bool delayed) {
    KThread* thread = KThread::currentThread();
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->process->normalBlockMutex);
    bool doDelete = false;
    if (thread) {
        CPU* cpu = thread->cpu;
        if (cpu && cpu->delayedFreeBlock && cpu->delayedFreeBlock != cpu->currentBlock) {
            DecodedBlock* b = cpu->delayedFreeBlock;
            cpu->delayedFreeBlock = nullptr;
            b->dealloc(false);
        }
        if (cpu && ((delayed && !cpu->delayedFreeBlock) || this == cpu->currentBlock)) {
            cpu->delayedFreeBlock = this;
        } else {
            if (op) {
                this->op->dealloc(true);
                this->op = nullptr;
            }
            doDelete = true;

        }
    } else {
        this->op->dealloc(true);
        this->op = nullptr;
        doDelete = true;
    }
    if (this->next1) {
        this->next1->removeReferenceFrom(this);
        this->next1 = nullptr;
    }
    if (this->next2) {
        this->next2->removeReferenceFrom(this);
        this->next2 = nullptr;
    }
    DecodedBlockFromNode* from = this->referencedFrom;
    while (from) {
        DecodedBlockFromNode* n = from->next;
        if (from->block) {
            if (from->block->next1 == this) {
                from->block->next1 = nullptr;
            }
            if (from->block->next2 == this) {
                from->block->next2 = nullptr;
            }
        }
        from->dealloc();
        from = n;
    }
    this->referencedFrom = nullptr;
    if (doDelete) {
        freeBlocks.put(this);
    }
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
            kpanic_fmt("Unexepected memory width for locked instruction: %d", instructionInfo[op->inst].readMemWidth);
        }
    } else
#endif
        if (!op->pfn) { // callback will be set by decoder
            op->pfn = normalOps[op->inst];
        }
    return usesLock;
}

DecodedBlock* NormalCPU::getBlockForInspectionButNotUsed(CPU* cpu, U32 address, bool big) {
    DecodedBlock* block = NormalBlock::alloc();
    decodeBlock(fetchByte, cpu, address, big, 0, 0, 0, block);
    block->address = address;
    initNormalOps();
    DecodedOp* op = block->op;

    while (op) {
        if (!op->pfn) // callback will be set by decoder
            setNormalFunction(op);
        op = op->next;
    }
    return block;
}

DecodedOp* NormalCPU::decodeSingleOp(CPU* cpu, U32 address) {
    thread_local static DecodedBlock* block = new DecodedBlock();
    decodeBlock(fetchByte, cpu, address, cpu->isBig(), 1, K_PAGE_SIZE, 0, block);
    DecodedOp* op = block->op;
    setNormalFunction(op);
    block->op = nullptr;
    return op;
}

DecodedBlock* NormalCPU::getNextBlock() {
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    return nullptr;
#else
    if (!this->thread->process) // exit was called, don't need to pre-cache the next block
        return nullptr;
    
    U32 startIp = (this->big?this->eip.u32:this->eip.u16) + this->seg[CS].address;
    DecodedBlock* block = this->thread->memory->getCodeBlock(startIp);

    if (!block) {
        bool blockCreated = false;
        {
            BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(thread->process->normalBlockMutex);
            block = this->thread->memory->getCodeBlock(startIp);
            if (!block) {
                block = NormalBlock::alloc();
                blockCreated = true;
                decodeBlock(fetchByte, this, startIp, this->isBig(), 0, K_PAGE_SIZE, 0, block);
                block->address = startIp;

                DecodedOp* op = block->op;
                bool excludeFromJIT = false;
                while (op) {
                    if (setNormalFunction(op)) {
                        // these 4 will call the appropriate lock function
                        if (op->inst != CmpXchgE8R8 && op->inst != CmpXchgE16R16 && op->inst != CmpXchgE32R32 && op->inst != CmpXchg8b) {
                            excludeFromJIT = true;
                        }                        
                    }
                    op = op->next;
                }
#ifdef BOXEDWINE_MULTI_THREADED
                if (!excludeFromJIT)
#endif
                    if (this->firstOp) {
                        op = DecodedOp::alloc();
                        op->inst = Custom1;
                        op->pfn = this->firstOp;
                        op->next = block->op;
                        block->op = op;
                    }
            }
        }
        if (blockCreated) {
            this->thread->memory->addCodeBlock(block);
        }
    }
    return block;
#endif
}

void NormalCPU::run() {    
    currentBlock = this->nextBlock;
    currentBlock->run(this);    
#ifdef _DEBUG
    if (!this->nextBlock && !this->yield && !this->thread->terminating) {
        kpanic("NormalCPU::run no block set");
    }
    currentBlock = nullptr;
#endif
}

void NormalCPU::clearCache() {
    NormalBlock::clearCache();
}
