#include "../common/common_other.h"
void OPCALL dynamic_bound16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_bound16, true, false, true, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bound32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_bound32, true, false, true, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_daa(CPU* cpu, DecodedOp* op) {
    callHostFunction(daa, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_das(CPU* cpu, DecodedOp* op) {
    callHostFunction(das, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aaa(CPU* cpu, DecodedOp* op) {
    callHostFunction(aaa, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aas(CPU* cpu, DecodedOp* op) {
    callHostFunction(aas, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aad(CPU* cpu, DecodedOp* op) {
    callHostFunction(aad, false, false, false, 2, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_aam(CPU* cpu, DecodedOp* op) {
    callHostFunction(aam, true, false, true, 2, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_nop(CPU* cpu, DecodedOp* op) {
    // Nop
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_done(CPU* cpu, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_wait(CPU* cpu, DecodedOp* op) {
    // Wait
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cwd(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cwd, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cwq(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cwq, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_callAp(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
}
void OPCALL dynamic_callFar(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
}
void OPCALL dynamic_jmpAp(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
}
void OPCALL dynamic_jmpFar(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
     callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->disp, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
}
void OPCALL dynamic_retf16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);
    callHostFunction(common_ret, false, false, false, 3, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
}
void OPCALL dynamic_retf32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);
    callHostFunction(common_ret, false, false, false, 3, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32);
}
void OPCALL dynamic_iret(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_iret, false, false, false, 3, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
}
void OPCALL dynamic_iret32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_iret, false, false, false, 3, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, DYN_SRC, DYN_PARAM_REG_32);
}
void OPCALL dynamic_sahf(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_fillFlags, false, false, false, 1, 0, DYN_PARAM_CPU);
    callHostFunction(common_setFlags, false, false, false, 3, 0, DYN_PARAM_CPU, offsetof(CPU, reg[0].h8), DYN_PARAM_CPU_ADDRESS_8, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lahf(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_fillFlags, false, false, false, 1, 0, DYN_PARAM_CPU);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, flags), DYN_32bit);
    instRegImm('&', DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    movToCpuFromReg(offsetof(CPU, reg[0].h8), DYN_SRC, DYN_8bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_salc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_salc, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_retn16Iw(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop16, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[4].u16), DYN_16bit);
    instRegImm('+', DYN_SRC, DYN_16bit, op->imm);
    movToCpuFromReg(offsetof(CPU, reg[4].u16), DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);
}
void OPCALL dynamic_retn32Iw(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop32, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[4].u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->imm);
    movToCpuFromReg(offsetof(CPU, reg[4].u32), DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);
}
void OPCALL dynamic_retn16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop16, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);
}
void OPCALL dynamic_retn32(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_pop32, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_CALL_RESULT, DYN_32bit);
}
void OPCALL dynamic_invalid(CPU* cpu, DecodedOp* op) {
    kpanic("Dyn:Invalid instruction %x\n", op->inst);
}
void OPCALL dynamic_int80(CPU* cpu, DecodedOp* op) {
    callHostFunction(ksyscall, false, false, false, 2, 0, DYN_PARAM_CPU, op->len, DYN_PARAM_CONST_32);
}
void OPCALL dynamic_int98(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_int98, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_int99(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_int99, false, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_intIb(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_intIb, false, false, false, 1, 0, DYN_PARAM_CPU);
}
void OPCALL dynamic_xlat(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[0].u8), DYN_8bit);
    if (op->ea16) {
        movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[3].u16), DYN_16bit);
        movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit);
        instRegReg('+', DYN_DEST, DYN_SRC, DYN_16bit);
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit);
    } else {
        movToRegFromCpu(DYN_DEST, offsetof(CPU, reg[3].u32), DYN_32bit);
        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit);
        instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);
    }
    movToRegFromCpu(DYN_SRC, offsetof(CPU, seg[op->base].address), DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit);
    movFromMem(DYN_8bit, DYN_DEST);
    movToCpuFromReg(offsetof(CPU, reg[0].u8), DYN_READ_RESULT, DYN_8bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_hlt(CPU* cpu, DecodedOp* op) {
    kpanic("Dyn:Hlt");
}
void OPCALL dynamic_cmc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cmc, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_clc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_clc, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_stc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_stc, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cli(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cli, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_sti(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_sti, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cld(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cld, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_std(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_std, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_rdtsc(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_rdtsc, true, false, false, 2, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cpuid(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_cpuid, true, false, false, 1, 0, DYN_PARAM_CPU);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_enter16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_enter, true, false, false, 4, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_enter32(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_enter, true, false, false, 4, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, op->imm, DYN_PARAM_CONST_32, op->reg, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_leave16(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, reg[4].u16), offsetof(CPU, reg[5].u16), DYN_16bit, DYN_SRC);
    callHostFunction(common_pop16, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, reg[5].u16), DYN_CALL_RESULT, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_leave32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, reg[4].u32), offsetof(CPU, reg[5].u32), DYN_32bit, DYN_SRC);
    callHostFunction(common_pop32, true, false, false, 1, 0, DYN_PARAM_CPU);
    movToCpuFromReg(offsetof(CPU, reg[5].u32), DYN_CALL_RESULT, DYN_32bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_loopnz(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_loopnz, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    } else {
        callHostFunction(common_loopnz32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    }
}
void OPCALL dynamic_loopz(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_loopz, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    } else {
        callHostFunction(common_loopz32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    }
}
void OPCALL dynamic_loop(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_loop, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    } else {
        callHostFunction(common_loop32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    }
}
void OPCALL dynamic_jcxz(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {
        callHostFunction(common_jcxz, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    } else {
        callHostFunction(common_jcxz32, false, false, false, 3, 0, DYN_PARAM_CPU, op->imm+op->len, DYN_PARAM_CONST_32, op->len, DYN_PARAM_CONST_32);
    }
}
void OPCALL dynamic_InAlIb(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[0].u8), DYN_8bit, 0xFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InAxIb(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[0].u16), DYN_16bit, 0xFFFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InEaxIb(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[0].u32), DYN_32bit, 0xFFFFFFFF);
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
    movToCpu(offsetof(CPU, reg[0].u8), DYN_8bit, 0xFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InAxDx(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[0].u16), DYN_16bit, 0xFFFF);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_InEaxDx(CPU* cpu, DecodedOp* op) {
    movToCpu(offsetof(CPU, reg[0].u32), DYN_32bit, 0xFFFFFFFF);
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
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len+(S32)((S16)op->imm));
}
void OPCALL dynamic_callJd(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len+(S32)op->imm);
}
void OPCALL dynamic_jmp8(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len+(S32)((S8)op->imm));
}
void OPCALL dynamic_jmp16(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len+(S32)((S16)op->imm));
}
void OPCALL dynamic_jmp32(CPU* cpu, DecodedOp* op) {
    INCREMENT_EIP(op->len+(S32)op->imm);
}
void OPCALL dynamic_callR16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit); movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);
}
void OPCALL dynamic_callR32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);
    movToCpuFromCpu(offsetof(CPU, eip.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
}
void OPCALL dynamic_callE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push16, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_16);
    movToRegFromReg(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);
}
void OPCALL dynamic_callE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(common_push32, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_SRC, DYN_PARAM_REG_32);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);
}
void OPCALL dynamic_jmpR16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].u16), DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_SRC, DYN_32bit);
}
void OPCALL dynamic_jmpR32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, eip.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
}
void OPCALL dynamic_jmpE16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    movToRegFromReg(DYN_READ_RESULT, DYN_32bit, DYN_READ_RESULT, DYN_16bit);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);
}
void OPCALL dynamic_jmpE32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    movToCpuFromReg(offsetof(CPU, eip.u32), DYN_READ_RESULT, DYN_32bit);
}
void OPCALL dynamic_callFarE16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_READ_RESULT, DYN_16bit);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_16, DYN_DEST, DYN_PARAM_REG_32);
}
void OPCALL dynamic_callFarE32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_READ_RESULT, DYN_32bit);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(common_call, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_32, DYN_DEST, DYN_PARAM_REG_32);
}
void OPCALL dynamic_jmpFarE16(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_READ_RESULT, DYN_16bit);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 0, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_16, DYN_DEST, DYN_PARAM_REG_32);
}
void OPCALL dynamic_jmpFarE32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, offsetof(CPU, eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_READ_RESULT, DYN_32bit);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(common_jmp, false, false, false, 5, 0, DYN_PARAM_CPU, 1, DYN_PARAM_CONST_32, DYN_READ_RESULT, DYN_PARAM_REG_16, DYN_SRC, DYN_PARAM_REG_32, DYN_DEST, DYN_PARAM_REG_32);
}
void OPCALL dynamic_larr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_larr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_larr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_larr16e16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lslr16r16(CPU* cpu, DecodedOp* op) {
    callHostFunction(common_lslr16r16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, op->rm, DYN_PARAM_CONST_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_lslr16e16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_lslr16e16, false, false, false, 3, 0, DYN_PARAM_CPU, op->reg, DYN_PARAM_CONST_32, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xaddr32r32(CPU* cpu, DecodedOp* op) {
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    movToCpuFromCpu(offsetof(CPU, dst.u32), offsetof(CPU, reg[op->rm].u32), DYN_32bit, DYN_DEST);
    instRegReg('+', DYN_SRC, DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->rm].u32), DYN_SRC, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_xaddr32e32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movToCpuFromCpu(offsetof(CPU, src.u32), offsetof(CPU, reg[op->reg].u32), DYN_32bit, DYN_SRC);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    movToCpuFromReg(offsetof(CPU, dst.u32), DYN_READ_RESULT, DYN_32bit);
    instRegReg('+', DYN_SRC, DYN_READ_RESULT, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, result.u32), DYN_SRC, DYN_32bit);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_READ_RESULT, DYN_32bit);
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit);
    movToCpu(offsetof(CPU, lazyFlags), Dyn_PtrSize, (DYN_PTR_SIZE)FLAGS_ADD32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_bswap32(CPU* cpu, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].byte[0]), DYN_8bit);
    movToCpuFromCpu(offsetof(CPU, reg[op->reg].byte[0]), offsetof(CPU, reg[op->reg].byte[3]), DYN_8bit, DYN_DEST);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].byte[3]), DYN_SRC, DYN_8bit);
    movToRegFromCpu(DYN_SRC, offsetof(CPU, reg[op->reg].byte[1]), DYN_8bit);
    movToCpuFromCpu(offsetof(CPU, reg[op->reg].byte[1]), offsetof(CPU, reg[op->reg].byte[2]), DYN_8bit, DYN_DEST);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].byte[2]), DYN_SRC, DYN_8bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_cmpxchgg8b(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction(common_cmpxchgg8b, false, false, false, 2, 0, DYN_PARAM_CPU, DYN_ADDRESS, DYN_PARAM_REG_32);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_loadSegment16(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    movToRegFromReg(DYN_DEST, DYN_16bit, DYN_READ_RESULT, DYN_16bit);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32,  DYN_READ_RESULT, DYN_PARAM_REG_16);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_DEST, DYN_16bit);
    INCREMENT_EIP(op->len);
}
void OPCALL dynamic_loadSegment32(CPU* cpu, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_READ_RESULT, DYN_32bit);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS);
    callHostFunction(common_setSegment, true, false, true, 3, 0, DYN_PARAM_CPU, op->imm, DYN_PARAM_CONST_32,  DYN_READ_RESULT, DYN_PARAM_REG_16);
    movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_DEST, DYN_32bit);
    INCREMENT_EIP(op->len);
}
