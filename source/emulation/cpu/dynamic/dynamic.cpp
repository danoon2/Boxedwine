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
    movToCpu(offsetof(CPU, nextOp), DYN_32bit, (DYN_PTR_SIZE)op);
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
    // find longest block where all direction jumps don't go past the block
    U32 lastFurthestEip = eip;
    while (true) {
        nextOp = op;
        this->lastOpEip = this->startingEip;
        while (nextOp && this->lastOpEip < lastFurthestEip) {
            if (nextOp->isDirectJumpBranch()) {
                U32 target = this->lastOpEip + nextOp->len + nextOp->imm;
                if (target >= lastFurthestEip || target < this->startingEip) {
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

U32 DynamicCodeGen::cpuOffsetResult(DynWidth width) {
    if (width == DYN_8bit)
        return CPU_OFFSET_OF(result.u8);
    else if (width == DYN_16bit)
        return CPU_OFFSET_OF(result.u16);
    else if (width == DYN_32bit)
        return CPU_OFFSET_OF(result.u32);
    else {
        kpanic_fmt("dynamic cpuOffsetResult unexpected width: %d", width);
        return 0;
    }
}

U32 DynamicCodeGen::cpuOffsetDst(DynWidth width) {
    if (width == DYN_8bit)
        return CPU_OFFSET_OF(dst.u8);
    else if (width == DYN_16bit)
        return CPU_OFFSET_OF(dst.u16);
    else if (width == DYN_32bit)
        return CPU_OFFSET_OF(dst.u32);
    else {
        kpanic_fmt("dynamic cpuOffsetDst unexpected width: %d", width);
        return 0;
    }
}

U32 DynamicCodeGen::cpuOffsetSrc(DynWidth width) {
    if (width == DYN_8bit)
        return CPU_OFFSET_OF(src.u8);
    else if (width == DYN_16bit)
        return CPU_OFFSET_OF(src.u16);
    else if (width == DYN_32bit)
        return CPU_OFFSET_OF(src.u32);
    else {
        kpanic_fmt("dynamic cpuOffsetSrc unexpected width: %d", width);
        return 0;
    }
}

void DynamicCodeGen::loadRegStoreReg(U8 dst, U8 src, DynWidth width, DynReg tmpReg) {
    loadReg(src, tmpReg, width);
    storeReg(dst, tmpReg, width, true);
}

void DynamicCodeGen::loadRegStoreSrc(U8 src, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    loadReg(src, tmpReg, width);
    movToCpuFromReg(cpuOffsetSrc(width), tmpReg, width, doneWithTmpReg);
}

void DynamicCodeGen::loadRegStoreDst(U8 src, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    loadReg(src, tmpReg, width);
    movToCpuFromReg(cpuOffsetDst(width), tmpReg, width, doneWithTmpReg);
}

void DynamicCodeGen::loadRegStoreEip(U8 src, DynReg tmpReg) {
    loadReg(src, tmpReg, DYN_32bit);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::loadSegValueStoreReg(U8 reg, U8 seg, DynReg tmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(tmpReg, CPU::offsetofSegValue(seg), DYN_32bit);
    storeReg(reg, tmpReg, DYN_32bit, true);
}

// all reg read/write access should go through loadReg/storeReg, no one else should index a reg in the CPU
U32 dontUseCpuOffset(U32 r, DynWidth width) {
    if (width == DYN_8bit)
        return CPU::offsetofReg8(r);
    else if (width == DYN_16bit)
        return CPU::offsetofReg16(r);
    else if (width == DYN_32bit)
        return CPU::offsetofReg32(r);
    else {
        kpanic_fmt("dynamic cpuOffset unexpected width: %d", width);
        return 0;
    }
}

void DynamicCodeGen::loadReg(U8 reg, DynReg dstReg, DynWidth width) {
    movToRegFromCpu(dstReg, dontUseCpuOffset(reg, width), width);
}

void DynamicCodeGen::storeReg(U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(dontUseCpuOffset(reg, width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeReg(U8 reg, DynWidth dstWidth, U32 imm) {
    movToCpu(dontUseCpuOffset(reg, dstWidth), dstWidth, imm);
}

void DynamicCodeGen::storeRegFromMem(U8 reg, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    storeReg(reg, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

void DynamicCodeGen::loadSegAddress(U8 seg, DynReg reg) {
    movToRegFromCpu(reg, CPU::offsetofSegAddress(seg), DYN_32bit);
}

void DynamicCodeGen::loadSegValue(U8 seg, DynReg reg) {
    movToRegFromCpu(reg, CPU::offsetofSegValue(seg), DYN_32bit);
}

void DynamicCodeGen::loadCPUFlags(DynReg reg) {
    movToRegFromCpu(reg, CPU_OFFSET_OF(flags), DYN_32bit);
}

void DynamicCodeGen::loadLazyFlagsResult(DynReg reg, DynWidth width) {
    movToRegFromCpu(reg, cpuOffsetResult(width), width);
}

void DynamicCodeGen::loadLazyFlagsSrc(DynReg reg, DynWidth width) {
    movToRegFromCpu(reg, cpuOffsetSrc(width), width);
}

void DynamicCodeGen::loadLazyFlagsOldCF(DynReg reg) {
    movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
}

void DynamicCodeGen::loadEip(DynReg reg) {
    movToRegFromCpu(reg, CPU_OFFSET_OF(eip.u32), DYN_32bit);
}

void DynamicCodeGen::loadStackMask(DynReg reg) {
    movToRegFromCpu(reg, CPU_OFFSET_OF(stackMask), DYN_32bit);
}

void DynamicCodeGen::loadStackNotMask(DynReg reg) {
    movToRegFromCpu(reg, CPU_OFFSET_OF(stackNotMask), DYN_32bit);
}

void DynamicCodeGen::loadLazyFlags(DynReg reg) {
    movToRegFromCpu(reg, offsetof(CPU, lazyFlags), DYN_32bit);
}

void DynamicCodeGen::loadLazyFlagsDst(DynReg reg, DynWidth width) {
    movToRegFromCpu(reg, cpuOffsetDst(width), width);
}

void DynamicCodeGen::storeLazyFlagsResult(DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(cpuOffsetResult(width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsDst(DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(cpuOffsetDst(width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsSrc(DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(cpuOffsetSrc(width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsOldCF(DynReg srcReg, bool doneWithSrcReg) {
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), srcReg, DYN_32bit, doneWithSrcReg);
}

void DynamicCodeGen::storeEip(DynReg srcReg, bool doneWithSrcReg) {
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), srcReg, DYN_32bit, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsSrc(DynWidth width, U32 imm) {
    movToCpu(cpuOffsetSrc(width), width, imm);
}

void DynamicCodeGen::storeLazyFlags(const LazyFlags* lazyFlags) {
    movToCpu(CPU_OFFSET_OF(lazyFlags), DYN_32bit, (U32)lazyFlags);
}

void DynamicCodeGen::movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+dstOffset], eax
    movToCpuFromReg(dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

void DynamicCodeGen::storeLazyFlagsDstFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movToCpuFromMem(cpuOffsetDst(width), width, addressReg, doneWithAddressReg, doneWithCallResult);
}

void DynamicCodeGen::storeLazyFlagsSrcFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movToCpuFromMem(cpuOffsetSrc(width), width, addressReg, doneWithAddressReg, doneWithCallResult);
}

bool DynamicCodeGen::isParamTypeReg(DynCallParamType paramType) {
    return paramType == DYN_PARAM_REG_8 || paramType == DYN_PARAM_REG_16 || paramType == DYN_PARAM_REG_32;
}

void DynamicCodeGen::movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg) {
    DynCallParamType paramType;

    if (width == DYN_8bit)
        paramType = DYN_PARAM_REG_8;
    else if (width == DYN_16bit)
        paramType = DYN_PARAM_REG_16;
    else if (width == DYN_32bit)
        paramType = DYN_PARAM_REG_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromReg", width);

    movToMem(addressReg, width, reg, paramType, doneWithReg, doneWithAddressReg, tmpReg);
}

void DynamicCodeGen::movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    DynCallParamType paramType;

    if (width == DYN_8bit)
        paramType = DYN_PARAM_CONST_8;
    else if (width == DYN_16bit)
        paramType = DYN_PARAM_CONST_16;
    else if (width == DYN_32bit)
        paramType = DYN_PARAM_CONST_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromImm", width);

    movToMem(addressReg, width, imm, paramType, false, doneWithAddressReg, tmpReg);
}

void DynamicCodeGen::instCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg, InstRegReg pfnInstRegReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPUReg");
    }
    loadReg(regIndex, tmpReg, regWidth);
    (this->*pfnInstRegReg)(tmpReg, rm, regWidth, doneWithRmReg);
    storeReg(regIndex, tmpReg, regWidth, true);
}

void DynamicCodeGen::addCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::addRegReg);
}

void DynamicCodeGen::orCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::orRegReg);
}

void DynamicCodeGen::subCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::subRegReg);
}

void DynamicCodeGen::andCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::andRegReg);
}

void DynamicCodeGen::xorCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::xorRegReg);
}

void DynamicCodeGen::shrCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::shrRegReg);
}

