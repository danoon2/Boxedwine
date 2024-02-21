#include "../common/common_other.h"
/*
 *  Copyright (C) 2016  The BoxedWine Team
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

void OPCALL normal_bound16(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    if (common_bound16(cpu, op->reg, eaa(cpu, op))) {
        NEXT();
    } else { 
        NEXT_DONE();
    }
}
void OPCALL normal_bound32(CPU* cpu, DecodedOp* op){
    START_OP(cpu, op);
    if (common_bound32(cpu, op->reg, eaa(cpu, op))) {
        NEXT();
    } else { 
        NEXT_DONE();
    }
}
void OPCALL normal_daa(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    daa(cpu);
    NEXT();
}
void OPCALL normal_das(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    das(cpu);
    NEXT();
}
void OPCALL normal_aaa(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    aaa(cpu);
    NEXT();
}
void OPCALL normal_aas(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    aas(cpu);
    NEXT();
}
void OPCALL normal_aad(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    aad(cpu, op->imm);
    NEXT();
}
void OPCALL normal_aam(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (aam(cpu, op->imm)) {
        NEXT();
    } else {
        NEXT_DONE();
    }
}
void OPCALL normal_nop(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_done(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT_DONE();
}
void OPCALL normal_wait(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_cwd(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (((S16)AX) < 0)
        DX = 0xFFFF;
    else
        DX = 0;
    NEXT();
}
void OPCALL normal_cwq(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (((S32)EAX) < 0)
        EDX = 0xFFFFFFFF;
    else
        EDX = 0;
    NEXT();
}
void OPCALL normal_callAp(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->call(0, op->imm, op->disp, cpu->eip.u32+op->len);
    NEXT_DONE();
}
void OPCALL normal_callFar(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->call(1, op->imm, op->disp, cpu->eip.u32+op->len);
    NEXT_DONE();
}
void OPCALL normal_jmpAp(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->jmp(0, op->imm, op->disp, cpu->eip.u32+op->len);
    NEXT_DONE();
}
void OPCALL normal_jmpFar(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->jmp(1, op->imm, op->disp, cpu->eip.u32+op->len);
    NEXT_DONE();
}
void OPCALL normal_retf16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->ret(0, op->imm);
    NEXT_DONE();
}
void OPCALL normal_retf32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->ret(1, op->imm);
    NEXT_DONE();
}
void OPCALL normal_iret(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->iret(0, cpu->eip.u32+op->len);
    NEXT_DONE();
}
void OPCALL normal_iret32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->iret(1, cpu->eip.u32+op->len);
    NEXT_DONE();
}
void OPCALL normal_sahf(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->fillFlags(); cpu->setFlags(AH, FMASK_ALL & 0xFF);
    NEXT();
}
void OPCALL normal_lahf(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->fillFlags(); AH = (cpu->flags & (SF|ZF|AF|PF|CF)) | 2;
    NEXT();
}
void OPCALL normal_salc(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) AL = 0xFF; else AL = 0;
    NEXT();
}
void OPCALL normal_retn16Iw(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 eip = cpu->pop16();
    SP = SP+op->imm;
    cpu->eip.u32 = eip;
    NEXT_DONE();
}
void OPCALL normal_retn32Iw(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eip = cpu->pop32();
    ESP = ESP+op->imm;
    cpu->eip.u32 = eip;
    NEXT_DONE();
}
void OPCALL normal_retn16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 = cpu->pop16();
    NEXT_DONE();
}
void OPCALL normal_retn32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 = cpu->pop32();
    NEXT_DONE();
}
void OPCALL normal_invalid(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->thread->signalIllegalInstruction(5);
    NEXT_DONE();
}
void OPCALL normal_int80(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ksyscall(cpu, op->len);
    if (cpu->thread->terminating) {
        return;
    }
    NEXT_DONE();
}
void OPCALL normal_int98(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 index = cpu->peek32(0);
    if (index<wine_callbackSize && wine_callback[index]) {
        wine_callback[index](cpu);
    } else if (index >= wine_audio_callback_base && index < wine_audio_callback_base + wine_audio_callback_size) {
        wine_audio_callback[index - wine_audio_callback_base](cpu);
    } else {
        kpanic("Uknown int 98 call: %d", index);
    }
    NEXT();
}
void OPCALL normal_int99(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 index = cpu->peek32(0);
    callOpenGL(cpu, index);
    NEXT();
}
void OPCALL normal_int9A(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
#ifdef BOXEDWINE_VULKAN
    U32 index = cpu->peek32(0);
    callVulkan(cpu, index);
#endif
    NEXT();
}
void OPCALL normal_int3(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->thread->signalTrap(1);// 1=TRAP_BRKPT
    NEXT_DONE();
}
void OPCALL normal_intIb(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess
    NEXT_DONE();
}
void OPCALL normal_xlat(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16) {
        AL = cpu->memory->readb(cpu->seg[op->base].address + (U16)(BX + AL));
    } else {
        AL = cpu->memory->readb(cpu->seg[op->base].address + EBX + AL);
    }
    NEXT();
}
void OPCALL normal_hlt(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    kpanic("Hlt");
}
void OPCALL normal_cmc(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->fillFlags();
    cpu->setCF(!cpu->getCF());
    NEXT();
}
void OPCALL normal_clc(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->fillFlags();
    cpu->removeCF();
    NEXT();
}
void OPCALL normal_stc(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->fillFlags();
    cpu->addCF();
    NEXT();
}
void OPCALL normal_cli(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->removeFlag(IF);
    NEXT();
}
void OPCALL normal_sti(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->addFlag(IF);
    NEXT();
}
void OPCALL normal_cld(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->removeFlag(DF);
    NEXT();
}
void OPCALL normal_std(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->addFlag(DF);
    NEXT();
}
void OPCALL normal_rdtsc(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U64 t = cpu->instructionCount+cpu->blockInstructionCount+op->imm;
    EAX = (U32)t;
    EDX = (U32)(t >> 32);
    NEXT();
}
void OPCALL normal_cpuid(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->cpuid();
    NEXT();
}
void OPCALL normal_enter16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->enter(0, op->imm, op->disp);
    NEXT();
}
void OPCALL normal_enter32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->enter(1, op->imm, op->disp);
    NEXT();
}
void OPCALL normal_leave16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    SP = BP;
    BP = cpu->pop16();
    NEXT();
}
void OPCALL normal_leave32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    ESP = EBP;
    EBP = cpu->pop32();
    NEXT();
}
void OPCALL normal_loopnz(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16){
        CX--;
        if (CX!=0 && !cpu->getZF()) {
            cpu->eip.u32+=op->imm;
            NEXT_BRANCH1();
        } else {
            NEXT_BRANCH2();
        }
    } else {
        ECX--;
        if (ECX!=0 && !cpu->getZF()) {
            cpu->eip.u32+=op->imm;
            NEXT_BRANCH1();
        } else {
            NEXT_BRANCH2();
        }
    }
}
void OPCALL normal_loopz(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16){
        CX--;
        if (CX!=0 && cpu->getZF()) {
            cpu->eip.u32+=op->imm;
            NEXT_BRANCH1();
        } else {
            NEXT_BRANCH2();
        }
    } else {
        ECX--;
        if (ECX!=0 && cpu->getZF()) {
            cpu->eip.u32+=op->imm;
            NEXT_BRANCH1();
        } else {
            NEXT_BRANCH2();
        }
    }
}
void OPCALL normal_loop(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (op->ea16){
        CX--;
        if (CX!=0) {
            cpu->eip.u32+=op->imm;
            NEXT_BRANCH1();
        } else {
            NEXT_BRANCH2();
        }
    } else {
        ECX--;
        if (ECX!=0) {
            cpu->eip.u32+=op->imm;
            NEXT_BRANCH1();
        } else {
            NEXT_BRANCH2();
        }
    }
}
void OPCALL normal_jcxz(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if ((op->ea16?CX:ECX)==0) {
        cpu->eip.u32+=op->imm;
        NEXT_BRANCH1();
    } else {
        NEXT_BRANCH2();
    }
}
void OPCALL normal_InAlIb(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    AL=0xFF;
    NEXT();
}
void OPCALL normal_InAxIb(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    AX=0xFFFF;
    NEXT();
}
void OPCALL normal_InEaxIb(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    EAX=0xFFFFFFFF;
    NEXT();
}
void OPCALL normal_OutIbAl(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_OutIbAx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_OutIbEax(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_InAlDx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    AL=0xFF;
    NEXT();
}
void OPCALL normal_InAxDx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    AX=0xFFFF;
    NEXT();
}
void OPCALL normal_InEaxDx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    EAX=0xFFFFFFFF;
    NEXT();
}
void OPCALL normal_OutDxAl(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_OutDxAx(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_OutDxEax(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    NEXT();
}
void OPCALL normal_callJw(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->push16(cpu->eip.u32 + op->len);
    cpu->eip.u32 += (S16)op->imm;
    NEXT_BRANCH1();
}
void OPCALL normal_callJd(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->push32(cpu->eip.u32 + op->len);
    cpu->eip.u32 += (S32)op->imm;
    NEXT_BRANCH1();
}
void OPCALL normal_jmp8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 += (S8)op->imm;
    NEXT_BRANCH1();
}
void OPCALL normal_jmp16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 += (S16)op->imm;
    NEXT_BRANCH1();
}
void OPCALL normal_jmp32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 += (S32)op->imm;
    NEXT_BRANCH1();
}
void OPCALL normal_callR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U16 dest = cpu->reg[op->reg].u16;
    cpu->push16(cpu->eip.u32+op->len);
    cpu->eip.u32 = dest;
    NEXT_DONE();
}
void OPCALL normal_callR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 dest = cpu->reg[op->reg].u32;
    cpu->push32(cpu->eip.u32+op->len);
    cpu->eip.u32 = dest;
    NEXT_DONE();
}
void OPCALL normal_callE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 neweip = cpu->memory->readw(eaa(cpu, op));
    cpu->push16(cpu->eip.u32+op->len);
    cpu->eip.u32 = neweip;
    NEXT_DONE();
}
void OPCALL normal_callE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 neweip = cpu->memory->readd(eaa(cpu, op));
    cpu->push32(cpu->eip.u32+op->len);
    cpu->eip.u32 = neweip;
    NEXT_DONE();
}
void OPCALL normal_jmpR16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 = cpu->reg[op->reg].u16;
    NEXT_DONE();
}
void OPCALL normal_jmpR32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->eip.u32 = cpu->reg[op->reg].u32;
    NEXT_DONE();
}
void OPCALL normal_jmpE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 neweip = cpu->memory->readw(eaa(cpu, op));
    cpu->eip.u32 = neweip;
    NEXT_DONE();
}
void OPCALL normal_jmpE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 neweip = cpu->memory->readd(eaa(cpu, op));
    cpu->eip.u32 = neweip;
    NEXT_DONE();
}
void OPCALL normal_callFarE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    U16 newip = cpu->memory->readw(eaa);
    U16 newcs = cpu->memory->readw(eaa+2);
    cpu->call(0, newcs, newip, cpu->eip.u32 + op->len);
    NEXT_DONE();
}
void OPCALL normal_callFarE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    U32 newip = cpu->memory->readd(eaa);
    U16 newcs = cpu->memory->readw(eaa+4);
    cpu->call(1, newcs, newip, cpu->eip.u32 + op->len);
    NEXT_DONE();
}
void OPCALL normal_jmpFarE16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    U16 newip = cpu->memory->readw(eaa);
    U16 newcs = cpu->memory->readw(eaa+2);
    cpu->jmp(0, newcs, newip, cpu->eip.u32 + op->len);
    NEXT_DONE();
}
void OPCALL normal_jmpFarE32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    U32 newip = cpu->memory->readd(eaa);
    U16 newcs = cpu->memory->readw(eaa+4);
    cpu->jmp(1, newcs, newip, cpu->eip.u32 + op->len);
    NEXT_DONE();
}
void OPCALL normal_larr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->lar(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_larr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->lar(cpu->memory->readw(eaa(cpu, op)), cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_lslr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->lsl(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_lslr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u16 = cpu->lsl(cpu->memory->readw(eaa(cpu, op)), cpu->reg[op->reg].u16);
    NEXT();
}
void OPCALL normal_lslr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->lsl(cpu->reg[op->rm].u32, cpu->reg[op->reg].u32);
    NEXT();
}
void OPCALL normal_lslr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->reg[op->reg].u32 = cpu->lsl(cpu->memory->readw(eaa(cpu, op)), cpu->reg[op->reg].u32); // intentional 16-bit read
    NEXT();
}
void OPCALL normal_verre16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->verr(cpu->memory->readw(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_verwe16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->verw(cpu->memory->readw(eaa(cpu, op)));
    NEXT();
}
void OPCALL normal_xaddr8r8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->dst.u8 = *cpu->reg8[op->rm];
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] = cpu->dst.u8;
    *cpu->reg8[op->rm] = cpu->result.u8;
    NEXT();
}
void OPCALL normal_xaddr8e8(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    cpu->src.u8 = *cpu->reg8[op->reg];
    cpu->dst.u8 = cpu->memory->readb(address);
    cpu->result.u8 = cpu->dst.u8 + cpu->src.u8;
    cpu->lazyFlags = FLAGS_ADD8;
    *cpu->reg8[op->reg] = cpu->dst.u8;
    cpu->memory->writeb(address, cpu->result.u8);
    NEXT();
}
void OPCALL normal_xaddr16r16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->dst.u16 = cpu->reg[op->rm].u16;
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 = cpu->dst.u16;
    cpu->reg[op->rm].u16 = cpu->result.u16;
    NEXT();
}
void OPCALL normal_xaddr16e16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    cpu->src.u16 = cpu->reg[op->reg].u16;
    cpu->dst.u16 = cpu->memory->readw(address);
    cpu->result.u16 = cpu->dst.u16 + cpu->src.u16;
    cpu->lazyFlags = FLAGS_ADD16;
    cpu->reg[op->reg].u16 = cpu->dst.u16;
    cpu->memory->writew(address, cpu->result.u16);
    NEXT();
}
void OPCALL normal_xaddr32r32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->dst.u32 = cpu->reg[op->rm].u32;
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 = cpu->dst.u32;
    cpu->reg[op->rm].u32 =  cpu->result.u32;
    NEXT();
}
void OPCALL normal_xaddr32e32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 address = eaa(cpu, op);
    cpu->src.u32 = cpu->reg[op->reg].u32;
    cpu->dst.u32 = cpu->memory->readd(address);
    cpu->result.u32 = cpu->dst.u32 + cpu->src.u32;
    cpu->lazyFlags = FLAGS_ADD32;
    cpu->reg[op->reg].u32 = cpu->dst.u32;
    cpu->memory->writed(address, cpu->result.u32);
    NEXT();
}
void OPCALL normal_bswap32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 val = cpu->reg[op->reg].u32;
    cpu->reg[op->reg].u32 = (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) | ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24));
NEXT();
}
void OPCALL normal_cmpxchgg8b(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    common_cmpxchgg8b(cpu, eaa(cpu, op));
    NEXT();
}
void OPCALL normal_loadSegment16(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    U16 val = cpu->memory->readw(eaa);
    U32 selector = cpu->memory->readw(eaa+2);
    if (cpu->setSegment(op->imm, selector)) {
        cpu->reg[op->reg].u16 = val;
        NEXT();
    } else {
        NEXT_DONE();
    }
}
void OPCALL normal_loadSegment32(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    U32 val = cpu->memory->readd(eaa);
    U32 selector = cpu->memory->readw(eaa+4);
    if (cpu->setSegment(op->imm, selector)) {
        cpu->reg[op->reg].u32 = val;
        NEXT();
    } else {
        NEXT_DONE();
    }
}

void OPCALL normal_fxsave(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    common_fxsave(cpu, eaa);
    NEXT();
}

void OPCALL normal_fxrstor(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    common_fxrstor(cpu, eaa);
    NEXT();
}

void OPCALL normal_xsave(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    common_xsave(cpu, eaa);
    NEXT();
}

void OPCALL normal_xrstor(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    U32 eaa = eaa(cpu, op);
    common_xrstor(cpu, eaa);
    NEXT();
}
