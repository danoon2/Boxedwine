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
void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg);
void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg);

// arith
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg);
void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg, DynReg tmpReg);

void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg);
void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm, DynReg tmpReg);

void instReg(char inst, DynReg reg, DynWidth regWidth);
void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg);
void instCPU(char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg);

// if conditions
void jumpIf(DynamicData* data, DynReg reg, DynCondition condition, bool doneWithReg, U32 address, DynWidth regWidth = DYN_32bit);
void startIf(DynReg reg, DynCondition condition, bool doneWithReg, DynWidth regWidth = DYN_32bit);
void startIfCmpValue(DynReg reg, U32 value, bool isEqual, DynWidth regWidth, bool doneWithReg);
void startIfCmpPtr(DynReg reg, DYN_PTR_SIZE value, bool isEqual, bool doneWithReg);
void startIf(DynReg reg, DynReg reg2, bool isEqual, DynWidth regWidth, bool doneWithReg, bool doneWithReg2);
void startElse();
void endIf();
void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition);
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg);

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

#include "x86Asm.h"

static X86Asm x86;

void outb(U8 b) {
    x86.buffer.push_back(b);
}

void outw(U16 w) {
    outb((U8)w);
    outb((U8)(w>>8));
}

void outd(U32 d) {
    outb((U8)d);
    outb((U8)(d>>8));
    outb((U8)(d>>16));
    outb((U8)(d>>24));
}

void calculateEaa(DecodedOp* op, DynReg reg) {
    regUsed[reg]=true;

    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)

        // xor eax
        outb(0x31);
        outb(0xc0|reg|(reg<<3));

        // add ax, op->disp 
        if (op->data.disp) {
            outb(0x66);
            outb(0x81);
            outb(0xC0+reg);
            outw(op->data.disp);
        }
        // add ax, [edi+cpu->reg[op->rm].u16]
        if (op->rm != 8) {
            outb(0x66);
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg16(op->rm));
        }

        // add ax, [cpu->reg[op->sibIndex].u16]
        if (op->sibIndex != 8) {
            outb(0x66);
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg16(op->sibIndex));
        }

        // seg[6] is always 0
        if (op->base<6) { 
            // add eax, [cpu->seg[op->base].address]
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofSegAddress(op->base));
        }
    } else {
        // cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->disp
        bool initiallized = false;

        if (op->sibIndex!=8) {
            initiallized = true;
            // mov eax, [cpu->reg[op->sibIndex].u32];
            outb(0x8b);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg32(op->sibIndex));

            if (op->sibScale) {                
                // shl eax, op->sibScale
                if (op->sibScale==1) {
                    outb(0xd1);
                    outb(0xe0 | reg);
                } else {
                    outb(0xc1);
                    outb(0xe0 | reg);
                    outb(op->sibScale);
                }
            }
            // seg[6] is always 0
            if (op->base<6 && KThread::currentThread()->process->hasSetSeg[op->base]) { 
                // add eax, [cpu->seg[op->base].address]
                outb(0x03);
                outb(0x47 | (reg << 3));
                outb(CPU::offsetofSegAddress(op->base));
            }
        } else {
            // seg[6] is always 0
            if (op->base<6 && KThread::currentThread()->process->hasSetSeg[op->base]) { 
                initiallized = true;
                // mov eax, [cpu->seg[op->base].address]
                outb(0x8b);
                outb(0x47 | (reg << 3));
                outb(CPU::offsetofSegAddress(op->base));
            }
        }
        // add eax, [cpu->reg[op->rm].u32]
        if (op->rm != 8) {
            if (!initiallized) {
                initiallized = true;
                outb(0x8b); // mov
            } else {
                outb(0x03); // add
            }
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg32(op->rm));
        }

        // add eax, op->disp 
        if (op->data.disp) {
            if (!initiallized) {
                initiallized = true;
                outb(0xb8+reg); // mov
            } else {
                outb(0x81); // add
                outb(0xc0 | reg);
            }            
            outd(op->data.disp);
        }       
        if (!initiallized) {
            // xor reg, reg
            outb(0x31);
            outb(0xc0|reg|(reg<<3));
        }
    }
}

void byteSwapReg32(DynReg reg) {
    outb(0x0f);
    outb(0xc8 + reg);
}