void DynamicCodeGen::sarCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::sarRegReg);
}

void DynamicCodeGen::shlCPUReg(U8 regIndex, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    instCPUReg(regIndex, rm, regWidth, doneWithRmReg, tmpReg, &DynamicCodeGen::shlRegReg);
}

void DynamicCodeGen::instCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg, InstRegImm pfnInstRegImm) {
    if (regUsed[tmpReg]) {
        kpanic("instCPUImm");
    }
    loadReg(regIndex, tmpReg, regWidth);
    (this->*pfnInstRegImm)(tmpReg, regWidth, imm);
    storeReg(regIndex, tmpReg, regWidth, true);
}

void DynamicCodeGen::addCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::addRegImm);
}

void DynamicCodeGen::orCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::orRegImm);
}

void DynamicCodeGen::subCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::subRegImm);
}

void DynamicCodeGen::andCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::andRegImm);
}

void DynamicCodeGen::xorCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::xorRegImm);
}

void DynamicCodeGen::shrCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::shrRegImm);
}

void DynamicCodeGen::sarCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::sarRegImm);
}

void DynamicCodeGen::shlCPUImm(U8 regIndex, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    instCPUImm(regIndex, regWidth, imm, tmpReg, &DynamicCodeGen::shlRegImm);
}

void DynamicCodeGen::xorCPUFlagsImm(U32 imm, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("DynamicCodeGen::xorCPUFlagsImm");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    xorRegImm(tmpReg, DYN_32bit, imm);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::andCPUFlagsImm(U32 imm, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("DynamicCodeGen::andCPUFlagsImm");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    andRegImm(tmpReg, DYN_32bit, imm);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::orCPUFlagsImm(U32 imm, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("DynamicCodeGen::orCPUFlagsImm");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    orRegImm(tmpReg, DYN_32bit, imm);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::setCPUFlags(DynReg reg, U32 mask, DynReg tmpReg, bool doneWithReg) {
    if (regUsed[tmpReg]) {
        kpanic("DynamicCodeGen::setCPUFlags");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    andRegImm(tmpReg, DYN_32bit, ~mask);
    orRegReg(tmpReg, reg, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::orCPUFlagsReg(DynReg reg, DynReg tmpReg, bool doneWithReg) {
    if (regUsed[tmpReg]) {
        kpanic("DynamicCodeGen::orCPUFlagsReg");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    orRegReg(tmpReg, reg, DYN_32bit, doneWithReg);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::instMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg, InstRegImm pfnInstRegImm) {
    if (regUsed[0]) {
        kpanic("DynamicCodeGen::instMemImm");
    }
    movFromMem(regWidth, addressReg, false);
    readWriteMem(regWidth, addressReg, tmpReg, doneWithAddressReg, [regWidth, imm, pfnInstRegImm, this]() {
        (this->*pfnInstRegImm)(DYN_CALL_RESULT, regWidth, imm);
    });
}

void DynamicCodeGen::addMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::addRegImm);
}

void DynamicCodeGen::orMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::orRegImm);
}

void DynamicCodeGen::subMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::subRegImm);
}

void DynamicCodeGen::andMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::andRegImm);
}

