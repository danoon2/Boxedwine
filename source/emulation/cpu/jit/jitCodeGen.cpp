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

#ifdef BOXEDWINE_JIT
#include "jitCodeGen.h"
#include "../normal/normalCPU.h"
#include "jitFlags.h"
#include "../../softmmu/kmemory_soft.h"

static JitCodeGen::OpFunction dynamicOps[NUMBER_OF_OPS];
static std::once_flag dynamicOpsInitFlag;

void JitCodeGen::onTestEnd(DecodedOp* op) {
    writeCurrentEip(0);    
    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), (DYN_PTR_SIZE)op);
    blockExit();
}

static void initDynamicOps() {
    std::call_once(dynamicOpsInitFlag, []() {
    static_assert(offsetof(CPU, eip.u32) <= 127, "Jit needs eip to be in the first 127 bytes of the CPU");
    static_assert(offsetof(CPU, reg[8].u32) <= 127, "Jit needs reg to be in the first 127 bytes of the CPU");
    static_assert(offsetof(CPU, seg[6].address) <= 127, "Jit needs seg to be in the first 127 bytes of the CPU");

    for (int i = 0; i < InstructionCount; i++) {
        dynamicOps[i] = &Jit::dynamic_invalid_op;
    }
#define INIT_CPU(e, f) dynamicOps[e] = &Jit::dynamic_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) dynamicOps[e##_Lock] = &Jit::dynamic_##f##_lock;
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
    dynamicOps[SIDT] = &Jit::dynamic_sidt;
    dynamicOps[LGDT] = 0;
    dynamicOps[LIDT] = 0;
    dynamicOps[SMSWRreg] = 0;
    dynamicOps[SMSW] = 0;
    dynamicOps[LMSWRreg] = 0;
    dynamicOps[LMSW] = 0;
    dynamicOps[INVLPG] = 0;
    dynamicOps[Callback] = &Jit::dynamic_callback;
    dynamicOps[TestEnd] = &JitCodeGen::dynamic_onTestEnd;
    });
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

bool JitCodeGen::calculateLongestBlock(DecodedOp* op) {
    U32 eip = this->startingEip;
    DecodedOp* nextOp = op;
    U32 furthestJump = 0;
    bool hasTerminalRet = false;
    U32 terminalRetEip = 0;

    // find the longest block we can compile
    // branches that jump out of the block will be the end of the block

    // 1st pass, find longest block including all direct jumps (conditional jumps, direct jumps, loop, etc)

    // jumpTo will keep track of valid jump targets.  We need this if we are going to decode more instructions (cpu->getOp)
    // Without this the next byte of instruction may actually be invalid, I have seen skipped bytes in the instructions,
    // I assume its for alignment/performance reasons.  Firefight installer will trigger this
    BHashTable<U32, DecodedOp*> jumpTo;

    // opentdd will trigger this isValid check
    while (nextOp && nextOp->isValid()) {
        if (eip != this->startingEip && shouldStopBlockBefore(eip, nextOp)) {
            break;
        }
        // could be ret, call, int.  Basically this is an instruction where we are not guaranteed to see a next instruction
        if (nextOp->isBranch() && !nextOp->isDirectJumpBranch()) {
            {
            // Stop on a ret unless a known direct branch reaches code after it.
            if (nextOp->isRet() && furthestJump < eip + nextOp->len) {
                hasTerminalRet = true;
                terminalRetEip = eip;
                eip += nextOp->len;
                break;
            }
#ifndef BOXEDWINE_WASM_JIT
            if (nextOp->isIndirectJump()) {
                // opentdd needs this when creating a new game, I'm not sure why data.cpu->memory->getDecodedOp(eip + nextOp->len) will find an op but its not correct, might be another bug 
                break;
            }
#endif
            // These next 4 look aheads, nextOp->next =
            // They don't improve performance on Quake 2, but do make a significant improvement for Cinebench, 10-20%
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
            if (!nextOp->next && jumpTo.contains(eip + nextOp->len)) {
                // this gives a 30% improvement to cinebench, but makes F-16 unstable.  I wonder if something is wrong with this line of code
                // or if by creating a larger block it increases the chance that self modifying code will hit it and there is someting wrong
                // with how I handle self modifying code
                nextOp->next = this->cpu->getOp(eip + nextOp->len, 0);
            }
            
            if (!nextOp->next) {
                // since we couldn't figure out if the next byte is part of a valid instruction, we are done looking
                break;
            }
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
#ifdef __TEST
        if (nextOp && nextOp->inst == TestEnd) {
            eip += nextOp->len;
            break;
        }
#endif
    }
    // find longest block where all direction jumps don't go past the block
    U32 lastFurthestEip = eip;
    while (true) {
        nextOp = op;
        this->lastOpEip = this->startingEip;
        while (nextOp && this->lastOpEip < lastFurthestEip) {
            if (hasTerminalRet && this->lastOpEip == terminalRetEip && nextOp->isRet()) {
                nextOp = nullptr;
                break;
            }
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

    BHashTable<U32, DecodedOp*> ops;

    nextOp = op;
    eip = this->startingEip;
    while (nextOp && eip < this->lastOpEip) {
        ops.set(eip, nextOp);
        eip += nextOp->len;
        nextOp = nextOp->next;
    }

    nextOp = op;
    eip = this->startingEip;
    while (nextOp && eip < this->lastOpEip) {
        if (nextOp->isDirectBranch()) {
            // MW3 preview will jump to this jnz, which means the cmp/jnz pair should not be optimized as direct, see Jit::dynamic_cmpr32r32
            // without this check, MW3 will have weird drawing/camera
            // 4d7d1e Cmp EBX, ESI
            // 4d7d20 JNZ 7 -> 4d7d29
            // ...
            // 4d7d3e JLE ffffffe0 -> 4d7d20
            U32 target = eip + nextOp->len + nextOp->imm;
            DecodedOp* targetOp = nullptr;
            if (ops.get(target, targetOp)) {
                targetOp->flags2 |= OP_FLAG2_JUMP_TARGET;
            }
        }
        eip += nextOp->len;
        nextOp = nextOp->next;
    }
    return true;
}

static DecodedOp* removeJITBlock(DecodedOp* op) {
	U32 count = op->blockOpCount;

    for (U32 i = 0; i < count; i++) {
        op->pfnJitCode = nullptr;
        op->jitLen = 0;
        op->pfn = NormalCPU::getFunctionForOp(op);
        op->blockStart = nullptr;
        op->blockLen = 0;
        op->blockOpCount = 0;
        op = op->next;
    }
    return op;
}

void JitCodeGen::removeJIT(DecodedOp* op, U32 count) {
    for (U32 i = 0; i < count; i++) {
        if (op->blockStart) {
            removeJITBlock(op->blockStart);
        }
        op = op->next;
    }
}

JitConditional JitCodeGen::getJumpConditionFromOp(DecodedOp* op) {
    switch (op->inst) {
    case JumpO: return JitConditional::O;
    case JumpNO: return JitConditional::NO;
    case JumpB: return JitConditional::B;
    case JumpNB: return JitConditional::NB;
    case JumpZ: return JitConditional::Z;
    case JumpNZ: return JitConditional::NZ;
    case JumpBE: return JitConditional::BE;
    case JumpNBE: return JitConditional::NBE;
    case JumpS: return JitConditional::S;
    case JumpNS: return JitConditional::NS;
    case JumpP: return JitConditional::P;
    case JumpNP: return JitConditional::NP;
    case JumpL: return JitConditional::L;
    case JumpNL: return JitConditional::NL;
    case JumpLE: return JitConditional::LE;
    case JumpNLE: return JitConditional::NLE;
    default:
        kpanic("JitCodeGen::getJumpConditionFromOp");
    }
    return JitConditional::O;
}

JitConditional JitCodeGen::getCmovConditionFromOp(DecodedOp* op) {
    switch (op->inst) {
    case CmovO_R16R16: 
    case CmovO_R16E16:
        return JitConditional::O;
    case CmovNO_R16R16:
    case CmovNO_R16E16:
        return JitConditional::NO;
    case CmovB_R16R16:
    case CmovB_R16E16:
        return JitConditional::B;
    case CmovNB_R16R16:
    case CmovNB_R16E16:
        return JitConditional::NB;
    case CmovZ_R16R16:
    case CmovZ_R16E16:
        return JitConditional::Z;
    case CmovNZ_R16R16:
    case CmovNZ_R16E16:
        return JitConditional::NZ;
    case CmovBE_R16R16:
    case CmovBE_R16E16:
        return JitConditional::BE;
    case CmovNBE_R16R16:
    case CmovNBE_R16E16:
        return JitConditional::NBE;
    case CmovS_R16R16:
    case CmovS_R16E16:
        return JitConditional::S;
    case CmovNS_R16R16:
    case CmovNS_R16E16:
        return JitConditional::NS;
    case CmovP_R16R16:
    case CmovP_R16E16:
        return JitConditional::P;
    case CmovNP_R16R16:
    case CmovNP_R16E16:
        return JitConditional::NP;
    case CmovL_R16R16:
    case CmovL_R16E16:
        return JitConditional::L;
    case CmovNL_R16R16:
    case CmovNL_R16E16:
        return JitConditional::NL;
    case CmovLE_R16R16:
    case CmovLE_R16E16:
        return JitConditional::LE;
    case CmovNLE_R16R16:
    case CmovNLE_R16E16:
        return JitConditional::NLE;

    case CmovO_R32R32:
    case CmovO_R32E32:
        return JitConditional::O;
    case CmovNO_R32R32:
    case CmovNO_R32E32:
        return JitConditional::NO;
    case CmovB_R32R32:
    case CmovB_R32E32:
        return JitConditional::B;
    case CmovNB_R32R32:
    case CmovNB_R32E32:
        return JitConditional::NB;
    case CmovZ_R32R32:
    case CmovZ_R32E32:
        return JitConditional::Z;
    case CmovNZ_R32R32:
    case CmovNZ_R32E32:
        return JitConditional::NZ;
    case CmovBE_R32R32:
    case CmovBE_R32E32:
        return JitConditional::BE;
    case CmovNBE_R32R32:
    case CmovNBE_R32E32:
        return JitConditional::NBE;
    case CmovS_R32R32:
    case CmovS_R32E32:
        return JitConditional::S;
    case CmovNS_R32R32:
    case CmovNS_R32E32:
        return JitConditional::NS;
    case CmovP_R32R32:
    case CmovP_R32E32:
        return JitConditional::P;
    case CmovNP_R32R32:
    case CmovNP_R32E32:
        return JitConditional::NP;
    case CmovL_R32R32:
    case CmovL_R32E32:
        return JitConditional::L;
    case CmovNL_R32R32:
    case CmovNL_R32E32:
        return JitConditional::NL;
    case CmovLE_R32R32:
    case CmovLE_R32E32:
        return JitConditional::LE;
    case CmovNLE_R32R32:
    case CmovNLE_R32E32:
        return JitConditional::NLE;
    default:
        kpanic("JitCodeGen::getCmovConditionFromOp");
    }
    return JitConditional::O;
}

JitConditional JitCodeGen::getSetConditionFromOp(DecodedOp* op) {
    switch (op->inst) {
    case SetO_R8:
    case SetO_E8:
        return JitConditional::O;
    case SetNO_R8:
    case SetNO_E8:
        return JitConditional::NO;
    case SetB_R8:
    case SetB_E8:
        return JitConditional::B;
    case SetNB_R8:
    case SetNB_E8:
        return JitConditional::NB;
    case SetZ_R8:
    case SetZ_E8:
        return JitConditional::Z;
    case SetNZ_R8:
    case SetNZ_E8:
        return JitConditional::NZ;
    case SetBE_R8:
    case SetBE_E8:
        return JitConditional::BE;
    case SetNBE_R8:
    case SetNBE_E8:
        return JitConditional::NBE;
    case SetS_R8:
    case SetS_E8:
        return JitConditional::S;
    case SetNS_R8:
    case SetNS_E8:
        return JitConditional::NS;
    case SetP_R8:
    case SetP_E8:
        return JitConditional::P;
    case SetNP_R8:
    case SetNP_E8:
        return JitConditional::NP;
    case SetL_R8:
    case SetL_E8:
        return JitConditional::L;
    case SetNL_R8:
    case SetNL_E8:
        return JitConditional::NL;
    case SetLE_R8:
    case SetLE_E8:
        return JitConditional::LE;
    case SetNLE_R8:
    case SetNLE_E8:
        return JitConditional::NLE;    
    default:
        kpanic("JitCodeGen::getSetConditionFromOp");
    }
    return JitConditional::O;
}

enum class DirectType {
    None,
    Jump,
    CMov,
    SetCC
};

void JitCodeGen::tryDirect(DecodedOp* op, std::function<void()> callback, std::function<void()> fallback) {
    DecodedOp* nextOp = op->next;
    U32 skipped = 0;
    DirectType directType = DirectType::None;
    JitConditional cond = JitConditional::O;

    for (int i = 0; i < 8 && nextOp; i++) {
        if (nextOp->flags2 & OP_FLAG2_JUMP_TARGET) {
            break;
        }
        if (nextOp->isJumpCC()) {
            cond = getJumpConditionFromOp(nextOp);
            directType = DirectType::Jump;
            break;
        }
        /*
        if (nextOp->isCMovCC() && instructionInfo[nextOp->inst].readMemWidth == 0) {
            cond = getCmovConditionFromOp(nextOp);
            directType = DirectType::CMov;
            break;
        }
        if (nextOp->isSetCC() && instructionInfo[nextOp->inst].writeMemWidth == 0) {
            cond = getSetConditionFromOp(nextOp);
            directType = DirectType::SetCC;
            break;
        }
        */
        if (instructionInfo[nextOp->inst].flagsSets) {
            break;
        }
        if (directDoesAffectFlags(nextOp)) {
            break;
        }
        skipped++;
        nextOp = nextOp->next;
    }
    if (directType != DirectType::None && !nextOp->getNeededFlagsAfter(FMASK_TEST) && supportsDirectCondition(cond)) {
        DecodedOp* nextOp = op->next;
        U32 opEip = currentEip + op->len;

        for (U32 i = 0; i < skipped; i++) {
            opEip += nextOp->len;
            nextOp = nextOp->next;
        }
        if (directType == DirectType::Jump && !canJumpInBlock(opEip, nextOp)) {
            fallback();
            return;
        }
        callback();
        postCompile(op);

        nextOp = op->next;
        for (U32 i = 0; i < skipped; i++) {
            preCompile(nextOp, true);
            compile(nextOp);
            postCompile(nextOp);
            nextOp = nextOp->next;
        }
        preCompile(nextOp, true);
        switch (directType) {
            case DirectType::Jump:
                direct_jump(cond, opEip + nextOp->len + nextOp->imm);
                break;
            case DirectType::CMov:
            {
                JitWidth width = nextOp->inst >= CmovO_R32R32 ? JitWidth::b32 : JitWidth::b16;
                direct_cmov(width, cond, getReg(nextOp->reg), instructionInfo[nextOp->inst].readMemWidth ? read(width, calculateEaa(nextOp)) : getReg(nextOp->rm));
                break;
            }
            case DirectType::SetCC:
                if (instructionInfo[nextOp->inst].readMemWidth) {
                    kpanic("JitCodeGen::tryDirect set cc mem");
                } else {
                    direct_setcc(cond, getReg8(nextOp->reg, false));
                }
                break;
            case DirectType::None:
                kpanic("JitCodeGen::tryDirect should not get here");
                break;
        }
        skipToOp = nextOp;
    } else {
        fallback();
    }
}

void JitCodeGen::preCompile(DecodedOp* op, bool skippedOp) {
    this->emulatedLen += op->len;
    this->blockOpCount++;
    this->eipToBufferPos.set(this->currentEip, skippedOp ? SKIPPED_OP : markBufferLocation());
    if (op->lock) {
        // so that intra block jumps that try to skip a lock will find the lock version of the op anyway
        this->eipToBufferPos.set(this->currentEip + 1, skippedOp ? SKIPPED_OP : markBufferLocation());
    }
    preOp(op);
}

void JitCodeGen::compile(DecodedOp* op) {
    (this->*dynamicOps[op->inst])(op);
}

void JitCodeGen::postCompile(DecodedOp* op) {
    this->currentEip += op->len;
    if (getIfJumpSize()) {
        kpanic_fmt("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
    }
}

bool JitCodeGen::compileOps(DecodedOp* op) {
    DecodedOp* nextOp = op;
    this->emulatedLen = 0;
    this->blockOpCount = 0;

    while (nextOp) {
        if (nextOp->flags & OP_FLAG_NO_JIT) {
            return false;
        }

#ifndef __TEST
#ifdef _DEBUG
        //callHostFunction(common_log, false, 2, 0, JitCallParamType::CPU, false, (DYN_PTR_SIZE)nextOp, JitCallParamType::CONST_PTR, false);
#endif
#endif
        preCompile(nextOp);
        // enable this if you want to see the current eip while debugging the generated asm
        // movValue(JitWidth::b32, getTmpReg(), this->currentEip);

        // this is a nice way to figure out what jit instruction is bugged
        // assuming the normal, non jit code works. If there is a bug in the jit,
        // we can emulate instructions using the normal core for certain ranges of instructions
        // until we find the jit instruction that is causing the bug
        // 
        // this is currently setup to start debugging with SSE emulated
        // 
        // 911 = AddpsXmm 
        // 1282 = ShufpdXmmE128
        //if (nextOp->inst >= 911 && nextOp->inst <= 1282) {
         //   emulateSingleOp();
        //} else {
            compile(nextOp);
        //}
        
        if (skipToOp) {
            nextOp = skipToOp;
            skipToOp = nullptr;
        }
        postCompile(nextOp);
        if (this->currentEip > this->lastOpEip) {
            break;
        } else {
            if (nextOp->next && nextOp->next->inst == Done) {
                if (!nextOp->isBranch()) { // f16 needs this
                    writeCurrentEip(0);
                }
                RegPtr eip = getTmpReg();
                movValue(JitWidth::b32, eip, currentEip - cpu->seg[CS].address);
                jumpEip(eip);
                break;
            }
            nextOp = nextOp->next;
        }
    }
    return true;
}

void JitCodeGen::doJIT(U32 address, DecodedOp* op) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
    // did another thread beat us to JITing this block?
    if (op->flags & OP_FLAG_JIT) {
        // this will get triggered a few times, especially during shutdown
        // I have see this in firefight installer at the end and opentdd start up
        return;
    }
    if (!cpu->thread->process->startJITOp) {
        JitCodeGen* jit = startNewJIT(cpu);
        cpu->thread->process->syncToHost = jit->createSyncToHost();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->syncFromHost = jit->createSyncFromHost();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->blockExit = jit->createBlockExit();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->emulateSingleOp = jit->createEmulateSingleOp();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->startJITOp = (OpCallback)jit->createStartJITCode();
        delete jit;
#if defined(BOXEDWINE_POSIX) && defined(BOXEDWINE_HOST_EXCEPTIONS)
        jit = startNewJIT(cpu);
        cpu->thread->process->signalHandler = jit->createSignalHandler();
        delete jit;
#endif
        createHelpers();
        for (U32 i = 0; i < FLAGS_NULL; i++) {
            jit = startNewJIT(cpu);
            jit->disableTmps = true; // the functions created should not call getTmp since the caller won't know what tmps are in use
            cpu->thread->process->calculateCF[i] = jit->createCalculationCF((LazyFlagType)i);
            delete jit;
        }
    }
    if (!cpu->calculateCF[0]) {
        if (!cpu->thread->process->calculateCF[0]) {
            kpanic("JitCodeGen::doJIT");
        }
        memcpy(cpu->calculateCF, cpu->thread->process->calculateCF, sizeof(cpu->thread->process->calculateCF));
    }    
    this->currentEip = address;
    this->startingEip = address;

    initDynamicOps();

    if (!calculateLongestBlock(op)) {
        return;
    }
    if (!compileOps(op)) {
        return;    
    }    
    onBlockPreCommit(op);
    commitJIT(op);
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    bool shouldStartJit = op->runCount == 0;
#else
    // done check is for long blocks that get broken up, affects f-22/f-16
    bool shouldStartJit = op->runCount == JIT_RUN_COUNT && op->inst != Done && !(op->flags & OP_FLAG_JIT);
#endif    
    if (shouldStartJit) {
        startNewJIT(cpu, cpu->getEipAddress(), op);
    }
#ifdef _DEBUG
    if (op->pfnJitCode && cpu->calculateCF[0] == nullptr) {
        //kpanic("firstDynamicOp");
    }
#endif
    op->runCount++;
    op->pfn(cpu, op);
}

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

bool JitCodeGen::isParamTypeReg(JitCallParamType paramType) {
    return paramType == JitCallParamType::REG_8 || paramType == JitCallParamType::REG_16 || paramType == JitCallParamType::REG_32;
}

void JitCodeGen::jumpEip(RegPtr reg) {
    RegPtr tmp = getTmpReg();
    mov(JitWidth::b32, tmp, reg);
    jumpToEipIfCached(tmp); // jumpToEipIfCached can modify the passed in reg
    writeEip(reg);
    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), 0);
    blockExit();
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void JitCodeGen::blockNext1(U32 eip, DecodedOp* op) {
    // if (!(*(op->nextJump))) {
    //     *(op->nextJump) = cpu->getNextOp();
    // }
    // cpu->nextOp = *(op->nextJump);
    if (op->data.nextJump) { // don't code for it if it hasn't happened, this gives about a 1% performance boost by keeping the code more compact
        RegPtr opReg = getTmpReg();
        movValue(DYN_PTR, opReg, (DYN_PTR_SIZE)op);
        // ebx = op->nextJump
        // mov ebx, [edx + offsetof(DecodedOp, nextJump)]
        RegPtr nextJump = getTmpReg();
        readHost(DYN_PTR, createMemPtr(opReg, (U32)offsetof(DecodedOp, data.nextJump)), nextJump, false);
        opReg = nullptr;

        // eax = *(op->nextJump)
        RegPtr nextOp = getTmpReg();
        readHost(DYN_PTR, createMemPtr(nextJump, 0), nextOp, false);
        // if (!(*(op->nextJump))) 
        If(DYN_PTR, nextOp); {
            RegPtr jit = getTmpReg();
            readHost(DYN_PTR, createMemPtr(nextOp, (U32)offsetof(DecodedOp, pfnJitCode)), jit, false);
            If(DYN_PTR, jit); {
                jmpHost(jit);
            } EndIf();
        } EndIf();
    }

    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), 0);
    writeEip(eip - cpu->seg[CS].address);
    blockExit();
}

void JitCodeGen::blockNext2(U32 eip, DecodedOp* op) {
    // if (!op->next) { 
    //     op->next = cpu->getNextOp(); 
    // }
    // cpu->nextOp = op->next;

    if (op->next) {
        RegPtr opReg = getTmpReg();
        movValue(DYN_PTR, opReg, (DYN_PTR_SIZE)op);

        // mov eax, [ebx + offsetof(DecodedOp, next)]
        RegPtr nextReg = getTmpReg();
        readHost(DYN_PTR, createMemPtr(opReg, (U32)offsetof(DecodedOp, next)), nextReg, false);

        If(DYN_PTR, nextReg); {
            RegPtr jit = getTmpReg();
            readHost(DYN_PTR, createMemPtr(nextReg, (U32)offsetof(DecodedOp, pfnJitCode)), jit, false);
            If(DYN_PTR, jit); {
                jmpHost(jit);
            } EndIf();
        } EndIf();
    }    

    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), 0);
    writeEip(eip - cpu->seg[CS].address);
    blockExit();
}

