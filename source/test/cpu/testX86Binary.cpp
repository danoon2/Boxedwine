/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#ifdef __TEST

#include "testX86Binary.h"
#include "testCPU.h"
#include "testX86Util.h"
#include "testAsmJit.h"

#define cpu (testContext().cpu)
#define memory (testContext().memory)
#define pushCode8 testPushCode8
#define newInstruction testNewInstruction
#define runTestCPU testRunCPU
#define failed testFail

namespace {

using namespace TestX86;

constexpr U32 REG_GUARD = 0xA55A0000;
constexpr U32 MEM_BASE = 0x10000;
constexpr U32 BINARY_FLAG_MASK = CF | PF | AF | ZF | SF | OF;

enum RegId {
    R_AX,
    R_CX,
    R_DX,
    R_BX,
    R_SP,
    R_BP,
    R_SI,
    R_DI
};

struct BinaryExpected {
    U32 result;
    U32 flags;
};

enum BinaryFlagMode {
    BINARY_FLAGS_CHECKED,
    BINARY_FLAGS_OVERWRITTEN
};

enum Group1ImmediateSize {
    GROUP1_IMM8,
    GROUP1_IMM_FULL,
    GROUP1_IMM_SIGN_EXTEND_8
};

const char* opName(TestBinaryOp op) {
    if (op == TEST_BINARY_ADD) {
        return "add";
    }
    if (op == TEST_BINARY_ADC) {
        return "adc";
    }
    if (op == TEST_BINARY_SBB) {
        return "sbb";
    }
    if (op == TEST_BINARY_SUB) {
        return "sub";
    }
    if (op == TEST_BINARY_CMP) {
        return "cmp";
    }
    if (op == TEST_BINARY_AND) {
        return "and";
    }
    if (op == TEST_BINARY_XOR) {
        return "xor";
    }
    if (op == TEST_BINARY_TEST) {
        return "test";
    }
    return "or";
}

U32 calculatedBinaryResult(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags) {
    U32 mask = widthMask(width);
    if (op == TEST_BINARY_ADD) {
        return (dst + src) & mask;
    }
    if (op == TEST_BINARY_ADC) {
        return (dst + src + ((initialFlags & CF) ? 1 : 0)) & mask;
    }
    if (op == TEST_BINARY_AND) {
        return (dst & src) & mask;
    }
    if (op == TEST_BINARY_XOR) {
        return (dst ^ src) & mask;
    }
    if (op == TEST_BINARY_OR) {
        return (dst | src) & mask;
    }
    if (op == TEST_BINARY_TEST) {
        return (dst & src) & mask;
    }
    if (op == TEST_BINARY_SBB) {
        return (dst - src - ((initialFlags & CF) ? 1 : 0)) & mask;
    }
    return (dst - src) & mask;
}

bool writesDestination(TestBinaryOp op) {
    return op != TEST_BINARY_CMP && op != TEST_BINARY_TEST;
}

bool supportsLockPrefix(TestBinaryOp op) {
    return op != TEST_BINARY_CMP && op != TEST_BINARY_TEST;
}

BinaryExpected calculatedBinary(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags) {
    U32 mask = widthMask(width);
    U32 sign = signBit(width);
    U32 result;
    U32 flags = 0;

    if (op == TEST_BINARY_ADD || op == TEST_BINARY_ADC) {
        U32 carry = op == TEST_BINARY_ADC && (initialFlags & CF) ? 1 : 0;
        result = calculatedBinaryResult(op, dst, src, width, initialFlags);
        if (width == 32) {
            if ((U64)dst + (U64)src + carry > 0xffffffffULL) {
                flags |= CF;
            }
        } else if (dst + src + carry > mask) {
            flags |= CF;
        }
        if (((dst ^ result) & (src ^ result) & sign) != 0) {
            flags |= OF;
        }
    } else if (op == TEST_BINARY_SBB) {
        U32 carry = (initialFlags & CF) ? 1 : 0;
        U64 subtrahend = (U64)(src & mask) + carry;
        dst &= mask;
        src &= mask;
        result = calculatedBinaryResult(op, dst, src, width, initialFlags);
        if ((U64)dst < subtrahend) {
            flags |= CF;
        }
        if (((dst ^ src) & (dst ^ result) & sign) != 0) {
            flags |= OF;
        }
    } else if (op == TEST_BINARY_SUB || op == TEST_BINARY_CMP) {
        dst &= mask;
        src &= mask;
        result = calculatedBinaryResult(op, dst, src, width, initialFlags);
        if (dst < src) {
            flags |= CF;
        }
        if (((dst ^ src) & (dst ^ result) & sign) != 0) {
            flags |= OF;
        }
    } else {
        dst &= mask;
        src &= mask;
        result = calculatedBinaryResult(op, dst, src, width, initialFlags);
    }

    if (op != TEST_BINARY_AND && op != TEST_BINARY_XOR && op != TEST_BINARY_OR && op != TEST_BINARY_TEST && ((dst ^ src ^ result) & 0x10) != 0) {
        flags |= AF;
    }
    if (result == 0) {
        flags |= ZF;
    }
    if ((result & sign) != 0) {
        flags |= SF;
    }

    U8 low = (U8)result;
    low ^= low >> 4;
    low &= 0x0f;
    if (((0x6996 >> low) & 1) == 0) {
        flags |= PF;
    }
    return {result, flags};
}

#if defined(_MSC_VER) && defined(_M_IX86)
#define TEST_BINARY_HAS_HARDWARE_ORACLE 1

BinaryExpected hardwareBinary(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags) {
    U32 result = 0;
    U32 flags = 0;

    if (op == TEST_BINARY_ADD) {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                add al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                add ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                add eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    } else if (op == TEST_BINARY_ADC) {
        if (width == 8) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    stc
                    adc al, cl
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    clc
                    adc al, cl
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else if (width == 16) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    stc
                    adc ax, cx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    clc
                    adc ax, cx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else {
            if (initialFlags & CF) {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    stc
                    adc eax, ecx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    clc
                    adc eax, ecx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        }
    } else if (op == TEST_BINARY_SBB) {
        if (width == 8) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    stc
                    sbb al, cl
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    clc
                    sbb al, cl
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else if (width == 16) {
            if (initialFlags & CF) {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    stc
                    sbb ax, cx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    clc
                    sbb ax, cx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        } else {
            if (initialFlags & CF) {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    stc
                    sbb eax, ecx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            } else {
                __asm {
                    mov eax, dst
                    mov ecx, src
                    clc
                    sbb eax, ecx
                    pushfd
                    pop edx
                    mov result, eax
                    mov flags, edx
                }
            }
        }
    } else if (op == TEST_BINARY_SUB) {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                sub al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                sub ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                sub eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    } else if (op == TEST_BINARY_CMP) {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                cmp al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                cmp ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                cmp eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    } else if (op == TEST_BINARY_AND) {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                and al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                and ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                and eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    } else if (op == TEST_BINARY_XOR) {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                xor al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                xor ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                xor eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    } else if (op == TEST_BINARY_OR) {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                or al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                or ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                or eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    } else {
        if (width == 8) {
            __asm {
                mov eax, dst
                mov ecx, src
                test al, cl
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else if (width == 16) {
            __asm {
                mov eax, dst
                mov ecx, src
                test ax, cx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        } else {
            __asm {
                mov eax, dst
                mov ecx, src
                test eax, ecx
                pushfd
                pop edx
                mov result, eax
                mov flags, edx
            }
        }
    }

    return {result & widthMask(width), flags & BINARY_FLAG_MASK};
}
#else
#define TEST_BINARY_HAS_HARDWARE_ORACLE 0
#endif

BinaryExpected expectedBinary(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags) {
#if TEST_BINARY_HAS_HARDWARE_ORACLE
    BinaryExpected hardware = hardwareBinary(op, dst, src, width, initialFlags);
    BinaryExpected calculated = calculatedBinary(op, dst, src, width, initialFlags);
    if ((writesDestination(op) && hardware.result != calculated.result) || (hardware.flags & BINARY_FLAG_MASK) != (calculated.flags & BINARY_FLAG_MASK)) {
        failed("hardware %s oracle mismatch", opName(op));
    }
    return hardware;
#else
    return calculatedBinary(op, dst, src, width, initialFlags);
#endif
}

U32 binaryResult(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags) {
    return expectedBinary(op, dst, src, width, initialFlags).result;
}

U32 expectedFlags(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags) {
    return expectedBinary(op, dst, src, width, initialFlags).flags;
}

void pushGeneratedCode(const asmjit::CodeHolder& code) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    for (size_t i = 0; i < buffer.size(); ++i) {
        pushCode8(buffer.data()[i]);
    }
}

void pushGeneratedCodeWithOpcode(const asmjit::CodeHolder& code, U8 opcode) {
    const asmjit::CodeBuffer& buffer = code.text_section()->buffer();
    bool replaced = false;
    for (size_t i = 0; i < buffer.size(); ++i) {
        U8 value = buffer.data()[i];
        if (!replaced && (value == 0x80 || value == 0x81 || value == 0x83)) {
            value = opcode;
            replaced = true;
        }
        pushCode8(value);
    }
    if (!replaced) {
        failed("asmjit %02x opcode not found", opcode);
    }
}

void initCode(asmjit::CodeHolder& code) {
    asmjit::Environment env(asmjit::Arch::kX86);
    if (code.init(env) != asmjit::Error::kOk) {
        failed("asmjit code init failed");
    }
}

int group1Extension(TestBinaryOp op) {
    if (op == TEST_BINARY_ADD) {
        return 0;
    }
    if (op == TEST_BINARY_OR) {
        return 1;
    }
    if (op == TEST_BINARY_ADC) {
        return 2;
    }
    if (op == TEST_BINARY_SBB) {
        return 3;
    }
    if (op == TEST_BINARY_AND) {
        return 4;
    }
    if (op == TEST_BINARY_SUB) {
        return 5;
    }
    if (op == TEST_BINARY_XOR) {
        return 6;
    }
    return 7;
}

Group1ImmediateSize group1ImmediateSize(int width, bool signExtend8) {
    if (width == 8) {
        return GROUP1_IMM8;
    }
    if (signExtend8) {
        return GROUP1_IMM_SIGN_EXTEND_8;
    }
    return GROUP1_IMM_FULL;
}

TestBinaryCase normalizedImmediateCase(const TestBinaryCase& data, int width, Group1ImmediateSize immediateSize) {
    TestBinaryCase result = data;
    if (immediateSize == GROUP1_IMM_SIGN_EXTEND_8) {
        result.src = (U32)(S32)(S8)data.src & widthMask(width);
    } else {
        result.src &= widthMask(immediateSize == GROUP1_IMM8 ? 8 : width);
    }
    return result;
}

U32 encodedImmediateValue(const TestBinaryCase& data, Group1ImmediateSize immediateSize) {
    if (immediateSize == GROUP1_IMM_FULL) {
        return data.src;
    }
    return data.src & 0xff;
}

void emitImmediateBytes(int width, U32 value, Group1ImmediateSize immediateSize) {
    if (immediateSize != GROUP1_IMM_FULL) {
        pushCode8(value & 0xff);
    } else if (width == 16) {
        pushCode8(value & 0xff);
        pushCode8((value >> 8) & 0xff);
    } else {
        pushCode8(value & 0xff);
        pushCode8((value >> 8) & 0xff);
        pushCode8((value >> 16) & 0xff);
        pushCode8((value >> 24) & 0xff);
    }
}

void emitBinaryGroup1Immediate(TestBinaryOp op, int dstReg, int width, U8 opcode, Group1ImmediateSize immediateSize, U32 value) {
    if (width == 16) {
        pushCode8(0x66);
    }
    pushCode8(opcode);
    pushCode8(0xc0 | (group1Extension(op) << 3) | dstReg);
    emitImmediateBytes(width, value, immediateSize);
}

void emitBinary(TestBinaryOp op, asmjit::x86::Gp dst, asmjit::x86::Gp src) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = asmjit::Error::kOk;
    if (op == TEST_BINARY_ADD) {
        err = a.add(dst, src);
    } else if (op == TEST_BINARY_ADC) {
        err = a.adc(dst, src);
    } else if (op == TEST_BINARY_SBB) {
        err = a.sbb(dst, src);
    } else if (op == TEST_BINARY_SUB) {
        err = a.sub(dst, src);
    } else if (op == TEST_BINARY_CMP) {
        err = a.cmp(dst, src);
    } else if (op == TEST_BINARY_AND) {
        err = a.and_(dst, src);
    } else if (op == TEST_BINARY_XOR) {
        err = a.xor_(dst, src);
    } else if (op == TEST_BINARY_TEST) {
        err = a.test(dst, src);
    } else {
        err = a.or_(dst, src);
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s reg, reg failed", opName(op));
    }
    pushGeneratedCode(code);
}

void emitBinaryReverse(TestBinaryOp op, asmjit::x86::Gp dst, asmjit::x86::Gp src) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = asmjit::Error::kOk;
    if (op == TEST_BINARY_ADD) {
        err = a.mod_mr().add(dst, src);
    } else if (op == TEST_BINARY_ADC) {
        err = a.mod_mr().adc(dst, src);
    } else if (op == TEST_BINARY_SBB) {
        err = a.mod_mr().sbb(dst, src);
    } else if (op == TEST_BINARY_SUB) {
        err = a.mod_mr().sub(dst, src);
    } else if (op == TEST_BINARY_CMP) {
        err = a.mod_mr().cmp(dst, src);
    } else if (op == TEST_BINARY_AND) {
        err = a.mod_mr().and_(dst, src);
    } else if (op == TEST_BINARY_XOR) {
        err = a.mod_mr().xor_(dst, src);
    } else if (op == TEST_BINARY_TEST) {
        err = a.mod_mr().test(dst, src);
    } else {
        err = a.mod_mr().or_(dst, src);
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s reverse reg, reg failed", opName(op));
    }
    pushGeneratedCode(code);
}

void emitBinary(TestBinaryOp op, const asmjit::x86::Mem& dst, asmjit::x86::Gp src, bool lockPrefix) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (lockPrefix) {
        a.lock();
    }
    asmjit::Error err = asmjit::Error::kOk;
    if (op == TEST_BINARY_ADD) {
        err = a.add(dst, src);
    } else if (op == TEST_BINARY_ADC) {
        err = a.adc(dst, src);
    } else if (op == TEST_BINARY_SBB) {
        err = a.sbb(dst, src);
    } else if (op == TEST_BINARY_SUB) {
        err = a.sub(dst, src);
    } else if (op == TEST_BINARY_CMP) {
        err = a.cmp(dst, src);
    } else if (op == TEST_BINARY_AND) {
        err = a.and_(dst, src);
    } else if (op == TEST_BINARY_XOR) {
        err = a.xor_(dst, src);
    } else if (op == TEST_BINARY_TEST) {
        err = a.test(dst, src);
    } else {
        err = a.or_(dst, src);
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s mem, reg failed", opName(op));
    }
    pushGeneratedCode(code);
}

void emitBinary(TestBinaryOp op, asmjit::x86::Gp dst, const asmjit::x86::Mem& src) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = asmjit::Error::kOk;
    if (op == TEST_BINARY_ADD) {
        err = a.add(dst, src);
    } else if (op == TEST_BINARY_ADC) {
        err = a.adc(dst, src);
    } else if (op == TEST_BINARY_SBB) {
        err = a.sbb(dst, src);
    } else if (op == TEST_BINARY_SUB) {
        err = a.sub(dst, src);
    } else if (op == TEST_BINARY_CMP) {
        err = a.cmp(dst, src);
    } else if (op == TEST_BINARY_AND) {
        err = a.and_(dst, src);
    } else if (op == TEST_BINARY_XOR) {
        err = a.xor_(dst, src);
    } else if (op == TEST_BINARY_TEST) {
        failed("asmjit test reg, mem unsupported");
        return;
    } else {
        err = a.or_(dst, src);
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s reg, mem failed", opName(op));
    }
    pushGeneratedCode(code);
}

void emitBinaryAccumulatorImmediate(TestBinaryOp op, int width, U32 value) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    asmjit::Error err = asmjit::Error::kOk;
    if (op == TEST_BINARY_ADD) {
        err = a.add(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_ADC) {
        err = a.adc(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_SBB) {
        err = a.sbb(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_SUB) {
        err = a.sub(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_CMP) {
        err = a.cmp(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_AND) {
        err = a.and_(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_XOR) {
        err = a.xor_(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else if (op == TEST_BINARY_TEST) {
        err = a.test(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    } else {
        err = a.or_(regForWidth(R_AX, width), asmjit::Imm(value & widthMask(width)));
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s accumulator, immediate failed", opName(op));
    }
    pushGeneratedCode(code);
}

void emitBinaryGroup1Immediate(TestBinaryOp op, const asmjit::x86::Mem& dst, int width, U8 opcode, Group1ImmediateSize immediateSize, U32 value, bool lockPrefix) {
    asmjit::CodeHolder code;
    initCode(code);
    asmjit::x86::Assembler a(&code);
    if (lockPrefix) {
        a.lock();
    }

    asmjit::Imm imm(immediateSize == GROUP1_IMM_SIGN_EXTEND_8 ? (S8)value : (S32)(value & widthMask(immediateSize == GROUP1_IMM8 ? 8 : width)));
    asmjit::Error err = asmjit::Error::kOk;
    if (op == TEST_BINARY_ADD) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().add(dst, imm) : a.short_().add(dst, imm);
    } else if (op == TEST_BINARY_ADC) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().adc(dst, imm) : a.short_().adc(dst, imm);
    } else if (op == TEST_BINARY_SBB) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().sbb(dst, imm) : a.short_().sbb(dst, imm);
    } else if (op == TEST_BINARY_SUB) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().sub(dst, imm) : a.short_().sub(dst, imm);
    } else if (op == TEST_BINARY_CMP) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().cmp(dst, imm) : a.short_().cmp(dst, imm);
    } else if (op == TEST_BINARY_AND) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().and_(dst, imm) : a.short_().and_(dst, imm);
    } else if (op == TEST_BINARY_XOR) {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().xor_(dst, imm) : a.short_().xor_(dst, imm);
    } else {
        err = immediateSize == GROUP1_IMM_FULL ? a.long_().or_(dst, imm) : a.short_().or_(dst, imm);
    }
    if (err != asmjit::Error::kOk) {
        failed("asmjit %s mem, immediate failed", opName(op));
    }
    pushGeneratedCodeWithOpcode(code, opcode);
}

void beginGeneratedInstruction(U32 flags) {
    newInstruction(flags);
    cpu->big = true;
}

void overwriteFlagsIfNeeded(BinaryFlagMode flagMode) {
    if (flagMode == BINARY_FLAGS_OVERWRITTEN) {
        pushCode8(0x39);
        pushCode8(0xc0); // cmp eax, eax
    }
}

bool shouldVerifyFlags(BinaryFlagMode flagMode) {
    return flagMode == BINARY_FLAGS_CHECKED;
}

void verifyFlags(TestBinaryOp op, U32 dst, U32 src, int width, U32 initialFlags, const char* name) {
    U32 expected = expectedFlags(op, dst, src, width, initialFlags);
    U32 actual = 0;

    if (cpu->getCF()) actual |= CF;
    if (cpu->getPF()) actual |= PF;
    if (cpu->getAF()) actual |= AF;
    if (cpu->getZF()) actual |= ZF;
    if (cpu->getSF()) actual |= SF;
    if (cpu->getOF()) actual |= OF;

    if ((actual & BINARY_FLAG_MASK) != (expected & BINARY_FLAG_MASK)) {
        failed("%s flags", name);
    }
}

bool registerOverlapsAddressReg(int reg, int width, int addressReg) {
    if (width == 8) {
        return physicalReg8(reg) == addressReg;
    }
    return reg == addressReg;
}

void setRegisterInputs(TestBinaryOp op, int dst, int src, int width, const TestBinaryCase& data, U32* expectedRegs, U32& actualDst, U32& actualSrc) {
    for (int i = 0; i < 8; ++i) {
        cpu->reg[i].u32 = REG_GUARD | (0x0100 + i);
        expectedRegs[i] = cpu->reg[i].u32;
    }

    applyRegValue(expectedRegs, dst, width, data.dst);
    setRegValue(cpu, dst, width, data.dst);
    if (dst != src) {
        applyRegValue(expectedRegs, src, width, data.src);
        setRegValue(cpu, src, width, data.src);
    }

    actualDst = getRegValue(cpu, dst, width);
    actualSrc = getRegValue(cpu, src, width);
    if (writesDestination(op)) {
        applyRegValue(expectedRegs, dst, width, binaryResult(op, actualDst, actualSrc, width, data.initialFlags));
    }
}

U32 segmentBaseForAddressReg(int baseReg) {
    return (baseReg == R_SP || baseReg == R_BP) ? cpu->seg[SS].address : cpu->seg[DS].address;
}

void initAddressRegisters(U32* regs) {
    regs[R_AX] = MEM_BASE + 0x0100;
    regs[R_CX] = MEM_BASE + 0x0200;
    regs[R_DX] = MEM_BASE + 0x0300;
    regs[R_BX] = MEM_BASE + 0x0400;
    regs[R_SP] = MEM_BASE + 0x0500;
    regs[R_BP] = MEM_BASE + 0x0600;
    regs[R_SI] = MEM_BASE + 0x0700;
    regs[R_DI] = MEM_BASE + 0x0800;
}

U32 baseMemoryOffset(int base, int width, int valueReg, U32 value) {
    if (width == 8) {
        return 0x2022 + base * 0x80;
    }
    if (width == 16 && valueReg == base && (value & 0xffff) >= 0xfe00) {
        return 0x2000 + base * 0x80;
    }
    return MEM_BASE + 0x1000 + base * 0x80;
}

void prepareMemTarget(U32 address, U32 dst, int width) {
    if (width == 8) {
        memory->writeb(address - 1, 0x11);
        memory->writeb(address, dst);
        memory->writeb(address + 1, 0x33);
        memory->writeb(address + 2, 0x44);
        memory->writeb(address + 3, 0x55);
    } else if (width == 16) {
        memory->writew(address - 2, 0x1111);
        memory->writew(address, dst);
        memory->writew(address + 2, 0x3333);
    } else {
        memory->writew(address - 2, 0x1111);
        memory->writed(address, dst);
        memory->writew(address + 4, 0x3333);
    }
}

void verifyMemTarget(U32 address, U32 expected, int width, const char* name) {
    if (width == 8) {
        if (memory->readb(address) != expected) {
            failed("%s memory value", name);
        }
        if (memory->readb(address - 1) != 0x11 || memory->readb(address + 1) != 0x33 || memory->readb(address + 2) != 0x44 || memory->readb(address + 3) != 0x55) {
            failed("%s memory guard", name);
        }
    } else if (width == 16) {
        if (memory->readw(address) != expected) {
            failed("%s memory value", name);
        }
        if (memory->readw(address - 2) != 0x1111 || memory->readw(address + 2) != 0x3333) {
            failed("%s memory guard", name);
        }
    } else {
        if (memory->readd(address) != expected) {
            failed("%s memory value", name);
        }
        if (memory->readw(address - 2) != 0x1111 || memory->readw(address + 4) != 0x3333) {
            failed("%s memory guard", name);
        }
    }
}

void runPreparedMemoryCase(TestBinaryOp op, int srcReg, int width, U32 linearAddress, const TestBinaryCase& data, BinaryFlagMode flagMode, const char* name) {
    U32 expectedRegs[8];
    U32 actualSrc = getRegValue(cpu, srcReg, width);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }

    prepareMemTarget(linearAddress, data.dst, width);
    runTestCPU();
    U32 expectedMemory = writesDestination(op) ? binaryResult(op, data.dst, actualSrc, width, data.initialFlags) : data.dst;
    verifyMemTarget(linearAddress, expectedMemory, width, name);
    verifyRegisters(cpu, expectedRegs, name);
    if (shouldVerifyFlags(flagMode)) {
        verifyFlags(op, data.dst, actualSrc, width, data.initialFlags, name);
    }
}

void runPreparedMemorySourceCase(TestBinaryOp op, int dstReg, int width, U32 linearAddress, const TestBinaryCase& data, BinaryFlagMode flagMode, const char* name) {
    U32 expectedRegs[8];
    U32 actualDst = getRegValue(cpu, dstReg, width);

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }
    if (writesDestination(op)) {
        applyRegValue(expectedRegs, dstReg, width, binaryResult(op, actualDst, data.src, width, data.initialFlags));
    }

    prepareMemTarget(linearAddress, data.src, width);
    runTestCPU();
    verifyMemTarget(linearAddress, data.src, width, name);
    verifyRegisters(cpu, expectedRegs, name);
    if (shouldVerifyFlags(flagMode)) {
        verifyFlags(op, actualDst, data.src, width, data.initialFlags, name);
    }
}

void runPreparedMemoryImmediateCase(TestBinaryOp op, int width, U32 linearAddress, const TestBinaryCase& data, BinaryFlagMode flagMode, const char* name) {
    U32 expectedRegs[8];

    for (int i = 0; i < 8; ++i) {
        expectedRegs[i] = cpu->reg[i].u32;
    }

    prepareMemTarget(linearAddress, data.dst, width);
    runTestCPU();
    U32 expectedMemory = writesDestination(op) ? binaryResult(op, data.dst, data.src, width, data.initialFlags) : data.dst;
    verifyMemTarget(linearAddress, expectedMemory, width, name);
    verifyRegisters(cpu, expectedRegs, name);
    if (shouldVerifyFlags(flagMode)) {
        verifyFlags(op, data.dst, data.src, width, data.initialFlags, name);
    }
}

void runBaseMemoryCases(TestBinaryOp op, int srcReg, int width, const TestBinaryCase& data, bool lockPrefix, BinaryFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        U32 baseOffset = baseMemoryOffset(base, width, srcReg, data.src);
        initAddressRegisters(regs);
        regs[base] = baseOffset;
        if (width != 32 || srcReg != base) {
            applyRegValue(regs, srcReg, width, data.src);
        }

        if (base != R_BP && testRunMemoryBaseDisplacement(base, 0)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinary(op, memPtr(reg32(base), 0, width), regForWidth(srcReg, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryCase(op, srcReg, width, segmentBaseForAddressReg(base) + regs[base], data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinary(op, memPtr(reg32(base), 0x11, width), regForWidth(srcReg, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryCase(op, srcReg, width, segmentBaseForAddressReg(base) + regs[base] + 0x11, data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinary(op, memPtr(reg32(base), 0x123, width), regForWidth(srcReg, width), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryCase(op, srcReg, width, segmentBaseForAddressReg(base) + regs[base] + 0x123, data, flagMode, name);
        }
    }
}

void runBaseMemorySourceCases(TestBinaryOp op, int dstReg, int width, const TestBinaryCase& data, BinaryFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        U32 baseOffset = baseMemoryOffset(base, width, dstReg, data.dst);
        initAddressRegisters(regs);
        regs[base] = baseOffset;
        if (!registerOverlapsAddressReg(dstReg, width, base)) {
            applyRegValue(regs, dstReg, width, data.dst);
        }

        if (base != R_BP && testRunMemoryBaseDisplacement(base, 0)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinary(op, regForWidth(dstReg, width), memPtr(reg32(base), 0, width));
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemorySourceCase(op, dstReg, width, segmentBaseForAddressReg(base) + cpu->reg[base].u32, data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinary(op, regForWidth(dstReg, width), memPtr(reg32(base), 0x11, width));
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemorySourceCase(op, dstReg, width, segmentBaseForAddressReg(base) + cpu->reg[base].u32 + 0x11, data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinary(op, regForWidth(dstReg, width), memPtr(reg32(base), 0x123, width));
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemorySourceCase(op, dstReg, width, segmentBaseForAddressReg(base) + cpu->reg[base].u32 + 0x123, data, flagMode, name);
        }
    }
}

void runAbsoluteMemoryCases(TestBinaryOp op, int srcReg, int width, const TestBinaryCase& data, bool lockPrefix, BinaryFlagMode flagMode, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);
    applyRegValue(regs, srcReg, width, data.src);

    beginGeneratedInstruction(data.initialFlags);
    emitBinary(op, memPtr(offset, width), regForWidth(srcReg, width), lockPrefix);
    overwriteFlagsIfNeeded(flagMode);
    writeRegs(cpu, regs);
    runPreparedMemoryCase(op, srcReg, width, cpu->seg[DS].address + offset, data, flagMode, name);
}

void runAbsoluteMemorySourceCases(TestBinaryOp op, int dstReg, int width, const TestBinaryCase& data, BinaryFlagMode flagMode, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);
    applyRegValue(regs, dstReg, width, data.dst);

    beginGeneratedInstruction(data.initialFlags);
    emitBinary(op, regForWidth(dstReg, width), memPtr(offset, width));
    overwriteFlagsIfNeeded(flagMode);
    writeRegs(cpu, regs);
    runPreparedMemorySourceCase(op, dstReg, width, cpu->seg[DS].address + offset, data, flagMode, name);
}

void runSibMemoryCases(TestBinaryOp op, int srcReg, int width, const TestBinaryCase& data, bool lockPrefix, BinaryFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                if (!testRunMemorySib(base, index, shift)) {
                    continue;
                }
                U32 regs[8];
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;
                if (width != 32 || (srcReg != base && srcReg != index)) {
                    applyRegValue(regs, srcReg, width, data.src);
                }

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                beginGeneratedInstruction(data.initialFlags);
                emitBinary(op, memPtr(reg32(base), reg32(index), shift, disp, width), regForWidth(srcReg, width), lockPrefix);
                overwriteFlagsIfNeeded(flagMode);
                writeRegs(cpu, regs);
                runPreparedMemoryCase(op, srcReg, width, segmentBaseForAddressReg(base) + targetOffset, data, flagMode, name);
            }
        }
    }
}

void runSibMemorySourceCases(TestBinaryOp op, int dstReg, int width, const TestBinaryCase& data, BinaryFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                if (!testRunMemorySib(base, index, shift)) {
                    continue;
                }
                U32 regs[8];
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;
                if (!registerOverlapsAddressReg(dstReg, width, base) && !registerOverlapsAddressReg(dstReg, width, index)) {
                    applyRegValue(regs, dstReg, width, data.dst);
                }

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                beginGeneratedInstruction(data.initialFlags);
                emitBinary(op, regForWidth(dstReg, width), memPtr(reg32(base), reg32(index), shift, disp, width));
                overwriteFlagsIfNeeded(flagMode);
                writeRegs(cpu, regs);
                runPreparedMemorySourceCase(op, dstReg, width, segmentBaseForAddressReg(base) + targetOffset, data, flagMode, name);
            }
        }
    }
}

