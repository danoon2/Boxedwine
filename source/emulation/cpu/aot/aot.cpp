#include "boxedwine.h"
#ifdef BOXEDWINE_GENERATE_SOURCE

// up to 2 will be needed at a time
static char tmpValue[2][64];
static int tmpValueIndex;
static const char* getOffsetValue(const char* pre, U32 value, const char* post) {
    std::string result = pre + std::to_string(value) + post;
    tmpValueIndex++;
    if (tmpValueIndex > 1) {
        tmpValueIndex = 0;
    }
    strcpy(tmpValue[tmpValueIndex], result.c_str());    
    return tmpValue[tmpValueIndex];
}

const char* r8(int r);
const char* r16(int r);
const char* r32(int r);

#define OFFSET_REG8(x) r8(x)
#define CPU_OFFSET_OF(x) "cpu->"#x
#define CPU_OFFSET_OF_REG8(x) r8(x)
#define CPU_OFFSET_OF_REG16(x) r16(x)
#define CPU_OFFSET_OF_REG32(x) r32(x)
#define CPU_OFFSET_OF_SEG_VALUE(x) getOffsetValue("cpu->seg[", x, "].value")
#define CPU_OFFSET_OF_SEG_ADDRESS(x) getOffsetValue("cpu->seg[", x, "].address")
#define CPU_OFFSET_TYPE const char*
#define DYN_HOST_FN(x) #x
#define DYN_LAZY_FLAG(x) x->name
#include "aot.h"
#include "../common/lazyFlags.h"
#include "../dynamic/dynamic.h"
#include "crc.h"

// cdecl calling convention states EAX, ECX, and EDX are caller saved

/********************************************************/
/* Following is required to be defined for dynamic code */
/********************************************************/

#define INCREMENT_EIP(x, y) incrementEip(x, y)

// DynReg is a required type, but the values inside are local to this file
// Used only these 4 because it is possible to use 8-bit calls with them, like add al, cl
enum DynReg {
    DYN_TMP0 = 0,
    DYN_TMP1 = 1,
    DYN_TMP2 = 2,
    DYN_TMP3 = 3,
    DYN_NOT_SET = 0xff
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

#define DYN_CALL_RESULT DYN_TMP0
#define DYN_SRC DYN_TMP1
#define DYN_DEST DYN_TMP2
#define DYN_ADDRESS DYN_TMP3
#define DYN_ANY DYN_DEST

#define DYN_PTR_SIZE U32

enum DynWidth {
    DYN_8bit = 0,
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
    DYN_PARAM_OP,
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
void movToCpuFromMem(std::string dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
void movToCpuFromCpu(std::string dstOffset, std::string srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
void calculateEaa(DecodedOp* op, DynReg reg);

void byteSwapReg32(DynReg reg);

// REG to REG
void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);
void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);

// to Reg
void movToReg(DynReg reg, DynWidth width, U32 imm);

// to CPU
void movToCpuFromReg(std::string dstOffset, DynReg reg, DynWidth width, bool doneWithReg);
void movToCpu(std::string dstOffset, DynWidth dstWidth, U32 imm);
void movToCpuLazyFlags(std::string dstOffset, std::string lazyFlags);

// from CPU
void movToRegFromCpu(DynReg reg, std::string srcOffset, DynWidth width);

// from Mem to DYN_READ_RESULT
void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg);

// to Mem
void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg);
void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg);

// arith
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg);
void instCPUReg(char inst, std::string dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg);

void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg);
void instCPUImm(char inst, std::string dstOffset, DynWidth regWidth, U32 imm);

void instReg(char inst, DynReg reg, DynWidth regWidth);
void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg);
void instCPU(char inst, std::string dstOffset, DynWidth regWidth);

// if conditions
void startIf(DynReg reg, DynCondition condition, bool doneWithReg);
void startElse();
void endIf();
void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
void setCPU(DynamicData* data, std::string offset, DynWidth regWidth, DynConditional condition);
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg);

// call into emulator, like setFlags, getCF, etc
void callHostFunction(std::string func, bool hasReturn = false, U32 argCount = 0, U32 arg1 = 0, DynCallParamType arg1Type = DYN_PARAM_CONST_32, bool doneWithArg1 = true, U32 arg2 = 0, DynCallParamType arg2Type = DYN_PARAM_CONST_32, bool doneWithArg2 = true, U32 arg3 = 0, DynCallParamType arg3Type = DYN_PARAM_CONST_32, bool doneWithArg3 = true, U32 arg4 = 0, DynCallParamType arg4Type = DYN_PARAM_CONST_32, bool doneWithArg4 = true, U32 arg5 = 0, DynCallParamType arg5Type = DYN_PARAM_CONST_32, bool doneWithArg5 = true);

// set up the cpu to the correct next block