U8* JitCodeGen::createDynamicExecutableMemory(U32* pSize) {
    U32 size = getBufferSize();
    if (pSize) {
        *pSize = size;
    }
    U8* begin = (U8*)cpu->memory->allocCodeMemory(size);

    Platform::writeCodeToMemory(begin, size, [begin, size, this]() {
        copyBuffer(begin, size);
    });
    Platform::clearInstructionCache(begin, size);

    return begin;
}

void JitCodeGen::jumpToEipIfCached(RegPtr eipReg) {
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
    if (!eipReg) {
        eipReg = readEip();
    }
    if (cpu->thread->process->hasSetSeg[CS]) {
        addReg(JitWidth::b32, eipReg, getReadOnlySegAddress(CS));
    }
    RegPtr pageReg = getTmpReg();
    shrValueWithDest(JitWidth::b32, pageReg, eipReg, K_PAGE_SHIFT);

    RegPtr firstPageIndexReg = getTmpReg();
    shrValueWithDest(JitWidth::b32, firstPageIndexReg, pageReg, 10);

    RegPtr tmp = readCPU(DYN_PTR, offsetof(CPU, opCache));
    readHost(DYN_PTR, createMemPtr(tmp, firstPageIndexReg, DYN_PTR_LSL, 0), tmp, false);

    // page & 0x3ff
    andValue(JitWidth::b32, pageReg, 0x3ff);
    readHost(DYN_PTR, createMemPtr(tmp, pageReg, DYN_PTR_LSL, 0), tmp, false);
    // tmp contains page of DecodedOp*
    If(DYN_PTR, tmp); {
        andValueWithDest(JitWidth::b32, pageReg, eipReg, K_PAGE_MASK);
        readHost(DYN_PTR, createMemPtr(tmp, pageReg, DYN_PTR_LSL, 0), tmp, false);
        // tmp contains DecodedOp
        If(DYN_PTR, tmp); {
            readHost(DYN_PTR, createMemPtr(tmp, (U32)offsetof(DecodedOp, pfnJitCode)), tmp, false);
            // tmp contains pfnJitCode
            If(DYN_PTR, tmp); {
                jmpHost(tmp);
            } EndIf();
        } EndIf();
    } EndIf();
}