void runBaseMemoryImmediateCases(TestBinaryOp op, int width, const TestBinaryCase& data, U8 opcode, Group1ImmediateSize immediateSize, bool lockPrefix, BinaryFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        if (!testRunMemoryBase(base)) {
            continue;
        }
        U32 regs[8];
        initAddressRegisters(regs);
        regs[base] = baseMemoryOffset(base, width, R_AX, 0);

        if (base != R_BP && testRunMemoryBaseDisplacement(base, 0)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinaryGroup1Immediate(op, memPtr(reg32(base), 0, width), width, opcode, immediateSize, encodedImmediateValue(data, immediateSize), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryImmediateCase(op, width, segmentBaseForAddressReg(base) + regs[base], data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 1)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinaryGroup1Immediate(op, memPtr(reg32(base), 0x11, width), width, opcode, immediateSize, encodedImmediateValue(data, immediateSize), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryImmediateCase(op, width, segmentBaseForAddressReg(base) + regs[base] + 0x11, data, flagMode, name);
        }

        if (testRunMemoryBaseDisplacement(base, 2)) {
            beginGeneratedInstruction(data.initialFlags);
            emitBinaryGroup1Immediate(op, memPtr(reg32(base), 0x123, width), width, opcode, immediateSize, encodedImmediateValue(data, immediateSize), lockPrefix);
            overwriteFlagsIfNeeded(flagMode);
            writeRegs(cpu, regs);
            runPreparedMemoryImmediateCase(op, width, segmentBaseForAddressReg(base) + regs[base] + 0x123, data, flagMode, name);
        }
    }
}

void runAbsoluteMemoryImmediateCases(TestBinaryOp op, int width, const TestBinaryCase& data, U8 opcode, Group1ImmediateSize immediateSize, bool lockPrefix, BinaryFlagMode flagMode, const char* name) {
    U32 regs[8];
    U32 offset = MEM_BASE + 0x3000;
    initAddressRegisters(regs);

    beginGeneratedInstruction(data.initialFlags);
    emitBinaryGroup1Immediate(op, memPtr(offset, width), width, opcode, immediateSize, encodedImmediateValue(data, immediateSize), lockPrefix);
    overwriteFlagsIfNeeded(flagMode);
    writeRegs(cpu, regs);
    runPreparedMemoryImmediateCase(op, width, cpu->seg[DS].address + offset, data, flagMode, name);
}

void runSibMemoryImmediateCases(TestBinaryOp op, int width, const TestBinaryCase& data, U8 opcode, Group1ImmediateSize immediateSize, bool lockPrefix, BinaryFlagMode flagMode, const char* name) {
    for (int base = 0; base < 8; ++base) {
        for (int index = 0; index < 8; ++index) {
            if (index == R_SP) {
                continue;
            }
            for (int shift = 0; shift < 4; ++shift) {
                if (!testRunMemorySib(base, index, shift)) {
                    continue;
                }
                U32 regs[8];
                U32 targetOffset = MEM_BASE + 0x7000 + base * 0x200 + index * 0x20 + shift * 4;
                initAddressRegisters(regs);
                regs[base] = MEM_BASE + 0x0100 + base * 0x40;
                regs[index] = 3;

                S32 disp = (S32)(targetOffset - regs[base] - (regs[index] << shift));
                beginGeneratedInstruction(data.initialFlags);
                emitBinaryGroup1Immediate(op, memPtr(reg32(base), reg32(index), shift, disp, width), width, opcode, immediateSize, encodedImmediateValue(data, immediateSize), lockPrefix);
                overwriteFlagsIfNeeded(flagMode);
                writeRegs(cpu, regs);
                runPreparedMemoryImmediateCase(op, width, segmentBaseForAddressReg(base) + targetOffset, data, flagMode, name);
            }
        }
    }
}

} // namespace