// this is called for cases where we don't know ahead of time where the next block will be, so we need to look it up
void blockDone();
// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1();
void blockNext2();

const char* getReg8(int r);

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

#define NUMBER_OF_TEMP_REGS 4
static bool needReg[NUMBER_OF_TEMP_REGS];
static std::string body;
static U32 indentLevel = 1;
static std::string inlineEaa;

static void initNewFunction() {
    indentLevel = 1;
    inlineEaa = "";
    body = "";
    for (U32 i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
        needReg[i] = false;
    }
}

static void outStartLine() {
    for (U32 i = 0; i < indentLevel; i++) {
        body += "    ";
    }
}

static void out(const std::string& s) {    
    body += s;
}

static void out(U32 value) {
    std::string i = std::to_string(value);
    out(i);
}

static const char* getReg8(int r) {
    switch (r) {
    case 0:
        return "reg[0].u8";
    case 1:
        return "reg[1].u8";
    case 2:
        return "reg[2].u8";
    case 3:
        return "reg[3].u8";
    case 4:
        return "reg[0].h8";
    case 5:
        return "reg[1].h8";
    case 6:
        return "reg[2].h8";
    case 7:
        return "reg[3].h8";
    default:
        kpanic("getReg8");
        return nullptr;
    }
}

static void outDynRegName(DynReg reg) {
    if (reg == 0) {
        needReg[reg] = true;
        out("reg0");
    } else if (reg == 1) {
        needReg[reg] = true;
        out("reg1");
    } else if (reg == 2) {
        needReg[reg] = true;
        out("reg2");
    } else if (reg == 3) {
        needReg[reg] = true;
        out("reg3");
    } else {
        kpanic("getDynReg");
    }
}

static void outDynRegWidth(DynWidth width) {
    if (width == DYN_32bit) {
        out(".u32");
    }
    else if (width == DYN_16bit) {
        out(".u16");
    }
    else if (width == DYN_8bit) {
        out(".u8");
    }
    else {
        kpanic("unknown width in aot::outDynRegWidth %d", width);
    }
}

static void outDynReg(DynReg reg, DynWidth width) {
    outDynRegName(reg);
    outDynRegWidth(width);
}

static const char* r8(int r) {
    switch (r) {
    case 0: return "AL";
    case 1: return "CL";
    case 2: return "DL";
    case 3: return "BL";
    case 4: return "AH";
    case 5: return "CH";
    case 6: return "DH";
    case 7: return "BH";
    }
    return "r8";
}

static const char* r16(int r) {
    switch (r) {
    case 0: return "AX";
    case 1: return "CX";
    case 2: return "DX";
    case 3: return "BX";
    case 4: return "SP";
    case 5: return "BP";
    case 6: return "SI";
    case 7: return "DI";
    }
    return "r16";
}

static const char* r32(int r) {
    switch (r) {
    case 0: return "EAX";
    case 1: return "ECX";
    case 2: return "EDX";
    case 3: return "EBX";
    case 4: return "ESP";
    case 5: return "EBP";
    case 6: return "ESI";
    case 7: return "EDI";
    }
    return "r32";
}

static const char* getBase(int base) {
    switch (base) {
    case ES: return "ES";
    case CS: return "CS";
    case SS: return "SS";
    case DS: return "DS";
    case FS: return "FS";
    case GS: return "GS";
    }
    return "getBase";
}

static std::string getEaa16(DecodedOp* op) {
    std::string result;

    if (op->base < 6) {
        result = "cpu->seg[";
        result += getBase(op->base);
        result += "].address";
    }
    if (op->rm < 8 || op->disp) {
        result += "+ (U16)(";
        if (op->rm < 8) {
            result += r16(op->rm);
            if (op->sibIndex < 8) {
                result += " + (S16)";
                result += r16(op->sibIndex);
            }
        }
        if (op->disp) {
            char tmp[32];
            if (op->rm < 8) {
                result += " + ";
            }
            result += "0x";
            itoa(op->disp, tmp, 16);
            result += tmp;
        }
        result += ")";
    }
    if (!result.size()) {
        result = "0";
    }
    return result;
}

static std::string getEaa32(DecodedOp* op) {
    std::string result;

    if (op->base < 6) {
        result = "cpu->seg[";
        result += getBase(op->base);
        result += "].address";
    }
    if (op->rm < 8) {
        if (result.length())
            result += " + ";
        result += r32(op->rm);
    }
    if (op->sibIndex < 8) {
        if (result.length())
            result += " + ";
        if (op->sibScale)
            result += "(";
        result += r32(op->sibIndex);
        if (op->sibScale) {
            result += " << ";
            result += std::to_string(op->sibScale);
            result += ")";
        }
    }
    if (op->disp) {
        char tmp[32];
        if (result.length())
            result += " + ";
        result += "0x";
        itoa(op->disp, tmp, 16);
        result += tmp;
    }
    if (!result.size()) {
        result = "0";
    }
    return result;
}

