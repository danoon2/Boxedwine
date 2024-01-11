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

void OPCALL normal_cmovO_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovO_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovO_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovO_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNO_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNO_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNO_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNO_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovB_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovB_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovB_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovB_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNB_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNB_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNB_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNB_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovZ_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovZ_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovZ_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovZ_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNZ_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNZ_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNZ_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNZ_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovBE_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovBE_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovBE_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovBE_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNBE_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNBE_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNBE_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNBE_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovS_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovS_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovS_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovS_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNS_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNS_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNS_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNS_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovP_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovP_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovP_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovP_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNP_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNP_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNP_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNP_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovL_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovL_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovL_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovL_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNL_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNL_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNL_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNL_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovLE_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovLE_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovLE_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovLE_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNLE_16_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->reg[op->rm].u16;
    }
    NEXT();
}
void OPCALL normal_cmovNLE_16_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u16 = cpu->memory->readw(eaa(cpu, op));
    }
    NEXT();
}
void OPCALL normal_cmovNLE_32_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
    }
    NEXT();
}
void OPCALL normal_cmovNLE_32_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->reg[op->reg].u32 = cpu->memory->readd(eaa(cpu, op));
    }
    NEXT();
}
