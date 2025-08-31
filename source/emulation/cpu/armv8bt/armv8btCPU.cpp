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

#ifdef BOXEDWINE_ARMV8BT
#include "armv8btCPU.h"
#include "armv8btAsm.h"
#include "armv8btOps.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"

#undef u8

CPU* CPU::allocCPU(KMemory* memory) {
    return new Armv8btCPU(memory);
}

Armv8btCPU::Armv8btCPU(KMemory* memory) : BtCPU(memory), data1(this), data2(this) {
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE].pd.f64[0] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE].pd.f64[1] = 2147483648.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE].pd.f64[0] = -2147483649.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE].pd.f64[1] = -2147483649.0;

    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[0] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[1] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[2] = 2147483648.0;
    sseConstants[SSE_MAX_INT32_PLUS_ONE_AS_FLOAT].ps.f32[3] = 2147483648.0;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[0] = -2147483649.0f;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[1] = -2147483649.0f;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[2] = -2147483649.0f;
    sseConstants[SSE_MIN_INT32_MINUS_ONE_AS_FLOAT].ps.f32[3] = -2147483649.0f;

    sseConstants[SSE_INT32_BIT_MASK].ps.u32[0] = 1;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[1] = 2;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[2] = 4;
    sseConstants[SSE_INT32_BIT_MASK].ps.u32[3] = 8;

    sseConstants[SSE_BYTE8_BIT_MASK].pi.u8[0] = 1;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[1] = 2;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[2] = 4;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[3] = 8;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[4] = 16;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[5] = 32;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[6] = 64;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[7] = 128;
    sseConstants[SSE_BYTE8_BIT_MASK].pi.u8[8] = 1;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[9] = 2;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[10] = 4;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[11] = 8;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[12] = 16;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[13] = 32;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[14] = 64;
    sseConstants[SSE_BYTE8_BIT_MASK].ps.u8[15] = 128;

    largeAddressJumpInstruction = 0xf8400149;
    pageJumpInstruction = 0xf86f7929;
    pageOffsetJumpInstruction = 0x38400131;
}

void Armv8btCPU::setSeg(U32 index, U32 address, U32 value) {
    CPU::setSeg(index, address, value);
}

void Armv8btCPU::restart() {
	this->exitToStartThreadLoop = true;
}

