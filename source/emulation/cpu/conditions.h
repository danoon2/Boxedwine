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

void OPCALL cmovO_16_reg(struct CPU* cpu, struct Op* op) {
    if (getOF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovO_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovO_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovO_32_reg(struct CPU* cpu, struct Op* op) {
    if (getOF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovO_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovO_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNO_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getOF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNO_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNO_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNO_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getOF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNO_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNO_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovB_16_reg(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovB_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovB_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovB_32_reg(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovB_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovB_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNB_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getCF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNB_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNB_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNB_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getCF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNB_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNB_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovZ_16_reg(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovZ_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovZ_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovZ_32_reg(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovZ_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovZ_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNZ_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNZ_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNZ_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNZ_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNZ_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNZ_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovBE_16_reg(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getCF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovBE_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovBE_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovBE_32_reg(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getCF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovBE_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovBE_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNBE_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNBE_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNBE_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNBE_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNBE_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNBE_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && !getCF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovS_16_reg(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovS_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovS_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovS_32_reg(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovS_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovS_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNS_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getSF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNS_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getSF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNS_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getSF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNS_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getSF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNS_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getSF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNS_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getSF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovP_16_reg(struct CPU* cpu, struct Op* op) {
    if (getPF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovP_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getPF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovP_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getPF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovP_32_reg(struct CPU* cpu, struct Op* op) {
    if (getPF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovP_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getPF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovP_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getPF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNP_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getPF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNP_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getPF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNP_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getPF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNP_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getPF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNP_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getPF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNP_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getPF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovL_16_reg(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovL_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovL_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovL_32_reg(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovL_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovL_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNL_16_reg(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNL_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNL_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNL_32_reg(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNL_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNL_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovLE_16_reg(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovLE_16_mem16(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovLE_16_mem32(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovLE_32_reg(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovLE_32_mem16(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovLE_32_mem32(struct CPU* cpu, struct Op* op) {
    if (getZF(cpu) || getSF(cpu)!=getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNLE_16_reg(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u16 = cpu->reg[op->r2].u16;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNLE_16_mem16(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNLE_16_mem32(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u16 = readw(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNLE_32_reg(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNLE_32_mem16(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa16(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
void OPCALL cmovNLE_32_mem32(struct CPU* cpu, struct Op* op) {
    if (!getZF(cpu) && getSF(cpu)==getOF(cpu)) {
        cpu->reg[op->r1].u32 = readd(cpu->thread, eaa32(cpu, op));
    }
    CYCLES(1);
    NEXT();
}