void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {    
    regUsed[dst] = true;
    if (dstWidth<=srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (dstWidth==DYN_32bit) {
            if (srcWidth==DYN_16bit) {
                outb(0x0f);
                outb(0xbf);
                outb(0xC0 | (src << 3) | dst);
            } else if (srcWidth==DYN_8bit) {
                outb(0x0f);
                outb(0xbe);
                outb(0xC0 | (src << 3) | dst);
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth==DYN_16bit) {
            if (srcWidth==DYN_8bit) {
                outb(0x66);
                outb(0x0f);
                outb(0xbe);
                outb(0xC0 | (src << 3) | dst);
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
    if (dstWidth<=srcWidth) {
        if (dst==src) // downsizing doesn't need anything
            return;
        if (dstWidth==DYN_32bit) {
            outb(0x89);
            outb(0xC0 | (src << 3) | dst);
        } else if (dstWidth==DYN_16bit) {
            outb(0x66);
            outb(0x89);
            outb(0xC0 | (src << 3) | dst);
        } else if (dstWidth==DYN_8bit) {
            outb(0x88);
            if (src>=4 || dst>=4) {
                kpanic("x32CPU::movToRegFromReg invalid code, only first 4 regs allowed");
            }
            outb(0xC0 | (src << 3) | dst);
        }  else {
            kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromReg %d", dstWidth);
        }
    } else {
        if (dstWidth==DYN_32bit) {
            if (srcWidth==DYN_16bit) {
                outb(0x0f);
                outb(0xb7);
                outb(0xC0 | (dst << 3) | src);
            } else if (srcWidth==DYN_8bit) {
                outb(0x0f);
                outb(0xb6);
                outb(0xC0 | (dst << 3) | src);
            } else {
                kpanic_fmt("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth==DYN_16bit) {
            if (srcWidth==DYN_8bit) {
                outb(0x66);
                outb(0x0f);
                outb(0xb6);
                outb(0xC0 | (dst << 3) | src);
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
        outb(0x8b);
    } else if (width == DYN_16bit) {
        outb(0x66);
        outb(0x8b);
    } else if (width == DYN_8bit) {
        outb(0x8a);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToRegFromCpu %d", width);
    }

    if (srcOffset<=127) {
        outb(0x47|(reg << 3));
        outb((U8)srcOffset);
    } else {
        outb(0x87|(reg << 3));
        outd(srcOffset);
    }
}

void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    // mov [edi+dstOffset], reg
    if (width == DYN_32bit) {
        outb(0x89);
    } else if (width == DYN_16bit) {
        outb(0x66);
        outb(0x89);
    } else if (width == DYN_8bit) {
        outb(0x88);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpuFromReg %d", width);
    }
    if (dstOffset<=127) {
        outb(0x47|(reg << 3));
        outb((U8)dstOffset);
    } else {
        outb(0x87|(reg << 3));
        outd(dstOffset);
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
    // mov [cpu+dstOffset], imm
    if (dstWidth == DYN_32bit) {
        outb(0xc7);
    } else if (dstWidth == DYN_16bit) {
        outb(0x66);
        outb(0xc7);
    } else if (dstWidth == DYN_8bit) {
        outb(0xc6);
    } else {
        kpanic_fmt("unknown dstWidth in x32CPU::movToCpu %d", dstWidth);
    }

    if (dstOffset<=127) {
        outb(0x47);
        outb((U8)dstOffset);
    } else {
        outb(0x87);
        outd(dstOffset);
    }

    if (dstWidth==DYN_32bit) {
        outd(imm);
    } else if (dstWidth==DYN_16bit) {
        outw(imm);
    } else if (dstWidth == DYN_8bit) {
        outb(imm);
    } else {
        kpanic_fmt("unknown width in x32CPU::movToCpu %d", dstWidth);
    }
}

void zeroExtendReg16To32(DynReg dest, DynReg src) {
    outb(0x0f);
    outb(0xb7);
    outb(0xc0 | src | (dest << 3));
}

void movToReg(DynReg reg, DynWidth width, U32 imm) {
    regUsed[reg] = true;
    if (imm==0) {
        outb(0x31);
        outb(0xc0|reg|(reg << 3));
    } else {
        outb(0xb8+reg);
        outd(imm);
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
    regUsed[DYN_EAX] = true;
    U32 firstCheckPos=0;

    // make sure we only use the fast path if the entire read will take place on the same page
    if (width==DYN_16bit) {
        // if ((address & 0xFFF) < 0xFFF)
    
        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3));

        // and eax, 0xfff
        outb(0x25);
        outd(0xFFF);

        // cmp eax, 0xfff
        outb(0x3D);
        outd(0xfff);

        // jnb
        outb(0x73);
        firstCheckPos = x86.buffer.size();
        outb(0);

    } else if (width==DYN_32bit) {
        // if ((address & 0xFFF) < 0xFFD)

        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3));

        // and eax, 0xfff
        outb(0x25);
        outd(0xFFF);

        // cmp eax, 0xffd
        outb(0x3D);
        outd(0xffd);

        // jnb
        outb(0x73);
        firstCheckPos = x86.buffer.size();
        outb(0);
    }

    // int index = address >> 12;
    // if (Memory::currentMMUReadPtr[index])
    //     return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
    // else
    //     return readd(address);

    // mov eax, addressReg
    outb(0x89);
    outb(0xc0 | (addressReg<<3));

    // address >> 12
    // shr eax, 12
    outb(0xc1);
    outb(0xe8);
    outb(0x0c);

    // mov eax, [mmu+sizeof(MMU)*index];
    outb(0x8b);
    outb(0x04);
    outb(0x85);
    outd((U32)getMemData(KThread::currentThread()->memory)->mmu);

    // test eax, 0x40000000 (mmu[index].canReadRam)
    outb(0xa9);
    outd(0x40000000);

    // jz
    outb(0x74);
    U32 jzPos = x86.buffer.size();
    outb(0); // skip over cached read

    // and eax, 0xfffff (bottom 20 bits of mmu contains ram page index)
    outb(0x25);
    outd(0xfffff);

    // mov eax, [eax*4 + ramPages]
    outb(0x8b);
    outb(0x04);
    outb(0x85);
    outd((U32)ramPages);

    // mov eax, [eax+(address & 0xFFF)]
    U32 reg;
    bool pushedReg = false;
    if (doneWithAddressReg) {
        reg = addressReg;
    } else {
        if (!regUsed[DYN_ECX]) {
            reg = DYN_ECX;
        } else if (!regUsed[DYN_EDX]) {
            reg = DYN_EDX;
        } else {
    #ifdef _DEBUG
            klog("movFromMem ran out of regs");
    #endif
            reg = DYN_ECX;
            pushedReg = true;
            outb(0x51);
        }

        // mov reg, addressReg
        outb(0x89);
        outb(0xc0|reg|(addressReg<<3));
    }
    // and reg, 0xfff
    outb(0x81);
    outb(0xe0+reg);
    outd(0xfff);

    // mov eax, [eax+reg]
    if (width==DYN_8bit) {
        outb(0x8a);
        outb(0x04);
        outb(reg << 3);
    } else if (width==DYN_16bit) {
        outb(0x66);
        outb(0x8b);
        outb(0x04);
        outb(reg << 3);
    } else if (width==DYN_32bit) {
        outb(0x8b);
        outb(0x04);
        outb(reg << 3);
    }

    if (pushedReg) {
        outb(0x59);
    }

    // jmp over slow read
    outb(0xeb);
    U32 slowPos = x86.buffer.size();
    outb(0);

    x86.buffer[jzPos] = (U8)(x86.buffer.size() -jzPos-1);
    if (firstCheckPos)
        x86.buffer[firstCheckPos] = (U8)(x86.buffer.size() -firstCheckPos-1);

    // will set EAX so don't push it then clobber the result with a pop

    if (regUsed[DYN_ECX] && addressReg!=DYN_ECX)
        outb(0x51);
    if (regUsed[DYN_EDX] && addressReg!=DYN_EDX)
        outb(0x52);

    // push addressReg
    outb(0x50+addressReg);

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

    outb(0xe8);
    x86.patch.push_back(x86.buffer.size());
    outd((U32)address);

    // add esp, 4
    outb(0x83);
    outb(0xc4);
    outb(0x04);
    
    if (regUsed[DYN_EDX] && addressReg!=DYN_EDX)
        outb(0x5a);
    if (regUsed[DYN_ECX] && addressReg!=DYN_ECX)
        outb(0x59);

    x86.buffer[slowPos] =  (U8)(x86.buffer.size() -slowPos-1);
    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
}

void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+srcOffset], eax
    movToCpuFromReg(dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

void pushValue(U32 arg, DynCallParamType argType, bool doneWithReg) {
    switch (argType) {
    case DYN_PARAM_REG_8:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg8(arg));
        x86.push(X86Asm::Reg32(arg));
        if (doneWithReg) {
            regUsed[arg] = false;
        }
        break;
    case DYN_PARAM_REG_16:
        x86.movzx(X86Asm::Reg32(arg), X86Asm::Reg16(arg));
        x86.push(X86Asm::Reg32(arg));
        if (doneWithReg) {
            regUsed[arg] = false;
        }
        break;
    case DYN_PARAM_REG_32:
        x86.push(X86Asm::Reg32(arg));
        if (doneWithReg) {
            regUsed[arg] = false;
        }
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
void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithValueReg) {    
    U32 firstCheckPos=0;

    U32 reg1;
    bool pushedReg1 = false;
    bool isParamReg = isParamTypeReg(paramType);
    if (!regUsed[DYN_EAX] && !(isParamReg && value == DYN_EAX)) {
        reg1 = DYN_EAX;
    } else if (!regUsed[DYN_ECX] && !(isParamReg && value == DYN_ECX)) {
        reg1 = DYN_ECX;
    } else if (!regUsed[DYN_EDX] && !(isParamReg && value == DYN_EDX)) {
        reg1 = DYN_EDX;
    } else if (!regUsed[DYN_EBX] && !(isParamReg && value == DYN_EBX)) {
        reg1 = DYN_EBX;
    } else {
        if (isParamReg && value == DYN_EAX) {
            reg1 = DYN_EDX;
        } else {
            reg1 = DYN_EAX;
        }
        outb(0x50+reg1);
        pushedReg1 = true;
    }

    // make sure we only use the fast path if the entire read will take place on the same page
    if (width==DYN_16bit) {
        // if ((address & 0xFFF) < 0xFFF)
    
        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3) | reg1);

        // and eax, 0xfff
        if (reg1==DYN_EAX) {
            outb(0x25);
            outd(0xFFF);
        } else {
            outb(0x81);
            outb(0xe0|reg1);
            outd(0xfff);
        }

        // cmp eax, 0xfff
        if (reg1==DYN_EAX) {
            outb(0x3D);
            outd(0xfff);
        } else {
            outb(0x81);
            outb(0xf8|reg1);
            outd(0xfff);
        }

        // jnb
        outb(0x73);
        firstCheckPos = x86.buffer.size();
        outb(0);

    } else if (width==DYN_32bit) {
        // if ((address & 0xFFF) < 0xFFD)

        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3) | reg1);

        // and eax, 0xfff
        if (reg1==DYN_EAX) {
            outb(0x25);
            outd(0xFFF);
        } else {
            outb(0x81);
            outb(0xe0|reg1);
            outd(0xfff);
        }

        // cmp eax, 0xffd
        if (reg1==DYN_EAX) {
            outb(0x3D);
            outd(0xffd);
        } else {
            outb(0x81);
            outb(0xf8|reg1);
            outd(0xffd);
        }

        // jnb
        outb(0x73);
        firstCheckPos = x86.buffer.size();
        outb(0);
    }

    // int index = address >> 12;
    // if (Memory::currentMMUWritePtr[index])
    //     *(U32*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
    // else
    //     Memory::currentMMU[index]->writed(address, value);	

    // mov reg1, addressReg
    outb(0x89);
    outb(0xc0 | (addressReg<<3) | reg1);

    // address >> 12
    // shr reg1, 10
    outb(0xc1);
    outb(0xe8 | reg1);
    outb(0x0c);

    // mov reg1, [mmu+sizeof(MMU*)*index];
    outb(0x8b);
    outb(0x04|(reg1<<3));
    outb(0x85|(reg1<<3));
    outd((U32)getMemData(KThread::currentThread()->memory)->mmu);

    // test reg1, 0x80000000
    outb(0xf7);
    outb(0xc0 | reg1);
    outd(0x80000000);

    // jz
    outb(0x74);
    U32 jzPos = x86.buffer.size();
    outb(0); // skip over cached read

    // and reg1, 0xfffff (bottom 20 bits of mmu contains ram page index)
    outb(0x81);
    outb(0xe0 | reg1);
    outd(0xfffff);

    // mov reg1, [reg1*4 + ramPages]
    outb(0x8b);
    outb(0x04 | (reg1 << 3));
    outb(0x85 | (reg1 << 3));
    outd((U32)ramPages);

    // mov eax, [eax+(address & 0xFFF)]
    U32 reg2;
    bool pushedReg2 = false;
    if (!regUsed[DYN_ECX] && reg1!=DYN_ECX && !(isParamReg && value == DYN_ECX)) {
        reg2 = DYN_ECX;
    } else if (!regUsed[DYN_EDX] && reg1!=DYN_EDX && !(isParamReg && value == DYN_EDX)) {
        reg2 = DYN_EDX;
    } else if (!regUsed[DYN_EBX] && reg1!=DYN_EBX && !(isParamReg && value == DYN_EBX)) {
        reg2 = DYN_EBX;
    } else {
        if ((isParamReg && value == DYN_ECX) || reg1 == DYN_ECX) {
            if (reg1==DYN_EAX || (isParamReg && value == DYN_EAX)) {
                reg2 = DYN_EDX;
            } else {
                reg2 = DYN_EAX;
            }
        } else {
            reg2 = DYN_ECX;
        }
        pushedReg2 = true;
        outb(0x50+reg2);
    }

    // mov reg2, addressReg
    outb(0x89);
    outb(0xc0|reg2|(addressReg<<3));

    // and reg, 0xfff
    outb(0x81);
    outb(0xe0+reg2);
    outd(0xfff);

    // mov [eax+reg2], value
    if (paramType == DYN_PARAM_CONST_8 || paramType == DYN_PARAM_CONST_16 || paramType == DYN_PARAM_CONST_32) {
        if (width==DYN_8bit) {
            outb(0xc6);
            outb(0x04);
            outb(reg1 | (reg2 << 3));
            outb((U8)value);
        } else if (width==DYN_16bit) {
            outb(0x66);
            outb(0xc7);
            outb(0x04);
            outb(reg1 | (reg2 << 3));
            outw((U16)value);
        } else if (width==DYN_32bit) {
            outb(0xc7);
            outb(0x04);
            outb(reg1 | (reg2 << 3));
            outd((U32)value);
        }
    } else if (paramType == DYN_PARAM_REG_8 || paramType == DYN_PARAM_REG_16 || paramType == DYN_PARAM_REG_32) {
        if (width==DYN_8bit) {
            outb(0x88);
            outb(0x04|(value<<3));
            outb(reg1 | (reg2 << 3));
        } else if (width==DYN_16bit) {
            outb(0x66);
            outb(0x89);
            outb(0x04|(value<<3));
            outb(reg1 | (reg2 << 3));
        } else if (width==DYN_32bit) {
            outb(0x89);
            outb(0x04|(value<<3));
            outb(reg1 | (reg2 << 3));
        }
    } else {
        kpanic_fmt("x32CPU::movToMem unknown param type: %d", paramType);
    }
    if (pushedReg2) {
        outb(0x58+reg2);
    }

    // jmp over slow path
    outb(0xeb);
    U32 slowPos = x86.buffer.size();
    outb(0);

    x86.buffer[jzPos] = (U8)(x86.buffer.size() -jzPos-1);
    if (firstCheckPos)
        x86.buffer[firstCheckPos] = (U8)(x86.buffer.size() -firstCheckPos-1);

    if (regUsed[DYN_EAX] && (reg1!=DYN_EAX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EAX))
        outb(0x50);  
    if (regUsed[DYN_ECX] && (reg1!=DYN_ECX || !pushedReg1) && (!doneWithValueReg || value!=DYN_ECX))
        outb(0x51);
    if (regUsed[DYN_EDX] && (reg1!=DYN_EDX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EDX))
        outb(0x52);

    pushValue(value, paramType, doneWithValueReg);

    // push addressReg
    outb(0x50+addressReg);

    void* address;

    // call write
    if (width == DYN_32bit) {
        address = writed;        
    } else if (width == DYN_16bit) {
        address = writew;
    } else if (width == DYN_8bit) {
        address = writeb;
    } else {
        kpanic_fmt("unknown width in x32CPU::movToMem %d", width);
    }    

    outb(0xe8);
    x86.patch.push_back(x86.buffer.size());
    outd((U32)address);

    // add esp, 8
    outb(0x83);
    outb(0xc4);
    outb(0x08);

    if (regUsed[DYN_EDX] && (reg1!=DYN_EDX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EDX))
        outb(0x5a);
    if (regUsed[DYN_ECX] && (reg1!=DYN_ECX || !pushedReg1) && (!doneWithValueReg || value!=DYN_ECX))
        outb(0x59);
    if (regUsed[DYN_EAX] && (reg1!=DYN_EAX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EAX))
        outb(0x58);  

    x86.buffer[slowPos] =  (U8)(x86.buffer.size() -slowPos-1);
    if (pushedReg1) {
        outb(0x58+reg1);
    }
}

void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg) {
    // push reg
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_REG_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_REG_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_REG_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromReg", width);

    movToMem(addressReg, width, reg, paramType, doneWithReg);   
    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg) {    
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_CONST_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_CONST_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_CONST_32;
    else
        kpanic_fmt("unknown width %d in x32CPU::movToMemFromImm", width);

    movToMem(addressReg, width, imm, paramType, false);    
    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
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
        pushValue(arg5, arg5Type, doneWithArg5);
    }
    if (argCount >= 4) {
        pushValue(arg4, arg4Type, doneWithArg4);
    }
    if (argCount >= 3) {
        pushValue(arg3, arg3Type, doneWithArg3);
    }
    if (argCount >= 2) {
        pushValue(arg2, arg2Type, doneWithArg2);
    }
    if (argCount >= 1) {
        pushValue(arg1, arg1Type, doneWithArg1);
    }

    x86.call(address);

    // sub esp, 4*argCount
    if (argCount) {
        x86.add(x86.esp, 4 * argCount);
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

void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg) {
    DynReg reg;
    bool pushed = false;

    if (addressReg==DYN_EAX) {
        kpanic("x32CPU::instMemImm doesn't handle passing the address reg in EAX");
    }
    if (!regUsed[DYN_EAX] && addressReg != DYN_EAX) {
        reg = DYN_EAX;
    } else {
        x86.push(x86.eax);
        reg = DYN_EAX;
        pushed = true;
#ifdef _DEBUG
        klog("TODO: x32CPU::instMem pushed EAX.");
#endif
    }    
    movFromMem(regWidth, addressReg, false);
    instRegImm(inst, DYN_EAX, regWidth, imm);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true);
    if (pushed) {
        x86.pop(x86.eax);
    }
}
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg) {
    DynReg reg;
    bool pushed = false;

    if (addressReg==DYN_EAX) {
        kpanic("x32CPU::instMemReg doesn't handle passing the address reg in EAX");
    }
    if (rm==DYN_EAX) {
        kpanic("x32CPU::instMemReg doesn't handle passing the rm reg in EAX");
    }
    if (!regUsed[DYN_EAX] && addressReg != DYN_EAX && rm != DYN_EAX) {
        reg = DYN_EAX;
    } else {
        x86.push(x86.eax);
        reg = DYN_EAX;
        pushed = true;
#ifdef _DEBUG
        klog("TODO: x32CPU::instMem pushed EAX.");
#endif
    }    
    movFromMem(regWidth, addressReg, false);
    instRegReg(inst, DYN_EAX, rm, regWidth, doneWithRmReg);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true);
    if (pushed) {
        x86.pop(x86.eax);
    }
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

