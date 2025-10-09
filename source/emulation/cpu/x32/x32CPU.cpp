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
#ifdef BOXEDWINE_DYNAMIC32

#include "x32CPU.h"
#include "../common/lazyFlags.h"
#include "../dynamic/dynamic.h"
#include "../../softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"

#include <unordered_set>

extern U8* ramPages[K_NUMBER_OF_PAGES];

// cdecl calling convention states EAX, ECX, and EDX are caller saved

/********************************************************/
/* Following is required to be defined for dynamic code */
/********************************************************/

#define INCREMENT_EIP(x, y) incrementEip(x, y)

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

// DynReg is a required type, but the values inside are local to this file
// Used only these 4 because it is possible to use 8-bit calls with them, like add al, cl
enum DynReg {
    DYN_EAX=0,
    DYN_ECX=1,
    DYN_EDX=2,
    DYN_EBX=3,  
    DYN_NOT_SET=0xff
};

enum DynCondition {
    DYN_EQUALS_ZERO,
    DYN_NOT_EQUALS_ZERO
};

enum DynConditionEvaluate {
    DYN_EQUALS,
    DYN_NOT_EQUALS,
    DYN_LESS_THAN_UNSIGNED,
    DYN_LESS_THAN_EQUAL_UNSIGNED,
    DYN_GREATER_THAN_EQUAL_UNSIGNED,
    DYN_LESS_THAN_SIGNED,
    DYN_LESS_THAN_EQUAL_SIGNED,
};

#define DYN_CALL_RESULT DYN_EAX
#define DYN_SRC DYN_ECX
#define DYN_DEST DYN_EDX
#define DYN_ADDRESS DYN_EBX
#define DYN_ANY DYN_DEST

#define DYN_PTR_SIZE U32

enum DynWidth {
    DYN_8bit=0,
    DYN_16bit,
    DYN_32bit,
};

enum DynCallParamType {
    DYN_PARAM_REG_8,    
    DYN_PARAM_REG_16,
    DYN_PARAM_REG_32,
    DYN_PARAM_CONST_8,
    DYN_PARAM_CONST_16,
    DYN_PARAM_CONST_32,
    DYN_PARAM_CONST_PTR,
    DYN_PARAM_CPU_ADDRESS_8,
    DYN_PARAM_CPU_ADDRESS_16,
    DYN_PARAM_CPU_ADDRESS_32,
    DYN_PARAM_CPU,
};

enum DynConditional {
    O,
    NO,
    B,
    NB,
    Z,
    NZ,
    BE,
    NBE,
    S,
    NS,
    P,
    NP,
    L,
    NL,
    LE,
    NLE
};

#define Dyn_PtrSize DYN_32bit

