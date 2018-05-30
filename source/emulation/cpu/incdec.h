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

void OPCALL inc8_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf=getCF(cpu);
    cpu->dst.u8=*cpu->reg8[op->r1];
    cpu->result.u8=cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
    *cpu->reg8[op->r1] = cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL inc8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u8=readb(cpu->thread, eaa);
    cpu->result.u8=cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
    writeb(cpu->thread, eaa, cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL inc8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u8=readb(cpu->thread, eaa);
    cpu->result.u8=cpu->dst.u8 + 1;
    cpu->lazyFlags = FLAGS_INC8;
    writeb(cpu->thread, eaa, cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL inc16_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf=getCF(cpu);
    cpu->dst.u16=cpu->reg[op->r1].u16;
    cpu->result.u16=cpu->dst.u16 + 1;
    cpu->lazyFlags = FLAGS_INC16;
    cpu->reg[op->r1].u16 = cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL inc16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u16=readw(cpu->thread, eaa);
    cpu->result.u16=cpu->dst.u16 + 1;
    cpu->lazyFlags = FLAGS_INC16;
    writew(cpu->thread, eaa, cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL inc16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u16=readw(cpu->thread, eaa);
    cpu->result.u16=cpu->dst.u16 + 1;
    cpu->lazyFlags = FLAGS_INC16;
    writew(cpu->thread, eaa, cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL inc32_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf=getCF(cpu);
    cpu->dst.u32=cpu->reg[op->r1].u32;
    cpu->result.u32=cpu->dst.u32 + 1;
    cpu->lazyFlags = FLAGS_INC32;
    cpu->reg[op->r1].u32 = cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL inc32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u32=readd(cpu->thread, eaa);
    cpu->result.u32=cpu->dst.u32 + 1;
    cpu->lazyFlags = FLAGS_INC32;
    writed(cpu->thread, eaa, cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL inc32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u32=readd(cpu->thread, eaa);
    cpu->result.u32=cpu->dst.u32 + 1;
    cpu->lazyFlags = FLAGS_INC32;
    writed(cpu->thread, eaa, cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL dec8_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf=getCF(cpu);
    cpu->dst.u8=*cpu->reg8[op->r1];
    cpu->result.u8=cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
    *cpu->reg8[op->r1] = cpu->result.u8;
    CYCLES(1);
    NEXT();
}
void OPCALL dec8_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u8=readb(cpu->thread, eaa);
    cpu->result.u8=cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
    writeb(cpu->thread, eaa, cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL dec8_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u8=readb(cpu->thread, eaa);
    cpu->result.u8=cpu->dst.u8 - 1;
    cpu->lazyFlags = FLAGS_DEC8;
    writeb(cpu->thread, eaa, cpu->result.u8);
    CYCLES(3);
    NEXT();
}
void OPCALL dec16_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf=getCF(cpu);
    cpu->dst.u16=cpu->reg[op->r1].u16;
    cpu->result.u16=cpu->dst.u16 - 1;
    cpu->lazyFlags = FLAGS_DEC16;
    cpu->reg[op->r1].u16 = cpu->result.u16;
    CYCLES(1);
    NEXT();
}
void OPCALL dec16_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u16=readw(cpu->thread, eaa);
    cpu->result.u16=cpu->dst.u16 - 1;
    cpu->lazyFlags = FLAGS_DEC16;
    writew(cpu->thread, eaa, cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL dec16_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u16=readw(cpu->thread, eaa);
    cpu->result.u16=cpu->dst.u16 - 1;
    cpu->lazyFlags = FLAGS_DEC16;
    writew(cpu->thread, eaa, cpu->result.u16);
    CYCLES(3);
    NEXT();
}
void OPCALL dec32_reg(struct CPU* cpu, struct Op* op) {
    cpu->oldcf=getCF(cpu);
    cpu->dst.u32=cpu->reg[op->r1].u32;
    cpu->result.u32=cpu->dst.u32 - 1;
    cpu->lazyFlags = FLAGS_DEC32;
    cpu->reg[op->r1].u32 = cpu->result.u32;
    CYCLES(1);
    NEXT();
}
void OPCALL dec32_mem16(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u32=readd(cpu->thread, eaa);
    cpu->result.u32=cpu->dst.u32 - 1;
    cpu->lazyFlags = FLAGS_DEC32;
    writed(cpu->thread, eaa, cpu->result.u32);
    CYCLES(3);
    NEXT();
}
void OPCALL dec32_mem32(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    cpu->oldcf=getCF(cpu);
    cpu->dst.u32=readd(cpu->thread, eaa);
    cpu->result.u32=cpu->dst.u32 - 1;
    cpu->lazyFlags = FLAGS_DEC32;
    writed(cpu->thread, eaa, cpu->result.u32);
    CYCLES(3);
    NEXT();
}