void jitRunSingleOp(CPU* cpu) {
    cpu->runNextSingleOp();
}

#if defined(BOXEDWINE_POSIX) && defined(BOXEDWINE_HOST_EXCEPTIONS)
void signalHandler(CPU* cpu);

U8* JitCodeGen::createSignalHandler() {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    callHostFunction((void*)signalHandler, params, true, false);
    blockExit();

    return createDynamicExecutableMemory();
}
#endif

U8* JitCodeGen::createEmulateSingleOp() {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    callHostFunction((void*)jitRunSingleOp, params);    
    blockExit();

    return createDynamicExecutableMemory();
}

void JitCodeGen::commitJIT(DecodedOp* op) {
    if (!blockOpCount) {
        return;
    }
    U32 size = 0;
    U8* begin = createDynamicExecutableMemory(&size);

    removeJIT(op, blockOpCount);
    op->blockLen = emulatedLen;
    op->blockOpCount = blockOpCount;
#ifdef _DEBUG
    if (0) {
        logBlock(cpu, startingEip, op, op->blockLen);
    }
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
    static int totalEmulatedLen;
    totalEmulatedLen += emulatedLen;
    static int totalCompiledLen;
    totalCompiledLen += size;
    if ((totalBlocks % 1000) == 0) {
        klog_fmt("Compiled Blocks: %d, ave block size: %d ops.  ops = %d, elen = %d, clen = %d", totalBlocks, totalOps / totalBlocks, totalOps, totalEmulatedLen, totalCompiledLen);
    }
#endif
    DecodedOp* lastJitOp = nullptr;
    U32 lastJitEip = 0;

    for (U32 i = 0; i < blockOpCount; i++) {
        U32 bufferIndex = 0;

        if (!eipToBufferPos.get(address, bufferIndex)) {
            kpanic("x32CPU commitJIT 2");
        }
        if (bufferIndex == SKIPPED_OP) {
            nextOp->flags2 |= OP_FLAG2_JUMP_TARGET_ASSUMED_FALSE;
        } else {
            bufferIndex = getBufferLocation(bufferIndex);
            nextOp->pfnJitCode = begin + bufferIndex;
            nextOp->jitLen = 0;
            nextOp->pfn = cpu->thread->process->startJITOp;
            if (lastJitOp) {
                lastJitOp->jitLen = static_cast<U16>((U8*)nextOp->pfnJitCode - (U8*)lastJitOp->pfnJitCode);
#ifdef BOXEDWINE_HOST_EXCEPTIONS
                getMemData(cpu->memory)->jitAddressToEip[(U8*)lastJitOp->pfnJitCode] = JitData(lastJitOp->jitLen, lastJitEip - cpu->seg[CS].address);
#endif
            }
            lastJitEip = address;
            lastJitOp = nextOp;
        }
        nextOp->flags |= OP_FLAG_JIT;
        nextOp->blockStart = op;
        address += nextOp->len;
        last = nextOp;
        nextOp = nextOp->next;
    }
    if (lastJitOp && !lastJitOp->jitLen) {
        lastJitOp->jitLen = (U16)(size - static_cast<U32>((U8*)lastJitOp->pfnJitCode - (U8*)begin));
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        getMemData(cpu->memory)->jitAddressToEip[(U8*)lastJitOp->pfnJitCode] = JitData(lastJitOp->jitLen, lastJitEip - cpu->seg[CS].address);
#endif
    }
}

