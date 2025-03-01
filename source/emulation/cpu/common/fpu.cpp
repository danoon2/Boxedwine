/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include <math.h>
#include "fpu.h"

#define FMASK_TEST (CF | PF | AF | ZF | SF | OF)    

#ifdef LOG_FPU
BWriteFile fpuLogFile;

void preFpuLog() {
    if (KThread::currentThread()) {
        KThread* thread = KThread::currentThread();
        CPU* cpu = thread->cpu;
        BString name = thread->process->getModuleName(cpu->seg[CS].address + cpu->eip.u32);
        U32 offset = thread->process->getModuleEip(cpu->seg[CS].address + cpu->eip.u32);
        fpuLogFile.write("%s %.08X ", name.c_str(), offset);
    }
}
#define LOG flog
#else
#define LOG
#endif

#ifdef BOXEDWINE_MSVC
#include <float.h>
#ifndef isnan
#define isnan(x) _isnan(x)
#endif
#ifndef isinf
#define isinf(x) (!_finite(x))
#endif
#endif

bool F80_isnan(extFloat80_t& f) {
    if ((f.signExp & 0x7FFF) != 0x7FFF) return false;
    return (f.signif & UINT64_C(0x7FFFFFFFFFFFFFFF)) != 0;
}

bool F80_isinf(extFloat80_t& f) {
    if ((f.signExp & 0x7FFF) != 0x7FFF) return false;
    return (f.signif & UINT64_C(0x7FFFFFFFFFFFFFFF)) == 0;
}

bool F80_isPosInf(extFloat80_t& f) {
    if ((f.signExp & 0x7FFF) != 0x7FFF) return false;
    return (f.signif & UINT64_C(0x7FFFFFFFFFFFFFFF)) == 0 && (f.signExp & 0x8000) == 0;
}

bool F80_isNegInf(extFloat80_t& f) {
    if ((f.signExp & 0x7FFF) != 0x7FFF) return false;
    return (f.signif & UINT64_C(0x7FFFFFFFFFFFFFFF)) == 0 && (f.signExp & 0x8000) != 0;
}

bool F80_isfinite(extFloat80_t& f) {
    return (f.signExp & 0x7FFF) != 0x7FFF;
}

bool F80_iszero(extFloat80_t& f) {
    return (f.signExp & 0x7FFF) == 0 && f.signif == 0;
}

static const extFloat80_t fx80_zero = { 0x0000000000000000U, 0x0000 };
static const extFloat80_t fx80_neg_zero = { 0x0000000000000000U, 0x8000 };
static const extFloat80_t fx80_one = { 0x8000000000000000U, 0x3fff };

static const extFloat80_t fx80_ninf = { 0x8000000000000000U, 0xffff };
static const extFloat80_t fx80_inf = { 0x8000000000000000U, 0x7fff };
static const extFloat80_t fx80_nan = { 0xC000000000000000U, 0x7fff };
static const extFloat80_t fx80_l2t = { 0xd49a784bcd1b8afeU, 0x4000 };
static const extFloat80_t fx80_l2e = { 0xb8aa3b295c17f0bcU, 0x3fff };
static const extFloat80_t fx80_pi = { 0xc90fdaa22168c235U, 0x4000 };
static const extFloat80_t fx80_lg2 = { 0x9a209a84fbcff799U, 0x3ffd };
static const extFloat80_t fx80_ln2 = { 0xb17217f7d1cf79ac, 0x3ffe };

//static const U64 DOUBLE_POSITIVE_INFINITY_BITS = 0x7ff0000000000000;
//static const U64 DOUBLE_NEGATIVE_INFINITY_BITS = 0xfff0000000000000;
static const U64 DOUBLE_QUIET_NAN_BITS = 0x7FF8000000000000;

struct FPU_Float {
    union {
        float f;
        U32   i;
    };
};

#define ROUND_Nearest 0
#define ROUND_Down 1
#define ROUND_Up 2
#define ROUND_Chop 3

#define FPU_GET_TOP(fpu) (((fpu)->sw & 0x3800) >> 11)
#define FPU_SET_TOP(fpu, val) (fpu)->sw &= ~0x3800; (fpu)->sw |= (val & 7) << 11

void FPU::LOG_STACK() {
#ifdef LOG_FPU
    U32 i;

    for (i = 0; i < 8; i++) {
        if (this->tags[STV(i)] == TAG_Empty) {
            fpuLogFile.write("    Empty ");
        } else {
            fpuLogFile.write("    Valid ");
        }
        if (isnan(this->regs[STV(i)].d)) {
            fpuLogFile.write("NAN %f %d %d\n", this->regs[STV(i)].d, this->tags[STV(i)], STV(i));
        } else if (isinf(this->regs[STV(i)].d)) {
            fpuLogFile.write("INF %f %d %d\n", this->regs[STV(i)].d, this->tags[STV(i)], STV(i));
        } else {
            fpuLogFile.write("%f %d %d\n", this->regs[STV(i)].d, this->tags[STV(i)], STV(i));
        }
    }
    fpuLogFile.write("    Temp  %f %d %d\n", this->regs[8].d, this->tags[8], 8);
#endif
}

void FPU::SetTagFromAbridged(U8 tag) {
    for (U32 i = 0; i < 8; i++) {
        if (!(tag & (1 << i))) {
            this->tags[i] = TAG_Empty;
        } else {
            this->tags[i] = TAG_Valid;
        }
    }
}

U8 FPU::GetAbridgedTag(CPU* cpu) {
    U8 tag = 0;
    for (U32 i = 0; i < 8; i++) {
        if (GetTag(cpu, i) != TAG_Empty) {
            tag |= (1 << i);
        }
    }
    return tag;
}

