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

static const char* r8(int r);
static const char* r16(int r);
static const char* r32(int r);

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

#ifdef BOXEDWINE_64
#define DYN_PTR_SIZE U64
#else
#define DYN_PTR_SIZE U32
#endif

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
static void movToCpuFromMem(std::string dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
static void movToCpuFromCpu(std::string dstOffset, std::string srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
static void calculateEaa(DecodedOp* op, DynReg reg);

static void byteSwapReg32(DynReg reg);

// REG to REG
static void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);
static void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);

// to Reg
static void movToReg(DynReg reg, DynWidth width, U32 imm);

// to CPU
static void movToCpuFromReg(std::string dstOffset, DynReg reg, DynWidth width, bool doneWithReg);
static void movToCpu(std::string dstOffset, DynWidth dstWidth, U32 imm);
static void movToCpuLazyFlags(std::string dstOffset, std::string lazyFlags);

// from CPU
static void movToRegFromCpu(DynReg reg, std::string srcOffset, DynWidth width);

// from Mem to DYN_READ_RESULT
static void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg);

// to Mem
static void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg);
static void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg);

// arith
static void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
static void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg);
static void instCPUReg(char inst, std::string dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg);

static void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
static void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg);
static void instCPUImm(char inst, std::string dstOffset, DynWidth regWidth, U32 imm);

static void instReg(char inst, DynReg reg, DynWidth regWidth);
static void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg);
static void instCPU(char inst, std::string dstOffset, DynWidth regWidth);

// if conditions
static void startIf(DynReg reg, DynCondition condition, bool doneWithReg);
static void startElse();
static void endIf();
static void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
static void setCPU(DynamicData* data, std::string offset, DynWidth regWidth, DynConditional condition);
static void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg);

// call into emulator, like setFlags, getCF, etc
static void callHostFunction(std::string func, bool hasReturn = false, U32 argCount = 0, DYN_PTR_SIZE arg1 = 0, DynCallParamType arg1Type = DYN_PARAM_CONST_32, bool doneWithArg1 = true, DYN_PTR_SIZE arg2 = 0, DynCallParamType arg2Type = DYN_PARAM_CONST_32, bool doneWithArg2 = true, DYN_PTR_SIZE arg3 = 0, DynCallParamType arg3Type = DYN_PARAM_CONST_32, bool doneWithArg3 = true, DYN_PTR_SIZE arg4 = 0, DynCallParamType arg4Type = DYN_PARAM_CONST_32, bool doneWithArg4 = true, DYN_PTR_SIZE arg5 = 0, DynCallParamType arg5Type = DYN_PARAM_CONST_32, bool doneWithArg5 = true);

// set up the cpu to the correct next block

// this is called for cases where we don't know ahead of time where the next block will be, so we need to look it up
static void blockDone();
// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
static void blockNext1();
static void blockNext2();

static const char* getReg8(int r);

/********************************************************/
/* End required for dynamic code                        */
/********************************************************/

// referenced in macro above
static void incrementEip(DynamicData* data, DecodedOp* op);
static void incrementEip(DynamicData* data, U32 len);

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

class InlineReg {
public:
    InlineReg() : calculated(false), pendingClear(false) {}
    void reset() {
        calculated = false;
        value = "";
        pendingClear = false;
        for (int i = 0; i < 8; i++) {
            readReg[i] = false;
        }
    }
    bool readReg[8];
    bool calculated;
    std::string value;
    DynWidth width;
    bool pendingClear;
};

static bool needReg[NUMBER_OF_TEMP_REGS];
static InlineReg inlineReg[NUMBER_OF_TEMP_REGS];
static std::string body;
static U32 indentLevel = 1;

static void initNewFunction() {
    indentLevel = 1;
    body = "";
    for (U32 i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
        needReg[i] = false;
        inlineReg[i].reset();
    }
}

static void clearPendingInlineRegs() {
    for (U32 i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
        if (inlineReg[i].pendingClear) {
            inlineReg[i].pendingClear = false;
            inlineReg[i].value = "";
        }
    }
}

