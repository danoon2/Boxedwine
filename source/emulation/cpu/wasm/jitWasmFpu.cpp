/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
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

#ifdef BOXEDWINE_WASM_JIT

#include "jitWasmCodeGen.h"
#include "wasmJitImports.h"
#include <cstddef>

// Numeric x87 lowering: extended/double/integer representations, FPREM,
// FSCALE and FXTRACT. Cache allocation, memory access and block control
// remain in jitWasmCodeGen.cpp. These methods share its emitter and locals.

// Extended-to-double conversion, including SoftFloat rounding and exception flags.
void JitWasmCodeGen::convertExtendedFpu(FPURegPtr dst, RegPtr index, bool nearest) {
    static_assert(sizeof(softfloat_roundingMode) == 1 && sizeof(softfloat_exceptionFlags) == 1 &&
        sizeof(softfloat_detectTininess) == 1, "Wasm SoftFloat state must use byte fields");
    U32 signExp = allocScratch();
    U32 exponent = allocScratch();
    U32 flags = allocScratch();
    U32 tiny = allocScratch();
    U32 roundBits = allocScratch();
    U32 roundingMode = nearest ? 0 : allocScratch();
    U32 roundIncrement = nearest ? 0 : allocScratch();
    auto stateAddress = [&](U32 offset) {
        m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offset);
    };
    if (nearest) {
        // FPU::getF64 always converts with nearest/even, independently of guest CW.
        stateAddress(offsetof(CPU, wasmSoftFloatRounding));
        m_emitter.emitI32Const(softfloat_round_near_even); m_emitter.emitI32Store8(0);
    } else {
        // FSCALE calls extF80_to_f64 directly and preserves the current mode.
        stateAddress(offsetof(CPU, wasmSoftFloatRounding)); m_emitter.emitI32Load8U(0);
        m_emitter.emitLocalSet(roundingMode);
    }
    auto increment = [this, nearest, roundIncrement] {
        if (nearest) m_emitter.emitI64Const(0x200);
        else { m_emitter.emitLocalGet(roundIncrement); m_emitter.emitOp(WASM_I64_EXTEND_I32_U); }
    };
    emitFpuSlotAddress(index); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalTee(signExp); m_emitter.emitI32Const(0x7fff);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitLocalSet(exponent);
    if (!nearest) {
        m_emitter.emitI32Const(0x200); m_emitter.emitLocalSet(roundIncrement);
        m_emitter.emitLocalGet(roundingMode); m_emitter.emitI32Const(softfloat_round_near_even);
        m_emitter.emitOp(WASM_I32_NE);
        m_emitter.emitLocalGet(roundingMode); m_emitter.emitI32Const(softfloat_round_near_maxMag);
        m_emitter.emitOp(WASM_I32_NE); m_emitter.emitOp(WASM_I32_AND); m_emitter.emitIf();
            m_emitter.emitLocalGet(roundingMode);
            m_emitter.emitI32Const(softfloat_round_min); m_emitter.emitI32Const(softfloat_round_max);
            m_emitter.emitLocalGet(signExp); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_SELECT); m_emitter.emitOp(WASM_I32_EQ);
            m_emitter.emitI32Const(0x3ff); m_emitter.emitOp(WASM_I32_MUL);
            m_emitter.emitLocalSet(roundIncrement);
        m_emitter.emitEnd();
    }
    emitFpuSlotAddress(index); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitI32Const(0); m_emitter.emitLocalSet(flags);
    m_emitter.emitI32Const(0); m_emitter.emitLocalSet(tiny);

    m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0x7fff);
    m_emitter.emitOp(WASM_I32_EQ); m_emitter.emitIf();
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x7fffffffffffffffll);
        m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ);
        m_emitter.emitIf();
            m_emitter.emitI64Const(0x7ff0000000000000ll); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitElse();
            // Quiet NaNs without losing their sign or the retained payload.
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(62);
            m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I32_WRAP_I64);
            m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitI32Const(softfloat_flag_invalid);
            m_emitter.emitOp(WASM_I32_MUL); m_emitter.emitLocalSet(flags);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(11);
            m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitI64Const(0x000fffffffffffffll);
            m_emitter.emitOp(WASM_I64_AND); m_emitter.emitI64Const(0x7ff8000000000000ll);
            m_emitter.emitOp(WASM_I64_OR); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitEnd();
    m_emitter.emitElse();
        // Keep ten rounding bits plus a sticky bit, matching the existing
        // SoftFloat conversion even for noncanonical extended encodings.
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(1);
        m_emitter.emitOp(WASM_I64_SHR_U);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(1);
        m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_OR);
        m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0x3c01);
        m_emitter.emitOp(WASM_I32_SUB); m_emitter.emitLocalSet(exponent);
        m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0);
        m_emitter.emitOp(WASM_I32_LT_S); m_emitter.emitIf();
            // Preserve SoftFloat's before/after-rounding tininess policy.
            stateAddress(offsetof(CPU, wasmSoftFloatTininess)); m_emitter.emitI32Load8U(0);
            m_emitter.emitI32Const(softfloat_tininess_beforeRounding); m_emitter.emitOp(WASM_I32_EQ);
            m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(-1);
            m_emitter.emitOp(WASM_I32_LT_S); m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); increment();
            m_emitter.emitOp(WASM_I64_ADD); m_emitter.emitI64Const(63);
            m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I64_EQZ);
            m_emitter.emitOp(WASM_I32_OR); m_emitter.emitLocalSet(tiny);
            m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(-64);
            m_emitter.emitOp(WASM_I32_GT_S); m_emitter.emitIf();
                // A guarded distance prevents Wasm's modulo-64 shift behavior.
                m_emitter.emitLocalGet(WASM_I64_SCRATCH);
                m_emitter.emitI32Const(0); m_emitter.emitLocalGet(exponent);
                m_emitter.emitOp(WASM_I32_SUB); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
                m_emitter.emitOp(WASM_I64_SHR_U);
                m_emitter.emitLocalGet(WASM_I64_SCRATCH);
                m_emitter.emitI32Const(64); m_emitter.emitLocalGet(exponent);
                m_emitter.emitOp(WASM_I32_ADD); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
                m_emitter.emitOp(WASM_I64_SHL); m_emitter.emitOp(WASM_I64_EQZ);
                m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
                m_emitter.emitOp(WASM_I64_OR); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitElse();
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_EQZ);
                m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
                m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitEnd();
            m_emitter.emitI32Const(0); m_emitter.emitLocalSet(exponent);
        m_emitter.emitEnd();
        m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0x7fd);
        m_emitter.emitOp(WASM_I32_GT_S);
        m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0x7fd);
        m_emitter.emitOp(WASM_I32_EQ);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); increment();
        m_emitter.emitOp(WASM_I64_ADD); m_emitter.emitI64Const(63);
        m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitIf();
            m_emitter.emitI64Const(0x7ff0000000000000ll);
            if (!nearest) {
                m_emitter.emitLocalGet(roundIncrement); m_emitter.emitOp(WASM_I32_EQZ);
                m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SUB);
            }
            m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitI32Const(softfloat_flag_overflow | softfloat_flag_inexact); m_emitter.emitLocalSet(flags);
        m_emitter.emitElse();
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I32_WRAP_I64);
            m_emitter.emitI32Const(0x3ff); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalTee(roundBits); m_emitter.emitIf();
                m_emitter.emitLocalGet(tiny); m_emitter.emitI32Const(softfloat_flag_underflow);
                m_emitter.emitOp(WASM_I32_MUL); m_emitter.emitI32Const(softfloat_flag_inexact);
                m_emitter.emitOp(WASM_I32_OR); m_emitter.emitLocalSet(flags);
            m_emitter.emitEnd();
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); increment();
            m_emitter.emitOp(WASM_I64_ADD); m_emitter.emitI64Const(10);
            m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitLocalGet(roundBits); m_emitter.emitI32Const(0x200);
            m_emitter.emitOp(WASM_I32_EQ);
            if (!nearest) {
                m_emitter.emitLocalGet(roundingMode); m_emitter.emitI32Const(softfloat_round_near_even);
                m_emitter.emitOp(WASM_I32_EQ); m_emitter.emitOp(WASM_I32_AND);
            }
            m_emitter.emitIf();
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(-2);
                m_emitter.emitOp(WASM_I64_AND); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitEnd();
            if (!nearest) {
                m_emitter.emitLocalGet(roundBits); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
                m_emitter.emitLocalGet(roundingMode); m_emitter.emitI32Const(softfloat_round_odd);
                m_emitter.emitOp(WASM_I32_EQ); m_emitter.emitOp(WASM_I32_AND); m_emitter.emitIf();
                    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(1);
                    m_emitter.emitOp(WASM_I64_OR); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                m_emitter.emitEnd();
            }
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_EQZ);
            m_emitter.emitIf();
                m_emitter.emitI32Const(0); m_emitter.emitLocalSet(exponent);
            m_emitter.emitEnd();
            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitLocalGet(exponent); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitI64Const(52); m_emitter.emitOp(WASM_I64_SHL);
            m_emitter.emitOp(WASM_I64_ADD); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitEnd();
    m_emitter.emitEnd();
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitLocalGet(signExp); m_emitter.emitI32Const(0x8000);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
    m_emitter.emitI64Const(48); m_emitter.emitOp(WASM_I64_SHL);
    m_emitter.emitOp(WASM_I64_OR); m_emitter.emitOp(WASM_F64_REINTERPRET_I64);
    m_emitter.emitLocalSet(dst->hardwareReg());
    m_emitter.emitLocalGet(flags); m_emitter.emitIf();
        stateAddress(offsetof(CPU, wasmSoftFloatFlags));
        stateAddress(offsetof(CPU, wasmSoftFloatFlags)); m_emitter.emitI32Load8U(0);
        m_emitter.emitLocalGet(flags); m_emitter.emitOp(WASM_I32_OR); m_emitter.emitI32Store8(0);
    m_emitter.emitEnd();
    freeScratch(roundBits); freeScratch(tiny); freeScratch(flags);
    freeScratch(exponent); freeScratch(signExp);
    if (!nearest) { freeScratch(roundIncrement); freeScratch(roundingMode); }
}

