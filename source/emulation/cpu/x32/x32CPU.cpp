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
#include "../dynamic/dynamic_memory.h"
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
void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
void calculateEaa(DecodedOp* op, DynReg reg);

void byteSwapReg32(DynReg reg);

// REG to REG
void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);
void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);

// to Reg
void movToReg(DynReg reg, DynWidth width, U32 imm);
void zeroExtendReg16To32(DynReg dest, DynReg src);

// to CPU
void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg);
void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm);
void movToCpuPtr(U32 dstOffset, DYN_PTR_SIZE imm) {movToCpu(dstOffset, DYN_32bit, imm);}

// from CPU
void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width);
void movToRegFromCpuPtr(DynReg reg, U32 srcOffset) { movToRegFromCpu(reg, srcOffset, DYN_32bit); }

// from Mem to DYN_READ_RESULT
void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg);

// to Mem
void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg);
void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg);

// arith
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg);
void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg);

void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg);
void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm, DynReg tmpReg);

void instReg(char inst, DynReg reg, DynWidth regWidth);
void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg);
void instCPU(char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg);

#include "x86Asm.h"

// if conditions
void IfLessThan(X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void IfBitSet(X86Asm::Reg32 reg, U32 value, bool doneWithReg);
void If(X86Asm::Reg32 reg, bool doneWithReg);
void IfNot(X86Asm::Reg32 reg, bool doneWithReg);
void IfPtrEqual(X86Asm::Reg32 reg, DYN_PTR_SIZE value, bool doneWithReg);
void StartElse();
void EndIf();
void JumpIf(DynamicData* data, DynReg reg, bool doneWithReg, U32 address);
void JumpIfNot(DynamicData* data, DynReg reg, bool doneWithReg, U32 address);
void JumpInBlock(DynamicData* data, U32 address);

void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition);
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg, DynReg tmpReg);

// call into emulator, like setFlags, getCF, etc
void callHostFunction(void* address, bool hasReturn=false, U32 argCount=0, U32 arg1=0, DynCallParamType arg1Type=DYN_PARAM_CONST_32, bool doneWithArg1=true, U32 arg2=0, DynCallParamType arg2Type=DYN_PARAM_CONST_32, bool doneWithArg2=true, U32 arg3=0, DynCallParamType arg3Type=DYN_PARAM_CONST_32, bool doneWithArg3=true, U32 arg4=0, DynCallParamType arg4Type=DYN_PARAM_CONST_32, bool doneWithArg4=true, U32 arg5=0, DynCallParamType arg5Type=DYN_PARAM_CONST_32, bool doneWithArg5=true);

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

// per instruction, not per block.  
// will allow us to determine if ecx or edx needs to be saved before calling an external function
bool regUsed[4];

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

static X86Asm x86;

void calculateEaa(DecodedOp* op, DynReg reg) {
    regUsed[reg] = true;

    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
        X86Asm::Reg16 reg16(reg);

        x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));

        if (op->data.disp) {
            x86.add(reg16, (U16)op->data.disp);
        }
        if (op->rm != 8) {
            // intentional 16-bit add
            x86.addMem(reg16, x86.edi, CPU::offsetofReg16(op->rm));
        }

        if (op->sibIndex != 8) {
            // intentional 16-bit add
            x86.addMem(reg16, x86.edi, CPU::offsetofReg16(op->sibIndex));
        }

        // seg[6] is always 0
        if (op->base < 6) {
            // intentional 32-bit add
            x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    } else {
        if (op->sibIndex != 8) {
            x86.readMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->sibIndex));
            if (op->sibScale) {
                x86.shl(X86Asm::Reg32(reg), op->sibScale);
            }
            if (op->rm != 8) {
                x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->rm));
            }
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->rm != 8) {
            x86.readMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofReg32(op->rm));
            if (op->data.disp) {
                x86.add(X86Asm::Reg32(reg), op->data.disp);
            }
        } else if (op->data.disp) {
            x86.mov(X86Asm::Reg32(reg), op->data.disp);
        } else {
            x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
        }

        // seg[6] is always 0
        if (op->base < 6 && KThread::currentThread()->process->hasSetSeg[op->base]) {
            x86.addMem(X86Asm::Reg32(reg), x86.edi, CPU::offsetofSegAddress(op->base));
        }
    }
}

void byteSwapReg32(DynReg reg) {
    x86.bswap(reg);
}

void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {    
    regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                x86.movsx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                x86.movsx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
        }
        if (doneWithSrcReg) {
            regUsed[src] = false;
        }
    }
}

