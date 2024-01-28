#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC_ARMV7

#include "armv7CPU.h"
#include "../common/lazyFlags.h"
#include "../dynamic/dynamic.h"
#include "knativethread.h"

#define DYN_NEEDS_PUSH_POP_REG_FOR_FUNCTION_PARAMS

/********************************************************/
/* Following is required to be defined for dynamic code */
/********************************************************/

#define INCREMENT_EIP(data, op) incrementEip(data, op)

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

#define NUMBER_OF_REGS 8

// per instruction, not per block.  
static bool regUsed[NUMBER_OF_REGS];
static bool regWasUsed[NUMBER_OF_REGS];

void setRegUsed(U8 reg) {
    regUsed[reg] = true;
    regWasUsed[reg] = true;
}

void clearRegUsed(U8 reg) {
    regUsed[reg] = false;
}

void resetRegsUsed() {
    memset(regUsed, 0, sizeof(regUsed));
    memset(regWasUsed, 0, sizeof(regWasUsed));
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

// don't use regs 0-3 because they are used for parameter passing and won't be preserved when calling a function
#define DYN_SRC DYN_R4
#define DYN_DEST DYN_R5
#define DYN_ADDRESS DYN_R6
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
void callHostFunction(void* address, bool hasReturn=false, U32 argCount=0, U32 arg1=0, DynCallParamType arg1Type=DYN_PARAM_CONST_32, bool doneWithArg1=true, U32 arg2=0, DynCallParamType arg2Type=DYN_PARAM_CONST_32, bool doneWithArg2=true, U32 arg3=0, DynCallParamType arg3Type=DYN_PARAM_CONST_32, bool doneWithArg3=true, U32 arg4=0, DynCallParamType arg4Type=DYN_PARAM_CONST_32, bool doneWithArg4=true, U32 arg5=0, DynCallParamType arg5Type=DYN_PARAM_CONST_32, bool doneWithArg5=true);

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

// r14 is the link register. (The BL instruction, used in a subroutine call, stores the return address in this register.)
// r13 is the stack pointer. (The Push / Pop instructions in "Thumb" operating mode use this register only.)
// r12 is the Intra - Procedure - call scratch register.
// r4 to r11 : used to hold local variables.
// r0 to r3 : used to hold argument values passed to a subroutine, and also hold results returned from a subroutine.

// Subroutines must preserve the contents of r4 to r11 and the stack pointer (perhaps by saving them to the stack in the 
// function prologue, then using them as scratch space, then restoring them from the stack in the function epilogue). In particular, 
// subroutines that call other subroutines must save the return address in the link register r14 to the stack before calling those 
// other subroutines. However, such subroutines do not need to return that value to r14—they merely need to load that value into r15, 
// the program counter, to return. 

#define REG_CPU DYN_R7

#define REG_PC 15
#define REG_LR 14
#define REG_SP 13

// save regs R4-R7 and LR
#define BLOCK_REGS_SAVED 0x41F0

#define MIN_UNSAVED_REG 1
#define MAX_UNSAVED_REG 3

DynReg getUnsavedTmpReg() {
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

#ifdef THUMB_MODE
void movw(U8 reg, U16 value) {
    U8 tmp = (value >> 8) & 0xF);
    outb(0x40 | (value >> 12));
    outb(0xf2 | (tmp > 7 ? 4 : 0));
    outb((U8)value;
    outb(reg | ((value >> 8) & 0x7) << 4);
}

void movt(U8 reg, U16 value) {
    U8 tmp = (value >> 8) & 0xF);
    outb(0xC0 | (value >> 12));
    outb(0xf2 | (tmp > 7 ? 4 : 0));
    outb((U8)value;
    outb(reg | ((value >> 8) & 0x7) << 4);
}

void movs(U8 reg, U8 value) {
    outb(value);
    outb(0x20 | reg);
}

void clearTop16(U8 reg) {
    movt(reg, 0);
}

void loadConst(U8 reg, U32 value) {
    if (value <= 0xFF) {
        movs(reg, value);
    } else {
        movw(reg, op->disp);
        if (op->disp > 0xFFFF) {
            movt(reg, op->disp >> 16);
        }
    }
}

void loadCpuOffset16(U8 reg, U32 offset) {
}

void loadCpuOffset32(U8 reg, U32 offset) {
}

void addRegs(U8 reg, U8 reg2) {
    outb((reg2 << 7) | (reg & 7) | ((reg & 8) ? 0x80 : 0));
    outb(0x44);
}

void addConst(U8 reg, U32 value) {
    if (value < 0x100) {
        // add reg, op->disp
        outb(reg);
        outb(0xf1);
        outb(op->disp);
        outb(reg);
    } else {
        loadConst(DYN_LDR_TMP, op->disp);
        addRegs(reg, DYN_LDR_TMP);
    }
}

void zeroReg(U8 reg) {
    movs(reg, 0);
}

check
void shiftLeft(U8 reg, U8 amount) {
    outb(0x4f);
    outb(0xea);
    outb(reg | 0x30 | ((amount << 6) & 0xc));
    outb(reg | (amount << 2) & 0x70);
}

check
void shiftRight(U8 reg, U8 amount) {
    outb(0x4f);
    outb(0xea);
    outb(reg | 0x30 | ((amount << 6) & 0xc));
    outb(reg | (amount << 2) & 0x70);
}

check
void bswap16(U8 reg) {
    // ror
    outb(0x4f);
    outb(0xea);
    outb(reg | 0x30 | ((amount << 6) & 0xc));
    outb(reg | (amount << 2) & 0x70);
}

void mov(U8 dst, U8 src) {
    outb((src << 7) | (dst & 7) | ((dst & 8) ? 0x80 : 0));
    outb(0x46);
}

void clearBottom16(U8 reg) {
    // shift right 16
    // shift left 16
}

void or(U8 dst, U8 src) {
    outb(0x40);
    outb(0xea);
    outb(src);
    outb(dst);
}

#else
void addRegs32(U8 reg, U8 reg2);

void movw(U8 reg, U16 value) {
    outb((U8)value);
    outb((reg << 4) | ((value >> 8) & 0xF));
    outb((value >> 12) & 0xF);
    outb(0xe3);
}

void movt(U8 reg, U16 value) {
    outb((U8)value);
    outb((reg << 4) | ((value >> 8) & 0xF));
    outb(((value >> 12) & 0xF) | 0x40);
    outb(0xe3);
}

void clearTop16(U8 reg) {
    movt(reg, 0);
}

void loadConst32(U8 reg, U32 value) {
    movw(reg, value);
    if (value > 0xFFFF) {
        movt(reg, value >> 16);
    }
}

void loadConstPtr(U8 reg, DYN_PTR_SIZE value) {
    loadConst32(reg, value);
}

void readMem8(U8 dst, U8 src, U32 offset) {
    if (offset > 0xFFF) {
        kpanic("readMem8: offset is larger than 0xFFF: %x", offset);
    }
    outb(offset & 0xFF);
    outb(((offset >> 8) & 0xF) | (dst << 4));
    outb(0xd0 | src);
    outb(0xe5);
}

void readMem16(U8 dst, U8 src, U32 offset) {
    if (offset > 0xFF) {
        loadConst32(DYN_R8, offset);
        addRegs32(DYN_R8, src);
        src = DYN_R8;
        offset = 0;
    }
    outb(0xb0 | (offset & 0xF));
    outb(((offset >> 4) & 0xF) | (dst << 4));
    outb(0xd0 | src);
    outb(0xe1);
}

void readMem32(U8 dst, U8 src, U32 offset) {
    if (offset > 0xFFF) {
        kpanic("readMem32: offset is larger than 0xFFF: %x", offset);
    }
    outb(offset & 0xFF);
    outb(((offset >> 8) & 0xF) | (dst << 4));
    outb(0x90 | src);
    outb(0xe5);
}

void readMemPtr(U8 dst, U8 base, U64 offset) {
    readMem32(dst, base, offset);
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
    if (offset > 0xFFF) {
        kpanic("saveRegToCpuOffset8: offset is larger than 0xFFF: %x", offset);
    }
    outb(offset & 0xFF);
    outb(((offset >> 8) & 0xF) | (reg << 4));
    outb(0xc0 | REG_CPU);
    outb(0xe5);
}

void saveRegToCpuOffset16(U8 reg, U32 offset) {
    U32 cpuReg = REG_CPU;
    if (offset > 0xFF) {
        loadConst32(DYN_R8, offset);
        addRegs32(DYN_R8, REG_CPU);
        cpuReg = DYN_R8;
        offset = 0;
    }
    outb(0xb0 | (offset & 0xF));
    outb(((offset >> 4) & 0xF) | (reg << 4));
    outb(0xc0 | cpuReg);
    outb(0xe1);
}

void saveRegToCpuOffset32(U8 reg, U32 offset) {
    if (offset > 0xFFF) {
        kpanic("saveRegToCpuOffset32: offset is larger than 0xFFF: %x", offset);
    }
    outb(offset & 0xFF);
    outb(((offset >> 8) & 0xF) | (reg << 4));
    outb(0x80 | REG_CPU);
    outb(0xe5);
}

void saveRegToCpuOffsetPtr(U8 reg, U32 offset) {
    saveRegToCpuOffset32(reg, offset);
}

void saveValueToCpuOffset8(U8 value, U32 offset) {
    U8 tmpReg = getUnsavedTmpReg();
    loadConst32(tmpReg, value);
    saveRegToCpuOffset8(tmpReg, offset);
    clearRegUsed(tmpReg);
}

void saveValueToCpuOffset16(U16 value, U32 offset) {
    U8 tmpReg = getUnsavedTmpReg();
    loadConst32(tmpReg, value);
    saveRegToCpuOffset16(tmpReg, offset);
    clearRegUsed(tmpReg);
}

void saveValueToCpuOffset32(U32 value, U32 offset) {
    U8 tmpReg = getUnsavedTmpReg();
    loadConst32(tmpReg, value);
    saveRegToCpuOffset32(tmpReg, offset);
    clearRegUsed(tmpReg);
}

void saveValueToCpuOffsetPtr(DYN_PTR_SIZE value, U32 offset) {
    saveValueToCpuOffset32(value, offset);
}

void addRegs32(U8 reg, U8 reg2) {
    outb(reg2);
    outb(reg << 4);
    outb(0x80 | reg);
    outb(0xe0);
}

void addValue32(U8 reg, U32 value) {
    if (value <= 255) {
        outb(value);
        outb(reg << 4);
        outb(0x80 | reg);
        outb(0xe2);
    } else {
        U8 tmpReg = getUnsavedTmpReg();
        loadConst32(tmpReg, value);
        addRegs32(reg, tmpReg);
        clearRegUsed(tmpReg);
    }
}

void subRegs32(U8 reg, U8 reg2) {
    outb(reg2);
    outb(reg << 4);
    outb(0x40 | reg);
    outb(0xe0);
}

void subValue32(U8 reg, U32 value) {
    if (value <= 255) {
        outb(value);
        outb(reg << 4);
        outb(0x40 | reg);
        outb(0xe2);
    } else {
        U8 tmpReg = getUnsavedTmpReg();
        loadConst32(tmpReg, value);
        subRegs32(reg, tmpReg);
        clearRegUsed(tmpReg);
    }
}

void orRegs32(U8 reg, U8 reg2) {
    outb(reg2);
    outb(reg << 4);
    outb(0x80 | reg);
    outb(0xe1);
}

void orValue32(U8 reg, U32 value) {
    if (value <= 255) {
        outb(value);
        outb(reg << 4);
        outb(0x80 | reg);
        outb(0xe3);
    } else {
        U8 tmpReg = getUnsavedTmpReg();
        loadConst32(tmpReg, value);
        orRegs32(reg, tmpReg);
        clearRegUsed(tmpReg);
    }
}

void xorRegs32(U8 reg, U8 reg2) {
    outb(reg2);
    outb(reg << 4);
    outb(0x20 | reg);
    outb(0xe0);
}

void xorValue32(U8 reg, U32 value) {
    if (value <= 255) {
        outb(value);
        outb(reg << 4);
        outb(0x20 | reg);
        outb(0xe2);
    } else {
        U8 tmpReg = getUnsavedTmpReg();
        loadConst32(tmpReg, value);
        xorRegs32(reg, tmpReg);
        clearRegUsed(tmpReg);
    }
}

void andRegs32(U8 dst, U8 src1, U8 src2) {
    outb(src1);
    outb(dst << 4);
    outb(src2);
    outb(0xe0);
}

void andRegs32(U8 reg, U8 reg2) {
    andRegs32(reg, reg, reg2);
}

void andValue32(U8 reg, U8 src, U32 value) {
    if (value <= 255) {
        outb(value);
        outb(reg << 4);
        outb(src);
        outb(0xe2);
    } else {
        U8 tmpReg = getUnsavedTmpReg();
        loadConst32(tmpReg, value);
        andRegs32(reg, src, tmpReg);
        clearRegUsed(tmpReg);
    }
}

void andValue32(U8 reg, U32 value) {
    andValue32(reg, reg, value);
}

void negReg32(U8 reg) {
    // rsb reg, reg, 0
    outb(0x0);
    outb(reg << 4);
    outb(0x60 | reg);
    outb(0xe2);
}

void notReg32(U8 reg) {
    // mvn reg, reg
    outb(reg);
    outb(reg << 4);
    outb(0xe0);
    outb(0xe1);
}

void zeroReg32(U8 reg) {
    outb(0x00);
    outb(reg << 4);
    outb(0xa0);
    outb(0xe3);
}

void shiftLeft32(U8 reg, U8 amount) {
    //  LSL reg, src, amount
    outb(reg | ((amount & 1)?0x80:0));
    outb((reg << 4) | ((amount >> 1) & 0xf));
    outb(0xa0);
    outb(0xe1);
}

void shiftLeft32WithReg(U8 reg, U8 amount) {
    //  LSL reg, src, amount
    outb(reg | 0x10);
    outb((reg << 4) | amount);
    outb(0xa0);
    outb(0xe1);
}

void shiftRight32(U8 reg, U8 amount) {
    // LSR reg, src, amount
    outb(reg | ((amount & 1) ? 0x80 : 0) | 0x20);
    outb((reg << 4) | ((amount >> 1) & 0xf));
    outb(0xa0);
    outb(0xe1);
}

void shiftRight32WithReg(U8 reg, U8 amount) {
    // LSR reg, src, amount
    outb(reg | 0x30);
    outb((reg << 4) | amount);
    outb(0xa0);
    outb(0xe1);
}

void shiftRightSigned32(U8 reg, U8 amount) {
    // ASR reg, src, amount
    outb(reg | ((amount & 1) ? 0x80 : 0) | 0x40);
    outb((reg << 4) | ((amount >> 1) & 0xf));
    outb(0xa0);
    outb(0xe1);
}

void shiftRightSigned32WithReg(U8 reg, U8 amount) {
    // ASR reg, src, amount
    outb(reg | 0x50);
    outb((reg << 4) | amount);
    outb(0xa0);
    outb(0xe1);
}

void mov32(U8 dst, U8 src) {
    outb(src);
    outb(dst << 4);
    outb(0xa0);
    outb(0xe1);
}

void movPtr(U8 dst, U8 src) {
    mov32(dst, src);
}

void mov32zx16(U8 dst, U8 src) {
    // UXTH dst, src
    outb(0x70 | src);
    outb(dst << 4);
    outb(0xff);
    outb(0xe6);
}

void mov32zx8(U8 dst, U8 src) {
    // UXTB dst, src
    outb(0x70 | src);
    outb(dst << 4);
    outb(0xef);
    outb(0xe6);
}

void mov32sx16(U8 dst, U8 src) {
    // SXTH dst, src
    outb(0x70 | src);
    outb(dst << 4);
    outb(0xbf);
    outb(0xe6);
}

void mov32sx8(U8 dst, U8 src) {
    // SXTB dst, src
    outb(0x70 | src);
    outb(dst << 4);
    outb(0xaf);
    outb(0xe6);
}

void pushRegs(U16 bitMask) {
    // STMDB SP!, { regs }
    outb(bitMask & 0xFF);
    outb((bitMask >> 8) & 0xFF);
    outb(0x2d);
    outb(0xe9);
}

void popRegs(U16 bitMask) {
    // LDMIA SP!, { regs }
    outb(bitMask & 0xFF);
    outb((bitMask >> 8) & 0xFF);
    outb(0xbd);
    outb(0xe8);
}

void cmpRegs32(U8 r1, U8 r2) {
    outb(r2);
    outb(0);
    outb(0x50 | r1);
    outb(0xe1);
}

void cmpRegValue32(U8 reg, U32 value) {
    if (value <= 255) {
        outb(value);
        outb(0);
        outb(0x50 | reg);
        outb(0xe3);
    } else {
        U8 tmpReg = getUnsavedTmpReg();
        loadConst32(tmpReg, value);
        cmpRegs32(reg, tmpReg);
        clearRegUsed(tmpReg);
    }
}

U32 jumpIfEqual() {
    U32 pos = outBufferPos;
    outb(0);
    outb(0);
    outb(0);
    outb(0x0a); // beq
    return pos;
}

U32 jumpIfNotEqual() {
    U32 pos = outBufferPos;
    outb(0);
    outb(0);
    outb(0);
    outb(0x1a); // bne
    return pos;
}

U32 unconditionalJump() {
    U32 pos = outBufferPos;
    outb(0);
    outb(0);
    outb(0);
    outb(0xea); // b
    return pos;
}

void writeJumpAmount(U32 pos, U32 toLocation) {
    U32 amount = (toLocation - pos - 8) >> 2;
    if (amount > 0xFFF) {
        kpanic("armv7::endIf large if/else blocks not supported: %d", amount);
    }
    outBuffer[pos] = (U8)(amount);
    outBuffer[pos + 1] = (U8)(amount >> 8);
    outBuffer[pos + 2] = (U8)(amount >> 16);
}

void evaluateCondition(U8 reg, DynConditionEvaluate condition) {
    switch (condition) {
    case DYN_EQUALS:
        // moveq reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0x03);
        // movne reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0x13);
        break;
    case DYN_NOT_EQUALS:        
        // movne reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0x13);
        // moveq reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0x03);
        break;
    case DYN_LESS_THAN_UNSIGNED:
        // movlo reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0x33);
        // movhs reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0x23);
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        // movls reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0x93);
        // movhi reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0x83);
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:        
        // movhs reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0x23);
        // movlo reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0x33);
        break;
    case DYN_LESS_THAN_SIGNED:
        // movlt reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0xb3);
        // movge reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0xa3);
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        // movle reg, 1
        outb(0x01);
        outb(reg << 4);
        outb(0xa0);
        outb(0xd3);
        // movgt reg, 0
        outb(0x00);
        outb(reg << 4);
        outb(0xa0);
        outb(0xc3);
        break;
    default:
        kpanic("armv7::evaluateToRegFromRegs unknown condition %d", condition);
    }
}

void rtn() {
    // bx lr
    outb(0x1e);
    outb(0xff);
    outb(0x2f);
    outb(0xe1);
}

void callFunctionReg32(U8 reg) {
    // blx reg
    outb(0x30 | reg);
    outb(0xff);
    outb(0x2f);
    outb(0xe1);
}

void byteSwapReg32(DynReg reg) {
    // rev reg, reg
    outb(0x30 | reg);
    outb(reg << 4 | 0x0f);
    outb(0xbf);
    outb(0xe6);
}

#endif

void startBlock() {
    pushRegs(BLOCK_REGS_SAVED); 
    movToRegFromReg(REG_CPU, DYN_32bit, DYN_R0, DYN_32bit, false);
}

void endBlock() {
    popRegs(BLOCK_REGS_SAVED);
    rtn();
}

void callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5);

#include "../dynamic/dynamic_generic_base.h"

void callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[NUMBER_OF_REGS] = { false, false, false, false, false, false, false, false };

    if (argCount >= 5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5 >= NUMBER_OF_REGS)
                kpanic("armv7::callHostFunction bad param 5: arg=%d argType=%d", arg5, arg5Type);
            regDone[arg5] = true;
        }
    }
    if (argCount >= 4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4 >= NUMBER_OF_REGS)
                kpanic("armv7::callHostFunction bad param 4: arg=%d argType=%d", arg4, arg4Type);
            regDone[arg4] = true;
        }
    }
    if (argCount >= 3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3 >= NUMBER_OF_REGS)
                kpanic("armv7::callHostFunction bad param 3: arg=%d argType=%d", arg3, arg3Type);
            regDone[arg3] = true;
        }
    }
    if (argCount >= 2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2 >= NUMBER_OF_REGS)
                kpanic("armv7::callHostFunction bad param 5: arg=%d argType=%d", arg2, arg2Type);
            regDone[arg2] = true;
        }
    }
    if (argCount >= 1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1 >= NUMBER_OF_REGS)
                kpanic("armv7::callHostFunction bad param 5: arg=%d argType=%d", arg1, arg1Type);
            regDone[arg1] = true;
        }
    }

    if (argCount >= 5) {
        pushValue(arg5, arg5Type);
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
    if (regUsed[DYN_CALL_RESULT] && !hasReturn && !regDone[DYN_CALL_RESULT]) {
        int ii = 0;
    }    
    // reg can't be R0-R3 because that is used for params 
    // dyn_src, dyn_dest, dyn_address need to be preserved
    loadConst32(DYN_R8, (U32)address);
    callFunctionReg32(DYN_R8);
    if (argCount >= 5) {
        addValue32(REG_SP, 4);
    }
    for (int i = 0; i < NUMBER_OF_REGS; i++) {
        if (regDone[i]) {
            clearRegUsed(i);
        }
    }
    if (hasReturn) {
        regUsed[DYN_CALL_RESULT] = true;
    }
}

#endif