static void calculateEaa(DecodedOp* op, DynReg reg) {
    if (op->ea16) {
        inlineEaa = getEaa16(op);
    } else {
        inlineEaa = getEaa32(op);
    }
}

static void writeEaa(DynReg reg) {
    if (inlineEaa.size()) {
        outStartLine();
        outDynReg(reg, DYN_32bit);
        out(" = ");
        out(inlineEaa);
        out(";\r\n");
        inlineEaa = "";
    }
}

static void readFromMem(DynWidth width, DynReg addressReg) {
    if (width == DYN_32bit) {
        out("readd(");
    }
    else if (width == DYN_16bit) {
        out("readw(");
    }
    else if (width == DYN_8bit) {
        out("readb(");
    }
    else {
        kpanic("unknown width in aot::movFromMem %d", width);
    }
    if (inlineEaa.size()) {
        out(inlineEaa);
        inlineEaa = "";
    } else {
        outDynReg(addressReg, DYN_32bit);
    }
    out(")");
}

static void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
    if (!doneWithAddressReg) {
        writeEaa(addressReg);
    }
    outStartLine();
    outDynReg(DYN_CALL_RESULT, width);
    out(" = ");
    readFromMem(width, addressReg);
    out(";\r\n");
}

static const char* getBitCount(DynWidth width) {
    if (width == DYN_32bit) {
        return "32";
    } else if (width == DYN_16bit) {
        return "16";
    } else if (width == DYN_8bit) {
        return "8";
    } else {
        kpanic("unknown width in aot::getBitCount %d", width);
        return nullptr;
    }
}

static void movToCpuFromMem(std::string dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    if (!doneWithAddressReg) {
        writeEaa(addressReg);
    }    
    if (doneWithCallResult) {
        outStartLine();
        out(dstOffset);
        out(" = ");
        readFromMem(dstWidth, addressReg);
        out(";\r\n");
    } else {
        movFromMem(dstWidth, addressReg, doneWithAddressReg); // has its own outStartLine();
        outStartLine();
        out(dstOffset);
        out(" = ");
        outDynReg(DYN_CALL_RESULT, dstWidth);
        out(";\r\n");
    }
}

static void movToCpuFromCpu(std::string dstOffset, std::string srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    outStartLine();
    out(dstOffset);
    out(" = ");
    out(srcOffset);
    out(";\r\n");
    if (!doneWithTmpReg) {
        outStartLine();
        outDynReg(tmpReg, width);
        out(" = ");
        out(srcOffset);
        out(";\r\n");
    }
}

static void byteSwapReg32(DynReg reg) {
    outStartLine();
    outDynReg(reg, DYN_32bit);
    out(" = ");
    out("((("); outDynReg(reg, DYN_32bit); out(" & 0xff000000) >> 24) | (("); outDynReg(reg, DYN_32bit); out(" & 0x00ff0000) >> 8) | (("); outDynReg(reg, DYN_32bit); out(" & 0x0000ff00) << 8) | (("); outDynReg(reg, DYN_32bit); out(" & 0x000000ff) << 24)); \r\n");
}

static void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    outStartLine();
    outDynReg(dst, dstWidth);
    out(" = ");
    out("(U");
    out(getBitCount(dstWidth));
    out(")");
    out("(S");
    out(getBitCount(srcWidth));
    out(")");
    outDynReg(src, srcWidth);
    out(";\r\n");
}

static void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    outStartLine();
    outDynReg(dst, dstWidth);
    out(" = ");
    if (dstWidth < srcWidth) {
        out("(U");
        out(getBitCount(dstWidth));
        out(")");
    }
    outDynReg(src, srcWidth);
    out(";\r\n");
}

static void movToReg(DynReg reg, DynWidth width, U32 imm) {
    outStartLine();
    outDynReg(reg, width);
    out(" = ");
    std::string i = std::to_string(imm);
    out(i);
    out(";\r\n");
}

static void movToCpuFromReg(std::string dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (reg == DYN_ADDRESS && !doneWithReg) {
        writeEaa(reg);
    }
    outStartLine();
    out(dstOffset);
    out(" = ");
    if (reg == DYN_ADDRESS && inlineEaa.size()) {
        out(inlineEaa);
        inlineEaa = "";
    } else {
        outDynReg(reg, width);
    }
    out(";\r\n");
}

static void movToCpu(std::string dstOffset, DynWidth dstWidth, U32 imm) {
    outStartLine();
    out(dstOffset);
    out(" = ");
    std::string i = std::to_string(imm);
    out(i);
    out(";\r\n");
}

static void movToCpuLazyFlags(std::string dstOffset, std::string lazyFlags) {
    outStartLine();
    out(dstOffset);
    out(" = ");
    out(lazyFlags);
    out(";\r\n");
}