// Host math imports and classification of cached doubles or raw extended values.
void JitWasmCodeGen::fpuMath(FpuMath op, FPURegPtr dst, FPURegPtr src) {
    if (op == FpuMath::Exp2Minus1) m_emitter.emitF64ConstBits(0x4000000000000000ull);
    m_emitter.emitLocalGet(src->hardwareReg());
    U32 helper = op == FpuMath::Sin ? HELPER_FPU_SIN : op == FpuMath::Cos ? HELPER_FPU_COS :
        op == FpuMath::Tan ? HELPER_FPU_TAN : op == FpuMath::Log ? HELPER_FPU_LOG : HELPER_FPU_POW;
    m_emitter.emitCall(helper);
    if (op == FpuMath::Exp2Minus1) {
        m_emitter.emitF64ConstBits(0x3ff0000000000000ull);
        m_emitter.emitOp(WASM_F64_SUB);
    }
    m_emitter.emitLocalSet(dst->hardwareReg());
}

void JitWasmCodeGen::fpuAtan2(FPURegPtr dst, FPURegPtr y, FPURegPtr x) {
    m_emitter.emitLocalGet(y->hardwareReg());
    m_emitter.emitLocalGet(x->hardwareReg());
    m_emitter.emitCall(HELPER_FPU_ATAN2);
    m_emitter.emitLocalSet(dst->hardwareReg());
}

RegPtr JitWasmCodeGen::classifyFpu(FPURegPtr value) {
    RegPtr result = getTmpReg();
    auto bits = [this, value] {
        m_emitter.emitLocalGet(value->hardwareReg());
        m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    };
    bits();
    m_emitter.emitI64Const(63); m_emitter.emitOp(WASM_I64_SHR_U);
    m_emitter.emitOp(WASM_I32_WRAP_I64);
    m_emitter.emitI32Const(9); m_emitter.emitOp(WASM_I32_SHL); // C1 = sign
    bits();
    m_emitter.emitI64Const(0x7fffffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
    m_emitter.emitOp(WASM_I64_EQZ);
    m_emitter.emitIf(WasmType::I32);
        m_emitter.emitI32Const(0x4000); // zero
    m_emitter.emitElse();
        bits();
        m_emitter.emitI64Const(0x7ff0000000000000ll); m_emitter.emitOp(WASM_I64_AND);
        m_emitter.emitI64Const(0x7ff0000000000000ll); m_emitter.emitOp(WASM_I64_EQ);
        m_emitter.emitIf(WasmType::I32);
            m_emitter.emitI32Const(0x0500); // infinity
            m_emitter.emitI32Const(0x0100); // NaN
            bits();
            m_emitter.emitI64Const(0x000fffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
            m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_SELECT);
        m_emitter.emitElse();
            // Match FPU::FXAM, which classifies double denormals as normal.
            m_emitter.emitI32Const(0x0400);
        m_emitter.emitEnd();
    m_emitter.emitEnd();
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalSet(result->hardwareReg());
    return result;
}

RegPtr JitWasmCodeGen::classifyExtendedFpu(RegPtr index) {
    RegPtr result = getTmpReg();
    U32 address = allocScratch();
    U32 exponent = allocScratch();
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(index);
    m_emitter.emitI32Const(sizeof(extFloat80_t)); m_emitter.emitOp(WASM_I32_MUL);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Const(offsetof(CPU, fpu.regs)); m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitLocalTee(address);
    m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalTee(exponent);
    m_emitter.emitI32Const(6); m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitI32Const(0x0200); m_emitter.emitOp(WASM_I32_AND); // C1 = sign
    m_emitter.emitLocalGet(exponent);
    m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalTee(exponent);
    m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitLocalGet(address); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf(WasmType::I32);
        m_emitter.emitI32Const(0x4000);
    m_emitter.emitElse();
        m_emitter.emitLocalGet(exponent);
        m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_EQ);
        m_emitter.emitIf(WasmType::I32);
            m_emitter.emitI32Const(0x0500);
            m_emitter.emitI32Const(0x0100);
            m_emitter.emitLocalGet(address); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
            m_emitter.emitI64Const(0x7fffffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
            m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_SELECT);
        m_emitter.emitElse();
            m_emitter.emitI32Const(0x0400);
        m_emitter.emitEnd();
    m_emitter.emitEnd();
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitLocalSet(result->hardwareReg());
    freeScratch(exponent); freeScratch(address);
    return result;
}

void JitWasmCodeGen::fpuSlotArithmetic(U8 operation, RegPtr st0, RegPtr st1) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(st0);
    pushRegValue(st1); m_emitter.emitI32Const(3); m_emitter.emitOp(WASM_I32_SHL);
    m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitI32Const(operation << 6); m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitI32Store(offsetof(CPU, memHelperValue));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitCall(HELPER_FPU_SLOT_ARITHMETIC);
}

// FPREM/FPREM1 dispatch and exact integer-significand remainder paths.
void JitWasmCodeGen::fpuRemainder(FPURegPtr x, FPURegPtr y, RegPtr xValid, RegPtr yValid,
        RegPtr st0, RegPtr st1, bool nearest, const std::function<void()>& slow) {
    // The divisor's representation selects the interpreter algorithm. A raw
    // dividend is converted by getF64 only when the divisor is cached.
    pushRegValue(yValid);
    m_emitter.emitIf();
        loadFpuValue(x, st0, xValid);
        movValue(JitWidth::b32, xValid, 1);
        m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitLocalGet(x->hardwareReg());
        m_emitter.emitOp(WASM_F64_NE);
        m_emitter.emitLocalGet(y->hardwareReg()); m_emitter.emitLocalGet(y->hardwareReg());
        m_emitter.emitOp(WASM_F64_NE); m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitOp(WASM_F64_ABS);
        m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
        m_emitter.emitOp(WASM_F64_EQ); m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitIf();
            // NaN operands or an infinite dividend produce the interpreter's
            // canonical NaN. These early returns leave all condition bits alone.
            m_emitter.emitF64ConstBits(0x7ff8000000000000ull);
            m_emitter.emitLocalSet(x->hardwareReg());
        m_emitter.emitElse();
            m_emitter.emitLocalGet(y->hardwareReg()); m_emitter.emitOp(WASM_F64_ABS);
            m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
            m_emitter.emitOp(WASM_F64_NE);
            m_emitter.emitIf();
                m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitLocalGet(y->hardwareReg());
                m_emitter.emitOp(WASM_F64_DIV);
                if (nearest) m_emitter.emitOp(WASM_F64_NEAREST);
                // Match Emscripten's double-to-S64 cast, including overflow
                // and NaN/inf quotients from zero divisors, without trapping.
                m_emitter.emitI64TruncSatF64S();
                m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                m_emitter.emitLocalGet(x->hardwareReg());
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_F64_CONVERT_I64_S);
                m_emitter.emitLocalGet(y->hardwareReg()); m_emitter.emitOp(WASM_F64_MUL);
                m_emitter.emitOp(WASM_F64_SUB); m_emitter.emitLocalSet(x->hardwareReg());
                // C0,C3,C1 receive quotient bits 2,1,0; a complete reduction clears C2.
                m_emitter.emitLocalGet(WASM_CPU_LOCAL);
                m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, fpu.sw));
                m_emitter.emitI32Const(~0x4700); m_emitter.emitOp(WASM_I32_AND);
                for (U32 bit = 0; bit < 3; ++bit) {
                    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I32_WRAP_I64);
                    m_emitter.emitI32Const(1 << bit); m_emitter.emitOp(WASM_I32_AND);
                    m_emitter.emitI32Const(bit == 0 ? 9 : bit == 1 ? 13 : 6);
                    m_emitter.emitOp(WASM_I32_SHL); m_emitter.emitOp(WASM_I32_OR);
                }
                m_emitter.emitI32Store(offsetof(CPU, fpu.sw));
            m_emitter.emitEnd();
        m_emitter.emitEnd();
    m_emitter.emitElse();
        // The raw-divisor algorithm first widens a cached dividend. Retire its
        // local before materialization can overwrite this new raw value.
        pushRegValue(xValid); m_emitter.emitIf(); storeDoubleAsExtended(x, st0); m_emitter.emitEnd();
        movValue(JitWidth::b32, xValid, 0);
        U32 ySpecial = allocScratch();
        auto specialExponent = [this](RegPtr index) {
            emitFpuSlotAddress(index); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
            m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_EQ);
        };
        specialExponent(st1); m_emitter.emitLocalSet(ySpecial);
        specialExponent(st0); // Any nonfinite dividend produces canonical NaN.
        m_emitter.emitLocalGet(ySpecial);
        emitFpuSlotAddress(st1); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
        m_emitter.emitI64Const(0x7fffffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
        m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
        m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitIf();
            emitFpuSlotAddress(st0); m_emitter.emitI64Const((S64)0xc000000000000000ull);
            m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
            emitFpuSlotAddress(st0); m_emitter.emitI32Const(0x7fff);
            m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
        m_emitter.emitElse();
            // An infinite divisor leaves the exact raw dividend and SW alone.
            m_emitter.emitLocalGet(ySpecial); m_emitter.emitOp(WASM_I32_EQZ);
            m_emitter.emitIf(); fpuRemainderExact(st0, st1, nearest, slow); m_emitter.emitEnd();
        m_emitter.emitEnd();
        freeScratch(ySpecial);
    m_emitter.emitEnd();
}