void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg) {
    DynReg reg;
    bool pushed = false;

    if (addressReg==DYN_EAX) {
        kpanic("x32CPU::instMem doesn't handle passing the address reg in EAX");
    }
    if (!regUsed[DYN_EAX] && addressReg != DYN_EAX) {
        reg = DYN_EAX;
    } else {
        x86.push(x86.eax);
        reg = DYN_EAX;
        pushed = true;
#ifdef _DEBUG
        klog("TODO: x32CPU::instMem pushed EAX.");
#endif
    }    
    movFromMem(regWidth, addressReg, false);
    instReg(inst, DYN_EAX, regWidth);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true);   
    if (pushed) {
        x86.pop(x86.eax);
    }
}

void instCPU(char inst, U32 dstOffset, DynWidth regWidth, DynReg tmpReg) {
    if (regUsed[tmpReg]) {
        kpanic("instCPU");
    }
    movToRegFromCpu(tmpReg, dstOffset, regWidth);
    instReg(inst, tmpReg, regWidth);
    movToCpuFromReg(dstOffset, tmpReg, regWidth, true);
}

void jumpIf(DynamicData* data, DynReg reg, DynCondition condition, bool doneWithReg, U32 address, DynWidth regWidth) {
    // test reg, reg
    if (regWidth == DYN_8bit) {
        outb(0x84);
        outb(0xc0 | reg | (reg << 3));
    }
    else {
        if (regWidth == DYN_16bit) {
            outb(0x66);
        }
        outb(0x85);
        outb(0xc0 | reg | (reg << 3));
    }

    outb(0xf);
    if (condition == DYN_EQUALS_ZERO) {
        outb(0x84); // jz, jump over if not true
    }
    else if (condition == DYN_NOT_EQUALS_ZERO) {
        outb(0x85); // jnz, jump over not true
    }
    else {
        kpanic_fmt("x32CPU::startIf unknown condition %d", condition);
    }
    data->jumps.push_back(DynamicJump(address, x86.buffer.size()));
    outd(0); // jump over amount
    if (doneWithReg)
        regUsed[reg] = false;
}

