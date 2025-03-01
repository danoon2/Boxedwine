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
    cpu->fillFlagsNoCFOF();
    AX = (S16)((S8)AL) * (S8)(src);
    if ((S16)AX<-128 || (S16)AX>127) {
        cpu->flags|=CF|OF;
    } else {
        cpu->flags&=~(CF|OF);
    }
}
void common_mul8(CPU* cpu, U8 src) {
    cpu->fillFlagsNoCFOF();
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
