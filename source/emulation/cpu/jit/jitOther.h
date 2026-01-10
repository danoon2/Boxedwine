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
    emulateSingleOp();
}
void Jit::dynamic_bound32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_daa(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_das(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_aaa(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_aas(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_aad(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_aam(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_nop(DecodedOp* op) {
    // Nop
}
void Jit::dynamic_done(DecodedOp* op) {
    blockDone(false);
}
void Jit::dynamic_wait(DecodedOp* op) {
    // Wait
}
void Jit::dynamic_cwd(DecodedOp* op) {
    RegPtr dx = getReg(2);
    mov(JitWidth::b16, dx, getReg(0));
    sarValue(JitWidth::b16, dx, 15);
}
void Jit::dynamic_cwq(DecodedOp* op) {
    RegPtr edx = getReg(2);
    mov(JitWidth::b32, edx, getReg(0));
    sarValue(JitWidth::b32, edx, 31);
}
void Jit::dynamic_callAp(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_callFar(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_jmpAp(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_jmpFar(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_retf16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_retf32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_iret(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_iret32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_sahf(DecodedOp* op) {
    RegPtr flags = getTmpReg();
    movzx(JitWidth::b32, flags, JitWidth::b8, getReadOnlyReg8(4));
    setFlags(flags, FMASK_ALL & 0xFF);
    currentLazyFlags = FLAGS_NONE;
}
void Jit::dynamic_lahf(DecodedOp* op) {
    RegPtr flags = getReadOnlyFlags();

    andValue(JitWidth::b32, flags, SF | ZF | AF | PF | CF);
    orValue(JitWidth::b32, flags, 2);
    mov(JitWidth::b8, getReg8(4), flags);
}
void Jit::dynamic_salc(DecodedOp* op) {
    RegPtr cf = getCF();
    negReg2(JitWidth::b8, cf);
    mov(JitWidth::b8, getReg8(0), cf);
}
void Jit::dynamic_retn16Iw(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    pop16(eip, op->imm + 2);
    writeEip(eip);
    blockDone(false);
}
void Jit::dynamic_retn32Iw(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    pop32(eip, op->imm + 4);
    writeEip(eip);
    blockDone(false);
}
void Jit::dynamic_retn16(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    pop16(eip);
    writeEip(eip);
    blockDone(false);
}
void Jit::dynamic_retn32(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    pop32(eip);
    writeEip(eip);
    blockDone(false);
}
void Jit::dynamic_invalid(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_ud2(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_int80(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_int99(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_int9A(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_int9B(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_intIb(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_int3(DecodedOp* op) {
    emulateSingleOp();
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
}
void Jit::dynamic_hlt(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_cmc(DecodedOp* op) {
    fillFlags();
    xorCPUFlagsImmV2(CF);
}
void Jit::dynamic_clc(DecodedOp* op) {
    fillFlags();
    andCPUFlagsImmV2(~CF);
}
void Jit::dynamic_stc(DecodedOp* op) {
    fillFlags();
    orCPUFlagsImmV2(CF);
}
void Jit::dynamic_cli(DecodedOp* op) {
    andCPUFlagsImmV2(~IF);
}
void Jit::dynamic_sti(DecodedOp* op) {
    orCPUFlagsImmV2(IF);
}
void Jit::dynamic_cld(DecodedOp* op) {
    andCPUFlagsImmV2(~DF);
}
void Jit::dynamic_std(DecodedOp* op) {
    orCPUFlagsImmV2(DF);
}
void Jit::dynamic_rdtsc(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_cpuid(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_enter16(DecodedOp* op) {
    if (op->data.disp) {
        emulateSingleOp();
    } else {
        RegPtr ebp = getReg(5);        
        // push EBP
        push16(ebp);
        RegPtr esp = getReg(4); // if not cached, we don't want to read until after push
        //EBP = ESP;
        mov(JitWidth::b16, ebp, esp);
        // sub esp, bytes
        IfSmallStack(); {
            subValue(JitWidth::b16, esp, op->imm);
        } StartElse(); {
            subValue(JitWidth::b32, esp, op->imm);
        } EndIf();
    }
}
void Jit::dynamic_enter32(DecodedOp* op) {
    if (op->data.disp) {
        emulateSingleOp();
    } else {
        RegPtr ebp = getReg(5);
        // push EBP
        push32(ebp);
        RegPtr esp = getReg(4); // if not cached, we don't want to read until after push
        //EBP = ESP;
        mov(JitWidth::b32, ebp, esp);
        // sub esp, bytes
        IfSmallStack(); {
            subValue(JitWidth::b16, esp, op->imm);
        } StartElse(); {
            subValue(JitWidth::b32, esp, op->imm);
        } EndIf();
    }
}
void Jit::dynamic_leave16(DecodedOp* op) {
    // ESP and EBP each get loaded more than once, on a platform that doesnt' cache them, like x86, it might be interesting to reduce this
    mov(JitWidth::b16, getReg(4), getReadOnlyReg(5));
    mov(JitWidth::b16, getReg(5), pop16());
}
void Jit::dynamic_leave32(DecodedOp* op) {
    mov(JitWidth::b32, getReg(4, -1, false), getReadOnlyReg(5));
    mov(JitWidth::b32, getReg(5, -1, false), pop32());
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
        if (canJumpInBlock(op)) {                
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            writeCurrentEip(op->len + (S32)((S8)op->imm));
            blockNext1(op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        writeCurrentEip(op->len);
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
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            writeCurrentEip(op->len + (S32)((S8)op->imm));
            blockNext1(op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        writeCurrentEip(op->len);
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
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
        } else {
            writeCurrentEip(op->len + (S32)((S8)op->imm));
            blockNext1(op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        writeCurrentEip(op->len);
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
        if (canJumpInBlock(op)) {
            JumpInBlock(currentEip + op->len + op->imm);
        } else {
            writeCurrentEip(op->len + (S32)((S8)op->imm));
            blockNext1(op);
        }
    EndIf();
    if (!canJumpInBlock(op)) {
        writeCurrentEip(op->len);
        blockNext2(op);
    }
}
void Jit::dynamic_InAlIb(DecodedOp* op) {
    movValue(JitWidth::b8, getReg8(0), 0xff);
}
void Jit::dynamic_InAxIb(DecodedOp* op) {
    movValue(JitWidth::b16, getReg(0), 0xffff);
}
void Jit::dynamic_InEaxIb(DecodedOp* op) {
    movValue(JitWidth::b32, getReg(0), 0xffffffff);
}
void Jit::dynamic_OutIbAl(DecodedOp* op) {
    // do nothing
}
void Jit::dynamic_OutIbAx(DecodedOp* op) {
    // do nothing
}
void Jit::dynamic_OutIbEax(DecodedOp* op) {
    // do nothing
}
void Jit::dynamic_InAlDx(DecodedOp* op) {
    movValue(JitWidth::b8, getReg8(0), 0xff);
}
void Jit::dynamic_InAxDx(DecodedOp* op) {
    movValue(JitWidth::b16, getReg(0), 0xffff);
}
void Jit::dynamic_InEaxDx(DecodedOp* op) {
    movValue(JitWidth::b32, getReg(0), 0xffffffff);
}
void Jit::dynamic_OutDxAl(DecodedOp* op) {
    // do nothing
}
void Jit::dynamic_OutDxAx(DecodedOp* op) {
    // do nothing
}
void Jit::dynamic_OutDxEax(DecodedOp* op) {
    // do nothing
}
void Jit::dynamic_callJw(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    movValue(JitWidth::b32, eip, this->currentEip - cpu->seg[CS].address + op->len);
    push16(eip); 
    writeCurrentEip(op->len + (S32)((S16)op->imm));
    blockCall(op);
}
void Jit::dynamic_callJd(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    movValue(JitWidth::b32, eip, this->currentEip - cpu->seg[CS].address + op->len);
    push32(eip);
    writeCurrentEip(op->len + (S32)op->imm);
    blockCall(op);
}
void Jit::dynamic_jmp8(DecodedOp* op) {
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)((S8)op->imm));
    } else {
        writeCurrentEip(op->len + (S32)((S8)op->imm));
        blockNext1(op);
    }
}
void Jit::dynamic_jmp16(DecodedOp* op) {
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)((S16)op->imm));
    } else {
        writeCurrentEip(op->len + (S32)((S16)op->imm));
        blockNext1(op);
    }
}
void Jit::dynamic_jmp32(DecodedOp* op) {
    if (canJumpInBlock(op)) {
        JumpInBlock(currentEip + op->len + (S32)op->imm);
    } else {
        writeCurrentEip(op->len + (S32)op->imm);
        blockNext1(op);
    }
}
void Jit::dynamic_callR16(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    movValue(JitWidth::b32, eip, this->currentEip - cpu->seg[CS].address + op->len);
        
    if (op->reg == 4) {            
        RegPtr reg = getTmpReg(op->reg);
        push16(eip);
        mov(JitWidth::b16, eip, reg);
    } else {
        push16(eip);
        mov(JitWidth::b16, eip, getReadOnlyReg(op->reg));
    }
    movzx(JitWidth::b32, eip, JitWidth::b16, eip);
    writeEip(eip);
    blockDoneCall();
}
void Jit::dynamic_callR32(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    movValue(JitWidth::b32, eip, this->currentEip - cpu->seg[CS].address + op->len);

    if (op->reg == 4) {
        RegPtr reg = getTmpReg(op->reg);
        push32(eip);
        mov(JitWidth::b32, eip, reg);
    } else {
        push32(eip);
        mov(JitWidth::b32, eip, getReadOnlyReg(op->reg));
    }
    writeEip(eip);
    blockDoneCall();
}
void Jit::dynamic_callE16(DecodedOp* op) {
    {
        RegPtr newEip = read(JitWidth::b16, calculateEaa(op)); // read before push, in case esp is used
        RegPtr eip = getTmpReg();

        movValue(JitWidth::b32, eip, this->currentEip + op->len - this->cpu->seg[CS].address);
        push16(eip);
        mov(JitWidth::b16, eip, newEip);
        writeEip(newEip);
    }
    blockDoneCall();
}
void Jit::dynamic_callE32(DecodedOp* op) {
    RegPtr newEip = read(JitWidth::b32, calculateEaa(op)); // read before push, in case esp is used
    RegPtr eip = getTmpReg();

    movValue(JitWidth::b32, eip, this->currentEip + op->len - this->cpu->seg[CS].address);
    push32(eip);
    writeEip(newEip);
    blockDoneCall();
}
void Jit::dynamic_jmpR16(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    mov(JitWidth::b16, eip, getReadOnlyReg(op->reg));
    movzx(JitWidth::b32, eip, JitWidth::b16, eip);
    writeEip(eip);
    blockDoneJump();
}
void Jit::dynamic_jmpR32(DecodedOp* op) {
    writeEip(getReadOnlyReg(op->reg));
    blockDoneJump();
}
void Jit::dynamic_jmpE16(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    mov(JitWidth::b16, eip, read(JitWidth::b16, calculateEaa(op)));
    movzx(JitWidth::b32, eip, JitWidth::b16, eip);
    writeEip(eip);
    blockDoneJump();
}
void Jit::dynamic_jmpE32(DecodedOp* op) {
    RegPtr eip = getTmpReg();
    mov(JitWidth::b32, eip, read(JitWidth::b32, calculateEaa(op)));
    writeEip(eip);
    blockDoneJump();
}
void Jit::dynamic_callFarE16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_callFarE32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_jmpFarE16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_jmpFarE32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_larr16r16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_larr16e16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_lslr16r16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_lslr16e16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_lslr32r32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_lslr32e32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_verre16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_verwe16(DecodedOp* op) {
    emulateSingleOp();
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
}
void Jit::dynamic_cmpxchgg8b(DecodedOp* op) {
    // I haven't seen anything call this so no reason to inline, I imagine only the lock version of this is ever used
    emulateSingleOp();
}
void Jit::dynamic_loadSegment16(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_loadSegment32(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_fxsave(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_fxrstor(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_xsave(DecodedOp* op) {
    emulateSingleOp();
}
void Jit::dynamic_xrstor(DecodedOp* op) {
    emulateSingleOp();
}