void FPU::SetTag(U32 tag) {
    int i;
    for (i = 0; i < 8; i++) {
        this->tags[i] = tag;
    }
}

void FPU::SetSW(U16 word) {
    this->sw = word;
    this->top = FPU_GET_TOP(this);
}

void FPU::SetCW(U16 word) {
    this->cw = word;
    this->cw_mask_all = word | 0x3f;
    this->round = ((word >> 10) & 3);

#ifdef LOG_FPU
    const char* r;
    switch (this->round) {
    case ROUND_Nearest:
        r = "Nearest";
        break;
    case ROUND_Down:
        r = "Down";
        break;
    case ROUND_Up:
        r = "Up";
        break;
    case ROUND_Chop:
        r = "Chop";
        break;
    default:
        r = "Unknown";
        break;
    }
    fpuLogFile.write("    SetCW %X (%s)\n", word, r);
#endif
}

#define FPU_SET_C0(fpu, C) (fpu)->sw &= ~0x0100; if (C != 0) (fpu)->sw |= 0x0100    
#define FPU_SET_C1(fpu, C) (fpu)->sw &= ~0x0200; if (C != 0) (fpu)->sw |= 0x0200    
#define FPU_SET_C2(fpu, C) (fpu)->sw &= ~0x0400; if (C != 0) (fpu)->sw |= 0x0400    
#define FPU_SET_C3(fpu, C) (fpu)->sw &= ~0x4000; if (C != 0) (fpu)->sw |= 0x4000


void FPU::FINIT() {
#ifdef LOG_FPU
    if (!fpuLogFile.isOpen()) {
        fpuLogFile.createNew("fpu2.txt");
    }
#endif
    SetCW(0x37F);
    this->sw = 0;
    this->top = FPU_GET_TOP(this);
    this->tags[0] = TAG_Empty;
    this->tags[1] = TAG_Empty;
    this->tags[2] = TAG_Empty;
    this->tags[3] = TAG_Empty;
    this->tags[4] = TAG_Empty;
    this->tags[5] = TAG_Empty;
    this->tags[6] = TAG_Empty;
    this->tags[7] = TAG_Empty;
    this->isMMXInUse = false;
    memset(isRegCached, 0, sizeof(isRegCached));
}

void FPU::FCLEX() {
    this->sw &= 0x7f00;            //should clear exceptions
}

void FPU::PREP_PUSH() {
    this->top = (this->top - 1) & 7;
    this->tags[this->top] = TAG_Valid;
}

void FPU::FPOP() {
    this->tags[this->top] = TAG_Empty;
    //maybe set zero in it as well
    this->top = ((this->top + 1) & 7);
}

uint_fast8_t FPU::getSoftRounding() {
    switch (this->round) {
    case ROUND_Nearest:
        return softfloat_round_near_even;
    case ROUND_Down:
        return softfloat_round_min;
    case ROUND_Up:
        return softfloat_round_max;
    case ROUND_Chop:
        return softfloat_round_minMag;
    default:
        return softfloat_round_near_even;
    }
}

void FPU::ST80(CPU* cpu, U32 addr, int reg) {
    extFloat80_t& f80 = getReg(reg);
    cpu->memory->writeq(addr, f80.signif);
    cpu->memory->writew(addr + 8, f80.signExp);
}

void FPU::LD80(U32 reg, U64 low, U16 high) {
    extFloat80_t& f80 = getReg(reg);
    f80.signif = low;
    f80.signExp = high;
}

void FPU::ST80(U32 reg, U64* pLow, U64* pHigh) {
    extFloat80_t& f80 = getReg(reg);
    *pLow = f80.signif;
    *pHigh = f80.signExp;
}

void FPU::FLD_F32(U32 value, int store_to) {
    if (KSystem::useF64) {
        struct FPU_Float f;
        f.i = value;
        regCache[store_to].d = (double)f.f;
        isRegCached[store_to] = true;
    } else {
        float32_sf f;
        f.v = value;
        regs[store_to] = f32_to_extF80(f);
    }
}

void FPU::FLD_F64(U64 value, int store_to) {
    if (KSystem::useF64) {
        regCache[store_to].l = value;
        isRegCached[store_to] = true;
    } else {
        float64_sf f;
        f.v = value;
        regs[store_to] = f64_to_extF80(f);
    }
}

double FPU::FROUND(double in) {
    switch (this->round) {
    case ROUND_Nearest:
        if (in - floor(in) > 0.5) return (floor(in) + 1);
        else if (in - floor(in) < 0.5) return (floor(in));
        else return ((((long)(floor(in))) & 1) != 0) ? (floor(in) + 1) : (floor(in));
    case ROUND_Down:
        return (floor(in));
    case ROUND_Up:
        return (ceil(in));
    case ROUND_Chop:
        return in; // the caller will cast, which will chop
    default:
        return in;
    }
}

void FPU::FLD_F80(U64 low, S16 high) {
    extFloat80_t& f80 = getReg(this->top);
    f80.signif = low;
    f80.signExp = high;
}

void FPU::FLD_I16(S16 value, int store_to) {
    if (KSystem::useF64) {
        regCache[store_to].d = (double)value;
        isRegCached[store_to] = true;
    } else {
        regs[store_to] = i32_to_extF80((S32)value);
    }
}

void FPU::FLD_I32(S32 value, int store_to) {
    if (KSystem::useF64) {
        regCache[store_to].d = (double)value;
        isRegCached[store_to] = true;
    } else {
        regs[store_to] = i32_to_extF80(value);
    }
}

