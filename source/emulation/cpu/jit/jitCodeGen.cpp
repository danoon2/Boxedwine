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
#include "jitCodeGen.h"
#include "../normal/normalCPU.h"
#include "jitFlags.h"

static JitCodeGen::OpFunction dynamicOps[NUMBER_OF_OPS];
static U32 dynamicOpsInitialized;

void JitCodeGen::onTestEnd(DecodedOp* op) {
    writeCPUValue(DYN_PTR, offsetof(CPU, nextOp), (DYN_PTR_SIZE)op);
}

static DecodedOp lastOp;

void OPCALL onLastOp(CPU* cpu, DecodedOp* op) {
}

static void initDynamicOps() {
    if (dynamicOpsInitialized)
        return;
    lastOp.pfn = onLastOp;
    if (offsetof(CPU, eip.u32) > 127)
        kpanic("initDynamicOps wasn't expecting eip offset to be greater than 127");

    if (offsetof(CPU, reg[8].u32) > 127)
        kpanic("initDynamicOps wasn't expecting reg[8] offset to be greater than 127");

    if (offsetof(CPU, seg[6].address) > 127)
        kpanic("initDynamicOps wasn't expecting reg[8] offset to be greater than 127");

    dynamicOpsInitialized = 1;
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
#ifdef __TEST
        if (nextOp && nextOp->inst == TestEnd) {
            eip += nextOp->len;
            break;
        }
#endif
        if (nextOp && !shouldContinueCompilingAfterOp(nextOp)) {
            break;
        }
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

void JitCodeGen::removeJIT(DecodedOp* op, U32 count) {
    for (U32 i = 0; i < count; i++) {
        if (op->blockStart) {
            removeJITBlock(op->blockStart);
        }
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
        this->emulatedLen += nextOp->len;
        this->blockOpCount++;
        this->eipToBufferPos.set(this->currentEip, markBufferLocation());
        if (nextOp->lock) {
            // so that intra block jumps that try to skip a lock will find the lock version of the op anyway
            this->eipToBufferPos.set(this->currentEip + 1, markBufferLocation());
        }
        preOp(nextOp);
        //movValue(JitWidth::b32, getTmpReg(), this->currentEip);
        (this->*dynamicOps[nextOp->inst])(nextOp);
        this->currentEip += nextOp->len;
        if (getIfJumpSize()) {
            kpanic_fmt("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
        }
        if (this->currentEip > this->lastOpEip) {
            if (!shouldContinueCompilingAfterOp(nextOp)) {
                writeCurrentEip(0);
                jumpEip();
            }
            break;
        } else {
            if (nextOp->next && nextOp->next->inst == Done) {
                if (!nextOp->isBranch()) { // f16 needs this
                    writeCurrentEip(0);
                }
            }
            nextOp = nextOp->next;
        }
    }
    return true;
}

void JitCodeGen::doJIT(U32 address, DecodedOp* op) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
    if (!cpu->thread->process->startJITOp) {
        JitCodeGen* jit = startNewJIT(cpu);
        cpu->thread->process->syncToHost = jit->createSyncToHost();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->syncFromHost = jit->createSyncFromHost();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->startJITOp = (OpCallback)jit->createStartJITCode();
        delete jit;            
        jit = startNewJIT(cpu);
        cpu->thread->process->emulateSingleOp = jit->createEmulateSingleOp();
        delete jit;
        jit = startNewJIT(cpu);
        cpu->thread->process->jumpToNextJIT = jit->createJumpEip();
        delete jit;
        createHelpers();
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
    writeCurrentEip(0);
    blockExit();
    commitJIT(op);
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    if (op->runCount == 0) {
#else
    // done check for long blocks that get broken up, affects f-22/f-16
    if (op->runCount == JIT_RUN_COUNT && op->inst != Done && !(op->flags & OP_FLAG_JIT)) {
#endif    
        startNewJIT(cpu, cpu->getEipAddress(), op);
    }
    op->runCount++;
    op->pfn(cpu, op);
}

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

bool JitCodeGen::isParamTypeReg(JitCallParamType paramType) {
    return paramType == JitCallParamType::REG_8 || paramType == JitCallParamType::REG_16 || paramType == JitCallParamType::REG_32;
}

void JitCodeGen::blockCall(DecodedOp* op) {
    blockNext1(op);
    if (lastOpEip > currentEip) {
        blockExit();
    }
}

void JitCodeGen::blockDoneCall() {
    blockDone(false);
}

void JitCodeGen::blockDone(bool returnEarly) {
    // cpu->nextOp = cpu->nextOp();
    jumpEip();
}

void JitCodeGen::jumpEip() {
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

void JitCodeGen::blockDoneJump() {
    jumpToEipIfCached();        
    RegPtr result = callAndReturnPtr(getJitFunctionForCurrentOp);
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
void JitCodeGen::blockNext1(DecodedOp* op) {
    // if (!(*(op->nextJump))) {
    //     *(op->nextJump) = cpu->getNextOp();
    // }
    // cpu->nextOp = *(op->nextJump);
    RegPtr opReg = getTmpReg();
    movValue(DYN_PTR, opReg, (DYN_PTR_SIZE)op);
    // ebx = op->nextJump
    // mov ebx, [edx + offsetof(DecodedOp, nextJump)]
    RegPtr nextJump = getTmpReg();
    read(DYN_PTR, nextJump, opReg, offsetof(DecodedOp, data.nextJump));
    opReg = nullptr;

    // eax = *(op->nextJump)
    RegPtr nextOp = getTmpReg();
    read(DYN_PTR, nextOp, nextJump, 0);
    // if (!(*(op->nextJump))) 
    IfNot(DYN_PTR, nextOp);
        // *(op->nextJump) = cpu->getNextOp();
        mov(DYN_PTR, nextOp, callAndReturnPtr(dynamic_getNextOp));
        write(DYN_PTR, nextJump, offsetof(DecodedOp, next), nextOp);
    EndIf();

    // cpu->nextOp = *(op->nextJump);
    writeCPU(DYN_PTR, offsetof(CPU, nextOp), nextOp);

#ifdef BOXEDWINE_MULTI_THREADED
    RegPtr jit = getTmpReg();
    read(DYN_PTR, jit, nextOp, offsetof(DecodedOp, pfnJitCode));
    If(DYN_PTR, jit);
        jmp(jit);
    EndIf();
#endif
    blockExit();
}

void JitCodeGen::blockNext2(DecodedOp* op) {
    // if (!op->next) { 
    //     op->next = cpu->getNextOp(); 
    // }
    // cpu->nextOp = op->next;

    RegPtr opReg = getTmpReg();
    movValue(DYN_PTR, opReg, (DYN_PTR_SIZE)op);

    // mov eax, [ebx + offsetof(DecodedOp, next)]
    RegPtr nextReg = getTmpReg();
    read(DYN_PTR, nextReg, opReg, offsetof(DecodedOp, next));

    IfNot(DYN_PTR, nextReg); {
        // op->next = cpu->getNextOp();
        mov(DYN_PTR, nextReg, callAndReturnPtr(dynamic_getNextOp));
        // mov [ebx + offsetof(DecodedOp, next)], eax
        write(DYN_PTR, opReg, offsetof(DecodedOp, next), nextReg);
    } EndIf();
    opReg = nullptr;

    // cpu->nextOp = op->next
    writeCPU(DYN_PTR, offsetof(CPU, nextOp), nextReg);

#ifdef BOXEDWINE_MULTI_THREADED
    RegPtr jit = getTmpReg();
    read(DYN_PTR, jit, nextReg, offsetof(DecodedOp, pfnJitCode));
    If(DYN_PTR, jit); {
        jmp(jit);
    } EndIf();
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

U8* JitCodeGen::createDynamicExecutableMemory() {
    U32 size = getBufferSize();
    U8* begin = (U8*)cpu->memory->allocCodeMemory(size);

    Platform::writeCodeToMemory(begin, size, [begin, size, this]() {
        copyBuffer(begin, size);
        patch(begin);
    });
    Platform::clearInstructionCache(begin, size);

    return begin;
}

void JitCodeGen::jumpToEipIfCached() {
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
    RegPtr eipReg = readEip();
    if (cpu->thread->process->hasSetSeg[CS]) {
        addReg(JitWidth::b32, eipReg, getReadOnlySegAddress(CS));
    }
    RegPtr pageReg = getTmpReg();
    mov(JitWidth::b32, pageReg, eipReg);
    shrValue(JitWidth::b32, pageReg, K_PAGE_SHIFT);

    RegPtr firstPageIndexReg = getTmpReg();
    mov(JitWidth::b32, firstPageIndexReg, pageReg);
    shrValue(JitWidth::b32, firstPageIndexReg, 10);

#ifdef BOXEDWINE_64
#define CODE_CACHE_LSL 3
#else
#define CODE_CACHE_LSL 2
#endif
    RegPtr tmp = readCPU(DYN_PTR, offsetof(CPU, opCache));
    read(DYN_PTR, tmp, tmp, firstPageIndexReg, CODE_CACHE_LSL, 0); // :TODO: 3 on 64-bit system

    // tmp contains 2nd level of page op cache
    If(DYN_PTR, tmp);
        // page & 0x3ff
        andValue(JitWidth::b32, pageReg, 0x3ff);
        read(DYN_PTR, tmp, tmp, pageReg, CODE_CACHE_LSL, 0);// :TODO: 3 on 64-bit system
        // tmp contains page of DecodedOp*
        If(DYN_PTR, tmp);
            andValue(JitWidth::b32, eipReg, K_PAGE_MASK);
            read(DYN_PTR, tmp, tmp, eipReg, CODE_CACHE_LSL, 0); // :TODO: 3 on 64-bit system
            // tmp contains DecodedOp
            If(DYN_PTR, tmp);
                read(DYN_PTR, tmp, tmp, offsetof(DecodedOp, pfnJitCode));
                // tmp contains pfnJitCode
                If(DYN_PTR, tmp);
                    jmp(tmp);
                EndIf();
            EndIf();
        EndIf();
    EndIf();
#undef CODE_CACHE_LSL
 }

U8* JitCodeGen::createJumpEip() {
    jumpToEipIfCached();
    writeCPU(DYN_PTR, offsetof(CPU, nextOp), callAndReturnOp(common_getNextOp));
    blockExit();

    return createDynamicExecutableMemory();
}

void jitRunSingleOp(CPU* cpu) {
    DecodedOp* op = nullptr;
    try {
        op = cpu->getNextOp();
    } catch (...) {
        // at this point the previous getNextOp threw an exception and the eip is now pointing to the signal handler
        op = cpu->getNextOp();
    }

    if (op->inst != Int80 && op->inst != Int9B) {
        int ii = 0;
    }
    if (!op) {
        kpanic("jitRunSingleOp oops");
    }
    try {
        DecodedOp o = *op;        
        o.next = &lastOp;
        o.pfn = NormalCPU::getFunctionForOp(op);
        o.pfn(cpu, &o);        
    } catch (...) {
        // motorhead 3dfx will trigger this when pressing enter to start a new game
    }
    cpu->nextOp = cpu->getNextOp();
}

U8* JitCodeGen::createEmulateSingleOp() {
    std::vector<DynParam> params;
    params.push_back(DynParam(JitCallParamType::CPU));
    callHostFunction(jitRunSingleOp, params, false);    
    blockExit(false);

    return createDynamicExecutableMemory();
}

void JitCodeGen::commitJIT(DecodedOp* op) {
    preCommitJIT();

    if (!blockOpCount) {
        return;
    }

    U8* begin = createDynamicExecutableMemory();

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
        bufferIndex = getBufferLocation(bufferIndex);
        nextOp->pfnJitCode = (OpCallback)(begin + bufferIndex);
        nextOp->pfn = cpu->thread->process->startJITOp;
        nextOp->flags |= OP_FLAG_JIT;
        nextOp->blockStart = op;
        address += nextOp->len;
        last = nextOp;
        nextOp = nextOp->next;
    }
}

static JitWidth getWidthOfCondition(LazyFlagType flags) {
    U32 width = lazyFlags[flags]->width;
    if (width == 32) {
        return JitWidth::b32;
    }
    if (width == 16) {
        return JitWidth::b16;
    }
    if (width == 8) {
        return JitWidth::b8;
    }
    kpanic_fmt("getWidthOfCondition: invalid flag width: %d", width);
    return JitWidth::b32;
}

RegPtr JitCodeGen::getZF() {
    RegPtr result = getTmpReg8();
    RegPtr lazyFlags = getLazyFlagType();

    if (currentLazyFlags != FLAGS_NULL && currentLazyFlags != FLAGS_NONE) {
        IfEqual(JitWidth::b32, lazyFlags, currentLazyFlags); {
            readCPU(JitWidth::b32, offsetof(CPU, result.u32), result);
            If(getWidthOfCondition(currentLazyFlags), result); {
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

void JitCodeGen::fillFlags() {
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

static U32 readd2(CPU* cpu, U32 address) {
    return cpu->memory->readd(address);
}

static U32 readw2(CPU* cpu, U32 address) {
    return cpu->memory->readw(address);
}

static U32 readb2(CPU* cpu, U32 address) {
    return cpu->memory->readb(address);
}

static void writed2(CPU* cpu, U32 address, U32 value) {
    cpu->memory->writed(address, value);
}

static void writew2(CPU* cpu, U32 address, U32 value) {
    cpu->memory->writew(address, (U16)value);
}

static void writeb2(CPU* cpu, U32 address, U32 value) {
    cpu->memory->writeb(address, (U8)value);
}

RegPtr JitCodeGen::read(JitWidth width, RegPtr addressReg, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp, std::function<void()> failedMemoryOp, RegPtr tmp, bool checkAlignment) {
    if (!tmp) {
        tmp = getTmpRegForCallResult();
    }    
    RegPtr offsetReg;

    mov(JitWidth::b32, tmp, addressReg);
    shrValue(JitWidth::b32, tmp, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
    } else {
        offsetReg = getTmpReg();
        mov(JitWidth::b32, offsetReg, addressReg);
    }
    andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);

    if (width != JitWidth::b8 && checkAlignment) {
        // make sure we only use the fast path if the entire read will take place on the same page
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
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
        customMemoryOp(tmp, std::move(offsetReg));
    } else {
        // mov eax, [eax+reg]
        read(width, tmp, tmp, offsetReg, 0, 0);
    }

    return tmp;
}

RegPtr JitCodeGen::read(JitWidth width, U32 address) {
    RegPtr tmp = getTmpReg8();

    if (width == JitWidth::b16) {
        if ((address & 0xFFF) == 0xFFF) {
            emulateSingleOp();
            return tmp;
        }
    } else if (width == JitWidth::b32) {
        if ((address & 0xFFF) >= 0xFFD) {
            emulateSingleOp();
            return tmp;
        }
    } else if (width != JitWidth::b8) {
        kpanic_fmt("JitCodeGen::read unknown width %d", (U32)width);
    }
    
    readMMU(tmp, address >> K_PAGE_SHIFT);

    IfNotTestBit(JitWidth::b32, tmp, 0); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);

    read(width, tmp, tmp,  address & K_PAGE_MASK);

    return tmp;
}

void JitCodeGen::write(JitWidth width, U32 address, RegPtr src) {
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
    RegPtr tmp = getTmpReg8();
    readMMU(tmp, address >> K_PAGE_SHIFT);

    IfNotTestBit(JitWidth::b32, tmp, 1); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);

    write(width, tmp, address & K_PAGE_MASK, src);
}

void JitCodeGen::write(JitWidth width, RegPtr addressReg, RegPtr src, std::function<void(RegPtr address, RegPtr offset)> customMemoryOp, std::function<void()> failedMemoryOp, bool checkAlignment) {
    RegPtr tmp = getTmpReg();

    RegPtr offsetReg;

    mov(JitWidth::b32, tmp, addressReg);
    shrValue(JitWidth::b32, tmp, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    bool pushedAddress = false;
    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
    } else if (isTmpRegAvailable()) {
        offsetReg = getTmpReg();
        mov(JitWidth::b32, offsetReg, addressReg);
    } else {
        pushedAddress = true;
        pushReg(addressReg);
        offsetReg = addressReg;
    }
    andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);

    if (width != JitWidth::b8 && checkAlignment) {
        // make sure we only use the fast path if the entire read will take place on the same page
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
    }

    IfNotTestBit(JitWidth::b32, tmp, 1); {
        if (pushedAddress) {
            popReg(addressReg);
        }
        if (failedMemoryOp) {
            failedMemoryOp();
        } else {
            emulateSingleOp();
        }
    } EndIf();

    andValueNative(tmp, ~0xfff);

    if (customMemoryOp) {
        customMemoryOp(std::move(tmp), std::move(offsetReg));
    } else {
        write(width, tmp, offsetReg, 0, 0, src);
    }
    if (pushedAddress) {
        popReg(addressReg);
    }
}

void JitCodeGen::writeValue(JitWidth width, RegPtr addressReg, U32 imm) {
    RegPtr tmp = getTmpReg();

    RegPtr offsetReg;

    mov(JitWidth::b32, tmp, addressReg);
    shrValue(JitWidth::b32, tmp, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    if (addressReg.use_count() == 1) {
        offsetReg = addressReg;
    } else {
        offsetReg = getTmpReg();
        mov(JitWidth::b32, offsetReg, addressReg);
    }
    andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);

    if (width != JitWidth::b8) {
        // make sure we only use the fast path if the entire read will take place on the same page
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
    }

    IfNotTestBit(JitWidth::b32, tmp, 1); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);

    write(width, tmp, offsetReg, 0, 0, imm);
}

RegPtr JitCodeGen::readWriteMem(JitWidth width, RegPtr addressReg, std::function<void(RegPtr value)> prepareWrite, S8 hint) {
    RegPtr offsetReg;
    RegPtr tmp = getTmpRegWithHint(hint);

    mov(JitWidth::b32, tmp, addressReg);
    shrValue(JitWidth::b32, tmp, K_PAGE_SHIFT);
    readMMU(tmp, tmp);

    offsetReg = addressReg;
    andValue(JitWidth::b32, offsetReg, K_PAGE_MASK);

    if (width != JitWidth::b8) {
        // make sure we only use the fast path if the entire read will take place on the same page
        clearMMUPermissionIfSpansPage(width, offsetReg, tmp);
    }

    // if read/write
    RegPtr tmpReg2 = getTmpRegForCallResult();
    mov(JitWidth::b32, tmpReg2, tmp);
    andValue(JitWidth::b32, tmpReg2, 3);

    IfNotEqual(JitWidth::b32, tmpReg2, 3); {
        emulateSingleOp();
    } EndIf();

    andValueNative(tmp, ~0xfff);
    read(JitWidth::b32, tmpReg2, tmp, offsetReg, 0, 0);

    prepareWrite(tmpReg2);

    write(JitWidth::b32, tmp, offsetReg, 0, 0, tmpReg2);
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

RegPtr JitCodeGen::getCF() {
    if (currentLazyFlags != FLAGS_NULL) {
        RegPtr flags = getLazyFlagTypeInTmp();
        IfEqual(JitWidth::b32, flags, currentLazyFlags); {
            genCF(currentLazyFlags, flags);
        } StartElse(); {
            callAndReturn(common_getCF, flags);
        } EndIf();
        return flags;
    } else {
        return callAndReturn(common_getCF);
    }
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
                mov(JitWidth::b32, tmp, resultReg);
                andValue(JitWidth::b32, tmp, 1);

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
                mov(JitWidth::b32, tmp, resultReg);
                shrValue(JitWidth::b32, tmp, 11);
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
        compareValue(getWidthOfFlags(currentLazyFlags), getFlagResultReadOnly(resultReg), 0, JitEvaluate::EQUALS, resultReg);
        break;
    case JitConditional::NZ:
        compareValue(getWidthOfFlags(currentLazyFlags), getFlagResultReadOnly(resultReg), 0, JitEvaluate::NOT_EQUALS, resultReg);
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
            If(getWidthOfFlags(currentLazyFlags), getFlagResultReadOnly(resultReg)); { // if, so that we can skip genCF
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
            IfNot(getWidthOfFlags(currentLazyFlags), getFlagResultReadOnly(resultReg)); { // if, so that we can skip genCF
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
                    xorValue(JitWidth::b32, result, ZF);
                } StartElse();
            } else {
                needsEndIf = true;
                IfEqual(JitWidth::b32, zfMask, currentLazyFlags); {
                    if (getWidthOfFlags(currentLazyFlags) != JitWidth::b32) {
                        xorReg(JitWidth::b32, result, result);
                    }
                    readCPU(getWidthOfFlags(currentLazyFlags), CPU_OFFSET_OF(result.u32), result);
                } StartElse();
            }
        }
        getFlagResultReadOnly(result);
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
            If(JitWidth::b32, result);
        } else {
            IfNot(JitWidth::b32, result);
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
                    getFlagResultReadOnly(result);
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
            readCPU(JitWidth::b32, CPU_OFFSET_OF(result.u32), result);
            movzx(JitWidth::b32, sfMask, JitWidth::b8, sfMask);
            readCPU(JitWidth::b32, sfMask, 2, offsetof(CPU, flagSignMask), sfMask);
            andReg(JitWidth::b32, result, sfMask);
        } EndIf();

        if (needsEndIf) {
            EndIf();
        }
        if (condition == JitConditional::S) {
            If(JitWidth::b32, result);
        } else {
            IfNot(JitWidth::b32, result);
        }
        return;
    }
    if (currentLazyFlags == FLAGS_NULL || currentLazyFlags == FLAGS_NONE) {
        If(JitWidth::b32, getCondition(condition));
        return;
    }
    If(JitWidth::b32, calculateCondition(condition));
}

// cpu->reg[op->rm] = cpu->reg[op->rm] + cpu->reg[op->reg]
// cpu->reg[op->reg] = cpu->reg[op->rm]
void JitCodeGen::xaddReg(JitWidth regWidth, RegPtr reg, RegPtr rm) {
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

#endif