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

#ifdef BOXEDWINE_DYNAMIC
#include "dynamic.h"
#include "../normal/normalCPU.h"

extern U8* ramPages[K_NUMBER_OF_PAGES];
static DynamicCodeGen::OpFunction dynamicOps[NUMBER_OF_OPS];
static U32 dynamicOpsInitialized;

void DynamicCodeGen::onTestEnd(DecodedOp* op) {
    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), (DYN_PTR_SIZE)op);
}

static void initDynamicOps() {
    if (dynamicOpsInitialized)
        return;
    if (offsetof(CPU, eip.u32) > 127)
        kpanic("initDynamicOps wasn't expecting eip offset to be greater than 127");

    if (offsetof(CPU, reg[8].u32) > 127)
        kpanic("initDynamicOps wasn't expecting reg[8] offset to be greater than 127");

    if (offsetof(CPU, seg[6].address) > 127)
        kpanic("initDynamicOps wasn't expecting reg[8] offset to be greater than 127");

    dynamicOpsInitialized = 1;
    for (int i = 0; i < InstructionCount; i++) {
        dynamicOps[i] = &DynamicData::dynamic_invalid_op;
    }
#define INIT_CPU(e, f) dynamicOps[e] = &DynamicData::dynamic_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) dynamicOps[e##_Lock] = &DynamicData::dynamic_##f##_lock;
#include "../common/cpu_init_lock.h"
#endif

#undef INIT_CPU    

    dynamicOps[SLDTReg] = 0;
    dynamicOps[SLDTE16] = 0;
    dynamicOps[STRReg] = 0;
    dynamicOps[STRE16] = 0;
    dynamicOps[LLDTR16] = 0;
    dynamicOps[LLDTE16] = 0;
    dynamicOps[LTRR16] = 0;
    dynamicOps[LTRE16] = 0;
    dynamicOps[VERRR16] = 0;
    dynamicOps[VERWR16] = 0;
    dynamicOps[SGDT] = 0;
    dynamicOps[SIDT] = &DynamicData::dynamic_sidt;
    dynamicOps[LGDT] = 0;
    dynamicOps[LIDT] = 0;
    dynamicOps[SMSWRreg] = 0;
    dynamicOps[SMSW] = 0;
    dynamicOps[LMSWRreg] = 0;
    dynamicOps[LMSW] = 0;
    dynamicOps[INVLPG] = 0;
    dynamicOps[Callback] = &DynamicData::dynamic_callback;
    dynamicOps[TestEnd] = &DynamicCodeGen::dynamic_onTestEnd;
}

#ifdef _DEBUG
static void logBlock(CPU* cpu, U32 address, DecodedOp* op, U32 len) {
    static BWriteFile file;
    static int count;

    count++;
    if (!file.isOpen()) {
        file.createNew("jit.txt");
    }
    BString name = cpu->thread->process->getModuleName(address);
    U32 offset = cpu->thread->process->getModuleEip(address);
    file.writeFormat("Block %d in %s(%x)\n", count, name.c_str(), offset);
    while (op && len) {
        if (op->isDirectBranch()) {
            file.writeFormat("%x %x %s -> %x\n", cpu->thread->process->id, address, op->toString().c_str(), (address + op->len + op->imm));
        } else {
            file.writeFormat("%x %x %s\n", cpu->thread->process->id, address, op->toString().c_str());
        }
        address += op->len;
        len -= op->len;
        op = op->next;
    }
    file.write("\n");
    file.flush();
}
#endif

