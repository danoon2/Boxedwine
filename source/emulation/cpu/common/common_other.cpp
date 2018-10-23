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
void common_lslr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->reg[dstReg].u16 = cpu->lsl(cpu->reg[srcReg].u16, cpu->reg[dstReg].u16);
}
void common_lslr16e16(CPU* cpu, U32 reg, U32 address){
    cpu->reg[reg].u16 = cpu->lsl(readw(address), cpu->reg[reg].u16);
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
