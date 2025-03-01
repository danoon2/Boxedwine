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