bool DynamicCodeGen::calculateLongestBlock(DecodedOp* op) {
    U32 eip = this->startingEip;
    DecodedOp* nextOp = op;
    U32 furthestJump = 0;

    // find the longest block we can compile
    // branches that jump out of the block will be the end of the block

    // 1st pass, find longest block including all direct jumps (conditional jumps, direct jumps, loop, etc)

    // jumpTo will keep track of valid jump targets.  We need this if we are going to decode more instructions (cpu->getOp)
    // Without this the next byte of instruction may actually be invalid, I have seen skipped bytes in the instructions,
    // I assume its for alignment/performance reasons.  Firefight installer will trigger this
    BHashTable<U32, DecodedOp*> jumpTo;

    // opentdd will trigger this isValid check
    while (nextOp && nextOp->isValid()) {
        // could be ret, call, int.  Basically this is an instruction where we are not guaranteed to see a next instruction
        if (nextOp->isBranch() && !nextOp->isDirectJumpBranch()) {
            // is this the last return, if so, then don't decode more
            if (nextOp->isRet() && furthestJump < eip) {
                break;
            }
            if (nextOp->isIndirectJump()) {
                // opentdd needs this when creating a new game, I'm not sure why data.cpu->memory->getDecodedOp(eip + nextOp->len) will find an op but its not correct, might be another bug 
                break;
            }
            if (!nextOp->next) {
                // don't call cpu->getOp since that will decode and we are not sure the next byte is a valid instruction.
                // we can call memory->getDecodedOp to see if this instruction has already been decoded, in that case we know its valid.
                nextOp->next = this->cpu->memory->getDecodedOp(eip + nextOp->len);
            }            
            if (!nextOp->next && nextOp->isDirectBranchWithNext()) {
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            }
            if (!nextOp->next && nextOp->isCall() && furthestJump > eip) {
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            }
            /*
            * With this code enabled, need for speed 2 demo will fail to load, right as it should start the EA logo
            * I'm not sure why, this seems like it would be safe, since there is a jump that lands here, it should
            * be valid
            * 
            * With this code disabled, I didn't notice a performance hit to Quake 2, but the average number of ops fell
            * from 33 to 22 when launching an app.  Maybe those extra ops were never use anyway?
            * */
            if (!nextOp->next && jumpTo.contains(eip + nextOp->len)) {
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            }
            
            if (!nextOp->next) {
                // since we couldn't figure out if the next byte is part of a valid instruction, we are done looking
                break;
            }
        }
        if (nextOp->isDirectJumpBranch() && (eip + nextOp->len + nextOp->imm) < this->startingEip) {
            // if we have somewhere to go after this, then continue

            // see if we can restart this JIT with the target of this jump to before the incoming op argument to create a bigger JIT block
            DecodedOp* targetOp = this->cpu->memory->getDecodedOp(eip + nextOp->len + nextOp->imm);
            if (!targetOp) {
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            }
            if (targetOp) {
                if (!targetOp->pfnJitCode) {
                    startNewJIT(cpu, eip + nextOp->len + nextOp->imm, targetOp);
                    if (op->flags & OP_FLAG_JIT) {
                        // doJIT successfully compiled the previous code and it picked up our current block
                        return false;
                    }
                }
            }
            if (!nextOp->isDirectBranchWithNext() && !jumpTo.contains(eip + nextOp->len)) {
                // if we have no valid location after this what can we do?
                break;
            }
        }
        if (nextOp->isDirectBranch()) {
            U32 address = eip + nextOp->len + nextOp->imm;
            jumpTo.set(address, nextOp);
            furthestJump = std::max(address, furthestJump);
        }
        //
        // how to handle a call deep down in an if statement 
        //
        // allow call if there are valid jumps that go over it, the call should do an early block return in this case
        if (!nextOp->next) {
            if (nextOp->isDirectBranchWithNext() || jumpTo.contains(eip + nextOp->len)) {
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            } else {
                nextOp->next = this->cpu->memory->getDecodedOp(eip + nextOp->len);
                if (!nextOp->next && nextOp->isCall() && furthestJump > eip) {
                    nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
                }
            }
        }
        eip += nextOp->len;
        if (nextOp->next) {
            if (nextOp->next->flags & OP_FLAG_NO_JIT) {
                break;
            }
            if (nextOp->next->inst == Done) {
                // F-16 needs this
                break;
            }
        }
        nextOp = nextOp->next;
    }
    // find longest block where all direct jumps don't go past the block
    U32 lastFurthestEip = eip;
    while (true) {
        nextOp = op;
        this->lastOpEip = this->startingEip;
        while (nextOp && this->lastOpEip < lastFurthestEip) {
            if (nextOp->isDirectJumpBranch() && !(nextOp->inst == Loop || nextOp->inst == LoopZ || nextOp->inst == LoopNZ || nextOp->inst == Jcxz)) {
                U32 target = this->lastOpEip + nextOp->len + nextOp->imm;
                if (target > lastFurthestEip || target < this->startingEip) {
                    break;
                }
            }
            if (nextOp->next) {
                this->lastOpEip += nextOp->len;
            }
            nextOp = nextOp->next;
        }
        if (!nextOp || !nextOp->next) {
            break;
        }
        U32 lastEip = this->lastOpEip;
        if (lastFurthestEip == lastEip) {
            break;
        }
        // since we didn't make it to the end of the ops, try again
        // an op we just looked at might be going to a place between lastFurthestEip and data.lastOpEip which is not valid since it is now passed the end of the block
        lastFurthestEip = lastEip;
    }
    return true;
}

static DecodedOp* removeJITBlock(DecodedOp* op) {
    for (int i = 0; i < op->blockOpCount; i++) {
        op->pfnJitCode = nullptr;
        op->pfn = NormalCPU::getFunctionForOp(op);
        op->blockStart = nullptr;
        op->blockLen = 0;
        op->blockOpCount = 0;
        op = op->next;
    }
    return op;
}