void DynamicCodeGen::xorMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::xorRegImm);
}

void DynamicCodeGen::shrMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::shrRegImm);
}

void DynamicCodeGen::sarMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::sarRegImm);
}

void DynamicCodeGen::shlMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    instMemImm(addressReg, regWidth, imm, doneWithAddressReg, tmpReg, &DynamicCodeGen::shlRegImm);
}

void DynamicCodeGen::instMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg, InstRegReg pfnInstRegReg) {
    if (regUsed[0]) {
        kpanic("DynamicCodeGen::instMemReg");
    }
    readWriteMem(regWidth, addressReg, tmpReg, doneWithAddressReg, [pfnInstRegReg, this, rm, regWidth, doneWithRmReg]() {
        (this->*pfnInstRegReg)(DYN_CALL_RESULT, rm, regWidth, doneWithRmReg);
    });
}

void DynamicCodeGen::addMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::addRegReg);
}

void DynamicCodeGen::orMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::orRegReg);
}

void DynamicCodeGen::subMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::subRegReg);
}

void DynamicCodeGen::andMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::andRegReg);
}

void DynamicCodeGen::xorMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::xorRegReg);
}

void DynamicCodeGen::shrMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::shrRegReg);
}

void DynamicCodeGen::sarMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::sarRegReg);
}

void DynamicCodeGen::shlMemReg(DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    instMemReg(addressReg, rm, regWidth, doneWithAddressReg, doneWithRmReg, tmpReg, &DynamicCodeGen::shlRegReg);
}

void DynamicCodeGen::negMem(DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg) {
    if (regUsed[0]) {
        kpanic("x32CPU::instMem");
    }
    movFromMem(regWidth, addressReg, false);
    negReg(DYN_CALL_RESULT, regWidth);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true, tmpReg);
}

void DynamicCodeGen::notMem(DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg) {
    if (regUsed[0]) {
        kpanic("x32CPU::instMem");
    }
    movFromMem(regWidth, addressReg, false);
    notReg(DYN_CALL_RESULT, regWidth);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true, tmpReg);
}

void DynamicCodeGen::negCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPU");
    }
    loadReg(regIndex, tmpReg, regWidth);
    negReg(tmpReg, regWidth);
    storeReg(regIndex, tmpReg, regWidth, true);
}

void DynamicCodeGen::notCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPU");
    }
    loadReg(regIndex, tmpReg, regWidth);
    notReg(tmpReg, regWidth);
    storeReg(regIndex, tmpReg, regWidth, true);
}

