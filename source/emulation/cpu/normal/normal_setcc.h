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

void OPCALL normal_setO_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setO_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNO_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNO_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setB_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setB_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNB_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNB_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setZ_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setZ_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNZ_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNZ_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setBE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setBE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNBE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNBE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setS_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setS_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNS_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNS_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setP_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setP_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNP_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNP_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setL_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setL_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNL_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNL_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setLE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setLE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL normal_setNLE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL normal_setNLE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        cpu->memory->writeb(eaa(cpu, op), 1);
    } else {
        cpu->memory->writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
