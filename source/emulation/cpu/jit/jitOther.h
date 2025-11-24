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
void Jit::dynamic_bound16(DecodedOp* op) {
    RegPtr result = callAndReturn_IR(common_bound16, op->reg, JitWidth::b32, calculateEaa(op));
    IfNot(JitWidth::b32, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void Jit::dynamic_bound32(DecodedOp* op) {
    RegPtr result = callAndReturn_IR(common_bound32, op->reg, JitWidth::b32, calculateEaa(op));
    IfNot(JitWidth::b32, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void Jit::dynamic_daa(DecodedOp* op) {
    call(daa);
    incrementEip(op->len);
}
void Jit::dynamic_das(DecodedOp* op) {
    call(das);
    incrementEip(op->len);
}
void Jit::dynamic_aaa(DecodedOp* op) {
    call(aaa);
    incrementEip(op->len);
}
void Jit::dynamic_aas(DecodedOp* op) {
    call(aas);
    incrementEip(op->len);
}
void Jit::dynamic_aad(DecodedOp* op) {
    call_I(aad, op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_aam(DecodedOp* op) {
    RegPtr result = callAndReturn_I(aam, op->imm);
    IfNot(JitWidth::b32, result);
        blockDone(true);
    EndIf();
    incrementEip(op->len);
}
void Jit::dynamic_nop(DecodedOp* op) {
    // Nop
    incrementEip(op->len);
}
void Jit::dynamic_done(DecodedOp* op) {
    incrementEip(op->len);
    blockDone(false);
}
void Jit::dynamic_wait(DecodedOp* op) {
    // Wait
    incrementEip(op->len);
}
void Jit::dynamic_cwd(DecodedOp* op) {
    RegPtr dx = getReg(2);
    mov(JitWidth::b16, dx, getReg(0));
    sarValue(JitWidth::b16, dx, 15);
    incrementEip(op->len);
}
void Jit::dynamic_cwq(DecodedOp* op) {
    RegPtr edx = getReg(2);
    mov(JitWidth::b32, edx, getReg(0));
    sarValue(JitWidth::b32, edx, 31);
    incrementEip(op->len);
}
void Jit::dynamic_callAp(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    call_IIIR(common_call, 0, op->imm, op->data.disp, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_callFar(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    call_IIIR(common_call, 1, op->imm, op->data.disp, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_jmpAp(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    call_IIIR(common_jmp, 0, op->imm, op->data.disp, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_jmpFar(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    call_IIIR(common_jmp, 1, op->imm, op->data.disp, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_retf16(DecodedOp* op) {
    call_II(common_ret, 0, op->imm);
    blockDone(false);
}
void Jit::dynamic_retf32(DecodedOp* op) {
    call_II(common_ret, 1, op->imm);
    blockDone(false);
}
void Jit::dynamic_iret(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    call_IR(common_iret, 0, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_iret32(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    call_IR(common_iret, 1, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_sahf(DecodedOp* op) {
    RegPtr flags = getTmpReg();
    movzx(JitWidth::b32, flags, JitWidth::b8, getReadOnlyReg8(4));
    setFlags(flags, FMASK_ALL & 0xFF);
    currentLazyFlags = FLAGS_NONE;
    incrementEip(op->len);
}
void Jit::dynamic_lahf(DecodedOp* op) {
    RegPtr flags = getReadOnlyFlags();

    andValue(JitWidth::b32, flags, SF | ZF | AF | PF | CF);
    orValue(JitWidth::b32, flags, 2);
    mov(JitWidth::b8, getReg8(4), flags);
    incrementEip(op->len);
}
void Jit::dynamic_salc(DecodedOp* op) {
    RegPtr cf = getCF();
    negReg2(JitWidth::b8, cf);
    mov(JitWidth::b8, getReg8(0), cf);
    incrementEip(op->len);
}
void Jit::dynamic_retn16Iw(DecodedOp* op) {
    pop16(getEip(false), op->imm + 2);
    blockDone(false);
}
void Jit::dynamic_retn32Iw(DecodedOp* op) {
    pop32(getEip(false), op->imm + 4);
    blockDone(false);
}
void Jit::dynamic_retn16(DecodedOp* op) {
    pop16(getEip(false));
    blockDone(false);
}
void Jit::dynamic_retn32(DecodedOp* op) {
    pop32(getEip(false));
    blockDone(false);
}
void Jit::dynamic_invalid(DecodedOp* op) {
    call(common_ud2);
    blockDone(true);
}
void Jit::dynamic_ud2(DecodedOp* op) {
    call(common_ud2);
    blockDone(true);
}
void Jit::dynamic_int80(DecodedOp* op) {
    call_I(ksyscall, op->len);
    blockDone(false);
}
void Jit::dynamic_int99(DecodedOp* op) {
    call(common_int99);
    incrementEip(op->len);
}
void Jit::dynamic_int9A(DecodedOp* op) {
    call(common_int9A);
    incrementEip(op->len);
}
void Jit::dynamic_int9B(DecodedOp* op) {
    call(common_int9B);
    incrementEip(op->len);
}
void Jit::dynamic_intIb(DecodedOp* op) {
    call(common_intIb);
    blockDone(false);
}
void Jit::dynamic_int3(DecodedOp* op) {
    call(common_int3);
    blockDone(false);
}
void Jit::dynamic_xlat(DecodedOp* op) {
    RegPtr address = getTmpReg8(0);

    if (op->ea16) {
        // AL = cpu->memory->readb(cpu->seg[op->base].address + (U16)(BX + AL));
        movzx(JitWidth::b16, address, JitWidth::b8, address);
        addReg(JitWidth::b16, address, getReadOnlyReg(3));
        movzx(JitWidth::b32, address, JitWidth::b16, address);
    } else {
        // AL = cpu->memory->readb(cpu->seg[op->base].address + EBX + AL);
        movzx(JitWidth::b32, address, JitWidth::b8, address);
        addReg(JitWidth::b32, address, getReadOnlyReg(3));
    }
    addReg(JitWidth::b32, address, getReadOnlySegAddress(op->base));
    mov(JitWidth::b8, getReg8(0), read(JitWidth::b8, address));
    incrementEip(op->len);
}
void Jit::dynamic_hlt(DecodedOp* op) {
    call(common_hlt);
    blockDone(true);
}
void Jit::dynamic_cmc(DecodedOp* op) {
    fillFlags();
    xorCPUFlagsImmV2(CF);
    incrementEip(op->len);
}
void Jit::dynamic_clc(DecodedOp* op) {
    fillFlags();
    andCPUFlagsImmV2(~CF);
    incrementEip(op->len);
}
void Jit::dynamic_stc(DecodedOp* op) {
    fillFlags();
    orCPUFlagsImmV2(CF);
    incrementEip(op->len);
}
void Jit::dynamic_cli(DecodedOp* op) {
    andCPUFlagsImmV2(~IF);
    incrementEip(op->len);
}
void Jit::dynamic_sti(DecodedOp* op) {
    orCPUFlagsImmV2(IF);
    incrementEip(op->len);
}
void Jit::dynamic_cld(DecodedOp* op) {
    andCPUFlagsImmV2(~DF);
    incrementEip(op->len);
}
void Jit::dynamic_std(DecodedOp* op) {
    orCPUFlagsImmV2(DF);
    incrementEip(op->len);
}
void Jit::dynamic_rdtsc(DecodedOp* op) {
    call_I(common_rdtsc, op->imm);
    incrementEip(op->len);
}
void Jit::dynamic_cpuid(DecodedOp* op) {
    call(common_cpuid);
    incrementEip(op->len);
}
void Jit::dynamic_enter16(DecodedOp* op) {
    call_III(common_enter, 0, op->imm, op->data.disp);
    incrementEip(op->len);
}
void Jit::dynamic_enter32(DecodedOp* op) {
    call_III(common_enter, 1, op->imm, op->data.disp);
    incrementEip(op->len);
}
void Jit::dynamic_leave16(DecodedOp* op) {
    // ESP and EBP each get loaded more than once, on a platform that doesnt' cache them, like x86, it might be interesting to reduce this
    mov(JitWidth::b16, getReg(4), getReadOnlyReg(5));
    mov(JitWidth::b16, getReg(5), pop16());
    incrementEip(op->len);
}
void Jit::dynamic_leave32(DecodedOp* op) {
    mov(JitWidth::b32, getReg(4, -1, false), getReadOnlyReg(5));
    mov(JitWidth::b32, getReg(5, -1, false), pop32());
    incrementEip(op->len);
}
void Jit::dynamic_loopnz(DecodedOp* op) {
    // CX--;
    // if (CX != 0 && !cpu->getZF()) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;

    RegPtr reg = getTmpReg();
    {
        RegPtr cx = getReg(1);
        decReg(width, cx);
        mov(width, reg, cx);
    }
    IfCondition(JitConditional::Z);
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
    if (!canJumpInBlock(op)) {
        blockNext2(op);
    }
}
void Jit::dynamic_loopz(DecodedOp* op) {
    // CX--;
    // if (CX != 0 && cpu->getZF()) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;

    RegPtr reg = getTmpReg();
    {
        RegPtr cx = getReg(1);
        decReg(width, cx);
        mov(width, reg, cx);
    }
    IfCondition(JitConditional::NZ);
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
    if (!canJumpInBlock(op)) {
        blockNext2(op);
    }
}

void Jit::dynamic_loop(DecodedOp* op) {
    // ECX--;
    // if (ECX != 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;

    decReg(width, getReg(1));

    If(width, getReadOnlyReg(1));
        incrementEip(op->len + (S32)((S8)op->imm));
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            blockNext1(op);
        }
    EndIf();
    incrementEip(op->len);
    if (!canJumpInBlock(op)) {
        blockNext2(op);
    }
}
void Jit::dynamic_jcxz(DecodedOp* op) {
    // if (ECX == 0) {
    //    NEXT_BRANCH1();
    //}
    //else {
    //    NEXT_BRANCH2();
    //}
    JitWidth width = op->ea16 ? JitWidth::b16 : JitWidth::b32;

    IfNot(width, getReadOnlyReg(1));
        incrementEip(op->len + op->imm);
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + op->imm);
        } else {
            blockNext1(op);
        }
    EndIf();
    incrementEip(op->len);
    if (!canJumpInBlock(op)) {
        blockNext2(op);
    }
}
void Jit::dynamic_InAlIb(DecodedOp* op) {
    movValue(JitWidth::b8, getReg8(0), 0xff);
    incrementEip(op->len);
}
void Jit::dynamic_InAxIb(DecodedOp* op) {
    movValue(JitWidth::b16, getReg(0), 0xffff);
    incrementEip(op->len);
}
void Jit::dynamic_InEaxIb(DecodedOp* op) {
    movValue(JitWidth::b32, getReg(0), 0xffffffff);
    incrementEip(op->len);
}
void Jit::dynamic_OutIbAl(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void Jit::dynamic_OutIbAx(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void Jit::dynamic_OutIbEax(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void Jit::dynamic_InAlDx(DecodedOp* op) {
    movValue(JitWidth::b8, getReg8(0), 0xff);
    incrementEip(op->len);
}
void Jit::dynamic_InAxDx(DecodedOp* op) {
    movValue(JitWidth::b16, getReg(0), 0xffff);
    incrementEip(op->len);
}
void Jit::dynamic_InEaxDx(DecodedOp* op) {
    movValue(JitWidth::b32, getReg(0), 0xffffffff);
    incrementEip(op->len);
}
void Jit::dynamic_OutDxAl(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void Jit::dynamic_OutDxAx(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void Jit::dynamic_OutDxEax(DecodedOp* op) {
    // do nothing
    incrementEip(op->len);
}
void Jit::dynamic_callJw(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b16, eip, op->len);
    push16(eip);    
    incrementEip(op->len+(S32)((S16)op->imm));
    blockCall(op);
}
void Jit::dynamic_callJd(DecodedOp* op) {
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);
    push32(eip);
    incrementEip(op->len + (S32)op->imm);
    blockCall(op);
}
void Jit::dynamic_jmp8(DecodedOp* op) {
    incrementEip(op->len+(S32)((S8)op->imm));
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
    } else {
        blockNext1(op);
    }
}
void Jit::dynamic_jmp16(DecodedOp* op) {
    incrementEip(op->len+(S32)((S16)op->imm));
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)((S16)op->imm));
    } else {
        blockNext1(op);
    }
}
void Jit::dynamic_jmp32(DecodedOp* op) {
    incrementEip(op->len+(S32)op->imm);
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)op->imm);
    } else {
        blockNext1(op);
    }
}
void Jit::dynamic_callR16(DecodedOp* op) {
    {
        RegPtr eip = getEip();
        addValue(JitWidth::b16, eip, op->len);
        
        if (op->reg == 4) {            
            RegPtr reg = getTmpReg(op->reg);
            push16(eip);
            mov(JitWidth::b16, eip, reg);
        } else {
            push16(eip);
            mov(JitWidth::b16, eip, getReadOnlyReg(op->reg));
        }
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void Jit::dynamic_callR32(DecodedOp* op) {
    {
        RegPtr eip = getEip();
        addValue(JitWidth::b32, eip, op->len);

        if (op->reg == 4) {
            RegPtr reg = getTmpReg(op->reg);
            push32(eip);
            mov(JitWidth::b32, eip, reg);
        } else {
            push32(eip);
            mov(JitWidth::b32, eip, getReadOnlyReg(op->reg));
        }
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void Jit::dynamic_callE16(DecodedOp* op) {
    {
        RegPtr newEip = read(JitWidth::b16, calculateEaa(op)); // read before push, incase esp is used
        RegPtr eip = getEip();
        addValue(JitWidth::b16, eip, op->len);
        push16(eip);
        mov(JitWidth::b16, eip, newEip);
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void Jit::dynamic_callE32(DecodedOp* op) {
    {
        RegPtr newEip = read(JitWidth::b32, calculateEaa(op)); // read before push, incase esp is used
        RegPtr eip = getEip();
        addValue(JitWidth::b32, eip, op->len);
        push32(eip);
        mov(JitWidth::b32, eip, newEip);
        // will write back eip before calling blockDoneCall
    }
    blockDoneCall();
}
void Jit::dynamic_jmpR16(DecodedOp* op) {
    mov(JitWidth::b16, getEip(), getReadOnlyReg(op->reg));
    blockDoneJump();
}
void Jit::dynamic_jmpR32(DecodedOp* op) {
    mov(JitWidth::b32, getEip(), getReadOnlyReg(op->reg));
    blockDoneJump();
}
void Jit::dynamic_jmpE16(DecodedOp* op) {
    mov(JitWidth::b16, getEip(), read(JitWidth::b16, calculateEaa(op)));
    blockDoneJump();
}
void Jit::dynamic_jmpE32(DecodedOp* op) {
    mov(JitWidth::b32, getEip(), read(JitWidth::b32, calculateEaa(op)));
    blockDoneJump();
}
void Jit::dynamic_callFarE16(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    RegPtr offset = read(JitWidth::b16, address, nullptr, nullptr, false, getTmpReg());
    addValue(JitWidth::b32, address, 2);
    RegPtr sel = read(JitWidth::b16, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b16, eip, op->len);

    call_IRRR(common_call, 0, JitWidth::b16, sel, JitWidth::b16, offset, JitWidth::b16, eip);
    blockDone(false);
}
void Jit::dynamic_callFarE32(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    RegPtr offset = read(JitWidth::b32, address, nullptr, nullptr, false, getTmpReg());
    addValue(JitWidth::b32, address, 4);
    RegPtr sel = read(JitWidth::b16, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);

    call_IRRR(common_call, 1, JitWidth::b16, sel, JitWidth::b32, offset, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_jmpFarE16(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    RegPtr offset = read(JitWidth::b16, address, nullptr, nullptr, false, getTmpReg());
    addValue(JitWidth::b32, address, 2);
    RegPtr sel = read(JitWidth::b16, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b16, eip, op->len);

    call_IRRR(common_jmp, 0, JitWidth::b16, sel, JitWidth::b16, offset, JitWidth::b16, eip);
    blockDone(false);
}
void Jit::dynamic_jmpFarE32(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    RegPtr offset = read(JitWidth::b32, address, nullptr, nullptr, false, getTmpReg());
    addValue(JitWidth::b32, address, 4);
    RegPtr sel = read(JitWidth::b16, address);
    address = nullptr;
    RegPtr eip = getTmpEip();
    addValue(JitWidth::b32, eip, op->len);

    call_IRRR(common_jmp, 1, JitWidth::b16, sel, JitWidth::b32, offset, JitWidth::b32, eip);
    blockDone(false);
}
void Jit::dynamic_larr16r16(DecodedOp* op) {
    call_II(common_larr16r16, op->reg, op->rm);
    incrementEip(op->len);
}
void Jit::dynamic_larr16e16(DecodedOp* op) {
    call_IR(common_larr16e16, op->reg, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_lslr16r16(DecodedOp* op) {
    call_II(common_lslr16r16, op->reg, op->rm);
    incrementEip(op->len);
}
void Jit::dynamic_lslr16e16(DecodedOp* op) {
    call_IR(common_lslr16e16, op->reg, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_lslr32r32(DecodedOp* op) {
    call_II(common_lslr32r32, op->reg, op->rm);
    incrementEip(op->len);
}
void Jit::dynamic_lslr32e32(DecodedOp* op) {
    call_IR(common_lslr32e32, op->reg, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_verre16(DecodedOp* op) {
    call_R(common_verre16, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_verwe16(DecodedOp* op) {
    call_R(common_verwe16, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_xaddr8r8(DecodedOp* op) {
    dynamic_RR_WriteBoth(op, JitWidth::b8, & Jit::xaddReg, FLAGS_ADD8);
}
void Jit::dynamic_xaddr8e8(DecodedOp* op) {
    dynamic_RM_WriteM(op, JitWidth::b8, &Jit::xaddReg, FLAGS_ADD8);
}
void Jit::dynamic_xaddr16r16(DecodedOp* op) {
    dynamic_RR_WriteBoth(op, JitWidth::b16, &Jit::xaddReg, FLAGS_ADD16);
}
void Jit::dynamic_xaddr16e16(DecodedOp* op) {
    dynamic_RM_WriteM(op, JitWidth::b16, &Jit::xaddReg, FLAGS_ADD16);
}
void Jit::dynamic_xaddr32r32(DecodedOp* op) {
    dynamic_RR_WriteBoth(op, JitWidth::b32, &Jit::xaddReg, FLAGS_ADD32);
}
void Jit::dynamic_xaddr32e32(DecodedOp* op) {
    dynamic_RM_WriteM(op, JitWidth::b32, &Jit::xaddReg, FLAGS_ADD32);
}
void Jit::dynamic_bswap32(DecodedOp* op) {
    byteSwapReg32(getReg(op->reg));
    incrementEip(op->len);
}
void Jit::dynamic_cmpxchgg8b(DecodedOp* op) {
    call_R(common_cmpxchg8b, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}
void Jit::dynamic_loadSegment16(DecodedOp* op) {
    //U16 val = cpu->memory->readw(eaa);
    //U32 selector = cpu->memory->readw(eaa + 2);
    //if (cpu->setSegment(op->imm, selector)) {
    //    cpu->reg[op->reg].u16 = val;

    RegPtr address = calculateEaa(op);
    RegPtr value = read(JitWidth::b16, address, nullptr, nullptr, false, getTmpReg());
    addValue(JitWidth::b32, address, 2);
    RegPtr sel = read(JitWidth::b16, std::move(address), nullptr, nullptr, false, getTmpReg()); // std::move will release address in this function and give it to read, this way read will see there is only 1 reference to address and can re-use it as a tmp register

    RegPtr result = callAndReturn_IR(common_setSegment, op->imm, JitWidth::b16, sel);

    IfNot(JitWidth::b32, result);
        blockDone(true);
    EndIf();
    mov(JitWidth::b16, getReg(op->reg), value);
    incrementEip(op->len);
}
void Jit::dynamic_loadSegment32(DecodedOp* op) {
    RegPtr address = calculateEaa(op);
    RegPtr value = read(JitWidth::b32, address, nullptr, nullptr, false, getTmpReg());
    addValue(JitWidth::b32, address, 4);
    RegPtr sel = read(JitWidth::b16, std::move(address), nullptr, nullptr, false, getTmpReg()); // std::move will release address in this function and give it to read, this way read will see there is only 1 reference to address and can re-use it as a tmp register

    RegPtr result = callAndReturn_IR(common_setSegment, op->imm, JitWidth::b16, sel);

    IfNot(JitWidth::b32, result);
        blockDone(true);
    EndIf();
    mov(JitWidth::b32, getReg(op->reg, -1, false), value);
    incrementEip(op->len);
}

void Jit::dynamic_fxsave(DecodedOp* op) {
    call_R(common_fxsave, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}

void Jit::dynamic_fxrstor(DecodedOp* op) {
    call_R(common_fxrstor, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}

void Jit::dynamic_xsave(DecodedOp* op) {
    call_R(common_xsave, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}

void Jit::dynamic_xrstor(DecodedOp* op) {
    call_R(common_xrstor, JitWidth::b32, calculateEaa(op));
    incrementEip(op->len);
}