// this is good generic code to copy for other implementations that don't have something like setcc
/*
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpu(offset, regWidth, 0);
    startElse();
    movToCpu(offset, regWidth, 1);
    EndIf();
}
*/

void DynamicCodeGen::setCPUReg(U8 regIndex, DynWidth regWidth, DynConditional condition) {
    setConditional(condition);

    if (regWidth != DYN_8bit) {
        movToRegFromReg(DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    storeReg(regIndex, DYN_CALL_RESULT, regWidth, true);
}

/*
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToReg(DYN_SRC, regWidth, 0);
    startElse();
    movToReg(DYN_SRC, regWidth, 1);
    EndIf();
    // don't put this movToMem in the if statement because it is big and will be inlines once in each block of the if statement
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, regWidth, doneWithAddressReg, true);
}
*/
void DynamicCodeGen::setMem(DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg) {
    setConditional(condition);

    if (regWidth != DYN_8bit) {
        movToRegFromReg(DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true, tmpReg);
}

void DynamicCodeGen::incrementEip(U32 inc) {
    if (regUsed[DYN_SRC]) {
        kpanic("incrementEip");
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    addRegImm(DYN_SRC, DYN_32bit, inc);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_SRC, DYN_32bit, true);
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
    movToReg(DYN_CALL_RESULT, DYN_PTR, (DYN_PTR_SIZE)cpu->thread->process->jumpToNextJIT);
    jmp(DYN_CALL_RESULT);
}

static OpCallback getJitFunctionForCurrentOp(CPU* cpu) {
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
    return op->pfnJitCode;
}

void DynamicCodeGen::blockDoneJump() {
    jumpToEipIfCached();        
    callHostFunction(getJitFunctionForCurrentOp, true, 1, 0, DYN_PARAM_CPU, false);
    IfNot(DYN_CALL_RESULT, true);
        blockExit();
    EndIf();
    jmp(DYN_CALL_RESULT);
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void DynamicCodeGen::blockNext1(DecodedOp* op) {
    // if (!(*(op->nextJump))) {
    //     *(op->nextJump) = cpu->getNextOp();
    // }
    // cpu->nextOp = *(op->nextJump);
    movToReg(DYN_DEST, DYN_32bit, (U32)op);
    // ebx = op->nextJump
    // mov ebx, [edx + offsetof(DecodedOp, nextJump)]
    regUsed[DYN_ADDRESS] = true;
    readMem(DYN_ADDRESS, DYN_32bit, DYN_DEST, 0, offsetof(DecodedOp, data.nextJump));
    regUsed[DYN_DEST] = false;

    // eax = *(op->nextJump)
    readMem(DYN_CALL_RESULT, DYN_32bit, DYN_ADDRESS, 0, 0);
    // if (!(*(op->nextJump))) 
    IfNot(DYN_CALL_RESULT, false);
    // *(op->nextJump) = cpu->getNextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    writeMem(DYN_CALL_RESULT, DYN_32bit, DYN_ADDRESS, 0);
    EndIf();

    // cpu->nextOp = *(op->nextJump);        
    regUsed[DYN_ADDRESS] = false;
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, false);

#ifdef BOXEDWINE_MULTI_THREADED
    readMem(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, 0, offsetof(DecodedOp, pfnJitCode));
    If(DYN_CALL_RESULT, true);
    jmp(DYN_CALL_RESULT);
    EndIf();
#endif
}

void DynamicCodeGen::blockNext2(DecodedOp* op) {
    // if (!op->next) { 
    //     op->next = cpu->getNextOp(); 
    // }
    // cpu->nextOp = op->next;    
    movToReg(DYN_ADDRESS, DYN_32bit, (U32)op);

    // mov eax, [ebx + offsetof(DecodedOp, next)]
    readMem(DYN_CALL_RESULT, DYN_32bit, DYN_ADDRESS, 0, offsetof(DecodedOp, next));

    IfNot(DYN_CALL_RESULT, false);
    // op->next = cpu->getNextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    // mov [ebx + offsetof(DecodedOp, next)], eax
    writeMem(DYN_CALL_RESULT, DYN_32bit, DYN_ADDRESS, offsetof(DecodedOp, next));
    EndIf();
    regUsed[DYN_ADDRESS] = false;

    // cpu->nextOp = op->next
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, false);

#ifdef BOXEDWINE_MULTI_THREADED
    readMem(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, 0, offsetof(DecodedOp, pfnJitCode));
    If(DYN_CALL_RESULT, true);
    jmp(DYN_CALL_RESULT);
    EndIf();
#endif 
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

// will inline the following code snippet (this code is for 32-bit, but it will do a similiar thing for 16-bit and 8-bit)
//
// if ((address & 0xFFF) < 0xFFD) {
//      int index = address >> 12;
//      if (Memory::currentMMUWritePtr[index])
//          *(U32*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
//      else
//          Memory::currentMMU[index]->writed(address, value);		
//  } else {
//      Memory::currentMMU[index]->writed(address, value);		
//  }
void DynamicCodeGen::movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp, std::function<void(DynReg address, DynReg offset)> customMemoryOp, std::function<void()> failedMemoryOp, bool bigJump) {
    U32 firstCheckPos = 0;

    if (regUsed[tmp]) {
        kpanic("movToMem");
    }

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        movToRegFromReg(tmp, DYN_32bit, addressReg, DYN_32bit, false);
        and32(tmp, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(tmp, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(tmp, 0xFFD, false);
        } else if (width == DYN_64bit) {
            // if ((address & 0xFFF) < 0xFF9)
            IfLessThan(tmp, 0xFF9, false);
        } else if (width == DYN_128bit) {
            // if ((address & 0xFFF) < 0xFF1)
            IfLessThan(tmp, 0xFF1, false);
        } else {
            kpanic_fmt("DynamicCodeGen::movToMem unknown width %d", (U32)width);
        }
    }
    movToRegFromReg(tmp, DYN_32bit, addressReg, DYN_32bit, false);
    shr32(tmp, K_PAGE_SHIFT);
    readMem(tmp, DYN_32bit, tmp, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    IfBitSet(tmp, 0x80000000, false, bigJump);

    // bottom 20 bits of mmu contains ram page index
    and32(tmp, 0xfffff);
    readMem(tmp, DYN_32bit, tmp, 2, (U32)ramPages);

    if (!doneWithAddressReg) {
        kpanic("DynamicCodeGen::movToMem ran out of regs");
    }
    and32(addressReg, K_PAGE_MASK);

    if (customMemoryOp) {
        customMemoryOp(tmp, addressReg);
    } else if (isParamTypeReg(paramType)) {
        writeMem((DynReg)value, width, tmp, addressReg, 0, 0);
    } else {
        writeMem(value, width, tmp, addressReg, 0, 0);
    }

    regUsed[tmp] = false;

    StartElse(bigJump);

    if (failedMemoryOp) {
        failedMemoryOp();
    } else {
        void* address;

        if (width == DYN_32bit) {
            address = writed;
        } else if (width == DYN_16bit) {
            address = writew;
        } else if (width == DYN_8bit) {
            address = writeb;
        } else {
            kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
        }
        callHostFunction(address, false, 2, addressReg, DYN_PARAM_REG_32, false, value, paramType, false);
    }
    EndIf(bigJump);

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
    if (doneWithReg && isParamTypeReg(paramType)) {
        regUsed[value] = false;
    }
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

void DynamicCodeGen::movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg, std::function<void(DynReg address, DynReg offset)> customMemoryOp, std::function<void()> failedMemoryOp, bool isBigJump) {
    regUsed[DYN_CALL_RESULT] = true;

    if (addressReg != DYN_ADDRESS) {
        kpanic("movFromMem");
    }
    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, addressReg, DYN_32bit, false);
        and32(DYN_CALL_RESULT, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(DYN_CALL_RESULT, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(DYN_CALL_RESULT, 0xFFD, false);
        } else if (width == DYN_64bit) {
            // if ((address & 0xFFF) < 0xFF9)
            IfLessThan(DYN_CALL_RESULT, 0xFF9, false);
        } else if (width == DYN_128bit) {
            // if ((address & 0xFFF) < 0xFF1)
            IfLessThan(DYN_CALL_RESULT, 0xFF1, false);
        } else {
            kpanic_fmt("DynamicCodeGen::movFromMem unknown width %d", (U32)width);
        }
    }
    // int index = address >> 12;
    // if (Memory::currentMMUReadPtr[index])
    //     return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
    // else
    //     return readd(address);

    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, addressReg, DYN_32bit, false);
    shr32(DYN_CALL_RESULT, K_PAGE_SHIFT);
    readMem(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test eax, 0x40000000 (mmu[index].canReadRam)
    IfBitSet(DYN_CALL_RESULT, 0x40000000, false, isBigJump);

    // bottom 20 bits of mmu contains ram page index
    and32(DYN_CALL_RESULT, 0xfffff);
    readMem(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, 2, (U32)ramPages);

    DynReg offsetReg(addressReg);

    if (!doneWithAddressReg) {
        if (!regUsed[DYN_SRC]) {
            offsetReg = DYN_SRC;
        } else if (!regUsed[DYN_DEST]) {
            offsetReg = DYN_DEST;
        } else {
            kpanic("DynamicCodeGen::movFromMem ran out of regs");
        }
        movToRegFromReg(offsetReg, DYN_32bit, addressReg, DYN_32bit, false);
    }
    and32(offsetReg, K_PAGE_MASK);

    if (customMemoryOp) {
        customMemoryOp(DYN_CALL_RESULT, offsetReg);
    } else {
        // mov eax, [eax+reg]
        readMem(DYN_CALL_RESULT, width, DYN_CALL_RESULT, offsetReg, 0, 0);
    }
    if (!doneWithAddressReg) {
        regUsed[offsetReg] = false;
    }
    StartElse(isBigJump);

    if (failedMemoryOp) {
        failedMemoryOp();
    } else {
        void* address;

        // call read
        if (width == DYN_32bit) {
            address = readd;
        } else if (width == DYN_16bit) {
            address = readw;
        } else if (width == DYN_8bit) {
            address = readb;
        } else {
            kpanic_fmt("unknown width in x32CPU::movFromMem %d", width);
        }

        callHostFunction(address, true, 1, addressReg, DYN_PARAM_REG_32, false);
    }
    EndIf(isBigJump);

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
}

RegPtr DynamicCodeGen::calculateEaa2(DecodedOp* op, bool popEsp) {    
    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        RegPtr result = getTmpReg();

        xorReg(DYN_32bit, result, result, false); // clear top bits

        if (op->data.disp) {
            movValue(DYN_16bit, result, op->data.disp);
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
            addReg(DYN_32bit, result, getSegAddress(op->base), false);
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
            addReg(DYN_32bit, result, getSegAddress(op->base), false);
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
        customMemoryOp(tmp, offsetReg);
    } else {
        // mov eax, [eax+reg]
        read(width, tmp, tmp, offsetReg, 0, 0);
    }
    StartElse(isBigJump);

    if (failedMemoryOp) {
        failedMemoryOp();
    } else {
        void* address;

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
        callAndReturn(address, DYN_32bit, addressReg, tmp);
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

    andValue(DYN_32bit, addressReg, K_PAGE_MASK, false);

    if (customMemoryOp) {
        customMemoryOp(tmp, addressReg);
    } else {
        write(width, tmp, addressReg, 0, 0, src);
    }    

    StartElse(isBigJump);

    if (failedMemoryOp) {
        failedMemoryOp();
    } else {
        void* address;

        if (width == DYN_32bit) {
            address = writed2;
        } else if (width == DYN_16bit) {
            address = writew2;
        } else if (width == DYN_8bit) {
            address = writeb2;
        } else {
            kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
        }
        call(address, DYN_32bit, addressReg, width, src);
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
        call(address, DYN_32bit, addressReg, imm);
    
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

        void* address;
        // call read
        if (width == DYN_32bit) {
            address = readd2;
        } else if (width == DYN_16bit) {
            address = readw2;
        } else if (width == DYN_8bit) {
            address = readb2;
        } else {
            kpanic("DynamicCodeGen::readWriteMem");
        }
        callAndReturn(address, DYN_32bit, addressReg, tmpReg2);
        xorReg(DYN_32bit, tmpReg, tmpReg, false); // so that the next if statement will also choose calling write

    EndIf();

    prepareWrite(tmpReg2);

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    If(DYN_32bit, tmpReg);

        write(DYN_32bit, tmpReg, addressReg, 0, 0, tmpReg2);

    StartElse();

        if (width == DYN_32bit) {
            address = writed2;
        } else if (width == DYN_16bit) {
            address = writew2;
        } else if (width == DYN_8bit) {
            address = writeb2;
        }
        call(address, DYN_32bit, addressReg, DYN_32bit, tmpReg2);

    EndIf();
}

void DynamicCodeGen::readWriteMem(DynWidth width, DynReg addressReg, DynReg tmpReg, bool doneWithAddressReg, std::function<void()> prepareWrite) {
    U32 firstCheckPos = 0;

    if (regUsed[tmpReg]) {
        kpanic("DynamicCodeGen::readWriteMem");
    }

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        movToRegFromReg(tmpReg, DYN_32bit, addressReg, DYN_32bit, false);
        and32(tmpReg, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(tmpReg, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(tmpReg, 0xFFD, false);
        } else {
            kpanic_fmt("DynamicCodeGen::readWriteMem unknown width %d", (U32)width);
        }
    }
    movToRegFromReg(tmpReg, DYN_32bit, addressReg, DYN_32bit, false);
    shr32(tmpReg, K_PAGE_SHIFT);
    readMem(tmpReg, DYN_32bit, tmpReg, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // if read/write
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, tmpReg, DYN_32bit, false);
    andRegImm(DYN_CALL_RESULT, DYN_32bit, 0x40000000 | 0x80000000);
    subRegImm(DYN_CALL_RESULT, DYN_32bit, 0x40000000 | 0x80000000);

    IfNot(DYN_CALL_RESULT, false);

        // bottom 20 bits of mmu contains ram page index
        and32(tmpReg, 0xfffff);
        readMem(tmpReg, DYN_32bit, tmpReg, 2, (U32)ramPages);

        if (!doneWithAddressReg) {
            kpanic("DynamicCodeGen::readWriteMem ran out of regs");
        }
        and32(addressReg, K_PAGE_MASK);

        // mov eax, [eax+reg]
        readMem(DYN_CALL_RESULT, width, tmpReg, addressReg, 0, 0);

    StartElse();

        void* address;

        // call read
        if (width == DYN_32bit) {
            address = readd;
        } else if (width == DYN_16bit) {
            address = readw;
        } else if (width == DYN_8bit) {
            address = readb;
        }

        regUsed[tmpReg] = false; // no reason to push/pop it in callHostFunction
        callHostFunction(address, true, 1, addressReg, DYN_PARAM_REG_32, false);
        xorRegReg(tmpReg, tmpReg, DYN_32bit, false); // so that the next if statement will also choose calling write

    EndIf();

    prepareWrite();

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    If(tmpReg, false);

        writeMem(DYN_CALL_RESULT, width, tmpReg, addressReg, 0, 0);

    StartElse();

        if (width == DYN_32bit) {
            address = writed;
        } else if (width == DYN_16bit) {
            address = writew;
        } else if (width == DYN_8bit) {
            address = writeb;
        }
        callHostFunction(address, false, 2, addressReg, DYN_PARAM_REG_32, true, DYN_CALL_RESULT, DYN_PARAM_REG_32, true);

    EndIf();

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
    regUsed[tmpReg] = false;
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
    const DynReg eipReg = DYN_SRC;
    const DynReg pageReg = DYN_DEST;
    const DynReg firstPageIndexReg = DYN_ADDRESS;
    const DynReg secondPageIndexReg = DYN_DEST;
    const DynReg pageOffsetReg = DYN_SRC;

    movToRegFromCpu(eipReg, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    if (cpu->thread->process->hasSetSeg[CS]) {
        loadSegAddress(CS, DYN_DEST);
        addRegReg(eipReg, DYN_DEST, DYN_32bit, true);
    }
    movToRegFromReg(pageReg, DYN_32bit, eipReg, DYN_32bit, false);
    shrRegImm(pageReg, DYN_32bit, K_PAGE_SHIFT);

    // page >> 10
    movToRegFromReg(firstPageIndexReg, DYN_32bit, pageReg, DYN_32bit, false);
    shrRegImm(firstPageIndexReg, DYN_32bit, 10);
    movToRegFromCpu(DYN_CALL_RESULT, offsetof(CPU, opCache), DYN_PTR);
    readMem(DYN_CALL_RESULT, DYN_PTR, DYN_CALL_RESULT, firstPageIndexReg, 2, 0); // :TODO: 3 on 64-bit system

    // DYN_CALL_RESULT contains 2nd level of page op cache
    If(DYN_CALL_RESULT, false);
        // page & 0x3ff        
        andRegImm(secondPageIndexReg, DYN_32bit, 0x3ff);
        readMem(DYN_CALL_RESULT, DYN_PTR, DYN_CALL_RESULT, secondPageIndexReg, 2, 0); // :TODO: 3 on 64-bit system
        // DYN_CALL_RESULT contains page of DecodedOp*
        If(DYN_CALL_RESULT, false);
            andRegImm(pageOffsetReg, DYN_32bit, K_PAGE_MASK);
            readMem(DYN_CALL_RESULT, DYN_PTR, DYN_CALL_RESULT, pageOffsetReg, 2, 0); // :TODO: 3 on 64-bit system
            // DYN_CALL_RESULT contains DecodedOp
            If(DYN_CALL_RESULT, false);
                readMem(DYN_CALL_RESULT, DYN_PTR, DYN_CALL_RESULT, 0, offsetof(DecodedOp, pfnJitCode));
                // DYN_CALL_RESULT contains pfnJitCode
                If(DYN_CALL_RESULT, false);
                    jmp(DYN_CALL_RESULT);
                EndIf();
            EndIf();
        EndIf();
    EndIf();
    regUsed[0] = false;
    regUsed[1] = false;
    regUsed[2] = false;
    regUsed[3] = false;
 }

U8* DynamicCodeGen::createJumpEip() {
    jumpToEipIfCached();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, true);
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

void DynamicCodeGen::arithRR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstCPUReg pfnInstCpuReg) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (cf) {
        dynamic_getCF();
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        loadReg(op->rm, DYN_SRC, width);
        if (cf) {
            if (width != DYN_32bit) {
                movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            addRegReg(DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        (this->*pfnInstCpuReg)(op->reg, DYN_SRC, width, true, DYN_DEST);
    } else {
        if (cf) {
            storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        loadRegStoreSrc(op->rm, width, DYN_SRC, false);
        loadRegStoreDst(op->reg, width, DYN_DEST, false);

        if (cf) {
            if (width != DYN_32bit) {
                movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            addRegReg(DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        (this->*pfnInstRegReg)(DYN_DEST, DYN_SRC, width, true);
        storeLazyFlagsResult(DYN_DEST, width, !store);
        if (store) {
            storeReg(op->reg, DYN_DEST, width, true);
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
    incrementEip(op->len);
}

void DynamicCodeGen::arithRM(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstCPUReg pfnInstCpuReg) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (cf) {
        dynamic_getCF();
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        calculateEaa(op, DYN_ADDRESS);
        if (cf) {
            movToRegFromReg(DYN_DEST, width, DYN_CALL_RESULT, DYN_32bit, false);
            movFromMem(width, DYN_ADDRESS, true);
            addRegReg(DYN_CALL_RESULT, DYN_DEST, width, true);
        } else {
            movFromMem(width, DYN_ADDRESS, true);
        }
        (this->*pfnInstCpuReg)(op->reg, DYN_CALL_RESULT, width, true, DYN_DEST);
    } else {
        if (cf) {
            storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        if (cf) {
            movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsSrcFromMem(width, DYN_ADDRESS, true, false);
        loadRegStoreDst(op->reg, width, DYN_DEST, false);
        if (cf) {
            addRegReg(DYN_CALL_RESULT, DYN_SRC, width, true);
        }
        (this->*pfnInstRegReg)(DYN_DEST, DYN_CALL_RESULT, width, true);
        storeLazyFlagsResult(DYN_DEST, width, !store);
        if (store) {
            storeReg(op->reg, DYN_DEST, width, true);
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
    incrementEip(op->len);
}

void DynamicCodeGen::arithRI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstRegImm pfnInstRegImm, InstCPUReg pfnInstCpuReg, InstCPUImm pfnInstCpuImm) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (cf) {
        dynamic_getCF();
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        if (cf) {
            if (width != DYN_32bit) {
                movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            addRegImm(DYN_CALL_RESULT, width, op->imm);
            (this->*pfnInstCpuReg)(op->reg, DYN_CALL_RESULT, width, true, DYN_DEST);
        } else {
            (this->*pfnInstCpuImm)(op->reg, width, op->imm, DYN_DEST);
        }
    } else {
        if (cf) {
            storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        storeLazyFlagsSrc(width, op->imm);
        loadRegStoreDst(op->reg, width, DYN_DEST, false);
        (this->*pfnInstRegImm)(DYN_DEST, width, op->imm);
        if (cf) {
            if (width != DYN_32bit) {
                movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            (this->*pfnInstRegReg)(DYN_DEST, DYN_CALL_RESULT, width, true);
        }
        storeLazyFlagsResult(DYN_DEST, width, !store);
        if (store) {
            storeReg(op->reg, DYN_DEST, width, true);
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
    incrementEip(op->len);
}

void DynamicCodeGen::arithMR(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstMemReg pfnInstMemReg) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (cf) {
        dynamic_getCF();
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        calculateEaa(op, DYN_ADDRESS);
        loadReg(op->reg, DYN_SRC, width);
        if (cf) {
            if (width != DYN_32bit) {
                movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
            }
            addRegReg(DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        (this->*pfnInstMemReg)(DYN_ADDRESS, DYN_SRC, width, true, true, DYN_DEST);
    } else {
        if (cf) {
            storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        loadRegStoreSrc(op->reg, width, DYN_SRC, false);
        if (cf) {
            if (width != DYN_32bit) {
                movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, true);
            }
            addRegReg(DYN_SRC, DYN_CALL_RESULT, width, true);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsDstFromMem(width, DYN_ADDRESS, !store, false);
        (this->*pfnInstRegReg)(DYN_CALL_RESULT, DYN_SRC, width, true);
        storeLazyFlagsResult(DYN_CALL_RESULT, width, !store);
        if (store) {
            movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true, DYN_DEST);
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
    incrementEip(op->len);
}

void DynamicCodeGen::arithMI(DecodedOp* op, DynWidth width, bool cf, bool store, const LazyFlags* flags, InstRegReg pfnInstRegReg, InstRegImm pfnInstRegImm, InstMemReg pfnInstMemReg, InstMemImm pfnInstMemImm) {
    U32 needsToSetFlags = op->needsToSetFlags(cpu);

    if (cf) {
        dynamic_getCF();
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        calculateEaa(op, DYN_ADDRESS);
        if (cf) {
            movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
            addRegImm(DYN_SRC, width, op->imm);
            (this->*pfnInstMemReg)(DYN_ADDRESS, DYN_SRC, width, true, true, DYN_DEST);
        } else {
            (this->*pfnInstMemImm)(DYN_ADDRESS, width, op->imm, true, DYN_DEST);
        }
    } else {
        if (cf) {
            storeLazyFlagsOldCF(DYN_CALL_RESULT, false);
        }
        calculateEaa(op, DYN_ADDRESS);
        storeLazyFlagsSrc(width, op->imm);
        if (cf) {
            movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
            storeLazyFlagsDstFromMem(width, DYN_ADDRESS, !store, false);
            addRegImm(DYN_SRC, width, op->imm);
            (this->*pfnInstRegReg)(DYN_CALL_RESULT, DYN_SRC, width, true);
        } else {
            storeLazyFlagsDstFromMem(width, DYN_ADDRESS, !store, false);
            (this->*pfnInstRegImm)(DYN_CALL_RESULT, width, op->imm);
        }
        storeLazyFlagsResult(DYN_CALL_RESULT, width, !store);
        if (store) {
            movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true, DYN_DEST);
        }
        storeLazyFlags(flags);
        currentLazyFlags = flags;
    }
    incrementEip(op->len);
}

#endif