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
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_bound16, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}
void dynamic_bound32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_bound32, true, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}
void dynamic_daa(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)daa, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_das(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)das, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_aaa(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)aaa, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_aas(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)aas, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_aad(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)aad, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_aam(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)aam, true, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->incrementEip(op->len);
}
void dynamic_nop(DynamicData* data, DecodedOp* op) {
    // Nop
    data->incrementEip(op->len);
}
void dynamic_done(DynamicData* data, DecodedOp* op) {
    data->incrementEip(op->len);
    data->blockDone(false);
}
void dynamic_wait(DynamicData* data, DecodedOp* op) {
    // Wait
    data->incrementEip(op->len);
}
void dynamic_cwd(DynamicData* data, DecodedOp* op) {
    data->loadReg(0, DYN_SRC, DYN_16bit, true);
    data->instRegImm('&', DYN_SRC, DYN_16bit, 0x8000);
    data->instRegImm(')', DYN_SRC, DYN_16bit, 15);
    data->storeReg(2, DYN_SRC, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_cwq(DynamicData* data, DecodedOp* op) {
    data->loadReg(0, DYN_SRC, DYN_32bit, true);
    data->instRegImm('&', DYN_SRC, DYN_32bit, 0x80000000);
    data->instRegImm(')', DYN_SRC, DYN_32bit, 31);
    data->storeReg(2, DYN_SRC, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_callAp(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->blockDone(false);
}
void dynamic_callFar(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->blockDone(false);
}
void dynamic_jmpAp(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->blockDone(false);
}
void dynamic_jmpFar(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->blockDone(false);
}
void dynamic_retf16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->blockDone(false);
}
void dynamic_retf32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_ret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false);
    data->blockDone(false);
}
void dynamic_iret(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->blockDone(false);
}
void dynamic_iret32(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_iret, false, 3, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->blockDone(false);
}
void dynamic_sahf(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->callHostFunction((void*)common_setFlags, false, 3, 0, DYN_PARAM_CPU, false, CPU::offsetofReg8(4), DYN_PARAM_CPU_ADDRESS_8, false, FMASK_ALL & 0xFF, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_lahf(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->loadCPUFlags(DYN_SRC);
    data->instRegImm('&', DYN_SRC, DYN_32bit, SF|ZF|AF|PF|CF);
    data->instRegImm('|', DYN_SRC, DYN_32bit, 2);
    data->storeReg(4, DYN_SRC, DYN_8bit, true);
    data->incrementEip(op->len);
}
void dynamic_salc(DynamicData* data, DecodedOp* op) {
    dynamic_getCF(data);    
    data->instReg('-', DYN_CALL_RESULT, DYN_32bit);
    data->storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
    data->incrementEip(op->len);
}
void dynamic_retn16Iw(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    DynReg reg = data->loadReg(4, DYN_SRC, DYN_16bit);
    data->instRegImm('+', reg, DYN_16bit, op->imm);
    data->storeReg(4, reg, DYN_16bit, true);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDone(false);
}
void dynamic_retn32Iw(DynamicData* data, DecodedOp* op) {
    dynamic_pop32(data);
    DynReg reg = data->loadReg(4, DYN_SRC, DYN_32bit);
    data->instRegImm('+', reg, DYN_32bit, op->imm);
    data->storeReg(4, reg, DYN_32bit, true);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDone(false);
}
void dynamic_retn16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDone(false);
}
void dynamic_retn32(DynamicData* data, DecodedOp* op) {
    dynamic_pop32(data);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDone(false);
}
void dynamic_invalid(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    data->blockDone(true);
}
void dynamic_ud2(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_ud2, false, 1, 0, DYN_PARAM_CPU, false);
    data->blockDone(true);
}
void dynamic_int80(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)ksyscall, false, 2, 0, DYN_PARAM_CPU, false, op->len, DYN_PARAM_CONST_32, false);
    data->blockDone(false);
}
void dynamic_int99(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_int99, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_int9A(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_int9A, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_int9B(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_int9B, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_intIb(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_intIb, false, 1, 0, DYN_PARAM_CPU, false);
    data->blockDone(false);
}
void dynamic_int3(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_int3, false, 1, 0, DYN_PARAM_CPU, false);
    data->blockDone(false);
}
void dynamic_xlat(DynamicData* data, DecodedOp* op) {
    data->loadReg(0, DYN_SRC, DYN_8bit, true);
    if (op->ea16) {
        data->loadReg(3, DYN_ADDRESS, DYN_16bit, true);
        data->movToRegFromReg(DYN_SRC, DYN_16bit, DYN_SRC, DYN_8bit, false);
        data->instRegReg('+', DYN_ADDRESS, DYN_SRC, DYN_16bit, true);
        data->movToRegFromReg(DYN_ADDRESS, DYN_32bit, DYN_ADDRESS, DYN_16bit, false);
    } else {
        data->loadReg(3, DYN_ADDRESS, DYN_32bit, true);
        data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
        data->instRegReg('+', DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    }
    data->loadSegAddress(op->base, DYN_SRC);
    data->instRegReg('+', DYN_ADDRESS, DYN_SRC, DYN_32bit, true);
    data->movFromMem(DYN_8bit, DYN_ADDRESS, true);
    data->storeReg(0, DYN_CALL_RESULT, DYN_8bit, true);
    data->incrementEip(op->len);
}
void dynamic_hlt(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_hlt, false, 1, 0, DYN_PARAM_CPU, false);
    data->blockDone(true);
}
void dynamic_cmc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->instCPUImm('^', CPU_OFFSET_OF(flags), DYN_32bit, CF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_clc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->instCPUImm('&', CPU_OFFSET_OF(flags), DYN_32bit, ~CF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_stc(DynamicData* data, DecodedOp* op) {
    dynamic_fillFlags(data);
    data->instCPUImm('|', CPU_OFFSET_OF(flags), DYN_32bit, CF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_cli(DynamicData* data, DecodedOp* op) {
    data->instCPUImm('&', CPU_OFFSET_OF(flags), DYN_32bit, ~IF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_sti(DynamicData* data, DecodedOp* op) {
    data->instCPUImm('|', CPU_OFFSET_OF(flags), DYN_32bit, IF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_cld(DynamicData* data, DecodedOp* op) {
    data->instCPUImm('&', CPU_OFFSET_OF(flags), DYN_32bit, ~DF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_std(DynamicData* data, DecodedOp* op) {
    data->instCPUImm('|', CPU_OFFSET_OF(flags), DYN_32bit, DF, DYN_DEST);
    data->incrementEip(op->len);
}
void dynamic_rdtsc(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_rdtsc, false, 2, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_cpuid(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_cpuid, false, 1, 0, DYN_PARAM_CPU, false);
    data->incrementEip(op->len);
}
void dynamic_enter16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_enter32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_enter, false, 4, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, op->imm, DYN_PARAM_CONST_32, false, op->data.disp, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_leave16(DynamicData* data, DecodedOp* op) {
    data->loadRegStoreReg(4, 5, DYN_16bit, DYN_SRC, true);
    data->callHostFunction((void*)common_pop16, true, 1, 0, DYN_PARAM_CPU, false);
    data->storeReg(5, DYN_CALL_RESULT, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_leave32(DynamicData* data, DecodedOp* op) {
    data->loadRegStoreReg(4, 5, DYN_32bit, DYN_SRC, true);
    dynamic_pop32(data);
    data->storeReg(5, DYN_CALL_RESULT, DYN_32bit, true);
    data->incrementEip(op->len);
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

    data->loadReg(1, DYN_DEST, width, true);
    data->instRegImm('-', DYN_DEST, width, 1);
    data->storeReg(1, DYN_DEST, width, false);

    // if nz flag, then set 1 in DYN_SRC
    data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    data->instRegImm('<', DYN_SRC, width, op->ea16 ? 15 : 31);
    data->instRegImm(')', DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is nz flag else 0) to CX
    data->instRegReg('&', DYN_DEST, DYN_SRC, width, true);

    if (width == DYN_16bit) {
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
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

    data->loadReg(1, DYN_DEST, width, true);
    data->instRegImm('-', DYN_DEST, width, 1);
    data->storeReg(1, DYN_DEST, width, false);

    // if z flag, then set 1 in DYN_SRC
    data->evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, false, false);
    // if DYN_SRC is 1 then expand into a mask of 0xffffffff
    data->instRegImm('<', DYN_SRC, width, op->ea16 ? 15 : 31);
    data->instRegImm(')', DYN_SRC, width, op->ea16 ? 15 : 31);
    // apply mask (0xffffffff is z flag else 0) to CX
    data->instRegReg('&', DYN_DEST, DYN_SRC, width, true);    

    if (width == DYN_16bit) {
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
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
    data->loadReg(1, DYN_DEST, width, true);
    data->instRegImm('-', DYN_DEST, width, 1);
    data->storeReg(1, DYN_DEST, width, false);

    if (width == DYN_16bit) {
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
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
    data->loadReg(1, DYN_DEST, width, true);

    if (width == DYN_16bit) {
        data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, width, false);
    }
    dynamic_jumpIfRegNotSet(data, op, DYN_DEST, true);
}
void dynamic_InAlIb(DynamicData* data, DecodedOp* op) {
    data->storeReg(0, DYN_8bit, 0xFF);
    data->incrementEip(op->len);
}
void dynamic_InAxIb(DynamicData* data, DecodedOp* op) {
    data->storeReg(0, DYN_16bit, 0xFFFF);
    data->incrementEip(op->len);
}
void dynamic_InEaxIb(DynamicData* data, DecodedOp* op) {
    data->storeReg(0, DYN_32bit, 0xFFFFFFFF);
    data->incrementEip(op->len);
}
void dynamic_OutIbAl(DynamicData* data, DecodedOp* op) {
    // do nothing
    data->incrementEip(op->len);
}
void dynamic_OutIbAx(DynamicData* data, DecodedOp* op) {
    // do nothing
    data->incrementEip(op->len);
}
void dynamic_OutIbEax(DynamicData* data, DecodedOp* op) {
    // do nothing
    data->incrementEip(op->len);
}
void dynamic_InAlDx(DynamicData* data, DecodedOp* op) {
    data->storeReg(0, DYN_8bit, 0xFF);
    data->incrementEip(op->len);
}
void dynamic_InAxDx(DynamicData* data, DecodedOp* op) {
    data->storeReg(0, DYN_16bit, 0xFFFF);
    data->incrementEip(op->len);
}
void dynamic_InEaxDx(DynamicData* data, DecodedOp* op) {
    data->storeReg(0, DYN_32bit, 0xFFFFFFFF);
    data->incrementEip(op->len);
}
void dynamic_OutDxAl(DynamicData* data, DecodedOp* op) {
    // do nothing
    data->incrementEip(op->len);
}
void dynamic_OutDxAx(DynamicData* data, DecodedOp* op) {
    // do nothing
    data->incrementEip(op->len);
}
void dynamic_OutDxEax(DynamicData* data, DecodedOp* op) {
    // do nothing
    data->incrementEip(op->len);
}
void dynamic_callJw(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len+(S32)((S16)op->imm));
    data->blockCall(op);
}
void dynamic_callJd(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(data, DYN_SRC, true);;
    data->incrementEip(op->len+(S32)op->imm);
    data->blockCall(op);
}
void dynamic_jmp8(DynamicData* data, DecodedOp* op) {
    data->incrementEip(op->len+(S32)((S8)op->imm));
    if (data->canJumpInBlock(op)) {
        data->JumpInBlock(data->currentEip + op->len + (S32)((S8)op->imm));
    } else {
        data->blockNext1(op);
    }
}
void dynamic_jmp16(DynamicData* data, DecodedOp* op) {
    data->incrementEip(op->len+(S32)((S16)op->imm));
    if (data->canJumpInBlock(op)) {
        data->JumpInBlock(data->currentEip + op->len + (S32)((S16)op->imm));
    } else {
        data->blockNext1(op);
    }
}
void dynamic_jmp32(DynamicData* data, DecodedOp* op) {
    data->incrementEip(op->len+(S32)op->imm);
    if (data->canJumpInBlock(op)) {
        data->JumpInBlock(data->currentEip + op->len + (S32)op->imm);
    } else {
        data->blockNext1(op);
    }
}
void dynamic_callR16(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->loadReg(op->reg, DYN_DEST, DYN_16bit, true);
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, true);    
    data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_DEST, DYN_16bit, false); 
    data->storeEip(DYN_DEST, true);
    data->blockDoneCall();
}
void dynamic_callR32(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    DynReg reg = data->loadReg(op->reg, DYN_CALL_RESULT, DYN_32bit);
    dynamic_pushReg32(data, DYN_SRC, true);
    data->storeEip(reg, true);
    data->blockDoneCall();
}
void dynamic_callE16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, true); // DYN_CALL_RESULT could get clobbered before used
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    data->callHostFunction((void*)common_push16, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_16, true);
    data->storeEip(DYN_DEST, true);
    data->blockDoneCall();
}
void dynamic_callE32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, true);
    data->loadEip(DYN_SRC);
    data->instRegImm('+', DYN_SRC, DYN_32bit, op->len);
    dynamic_pushReg32(data, DYN_SRC, true);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDoneCall();
}
void dynamic_jmpR16(DynamicData* data, DecodedOp* op) {
    data->loadReg(op->reg, DYN_SRC, DYN_16bit, true);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_16bit, false);
    data->storeEip(DYN_SRC, true);
    data->blockDoneJump();
}
void dynamic_jmpR32(DynamicData* data, DecodedOp* op) {
    data->loadRegStoreEip(op->reg, DYN_SRC, true);
    data->blockDoneJump();
}
void dynamic_jmpE16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->movToRegFromReg(DYN_CALL_RESULT, DYN_32bit, DYN_CALL_RESULT, DYN_16bit, false);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDoneJump();
}
void dynamic_jmpE32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, true);
    data->storeEip(DYN_CALL_RESULT, true);
    data->blockDoneJump();
}
void dynamic_callFarE16(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_DEST);
    data->instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
    data->movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    data->blockDone(false);
}
void dynamic_callFarE32(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_DEST);
    data->instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_call, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    data->blockDone(false);
}
void dynamic_jmpFarE16(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_DEST);
    data->instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
    data->movToRegFromReg(DYN_SRC, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 0, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_16, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    data->blockDone(false);
}
void dynamic_jmpFarE32(DynamicData* data, DecodedOp* op) {
    data->loadEip(DYN_DEST);
    data->instRegImm('+', DYN_DEST, DYN_32bit, op->len);
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
    data->movToRegFromReg(DYN_SRC, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_jmp, false, 5, 0, DYN_PARAM_CPU, false, 1, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true, DYN_SRC, DYN_PARAM_REG_32, true, DYN_DEST, DYN_PARAM_REG_32, true); 
    data->blockDone(false);
}
void dynamic_larr16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_larr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_larr16e16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_larr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_lslr16r16(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_lslr16r16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_lslr16e16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_lslr16e16, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_lslr32r32(DynamicData* data, DecodedOp* op) {
    data->callHostFunction((void*)common_lslr32r32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, op->rm, DYN_PARAM_CONST_32, false);
    data->incrementEip(op->len);
}
void dynamic_lslr32e32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_lslr32e32, false, 3, 0, DYN_PARAM_CPU, false, op->reg, DYN_PARAM_CONST_32, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_verre16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_verwe16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_verre16, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}
void dynamic_xaddr8r8(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_8bit, true);
        DynReg dst = data->loadReg(op->rm, DYN_DEST, DYN_8bit);
        data->instRegReg('+', DYN_SRC, dst, DYN_8bit, false);
        data->storeReg(op->reg, dst, DYN_8bit, true);
        data->storeReg(op->rm, DYN_SRC, DYN_8bit, true);
    }
    else {
        data->loadRegStoreSrc(op->reg, DYN_8bit, DYN_SRC, false);
        data->loadRegStoreDst(op->rm, DYN_8bit, DYN_DEST, false);
        data->instRegReg('+', DYN_SRC, DYN_DEST, DYN_8bit, false);
        data->storeLazyFlagsResult(DYN_SRC, DYN_8bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_8bit, true);
        data->storeReg(op->rm, DYN_SRC, DYN_8bit, true);
        data->storeLazyFlags(FLAGS_ADD8);
    }
    data->incrementEip(op->len);
}
void dynamic_xaddr8e8(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_8bit, true);
        data->movFromMem(DYN_8bit, DYN_ADDRESS, false);
        data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_8bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
    }
    else {
        data->loadRegStoreSrc(op->reg, DYN_8bit, DYN_SRC, false);
        data->movFromMem(DYN_8bit, DYN_ADDRESS, false);
        data->storeLazyFlagsDst(DYN_CALL_RESULT, DYN_8bit, false);
        data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_8bit, false);
        data->storeLazyFlagsResult(DYN_SRC, DYN_8bit, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_8bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_8bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_ADD8);
    }
    data->incrementEip(op->len);
}
void dynamic_xaddr16r16(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_16bit, true);
        DynReg reg = data->loadReg(op->rm, DYN_DEST, DYN_16bit);
        data->instRegReg('+', DYN_SRC, reg, DYN_16bit, false);
        data->storeReg(op->reg, reg, DYN_16bit, true);
        data->storeReg(op->rm, DYN_SRC, DYN_16bit, true);
    }
    else {
        data->loadRegStoreSrc(op->reg, DYN_16bit, DYN_SRC, false);
        data->loadRegStoreDst(op->rm, DYN_16bit, DYN_DEST, false);
        data->instRegReg('+', DYN_SRC, DYN_DEST, DYN_16bit, false);
        data->storeLazyFlagsResult(DYN_SRC, DYN_16bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_16bit, true);
        data->storeReg(op->rm, DYN_SRC, DYN_16bit, true);
        data->storeLazyFlags(FLAGS_ADD16);
    }
    data->incrementEip(op->len);
}
void dynamic_xaddr16e16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_16bit, true);
        data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
        data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
    }
    else {
        data->loadRegStoreSrc(op->reg, DYN_16bit, DYN_SRC, false);
        data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
        data->storeLazyFlagsDst(DYN_CALL_RESULT, DYN_16bit, false);
        data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_16bit, false);
        data->storeLazyFlagsResult(DYN_SRC, DYN_16bit, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_16bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_16bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_ADD16);
    }
    data->incrementEip(op->len);
}
void dynamic_xaddr32r32(DynamicData* data, DecodedOp* op) {
    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_32bit, true);
        DynReg reg = data->loadReg(op->rm, DYN_DEST, DYN_32bit);
        data->instRegReg('+', DYN_SRC, reg, DYN_32bit, false);
        data->storeReg(op->reg, reg, DYN_32bit, true);
        data->storeReg(op->rm, DYN_SRC, DYN_32bit, true);
    } else {
        data->loadRegStoreSrc(op->reg, DYN_32bit, DYN_SRC, false);
        data->loadRegStoreDst(op->rm, DYN_32bit, DYN_DEST, false);
        data->instRegReg('+', DYN_SRC, DYN_DEST, DYN_32bit, false);
        data->storeLazyFlagsResult(DYN_SRC, DYN_32bit, false);
        data->storeReg(op->reg, DYN_DEST, DYN_32bit, true);
        data->storeReg(op->rm, DYN_SRC, DYN_32bit, true);
        data->storeLazyFlags(FLAGS_ADD32);
    }
    data->incrementEip(op->len);
}
void dynamic_xaddr32e32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);

    if (!op->needsToSetFlags(data->cpu)) {
        data->loadReg(op->reg, DYN_SRC, DYN_32bit, true);
        data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
        data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
    } else {    
        data->loadRegStoreSrc(op->reg, DYN_32bit, DYN_SRC, false);
        data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
        data->storeLazyFlagsDst(DYN_CALL_RESULT, DYN_32bit, false);
        data->instRegReg('+', DYN_SRC, DYN_CALL_RESULT, DYN_32bit, false);
        data->storeLazyFlagsResult(DYN_SRC, DYN_32bit, false);
        data->storeReg(op->reg, DYN_CALL_RESULT, DYN_32bit, true);
        data->movToMemFromReg(DYN_ADDRESS, DYN_SRC, DYN_32bit, true, true, DYN_DEST);
        data->storeLazyFlags(FLAGS_ADD32);
    }
    data->incrementEip(op->len);
}
void dynamic_bswap32(DynamicData* data, DecodedOp* op) {
    DynReg reg = data->loadReg(op->reg, DYN_SRC, DYN_32bit);
    data->byteSwapReg32(reg);
    data->storeReg(op->reg, reg, DYN_32bit, true);
    data->incrementEip(op->len);
}
void dynamic_cmpxchgg8b(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_cmpxchg8b, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->currentLazyFlags=FLAGS_NONE;
    data->incrementEip(op->len);
}
void dynamic_loadSegment16(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, false);
    data->movToRegFromReg(DYN_DEST, DYN_16bit, DYN_CALL_RESULT, DYN_16bit, true);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, 2);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->storeReg(op->reg, DYN_DEST, DYN_16bit, true);
    data->incrementEip(op->len);
}
void dynamic_loadSegment32(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->movFromMem(DYN_32bit, DYN_ADDRESS, false);
    data->movToRegFromReg(DYN_DEST, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    data->instRegImm('+', DYN_ADDRESS, DYN_32bit, 4);
    data->movFromMem(DYN_16bit, DYN_ADDRESS, true);
    data->callHostFunction((void*)common_setSegment, true, 3, 0, DYN_PARAM_CPU, false, op->imm, DYN_PARAM_CONST_32, false, DYN_CALL_RESULT, DYN_PARAM_REG_16, true);
    data->IfNot(DYN_CALL_RESULT, true);
    data->blockDone(true);
    data->EndIf();
    data->storeReg(op->reg, DYN_DEST, DYN_32bit, true);
    data->incrementEip(op->len);
}

void dynamic_fxsave(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_fxsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}

void dynamic_fxrstor(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_fxrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}

void dynamic_xsave(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_xsave, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}

void dynamic_xrstor(DynamicData* data, DecodedOp* op) {
    data->calculateEaa(op, DYN_ADDRESS);
    data->callHostFunction((void*)common_xrstor, false, 2, 0, DYN_PARAM_CPU, false, DYN_ADDRESS, DYN_PARAM_REG_32, true);
    data->incrementEip(op->len);
}