static void movToRegFromCpu(DynReg reg, std::string srcOffset, DynWidth width) {
    outStartLine();
    outDynReg(reg, width);
    out(" = ");
    out(srcOffset);
    out(";\r\n");
}

static void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg) {
    if (!doneWithAddressReg) {
        writeEaa(addressReg);
    }
    outStartLine();
    if (width == DYN_32bit) {
        out("writed(");
    }
    else if (width == DYN_16bit) {
        out("writew(");
    }
    else if (width == DYN_8bit) {
        out("writeb(");
    }
    else {
        kpanic("unknown width in aot::movToMemFromReg %d", width);
    }
    if (inlineEaa.size()) {
        out(inlineEaa);
        inlineEaa = "";
    } else {
        outDynReg(addressReg, DYN_32bit);
    }
    out(", ");
    outDynReg(reg, width);
    out(");\r\n");
}

static void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg) {
    if (!doneWithAddressReg) {
        writeEaa(addressReg);
    }
    outStartLine();
    if (width == DYN_32bit) {
        out("writed(");
    }
    else if (width == DYN_16bit) {
        out("writew(");
    }
    else if (width == DYN_8bit) {
        out("writeb(");
    }
    else {
        kpanic("unknown width in aot::movToMemFromReg %d", width);
    }
    if (inlineEaa.size()) {
        out(inlineEaa);
        inlineEaa = "";
    } else {
        outDynReg(addressReg, DYN_32bit);
    }
    out(", ");
    std::string i = std::to_string(imm);
    out(i);
    out(");\r\n");
}

// inst can be +, |, -, &, ^, <, >, ) right parens is for signed right shift
static void outInst(char inst, std::function<void(void)> dest, std::function<void(void)> left, std::function<void(void)> right, DynWidth regWidth) {
    std::string op(1, inst);
    bool isSigned = false;

    outStartLine();
    if (op == "<") {
        op = "<<";
    }
    else if (op == ">") {
        op = ">>";
    }
    else if (op == ")") {
        op = ">>";
        isSigned = true;
    }
    dest();
    out(" = ");
    if (isSigned) {
        out("(S");
        out(getBitCount(regWidth));
        out(")");
    }
    left();
    out(" ");
    out(op);
    out(" ");
    right();
    out(";\r\n");
}

static void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    outInst(inst, 
        [reg, regWidth]() {
            outDynReg(reg, regWidth);
        },
        [reg, regWidth]() {
            outDynReg(reg, regWidth);
        },
        [rm, regWidth]() {
            outDynReg(rm, regWidth);
        }, regWidth);
}

static void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg) {
    writeEaa(addressReg);
    movFromMem(regWidth, addressReg, false);
    instRegReg(inst, DYN_CALL_RESULT, rm, regWidth, true);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true);
}

static void instCPUReg(char inst, std::string dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    outInst(inst,
        [dstOffset]() {
            out(dstOffset);
        },
        [dstOffset]() {
            out(dstOffset);
        },
            [rm, regWidth]() {
            outDynReg(rm, regWidth);
        }, regWidth);
}

static void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    outInst(inst,
        [reg, regWidth]() {
            outDynReg(reg, regWidth);
        },
        [reg, regWidth]() {
            outDynReg(reg, regWidth);
        },
            [imm]() {
            std::string i = std::to_string(imm);
            out(i);
        }, regWidth);
}

static void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg) {
    writeEaa(addressReg);
    movFromMem(regWidth, addressReg, false);
    instRegImm(inst, DYN_CALL_RESULT, regWidth, imm);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true);
}

static void instCPUImm(char inst, std::string dstOffset, DynWidth regWidth, U32 imm) {
    outInst(inst,
        [dstOffset]() {
            out(dstOffset);
        },
        [dstOffset]() {
            out(dstOffset);
        },
            [imm]() {
            std::string i = std::to_string(imm);
            out(i);
        }, regWidth);
}

// inst can be: ~ or -
static void instReg(char inst, DynReg reg, DynWidth regWidth) {
    std::string op(1, inst);
    outStartLine();
    outDynReg(reg, regWidth);
    out(" = ");
    if (op == "~") {
        out(op);
        out(" ");
    } else {
        out("0 - ");
    }
    outDynReg(reg, regWidth);
    out(";\r\n");
}

static void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg) {
    writeEaa(addressReg);
    movFromMem(regWidth, addressReg, false);
    instReg(inst, DYN_CALL_RESULT, regWidth);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, true, true);
}

static void instCPU(char inst, std::string dstOffset, DynWidth regWidth) {
    std::string op(1, inst);
    outStartLine();
    out(dstOffset);
    out(" = ");
    if (op == "~") {
        out(op);
        out(" ");
    }
    else {
        out("0 - ");
    }
    out(dstOffset);
    out(";\r\n");
}