void testRunBinaryRegister(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name, bool reverseEncoding) {
    for (size_t i = 0; i < caseCount; ++i) {
        const TestBinaryCase& data = cases[i];
        for (int flagMode = BINARY_FLAGS_CHECKED; flagMode <= BINARY_FLAGS_OVERWRITTEN; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                for (int src = 0; src < 8; ++src) {
                    if (!testRunRegisterPair(dst, src)) {
                        continue;
                    }
                    U32 expectedRegs[8];
                    U32 actualDst;
                    U32 actualSrc;

                    beginGeneratedInstruction(data.initialFlags);
                    if (reverseEncoding) {
                        emitBinaryReverse(op, regForWidth(dst, width), regForWidth(src, width));
                    } else {
                        emitBinary(op, regForWidth(dst, width), regForWidth(src, width));
                    }
                    overwriteFlagsIfNeeded((BinaryFlagMode)flagMode);
                    setRegisterInputs(op, dst, src, width, data, expectedRegs, actualDst, actualSrc);
                    runTestCPU();
                    verifyRegisters(cpu, expectedRegs, name);
                    if (shouldVerifyFlags((BinaryFlagMode)flagMode)) {
                        verifyFlags(op, actualDst, actualSrc, width, data.initialFlags, name);
                    }
                }
            }
        }
    }
}