void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    regUsed[dst] = true;
    if (dstWidth <= srcWidth) {
        if (dst == src) // downsizing doesn't need anything
            return;
        if (dstWidth == DYN_32bit) {
            x86.mov(X86Asm::Reg32(dst), X86Asm::Reg32(src));
        } else if (dstWidth == DYN_16bit) {
            x86.mov(X86Asm::Reg16(dst), X86Asm::Reg16(src));
        } else if (dstWidth == DYN_8bit) {
            x86.mov(X86Asm::Reg8(dst), X86Asm::Reg8(src));
        } else {
            kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromReg %d", dstWidth);
        }
    } else {
        if (dstWidth == DYN_32bit) {
            if (srcWidth == DYN_16bit) {
                x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg16(src));
            } else if (srcWidth == DYN_8bit) {
                x86.movzx(X86Asm::Reg32(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth == DYN_16bit) {
            if (srcWidth == DYN_8bit) {
                x86.movzx(X86Asm::Reg16(dst), X86Asm::Reg8(src));
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else {
            kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
        }
    }
    if (doneWithSrcReg) {
        regUsed[src] = false;
    }
}

void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {    
    regUsed[reg] = true;
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        x86.readMem(X86Asm::Reg32(reg), x86.edi, srcOffset);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(reg), x86.edi, srcOffset);
    } else if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(reg), x86.edi, srcOffset);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromCpu %d", width);
    }
}

void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (width == DYN_32bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg32(reg));
    } else if (width == DYN_16bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg16(reg));
    } else if (width == DYN_8bit) {
        x86.writeMem(x86.edi, dstOffset, X86Asm::Reg8(reg));
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpuFromReg %d", width);
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(tmpReg, srcOffset, width);

    // mov [cpu+dstOffset], tmpReg
    movToCpuFromReg(dstOffset, tmpReg, width, doneWithTmpReg);    
}

void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) {
    if (dstWidth == DYN_32bit) {
        x86.writeMem(x86.edi, dstOffset, imm);
    } else if (dstWidth == DYN_16bit) {
        x86.writeMem(x86.edi, dstOffset, (U16)imm);
    } else if (dstWidth == DYN_8bit) {
        x86.writeMem(x86.edi, dstOffset, (U8)imm);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpu %d", dstWidth);
    }
}

void zeroExtendReg16To32(DynReg dest, DynReg src) {
    x86.movzx(X86Asm::Reg32(dest), X86Asm::Reg16(src));
    regUsed[dest] = true;
}

