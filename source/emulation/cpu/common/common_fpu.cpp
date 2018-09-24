#include "boxedwine.h"

/* WATCHIT : ALWAYS UPDATE REGISTERS BEFORE AND AFTER USING THEM
            STATUS WORD =>	cpu->fpu.SET_TOP(fpu.top) BEFORE a read
            fpu.top=cpu->fpu.GET_TOP() after a write;
            */

void common_FADD_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FADD_EA();
}

void common_FMUL_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FMUL_EA();
}

void common_FCOM_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
}

void common_FCOM_SINGLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
    cpu->fpu.FPOP();
}

void common_FSUB_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FSUB_EA();
}

void common_FSUBR_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
}

void common_FDIV_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FDIV_EA();
}

void common_FDIVR_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
}

void common_FADD_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FADD(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FMUL_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FMUL(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FCOM_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FCOM_STi_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
}

void common_FSUB_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FSUB(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FSUBR_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FSUBR(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FDIV_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FDIV(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FDIVR_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FDIVR(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FLD_SINGLE_REAL(CPU* cpu,U32 address) {
    U32 value = readd(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));
}

void common_FST_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FST_F32(cpu, address);
}

void common_FST_SINGLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_F32(cpu, address);
    cpu->fpu.FPOP();
}

void common_FLDENV(CPU* cpu, U32 address) {
    cpu->fpu.FLDENV(cpu, address);
}

void common_FLDCW(CPU* cpu, U32 address) {
    cpu->fpu.FLDCW(cpu, address);
}

void common_FNSTENV(CPU* cpu, U32 address) {
    cpu->fpu.FSTENV(cpu, address);
}

void common_FNSTCW(CPU* cpu, U32 address) {
    writew(address, cpu->fpu.CW());
}

void common_FLD_STi(CPU* cpu, U32 reg) {
    int reg_from = cpu->fpu.STV(reg);
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FST(reg_from, cpu->fpu.STV(0));
}

void common_FXCH_STi(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    LOG("FXCH_STi %d,%d(%d)", cpu->fpu.STV(0), cpu->fpu.STV(reg), reg); 
    LOG("    before");
    LOG_STACK();
#endif
    cpu->fpu.FXCH(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FNOP(CPU* cpu) {
}

void common_FCHS(CPU* cpu) {
    cpu->fpu.FCHS();
}

void common_FABS(CPU* cpu) {
    cpu->fpu.FABS();
}

void common_FTST(CPU* cpu) {
    cpu->fpu.FTST();
}

void common_FXAM(CPU* cpu) {
    cpu->fpu.FXAM();
}

void common_FLD1(CPU* cpu) {
    cpu->fpu.FLD1();
}

void common_FLDL2T(CPU* cpu) {
    cpu->fpu.FLDL2T();
}

void common_FLDL2E(CPU* cpu) {
    cpu->fpu.FLDL2E();
}

void common_FLDPI(CPU* cpu) {
    cpu->fpu.FLDPI();
}

void common_FLDLG2(CPU* cpu) {
    cpu->fpu.FLDLG2();
}

void common_FLDLN2(CPU* cpu) {
    cpu->fpu.FLDLN2();
}

void common_FLDZ(CPU* cpu) {
    cpu->fpu.FLDZ();
}

void common_F2XM1(CPU* cpu) {
    cpu->fpu.F2XM1();
}

void common_FYL2X(CPU* cpu) {
    cpu->fpu.FYL2X();
}

void common_FPTAN(CPU* cpu) {
    cpu->fpu.FPTAN();    
}

void common_FPATAN(CPU* cpu) {
    cpu->fpu.FPATAN();
}

void common_FXTRACT(CPU* cpu) {
    cpu->fpu.FXTRACT();
}

void common_FPREM(CPU* cpu) {
    cpu->fpu.FPREM();
}

void common_FPREM_nearest(CPU* cpu) {
    cpu->fpu.FPREM1();
}

void common_FDECSTP(CPU* cpu) {
    cpu->fpu.FDECSTP();
}

void common_FINCSTP(CPU* cpu) {
    cpu->fpu.FINCSTP();
}

void common_FYL2XP1(CPU* cpu) {
    cpu->fpu.FYL2XP1();
}

void common_FSQRT(CPU* cpu) {
    cpu->fpu.FSQRT();
}

void common_FSINCOS(CPU* cpu) {
    cpu->fpu.FSINCOS();
}

void common_FRNDINT(CPU* cpu) {
    cpu->fpu.FRNDINT();
}

void common_FSCALE(CPU* cpu) {
    cpu->fpu.FSCALE();
}

void common_FSIN(CPU* cpu) {
    cpu->fpu.FSIN();
}

void common_FCOS(CPU* cpu) {
    cpu->fpu.FCOS();
}

void common_FST_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FST_STi_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
}

void common_FIADD_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FADD_EA();
}

void common_FIMUL_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FMUL_EA();
}

void common_FICOM_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
}