void DynamicCodeGen::removeJIT(DecodedOp* op, U32 count) {
    for (U32 i = 0; i < count; i++) {
        if (op->blockStart) {
            removeJITBlock(op->blockStart);
        }
    }
}

bool DynamicCodeGen::compileOps(DecodedOp* op) {
    DecodedOp* nextOp = op;
    this->emulatedLen = 0;
    this->blockOpCount = 0;

    while (nextOp) {
        if (nextOp->flags & OP_FLAG_NO_JIT) {
            return false;
        }

        memset(this->regUsed, 0, sizeof(this->regUsed));
#ifndef __TEST
#ifdef _DEBUG
        //callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)nextOp, DYN_PARAM_CONST_PTR, false);
#endif
#endif
        this->emulatedLen += nextOp->len;
        this->blockOpCount++;
        this->eipToBufferPos.set(this->currentEip, getBufferSize());
        if (nextOp->lock) {
            // so that intra block jumps that try to skip a lock will find the lock version of the op anyway
            this->eipToBufferPos.set(this->currentEip + 1, getBufferSize());
        }
        preOp(nextOp);
        (this->*dynamicOps[nextOp->inst])(nextOp);
        postOp(nextOp);
        this->currentEip += nextOp->len;
        if (getIfJumpSize()) {
            kpanic_fmt("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
        }
        if (this->currentEip > this->lastOpEip) {
            break;
        } else {
            nextOp = nextOp->next;
        }
    }
    return true;
}

void DynamicCodeGen::doJIT(U32 address, DecodedOp* op) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
    if (!cpu->thread->process->startJITOp) {
        DynamicCodeGen* jit = startNewJIT(cpu);
        cpu->thread->process->startJITOp = (OpCallback)jit->createStartJITCode();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->jumpToNextJIT = jit->createJumpEip();
        delete jit;
    }

    // did another thread beat us to JITing this block?
    if (op->flags & OP_FLAG_JIT) {
        // this will get triggered a few times, especially during shutdown
        // I have see this in firefight installer at the end and opentdd start up
        return;
    }
    this->currentEip = address;
    this->startingEip = address;

    initDynamicOps();
    DecodedOp* nextOp = op;

    if (!calculateLongestBlock(op)) {
        return;
    }
    if (!compileOps(op)) {
        return;
    }
    blockExit();
    commitJIT(op);
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    if (op->runCount == 0) {
#else
    // done check for long blocks that get broken up, affects f-22/f-16
    if (op->runCount == JIT_RUN_COUNT && op->inst != Done) {
#endif    
        startNewJIT(cpu, cpu->getEipAddress(), op);
    }
    op->runCount++;
    op->pfn(cpu, op);
}

#include "../../softmmu/kmemory_soft.h"

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

void DynamicCodeGen::storeLazyFlags(const LazyFlags* lazyFlags) {
    writeCPUValue(DYN_PTR, CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)lazyFlags);
}

bool DynamicCodeGen::isParamTypeReg(DynCallParamType paramType) {
    return paramType == DYN_PARAM_REG_8 || paramType == DYN_PARAM_REG_16 || paramType == DYN_PARAM_REG_32;
}

void DynamicCodeGen::xorCPUFlagsImm(U32 imm) {
    RegPtr reg = readCPU(DYN_32bit, CPU_OFFSET_OF(flags));
    xorValue(DYN_32bit, reg, imm, false);
    writeCPU(DYN_32bit, CPU_OFFSET_OF(flags), reg);
}

void DynamicCodeGen::andCPUFlagsImm(U32 imm) {
    RegPtr reg = readCPU(DYN_32bit, CPU_OFFSET_OF(flags));
    andValue(DYN_32bit, reg, imm, false);
    writeCPU(DYN_32bit, CPU_OFFSET_OF(flags), reg);
    if ((imm & DF) == 0) {
        writeCPUValue(DYN_32bit, CPU_OFFSET_OF(df), 1);
    }
}

void DynamicCodeGen::orCPUFlagsImm(U32 imm) {
    RegPtr reg = readCPU(DYN_32bit, CPU_OFFSET_OF(flags));
    orValue(DYN_32bit, reg, imm, false);
    writeCPU(DYN_32bit, CPU_OFFSET_OF(flags), reg);
    if (imm & DF) {
        writeCPUValue(DYN_32bit, CPU_OFFSET_OF(df), (U32)(S32)(-1));
    }
}

void DynamicCodeGen::orCPUFlagsReg(RegPtr src) {
    RegPtr reg = readCPU(DYN_32bit, CPU_OFFSET_OF(flags));
    orReg(DYN_32bit, reg, src, false);
    writeCPU(DYN_32bit, CPU_OFFSET_OF(flags), reg);
}

