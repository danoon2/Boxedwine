#include "boxedwine.h"

/* WATCHIT : ALWAYS UPDATE REGISTERS BEFORE AND AFTER USING THEM
            STATUS WORD =>	cpu->fpu.SET_TOP(fpu.top) BEFORE a read
            fpu.top=cpu->fpu.GET_TOP() after a write;
            */

void common_FADD_SINGLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FADD_EA();
#ifdef LOG_FPU
    flog("FADD %f + %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FMUL_SINGLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FMUL_EA();
#ifdef LOG_FPU
    flog("FMUL %f * %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOM_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FCOM %f  %f", cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOM_SINGLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FCOMP %f  %f", cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUB_SINGLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FSUB_EA();
#ifdef LOG_FPU
    flog("FSUB %f - %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUBR_SINGLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
#ifdef LOG_FPU
    flog("FSUBR %f - %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIV_SINGLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FDIV_EA();
#ifdef LOG_FPU
    flog("FDIV %f / %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIVR_SINGLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F32_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
#ifdef LOG_FPU
    flog("FDIVR %f / %f = %f", cpu->fpu.regs[8].d, d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FADD_ST0_STj(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FADD(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FADD %f + %f = %f", d1, d2, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FMUL_ST0_STj(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FMUL(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FMUL %f * %f = %f", d1, d2, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOM_STi(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FCOM %f %f", d1, d2);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOM_STi_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FCOMP %f %f", d1, d2);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUB_ST0_STj(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FSUB(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FSUB %f - %f = %f", d1, d2, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUBR_ST0_STj(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FSUBR(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FSUBR %f - %f = %f", d2, d1, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIV_ST0_STj(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FDIV(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FDIV %f / %f = %f", d1, d2, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIVR_ST0_STj(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d1 = cpu->fpu.regs[cpu->fpu.STV(0)].d;
    double d2 = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FDIVR(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FDIVR %f / %f = %f", d2, d1, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLD_SINGLE_REAL(CPU* cpu,U32 address) {
    U32 value = cpu->memory->readd(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_F32(value, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FLD F32 %f (%.08X)", cpu->fpu.regs[cpu->fpu.STV(0)].d, value);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FST_SINGLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FST_F32(cpu, address);
#ifdef LOG_FPU
    flog("FST F32 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FST_SINGLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_F32(cpu, address);
#ifdef LOG_FPU
    flog("FSTP F32 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);    
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDENV(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    flog("FLDENV @%.08X", address);
#endif
    cpu->fpu.FLDENV(cpu, address);
}

void common_FLDCW(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    flog("FLDCW @%.08X", address);
#endif
    cpu->fpu.FLDCW(cpu, address);
}

void common_FNSTENV(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    flog("FSTENV @%.08X", address);
#endif
    cpu->fpu.FSTENV(cpu, address);
}

void common_FNSTCW(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    flog("FNSTCW %.04X @%.08X", cpu->fpu.CW(), address);
#endif
    cpu->memory->writew(address, cpu->fpu.CW());
}

void common_FLD_STi(CPU* cpu, U32 reg) {
    int reg_from = cpu->fpu.STV(reg);
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FST(reg_from, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FLD ST(%d)", reg);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FXCH_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FXCH(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FXCH ST(%d)", reg);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FNOP(CPU* cpu) {
#ifdef LOG_FPU
    flog("FNOP");
#endif
}

void common_FCHS(CPU* cpu) {
    cpu->fpu.FCHS();
#ifdef LOG_FPU
    flog("FCHS");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FABS(CPU* cpu) {
    cpu->fpu.FABS();
#ifdef LOG_FPU
    flog("FABS");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FTST(CPU* cpu) {
    cpu->fpu.FTST();
#ifdef LOG_FPU
    flog("FTST = FCOM ST(0), 0.0");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FXAM(CPU* cpu) {
    cpu->fpu.FXAM();
#ifdef LOG_FPU
    flog("FXAM");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLD1(CPU* cpu) {
    cpu->fpu.FLD1();
#ifdef LOG_FPU
    flog("FLD1");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDL2T(CPU* cpu) {
    cpu->fpu.FLDL2T();
#ifdef LOG_FPU
    flog("FLDL2T");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDL2E(CPU* cpu) {
    cpu->fpu.FLDL2E();
#ifdef LOG_FPU
    flog("FLDL2E");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDPI(CPU* cpu) {
    cpu->fpu.FLDPI();
#ifdef LOG_FPU
    flog("FLDPI");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDLG2(CPU* cpu) {
    cpu->fpu.FLDLG2();
#ifdef LOG_FPU
    flog("FLDLG2");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDLN2(CPU* cpu) {
    cpu->fpu.FLDLN2();
#ifdef LOG_FPU
    flog("FLDLN2");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLDZ(CPU* cpu) {
    cpu->fpu.FLDZ();
#ifdef LOG_FPU
    flog("FLDZ");
    cpu->fpu.LOG_STACK();
#endif
}

void common_F2XM1(CPU* cpu) {
    cpu->fpu.F2XM1();
#ifdef LOG_FPU
    flog("F2XM1");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FYL2X(CPU* cpu) {
    cpu->fpu.FYL2X();
#ifdef LOG_FPU
    flog("FYL2X");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FPTAN(CPU* cpu) {
    cpu->fpu.FPTAN();    
#ifdef LOG_FPU
    flog("FPTAN");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FPATAN(CPU* cpu) {
    cpu->fpu.FPATAN();
#ifdef LOG_FPU
    flog("FPATAN");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FXTRACT(CPU* cpu) {
    cpu->fpu.FXTRACT();
#ifdef LOG_FPU
    flog("FXTRACT");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FPREM(CPU* cpu) {
    cpu->fpu.FPREM();
#ifdef LOG_FPU
    flog("FPREM");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FPREM_nearest(CPU* cpu) {
    cpu->fpu.FPREM1();
#ifdef LOG_FPU
    flog("FPREM1");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDECSTP(CPU* cpu) {
    cpu->fpu.FDECSTP();
#ifdef LOG_FPU
    flog("FDECSTP");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FINCSTP(CPU* cpu) {
    cpu->fpu.FINCSTP();
#ifdef LOG_FPU
    flog("FINCSTP");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FYL2XP1(CPU* cpu) {
    cpu->fpu.FYL2XP1();
#ifdef LOG_FPU
    flog("FYL2XP1");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSQRT(CPU* cpu) {
    cpu->fpu.FSQRT();
#ifdef LOG_FPU
    flog("FSQRT");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSINCOS(CPU* cpu) {
    cpu->fpu.FSINCOS();
#ifdef LOG_FPU
    flog("FSINCOS");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FRNDINT(CPU* cpu) {
    cpu->fpu.FRNDINT();
#ifdef LOG_FPU
    flog("FRNDINT");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSCALE(CPU* cpu) {
    cpu->fpu.FSCALE();
#ifdef LOG_FPU
    flog("FSCALE");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSIN(CPU* cpu) {
    cpu->fpu.FSIN();
#ifdef LOG_FPU
    flog("FSIN");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOS(CPU* cpu) {
    cpu->fpu.FCOS();
#ifdef LOG_FPU
    flog("FCOS");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FST_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FST ST(reg)");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FST_STi_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FST(cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FSTP ST(reg)");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIADD_DWORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FADD_EA();
#ifdef LOG_FPU
    flog("FIADD %f + %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readd(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIMUL_DWORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FMUL_EA();
#ifdef LOG_FPU
    flog("FIMUL %f * %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readd(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FICOM_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FICOM %f  %f (%X)", cpu->fpu.regs[8].d, cpu->memory->readd(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FICOM_DWORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FICOMP %f  %f (%X)", cpu->fpu.regs[8].d, cpu->memory->readd(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISUB_DWORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FSUB_EA();
#ifdef LOG_FPU
    flog("FISUB %f - %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readd(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISUBR_DWORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
#ifdef LOG_FPU
    flog("FISUBR %f (%X) - %f = %f", cpu->fpu.regs[8].d, cpu->memory->readd(address), d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIDIV_DWORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FDIV_EA();
#ifdef LOG_FPU
    flog("FIDIV %f / %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readd(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIDIVR_DWORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I32_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
#ifdef LOG_FPU
    flog("FIDIVR %f (%X) / %f = %f", cpu->fpu.regs[8].d, cpu->memory->readd(address), d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCMOV_ST0_STj_CF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV cf ST(%d)", reg);
#endif
    if (cpu->getCF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_ZF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV zf ST(%d)", reg);
#endif
    if (cpu->getZF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_CF_OR_ZF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV cf or zf ST(%d)", reg);
#endif
    if (cpu->getCF() || cpu->getZF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_PF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV pf ST(%d)", reg);
#endif
    if (cpu->getPF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_NCF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV !cf ST(%d)", reg);
#endif
    if (!cpu->getCF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_NZF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV !zf ST(%d)", reg);
#endif
    if (!cpu->getZF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_NCF_AND_NZF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV !cf and !zf ST(%d)", reg);
#endif
    if (!cpu->getCF() && !cpu->getZF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FCMOV_ST0_STj_NPF(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCMOV !pf ST(%d)", reg);
#endif
    if (!cpu->getPF()) {
        cpu->fpu.FST(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
        cpu->fpu.LOG_STACK();
#endif
    }
}

void common_FUCOMPP(CPU* cpu) {
#ifdef LOG_FPU
    flog("FUCOMPP %f %f", cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(1)].d);
#endif
    cpu->fpu.FUCOM(cpu->fpu.STV(0), cpu->fpu.STV(1));
    cpu->fpu.FPOP();
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FILD_DWORD_INTEGER(CPU* cpu, U32 address) {
    U32 value = cpu->memory->readd(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I32(value, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FILD I32 %f (%d)", cpu->fpu.regs[cpu->fpu.STV(0)].d, value);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISTTP32(CPU* cpu, U32 address) {
    cpu->fpu.FSTT_I32(cpu, address);
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FISTTP32 %d @%.08X", cpu->memory->readd(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIST_DWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FST_I32(cpu, address);
#ifdef LOG_FPU
    flog("FIST I32 %d @%.08X", cpu->memory->readd(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIST_DWORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_I32(cpu, address);
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FISTP I32 %d @%.08X", cpu->memory->readd(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLD_EXTENDED_REAL(CPU* cpu, U32 address) {
    U64 low = cpu->memory->readq(address); // might generate PF, so do before we adjust the stack
    U32 high = cpu->memory->readw(address + 8);
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_F80(low, high);
#ifdef LOG_FPU
    flog("FLD 80 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSTP_EXTENDED_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    flog("FSTP 80 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
#endif
    cpu->fpu.FST_F80(cpu, address);
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FNCLEX(CPU* cpu) {
    cpu->fpu.FCLEX();
#ifdef LOG_FPU
    flog("FNCLEX");
    cpu->fpu.LOG_STACK();
#endif
}

void common_FNINIT(CPU* cpu) {
#ifdef LOG_FPU
    flog("FINIT");
#endif
    cpu->fpu.FINIT();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

// Quiet compare
void common_FUCOMI_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FUCOMI ST(%d) %f %f", reg, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FUCOMI_ST0_STj_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FUCOMIP ST(%d) %f %f", reg, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

// Signaling compare :TODO:
void common_FCOMI_ST0_STj(CPU* cpu, U32 reg) {
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FCOMI ST(%d) %f %f", reg, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOMI_ST0_STj_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    flog("FCOMIP ST(%d) %f %f", reg, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FCOMI(cpu, cpu->fpu.STV(0), cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FADD_DOUBLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FADD_EA();
#ifdef LOG_FPU
    flog("FADD %f + %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FMUL_DOUBLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FMUL_EA();
#ifdef LOG_FPU
    flog("FMUL %f * %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOM_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FCOM %f %f", cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOM_DOUBLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FCOMP %f %f", cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUB_DOUBLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FSUB_EA();
#ifdef LOG_FPU
    flog("FSUB %f - %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUBR_DOUBLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
#ifdef LOG_FPU
    flog("FSUBR %f - %f = %f", cpu->fpu.regs[8].d, d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIV_DOUBLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FDIV_EA();
#ifdef LOG_FPU
    flog("FDIV %f / %f = %f", d, cpu->fpu.regs[8].d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIVR_DOUBLE_REAL(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_F64_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
#ifdef LOG_FPU
    flog("FDIVR %f / %f = %f", cpu->fpu.regs[8].d, d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FADD_STi_ST0(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FADD(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FADD %f + %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FADD_STi_ST0_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FADD(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FADDP %f + %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FMUL_STi_ST0(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FMUL(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FMUL %f * %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FMUL_STi_ST0_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FMUL(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FMULP %f * %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUBR_STi_ST0(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FSUBR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FSUBR %f - %f = %f", cpu->fpu.regs[cpu->fpu.STV(0)].d, 3, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUBR_STi_ST0_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FSUBR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FSUBR %f - %f = %f", cpu->fpu.regs[cpu->fpu.STV(0)].d, d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUB_STi_ST0(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FSUB(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FSUB %f - %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FSUB_STi_ST0_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FSUB(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FSUB %f - %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIVR_STi_ST0(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FDIVR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FDIVR %f / %f = %f", cpu->fpu.regs[cpu->fpu.STV(0)].d, d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIVR_STi_ST0_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FDIVR(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FDIVR %f / %f = %f", cpu->fpu.regs[cpu->fpu.STV(0)].d, d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIV_STi_ST0(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FDIV(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FDIV %f / %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FDIV_STi_ST0_Pop(CPU* cpu, U32 reg) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(reg)].d;
#endif
    cpu->fpu.FDIV(cpu->fpu.STV(reg), cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FDIV %f / %f = %f", d, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FLD_DOUBLE_REAL(CPU* cpu, U32 address) {
    U64 value = cpu->memory->readq(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_F64(value, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FLD F64 %f %" PRIx64 "@%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, value, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISTTP64(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FSTT_I64(cpu, address);
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FISTTP64 %f -> %" PRIx64 " @%.08X", d, cpu->memory->readq(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FST_DOUBLE_REAL(CPU* cpu, U32 address) {
    cpu->fpu.FST_F64(cpu, address);
#ifdef LOG_FPU
    flog("FST F64 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FST_DOUBLE_REAL_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_F64(cpu, address);
#ifdef LOG_FPU
    flog("FSTP F64 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FRSTOR(CPU* cpu, U32 address) {
    cpu->fpu.FRSTOR(cpu, address);
#ifdef LOG_FPU
    flog("FRSTOR @%.08X", address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FNSAVE(CPU* cpu, U32 address) {
    cpu->fpu.FSAVE(cpu, address);
#ifdef LOG_FPU
    flog("FNSAVE @%.08X", address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FNSTSW(CPU* cpu, U32 address) {
    cpu->memory->writew(address, cpu->fpu.SW());
#ifdef LOG_FPU
    flog("FNSTSW %X @%.08X", cpu->fpu.SW(), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FFREE_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FFREE_STi(cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FFREE ST(%d)", reg);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FUCOM_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FUCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));    
#ifdef LOG_FPU
    flog("FUCOM ST(%d) %f %f", reg, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FUCOM_STi_Pop(CPU* cpu, U32 reg) {
    cpu->fpu.FUCOM(cpu->fpu.STV(0), cpu->fpu.STV(reg));
#ifdef LOG_FPU
    flog("FUCOMP ST(%d) %f %f", reg, cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(reg)].d);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIADD_WORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FADD_EA();
#ifdef LOG_FPU
    flog("FIADD %f + %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readw(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIMUL_WORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FMUL_EA();
#ifdef LOG_FPU
    flog("FIMUL %f * %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readw(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FICOM_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FICOM %f %f (%X)", cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[8].d, cpu->memory->readw(address));
    cpu->fpu.LOG_STACK();
#endif
}

void common_FICOM_WORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FCOM_EA();
#ifdef LOG_FPU
    flog("FICOM %f %f (%X)", cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[8].d, cpu->memory->readw(address));
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISUB_WORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FSUB_EA();
#ifdef LOG_FPU
    flog("FISUB %f - %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readw(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISUBR_WORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FSUBR_EA();
#ifdef LOG_FPU
    flog("FISUBR %f (%X) - f = %f", cpu->fpu.regs[8].d, cpu->memory->readw(address), d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIDIV_WORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FDIV_EA();
#ifdef LOG_FPU
    flog("FIDIV %f / %f (%X) = %f", d, cpu->fpu.regs[8].d, cpu->memory->readw(address), cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIDIVR_WORD_INTEGER(CPU* cpu, U32 address) {
#ifdef LOG_FPU
    double d = cpu->fpu.regs[cpu->fpu.STV(0)].d;
#endif
    cpu->fpu.FLD_I16_EA(cpu, address);
    cpu->fpu.FDIVR_EA();
#ifdef LOG_FPU
    flog("FIDIVR %f (%X) / f = %f", cpu->fpu.regs[8].d, cpu->memory->readw(address), d, cpu->fpu.regs[cpu->fpu.STV(0)].d);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FCOMPP(CPU* cpu) {
    cpu->fpu.FCOM(cpu->fpu.STV(0), cpu->fpu.STV(1));
#ifdef LOG_FPU
    flog("FCOMPP %f  %f", cpu->fpu.regs[cpu->fpu.STV(0)].d, cpu->fpu.regs[cpu->fpu.STV(1)].d);
#endif
    cpu->fpu.FPOP();
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FILD_WORD_INTEGER(CPU* cpu, U32 address) {
    S16 value = (S16)cpu->memory->readw(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I16(value, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FILD I16 %f (%d) @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, value, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISTTP16(CPU* cpu, U32 address) {
    cpu->fpu.FSTT_I16(cpu, address);
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FISTTP16 %d @%.08X", cpu->memory->readw(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIST_WORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FST_I16(cpu, address);
#ifdef LOG_FPU
    flog("FIST I16 %d @%.08X", cpu->memory->readw(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FIST_WORD_INTEGER_Pop(CPU* cpu, U32 address) {
    cpu->fpu.FST_I16(cpu, address);
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FISTP I16 %d @%.08X", cpu->memory->readw(address), address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FBLD_PACKED_BCD(CPU* cpu, U32 address) {
    U8 value[10];
    cpu->memory->memcpy(value, address, 10); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FBLD(value, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FBLD %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FILD_QWORD_INTEGER(CPU* cpu, U32 address) {
    U64 value = cpu->memory->readq(address); // might generate PF, so do before we adjust the stack
    cpu->fpu.PREP_PUSH();
    cpu->fpu.FLD_I64(value, cpu->fpu.STV(0));
#ifdef LOG_FPU
    flog("FILD I64 %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FBSTP_PACKED_BCD(CPU* cpu, U32 address) {
    cpu->fpu.FBST(cpu, address);
#ifdef LOG_FPU
    flog("FBSTP %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FISTP_QWORD_INTEGER(CPU* cpu, U32 address) {
    cpu->fpu.FST_I64(cpu, address);
#ifdef LOG_FPU
    flog("FISTP %f @%.08X", cpu->fpu.regs[cpu->fpu.STV(0)].d, address);
#endif
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    cpu->fpu.LOG_STACK();
#endif
}

void common_FFREEP_STi(CPU* cpu, U32 reg) {
    cpu->fpu.FFREE_STi(cpu->fpu.STV(reg));
    cpu->fpu.FPOP();
#ifdef LOG_FPU
    flog("FFREEP ST(%d)", reg);
    cpu->fpu.LOG_STACK();
#endif
}

void common_FNSTSW_AX(CPU* cpu) {
    AX = cpu->fpu.SW();
#ifdef LOG_FPU
    flog("FNSTSW SW=%X", cpu->fpu.SW());
    cpu->fpu.LOG_STACK();
#endif
}
