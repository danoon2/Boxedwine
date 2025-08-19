/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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

#include "../common/common_other.h"
void dynamic_bound16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bound16, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_bound32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bound32, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_daa(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)daa, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_das(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)das, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aaa(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)aaa, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aas(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)aas, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aad(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)aad, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aam(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)aam, true, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    INCREMENT_EIP(data, op);
}
void dynamic_nop(DynamicData* data, DecodedOp* op) {
    // Nop
    INCREMENT_EIP(data, op);
}
void dynamic_done(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op);
    blockDone(data, false);
}
void dynamic_wait(DynamicData* data, DecodedOp* op) {
    // Wait
    INCREMENT_EIP(data, op);
}
void dynamic_cwd(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u16), DYN_16bit);
    instRegImm('&', DYN_SRC, DYN_16bit, 0x8000);
    instRegImm(')', DYN_SRC, DYN_16bit, 15);
    movToCpuFromReg(CPU_OFFSET_OF(reg[2].u16), DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_cwq(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[0].u32), DYN_32bit);
    instRegImm('&', DYN_SRC, DYN_32bit, 0x80000000);
    instRegImm(')', DYN_SRC, DYN_32bit, 31);
    movToCpuFromReg(CPU_OFFSET_OF(reg[2].u32), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_callAp(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_callFar(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_jmpAp(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_jmpFar(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_retf16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone(data, false);
}
void dynamic_retf32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone(data, false);
}
void dynamic_iret(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_iret32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_sahf(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, CPU_OFFSET_OF(reg[0].h8), DYN_PARAM_CPU_ADDRESS_8, false, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_lahf(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(flags), DYN_32bit);
    instRegImm('&', DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);
    instRegImm('|', DYN_SRC, DYN_32bit, 2);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].h8), DYN_SRC, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_salc(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
    instReg('-', DYN_CALL_RESULT, DYN_32bit);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u8), DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_retn16Iw(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u16), DYN_16bit);
    instRegImm('+', DYN_SRC, DYN_16bit, op->imm);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u16), DYN_SRC, DYN_16bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); 
    blockDone(data, false);
}
void dynamic_retn32Iw(DynamicData* data, DecodedOp* op) {
    dynamic_pop32(data);
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->imm);
    movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_SRC, DYN_32bit, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); 
    blockDone(data, false);
}
void dynamic_retn16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); 
    blockDone(data, false);
}
void dynamic_retn32(DynamicData* data, DecodedOp* op) {
    dynamic_pop32(data);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); 
    blockDone(data, false);
}
void dynamic_invalid(DynamicData* data, DecodedOp* op) {
    kpanic_fmt("Dyn:Invalid instruction %x\n", op->inst);
}
void dynamic_ud2(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, true);
}
void dynamic_int80(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)ksyscall, false, 2, 0, DYN_PARAM_CPU, false, op->len, DYN_PARAM_CONST_32, false);
    if (!data->isFunction) {
        blockDone(data, false);
    } else {
        movToRegFromCpu(DYN_CALL_RESULT, CPU_OFFSET_OF(eip.u32), DYN_32bit);
        instRegImm('-', DYN_CALL_RESULT, DYN_32bit, data->currentEip + op->len);
        startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true, DYN_32bit, false);
        blockDone(data, true);
        endIf();
    }
}
void dynamic_int99(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_int99, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_int9A(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_int9A, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_int9B(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_int9B, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_intIb(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_intIb, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, false);
}
void dynamic_int3(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_int3, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, false);
}
void dynamic_xlat(DynamicData* data, DecodedOp* op) {
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
    movToRegFromCpu(DYN_SRC, CPU::offsetofSegAddress(op->base), DYN_32bit);
    instRegReg('+', DYN_DEST, DYN_SRC, DYN_32bit, true);
    movFromMem(DYN_8bit, DYN_DEST, true);
    movToCpuFromReg(CPU_OFFSET_OF(reg[0].u8), DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_hlt(DynamicData* data, DecodedOp* op) {
    kpanic("Dyn:Hlt");
}
void dynamic_cmc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    instCPUImm('^', CPU_OFFSET_OF(flags), DYN_32bit, CF);
    INCREMENT_EIP(data, op);
}
void dynamic_clc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    instCPUImm('&', CPU_OFFSET_OF(flags), DYN_32bit, ~CF);
    INCREMENT_EIP(data, op);
}
void dynamic_stc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    instCPUImm('|', CPU_OFFSET_OF(flags), DYN_32bit, CF);
    INCREMENT_EIP(data, op);
}
void dynamic_cli(DynamicData* data, DecodedOp* op) {
    instCPUImm('&', CPU_OFFSET_OF(flags), DYN_32bit, ~IF);
    INCREMENT_EIP(data, op);
}
void dynamic_sti(DynamicData* data, DecodedOp* op) {
    instCPUImm('|', CPU_OFFSET_OF(flags), DYN_32bit, IF);
    INCREMENT_EIP(data, op);
}
void dynamic_cld(DynamicData* data, DecodedOp* op) {
    instCPUImm('&', CPU_OFFSET_OF(flags), DYN_32bit, ~DF);
    INCREMENT_EIP(data, op);
}
void dynamic_std(DynamicData* data, DecodedOp* op) {
    instCPUImm('|', CPU_OFFSET_OF(flags), DYN_32bit, DF);
    INCREMENT_EIP(data, op);
}
void dynamic_rdtsc(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_rdtsc, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_cpuid(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_cpuid, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_enter16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_enter32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_leave16(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(reg[4].u16), CPU_OFFSET_OF(reg[5].u16), DYN_16bit, DYN_SRC, true);
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(CPU_OFFSET_OF(reg[5].u16), DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_leave32(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(reg[4].u32), CPU_OFFSET_OF(reg[5].u32), DYN_32bit, DYN_SRC, true);
    dynamic_pop32(data);
    movToCpuFromReg(CPU_OFFSET_OF(reg[5].u32), DYN_CALL_RESULT, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_loopnz(DynamicData* data, DecodedOp* op) {
    // CX--;
    // if (CX != 0 && !cpu->getZF()) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    movToRegFromCpu(DYN_DEST, cpuOffset(1, width), width);
    instRegImm('-', DYN_DEST, width, 1);
    movToCpuFromReg(cpuOffset(1, width), DYN_DEST, width, false);
    setConditionInReg(data, NZ, DYN_SRC);

    // if nz flag, then set 1 in DYN_SRC
    evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    instRegImm('<', DYN_SRC, width, op->ea16 ? 15 : 31);
    instRegImm(')', DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is nz flag else 0) to CX
    instRegReg('&', DYN_DEST, DYN_SRC, width, true);

    dynamic_jumpIfRegSet(data, op, DYN_DEST, width);
}
void dynamic_loopz(DynamicData* data, DecodedOp* op) {
    // CX--;
    // if (CX != 0 && cpu->getZF()) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    movToRegFromCpu(DYN_DEST, cpuOffset(1, width), width);
    instRegImm('-', DYN_DEST, width, 1);
    movToCpuFromReg(cpuOffset(1, width), DYN_DEST, width, false);
    setConditionInReg(data, Z, DYN_SRC);

    // if z flag, then set 1 in DYN_SRC
    evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    instRegImm('<', DYN_SRC, width, op->ea16 ? 15 : 31);
    instRegImm(')', DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is z flag else 0) to CX
    instRegReg('&', DYN_DEST, DYN_SRC, width, true);    

    dynamic_jumpIfRegSet(data, op, DYN_DEST, width);
}

void dynamic_loop(DynamicData* data, DecodedOp* op) {
    // ECX--;
    // if (ECX != 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    movToRegFromCpu(DYN_DEST, cpuOffset(1, width), width);
    instRegImm('-', DYN_DEST, width, 1);
    movToCpuFromReg(cpuOffset(1, width), DYN_DEST, width, false);

    dynamic_jumpIfRegSet(data, op, DYN_DEST, width);
}
void dynamic_jcxz(DynamicData* data, DecodedOp* op) {
    // if (ECX == 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    movToRegFromCpu(DYN_DEST, cpuOffset(1, width), width);

    dynamic_jumpIfRegNotSet(data, op, DYN_DEST, width);
}
void dynamic_InAlIb(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u8), DYN_8bit, 0xFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InAxIb(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u16), DYN_16bit, 0xFFFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InEaxIb(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u32), DYN_32bit, 0xFFFFFFFF);
    INCREMENT_EIP(data, op);
}
void dynamic_OutIbAl(DynamicData* data, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(data, op);
}
void dynamic_OutIbAx(DynamicData* data, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(data, op);
}
void dynamic_OutIbEax(DynamicData* data, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(data, op);
}
void dynamic_InAlDx(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u8), DYN_8bit, 0xFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InAxDx(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u16), DYN_16bit, 0xFFFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InEaxDx(DynamicData* data, DecodedOp* op) {
    movToCpu(CPU_OFFSET_OF(reg[0].u32), DYN_32bit, 0xFFFFFFFF);
    INCREMENT_EIP(data, op);
}
void dynamic_OutDxAl(DynamicData* data, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(data, op);
}
void dynamic_OutDxAx(DynamicData* data, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(data, op);
}
void dynamic_OutDxEax(DynamicData* data, DecodedOp* op) {
    // do nothing
    INCREMENT_EIP(data, op);
}
void dynamic_callJw(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op->len+(S32)((S16)op->imm));
    blockCall(data, op);
}
void dynamic_callJd(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(data, DYN_SRC, true);;
    INCREMENT_EIP(data, op->len+(S32)op->imm);
    blockCall(data, op);
}
void dynamic_jmp8(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op->len+(S32)((S8)op->imm));
    blockNext1(data, op);
}
void dynamic_jmp16(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op->len+(S32)((S16)op->imm));
    blockNext1(data, op);
}
void dynamic_jmp32(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op->len+(S32)op->imm);
    blockNext1(data, op);
}
void dynamic_callR16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    movToRegFromCpu(DYN_DEST, CPU::offsetofReg16(op->reg), DYN_16bit);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);    
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false); movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_DEST, DYN_32bit, true); 
    blockDoneCall(data);
}
void dynamic_callR32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    movToRegFromCpu(DYN_DEST, CPU::offsetofReg32(op->reg), DYN_32bit);
    dynamic_pushReg32(data, DYN_SRC, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_DEST, DYN_32bit, true); 
    blockDoneCall(data);
}
void dynamic_callE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, true); // DYN_CALL_RESULT could get clobbered before used
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_16, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_DEST, DYN_32bit, true); 
    blockDoneCall(data);
}
void dynamic_callE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true); // DYN_CALL_RESULT could get clobbered before used
    movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(data, DYN_SRC, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_DEST, DYN_32bit, true); 
    blockDoneCall(data);
}
void dynamic_jmpR16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_SRC, DYN_32bit, true); 
    blockDoneJump(data);
}
void dynamic_jmpR32(DynamicData* data, DecodedOp* op) {
    movToCpuFromCpu(CPU_OFFSET_OF(eip.u32), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_SRC, true); 
    blockDoneJump(data);
}
void dynamic_jmpE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); 
    blockDoneJump(data);
}
void dynamic_jmpE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    movToCpuFromReg(CPU_OFFSET_OF(eip.u32), DYN_CALL_RESULT, DYN_32bit, true); 
    blockDoneJump(data);
}
void dynamic_callFarE16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_callFarE32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_jmpFarE16(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_jmpFarE32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(eip.u32), DYN_32bit);
    instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_larr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_larr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_larr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_larr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_lslr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_lslr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction((void*)common_lslr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_lslr32e32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_verre16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_verwe16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr8r8(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->reg), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU::offsetofReg8(op->rm), DYN_8bit);
        instRegReg('+', DYN_SRC, DYN_DEST, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_DEST, DYN_8bit, true);
        movToCpuFromReg(CPU::offsetofReg8(op->rm), DYN_SRC, DYN_8bit, true);
    }
    else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u8), CPU::offsetofReg8(op->reg), DYN_8bit, DYN_SRC, false);
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u8), CPU::offsetofReg8(op->rm), DYN_8bit, DYN_DEST, false);
        instRegReg('+', DYN_SRC, DYN_DEST, DYN_8bit, false);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_SRC, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_DEST, DYN_8bit, true);
        movToCpuFromReg(CPU::offsetofReg8(op->rm), DYN_SRC, DYN_8bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_ADD8);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr8e8(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg8(op->reg), DYN_8bit);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
    }
    else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u8), CPU::offsetofReg8(op->reg), DYN_8bit, DYN_SRC, false);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        movToCpuFromReg(CPU_OFFSET_OF(dst.u8), DYN_CALL_RESULT, DYN_8bit, false);
        instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        movToCpuFromReg(CPU_OFFSET_OF(result.u8), DYN_SRC, DYN_8bit, false);
        movToCpuFromReg(CPU::offsetofReg8(op->reg), DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_ADD8);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr16r16(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU::offsetofReg16(op->rm), DYN_16bit);
        instRegReg('+', DYN_SRC, DYN_DEST, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
        movToCpuFromReg(CPU::offsetofReg16(op->rm), DYN_SRC, DYN_16bit, true);
    }
    else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u16), CPU::offsetofReg16(op->reg), DYN_16bit, DYN_SRC, false);
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u16), CPU::offsetofReg16(op->rm), DYN_16bit, DYN_DEST, false);
        instRegReg('+', DYN_SRC, DYN_DEST, DYN_16bit, false);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_SRC, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
        movToCpuFromReg(CPU::offsetofReg16(op->rm), DYN_SRC, DYN_16bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_ADD16);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg16(op->reg), DYN_16bit);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
    }
    else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u16), CPU::offsetofReg16(op->reg), DYN_16bit, DYN_SRC, false);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        movToCpuFromReg(CPU_OFFSET_OF(dst.u16), DYN_CALL_RESULT, DYN_16bit, false);
        instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        movToCpuFromReg(CPU_OFFSET_OF(result.u16), DYN_SRC, DYN_16bit, false);
        movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_ADD16);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr32r32(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU::offsetofReg32(op->rm), DYN_32bit);
        instRegReg('+', DYN_SRC, DYN_DEST, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
        movToCpuFromReg(CPU::offsetofReg32(op->rm), DYN_SRC, DYN_32bit, true);
    } else {
        movToCpuFromCpu(CPU_OFFSET_OF(src.u32), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_SRC, false);
        movToCpuFromCpu(CPU_OFFSET_OF(dst.u32), CPU::offsetofReg32(op->rm), DYN_32bit, DYN_DEST, false);
        instRegReg('+', DYN_SRC, DYN_DEST, DYN_32bit, false);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_SRC, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
        movToCpuFromReg(CPU::offsetofReg32(op->rm), DYN_SRC, DYN_32bit, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_ADD32);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
    } else {    
        movToCpuFromCpu(CPU_OFFSET_OF(src.u32), CPU::offsetofReg32(op->reg), DYN_32bit, DYN_SRC, false);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        movToCpuFromReg(CPU_OFFSET_OF(dst.u32), DYN_CALL_RESULT, DYN_32bit, false);
        instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        movToCpuFromReg(CPU_OFFSET_OF(result.u32), DYN_SRC, DYN_32bit, false);
        movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true);
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)FLAGS_ADD32);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_bswap32(DynamicData* data, DecodedOp* op) {
    movToRegFromCpu(DYN_SRC, CPU::offsetofReg32(op->reg), DYN_32bit);
    byteSwapReg32(DYN_SRC);
    movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgg8b(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_cmpxchg8b, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_loadSegment16(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    movToCpuFromReg(CPU::offsetofReg16(op->reg), DYN_DEST, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_loadSegment32(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    blockDone(data, true);
    endIf();
    movToCpuFromReg(CPU::offsetofReg32(op->reg), DYN_DEST, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}

void dynamic_fxsave(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_fxsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}

void dynamic_fxrstor(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_fxrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}

void dynamic_xsave(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_xsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}

void dynamic_xrstor(DynamicData* data, DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_xrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}