RegPtr DynamicCodeGen::getDF() {
    return readCPU(DYN_32bit, offsetof(CPU, df));
}

void DynamicCodeGen::incrementEip(U32 inc) {
    RegPtr reg = readCPU(DYN_32bit, CPU_OFFSET_OF(eip.u32));
    addValue(DYN_32bit, reg, inc, false);
    writeCPU(DYN_32bit, CPU_OFFSET_OF(eip.u32), reg);
}

void DynamicCodeGen::blockCall(DecodedOp* op) {
    blockNext1(op);
    if (lastOpEip > currentEip) {
        blockExit();
    }
}

void DynamicCodeGen::blockDoneCall() {
    blockDone(false);
}

void DynamicCodeGen::blockDone(bool returnEarly) {
    // cpu->nextOp = cpu->nextOp();
    jumpEip();
}

void DynamicCodeGen::jumpEip() {
    RegPtr target = getTmpReg();
    movValue(DYN_PTR, target, (DYN_PTR_SIZE)cpu->thread->process->jumpToNextJIT);
    jmp(target);
}

static DYN_PTR_SIZE getJitFunctionForCurrentOp(CPU* cpu) {
    DecodedOp* op = cpu->memory->getDecodedOp(cpu->getEipAddress());
    if (!op) {
        op = cpu->getNextOp();
    }
    cpu->nextOp = op;
    // runCount could be > JIT_RUN_COUNT with no jit code if it contains an instruction we don't support
    if (!op->pfnJitCode && op->runCount <= JIT_RUN_COUNT) {
        startNewJIT(cpu, cpu->getEipAddress(), op);
        op->runCount = JIT_RUN_COUNT + 1;
    }
    return (DYN_PTR_SIZE)op->pfnJitCode;
}

void DynamicCodeGen::blockDoneJump() {
    jumpToEipIfCached();        
    RegPtr result = callAndReturn(getJitFunctionForCurrentOp);
    IfNot(DYN_PTR, result);
        blockExit();
    EndIf();
    jmp(result);
}

