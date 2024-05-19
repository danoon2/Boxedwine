#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC_ARMV8

#include "armv8CPU.h"
#include "../common/lazyFlags.h"
#include "../dynamic/dynamic.h"
#include "knativethread.h"
#include "llvm_helper.h"

/********************************************************/
/* Following is required to be defined for dynamic code */
/********************************************************/

// R0 is used for returns from function calls
// R1 - R4 are parameters for function calls
// R5 - R11 is tmp
// R12 is src
// R13 is dst
// R14 is address
// R15 is used to hold CPU

// when calling a function, R1 (cpu) and R30 (LR) will be pushed onto the stack to save them
// if R1-R15 is in use then they will be saved before a function call
#define INCREMENT_EIP(data, op) incrementEip(data, op)

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

// per instruction, not per block.  
#define NUMBER_OF_REGS 32
static bool regUsed[NUMBER_OF_REGS];
static bool regWasUsed[NUMBER_OF_REGS];
static bool regContainsValue[NUMBER_OF_REGS];
static U64 regValue[NUMBER_OF_REGS];

void setRegUsed(U8 reg) {
    regUsed[reg] = true;
    regWasUsed[reg] = true;
    regContainsValue[reg] = false;
}

void clearRegUsed(U8 reg) {
    regUsed[reg] = false;
}

void resetRegsUsed() {
    memset(regUsed, 0, sizeof(regUsed));    
}

// DynReg is a required type, but the values inside are local to this file
enum DynReg {
    DYN_R0 = 0,
    DYN_R1 = 1,
    DYN_R2 = 2,
    DYN_R3 = 3,
    DYN_R4 = 4,
    DYN_R5 = 5,
    DYN_R6 = 6,
    DYN_R7 = 7,  
    DYN_R8 = 8,
    DYN_R9 = 9,
    DYN_R10 = 10,
    DYN_R11 = 11,
    DYN_R12 = 12,
    DYN_R13 = 13,
    DYN_R14 = 14,
    DYN_R15 = 15,
    DYN_R16 = 16,
    DYN_R17 = 17,
    DYN_R18 = 18,
    DYN_R19 = 19,
    DYN_R20 = 20,
    DYN_R21 = 21,
    DYN_R22 = 22,
    DYN_R23 = 23,
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

// does not have be saved across function calls
#define DYN_CALL_RESULT DYN_R0

// 19-29 are preserved
#define DYN_SRC DYN_R12
#define DYN_DEST DYN_R13
#define DYN_ADDRESS DYN_R14
#define DYN_ANY DYN_DEST

#define DYN_PTR_SIZE U64

enum DynWidth {
    DYN_8bit=0,
    DYN_16bit,
    DYN_32bit,
    DYN_64bit
};

enum DynCallParamType {
    DYN_PARAM_REG_8,    
    DYN_PARAM_REG_16,
    DYN_PARAM_REG_32,
    DYN_PARAM_CONST_8,
    DYN_PARAM_CONST_16,
    DYN_PARAM_CONST_32,
    DYN_PARAM_CONST_PTR,
    DYN_PARAM_ABSOLUTE_ADDRESS_8,
    DYN_PARAM_ABSOLUTE_ADDRESS_16,
    DYN_PARAM_ABSOLUTE_ADDRESS_32,
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

#define Dyn_PtrSize DYN_64bit

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

// to CPU
void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg);
void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm);
void movToCpuPtr(U32 dstOffset, DYN_PTR_SIZE imm);

// from CPU
void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width);

// from Mem to DYN_CALL_RESULT
void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg);

// to Mem
void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg);
void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg);

// arith
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg);
void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg);

void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg);
void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm);

void instReg(char inst, DynReg reg, DynWidth regWidth);
void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg);
void instCPU(char inst, U32 dstOffset, DynWidth regWidth);

// if conditions
void startIf(DynReg reg, DynCondition condition, bool doneWithReg);
void startElse();
void endIf();
void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition);
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg);

