#include "boxedwine.h"
#ifdef BOXEDWINE_BINARY_TRANSLATOR
#define NEXT_BRANCH1()
#define NEXT_BRANCH2()
#else
#define NEXT_BRANCH1() if (!DecodedBlock::currentBlock->next1) {DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next1
#define NEXT_BRANCH2() if (!DecodedBlock::currentBlock->next2) {DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next2
#endif
U32 common_bound16(CPU* cpu, U32 reg, U32 address){
    if (cpu->reg[reg].u16<cpu->memory->readw(address) || cpu->reg[reg].u16>cpu->memory->readw(address+2)) {
        cpu->prepareException(EXCEPTION_BOUND, 0);
        return 0;
    } else { 
        return 1;
    }
}
U32 common_bound32(CPU* cpu, U32 reg, U32 address){
    if (cpu->reg[reg].u32<cpu->memory->readd(address) || cpu->reg[reg].u32>cpu->memory->readd(address+4)) {
        cpu->prepareException(EXCEPTION_BOUND, 0);
        return 0;
    } else { 
        return 1;
    }
}
void common_int98(CPU* cpu){
    U32 index = cpu->peek32(0);
    if (index<wine_callbackSize && wine_callback[index]) {
        wine_callback[index](cpu);
    } else if (index >= wine_audio_callback_base && index < wine_audio_callback_base + wine_audio_callback_size) {
        wine_audio_callback[index - wine_audio_callback_base](cpu);
    } else {
        kpanic("Uknown int 98 call: %d", index);
    }
}
void common_int99(CPU* cpu){
    U32 index = cpu->peek32(0);
    callOpenGL(cpu, index);    
}
void common_int9A(CPU* cpu) {
#ifdef BOXEDWINE_VULKAN
    U32 index = cpu->peek32(0);
    callVulkan(cpu, index);
#endif
}
void common_intIb(CPU* cpu){
    cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess
}
void common_int3(CPU* cpu) {
    cpu->thread->signalTrap(1);// 1=TRAP_BRKPT
}
void common_cpuid(CPU* cpu){
    cpu->cpuid();
}
void common_loopnz32(CPU* cpu, U32 offset1, U32 offset2) {
    ECX--;
    if (ECX!=0 && !cpu->getZF()) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_loopnz(CPU* cpu, U32 offset1, U32 offset2){
    CX--;
    if (CX!=0 && !cpu->getZF()) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_loopz32(CPU* cpu, U32 offset1, U32 offset2) {
    ECX--;
    if (ECX!=0 && cpu->getZF()) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_loopz(CPU* cpu, U32 offset1, U32 offset2){
    CX--;
    if (CX!=0 && cpu->getZF()) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_loop32(CPU* cpu, U32 offset1, U32 offset2) {
    ECX--;
    if (ECX!=0) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_loop(CPU* cpu, U32 offset1, U32 offset2){
    CX--;
    if (CX!=0) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_jcxz32(CPU* cpu, U32 offset1, U32 offset2) {
    if (ECX==0) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_jcxz(CPU* cpu, U32 offset1, U32 offset2){
    if (CX==0) {
        cpu->eip.u32+=offset1;
        NEXT_BRANCH1();
    } else {
        cpu->eip.u32+=offset2;
        NEXT_BRANCH2();
    }
}
void common_larr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u16 = cpu->lar(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);
}
void common_larr16e16(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u16 = cpu->lar(cpu->memory->readw(address), cpu->reg[reg].u16);
}

U32 common_lar(CPU* cpu, U32 selector, U32 ar) {
    return cpu->lar(selector, ar);
}

void common_lslr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u16 = cpu->lsl(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);
}
void common_lslr16e16(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u16 = cpu->lsl(cpu->memory->readw(address), cpu->reg[reg].u16);
}

void common_lslr32r32(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u32 = cpu->lsl(cpu->reg[srcReg].u32, cpu->reg[dstReg].u32);
}
void common_lslr32e32(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u32 = cpu->lsl(cpu->memory->readw(address), cpu->reg[reg].u32); // intentional 16-bit read
}

U32 common_lsl(CPU* cpu, U32 selector, U32 limit) {
    return cpu->lsl(selector, limit);
}

void common_verre16(CPU* cpu, U32 address){
    cpu->verr(cpu->memory->readw(address));
}
void common_verwe16(CPU* cpu, U32 address){
    cpu->verw(cpu->memory->readw(address));
}

void common_verr(CPU* cpu, U32 selector){
    cpu->verr(selector);
}
void common_verw(CPU* cpu, U32 selector){
    cpu->verw(selector);
}

void common_cmpxchgg8b(CPU* cpu, U32 address){
    U64 value1 = ((U64)EDX) << 32 | EAX;
    U64 value2 = cpu->memory->readq(address);
    cpu->fillFlags();
    if (value1 == value2) {
        cpu->addZF();
        cpu->memory->writed(address, EBX);
        cpu->memory->writed(address + 4, ECX);
    } else {
        cpu->removeZF();
        EDX = (U32)(value2 >> 32);
        EAX = (U32)value2;
    }
}

void common_fxsave(CPU* cpu, U32 address) {
    cpu->memory->writew(address + 0, (U16)cpu->fpu.CW());
    cpu->memory->writew(address + 2, (U16)cpu->fpu.SW());
    cpu->memory->writeb(address + 4, cpu->fpu.GetAbridgedTag());
    cpu->memory->writeb(address + 5, 0);
    cpu->memory->writew(address + 6, 0); // fop
    cpu->memory->writed(address + 8, 0); // fip
    cpu->memory->writew(address + 12, 0); // f cs
    cpu->memory->writew(address + 14, 0); // reserved
    cpu->memory->writed(address + 16, 0); // f dp
    cpu->memory->writew(address + 20, 0); // f ds
    cpu->memory->writew(address + 22, 0); // reserved
    cpu->memory->writed(address + 24, 0x1F80); // mxcsr
    cpu->memory->writed(address + 28, 0xFFFF); // mxcsr mask

    if (cpu->isMMXinUse()) {
        for (int i=0;i<8;i++) {
            cpu->memory->writeq(address+32+i*16, cpu->reg_mmx[i].q);
            cpu->memory->writeq(address+40+i*16, 0);
        }
    } else {
        for (int i=0;i<8;i++) {
            U32 index = (i - cpu->fpu.GetTop()) & 7;
            cpu->fpu.ST80(cpu, address+32+index*16, i);
        }
    }
    for (int i=0;i<8;i++) {
        cpu->memory->writeq(address+160+i*16, cpu->xmm[i].pi.u64[0]);
        cpu->memory->writeq(address+168+i*16, cpu->xmm[i].pi.u64[1]);
    }
}

void common_fxrstor(CPU* cpu, U32 address) {
    cpu->fpu.SetCW(cpu->memory->readw(address));
    cpu->fpu.SetSW(cpu->memory->readw(address+2));
    cpu->fpu.SetTagFromAbridged(cpu->memory->readb(address+4));
    for (int i=0;i<8;i++) {
        cpu->reg_mmx[i].q = cpu->memory->readq(address+32+i*16);
        U32 index = (i - cpu->fpu.GetTop()) & 7;
        cpu->fpu.regs[i].d = cpu->fpu.FLD80(cpu->memory->readq(address+32+index*16), (S16)cpu->memory->readw(address+40+index*16));
    }
    for (int i=0;i<8;i++) {
        cpu->xmm[i].pi.u64[0] = cpu->memory->readq(address+160+i*16);
        cpu->xmm[i].pi.u64[1] = cpu->memory->readq(address+168+i*16);
    }
}

void common_xsave(CPU* cpu, U32 address) {
    kpanic("xsave not implemented");
}

void common_xrstor(CPU* cpu, U32 address) {
    kpanic("xrstore not implemented");
}