void FPU::FLD_I64(S64 value, int store_to) {
    // ignore useF64 since we don't want to loose precision until we do a calculation
    // some apps seems to do a memcpy like thing with data pushed in and out and we
    // don't want this value to change if its writen directly back out
    regs[store_to] = i64_to_extF80(value);
    isRegCached[store_to] = false;
}

void FPU::FBLD(U8 data[], int store_to) {
    S64 value = 0;

    // 18 digits
    for (U32 i = 9; i >= 0; i--) {
        U8 v = data[i];
        if (i == 9) {
            v &= 0xf;
        }
        value = (value * 100) + ((v >> 4) * 10) + (v & 0xf);
    }
    if (data[9] & 0x80) {
        value *= -1;
    }
    FLD_I64(value, store_to);
}

void FPU::FLD_F32_EA(CPU* cpu, U32 address) {
    FLD_F32(cpu->memory->readd(address), 8);
}

void FPU::FLD_F64_EA(CPU* cpu, U32 address) {
    FLD_F64(cpu->memory->readq(address), 8);
}

void FPU::FLD_I32_EA(CPU* cpu, U32 address) {
    FLD_I32(cpu->memory->readd(address), 8);
}

void FPU::FLD_I16_EA(CPU* cpu, U32 address) {
    FLD_I16(cpu->memory->readw(address), 8);
}

void FPU::FST_F32(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        struct FPU_Float f;
        f.f = (float)this->regCache[this->top].d;
        cpu->memory->writed(addr, f.i);
    } else {
        softfloat_roundingMode = getSoftRounding();
        cpu->memory->writed(addr, extF80_to_f32(this->regs[this->top]).v);
    }
}

void FPU::FST_F64(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        cpu->memory->writeq(addr, this->regCache[this->top].l);
    } else {
        softfloat_roundingMode = getSoftRounding();
        cpu->memory->writeq(addr, extF80_to_f64(this->regs[this->top]).v);
    }
}

void FPU::FST_F80(CPU* cpu, U32 addr) {
    ST80(cpu, addr, this->top);
}

void FPU::FST_I16(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        S16 value = (S16)(FROUND(this->regCache[this->top].d));
        cpu->memory->writew(addr, value);
    } else {
        cpu->memory->writew(addr, (S16)extF80_to_i32(this->regs[this->top], getSoftRounding(), getSoftExact()));
    }
}

void FPU::FSTT_I16(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        S16 value = (S16)(this->regCache[this->top].d);
        cpu->memory->writew(addr, value);
    } else {
        cpu->memory->writew(addr, (S16)extF80_to_i32_r_minMag(this->regs[this->top], getSoftExact()));
    }
}

void FPU::FSTT_I32(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        S32 value = (S32)this->regCache[this->top].d;
        cpu->memory->writed(addr, value);
    } else {
        cpu->memory->writed(addr, extF80_to_i32_r_minMag(this->regs[this->top], getSoftExact()));
    }
}

void FPU::FST_I32(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        S32 value = (S32)(FROUND(this->regCache[this->top].d));
        cpu->memory->writed(addr, value);
    } else {
        cpu->memory->writed(addr, extF80_to_i32(this->regs[this->top], getSoftRounding(), getSoftExact()));
    }
}

void FPU::FSTT_I64(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        cpu->memory->writeq(addr, (S64)this->regCache[this->top].d);
    } else {
        cpu->memory->writeq(addr, extF80_to_i64_r_minMag(this->regs[this->top], getSoftExact()));
    }
}

void FPU::FST_I64(CPU* cpu, U32 addr) {
    if (isRegCached[this->top]) {
        cpu->memory->writeq(addr, (S64)(FROUND(this->regCache[this->top].d)));
    } else {
        cpu->memory->writeq(addr, extF80_to_i64(this->regs[this->top], getSoftRounding(), getSoftExact()));
    }
}

void FPU::FBST(CPU* cpu, U32 addr) {
    S64 sValue;
    if (isRegCached[this->top]) {
        sValue = (S64)this->regCache[this->top].d;
    } else {
        sValue = extF80_to_i64(regs[top], getSoftRounding(), getSoftExact());
    }
    U64 value = sValue < 0 ? (U64)(-1 * sValue) : (U64)sValue;
    U8 data[10] = { 0 };

    for (int i = 0; i < 9; i++) {

        data[i] = (value % 10);
        value /= 10;
        data[i] |= (value % 10) << 4;
        value /= 10;
    }
    data[9] = (sValue < 0) ? 0x80 : 0;
    cpu->memory->memcpy(addr, data, 10);
}

void FPU::FADD(int op1, int op2) {
    if (KSystem::useF64) {
        this->regCache[op1].d = getF64(op1) + getF64(op2);
    } else {
        this->regs[op1] = extF80_add(this->regs[op1], this->regs[op2]);
    }
    //flags and such :)
}

void FPU::FDIV(int st, int other) {
    if (KSystem::useF64) {
        this->regCache[st].d = getF64(st) / getF64(other);
    } else {
        this->regs[st] = extF80_div(this->regs[st], this->regs[other]);
    }
    //flags and such :)
}

void FPU::FDIVR(int st, int other) {
    if (KSystem::useF64) {
        this->regCache[st].d = getF64(other) / getF64(st);
    } else {
        this->regs[st] = extF80_div(this->regs[other], this->regs[st]);
    }
    // flags and such :)
}

void FPU::FMUL(int st, int other) {
    if (KSystem::useF64) {
        this->regCache[st].d = getF64(st) * getF64(other);
    } else {
        this->regs[st] = extF80_mul(this->regs[st], this->regs[other]);
    }
    //flags and such :)
}