static DYN_PTR_SIZE dynamic_getNextOp(CPU* cpu) {
    if (cpu->thread->terminating) {
        return 0;
    }
    return (DYN_PTR_SIZE)cpu->getNextOp();
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void DynamicCodeGen::blockNext1(DecodedOp* op) {
    // if (!(*(op->nextJump))) {
    //     *(op->nextJump) = cpu->getNextOp();
    // }
    // cpu->nextOp = *(op->nextJump);
    RegPtr opReg = getTmpReg();
    movValue(DYN_PTR, opReg, (DYN_PTR_SIZE)op);
    // ebx = op->nextJump
    // mov ebx, [edx + offsetof(DecodedOp, nextJump)]
    RegPtr nextJump = getTmpReg();
    read(DYN_PTR, nextJump, opReg, 0, offsetof(DecodedOp, data.nextJump));
    opReg = nullptr;

    // eax = *(op->nextJump)
    RegPtr nextOp = getTmpReg();
    read(DYN_PTR, nextOp, nextJump, 0, 0);
    // if (!(*(op->nextJump))) 
    IfNot(DYN_PTR, nextOp);
        // *(op->nextJump) = cpu->getNextOp();
        mov(DYN_PTR, nextOp, callAndReturn(dynamic_getNextOp));
        write(DYN_PTR, nextJump, offsetof(DecodedOp, next), nextOp);
    EndIf();

    // cpu->nextOp = *(op->nextJump);
    writeCPU(DYN_PTR, offsetof(CPU, nextOp), nextOp);

#ifdef BOXEDWINE_MULTI_THREADED
    RegPtr jit = getTmpReg();
    read(DYN_PTR, jit, nextOp, 0, offsetof(DecodedOp, pfnJitCode));
    If(DYN_PTR, jit);
        jmp(jit);
    EndIf();
#endif
    blockExit();
}

void DynamicCodeGen::blockNext2(DecodedOp* op) {
    // if (!op->next) { 
    //     op->next = cpu->getNextOp(); 
    // }
    // cpu->nextOp = op->next;
    
    RegPtr opReg = getTmpReg();
    movValue(DYN_PTR, opReg, (DYN_PTR_SIZE)op);

    // mov eax, [ebx + offsetof(DecodedOp, next)]
    RegPtr nextReg = getTmpReg();
    read(DYN_PTR, nextReg, opReg, 0, offsetof(DecodedOp, next));

    IfNot(DYN_PTR, nextReg);
        // op->next = cpu->getNextOp();
        mov(DYN_PTR, nextReg, callAndReturn(dynamic_getNextOp));
        // mov [ebx + offsetof(DecodedOp, next)], eax
        write(DYN_PTR, opReg, offsetof(DecodedOp, next), nextReg);
    EndIf();
    opReg = nullptr;

    // cpu->nextOp = op->next
    writeCPU(DYN_PTR, offsetof(CPU, nextOp), nextReg);

#ifdef BOXEDWINE_MULTI_THREADED
    RegPtr jit = getTmpReg();
    read(DYN_PTR, jit, nextReg, 0, offsetof(DecodedOp, pfnJitCode));
    If(DYN_PTR, jit);
        jmp(jit);
    EndIf();
#endif 
    blockExit();
}

void writed(U32 address, U32 value) {
    KThread::currentThread()->memory->writed(address, value);
}

void writew(U32 address, U16 value) {
    KThread::currentThread()->memory->writew(address, value);
}

void writeb(U32 address, U8 value) {
    KThread::currentThread()->memory->writeb(address, value);
}

U32 readd(U32 address) {
    return KThread::currentThread()->memory->readd(address);
}

U32 readw(U32 address) {
    return KThread::currentThread()->memory->readw(address);
}

U32 readb(U32 address) {
    return KThread::currentThread()->memory->readb(address);
}

U32 readd2(CPU* cpu, U32 address) {
    return cpu->memory->readd(address);
}

U32 readw2(CPU* cpu, U32 address) {
    return cpu->memory->readw(address);
}

U32 readb2(CPU* cpu, U32 address) {
    return cpu->memory->readb(address);
}

void writed2(CPU* cpu, U32 address, U32 value) {
    cpu->memory->writed(address, value);
}

void writew2(CPU* cpu, U32 address, U32 value) {
    cpu->memory->writew(address, (U16)value);
}

void writeb2(CPU* cpu, U32 address, U32 value) {
    cpu->memory->writeb(address, (U8)value);
}

RegPtr DynamicCodeGen::calculateEaa(DecodedOp* op, U32 popEspAmount) {
    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        RegPtr result = getTmpReg();

        xorReg(DYN_32bit, result, result, false); // clear top bits

        if (popEspAmount && op->rm != 4 && op->sibIndex != 4) {
            popEspAmount = 0;
        } else if (popEspAmount && op->rm == 4 && op->sibIndex == 4) {
            popEspAmount *= 2;
        } else if (popEspAmount) {
            kpanic("found sp pop");
        }
        if (op->data.disp) {
            movValue(DYN_16bit, result, op->data.disp + popEspAmount);
        } else if (popEspAmount) {
            movValue(DYN_16bit, result, popEspAmount);
        }
        if (op->rm == op->sibIndex && op->rm != 8) {
            RegPtr reg = getReadOnlyReg(op->rm);
            addReg(DYN_16bit, result, reg, false);
            addReg(DYN_16bit, result, reg, false);
        } else {
            if (op->rm != 8) {
                addReg(DYN_16bit, result, getReadOnlyReg(op->rm), false);
            }
            if (op->sibIndex != 8) {
                addReg(DYN_16bit, result, getReadOnlyReg(op->sibIndex), false);
            }
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            addReg(DYN_32bit, result, getReadOnlySegAddress(op->base), false);
        }
        return result;
    } else {
        RegPtr result;

        if (op->sibIndex != 8) {
            result = getTmpReg(op->sibIndex);
            if (op->sibScale) {
                shlValue(DYN_32bit, result, op->sibScale, false);
            }

            if (op->rm != 8) {
                addReg(DYN_32bit, result, getReadOnlyReg(op->rm), false);
            }
            if (op->data.disp) {
                addValue(DYN_32bit, result, op->data.disp, false);
            }
        } else if (op->rm != 8) {
            result = getTmpReg(op->rm);
            if (op->data.disp) {
                addValue(DYN_32bit, result, op->data.disp, false);
            }
        } else if (op->data.disp) {
            result = getTmpReg();
            movValue(DYN_32bit, result, op->data.disp);
        } else {
            result = getTmpReg();
            xorReg(DYN_32bit, result, result, false);
        }

        // seg[6] is always 0
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            addReg(DYN_32bit, result, getReadOnlySegAddress(op->base), false);
        }
        return result;
    }
}