void movToReg(DynReg reg, DynWidth width, U32 imm) {
    regUsed[reg] = true;
    if (width == DYN_32bit) {
        x86.mov(X86Asm::Reg32(reg), imm);
    } else if (width == DYN_16bit) {
        x86.mov(X86Asm::Reg16(reg), (U16)imm);
    } else if (width == DYN_8bit) {
        x86.mov(X86Asm::Reg8(reg), (U8)imm);
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

void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
    regUsed[x86.eax.reg] = true;

    if (addressReg != DYN_ADDRESS) {
        kpanic("movFromMem");
    }
    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        x86.mov(x86.eax, X86Asm::Reg32(addressReg));
        x86.and_(x86.eax, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(x86.eax, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(x86.eax, 0xFFD, false);
        }
    }
    // int index = address >> 12;
    // if (Memory::currentMMUReadPtr[index])
    //     return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
    // else
    //     return readd(address);

    x86.mov(x86.eax, X86Asm::Reg32(addressReg));
    x86.shr(x86.eax, K_PAGE_SHIFT);
    x86.readMem(x86.eax, x86.eax, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test eax, 0x40000000 (mmu[index].canReadRam)
    IfBitSet(x86.eax, 0x40000000, false);

    // bottom 20 bits of mmu contains ram page index
    x86.and_(x86.eax, 0xfffff);
    x86.readMem(x86.eax, x86.eax, 2, (U32)ramPages);
    X86Asm::Reg32 offsetReg(addressReg);

    if (!doneWithAddressReg) {
        x86.push(X86Asm::Reg32(addressReg));
    }
    x86.and_(offsetReg, K_PAGE_MASK);

    // mov eax, [eax+reg]
    if (width == DYN_8bit) {
        x86.readMem(X86Asm::Reg8(x86.eax.reg), x86.eax, offsetReg, 0, 0);
    } else if (width == DYN_16bit) {
        x86.readMem(X86Asm::Reg16(x86.eax.reg), x86.eax, offsetReg, 0, 0);
    } else if (width == DYN_32bit) {
        x86.readMem(x86.eax, x86.eax, offsetReg, 0, 0);
    }

    if (!doneWithAddressReg) {
        x86.pop(X86Asm::Reg32(addressReg));
    }

    StartElse();

    // will set EAX when call returns, so don't push it then clobber the result with a pop
    if (regUsed[x86.ecx.reg]) {
        x86.push(x86.ecx);
    }
    if (regUsed[x86.edx.reg]) {
        x86.push(x86.edx);
    }

    x86.push(X86Asm::Reg32(addressReg));

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

    x86.call(address);

    x86.add(x86.esp, 4);

    if (regUsed[x86.edx.reg]) {
        x86.pop(x86.edx);
    }
    if (regUsed[x86.ecx.reg]) {
        x86.pop(x86.ecx);
    }

    EndIf();

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
}

void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+dstOffset], eax
    movToCpuFromReg(dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

void pushValue(U32 arg, DynCallParamType argType) {
    switch (argType) {
    case DYN_PARAM_REG_8:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg8(arg));
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_16:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg16(arg));
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_REG_32:
        x86.push(X86Asm::Reg32(arg));
        break;
    case DYN_PARAM_CPU:
        x86.push(x86.edi);
        break;
    case DYN_PARAM_CONST_8:
        x86.push((U32)(arg & 0xFF));
        break;
    case DYN_PARAM_CONST_16:
        x86.push((U32)(arg & 0xFFFF));
        break;
    case DYN_PARAM_CONST_32:
        x86.push(arg);
        break;
    case DYN_PARAM_CONST_PTR:
        x86.push((U32)arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_8:
        x86.readMem(X86Asm::Reg8(x86.eax.reg), x86.edi, arg);
        x86.movzx(x86.eax, X86Asm::Reg8(x86.eax.reg));
        x86.push(x86.eax);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        x86.readMem(X86Asm::Reg16(x86.eax.reg), x86.edi, arg);
        x86.movzx(x86.eax, X86Asm::Reg16(x86.eax.reg));
        x86.push(x86.eax);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        x86.readMem(x86.eax, x86.edi, arg);
        x86.push(x86.eax);
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
void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithReg, bool doneWithAddressReg, DynReg tmp) {
    U32 firstCheckPos = 0;

    if (regUsed[tmp]) {
        kpanic("movToMem");
    }
    X86Asm::Reg32 tmpReg(tmp);

    if (width != DYN_8bit) {
        // make sure we only use the fast path if the entire read will take place on the same page
        x86.mov(tmpReg, X86Asm::Reg32(addressReg));
        x86.and_(tmpReg, K_PAGE_MASK);

        if (width == DYN_16bit) {
            // if ((address & 0xFFF) < 0xFFF)                    
            IfLessThan(tmpReg, K_PAGE_MASK, false);
        } else if (width == DYN_32bit) {
            // if ((address & 0xFFF) < 0xFFD)
            IfLessThan(tmpReg, 0xFFD, false);
        }
    }
    x86.mov(tmpReg, X86Asm::Reg32(addressReg));
    x86.shr(tmpReg, K_PAGE_SHIFT);
    x86.readMem(tmpReg, tmpReg, 2, (U32)getMemData(KThread::currentThread()->memory)->mmu);

    if (width != DYN_8bit) {
        EndIf();
    }

    // test reg, 0x80000000 (mmu[index].canWriteRam)
    IfBitSet(tmpReg, 0x80000000, false);

    // bottom 20 bits of mmu contains ram page index
    x86.and_(tmpReg, 0xfffff);
    x86.readMem(tmpReg, tmpReg, 2, (U32)ramPages);
    X86Asm::Reg32 offsetReg(addressReg);

    if (!doneWithAddressReg) {
        x86.push(x86.esi);
        offsetReg = x86.esi;
        x86.mov(offsetReg, X86Asm::Reg32(addressReg));
    }
    x86.and_(offsetReg, K_PAGE_MASK);

    if (isParamTypeReg(paramType)) {
        if (width == DYN_8bit) {
            x86.writeMem(tmpReg, offsetReg, 0, 0, X86Asm::Reg8(value));
        } else if (width == DYN_16bit) {
            x86.writeMem(tmpReg, offsetReg, 0, 0, X86Asm::Reg16(value));
        } else if (width == DYN_32bit) {
            x86.writeMem(tmpReg, offsetReg, 0, 0, X86Asm::Reg32(value));
        } else {
            kpanic("movToMem");
        }
    } else {
        if (width == DYN_8bit) {
            x86.writeMem(tmpReg, offsetReg, 0, 0, (U8)value);
        } else if (width == DYN_16bit) {
            x86.writeMem(tmpReg, offsetReg, 0, 0, (U16)value);
        } else if (width == DYN_32bit) {
            x86.writeMem(tmpReg, offsetReg, 0, 0, value);
        } else {
            kpanic("movToMem");
        }
    }
    if (!doneWithAddressReg) {
        x86.pop(x86.esi);
    }
    regUsed[tmpReg.reg] = false;

    StartElse();

    bool regDone[4] = { false, false, false, false };
    if (doneWithReg && isParamTypeReg(paramType)) {
        regDone[value] = true;
    }
    if (regUsed[x86.eax.reg] && !regDone[x86.eax.reg]) {
        x86.push(x86.eax);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.push(x86.ecx);
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.push(x86.edx);
    }

    pushValue(value, paramType);
    x86.push(X86Asm::Reg32(addressReg));

    if (width == DYN_32bit) {
        x86.call(writed);
    } else if (width == DYN_16bit) {
        x86.call(writew);
    } else if (width == DYN_8bit) {
        x86.call(writeb);
    } else {
        kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
    }

    x86.add(x86.esp, 8);

    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.pop(x86.edx);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.pop(x86.ecx);
    }
    if (regUsed[x86.eax.reg] && !regDone[x86.eax.reg]) {
        x86.pop(x86.eax);
    }
    EndIf();

    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
    if (doneWithReg && isParamTypeReg(paramType)) {
        regUsed[value] = false;
    }
}

void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg, DynReg tmpReg) {
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_REG_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_REG_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_REG_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromReg", width);

    movToMem(addressReg, width, reg, paramType, doneWithReg, doneWithAddressReg, tmpReg);
}