void FPU::FSUB(int st, int other) {
    if (KSystem::useF64) {
        this->regCache[st].d = getF64(st) - getF64(other);
    } else {
        this->regs[st] = extF80_sub(this->regs[st], this->regs[other]);
    }
    //flags and such :)
}

void FPU::FSUBR(int st, int other) {
    if (KSystem::useF64) {
        this->regCache[st].d = getF64(other) - getF64(st);
    } else {
        this->regs[st] = extF80_sub(this->regs[other], this->regs[st]);
    }
    //flags and such :)
}

void FPU::FXCH(int st, int other) {
    int tag = this->tags[other];    
    this->tags[other] = this->tags[st];
    this->tags[st] = tag;

    if (isRegCached[other] != isRegCached[st]) {
        if (isRegCached[st]) {
            std::swap(st, other);
        }
        regCache[st] = regCache[other];
        isRegCached[st] = true;
        regs[other] = regs[st];
        isRegCached[other] = false;
    } else if (isRegCached[other]) {
        double& d1 = regCache[other].d;
        double& d2 = regCache[st].d;
        double d = d1;
        d1 = d2;
        d2 = d;
    } else {
        extFloat80_t& f1 = getReg(other);
        extFloat80_t& f2 = getReg(st);
        extFloat80_t reg = f2;
        f2 = f1;
        f1 = reg;
    }
}

void FPU::FST(int st, int other) {
    this->tags[other] = this->tags[st];
    if (isRegCached[st]) {
        this->regCache[other] = this->regCache[st];
        this->isRegCached[other] = true;
    } else {        
        this->regs[other] = this->regs[st];
        this->isRegCached[other] = false;
    }
}

static void setFlags(CPU* cpu, int newFlags) {
    cpu->lazyFlags = FLAGS_NONE;
    cpu->flags &= ~FMASK_TEST;
    cpu->flags |= (newFlags & FMASK_TEST);
}

void FPU::FCOMI(CPU* cpu, int st, int other) {
    U32 stTag = GetTag(cpu, st);
    U32 otherTag = GetTag(cpu, other);

    // don't want to convert to cache just for inspection
    if (isRegCached[other] && isRegCached[st]) {
        if (stTag == TAG_Empty || otherTag == TAG_Empty || isnan(this->regCache[st].d) || isnan(this->regCache[other].d)) {
            setFlags(cpu, ZF | PF | CF);
            return;
        }
        if (this->regCache[st].d == this->regCache[other].d) {
            setFlags(cpu, ZF);
            return;
        }
        if (this->regCache[st].d < this->regCache[other].d) {
            setFlags(cpu, CF);
            return;
        }
        // st > other
        setFlags(cpu, 0);
    } else {
        extFloat80_t f1 = getReg(st);
        extFloat80_t f2 = getReg(other);

        if (stTag == TAG_Empty || otherTag == TAG_Empty || F80_isnan(f1) || F80_isnan(f2)) {
            setFlags(cpu, ZF | PF | CF);
            return;
        }
        if (extF80_eq(f1, f2)) {
            setFlags(cpu, ZF);
            return;
        }
        if (extF80_lt(f1, f2)) {
            setFlags(cpu, CF);
            return;
        }
        // st > other
        setFlags(cpu, 0);
    }
}

void FPU::FCOM(CPU* cpu, int st, int other) {
    U32 stTag = GetTag(cpu, st);
    U32 otherTag = GetTag(cpu, other);

    // don't want to convert to cache just for inspection
    if (isRegCached[other] && isRegCached[st]) {
        if (stTag == TAG_Empty || otherTag == TAG_Empty || isnan(this->regCache[st].d) || isnan(this->regCache[other].d)) {
            FPU_SET_C3(this, 1);
            FPU_SET_C2(this, 1);
            FPU_SET_C0(this, 1);
            return;
        }
        if (this->regCache[st].d == this->regCache[other].d) {
            FPU_SET_C3(this, 1);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 0);
            return;
        }
        if (this->regCache[st].d < this->regCache[other].d) {
            FPU_SET_C3(this, 0);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 1);
            return;
        }
        // st > other
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 0);
    } else {
        extFloat80_t f1 = getReg(st);
        extFloat80_t f2 = getReg(other);
        if (stTag == TAG_Empty || otherTag == TAG_Empty || F80_isnan(f1) || F80_isnan(f2)) {
            FPU_SET_C3(this, 1);
            FPU_SET_C2(this, 1);
            FPU_SET_C0(this, 1);
            return;
        }
        if (extF80_eq(f1, f2)) {
            FPU_SET_C3(this, 1);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 0);
            return;
        }
        if (extF80_lt(f1, f2)) {
            FPU_SET_C3(this, 0);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 1);
            return;
        }
        // st > other
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 0);
    }
}

void FPU::FUCOM(CPU* cpu, int st, int other) {
    //does atm the same as fcom
    FCOM(cpu, st, other);
}

void FPU::FRNDINT() {
    if (isRegCached[this->top]) {
        double value = this->regCache[this->top].d;
        this->regCache[this->top].d = (double)(S64)FROUND(value);
    } else {
        this->regs[this->top] = extF80_roundToInt(this->regs[this->top], getSoftRounding(), getSoftExact());
    }
}

/*
* from https://www.felixcloutier.com/x86/fprem
D := exponent(ST(0)) – exponent(ST(1));
IF D < 64
    THEN
        Q := Integer(TruncateTowardZero(ST(0) / ST(1)));
        ST(0) := ST(0) – (ST(1) * Q);
        C2 := 0;
        C0, C3, C1 := LeastSignificantBits(Q); (* Q2, Q1, Q0 *)
    ELSE
        C2 := 1;
        N := An implementation-dependent number between 32 and 63;
        QQ := Integer(TruncateTowardZero((ST(0) / ST(1)) / 2 ^ (D - N)));
        ST(0) := ST(0) – (ST(1) * QQ * 2 ^ (D - N));
FI;
*/