// helper, can be done with multiple other calls
void movToCpuFromMem(DynamicData* data, U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
void movToCpuFromCpu(DynamicData* data, U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
void calculateEaa(DynamicData* data, DecodedOp* op, DynReg reg);

void byteSwapReg32(DynamicData* data, DynReg reg);

// REG to REG
void movToRegFromRegSignExtend(DynamicData* data, DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);
void movToRegFromReg(DynamicData* data, DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);

// to Reg
void movToReg(DynamicData* data, DynReg reg, DynWidth width, U32 imm);
void zeroExtendReg16To32(DynamicData* data, DynReg dest, DynReg src);

// to CPU
void movToCpuFromReg(DynamicData* data, U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg);
void movToCpu(DynamicData* data, U32 dstOffset, DynWidth dstWidth, U32 imm);
void movToCpuPtr(DynamicData* data, U32 dstOffset, DYN_PTR_SIZE imm) {movToCpu(data, dstOffset, DYN_32bit, imm);}

// from CPU
void movToRegFromCpu(DynamicData* data, DynReg reg, U32 srcOffset, DynWidth width);
void movToRegFromCpuPtr(DynamicData* data, DynReg reg, U32 srcOffset) { movToRegFromCpu(data, reg, srcOffset, DYN_32bit); }

// from Mem to DYN_READ_RESULT
void movFromMem(DynamicData* data, DynWidth width, DynReg addressReg, bool doneWithAddressReg);

// to Mem
void movToMemFromReg(DynamicData* data, DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg);
void movToMemFromImm(DynamicData* data, DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg);

// arith
void instRegReg(DynamicData* data, char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
void instMemReg(DynamicData* data, char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg);
void instCPUReg(DynamicData* data, char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg);

void instRegImm(DynamicData* data, U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instMemImm(DynamicData* data, char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg);
void instCPUImm(DynamicData* data, char inst, U32 dstOffset, DynWidth regWidth, U32 imm, DynReg tmpReg);

void instReg(DynamicData* data, char inst, DynReg reg, DynWidth regWidth);
void instMem(DynamicData* data, char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg);
void instCPU(DynamicData* data, char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg);

// if conditions
void IfLessThan(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void IfEqual(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void IfNotEqual(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void IfBitSet(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void If(DynamicData* data, X86Asm::Reg32 reg, bool doneWithReg);
void IfNot(DynamicData* data, X86Asm::Reg32 reg, bool doneWithReg);
void IfPtrEqual(DynamicData* data, X86Asm::Reg32 reg, DYN_PTR_SIZE value, bool doneWithReg);
void StartElse(DynamicData* data);
void EndIf(DynamicData* data);
void JumpIf(DynamicData* data, DynReg reg, bool doneWithReg, U32 address);
void JumpIfNot(DynamicData* data, DynReg reg, bool doneWithReg, U32 address);
void JumpInBlock(DynamicData* data, U32 address);

void evaluateToReg(DynamicData* data, DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition);
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg);

// call into emulator, like setFlags, getCF, etc
void callHostFunction(DynamicData* data, void* address, bool hasReturn=false, U32 argCount=0, U32 arg1=0, DynCallParamType arg1Type=DYN_PARAM_CONST_32, bool doneWithArg1=true, U32 arg2=0, DynCallParamType arg2Type=DYN_PARAM_CONST_32, bool doneWithArg2=true, U32 arg3=0, DynCallParamType arg3Type=DYN_PARAM_CONST_32, bool doneWithArg3=true, U32 arg4=0, DynCallParamType arg4Type=DYN_PARAM_CONST_32, bool doneWithArg4=true, U32 arg5=0, DynCallParamType arg5Type=DYN_PARAM_CONST_32, bool doneWithArg5=true);

// set up the cpu to the correct next block

// this is called for cases where we don't know ahead of time where the next block will be, so we need to look it up
void blockDone(DynamicData* data, bool returnEarly);
// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1(DynamicData* data, DecodedOp* op);
void blockNext2(DynamicData* data, DecodedOp* op);
void blockCall(DynamicData* data, DecodedOp* op);
void blockDoneCall(DynamicData* data);
void blockDoneJump(DynamicData* data);

/********************************************************/
/* End required for dynamic code                        */
/********************************************************/

// referenced in macro above
void incrementEip(DynamicData* data, DecodedOp* op);
void incrementEip(DynamicData* data, U32 len);

#include "../normal/instructions.h"
#include "../common/common_arith.h"
#include "../common/common_pushpop.h"
#include "../dynamic/dynamic_func.h"
#include "../dynamic/dynamic_arith.h"
#include "../dynamic/dynamic_mov.h"
#include "../dynamic/dynamic_incdec.h"
#include "../dynamic/dynamic_jump.h"
#include "../dynamic/dynamic_pushpop.h"
#include "../dynamic/dynamic_strings.h"
#include "../dynamic/dynamic_shift.h"
#include "../dynamic/dynamic_conditions.h"
#include "../dynamic/dynamic_setcc.h"
#include "../dynamic/dynamic_xchg.h"
#include "../dynamic/dynamic_bit.h"
#include "../dynamic/dynamic_other.h"
#include "../dynamic/dynamic_mmx.h"
#include "../dynamic/dynamic_sse.h"
#include "../dynamic/dynamic_sse2.h"
#include "../dynamic/dynamic_fpu.h"
#include "../dynamic/dynamic_lock.h"

void calculateEaa(DynamicData* data, DecodedOp* op, DynReg reg) {
    data->regUsed[reg] = true;

    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        X86Asm::Reg16 reg16(reg);

        data->x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));

        if (op->data.disp) {
            data->x86.add(reg16, (U16)op->data.disp);
        }
        if (op->rm != 8) {
            // intentional 16-bit add
            data->x86.addMem(reg16, data->x86.edi, CPU::offsetofReg16(op->rm));
        }

        if (op->sibIndex != 8) {
            // intentional 16-bit add
            data->x86.addMem(reg16, data->x86.edi, CPU::offsetofReg16(op->sibIndex));
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            data->x86.addMem(X86Asm::Reg32(reg), data->x86.edi, CPU::offsetofSegAddress(op->base));
        }
    } else {
        if (op->sibIndex != 8) {
            data->x86.readMem(X86Asm::Reg32(reg), data->x86.edi, CPU::offsetofReg32(op->sibIndex));
            if (op->sibScale) {
                data->x86.shl(X86Asm::Reg32(reg), op->sibScale);
            }
            if (op->rm != 8) {
                data->x86.addMem(X86Asm::Reg32(reg), data->x86.edi, CPU::offsetofReg32(op->rm));
            }
            if (op->data.disp) {
                data->x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->rm != 8) {
            data->x86.readMem(X86Asm::Reg32(reg), data->x86.edi, CPU::offsetofReg32(op->rm));
            if (op->data.disp) {
                data->x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->data.disp) {
            data->x86.mov(X86Asm::Reg32(reg), op->data.disp);
        } else {
            data->x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
        }

        // seg[6] is always 0
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            data->x86.addMem(X86Asm::Reg32(reg), data->x86.edi, CPU::offsetofSegAddress(op->base));
        }
    }
}

void byteSwapReg32(DynamicData* data, DynReg reg) {
    data->x86.bswap(reg);
}

void movToRegFromRegSignExtend(DynamicData* data, DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    data->regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        movToRegFromReg(data, dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                data->x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                data->x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                data->x86.movsx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
        }
        if (doneWithSrcReg) {
            data->regUsed[src] = false;
        }
    }
}

void movToRegFromReg(DynamicData* data, DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    data->regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        if (dst == src) // downsizing doesn't need anything
            return;
        if (dstWidth == DYN_32bit) {
            data->x86.mov(X86Asm::Reg32(dst), X86Asm::Reg32(src));
        } else if (dstWidth == DYN_16bit) {
            data->x86.mov(X86Asm::Reg16(dst), X86Asm::Reg16(src));
        } else if (dstWidth == DYN_8bit) {
            data->x86.mov(X86Asm::Reg8(dst), X86Asm::Reg8(src));
        } else {
            kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromReg %d", dstWidth);
        }
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                data->x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                data->x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                data->x86.movzx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
        }
    }
    if (doneWithSrcReg) {
        data->regUsed[src] = false;
    }
}

void movToRegFromCpu(DynamicData* data, DynReg reg, U32 srcOffset, DynWidth width) {
    data->regUsed[reg] = true;
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        data->x86.readMem(X86Asm::Reg32(reg), data->x86.edi, srcOffset);
    } else if (width == DYN_16bit) {
        data->x86.readMem(X86Asm::Reg16(reg), data->x86.edi, srcOffset);
    } else if (width == DYN_8bit) {
        data->x86.readMem(X86Asm::Reg8(reg), data->x86.edi, srcOffset);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromCpu %d", width);
    }
}

void movToCpuFromReg(DynamicData* data, U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (width == DYN_32bit) {
        data->x86.writeMem(data->x86.edi, dstOffset, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        data->x86.writeMem(data->x86.edi, dstOffset, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        data->x86.writeMem(data->x86.edi, dstOffset, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpuFromReg %d", width);
    }
    if (doneWithReg) {
        data->regUsed[reg] = false;
    }
}

void movToCpuFromCpu(DynamicData* data, U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(data, tmpReg, srcOffset, width);

    // mov [cpu+dstOffset], tmpReg
    movToCpuFromReg(data, dstOffset, tmpReg, width, doneWithTmpReg);    
}

void movToCpu(DynamicData* data, U32 dstOffset, DynWidth dstWidth, U32 imm) {
    if (dstWidth == DYN_32bit) {
        data->x86.writeMem(data->x86.edi, dstOffset, imm);
    } else if (dstWidth == DYN_16bit) {
        data->x86.writeMem(data->x86.edi, dstOffset, (U16)imm);
    } else if (dstWidth == DYN_8bit) {
        data->x86.writeMem(data->x86.edi, dstOffset, (U8)imm);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpu %d", dstWidth);
    }
}

void zeroExtendReg16To32(DynamicData* data, DynReg dest, DynReg src) {
    data->x86.movzx(X86Asm::Reg32(dest), X86Asm::Reg16(src));
    data->regUsed[dest] = true;
}

void movToReg(DynamicData* data, DynReg reg, DynWidth width, U32 imm) {
    data->regUsed[reg] = true;
    if (width == DYN_32bit) {
        data->x86.mov(X86Asm::Reg32(reg), imm);
    } else if (width == DYN_16bit) {
        data->x86.mov(X86Asm::Reg16(reg), (U16)imm);
    } else if (width == DYN_8bit) {
        data->x86.mov(X86Asm::Reg8(reg), (U8)imm);
    } else {
        kpanic_fmt("unknown width in x32CPU::movToReg %d", width);
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

static void writed(U32 address, U32 value) {
    KThread::currentThread()->memory->writed(address, value);
}

static void writew(U32 address, U16 value) {
    KThread::currentThread()->memory->writew(address, value);
}

static void writeb(U32 address, U8 value) {
    KThread::currentThread()->memory->writeb(address, value);
}

void movFromMem(DynamicData* data, DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
    data->regUsed[data->x86.eax.reg] = true;

    if (addressReg != DYN_ADDRESS) {
        kpanic("movFromMem");
    }
    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        data->x86.mov(data->x86.eax, X86Asm::Reg32(addressReg));
        data->x86.and_(data->x86.eax, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(data, data->x86.eax, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(data, data->x86.eax, 0xFFD, false);
        }
    }
    // int index = address >> 12;
    // if (Memory::currentMMUReadPtr[index])
    //     return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
    // else
    //     return readd(address);

    data->x86.mov(data->x86.eax, X86Asm::Reg32(addressReg));
    data->x86.shr(data->x86.eax, K_PAGE_SHIFT);
    data->x86.readMem(data->x86.eax, data->x86.eax, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf(data);
    }

    // test eax, 0x40000000 (mmu[index].canReadRam)
    IfBitSet(data, data->x86.eax, 0x40000000, false);

    // bottom 20 bits of mmu contains ram page index
    data->x86.and_(data->x86.eax, 0xfffff);
    data->x86.readMem(data->x86.eax, data->x86.eax, 2, (U32)ramPages);
    X86Asm::Reg32 offsetReg(addressReg);

    if (!doneWithAddressReg) {
        data->x86.push(X86Asm::Reg32(addressReg));
    }
    data->x86.and_(offsetReg, K_PAGE_MASK);

    // mov eax, [eax+reg]
    if (width == DYN_8bit) {
        data->x86.readMem(X86Asm::Reg8(data->x86.eax.reg), data->x86.eax, offsetReg, 0, 0);
    } else if (width == DYN_16bit) {
        data->x86.readMem(X86Asm::Reg16(data->x86.eax.reg), data->x86.eax, offsetReg, 0, 0);
    } else if (width == DYN_32bit) {
        data->x86.readMem(data->x86.eax, data->x86.eax, offsetReg, 0, 0);
    }

    if (!doneWithAddressReg) {
        data->x86.pop(X86Asm::Reg32(addressReg));
    }

    StartElse(data);

    // will set EAX when call returns, so don't push it then clobber the result with a pop
    if (data->regUsed[data->x86.ecx.reg]) {
        data->x86.push(data->x86.ecx);
    }
    if (data->regUsed[data->x86.edx.reg]) {
        data->x86.push(data->x86.edx);
    }

    data->x86.push(X86Asm::Reg32(addressReg));

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

    data->x86.call(address);

    data->x86.add(data->x86.esp, 4);

    if (data->regUsed[data->x86.edx.reg]) {
        data->x86.pop(data->x86.edx);
    }
    if (data->regUsed[data->x86.ecx.reg]) {
        data->x86.pop(data->x86.ecx);
    }

    EndIf(data);

    if (doneWithAddressReg) {
        data->regUsed[addressReg] = false;
    }
}

void movToCpuFromMem(DynamicData* data, U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(data, dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+dstOffset], eax
    movToCpuFromReg(data, dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

void pushValue(DynamicData* data, U32 arg, DynCallParamType argType) {
    switch (argType) {
    case DYN_PARAM_REG_8:
        data->x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg8(arg));
        data->x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_16:
        data->x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg16(arg));
        data->x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_32:
        data->x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_CPU:
        data->x86.push(data->x86.edi);
        break;
    case DYN_PARAM_CONST_8:
        data->x86.push((U32)(arg & 0xFF));
        break;
    case DYN_PARAM_CONST_16:
        data->x86.push((U32)(arg & 0xFFFF));
        break;
    case DYN_PARAM_CONST_32:
        data->x86.push(arg);
        break;
    case DYN_PARAM_CONST_PTR:
        data->x86.push((U32)arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_8:
        data->x86.readMem(X86Asm::Reg8(data->x86.eax.reg), data->x86.edi, arg);
        data->x86.movzx(data->x86.eax, X86Asm::Reg8(data->x86.eax.reg));
        data->x86.push(data->x86.eax);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        data->x86.readMem(X86Asm::Reg16(data->x86.eax.reg), data->x86.edi, arg);
        data->x86.movzx(data->x86.eax, X86Asm::Reg16(data->x86.eax.reg));
        data->x86.push(data->x86.eax);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        data->x86.readMem(data->x86.eax, data->x86.edi, arg);
        data->x86.push(data->x86.eax);
        break;
    default:
        kpanic_fmt("x32CPU: unknown argType: %d", argType);
        break;
    }
}

bool isParamTypeReg(DynCallParamType paramType) {
    return paramType==DYN_PARAM_REG_8 || paramType==DYN_PARAM_REG_16 || paramType==DYN_PARAM_REG_32;
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
void movToMem(DynamicData* data, DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp) {
    U32 firstCheckPos = 0;

    if (data->regUsed[tmp]) {
        kpanic("movToMem");
    }
    X86Asm::Reg32 tmpReg(tmp);

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        data->x86.mov(tmpReg, X86Asm::Reg32(addressReg));
        data->x86.and_(tmpReg, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(data, tmpReg, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(data, tmpReg, 0xFFD, false);
        }
    }
    data->x86.mov(tmpReg, X86Asm::Reg32(addressReg));
    data->x86.shr(tmpReg, K_PAGE_SHIFT);
    data->x86.readMem(tmpReg, tmpReg, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf(data);
    }

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    IfBitSet(data, tmpReg, 0x80000000, false);

    // bottom 20 bits of mmu contains ram page index
    data->x86.and_(tmpReg, 0xfffff);
    data->x86.readMem(tmpReg, tmpReg, 2, (U32)ramPages);
    X86Asm::Reg32 offsetReg(addressReg);

    if (!doneWithAddressReg) {
        data->x86.push(data->x86.esi);
        offsetReg = data->x86.esi;
        data->x86.mov(offsetReg, X86Asm::Reg32(addressReg));
    }
    data->x86.and_(offsetReg, K_PAGE_MASK);

    if (isParamTypeReg(paramType)) {
        if (width == DYN_8bit) {
            data->x86.writeMem(tmpReg, offsetReg, 0, 0, X86Asm::Reg8(value));
        } else if (width == DYN_16bit) {
            data->x86.writeMem(tmpReg, offsetReg, 0, 0, X86Asm::Reg16(value));
        } else if (width == DYN_32bit) {
            data->x86.writeMem(tmpReg, offsetReg, 0, 0, X86Asm::Reg32(value));
        } else {
            kpanic("movToMem");
        }
    } else {
        if (width == DYN_8bit) {
            data->x86.writeMem(tmpReg, offsetReg, 0, 0, (U8)value);
        } else if (width == DYN_16bit) {
            data->x86.writeMem(tmpReg, offsetReg, 0, 0, (U16)value);
        } else if (width == DYN_32bit) {
            data->x86.writeMem(tmpReg, offsetReg, 0, 0, value);
        } else {
            kpanic("movToMem");
        }
    }
    if (!doneWithAddressReg) {
        data->x86.pop(data->x86.esi);
    }
    data->regUsed[tmpReg.reg] = false;

    StartElse(data);

    bool regDone[4] = { false, false, false, false };
    if (doneWithReg && isParamTypeReg(paramType)) {
        regDone[value] = true;
    }
    if (data->regUsed[data->x86.eax.reg] && !regDone[data->x86.eax.reg]) {
        data->x86.push(data->x86.eax);
    }
    if (data->regUsed[data->x86.ecx.reg] && !regDone[data->x86.ecx.reg]) {
        data->x86.push(data->x86.ecx);
    }
    if (data->regUsed[data->x86.edx.reg] && !regDone[data->x86.edx.reg]) {
        data->x86.push(data->x86.edx);
    }

    pushValue(data, value, paramType);
    data->x86.push(X86Asm::Reg32(addressReg));

    if (width == DYN_32bit) {
        data->x86.call(writed);
    } else if (width == DYN_16bit) {
        data->x86.call(writew);
    } else if (width == DYN_8bit) {
        data->x86.call(writeb);
    } else {
        kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
    }

    data->x86.add(data->x86.esp, 8);

    if (data->regUsed[data->x86.edx.reg] && !regDone[data->x86.edx.reg]) {
        data->x86.pop(data->x86.edx);
    }
    if (data->regUsed[data->x86.ecx.reg] && !regDone[data->x86.ecx.reg]) {
        data->x86.pop(data->x86.ecx);
    }
    if (data->regUsed[data->x86.eax.reg] && !regDone[data->x86.eax.reg]) {
        data->x86.pop(data->x86.eax);
    }
    EndIf(data);

    if (doneWithAddressReg) {
        data->regUsed[addressReg] = false;
    }
    if (doneWithReg && isParamTypeReg(paramType)) {
        data->regUsed[value] = false;
    }
}

void movToMemFromReg(DynamicData* data, DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg) {
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_REG_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_REG_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_REG_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromReg", width);

    movToMem(data, addressReg, width, reg, paramType, doneWithReg, doneWithAddressReg, tmpReg);
}

void movToMemFromImm(DynamicData* data, DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_CONST_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_CONST_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_CONST_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromImm", width);

    movToMem(data, addressReg, width, imm, paramType, false, doneWithAddressReg, tmpReg);
}

void callHostFunction(DynamicData* data, void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[4] = { false, false, false, false };

    if (argCount >= 5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg5, arg5Type);
            }
            regDone[arg5] = true;
        }
    }
    if (argCount >= 4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 4: arg=%d argType=%d", arg4, arg4Type);
            }
            regDone[arg4] = true;
        }
    }
    if (argCount >= 3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 3: arg=%d argType=%d", arg3, arg3Type);
            }
            regDone[arg3] = true;
        }
    }
    if (argCount >= 2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg2, arg2Type);
            }
            regDone[arg2] = true;
        }
    }
    if (argCount >= 1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1 >= 4) {
                kpanic_fmt("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg1, arg1Type);
            }
            regDone[arg1] = true;
        }
    }

    if (hasReturn) {
        data->regUsed[data->x86.eax.reg] = true;
    } else if (data->regUsed[data->x86.eax.reg] && !hasReturn && !regDone[data->x86.eax.reg]) {
        data->x86.push(data->x86.eax);
    }
    if (data->regUsed[data->x86.ecx.reg] && !regDone[data->x86.ecx.reg]) {
        data->x86.push(data->x86.ecx);
    }
    if (data->regUsed[data->x86.edx.reg] && !regDone[data->x86.edx.reg]) {
        data->x86.push(data->x86.edx);
    }
    if (argCount >= 5) {
        pushValue(data, arg5, arg5Type);
    }
    if (argCount >= 4) {
        pushValue(data, arg4, arg4Type);
    }
    if (argCount >= 3) {
        pushValue(data, arg3, arg3Type);
    }
    if (argCount >= 2) {
        pushValue(data, arg2, arg2Type);
    }
    if (argCount >= 1) {
        pushValue(data, arg1, arg1Type);
    }

    data->x86.call(address);

    if (argCount) {
        data->x86.add(data->x86.esp, 4 * argCount);
    }    
    if (data->regUsed[data->x86.edx.reg] && !regDone[data->x86.edx.reg]) {
        data->x86.pop(data->x86.edx);
    }
    if (data->regUsed[data->x86.ecx.reg] && !regDone[data->x86.ecx.reg]) {
        data->x86.pop(data->x86.ecx);
    }
    if (!hasReturn && data->regUsed[data->x86.eax.reg] && !regDone[data->x86.eax.reg]) {
        data->x86.pop(data->x86.eax);
    }
    for (int i = 0; i < 4; i++) {
        if (regDone[i]) {
            data->regUsed[i] = false;
        }
    }
}

