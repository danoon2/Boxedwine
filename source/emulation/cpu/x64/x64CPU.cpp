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

#ifdef BOXEDWINE_X64
#include "x64Ops.h"
#include "x64CPU.h"
#include "x64Asm.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"

CPU* CPU::allocCPU(KMemory* memory) {
    return new x64CPU(memory);
}

// hard to guage the benifit, seems like 1% to 3% with quake 2 and quake 3
bool x64CPU::hasBMI2 = true;
bool x64Intialized = false;

x64CPU::x64CPU(KMemory* memory) : BtCPU(memory), data1(this), data2(this) {
    if (!x64Intialized) {
        x64Intialized = true;
        x64CPU::hasBMI2 = platformHasBMI2();
    }
    largeAddressJumpInstruction = 0xCE24FF43;
    pageJumpInstruction = 0x0A8B4566;
    pageOffsetJumpInstruction = 0xCA148B4F;    
}

void x64CPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
    this->negSegAddress[index] = (U32)(-((S32)(this->seg[index].address)));
}

void x64CPU::restart() {
	for (int i = 0; i < 6; i++) {
		this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
	}
	this->exitToStartThreadLoop = true;
}

void* x64CPU::init() {
    X64Asm data(this);
    KMemoryData* mem = getMemData(memory);
    x64CPU* cpu = this;

    for (int i = 0; i < 6; i++) {
        this->negSegAddress[i] = (U32)(-((S32)(this->seg[i].address)));
    }

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);

	// will push 15 regs, since it is odd, it will balance rip being pushed on the stack and give use a 16-byte alignment
	data.saveNativeState(); // also sets HOST_CPU

    //data.writeToRegFromValue(HOST_CPU, true, (U64)this, 8);
    KMemoryData* memData = getMemData(memory);
    data.writeToRegFromValue(HOST_MEM_READ, true, (U64)memData->mmuReadPtrAdjusted, 8);
    data.setNativeFlags(this->flags, FMASK_TEST|DF);

    data.writeToRegFromValue(0, false, EAX, 4);
    data.writeToRegFromValue(1, false, ECX, 4);
    data.writeToRegFromValue(2, false, EDX, 4);
    data.writeToRegFromValue(3, false, EBX, 4);
    data.writeToRegFromValue(HOST_ESP, true, ESP, 4);
    data.writeToRegFromValue(5, false, EBP, 4);
    data.writeToRegFromValue(6, false, ESI, 4);
    data.writeToRegFromValue(7, false, EDI, 4);        
    
    data.calculatedEipLen = 1; // will force the long x64 chunk jump
    data.jmpAddress(cpu->eip.u32);
    void* result = data.commit(memory);
    //link(&data, chunk);
    this->pendingCodePages.clear();    

    if (!this->thread->process->returnToLoopAddress) {
        X64Asm returnData(this);
        returnData.restoreNativeState();
        returnData.write8(0xfc); // cld
        returnData.write8(0xc3); // retn
        this->thread->process->returnToLoopAddress = returnData.commit(memory);
    }
    this->returnToLoopAddress = this->thread->process->returnToLoopAddress;

    if (!this->thread->process->reTranslateChunkAddress) {
        X64Asm translateData(this);
        translateData.createCodeForRetranslateChunk();
        this->thread->process->reTranslateChunkAddress = translateData.commit(memory);
    }
    this->reTranslateChunkAddress = this->thread->process->reTranslateChunkAddress;
    if (!this->thread->process->syncToHostAddress) {
        X64Asm translateData(this);
        translateData.createCodeForSyncToHost();
        this->thread->process->syncToHostAddress = translateData.commit(memory);
    }
    this->syncToHostAddress = this->thread->process->syncToHostAddress;
    if (!this->thread->process->syncFromHostAddress) {
        X64Asm translateData(this);
        translateData.createCodeForSyncFromHost();
        this->thread->process->syncFromHostAddress = translateData.commit(memory);
    }
    this->syncFromHostAddress = this->thread->process->syncFromHostAddress;    
    if (!this->thread->process->doSingleOpAddress) {
        X64Asm translateData(this);
        translateData.createCodeForDoSingleOp();
        this->thread->process->doSingleOpAddress = translateData.commit(memory);
    }
    this->doSingleOpAddress = this->thread->process->doSingleOpAddress;
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        X64Asm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary(true);
        this->thread->process->jmpAndTranslateIfNecessary = translateData.commit(memory);
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
#ifdef BOXEDWINE_POSIX
    if (!this->thread->process->runSignalAddress) {
        X64Asm translateData(this);
        translateData.createCodeForRunSignal();
        this->thread->process->runSignalAddress = translateData.commit();
    }
