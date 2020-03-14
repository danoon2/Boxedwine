#include "boxedwine.h"
#define NEXT_BRANCH1() if (!DecodedBlock::currentBlock->next1) {DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next1
#define NEXT_BRANCH2() if (!DecodedBlock::currentBlock->next2) {DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);} cpu->nextBlock = DecodedBlock::currentBlock->next2
U32 common_bound16(CPU* cpu, U32 reg, U32 address){
    if (cpu->reg[reg].u16<readw(address) || cpu->reg[reg].u16>readw(address+2)) {
        cpu->prepareException(EXCEPTION_BOUND, 0);
        return 0;
    } else { 
        return 1;
    }
}
U32 common_bound32(CPU* cpu, U32 reg, U32 address){
    if (cpu->reg[reg].u32<readd(address) || cpu->reg[reg].u32>readd(address+4)) {
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
    } else {
        kpanic("Uknown int 98 call: %d", index);
    }
}
void common_int99(CPU* cpu){
    U32 index = cpu->peek32(0);
    if (index<int99CallbackSize && int99Callback[index]) {
        lastGlCallTime = KSystem::getMilliesSinceStart();
        int99Callback[index](cpu);
    } else {
        kpanic("Uknown int 99 call: %d", index);
    }
}
void common_intIb(CPU* cpu){
    cpu->thread->signalIllegalInstruction(5);// 5=ILL_PRVOPC  // :TODO: just a guess
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
    if (CX!=0 && !cpu->getZF()) {
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
    cpu->reg[reg].u16 = cpu->lar(readw(address), cpu->reg[reg].u16);
}

U32 common_lar(CPU* cpu, U32 selector, U32 ar) {
    return cpu->lar(selector, ar);
}

void common_lslr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u16 = cpu->lsl(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);
}
void common_lslr16e16(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u16 = cpu->lsl(readw(address), cpu->reg[reg].u16);
}

void common_lslr32r32(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u32 = cpu->lsl(cpu->reg[srcReg].u32, cpu->reg[dstReg].u32);
}
void common_lslr32e32(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u32 = cpu->lsl(readw(address), cpu->reg[reg].u32); // intentional 16-bit read
}

U32 common_lsl(CPU* cpu, U32 selector, U32 limit) {
    if (!cpu->logFile) cpu->logFile = fopen("lsl.txt", "w");
    return cpu->lsl(selector, limit);
}

void common_verre16(CPU* cpu, U32 address){
    cpu->verr(readw(address));
}
void common_verwe16(CPU* cpu, U32 address){
    cpu->verw(readw(address));
}

void common_verr(CPU* cpu, U32 selector){
    cpu->verr(selector);
}
void common_verw(CPU* cpu, U32 selector){
    cpu->verw(selector);
}

void common_cmpxchgg8b(CPU* cpu, U32 address){
    U64 value1 = ((U64)EDX) << 32 | EAX;
    U64 value2 = readq(address);
    cpu->fillFlags();
    if (value1 == value2) {
        cpu->addZF();
        writed(address, EBX);
        writed(address + 4, ECX);
    } else {
        cpu->removeZF();
        EDX = (U32)(value2 >> 32);
        EAX = (U32)value2;
    }
}

void common_fxsave(CPU* cpu, U32 address) {
    writew(address + 0, (U16)cpu->fpu.CW());
    writew(address + 2, (U16)cpu->fpu.SW());
    writeb(address + 4, cpu->fpu.GetAbridgedTag());
    writeb(address + 5, 0);
    writew(address + 6, 0); // fop
    writed(address + 8, 0); // fip
    writew(address + 12, 0); // f cs
    writew(address + 14, 0); // reserved
    writed(address + 16, 0); // f dp
    writew(address + 20, 0); // f ds
    writew(address + 22, 0); // reserved
    writed(address + 24, 0x1F80); // mxcsr
    writed(address + 28, 0xFFFF); // mxcsr mask

    if (cpu->isMMXinUse()) {
        for (int i=0;i<8;i++) {
            writeq(address+32+i*16, cpu->reg_mmx[i].q);
            writeq(address+40+i*16, 0);
        }
    } else {
        for (int i=0;i<8;i++) {
            cpu->fpu.ST80(cpu, address+32+i*16, i);
        }
    }
    for (int i=0;i<8;i++) {
        writeq(address+160+i*16, cpu->xmm[i].pi.u64[0]);
        writeq(address+168+i*16, cpu->xmm[i].pi.u64[1]);
    }
}

void common_fxrstor(CPU* cpu, U32 address) {
    cpu->fpu.SetCW(readw(address));
    cpu->fpu.SetCW(readw(address+2));
    cpu->fpu.SetTagFromAbridged(readb(address+4));
    for (int i=0;i<8;i++) {
        cpu->reg_mmx[i].q = readq(address+32+i*16);
        cpu->fpu.FLD_F80(readq(address+32+i*16), (S16)readw(address+40+i*16));
    }
    for (int i=0;i<8;i++) {
        cpu->xmm[i].pi.u64[0] = readq(address+160+i*16);
        cpu->xmm[i].pi.u64[1] = readq(address+168+i*16);
    }
}

void common_xsave(CPU* cpu, U32 address) {
    kpanic("xsave not implemented");
}

void common_xrstor(CPU* cpu, U32 address) {
    kpanic("xrstore not implemented");
}