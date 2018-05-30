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

void OPCALL cmovO_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovO_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getOF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovO_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovO_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getOF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNO_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNO_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNO_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNO_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovB_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovB_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getCF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovB_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovB_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getCF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNB_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNB_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNB_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNB_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovZ_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovZ_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovZ_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovZ_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNZ_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNZ_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNZ_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNZ_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovBE_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovBE_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovBE_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovBE_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNBE_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNBE_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNBE_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNBE_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovS_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovS_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovS_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovS_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNS_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNS_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNS_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNS_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovP_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getPF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovP_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getPF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovP_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getPF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovP_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getPF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNP_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNP_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNP_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNP_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovL_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovL_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovL_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovL_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNL_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNL_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNL_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNL_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovLE_16_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovLE_16_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovLE_32_reg(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovLE_32_mem(CPU* cpu, DecodedOp* op) {
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
void OPCALL cmovNLE_16_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
}
void OPCALL cmovNLE_16_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = readw(eaa(cpu, op));
    }
}
void OPCALL cmovNLE_32_reg(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
}
void OPCALL cmovNLE_32_mem(CPU* cpu, DecodedOp* op) {
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
    }
}