static void outStartLine() {
    for (U32 i = 0; i < indentLevel; i++) {
        body += "    ";        
    }
    clearPendingInlineRegs();
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

static const char* dynRegName(DynReg reg) {
    if (reg == 0) {
        needReg[reg] = true;
        return "reg0";
    } else if (reg == 1) {
        needReg[reg] = true;
        return "reg1";
    } else if (reg == 2) {
        needReg[reg] = true;
        return "reg2";
    } else if (reg == 3) {
        needReg[reg] = true;
        return "reg3";
    } else {
        kpanic("getDynReg");
    }
}

static const char* dynRegWidth(DynWidth width) {
    if (width == DYN_32bit) {
        return ".u32";
    }
    else if (width == DYN_16bit) {
        return ".u16";
    }
    else if (width == DYN_8bit) {
        return ".u8";
    }
    else {
        kpanic("unknown width in aot::outDynRegWidth %d", width);
        return nullptr;
    }
}

static void outDynRegRead(DynReg reg, DynWidth width) {
    if (inlineReg[reg].value.length()) {
        out(inlineReg[reg].value);
    } else {
        out(dynRegName(reg));
        out(dynRegWidth(width));
    }
}

static std::string dynRegRead(DynReg reg, DynWidth width) {
    if (inlineReg[reg].value.length()) {
        return inlineReg[reg].value;
    }
    return dynRegName(reg) + std::string(dynRegWidth(width));
}

static void outDynRegWrite(DynReg reg, DynWidth width) {
    out(dynRegName(reg));
    out(dynRegWidth(width));
    inlineReg[reg].pendingClear = true;
}

static std::string dynRegWrite(DynReg reg, DynWidth width) {
    inlineReg[reg].pendingClear = true;
    return dynRegName(reg) + std::string(dynRegWidth(width));
}

static int getRegIndex(const std::string& r) {
    if (r == "AL" || r == "AH" || r == "AX" || r == "EAX") {
        return 0;
    }
    if (r == "CL" || r == "CH" || r == "CX" || r == "ECX") {
        return 1;
    }
    if (r == "DL" || r == "DH" || r == "DX" || r == "EDX") {
        return 2;
    }
    if (r == "BL" || r == "BH" || r == "BX" || r == "EBX") {
        return 3;
    }
    if (r == "SP" || r == "ESP") {
        return 4;
    }
    if (r == "BP" || r == "EBP") {
        return 5;
    }
    if (r == "SI" || r == "ESI") {
        return 6;
    }
    if (r == "DI" || r == "EDI") {
        return 7;
    }
    return -1;
}

static void ensureInlineRegNotClobbered(int regIndex, int ignoreTmpRegIndex) {
    if (regIndex >= 0 && regIndex < 8) {
        for (int i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
            if (i == ignoreTmpRegIndex) {
                continue;
            }
            if (inlineReg[i].pendingClear) {
                inlineReg[i].pendingClear = false;
                inlineReg[i].reset();
            }
            if (inlineReg[i].value.size() && inlineReg[i].readReg[regIndex]) {
                outStartLine();
                DynReg reg = (DynReg)i;
                outDynRegWrite(reg, inlineReg[i].width);
                out(" = ");
                out(inlineReg[reg].value);
                out(";\r\n");
                inlineReg[reg].reset();
            }
        }
    }
}

static void ensureCalculatedReg(DynReg reg, DynWidth width) {
    if (inlineReg[reg].pendingClear) {
        inlineReg[reg].pendingClear = false;
        inlineReg[reg].reset();
    }
    if (inlineReg[reg].value.size() && inlineReg[reg].calculated) {
        outStartLine();
        outDynRegWrite(reg, width);
        out(" = ");
        out(inlineReg[reg].value);
        out(";\r\n");
        inlineReg[reg].reset();
    }
}

static void flushRegs() {
    for (int i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
        if (inlineReg[i].pendingClear) {
            inlineReg[i].pendingClear = false;
            inlineReg[i].reset();
        }
        if (inlineReg[i].value.size()) {
            outStartLine();
            DynReg reg = (DynReg)i;
            outDynRegWrite(reg, inlineReg[i].width);
            out(" = ");
            out(inlineReg[reg].value);
            out(";\r\n");
            inlineReg[reg].reset();
        }
    }
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
            if (op->rm < 8) {
                result += " + ";
            }
            result += "0x";
            result += toHexString(op->disp);
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
        if (result.length())
            result += " + ";
        result += "0x";
        result += toHexString(op->disp);
    }
    if (!result.size()) {
        result = "0";
    }
    return result;
}

static void calculateEaa(DecodedOp* op, DynReg reg) {
    if (op->ea16) {
        inlineReg[DYN_ADDRESS].value = getEaa16(op);
    } else {
        inlineReg[DYN_ADDRESS].value = getEaa32(op);
    }
    inlineReg[DYN_ADDRESS].calculated = true;
    inlineReg[DYN_ADDRESS].width = DYN_32bit;
    if (op->rm < 8) {
        inlineReg[DYN_ADDRESS].readReg[op->rm] = true;
    }
    if (op->sibIndex < 8) {
        inlineReg[DYN_ADDRESS].readReg[op->sibIndex] = true;
    }
}

