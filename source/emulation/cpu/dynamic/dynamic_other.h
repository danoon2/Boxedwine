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
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_bound16, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_bound32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_bound32, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    INCREMENT_EIP(data, op);
}
void dynamic_daa(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)daa, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_das(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)das, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aaa(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)aaa, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aas(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)aas, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aad(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)aad, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_aam(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)aam, true, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
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
    loadReg(data, 0, DYN_SRC, DYN_16bit, true);
    instRegImm(data, '&', DYN_SRC, DYN_16bit, 0x8000);
    instRegImm(data, ')', DYN_SRC, DYN_16bit, 15);
    storeReg(data, 2, DYN_SRC, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_cwq(DynamicData* data, DecodedOp* op) {
    loadReg(data, 0, DYN_SRC, DYN_32bit, true);
    instRegImm(data, '&', DYN_SRC, DYN_32bit, 0x80000000);
    instRegImm(data, ')', DYN_SRC, DYN_32bit, 31);
    storeReg(data, 2, DYN_SRC, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_callAp(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_callFar(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_jmpAp(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_jmpFar(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_retf16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone(data, false);
}
void dynamic_retf32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone(data, false);
}
void dynamic_iret(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_iret32(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(data, false);
}
void dynamic_sahf(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    callHostFunction(data, (void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, CPU_OFFSET_OF(reg[0].h8), DYN_PARAM_CPU_ADDRESS_8, false, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_lahf(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    loadCPUFlags(data, DYN_SRC);
    instRegImm(data, '&', DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);
    instRegImm(data, '|', DYN_SRC, DYN_32bit, 2);
    storeReg(data, 4, DYN_SRC, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_salc(DynamicData* data, DecodedOp* op) {
    dynamic_getCF(data);    
    instReg(data, '-', DYN_CALL_RESULT, DYN_32bit);
    storeReg(data, 0, DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_retn16Iw(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    DynReg reg = loadReg(data, 4, DYN_SRC, DYN_16bit);
    instRegImm(data, '+', reg, DYN_16bit, op->imm);
    storeReg(data, 4, reg, DYN_16bit, true);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDone(data, false);
}
void dynamic_retn32Iw(DynamicData* data, DecodedOp* op) {
    dynamic_pop32(data);
    DynReg reg = loadReg(data, 4, DYN_SRC, DYN_32bit);
    instRegImm(data, '+', reg, DYN_32bit, op->imm);
    storeReg(data, 4, reg, DYN_32bit, true);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDone(data, false);
}
void dynamic_retn16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDone(data, false);
}
void dynamic_retn32(DynamicData* data, DecodedOp* op) {
    dynamic_pop32(data);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDone(data, false);
}
void dynamic_invalid(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, true);
}
void dynamic_ud2(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, true);
}
void dynamic_int80(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)ksyscall, false, 2, 0, DYN_PARAM_CPU, false, op->len, DYN_PARAM_CONST_32, false);
    blockDone(data, false);
}
void dynamic_int99(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_int99, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_int9A(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_int9A, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_int9B(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_int9B, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_intIb(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_intIb, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, false);
}
void dynamic_int3(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_int3, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, false);
}
void dynamic_xlat(DynamicData* data, DecodedOp* op) {
    loadReg(data, 0, DYN_SRC, DYN_8bit, true);
    if (op->ea16) {
        loadReg(data, 3, DYN_ADDRESS, DYN_16bit, true);
        movToRegFromReg(data, DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false);
        instRegReg(data, '+', DYN_ADDRESS, DYN_SRC, DYN_16bit, true);
        movToRegFromReg(data, DYN_ADDRESS, DYN_32bit, DYN_ADDRESS, DYN_16bit, false);
    } else {
        loadReg(data, 3, DYN_ADDRESS, DYN_32bit, true);
        movToRegFromReg(data, DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
        instRegReg(data, '+', DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    }
    loadSegAddress(data, op->base, DYN_SRC);
    instRegReg(data, '+', DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    movFromMem(data, DYN_8bit, DYN_ADDRESS, true);
    storeReg(data, 0, DYN_CALL_RESULT, DYN_8bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_hlt(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_hlt, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(data, true);
}
void dynamic_cmc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    instCPUImm(data, '^', CPU_OFFSET_OF(flags), DYN_32bit, CF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_clc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    instCPUImm(data, '&', CPU_OFFSET_OF(flags), DYN_32bit, ~CF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_stc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    instCPUImm(data, '|', CPU_OFFSET_OF(flags), DYN_32bit, CF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_cli(DynamicData* data, DecodedOp* op) {
    instCPUImm(data, '&', CPU_OFFSET_OF(flags), DYN_32bit, ~IF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_sti(DynamicData* data, DecodedOp* op) {
    instCPUImm(data, '|', CPU_OFFSET_OF(flags), DYN_32bit, IF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_cld(DynamicData* data, DecodedOp* op) {
    instCPUImm(data, '&', CPU_OFFSET_OF(flags), DYN_32bit, ~DF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_std(DynamicData* data, DecodedOp* op) {
    instCPUImm(data, '|', CPU_OFFSET_OF(flags), DYN_32bit, DF, DYN_DEST);
    INCREMENT_EIP(data, op);
}
void dynamic_rdtsc(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_rdtsc, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_cpuid(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_cpuid, false, 1, 0, DYN_PARAM_CPU, false);
    INCREMENT_EIP(data, op);
}
void dynamic_enter16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_enter32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_leave16(DynamicData* data, DecodedOp* op) {
    loadRegStoreReg(data, 4, 5, DYN_16bit, DYN_SRC, true);
    callHostFunction(data, (void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    storeReg(data, 5, DYN_CALL_RESULT, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_leave32(DynamicData* data, DecodedOp* op) {
    loadRegStoreReg(data, 4, 5, DYN_32bit, DYN_SRC, true);
    dynamic_pop32(data);
    storeReg(data, 5, DYN_CALL_RESULT, DYN_32bit, true);
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

    setConditionInReg(data, NZ, DYN_SRC); // setConditionInReg uses DYN_DEST as tmp, so this call must be first

    loadReg(data, 1, DYN_DEST, width, true);
    instRegImm(data, '-', DYN_DEST, width, 1);
    storeReg(data, 1, DYN_DEST, width, false);

    // if nz flag, then set 1 in DYN_SRC
    evaluateToReg(data, DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    instRegImm(data, '<', DYN_SRC, width, op->ea16 ? 15 : 31);
    instRegImm(data, ')', DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is nz flag else 0) to CX
    instRegReg(data, '&', DYN_DEST, DYN_SRC, width, true);

    if (width == DYN_16bit) {
        movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegSet(data, op, DYN_DEST, true);
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

    setConditionInReg(data, Z, DYN_SRC); // setConditionInReg uses DYN_DEST as tmp, so this call must be first

    loadReg(data, 1, DYN_DEST, width, true);
    instRegImm(data, '-', DYN_DEST, width, 1);
    storeReg(data, 1, DYN_DEST, width, false);

    // if z flag, then set 1 in DYN_SRC
    evaluateToReg(data, DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    instRegImm(data, '<', DYN_SRC, width, op->ea16 ? 15 : 31);
    instRegImm(data, ')', DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is z flag else 0) to CX
    instRegReg(data, '&', DYN_DEST, DYN_SRC, width, true);    

    if (width == DYN_16bit) {
        movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegSet(data, op, DYN_DEST, true);
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
    loadReg(data, 1, DYN_DEST, width, true);
    instRegImm(data, '-', DYN_DEST, width, 1);
    storeReg(data, 1, DYN_DEST, width, false);

    if (width == DYN_16bit) {
        movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegSet(data, op, DYN_DEST, true);
}
void dynamic_jcxz(DynamicData* data, DecodedOp* op) {
    // if (ECX == 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    loadReg(data, 1, DYN_DEST, width, true);

    if (width == DYN_16bit) {
        movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegNotSet(data, op, DYN_DEST, true);
}
void dynamic_InAlIb(DynamicData* data, DecodedOp* op) {
    storeReg(data, 0, DYN_8bit, 0xFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InAxIb(DynamicData* data, DecodedOp* op) {
    storeReg(data, 0, DYN_16bit, 0xFFFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InEaxIb(DynamicData* data, DecodedOp* op) {
    storeReg(data, 0, DYN_32bit, 0xFFFFFFFF);
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
    storeReg(data, 0, DYN_8bit, 0xFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InAxDx(DynamicData* data, DecodedOp* op) {
    storeReg(data, 0, DYN_16bit, 0xFFFF);
    INCREMENT_EIP(data, op);
}
void dynamic_InEaxDx(DynamicData* data, DecodedOp* op) {
    storeReg(data, 0, DYN_32bit, 0xFFFFFFFF);
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
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op->len+(S32)((S16)op->imm));
    blockCall(data, op);
}
void dynamic_callJd(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(data, DYN_SRC, true);;
    INCREMENT_EIP(data, op->len+(S32)op->imm);
    blockCall(data, op);
}
void dynamic_jmp8(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op->len+(S32)((S8)op->imm));
    if (data->canJumpInBlock(op)) {
        JumpInBlock(data, data->currentEip + op->len + (S32)((S8)op->imm));
    } else {
        blockNext1(data, op);
    }
}
void dynamic_jmp16(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op->len+(S32)((S16)op->imm));
    if (data->canJumpInBlock(op)) {
        JumpInBlock(data, data->currentEip + op->len + (S32)((S16)op->imm));
    } else {
        blockNext1(data, op);
    }
}
void dynamic_jmp32(DynamicData* data, DecodedOp* op) {
    INCREMENT_EIP(data, op->len+(S32)op->imm);
    if (data->canJumpInBlock(op)) {
        JumpInBlock(data, data->currentEip + op->len + (S32)op->imm);
    } else {
        blockNext1(data, op);
    }
}
void dynamic_callR16(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    loadReg(data, op->reg, DYN_DEST, DYN_16bit, true);
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);    
    movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false); 
    storeEip(data, DYN_DEST, true);
    blockDoneCall(data);
}
void dynamic_callR32(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    DynReg reg = loadReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit);
    dynamic_pushReg32(data, DYN_SRC, true);
    storeEip(data, reg, true);
    blockDoneCall(data);
}
void dynamic_callE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, true); // DYN_CALL_RESULT could get clobbered before used
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    callHostFunction(data, (void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_16, true);
    storeEip(data, DYN_DEST, true);
    blockDoneCall(data);
}
void dynamic_callE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    loadEip(data, DYN_SRC);
    instRegImm(data, '+', DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(data, DYN_SRC, true);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDoneCall(data);
}
void dynamic_jmpR16(DynamicData* data, DecodedOp* op) {
    loadReg(data, op->reg, DYN_SRC, DYN_16bit, true);
    movToRegFromReg(data, DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    storeEip(data, DYN_SRC, true);
    blockDoneJump(data);
}
void dynamic_jmpR32(DynamicData* data, DecodedOp* op) {
    loadRegStoreEip(data, op->reg, DYN_SRC, true);
    blockDoneJump(data);
}
void dynamic_jmpE16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(data, DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDoneJump(data);
}
void dynamic_jmpE32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, true);
    storeEip(data, DYN_CALL_RESULT, true);
    blockDoneJump(data);
}
void dynamic_callFarE16(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_DEST);
    instRegImm(data, '+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(data, DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_callFarE32(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_DEST);
    instRegImm(data, '+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(data, DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_jmpFarE16(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_DEST);
    instRegImm(data, '+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(data, DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_jmpFarE32(DynamicData* data, DecodedOp* op) {
    loadEip(data, DYN_DEST);
    instRegImm(data, '+', DYN_DEST, DYN_32bit, op->len);
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(data, DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(data, false);
}
void dynamic_larr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_larr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_larr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_larr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr16r16(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_lslr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_lslr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr32r32(DynamicData* data, DecodedOp* op) {
    callHostFunction(data, (void*)common_lslr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    INCREMENT_EIP(data, op);
}
void dynamic_lslr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_lslr32e32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_verre16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_verwe16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr8r8(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        loadReg(data, op->reg, DYN_SRC, DYN_8bit, true);
        DynReg dst = loadReg(data, op->rm, DYN_DEST, DYN_8bit);
        instRegReg(data, '+', DYN_SRC, dst, DYN_8bit, false);
        storeReg(data, op->reg, dst, DYN_8bit, true);
        storeReg(data, op->rm, DYN_SRC, DYN_8bit, true);
    }
    else {
        loadRegStoreSrc(data, op->reg, DYN_8bit, DYN_SRC, false);
        loadRegStoreDst(data, op->rm, DYN_8bit, DYN_DEST, false);
        instRegReg(data, '+', DYN_SRC, DYN_DEST, DYN_8bit, false);
        storeLazyFlagsResult(data, DYN_SRC, DYN_8bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_8bit, true);
        storeReg(data, op->rm, DYN_SRC, DYN_8bit, true);
        storeLazyFlags(data, FLAGS_ADD8);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr8e8(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        loadReg(data, op->reg, DYN_SRC, DYN_8bit, true);
        movFromMem(data, DYN_8bit, DYN_ADDRESS, false);
        instRegReg(data, '+', DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        storeReg(data, op->reg, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
    }
    else {
        loadRegStoreSrc(data, op->reg, DYN_8bit, DYN_SRC, false);
        movFromMem(data, DYN_8bit, DYN_ADDRESS, false);
        storeLazyFlagsDst(data, DYN_CALL_RESULT, DYN_8bit, false);
        instRegReg(data, '+', DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        storeLazyFlagsResult(data, DYN_SRC, DYN_8bit, false);
        storeReg(data, op->reg, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_ADD8);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr16r16(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        loadReg(data, op->reg, DYN_SRC, DYN_16bit, true);
        DynReg reg = loadReg(data, op->rm, DYN_DEST, DYN_16bit);
        instRegReg(data, '+', DYN_SRC, reg, DYN_16bit, false);
        storeReg(data, op->reg, reg, DYN_16bit, true);
        storeReg(data, op->rm, DYN_SRC, DYN_16bit, true);
    }
    else {
        loadRegStoreSrc(data, op->reg, DYN_16bit, DYN_SRC, false);
        loadRegStoreDst(data, op->rm, DYN_16bit, DYN_DEST, false);
        instRegReg(data, '+', DYN_SRC, DYN_DEST, DYN_16bit, false);
        storeLazyFlagsResult(data, DYN_SRC, DYN_16bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_16bit, true);
        storeReg(data, op->rm, DYN_SRC, DYN_16bit, true);
        storeLazyFlags(data, FLAGS_ADD16);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr16e16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        loadReg(data, op->reg, DYN_SRC, DYN_16bit, true);
        movFromMem(data, DYN_16bit, DYN_ADDRESS, false);
        instRegReg(data, '+', DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        storeReg(data, op->reg, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    }
    else {
        loadRegStoreSrc(data, op->reg, DYN_16bit, DYN_SRC, false);
        movFromMem(data, DYN_16bit, DYN_ADDRESS, false);
        storeLazyFlagsDst(data, DYN_CALL_RESULT, DYN_16bit, false);
        instRegReg(data, '+', DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        storeLazyFlagsResult(data, DYN_SRC, DYN_16bit, false);
        storeReg(data, op->reg, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_ADD16);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr32r32(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        loadReg(data, op->reg, DYN_SRC, DYN_32bit, true);
        DynReg reg = loadReg(data, op->rm, DYN_DEST, DYN_32bit);
        instRegReg(data, '+', DYN_SRC, reg, DYN_32bit, false);
        storeReg(data, op->reg, reg, DYN_32bit, true);
        storeReg(data, op->rm, DYN_SRC, DYN_32bit, true);
    } else {
        loadRegStoreSrc(data, op->reg, DYN_32bit, DYN_SRC, false);
        loadRegStoreDst(data, op->rm, DYN_32bit, DYN_DEST, false);
        instRegReg(data, '+', DYN_SRC, DYN_DEST, DYN_32bit, false);
        storeLazyFlagsResult(data, DYN_SRC, DYN_32bit, false);
        storeReg(data, op->reg, DYN_DEST, DYN_32bit, true);
        storeReg(data, op->rm, DYN_SRC, DYN_32bit, true);
        storeLazyFlags(data, FLAGS_ADD32);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_xaddr32e32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        loadReg(data, op->reg, DYN_SRC, DYN_32bit, true);
        movFromMem(data, DYN_32bit, DYN_ADDRESS, false);
        instRegReg(data, '+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        storeReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
    } else {    
        loadRegStoreSrc(data, op->reg, DYN_32bit, DYN_SRC, false);
        movFromMem(data, DYN_32bit, DYN_ADDRESS, false);
        storeLazyFlagsDst(data, DYN_CALL_RESULT, DYN_32bit, false);
        instRegReg(data, '+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        storeLazyFlagsResult(data, DYN_SRC, DYN_32bit, false);
        storeReg(data, op->reg, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(data, DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(data, FLAGS_ADD32);
    }
    INCREMENT_EIP(data, op);
}
void dynamic_bswap32(DynamicData* data, DecodedOp* op) {
    DynReg reg = loadReg(data, op->reg, DYN_SRC, DYN_32bit);
    byteSwapReg32(data, reg);
    storeReg(data, op->reg, reg, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_cmpxchgg8b(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_cmpxchg8b, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    INCREMENT_EIP(data, op);
}
void dynamic_loadSegment16(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(data, DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    storeReg(data, op->reg, DYN_DEST, DYN_16bit, true);
    INCREMENT_EIP(data, op);
}
void dynamic_loadSegment32(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    movFromMem(data, DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(data, DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    instRegImm(data, '+', DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(data, DYN_16bit, DYN_ADDRESS, true);
    callHostFunction(data, (void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(data, DYN_CALL_RESULT, true);
    blockDone(data, true);
    EndIf(data);
    storeReg(data, op->reg, DYN_DEST, DYN_32bit, true);
    INCREMENT_EIP(data, op);
}

void dynamic_fxsave(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_fxsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}

void dynamic_fxrstor(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_fxrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}

void dynamic_xsave(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_xsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}

void dynamic_xrstor(DynamicData* data, DecodedOp* op) {
    calculateEaa(data, op, DYN_ADDRESS);
    callHostFunction(data, (void*)common_xrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    INCREMENT_EIP(data, op);
}