// call into emulator, like setFlags, getCF, etc
void callHostFunction(void* address, bool hasReturn=false, U32 argCount=0, DYN_PTR_SIZE arg1=0, DynCallParamType arg1Type=DYN_PARAM_CONST_32, bool doneWithArg1=true, DYN_PTR_SIZE arg2=0, DynCallParamType arg2Type=DYN_PARAM_CONST_32, bool doneWithArg2=true, DYN_PTR_SIZE arg3=0, DynCallParamType arg3Type=DYN_PARAM_CONST_32, bool doneWithArg3=true, DYN_PTR_SIZE arg4=0, DynCallParamType arg4Type=DYN_PARAM_CONST_32, bool doneWithArg4=true, DYN_PTR_SIZE arg5=0, DynCallParamType arg5Type=DYN_PARAM_CONST_32, bool doneWithArg5=true);

// set up the cpu to the correct next block

// this is called for cases where we don't know ahead of time where the next block will be, so we need to look it up
void blockDone();
// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1();
void blockNext2();

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

static U8* outBuffer;
static U32 outBufferSize;
static U32 outBufferPos;

static std::vector<U32> ifJump;

//x30 is the link register (used to return from subroutines)
//x29 is the frame register
//x19 to x29 are callee - saved
//x18 is the 'platform register', used for some operating - system - specific special purpose, or an additional caller - saved register
//x16 and x17 are the Intra - Procedure - call scratch register
//x9 to x15 : used to hold local variables(caller saved)
//x8 : used to hold indirect return value address
//x0 to x7 : used to hold argument values passed to a subroutine, and also hold results returned from a subroutine

#define REG_CPU DYN_R15
#define REG_LR 30

#define MIN_UNSAVED_REG 5
#define MAX_UNSAVED_REG 11

DynReg getUnsavedTmpReg() {
    for (int i = MIN_UNSAVED_REG; i <= MAX_UNSAVED_REG; i++) {
        if (!regUsed[i] && !regContainsValue[i]) {
            setRegUsed(i);
            return (DynReg)i;
        }
    }
    for (int i = MIN_UNSAVED_REG; i <= MAX_UNSAVED_REG; i++) {
        if (!regUsed[i]) {
            setRegUsed(i);
            return (DynReg)i;
        }
    }
    kpanic("Could not find unused tmp reg");
    return (DynReg)0;
}

void ensureBufferSize(U32 grow) {
    if (!outBuffer) {
        outBuffer = new U8[256];
        outBufferSize = 256;
    }
    if (outBufferSize-outBufferPos<grow) {
        U8* t =  new U8[outBufferSize*2];
        memcpy(t, outBuffer, outBufferSize);
        delete[] outBuffer;
        outBuffer = t;
        outBufferSize = outBufferSize*2;
    }
}