RegPtr DynamicCodeGen::read(DynWidth width, RegPtr addressReg, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp, std::function<void()> failedMemoryOp, bool isBigJump, RegPtr tmp) {
    if (!tmp) {
        tmp = getTmpRegForCallResult();
    }
    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        mov(DYN_32bit, tmp, addressReg);
        andValue(DYN_32bit, tmp, K_PAGE_MASK, false);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan2(DYN_32bit, tmp, K_PAGE_MASK);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan2(DYN_32bit, tmp, 0xFFD);
        } else if (width == DYN_64bit) {
            // if ((address & 0xFFF) < 0xFF9)
            IfLessThan2(DYN_32bit, tmp, 0xFF9);
        } else if (width == DYN_128bit) {
            // if ((address & 0xFFF) < 0xFF1)
            IfLessThan2(DYN_32bit, tmp, 0xFF1);
        } else if (width == DYN_256bit) {
            // if ((address & 0xFFF) < 0xFE1)
            IfLessThan2(DYN_32bit, tmp, 0xFE1);
        } else {
            kpanic_fmt("DynamicCodeGen::read unknown width %d", (U32)width);
        }
    }
    // int index = address >> 12;
    // if (Memory::currentMMUReadPtr[index])
    //     return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
    // else
    //     return readd(address);

    mov(DYN_32bit, tmp, addressReg);
    shrValue(DYN_32bit, tmp, K_PAGE_SHIFT, false);
    read(DYN_32bit, tmp, tmp, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test eax, 0x40000000 (mmu[index].canReadRam)
    IfBitSet2(DYN_32bit, tmp, 0x40000000, isBigJump);

    // bottom 20 bits of mmu contains ram page index
    andValue(DYN_32bit, tmp, 0xfffff, false);
    read(DYN_32bit, tmp, tmp, 2, (U32)ramPages);

    RegPtr offsetReg;
    
    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
    } else {
        offsetReg = getTmpReg();
        mov(DYN_32bit, offsetReg, addressReg);
    }
    andValue(DYN_32bit, offsetReg, K_PAGE_MASK, false);

    if (customMemoryOp) {
        // regs are not used after this, so give them back
        addressReg = nullptr;
        customMemoryOp(std::move(tmp), std::move(offsetReg));
    } else {
        // mov eax, [eax+reg]
        read(width, tmp, tmp, offsetReg, 0, 0);
    }
    StartElse(isBigJump);

    if (failedMemoryOp) {
        failedMemoryOp();
    } else {
        DynamicData::CallReturnR address;

        // call read
        if (width == DYN_32bit) {
            address = readd2;
        } else if (width == DYN_16bit) {
            address = readw2;
        } else if (width == DYN_8bit) {
            address = readb2;
        } else {
            kpanic_fmt("unknown width in x32CPU::movFromMem %d", width);
        }
        callAndReturn_R(address, DYN_32bit, addressReg, tmp);
    }
    EndIf(isBigJump);
    return tmp;
}

void DynamicCodeGen::write(DynWidth width, RegPtr addressReg, RegPtr src, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp, std::function<void()> failedMemoryOp, bool isBigJump) {
    RegPtr tmp = getTmpReg();

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        mov(DYN_32bit, tmp, addressReg);
        andValue(DYN_32bit, tmp, K_PAGE_MASK, false);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan2(DYN_32bit, tmp, K_PAGE_MASK);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan2(DYN_32bit, tmp, 0xFFD);
        } else if (width == DYN_64bit) {
            // if ((address & 0xFFF) < 0xFF9)
            IfLessThan2(DYN_32bit, tmp, 0xFF9);
        } else if (width == DYN_128bit) {
            // if ((address & 0xFFF) < 0xFF1)
            IfLessThan2(DYN_32bit, tmp, 0xFF1);
        } else if (width == DYN_256bit) {
            // if ((address & 0xFFF) < 0xFE1)
            IfLessThan2(DYN_32bit, tmp, 0xFE1);
        } else {
            kpanic_fmt("DynamicCodeGen::write unknown width %d", (U32)width);
        }
    }
    mov(DYN_32bit, tmp, addressReg);
    shrValue(DYN_32bit, tmp, K_PAGE_SHIFT, false);
    read(DYN_32bit, tmp, tmp, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    IfBitSet2(DYN_8bit, tmp, 0x80000000, isBigJump);

    // bottom 20 bits of mmu contains ram page index
    andValue(DYN_32bit, tmp, 0xfffff, false);
    read(DYN_32bit, tmp, tmp, 2, (U32)ramPages);

    RegPtr offsetReg;
    bool pushedAddress = false;

    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
    } else if (isTmpRegAvailable()) {
        offsetReg = getTmpReg();
        mov(DYN_32bit, offsetReg, addressReg);
    } else {
        pushedAddress = true;
        pushReg(addressReg);
        offsetReg = addressReg;
    }
    andValue(DYN_32bit, offsetReg, K_PAGE_MASK, false);

    if (customMemoryOp) {
        customMemoryOp(std::move(tmp), std::move(offsetReg));
    } else {
        write(width, tmp, offsetReg, 0, 0, src);
    }    
    if (pushedAddress) {
        popReg(addressReg);
    }
    StartElse(isBigJump);

    if (failedMemoryOp) {
        offsetReg = nullptr;
        addressReg = nullptr;
        tmp = nullptr;
        failedMemoryOp();
    } else {
        DynamicData::CallRR address;

        if (width == DYN_32bit) {
            address = writed2;
        } else if (width == DYN_16bit) {
            address = writew2;
        } else if (width == DYN_8bit) {
            address = writeb2;
        } else {
            kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
        }
        call_RR(address, DYN_32bit, addressReg, width, src);
    }
    EndIf(isBigJump);
}

