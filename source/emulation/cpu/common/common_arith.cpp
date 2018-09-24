#include "boxedwine.h"
void common_dimul16(CPU* cpu, U32 arg1, U32 arg2, U32 regResult) {
    S32 res=(S16)arg1 * (S32)((S16)arg2);
    cpu->fillFlagsNoCFOF();
    if ((res >= -32767) && (res <= 32767)) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[regResult].u16 = (U16)res;
}
void common_dimul32(CPU* cpu, U32 arg1, U32 arg2, U32 regResult) {
    S64 res=(S32)(arg1) * (S64)((S32)arg2);
    cpu->fillFlagsNoCFOF();
    if (res>=-2147483647l && res<=2147483647l) {
        cpu->removeFlag(CF|OF);
    } else {
        cpu->addFlag(CF|OF);
    }
    cpu->reg[regResult].u32 = (U32)res;
}
void common_imul8(CPU* cpu, U8 src) {
    AX = (S16)((S8)AL) * (S8)(src);
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void common_mul8(CPU* cpu, U8 src) {
    AX = AL * src;
    if (AH) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void common_imul16(CPU* cpu, U16 src) {
    S32 result = (S32)((S16)AX) * (S16)src;
    cpu->fillFlagsNoCFOF();
    AX = (U16)result;
    DX = (U16)(result >> 16);
    if (result>32767 || result<-32768) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void common_mul16(CPU* cpu, U16 src) {
    U32 result = (U32)AX * src;
    cpu->fillFlagsNoCFOF();
    AX = (U16)result;
    DX = (U16)(result >> 16);
    if (DX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void common_imul32(CPU* cpu, U32 src) {
    S64 result = (S64)((S32)EAX) * ((S32)(src));
    cpu->fillFlagsNoCFOF();
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    if (result>0x7fffffffl || result<-0x7fffffffl) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void common_mul32(CPU* cpu, U32 src) {
    U64 result = (U64)EAX * src;
    cpu->fillFlagsNoCFOF();
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    if (EDX) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