void FPU::FPREM(bool truncate) {
    // don't want to convert STV(1) to cache just for inspection
    if (isRegCached[STV(1)]) {
        double valtop = getF64(this->top);
        double valdiv = getF64(STV(1));

        if (isnan(valtop) || isnan(valdiv) || isinf(valtop)) {
            regCache[top].l = DOUBLE_QUIET_NAN_BITS;
            isRegCached[top] = true;
            return;
        }
        if (isinf(valdiv)) {
            return;
        }

        S64 ressaved = (S64)((valtop / valdiv));
        // Some backups
        // Real64 res=valtop - ressaved*valdiv;
        // res= fmod(valtop,valdiv);
        this->regCache[this->top].d = valtop - ressaved * valdiv;
        FPU_SET_C0(this, (int)(ressaved & 4));
        FPU_SET_C3(this, (int)(ressaved & 2));
        FPU_SET_C1(this, (int)(ressaved & 1));
        FPU_SET_C2(this, 0);
        return;
    }
    extFloat80_t& top = getReg(this->top);
    extFloat80_t& bottom = getReg(STV(1));

    // conditions based on spec and what hardware returned in unit tests
    if (F80_isnan(bottom) || F80_isnan(top)) {
        regs[this->top] = fx80_nan;
        return;
    }
    if (F80_isinf(top)) {
        regs[this->top] = fx80_nan;
        return;
    }
    if (F80_isinf(bottom)) {
        return;
    }
    // D := exponent(ST(0)) – exponent(ST(1));
    S32 d = (top.signExp & 0x7FFF) - (bottom.signExp & 0x7FFF);
    if (d < 64) {
        // Q := Integer(TruncateTowardZero(ST(0) / ST(1)));
        extFloat80_t divResult = extF80_div(top, bottom);
        S64 q = extF80_to_i64(divResult, truncate ? softfloat_round_minMag : softfloat_round_near_even, false);
        // ST(0) := ST(0) – (ST(1) * Q);
        regs[this->top] = extF80_sub(top, extF80_mul(bottom, i64_to_extF80(q)));
        // C2 := 0;
        FPU_SET_C2(this, 0);
        // C0, C3, C1 : = LeastSignificantBits(Q); (*Q2, Q1, Q0*)
        FPU_SET_C1(this, (q & 1) ? 1 : 0);
        FPU_SET_C3(this, (q & 2) ? 1 : 0);
        FPU_SET_C0(this, (q & 4) ? 1 : 0);
    } else {
        // C2 := 1;
        FPU_SET_C2(this, 0);
        // N: = An implementation - dependent number between 32 and 63;
        S32 n = 58;
        // QQ: = Integer(TruncateTowardZero((ST(0) / ST(1)) / 2 ^ (D - N)));
        float64_sf p64;
        long2Double d2l;
        d2l.d = pow(2.0, d - n);
        p64.v = d2l.l;
        extFloat80_t p80 = f64_to_extF80(p64);

        extFloat80_t qq = extF80_roundToInt(extF80_div(extF80_div(top, bottom), p80), softfloat_round_minMag, false);
        // ST(0) := ST(0) – (ST(1) * QQ * 2 ^ (D - N));
        regs[this->top] = extF80_sub(top, extF80_mul(bottom, extF80_mul(qq, p80)));
    }
}

void FPU::FPREM1() {
    // don't want to convert STV(1) to cache just for inspection
    if (isRegCached[STV(1)]) {
        double valtop = getF64(this->top);
        double valdiv = getF64(STV(1));

        if (isnan(valtop) || isnan(valdiv) || isinf(valtop)) {
            regCache[top].l = DOUBLE_QUIET_NAN_BITS;
            isRegCached[top] = true;
            return;
        }
        if (isinf(valdiv)) {
            return;
        }

        double quot = valtop / valdiv;
        double quotf = floor(quot);
        S64 ressaved = 0;
        if (quot - quotf > 0.5) ressaved = (S64)(quotf + 1);
        else if (quot - quotf < 0.5) ressaved = (S64)(quotf);
        else ressaved = (S64)(((((S64)(quotf)) & 1) != 0) ? (quotf + 1) : (quotf));
        this->regCache[this->top].d = valtop - ressaved * valdiv;
        FPU_SET_C0(this, (int)(ressaved & 4));
        FPU_SET_C3(this, (int)(ressaved & 2));
        FPU_SET_C1(this, (int)(ressaved & 1));
        FPU_SET_C2(this, 0);
        return;
    }
    FPREM(false);
}