void common_FICOM_DWORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
    cpu->fpu.FPOP();
}

void common_FISUB_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FSUB_EA();
}

void common_FISUBR_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
}

void common_FIDIV_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FDIV_EA();
}

void common_FIDIVR_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
}

void common_FCMOV_ST0_STj_CF(CPU* cpu, U32 reg) {
    if (cpu->getCF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_ZF(CPU* cpu, U32 reg) {
    if (cpu->getZF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_CF_OR_ZF(CPU* cpu, U32 reg) {
    if (cpu->getCF() || cpu->getZF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_PF(CPU* cpu, U32 reg) {
    if (cpu->getPF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_NCF(CPU* cpu, U32 reg) {
    if (!cpu->getCF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_NZF(CPU* cpu, U32 reg) {
    if (!cpu->getZF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_NCF_AND_NZF(CPU* cpu, U32 reg) {
    if (!cpu->getCF() && !cpu->getZF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FCMOV_ST0_STj_NPF(CPU* cpu, U32 reg) {
    if (!cpu->getPF())
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FUCOMPP(CPU* cpu) {
    cpu->fpu.FUCOM(cpu->fpu.STV(0), cpu->fpu.STV(1));
    cpu->fpu.FPOP();
    cpu->fpu.FPOP();
}

void common_FILD_DWORD_INTEGER(CPU* cpu, U32 address) {
    U32 value = readd(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));
}

void common_FISTTP32(CPU* cpu, U32 address) {
    cpu->fpu.FSTT_I32(cpu, address);
    cpu->fpu.FPOP();
}

void common_FIST_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FST_I32(cpu, address);
}

void common_FIST_DWORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_I32(cpu, address);
    cpu->fpu.FPOP();
}

void common_FLD_EXTENDED_REAL(CPU* cpu, U32 address) {
    U64 low = readq(address); // might generate PF, so do before we adjust the stack
    U32 high = readw(address + 8);
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_F80(low, high);
}

void common_FSTP_EXTENDED_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FST_F80(cpu, address);
    cpu->fpu.FPOP();
}

void common_FNCLEX(CPU* cpu) {
    cpu->fpu.FCLEX();
}

void common_FNINIT(CPU* cpu) {
    cpu->fpu.FINIT();
}

// Quiet compare
void common_FUCOMI_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FUCOMI_ST0_STj_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
}

// Signaling compare :TODO:
void common_FCOMI_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
}

void common_FCOMI_ST0_STj_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
}

void common_FADD_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FADD_EA();
}

void common_FMUL_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FMUL_EA();
}

void common_FCOM_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FCOM_EA();
}

void common_FCOM_DOUBLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FCOM_EA();
    cpu->fpu.FPOP();
}

void common_FSUB_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FSUB_EA();
}

void common_FSUBR_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
}

void common_FDIV_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FDIV_EA();
}

void common_FDIVR_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
}