void startIfCmpValue(DynReg reg, U32 value, bool isEqual, DynWidth regWidth, bool doneWithReg) {
    S32 sValue = (S32)value;
    bool isOneByte = sValue >= -128 && sValue <= 127;

    // cmp reg, value
    if (regWidth == DYN_8bit) {
        outb(0x80);
        outb(0xf8 | reg | (reg << 3));
        outb((U8)value);
    } else {
        if (regWidth == DYN_16bit) {
            outb(0x66);
        }
        outb(isOneByte ? 0x83 : 0x81);
        outb(0xf8 | reg);
        if (isOneByte) {
            outb((U8)value);
        } else if (regWidth == DYN_16bit) {
            outw((U16)value);
        } else {
            outd(value);
        }
    }    

    if (isEqual) {
        outb(0x75);
    } else {
        outb(0x74);
    }

    x86.ifJump.push_back(x86.buffer.size());
    outb(0); // jump over amount
    if (doneWithReg)
        regUsed[reg] = false;
}

void startIfCmpPtr(DynReg reg, DYN_PTR_SIZE value, bool isEqual, bool doneWithReg) {
    startIfCmpValue(reg, value, isEqual, DYN_32bit, doneWithReg);
}

void startIf(DynReg reg, DynReg reg2, bool isEqual, DynWidth regWidth, bool doneWithReg, bool doneWithReg2) {

    // cmp reg, reg2
    if (regWidth == DYN_8bit) {
        outb(0x39);
        outb(0xc0 | reg | (reg << 3));
    } else {
        if (regWidth == DYN_16bit) {
            outb(0x66);
        }
        outb(0x39);
        outb(0xc0 | reg | (reg << 3));
    }

    if (isEqual) {
        outb(0x74); // jz, jump over if not true
    } else {
        outb(0x75); // jnz, jump over not true
    }

    x86.ifJump.push_back(x86.buffer.size());
    outb(0); // jump over amount
    if (doneWithReg)
        regUsed[reg] = false;
    if (doneWithReg2)
        regUsed[reg2] = false;
}