// if conditions
static void startIf(DynReg reg, DynCondition condition, bool doneWithReg) {
    outStartLine();
    out("if (");    
    if (condition == DYN_EQUALS_ZERO) {
        out("!");
    }
    outDynReg(reg, DYN_32bit);
    out("){\r\n");
    indentLevel++;
}

static void startElse() {
    indentLevel--;
    outStartLine();
    out("} else {\r\n");
    indentLevel++;
}

static void endIf() {
    indentLevel--;
    outStartLine();
    out("}\r\n");
}

static void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    bool isSigned = condition == DYN_LESS_THAN_SIGNED || condition == DYN_LESS_THAN_EQUAL_SIGNED;
    outStartLine();
    outDynReg(reg, dstWidth);
    out(" = ");
    if (isSigned) {
        out("(S");
        out(getBitCount(regWidth));
        out(")");
    }
    outDynReg(left, regWidth);
    switch (condition) {
    case DYN_EQUALS:
        out(" == ");
        break;
    case DYN_NOT_EQUALS:
        out(" != ");
        break;
    case DYN_LESS_THAN_UNSIGNED:
        out(" < ");
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        out(" <= ");
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:
        out(" >= ");
        break;
    case DYN_LESS_THAN_SIGNED:
        out(" < ");
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        out(" <= ");
        break;
    default:
        kpanic("aot::evaluateToReg");
        break;
    }
    if (isSigned) {
        out("(S");
        out(getBitCount(regWidth));
        out(")");
    }
    if (isRightConst) {
        std::string i = std::to_string(rightConst);
        out(i);
    } else {
        outDynReg(right, regWidth);
    }
    out(";\r\n");
}

static void setCPU(DynamicData* data, std::string offset, DynWidth regWidth, DynConditional condition) {
    DynCondition cond = DYN_EQUALS_ZERO;
    // changing conditions to ones that are optimized
    switch (condition) {
    case NO:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = O;
        break;
    case NB:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = B;
        break;
    case Z:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = NZ;
        break;
    case NBE:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = BE;
        break;
    case NS:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = S;
        break;
    case NL:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = L;
        break;
    case NLE:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = LE;
        break;
    }

    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, cond, true);
    movToCpu(offset, regWidth, 0);
    startElse();
    movToCpu(offset, regWidth, 1);
    endIf();
}

static void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    writeEaa(addressReg);
    DynCondition cond = DYN_EQUALS_ZERO;
    // changing conditions to ones that are optimized
    switch (condition) {
    case NO:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = O;
        break;
    case NB:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = B;
        break;
    case Z:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = NZ;
        break;
    case NBE:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = BE;
        break;
    case NS:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = S;
        break;
    case NL:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = L;
        break;
    case NLE:
        cond = DYN_NOT_EQUALS_ZERO;
        condition = LE;
        break;
    }

    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, cond, true);
    movToMemFromImm(addressReg, regWidth, 0, true);
    startElse();
    movToMemFromImm(addressReg, regWidth, 1, true);
    endIf();
}