void JitWasmCodeGen::fpuRemainderExact(RegPtr st0, RegPtr st1, bool nearest, const std::function<void()>& slow) {
    // Both slots are finite and raw. Recognize exact zero reductions without
    // narrowing the significands or changing SoftFloat's current precision.
    static_assert(sizeof(extF80_roundingPrecision) == 1, "SoftFloat precision access");
    U32 xAddress = allocScratch(), yAddress = allocScratch();
    U32 xSignExp = allocScratch(), ySignExp = allocScratch();
    U32 difference = allocScratch(), precision = allocScratch(), kind = allocScratch();
    emitFpuSlotAddress(st0); m_emitter.emitLocalSet(xAddress);
    emitFpuSlotAddress(st1); m_emitter.emitLocalSet(yAddress);
    m_emitter.emitLocalGet(xAddress); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(xSignExp);
    m_emitter.emitLocalGet(yAddress); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(ySignExp);
    auto exponent = [this](U32 local) {
        m_emitter.emitLocalGet(local); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
    };
    auto significand = [this](U32 address) {
        m_emitter.emitLocalGet(address); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    };
    m_emitter.emitI32Const(0); m_emitter.emitLocalSet(kind);
    m_emitter.emitI32Const(0); m_emitter.emitLocalSet(difference);
    exponent(ySignExp); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
    significand(yAddress); m_emitter.emitI64Const(63); m_emitter.emitOp(WASM_I64_SHR_U);
    m_emitter.emitOp(WASM_I32_WRAP_I64);
    m_emitter.emitOp(WASM_I32_AND); // canonical, nonzero normal divisor
    m_emitter.emitIf();
        exponent(xSignExp); m_emitter.emitOp(WASM_I32_EQZ);
        significand(xAddress); m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitIf();
            m_emitter.emitI32Const(1); m_emitter.emitLocalSet(kind); // zero dividend
        m_emitter.emitElse();
            exponent(xSignExp); exponent(ySignExp); m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitLocalSet(difference);
            exponent(xSignExp); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
            significand(xAddress); significand(yAddress); m_emitter.emitOp(WASM_I64_EQ);
            m_emitter.emitOp(WASM_I32_AND);
            // d=63 can overflow the helper's signed quotient. For d>=64 it
            // uses partial reduction with double pow(2,d-58), finite to 1081.
            m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(1081); m_emitter.emitOp(WASM_I32_LE_U);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(63); m_emitter.emitOp(WASM_I32_NE);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatPrecision));
            m_emitter.emitI32Load8U(0); m_emitter.emitLocalSet(precision);
            // Multiplication rounds even an exact product at reduced precision.
            // Only skip it if the significand already fits that precision.
            significand(xAddress);
            m_emitter.emitLocalGet(precision); m_emitter.emitI32Const(32); m_emitter.emitOp(WASM_I32_EQ);
            m_emitter.emitIf(WasmType::I64);
                m_emitter.emitI64Const(0x000000ffffffffffll);
            m_emitter.emitElse();
                m_emitter.emitI64Const(0x7ff); m_emitter.emitI64Const(0);
                m_emitter.emitLocalGet(precision); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_EQ);
                m_emitter.emitOp(WASM_SELECT);
            m_emitter.emitEnd();
            m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitIf(); m_emitter.emitI32Const(2); m_emitter.emitLocalSet(kind); m_emitter.emitEnd();
        m_emitter.emitEnd();
    m_emitter.emitEnd();
    m_emitter.emitLocalGet(kind);
    m_emitter.emitIf();
        m_emitter.emitLocalGet(xAddress); m_emitter.emitI64Const(0);
        m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
        m_emitter.emitLocalGet(xAddress);
        // Cancellation yields -0 only when rounding down. For a zero dividend
        // and opposite divisor sign, subtraction instead preserves its sign.
        m_emitter.emitLocalGet(kind); m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_EQ);
        m_emitter.emitLocalGet(xSignExp); m_emitter.emitLocalGet(ySignExp); m_emitter.emitOp(WASM_I32_XOR);
        m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitIf(WasmType::I32);
            m_emitter.emitLocalGet(xSignExp); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitElse();
            m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatRounding));
            m_emitter.emitI32Load8U(0); m_emitter.emitI32Const(softfloat_round_min); m_emitter.emitOp(WASM_I32_EQ);
            m_emitter.emitI32Const(15); m_emitter.emitOp(WASM_I32_SHL);
        m_emitter.emitEnd();
        m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
        m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_GE_U);
        m_emitter.emitIf(WasmType::I32);
            m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, fpu.sw));
            m_emitter.emitI32Const(0x0400); m_emitter.emitOp(WASM_I32_OR); // partial: retain quotient bits
        m_emitter.emitElse();
            m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, fpu.sw));
            m_emitter.emitI32Const(~0x4700); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalGet(kind); m_emitter.emitI32Const(2); m_emitter.emitOp(WASM_I32_EQ);
            m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(3); m_emitter.emitOp(WASM_I32_LT_U);
            m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitIf(WasmType::I32);
                m_emitter.emitI32Const(0); m_emitter.emitI32Const(1);
                m_emitter.emitLocalGet(difference); m_emitter.emitOp(WASM_I32_SHL);
                m_emitter.emitLocalTee(precision); m_emitter.emitOp(WASM_I32_SUB);
                m_emitter.emitLocalGet(precision);
                m_emitter.emitLocalGet(xSignExp); m_emitter.emitLocalGet(ySignExp); m_emitter.emitOp(WASM_I32_XOR);
                m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitOp(WASM_SELECT); m_emitter.emitLocalSet(precision); // signed quotient's low bits
                m_emitter.emitI32Const(0);
                for (U32 bit = 0; bit < 3; ++bit) {
                    m_emitter.emitLocalGet(precision); m_emitter.emitI32Const(1 << bit); m_emitter.emitOp(WASM_I32_AND);
                    m_emitter.emitI32Const(bit == 0 ? 9 : bit == 1 ? 13 : 6);
                    m_emitter.emitOp(WASM_I32_SHL); m_emitter.emitOp(WASM_I32_OR);
                }
            m_emitter.emitElse(); m_emitter.emitI32Const(0); m_emitter.emitEnd();
            m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitEnd();
        m_emitter.emitI32Store(offsetof(CPU, fpu.sw));
    m_emitter.emitElse();
        freeScratch(kind); freeScratch(precision); freeScratch(difference);
        freeScratch(ySignExp); freeScratch(xSignExp); freeScratch(yAddress); freeScratch(xAddress);
        fpuRemainderNonzero(st0, st1, nearest, slow);
    m_emitter.emitEnd();
}

