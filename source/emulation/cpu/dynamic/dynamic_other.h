#include "../common/common_other.h"
void OPCALL dynamic_bound16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_bound16, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bound32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_bound32, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_daa(CPU* cpu, DecodedOp* op) {
    callHostFunction(daa, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_das(CPU* cpu, DecodedOp* op) {
    callHostFunction(das, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aaa(CPU* cpu, DecodedOp* op) {
    callHostFunction(aaa, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aas(CPU* cpu, DecodedOp* op) {
    callHostFunction(aas, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aad(CPU* cpu, DecodedOp* op) {
    callHostFunction(aad, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aam(CPU* cpu, DecodedOp* op) {
    callHostFunction(aam, true, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_nop(CPU* cpu, DecodedOp* op) {
    // Nop
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_done(CPU* cpu, DecodedOp* op) {
    blockDone();
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_wait(CPU* cpu, DecodedOp* op) {
    // Wait
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cwd(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cwd, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cwq(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cwq, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_callAp(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone();
}
void OPCALL dynamic_callFar(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone();
}
void OPCALL dynamic_jmpAp(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone();
}
void OPCALL dynamic_jmpFar(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
     callHostFunction(common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone();
}
void OPCALL dynamic_retf16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_SRC, DYN_32bit, true);
    callHostFunction(common_ret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone();
}
void OPCALL dynamic_retf32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_SRC, DYN_32bit, true);
    callHostFunction(common_ret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone();
}
void OPCALL dynamic_iret(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_iret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone();
}
void OPCALL dynamic_iret32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_iret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone();
}
void OPCALL dynamic_sahf(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_fillFlags, false, 1, 0, DYN_PARAM_CPU, false);
    callHostFunction(common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, CPU_OFFSET_OF(reg[0].h8), DYN_PARAM_CPU_ADDRESS_8, false, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lahf(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_fillFlags, false, 1, 0, DYN_PARAM_CPU, false);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);
    instRegImm('&', DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].h8), DYN_SRC, DYN_8bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_salc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_salc, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_retn16Iw(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u16), DYN_16bit);
    instRegImm('+', DYN_SRC, DYN_16bit, op->imm);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u16), DYN_SRC, DYN_16bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_retn32Iw(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->imm);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_retn16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_retn32(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_invalid(CPU* cpu, DecodedOp* op) {
    kpanic("Dyn:Invalid instruction %x\n", op->inst);
}
void OPCALL dynamic_int80(CPU* cpu, DecodedOp* op) {
    callHostFunction(ksyscall, false, 2, 0, DYN_PARAM_CPU, false, op->len, DYN_PARAM_CONST_32, false);
    blockDone();
}
void OPCALL dynamic_int98(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_int98, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_int99(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_int99, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_intIb(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_intIb, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone();
}
void OPCALL dynamic_xlat(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u8), DYN_8bit);
    if (op->ea16) {
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[3].u16), DYN_16bit);
        movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false);
        instRegReg('+', DYN_DEST, DYN_SRC, DYN_16bit, true);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false);
    } else {
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(reg[3].u32), DYN_32bit);
        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
        instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit, true);
    }
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(seg[op->base].address), DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit, true);
    movFromMem(DYN_8bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u8), DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_hlt(CPU* cpu, DecodedOp* op) {
    kpanic("Dyn:Hlt");
}
void OPCALL dynamic_cmc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cmc, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_clc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_clc, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_stc, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cli(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cli, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sti(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_sti, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cld(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cld, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_std(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_std, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rdtsc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_rdtsc, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cpuid(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cpuid, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_enter16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_enter, false, 4, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_enter32(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_enter, false, 4, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->reg, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_leave16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(reg[4].u16), CPU_OFFSET_OF(reg[5].u16), DYN_16bit, DYN_SRC, true);
    callHostFunction(common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[5].u16), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_leave32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(reg[4].u32), CPU_OFFSET_OF(reg[5].u32), DYN_32bit, DYN_SRC, true);
    callHostFunction(common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[5].u32), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_loopnz(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_loopnz, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    } else {
        callHostFunction(common_loopnz32, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    }
}
void OPCALL dynamic_loopz(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_loopz, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    } else {
        callHostFunction(common_loopz32, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    }
}
void OPCALL dynamic_loop(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_loop, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    } else {
        callHostFunction(common_loop32, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    }
}
void OPCALL dynamic_jcxz(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_jcxz, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    } else {
        callHostFunction(common_jcxz32, false, 3, 0, DYN_PARAM_CPU, false, op->imm+op->len, DYN_PARAM_CONST_32, false, op->len, DYN_PARAM_CONST_32, false);
    }
}
void OPCALL dynamic_InAlIb(CPU* cpu, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u8), DYN_8bit, 0xFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InAxIb(CPU* cpu, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u16), DYN_16bit, 0xFFFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InEaxIb(CPU* cpu, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u32), DYN_32bit, 0xFFFFFFFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_OutIbAl(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_OutIbAx(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_OutIbEax(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InAlDx(CPU* cpu, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u8), DYN_8bit, 0xFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InAxDx(CPU* cpu, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u16), DYN_16bit, 0xFFFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InEaxDx(CPU* cpu, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u32), DYN_32bit, 0xFFFFFFFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_OutDxAl(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_OutDxAx(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_OutDxEax(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_callJw(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len+(S32)((S16)op->imm));
    blockNext1();
}
void OPCALL dynamic_callJd(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len+(S32)op->imm);
    blockNext1();
}
void OPCALL dynamic_jmp8(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len+(S32)((S8)op->imm));
    blockNext1();
}
void OPCALL dynamic_jmp16(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len+(S32)((S16)op->imm));
    blockNext1();
}
void OPCALL dynamic_jmp32(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len+(S32)op->imm);
    blockNext1();
}
void OPCALL dynamic_callR16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false); movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_SRC, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_callR32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    movToCpuFromCpu(CPU_OFFSET_OF(eip.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_SRC, true); blockDone();
}
void OPCALL dynamic_callE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_16, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_callE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_jmpR16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].u16), DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_SRC, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_jmpR32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(eip.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_SRC, true); blockDone();
}
void OPCALL dynamic_jmpE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_jmpE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); blockDone();
}
void OPCALL dynamic_callFarE16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); blockDone();
}
void OPCALL dynamic_callFarE32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); blockDone();
}
void OPCALL dynamic_jmpFarE16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); blockDone();
}
void OPCALL dynamic_jmpFarE32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); blockDone();
}
void OPCALL dynamic_larr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_larr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_larr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_larr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lslr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_lslr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lslr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_lslr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xaddr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(src.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_SRC, false);
    movToCpuFromCpu(CPU_OFFSET_OF(dst.u32), CPU_OFFSET_OF(reg[op->rm].u32), DYN_32bit, DYN_DEST, false);
    instRegReg('+', DYN_SRC, DYN_DEST, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_SRC, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->rm].u32), DYN_SRC, DYN_32bit, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xaddr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromCpu(CPU_OFFSET_OF(src.u32), CPU_OFFSET_OF(reg[op->reg].u32), DYN_32bit, DYN_SRC, false);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToCpuFromReg(CPU_OFFSET_OF(dst.u32), DYN_CALL_RESULT, DYN_32bit, false);
    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_SRC, DYN_32bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_CALL_RESULT, DYN_32bit, true);
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
    movToCpu(CPU_OFFSET_OF(lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bswap32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].byte[0]), DYN_8bit);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].byte[0]), CPU_OFFSET_OF(reg[op->reg].byte[3]), DYN_8bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].byte[3]), DYN_SRC, DYN_8bit, true);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[op->reg].byte[1]), DYN_8bit);
    movToCpuFromCpu(CPU_OFFSET_OF(reg[op->reg].byte[1]), CPU_OFFSET_OF(reg[op->reg].byte[2]), DYN_8bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].byte[2]), DYN_SRC, DYN_8bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpxchgg8b(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_cmpxchgg8b, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_loadSegment16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u16), DYN_DEST, DYN_16bit, true);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_loadSegment32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone();
    endIf();
    movToCpuFromReg(CPU_OFFSET_OF(reg[op->reg].u32), DYN_DEST, DYN_32bit, true);
    INCREMENT_EIP(op->len);
}