RegPtr JitCodeGen::getZF() {
    RegPtr result = getTmpReg8();
    RegPtr lazyFlags = getLazyFlagType();

    if (currentLazyFlags != FLAGS_NULL && currentLazyFlags != FLAGS_NONE) {
        IfEqual(JitWidth::b32, lazyFlags, currentLazyFlags); {
            getFlagResultTmp(result);
            If(getWidthOfFlags(currentLazyFlags), result); {
                xorReg(JitWidth::b32, result, result);
            } StartElse(); {
                movValue(JitWidth::b32, result, ZF);
            } EndIf();
        } StartElse(); {
            fillFlags();
            getFlagsInTmp(result);
            andValue(JitWidth::b32, result, ZF);
        } EndIf();        
    } else {
        fillFlags();
        getFlagsInTmp(result);
        andValue(JitWidth::b32, result, ZF);
    }
    return result;
}

void JitCodeGen::fillFlags(U32 flagsToFill) {
    RegPtr flags = getLazyFlagType();
    If(JitWidth::b32, flags); // FLAGS_NONE = 0
        call(common_fillFlags);
    EndIf();
    currentLazyFlags = FLAGS_NONE;
}

void JitCodeGen::storeLazyFlagType(LazyFlagType flags) {
    if (flags == FLAGS_CFOF) {
        RegPtr lazyFlags = getLazyFlagType();
        IfNotEqual(JitWidth::b32, lazyFlags, FLAGS_CFOF); {
            writeCPU(JitWidth::b8, CPU_OFFSET_OF(lazyFlagTypePrev), lazyFlags);
            writeCPUValue(JitWidth::b8, CPU_OFFSET_OF(lazyFlagType), FLAGS_CFOF);
        } EndIf();
    } else {
        writeCPUValue(JitWidth::b8, CPU_OFFSET_OF(lazyFlagType), flags);
    }
}

void JitCodeGen::writeFlags(RegPtr flags) {
    writeCPU(JitWidth::b32, CPU_OFFSET_OF(flags), flags);
}

void JitCodeGen::storeLazyFlagsDest(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32, CPU_OFFSET_OF(dst.u32), reg);
}

void JitCodeGen::storeLazyFlagsSrc(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32, CPU_OFFSET_OF(src.u32), reg);
}

void JitCodeGen::storeLazyFlagsSrc(U32 value) {
    writeCPUValue(JitWidth::b32, CPU_OFFSET_OF(src.u32), value);
}

void JitCodeGen::storeLazyFlagsResult(RegPtr reg) {
    writeCPU(reg->isHigh ? JitWidth::b8 : JitWidth::b32, CPU_OFFSET_OF(result.u32), reg);
}

void JitCodeGen::storeLazyFlagsOldCF(RegPtr reg) {
    writeCPU(JitWidth::b32, CPU_OFFSET_OF(oldCF), reg);
}

void JitCodeGen::xorCPUFlagsImmV2(U32 imm) {
    RegPtr reg = getFlagsInTmp();
    xorValue(JitWidth::b32, reg, imm);
    writeFlags(reg);
}

void JitCodeGen::andCPUFlagsImmV2(U32 imm) {
    RegPtr reg = getFlagsInTmp();
    andValue(JitWidth::b32, reg, imm);
    writeFlags(reg);
}

void JitCodeGen::orCPUFlagsImmV2(U32 imm) {
    RegPtr reg = getFlagsInTmp();
    orValue(JitWidth::b32, reg, imm);
    writeFlags(reg);
}

void JitCodeGen::orCPUFlags(RegPtr flags) {
    RegPtr reg = getFlagsInTmp();
    orReg(JitWidth::b32, reg, flags);
    writeFlags(reg);
}

RegPtr JitCodeGen::read(JitWidth width, RegPtr addressReg, std::function<void(MemPtr address)> customMemoryOp, std::function<void()> failedMemoryOp, RegPtr tmp, bool checkAlignment) {
    if (!tmp) {
        tmp = getTmpReg8();
    }

#ifdef BOXEDWINE_MEM_CACHE
    if (currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
#ifdef _DEBUG
        writeCurrentEip(0);
#endif
        shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);

        readMMU(tmp, tmp, K_NUMBER_OF_PAGES * sizeof(void*));

        if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
            RegPtr offsetReg = getTmpReg();

            andValueWithDest(JitWidth::b32, offsetReg, addressReg, K_PAGE_MASK);
            addReg(DYN_PTR, tmp, addressReg);
            clearIfSpansPage(width, offsetReg, tmp);
            if (customMemoryOp) {
                customMemoryOp(createMemPtr(tmp));
            } else {
                readHost(width, createMemPtr(tmp), tmp);
            }
        } else {
            if (customMemoryOp) {
                customMemoryOp(createMemPtr(tmp, addressReg));
            } else {
                readHost(width, createMemPtr(tmp, addressReg), tmp);
            }
        }
        return tmp;
    }
#endif 
    RegPtr offsetReg;

    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
        andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);
    } else {
        offsetReg = getTmpReg();
        andValueWithDest(JitWidth::b32, offsetReg, addressReg, K_PAGE_MASK);
    }    

    if (width != JitWidth::b8 && checkAlignment) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (KSystem::canJitUse4KPage) {
#ifdef _DEBUG
            writeCurrentEip(0);
#endif
            if (currentOp->exceptionCount == MAX_OP_EXCEPTION_COUNT) {
                clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
            }
        } else
#endif
        {
            // make sure we only use the fast path if the entire read will take place on the same page
            clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        }
    }
    
    IfNotTestBit(JitWidth::b32, tmp, 0); {
        if (failedMemoryOp) {
            failedMemoryOp();
        } else {
            emulateSingleOp();
        }
    } EndIf();

    andValueNative(tmp, ~0xfff);

    if (customMemoryOp) {
        // regs are not used after this, so give them back
        addressReg = nullptr;
        customMemoryOp(createMemPtr(tmp, offsetReg));
    } else {
        // mov eax, [eax+reg]
        readHost(width, createMemPtr(tmp, offsetReg), tmp);
    }

    return tmp;
}

RegPtr JitCodeGen::calculateAddress(MemPtr mem) {
    RegPtr tmp = getTmpReg();
    if (mem->sib && mem->lsl) {
        shlValueWithDest(DYN_PTR, tmp, mem->sib, mem->lsl);
        if (mem->rm) {
            addReg(DYN_PTR, tmp, mem->rm);
        }
        if (mem->offset) {
            addValue(DYN_PTR, tmp, mem->offset);
        }
    } else if (mem->sib) {
        if (mem->rm) {
            if (mem->offset) {
                addValueWithDest(DYN_PTR, tmp, mem->sib, mem->offset);
                addReg(DYN_PTR, tmp, mem->rm);
            } else {
                addRegWithDest(DYN_PTR, tmp, mem->rm, mem->sib);
            }
        } else if (mem->offset) {
            addValueWithDest(DYN_PTR, tmp, mem->sib, mem->offset);
        } else {
            return mem->sib;
        }
    } else if (mem->rm) {
        if (mem->offset) {
            addValueWithDest(DYN_PTR, tmp, mem->rm, mem->offset);
        } else {
            return mem->rm;
        }
    } else {
        movValue(DYN_PTR, tmp, mem->offset);
    }
    return tmp;
}

static bool doesSpanPage(JitWidth width, U32 offset) {
    if (width == JitWidth::b8) {
        return false;
    } else if (width == JitWidth::b16) {
        return (offset & 0xFFF) == 0xFFF;
    } else if (width == JitWidth::b32) {
        return (offset & 0xFFF) >= 0xFFD;
    } else {
        kpanic("doesSpanPage");
        return true;
    }
}

RegPtr JitCodeGen::read(JitWidth width, MemPtr mem, RegPtr result) {
    if (!mem->emulatedAddress) {
        if (!result) {
            result = getTmpReg8();
        }
        readHost(width, mem, result);
        return result;
    }

    if (mem->rm || mem->sib) {
        return read(width, calculateAddress(mem), nullptr, nullptr, result);
    }
    if (!result) {
        result = getTmpReg8();
    }
#ifdef BOXEDWINE_MEM_CACHE
    if (currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT && (KSystem::canJitUse4KPage || !doesSpanPage(width, mem->offset))) {
#ifdef _DEBUG
        writeCurrentEip(0);
#endif
        readMMU(result, K_NUMBER_OF_PAGES + (mem->offset >> K_PAGE_SHIFT));
        readHost(width, createMemPtr(result, mem->offset), result);
        return result;
    }
#endif 
    U32 address = mem->offset;

    if (width == JitWidth::b16) {
        if ((address & 0xFFF) == 0xFFF) {
            emulateSingleOp();
            return result;
        }
    } else if (width == JitWidth::b32) {
        if ((address & 0xFFF) >= 0xFFD) {
            emulateSingleOp();
            return result;
        }
    } else if (width != JitWidth::b8) {
        kpanic_fmt("JitCodeGen::read unknown width %d", (U32)width);
    }
    
    readMMU(result, address >> K_PAGE_SHIFT);

    IfNotTestBit(JitWidth::b32, result, 0); {
        emulateSingleOp();
    } EndIf();

    andValueNative(result, ~0xfff);

    readHost(width, createMemPtr(result,  address & K_PAGE_MASK), result);

    return result;
}