void JitWasmCodeGen::fpuRemainderNonzero(RegPtr st0, RegPtr st1, bool nearest, const std::function<void()>& slow) {
    // Exact division by a power of two, or equal significands with |x/y|<=1/2.
    // Require normal operands and a significand that fits the active precision:
    // division, multiplication and subtraction then raise no new SoftFloat flags.
    U32 xAddress = allocScratch(), yAddress = allocScratch();
    U32 xSignExp = allocScratch(), ySignExp = allocScratch(), difference = allocScratch();
    U32 precisionOrSign = allocScratch(), shift = allocScratch(), quotient = allocScratch();
    emitFpuSlotAddress(st0); m_emitter.emitLocalSet(xAddress);
    emitFpuSlotAddress(st1); m_emitter.emitLocalSet(yAddress);
    auto exponent = [this](U32 local) {
        m_emitter.emitLocalGet(local); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
    };
    auto significand = [this](U32 address) {
        m_emitter.emitLocalGet(address); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    };
    m_emitter.emitLocalGet(xAddress); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(xSignExp);
    m_emitter.emitLocalGet(yAddress); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(ySignExp);
    exponent(xSignExp); exponent(ySignExp); m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(difference);
    m_emitter.emitI32Const(1);
    for (auto pair : {std::make_pair(xAddress,xSignExp),std::make_pair(yAddress,ySignExp)}) {
        exponent(pair.second); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
        m_emitter.emitOp(WASM_I32_AND);
        significand(pair.first); m_emitter.emitI64Const(63); m_emitter.emitOp(WASM_I64_SHR_U);
        m_emitter.emitOp(WASM_I32_WRAP_I64); m_emitter.emitOp(WASM_I32_AND);
    }
    // Keep the intermediate quotient normal and the partial-reduction power
    // finite. d=63 can overflow the signed quotient and remains on SoftFloat.
    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(16382); m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Const(16382 + 1081); m_emitter.emitOp(WASM_I32_LE_U); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(63); m_emitter.emitOp(WASM_I32_NE);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatPrecision));
    m_emitter.emitI32Load8U(0); m_emitter.emitLocalSet(precisionOrSign);
    significand(xAddress);
    m_emitter.emitLocalGet(precisionOrSign); m_emitter.emitI32Const(32); m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.emitIf(WasmType::I64); m_emitter.emitI64Const(0x000000ffffffffffll);
    m_emitter.emitElse();
        m_emitter.emitI64Const(0x7ff); m_emitter.emitI64Const(0);
        m_emitter.emitLocalGet(precisionOrSign); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_EQ);
        m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitEnd();
    m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_AND);
    significand(yAddress); m_emitter.emitI64Const((S64)0x8000000000000000ull); m_emitter.emitOp(WASM_I64_EQ);
    // Rounding the integer quotient up must not overflow its signed range or
    // the subsequent multiplication by y. These rare boundaries use the helper.
    exponent(xSignExp); m_emitter.emitI32Const(0x7ffe); m_emitter.emitOp(WASM_I32_NE);
    m_emitter.emitOp(WASM_I32_AND);
    if (nearest) {
        m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(62); m_emitter.emitOp(WASM_I32_EQ);
        significand(xAddress); m_emitter.emitI64Const(-1); m_emitter.emitOp(WASM_I64_EQ);
        m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_AND);
    }
    significand(xAddress); significand(yAddress); m_emitter.emitOp(WASM_I64_EQ);
    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(0); m_emitter.emitOp(WASM_I32_LT_S);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf();
        significand(xAddress); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitI32Const(0); m_emitter.emitLocalSet(quotient);
        m_emitter.emitLocalGet(xSignExp); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalSet(precisionOrSign); // now the result's sign
        auto flipSign = [this, precisionOrSign] {
            m_emitter.emitLocalGet(precisionOrSign); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_XOR);
            m_emitter.emitLocalSet(precisionOrSign);
        };
        auto unit = [this, shift] {
            m_emitter.emitI64Const(1); m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitOp(WASM_I64_SHL);
        };
        m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(0); m_emitter.emitOp(WASM_I32_LT_S);
        m_emitter.emitIf();
            if (nearest) {
                m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(-1); m_emitter.emitOp(WASM_I32_EQ);
                significand(xAddress); significand(yAddress); m_emitter.emitOp(WASM_I64_GT_U);
                m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitIf();
                    // 1/2 < |x/y| < 1: q=1, remainder magnitude is 2^64-sigX.
                    m_emitter.emitI64Const(0); m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_SUB);
                    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                    m_emitter.emitI32Const(1); m_emitter.emitLocalSet(quotient); flipSign();
                m_emitter.emitEnd();
            }
        m_emitter.emitElse();
            m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_GE_U);
            m_emitter.emitIf(WasmType::I32); m_emitter.emitI32Const(5); // partial: truncate sigX/2^5
            m_emitter.emitElse();
                m_emitter.emitI32Const(63); m_emitter.emitLocalGet(difference); m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitEnd(); m_emitter.emitLocalSet(shift);
            significand(xAddress); m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
            m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I32_WRAP_I64); m_emitter.emitLocalSet(quotient);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); unit(); m_emitter.emitI64Const(1); m_emitter.emitOp(WASM_I64_SUB);
            m_emitter.emitOp(WASM_I64_AND); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            if (nearest) {
                m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_LT_U);
                m_emitter.emitIf();
                    auto half = [this, unit] { unit(); m_emitter.emitI64Const(1); m_emitter.emitOp(WASM_I64_SHR_U); };
                    m_emitter.emitLocalGet(WASM_I64_SCRATCH); half(); m_emitter.emitOp(WASM_I64_GT_U);
                    m_emitter.emitLocalGet(WASM_I64_SCRATCH); half(); m_emitter.emitOp(WASM_I64_EQ);
                    m_emitter.emitLocalGet(quotient); m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_AND);
                    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR);
                    m_emitter.emitIf();
                        unit(); m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_SUB);
                        m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                        m_emitter.emitLocalGet(quotient); m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_ADD);
                        m_emitter.emitLocalSet(quotient); flipSign();
                    m_emitter.emitEnd();
                m_emitter.emitEnd();
            }
        m_emitter.emitEnd();
        storeFpuRemainder(xAddress, xSignExp, ySignExp, precisionOrSign, quotient, difference);
    m_emitter.emitElse();
        freeScratch(quotient); freeScratch(shift); freeScratch(precisionOrSign); freeScratch(difference);
        freeScratch(ySignExp); freeScratch(xSignExp); freeScratch(yAddress); freeScratch(xAddress);
        fpuRemainderDyadic(st0, st1, nearest, slow);
    m_emitter.emitEnd();
}

