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

void OPCALL pushReg16(struct CPU* cpu, struct Op* op){
    push16(cpu, cpu->reg[op->r1].u16);
    CYCLES(1);
    NEXT();
}
void OPCALL popReg16(struct CPU * cpu, struct Op * op){
    cpu->reg[op->r1].u16 = pop16(cpu);
    CYCLES(1);
    NEXT();
}
void OPCALL pushReg32(struct CPU* cpu, struct Op* op){
    push32(cpu, cpu->reg[op->r1].u32);
    CYCLES(1);
    NEXT();
}
void OPCALL popReg32(struct CPU * cpu, struct Op * op){
    cpu->reg[op->r1].u32 = pop32(cpu);
    CYCLES(1);
    NEXT();
}