void JitCodeGen::write(JitWidth width, MemPtr mem, RegPtr src) {
    if (!mem->emulatedAddress) {
        writeHost(width, mem, src);
        return;
    }
    if (mem->rm || mem->sib) {
        write(width, calculateAddress(mem), src);
        return;
    }
    RegPtr tmp = getTmpReg8();
#ifdef BOXEDWINE_MEM_CACHE
    if (currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT && (KSystem::canJitUse4KPage || !doesSpanPage(width, mem->offset))) {
#ifdef _DEBUG
        writeCurrentEip(0);
#endif
        readMMU(tmp, K_NUMBER_OF_PAGES * 2 + (mem->offset >> K_PAGE_SHIFT));
        writeHost(width, createMemPtr(tmp, mem->offset), src);
        return;
    }
#endif

    U32 address = mem->offset;
    if (width == JitWidth::b16) {
        if ((address & 0xFFF) == 0xFFF) {
            emulateSingleOp();
            return;
        }
    } else if (width == JitWidth::b32) {
        if ((address & 0xFFF) >= 0xFFD) {
            emulateSingleOp();
            return;
        }
    } else if (width != JitWidth::b8) {
        kpanic_fmt("JitCodeGen::write unknown width %d", (U32)width);
    }    
    readMMU(tmp, address >> K_PAGE_SHIFT);

    IfNotTestBit(JitWidth::b32, tmp, 1); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);    
    writeHost(width, createMemPtr(tmp, address & K_PAGE_MASK), src);
}

void JitCodeGen::write(JitWidth width, RegPtr addressReg, RegPtr src, std::function<void(MemPtr address)> customMemoryOp, std::function<void()> failedMemoryOp, bool checkAlignment) {
    RegPtr tmp = getTmpReg();

    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
#ifdef BOXEDWINE_MEM_CACHE
    if (currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
#ifdef _DEBUG
        writeCurrentEip(0);
#endif        
        readMMU(tmp, tmp, K_NUMBER_OF_PAGES * sizeof(void*) * 2);

        if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
            RegPtr offsetReg = getTmpReg();

            addReg(DYN_PTR, tmp, addressReg);
            andValueWithDest(JitWidth::b32, offsetReg, addressReg, K_PAGE_MASK);
            clearIfSpansPage(width, std::move(offsetReg), tmp);
            if (customMemoryOp) {
                customMemoryOp(createMemPtr(tmp));
            } else {
                writeHost(width, createMemPtr(tmp), src);
            }
        } else {
            if (customMemoryOp) {
                customMemoryOp(createMemPtr(tmp, addressReg));
            } else {
                writeHost(width, createMemPtr(tmp, addressReg), src);
            }
        }
        return;
    }
#endif    

    RegPtr offsetReg;

    readMMU(tmp, tmp);

    bool pushedAddress = false;
    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
        andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);
    } else if (isTmpRegAvailable()) {
        offsetReg = getTmpReg();
        andValueWithDest(JitWidth::b32, offsetReg, addressReg, K_PAGE_MASK);
    } else {
        pushedAddress = true;
        currentOp->flags2 |= OP_FLAG2_SAVED_TMP_REG;
        writeCPU(JitWidth::b32, offsetof(CPU, tmpReg), addressReg);
        offsetReg = addressReg;
        andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);
    }    

    if (width != JitWidth::b8 && checkAlignment) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (KSystem::canJitUse4KPage) {
#ifdef _DEBUG
            writeCurrentEip(0);
#endif
            if (currentOp->exceptionCount == MAX_OP_EXCEPTION_COUNT) {
                clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
            }

        } else 
#endif
        {
            // make sure we only use the fast path if the entire read will take place on the same page
            clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        }
    }

    IfNotTestBit(JitWidth::b32, tmp, 1); {
        if (pushedAddress) {
            readCPU(JitWidth::b32, offsetof(CPU, tmpReg), addressReg);
        }
        if (failedMemoryOp) {
            failedMemoryOp();
        } else {
            emulateSingleOp();
        }
    } EndIf();

    andValueNative(tmp, ~0xfff);

    MemPtr mem = createMemPtr(std::move(tmp), std::move(offsetReg));
    if (customMemoryOp) {
        customMemoryOp(std::move(mem));
    } else {
        writeHost(width, std::move(mem), src);
    }
    if (pushedAddress) {
        readCPU(JitWidth::b32, offsetof(CPU, tmpReg), addressReg);
    }
}

void JitCodeGen::write(JitWidth width, MemPtr mem, U32 imm) {
    if (!mem->emulatedAddress) {
        writeHost(width, mem, imm);
        return;
    }
#ifdef BOXEDWINE_MEM_CACHE
    if (currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
        RegPtr tmp = getTmpReg();
        RegPtr addressReg = calculateAddress(mem);

#ifdef _DEBUG
        writeCurrentEip(0);
#endif
        shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
        readMMU(tmp, tmp, K_NUMBER_OF_PAGES * sizeof(void*) * 2);
        if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
            RegPtr offsetReg = getTmpReg();

            addReg(DYN_PTR, tmp, addressReg);
            andValueWithDest(JitWidth::b32, offsetReg, addressReg, K_PAGE_MASK);
            clearIfSpansPage(width, std::move(offsetReg), tmp);
            writeHost(width, createMemPtr(tmp), imm);
        } else {
            writeHost(width, createMemPtr(tmp, addressReg), imm);
        }
        return;
    }
#endif

    RegPtr addressReg = calculateAddress(mem);
    RegPtr tmp = getTmpReg();
    RegPtr offsetReg;

    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    offsetReg = addressReg;
    andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);
 
    if (width != JitWidth::b8) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (KSystem::canJitUse4KPage) {
#ifdef _DEBUG
            writeCurrentEip(0);
#endif
            if (currentOp->exceptionCount == MAX_OP_EXCEPTION_COUNT) {
                clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
            }
        } else 
#endif
        {
            // make sure we only use the fast path if the entire read will take place on the same page
            clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        }
    }

    IfNotTestBit(JitWidth::b32, tmp, 1); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);
    writeHost(width, createMemPtr(tmp, offsetReg, 0, 0), imm);
}

RegPtr JitCodeGen::readWriteMem(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint) {
    RegPtr tmp = getTmpRegWithHint(hint);

#ifdef BOXEDWINE_MEM_CACHE
    if (currentOp->exceptionCount < MAX_OP_EXCEPTION_COUNT) {
#ifdef _DEBUG
        writeCurrentEip(0);
#endif
        shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);       
        readMMU(tmp, tmp, K_NUMBER_OF_PAGES * sizeof(void*) * 2);
        if (!KSystem::canJitUse4KPage && width != JitWidth::b8) {
            RegPtr offsetReg = getTmpReg();

            addReg(DYN_PTR, tmp, addressReg);
            andValueWithDest(JitWidth::b32, offsetReg, addressReg, K_PAGE_MASK);
            clearIfSpansPage(width, offsetReg, tmp);
            readHost(width, createMemPtr(tmp), offsetReg);
            prepareWrite(offsetReg);
            writeHost(width, createMemPtr(tmp), offsetReg);
            return offsetReg;
        } else {
            RegPtr value = getTmpReg();

            readHost(width, createMemPtr(tmp, addressReg), value);
            prepareWrite(value);
            writeHost(width, createMemPtr(tmp, addressReg), value);
            return value;
        }        
    }
#endif
    RegPtr offsetReg;    

    shrValueWithDest(JitWidth::b32, tmp, addressReg, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    offsetReg = addressReg;
    andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);

    if (width != JitWidth::b8) {
#ifdef BOXEDWINE_HOST_EXCEPTIONS
        if (KSystem::canJitUse4KPage) {
#ifdef _DEBUG
            writeCurrentEip(0);
#endif
            if (currentOp->exceptionCount == MAX_OP_EXCEPTION_COUNT) {
                clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
            }
        } else 
#endif
        {
            // make sure we only use the fast path if the entire read will take place on the same page
            clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
        }
    }

    // if read/write
    RegPtr tmpReg2 = getTmpRegForCallResult();
    andValueWithDest(JitWidth::b32, tmpReg2, tmp, 3);

    IfNotEqual(JitWidth::b32, tmpReg2, 3); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);
    readHost(width, createMemPtr(tmp, offsetReg, 0, 0), tmpReg2);

    prepareWrite(tmpReg2);

    writeHost(width, createMemPtr(tmp, offsetReg, 0, 0), tmpReg2);
    return tmpReg2;
}

RegPtr JitCodeGen::getFlagDestReadOnly(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(dst.u32), result);
}

RegPtr JitCodeGen::getFlagDestTmp(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(dst.u32), result);
}

RegPtr JitCodeGen::getFlagSrcTmp(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(src.u32), result);
}


RegPtr JitCodeGen::getFlagSrcReadOnly(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(src.u32), result);
}

RegPtr JitCodeGen::getFlagResultReadOnly(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(result.u32), result);
}

RegPtr JitCodeGen::getFlagResultTmp(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(result.u32), result);
}

RegPtr JitCodeGen::getFlagsInTmp(RegPtr reg) {
    if (!reg) {
        reg = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(flags), reg);
}

RegPtr JitCodeGen::getFlagCF(RegPtr result) {
    if (!result) {
        result = getTmpReg8();
    }
    return readCPU(JitWidth::b32, CPU_OFFSET_OF(oldCF), result);
}

RegPtr JitCodeGen::getLazyFlagType() {
    RegPtr result = getTmpReg8();
    xorReg(JitWidth::b32, result, result);
    return readCPU(JitWidth::b8, CPU_OFFSET_OF(lazyFlagType), result);
}

RegPtr JitCodeGen::getLazyFlagTypeInTmp() {
    RegPtr result = getTmpReg8();
    xorReg(JitWidth::b32, result, result);
    return readCPU(JitWidth::b8, CPU_OFFSET_OF(lazyFlagType), result);
}