void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_CONST_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_CONST_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_CONST_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromImm", width);

    movToMem(addressReg, width, imm, paramType, false, doneWithAddressReg, tmpReg);
}

void callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
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
        regUsed[x86.eax.reg] = true;
    } else if (regUsed[x86.eax.reg] && !hasReturn && !regDone[x86.eax.reg]) {
        x86.push(x86.eax);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.push(x86.ecx);
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.push(x86.edx);
    }
    if (argCount >= 5) {
        pushValue(arg5, arg5Type);
    }
    if (argCount >= 4) {
        pushValue(arg4, arg4Type);
    }
    if (argCount >= 3) {
        pushValue(arg3, arg3Type);
    }
    if (argCount >= 2) {
        pushValue(arg2, arg2Type);
    }
    if (argCount >= 1) {
        pushValue(arg1, arg1Type);
    }

    x86.call(address);

    if (argCount) {
        x86.add(x86.esp, 4 * argCount);
    }
    for (int i = 0; i < 4; i++) {
        if (regDone[i]) {
            regUsed[i] = false;
        }
    }
    if (regUsed[x86.edx.reg] && !regDone[x86.edx.reg]) {
        x86.pop(x86.edx);
    }
    if (regUsed[x86.ecx.reg] && !regDone[x86.ecx.reg]) {
        x86.pop(x86.ecx);
    }
    if (!hasReturn && regUsed[x86.eax.reg] && !regDone[x86.eax.reg]) {
        x86.pop(x86.eax);
    }
}