void outb(U8 b) {
    ensureBufferSize(1);
    outBuffer[outBufferPos++]=b;
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

void addRegs32(U8 reg, U8 reg2);
void addRegs64(U8 reg, U8 reg2);

// shift can be 0, 16, 32 or 48
void movk(U8 reg, U16 value, U8 shift) {
    U8 shiftBit = 0;
    if (shift == 16) {
        shiftBit = 0x20;
    } else if (shift == 32) {
        shiftBit = 0x40;
    } else if (shift == 48) {
        shiftBit = 0x60;
    } else {
        kpanic("armv8: bad shift value of % in movk", shift);
    }
    outb((U8)(value << 5) | reg); // bottom 3 bits of the value in the top 3 bits
    outb((U8)(value >> 3)); // bits 4-11
    outb(0x80 | (U8)(value >> 11) | shiftBit); // bits 12-16
    outb(0xf2); // 64-bit
}

// shift can be 0, 16, 32 or 48
void movz(U8 reg, U16 value, U8 shift) {
    U8 shiftBit = 0;
    if (shift == 16) {
        shiftBit = 0x20;
    } else if (shift == 32) {
        shiftBit = 0x40;
    } else if (shift == 48) {
        shiftBit = 0x60;
    } else if (shift!=0) {
        kpanic("armv8: bad shift value of % in movk", shift);
    }
    outb((U8)(value << 5) | reg); // bottom 3 bits of the value in the top 3 bits
    outb((U8)(value >> 3)); // bits 4-11
    outb(0x80 | (U8)(value >> 11) | shiftBit); // bits 12-16
    outb(0xd2); // 64-bit
}

void mov(U8 reg, U16 value) {
    outb((U8)(value << 5) | reg); // bottom 3 bits of the value in the top 3 bits
    outb((U8)(value >> 3)); // bits 4-11
    outb(0x80 | (U8)(value >> 11)); // bits 12-16
    outb(0xd2); // 64-bit
}

void clearTop16(U8 reg) {
    movk(reg, 0, 16);
}

void loadConstPtr(U8 reg, U64 value) {
    U32 shift = __builtin_ctzll(value);
    if (shift < 16) {
        shift = 0;
    } else if (shift < 32) {
        shift = 16;
    } else if (shift < 48) {
        shift = 32;
    } else {
        shift = 48;
    }
    U64 v = value >> shift;
    if (v <= 0xFFFF) {
        movz(reg, v, shift);
    } else {
        mov(reg, (U16)value);
        if (value > 0xFFFF) {
            U16 v = (U16)(value >> 16);
            if (v) {
                movk(reg, v, 16);
            }
        }
        if (value > 0xFFFFFFFF) {
            U16 v = (U16)(value >> 32);
            if (v) {
                movk(reg, v, 32);
            }
        }
        if (value > 0xFFFFFFFFFFFF) {
            movk(reg, (U16)(value >> 48), 48);
        }
    }
    if (ifJump.size() == 0) {
        // don't cache value that are conditionally set
        regContainsValue[reg] = true;
        regValue[reg] = value;
    } else {
        regContainsValue[reg] = false;
    }
}

void loadConst32(U8 reg, U32 value) {
    loadConstPtr(reg, value);
}

U8 getRegWithConstPtr(U64 value) {
    // if there is a register that already contains the value, then just use it without reloading the value
    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (regContainsValue[i] && regValue[i] == value && !regUsed[i]) {
            regUsed[i] = true;
            return i;
        }
    }
    U8 tmp = 0xFF;
    static U8 lastConst;

    // This is meant to be a cheap least recently used register
    //
    // If the dynamic recompiler ever gets to a 2 pass solution, then this could be greatly optimized
    //
    // Save the bottom 3 tmp regs for SRC, DST and ADDRESS so that there is less thrashing
    for (int i = MIN_UNSAVED_REG+3; i <= MAX_UNSAVED_REG; i++) {
        if (!regUsed[i] && !regContainsValue[i]) {
            tmp = i;
            regUsed[i] = true;
            regWasUsed[i] = true;
            break;
        }
    }
    if (tmp == 0xFF) {
        if (lastConst) {
            for (int i = lastConst+1; i <= MAX_UNSAVED_REG; i++) {
                if (!regUsed[i]) {
                    tmp = i;
                    regUsed[i] = true;
                    regWasUsed[i] = true;
                    break;
                }
            }
        }
        if (tmp == 0xFF) {
            for (int i = MIN_UNSAVED_REG + 3; i <= MAX_UNSAVED_REG; i++) {
                if (!regUsed[i]) {
                    tmp = i;
                    regUsed[i] = true;
                    regWasUsed[i] = true;
                    break;
                }
            }
        }
    }
    if (tmp == 0xFF) {
        tmp = getUnsavedTmpReg();
    } else {
        lastConst = tmp;
    }
    loadConstPtr(tmp, value);
    return tmp;
}

U8 getRegWithConst(U32 value) {
    return getRegWithConstPtr(value);
}

void readMem8(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getUnsavedTmpReg();
        loadConstPtr(tmp, offset);
        
        // LDRB
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x60 | tmp);
        outb(0x38);
        clearRegUsed(tmp);
    } else {
        // LDRUB
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb(0x40 | ((offset >> 4) & 0xF));
        outb(0x38);
    }
}

void readMem16(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // LDRH
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x60 | tmp);
        outb(0x78);
        clearRegUsed(tmp);
    } else {
        // LDRUH
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb(0x40 | ((offset >> 4) & 0xF));
        outb(0x78);
    }
}

void readMem32(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // LDR
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x60 | tmp);
        outb(0xb8);
        clearRegUsed(tmp);
    } else {
        // LDUR
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb(0x40 | ((offset >> 4) & 0xF));
        outb(0xb8);
    }
}