// inst can be +, |, - , &, ^, <, >, ) right parens is for signed right shift
void instRegImm(DynamicData* data, U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    switch (inst) {
    case '+':
        if (regWidth == DYN_32bit) {
            data->x86.add(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.add(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.add(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm ADD");
        }
        break;
    case '-':
        if (regWidth == DYN_32bit) {
            data->x86.sub(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.sub(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.sub(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SUB");
        }
        break;
    case '&':
        if (regWidth == DYN_32bit) {
            data->x86.and_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.and_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.and_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm AND");
        }
        break;
    case '|':
        if (regWidth == DYN_32bit) {
            data->x86.or_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.or_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.or_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm OR");
        }
        break;
    case '^':
        if (regWidth == DYN_32bit) {
            data->x86.xor_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.xor_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.xor_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm XOR");
        }
        break;
    case '<':
        if (regWidth == DYN_32bit) {
            data->x86.shl(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.shl(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.shl(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SHL");
        }
        break;
    case '>':
        if (regWidth == DYN_32bit) {
            data->x86.shr(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.shr(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.shr(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SHR");
        }
        break;
    case ')':
        if (regWidth == DYN_32bit) {
            data->x86.sar(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            data->x86.sar(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            data->x86.sar(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SAR");
        }
        break;
    default:
        kpanic("instRegImm");
    }
}
void instCPUReg(DynamicData* data, char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    if (data->regUsed[tmpReg]) {
        kpanic("instCPUReg");
    }
    movToRegFromCpu(data, tmpReg, dstOffset, regWidth);
    instRegReg(data, inst, tmpReg, rm, regWidth, doneWithRmReg);
    movToCpuFromReg(data, dstOffset, tmpReg, regWidth, true);
}
void instCPUImm(DynamicData* data, char inst, U32 dstOffset, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    if (data->regUsed[tmpReg]) {
        kpanic("instCPUImm");
    }
    movToRegFromCpu(data, tmpReg, dstOffset, regWidth);
    instRegImm(data, inst, tmpReg, regWidth, imm);
    movToCpuFromReg(data, dstOffset, tmpReg, regWidth, true);
}

void instMemImm(DynamicData* data, char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    if (data->regUsed[0]) {
        kpanic("x32CPU::instMemImm");
    }
    movFromMem(data, regWidth, addressReg, false);
    instRegImm(data, inst, DYN_EAX, regWidth, imm);
    movToMemFromReg(data, addressReg, DYN_EAX, regWidth, true, true, tmpReg);
}
void instMemReg(DynamicData* data, char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    if (data->regUsed[0]) {
        kpanic("x32CPU::instMemReg");
    }    
    movFromMem(data, regWidth, addressReg, false);
    instRegReg(data, inst, DYN_EAX, rm, regWidth, doneWithRmReg);
    movToMemFromReg(data, addressReg, DYN_EAX, regWidth, true, true, tmpReg);
}

// inst can be +, |, -, &, ^, <, >, ) right parens is for signed right shift
void instRegReg(DynamicData* data, char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    switch (inst) {
    case '+':
        if (regWidth == DYN_32bit) {
            data->x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.add(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.add(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg ADD");
        }
        break;
    case '-':
        if (regWidth == DYN_32bit) {
            data->x86.sub(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.sub(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.sub(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SUB");
        }
        break;
    case '&':
        if (regWidth == DYN_32bit) {
            data->x86.and_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.and_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.and_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg AND");
        }
        break;
    case '|':
        if (regWidth == DYN_32bit) {
            data->x86.or_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.or_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.or_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg OR");
        }
        break;
    case '^':
        if (regWidth == DYN_32bit) {
            data->x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.xor_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.xor_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg XOR");
        }
        break;
    case '<':
        if (regWidth == DYN_32bit) {
            data->x86.shl(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.shl(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.shl(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SHL");
        }
        break;
    case '>':
        if (regWidth == DYN_32bit) {
            data->x86.shr(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.shr(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.shr(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SHR");
        }
        break;
    case ')':
        if (regWidth == DYN_32bit) {
            data->x86.sar(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            data->x86.sar(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            data->x86.sar(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SAR");
        }
        break;
    default:
        kpanic("instRegReg");
    }
    if (doneWithRmReg) {
        data->regUsed[rm] = false;
    }
}

// inst can be: ~ or -
void instReg(DynamicData* data, char inst, DynReg reg, DynWidth regWidth) {
    if (inst == '-') {
        if (regWidth == DYN_32bit) {
            data->x86.neg(X86Asm::Reg32(reg));
        } else if (regWidth == DYN_16bit) {
            data->x86.neg(X86Asm::Reg16(reg));
        } else if (regWidth == DYN_8bit) {
            data->x86.neg(X86Asm::Reg8(reg));
        } else {
            kpanic("instReg NEG");
        }
    } else if (inst == '~') {
        if (regWidth == DYN_32bit) {
            data->x86.not_(X86Asm::Reg32(reg));
        } else if (regWidth == DYN_16bit) {
            data->x86.not_(X86Asm::Reg16(reg));
        } else if (regWidth == DYN_8bit) {
            data->x86.not_(X86Asm::Reg8(reg));
        } else {
            kpanic("instReg NOT");
        }
    } else {
        kpanic("instReg");
    }
}

void instMem(DynamicData* data, char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg) {
    if (data->regUsed[0]) {
        kpanic("x32CPU::instMem");
    }  
    movFromMem(data, regWidth, addressReg, false);
    instReg(data, inst, DYN_EAX, regWidth);
    movToMemFromReg(data, addressReg, DYN_EAX, regWidth, true, true, tmpReg);   
}

void instCPU(DynamicData* data, char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg) {
    if (data->regUsed[tmpReg]) {
        kpanic("instCPU");
    }
    movToRegFromCpu(data, tmpReg, dstOffset, regWidth);
    instReg(data, inst, tmpReg, regWidth);
    movToCpuFromReg(data, dstOffset, tmpReg, regWidth, true);
}

void JumpIf(DynamicData* data, DynReg reg, bool doneWithReg, U32 address) {
    data->x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    data->x86.jnz(address);
    if (doneWithReg) {
        data->regUsed[reg] = false;
    }
}

void JumpIfNot(DynamicData* data, DynReg reg, bool doneWithReg, U32 address) {
    data->x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    data->x86.jz(address);
    if (doneWithReg) {
        data->regUsed[reg] = false;
    }
}

void JumpInBlock(DynamicData* data, U32 address) {
    data->x86.jmp(address);
}

void IfPtrEqual(DynamicData* data, X86Asm::Reg32 reg, DYN_PTR_SIZE value, bool doneWithReg) {
    IfEqual(data, reg, value, doneWithReg);
}

void IfLessThan(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    data->x86.IfLessThan(reg, value);

    if (doneWithReg) {
        data->regUsed[reg.reg] = false;
    }
}

void IfEqual(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    data->x86.IfEqual(reg, value);
    if (doneWithReg) {
        data->regUsed[reg.reg] = false;
    }
}

void IfNotEqual(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    data->x86.IfNotEqual(reg, value);
    if (doneWithReg) {
        data->regUsed[reg.reg] = false;
    }
}


void IfBitSet(DynamicData* data, X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    data->x86.IfBitSet(reg, value);
    if (doneWithReg) {
        data->regUsed[reg.reg] = false;
    }
}

void If(DynamicData* data, X86Asm::Reg32 reg, bool doneWithReg) {
    data->x86.IfNotZero(reg);
    if (doneWithReg) {
        data->regUsed[reg.reg] = false;
    }
}

void IfNot(DynamicData* data, X86Asm::Reg32 reg, bool doneWithReg) {
    data->x86.IfZero(reg);
    if (doneWithReg) {
        data->regUsed[reg.reg] = false;
    }
}

void StartElse(DynamicData* data) {
    data->x86.Else();
}

void EndIf(DynamicData* data) {
    data->x86.EndIf();
}

void setCC(DynamicData* data, X86Asm::Reg32 reg, DynConditionEvaluate condition) {

    switch (condition) {
    case DYN_EQUALS:
        data->x86.setz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_NOT_EQUALS:
        data->x86.setnz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_UNSIGNED:
        data->x86.setb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        data->x86.setbe(X86Asm::Reg8(reg.reg));
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:
        data->x86.setnb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_SIGNED:
        data->x86.setl(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        data->x86.setle(X86Asm::Reg8(reg.reg));
        break;
    default:
        kpanic_fmt("x32CPU::evaluateToRegFromRegs unknown condition %d", condition);
    }
}

void evaluateToReg(DynamicData* data, X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (regWidth == DYN_32bit) {
        data->x86.cmp(left, right);
    } else if (regWidth == DYN_16bit) {
        data->x86.cmp(X86Asm::Reg16(left.reg), X86Asm::Reg16(right.reg));
    } else if (regWidth == DYN_8bit) {
        data->x86.cmp(X86Asm::Reg8(left.reg), X86Asm::Reg8(right.reg));
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    data->regUsed[reg.reg] = true;
    setCC(data, reg, condition);
    data->x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        data->regUsed[left.reg] = false;
    }
    if (doneWithRightReg) {
        data->regUsed[right.reg] = false;
    }
}

void evaluateToReg(DynamicData* data, X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg) {
    if (regWidth == DYN_32bit) {
        data->x86.cmp(left, rightConst);
    } else if (regWidth == DYN_16bit) {
        data->x86.cmp(X86Asm::Reg16(left.reg), (U16)rightConst);
    } else if (regWidth == DYN_8bit) {
        data->x86.cmp(X86Asm::Reg8(left.reg), (U8)rightConst);
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    data->regUsed[reg.reg] = true;
    setCC(data, reg, condition);
    data->x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        data->regUsed[left.reg] = false;
    }
}

void evaluateToReg(DynamicData* data, DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (reg>=4) {
        kpanic_fmt("x32CPU::evaluateToRegFromRegs doesn't support reg %d", reg);
    }
    if (isRightConst) {
        evaluateToReg(data, X86Asm::Reg32(reg), X86Asm::Reg32(left), rightConst, regWidth, condition, doneWithLeftReg);
    } else {
        evaluateToReg(data, X86Asm::Reg32(reg), X86Asm::Reg32(left), X86Asm::Reg32(right), regWidth, condition, doneWithLeftReg, doneWithRightReg);
    }
}

// this is good generic code to copy for other implementations that don't have something like setcc
/*
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpu(offset, regWidth, 0);
    startElse();
    movToCpu(offset, regWidth, 1);
    endIf();
}
*/

void setConditional(DynamicData* data, DynConditional condition) {
    bool setnz = true;
    // changing conditions to ones that are optimized
    if (condition == Z) {
        condition = NZ;
        setnz = false;
    } else if (condition == NS) {
        condition = S;
        setnz = false;
    } else if (condition == NB) {
        condition = B;
        setnz = false;
    } else if (condition == NO) {
        condition = O;
        setnz = false;
    } else if (condition == NBE) {
        condition = BE;
        setnz = false;
    } else if (condition == NLE) {
        condition = LE;
        setnz = false;
    } else if (condition == NL) {
        condition = L;
        setnz = false;
    }
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    data->x86.test(data->x86.eax, data->x86.eax);
    if (setnz) {
        data->x86.setnz(X86Asm::Reg8(data->x86.eax.reg));
    } else {
        data->x86.setz(X86Asm::Reg8(data->x86.eax.reg));
    }
}

void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    setConditional(data, condition);

    if (regWidth != DYN_8bit) {
        movToRegFromReg(data, DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    movToCpuFromReg(data, offset, DYN_CALL_RESULT, regWidth, true);
}

/*
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToReg(DYN_SRC, regWidth, 0);
    startElse();
    movToReg(DYN_SRC, regWidth, 1);
    endIf();
    // don't put this movToMem in the if statement because it is big and will be inlines once in each block of the if statement
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, regWidth, doneWithAddressReg, true);
}
*/
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg) {
    setConditional(data, condition);

    if (regWidth != DYN_8bit) {
        movToRegFromReg(data, DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    movToMemFromReg(data, addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true, tmpReg);
}

void incrementEip(DynamicData* data, U32 inc) {
    data->x86.addMem(data->x86.edi, offsetof(CPU, eip.u32), inc);
}

void incrementEip(DynamicData* data, DecodedOp* op) {
    incrementEip(data, op->len);
}

void blockExit(DynamicData* data) {
    data->x86.pop(data->x86.edi);
    data->x86.pop(data->x86.ebx);

#ifdef _DEBUG
    data->x86.mov(data->x86.esp, data->x86.ebp);
    data->x86.pop(data->x86.ebp);
#endif

    data->x86.ret();
}

void blockCall(DynamicData* data, DecodedOp* op) {
    blockNext1(data, op);
    if (data->lastOpEip > data->currentEip) {
        blockExit(data);
    }
}

void blockDoneCall(DynamicData* data) {
    blockDone(data, false);
}

void blockDone(DynamicData* data, bool returnEarly) {
    // cpu->nextOp = cpu->nextOp();
    callHostFunction(data, common_getNextOp, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(data, offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, true);
    if (returnEarly || data->lastOpEip > data->currentEip) {
        blockExit(data);
    }
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1(DynamicData* data, DecodedOp* op) {
    // if (!(*(op->nextJump))) {
    //     *(op->nextJump) = cpu->getNextOp();
    // }
    // cpu->nextOp = *(op->nextJump);
    movToReg(data, DYN_EDX, DYN_32bit, (U32)op);
    // ebx = op->nextJump
    // mov ebx, [edx + offsetof(DecodedOp, nextJump)]
    data->regUsed[DYN_EBX] = true;
    data->x86.readMem(data->x86.ebx, data->x86.edx, offsetof(DecodedOp, data.nextJump));
    data->regUsed[DYN_EDX] = false;

    // eax = *(op->nextJump)
    data->x86.readMem(data->x86.eax, data->x86.ebx, 0);
    // if (!(*(op->nextJump))) 
    IfNot(data, DYN_EAX, false);
    // *(op->nextJump) = cpu->getNextOp();
    callHostFunction(data, common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    data->x86.writeMem(data->x86.ebx, 0, data->x86.eax);
    EndIf(data);

    // cpu->nextOp = *(op->nextJump);        
    data->regUsed[DYN_EBX] = false;
    movToCpuFromReg(data, offsetof(CPU, nextOp), DYN_EAX, DYN_32bit, false);
    
#ifdef BOXEDWINE_MULTI_THREADED
    data->x86.readMem(data->x86.eax, data->x86.eax, offsetof(DecodedOp, pfnJitCode));
    If(data, DYN_CALL_RESULT, true);
    data->x86.jmp(data->x86.eax);
    EndIf(data);
#endif
}

void blockNext2(DynamicData* data, DecodedOp* op) {
    // if (!op->next) { 
    //     op->next = cpu->getNextOp(); 
    // }
    // cpu->nextOp = op->next;    
    movToReg(data, DYN_EBX, DYN_32bit, (U32)op);

    // mov eax, [ebx + offsetof(DecodedOp, next)]
    data->x86.readMem(data->x86.eax, data->x86.ebx, offsetof(DecodedOp, next));

    IfNot(data, DYN_EAX, false);
    // op->next = cpu->getNextOp();
    callHostFunction(data, common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    // mov [ebx + offsetof(DecodedOp, next)], eax
    data->x86.writeMem(data->x86.ebx, offsetof(DecodedOp, next), data->x86.eax);
    EndIf(data);
    data->regUsed[DYN_EBX] = false;

    // cpu->nextOp = op->next
    movToCpuFromReg(data, offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, false);

#ifdef BOXEDWINE_MULTI_THREADED
    data->x86.readMem(data->x86.eax, data->x86.eax, offsetof(DecodedOp, pfnJitCode));
    If(data, DYN_CALL_RESULT, true);
    data->x86.jmp(data->x86.eax);
    EndIf(data);
#endif 
}

void x32_sidt(DynamicData* data, DecodedOp* op) {
}

void x32_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void x32_callback(DynamicData* data, DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        callHostFunction(data, x32_onExitSignal, false, 1, 0, DYN_PARAM_CPU);
    } else {
        kpanic("x32CPU::x32_callback unhandled callback");
    }
}

void x32_invalid_op(DynamicData* data, DecodedOp* op) {
    kpanic_fmt("Invalid instruction %x\n", op->inst);
}

void x32_onTestEnd(DynamicData* data, DecodedOp* op) {
    data->cpu->nextOp = op;
}

static pfnDynamicOp x32Ops[NUMBER_OF_OPS];
static U32 x32OpsInitialized;

static void initX32Ops() {
    if (x32OpsInitialized)
        return;
    if (offsetof(CPU, eip.u32)>127)
        kpanic("x32CPU::initX32Ops wasn't expecting eip offset to be greater than 127");

    if (offsetof(CPU, reg[8].u32)>127)
        kpanic("x32CPU::initX32Ops wasn't expecting reg[8] offset to be greater than 127");

    if (offsetof(CPU, seg[6].address)>127)
        kpanic("x32CPU::initX32Ops wasn't expecting reg[8] offset to be greater than 127");

    x32OpsInitialized = 1;   
    for (int i=0;i<InstructionCount;i++) {
        x32Ops[i] = x32_invalid_op;
    }
#define INIT_CPU(e, f) x32Ops[e] = dynamic_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#ifdef BOXEDWINE_MULTI_THREADED
#define INIT_CPU_LOCK(e, f) x32Ops[e##_Lock] = dynamic_##f##_lock;
#include "../common/cpu_init_lock.h"
#endif

#undef INIT_CPU    
    
    x32Ops[SLDTReg] = 0; 
    x32Ops[SLDTE16] = 0;
    x32Ops[STRReg] = 0; 
    x32Ops[STRE16] = 0;
    x32Ops[LLDTR16] = 0; 
    x32Ops[LLDTE16] = 0;
    x32Ops[LTRR16] = 0; 
    x32Ops[LTRE16] = 0;
    x32Ops[VERRR16] = 0; 
    x32Ops[VERWR16] = 0; 
    x32Ops[SGDT] = 0;
    x32Ops[SIDT] = x32_sidt;
    x32Ops[LGDT] = 0;
    x32Ops[LIDT] = 0;
    x32Ops[SMSWRreg] = 0; 
    x32Ops[SMSW] = 0;
    x32Ops[LMSWRreg] = 0; 
    x32Ops[LMSW] = 0;
    x32Ops[INVLPG] = 0;
    x32Ops[Callback] = x32_callback;
    x32Ops[TestEnd] = x32_onTestEnd;
}

static U8* createDynamicExecutableMemory(DynamicData& data, KMemory* processMemory) {
    U8* begin = (U8*)processMemory->allocCodeMemory(data.x86.buffer.size());

    Platform::writeCodeToMemory(begin, data.x86.buffer.size(), [begin,&data]() {
        memcpy(begin, data.x86.buffer.data(), data.x86.buffer.size());
        });
    return begin;
}

static U8* createStartJITCode(KMemory* memory) {
    DynamicData data;

#ifdef _DEBUG
    data.x86.push(data.x86.ebp);
    data.x86.mov(data.x86.ebp, data.x86.esp);
#endif

    data.x86.push(data.x86.ebx);
    data.x86.push(data.x86.edi);
    // on win32 ecx contains cpu
    data.x86.mov(data.x86.edi, data.x86.ecx);

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    data.x86.readMem(data.x86.eax, data.x86.edx, offsetof(DecodedOp, pfnJitCode));
    data.x86.jmp(data.x86.eax);
    U8* result = createDynamicExecutableMemory(data, memory);
    return result;
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
    BString offset = cpu->thread->process->getModuleEip(address);
    file.writeFormat("Block %d in %s(%d)\n", count, name.c_str(), offset);
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

static void doJIT(CPU* cpu, U32 address, DecodedOp* op);
static bool calculateLongestBlock(DynamicData& data, DecodedOp* op) {
    U32 eip = data.startingEip;
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
            if (!nextOp->next) {
                // don't call cpu->getOp since that will decode and we are not sure the next byte is a valid instruction.
                // we can call memory->getDecodedOp to see if this instruction has already been decoded, in that case we know its valid.
                nextOp->next = data.cpu->memory->getDecodedOp(eip + nextOp->len);
            }
            if (!nextOp->next) {
                // since we couldn't figure out if the next byte is part of a valid instruction, we are done looking
                break;
            }
        }
        if (nextOp->isDirectJumpBranch() && (eip + nextOp->len + nextOp->imm) < data.startingEip) {
            // if we have somewhere to go after this, then continue

            // see if we can restart this JIT with the target of this jump to before the incoming op argument to create a bigger JIT block
            DecodedOp* targetOp = data.cpu->memory->getDecodedOp(eip + nextOp->len + nextOp->imm);
            if (!targetOp) {
                nextOp->next = data.cpu->getOp(eip + nextOp->len, 0);
            }
            if (targetOp && !(op->flags & OP_FLAG_JIT)) {
                if (!targetOp->pfnJitCode) {
                    doJIT(data.cpu, eip + nextOp->len + nextOp->imm, targetOp);
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
                nextOp->next = data.cpu->getOp(eip + nextOp->len, 0);
            } else {
                nextOp->next = data.cpu->memory->getDecodedOp(eip + nextOp->len);
                if (!nextOp->next && nextOp->isCall() && furthestJump > eip) {
                    nextOp->next = data.cpu->getOp(eip + nextOp->len, 0);
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
        data.lastOpEip = data.startingEip;
        while (nextOp && data.lastOpEip < lastFurthestEip) {
            if (nextOp->isDirectJumpBranch()) {
                U32 target = data.lastOpEip + nextOp->len + nextOp->imm;
                if (target >= lastFurthestEip || target < data.startingEip) {
                    break;
                }
            }
            if (nextOp->next) {
                data.lastOpEip += nextOp->len;
            }
            nextOp = nextOp->next;
        }
        if (!nextOp || !nextOp->next) {
            break;
        }
        U32 lastEip = data.lastOpEip;
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

static void removeJIT(CPU* cpu, DecodedOp* op, U32 count) {
    for (U32 i = 0; i < count; i++) {
        if (op->blockStart) {
            removeJITBlock(op->blockStart);
        }
    }
}

static void commitJIT(DynamicData& data, DecodedOp* op) {
    for (DynamicJump& jmp : data.x86.jumps) {
        U32 bufferIndex = 0;

        if (!data.eipToBufferPos.get(jmp.eip, bufferIndex)) {
            kpanic("x32CPU firstDynamicOp");
        }
        *(U32*)&data.x86.buffer.data()[jmp.bufferPos] = bufferIndex - jmp.bufferPos - 4;
    }

    if (!data.blockOpCount) {
        return;
    }

    U8* begin = createDynamicExecutableMemory(data, data.cpu->memory);

    for (U32 i = 0; i < data.x86.patch.size(); i++) {
        U32 pos = data.x86.patch[i];
        U32* value = (U32*)(&begin[pos]);
        *value = *value - (U32)(begin + pos + 4);
    }
    
    removeJIT(data.cpu, op, data.blockOpCount);
    op->blockLen = data.emulatedLen;
    op->blockOpCount = data.blockOpCount;
#ifdef _DEBUG
    logBlock(data.cpu, data.startingEip, op, op->blockLen);
#endif 
    U32 address = data.startingEip;
    DecodedOp* nextOp = op;
    DecodedOp* last = op;
#ifdef _DEBUG
    BOXEDWINE_CRITICAL_SECTION;
    static int totalBlocks;
    totalBlocks++;
    static int totalOps;
    totalOps += data.blockOpCount;
    if ((totalBlocks % 1000) == 0) {
        klog_fmt("Compiled Blocks: %d, ave block size: %d ops", totalBlocks, totalOps / totalBlocks);
    }
#endif
    for (U32 i = 0; i < data.blockOpCount; i++) {
        U32 bufferIndex = 0;

        if (!data.eipToBufferPos.get(address, bufferIndex)) {
            kpanic("x32CPU firstDynamicOp");
        }
        nextOp->pfnJitCode = (OpCallback)(begin + bufferIndex);
        nextOp->pfn = data.cpu->thread->process->startJITOp;
        nextOp->flags |= OP_FLAG_JIT;
        nextOp->blockStart = op;
        address += nextOp->len;
        last = nextOp;
        nextOp = nextOp->next;
    }
}

static bool compileOps(DynamicData& data, DecodedOp* op) {
    DecodedOp* nextOp = op;
    data.emulatedLen = 0;
    data.blockOpCount = 0;

    while (nextOp) {
        if (nextOp->flags & OP_FLAG_NO_JIT) {
            return false;
        }

        memset(data.regUsed, 0, sizeof(data.regUsed));
#ifndef __TEST
#ifdef _DEBUG
        //callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)nextOp, DYN_PARAM_CONST_PTR, false);
#endif
#endif
        data.emulatedLen += nextOp->len;
        data.blockOpCount++;
        data.eipToBufferPos.set(data.currentEip, data.x86.buffer.size());
        if (nextOp->lock) {
            // so that intra block jumps that try to skip a lock will find the lock version of the op anyway
            data.eipToBufferPos.set(data.currentEip + 1, data.x86.buffer.size());
        }
        x32Ops[nextOp->inst](&data, nextOp);
        data.currentEip += nextOp->len;
        if (data.x86.ifJump.size()) {
            kpanic_fmt("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
        }
        if (data.currentEip > data.lastOpEip) {
            if (!nextOp->isBranch()) {
                int ii = 0;
            }
            break;
        } else if (nextOp->isBranch()) {
            nextOp = nextOp->next;
        } else {
            nextOp = nextOp->next;
        }
    }
    return true;
}

static void doJIT(CPU* cpu, U32 address, DecodedOp* op) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(cpu->memory->mutex);
    if (!cpu->thread->process->startJITOp) {
        cpu->thread->process->startJITOp = (OpCallback)createStartJITCode(cpu->memory);
    }

    // did another thread beat us to JITing this block?
    if (op->flags & OP_FLAG_JIT) {
        // this will get triggered a few times, especially during shutdown
        // I have see this in firefight installer at the end and opentdd start up
        return;
    }
    DynamicData data;
    data.cpu = cpu;
    data.currentEip = address;
    data.startingEip = address;
       
    initX32Ops();
    DecodedOp* nextOp = op;

    if (!calculateLongestBlock(data, op)) {
        return;
    }
    if (!compileOps(data, op)) {
        return;
    }
    blockExit(&data);
    commitJIT(data, op);
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {    
#ifdef __TEST
    if (op->runCount == 0) {
#else
    // done check for long blocks that get broken up, affects f-22/f-16
    if (op->runCount == JIT_RUN_COUNT && op->inst != Done) {
#endif        
        doJIT(cpu, cpu->getEipAddress(), op);
    }
    op->runCount++;
    op->pfn(cpu, op);
}

static OpCallback getJitFunctionForCurrentOp(CPU* cpu) {
    DecodedOp* op = cpu->memory->getDecodedOp(cpu->getEipAddress());
    if (!op) {
        op = cpu->getNextOp();        
    }
    cpu->nextOp = op;
    // runCount could be > JIT_RUN_COUNT with no jit code if it contains an instruction we don't support
    if (!op->pfnJitCode && op->runCount <= JIT_RUN_COUNT) {
        doJIT(cpu, cpu->getEipAddress(), op);
        op->runCount = JIT_RUN_COUNT + 1;
    }
    return op->pfnJitCode;
}

void blockDoneJump(DynamicData* data) {
    // :TODO: what about caching result for direct calls or direct jumps?
    callHostFunction(data, getJitFunctionForCurrentOp, true, 1, 0, DYN_PARAM_CPU, false);
    IfNot(data, DYN_EAX, true);
    blockExit(data);
    EndIf(data);

    data->x86.jmp(data->x86.eax);
}

#endif