void* Armv8btCPU::init() {
    Armv8btAsm data(this);
    void* result;
    KMemoryData* mem = getMemData(memory);
    Armv8btCPU* cpu = this;

    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(memory->mutex);
	data.saveNativeState();

    data.writeToRegFromValue(xCPU, (U64)this);
    data.writeToRegFromValue(xES, (U32)cpu->seg[ES].address);    
    data.writeToRegFromValue(xCS, (U32)cpu->seg[CS].address);
    data.writeToRegFromValue(xSS, (U32)cpu->seg[SS].address);
    data.writeToRegFromValue(xDS, (U32)cpu->seg[DS].address);
    data.writeToRegFromValue(xFS, (U32)cpu->seg[FS].address);
    data.writeToRegFromValue(xGS, (U32)cpu->seg[GS].address);

    data.writeToRegFromValue(xStackMask, (U32)cpu->stackMask);

    data.setNativeFlags(this->flags, FMASK_TEST|DF|ID);

    data.writeToRegFromValue(xEAX, EAX);
    data.writeToRegFromValue(xECX, ECX);
    data.writeToRegFromValue(xEDX, EDX);
    data.writeToRegFromValue(xEBX, EBX);
    data.writeToRegFromValue(xESP, ESP);
    data.writeToRegFromValue(xEBP, EBP);
    data.writeToRegFromValue(xESI, ESI);
    data.writeToRegFromValue(xEDI, EDI);        
    
#ifdef xMemRead
    KMemoryData* memData = getMemData(memory);
    data.writeToRegFromValue(xMemRead, (U64)memData->mmuReadPtrAdjusted);
    data.writeToRegFromValue(xMemWrite, (U64)memData->mmuWritePtrAdjusted);
#endif

    data.calculatedEipLen = 1; // will force the long x64 chunk jump
    data.doJmp(false);
    result = data.commit(memory);
    //link(&data, chunk);
    this->pendingCodePages.clear();    

    if (!this->thread->process->returnToLoopAddress) {
        Armv8btAsm returnData(this);
        returnData.restoreNativeState();
        returnData.addReturn();
        this->thread->process->returnToLoopAddress = returnData.commit(memory);
    }
    this->returnToLoopAddress = this->thread->process->returnToLoopAddress;
    if (!this->thread->process->jmpAndTranslateIfNecessary) {
        Armv8btAsm translateData(this);
        translateData.createCodeForJmpAndTranslateIfNecessary();
        this->thread->process->jmpAndTranslateIfNecessary = translateData.commit(memory);
    }
    this->jmpAndTranslateIfNecessary = this->thread->process->jmpAndTranslateIfNecessary;
    if (!this->thread->process->syncToHostAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForSyncToHost();
        this->thread->process->syncToHostAddress = translateData.commit(memory);
    }
    this->syncToHostAddress = this->thread->process->syncToHostAddress;
    if (!this->thread->process->syncFromHostAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForSyncFromHost();
        this->thread->process->syncFromHostAddress = translateData.commit(memory);
    }
    this->syncFromHostAddress = this->thread->process->syncFromHostAddress;
    if (!this->thread->process->doSingleOpAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForDoSingleOp();
        this->thread->process->doSingleOpAddress = translateData.commit(memory);
    }
    this->doSingleOpAddress = this->thread->process->doSingleOpAddress;
#ifdef BOXEDWINE_POSIX
    if (!this->thread->process->runSignalAddress) {
        Armv8btAsm translateData(this);
        translateData.createCodeForRunSignal();
        this->thread->process->runSignalAddress = translateData.commit(memory);
    }
#endif
    return result;
}

#ifdef __TEST
void Armv8btCPU::addReturnFromTest() {
    Armv8btAsm data(this);
    data.addReturnFromTest();
    data.commit(memory);
}
#endif

void Armv8btCPU::writeJumpAmount(BtData* data, U32 pos, U32 toLocation, U8* offset) {
    S32 amount = (S32)(toLocation) >> 2;
    if (offset[pos + 3] == 0x14) {
        if (amount > 0xFFFFFF) {
            kpanic_fmt("Armv8btCPU::writeJumpAmount in large jump not supported: %d", amount);
        }
        offset[pos] = (U8)amount;
        offset[pos + 1] = (U8)(amount >> 8);
        offset[pos + 2] = (U8)(amount >> 16);
        offset[pos + 3] |= (U8)((amount >> 24) & 3);
    } else if (offset[pos + 3] == 0x34 || offset[pos + 3] == 0x35) {
        if (amount >= 0x40000 || amount <= -0x40000) {
            kpanic_fmt("Armv8btCPU::writeJumpAmount in large jump not supported: %d", amount);
        }
        offset[pos] |= (U8)(amount << 5);
        offset[pos + 1] = (U8)(amount >> 3);
        offset[pos + 2] = (U8)(amount >> 11);
    } else {
        kpanic_fmt("Armv8btCPU::writeJumpAmount unknown jump: %d", offset[pos + 3]);
    }
}

void Armv8btCPU::link(BtData* data, void* hostAddress) {
    for (auto& todoJump : data->todoJump) {
        // todoJump.eip is where we are jumping to
        // todoJump.bufferPos is offset in host memory of instruction doing the jump
        U32 eip = this->seg[CS].address + todoJump.eip;
        U32 offset = data->getHostOffsetFromEip(eip) - todoJump.bufferPos;

#ifdef BOXEDWINE_MAC_JIT
        if (__builtin_available(macOS 11.0, *)) {
            pthread_jit_write_protect_np(false);
        }
#endif
        writeJumpAmount(data, todoJump.bufferPos, offset, (U8*)hostAddress);
#ifdef BOXEDWINE_MAC_JIT
        if (__builtin_available(macOS 11.0, *)) {
            pthread_jit_write_protect_np(true);
        }
#endif
    }
}

#endif