void DynamicCodeGen::writeValue(DynWidth width, RegPtr addressReg, U32 imm) {
    RegPtr tmp = getTmpReg();

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        mov(DYN_32bit, tmp, addressReg);
        andValue(DYN_32bit, tmp, K_PAGE_MASK, false);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan2(DYN_32bit, tmp, K_PAGE_MASK);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan2(DYN_32bit, tmp, 0xFFD);
        } else if (width == DYN_64bit) {
            // if ((address & 0xFFF) < 0xFF9)
            IfLessThan2(DYN_32bit, tmp, 0xFF9);
        } else if (width == DYN_128bit) {
            // if ((address & 0xFFF) < 0xFF1)
            IfLessThan2(DYN_32bit, tmp, 0xFF1);
        } else {
            kpanic_fmt("DynamicCodeGen::write unknown width %d", (U32)width);
        }
    }
    mov(DYN_32bit, tmp, addressReg);
    shrValue(DYN_32bit, tmp, K_PAGE_SHIFT, false);
    read(DYN_32bit, tmp, tmp, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    IfBitSet2(DYN_8bit, tmp, 0x80000000);

    // bottom 20 bits of mmu contains ram page index
    andValue(DYN_32bit, tmp, 0xfffff, false);
    read(DYN_32bit, tmp, tmp, 2, (U32)ramPages);

    andValue(DYN_32bit, addressReg, K_PAGE_MASK, false);

    write(width, tmp, addressReg, 0, 0, imm);

    StartElse();

        CallRI address;

        if (width == DYN_32bit) {
            address = writed2;
        } else if (width == DYN_16bit) {
            address = writew2;
        } else if (width == DYN_8bit) {
            address = writeb2;
        } else {
            kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
        }
        call_RI(address, DYN_32bit, addressReg, imm);
    
    EndIf();
}

void DynamicCodeGen::readWriteMem(DynWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint) {
    U32 firstCheckPos = 0;

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        RegPtr tmpReg = getTmpReg();

        mov(DYN_32bit, tmpReg, addressReg);
        andValue(DYN_32bit, tmpReg, K_PAGE_MASK, false);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan2(DYN_32bit, tmpReg, K_PAGE_MASK); // :TODO: V2
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan2(DYN_32bit, tmpReg, 0xFFD); // :TODO: V2
        } else {
            kpanic_fmt("DynamicCodeGen::readWriteMem unknown width %d", (U32)width);
        }
    }
    RegPtr tmpReg = getTmpRegWithHint(hint);
    mov(DYN_32bit, tmpReg, addressReg);

    shrValue(DYN_32bit, tmpReg, K_PAGE_SHIFT, false);
    read(DYN_32bit, tmpReg, tmpReg, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // if read/write
    RegPtr tmpReg2 = getTmpRegForCallResult();
    mov(DYN_32bit, tmpReg2, tmpReg);
    andValue(DYN_32bit, tmpReg2, 0x40000000 | 0x80000000, false);

    IfEqual(DYN_32bit, tmpReg2, 0x40000000 | 0x80000000);

        // bottom 20 bits of mmu contains ram page index
        andValue(DYN_32bit, tmpReg, 0xfffff, false);
        read(DYN_32bit, tmpReg, tmpReg, 2, (U32)ramPages);
        andValue(DYN_32bit, addressReg, K_PAGE_MASK, false);
        read(DYN_32bit, tmpReg2, tmpReg, addressReg, 0, 0);

    StartElse();

        DynamicData::CallReturnR addressRead;
        // call read
        if (width == DYN_32bit) {
            addressRead = readd2;
        } else if (width == DYN_16bit) {
            addressRead = readw2;
        } else if (width == DYN_8bit) {
            addressRead = readb2;
        } else {
            kpanic("DynamicCodeGen::readWriteMem");
        }
        callAndReturn_R(addressRead, DYN_32bit, addressReg, tmpReg2);
        xorReg(DYN_32bit, tmpReg, tmpReg, false); // so that the next if statement will also choose calling write

    EndIf();

    prepareWrite(tmpReg2);

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    If(DYN_32bit, tmpReg);

        write(DYN_32bit, tmpReg, addressReg, 0, 0, tmpReg2);

    StartElse();
        DynamicData::CallRR address;

        if (width == DYN_32bit) {
            address = writed2;
        } else if (width == DYN_16bit) {
            address = writew2;
        } else if (width == DYN_8bit) {
            address = writeb2;
        }
        call_RR(address, DYN_32bit, addressReg, DYN_32bit, tmpReg2);

    EndIf();
}

