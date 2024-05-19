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
#ifndef isnan
#define isinf(x) (!_finite(x))
#endif
#endif

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

#define PI 3.14159265358979323846
#define L2E 1.4426950408889634
#define L2T 3.3219280948873623
#define LN2 0.69314718055994531
#define LG2 0.3010299956639812

#define FPU_GET_TOP(fpu) (((fpu)->sw & 0x3800) >> 11)
#define FPU_SET_TOP(fpu, val) (fpu)->sw &= ~0x3800; (fpu)->sw |= (val & 7) << 11

void FPU::LOG_STACK() {
#ifdef LOG_FPU
    U32 i;

    for (i=0;i<8;i++) {
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

void FPU::setReg(U32 index, double value) {
    this->regs[index].d = value;
    this->isIntegerLoaded[index] = false;
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

U8 FPU::GetAbridgedTag() {
    U8 tag = 0;
    for(U32 i = 0; i < 8; i++) {
        if (this->tags[i] != TAG_Empty) {
            tag |= (1 << i);
        }
    }
    return tag;
}

void FPU::SetTag(U32 tag) {
    int i;
    for (i = 0; i < 8; i++)
        this->tags[i] = ((tag >> (2 * i)) & 3);
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
    this->tags[8] = TAG_Valid; // is only used by us
}

void FPU::FCLEX() {
    this->sw &= 0x7f00;            //should clear exceptions
}

void FPU::PUSH(double in) {
    this->top = (this->top - 1) & 7;
    //actually check if empty
    this->tags[this->top] = TAG_Valid;
    this->regs[this->top].d = in;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::PREP_PUSH() {
    this->top = (this->top - 1) & 7;
    this->tags[this->top] = TAG_Valid;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FPOP() {
    this->tags[this->top] = TAG_Empty;
    //maybe set zero in it as well
    this->top = ((this->top + 1) & 7);
}

double FPU::FROUND(double in) {
    switch (this->round) {
        case ROUND_Nearest:
            if (in - floor(in) > 0.5) return (floor(in) + 1);
            else if (in - floor(in) < 0.5) return (floor(in));
            else return ((((long) (floor(in))) & 1) != 0) ? (floor(in) + 1) : (floor(in));
        case ROUND_Down:
            return (floor(in));
        case ROUND_Up:
            return (ceil(in));
        case ROUND_Chop:
            return in; //the cast afterwards will do it right maybe cast here
        default:
            return in;
    }
}

#define BIAS80 16383
#define BIAS64 1023

double FPU::FLD80(U64 eind, S16 begin) {
    S64 exp64 = (((begin & 0x7fff) - BIAS80));
    S64 blah = ((exp64 > 0) ? exp64 : -exp64) & 0x3ff;
    S64 exp64final = ((exp64 > 0) ? blah : -blah) + BIAS64;
    struct FPU_Reg result = {};

    // 0x3FFF is for rounding
    U32 round = 0;
    if (round == ROUND_Nearest)
        round = 0x3FF;
    else if (round == ROUND_Up) {
        round = 0x7FF;
    }

    S64 mant64 = ((eind + round) >> 11) & 0xfffffffffffffl;
    S64 sign = (begin & 0x8000) != 0 ? 1 : 0;
    result.l = (sign << 63) | (exp64final << 52) | mant64;

    if (eind == 0x8000000000000000l && (begin & 0x7fff) == 0x7fff) {
        //Detect INF and -INF (score 3.11 when drawing a slur.)
        result.d = sign ? -HUGE_VAL : HUGE_VAL;
    }
    return result.d;

    //mant64= test.mant80/2***64    * 2 **53
}

void FPU::ST80(CPU* cpu, U32 addr, int reg) {
    S64 value = ((struct FPU_Reg*)&this->regs[reg])->l;
    U16 sign80 = (value & (0x8000000000000000l)) != 0 ? 1 : 0;
    S64 exp80 = value & (0x7ff0000000000000l);
    U16 exp80final = (U16)(exp80 >> 52);
    S64 mant80 = value & (0x000fffffffffffffl);
    S64 mant80final = (mant80 << 11);
    if (this->regs[reg].d != 0) { //Zero is a special case
        // Elvira wants the 8 and tcalc doesn't
        mant80final |= 0x8000000000000000l;
        //Ca-cyber doesn't like this when result is zero.
        exp80final += (BIAS80 - BIAS64);
    }
    cpu->memory->writed(addr, (U32) mant80final);
    cpu->memory->writed(addr + 4, (U32) (mant80final >> 32));
    cpu->memory->writew(addr + 8, ((sign80 << 15) | (exp80final)));
}

void FPU::ST80(U32 reg, U64* pLow, U64* pHigh) {
    S64 value = ((struct FPU_Reg*)&this->regs[reg])->l;
    U16 sign80 = (value & (0x8000000000000000l)) != 0 ? 1 : 0;
    S64 exp80 = value & (0x7ff0000000000000l);
    U16 exp80final = (U16)(exp80 >> 52);
    S64 mant80 = value & (0x000fffffffffffffl);
    S64 mant80final = (mant80 << 11);
    if (this->regs[reg].d != 0) { //Zero is a special case
        // Elvira wants the 8 and tcalc doesn't
        mant80final |= 0x8000000000000000l;
        //Ca-cyber doesn't like this when result is zero.
        exp80final += (BIAS80 - BIAS64);
    }
    *pLow = mant80final;
    *pHigh = (U16)(((sign80 << 15) | (exp80final)));
}

void FPU::FLD_F32(U32 value, int store_to) {
    struct FPU_Float f;
    f.i = value;
    this->regs[store_to].d = f.f;
    this->isIntegerLoaded[store_to] = 0;
}

void FPU::FLD_F64(U64 value, int store_to) {
    this->regs[store_to].l = value;
}

void FPU::FLD_F80(U64 low, S16 high) {
    this->regs[this->top].d = FLD80(low, high);
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLD_I16(S16 value, int store_to) {
    this->regs[store_to].d = value;
    this->isIntegerLoaded[store_to] = 0;
}

void FPU::FLD_I32(S32 value, int store_to) {
    this->regs[store_to].d = value;
    this->isIntegerLoaded[store_to] = 0;
}

void FPU::FLD_I64(S64 value, int store_to) {
    this->regs[store_to].d = (double)value;
    this->loadedInteger[store_to] = value;
    this->isIntegerLoaded[store_to] = 1;
}

void FPU::FBLD(U8 data[], int store_to) {
    S64 val = 0;
    int in = 0;
    U64 base = 1;

    for(int i = 0;i < 9;i++){
        in = data[i];
        val += ( (in&0xf) * base); //in&0xf shouldn't be higher then 9
        base *= 10;
        val += ((( in>>4)&0xf) * base);
        base *= 10;
    }
    //last number, only now convert to float in order to get
    //the best signification
    double temp = (double)(val);
    in = data[9];
    temp += ( (in&0xf) * base );
    if(in&0x80) temp *= -1.0;
    this->regs[store_to].d = temp; 
    this->isIntegerLoaded[store_to] = 0;
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
    //should depend on rounding method
    struct FPU_Float f;
    f.f = (float)this->regs[this->top].d;
    cpu->memory->writed(addr, f.i);
}

void FPU::FST_F64(CPU* cpu, U32 addr) {
    cpu->memory->writeq(addr, this->regs[this->top].l);
}

void FPU::FST_F80(CPU* cpu, U32 addr) {
    ST80(cpu, addr, this->top);
}

void FPU::FST_I16(CPU* cpu, U32 addr) {
    S16 value = (S16) (FROUND(this->regs[this->top].d));
    cpu->memory->writew(addr, value);
}

void FPU::FSTT_I16(CPU* cpu, U32 addr) {
    S16 value = (S16)this->regs[this->top].d;
    cpu->memory->writew(addr, value);
}

void FPU::FSTT_I32(CPU* cpu, U32 addr) {
    S32 value = (S32)this->regs[this->top].d;
    cpu->memory->writed(addr, value);
}

void FPU::FST_I32(CPU* cpu, U32 addr) {
    S32 value = (S32) (FROUND(this->regs[this->top].d));
    cpu->memory->writed(addr, value);
}

void FPU::FSTT_I64(CPU* cpu, U32 addr) {
    if (this->isIntegerLoaded[this->top])
        cpu->memory->writeq(addr, this->loadedInteger[this->top]);
    else
        cpu->memory->writeq(addr, (S64)this->regs[this->top].d);
}

void FPU::FST_I64(CPU* cpu, U32 addr) {
    if (this->isIntegerLoaded[this->top])
        cpu->memory->writeq(addr, this->loadedInteger[this->top]);
    else
        cpu->memory->writeq(addr, (S64) (FROUND(this->regs[this->top].d)));
}

void FPU::FBST(CPU* cpu, U32 addr) {
    struct FPU_Reg val = this->regs[this->top];
    U8 sign = 0;

    if (this->regs[this->top].l & 0x8000000000000000l) { //sign
        sign=1;
        val.d=-val.d;
    }
    //numbers from back to front
    double temp=val.d;
    for(int i=0;i<9;i++){
        val.d=temp;
        temp = (double)((S64)(floor(val.d/10.0)));
        int p = (int)(val.d - 10.0*temp);
        val.d=temp;
        temp = (double)((S64)(floor(val.d/10.0)));
        p |= ((int)(val.d - 10.0*temp)<<4);
        cpu->memory->writeb(addr+i,p);
    }
    val.d=temp;
    temp = (double)((S64)(floor(val.d/10.0)));
    int p = (int)(val.d - 10.0*temp);
    if(sign)
        p|=0x80;
    cpu->memory->writeb(addr+9,p);
}

void FPU::FADD(int op1, int op2) {
    this->regs[op1].d += this->regs[op2].d;
    this->isIntegerLoaded[op1] = 0;
    //flags and such :)
}

void FPU::FDIV(int st, int other) {
    this->regs[st].d = this->regs[st].d / this->regs[other].d;
    this->isIntegerLoaded[st] = 0;
    //flags and such :)
}

void FPU::FDIVR(int st, int other) {
    this->regs[st].d = this->regs[other].d / this->regs[st].d;
    this->isIntegerLoaded[st] = 0;
    // flags and such :)
}

void FPU::FMUL(int st, int other) {
    this->regs[st].d *= this->regs[other].d;
    this->isIntegerLoaded[st] = 0;
    //flags and such :)
}

void FPU::FSUB(int st, int other) {
    this->regs[st].d = this->regs[st].d - this->regs[other].d;
    this->isIntegerLoaded[st] = 0;
    //flags and such :)
}

void FPU::FSUBR(int st, int other) {
    this->regs[st].d = this->regs[other].d - this->regs[st].d;
    this->isIntegerLoaded[st] = 0;
    //flags and such :)
}

void FPU::FXCH(int st, int other) {
    int tag = this->tags[other];
    struct FPU_Reg reg = this->regs[other];
    this->tags[other] = this->tags[st];
    this->regs[other] = this->regs[st];
    this->tags[st] = tag;
    this->regs[st] = reg;
}

void FPU::FST(int st, int other) {
    this->tags[other] = this->tags[st];
    this->regs[other] = this->regs[st];
}

static void setFlags(CPU* cpu, int newFlags) {
    cpu->lazyFlags = FLAGS_NONE;
    cpu->flags &= ~FMASK_TEST;
    cpu->flags |= (newFlags & FMASK_TEST);
}

void FPU::FCOMI(CPU* cpu, int st, int other) {
    if (((this->tags[st] != TAG_Valid) && (this->tags[st] != TAG_Zero)) ||
            ((this->tags[other] != TAG_Valid) && (this->tags[other] != TAG_Zero)) || isnan(this->regs[st].d) || isnan(this->regs[other].d)) {
        setFlags(cpu, ZF | PF | CF);
        return;
    }
    if (this->regs[st].d == this->regs[other].d) {
        setFlags(cpu, ZF);
        return;
    }
    if (this->regs[st].d < this->regs[other].d) {
        setFlags(cpu, CF);
        return;
    }
    // st > other
    setFlags(cpu, 0);
}

void FPU::FCOM(int st, int other) {
    if (((this->tags[st] != TAG_Valid) && (this->tags[st] != TAG_Zero)) ||
            ((this->tags[other] != TAG_Valid) && (this->tags[other] != TAG_Zero)) || isnan(this->regs[st].d) || isnan(this->regs[other].d)) {
        FPU_SET_C3(this, 1);
        FPU_SET_C2(this, 1);
        FPU_SET_C0(this, 1);
        return;
    }
    if (this->regs[st].d == this->regs[other].d) {
        FPU_SET_C3(this, 1);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 0);
        return;
    }
    if (this->regs[st].d < this->regs[other].d) {
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

void FPU::FUCOM(int st, int other) {
    //does atm the same as fcom
    FCOM(st, other);
}

void FPU::FRNDINT() {        
    double value = this->regs[this->top].d;
    S64 temp = (S64)FROUND(value);
    this->regs[this->top].d = (double) (temp);
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FPREM() {        
    double valtop = this->regs[this->top].d;
    double valdiv = this->regs[STV(1)].d;
    S64 ressaved = (S64)( (valtop/valdiv) );
    // Some backups
    // Real64 res=valtop - ressaved*valdiv;
    // res= fmod(valtop,valdiv);
    this->regs[this->top].d = valtop - ressaved*valdiv;
    this->isIntegerLoaded[this->top] = 0;
    FPU_SET_C0(this, (int)(ressaved & 4));
    FPU_SET_C3(this, (int)(ressaved & 2));
    FPU_SET_C1(this, (int)(ressaved & 1));
    FPU_SET_C2(this, 0); 
}

void FPU::FPREM1() {        
    double valtop = this->regs[this->top].d;
    double valdiv = this->regs[STV(1)].d;
    double quot = valtop/valdiv;
    double quotf = floor(quot);
    S64 ressaved = 0;
    if (quot-quotf>0.5) ressaved = (S64)(quotf+1);
    else if (quot-quotf<0.5) ressaved = (S64)(quotf);
    else ressaved = (S64)(((((S64)(quotf))&1)!=0)?(quotf+1):(quotf));
    this->regs[this->top].d = valtop - ressaved*valdiv;
    this->isIntegerLoaded[this->top] = 0;
    FPU_SET_C0(this, (int)(ressaved&4));
    FPU_SET_C3(this, (int)(ressaved&2));
    FPU_SET_C1(this, (int)(ressaved&1));
    FPU_SET_C2(this, 0); 
}

void FPU::FXAM() {
    S64 bits = this->regs[this->top].l;
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
    if (isnan(this->regs[this->top].d)) {
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 0);
        FPU_SET_C0(this, 1);
    } else if (isinf(this->regs[this->top].d)) {
        FPU_SET_C3(this, 0);
        FPU_SET_C2(this, 1);
        FPU_SET_C0(this, 1);
    } else if (this->regs[this->top].d == 0.0)        //zero or normalized number.
    {
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
    this->regs[this->top].d = pow(2.0, this->regs[this->top].d) - 1;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FYL2X() {
    this->regs[STV(1)].d *= log(this->regs[this->top].d) / log(2.0);
    this->isIntegerLoaded[STV(1)] = 0;
    FPOP();
}

void FPU::FPTAN() {
    this->regs[this->top].d = tan(this->regs[this->top].d);
    this->isIntegerLoaded[this->top] = 0;
    PUSH(1.0);
    FPU_SET_C2(this, 0);
    //flags and such :)
}

void FPU::FPATAN() {
    this->regs[STV(1)].d = atan2(this->regs[STV(1)].d, this->regs[this->top].d);
    this->isIntegerLoaded[STV(1)] = 0;
    FPOP();
    //flags and such :)
}

void FPU::FYL2XP1() {
    this->regs[STV(1)].d *= log(this->regs[this->top].d + 1.0) / log(2.0);
    this->isIntegerLoaded[STV(1)] = 0;
    FPOP();
}

void FPU::FSQRT() {
    this->regs[this->top].d = sqrt(this->regs[this->top].d);
    this->isIntegerLoaded[this->top] = 0;
    //flags and such :)
}

void FPU::FSINCOS() {
    double temp = this->regs[this->top].d;
    this->regs[this->top].d = sin(temp);
    this->isIntegerLoaded[this->top] = 0;
    PUSH(cos(temp));
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
static const U64 DOUBLE_POSITIVE_INFINITY_BITS = 0x7ff0000000000000;
static const U64 DOUBLE_NEGATIVE_INFINITY_BITS = 0xfff0000000000000;
static const U64 DOUBLE_QUIET_NAN_BITS = 0x7FF8000000000000;

void FPU::FSCALE() {
    double value = this->regs[STV(1)].d;
    S64 chopped = (S64)value;
    if (this->regs[this->top].l == DOUBLE_NEGATIVE_INFINITY_BITS || this->regs[this->top].l == DOUBLE_POSITIVE_INFINITY_BITS) {
        if (this->regs[STV(1)].l == DOUBLE_POSITIVE_INFINITY_BITS) {
            return; // keep top at DOUBLE_NEGATIVE_INFINITY_BITS/DOUBLE_POSITIVE_INFINITY_BITS
        } else if (this->regs[STV(1)].l == DOUBLE_NEGATIVE_INFINITY_BITS) {
            this->regs[this->top].l = DOUBLE_QUIET_NAN_BITS;
            return;
        }
    }
    if (this->regs[STV(1)].l == DOUBLE_POSITIVE_INFINITY_BITS && isfinite(this->regs[this->top].d)) {
        if (this->regs[this->top].d == +0.0 || this->regs[this->top].d == -0.0) {
            this->regs[this->top].l = DOUBLE_QUIET_NAN_BITS;
        } else if (this->regs[this->top].d < 0.0) {
            this->regs[this->top].l = DOUBLE_NEGATIVE_INFINITY_BITS;
        } else {
            this->regs[this->top].l = DOUBLE_POSITIVE_INFINITY_BITS;
        }
        this->isIntegerLoaded[this->top] = 0;
        return;
    }
    if (this->regs[STV(1)].l == DOUBLE_NEGATIVE_INFINITY_BITS) {
        if (isfinite(this->regs[this->top].d)) {
            if (this->regs[this->top].d < 0.0) {
                this->regs[this->top].d = -0.0;
            }
            else {
                this->regs[this->top].d = +0.0;
            }
        } else {
            this->regs[this->top].l = DOUBLE_QUIET_NAN_BITS;
        }
    }
    if (isnan(this->regs[STV(1)].d)) {
        this->regs[this->top].l = DOUBLE_QUIET_NAN_BITS;
        this->isIntegerLoaded[this->top] = 0;
        return;
    }
    this->regs[this->top].d *= pow(2.0, (double)chopped);
    this->isIntegerLoaded[this->top] = 0;
    //2^x where x is chopped.
}

void FPU::FSIN() {
    this->regs[this->top].d = sin(this->regs[this->top].d);
    this->isIntegerLoaded[this->top] = 0;
    FPU_SET_C2(this, 0);
}

void FPU::FCOS() {
    this->regs[this->top].d = cos(this->regs[this->top].d);
    this->isIntegerLoaded[this->top] = 0;
    FPU_SET_C2(this, 0);
    //flags and such :)
}

int FPU::GetTag() {        
    int tag = 0;
    int i;

    for (i = 0; i < 8; i++)
        tag |= ((this->tags[i] & 3) << (2 * i));
    return tag;
}

U32 FPU::SW() {
    FPU_SET_TOP(this, this->top); 
    return this->sw;
}

void FPU::FSTENV(CPU* cpu, U32 addr) {
    FPU_SET_TOP(this, this->top);
    if (!cpu->isBig()) {
        cpu->memory->writew(addr + 0, this->cw);
        cpu->memory->writew(addr + 2, this->sw);
        cpu->memory->writew(addr + 4, GetTag());
        cpu->memory->writew(addr + 6, envData[0]); // instruction pointer
        cpu->memory->writew(addr + 8, envData[1]); // op code
        cpu->memory->writew(addr + 10, envData[2]); // data pointer
        cpu->memory->writew(addr + 12, envData[3]); // data pointer selector
    } else {
        cpu->memory->writed(addr + 0, this->cw);
        cpu->memory->writed(addr + 4, this->sw);
        cpu->memory->writed(addr + 8, GetTag());
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
        this->regs[STV(i)].d = FLD80(cpu->memory->readq(addr + start), cpu->memory->readw(addr + start + 8));
        this->isIntegerLoaded[STV(i)] = 0;
        start += 10;
    }
}

void FPU::FXTRACT() {
    // function stores real bias in st and
    // pushes the significant number onto the stack
    // if double ever uses a different base please correct this function
    struct FPU_Reg tmp = this->regs[this->top];        
    S64 exp80 = tmp.l & 0x7ff0000000000000l;        
    S64 exp80final = (exp80 >> 52) - BIAS64;        
    double mant = tmp.d / (pow(2.0, (double) (exp80final)));
        
    this->regs[this->top].d = (double) (exp80final);
    this->isIntegerLoaded[this->top] = 0;
    PUSH(mant);
}

void FPU::FCHS() {
    this->regs[this->top].d = -1.0 * (this->regs[this->top].d);
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FABS() {
    this->regs[this->top].d = fabs(this->regs[this->top].d);
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FTST() {
    this->regs[8].d = 0.0;
    this->isIntegerLoaded[this->top] = 0;
    FCOM(this->top, 8);
}

void FPU::FLD1() {
    PREP_PUSH();
    this->regs[this->top].d = 1.0;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLDL2T() {
    PREP_PUSH();
    this->regs[this->top].d = L2T;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLDL2E() {
    PREP_PUSH();
    this->regs[this->top].d = L2E;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLDPI() {
    PREP_PUSH();
    this->regs[this->top].d = PI;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLDLG2() {
    PREP_PUSH();
    this->regs[this->top].d = LG2;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLDLN2() {
    PREP_PUSH();
    this->regs[this->top].d = LN2;
    this->isIntegerLoaded[this->top] = 0;
}

void FPU::FLDZ() {
    PREP_PUSH();
    this->regs[this->top].d = 0.0;
    this->tags[this->top] = TAG_Zero;
    this->isIntegerLoaded[this->top] = 0;
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

void FPU::FCOM_EA() {
    FCOM(this->top, 8);
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
	FPU_SET_TOP(this, 0);
	this->top=0;
	SetTag(TAG_Empty);
    memset(envData, 0, sizeof(envData));
}