#endif
    return result;
}

#ifdef __TEST
void x64CPU::postTestRun() {
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
    if (!thread->process->emulateFPU) 
#endif
    {
        for (int i = 0; i < 8; i++) {
            xmm[i].pi.u64[0] = fpuState.xmm[i].low;
            xmm[i].pi.u64[1] = fpuState.xmm[i].high;
        }        
#ifdef BOXEDWINE_USE_SSE_FOR_FPU
        if (fpu.isMMXInUse) 
#endif
        {
            U16 controlWord = fpuState.fcw;
            U16 statusWord = fpuState.fsw;
            U8 tag = fpuState.ftw;
            fpu.SetCW(controlWord);
            fpu.SetSW(statusWord);
            fpu.SetTagFromAbridged(tag);

            for (U32 i = 0; i < 8; i++) {
                U32 index = (i - fpu.GetTop()) & 7;
                fpu.LD80(i, fpuState.st_mm[index].low, (U16)fpuState.st_mm[index].high);
            }
        }
    }
}

void x64CPU::addReturnFromTest() {
    X64Asm data(this);
    data.addReturnFromTest();
    data.commit(memory);
}
#endif

void x64CPU::link(BtData* data, void* hostAddress) {
    for (auto& todoJump : data->todoJump) {
        U32 eip = seg[CS].address + todoJump.eip;
        U8* offset = (U8*)hostAddress + todoJump.bufferPos;

        U8* host = (U8*)hostAddress +  data->getHostOffsetFromEip(eip);
        data->write32Buffer(offset, (U32)(host - offset - 4));            
    }
}

void x64CPU::loadFxState(U32 inst) {
    for (U32 i = 0; i < 8; i++) {
        this->xmm[i].pd.u64[0] = this->fpuState.xmm[i].low;
        this->xmm[i].pd.u64[1] = this->fpuState.xmm[i].high;
    }
#ifdef BOXEDWINE_USE_SSE_FOR_FPU
    if (this->fpu.isMMXInUse) {
        klog("mmx");
        for (U32 i = 0; i < 8; i++) {
            this->fpu.ST80(i, (U64*)&this->fpuState.st_mm[i].low, (U64*)&this->fpuState.st_mm[i].high);
        }
    }
#else
    if (!this->thread->process->emulateFPU) {
        U16 controlWord = this->fpuState.fcw;
        U16 statusWord = this->fpuState.fsw;
        U8 tag = ((x64CPU*)this)->fpuState.ftw;
        this->fpu.SetCW(controlWord);
        this->fpu.SetSW(statusWord);
        this->fpu.SetTagFromAbridged(tag);

        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - this->fpu.GetTop()) & 7;
            fpu.LD80(i, this->fpuState.st_mm[index].low, (U16)this->fpuState.st_mm[index].high);
        }
    }
#endif
}

void x64CPU::saveToFxState(U32 inst) {
    for (U32 i = 0; i < 8; i++) {
        this->fpuState.xmm[i].low = this->xmm[i].pd.u64[0];
        this->fpuState.xmm[i].high = this->xmm[i].pd.u64[1];
    }
#ifdef BOXEDWINE_USE_SSE_FOR_FPU
    if (this->fpu.isMMXInUse) {
        for (U32 i = 0; i < 8; i++) {
            this->fpu.ST80(i, (U64*)&this->fpuState.st_mm[i].low, (U64*)&this->fpuState.st_mm[i].high);
        }
    }
#else
    if (!this->thread->process->emulateFPU) {
        this->fpuState.fcw = this->fpu.CW();
        this->fpuState.fsw = this->fpu.SW();
        this->fpuState.ftw = this->fpu.GetAbridgedTag(this);
        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - this->fpu.GetTop()) & 7;
            this->fpu.ST80(i, (U64*)&this->fpuState.st_mm[index].low, (U64*)&this->fpuState.st_mm[index].high);
        }
    }
#endif
}