void testRunBinaryMemoryDestination(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name) {
    for (size_t i = 0; i < caseCount; ++i) {
        const TestBinaryCase& data = cases[i];
        for (int flagMode = BINARY_FLAGS_CHECKED; flagMode <= BINARY_FLAGS_OVERWRITTEN; ++flagMode) {
            for (int src = 0; src < 8; ++src) {
                if (!testRunRegister(src)) {
                    continue;
                }
                int lockPrefixCount = supportsLockPrefix(op) ? 2 : 1;
                for (int lockPrefix = 0; lockPrefix < lockPrefixCount; ++lockPrefix) {
                    runBaseMemoryCases(op, src, width, data, lockPrefix != 0, (BinaryFlagMode)flagMode, name);
                    runAbsoluteMemoryCases(op, src, width, data, lockPrefix != 0, (BinaryFlagMode)flagMode, name);
                    runSibMemoryCases(op, src, width, data, lockPrefix != 0, (BinaryFlagMode)flagMode, name);
                }
            }
        }
    }
}

void testRunBinaryMemorySource(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name) {
    for (size_t i = 0; i < caseCount; ++i) {
        const TestBinaryCase& data = cases[i];
        for (int flagMode = BINARY_FLAGS_CHECKED; flagMode <= BINARY_FLAGS_OVERWRITTEN; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                if (!testRunRegister(dst)) {
                    continue;
                }
                runBaseMemorySourceCases(op, dst, width, data, (BinaryFlagMode)flagMode, name);
                runAbsoluteMemorySourceCases(op, dst, width, data, (BinaryFlagMode)flagMode, name);
                runSibMemorySourceCases(op, dst, width, data, (BinaryFlagMode)flagMode, name);
            }
        }
    }
}