void JitWasmCodeGen::fpuRemainderDyadic(RegPtr st0, RegPtr st1, bool nearest, const std::function<void()>& slow) {
    // sigY = odd * 2^t. If odd divides sigX, the quotient is exactly
    // (sigX/odd) * 2^(d-t), so it can be reduced without extended division.
    U32 xAddress = allocScratch(), yAddress = allocScratch();
    U32 xSignExp = allocScratch(), ySignExp = allocScratch(), difference = allocScratch();
    U32 precisionOrSign = allocScratch(), divisorShift = allocScratch(), shift = allocScratch(), quotient = allocScratch();
    emitFpuSlotAddress(st0); m_emitter.emitLocalSet(xAddress);
    emitFpuSlotAddress(st1); m_emitter.emitLocalSet(yAddress);
    auto exponent = [this](U32 local) {
        m_emitter.emitLocalGet(local); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
    };
    auto significand = [this](U32 address) {
        m_emitter.emitLocalGet(address); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    };
    auto oddDivisor = [this, significand, yAddress, divisorShift] {
        significand(yAddress); m_emitter.emitLocalGet(divisorShift); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
        m_emitter.emitOp(WASM_I64_SHR_U);
    };
    m_emitter.emitLocalGet(xAddress); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(xSignExp);
    m_emitter.emitLocalGet(yAddress); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(ySignExp);
    exponent(xSignExp); exponent(ySignExp); m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(difference);
    m_emitter.emitI32Const(1);
    for (auto pair : {std::make_pair(xAddress,xSignExp),std::make_pair(yAddress,ySignExp)}) {
        exponent(pair.second); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
        m_emitter.emitOp(WASM_I32_AND);
        significand(pair.first); m_emitter.emitI64Const(63); m_emitter.emitOp(WASM_I64_SHR_U);
        m_emitter.emitOp(WASM_I32_WRAP_I64); m_emitter.emitOp(WASM_I32_AND);
    }
    significand(yAddress); m_emitter.emitI64Const((S64)0x8000000000000000ull); m_emitter.emitOp(WASM_I64_NE);
    m_emitter.emitOp(WASM_I32_AND); // powers of two use the preceding path
    // Both inputs are finite at this entry. Keep the quotient normal even
    // when sigX<sigY, and keep the helper's double partial-reduction power finite.
    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(16381); m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Const(16381 + 1081); m_emitter.emitOp(WASM_I32_LE_U); m_emitter.emitOp(WASM_I32_AND);
    // Leave signed-quotient and rounded-product overflow boundaries on SoftFloat.
    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(62); m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_GT_U); m_emitter.emitOp(WASM_I32_AND);
    exponent(xSignExp); m_emitter.emitI32Const(0x7ffe); m_emitter.emitOp(WASM_I32_NE);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatPrecision));
    m_emitter.emitI32Load8U(0); m_emitter.emitLocalSet(precisionOrSign);
    // When odd divides sigX, the reduced numerator has no more significant
    // bits than sigX. Its integer reduction and multiplication by odd also
    // fit this precision, including exact subnormal remainders.
    significand(xAddress);
    m_emitter.emitLocalGet(precisionOrSign); m_emitter.emitI32Const(32); m_emitter.emitOp(WASM_I32_EQ);
    m_emitter.emitIf(WasmType::I64); m_emitter.emitI64Const(0x000000ffffffffffll);
    m_emitter.emitElse();
        m_emitter.emitI64Const(0x7ff); m_emitter.emitI64Const(0);
        m_emitter.emitLocalGet(precisionOrSign); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_EQ);
        m_emitter.emitOp(WASM_SELECT);
    m_emitter.emitEnd();
    m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf(WasmType::I32);
        significand(yAddress); m_emitter.emitOp(WASM_I64_CTZ); m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitLocalSet(divisorShift);
        // The normal-divisor guard makes odd nonzero before this integer divide.
        significand(xAddress); oddDivisor(); m_emitter.emitOp(WASM_I64_DIV_U);
        m_emitter.emitLocalTee(WASM_I64_SCRATCH); oddDivisor(); m_emitter.emitOp(WASM_I64_MUL);
        significand(xAddress); m_emitter.emitOp(WASM_I64_EQ);
    m_emitter.emitElse(); m_emitter.emitI32Const(0); m_emitter.emitEnd();
    m_emitter.emitIf();
        m_emitter.emitLocalGet(xSignExp); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalSet(precisionOrSign); // now the result's sign
        m_emitter.emitI32Const(0); m_emitter.emitLocalSet(quotient);
        m_emitter.emitLocalGet(divisorShift);
        m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_GE_S);
        m_emitter.emitIf(WasmType::I32); m_emitter.emitI32Const(58);
        m_emitter.emitElse(); m_emitter.emitLocalGet(difference); m_emitter.emitEnd();
        m_emitter.emitOp(WASM_I32_SUB); m_emitter.emitLocalSet(shift);
        m_emitter.emitLocalGet(shift); m_emitter.emitI32Const(0); m_emitter.emitOp(WASM_I32_LE_S);
        m_emitter.emitIf();
            // The quotient is already integral; only its low three bits matter.
            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitI32Const(0); m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHL);
            m_emitter.emitOp(WASM_I32_WRAP_I64); m_emitter.emitLocalSet(quotient);
            m_emitter.emitI64Const(0); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitElse();
            m_emitter.emitLocalGet(shift); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_LT_U);
            m_emitter.emitIf();
                auto unit = [this, shift] {
                    m_emitter.emitI64Const(1); m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
                    m_emitter.emitOp(WASM_I64_SHL);
                };
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitLocalGet(shift);
                m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHR_U);
                m_emitter.emitOp(WASM_I32_WRAP_I64); m_emitter.emitLocalSet(quotient);
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); unit(); m_emitter.emitI64Const(1); m_emitter.emitOp(WASM_I64_SUB);
                m_emitter.emitOp(WASM_I64_AND); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                if (nearest) {
                    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_LT_S);
                    m_emitter.emitIf();
                        auto half = [this, unit] { unit(); m_emitter.emitI64Const(1); m_emitter.emitOp(WASM_I64_SHR_U); };
                        m_emitter.emitLocalGet(WASM_I64_SCRATCH); half(); m_emitter.emitOp(WASM_I64_GT_U);
                        m_emitter.emitLocalGet(WASM_I64_SCRATCH); half(); m_emitter.emitOp(WASM_I64_EQ);
                        m_emitter.emitLocalGet(quotient); m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_AND);
                        m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR);
                        m_emitter.emitIf();
                            unit(); m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_SUB);
                            m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                            m_emitter.emitLocalGet(quotient); m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_ADD);
                            m_emitter.emitLocalSet(quotient);
                            m_emitter.emitLocalGet(precisionOrSign); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_XOR);
                            m_emitter.emitLocalSet(precisionOrSign);
                        m_emitter.emitEnd();
                    m_emitter.emitEnd();
                }
            m_emitter.emitEnd();
            // A shift >=64 has zero quotient. A non-power-of-two divisor's
            // odd factor is >=3, so its reduced numerator is strictly below
            // 2^63 and cannot round that quotient up. Powers of two were
            // already handled by fpuRemainderNonzero.
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); oddDivisor(); m_emitter.emitOp(WASM_I64_MUL);
            m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitEnd();
        storeFpuRemainder(xAddress, xSignExp, ySignExp, precisionOrSign, quotient, difference);
    m_emitter.emitElse();
        freeScratch(quotient); freeScratch(shift); freeScratch(divisorShift); freeScratch(precisionOrSign); freeScratch(difference);
        freeScratch(ySignExp); freeScratch(xSignExp); freeScratch(yAddress); freeScratch(xAddress);
        slow();
    m_emitter.emitEnd();
}

void JitWasmCodeGen::storeFpuRemainder(U32 xAddress, U32 xSignExp, U32 ySignExp, U32 resultSign, U32 quotient, U32 difference) {
    // The i64 scratch holds the exact magnitude in the original dividend units.
    U32 shift = allocScratch();
    m_emitter.emitLocalGet(xSignExp); m_emitter.emitLocalGet(ySignExp); m_emitter.emitOp(WASM_I32_XOR);
    m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf();
        m_emitter.emitI32Const(0); m_emitter.emitLocalGet(quotient); m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitLocalSet(quotient);
    m_emitter.emitEnd();
    m_emitter.emitLocalGet(xSignExp); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(xSignExp);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_EQZ);
    m_emitter.emitIf();
        m_emitter.emitI32Const(0); m_emitter.emitLocalSet(xSignExp);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatRounding));
        m_emitter.emitI32Load8U(0); m_emitter.emitI32Const(softfloat_round_min); m_emitter.emitOp(WASM_I32_EQ);
        m_emitter.emitI32Const(15); m_emitter.emitOp(WASM_I32_SHL); m_emitter.emitLocalSet(resultSign);
    m_emitter.emitElse();
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_CLZ);
        m_emitter.emitOp(WASM_I32_WRAP_I64); m_emitter.emitLocalSet(shift);
        m_emitter.emitLocalGet(shift); m_emitter.emitLocalGet(xSignExp); m_emitter.emitOp(WASM_I32_LT_U);
        m_emitter.emitIf();
            m_emitter.emitLocalGet(xSignExp); m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitLocalSet(xSignExp);
        m_emitter.emitElse();
            // Exact subnormal: preserve the exponent-one significand unit.
            m_emitter.emitLocalGet(xSignExp); m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitLocalSet(shift); m_emitter.emitI32Const(0); m_emitter.emitLocalSet(xSignExp);
        m_emitter.emitEnd();
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
        m_emitter.emitOp(WASM_I64_SHL); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitEnd();
    m_emitter.emitLocalGet(xAddress); m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
    m_emitter.emitLocalGet(xAddress); m_emitter.emitLocalGet(xSignExp); m_emitter.emitLocalGet(resultSign);
    m_emitter.emitOp(WASM_I32_OR); m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    m_emitter.emitLocalGet(difference); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_GE_S);
    m_emitter.emitIf(WasmType::I32);
        m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, fpu.sw));
        m_emitter.emitI32Const(0x0400); m_emitter.emitOp(WASM_I32_OR);
    m_emitter.emitElse();
        m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, fpu.sw));
        m_emitter.emitI32Const(~0x4700); m_emitter.emitOp(WASM_I32_AND);
        for (U32 bit = 0; bit < 3; ++bit) {
            m_emitter.emitLocalGet(quotient); m_emitter.emitI32Const(1 << bit); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const(bit == 0 ? 9 : bit == 1 ? 13 : 6);
            m_emitter.emitOp(WASM_I32_SHL); m_emitter.emitOp(WASM_I32_OR);
        }
    m_emitter.emitEnd(); m_emitter.emitI32Store(offsetof(CPU, fpu.sw));
    freeScratch(shift);
}

// Representation helpers shared by remainder, scale and extract.
void JitWasmCodeGen::emitFpuNormalOrZero(FPURegPtr value) {
    // Reject denormals, infinities and NaNs; signed zero is supported.
    m_emitter.emitLocalGet(value->hardwareReg()); m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    m_emitter.emitI64Const(0x7fffffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
    m_emitter.emitOp(WASM_I64_EQZ);
    m_emitter.emitLocalGet(value->hardwareReg()); m_emitter.emitOp(WASM_F64_ABS);
    m_emitter.emitF64ConstBits(0x0010000000000000ull);
    m_emitter.emitOp(WASM_F64_GE);
    m_emitter.emitLocalGet(value->hardwareReg()); m_emitter.emitOp(WASM_F64_ABS);
    m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
    m_emitter.emitOp(WASM_F64_LT);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR);
}

void JitWasmCodeGen::emitFpuSlotAddress(RegPtr index) {
    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(index); m_emitter.emitI32Const(sizeof(extFloat80_t));
    m_emitter.emitOp(WASM_I32_MUL); m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitI32Const(offsetof(CPU, fpu.regs)); m_emitter.emitOp(WASM_I32_ADD);
}

