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

void OPCALL bound16(CPU* cpu, DecodedOp* op){
    U32 eaa = eaa(cpu, op);
    if (cpu->reg[op->reg].u16<readw(eaa) || cpu->reg[op->reg].u16>readw(eaa+2)) {cpu->prepareException(EXCEPTION_BOUND, 0);} else { cpu->eip.u32+=op->len;}
}
void OPCALL bound32(CPU* cpu, DecodedOp* op){
    U32 eaa = eaa(cpu, op);
    if (cpu->reg[op->reg].u32<readd(eaa) || cpu->reg[op->reg].u32>readd(eaa+4)) {cpu->prepareException(EXCEPTION_BOUND, 0);} else { cpu->eip.u32+=op->len;}
}
void OPCALL normal_daa(CPU* cpu, DecodedOp* op) {
    daa(cpu);
}
void OPCALL normal_das(CPU* cpu, DecodedOp* op) {
    das(cpu);
}
void OPCALL normal_aaa(CPU* cpu, DecodedOp* op) {
    aaa(cpu);
}
void OPCALL normal_aas(CPU* cpu, DecodedOp* op) {
    aas(cpu);
}
void OPCALL normal_aad(CPU* cpu, DecodedOp* op) {
    aad(cpu, op->imm);
}
void OPCALL normal_aam(CPU* cpu, DecodedOp* op) {
    if (aam(cpu, op->imm)) {cpu->eip.u32+=op->len;}
}
void OPCALL normal_nop(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_wait(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_cwd(CPU* cpu, DecodedOp* op) {
    if (((S16)AX) < 0) DX = 0xFFFF; else  DX = 0;
}
void OPCALL normal_cwq(CPU* cpu, DecodedOp* op) {
    if (((S32)EAX) < 0) EDX = 0xFFFFFFFF; else EDX = 0;
}
void OPCALL normal_callAp(CPU* cpu, DecodedOp* op) {
    cpu->call(0, op->disp, op->imm, cpu->eip.u32+op->len);
}
void OPCALL normal_callFar(CPU* cpu, DecodedOp* op) {
    cpu->call(1, op->disp, op->imm, cpu->eip.u32+op->len);
}
void OPCALL normal_jmpAp(CPU* cpu, DecodedOp* op) {
    cpu->jmp(0, op->disp, op->imm, cpu->eip.u32+op->len);
}
void OPCALL normal_jmpFar(CPU* cpu, DecodedOp* op) {
    cpu->jmp(1, op->disp, op->imm, cpu->eip.u32+op->len);
}
void OPCALL normal_retf16(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; cpu->ret(0, op->imm);
}
void OPCALL normal_retf32(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len; cpu->ret(1, op->imm);
}
void OPCALL normal_iret(CPU* cpu, DecodedOp* op) {
    cpu->iret(0, cpu->eip.u32+op->len);
}
void OPCALL normal_iret32(CPU* cpu, DecodedOp* op) {
    cpu->iret(1, cpu->eip.u32+op->len);
}
void OPCALL normal_sahf(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags(); cpu->setFlags(AH, FMASK_ALL & 0xFF);
}
void OPCALL normal_lahf(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags(); AH = (cpu->flags & (SF|ZF|AF|PF|CF)) | 2;
}
void OPCALL normal_salc(CPU* cpu, DecodedOp* op) {
    if (cpu->getCF()) AL = 0xFF; else AL = 0;
}
void OPCALL normal_retn16Iw(CPU* cpu, DecodedOp* op) {
    U16 eip = cpu->pop16();
    SP = SP+op->imm;
    cpu->eip.u32 = eip;
}
void OPCALL normal_retn32Iw(CPU* cpu, DecodedOp* op) {
    U32 eip = cpu->pop32();
    ESP = ESP+op->imm;
    cpu->eip.u32 = eip;
}
void OPCALL normal_retn16(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 = cpu->pop16();
}
void OPCALL normal_retn32(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 = cpu->pop32();
}
void OPCALL normal_invalid(CPU* cpu, DecodedOp* op) {
    kpanic("Invalid instruction %x\n", op->inst);
}
void OPCALL normal_int80(CPU* cpu, DecodedOp* op) {
    ksyscall(cpu, op->len);
}
void OPCALL normal_int98(CPU* cpu, DecodedOp* op) {
    U32 index = cpu->peek32(0); if (index<wine_callbackSize && wine_callback[index]) {wine_callback[index](cpu);} else {kpanic("Uknown int 98 call: %d", index);}
}
void OPCALL normal_int99(CPU* cpu, DecodedOp* op) {
    U32 index = cpu->peek32(0); if (index<int99CallbackSize && int99Callback[index]) {int99Callback[index](cpu);} else {kpanic("Uknown int 99 call: %d", index);}
}
void OPCALL normal_xlat(CPU* cpu, DecodedOp* op) {
    if (op->ea16) {AL = readb(cpu->seg[op->base].address + (U16)(BX + AL));} else {AL = readb(cpu->seg[op->base].address + EBX + AL);}
}
void OPCALL normal_hlt(CPU* cpu, DecodedOp* op) {
    kpanic("Hlt");
}
void OPCALL normal_cmc(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->setCF(!cpu->getCF());
}
void OPCALL normal_clc(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->removeCF();
}
void OPCALL normal_stc(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->addCF();
}
void OPCALL normal_cli(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->removeFlag(IF);
}
void OPCALL normal_sti(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->addFlag(IF);
}
void OPCALL normal_cld(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->removeFlag(DF);cpu->df=1;
}
void OPCALL normal_std(CPU* cpu, DecodedOp* op) {
    cpu->fillFlags();cpu->addFlag(DF);cpu->df=-1;
}
void OPCALL normal_rdtsc(CPU* cpu, DecodedOp* op) {
    U64 t = cpu->instructionCount+op->imm; EAX = (U32)t; EDX = (U32)(t >> 32);
}
void OPCALL normal_cpuid(CPU* cpu, DecodedOp* op) {
    cpu->cpuid();
}
void OPCALL normal_enter16(CPU* cpu, DecodedOp* op) {
    cpu->enter(0, op->imm, op->reg);
}
void OPCALL normal_enter32(CPU* cpu, DecodedOp* op) {
    cpu->enter(1, op->imm, op->reg);
}
void OPCALL normal_leave16(CPU* cpu, DecodedOp* op) {
    SP = BP; BP = cpu->pop16();
}
void OPCALL normal_leave32(CPU* cpu, DecodedOp* op) {
    ESP = EBP; EBP = cpu->pop32();
}
void OPCALL normal_loopnz(CPU* cpu, DecodedOp* op) {
    if (op->ea16){CX--;cpu->eip.u32+=op->len;if (CX!=0 && !cpu->getZF()) {cpu->eip.u32+=op->imm;}} else {ECX--;cpu->eip.u32+=op->len;if (ECX!=0 && !cpu->getZF()) {cpu->eip.u32+=op->imm;}}
}
void OPCALL normal_loopz(CPU* cpu, DecodedOp* op) {
    if (op->ea16){CX--;cpu->eip.u32+=op->len;if (CX!=0 && cpu->getZF()) {cpu->eip.u32+=op->imm;}} else {ECX--;cpu->eip.u32+=op->len;if (ECX!=0 && cpu->getZF()) {cpu->eip.u32+=op->imm;}}
}
void OPCALL normal_loop(CPU* cpu, DecodedOp* op) {
    if (op->ea16){CX--;cpu->eip.u32+=op->len;if (CX!=0) {cpu->eip.u32+=op->imm;}} else {ECX--;cpu->eip.u32+=op->len;if (ECX!=0) {cpu->eip.u32+=op->imm;}}
}
void OPCALL normal_jcxz(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32+=op->len;if ((op->ea16?CX:ECX)==0) {cpu->eip.u32+=op->imm;}
}
void OPCALL normal_InAlIb(CPU* cpu, DecodedOp* op) {
    AL=0xFF;
}
void OPCALL normal_InAxIb(CPU* cpu, DecodedOp* op) {
    AX=0xFFFF;
}
void OPCALL normal_InEaxIb(CPU* cpu, DecodedOp* op) {
    EAX=0xFFFFFFFF;
}
void OPCALL normal_OutIbAl(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_OutIbAx(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_OutIbEax(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_InAlDx(CPU* cpu, DecodedOp* op) {
    AL=0xFF;
}
void OPCALL normal_InAxDx(CPU* cpu, DecodedOp* op) {
    AX=0xFFFF;
}
void OPCALL normal_InEaxDx(CPU* cpu, DecodedOp* op) {
    EAX=0xFFFFFFFF;
}
void OPCALL normal_OutDxAl(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_OutDxAx(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_OutDxEax(CPU* cpu, DecodedOp* op) {
    
}
void OPCALL normal_callJw(CPU* cpu, DecodedOp* op) {
    cpu->push16(cpu->eip.u32 + op->len); cpu->eip.u32 += op->len + (S16)op->imm;
}
void OPCALL normal_callJd(CPU* cpu, DecodedOp* op) {
    cpu->push32(cpu->eip.u32 + op->len); cpu->eip.u32 += op->len + (S32)op->imm;
}
void OPCALL normal_jmp8(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 += op->len + (S8)op->imm;
}
void OPCALL normal_jmp16(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 += op->len + (S16)op->imm;
}
void OPCALL normal_jmp32(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 += op->len + (S32)op->imm;
}
void OPCALL normal_callR16(CPU* cpu, DecodedOp* op) {
    cpu->push16(cpu->eip.u32+op->len); cpu->eip.u32 = cpu->reg[op->reg].u16;
}
void OPCALL normal_callR32(CPU* cpu, DecodedOp* op) {
    cpu->push32(cpu->eip.u32+op->len);cpu->eip.u32 = cpu->reg[op->reg].u32;
}
void OPCALL normal_callE16(CPU* cpu, DecodedOp* op) {
    U32 neweip = readw(eaa(cpu, op)); cpu->push16(cpu->eip.u32+op->len); cpu->eip.u32 = neweip;
}
void OPCALL normal_callE32(CPU* cpu, DecodedOp* op) {
    U32 neweip = readd(eaa(cpu, op));cpu->push32(cpu->eip.u32+op->len);cpu->eip.u32 = neweip;
}
void OPCALL normal_jmpR16(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 = cpu->reg[op->reg].u16;
}
void OPCALL normal_jmpR32(CPU* cpu, DecodedOp* op) {
    cpu->eip.u32 = cpu->reg[op->reg].u32;
}
void OPCALL normal_jmpE16(CPU* cpu, DecodedOp* op) {
    U32 neweip = readw(eaa(cpu, op)); cpu->eip.u32 = neweip;
}
void OPCALL normal_jmpE32(CPU* cpu, DecodedOp* op) {
    U32 neweip = readd(eaa(cpu, op)); cpu->eip.u32 = neweip;
}
void OPCALL normal_callFarE16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op); U16 newip = readw(eaa); U16 newcs = readw(eaa+2); cpu->call(0, newcs, newip, cpu->eip.u32 + op->len);
}
void OPCALL normal_callFarE32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op); U32 newip = readd(eaa); U16 newcs = readw(eaa+4); cpu->call(1, newcs, newip, cpu->eip.u32 + op->len);
}
void OPCALL normal_jmpFarE16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op); U16 newip = readw(eaa); U16 newcs = readw(eaa+2); cpu->jmp(0, newcs, newip, cpu->eip.u32 + op->len);
}
void OPCALL normal_jmpFarE32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op); U32 newip = readd(eaa); U16 newcs = readw(eaa+4); cpu->jmp(1, newcs, newip, cpu->eip.u32 + op->len);
}
void OPCALL normal_larr16r16(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->lar(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);
}
void OPCALL normal_larr16e16(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->lar(readw(eaa(cpu, op)), cpu->reg[op->reg].u16);
}
void OPCALL normal_lslr16r16(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->lsl(cpu->reg[op->rm].u16, cpu->reg[op->reg].u16);
}
void OPCALL normal_lslr16e16(CPU* cpu, DecodedOp* op) {
    cpu->reg[op->reg].u16 = cpu->lsl(readw(eaa(cpu, op)), cpu->reg[op->reg].u16);
}
void OPCALL normal_xaddr32r32(CPU* cpu, DecodedOp* op) {
    cpu->src.u32 = cpu->reg[op->reg].u32; cpu->dst.u32 = cpu->reg[op->rm].u32; cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32; cpu->reg[op->reg].u32 = cpu->dst.u32; cpu->reg[op->rm].u32 =  cpu->result.u32;
}
void OPCALL normal_xaddr32e32(CPU* cpu, DecodedOp* op) {
    U32 address = eaa(cpu, op); cpu->src.u32 = cpu->reg[op->reg].u32; cpu->dst.u32 = readd(address); cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32;cpu->reg[op->reg].u32 = cpu->dst.u32; writed(address, cpu->result.u32);
}
void OPCALL normal_bswap32(CPU* cpu, DecodedOp* op) {
    U32 val = cpu->reg[op->reg].u32; cpu->reg[op->reg].u32 = (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >>  8) | ((val & 0x0000ff00) <<  8) | ((val & 0x000000ff) << 24));
}
void OPCALL normal_cmpxchgg8b(CPU* cpu, DecodedOp* op) {
    U32 address = eaa(cpu, op); U64 value1 = ((U64)EDX) << 32 | EAX; U64 value2 = readq(address); cpu->fillFlags();if (value1 == value2) {cpu->addZF(); writed(address, EBX); writed(address + 4, ECX);} else {cpu->removeZF();EDX = (U32)(value2 >> 32);EAX = (U32)value2;}
}
void OPCALL loadSegment16(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    U16 val = readw(eaa);
    U32 selector = readw(eaa+2);
    if (cpu->setSegment(op->imm, selector)) {cpu->reg[op->reg].u16 = val; cpu->eip.u32+=op->len;}
}
void OPCALL loadSegment32(CPU* cpu, DecodedOp* op) {
    U32 eaa = eaa(cpu, op);
    U32 val = readd(eaa);
    U32 selector = readw(eaa+4);
    if (cpu->setSegment(op->imm, selector)) {cpu->reg[op->reg].u32 = val; cpu->eip.u32+=op->len;}
}