// winfish seems to jump into the middle of an instruction which changes it from cmp to mov
void x64CPU::translateData(BtData* data, BtData* firstPass) {
    KMemoryData* mem = getMemData(memory);
  
    data->firstPass = firstPass;
    data->currentOp = nullptr;
    data->firstOp = nullptr;
    DecodedOp* prevOp = nullptr;

    while (1) { 
        U32 address = this->seg[CS].address + data->ip;
        data->currentOp = getOp(address, 0);
        if (prevOp && !prevOp->next) {
            prevOp->next = data->currentOp;
        }
        if (!data->firstOp) {
            data->firstOp = data->currentOp;
        }        
        void* hostAddress = data->currentOp->pfnJitCode;
        if (hostAddress) {
            data->jumpTo(data->ip);
            break;
        }
        data->mapAddress(address, data->bufferPos);
        data->translateInstruction();
        if (data->done || data->currentOp->inst == Invalid) {
            break;
        }
        if (data->stopAfterInstruction!=-1 && (int)data->ipAddressCount==data->stopAfterInstruction) {
            break;
        }
        data->resetForNewOp();
        prevOp = data->currentOp;
    }  
}

extern bool writesFlags[InstructionCount];

void common_runSingleOp(x64CPU* cpu) {
    cpu->updateFlagsFromX64();
    DecodedOp* op = cpu->getNextOp();
    bool deallocOp = false;

    if (op->flags & OP_FLAG_EMULATED_OP) {
        op = cpu->decodeOneOp(cpu->getEipAddress());
        deallocOp = true;
    } else if (!op) {
        kpanic("common_runSingleOp oops");
    }
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
    for (U32 i = 0; i < 8; i++) {
        cpu->xmm[i].pd.u64[0] = cpu->fpuState.xmm[i].low;
        cpu->xmm[i].pd.u64[1] = cpu->fpuState.xmm[i].high;
        if (cpu->fpu.isMMXInUse && cpu->thread->process->emulateFPU) {
            cpu->fpu.LD80(i, cpu->fpuState.st_mm[i].low, (U16)cpu->fpuState.st_mm[i].high);
        }
    }
    if (!cpu->thread->process->emulateFPU) {
        U16 controlWord = cpu->fpuState.fcw;
        U16 statusWord = cpu->fpuState.fsw;
        U8 tag = cpu->fpuState.ftw;
        cpu->fpu.SetCW(controlWord);
        cpu->fpu.SetSW(statusWord);
        cpu->fpu.SetTagFromAbridged(tag);

        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            cpu->fpu.LD80(i, cpu->fpuState.st_mm[index].low, (U16)cpu->fpuState.st_mm[index].high);
        }
    }
#endif
    if (writesFlags[op->inst]) {
        cpu->flags &= ~(SF | AF | ZF | PF | CF | OF);
        cpu->flags |= ((cpu->instructionStoredFlags >> 8) & 0xff) | ((cpu->instructionStoredFlags & 1) << 11);
    }
    try {
        op->pfn(cpu, op);
    } catch (...) {
        int ii = 0;
    }
#ifndef BOXEDWINE_USE_SSE_FOR_FPU
    for (U32 i = 0; i < 8; i++) {
        cpu->fpuState.xmm[i].low = cpu->xmm[i].pd.u64[0];
        cpu->fpuState.xmm[i].high = cpu->xmm[i].pd.u64[1];
        if (cpu->fpu.isMMXInUse && cpu->thread->process->emulateFPU) {
            cpu->fpu.ST80(i, (U64*)&cpu->fpuState.st_mm[i].low, (U64*)&cpu->fpuState.st_mm[i].high);
        }
    }
    if (!cpu->thread->process->emulateFPU) {
        cpu->fpuState.fcw = cpu->fpu.CW();
        cpu->fpuState.fsw = cpu->fpu.SW();
        cpu->fpuState.ftw = cpu->fpu.GetAbridgedTag(cpu);
        for (U32 i = 0; i < 8; i++) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            cpu->fpu.ST80(i, (U64*)&cpu->fpuState.st_mm[index].low, (U64*)&cpu->fpuState.st_mm[index].high);
        }
    }
#endif
    cpu->fillFlags();
    cpu->updateX64Flags();
    if (deallocOp) {
        op->dealloc();
    }
}
#endif
