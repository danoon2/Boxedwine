#include "boxedwine.h"
void common_pushA32(CPU* cpu){
    U32 sp = ESP;
    cpu->push32(EAX);
    cpu->push32(ECX);
    cpu->push32(EDX);
    cpu->push32(EBX);
    cpu->push32(sp);
    cpu->push32(EBP);
    cpu->push32(ESI);
    cpu->push32(EDI);
}
void common_pushA16(CPU* cpu){
    U16 sp = SP;
    cpu->push16(AX);
    cpu->push16(CX);
    cpu->push16(DX);
    cpu->push16(BX);
    cpu->push16(sp);
    cpu->push16(BP);
    cpu->push16(SI);
    cpu->push16(DI);
}
void common_popA32(CPU* cpu){
    EDI = cpu->pop32();
    ESI = cpu->pop32();
    EBP = cpu->pop32();
    cpu->pop32();
    EBX = cpu->pop32();
    EDX = cpu->pop32();
    ECX = cpu->pop32();
    EAX = cpu->pop32();
}
void common_popA16(CPU* cpu){
    DI = cpu->pop16();
    SI = cpu->pop16();
    BP = cpu->pop16();
    cpu->pop16();
    BX = cpu->pop16();
    DX = cpu->pop16();
    CX = cpu->pop16();
    AX = cpu->pop16();
}