/*
* 
C1 := sign bit of ST; (* 0 for positive, 1 for negative *)
CASE (class of value or number in ST(0)) OF
    Unsupported:C3, C2, C0 := 000;
    NaN:
        C3, C2, C0 := 001;
    Normal:
        C3, C2, C0 := 010;
    Infinity:
        C3, C2, C0 := 011;
    Zero:
        C3, C2, C0 := 100;
    Empty:
        C3, C2, C0 := 101;
    Denormal:
        C3, C2, C0 := 110;
ESAC;
*/
void FPU::FXAM() {
    if (isRegCached[top]) {
        S64 bits = this->regCache[this->top].l;
        // this check before looking if the tag is empty is intentional
        if ((bits & 0x8000000000000000l) != 0)    //sign
        {
            FPU_SET_C1(this, 1);
        } else {
            FPU_SET_C1(this, 0);
        }

        if (this->tags[this->top] == TAG_Empty) {
            FPU_SET_C3(this, 1);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 1);
            return;
        }
        if (isnan(this->regCache[this->top].d)) {
            FPU_SET_C3(this, 0);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 1);
        } else if (isinf(this->regCache[this->top].d)) {
            FPU_SET_C3(this, 0);
            FPU_SET_C2(this, 1);
            FPU_SET_C0(this, 1);
        } else if (this->regCache[this->top].d == 0.0)        //zero or normalized number.
        {
            FPU_SET_C3(this, 1);
            FPU_SET_C2(this, 0);
            FPU_SET_C0(this, 0);
        } else {
            FPU_SET_C3(this, 0);
            FPU_SET_C2(this, 1);
            FPU_SET_C0(this, 0);
        }
        return;
    }

    // C1 := sign bit of ST; (* 0 for positive, 1 for negative *)
    if (this->regs[this->top].signExp & 0x8000) {
        FPU_SET_C1(this, 1);
    } else {
        FPU_SET_C1(this, 0);
    }

    if (this->tags[this->top] == TAG_Empty) {
        FPU_SET_C3(this, 1);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 1);
    } else if (F80_isnan(this->regs[this->top])) {
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 1);
    } else if (F80_isinf(this->regs[this->top])) {
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 1);
        FPU_SET_C0(this, 1);
    } else if ((this->regs[this->top].signExp & 0x7fff) == 0 && (this->regs[this->top].signif == 0)) {
        FPU_SET_C3(this, 1);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 0);
    } else {
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 1);
        FPU_SET_C0(this, 0);
    }
}


void FPU::F2XM1() {
    if (isRegCached[top]) {
        regCache[this->top].d = pow(2.0, this->regCache[this->top].d) - 1;
    } else {
        long2Double d;
        d.l = extF80_to_f64(this->regs[this->top]).v;
        d.d = pow(2.0, d.d) - 1;
        float64_sf f;
        f.v = d.l;
        this->regs[this->top] = f64_to_extF80(f);
    }
}

void FPU::FYL2X() {
    // if either is cached, then convert since top is popped and STV(1) is overwritten
    if (isRegCached[top] || isRegCached[STV(1)]) {
        double& d1 = getF64(top);
        double& d2 = getF64(STV(1));
        d2 = d2 * log(d1) / log(2.0);
    } else {
        long2Double d1;
        long2Double d2;
        d1.l = extF80_to_f64(this->regs[this->top]).v;
        d2.l = extF80_to_f64(this->regs[STV(1)]).v;
        d1.d = d2.d * log(d1.d) / log(2.0);
        float64_sf f;
        f.v = d1.l;
        this->regs[STV(1)] = f64_to_extF80(f);
    }
    FPOP();
}

void FPU::FPTAN() {
    if (isRegCached[top]) {
        regCache[this->top].d = tan(this->regCache[this->top].d);
    } else {
        long2Double d;
        d.l = extF80_to_f64(this->regs[this->top]).v;
        d.d = tan(d.d);
        float64_sf f;
        f.v = d.l;
        this->regs[this->top] = f64_to_extF80(f);
    }
    FLD1();

    FPU_SET_C2(this, 0);
    //flags and such :)
}

void FPU::FPATAN() {
    // if either is cached, then convert since top is popped and STV(1) is overwritten
    if (isRegCached[top] || isRegCached[STV(1)]) {
        double& d1 = getF64(top);
        double& d2 = getF64(STV(1));
        d2 = atan2(d2, d1);
    } else {
        long2Double d1;
        long2Double d2;
        d1.l = extF80_to_f64(this->regs[this->top]).v;
        d2.l = extF80_to_f64(this->regs[STV(1)]).v;
        d1.d = atan2(d2.d, d1.d);
        float64_sf f;
        f.v = d1.l;
        this->regs[STV(1)] = f64_to_extF80(f);
    }
    FPOP();
    //flags and such :)
}

void FPU::FYL2XP1() {
    // if either is cached, then convert since top is popped and STV(1) is overwritten
    if (isRegCached[top] || isRegCached[STV(1)]) {
        double& d1 = getF64(top);
        double& d2 = getF64(STV(1));
        d2 = d2 * log(d1 + 1.0) / log(2.0);
    } else {
        long2Double d1;
        long2Double d2;
        d1.l = extF80_to_f64(this->regs[this->top]).v;
        d2.l = extF80_to_f64(this->regs[STV(1)]).v;
        d1.d = d2.d * log(d1.d + 1.0) / log(2.0);
        float64_sf f;
        f.v = d1.l;
        this->regs[STV(1)] = f64_to_extF80(f);
    }
    FPOP();
}

void FPU::FSQRT() {
    if (isRegCached[top]) {
        regCache[this->top].d = sqrt(regCache[this->top].d);
    } else {
        this->regs[this->top] = extF80_sqrt(this->regs[this->top]);
    }
    //flags and such :)
}

// f-16 uses this
void FPU::FSINCOS() {
    if (this->isRegCached[this->top]) {
        double temp = this->regCache[this->top].d;
        this->regCache[this->top].d = sin(temp);
        PREP_PUSH();
        this->regCache[this->top].d = cos(temp);
        this->isRegCached[this->top] = true;
    } else {
        long2Double d;
        long2Double result;
        d.l = extF80_to_f64(this->regs[this->top]).v;
        result.d = sin(d.d);
        float64_sf f;
        f.v = result.l;
        this->regs[this->top] = f64_to_extF80(f);

        PREP_PUSH();

        d.d = cos(d.d);
        f.v = d.l;
        this->regs[this->top] = f64_to_extF80(f);
        this->isRegCached[this->top] = false;
    }
    FPU_SET_C2(this, 0);
    //flags and such :)
}


