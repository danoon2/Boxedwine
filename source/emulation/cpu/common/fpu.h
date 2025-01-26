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

#ifndef __FPU_H__
#define __FPU_H__

extern "C" {
#include "../../../../lib/softfloat/source/include/platform.h"
#include "../../../../lib/softfloat/source/include/softfloat.h"
}

class CPU;
//#define LOG_FPU

#ifdef LOG_FPU
#include <inttypes.h>
extern BWriteFile fpuLogFile;
void preFpuLog();

template <class... Args>
void flog(const char* format, Args&&... args) {
    BOXEDWINE_CRITICAL_SECTION;

    preFpuLog();
    auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
    BString msg(size + 1, '\0');
    std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
    fpuLogFile.write(msg);
    fpuLogFile.write("\n");
}
#endif

struct FPU_Reg {
    union {
        double d;
        U64 l;
    };    
};

typedef union {

    U64 q;

#ifndef WORDS_BIGENDIAN
    struct {
        U32 d0, d1;
    } ud;

    struct {
        S32 d0, d1;
    } sd;

    struct {
        U16 w0, w1, w2, w3;
    } uw;

    struct {
        S16 w0, w1, w2, w3;
    } sw;

    struct {
        U8 b0, b1, b2, b3, b4, b5, b6, b7;
    } ub;

    struct {
        S8 b0, b1, b2, b3, b4, b5, b6, b7;
    } sb;
#else
    struct {
        U32 d1, d0;
    } ud;

    struct {
        S32 d1, d0;
    } sd;

    struct {
        U16 w3, w2, w1, w0;
    } uw;

    struct {
        S16 w3, w2, w1, w0;
    } sw;

    struct {
        U8 b7, b6, b5, b4, b3, b2, b1, b0;
    } ub;

    struct {
        S8 b7, b6, b5, b4, b3, b2, b1, b0;
    } sb;
#endif

} MMX_reg;

#define TAG_Valid 0
#define TAG_Zero 1
#define TAG_Special 2
#define TAG_Empty 3

// binary translator assumes FPU->regs will be at offset 0
class FPU {
public:
    void reset();
    void startMMX();

    MMX_reg* getMMX(U8 r) {
        return (MMX_reg*)&regs[r].signif;
    }
    void FINIT();
    void FCLEX();
    void PREP_PUSH();
    void FPOP();
    uint_fast8_t getSoftRounding();
    bool getSoftExact() {
        return false;
    }
    void ST80(CPU* cpu, U32 addr, int reg);
    void ST80(U32 reg, U64* pLow, U64* pHigh);
    void LD80(U32 reg, U64 low, U16 high);

    void FLD_F32(U32 value, int store_to);
    void FLD_F64(U64 value, int store_to);
    void FLD_F80(U64 low, S16 high);
    void FLD_I16(S16 value, int store_to);
    void FLD_I32(S32 value, int store_to);
    void FLD_I64(S64 value, int store_to);
    void FBLD(U8 data[], int store_to);
    void FLD_F32_EA(CPU* cpu, U32 address);
    void FLD_F64_EA(CPU* cpu, U32 address);
    void FLD_I32_EA(CPU* cpu, U32 address);
    void FLD_I16_EA(CPU* cpu, U32 address);
    void FST_F32(CPU* cpu, U32 addr);
    void FST_F64(CPU* cpu, U32 addr);
    void FST_F80(CPU* cpu, U32 addr);
    void FST_I16(CPU* cpu, U32 addr);
    void FSTT_I16(CPU* cpu, U32 addr);
    void FST_I32(CPU* cpu, U32 addr);
    void FSTT_I32(CPU* cpu, U32 addr);
    void FST_I64(CPU* cpu, U32 addr);
    void FSTT_I64(CPU* cpu, U32 addr);
    void FBST(CPU* cpu, U32 addr);
    void FADD(int op1, int op2);
    void FDIV(int st, int other);
    void FDIVR(int st, int other);
    void FMUL(int st, int other);
    void FSUB(int st, int other);
    void FSUBR(int st, int other);
    void FXCH(int st, int other);
    void FST(int st, int other);
    void FCOMI(CPU* cpu, int st, int other);
    void FCOM(CPU* cpu, int st, int other);
    void FUCOM(CPU* cpu, int st, int other);
    void FRNDINT();
    void FPREM(bool truncate = true);
    void FPREM1();
    void FXAM();
    void F2XM1();
    void FYL2X();
    void FPTAN();
    void FPATAN();
    void FYL2XP1();
    void FSQRT();
    void FSINCOS();
    void FSCALE();
    void FSIN();
    void FCOS();    
    void FSTENV(CPU* cpu, U32 addr);
    void FLDENV(CPU* cpu, U32 addr);
    void FSAVE(CPU* cpu, U32 addr);
    void FRSTOR(CPU* cpu, U32 addr);
    void FXTRACT();
    void FCHS();
    void FABS();
    void FTST(CPU* cpu);
    void FLD1();
    void FLDL2T();
    void FLDL2E();
    void FLDPI();
    void FLDLG2();
    void FLDLN2();
    void FLDZ();
    void FADD_EA();
    void FMUL_EA();
    void FSUB_EA();
    void FSUBR_EA();
    void FDIV_EA();
    void FDIVR_EA();
    void FCOM_EA(CPU* cpu);
    void FLDCW(CPU* cpu, U32 addr);  
    void FFREE_STi(U32 st);

    inline U32 STV(U32 i) {return ((this->top + (i)) & 7);}
    inline U32 CW() {return this->cw;}    
    U32 SW();

    void FDECSTP();
    void FINCSTP();

    void SetCW(U16 word);
    void SetSW(U16 word);
    void SetTag(U32 tag);

    void SetTagFromAbridged(U8 tag);
    U8 GetAbridgedTag(CPU* cpu);
    inline U32 GetTop() {return this->top;}

    int GetTag(CPU* cpu);
    int GetTag(CPU* cpu, U32 index);
    void LOG_STACK();

    bool isMMXInUse;    

    U32 tags[9];
    U32 cw;
    U32 cw_mask_all;
    U32 sw;
    U32 top;
    U32 round;

    U32 envData[4];

private:
    extFloat80_t& getReg(U32 reg);
    double& getF64(U32 reg);
    float& getF32(U32 reg);
    double FROUND(double in);

    extFloat80_t regs[9];
    FPU_Reg regCache[9];
    bool isRegCached[9];    
};

#endif