void readMem64(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // LDR
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x60 | tmp);
        outb(0xf8);
        clearRegUsed(tmp);
    } else {
        // LDUR
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb(0x40 | ((offset >> 4) & 0xF));
        outb(0xf8);
    }
}

void readMemPtr(U8 dst, U8 base, U64 offset) {
    readMem64(dst, base, offset);
}

void writeMem8(U8 dst, U8 base, U64 offset) { 
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // STRB
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x20 | tmp);
        outb(0x38);
        clearRegUsed(tmp);
    } else {
        // STURB
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb((offset >> 4) & 0xF);
        outb(0x38);
    }
}

void writeMem16(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // STRH
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x20 | tmp);
        outb(0x78);
        clearRegUsed(tmp);
    } else {
        // STURH
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb((offset >> 4) & 0xF);
        outb(0x78);
    }
}

void writeMem32(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // STR
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x20 | tmp);
        outb(0xb8);
        clearRegUsed(tmp);
    } else {
        // STUR
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb((offset >> 4) & 0xF);
        outb(0xb8);
    }
}

void writeMem64(U8 dst, U8 base, U64 offset) {    
    if (offset > 0xFF) {
        U8 tmp = getRegWithConstPtr(offset);

        // STR
        outb(dst | (U8)(base << 5));
        outb(0x68 | (U8)(base >> 3));
        outb(0x20 | tmp);
        outb(0xf8);
        clearRegUsed(tmp);
    } else {
        // STUR
        outb(dst | (U8)(base << 5));
        outb(((offset & 0xF) << 4) | (U8)(base >> 3));
        outb((offset >> 4) & 0xF);
        outb(0xf8);
    }
}

void loadFromCpuOffset8(U8 reg, U32 offset) {
    readMem8(reg, REG_CPU, offset);
}

void loadFromCpuOffset16(U8 reg, U32 offset) {
    readMem16(reg, REG_CPU, offset);
}

void loadFromCpuOffset32(U8 reg, U32 offset) {
    readMem32(reg, REG_CPU, offset);
}

void saveRegToCpuOffset8(U8 reg, U32 offset) {
    writeMem8(reg, REG_CPU, offset);
}

void saveRegToCpuOffset16(U8 reg, U32 offset) {
    writeMem16(reg, REG_CPU, offset);
}

void saveRegToCpuOffset32(U8 reg, U32 offset) {
    writeMem32(reg, REG_CPU, offset);
}

void saveRegToCpuOffset64(U8 reg, U32 offset) {
    writeMem64(reg, REG_CPU, offset);
}

void saveRegToCpuOffsetPtr(U8 reg, U32 offset) {
    writeMem64(reg, REG_CPU, offset);
}

void saveValueToCpuOffset8(U8 value, U32 offset) {
    U8 reg = getRegWithConst(value);
    saveRegToCpuOffset8(reg, offset);
    clearRegUsed(reg);
}

void saveValueToCpuOffset16(U16 value, U32 offset) {
    U8 reg = getRegWithConst(value);
    saveRegToCpuOffset16(reg, offset);
    clearRegUsed(reg);
}

void saveValueToCpuOffset32(U32 value, U32 offset) {
    U8 reg = getRegWithConst(value);
    saveRegToCpuOffset32(reg, offset);
    clearRegUsed(reg);
}

void saveValueToCpuOffset64(U64 value, U32 offset) {
    U8 reg = getRegWithConstPtr(value);
    saveRegToCpuOffset64(reg, offset);
    clearRegUsed(reg);
}

void saveValueToCpuOffsetPtr(DYN_PTR_SIZE value, U32 offset) {
    saveValueToCpuOffset64(value, offset);
}

void addRegs32(U8 reg, U8 reg2) {
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3));
    outb(reg2);
    outb(0x0b); // 0b is 32-bit version (8b is 64-bit version)
}

void addRegs64(U8 reg, U8 reg2) {
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3));
    outb(reg2);
    outb(0x8b);
}

void addRegs64(U8 dst, U8 src1, U8 src2) {
    outb(dst | (U8)(src1 << 5));
    outb((U8)(src1 >> 3));
    outb(src2);
    outb(0x8b);
}

