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

void OPCALL setO_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setO_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getOF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNO_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNO_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getOF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setB_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setB_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getCF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNB_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNB_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getCF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setZ_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setZ_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNZ_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNZ_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setBE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setBE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getCF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNBE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNBE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && !cpu->getCF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setS_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setS_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNS_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNS_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getSF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setP_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setP_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getPF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNP_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNP_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getPF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setL_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setL_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()!=cpu->getOF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNL_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNL_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getSF()==cpu->getOF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setLE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setLE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (cpu->getZF() || cpu->getSF()!=cpu->getOF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
void OPCALL setNLE_reg(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        *cpu->reg8[op->reg] = 1;
    } else {
        *cpu->reg8[op->reg] = 0;
    }
    NEXT();
}
void OPCALL setNLE_mem(CPU* cpu, DecodedOp* op) {
    START_OP(cpu, op);
    if (!cpu->getZF() && cpu->getSF()==cpu->getOF()) {
        writeb(eaa(cpu, op), 1);
    } else {
        writeb(eaa(cpu, op), 0);
    }
    NEXT();
}