static void writeEaa(DynReg reg) {
    if (inlineReg[DYN_ADDRESS].value.size()) {
        outStartLine();
        outDynRegWrite(reg, DYN_32bit);
        out(" = ");
        out(inlineReg[DYN_ADDRESS].value);
        out(";\r\n");
        inlineReg[DYN_ADDRESS].reset();
    }
}

static std::string readFromMem(DynWidth width, DynReg addressReg) {
    std::string result;
    if (width == DYN_32bit) {
        result+="readd(";
    }
    else if (width == DYN_16bit) {
        result+="readw(";
    }
    else if (width == DYN_8bit) {
        result+="readb(";
    }
    else {
        kpanic("unknown width in aot::movFromMem %d", width);
    }
    result+=dynRegRead(addressReg, DYN_32bit);
    result+=")";
    return result;
}

static void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
    clearPendingInlineRegs();
    if (!doneWithAddressReg) {
        writeEaa(addressReg);
    }
    inlineReg[DYN_CALL_RESULT].value = readFromMem(width, addressReg);
    inlineReg[DYN_CALL_RESULT].calculated = true;
    inlineReg[DYN_CALL_RESULT].width = width;
    if (inlineReg[DYN_ADDRESS].value.size()) {
        memcpy(inlineReg[DYN_CALL_RESULT].readReg, inlineReg[DYN_ADDRESS].readReg, sizeof(inlineReg[DYN_ADDRESS].readReg));
    }
    
    if (doneWithAddressReg) {
        inlineReg[DYN_ADDRESS].reset();
    }
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
        out(readFromMem(dstWidth, addressReg));
        out(";\r\n");
    } else {
        movFromMem(dstWidth, addressReg, doneWithAddressReg); // has its own outStartLine();
        outStartLine();
        out(dstOffset);
        out(" = ");
        outDynRegRead(DYN_CALL_RESULT, dstWidth);
        out(";\r\n");
        inlineReg[DYN_CALL_RESULT].reset();
        inlineReg[DYN_CALL_RESULT].value = dstOffset;
        int regIndex = getRegIndex(dstOffset);
        if (regIndex >= 0 && regIndex < 8) {
            inlineReg[DYN_CALL_RESULT].readReg[regIndex] = true;
        }
    }
    if (doneWithAddressReg) {
        inlineReg[DYN_ADDRESS].reset();
    }
    if (doneWithCallResult) {
        inlineReg[DYN_CALL_RESULT].reset();
    }
}

static void movToCpuFromCpu(std::string dstOffset, std::string srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    ensureInlineRegNotClobbered(getRegIndex(dstOffset), -1);
    outStartLine();
    out(dstOffset);
    out(" = ");
    out(srcOffset);
    out(";\r\n");
    if (!doneWithTmpReg) {
        outStartLine();
        outDynRegWrite(tmpReg, width);
        out(" = ");
        out(srcOffset);
        out(";\r\n");
    }
}

static void byteSwapReg32(DynReg reg) {
    ensureCalculatedReg(reg, DYN_32bit);
    outStartLine();
    outDynRegWrite(reg, DYN_32bit);
    out(" = ");
    out("((("); outDynRegRead(reg, DYN_32bit); out(" & 0xff000000) >> 24) | (("); outDynRegRead(reg, DYN_32bit); out(" & 0x00ff0000) >> 8) | (("); outDynRegRead(reg, DYN_32bit); out(" & 0x0000ff00) << 8) | (("); outDynRegRead(reg, DYN_32bit); out(" & 0x000000ff) << 24)); \r\n");
}

static void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    if (!doneWithSrcReg) {
        ensureCalculatedReg(src, srcWidth);
    }
    outStartLine();
    outDynRegWrite(dst, dstWidth);
    out(" = ");
    out("(U");
    out(getBitCount(dstWidth));
    out(")");
    out("(S");
    out(getBitCount(srcWidth));
    out(")");
    outDynRegRead(src, srcWidth);
    out(";\r\n");
    if (doneWithSrcReg) {
        inlineReg[src].reset();
    }
}

static void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    if (!doneWithSrcReg) {
        ensureCalculatedReg(src, srcWidth);
    }
    outStartLine();
    outDynRegWrite(dst, dstWidth);
    out(" = ");
    if (dstWidth < srcWidth) {
        out("(U");
        out(getBitCount(dstWidth));
        out(")");
    }
    outDynRegRead(src, srcWidth);
    out(";\r\n");
    if (doneWithSrcReg) {
        inlineReg[src].reset();
    }
}