void addValue32(U8 reg, U32 value) {
    if (value <= 0xFFF) {
        outb(reg | (U8)(reg << 5));
        outb((U8)(reg >> 3) | (U8)(value << 2));
        outb(value >> 6);
        outb(0x11); // 11 is 32-bit version (91 is 64-bit version)
    } else {
        U8 tmp = getRegWithConst(value);
        addRegs32(reg, tmp);
        clearRegUsed(tmp);
    }
}

void subRegs32(U8 reg, U8 reg2) {
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3));
    outb(reg2);
    outb(0x4b); // 4b is 32-bit version (cb is 64-bit)
}

void subValue32(U8 reg, U32 value) {
    if (value <= 0xFFF) {
        outb(reg | (U8)(reg << 5));
        outb((U8)(reg >> 3) | (U8)(value << 2));
        outb(value >> 6);
        outb(0x51); // 51 is 32-bit version (d1 is 64-bit)
    } else {
        U8 tmp = getRegWithConst(value);
        subRegs32(reg, tmp);
        clearRegUsed(tmp);
    }
}

void orRegs32(U8 reg, U8 reg2) {
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3));
    outb(reg2);
    outb(0x2a); // 2a is 32-bit version (aa is 64-bit)
}

void orValue32(U8 reg, U32 value) {    
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        outb(reg | (U8)(reg << 5));
        outb((U8)(reg >> 3) | (U8)(imms << 2));
        outb((U8)(immr));
        outb(0x32); // 32 is 32-bit version (b2 is 64-bit)
    } else {
        U8 tmp = getRegWithConst(value);
        orRegs32(reg, tmp);
        clearRegUsed(tmp);
    }
}

void xorRegs32(U8 reg, U8 reg2) {
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3));
    outb(reg2);
    outb(0x4a); // 4a is 32-bit version (ca is 64-bit)
}

void xorValue32(U8 reg, U32 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        outb(reg | (U8)(reg << 5));
        outb((U8)(reg >> 3) | (U8)(imms << 2));
        outb((U8)(immr));
        outb(0x52); // 52 is 32-bit version (d2 is 64-bit)
    } else {
        U8 tmp = getRegWithConst(value);
        xorRegs32(reg, tmp);
        clearRegUsed(tmp);
    }
}

void andRegs32(U8 dst, U8 src1, U8 src2) {
    outb(dst | (U8)(src1 << 5));
    outb((U8)(src1 >> 3));
    outb(src2);
    outb(0x0a); // 0a is 32-bit version (8a is 64-bit)
}

void andRegs32(U8 reg, U8 reg2) {
    andRegs32(reg, reg, reg2);
}

void andValue32(U8 dst, U8 src, U32 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        outb(dst | (U8)(src << 5));
        outb((U8)(src >> 3) | (U8)(imms << 2));
        outb((U8)(immr));
        outb(0x12); // 12 is 32-bit version (92 is 64-bit)
    } else {
        U8 tmp = getRegWithConst(value);
        andRegs32(dst, src, tmp);
        clearRegUsed(tmp);
    }
}

void andValue32(U8 reg, U32 value) {
    andValue32(reg, reg, value);
}

void negReg32(U8 reg) {
    // neg reg, reg
    outb(reg | 0xE0);
    outb(0x03);
    outb(reg);
    outb(0x4b);
}

void notReg32(U8 reg) {
    // mvn reg, reg
    outb(reg | 0xE0);
    outb(0x03);
    outb(reg | 0x20);
    outb(0x2a);
}

void zeroReg32(U8 reg) {
    mov(reg, 0);
}

void ubfm32(U8 dst, U8 src, U8 immr, U8 imms) {
    outb(dst | (U8)(src << 5));
    outb((U8)(src >> 3) | (U8)(imms << 2));
    outb(immr);
    outb(0x53);
}

void sbfm32(U8 dst, U8 src, U8 immr, U8 imms) {
    outb(dst | (U8)(src << 5));
    outb((U8)(src >> 3) | (U8)(imms << 2));
    outb(immr);
    outb(0x13);
}

void shiftLeft32(U8 reg, U8 amount) {
    //  LSL reg, src, amount
    U8 a = (U8)(-(S8)amount);
    ubfm32(reg, reg, a % 32, (31 - amount));
}