void JitWasmCodeGen::storeNormalDoubleAsExtended(FPURegPtr value, RegPtr index) {
    // Caller has checked normal-or-zero. Conversion is exact bit rearrangement.
    U32 signExp = allocScratch();
    m_emitter.emitLocalGet(value->hardwareReg()); m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(48);
    m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I32_WRAP_I64);
    m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(signExp);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x7fffffffffffffffll);
    m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ);
    m_emitter.emitIf();
        m_emitter.emitI64Const(0); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitElse();
        m_emitter.emitLocalGet(signExp);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(52);
        m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitI32Const(0x7ff); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const(0x3fff - 0x3ff); m_emitter.emitOp(WASM_I32_ADD);
        m_emitter.emitOp(WASM_I32_OR); m_emitter.emitLocalSet(signExp);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x000fffffffffffffll);
        m_emitter.emitOp(WASM_I64_AND); m_emitter.emitI64Const(11); m_emitter.emitOp(WASM_I64_SHL);
        m_emitter.emitI64Const((S64)0x8000000000000000ull); m_emitter.emitOp(WASM_I64_OR);
        m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitEnd();
    emitFpuSlotAddress(index); m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
    emitFpuSlotAddress(index); m_emitter.emitLocalGet(signExp);
    m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
    setRegIsCached(index, false);
    freeScratch(signExp);
}

void JitWasmCodeGen::storeDoubleAsExtended(FPURegPtr value, RegPtr index) {
    // Widening binary64 is exact, including subnormals. Match SoftFloat's
    // NaN payload/sign and signaling-NaN flag without calling into CPU state.
    emitFpuNormalOrZero(value);
    m_emitter.emitIf();
        storeNormalDoubleAsExtended(value, index);
    m_emitter.emitElse();
        U32 signExp = allocScratch();
        U32 shift = allocScratch();
        m_emitter.emitLocalGet(value->hardwareReg()); m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
        m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(48);
        m_emitter.emitOp(WASM_I64_SHR_U); m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitLocalSet(signExp);
        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x7ff0000000000000ll);
        m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ);
        m_emitter.emitIf(); // nonzero subnormal
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x000fffffffffffffll);
            m_emitter.emitOp(WASM_I64_AND); m_emitter.emitLocalTee(WASM_I64_SCRATCH);
            m_emitter.emitOp(WASM_I64_CLZ); m_emitter.emitOp(WASM_I32_WRAP_I64);
            m_emitter.emitLocalSet(shift);
            m_emitter.emitLocalGet(signExp); m_emitter.emitI32Const(0x3c0c);
            m_emitter.emitLocalGet(shift); m_emitter.emitOp(WASM_I32_SUB);
            m_emitter.emitOp(WASM_I32_OR); m_emitter.emitLocalSet(signExp);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitLocalGet(shift);
            m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHL);
            m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitElse(); // infinity or NaN
            m_emitter.emitLocalGet(signExp); m_emitter.emitI32Const(0x7fff);
            m_emitter.emitOp(WASM_I32_OR); m_emitter.emitLocalSet(signExp);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x000fffffffffffffll);
            m_emitter.emitOp(WASM_I64_AND); m_emitter.emitLocalTee(WASM_I64_SCRATCH);
            m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
            m_emitter.emitIf(); // NaN: quiet it and retain the payload
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x0008000000000000ll);
                m_emitter.emitOp(WASM_I64_AND); m_emitter.emitOp(WASM_I64_EQZ);
                m_emitter.emitIf();
                    auto flagsAddress = [this] {
                        m_emitter.emitLocalGet(WASM_CPU_LOCAL);
                        m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatFlags));
                    };
                    flagsAddress(); flagsAddress(); m_emitter.emitI32Load8U(0);
                    m_emitter.emitI32Const(softfloat_flag_invalid); m_emitter.emitOp(WASM_I32_OR);
                    m_emitter.emitI32Store8(0);
                m_emitter.emitEnd();
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(0x0008000000000000ll);
                m_emitter.emitOp(WASM_I64_OR); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitEnd();
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(11);
            m_emitter.emitOp(WASM_I64_SHL); m_emitter.emitI64Const((S64)0x8000000000000000ull);
            m_emitter.emitOp(WASM_I64_OR); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitEnd();
        emitFpuSlotAddress(index); m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
        emitFpuSlotAddress(index); m_emitter.emitLocalGet(signExp);
        m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
        setRegIsCached(index, false);
        freeScratch(shift); freeScratch(signExp);
    m_emitter.emitEnd();
}