// ST(0) 		ST(1)
//        -inf -F   -0   +0   +F   +inf  NaN
// -inf   NaN  -inf -inf -inf -inf -inf  NaN
// -F     -0   -F   -F   -F   -F   -inf  NaN
// -0     -0   -0   -0   -0   -0   NaN   NaN
// +0     +0   +0   +0   +0   +0   NaN   NaN
// +F     +0   +F   +F   +F   +F   +inf  NaN
// +inf   NaN +inf  +inf +inf +inf +inf  NaN
// NaN    NaN NaN   NaN  NaN  NaN  NaN   NaN

void FPU::FSCALE() {
    // :TODO: what about a cached version
    extFloat80_t& d0 = getReg(top);
    extFloat80_t& d1 = getReg(STV(1));
    if (F80_isinf(d0)) {
        if (F80_isPosInf(d1)) {
            return; // keep same
        } else if (F80_isNegInf(d1)) {
            d0 = fx80_nan;
            return;
        }
    }
    if (F80_isPosInf(d1) && F80_isfinite(d0)) {
        if (F80_iszero(d0)) {
            d0 = fx80_nan;
        } else if (extF80_lt(d0, fx80_zero)) {
            d0 = fx80_ninf;
        } else {
            d0 = fx80_inf;
        }
        return;
    }
    if (F80_isNegInf(d1)) {
        if (F80_isfinite(d0)) {
            if (extF80_lt(d0, fx80_zero)) {
                d0 = fx80_neg_zero;
            } else {
                d0 = fx80_zero;
            }
        } else {
            d0 = fx80_nan;
        }
    }
    if (F80_isnan(d1)) {
        d0 = fx80_nan;
        return;
    }
    long2Double d;
    long2Double d2;
    d.l = extF80_to_f64(d0).v;
    d2.l = extF80_to_f64(d1).v;
    d.d = d.d * pow(2.0, (double)(S64)d2.d);
    float64_sf f;
    f.v = d.l;
    d0 = f64_to_extF80(f);    
}

void FPU::FSIN() {
    if (isRegCached[top]) {
        regCache[this->top].d = sin(this->regCache[this->top].d);
    } else {
        long2Double d;
        d.l = extF80_to_f64(this->regs[this->top]).v;
        d.d = sin(d.d);
        float64_sf f;
        f.v = d.l;
        this->regs[this->top] = f64_to_extF80(f);
    }

    FPU_SET_C2(this, 0);
}

void FPU::FCOS() {
    if (isRegCached[top]) {
        regCache[this->top].d = cos(this->regCache[this->top].d);
    } else {
        long2Double d;
        d.l = extF80_to_f64(this->regs[this->top]).v;
        d.d = cos(d.d);
        float64_sf f;
        f.v = d.l;
        this->regs[this->top] = f64_to_extF80(f);
    }
    FPU_SET_C2(this, 0);
    //flags and such :)
}

int FPU::GetTag(CPU* cpu, U32 index) {
    U32 tag = this->tags[index] & 3;
    if (isMMXInUse) {
        tag = TAG_Valid;
    }
    if (tag != TAG_Empty) {
        if (isRegCached[index]) {
            if (regCache[index].d == 0.0) {
                tag = TAG_Zero;
            } else if (isnan(regCache[index].d) || isinf(regCache[index].d)) {
                tag = TAG_Special;
            }
        } else {
            if (F80_iszero(regs[index])) {
                tag = TAG_Zero;
            } else if (F80_isnan(regs[index]) || F80_isinf(regs[index])) {
                tag = TAG_Special;
            }
        }
    }
    return tag;
}

int FPU::GetTag(CPU* cpu) {
    int tags = 0;

    for (U32 i = 0; i < 8; i++) {
        tags |= (GetTag(cpu, i) << (2 * i));
    }
    return tags;
}

U32 FPU::SW() {
    FPU_SET_TOP(this, this->top);
    return this->sw;
}

void FPU::FSTENV(CPU* cpu, U32 addr) {
    FPU_SET_TOP(this, this->top);
    if (!cpu->isBig()) {
        cpu->memory->writew(addr + 0, this->cw);
        cpu->memory->writew(addr + 2, SW());
        cpu->memory->writew(addr + 4, GetTag(cpu));
        cpu->memory->writew(addr + 6, envData[0]); // instruction pointer
        cpu->memory->writew(addr + 8, envData[1]); // op code
        cpu->memory->writew(addr + 10, envData[2]); // data pointer
        cpu->memory->writew(addr + 12, envData[3]); // data pointer selector
    } else {
        cpu->memory->writed(addr + 0, this->cw);
        cpu->memory->writed(addr + 4, SW());
        cpu->memory->writed(addr + 8, GetTag(cpu));
        cpu->memory->writed(addr + 12, envData[0]);
        cpu->memory->writed(addr + 16, envData[1]);
        cpu->memory->writed(addr + 20, envData[2]);
        cpu->memory->writed(addr + 24, envData[3]);
    }
}

void FPU::FLDENV(CPU* cpu, U32 addr) {
    U32 tag = 0;
    U32 cw = 0;

    if (!cpu->isBig()) {
        cw = cpu->memory->readw(addr + 0);
        this->sw = cpu->memory->readw(addr + 2);
        tag = cpu->memory->readw(addr + 4);
        for (int i = 0; i < 4; i++) {
            envData[i] = cpu->memory->readw(addr + 6 + i * 2);
        }
    } else {
        cw = cpu->memory->readd(addr + 0);
        this->sw = cpu->memory->readd(addr + 4);
        tag = cpu->memory->readd(addr + 8);
        for (int i = 0; i < 4; i++) {
            envData[i] = cpu->memory->readd(addr + 12 + i * 4);
        }
    }
    SetTag(tag);
    SetCW(cw);
    this->top = FPU_GET_TOP(this);
}