void common_FADD_STi_ST0(CPU* cpu, U32 reg) {
    cpu->fpu.FADD(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FADD_STi_ST0_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FADD(cpu->fpu.STV(reg), cpu->fpu.STV(0));
    cpu->fpu.FPOP();
}

void common_FMUL_STi_ST0(CPU* cpu, U32 reg) {
    cpu->fpu.FMUL(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FMUL_STi_ST0_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FMUL(cpu->fpu.STV(reg), cpu->fpu.STV(0));
    cpu->fpu.FPOP();
}

void common_FSUBR_STi_ST0(CPU* cpu, U32 reg) {
    cpu->fpu.FSUBR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FSUBR_STi_ST0_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FSUBR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
    cpu->fpu.FPOP();
}

void common_FSUB_STi_ST0(CPU* cpu, U32 reg) {
    cpu->fpu.FSUB(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FSUB_STi_ST0_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FSUB(cpu->fpu.STV(reg), cpu->fpu.STV(0));
    cpu->fpu.FPOP();
}

void common_FDIVR_STi_ST0(CPU* cpu, U32 reg) {
    cpu->fpu.FDIVR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FDIVR_STi_ST0_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FDIVR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
    cpu->fpu.FPOP();
}

void common_FDIV_STi_ST0(CPU* cpu, U32 reg) {
    cpu->fpu.FDIV(cpu->fpu.STV(reg), cpu->fpu.STV(0));
}

void common_FDIV_STi_ST0_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FDIV(cpu->fpu.STV(reg), cpu->fpu.STV(0));
    cpu->fpu.FPOP();
}

void common_FLD_DOUBLE_REAL(CPU* cpu, U32 address) {
    U64 value = readq(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_F64(value, cpu->fpu.STV(0));
}

void common_FISTTP64(CPU* cpu, U32 address) {
    cpu->fpu.FSTT_I64(cpu, address);
    cpu->fpu.FPOP();
}

void common_FST_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FST_F64(cpu, address);
}

void common_FST_DOUBLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_F64(cpu, address);
    cpu->fpu.FPOP();
}

void common_FRSTOR(CPU* cpu, U32 address) {
    cpu->fpu.FRSTOR(cpu, address);
}

void common_FNSAVE(CPU* cpu, U32 address) {
    cpu->fpu.FSAVE(cpu, address);
}

void common_FNSTSW(CPU* cpu, U32 address) {
    writew(address, cpu->fpu.SW());
}

void common_FFREE_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FFREE_STi(cpu->fpu.STV(reg));
}

void common_FUCOM_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FUCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));    
}

void common_FUCOM_STi_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FUCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
}

void common_FIADD_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FADD_EA();
}

void common_FIMUL_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FMUL_EA();
}

void common_FICOM_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FCOM_EA();
}

void common_FICOM_WORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FCOM_EA();
    cpu->fpu.FPOP();
}

void common_FISUB_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FSUB_EA();
}

void common_FISUBR_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
}

void common_FIDIV_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FDIV_EA();
}

void common_FIDIVR_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
}

void common_FCOMPP(CPU* cpu) {
    cpu->fpu.FCOM(cpu->fpu.STV(0), cpu->fpu.STV(1));
    cpu->fpu.FPOP();
    cpu->fpu.FPOP();
}

void common_FILD_WORD_INTEGER(CPU* cpu, U32 address) {
    S16 value = (S16)readw(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I16(value, cpu->fpu.STV(0));
}

void common_FISTTP16(CPU* cpu, U32 address) {
    cpu->fpu.FSTT_I16(cpu, address);
    cpu->fpu.FPOP();
}

void common_FIST_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FST_I16(cpu, address);
}

void common_FIST_WORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_I16(cpu, address);
    cpu->fpu.FPOP();
}

void common_FBLD_PACKED_BCD(CPU* cpu, U32 address) {
    U8 value[10];
    readMemory(value, address, 10); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FBLD(value, cpu->fpu.STV(0));
}

void common_FILD_QWORD_INTEGER(CPU* cpu, U32 address) {
    U64 value = readq(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I64(value, cpu->fpu.STV(0));
}

void common_FILD_QWORD_INTEGER_32(CPU* cpu, U32 address) {
    U64 value = readq(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I64(value, cpu->fpu.STV(0));
}

void common_FBSTP_PACKED_BCD(CPU* cpu, U32 address) {
    cpu->fpu.FBST(cpu, address);
    cpu->fpu.FPOP();
}

void common_FISTP_QWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FST_I64(cpu, address);
    cpu->fpu.FPOP();
}

void common_FFREEP_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FFREE_STi(cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
}

void common_FNSTSW_AX(CPU* cpu) {
    AX = cpu->fpu.SW();
}