U8* DynamicCodeGen::createDynamicExecutableMemory() {
    U8* begin = (U8*)cpu->memory->allocCodeMemory(getBufferSize());

    Platform::writeCodeToMemory(begin, getBufferSize(), [begin, this]() {
        memcpy(begin, this->getBuffer(), this->getBufferSize());
        });
    return begin;
}

void DynamicCodeGen::jumpToEipIfCached() {
    // U32 pageIndex = address >> K_PAGE_SHIFT;
    // DecodedOpPageCache* page = getPageCache(pageIndex, false);
    // if (page) {
    //     U32 offset = address & K_PAGE_MASK;
    //     if (page->ops[offset] == nullptr) {
    //         if (offset > 0 && page->ops[offset - 1] && page->ops[offset - 1]->lock) {
    //             return page->ops[offset - 1];
    //         }
    //     }
    //     return page->ops[offset];
    // }
    RegPtr eipReg = getTmpEip();
    if (cpu->thread->process->hasSetSeg[CS]) {
        addReg(DYN_32bit, eipReg, getReadOnlySegAddress(CS), false);
    }
    RegPtr pageReg = getTmpReg();
    mov(DYN_32bit, pageReg, eipReg);
    shrValue(DYN_32bit, pageReg, K_PAGE_SHIFT, false);

    RegPtr firstPageIndexReg = getTmpReg();
    mov(DYN_32bit, firstPageIndexReg, pageReg);
    shrValue(DYN_32bit, firstPageIndexReg, 10, false);

    RegPtr tmp = readCPU(DYN_PTR, offsetof(CPU, opCache));
    read(DYN_PTR, tmp, tmp, firstPageIndexReg, 2, 0); // :TODO: 3 on 64-bit system


    // tmp contains 2nd level of page op cache
    If(DYN_PTR, tmp);
        // page & 0x3ff
        andValue(DYN_32bit, pageReg, 0x3ff, false);
        read(DYN_PTR, tmp, tmp, pageReg, 2, 0);// :TODO: 3 on 64-bit system
        // tmp contains page of DecodedOp*
        If(DYN_PTR, tmp);
            andValue(DYN_32bit, eipReg, K_PAGE_MASK, false);
            read(DYN_PTR, tmp, tmp, eipReg, 2, 0); // :TODO: 3 on 64-bit system
            // tmp contains DecodedOp
            If(DYN_PTR, tmp);
                read(DYN_PTR, tmp, tmp, 0, offsetof(DecodedOp, pfnJitCode));
                // tmp contains pfnJitCode
                If(DYN_PTR, tmp);
                    jmp(tmp);
                EndIf();
            EndIf();
        EndIf();
    EndIf();
 }

U8* DynamicCodeGen::createJumpEip() {
    jumpToEipIfCached();
    writeCPU(DYN_PTR, offsetof(CPU, nextOp), callAndReturn(dynamic_getNextOp));
    blockExit();

    U8* result = createDynamicExecutableMemory();
    patch(result);
    return result;
}

void DynamicCodeGen::commitJIT(DecodedOp* op) {
    preCommitJIT();

    if (!blockOpCount) {
        return;
    }

    U8* begin = createDynamicExecutableMemory();

    patch(begin);

    removeJIT(op, blockOpCount);
    op->blockLen = emulatedLen;
    op->blockOpCount = blockOpCount;
#ifdef _DEBUG
    //logBlock(cpu, startingEip, op, op->blockLen);
#endif 
    U32 address = startingEip;
    DecodedOp* nextOp = op;
    DecodedOp* last = op;
#if defined (_DEBUG) && !defined (__TEST)
    BOXEDWINE_CRITICAL_SECTION;
    static int totalBlocks;
    totalBlocks++;
    static int totalOps;
    totalOps += blockOpCount;
    if ((totalBlocks % 1000) == 0) {
        klog_fmt("Compiled Blocks: %d, ave block size: %d ops", totalBlocks, totalOps / totalBlocks);
    }
#endif
    for (U32 i = 0; i < blockOpCount; i++) {
        U32 bufferIndex = 0;

        if (!eipToBufferPos.get(address, bufferIndex)) {
            kpanic("x32CPU commitJIT 2");
        }
        nextOp->pfnJitCode = (OpCallback)(begin + bufferIndex);
        nextOp->pfn = cpu->thread->process->startJITOp;
        nextOp->flags |= OP_FLAG_JIT;
        nextOp->blockStart = op;
        address += nextOp->len;
        last = nextOp;
        nextOp = nextOp->next;
    }
}

#endif