void FPU::FSAVE(CPU* cpu, U32 addr) {
    int start = (cpu->isBig() ? 28 : 14);
    int i;

    FSTENV(cpu, addr);

    for (i = 0; i < 8; i++) {
        ST80(cpu, addr + start, STV(i));
        start += 10;
    }
    FINIT();
}

void FPU::FRSTOR(CPU* cpu, U32 addr) {
    int start = (cpu->isBig() ? 28 : 14);
    int i;

    FLDENV(cpu, addr);

    for (i = 0; i < 8; i++) {
        regs[STV(i)].signif = cpu->memory->readq(addr + start);
        regs[STV(i)].signExp = cpu->memory->readw(addr + start + 8);
        isRegCached[STV(i)] = false;
        start += 10;
    }
}

void FPU::FXTRACT() {
    extFloat80_t& d0 = getReg(top);

    if (F80_iszero(d0)) {
        d0 = fx80_ninf;
        PREP_PUSH();
        regs[top] = fx80_zero;
        isRegCached[top] = false;
    } else {
        extFloat80_t exponent = i32_to_extF80((d0.signExp & 0x7fff) - 0x3fff);
        extFloat80_t mant = d0;
        mant.signExp &= ~0x7fff;
        mant.signExp |= 0x3fff;
        d0 = exponent;
        PREP_PUSH();
        regs[top] = mant;
        isRegCached[top] = false;
    }
}

void FPU::FCHS() {
    if (isRegCached[top]) {
        regCache[top].d = -1.0 * regCache[top].d;
    } else {
        regs[top].signExp ^= 0x8000;
    }
    FPU_SET_C1(this, 0);
}

void FPU::FABS() {
    if (isRegCached[top]) {
        regCache[top].d = fabs(regCache[top].d);
    } else {
        regs[top].signExp &= 0x7fff;
    }
    FPU_SET_C1(this, 0);
}

void FPU::FTST(CPU* cpu) {
    if (isRegCached[top]) { // check top since that is what we will compare against
        regCache[8].d = 0.0;
        tags[8] = TAG_Zero;
        isRegCached[8] = true;
    } else {
        regs[8] = fx80_zero;
        tags[8] = TAG_Zero;
        isRegCached[8] = false;
    }
    FCOM(cpu, this->top, 8);
}

void FPU::FLD1() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 1.0;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_one;
        isRegCached[top] = false;
    }
}

void FPU::FLDL2T() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 3.3219280948873623;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_l2t;
        isRegCached[top] = false;
    }
}

void FPU::FLDL2E() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 1.4426950408889634;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_l2e;
        isRegCached[top] = false;
    }
}

void FPU::FLDPI() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 3.14159265358979323846;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_pi;
        isRegCached[top] = false;
    }
}

void FPU::FLDLG2() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 0.3010299956639812;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_lg2;
        isRegCached[top] = false;
    }
}

void FPU::FLDLN2() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 0.69314718055994531;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_ln2;
        isRegCached[top] = false;
    }
}

void FPU::FLDZ() {
    PREP_PUSH();
    if (KSystem::useF64) {
        regCache[top].d = 0.0;
        isRegCached[top] = true;
    } else {
        regs[top] = fx80_zero;
        isRegCached[top] = false;
    }
}


void FPU::FADD_EA() {
    FADD(this->top, 8);
}

void FPU::FMUL_EA() {
    FMUL(this->top, 8);
}

void FPU::FSUB_EA() {
    FSUB(this->top, 8);
}

void FPU::FSUBR_EA() {
    FSUBR(this->top, 8);
}

void FPU::FDIV_EA() {
    FDIV(this->top, 8);
}

void FPU::FDIVR_EA() {
    FDIVR(this->top, 8);
}

void FPU::FCOM_EA(CPU* cpu) {
    FCOM(cpu, this->top, 8);
}

void FPU::FLDCW(CPU* cpu, U32 addr) {
    U32 temp = cpu->memory->readw(addr);
    SetCW(temp);
}

void FPU::FDECSTP() {
    this->top = (this->top - 1) & 7;
}

void FPU::FINCSTP() {
    this->top = (this->top + 1) & 7;
}

void FPU::FFREE_STi(U32 st) {
    this->tags[st] = TAG_Empty;
}

void FPU::reset() {
    memset(this, 0, sizeof(FPU));
    SetTag(TAG_Empty);
}

void FPU::startMMX() {
    SetSW(0);
    SetTag(0); // all TAG_Valid
    isMMXInUse = true;
    memset(isRegCached, 0, sizeof(isRegCached));
}

extFloat80_t& FPU::getReg(U32 reg) {
    if (isRegCached[reg]) {
        isRegCached[reg] = false;
        float64_sf f;
        f.v = regCache[reg].l;
        regs[reg] = f64_to_extF80(f);
    }
    return regs[reg];
}

double& FPU::getF64(U32 reg) {
    if (!isRegCached[reg]) {
        isRegCached[reg] = true;
        softfloat_roundingMode = softfloat_round_near_even;
        regCache[reg].l = extF80_to_f64(regs[reg]).v;
    }
    return regCache[reg].d;
}

U32 FPU::sizeofRegInRegsArray() {
    return (U32)(offsetof(FPU, regs[1])) - (U32)(offsetof(FPU, regs[0])); // weird offset calculation to take into account padding
}