U8* JitCodeGen::createCalculationCF(LazyFlagType flags) {
    RegPtr result = getConditionCalculationReg();
    getCF(flags, result);
    nakedReturn();
    return createDynamicExecutableMemory();
}

RegPtr JitCodeGen::getStringRegEcx() {
    return getReg(1, 1);
}

RegPtr JitCodeGen::getStringRegEsi() {
    return getReg(6, 6);
}

RegPtr JitCodeGen::getStringRegEdi() {
    return getReg(7, 2);
}

void JitCodeGen::getCF(LazyFlagType flags, RegPtr result) {
    if (flags == FLAGS_NONE || flags == FLAGS_CFOF) {
        andValueWithDest(JitWidth::b32, result, getReadOnlyFlags(result), CF);
    } else if (flags == FLAGS_ADD8) {
        // cpu->result.u8<cpu->dst.u8;
        compareReg(JitWidth::b8, getFlagResultReadOnly(result), getFlagDestReadOnly(getConditionCalculationReg(1)), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_ADD16) {
        // cpu->result.u16<cpu->dst.u16;
        compareReg(JitWidth::b16, getFlagResultReadOnly(result), getFlagDestReadOnly(getConditionCalculationReg(1)), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_ADD32) {
        // cpu->result.u32<cpu->dst.u32;
        compareReg(JitWidth::b32, getFlagResultReadOnly(result), getFlagDestReadOnly(getConditionCalculationReg(1)), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_OR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_OR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_OR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_ADC8) {
        // (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8));
        RegPtr tmp = getConditionCalculationReg(2);
        RegPtr fResult = getFlagResultTmp(getConditionCalculationReg(1));
        RegPtr fDest = getFlagDestReadOnly(tmp);
        compareReg(JitWidth::b8, fResult, fDest, JitEvaluate::LESS_THAN_UNSIGNED, result);
        compareReg(JitWidth::b8, fResult, fDest, JitEvaluate::EQUALS, fResult);
        andReg(JitWidth::b32, fResult, getFlagCF(tmp));
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_ADC16) {
        // (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));
        RegPtr tmp = getConditionCalculationReg(2);
        RegPtr fResult = getFlagResultTmp(getConditionCalculationReg(1));
        RegPtr fDest = getFlagDestReadOnly(tmp);
        compareReg(JitWidth::b16, fResult, fDest, JitEvaluate::LESS_THAN_UNSIGNED, result);
        compareReg(JitWidth::b16, fResult, fDest, JitEvaluate::EQUALS, fResult);
        andReg(JitWidth::b32, fResult, getFlagCF(tmp));
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_ADC32) {
        // (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));
        RegPtr tmp = getConditionCalculationReg(2);
        RegPtr fResult = getFlagResultTmp(getConditionCalculationReg(1));
        RegPtr fDest = getFlagDestReadOnly(tmp);
        compareReg(JitWidth::b32, fResult, fDest, JitEvaluate::LESS_THAN_UNSIGNED, result);
        compareReg(JitWidth::b32, fResult, fDest, JitEvaluate::EQUALS, fResult);
        andReg(JitWidth::b32, fResult, getFlagCF(tmp));
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_SBB8) {
        // (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8==0xff));
        compareReg(JitWidth::b8, getFlagDestReadOnly(getConditionCalculationReg(1)), getFlagResultReadOnly(getConditionCalculationReg(2)), JitEvaluate::LESS_THAN_UNSIGNED, result);
        RegPtr tmp = getConditionCalculationReg(1);
        RegPtr tmp2 = getConditionCalculationReg(2);
        movValue(JitWidth::b32, tmp2, 0xff);
        compareReg(JitWidth::b8, getFlagSrcReadOnly(tmp), tmp2, JitEvaluate::EQUALS, tmp);
        andReg(JitWidth::b32, tmp, getFlagCF(tmp2));
        orReg(JitWidth::b32, result, tmp);
    } else if (flags == FLAGS_SBB16) {
        // (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16==0xffff));
        compareReg(JitWidth::b16, getFlagDestReadOnly(getConditionCalculationReg(1)), getFlagResultReadOnly(getConditionCalculationReg(2)), JitEvaluate::LESS_THAN_UNSIGNED, result);
        RegPtr tmp = getConditionCalculationReg(1);
        RegPtr tmp2 = getConditionCalculationReg(2);
        movValue(JitWidth::b32, tmp2, 0xffff);
        compareReg(JitWidth::b16, getFlagSrcReadOnly(tmp), tmp2, JitEvaluate::EQUALS, tmp);
        andReg(JitWidth::b32, tmp, getFlagCF(tmp2));
        orReg(JitWidth::b32, result, tmp);
    } else if (flags == FLAGS_SBB32) {
        // (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32==0xffffffff));
        compareReg(JitWidth::b32, getFlagDestReadOnly(getConditionCalculationReg(1)), getFlagResultReadOnly(getConditionCalculationReg(2)), JitEvaluate::LESS_THAN_UNSIGNED, result);
        RegPtr tmp = getConditionCalculationReg(1);
        RegPtr tmp2 = getConditionCalculationReg(2);
        movValue(JitWidth::b32, tmp2, 0xffffffff);
        compareReg(JitWidth::b32, getFlagSrcReadOnly(tmp), tmp2, JitEvaluate::EQUALS, tmp);
        andReg(JitWidth::b32, tmp, getFlagCF(tmp2));
        orReg(JitWidth::b32, result, tmp);
    } else if (flags == FLAGS_AND8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_SUB8 || flags == FLAGS_CMP8) {
        // cpu->dst.u8<cpu->src.u8;
        compareReg(JitWidth::b8, getFlagDestReadOnly(getConditionCalculationReg(1)), getFlagSrcReadOnly(getConditionCalculationReg(2)), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_SUB16 || flags == FLAGS_CMP16) {
        // cpu->dst.u16<cpu->src.u16;
        compareReg(JitWidth::b16, getFlagDestReadOnly(getConditionCalculationReg(1)), getFlagSrcReadOnly(getConditionCalculationReg(2)), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_SUB32 || flags == FLAGS_CMP32) {
        // cpu->dst.u32<cpu->src.u32;
        compareReg(JitWidth::b32, getFlagDestReadOnly(getConditionCalculationReg(1)), getFlagSrcReadOnly(getConditionCalculationReg(2)), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_XOR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_XOR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_XOR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_INC8) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_INC16) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_INC32) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_DEC8) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_DEC16) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_DEC32) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_SHL8) {
        // last bit that was shifted out
        // ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        movValue(JitWidth::b32, reg, 8);
        subReg(JitWidth::b32, reg, getFlagSrcReadOnly(getConditionCalculationReg(1)));
        shrReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SHL16 || flags == FLAGS_DSHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        movValue(JitWidth::b32, reg, 16);
        subReg(JitWidth::b32, reg, getFlagSrcReadOnly(getConditionCalculationReg(1)));
        shrReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SHL32 || flags == FLAGS_DSHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        movValue(JitWidth::b32, reg, 32);
        subReg(JitWidth::b32, reg, getFlagSrcReadOnly(getConditionCalculationReg(1)));
        shrReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SHR8 || flags == FLAGS_SHR16 || flags == FLAGS_SHR32) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        subValueWithDest(JitWidth::b32, reg, getFlagSrcReadOnly(reg), 1);
        shrReg(getWidthOfFlags(flags), result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_DSHR16 || flags == FLAGS_DSHR32) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        subValueWithDest(JitWidth::b32, reg, getFlagSrcReadOnly(reg), 1);
        shrReg(JitWidth::b32, result, reg); // intentional 32-bit
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SAR8) {
        // (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        subValueWithDest(JitWidth::b32, reg, getFlagSrcReadOnly(reg), 1);
        sarReg(JitWidth::b8, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SAR16) {
        // (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        subValueWithDest(JitWidth::b32, reg, getFlagSrcReadOnly(reg), 1);
        sarReg(JitWidth::b16, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SAR32) {
        // (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getConditionCalculationReg(2);
        subValueWithDest(JitWidth::b32, reg, getFlagSrcReadOnly(reg), 1);
        sarReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_TEST8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8!=0;
        compareValue(JitWidth::b8, getFlagSrcReadOnly(getConditionCalculationReg(1)), 0, JitEvaluate::NOT_EQUALS, result);
    } else if (flags == FLAGS_NEG16) {
        // cpu->src.u16!=0;
        compareValue(JitWidth::b16, getFlagSrcReadOnly(getConditionCalculationReg(1)), 0, JitEvaluate::NOT_EQUALS, result);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32!=0;
        compareValue(JitWidth::b32, getFlagSrcReadOnly(getConditionCalculationReg(1)), 0, JitEvaluate::NOT_EQUALS, result);
    } else {
        kpanic("getCF unknown flags");
    }    
}

RegPtr JitCodeGen::getCF() {
    RegPtr tmpReg = getConditionCalculationReg();
    RegPtr flagsType = getLazyFlagType();

    // make sure these tmps are not in use by checking if they are available
    {
        getConditionCalculationReg(1);
        getConditionCalculationReg(2);
    }    

    if (currentLazyFlags != FLAGS_NULL) {
        IfEqual(JitWidth::b32, flagsType, currentLazyFlags); {
            getCF(currentLazyFlags, tmpReg);
        } StartElse();
    }    
    readCPU(DYN_PTR, flagsType, DYN_PTR_LSL, offsetof(CPU, calculateCF), tmpReg);
    nakedCall(tmpReg);
    if (currentLazyFlags != FLAGS_NULL) {
        EndIf();
    }
    return tmpReg;
}

RegPtr JitCodeGen::getCondition(JitConditional condition, RegPtr resultReg) {
    if (currentLazyFlags == FLAGS_NULL || currentLazyFlags == FLAGS_NONE) {
        if (!resultReg) {
            resultReg = getTmpReg();
        }
        RegPtr flags = getLazyFlagType();
        IfNot(JitWidth::b32, std::move(flags));
        getFlagsInTmp(resultReg);
        switch (condition) {
        case JitConditional::NO:
            shrValue(JitWidth::b32, resultReg, 11);
            andValue(JitWidth::b32, resultReg, 1);
            xorValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::O:
            shrValue(JitWidth::b32, resultReg, 11);
            andValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::NB:
            andValue(JitWidth::b32, resultReg, 1);
            xorValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::B:
            andValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::Z:
            shrValue(JitWidth::b32, resultReg, 6);
            andValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::NZ:
            shrValue(JitWidth::b32, resultReg, 6);
            andValue(JitWidth::b32, resultReg, 1);
            xorValue(JitWidth::b32, resultReg, 1);
            break;
        
        case JitConditional::BE:
        case JitConditional::NBE:
            {
                RegPtr tmp = getTmpReg();
                // CF
                andValueWithDest(JitWidth::b32, tmp, resultReg, 1);

                // ZF
                shrValue(JitWidth::b32, resultReg, 6);
                andValue(JitWidth::b32, resultReg, 1);

                orReg(JitWidth::b32, resultReg, tmp);
                if (condition == JitConditional::NBE) {
                    xorValue(JitWidth::b32, resultReg, 1);
                }
            }
            break;
        case JitConditional::NS:
            shrValue(JitWidth::b32, resultReg, 7);
            andValue(JitWidth::b32, resultReg, 1);
            xorValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::S:
            shrValue(JitWidth::b32, resultReg, 7);
            andValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::NP:
            shrValue(JitWidth::b32, resultReg, 2);
            andValue(JitWidth::b32, resultReg, 1);
            xorValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::P:
            shrValue(JitWidth::b32, resultReg, 2);
            andValue(JitWidth::b32, resultReg, 1);
            break;
        case JitConditional::NL:
        case JitConditional::L:
        case JitConditional::NLE:
        case JitConditional::LE:
            {
                RegPtr tmp = getTmpReg();
                
                // OF
                shrValueWithDest(JitWidth::b32, tmp, resultReg, 11);
                andValue(JitWidth::b32, tmp, 1);

                // SF
                shrValue(JitWidth::b32, resultReg, 7);
                andValue(JitWidth::b32, resultReg, 1);                

                if (condition == JitConditional::NL) {
                    // SF == OF
                    xorReg(JitWidth::b32, resultReg, tmp);
                    xorValue(JitWidth::b32, resultReg, 1);
                } else if (condition == JitConditional::L) {
                    xorReg(JitWidth::b32, resultReg, tmp);
                } else if (condition == JitConditional::NLE) {
                    xorReg(JitWidth::b32, resultReg, tmp);
                    xorValue(JitWidth::b32, resultReg, 1);
                    getFlagsInTmp(tmp);
                    // ZF
                    shrValue(JitWidth::b32, tmp, 6);
                    andValue(JitWidth::b32, tmp, 1);
                    // !ZF
                    xorValue(JitWidth::b32, tmp, 1);
                    // !ZF && SF == OF
                    andReg(JitWidth::b32, resultReg, tmp);

                } else if (condition == JitConditional::LE) {
                    xorReg(JitWidth::b32, resultReg, tmp);
                    getFlagsInTmp(tmp);
                    // ZF
                    shrValue(JitWidth::b32, tmp, 6);
                    andValue(JitWidth::b32, tmp, 1);

                    // ZF || SF != OF
                    orReg(JitWidth::b32, resultReg, tmp);
                }
            }
            break;
        // no default, should get compiler error if not all enum cases handled
        }
        StartElse(); {
            callGetCondition(condition, resultReg);
        } EndIf();
        return resultReg;
    }
    return calculateCondition(condition, resultReg);
}

RegPtr JitCodeGen::calculateCondition(JitConditional condition, RegPtr resultReg) {
    if (!resultReg) {
        resultReg = getTmpReg8();
    }
    U32 flagWidth = lazyFlags[currentLazyFlags] ? lazyFlags[currentLazyFlags]->width : 0;
    IfEqual(JitWidth::b32, getLazyFlagType(), currentLazyFlags);
    switch (condition) {
    case JitConditional::NO:
        genOF(currentLazyFlags, resultReg);
        xorValue(JitWidth::b32, resultReg, 1);
        break;
    case JitConditional::O:
        genOF(currentLazyFlags, resultReg);
        break;
    case JitConditional::NB:
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_EQUAL_UNSIGNED, resultReg);
        } else {
            genCF(currentLazyFlags, resultReg);
            xorValue(JitWidth::b32, resultReg, 1);
        }
        break;
    case JitConditional::B:
        genCF(currentLazyFlags, resultReg);
        break;
    case JitConditional::Z:
        compareValue(getWidthOfFlags(currentLazyFlags), getFlagResultTmp(resultReg), 0, JitEvaluate::EQUALS, resultReg);
        break;
    case JitConditional::NZ:
        compareValue(getWidthOfFlags(currentLazyFlags), getFlagResultTmp(resultReg), 0, JitEvaluate::NOT_EQUALS, resultReg);
        break;
    case JitConditional::NBE:
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_UNSIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_UNSIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_UNSIGNED, resultReg);
        } else {
            // return (!cpu->getZF() && !cpu->getCF()) ? 1 : 0;
            If(getWidthOfFlags(currentLazyFlags), getFlagResultTmp(resultReg)); { // if, so that we can skip genCF
                genCF(currentLazyFlags, resultReg);
                xorValue(JitWidth::b32, resultReg, 1);
            } StartElse(); {
                movValue(JitWidth::b32, resultReg, 0);
            } EndIf();
        }
        break;
    case JitConditional::BE: {
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_EQUAL_UNSIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_EQUAL_UNSIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_EQUAL_UNSIGNED, resultReg);
        } else {
            // return (cpu->getZF() || cpu->getCF()) ? 1 : 0;
            IfNot(getWidthOfFlags(currentLazyFlags), getFlagResultTmp(resultReg)); { // if, so that we can skip genCF
                movValue(JitWidth::b32, resultReg, 1);
            } StartElse(); {
                genCF(currentLazyFlags, resultReg);
            } EndIf();
        }
        break;
    }
    case JitConditional::NS:
        getFlagResultTmp(resultReg);
        shrValue(getWidthOfFlags(currentLazyFlags), resultReg, flagWidth - 1);
        if (flagWidth != 32) {
            // result needs to be a 32-bit 0 or 1
            movzx(JitWidth::b32, resultReg, getWidthOfFlags(currentLazyFlags), resultReg);
        }
        xorValue(JitWidth::b32, resultReg, 1);
        break;
    case JitConditional::S:
        getFlagResultTmp(resultReg);
        shrValue(getWidthOfFlags(currentLazyFlags), resultReg, flagWidth - 1);
        if (flagWidth != 32) {
            // result needs to be a 32-bit 0 or 1
            movzx(JitWidth::b32, resultReg, getWidthOfFlags(currentLazyFlags), resultReg);
        }
        break;
    case JitConditional::NP:
        genPF(resultReg);
        xorValue(JitWidth::b32, resultReg, 1);
        break;
    case JitConditional::P:
        genPF(resultReg);
        break;
    case JitConditional::NL:
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_EQUAL_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_EQUAL_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_EQUAL_SIGNED, resultReg);
        } else {
            // return (cpu->getSF()==cpu->getOF()) ? 1 : 0;
            RegPtr of = getTmpReg8();
            genOF(currentLazyFlags, of);
            getFlagResultTmp(resultReg);
            shrValue(getWidthOfFlags(currentLazyFlags), resultReg, flagWidth - 1); // resultReg will be 1 if SF
            // 8-bit compare, because above shr might not be 32-bit and we only care about the bottom bit anyway
            compareReg(JitWidth::b8, resultReg, of, JitEvaluate::EQUALS, resultReg);
        }
        break;
    case JitConditional::L: {
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_SIGNED, resultReg);
        } else {
            // return (cpu->getSF()!=cpu->getOF()) ? 1 : 0;
            RegPtr of = getTmpReg8();
            genOF(currentLazyFlags, of);
            getFlagResultTmp(resultReg);
            shrValue(getWidthOfFlags(currentLazyFlags), resultReg, flagWidth - 1); // resultReg will be 1 if SF
            // 8-bit compare, because above shr might not be 32-bit and we only care about the bottom bit anyway
            compareReg(JitWidth::b8, resultReg, of, JitEvaluate::NOT_EQUALS, resultReg); // resultReg will be 1 if reg != of
        }
        break;
    }
    case JitConditional::NLE:
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::GREATER_THAN_SIGNED, resultReg);
        } else {
            // return (!cpu->getZF() && cpu->getSF()==cpu->getOF()) ? 1 : 0;
            RegPtr of = getTmpReg8();
            getFlagResultTmp(resultReg);
            if (flagWidth != 32) {
                movzx(JitWidth::b32, resultReg, getWidthOfFlags(currentLazyFlags), resultReg);
            }
            If(getWidthOfFlags(currentLazyFlags), resultReg); { // if 0, then exit if and resultReg is 0
                genOF(currentLazyFlags, of);
                shrValue(getWidthOfFlags(currentLazyFlags), resultReg, flagWidth - 1); // reg will be 1 if SF
                // 8-bit compare, because above sar might not be 32-bit and we only care about the bottom bit anyway
                compareReg(JitWidth::b8, resultReg, of, JitEvaluate::EQUALS, resultReg); // will be 1 if OF==SF
            } EndIf();
        }
        break;
    case JitConditional::LE: {
        if (currentLazyFlags == FLAGS_SUB8) {
            compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_EQUAL_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB16) {
            compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_EQUAL_SIGNED, resultReg);
        } else if (currentLazyFlags == FLAGS_SUB32) {
            compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_EQUAL_SIGNED, resultReg);
        } else {
            //callAndReturn(common_condition_le, resultReg);            
            // return (cpu->getZF() || cpu->getSF()!=cpu->getOF()) ? 1 : 0;
            RegPtr of = getTmpReg8();
            getFlagResultTmp(resultReg);
            if (flagWidth != 32) {
                movzx(JitWidth::b32, resultReg, getWidthOfFlags(currentLazyFlags), resultReg);
            }
            If(getWidthOfFlags(currentLazyFlags), resultReg); { // if 0, it will exit the if, the 0 reg will cause a true condition at the end because flip == true
                genOF(currentLazyFlags, of);
                shrValue(getWidthOfFlags(currentLazyFlags), resultReg, flagWidth - 1); // reg will be 1 if SF                
                // 8-bit compare, because above shr might not be 32-bit and we only care about the bottom bit anyway
                compareReg(JitWidth::b8, resultReg, of, JitEvaluate::EQUALS, resultReg); // reg will be 0 if SF != OF (which is what we want because of the next xor)
            } EndIf();
            xorValue(JitWidth::b32, resultReg, 1);
        }
        break;
    }
    // no default, should get compiler error if not all enum cases handled
    }
    StartElse(); {
        callGetCondition(condition, resultReg);
    } EndIf();
    return resultReg;
}