static void outArg(U32 arg, DynCallParamType type, bool doneWithReg) {
    switch (type) {
    case DYN_PARAM_REG_8:
        outDynReg((DynReg)arg, DYN_8bit);
        break;
    case DYN_PARAM_REG_16:
        outDynReg((DynReg)arg, DYN_16bit);
        break;
    case DYN_PARAM_REG_32:
    {
        DynReg reg = (DynReg)arg;
        if (reg == DYN_ADDRESS && !doneWithReg) {
            writeEaa(reg);
        }
        if (reg == DYN_ADDRESS && inlineEaa.size()) {
            out(inlineEaa);
            inlineEaa = "";
        } else {
            outDynReg(reg, DYN_32bit);
        }
        break;
    }
    case DYN_PARAM_CONST_8:
    case DYN_PARAM_CONST_16:
    case DYN_PARAM_CONST_32:
    {
        std::string i = std::to_string(arg);
        out(i);
        break;
    }
    case DYN_PARAM_OP:
    {
        DecodedOp* op = (DecodedOp*)arg;
        out("getOp(");
        out(op->ea16 ? "true, " : "false, ");
        out(op->base);
        out(", ");
        out(op->reg);
        out(", ");
        out(op->rm);
        out(", ");
        out(op->disp);
        out(", ");
        out(op->sibIndex);
        out(", ");
        out(op->sibScale);
        out(")");
        break;
    }
    case DYN_PARAM_CPU_ADDRESS_8:
        out((const char*)arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        out((const char*)arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        out((const char*)arg);
        break;
    case DYN_PARAM_CPU:
        out("cpu");
        break;
    default:
        kpanic("aot::outArg");
        break;
    }
}

// call into emulator, like setFlags, getCF, etc
static void callHostFunction(std::string func, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    outStartLine();
    if (hasReturn) {
        outDynReg(DYN_CALL_RESULT, DYN_32bit);
        out(" = ");
    }    
    out(func);
    out("(");
    if (argCount > 0) {
        outArg(arg1, arg1Type, doneWithArg1);
    }
    if (argCount > 1) {
        out(", ");
        outArg(arg2, arg2Type, doneWithArg2);
    }
    if (argCount > 2) {
        out(", ");
        outArg(arg3, arg3Type, doneWithArg3);
    }
    if (argCount > 3) {
        out(", ");
        outArg(arg4, arg4Type, doneWithArg4);
    }
    if (argCount > 4) {
        out(", ");
        outArg(arg5, arg5Type, doneWithArg5);
    }
    out(");\r\n");
}

// set up the cpu to the correct next block

// this is called for cases where we don't know ahead of time where the next block will be, so we need to look it up
static void blockDone() {
    outStartLine();
    out("cpu->nextBlock = cpu->getNextBlock();\r\n");
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
static void writeNext1() {
    out("void nextBlock1(CPU* cpu) {\r\n");
    outStartLine();
    out("if (!DecodedBlock::currentBlock->next1) {\r\n");
    indentLevel++;
    outStartLine();
    out("DecodedBlock::currentBlock->next1 = cpu->getNextBlock();\r\n");
    outStartLine();
    out("DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);\r\n");
    indentLevel--;
    outStartLine();
    out("}\r\n");
    outStartLine();
    out("cpu->nextBlock = DecodedBlock::currentBlock->next1;\r\n");
    out("}\r\n");
}

static void blockNext1() {
    outStartLine();
    out("nextBlock1(cpu);\r\n");
}

static void writeNext2() {
    out("void nextBlock2(CPU* cpu) {\r\n");
    outStartLine();
    out("if (!DecodedBlock::currentBlock->next2) {\r\n");
    indentLevel++;
    outStartLine();
    out("DecodedBlock::currentBlock->next2 = cpu->getNextBlock();\r\n");
    outStartLine();
    out("DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);\r\n");
    indentLevel--;
    outStartLine();
    out("}\r\n");
    outStartLine();
    out("cpu->nextBlock = DecodedBlock::currentBlock->next2;\r\n");
    out("}\r\n");
}

static void blockNext2() {
    outStartLine();
    out("nextBlock2(cpu);\r\n");
}

static void writeGetOp() {
    out("DecodedOp* getOp(bool ea16, U32 base, U32 reg, U32 rm, U32 disp, U32 sibIndex, U32 sibScale) {\r\n");
    outStartLine();
    out("static DecodedOp op;\r\n");
    outStartLine();
    out("op.ea16 = ea16;\r\n");
    outStartLine();
    out("op.base = (U8)base;\r\n");
    outStartLine();
    out("op.reg = (U8)reg;\r\n");
    outStartLine();
    out("op.rm = (U8)rm;\r\n");
    outStartLine();
    out("op.disp = disp;\r\n");
    outStartLine();
    out("op.sibIndex = (U8)sibIndex;\r\n");
    outStartLine();
    out("op.sibScale = (U8)sibScale;\r\n");
    outStartLine();
    out("return &op;\r\n");
    out("}\r\n");
}

static void incrementEip(DynamicData* data, U32 inc) {
    outStartLine();
    if (data->skipEipUpdateLen) {
        kpanic("incrementEip had an unexpected update");
    }
    std::string len = std::to_string((S32)inc);

    out("cpu->eip.u32 += ");
    out(len);
    out(";\r\n");
}

static void incrementEip(DynamicData* data, DecodedOp* op) {
    if (op->next) {
        const InstructionInfo& info = instructionInfo[op->next->inst];
        if (!info.branch && !info.readMemWidth && !info.writeMemWidth && !info.throwsException) {
            data->skipEipUpdateLen += op->len;
            return;
        }
    }
    std::string len = std::to_string((S32)(op->len + data->skipEipUpdateLen));
    
    data->skipEipUpdateLen = 0;
    outStartLine();
    out("cpu->eip.u32 += ");
    out(len);
    out(";\r\n");
}

static void dyn_sidt(DynamicData* data, DecodedOp* op) {
}

static void dyn_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

static void dyn_callback(DynamicData* data, DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        callHostFunction(DYN_HOST_FN(dyn_onExitSignal), false, 1, 0, DYN_PARAM_CPU);
    }
    else {
        kpanic("dyn_callback unhandled callback");
    }
}

static void dyn_invalid_op(DynamicData* data, DecodedOp* op) {
    kpanic("Invalid instruction %x\n", op->inst);
}

class GeneratedFunction {
public:
    GeneratedFunction(const std::string& name, const unsigned char* bytes, U32 len, U32 crc, bool big) : name(name), bytes(bytes), len(len), crc(crc), next(0), big(big) {}
    ~GeneratedFunction() {
        if (bytes) {
            delete[] bytes;
        }
    };
    std::string name;
    const unsigned char* bytes;
    U32 len;
    U32 crc;
    std::shared_ptr<GeneratedFunction> next;
    bool big;
};

static pfnDynamicOp dynOps[NUMBER_OF_OPS];
static U32 dynOpsInitialized;
static std::unordered_map<U32, std::shared_ptr<GeneratedFunction>> functions;
static std::vector<std::shared_ptr<GeneratedFunction>> functionArray;
static U32 functionCount;
static std::string functionSource;

static std::shared_ptr<GeneratedFunction> getGeneratedFunction(U32 crc) {
    if (functions.count(crc)) {
        return functions[crc];
    }
    return nullptr;
}

static void initDynOps() {
    if (dynOpsInitialized)
        return;

    dynOpsInitialized = 1;
    for (int i = 0; i < InstructionCount; i++) {
        dynOps[i] = dyn_invalid_op;
    }
#define INIT_CPU(e, f) dynOps[e] = dynamic_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#undef INIT_CPU    

    dynOps[SLDTReg] = 0;
    dynOps[SLDTE16] = 0;
    dynOps[STRReg] = 0;
    dynOps[STRE16] = 0;
    dynOps[LLDTR16] = 0;
    dynOps[LLDTE16] = 0;
    dynOps[LTRR16] = 0;
    dynOps[LTRE16] = 0;
    dynOps[VERRR16] = 0;
    dynOps[VERWR16] = 0;
    dynOps[SGDT] = 0;
    dynOps[SIDT] = dyn_sidt;
    dynOps[LGDT] = 0;
    dynOps[LIDT] = 0;
    dynOps[SMSWRreg] = 0;
    dynOps[SMSW] = 0;
    dynOps[LMSWRreg] = 0;
    dynOps[LMSW] = 0;
    dynOps[INVLPG] = 0;
    dynOps[Callback] = dyn_callback;

    initNewFunction();
    out("#include \"boxedwine.h\"\r\n");
    out("#include \"../normal/instructions.h\"\r\n");
    out("#include \"../normal/normal_shift.h\"\r\n");
    out("#include \"../normal/normal_strings.h\"\r\n");
    out("#include \"../common/common_arith.h\"\r\n");
    out("#include \"../common/common_bit.h\"\r\n");
    out("#include \"../common/common_fpu.h\"\r\n");
    out("#include \"../common/common_mmx.h\"\r\n");
    out("#include \"../common/common_other.h\"\r\n");
    out("#include \"../common/common_sse.h\"\r\n");
    out("#include \"../common/common_sse2.h\"\r\n");
    out("#include \"../common/common_xchg.h\"\r\n");
    writeNext1();
    writeNext2();
    writeGetOp();
    functionSource += body;
}

static void generateSource(CPU* cpu, DecodedOp* op) {
    U32 len = DecodedBlock::currentBlock->bytes;
    unsigned char* bytes = new unsigned char[len];
    memcopyToNative(DecodedBlock::currentBlock->address, bytes, len);
    U32 crc = crc32b(bytes, len);
    std::shared_ptr<GeneratedFunction> existing = getGeneratedFunction(crc);
    while (existing) {
        if (len == existing->len && !memcmp(existing->bytes, bytes, len) && cpu->isBig() == existing->big) {
            delete[] bytes;
            return;
        }
        existing = existing->next;
    }

    DynamicData data;
    data.cpu = cpu;
    data.block = DecodedBlock::currentBlock;
    
    initDynOps();
    initNewFunction();
    DecodedOp* o = op->next;    

    while (o) {
#ifdef _DEBUG
        out("// ");
        out(instructionLog[o->inst].name);
        out("\r\n");
#endif
        dynOps[o->inst](&data, o);
        if (data.skipToOp) {
            o = data.skipToOp;
            data.skipToOp = NULL;
        }
        else if (data.done) {
            break;
        }
        else {
            o = o->next;
        }
    }
    std::string name = "generated" + std::to_string(functionCount++);
    std::string funcSource;
    funcSource = "void OPCALL ";
    funcSource += name;
    funcSource += "(CPU* cpu, DecodedOp* op) {\r\n";
    for (int i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
        if (needReg[i]) {
            funcSource += "    Reg reg";
            funcSource += std::to_string(i);
            funcSource += ";\r\n";
        }
    }
    funcSource += body;
    funcSource += "}\r\n";

    std::shared_ptr<GeneratedFunction> func = std::make_shared<GeneratedFunction>(name, bytes, len, crc, cpu->isBig());
    std::shared_ptr<GeneratedFunction> prev = getGeneratedFunction(crc);
    if (prev) {
        prev->next = func;
    } else {
        functions[crc] = func;
    }
    functionArray.push_back(func);
    functionSource += funcSource;    
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
    if (DecodedBlock::currentBlock->runCount == 50) {
        generateSource(cpu, op);
    }
    op->next->pfn(cpu, op->next);
}

static void outfp(FILE* fp, const char* str) {
    fwrite(str, strlen(str), 1, fp);
}

static bool compareGeneratedFunctionsByCRC(std::shared_ptr<GeneratedFunction>& f1, std::shared_ptr<GeneratedFunction>& f2)
{
    return (f1->crc < f2->crc);
}

static void writeLookups(FILE* fp) {
    char tmp[10];

    sort(functionArray.begin(), functionArray.end(), compareGeneratedFunctionsByCRC);

    for (U32 i = 0; i < functionArray.size(); i++) {
        outfp(fp, "const unsigned char data");
        itoa(i, tmp, 10);
        outfp(fp, tmp);
        outfp(fp, "[] = {");
        for (U32 j = 0; j < functionArray[i]->len; j++) {
            if (j > 0)
                outfp(fp, ", ");
            sprintf(tmp, "0x%.02X", ((U32)functionArray[i]->bytes[j]) & 0xFF);
            outfp(fp, tmp);
        }
        outfp(fp, "};\n");
    }

    std::string count = std::to_string(functionArray.size());
    outfp(fp, "class CompiledCode {\n");
    outfp(fp, "public:\n");
    outfp(fp, "    CompiledCode(U32 crc, OpCallback func = nullptr, const unsigned char* bytes = nullptr, U32 byteLen = 0, bool big = true): crc(crc), func(func), bytes(bytes), byteLen(byteLen), big(big) {}\n");
    outfp(fp, "    U32 crc; \n");
    outfp(fp, "    OpCallback func;\n");
    outfp(fp, "    const unsigned char* bytes;\n");
    outfp(fp, "    unsigned int byteLen;\n");
    outfp(fp, "    bool big;\n");
    outfp(fp, "    bool operator<(const CompiledCode & other) const {\n");
    outfp(fp, "        return crc < other.crc;\n");
    outfp(fp, "    }\n");
    outfp(fp, "}; \n");
    outfp(fp, "static CompiledCode compiledCode[] = {\n");
    
    for (U32 i = 0; i < functionArray.size(); i++) {
        outfp(fp, "CompiledCode(0x");
        itoa(functionArray[i]->crc, tmp, 16);
        outfp(fp, tmp);
        outfp(fp, ", ");
        outfp(fp, functionArray[i]->name.c_str());
        outfp(fp, ", data");
        itoa(i, tmp, 10);
        outfp(fp, tmp);
        outfp(fp, ", ");
        itoa(functionArray[i]->len, tmp, 10);
        outfp(fp, tmp);
        outfp(fp, ", ");
        outfp(fp, functionArray[i]->big ? "true" : "false");
        outfp(fp, "),\n");
    }
    outfp(fp, "};\n");

    outfp(fp, "static std::vector<CompiledCode> functions(compiledCode, compiledCode + ");
    outfp(fp, count.c_str());
    outfp(fp, ");\n");

    outfp(fp, "OpCallback getCompiledFunction(CPU* cpu, U32 crc, const unsigned char* bytes, U32 byteLen, U32 ip) {\n");
    outfp(fp, "    auto lower = std::lower_bound(functions.begin(), functions.end(), CompiledCode(crc));\n");
    outfp(fp, "    if (lower == functions.end()) return nullptr;\n");
    outfp(fp, "    U32 i = std::distance(functions.begin(), lower);\n");
    outfp(fp, "    while (functions[i].crc==crc) {\n");
    outfp(fp, "        if (functions[i].byteLen >= byteLen && cpu->isBig()==functions[i].big && !memcmp(bytes, functions[i].bytes, byteLen)) {\n");
    outfp(fp, "            U32 count = functions[i].byteLen - byteLen;\n");
    outfp(fp, "            U32 p = ip;\n");
    outfp(fp, "            U32 pos = byteLen;\n");
    outfp(fp, "            while (count) {\n");
    outfp(fp, "                if (functions[i].bytes[pos++]!=readb(p++))\n");
    outfp(fp, "                    break;\n");
    outfp(fp, "                count--;\n");
    outfp(fp, "            }\n");
    outfp(fp, "            if (count==0)\n");
    outfp(fp, "                return functions[i].func;\n");
    outfp(fp, "        }\n");
    outfp(fp, "        i++;\n");
    outfp(fp, "    }\n");
    outfp(fp, "    return nullptr;\n");
    outfp(fp, "}\n");
}

void writeSource(const std::string path) {
    FILE* fp = fopen(path.c_str(), "wb");
    if (fp) {
        fwrite(functionSource.c_str(), functionSource.size(), 1, fp);
        writeLookups(fp);
        fflush(fp);
        fclose(fp);
    }
}

#endif