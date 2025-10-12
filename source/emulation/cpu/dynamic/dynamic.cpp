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
            file.writeFormat("%x %x %s %x\n", cpu->thread->process->id, address, op->name(), (address + op->len + op->imm));
        } else {
            file.writeFormat("%x %x %s\n", cpu->thread->process->id, address, op->name());
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
            if (!nextOp->next && (jumpTo.contains(eip + nextOp->len) || nextOp->isDirectBranchWithNext())) {
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            }
            if (!nextOp->next && nextOp->isCall() && furthestJump > eip) {
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
            if (targetOp && !(op->flags & OP_FLAG_JIT)) {
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
        (this->*dynamicOps[nextOp->inst])(nextOp);
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
        cpu->thread->process->startJITOp = (OpCallback)createStartJITCode();
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

U32 DynamicCodeGen::cpuOffset(U32 r, DynWidth width) {
    if (width == DYN_8bit)
        return CPU::offsetofReg8(r);
    else if (width == DYN_16bit)
        return CPU::offsetofReg16(r);
    else if (width == DYN_32bit)
        return CPU::offsetofReg16(r);
    else {
        kpanic_fmt("dynamic cpuOffset unexpected width: %d", width);
        return 0;
    }
}

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

void DynamicCodeGen::movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(tmpReg, srcOffset, width);

    // mov [cpu+dstOffset], tmpReg
    movToCpuFromReg(dstOffset, tmpReg, width, doneWithTmpReg);
}

void DynamicCodeGen::loadRegStoreReg(U8 dst, U8 src, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    movToCpuFromCpu(cpuOffset(dst, width), cpuOffset(src, width), width, tmpReg, doneWithTmpReg);
}

void DynamicCodeGen::loadRegStoreSrc(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    movToCpuFromCpu(cpuOffsetSrc(width), cpuOffset(reg, width), width, tmpReg, doneWithTmpReg);
}

void DynamicCodeGen::loadRegStoreDst(U8 reg, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(reg, width), width, tmpReg, doneWithTmpReg);
}

void DynamicCodeGen::loadRegStoreEip(U8 reg, DynReg tmpReg, bool doneWithTmpReg) {
    movToCpuFromCpu(CPU_OFFSET_OF(eip.u32), cpuOffset(reg, DYN_32bit), DYN_32bit, tmpReg, doneWithTmpReg);
}

void DynamicCodeGen::loadSegValueStoreReg(U8 reg, U8 seg, DynReg tmpReg, bool doneWithTmpReg) {
    movToCpuFromCpu(cpuOffset(reg, DYN_32bit), CPU::offsetofSegValue(seg), DYN_32bit, tmpReg, doneWithTmpReg);
}

DynReg DynamicCodeGen::loadReg(U8 reg, DynReg tmpReg, DynWidth width, bool copyIntoTmp) {
    movToRegFromCpu(tmpReg, cpuOffset(reg, width), width);
    return tmpReg;
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

void DynamicCodeGen::storeReg( U8 reg, DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(cpuOffset(reg, width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsResult(DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(cpuOffsetResult(width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsDst(DynReg srcReg, DynWidth width, bool doneWithSrcReg) {
    movToCpuFromReg(cpuOffsetDst(width), srcReg, width, doneWithSrcReg);
}

void DynamicCodeGen::storeLazyFlagsOldCF(DynReg srcReg, bool doneWithSrcReg) {
    movToCpuFromReg(CPU_OFFSET_OF(oldCF), srcReg, DYN_32bit, doneWithSrcReg);
}

void DynamicCodeGen::storeEip(DynReg srcReg, bool doneWithSrcReg) {
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), srcReg, DYN_32bit, doneWithSrcReg);
}

void DynamicCodeGen::storeReg(U8 reg, DynWidth dstWidth, U32 imm) {
    movToCpu(cpuOffset(reg, dstWidth), dstWidth, imm);
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

void DynamicCodeGen::storeRegFromMem(U8 reg, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movToCpuFromMem(cpuOffset(reg, dstWidth), dstWidth, addressReg, doneWithAddressReg, doneWithCallResult);
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
    DynReg reg = loadReg(regIndex, tmpReg, regWidth);
    (this->*pfnInstRegReg)(reg, rm, regWidth, doneWithRmReg);
    movToCpuFromReg(cpuOffset(regIndex, regWidth), reg, regWidth, true);
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
    movToRegFromCpu(tmpReg, cpuOffset(regIndex, regWidth), regWidth);
    (this->*pfnInstRegImm)(tmpReg, regWidth, imm);
    movToCpuFromReg(cpuOffset(regIndex, regWidth), tmpReg, regWidth, true);
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
        kpanic("instCPUImm");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    xorRegImm(tmpReg, DYN_32bit, imm);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::andCPUFlagsImm(U32 imm, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPUImm");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    andRegImm(tmpReg, DYN_32bit, imm);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::orCPUFlagsImm(U32 imm, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPUImm");
    }
    movToRegFromCpu(tmpReg, CPU_OFFSET_OF(flags), DYN_32bit);
    orRegImm(tmpReg, DYN_32bit, imm);
    movToCpuFromReg(CPU_OFFSET_OF(flags), tmpReg, DYN_32bit, true);
}

void DynamicCodeGen::instMemImm(DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg, InstRegImm pfnInstRegImm) {
    if (regUsed[0]) {
        kpanic("x32CPU::instMemImm");
    }
    movFromMem(regWidth, addressReg, false);
    (this->*pfnInstRegImm)(DYN_CALL_RESULT, regWidth, imm);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true, tmpReg);
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
    movFromMem(regWidth, addressReg, false);
    (this->*pfnInstRegReg)(DYN_CALL_RESULT, rm, regWidth, doneWithRmReg);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true, tmpReg);
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
    movToRegFromCpu(tmpReg, cpuOffset(regIndex, regWidth), regWidth);
    negReg(tmpReg, regWidth);
    movToCpuFromReg(cpuOffset(regIndex, regWidth), tmpReg, regWidth, true);
}

void DynamicCodeGen::notCPU(U8 regIndex, DynWidth regWidth, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPU");
    }
    movToRegFromCpu(tmpReg, cpuOffset(regIndex, regWidth), regWidth);
    notReg(tmpReg, regWidth);
    movToCpuFromReg(cpuOffset(regIndex, regWidth), tmpReg, regWidth, true);
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
    movToCpuFromReg(cpuOffset(regIndex, regWidth), DYN_CALL_RESULT, regWidth, true);
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
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, true);
    if (returnEarly || lastOpEip > currentEip) {
        blockExit();
    }
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
    // :TODO: what about caching result for direct calls or direct jumps?
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

static void writed(U32 address, U32 value) {
    KThread::currentThread()->memory->writed(address, value);
}

static void writew(U32 address, U16 value) {
    KThread::currentThread()->memory->writew(address, value);
}

static void writeb(U32 address, U8 value) {
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
void DynamicCodeGen::movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp) {
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
        }
    }
    movToRegFromReg(tmp, DYN_32bit, addressReg, DYN_32bit, false);
    shr32(tmp, K_PAGE_SHIFT);
    readMem(tmp, DYN_32bit, tmp, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    IfBitSet(tmp, 0x80000000, false);

    // bottom 20 bits of mmu contains ram page index
    and32(tmp, 0xfffff);
    readMem(tmp, DYN_32bit, tmp, 2, (U32)ramPages);

    if (!doneWithAddressReg) {
        kpanic("DynamicCodeGen::movToMem ran out of regs");
    }
    and32(addressReg, K_PAGE_MASK);

    if (isParamTypeReg(paramType)) {
        writeMem((DynReg)value, width, tmp, addressReg, 0, 0);
    } else {
        writeMem(value, width, tmp, addressReg, 0, 0);
    }

    regUsed[tmp] = false;

    StartElse();

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

    EndIf();

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
    if (doneWithReg && isParamTypeReg(paramType)) {
        regUsed[value] = false;
    }
}

static U32 readd(U32 address) {
    return KThread::currentThread()->memory->readd(address);
}

static U32 readw(U32 address) {
    return KThread::currentThread()->memory->readw(address);
}

static U32 readb(U32 address) {
    return KThread::currentThread()->memory->readb(address);
}

void DynamicCodeGen::movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
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
    IfBitSet(DYN_CALL_RESULT, 0x40000000, false);

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

    // mov eax, [eax+reg]
    readMem(DYN_CALL_RESULT, width, DYN_CALL_RESULT, offsetReg, 0, 0);

    if (!doneWithAddressReg) {
        regUsed[offsetReg] = false;
    }
    StartElse();

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

    EndIf();

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
}

U8* DynamicCodeGen::createDynamicExecutableMemory() {
    U8* begin = (U8*)cpu->memory->allocCodeMemory(getBufferSize());

    Platform::writeCodeToMemory(begin, getBufferSize(), [begin, this]() {
        memcpy(begin, this->getBuffer(), this->getBufferSize());
        });
    return begin;
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
    //logBlock(data.cpu, data.startingEip, op, op->blockLen);
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
    bool needsToSetFlags = op->needsToSetFlags(cpu);

    if (cf) {
        dynamic_getCF();
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        incrementEip(op->len);
        return;
    }
    if (!needsToSetFlags) {
        loadReg(op->rm, DYN_SRC, width, true);
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
    bool needsToSetFlags = op->needsToSetFlags(cpu);

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
    bool needsToSetFlags = op->needsToSetFlags(cpu);

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
    bool needsToSetFlags = op->needsToSetFlags(cpu);

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
        loadReg(op->reg, DYN_SRC, width, true);
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
    bool needsToSetFlags = op->needsToSetFlags(cpu);

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