// FSCALE and FXTRACT preserve raw extended results when a double would lose bits.
void JitWasmCodeGen::fpuScale(FPURegPtr x, FPURegPtr y, RegPtr xValid, RegPtr yValid,
        RegPtr st0, RegPtr st1) {
    FPURegPtr result = getFPUTmp();
    auto multiplyByScale = [this, y, result](FPURegPtr input) {
        // Match the interpreter's binary64 multiplication by pow(2, trunc(y)).
        // y is not NaN here. Classify extremes before integer conversion.
        m_emitter.emitLocalGet(input->hardwareReg());
        m_emitter.emitLocalGet(y->hardwareReg());
        m_emitter.emitF64ConstBits(0x4090000000000000ull); // 1024
        m_emitter.emitOp(WASM_F64_GE);
        m_emitter.emitIf(WasmType::F64);
            m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
        m_emitter.emitElse();
            m_emitter.emitLocalGet(y->hardwareReg());
            m_emitter.emitF64ConstBits(0xc090cc0000000000ull); // -1075
            m_emitter.emitOp(WASM_F64_LE);
            m_emitter.emitIf(WasmType::F64);
                m_emitter.emitF64ConstBits(0);
            m_emitter.emitElse();
                U32 power = allocScratch();
                m_emitter.emitLocalGet(y->hardwareReg()); m_emitter.emitOp(WASM_I32_TRUNC_F64_S);
                m_emitter.emitLocalTee(power); m_emitter.emitI32Const(-1022);
                m_emitter.emitOp(WASM_I32_GE_S);
                m_emitter.emitIf(WasmType::I64);
                    m_emitter.emitLocalGet(power); m_emitter.emitI32Const(1023); m_emitter.emitOp(WASM_I32_ADD);
                    m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitI64Const(52); m_emitter.emitOp(WASM_I64_SHL);
                m_emitter.emitElse();
                    m_emitter.emitI64Const(1); m_emitter.emitLocalGet(power); m_emitter.emitI32Const(1074);
                    m_emitter.emitOp(WASM_I32_ADD); m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHL);
                m_emitter.emitEnd();
                m_emitter.emitOp(WASM_F64_REINTERPRET_I64);
                freeScratch(power);
            m_emitter.emitEnd();
        m_emitter.emitEnd();
        m_emitter.emitOp(WASM_F64_MUL);
        m_emitter.emitLocalSet(result->hardwareReg());
    };
    pushRegValue(xValid); pushRegValue(yValid); m_emitter.emitOp(WASM_I32_AND);
    for (FPURegPtr value : {x,y}) {
        m_emitter.emitLocalGet(value->hardwareReg()); m_emitter.emitOp(WASM_F64_ABS);
        m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
        m_emitter.emitOp(WASM_F64_LT); m_emitter.emitOp(WASM_I32_AND);
    }
    m_emitter.emitIf();
        multiplyByScale(x);
        storeDoubleAsExtended(result, st0);
        storeDoubleAsExtended(y, st1);
    m_emitter.emitElse();
        pushRegValue(xValid); pushRegValue(yValid); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitIf();
            // FSCALE widens both operands before any classification, quieting
            // signaling NaNs and raising invalid even on an early return.
            storeDoubleAsExtended(x, st0);
            storeDoubleAsExtended(y, st1);
            moveFpuReg(result, x);
            m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitLocalGet(x->hardwareReg());
            m_emitter.emitOp(WASM_F64_NE); m_emitter.emitIf();
                m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitOp(WASM_I64_REINTERPRET_F64);
                m_emitter.emitI64Const(0x0008000000000000ll); m_emitter.emitOp(WASM_I64_OR);
                m_emitter.emitOp(WASM_F64_REINTERPRET_I64); m_emitter.emitLocalSet(result->hardwareReg());
            m_emitter.emitEnd();
            m_emitter.emitLocalGet(y->hardwareReg()); m_emitter.emitLocalGet(y->hardwareReg());
            m_emitter.emitOp(WASM_F64_NE); m_emitter.emitIf();
                m_emitter.emitF64ConstBits(0x7ff8000000000000ull);
                m_emitter.emitLocalSet(result->hardwareReg());
            m_emitter.emitElse();
                m_emitter.emitLocalGet(y->hardwareReg());
                m_emitter.emitF64ConstBits(0xfff0000000000000ull);
                m_emitter.emitOp(WASM_F64_EQ); m_emitter.emitIf();
                    m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitOp(WASM_F64_ABS);
                    m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
                    m_emitter.emitOp(WASM_F64_LT); m_emitter.emitIf(WasmType::F64);
                        // extF80_lt(x, zero) is false for -0: preserve the
                        // interpreter's +0 result for either signed zero.
                        m_emitter.emitF64ConstBits(0x8000000000000000ull);
                        m_emitter.emitF64ConstBits(0);
                        m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitF64ConstBits(0);
                        m_emitter.emitOp(WASM_F64_LT); m_emitter.emitOp(WASM_SELECT);
                    m_emitter.emitElse();
                        m_emitter.emitF64ConstBits(0x7ff8000000000000ull);
                    m_emitter.emitEnd();
                    m_emitter.emitLocalSet(result->hardwareReg());
                m_emitter.emitElse();
                    m_emitter.emitLocalGet(y->hardwareReg());
                    m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
                    m_emitter.emitOp(WASM_F64_EQ); m_emitter.emitIf();
                        m_emitter.emitLocalGet(x->hardwareReg()); m_emitter.emitF64ConstBits(0);
                        m_emitter.emitOp(WASM_F64_EQ); m_emitter.emitIf(WasmType::F64);
                            m_emitter.emitF64ConstBits(0x7ff8000000000000ull);
                        m_emitter.emitElse();
                            m_emitter.emitLocalGet(result->hardwareReg());
                            m_emitter.emitF64ConstBits(0x7ff0000000000000ull);
                            m_emitter.emitOp(WASM_F64_MUL);
                        m_emitter.emitEnd();
                        m_emitter.emitLocalSet(result->hardwareReg());
                    m_emitter.emitElse();
                        multiplyByScale(result);
                    m_emitter.emitEnd();
                m_emitter.emitEnd();
            m_emitter.emitEnd();
            storeDoubleAsExtended(result, st0);
        m_emitter.emitElse();
            // At least one operand is extended. Widen cached operands first,
            // then classify the original extended encodings before conversion.
            pushRegValue(xValid); m_emitter.emitIf(); storeDoubleAsExtended(x, st0); m_emitter.emitEnd();
            pushRegValue(yValid); m_emitter.emitIf(); storeDoubleAsExtended(y, st1); m_emitter.emitEnd();
            U32 multiply = allocScratch(), ySignExp = allocScratch();
            U32 xSignExp = allocScratch(), xClass = allocScratch();
            auto rawResult = [this, st0](U64 significand, S32 signExp, U32 sign = 0) {
                emitFpuSlotAddress(st0); m_emitter.emitI64Const((S64)significand);
                m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
                emitFpuSlotAddress(st0); m_emitter.emitI32Const(signExp);
                if (sign) {
                    m_emitter.emitLocalGet(sign); m_emitter.emitI32Const(0x8000);
                    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_OR);
                }
                m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
            };
            m_emitter.emitI32Const(1); m_emitter.emitLocalSet(multiply);
            emitFpuSlotAddress(st1); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
            m_emitter.emitLocalTee(ySignExp); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_EQ); m_emitter.emitIf();
                m_emitter.emitI32Const(0); m_emitter.emitLocalSet(multiply);
                emitFpuSlotAddress(st1); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
                m_emitter.emitI64Const(0x7fffffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
                m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitIf(); // infinite scale
                    emitFpuSlotAddress(st0); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
                    m_emitter.emitLocalTee(xSignExp); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
                    m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_EQ); m_emitter.emitIf(WasmType::I32);
                        emitFpuSlotAddress(st0); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
                        m_emitter.emitI64Const(0x7fffffffffffffffll); m_emitter.emitOp(WASM_I64_AND);
                        m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_EQZ);
                        m_emitter.emitI32Const(2); m_emitter.emitOp(WASM_I32_ADD); // infinity=2, NaN=3
                    m_emitter.emitElse();
                        m_emitter.emitLocalGet(xSignExp); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
                        m_emitter.emitOp(WASM_I32_EQZ);
                        emitFpuSlotAddress(st0); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
                        m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_AND); // zero=1, other finite=0
                    m_emitter.emitEnd(); m_emitter.emitLocalSet(xClass);
                    m_emitter.emitLocalGet(ySignExp); m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
                    m_emitter.emitIf(); // -infinity
                        m_emitter.emitLocalGet(xClass); m_emitter.emitI32Const(2); m_emitter.emitOp(WASM_I32_LT_U);
                        m_emitter.emitIf();
                            m_emitter.emitLocalGet(xClass); m_emitter.emitIf();
                                rawResult(0, 0); // extF80_lt(-0,+0) is false
                            m_emitter.emitElse(); rawResult(0, 0, xSignExp); m_emitter.emitEnd();
                        m_emitter.emitElse(); rawResult(0xc000000000000000ull, 0x7fff); m_emitter.emitEnd();
                    m_emitter.emitElse(); // +infinity
                        m_emitter.emitLocalGet(xClass); m_emitter.emitI32Const(2); m_emitter.emitOp(WASM_I32_LT_U);
                        m_emitter.emitIf();
                            m_emitter.emitLocalGet(xClass); m_emitter.emitIf();
                                rawResult(0xc000000000000000ull, 0x7fff);
                            m_emitter.emitElse(); rawResult(0x8000000000000000ull, 0x7fff, xSignExp); m_emitter.emitEnd();
                        m_emitter.emitElse();
                            // Preserve raw infinity exactly, including unnormal
                            // encodings; NaNs instead pass through conversion.
                            m_emitter.emitLocalGet(xClass); m_emitter.emitI32Const(3);
                            m_emitter.emitOp(WASM_I32_EQ); m_emitter.emitLocalSet(multiply);
                        m_emitter.emitEnd();
                    m_emitter.emitEnd();
                m_emitter.emitElse(); rawResult(0xc000000000000000ull, 0x7fff); m_emitter.emitEnd();
            m_emitter.emitEnd();
            m_emitter.emitLocalGet(multiply); m_emitter.emitIf();
                convertExtendedFpu(x, st0, false);
                convertExtendedFpu(y, st1, false);
                multiplyByScale(x);
                storeDoubleAsExtended(result, st0);
            m_emitter.emitEnd();
            freeScratch(xClass); freeScratch(xSignExp); freeScratch(ySignExp); freeScratch(multiply);
        m_emitter.emitEnd();
    m_emitter.emitEnd();
}

void JitWasmCodeGen::fpuExtract(FPURegPtr x, RegPtr valid, RegPtr st0, RegPtr result) {
    U32 exponent = allocScratch();
    FPURegPtr exponentValue = getFPUTmp();
    // The complete widening converter handles cached subnormals and NaNs too.
    // Exact raw inputs remain untouched until the extraction below.
    pushRegValue(valid); m_emitter.emitIf();
        storeDoubleAsExtended(x, st0);
    m_emitter.emitEnd();
    emitFpuSlotAddress(st0); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalSet(exponent);
    emitFpuSlotAddress(st0); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0x7fff);
    m_emitter.emitOp(WASM_I32_AND); m_emitter.emitOp(WASM_I32_EQZ);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_EQZ);
    m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitIf();
        // Retain the interpreter's zero case: exponent -inf, significand +0.
        emitFpuSlotAddress(st0); m_emitter.emitI64Const((S64)0x8000000000000000ull);
        m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
        emitFpuSlotAddress(st0); m_emitter.emitI32Const(0xffff);
        m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
        emitFpuSlotAddress(result); m_emitter.emitI64Const(0);
        m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
        emitFpuSlotAddress(result); m_emitter.emitI32Const(0);
        m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
    m_emitter.emitElse();
        emitFpuSlotAddress(result); m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Store(offsetof(extFloat80_t, signif));
        emitFpuSlotAddress(result); m_emitter.emitLocalGet(exponent);
        m_emitter.emitI32Const(0x8000); m_emitter.emitOp(WASM_I32_AND);
        m_emitter.emitI32Const(0x3fff); m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitI32Store16(offsetof(extFloat80_t, signExp));
        m_emitter.emitLocalGet(exponent); m_emitter.emitI32Const(0x7fff);
        m_emitter.emitOp(WASM_I32_AND); m_emitter.emitI32Const(0x3fff);
        m_emitter.emitOp(WASM_I32_SUB); m_emitter.emitOp(WASM_F64_CONVERT_I32_S);
        m_emitter.emitLocalSet(exponentValue->hardwareReg());
        storeNormalDoubleAsExtended(exponentValue, st0);
    m_emitter.emitEnd();
    setRegIsCached(st0, false); setRegIsCached(result, false);
    freeScratch(exponent);
}


