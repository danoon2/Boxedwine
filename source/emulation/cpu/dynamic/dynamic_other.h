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
void DynamicData::dynamic_bound16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bound16, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_bound32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_bound32, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_daa(DecodedOp* op) {
    callHostFunction((void*)daa, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_das(DecodedOp* op) {
    callHostFunction((void*)das, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_aaa(DecodedOp* op) {
    callHostFunction((void*)aaa, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_aas(DecodedOp* op) {
    callHostFunction((void*)aas, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_aad(DecodedOp* op) {
    callHostFunction((void*)aad, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_aam(DecodedOp* op) {
    callHostFunction((void*)aam, true, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_nop(DecodedOp* op) {
    // Nop
    incrementEip(op->len);
}
void DynamicData::dynamic_done(DecodedOp* op) {
    incrementEip(op->len);
    blockDone(false);
}
void DynamicData::dynamic_wait(DecodedOp* op) {
    // Wait
    incrementEip(op->len);
}
void DynamicData::dynamic_cwd(DecodedOp* op) {
    loadReg(0, DYN_SRC, DYN_16bit);
    andRegImm(DYN_SRC, DYN_16bit, 0x8000);
    sarRegImm(DYN_SRC, DYN_16bit, 15);
    storeReg(2, DYN_SRC, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_cwq(DecodedOp* op) {
    loadReg(0, DYN_SRC, DYN_32bit);
    andRegImm(DYN_SRC, DYN_32bit, 0x80000000);
    sarRegImm(DYN_SRC, DYN_32bit, 31);
    storeReg(2, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_callAp(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(false);
}
void DynamicData::dynamic_callFar(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(false);
}
void DynamicData::dynamic_jmpAp(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(false);
}
void DynamicData::dynamic_jmpFar(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(false);
}
void DynamicData::dynamic_retf16(DecodedOp* op) {
    callHostFunction((void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone(false);
}
void DynamicData::dynamic_retf32(DecodedOp* op) {
    callHostFunction((void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    blockDone(false);
}
void DynamicData::dynamic_iret(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(false);
}
void DynamicData::dynamic_iret32(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    blockDone(false);
}
void DynamicData::dynamic_sahf(DecodedOp* op) {
    dynamic_fillFlags();
    callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, 4, DYN_PARAM_CPU_REG_8, false, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_lahf(DecodedOp* op) {
    dynamic_fillFlags();
    loadCPUFlags(DYN_SRC);
    andRegImm(DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);
    orRegImm(DYN_SRC, DYN_32bit, 2);
    storeReg(4, DYN_SRC, DYN_8bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_salc(DecodedOp* op) {
    dynamic_getCF();    
    negReg(DYN_CALL_RESULT, DYN_32bit);
    storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_retn16Iw(DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    loadReg(4, DYN_SRC, DYN_16bit);
    addRegImm(DYN_SRC, DYN_16bit, op->imm);
    storeReg(4, DYN_SRC, DYN_16bit, true);
    storeEip(DYN_CALL_RESULT, true);
    blockDone(false);
}
void DynamicData::dynamic_retn32Iw(DecodedOp* op) {
    dynamic_pop32();
    loadReg(4, DYN_SRC, DYN_32bit);
    addRegImm(DYN_SRC, DYN_32bit, op->imm);
    storeReg(4, DYN_SRC, DYN_32bit, true);
    storeEip(DYN_CALL_RESULT, true);
    blockDone(false);
}
void DynamicData::dynamic_retn16(DecodedOp* op) {
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    storeEip(DYN_CALL_RESULT, true);
    blockDone(false);
}
void DynamicData::dynamic_retn32(DecodedOp* op) {
    dynamic_pop32();
    storeEip(DYN_CALL_RESULT, true);
    blockDone(false);
}
void DynamicData::dynamic_invalid(DecodedOp* op) {
    callHostFunction((void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(true);
}
void DynamicData::dynamic_ud2(DecodedOp* op) {
    callHostFunction((void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(true);
}
void DynamicData::dynamic_int80(DecodedOp* op) {
    callHostFunction((void*)ksyscall, false, 2, 0, DYN_PARAM_CPU, false, op->len, DYN_PARAM_CONST_32, false);
    blockDone(false);
}
void DynamicData::dynamic_int99(DecodedOp* op) {
    callHostFunction((void*)common_int99, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_int9A(DecodedOp* op) {
    callHostFunction((void*)common_int9A, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_int9B(DecodedOp* op) {
    callHostFunction((void*)common_int9B, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_intIb(DecodedOp* op) {
    callHostFunction((void*)common_intIb, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(false);
}
void DynamicData::dynamic_int3(DecodedOp* op) {
    callHostFunction((void*)common_int3, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(false);
}
void DynamicData::dynamic_xlat(DecodedOp* op) {
    loadReg(0, DYN_SRC, DYN_8bit);
    if (op->ea16) {
        loadReg(3, DYN_ADDRESS, DYN_16bit);
        movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false);
        addRegReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true);
        movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_ADDRESS, DYN_16bit, false);
    } else {
        loadReg(3, DYN_ADDRESS, DYN_32bit);
        movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
        addRegReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    }
    loadSegAddress(op->base, DYN_SRC);
    addRegReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    movFromMem(DYN_8bit, DYN_ADDRESS, true);
    storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_hlt(DecodedOp* op) {
    callHostFunction((void*)common_hlt, false, 1, 0, DYN_PARAM_CPU, false);
    blockDone(true);
}
void DynamicData::dynamic_cmc(DecodedOp* op) {
    dynamic_fillFlags();
    xorCPUFlagsImm(CF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_clc(DecodedOp* op) {
    dynamic_fillFlags();
    andCPUFlagsImm(~CF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_stc(DecodedOp* op) {
    dynamic_fillFlags();
    orCPUFlagsImm(CF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_cli(DecodedOp* op) {
    andCPUFlagsImm(~IF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_sti(DecodedOp* op) {
    orCPUFlagsImm(IF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_cld(DecodedOp* op) {
    andCPUFlagsImm(~DF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_std(DecodedOp* op) {
    orCPUFlagsImm(DF, DYN_DEST);
    incrementEip(op->len);
}
void DynamicData::dynamic_rdtsc(DecodedOp* op) {
    callHostFunction((void*)common_rdtsc, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_cpuid(DecodedOp* op) {
    callHostFunction((void*)common_cpuid, false, 1, 0, DYN_PARAM_CPU, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_enter16(DecodedOp* op) {
    callHostFunction((void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_enter32(DecodedOp* op) {
    callHostFunction((void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_leave16(DecodedOp* op) {
    loadRegStoreReg(4, 5, DYN_16bit, DYN_SRC);
    callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    storeReg(5, DYN_CALL_RESULT, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_leave32(DecodedOp* op) {
    loadRegStoreReg(4, 5, DYN_32bit, DYN_SRC);
    dynamic_pop32();
    storeReg(5, DYN_CALL_RESULT, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_loopnz(DecodedOp* op) {
    // CX--;
    // if (CX != 0 && !cpu->getZF()) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;

    setConditionInReg(NZ, DYN_SRC); // setConditionInReg uses DYN_DEST as tmp, so this call must be first

    loadReg(1, DYN_DEST, width);
    subRegImm(DYN_DEST, width, 1);
    storeReg(1, DYN_DEST, width, false);

    // if nz flag, then set 1 in DYN_SRC
    evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    shlRegImm(DYN_SRC, width, op->ea16 ? 15 : 31);
    sarRegImm(DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is nz flag else 0) to CX
    andRegReg(DYN_DEST, DYN_SRC, width, true);

    if (width == DYN_16bit) {
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegSet(op, DYN_DEST, true);
}
void DynamicData::dynamic_loopz(DecodedOp* op) {
    // CX--;
    // if (CX != 0 && cpu->getZF()) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;

    setConditionInReg(Z, DYN_SRC); // setConditionInReg uses DYN_DEST as tmp, so this call must be first

    loadReg(1, DYN_DEST, width);
    subRegImm(DYN_DEST, width, 1);
    storeReg(1, DYN_DEST, width, false);

    // if z flag, then set 1 in DYN_SRC
    evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    shlRegImm(DYN_SRC, width, op->ea16 ? 15 : 31);
    sarRegImm(DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is z flag else 0) to CX
    andRegReg(DYN_DEST, DYN_SRC, width, true);    

    if (width == DYN_16bit) {
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegSet(op, DYN_DEST, true);
}

void DynamicData::dynamic_loop(DecodedOp* op) {
    // ECX--;
    // if (ECX != 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    loadReg(1, DYN_DEST, width);
    subRegImm(DYN_DEST, width, 1);
    storeReg(1, DYN_DEST, width, false);

    if (width == DYN_16bit) {
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegSet(op, DYN_DEST, true);
}
void DynamicData::dynamic_jcxz(DecodedOp* op) {
    // if (ECX == 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;
    loadReg(1, DYN_DEST, width);

    if (width == DYN_16bit) {
        movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegNotSet(op, DYN_DEST, true);
}
void DynamicData::dynamic_InAlIb(DecodedOp* op) {
    storeReg(0, DYN_8bit, 0xFF);
    incrementEip(op->len);
}
void DynamicData::dynamic_InAxIb(DecodedOp* op) {
    storeReg(0, DYN_16bit, 0xFFFF);
    incrementEip(op->len);
}
void DynamicData::dynamic_InEaxIb(DecodedOp* op) {
    storeReg(0, DYN_32bit, 0xFFFFFFFF);
    incrementEip(op->len);
}
void DynamicData::dynamic_OutIbAl(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void DynamicData::dynamic_OutIbAx(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void DynamicData::dynamic_OutIbEax(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void DynamicData::dynamic_InAlDx(DecodedOp* op) {
    storeReg(0, DYN_8bit, 0xFF);
    incrementEip(op->len);
}
void DynamicData::dynamic_InAxDx(DecodedOp* op) {
    storeReg(0, DYN_16bit, 0xFFFF);
    incrementEip(op->len);
}
void DynamicData::dynamic_InEaxDx(DecodedOp* op) {
    storeReg(0, DYN_32bit, 0xFFFFFFFF);
    incrementEip(op->len);
}
void DynamicData::dynamic_OutDxAl(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void DynamicData::dynamic_OutDxAx(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void DynamicData::dynamic_OutDxEax(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void DynamicData::dynamic_callJw(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    incrementEip(op->len+(S32)((S16)op->imm));
    blockCall(op);
}
void DynamicData::dynamic_callJd(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(DYN_SRC, true);;
    incrementEip(op->len+(S32)op->imm);
    blockCall(op);
}
void DynamicData::dynamic_jmp8(DecodedOp* op) {
    incrementEip(op->len+(S32)((S8)op->imm));
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
    } else {
        blockNext1(op);
    }
}
void DynamicData::dynamic_jmp16(DecodedOp* op) {
    incrementEip(op->len+(S32)((S16)op->imm));
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)((S16)op->imm));
    } else {
        blockNext1(op);
    }
}
void DynamicData::dynamic_jmp32(DecodedOp* op) {
    incrementEip(op->len+(S32)op->imm);
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)op->imm);
    } else {
        blockNext1(op);
    }
}
void DynamicData::dynamic_callR16(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    loadReg(op->reg, DYN_DEST, DYN_16bit);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);    
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false); 
    storeEip(DYN_DEST, true);
    blockDoneCall();
}
void DynamicData::dynamic_callR32(DecodedOp* op) {
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    loadReg(op->reg, DYN_CALL_RESULT, DYN_32bit);
    dynamic_pushReg32(DYN_SRC, true);
    storeEip(DYN_CALL_RESULT, true);
    blockDoneCall();
}
void DynamicData::dynamic_callE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, true); // DYN_CALL_RESULT could get clobbered before used
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_16, true);
    storeEip(DYN_DEST, true);
    blockDoneCall();
}
void DynamicData::dynamic_callE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    loadEip(DYN_SRC);
    addRegImm(DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(DYN_SRC, true);
    storeEip(DYN_CALL_RESULT, true);
    blockDoneCall();
}
void DynamicData::dynamic_jmpR16(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_16bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    storeEip(DYN_SRC, true);
    blockDoneJump();
}
void DynamicData::dynamic_jmpR32(DecodedOp* op) {
    loadRegStoreEip(op->reg, DYN_SRC);
    blockDoneJump();
}
void DynamicData::dynamic_jmpE16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    storeEip(DYN_CALL_RESULT, true);
    blockDoneJump();
}
void DynamicData::dynamic_jmpE32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, true);
    storeEip(DYN_CALL_RESULT, true);
    blockDoneJump();
}
void DynamicData::dynamic_callFarE16(DecodedOp* op) {
    loadEip(DYN_DEST);
    addRegImm(DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    addRegImm(DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(false);
}
void DynamicData::dynamic_callFarE32(DecodedOp* op) {
    loadEip(DYN_DEST);
    addRegImm(DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    addRegImm(DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(false);
}
void DynamicData::dynamic_jmpFarE16(DecodedOp* op) {
    loadEip(DYN_DEST);
    addRegImm(DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    addRegImm(DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(false);
}
void DynamicData::dynamic_jmpFarE32(DecodedOp* op) {
    loadEip(DYN_DEST);
    addRegImm(DYN_DEST, DYN_32bit, op->len);
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    addRegImm(DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    blockDone(false);
}
void DynamicData::dynamic_larr16r16(DecodedOp* op) {
    callHostFunction((void*)common_larr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_larr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_larr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr16r16(DecodedOp* op) {
    callHostFunction((void*)common_lslr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_lslr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr32r32(DecodedOp* op) {
    callHostFunction((void*)common_lslr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_lslr32e32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_verre16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_verwe16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr8r8(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_8bit);
        loadReg(op->rm, DYN_DEST, DYN_8bit);
        addRegReg(DYN_SRC, DYN_DEST, DYN_8bit, false);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        storeReg(op->rm, DYN_SRC, DYN_8bit, true);
    }
    else {
        loadRegStoreSrc(op->reg, DYN_8bit, DYN_SRC, false);
        loadRegStoreDst(op->rm, DYN_8bit, DYN_DEST, false);
        addRegReg(DYN_SRC, DYN_DEST, DYN_8bit, false);
        storeLazyFlagsResult(DYN_SRC, DYN_8bit, false);
        storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        storeReg(op->rm, DYN_SRC, DYN_8bit, true);
        storeLazyFlags(FLAGS_ADD8);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr8e8(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_8bit);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        addRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
    }
    else {
        loadRegStoreSrc(op->reg, DYN_8bit, DYN_SRC, false);
        movFromMem(DYN_8bit, DYN_ADDRESS, false);
        storeLazyFlagsDst(DYN_CALL_RESULT, DYN_8bit, false);
        addRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        storeLazyFlagsResult(DYN_SRC, DYN_8bit, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_8bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_ADD8);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr16r16(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        loadReg(op->rm, DYN_DEST, DYN_16bit);
        addRegReg(DYN_SRC, DYN_DEST, DYN_16bit, false);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        storeReg(op->rm, DYN_SRC, DYN_16bit, true);
    }
    else {
        loadRegStoreSrc(op->reg, DYN_16bit, DYN_SRC, false);
        loadRegStoreDst(op->rm, DYN_16bit, DYN_DEST, false);
        addRegReg(DYN_SRC, DYN_DEST, DYN_16bit, false);
        storeLazyFlagsResult(DYN_SRC, DYN_16bit, false);
        storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        storeReg(op->rm, DYN_SRC, DYN_16bit, true);
        storeLazyFlags(FLAGS_ADD16);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr16e16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_16bit);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        addRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    }
    else {
        loadRegStoreSrc(op->reg, DYN_16bit, DYN_SRC, false);
        movFromMem(DYN_16bit, DYN_ADDRESS, false);
        storeLazyFlagsDst(DYN_CALL_RESULT, DYN_16bit, false);
        addRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        storeLazyFlagsResult(DYN_SRC, DYN_16bit, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_ADD16);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr32r32(DecodedOp* op) {
    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_32bit);
        loadReg(op->rm, DYN_DEST, DYN_32bit);
        addRegReg(DYN_SRC, DYN_DEST, DYN_32bit, false);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        storeReg(op->rm, DYN_SRC, DYN_32bit, true);
    } else {
        loadRegStoreSrc(op->reg, DYN_32bit, DYN_SRC, false);
        loadRegStoreDst(op->rm, DYN_32bit, DYN_DEST, false);
        addRegReg(DYN_SRC, DYN_DEST, DYN_32bit, false);
        storeLazyFlagsResult(DYN_SRC, DYN_32bit, false);
        storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        storeReg(op->rm, DYN_SRC, DYN_32bit, true);
        storeLazyFlags(FLAGS_ADD32);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr32e32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(cpu)) {
        loadReg(op->reg, DYN_SRC, DYN_32bit);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        addRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
    } else {    
        loadRegStoreSrc(op->reg, DYN_32bit, DYN_SRC, false);
        movFromMem(DYN_32bit, DYN_ADDRESS, false);
        storeLazyFlagsDst(DYN_CALL_RESULT, DYN_32bit, false);
        addRegReg(DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        storeLazyFlagsResult(DYN_SRC, DYN_32bit, false);
        storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
        movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
        storeLazyFlags(FLAGS_ADD32);
    }
    incrementEip(op->len);
}
void DynamicData::dynamic_bswap32(DecodedOp* op) {
    loadReg(op->reg, DYN_SRC, DYN_32bit);
    byteSwapReg32(DYN_SRC);
    storeReg(op->reg, DYN_SRC, DYN_32bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgg8b(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_cmpxchg8b, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    currentLazyFlags=FLAGS_NONE;
    incrementEip(op->len);
}
void DynamicData::dynamic_loadSegment16(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_16bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    addRegImm(DYN_ADDRESS, DYN_32bit, 2);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    incrementEip(op->len);
}
void DynamicData::dynamic_loadSegment32(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    movFromMem(DYN_32bit, DYN_ADDRESS, false);
    movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    addRegImm(DYN_ADDRESS, DYN_32bit, 4);
    movFromMem(DYN_16bit, DYN_ADDRESS, true);
    callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    IfNot(DYN_CALL_RESULT, true);
    blockDone(true);
    EndIf();
    storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    incrementEip(op->len);
}

void DynamicData::dynamic_fxsave(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_fxsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}

void DynamicData::dynamic_fxrstor(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_fxrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}

void DynamicData::dynamic_xsave(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_xsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}

void DynamicData::dynamic_xrstor(DecodedOp* op) {
    calculateEaa(op, DYN_ADDRESS);
    callHostFunction((void*)common_xrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    incrementEip(op->len);
}