RegPtr JitCodeGen::callGetCondition(JitConditional condition, RegPtr resultReg) {
    switch (condition) {
    case JitConditional::O: return callAndReturn(common_condition_o, resultReg);
    case JitConditional::NO: return callAndReturn(common_condition_no, resultReg);
    case JitConditional::B: return callAndReturn(common_condition_b, resultReg);
    case JitConditional::NB: return callAndReturn(common_condition_nb, resultReg);
    case JitConditional::Z: return callAndReturn(common_condition_z, resultReg);
    case JitConditional::NZ: return callAndReturn(common_condition_nz, resultReg);
    case JitConditional::BE: return callAndReturn(common_condition_be, resultReg);
    case JitConditional::NBE: return callAndReturn(common_condition_nbe, resultReg);
    case JitConditional::S: return callAndReturn(common_condition_s, resultReg);
    case JitConditional::NS: return callAndReturn(common_condition_ns, resultReg);
    case JitConditional::P: return callAndReturn(common_condition_p, resultReg);
    case JitConditional::NP: return callAndReturn(common_condition_np, resultReg);
    case JitConditional::L: return callAndReturn(common_condition_l, resultReg);
    case JitConditional::NL: return callAndReturn(common_condition_nl, resultReg);
    case JitConditional::LE: return callAndReturn(common_condition_le, resultReg);
    case JitConditional::NLE: return callAndReturn(common_condition_nle, resultReg);
    // no default, should get compiler error if not all enum cases handled
    }
    return nullptr;
}