void testRunBinaryAccumulatorImmediate(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name) {
    for (size_t i = 0; i < caseCount; ++i) {
        const TestBinaryCase& data = cases[i];
        for (int flagMode = BINARY_FLAGS_CHECKED; flagMode <= BINARY_FLAGS_OVERWRITTEN; ++flagMode) {
            U32 expectedRegs[8];
            U32 actualDst;

            beginGeneratedInstruction(data.initialFlags);
            emitBinaryAccumulatorImmediate(op, width, data.src);
            overwriteFlagsIfNeeded((BinaryFlagMode)flagMode);

            for (int reg = 0; reg < 8; ++reg) {
                cpu->reg[reg].u32 = REG_GUARD | (0x0100 + reg);
                expectedRegs[reg] = cpu->reg[reg].u32;
            }

            applyRegValue(expectedRegs, R_AX, width, data.dst);
            setRegValue(cpu, R_AX, width, data.dst);
            actualDst = getRegValue(cpu, R_AX, width);
            if (writesDestination(op)) {
                applyRegValue(expectedRegs, R_AX, width, binaryResult(op, actualDst, data.src, width, data.initialFlags));
            }

            runTestCPU();
            verifyRegisters(cpu, expectedRegs, name);
            if (shouldVerifyFlags((BinaryFlagMode)flagMode)) {
                verifyFlags(op, actualDst, data.src, width, data.initialFlags, name);
            }
        }
    }
}