void startIf(DynReg reg, DynCondition condition, bool doneWithReg, DynWidth regWidth) {
    // test reg, reg
    if (regWidth == DYN_8bit) {
        outb(0x84);
        outb(0xc0 | reg | (reg << 3));
    } else {
        if (regWidth == DYN_16bit) {
            outb(0x66);
        }
        outb(0x85);
        outb(0xc0 | reg | (reg << 3));
    }
    if (condition==DYN_NOT_EQUALS_ZERO) {
        outb(0x74); // jz, jump over if not true
    } else if (condition==DYN_EQUALS_ZERO) {
        outb(0x75); // jnz, jump over not true
    } else {
        kpanic_fmt("x32CPU::startIf unknown condition %d", condition);
    }

    x86.ifJump.push_back(x86.buffer.size());
    outb(0); // jump over amount
    if (doneWithReg)
        regUsed[reg] = false;
}

void startElse() {    
    outb(0xeb); // previous block should jump over else statement
    U32 pos = x86.buffer.size();
    outb(0); // jump over amount

    // if statement will jump here if it wasn't true
    endIf();

    x86.ifJump.push_back(pos);
}

void endIf() {
    U32 pos = x86.ifJump.back();
    U32 amount = x86.buffer.size() -pos-1;
    if (amount>127) {
        kpanic_fmt("x32CPU::endIf large if/else blocks not supported: %d", amount);
    }
    x86.ifJump.pop_back();
    x86.buffer[pos] = (U8)(amount);
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
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    setConditional(data, condition);

    if (regWidth != DYN_8bit) {
        movToRegFromReg(DYN_CALL_RESULT, regWidth, DYN_CALL_RESULT, DYN_8bit, false);
    }
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
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
    if (!data->isFunction) {
        blockNext1(data, op);
        return;
    }
    blockDone(data, true);
}