void shiftLeft32WithReg(U8 reg, U8 amount) {
    //  LSL reg, src, amount
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3) | 0x20);
    outb(amount | 0xC0);
    outb(0x1a);
}

void shiftRight32(U8 reg, U8 amount) {
    ubfm32(reg, reg, amount, 31);
}

void shiftRight32WithReg(U8 reg, U8 amount) {
    // LSR reg, src, amount
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3) | 0x24  );
    outb(amount | 0xC0);
    outb(0x1a);
}

void shiftRightSigned32(U8 reg, U8 amount) {
    // ASR reg, src, amount
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3) | 0x7C);
    outb(amount);
    outb(0x13);
}

void shiftRightSigned32WithReg(U8 reg, U8 amount) {
    // ASR reg, src, amount
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3) | 0x28);
    outb(amount | 0xC0);
    outb(0x1a);
}

void mov32(U8 dst, U8 src) {
    outb(dst | 0xE0);
    outb(0x3);
    outb(src);
    outb(0x2a);
}

void mov64(U8 dst, U8 src) {
    outb(dst | 0xE0);
    outb(0x3);
    outb(src);
    outb(0xaa);
}

void movPtr(U8 dst, U8 src) {
    mov64(dst, src);
}

void mov32zx16(U8 dst, U8 src) {
    // UXTH dst, src
    outb(dst | (U8)(src << 5));
    outb((U8)(src >> 3) | 0x3c);
    outb(0);
    outb(0x53);
}

void mov32zx8(U8 dst, U8 src) {
    // UXTB dst, src
    outb(dst | (U8)(src << 5));
    outb((U8)(src >> 3) | 0x1c);
    outb(0);
    outb(0x53);
}

void mov32sx16(U8 dst, U8 src) {
    // SXTH dst, src
    outb(dst | (U8)(src << 5));
    outb((U8)(src >> 3) | 0x3c);
    outb(0);
    outb(0x13);
}

void mov32sx8(U8 dst, U8 src) {
    // SXTB dst, src
    outb(dst | (U8)(src << 5));
    outb((U8)(src >> 3) | 0x1c);
    outb(0);
    outb(0x13);
}

void cmpRegs32(U8 r1, U8 r2) {
    outb(0x1f | (U8)(r1 << 5));
    outb((U8)(r1 >> 3));
    outb(r2);
    outb(0x6b); // 6b is 32-bit version (eb is 64-bit)
}

void cmpRegValue32(U8 reg, U32 value) {
    if (value <= 0xFFF) {
        outb(0x1f | (U8)(reg << 5));
        outb((U8)(reg >> 3) | (U8)(value << 2));
        outb(value >> 6);
        outb(0x71); // 51 is 32-bit version (f1 is 64-bit)
    } else {
        U8 tmp = getRegWithConst(value);
        subRegs32(reg, tmp);
        clearRegUsed(tmp);
    }
}

U32 jumpIfEqual() {
    U32 pos = outBufferPos;
    // beq
    outb(0);
    outb(0);
    outb(0);
    outb(0x54); 
    return pos;
}

U32 jumpIfNotEqual() {
    U32 pos = outBufferPos;
    // bne
    outb(0x01);
    outb(0);
    outb(0);
    outb(0x54); 
    return pos;
}

U32 unconditionalJump() {
    U32 pos = outBufferPos;
    // b
    outb(0);
    outb(0);
    outb(0);
    outb(0x14); 
    return pos;
}

void writeJumpAmount(U32 pos, U32 toLocation) {
    U32 amount = (toLocation - pos) >> 2;
    if (outBuffer[pos + 3] == 0x14) {
        if (amount > 0xFFFFFF) {
            kpanic("armv8::jump in large if/else blocks not supported: %d", amount);
        }
        outBuffer[pos] = (U8)amount;
        outBuffer[pos+1] = (U8)(amount >> 8);
        outBuffer[pos+2] = (U8)(amount >> 16);
    } else {
        if (amount > 0x3FFFF) {
            kpanic("armv8::endIf large if/else blocks not supported: %d", amount);
        }
        outBuffer[pos] = (U8)(amount << 5) | outBuffer[pos];
        outBuffer[pos + 1] = (U8)(amount >> 3);
        outBuffer[pos + 2] = (U8)(amount >> 11);
    }
}