static void movToReg(DynReg reg, DynWidth width, U32 imm) {
    inlineReg[reg].value = std::to_string(imm);
    inlineReg[reg].width = width;
}

static void movToCpuFromReg(std::string dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    if (!doneWithReg) {
        ensureCalculatedReg(reg, width);
    }
    
    ensureInlineRegNotClobbered(getRegIndex(dstOffset), doneWithReg?reg:-1);
    outStartLine();
    out(dstOffset);
    out(" = ");
    outDynRegRead(reg, width);
    out(";\r\n");
    if (doneWithReg) {
        inlineReg[reg].reset();
    }
}

static void movToCpu(std::string dstOffset, DynWidth dstWidth, U32 imm) {
    ensureInlineRegNotClobbered(getRegIndex(dstOffset), -1);
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
    inlineReg[reg].value = srcOffset;
    inlineReg[reg].width = width;
    int regIndex = getRegIndex(srcOffset);
    if (regIndex >= 0 && regIndex < 8) {
        inlineReg[reg].readReg[regIndex] = true;
    }
}

static void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg) {
    if (!doneWithAddressReg) {
        ensureCalculatedReg(addressReg, DYN_32bit);
    }
    if (!doneWithAddressReg) {
        ensureCalculatedReg(reg, width);
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
    outDynRegRead(addressReg, DYN_32bit);
    out(", ");
    outDynRegRead(reg, width);
    out(");\r\n");
    if (doneWithAddressReg) {
        inlineReg[DYN_ADDRESS].reset();
    }
    if (doneWithReg) {
        inlineReg[reg].reset();
    }
}

static void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg) {
    if (!doneWithAddressReg) {
        ensureCalculatedReg(addressReg, DYN_32bit);
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
    outDynRegRead(addressReg, DYN_32bit);
    out(", ");
    std::string i = std::to_string(imm);
    out(i);
    out(");\r\n");
    if (doneWithAddressReg) {
        inlineReg[DYN_ADDRESS].reset();
    }
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
            outDynRegWrite(reg, regWidth);
        },
        [reg, regWidth]() {
            outDynRegRead(reg, regWidth);
        },
        [rm, regWidth]() {
            outDynRegRead(rm, regWidth);
        }, regWidth);
    if (doneWithRmReg) {
        inlineReg[rm].reset();
    }
}

static void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg) {
    writeEaa(addressReg);
    movFromMem(regWidth, addressReg, false);
    instRegReg(inst, DYN_CALL_RESULT, rm, regWidth, true);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, doneWithRmReg);
}

static void instCPUReg(char inst, std::string dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    ensureInlineRegNotClobbered(getRegIndex(dstOffset), doneWithRmReg?rm:-1);
    outInst(inst,
        [dstOffset]() {
            out(dstOffset);
        },
        [dstOffset]() {
            out(dstOffset);
        },
            [rm, regWidth]() {
            outDynRegRead(rm, regWidth);
        }, regWidth);
    if (doneWithRmReg) {
        inlineReg[rm].reset();
    }
}

static void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    outInst(inst,
        [reg, regWidth]() {
            outDynRegWrite(reg, regWidth);
        },
        [reg, regWidth]() {
            outDynRegRead(reg, regWidth);
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
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
}

static void instCPUImm(char inst, std::string dstOffset, DynWidth regWidth, U32 imm) {
    ensureInlineRegNotClobbered(getRegIndex(dstOffset), -1);
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
    outDynRegWrite(reg, regWidth);
    out(" = ");
    if (op == "~") {
        out(op);
        out(" ");
    } else {
        out("0 - ");
    }
    outDynRegRead(reg, regWidth);
    out(";\r\n");
}

static void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg) {
    writeEaa(addressReg);
    movFromMem(regWidth, addressReg, false);
    instReg(inst, DYN_CALL_RESULT, regWidth);
    movToMemFromReg(addressReg, DYN_CALL_RESULT, regWidth, doneWithAddressReg, true);
}

static void instCPU(char inst, std::string dstOffset, DynWidth regWidth) {
    ensureInlineRegNotClobbered(getRegIndex(dstOffset), -1);
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
    if (!doneWithReg) {
        ensureCalculatedReg(reg, DYN_32bit);
    }
    outStartLine();
    out("if (");    
    if (condition == DYN_EQUALS_ZERO) {
        out("!");
    }
    outDynRegRead(reg, DYN_32bit);
    out("){\r\n");
    indentLevel++;
    if (doneWithReg) {
        inlineReg[reg].reset();
    }
}