void blockDoneCall(DynamicData* data) {
    if (!data->isFunction) {
        blockDone(data, false);
        return;
    }
    blockDone(data, true);
}

void blockDone(DynamicData* data, bool returnEarly) {
    // cpu->nextOp = cpu->nextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, true);
    if (returnEarly || data->isFunction) {
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
    if (data->isFunction) {
        // only direct jumps will get here, like jmp8, jz, loopnz
        outb(0xe9);
        data->jumps.push_back(DynamicJump(data->currentEip + op->len + op->imm, x86.buffer.size()));
        outd(0);
        return;
    }
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
    startIf(DYN_EAX, DYN_EQUALS_ZERO, false);
    // *(op->nextJump) = cpu->getNextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    x86.writeMem(x86.ebx, 0, x86.eax);
    endIf();

    // cpu->nextOp = *(op->nextJump);        
    regUsed[DYN_EBX] = false;
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_EAX, DYN_32bit, false);
    
#ifdef BOXEDWINE_MULTI_THREADED
    x86.readMem(x86.eax, x86.eax, offsetof(DecodedOp, pfnJitCode));
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    x86.jmp(x86.eax);
    endIf();
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

    startIf(DYN_EAX, DYN_EQUALS_ZERO, false);
    // op->next = cpu->getNextOp();
    callHostFunction(common_getNextOp, true, 1, 0, DYN_PARAM_CPU);
    // mov [ebx + offsetof(DecodedOp, next)], eax
    x86.writeMem(x86.ebx, offsetof(DecodedOp, next), x86.eax);
    endIf();
    regUsed[DYN_EBX] = false;

    // cpu->nextOp = op->next
    movToCpuFromReg(offsetof(CPU, nextOp), DYN_CALL_RESULT, DYN_32bit, false);

#ifdef BOXEDWINE_MULTI_THREADED
    x86.readMem(x86.eax, x86.eax, offsetof(DecodedOp, pfnJitCode));
    startIf(DYN_CALL_RESULT, DYN_NOT_EQUALS_ZERO, true);
    x86.jmp(x86.eax);
    endIf();
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

        initX32Ops();
        DecodedOp* nextOp = op;
        x86.reset();
#ifdef BOXEDWINE_MULTI_THREADED
        // could be a call target is an indirect brant like JmpE32, which is common for some dynamic functions.
        // without this check I've seen drowned god have issues
        if (0) {
            if (decodeFunction(cpu, cpu->getEipAddress(), ops)) {
                BString name = cpu->thread->process->getModuleName(cpu->getEipAddress());
                U32 offset = cpu->thread->process->getModuleEip(cpu->getEipAddress());
                if (offset != 0) {
                    data.isFunction = true;
                    functionCount++;
                    if (0) {
                        klog_fmt("function %d %s %x", functionCount, name.c_str(), offset);
                        if (1) {
                            U32 nextAddress = ops.front().address; // don't use cpu->getEipAddress() here, function could jump to before entry

                            for (DecodedFunctionOp& fop : ops) {
                                if (fop.address != nextAddress) {
                                    klog_fmt("SKIPPED %d", fop.address - nextAddress);
                                }
                                klog_fmt("%x %s", fop.address, fop.op->name());
                                if (fop.op->isBranch() && !fop.op->isRet()) {
                                    klog_fmt(" -> %x", fop.address + fop.op->len + fop.op->imm);
                                }
                                nextAddress = fop.address + fop.op->len;
                            }
                        }
                    }
                }
            }
        }
        if (data.isFunction) {
            std::vector<DecodedFunctionOp> todo;
            // if an instruction is the target of a branch instruction, the data.currentLazyFlags might not be correct, so don't use it, force the code to fetch it from cpu->lazyFlags
            // cinebench 11.5 will trigger a bug here without this
            std::unordered_set<U32> branchTargets;

            U32 start = cpu->getEipAddress();
            // put the first op in the function call first, not the lowest address which is how ops is sorted
            while (ops.front().address < start) {
                ops.push_back(ops.front());
                ops.erase(ops.begin());
            }
            for (DecodedFunctionOp& fop : ops) {
                if (fop.op->isDirectBranch()) {
                    branchTargets.insert(fop.address + fop.op->len + fop.op->imm);
                }
            }
            for (DecodedFunctionOp& fop : ops) {
                memset(regUsed, 0, sizeof(regUsed));
                data.currentEip = fop.address;
                emulatedLen += fop.op->len;
                data.eipToBufferPos.set(data.currentEip, x86.buffer.size());
                //callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)fop.op, DYN_PARAM_CONST_PTR, false);
                if (branchTargets.contains(fop.address)) {
                    data.currentLazyFlags = nullptr;
                }
                x32Ops[fop.op->inst](&data, fop.op);
                fop.op->pfn = cpu->thread->process->startJITOp;
                if (x86.ifJump.size()) {
                    kpanic_fmt("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
                }
            }
        } else
#endif
        {
            count++;
            ops.clear();
            while (nextOp) {
                if (nextOp->flags & OP_FLAG_NO_JIT) {
                    count--;
                    return;
                }
#ifdef __TEST
                if (nextOp->pfnJitCode && nextOp->inst != TestEnd) {
#else
                if (nextOp->pfnJitCode) {
#endif
                    // :TODO: maybe relocate code so that we don't need a jump

                    x86.mov(x86.eax, (U32)nextOp->pfnJitCode);
                    x86.jmp(x86.eax);
                    break;
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
                ops.push_back(DecodedFunctionOp(data.currentEip, nextOp));
                x32Ops[nextOp->inst](&data, nextOp);
                data.currentEip += nextOp->len;
                if (x86.ifJump.size()) {
                    kpanic_fmt("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
                }
                if (!data.isFunction && nextOp->isBranch()) {
                    break;
                } else if (data.done) {
#ifndef __TEST
#ifdef _DEBUG
                    //if (nextOp->next)
                    //    callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)nextOp->next, DYN_PARAM_CONST_PTR, false);
#endif
#endif
                    for (DynamicJump& jmp : data.jumps) {
                        if (jmp.eip >= data.currentEip) {
                            data.done = false;
                            break;
                        }
                    }
                    if (data.done) {
                        break;
                    }
                } else {
                    nextOp = nextOp->next;
                }
            }
        }
        x86.pop(x86.edi);
        x86.pop(x86.ebx);

#ifdef _DEBUG
        x86.mov(x86.esp, x86.ebp);
        x86.pop(x86.ebp);
#endif
        x86.ret();

        for (DynamicJump& jmp : data.jumps) {
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
                getMemData(cpu->memory)->opCache.addJITCode_nolock(chunkOp, chunkAddress, chunkLen);
                chunkOp = fop.op;
                chunkLen = 0;
                chunkAddress = fop.address;
            }
            chunkLen += fop.op->len;
        }
        if (chunkLen) {
            getMemData(cpu->memory)->opCache.addJITCode_nolock(chunkOp, chunkAddress, chunkLen);
        }
#if !defined(BOXEDWINE_MULTI_THREADED)
        op->blockOpCount = opCount;
#endif        
        //if ((count % 1000) == 0) {
        //    klog_fmt("%d %d", count, functionCount);
        //}

        
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
    startIf(DYN_EAX, DYN_EQUALS_ZERO, true);
    x86.pop(x86.edi);
    x86.pop(x86.ebx);

#ifdef _DEBUG
    x86.mov(x86.esp, x86.ebp);
    x86.pop(x86.ebp);
#endif
    x86.ret();

    endIf();

    x86.jmp(x86.eax);
}

#endif