void evaluateCondition(U8 reg, DynConditionEvaluate condition) {
    switch (condition) {
    case DYN_EQUALS:
        // cset reg, eq
        outb(0xe0 | reg);
        outb(0x17);
        outb(0x9f);
        outb(0x1a);
        break;
    case DYN_NOT_EQUALS:        
        // cset reg, ne
        outb(0xe0 | reg);
        outb(0x07);
        outb(0x9f);
        outb(0x1a);
        break;
    case DYN_LESS_THAN_UNSIGNED:
        // cset reg, lo
        outb(0xe0 | reg);
        outb(0x27);
        outb(0x9f);
        outb(0x1a);
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        // cset reg, ls
        outb(0xe0 | reg);
        outb(0x87);
        outb(0x9f);
        outb(0x1a);
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:  
        // cset reg, hs
        outb(0xe0 | reg);
        outb(0x37);
        outb(0x9f);
        outb(0x1a);
        break;
    case DYN_LESS_THAN_SIGNED:
        // cset reg, lt
        outb(0xe0 | reg);
        outb(0xa7);
        outb(0x9f);
        outb(0x1a);
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        // cset reg, lt
        outb(0xe0 | reg);
        outb(0xc7);
        outb(0x9f);
        outb(0x1a);
        break;
    default:
        kpanic("armv8::evaluateToRegFromRegs unknown condition %d", condition);
    }
}

void rtn() {
    // ret
    outb(0xc0);
    outb(0x03);
    outb(0x5f);
    outb(0xd6);
}

void callFunctionReg32(U8 reg) {
    // blr reg
    outb((U8)(reg << 5));
    outb((U8)(reg >> 3));
    outb(0x3f);
    outb(0xd6);
}

void byteSwapReg32(DynReg reg) {
    // rev reg, reg
    outb(reg | (U8)(reg << 5));
    outb((U8)(reg >> 3) | 0x08);
    outb(0xc0);
    outb(0x5a);
}

void pushPair(U8 r1, U8 r2) {
    // stp r1, r2, [sp, #-16]!
    outb(0xe0 | r1);
    outb(0x03 | (U8)(r2 << 2));
    outb(0xbf);
    outb(0xa9);
}

void popPair(U8 r1, U8 r2) {
    // ldp r1, r2, [sp], #16
    outb(0xe0 | r1);
    outb(0x03 | (U8)(r2 << 2));
    outb(0xc1);
    outb(0xa8);
}

static bool calledFunction = false;

void startBlock() {
    // :TODO: if this block doesn't call any function, maybe we should use scratch registers instead of callee saved registers
    //pushPair(DYN_R19, DYN_R20);
    //pushPair(DYN_R21, DYN_R22);
    //pushPair(DYN_R23, REG_LR);
    movPtr(REG_CPU, DYN_R0);
    calledFunction = false;
    memset(regWasUsed, 0, sizeof(regWasUsed));
    memset(regContainsValue, 0, sizeof(regContainsValue));
}

void endBlock() {
    //popPair(DYN_R23, REG_LR);
    //popPair(DYN_R21, DYN_R22);
    //popPair(DYN_R19, DYN_R20);
    rtn();
}

void getRegsThatNeedToBeSaved(bool* regsThatNeedToBeSaved) {
    for (int i = 1; i <= 15; i++) {
        if (regUsed[i]) {
            regsThatNeedToBeSaved[i] = true;
        }
    }
    regsThatNeedToBeSaved[REG_LR] = true;
    regsThatNeedToBeSaved[REG_CPU] = true;
}

void pushRegsThatNeedToBeSaved(bool* regsThatNeedToBeSaved) {
    U8 firstReg = 0xFF;
    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (regsThatNeedToBeSaved[i]) {
            if (firstReg == 0xFF) {
                firstReg = i;
            } else {
                pushPair(firstReg, i);
                firstReg = 0xFF;
            }
        }
    }
    if (firstReg != 0xFF) {
        pushPair(firstReg-1, firstReg); // assumes firstReg is 30/LR
    }
}