// inst can be +, |, - , &, ^, <, >, ) right parens is for signed right shift
void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    switch (inst) {
    case '+':
        if (regWidth == DYN_32bit) {
            x86.add(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.add(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.add(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm ADD");
        }
        break;
    case '-':
        if (regWidth == DYN_32bit) {
            x86.sub(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.sub(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.sub(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SUB");
        }
        break;
    case '&':
        if (regWidth == DYN_32bit) {
            x86.and_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.and_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.and_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm AND");
        }
        break;
    case '|':
        if (regWidth == DYN_32bit) {
            x86.or_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.or_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.or_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm OR");
        }
        break;
    case '^':
        if (regWidth == DYN_32bit) {
            x86.xor_(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.xor_(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.xor_(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm XOR");
        }
        break;
    case '<':
        if (regWidth == DYN_32bit) {
            x86.shl(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.shl(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.shl(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SHL");
        }
        break;
    case '>':
        if (regWidth == DYN_32bit) {
            x86.shr(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.shr(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.shr(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SHR");
        }
        break;
    case ')':
        if (regWidth == DYN_32bit) {
            x86.sar(X86Asm::Reg32(reg), imm);
        } else if (regWidth == DYN_16bit) {
            x86.sar(X86Asm::Reg16(reg), (U16)imm);
        } else if (regWidth == DYN_8bit) {
            x86.sar(X86Asm::Reg8(reg), (U8)imm);
        } else {
            kpanic("instRegImm SAR");
        }
        break;
    default:
        kpanic("instRegImm");
    }
}
void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPUReg");
    }
    movToRegFromCpu(tmpReg, dstOffset, regWidth);
    instRegReg(inst, tmpReg, rm, regWidth, doneWithRmReg);
    movToCpuFromReg(dstOffset, tmpReg, regWidth, true);
}
void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPUImm");
    }
    movToRegFromCpu(tmpReg, dstOffset, regWidth);
    instRegImm(inst, tmpReg, regWidth, imm);
    movToCpuFromReg(dstOffset, tmpReg, regWidth, true);
}

void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg, DynReg tmpReg) {
    if (regUsed[0]) {
        kpanic("x32CPU::instMemImm");
    }
    movFromMem(regWidth, addressReg, false);
    instRegImm(inst, DYN_EAX, regWidth, imm);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true, tmpReg);
}
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg, DynReg tmpReg) {
    if (regUsed[0]) {
        kpanic("x32CPU::instMemReg");
    }    
    movFromMem(regWidth, addressReg, false);
    instRegReg(inst, DYN_EAX, rm, regWidth, doneWithRmReg);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true, tmpReg);
}

// inst can be +, |, -, &, ^, <, >, ) right parens is for signed right shift
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    switch (inst) {
    case '+':
        if (regWidth == DYN_32bit) {
            x86.add(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.add(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.add(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg ADD");
        }
        break;
    case '-':
        if (regWidth == DYN_32bit) {
            x86.sub(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.sub(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.sub(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SUB");
        }
        break;
    case '&':
        if (regWidth == DYN_32bit) {
            x86.and_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.and_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.and_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg AND");
        }
        break;
    case '|':
        if (regWidth == DYN_32bit) {
            x86.or_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.or_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.or_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg OR");
        }
        break;
    case '^':
        if (regWidth == DYN_32bit) {
            x86.xor_(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.xor_(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.xor_(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg XOR");
        }
        break;
    case '<':
        if (regWidth == DYN_32bit) {
            x86.shl(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.shl(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.shl(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SHL");
        }
        break;
    case '>':
        if (regWidth == DYN_32bit) {
            x86.shr(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.shr(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.shr(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SHR");
        }
        break;
    case ')':
        if (regWidth == DYN_32bit) {
            x86.sar(X86Asm::Reg32(reg), X86Asm::Reg32(rm));
        } else if (regWidth == DYN_16bit) {
            x86.sar(X86Asm::Reg16(reg), X86Asm::Reg16(rm));
        } else if (regWidth == DYN_8bit) {
            x86.sar(X86Asm::Reg8(reg), X86Asm::Reg8(rm));
        } else {
            kpanic("instRegReg SAR");
        }
        break;
    default:
        kpanic("instRegReg");
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

// inst can be: ~ or -
void instReg(char inst, DynReg reg, DynWidth regWidth) {
    if (inst == '-') {
        if (regWidth == DYN_32bit) {
            x86.neg(X86Asm::Reg32(reg));
        } else if (regWidth == DYN_16bit) {
            x86.neg(X86Asm::Reg16(reg));
        } else if (regWidth == DYN_8bit) {
            x86.neg(X86Asm::Reg8(reg));
        } else {
            kpanic("instReg NEG");
        }
    } else if (inst == '~') {
        if (regWidth == DYN_32bit) {
            x86.not_(X86Asm::Reg32(reg));
        } else if (regWidth == DYN_16bit) {
            x86.not_(X86Asm::Reg16(reg));
        } else if (regWidth == DYN_8bit) {
            x86.not_(X86Asm::Reg8(reg));
        } else {
            kpanic("instReg NOT");
        }
    } else {
        kpanic("instReg");
    }
}

void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg, DynReg tmpReg) {
    if (regUsed[0]) {
        kpanic("x32CPU::instMem");
    }  
    movFromMem(regWidth, addressReg, false);
    instReg(inst, DYN_EAX, regWidth);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true, tmpReg);   
}

void instCPU(char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPU");
    }
    movToRegFromCpu(tmpReg, dstOffset, regWidth);
    instReg(inst, tmpReg, regWidth);
    movToCpuFromReg(dstOffset, tmpReg, regWidth, true);
}

void JumpIf(DynamicData* data, DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jnz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void JumpIfNot(DynamicData* data, DynReg reg, bool doneWithReg, U32 address) {
    x86.test(X86Asm::Reg32(reg), X86Asm::Reg32(reg));
    x86.jz(address);
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void JumpInBlock(DynamicData* data, U32 address) {
    x86.jmp(address);
}

void IfPtrEqual(X86Asm::Reg32 reg, DYN_PTR_SIZE value, bool doneWithReg) {
    IfEqual(reg, value, doneWithReg);
}

void IfLessThan(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfLessThan(reg, value);

    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void IfEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void IfNotEqual(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfNotEqual(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}


void IfBitSet(X86Asm::Reg32 reg, U32 value, bool doneWithReg) {
    x86.IfBitSet(reg, value);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void If(X86Asm::Reg32 reg, bool doneWithReg) {
    x86.IfNotZero(reg);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void IfNot(X86Asm::Reg32 reg, bool doneWithReg) {
    x86.IfZero(reg);
    if (doneWithReg) {
        regUsed[reg.reg] = false;
    }
}

void StartElse() {
    x86.Else();
}

void EndIf() {
    x86.EndIf();
}

void setCC(X86Asm::Reg32 reg, DynConditionEvaluate condition) {

    switch (condition) {
    case DYN_EQUALS:
        x86.setz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_NOT_EQUALS:
        x86.setnz(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_UNSIGNED:
        x86.setb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        x86.setbe(X86Asm::Reg8(reg.reg));
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:
        x86.setnb(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_SIGNED:
        x86.setl(X86Asm::Reg8(reg.reg));
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        x86.setle(X86Asm::Reg8(reg.reg));
        break;
    default:
        kpanic_fmt("x32CPU::evaluateToRegFromRegs unknown condition %d", condition);
    }
}

void evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, X86Asm::Reg32 right, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (regWidth == DYN_32bit) {
        x86.cmp(left, right);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(left.reg), X86Asm::Reg16(right.reg));
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(left.reg), X86Asm::Reg8(right.reg));
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    regUsed[reg.reg] = true;
    setCC(reg, condition);
    x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        regUsed[left.reg] = false;
    }
    if (doneWithRightReg) {
        regUsed[right.reg] = false;
    }
}

void evaluateToReg(X86Asm::Reg32 reg, X86Asm::Reg32 left, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg) {
    if (regWidth == DYN_32bit) {
        x86.cmp(left, rightConst);
    } else if (regWidth == DYN_16bit) {
        x86.cmp(X86Asm::Reg16(left.reg), (U16)rightConst);
    } else if (regWidth == DYN_8bit) {
        x86.cmp(X86Asm::Reg8(left.reg), (U8)rightConst);
    } else {
        kpanic_fmt("x32CPU::evaluateToReg reg width %d", regWidth);
    }

    regUsed[reg.reg] = true;
    setCC(reg, condition);
    x86.movzx(reg, X86Asm::Reg8(reg.reg));

    if (doneWithLeftReg) {
        regUsed[left.reg] = false;
    }
}

void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (reg>=4) {
        kpanic_fmt("x32CPU::evaluateToRegFromRegs doesn't support reg %d", reg);
    }
    if (isRightConst) {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), rightConst, regWidth, condition, doneWithLeftReg);
    } else {
        evaluateToReg(X86Asm::Reg32(reg), X86Asm::Reg32(left), X86Asm::Reg32(right), regWidth, condition, doneWithLeftReg, doneWithRightReg);
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
    x86.test(x86.eax, x86.eax);
    if (setnz) {
        x86.setnz(X86Asm::Reg8(x86.eax.reg));
    } else {
        x86.setz(X86Asm::Reg8(x86.eax.reg));
    }
}

void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    setConditional(data, condition);

    if (regWidth != DYN_8bit) {
        movToRegFromReg(DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    movToCpuFromReg(offset, DYN_CALL_RESULT, regWidth, true);
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
        movToRegFromReg(DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true, tmpReg);
}

void incrementEip(U32 inc) {
    x86.addMem(x86.edi, offsetof(CPU, eip.u32), inc);
}

void incrementEip(DynamicData* data, DecodedOp* op) {
    incrementEip(op->len);
}

void incrementEip(DynamicData* data, U32 len) {
    incrementEip(len);
}

void blockCall(DynamicData* data, DecodedOp* op) {
    blockNext1(data, op);
}

void blockDoneCall(DynamicData* data) {
    blockDone(data, false);
}

void blockDone(DynamicData* data, bool returnEarly) {
    // cpu->nextOp = cpu->nextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, true);
    if (returnEarly) {
        x86.pop(x86.edi);
        x86.pop(x86.ebx);

#ifdef _DEBUG
        x86.mov(x86.esp, x86.ebp);
        x86.pop(x86.ebp);
#endif

        x86.ret();
    }
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1(DynamicData* data, DecodedOp* op) {
    // if (!(*(op->nextJump))) {
    //     *(op->nextJump) = cpu->getNextOp();
    // }
    // cpu->nextOp = *(op->nextJump);
    movToReg(DYN_EDX, DYN_32bit, (U32)op);
    // ebx = op->nextJump
    // mov ebx, [edx + offsetof(DecodedOp, nextJump)]
    regUsed[DYN_EBX] = true;
    x86.readMem(x86.ebx, x86.edx, offsetof(DecodedOp, data.nextJump));
    regUsed[DYN_EDX] = false;

    // eax = *(op->nextJump)
    x86.readMem(x86.eax, x86.ebx, 0);
    // if (!(*(op->nextJump))) 
    IfNot(DYN_EAX, false);
    // *(op->nextJump) = cpu->getNextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    x86.writeMem(x86.ebx, 0, x86.eax);
    EndIf();

    // cpu->nextOp = *(op->nextJump);        
    regUsed[DYN_EBX] = false;
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_EAX, DYN_32bit, false);
    
#ifdef BOXEDWINE_MULTI_THREADED
    x86.readMem(x86.eax, x86.eax, offsetof(DecodedOp, pfnJitCode));
    If(DYN_CALL_RESULT, true);
    x86.jmp(x86.eax);
    EndIf();
#endif
}

void blockNext2(DynamicData* data, DecodedOp* op) {
    // if (!op->next) { 
    //     op->next = cpu->getNextOp(); 
    // }
    // cpu->nextOp = op->next;    
    movToReg(DYN_EBX, DYN_32bit, (U32)op);

    // mov eax, [ebx + offsetof(DecodedOp, next)]
    x86.readMem(x86.eax, x86.ebx, offsetof(DecodedOp, next));

    IfNot(DYN_EAX, false);
    // op->next = cpu->getNextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    // mov [ebx + offsetof(DecodedOp, next)], eax
    x86.writeMem(x86.ebx, offsetof(DecodedOp, next), x86.eax);
    EndIf();
    regUsed[DYN_EBX] = false;

    // cpu->nextOp = op->next
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, false);

#ifdef BOXEDWINE_MULTI_THREADED
    x86.readMem(x86.eax, x86.eax, offsetof(DecodedOp, pfnJitCode));
    If(DYN_CALL_RESULT, true);
    x86.jmp(x86.eax);
    EndIf();
#endif 
}

void x32_sidt(DynamicData* data, DecodedOp* op) {
}

void x32_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void x32_callback(DynamicData* data, DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        callHostFunction(x32_onExitSignal, false, 1, 0, DYN_PARAM_CPU);
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

static U8* createDynamicExecutableMemory(KMemory* processMemory) {
    DynamicMemory* memory = getMemData(processMemory)->dynamicMemory;
    if (!memory) {
        memory = new DynamicMemory();
        getMemData(processMemory)->dynamicMemory = memory;
    }

    void* mem = NULL;

    if (memory->dynamicExecutableMemory.size() == 0) {
        int blocks = (x86.buffer.size() + 0xffff) / 0x10000;
        memory->dynamicExecutableMemoryLen = blocks * 0x10000;
        mem = Platform::alloc64kBlock(blocks, true);
        memory->dynamicExecutableMemoryPos = 0;
        memory->dynamicExecutableMemory.push_back(DynamicMemoryData(mem, blocks * 0x10000));
    } else {
        mem = memory->dynamicExecutableMemory[memory->dynamicExecutableMemory.size() - 1].p;
        if (memory->dynamicExecutableMemoryPos + x86.buffer.size() >= memory->dynamicExecutableMemoryLen) {
            int blocks = (x86.buffer.size() + 0xffff) / 0x10000;
            memory->dynamicExecutableMemoryLen = blocks * 0x10000;
            mem = Platform::alloc64kBlock(blocks, true);
            memory->dynamicExecutableMemoryPos = 0;
            memory->dynamicExecutableMemory.push_back(DynamicMemoryData(mem, blocks * 0x10000));
        }
    }
    U8* begin = (U8*)mem + memory->dynamicExecutableMemoryPos;
    memcpy(begin, x86.buffer.data(), x86.buffer.size());
    memory->dynamicExecutableMemoryPos += x86.buffer.size();
    return begin;
}

static U8* createStartJITCode(KMemory* memory) {
    x86.reset();

#ifdef _DEBUG
    x86.push(x86.ebp);
    x86.mov(x86.ebp, x86.esp);
#endif

    x86.push(x86.ebx);
    x86.push(x86.edi);
    // on win32 ecx contains cpu
    x86.mov(x86.edi, x86.ecx);

    // :TODO: what about other x86 platforms that use a different calling convention
    // 
    // jmp ((DecodedOp*)edx)->pfn
    x86.readMem(x86.eax, x86.edx, offsetof(DecodedOp, pfnJitCode));
    x86.jmp(x86.eax);
    U8* result = createDynamicExecutableMemory(memory);
    return result;
}

static void doJIT(CPU* cpu, DecodedOp* op) {
    static int count;
    static int functionCount;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(DecodedOpCache::lock);

        if (!cpu->thread->process->startJITOp) {
            cpu->thread->process->startJITOp = (OpCallback)createStartJITCode(cpu->memory);
        }

        // did another thread beat us to JITing this block?
        if (op->flags & OP_FLAG_JIT) {
            // this will get triggered a few times, especially during shutdown
            // I have see this in firefight installer at the end and opentdd start up
            return;
        }
        std::vector<DecodedFunctionOp> ops;
        U32 emulatedLen = 0;
        U32 opCount = 0;
        DynamicData data;
        data.cpu = cpu;
        data.currentEip = cpu->getEipAddress();
        data.startingEip = cpu->getEipAddress();
       
        initX32Ops();
        DecodedOp* nextOp = op;
        x86.reset();
        count++;

        U32 firstPassFarthestEip = cpu->getEipAddress();

        // find the longest block we can compile
        // branches that jump out of the block will be the end of the block

        // 1st pass, find longest block including all direct jumps (conditional jumps, direct jumps, loop, etc)
        
        // jumpTo will keep track of valid jump targets.  We need this if we are going to decode more instructions (cpu->getOp)
        // Without this the next byte of instruction may actually be invalid, I have seen skipped bytes in the instructions,
        // I assume its for alignment/performance reasons.  Firefight installer will trigger this
        BHashTable<U32, DecodedOp*> jumpTo;
        while (nextOp) {
            if (nextOp->isBranch() && (nextOp->isCall() || !nextOp->isDirectBranch())) {
                break;
            }
            if (nextOp->isDirectBranch() && (firstPassFarthestEip + nextOp->len + nextOp->imm) < data.startingEip) {
                break;
            }
            if (nextOp->isDirectBranch()) {
                jumpTo.set(firstPassFarthestEip + nextOp->len + nextOp->imm, nextOp);
            }
            //
            // how to handle a call deep down in an if statement 
            //
            // allow call if there are valid jumps that go over it, the call should do an early block return in this case
            if (nextOp->isDirectBranch() && !nextOp->next && jumpTo.contains(firstPassFarthestEip + nextOp->len)) {                
                // this doesn't lock cpu->memory->mutext
                nextOp->next = cpu->memory->getDecodedOp(firstPassFarthestEip + nextOp->len);
                if (!nextOp->next) {
                    // don't hold lock since getOp will lock memory->mutex, this creates an issue with CodePage which will lock memory->mutex then DecodedOpCache::lock
                    BOXEDWINE_MUTEX_UNLOCK(DecodedOpCache::lock);
                    nextOp->next = cpu->getOp(firstPassFarthestEip + nextOp->len, 0);
                    BOXEDWINE_MUTEX_LOCK(DecodedOpCache::lock);
                    x86.reset(); // this is global and could have been used when we were unlocked
                    if (op->flags & OP_FLAG_JIT) {
                        // In the small time we allowed other threads access to the JIT, someone else beat us to the op we were about to JIT
                        return;
                    }
                }
            }
            firstPassFarthestEip += nextOp->len;
            if (nextOp->next) {
                if (nextOp->next->flags & OP_FLAG_NO_JIT) {
                    break;
                }
            }            
            nextOp = nextOp->next;
        }        
        // find longest block where all direction jumps don't go past the block
        U32 lastFurthestEip = firstPassFarthestEip;
        while (true) {
            nextOp = op;
            data.lastOpEip = cpu->getEipAddress();
            while (nextOp && data.lastOpEip < lastFurthestEip) {
                if (nextOp->isDirectBranch()) {
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
        nextOp = op;

        while (nextOp) {
            if (nextOp->flags & OP_FLAG_NO_JIT) {
                count--;
                return;
            }

            memset(regUsed, 0, sizeof(regUsed));
#ifndef __TEST
#ifdef _DEBUG
            //callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)nextOp, DYN_PARAM_CONST_PTR, false);
#endif
#endif
            emulatedLen += nextOp->len;
            opCount++;
            data.eipToBufferPos.set(data.currentEip, x86.buffer.size());
            if (nextOp->lock) {
                // so that intra block jumps that try to skip a lock will find the lock version of the op anyway
                data.eipToBufferPos.set(data.currentEip+1, x86.buffer.size());
            }
            ops.push_back(DecodedFunctionOp(data.currentEip, nextOp));
            x32Ops[nextOp->inst](&data, nextOp);
            data.currentEip += nextOp->len;
            if (x86.ifJump.size()) {
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
        x86.pop(x86.edi);
        x86.pop(x86.ebx);

#ifdef _DEBUG
        x86.mov(x86.esp, x86.ebp);
        x86.pop(x86.ebp);
#endif
        x86.ret();

        for (DynamicJump& jmp : x86.jumps) {
            U32 bufferIndex = 0;

            if (!data.eipToBufferPos.get(jmp.eip, bufferIndex)) {
                kpanic("x32CPU firstDynamicOp");
            }
            *(U32*)&x86.buffer.data()[jmp.bufferPos] = bufferIndex - jmp.bufferPos - 4;
        }

        U8* begin = createDynamicExecutableMemory(cpu->memory);

        for (U32 i = 0; i < x86.patch.size(); i++) {
            U32 pos = x86.patch[i];
            U32* value = (U32*)(&begin[pos]);
            *value = *value - (U32)(begin + pos + 4);
        }
        
        if (!ops.size()) {
            return;
        }
        DecodedOp* chunkOp = ops.front().op;
        U32 chunkAddress = ops.front().address;
        U32 chunkLen = 0;

        for (DecodedFunctionOp& fop : ops) {
            if (chunkOp != fop.op && (chunkAddress + chunkLen != fop.address)) {
                getMemData(cpu->memory)->opCache.removeJITCode(chunkAddress, chunkLen);
                getMemData(cpu->memory)->opCache.addJITCode_nolock(chunkOp, chunkAddress, chunkLen);
                chunkOp = fop.op;
                chunkLen = 0;
                chunkAddress = fop.address;
            }
            chunkLen += fop.op->len;
        }
        if (chunkLen) {
            getMemData(cpu->memory)->opCache.removeJITCode(chunkAddress, chunkLen);
            getMemData(cpu->memory)->opCache.addJITCode_nolock(chunkOp, chunkAddress, chunkLen);
        }
#if !defined(BOXEDWINE_MULTI_THREADED)
        op->blockOpCount = opCount;
#endif        
        /*
        static int totalOps;
        totalOps += ops.size();
        if ((count % 1000) == 0) {
            klog_fmt("%d (ave %d)", count, ((totalOps + count/2)/count));
        }
        */
        
        for (DecodedFunctionOp& fop : ops) {
            U32 bufferIndex = 0;

            if (!data.eipToBufferPos.get(fop.address, bufferIndex)) {
                kpanic("x32CPU firstDynamicOp");
            }
            fop.op->pfnJitCode = (OpCallback)(begin + bufferIndex);
            fop.op->pfn = cpu->thread->process->startJITOp;            
            fop.op->flags |= OP_FLAG_JIT;
        }
    }
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {    
#ifdef __TEST
    if (op->runCount == 0) {
#else
    if (op->runCount == JIT_RUN_COUNT) {
#endif        
        doJIT(cpu, op);
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
        doJIT(cpu, op);
        op->runCount = JIT_RUN_COUNT + 1;
    }
    return op->pfnJitCode;
}

void blockDoneJump(DynamicData* data) {
    // :TODO: what about caching result for direct calls or direct jumps?
    callHostFunction(getJitFunctionForCurrentOp, true, 1, 0, DYN_PARAM_CPU, false);
    IfNot(DYN_EAX, true);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

#ifdef _DEBUG
    x86.mov(x86.esp, x86.ebp);
    x86.pop(x86.ebp);
#endif
    x86.ret();

    EndIf();

    x86.jmp(x86.eax);
}

#endif