void JitCodeGen::IfCondition(JitConditional condition) {
    bool negative = false;
    RegPtr reg;
    preIfCondition(condition, negative, reg);
    if (negative) {
        IfNot(JitWidth::b32, reg); 
    } else {
        If(JitWidth::b32, reg);
    }
}

void JitCodeGen::preIfCondition(JitConditional condition, bool& negative, RegPtr& reg) {
    if (condition == JitConditional::Z || condition == JitConditional::NZ) {
        // hard to measure small difference, but it seems like this gives about a 1% improvement
        // 
        // for FLAGS_NONE, result will be 0 and flags will contain ZF
        // for other flags, result will contain non 0 if lazy result is 0 and flags will be 0

        RegPtr zfMask = getLazyFlagTypeInTmp();
        RegPtr result = getTmpReg8();
        bool needsEndIf = false;

        if (currentLazyFlags != FLAGS_NULL) {
            if (currentLazyFlags == FLAGS_NONE) {
                needsEndIf = true;
                IfEqual(JitWidth::b32, zfMask, currentLazyFlags); {
                    getFlagsInTmp(result);
                    andValue(JitWidth::b32, result, ZF);
                    xorValue(JitWidth::b32, result, ZF); // toggle: non-zero if ZF=0, zero if ZF=1
                } StartElse();
            } else {
                needsEndIf = true;
                IfEqual(JitWidth::b32, zfMask, currentLazyFlags); {                    
                    getFlagResultTmp(result);
                    if (getWidthOfFlags(currentLazyFlags) != JitWidth::b32) {
                        movzx(JitWidth::b32, result, getWidthOfFlags(currentLazyFlags), result);
                    }
                } StartElse();
            }
        }
        getFlagResultTmp(result);
        RegPtr flags = getFlagsInTmp();
        
        movzx(JitWidth::b32, zfMask, JitWidth::b8, zfMask);
        readCPU(JitWidth::b32, zfMask, 2, offsetof(CPU, flagZeroMask), zfMask);
        andReg(JitWidth::b32, result, zfMask); // if using result, we need to mask it to the width of lazyFlagType

        movsx(JitWidth::b32, zfMask, JitWidth::b8, zfMask); // for FLAGS_NONE, zfMask will be 0 else 0xffffffff
        xorValue(JitWidth::b32, zfMask, 0xffffffff); // for FLAGS_NONE, zfMask will be 0xffffffff else 0

        andValue(JitWidth::b32, flags, ZF); // if ZF, then flags will be set to 0x40
        xorValue(JitWidth::b32, flags, ZF);
        andReg(JitWidth::b32, flags, zfMask); // if not FLAGS_NONE, then flags will be 0

        orReg(JitWidth::b32, result, flags);

        if (needsEndIf) {
            EndIf();
        }
        if (condition == JitConditional::NZ) {
            reg = result;
        } else {
            reg = result;
            negative = true;
        }
        return;
    } else if (1 && (condition == JitConditional::S || condition == JitConditional::NS)) {
        // hard to measure small difference, but it seems like this gives about a 0%-0.5% performance increase

        RegPtr sfMask = getLazyFlagTypeInTmp();
        RegPtr result = getTmpReg8();
        bool needsEndIf = false;

        if (currentLazyFlags != FLAGS_NULL) {
            if (currentLazyFlags == FLAGS_NONE) {
                needsEndIf = true;
                // should almost always be true, so good for branch prediction
                IfEqual(JitWidth::b32, sfMask, currentLazyFlags); {
                    getFlagsInTmp(result);
                    andValue(JitWidth::b32, result, SF);
                } StartElse();
            } else {
                needsEndIf = true;
                JitWidth width = getWidthOfFlags(currentLazyFlags);
                // should almost always be true, so good for branch prediction
                IfEqual(JitWidth::b32, sfMask, currentLazyFlags); {
                    getFlagResultTmp(result);
                    if (width == JitWidth::b32) {
                        andValue(JitWidth::b32, result, 0x80000000);
                    } else if (width == JitWidth::b16) {
                        andValue(JitWidth::b32, result, 0x8000);
                    } else {
                        andValue(JitWidth::b32, result, 0x80);
                    }
                } StartElse();
            }
        }
        // not so good for branch prediction
        IfNot(JitWidth::b32, sfMask); {
            getFlagsInTmp(result);
            andValue(JitWidth::b32, result, SF);
        } StartElse(); {
            getFlagResultTmp(result);
            movzx(JitWidth::b32, sfMask, JitWidth::b8, sfMask);
            readCPU(JitWidth::b32, sfMask, 2, offsetof(CPU, flagSignMask), sfMask);
            andReg(JitWidth::b32, result, sfMask);
        } EndIf();

        if (needsEndIf) {
            EndIf();
        }
        if (condition == JitConditional::S) {
            reg = result;
        } else {
            reg = result;
            negative = true;            
        }
        return;
    }
    if (currentLazyFlags == FLAGS_NULL || currentLazyFlags == FLAGS_NONE) {
        reg = getCondition(condition);
        return;
    }
    reg = calculateCondition(condition);
}

// cpu->reg[op->rm] = cpu->reg[op->rm] + cpu->reg[op->reg]
// cpu->reg[op->reg] = cpu->reg[op->rm]
void JitCodeGen::xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
    if (reg->hardwareReg() == rm->hardwareReg()) {
        addReg(regWidth, rm, reg);
        return;
    }
    RegPtr tmp = getTmpReg();
    mov(regWidth, tmp, rm);
    addReg(regWidth, rm, reg);
    mov(regWidth, reg, tmp);
}

void JitCodeGen::incReg(JitWidth regWidth, RegPtr reg) {
    addValue(regWidth, reg, 1);
}

void JitCodeGen::decReg(JitWidth regWidth, RegPtr reg) {
    subValue(regWidth, reg, 1);
}

void JitCodeGen::IfDF() {
    IfTestBit(JitWidth::b32, getReadOnlyFlags(), 10);
}

void JitCodeGen::IfSmallStack() {
    RegPtr reg = readCPU(JitWidth::b32, offsetof(CPU, stackNotMask));
    If(JitWidth::b32, reg);
}

void JitCodeGen::addRegWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, RegPtr rm) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    addReg(regWidth, dst, rm);
}

void JitCodeGen::andValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    andValue(regWidth, dst, value);
}

void JitCodeGen::addValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    addValue(regWidth, dst, value);
}

void JitCodeGen::subValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    subValue(regWidth, dst, value);
}

void JitCodeGen::shlValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    shlValue(regWidth, dst, value);
}

void JitCodeGen::shrValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    shrValue(regWidth, dst, value);
}

void JitCodeGen::sarValueWithDest(JitWidth regWidth, RegPtr dst, RegPtr reg, U32 value) {
    if (dst->hardwareReg() != reg->hardwareReg()) {
        mov(regWidth, dst, reg);
    }
    sarValue(regWidth, dst, value);
}

#endif