void testRunBinaryGroup1Immediate(TestBinaryOp op, int width, const TestBinaryCase* cases, size_t caseCount, const char* name, U8 opcode, bool signExtend8) {
    Group1ImmediateSize immediateSize = group1ImmediateSize(width, signExtend8);
    for (size_t i = 0; i < caseCount; ++i) {
        TestBinaryCase data = normalizedImmediateCase(cases[i], width, immediateSize);
        for (int flagMode = BINARY_FLAGS_CHECKED; flagMode <= BINARY_FLAGS_OVERWRITTEN; ++flagMode) {
            for (int dst = 0; dst < 8; ++dst) {
                if (!testRunRegister(dst)) {
                    continue;
                }
                U32 expectedRegs[8];
                U32 actualDst;

                beginGeneratedInstruction(data.initialFlags);
                emitBinaryGroup1Immediate(op, dst, width, opcode, immediateSize, encodedImmediateValue(data, immediateSize));
                overwriteFlagsIfNeeded((BinaryFlagMode)flagMode);

                for (int reg = 0; reg < 8; ++reg) {
                    cpu->reg[reg].u32 = REG_GUARD | (0x0100 + reg);
                    expectedRegs[reg] = cpu->reg[reg].u32;
                }

                applyRegValue(expectedRegs, dst, width, data.dst);
                setRegValue(cpu, dst, width, data.dst);
                actualDst = getRegValue(cpu, dst, width);
                if (writesDestination(op)) {
                    applyRegValue(expectedRegs, dst, width, binaryResult(op, actualDst, data.src, width, data.initialFlags));
                }

                runTestCPU();
                verifyRegisters(cpu, expectedRegs, name);
                if (shouldVerifyFlags((BinaryFlagMode)flagMode)) {
                    verifyFlags(op, actualDst, data.src, width, data.initialFlags, name);
                }
            }

            int lockPrefixCount = supportsLockPrefix(op) ? 2 : 1;
            for (int lockPrefix = 0; lockPrefix < lockPrefixCount; ++lockPrefix) {
                runBaseMemoryImmediateCases(op, width, data, opcode, immediateSize, lockPrefix != 0, (BinaryFlagMode)flagMode, name);
                runAbsoluteMemoryImmediateCases(op, width, data, opcode, immediateSize, lockPrefix != 0, (BinaryFlagMode)flagMode, name);
                runSibMemoryImmediateCases(op, width, data, opcode, immediateSize, lockPrefix != 0, (BinaryFlagMode)flagMode, name);
            }
        }
    }
}

#endif