void popRegsThatNeedToBeSaved(bool* regsThatNeedToBeSaved) {
    U8 firstReg = 0xFF;
    U32 todo[16];
    U32 count = 0;

    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (regsThatNeedToBeSaved[i]) {
            if (firstReg == 0xFF) {
                firstReg = i;
            } else {
                todo[count++] = firstReg | (i << 16);
                firstReg = 0xFF;
            }
        }
    }
    if (firstReg != 0xFF) {
        todo[count++] = (firstReg-1) | (firstReg << 16); // assumes firstReg is 30/LR        
    }
    for (int i = count - 1; i >= 0; i--) {
        popPair(todo[i] & 0xFFFF, todo[i] >> 16);
    }
}

#include "../dynamic/dynamic_generic_base.h"

void callHostFunction(void* address, bool hasReturn, U32 argCount, DYN_PTR_SIZE arg1, DynCallParamType arg1Type, bool doneWithArg1, DYN_PTR_SIZE arg2, DynCallParamType arg2Type, bool doneWithArg2, DYN_PTR_SIZE arg3, DynCallParamType arg3Type, bool doneWithArg3, DYN_PTR_SIZE arg4, DynCallParamType arg4Type, bool doneWithArg4, DYN_PTR_SIZE arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[NUMBER_OF_REGS] = { 0 };
    calledFunction = true;
    
    if (argCount >= 5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5 >= NUMBER_OF_REGS)
                kpanic("armv8::callHostFunction bad param 5: arg=%d argType=%d", arg5, arg5Type);
            regDone[arg5] = true;
        }
    }
    if (argCount >= 4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4 >= NUMBER_OF_REGS)
                kpanic("armv8::callHostFunction bad param 4: arg=%d argType=%d", arg4, arg4Type);
            regDone[arg4] = true;
        }
    }
    if (argCount >= 3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3 >= NUMBER_OF_REGS)
                kpanic("armv8::callHostFunction bad param 3: arg=%d argType=%d", arg3, arg3Type);
            regDone[arg3] = true;
        }
    }
    if (argCount >= 2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2 >= NUMBER_OF_REGS)
                kpanic("armv8::callHostFunction bad param 2: arg=%d argType=%d", arg2, arg2Type);
            regDone[arg2] = true;
        }
    }
    if (argCount >= 1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1 >= NUMBER_OF_REGS)
                kpanic("armv8::callHostFunction bad param 1: arg=%d argType=%d", arg1, arg1Type);
            regDone[arg1] = true;
        }
    }

    bool regsThatNeedToBeSaved[NUMBER_OF_REGS];
    memset(regsThatNeedToBeSaved, 0, sizeof(regsThatNeedToBeSaved));
    getRegsThatNeedToBeSaved(regsThatNeedToBeSaved);
    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (regDone[i]) {
            regsThatNeedToBeSaved[i]=false;
        }
    }
    pushRegsThatNeedToBeSaved(regsThatNeedToBeSaved);

    if (argCount >= 5) {
        setValue(arg5, arg4Type, 4);
    }
    if (argCount >= 4) {
        setValue(arg4, arg4Type, 3);
    }
    if (argCount >= 3) {
        setValue(arg3, arg3Type, 2);
    }    
    if (argCount >= 2) {
        setValue(arg2, arg2Type, 1);
    }
    if (argCount >= 1) {
        setValue(arg1, arg1Type, 0);
    }        
    for (int i = MIN_UNSAVED_REG; i <= MAX_UNSAVED_REG; i++) {
        if (regUsed[i] && !regDone[i]) {
            kpanic("Unsaved reg in use while calling function");
        }
    }
    if (regUsed[DYN_CALL_RESULT] && !hasReturn && !regDone[DYN_CALL_RESULT]) {
        kpanic("Unsaved reg 0 in use while calling function");
    }    
    U8 tmp = getRegWithConstPtr((DYN_PTR_SIZE)address);

    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (regDone[i]) {
            clearRegUsed(i);
        }
    }    

    callFunctionReg32(tmp);
    clearRegUsed(tmp);

    popRegsThatNeedToBeSaved(regsThatNeedToBeSaved);    
    memset(regContainsValue, 0, sizeof(regContainsValue));
    if (hasReturn) {
        regUsed[DYN_CALL_RESULT] = true;
    }
}

#endif