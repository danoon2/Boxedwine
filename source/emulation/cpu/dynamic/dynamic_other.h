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
    RegPtr result = callAndReturn_IR(common_bound16, op->reg, DYN_32bit, calculateEaa2(op));
    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_bound32(DecodedOp* op) {
    RegPtr result = callAndReturn_IR(common_bound32, op->reg, DYN_32bit, calculateEaa2(op));
    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_daa(DecodedOp* op) {
    call(daa);
    incrementEip(op->len);
}
void DynamicData::dynamic_das(DecodedOp* op) {
    call(das);
    incrementEip(op->len);
}
void DynamicData::dynamic_aaa(DecodedOp* op) {
    call(aaa);
    incrementEip(op->len);
}
void DynamicData::dynamic_aas(DecodedOp* op) {
    call(aas);
    incrementEip(op->len);
}
void DynamicData::dynamic_aad(DecodedOp* op) {
    call_I(aad, op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_aam(DecodedOp* op) {
    RegPtr result = callAndReturn_I(aam, op->imm);
    IfNot(DYN_32bit, result);
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
    RegPtr dx = getReg(2);
    mov(DYN_16bit, dx, getReg(0));
    sarValue(DYN_16bit, dx, 15, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_cwq(DecodedOp* op) {
    RegPtr edx = getReg(2);
    mov(DYN_32bit, edx, getReg(0));
    sarValue(DYN_32bit, edx, 31, false);
    incrementEip(op->len);
}
void DynamicData::dynamic_callAp(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    call_IIIR(common_call, 0, op->imm, op->data.disp, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_callFar(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    call_IIIR(common_call, 1, op->imm, op->data.disp, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_jmpAp(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    call_IIIR(common_jmp, 0, op->imm, op->data.disp, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_jmpFar(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    call_IIIR(common_jmp, 1, op->imm, op->data.disp, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_retf16(DecodedOp* op) {
    call_II(common_ret, 0, op->imm);
    blockDone(false);
}
void DynamicData::dynamic_retf32(DecodedOp* op) {
    call_II(common_ret, 1, op->imm);
    blockDone(false);
}
void DynamicData::dynamic_iret(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    call_IR(common_iret, 0, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_iret32(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    call_IR(common_iret, 1, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_sahf(DecodedOp* op) {
    dynamic_fillFlags();
    loadReg(4, DYN_SRC, DYN_8bit);
    movToRegFromReg(DYN_SRC, DYN_32bit, DYN_SRC, DYN_8bit, false);
    setCPUFlags(DYN_SRC, FMASK_ALL & 0xFF, DYN_DEST, true);
    currentLazyFlags = FLAGS_NONE;
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
    pop16(getEip(false), op->imm + 2);
    blockDone(false);
}
void DynamicData::dynamic_retn32Iw(DecodedOp* op) {
    pop32(getEip(false), op->imm + 4);
    blockDone(false);
}
void DynamicData::dynamic_retn16(DecodedOp* op) {
    pop16(getEip(false));
    blockDone(false);
}
void DynamicData::dynamic_retn32(DecodedOp* op) {
    pop32(getEip(false));
    blockDone(false);
}
void DynamicData::dynamic_invalid(DecodedOp* op) {
    call(common_ud2);
    blockDone(true);
}
void DynamicData::dynamic_ud2(DecodedOp* op) {
    call(common_ud2);
    blockDone(true);
}
void DynamicData::dynamic_int80(DecodedOp* op) {
    call_I(ksyscall, op->len);
    blockDone(false);
}
void DynamicData::dynamic_int99(DecodedOp* op) {
    call(common_int99);
    incrementEip(op->len);
}
void DynamicData::dynamic_int9A(DecodedOp* op) {
    call(common_int9A);
    incrementEip(op->len);
}
void DynamicData::dynamic_int9B(DecodedOp* op) {
    call(common_int9B);
    incrementEip(op->len);
}
void DynamicData::dynamic_intIb(DecodedOp* op) {
    call(common_intIb);
    blockDone(false);
}
void DynamicData::dynamic_int3(DecodedOp* op) {
    call(common_int3);
    blockDone(false);
}
void DynamicData::dynamic_xlat(DecodedOp* op) {
    RegPtr address = getTmpReg8(0);

    if (op->ea16) {
        // AL = cpu->memory->readb(cpu->seg[op->base].address + (U16)(BX + AL));
        movzx(DYN_16bit, address, DYN_8bit, address);
        addReg(DYN_16bit, address, getReadOnlyReg(3), false);
        movzx(DYN_32bit, address, DYN_16bit, address);
    } else {
        // AL = cpu->memory->readb(cpu->seg[op->base].address + EBX + AL);
        movzx(DYN_32bit, address, DYN_8bit, address);
        addReg(DYN_32bit, address, getReadOnlyReg(3), false);
    }
    addReg(DYN_32bit, address, getReadOnlySegAddress(op->base), false);
    mov(DYN_8bit, getReg8(0), read(DYN_8bit, address));
    incrementEip(op->len);
}
void DynamicData::dynamic_hlt(DecodedOp* op) {
    call(common_hlt);
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
    call_I(common_rdtsc, op->imm);
    incrementEip(op->len);
}
void DynamicData::dynamic_cpuid(DecodedOp* op) {
    call(common_cpuid);
    incrementEip(op->len);
}
void DynamicData::dynamic_enter16(DecodedOp* op) {
    call_III(common_enter, 0, op->imm, op->data.disp);
    incrementEip(op->len);
}
void DynamicData::dynamic_enter32(DecodedOp* op) {
    call_III(common_enter, 1, op->imm, op->data.disp);
    incrementEip(op->len);
}
void DynamicData::dynamic_leave16(DecodedOp* op) {
    // ESP and EBP each get loaded more than once, on a platform that doesnt' cache them, like x86, it might be interesting to reduce this
    mov(DYN_16bit, getReg(4), getReadOnlyReg(5));
    mov(DYN_16bit, getReg(5), pop16());
    incrementEip(op->len);
}
void DynamicData::dynamic_leave32(DecodedOp* op) {
    mov(DYN_32bit, getReg(4), getReadOnlyReg(5));
    mov(DYN_32bit, getReg(5), pop32());
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

    RegPtr reg = getTmpReg();
    {
        RegPtr cx = getReg(1);
        decReg(width, cx, false);
        mov(width, reg, cx);
    }
    IfCondition(Z);
        movValue(width, reg, 0);
    EndIf();
    If(width, reg);
        incrementEip(op->len + (S32)((S8)op->imm));
        if (canJumpInBlock(op)) {                
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(op);
        }
    EndIf();
    incrementEip(op->len);
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

    RegPtr reg = getTmpReg();
    {
        RegPtr cx = getReg(1);
        decReg(width, cx, false);
        mov(width, reg, cx);
    }
    IfCondition(NZ);
        movValue(width, reg, 0);
    EndIf();
    If(width, reg);
        incrementEip(op->len + (S32)((S8)op->imm));
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(op);
        }
    EndIf();
    incrementEip(op->len);
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

    {
        // commit cx before doing if statement
        RegPtr cx = getReg(1);
        decReg(width, cx, false);
    }

    If(width, getReadOnlyReg(1));
        incrementEip(op->len + (S32)((S8)op->imm));
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(op);
        }
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_jcxz(DecodedOp* op) {
    // if (ECX == 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    DynWidth width = op->ea16 ? DYN_16bit : DYN_32bit;

    IfNot(width, getReadOnlyReg(1));
        incrementEip(op->len + (S32)((S8)op->imm));
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(op);
        }
    EndIf();
    incrementEip(op->len);
}
void DynamicData::dynamic_InAlIb(DecodedOp* op) {
    movValue(DYN_8bit, getReg8(0), 0xff);
    incrementEip(op->len);
}
void DynamicData::dynamic_InAxIb(DecodedOp* op) {
    movValue(DYN_16bit, getReg(0), 0xffff);
    incrementEip(op->len);
}
void DynamicData::dynamic_InEaxIb(DecodedOp* op) {
    movValue(DYN_32bit, getReg(0), 0xffffffff);
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
    movValue(DYN_8bit, getReg8(0), 0xff);
    incrementEip(op->len);
}
void DynamicData::dynamic_InAxDx(DecodedOp* op) {
    movValue(DYN_16bit, getReg(0), 0xffff);
    incrementEip(op->len);
}
void DynamicData::dynamic_InEaxDx(DecodedOp* op) {
    movValue(DYN_32bit, getReg(0), 0xffffffff);
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
    RegPtr eip = getTmpEip();
    addValue(DYN_16bit, eip, op->len, false);
    push16(eip);    
    incrementEip(op->len+(S32)((S16)op->imm));
    blockCall(op);
}
void DynamicData::dynamic_callJd(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);
    push32(eip);
    incrementEip(op->len + (S32)op->imm);
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
    {
        RegPtr eip = getEip();
        addValue(DYN_16bit, eip, op->len, false);
        
        if (op->reg == 4) {            
            RegPtr reg = getTmpReg(op->reg);
            push16(eip);
            mov(DYN_16bit, eip, reg);
        } else {
            push16(eip);
            mov(DYN_16bit, eip, getReadOnlyReg(op->reg));
        }
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void DynamicData::dynamic_callR32(DecodedOp* op) {
    {
        RegPtr eip = getEip();
        addValue(DYN_32bit, eip, op->len, false);

        if (op->reg == 4) {
            RegPtr reg = getTmpReg(op->reg);
            push32(eip);
            mov(DYN_32bit, eip, reg);
        } else {
            push32(eip);
            mov(DYN_32bit, eip, getReadOnlyReg(op->reg));
        }
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void DynamicData::dynamic_callE16(DecodedOp* op) {
    {
        RegPtr newEip = read(DYN_16bit, calculateEaa2(op)); // read before push, incase esp is used
        RegPtr eip = getEip();
        addValue(DYN_16bit, eip, op->len, false);
        push16(eip);
        mov(DYN_16bit, eip, newEip);
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void DynamicData::dynamic_callE32(DecodedOp* op) {
    {
        RegPtr newEip = read(DYN_32bit, calculateEaa2(op)); // read before push, incase esp is used
        RegPtr eip = getEip();
        addValue(DYN_32bit, eip, op->len, false);
        push32(eip);
        mov(DYN_32bit, eip, newEip);
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void DynamicData::dynamic_jmpR16(DecodedOp* op) {
    mov(DYN_16bit, getEip(), getReadOnlyReg(op->reg));
    blockDoneJump();
}
void DynamicData::dynamic_jmpR32(DecodedOp* op) {
    mov(DYN_32bit, getEip(), getReadOnlyReg(op->reg));
    blockDoneJump();
}
void DynamicData::dynamic_jmpE16(DecodedOp* op) {
    mov(DYN_16bit, getEip(), read(DYN_16bit, calculateEaa2(op)));
    blockDoneJump();
}
void DynamicData::dynamic_jmpE32(DecodedOp* op) {
    mov(DYN_32bit, getEip(), read(DYN_32bit, calculateEaa2(op)));
    blockDoneJump();
}
void DynamicData::dynamic_callFarE16(DecodedOp* op) {
    RegPtr address = calculateEaa2(op);
    RegPtr offset = read(DYN_16bit, address, nullptr, nullptr, false, getTmpReg());
    addValue(DYN_32bit, address, 2, false);
    RegPtr sel = read(DYN_16bit, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(DYN_16bit, eip, op->len, false);

    call_IRRR(common_call, 0, DYN_16bit, sel, DYN_16bit, offset, DYN_16bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_callFarE32(DecodedOp* op) {
    RegPtr address = calculateEaa2(op);
    RegPtr offset = read(DYN_32bit, address, nullptr, nullptr, false, getTmpReg());
    addValue(DYN_32bit, address, 4, false);
    RegPtr sel = read(DYN_16bit, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);

    call_IRRR(common_call, 1, DYN_16bit, sel, DYN_32bit, offset, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_jmpFarE16(DecodedOp* op) {
    RegPtr address = calculateEaa2(op);
    RegPtr offset = read(DYN_16bit, address, nullptr, nullptr, false, getTmpReg());
    addValue(DYN_32bit, address, 2, false);
    RegPtr sel = read(DYN_16bit, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(DYN_16bit, eip, op->len, false);

    call_IRRR(common_jmp, 0, DYN_16bit, sel, DYN_16bit, offset, DYN_16bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_jmpFarE32(DecodedOp* op) {
    RegPtr address = calculateEaa2(op);
    RegPtr offset = read(DYN_32bit, address, nullptr, nullptr, false, getTmpReg());
    addValue(DYN_32bit, address, 4, false);
    RegPtr sel = read(DYN_16bit, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(DYN_32bit, eip, op->len, false);

    call_IRRR(common_jmp, 1, DYN_16bit, sel, DYN_32bit, offset, DYN_32bit, eip);
    blockDone(false);
}
void DynamicData::dynamic_larr16r16(DecodedOp* op) {
    call_II(common_larr16r16, op->reg, op->rm);
    incrementEip(op->len);
}
void DynamicData::dynamic_larr16e16(DecodedOp* op) {
    call_IR(common_larr16e16, op->reg, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr16r16(DecodedOp* op) {
    call_II(common_lslr16r16, op->reg, op->rm);
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr16e16(DecodedOp* op) {
    call_IR(common_lslr16e16, op->reg, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr32r32(DecodedOp* op) {
    call_II(common_lslr32r32, op->reg, op->rm);
    incrementEip(op->len);
}
void DynamicData::dynamic_lslr32e32(DecodedOp* op) {
    call_IR(common_lslr32e32, op->reg, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_verre16(DecodedOp* op) {
    call_R(common_verre16, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_verwe16(DecodedOp* op) {
    call_R(common_verwe16, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_xaddr8r8(DecodedOp* op) {
    dynamic_RR_WriteBoth(op, DYN_8bit, & DynamicData::xaddReg);
}
void DynamicData::dynamic_xaddr8e8(DecodedOp* op) {
    dynamic_RM_WriteM(op, DYN_8bit, &DynamicData::xaddReg);
}
void DynamicData::dynamic_xaddr16r16(DecodedOp* op) {
    dynamic_RR_WriteBoth(op, DYN_16bit, &DynamicData::xaddReg);
}
void DynamicData::dynamic_xaddr16e16(DecodedOp* op) {
    dynamic_RM_WriteM(op, DYN_16bit, &DynamicData::xaddReg);
}
void DynamicData::dynamic_xaddr32r32(DecodedOp* op) {
    dynamic_RR_WriteBoth(op, DYN_32bit, &DynamicData::xaddReg);
}
void DynamicData::dynamic_xaddr32e32(DecodedOp* op) {
    dynamic_RM_WriteM(op, DYN_32bit, &DynamicData::xaddReg);
}
void DynamicData::dynamic_bswap32(DecodedOp* op) {
    byteSwapReg32(getReg(op->reg));
    incrementEip(op->len);
}
void DynamicData::dynamic_cmpxchgg8b(DecodedOp* op) {
    call_R(common_cmpxchg8b, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}
void DynamicData::dynamic_loadSegment16(DecodedOp* op) {
    //U16 val = cpu->memory->readw(eaa);
    //U32 selector = cpu->memory->readw(eaa + 2);
    //if (cpu->setSegment(op->imm, selector)) {
    //    cpu->reg[op->reg].u16 = val;

    RegPtr address = calculateEaa2(op);
    RegPtr value = read(DYN_16bit, address, nullptr, nullptr, false, getTmpReg());
    addValue(DYN_32bit, address, 2, false);
    RegPtr sel = read(DYN_16bit, std::move(address), nullptr, nullptr, false, getTmpReg()); // std::move will release address in this function and give it to read, this way read will see there is only 1 reference to address and can re-use it as a tmp register

    RegPtr result = callAndReturn_IR(common_setSegment, op->imm, DYN_16bit, sel);

    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    mov(DYN_16bit, getReg(op->reg), value);
    incrementEip(op->len);
}
void DynamicData::dynamic_loadSegment32(DecodedOp* op) {
    RegPtr address = calculateEaa2(op);
    RegPtr value = read(DYN_32bit, address, nullptr, nullptr, false, getTmpReg());
    addValue(DYN_32bit, address, 4, false);
    RegPtr sel = read(DYN_16bit, std::move(address), nullptr, nullptr, false, getTmpReg()); // std::move will release address in this function and give it to read, this way read will see there is only 1 reference to address and can re-use it as a tmp register

    RegPtr result = callAndReturn_IR(common_setSegment, op->imm, DYN_16bit, sel);

    IfNot(DYN_32bit, result);
        blockDone(true);
    EndIf();
    mov(DYN_32bit, getReg(op->reg), value);
    incrementEip(op->len);
}

void DynamicData::dynamic_fxsave(DecodedOp* op) {
    call_R(common_fxsave, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}

void DynamicData::dynamic_fxrstor(DecodedOp* op) {
    call_R(common_fxrstor, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}

void DynamicData::dynamic_xsave(DecodedOp* op) {
    call_R(common_xsave, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}

void DynamicData::dynamic_xrstor(DecodedOp* op) {
    call_R(common_xrstor, DYN_32bit, calculateEaa2(op));
    incrementEip(op->len);
}