static void startElse() {
    flushRegs();
    indentLevel--;
    outStartLine();
    out("} else {\r\n");
    indentLevel++;
}

static void endIf() {
    flushRegs();
    indentLevel--;
    outStartLine();
    out("}\r\n");
}

static void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    bool isSigned = condition == DYN_LESS_THAN_SIGNED || condition == DYN_LESS_THAN_EQUAL_SIGNED;
    if (!doneWithLeftReg) {
        ensureCalculatedReg(left, regWidth);
    }
    if (!doneWithRightReg && !isRightConst) {
        ensureCalculatedReg(right, regWidth);
    }
    outStartLine();
    outDynRegWrite(reg, dstWidth);
    out(" = ");
    if (isSigned) {
        out("(S");
        out(getBitCount(regWidth));
        out(")");
    }
    outDynRegRead(left, regWidth);
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
        outDynRegRead(right, regWidth);
    }
    out(";\r\n");
    if (doneWithLeftReg) {
        inlineReg[left].reset();
    }
    if (doneWithRightReg && !isRightConst) {
        inlineReg[right].reset();
    }
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
    movToMemFromImm(addressReg, regWidth, 0, false);
    startElse();
    movToMemFromImm(addressReg, regWidth, 1, true);
    endIf();
}


static void outArg(DYN_PTR_SIZE arg, DynCallParamType type, bool doneWithReg) {
    switch (type) {
    case DYN_PARAM_REG_8:
    {
        DynReg reg = (DynReg)arg;
        if (!doneWithReg) {
            ensureCalculatedReg(reg, DYN_8bit);
        }
        outDynRegRead((DynReg)arg, DYN_8bit);
        if (doneWithReg) {
            inlineReg[reg].reset();
        }
        break;
    }
    case DYN_PARAM_REG_16:
    {
        DynReg reg = (DynReg)arg;
        if (!doneWithReg) {
            ensureCalculatedReg(reg, DYN_16bit);
        }
        outDynRegRead((DynReg)arg, DYN_16bit);
        if (doneWithReg) {
            inlineReg[reg].reset();
        }
        break;
    }
    case DYN_PARAM_REG_32:
    {
        DynReg reg = (DynReg)arg;
        if (!doneWithReg) {
            ensureCalculatedReg(reg, DYN_32bit);
        }
        outDynRegRead(reg, DYN_32bit);
        if (doneWithReg) {
            inlineReg[reg].reset();
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
static void callHostFunction(std::string func, bool hasReturn, U32 argCount, DYN_PTR_SIZE arg1, DynCallParamType arg1Type, bool doneWithArg1, DYN_PTR_SIZE arg2, DynCallParamType arg2Type, bool doneWithArg2, DYN_PTR_SIZE arg3, DynCallParamType arg3Type, bool doneWithArg3, DYN_PTR_SIZE arg4, DynCallParamType arg4Type, bool doneWithArg4, DYN_PTR_SIZE arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    outStartLine();
    if (hasReturn) {
        outDynRegWrite(DYN_CALL_RESULT, DYN_32bit);
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
    out("return;\r\n");
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
static void writeNext1() {
    out("inline void nextBlock1(CPU* cpu) {\r\n");
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
    out("inline void nextBlock2(CPU* cpu) {\r\n");
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

    out("// ");
    std::string s = toHexString(cpu->eip.u32);
    out(s);
    out("\r\n");
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
        for (int i = 0; i < NUMBER_OF_TEMP_REGS; i++) {
            inlineReg[i].reset();
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
#ifdef __TEST
    if (DecodedBlock::currentBlock->runCount == 0) {
#else
    if (DecodedBlock::currentBlock->runCount == 50) {
#endif
        generateSource(cpu, op);
    }
    op->next->pfn(cpu, op->next);
}

static void outfp(FILE* fp, const char* str) {
    fwrite(str, strlen(str), 1, fp);
}

static void outfp(FILE* fp, const std::string& str) {
    fwrite(str.c_str(), str.size(), 1, fp);
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
        outfp(fp, std::to_string(i));
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
        outfp(fp, toHexString(functionArray[i]->crc));
        outfp(fp, ", ");
        outfp(fp, functionArray[i]->name.c_str());
        outfp(fp, ", data");
        outfp(fp, std::to_string(i));
        outfp(fp, ", ");
        outfp(fp, std::to_string(functionArray[i]->len));
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