// Exact signed 64-bit integer conversion in both directions.
void JitWasmCodeGen::storeInt64AsExtended(RegPtr index) {
    // The signed input is in WASM_I64_SCRATCH. Normalize its magnitude using
    // integer operations so FILD and FBLD retain all 64 bits of precision.
    U32 signExp = allocScratch();
    U32 dist = allocScratch();
    m_emitter.emitI32Const(0);
    m_emitter.emitLocalSet(signExp);

    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Const(0);
    m_emitter.emitOp(WASM_I64_NE);
    m_emitter.emitIf();
    {
        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitI64Const(63);
        m_emitter.emitOp(WASM_I64_SHR_U);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitI32Const(15);
        m_emitter.emitOp(WASM_I32_SHL);
        m_emitter.emitLocalSet(signExp);

        m_emitter.emitLocalGet(signExp);
        m_emitter.emitIf();
        {
            m_emitter.emitI64Const(0);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitOp(WASM_I64_SUB);
            m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        }
        m_emitter.emitEnd();

        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitOp(WASM_I64_CLZ);
        m_emitter.emitOp(WASM_I32_WRAP_I64);
        m_emitter.emitLocalSet(dist);

        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
        m_emitter.emitLocalGet(dist);
        m_emitter.emitOp(WASM_I64_EXTEND_I32_U);
        m_emitter.emitOp(WASM_I64_SHL);
        m_emitter.emitLocalSet(WASM_I64_SCRATCH);

        m_emitter.emitLocalGet(signExp);
        m_emitter.emitI32Const(0x403e);
        m_emitter.emitLocalGet(dist);
        m_emitter.emitOp(WASM_I32_SUB);
        m_emitter.emitOp(WASM_I32_OR);
        m_emitter.emitLocalSet(signExp);
    }
    m_emitter.emitEnd();

    setRegIsCached(index, false);
    RegPtr topReg = getTmpReg();
    shlValueWithDest(JitWidth::b32, topReg, index, 4);

    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(topReg);
    m_emitter.emitI32Const((S32)offsetof(CPU, fpu.regs[0].signExp));
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitLocalGet(signExp);
    m_emitter.emitI32Store16(0);

    m_emitter.emitLocalGet(WASM_CPU_LOCAL);
    pushRegValue(topReg);
    m_emitter.emitI32Const((S32)offsetof(CPU, fpu.regs[0].signif));
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitOp(WASM_I32_ADD);
    m_emitter.emitLocalGet(WASM_I64_SCRATCH);
    m_emitter.emitI64Store(0);

    freeScratch(dist);
    freeScratch(signExp);
}


void JitWasmCodeGen::emitExtendedToInt64(RegPtr index, bool truncate) {
    // Keep the integer significand exact; a double intermediate loses low
    // bits above 2^53. The result is returned in WASM_I64_SCRATCH. Match the
    // existing SoftFloat calls with exact=false (only invalid is raised).
    U32 sign = allocScratch(), dist = allocScratch(), invalid = allocScratch();
    emitFpuSlotAddress(index); m_emitter.emitI32Load16U(offsetof(extFloat80_t, signExp));
    m_emitter.emitLocalTee(sign); m_emitter.emitI32Const(0x7fff); m_emitter.emitOp(WASM_I32_AND);
    m_emitter.emitLocalSet(dist);
    m_emitter.emitI32Const(0x403e); m_emitter.emitLocalGet(dist); m_emitter.emitOp(WASM_I32_SUB);
    m_emitter.emitLocalSet(dist);
    m_emitter.emitLocalGet(sign); m_emitter.emitI32Const(15); m_emitter.emitOp(WASM_I32_SHR_U);
    m_emitter.emitLocalSet(sign);
    emitFpuSlotAddress(index); m_emitter.emitI64Load(offsetof(extFloat80_t, signif));
    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
    m_emitter.emitI32Const(0); m_emitter.emitLocalSet(invalid);
    auto negate = [this, sign] {
        m_emitter.emitLocalGet(sign); m_emitter.emitIf();
            m_emitter.emitI64Const(0); m_emitter.emitLocalGet(WASM_I64_SCRATCH);
            m_emitter.emitOp(WASM_I64_SUB); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        m_emitter.emitEnd();
    };
    if (truncate) {
        m_emitter.emitLocalGet(dist); m_emitter.emitI32Const(0); m_emitter.emitOp(WASM_I32_LE_S);
        m_emitter.emitIf();
            // SoftFloat's truncating entry point accepts only canonical
            // INT64_MIN at this exponent, even for unnormal encodings.
            m_emitter.emitLocalGet(dist); m_emitter.emitOp(WASM_I32_EQZ);
            m_emitter.emitLocalGet(sign); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(INT64_MIN);
            m_emitter.emitOp(WASM_I64_EQ); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitLocalSet(invalid);
        m_emitter.emitElse();
            m_emitter.emitLocalGet(dist); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_GE_S);
            m_emitter.emitIf();
                m_emitter.emitI64Const(0); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitElse();
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitLocalGet(dist);
                m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHR_U);
                m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitEnd();
            negate();
        m_emitter.emitEnd();
    } else {
        U32 fraction = allocScratch(), above = allocScratch(), tie = allocScratch(), round = allocScratch();
        m_emitter.emitLocalGet(dist); m_emitter.emitI32Const(0); m_emitter.emitOp(WASM_I32_LT_S);
        m_emitter.emitIf();
            m_emitter.emitI32Const(1); m_emitter.emitLocalSet(invalid);
        m_emitter.emitElse();
            m_emitter.emitI32Const(0); m_emitter.emitLocalSet(fraction);
            m_emitter.emitI32Const(0); m_emitter.emitLocalSet(above);
            m_emitter.emitI32Const(0); m_emitter.emitLocalSet(tie);
            m_emitter.emitLocalGet(dist); m_emitter.emitIf();
                m_emitter.emitLocalGet(dist); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_LT_S);
                m_emitter.emitIf();
                    // Move the discarded fraction to the top of a u64. Its
                    // comparison with 2^63 distinguishes below/at/above half.
                    auto tail = [this, dist] {
                        m_emitter.emitLocalGet(WASM_I64_SCRATCH);
                        m_emitter.emitI32Const(64); m_emitter.emitLocalGet(dist); m_emitter.emitOp(WASM_I32_SUB);
                        m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHL);
                    };
                    tail(); m_emitter.emitOp(WASM_I64_EQZ); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitLocalSet(fraction);
                    tail(); m_emitter.emitI64Const(INT64_MIN); m_emitter.emitOp(WASM_I64_GT_U); m_emitter.emitLocalSet(above);
                    tail(); m_emitter.emitI64Const(INT64_MIN); m_emitter.emitOp(WASM_I64_EQ); m_emitter.emitLocalSet(tie);
                    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitLocalGet(dist);
                    m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_SHR_U);
                    m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                m_emitter.emitElse();
                    m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I64_EQZ);
                    m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitLocalSet(fraction);
                    m_emitter.emitLocalGet(dist); m_emitter.emitI32Const(64); m_emitter.emitOp(WASM_I32_EQ);
                    m_emitter.emitIf();
                        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(INT64_MIN);
                        m_emitter.emitOp(WASM_I64_GT_U); m_emitter.emitLocalSet(above);
                        m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(INT64_MIN);
                        m_emitter.emitOp(WASM_I64_EQ); m_emitter.emitLocalSet(tie);
                    m_emitter.emitEnd();
                    m_emitter.emitI64Const(0); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
                m_emitter.emitEnd();
            m_emitter.emitEnd();
            m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, fpu.round));
            m_emitter.emitLocalTee(round); m_emitter.emitOp(WASM_I32_EQZ);
            m_emitter.emitIf(WasmType::I32); // nearest, ties to even
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitOp(WASM_I32_WRAP_I64);
                m_emitter.emitI32Const(1); m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitLocalGet(tie); m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitLocalGet(above); m_emitter.emitOp(WASM_I32_OR);
            m_emitter.emitElse();
                m_emitter.emitLocalGet(round); m_emitter.emitI32Const(ROUND_Down); m_emitter.emitOp(WASM_I32_EQ);
                m_emitter.emitLocalGet(sign); m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitLocalGet(round); m_emitter.emitI32Const(ROUND_Up); m_emitter.emitOp(WASM_I32_EQ);
                m_emitter.emitLocalGet(sign); m_emitter.emitOp(WASM_I32_EQZ); m_emitter.emitOp(WASM_I32_AND);
                m_emitter.emitOp(WASM_I32_OR); m_emitter.emitLocalGet(fraction); m_emitter.emitOp(WASM_I32_AND);
            m_emitter.emitEnd();
            m_emitter.emitIf();
                m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(1); m_emitter.emitOp(WASM_I64_ADD);
                m_emitter.emitLocalSet(WASM_I64_SCRATCH);
            m_emitter.emitEnd();
            // Magnitude may reach 2^63 only for a negative result.
            m_emitter.emitLocalGet(WASM_I64_SCRATCH); m_emitter.emitI64Const(INT64_MAX);
            m_emitter.emitLocalGet(sign); m_emitter.emitOp(WASM_I64_EXTEND_I32_U); m_emitter.emitOp(WASM_I64_ADD);
            m_emitter.emitOp(WASM_I64_GT_U); m_emitter.emitLocalSet(invalid);
            negate();
        m_emitter.emitEnd();
        freeScratch(round); freeScratch(tie); freeScratch(above); freeScratch(fraction);
    }
    m_emitter.emitLocalGet(invalid); m_emitter.emitIf();
        m_emitter.emitI64Const(INT64_MIN); m_emitter.emitLocalSet(WASM_I64_SCRATCH);
        auto flagsAddress = [this] {
            m_emitter.emitLocalGet(WASM_CPU_LOCAL); m_emitter.emitI32Load(offsetof(CPU, wasmSoftFloatFlags));
        };
        flagsAddress(); flagsAddress(); m_emitter.emitI32Load8U(0);
        m_emitter.emitI32Const(softfloat_flag_invalid); m_emitter.emitOp(WASM_I32_OR); m_emitter.emitI32Store8(0);
    m_emitter.emitEnd();
    freeScratch(invalid); freeScratch(dist); freeScratch(sign);
}

#endif // BOXEDWINE_WASM_JIT
