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

void genArithRR(struct GenData* data, const char* op, const char* flags, const char* bits, const char* reg, const char* rm, U32 useResult, U32 useCF, const char* cycles) {
    if (useCF) {
        out(data, "cpu->oldcf = ");
        out(data, getFlag(data, CF));
        out(data, ";");
    }
    out(data, "cpu->dst.u");
    out(data, bits);
    out(data, " = ");
    out(data, reg);
    out(data, "; cpu->src.u");
    out(data, bits);
    out(data, " = ");
    out(data, rm);
    out(data, "; cpu->result.u");
    out(data, bits);
    out(data, " = cpu->dst.u");
    out(data, bits);
    out(data, " ");
    out(data, op);
    out(data, " cpu->src.u");
    out(data, bits);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, "cpu->oldcf");
    }
    out(data, "; cpu->lazyFlags = ");
    out(data, flags);
    if (useResult) {
        out(data, ";");
        out(data, reg);
        out(data, " = cpu->result.u");
        out(data, bits);        
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithRR_noflags(struct GenData* data, const char* op, const char* bits, const char* reg, const char* rm, U32 useCF, const char* cycles) {
    out(data, reg);
    out(data, " = ");
    out(data, reg);
    out(data, " ");
    out(data, op);
    out(data, " ");
    out(data, rm);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, getFlag(data, CF));
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithER(struct GenData* data, const char* op, const char* flags, const char* bits, const char* address, const char* memWidth, const char* reg, U32 useResult, U32 useCF, const char* cycles) {
    if (useCF) {
        out(data, "cpu->oldcf = ");
        out(data, getFlag(data, CF));
        out(data, ";");
    }
    out(data, "eaa = ");
    out(data, address);
    out(data, "; cpu->dst.u");
    out(data, bits);
    out(data, " = read");
    out(data, memWidth);
    out(data, "(cpu->thread, eaa); cpu->src.u");
    out(data, bits);
    out(data, " = ");
    out(data, reg);
    out(data, "; cpu->result.u");
    out(data, bits);
    out(data, " = cpu->dst.u");
    out(data, bits);
    out(data, " ");
    out(data, op);
    out(data, " cpu->src.u");
    out(data, bits);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, "cpu->oldcf");
    }
    out(data, "; cpu->lazyFlags = ");
    out(data, flags);
    if (useResult) {
        out(data, "; write");
        out(data, memWidth);
        out(data, "(cpu->thread, eaa,  cpu->result.u");
        out(data, bits);
        out(data, ")");
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithER_noflags(struct GenData* data, const char* op, const char* bits, const char* address, const char* memWidth, const char* reg, U32 useCF, const char* cycles) {
    out(data, "eaa = ");
    out(data, address);
    out(data, "; write");
    out(data, memWidth);
    out(data, "(cpu->thread, eaa, read");
    out(data, memWidth);
    out(data, "(cpu->thread, eaa) ");
    out(data, op);
    out(data, " ");
    out(data, reg);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, getFlag(data, CF));
    }
    out(data, ");CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithRE(struct GenData* data, const char* op, const char* flags, const char* bits, const char* address, const char* memWidth, const char* reg, U32 useResult, U32 useCF, const char* cycles) {
    if (useCF) {
        out(data, "cpu->oldcf = ");
        out(data, getFlag(data, CF));
        out(data, ";");
    }
    out(data, "cpu->dst.u");
    out(data, bits);
    out(data, " = ");
    out(data, reg);
    out(data, "; cpu->src.u");
    out(data, bits);
    out(data, " = read");
    out(data, memWidth);
    out(data, "(cpu->thread, ");
    out(data, address);
    out(data, "); cpu->result.u");
    out(data, bits);
    out(data, " = cpu->dst.u");
    out(data, bits);
    out(data, " ");
    out(data, op);
    out(data, " cpu->src.u");
    out(data, bits);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, "cpu->oldcf");
    }
    out(data, "; cpu->lazyFlags = ");
    out(data, flags);
    if (useResult) {
        out(data, "; ");
        out(data, reg);
        out(data, " = cpu->result.u");
        out(data, bits);
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithRE_noflags(struct GenData* data, const char* op, const char* bits, const char* address, const char* memWidth, const char* reg, U32 useCF, const char* cycles) {
    out(data, reg);
    out(data, " = ");
    out(data, reg);
    out(data, " ");
    out(data, op);
    out(data, " read");
    out(data, memWidth);
    out(data, "(cpu->thread, ");
    out(data, address);
    out(data, ")");
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, getFlag(data, CF));
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithR(struct GenData* data, const char* op, const char* flags, const char* bits, const char* reg, unsigned int value, U32 useResult, U32 useCF, const char* cycles) {
    char tmp[16];

    if (useCF) {
        out(data, "cpu->oldcf = ");
        out(data, getFlag(data, CF));
        out(data, ";");
    }
    out(data, "cpu->dst.u");
    out(data, bits);
    out(data, " = ");
    out(data, reg);
    out(data, "; cpu->src.u");
    out(data, bits);
    out(data, " = 0x");
    itoa(value, tmp, 16);
    out(data, tmp);
    out(data, "; cpu->result.u");
    out(data, bits);
    out(data, " = cpu->dst.u");
    out(data, bits);
    out(data, " ");
    out(data, op);
    out(data, " cpu->src.u");
    out(data, bits);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, "cpu->oldcf");
    }
    out(data, "; cpu->lazyFlags = ");
    out(data, flags);
    if (useResult) {
        out(data, "; ");
        out(data, reg);
        out(data, " = cpu->result.u");
        out(data, bits);
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithR_noflags(struct GenData* data, const char* op, const char* bits, const char* reg, unsigned int value, U32 useCF, const char* cycles) {
    char tmp[16];

    out(data, reg);
    out(data, " = ");
    out(data, reg);
    out(data, " ");
    out(data, op);
    out(data, " 0x");
    itoa(value, tmp, 16);
    out(data, tmp);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, getFlag(data, CF));
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithE(struct GenData* data, const char* op, const char* flags, const char* bits, const char* address, const char* memWidth, unsigned int value, U32 useResult, U32 useCF, const char* cycles) {
    char tmp[16];

    if (useCF) {
        out(data, "cpu->oldcf = ");
        out(data, getFlag(data, CF));
        out(data, ";");
    }
    out(data, "eaa = ");
    out(data, address);
    out(data, "; cpu->dst.u");
    out(data, bits);
    out(data, " = read");
    out(data, memWidth);
    out(data, "(cpu->thread, eaa); cpu->src.u");
    out(data, bits);
    out(data, " = 0x");
    itoa(value, tmp, 16);
    out(data, tmp);
    out(data, "; cpu->result.u");
    out(data, bits);
    out(data, " = cpu->dst.u");
    out(data, bits);
    out(data, " ");
    out(data, op);
    out(data, " cpu->src.u");
    out(data, bits);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, "cpu->oldcf");
    }
    out(data, "; cpu->lazyFlags = ");
    out(data, flags);
    if (useResult) {
        out(data, "; write");
        out(data, memWidth);
        out(data, "(cpu->thread, eaa,  cpu->result.u");
        out(data, bits);
        out(data, ")");
    }
    out(data, ";CYCLES(");
    out(data, cycles);
    out(data, ");");
}

void genArithE_noflags(struct GenData* data, const char* op, const char* bits, const char* address, const char* memWidth, unsigned int value, U32 useCF, const char* cycles) {
    char tmp[16];

    out(data, "eaa = ");
    out(data, address);
    out(data, "; write");
    out(data, memWidth);
    out(data, "(cpu->thread, eaa, read");
    out(data, memWidth);
    out(data, "(cpu->thread, eaa) ");
    out(data, op);
    out(data, " 0x");
    itoa(value, tmp, 16);
    out(data, tmp);
    if (useCF) {
        out(data, " ");
        out(data, op);
        out(data, " ");
        out(data, getFlag(data, CF));
    }
    out(data, ");CYCLES(");
    out(data, cycles);
    out(data, ");");
}
void OPCALL addr8r8(struct CPU* cpu, struct Op* op);
void OPCALL addr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adde8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL adde8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adde8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL adde8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen000(struct GenData* data, struct Op* op) {
    if (op->func==addr8r8) {
        genArithRR(data, "+", "FLAGS_ADD8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==addr8r8_noflags) {
        genArithRR_noflags(data, "+", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==adde8r8_16) {
        genArithER(data, "+", "FLAGS_ADD8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==adde8r8_16_noflags) {
        genArithER_noflags(data, "+", "8", geteaa(op), "b", r8(op->reg), 0,"3");
    } else if (op->func==adde8r8_32) {
        genArithER(data, "+", "FLAGS_ADD8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==adde8r8_32_noflags) {
        genArithER_noflags(data, "+", "8", getEaa32(op), "b", r8(op->reg), 0,"3");
    }
}
void OPCALL addr16r16(struct CPU* cpu, struct Op* op);
void OPCALL addr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adde16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL adde16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adde16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL adde16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen001(struct GenData* data, struct Op* op) {
    if (op->func==addr16r16) {
        genArithRR(data, "+", "FLAGS_ADD16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==addr16r16_noflags) {
        genArithRR_noflags(data, "+", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==adde16r16_16) {
        genArithER(data, "+", "FLAGS_ADD16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==adde16r16_16_noflags) {
        genArithER_noflags(data, "+", "16", geteaa(op), "w", r16(op->reg), 0,"3");
    } else if (op->func==adde16r16_32) {
        genArithER(data, "+", "FLAGS_ADD16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==adde16r16_32_noflags) {
        genArithER_noflags(data, "+", "16", getEaa32(op), "w", r16(op->reg), 0,"3");
    }
}
void OPCALL addr32r32(struct CPU* cpu, struct Op* op);
void OPCALL addr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adde32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL adde32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adde32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL adde32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen201(struct GenData* data, struct Op* op) {
    if (op->func==addr32r32) {
        genArithRR(data, "+", "FLAGS_ADD32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==addr32r32_noflags) {
        genArithRR_noflags(data, "+", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==adde32r32_16) {
        genArithER(data, "+", "FLAGS_ADD32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==adde32r32_16_noflags) {
        genArithER_noflags(data, "+", "32", geteaa(op), "d", r32(op->reg), 0,"3");
    } else if (op->func==adde32r32_32) {
        genArithER(data, "+", "FLAGS_ADD32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==adde32r32_32_noflags) {
        genArithER_noflags(data, "+", "32", getEaa32(op), "d", r32(op->reg), 0,"3");
    }
}
void OPCALL addr8r8(struct CPU* cpu, struct Op* op);
void OPCALL addr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL addr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL addr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL addr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL addr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen002(struct GenData* data, struct Op* op) {
    if (op->func==addr8r8) {
        genArithRR(data, "+", "FLAGS_ADD8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==addr8r8_noflags) {
        genArithRR_noflags(data, "+", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==addr8e8_16) {
        genArithRE(data, "+", "FLAGS_ADD8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==addr8e8_16_noflags) {
        genArithRE_noflags(data, "+", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==addr8e8_32) {
        genArithRE(data, "+", "FLAGS_ADD8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==addr8e8_32_noflags) {
        genArithRE_noflags(data, "+", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL addr16r16(struct CPU* cpu, struct Op* op);
void OPCALL addr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL addr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL addr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL addr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL addr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen003(struct GenData* data, struct Op* op) {
    if (op->func==addr16r16) {
        genArithRR(data, "+", "FLAGS_ADD16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==addr16r16_noflags) {
        genArithRR_noflags(data, "+", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==addr16e16_16) {
        genArithRE(data, "+", "FLAGS_ADD16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==addr16e16_16_noflags) {
        genArithRE_noflags(data, "+", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==addr16e16_32) {
        genArithRE(data, "+", "FLAGS_ADD16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==addr16e16_32_noflags) {
        genArithRE_noflags(data, "+", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL addr32r32(struct CPU* cpu, struct Op* op);
void OPCALL addr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL addr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL addr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL addr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL addr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen203(struct GenData* data, struct Op* op) {
    if (op->func==addr32r32) {
        genArithRR(data, "+", "FLAGS_ADD32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==addr32r32_noflags) {
        genArithRR_noflags(data, "+", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==addr32e32_16) {
        genArithRE(data, "+", "FLAGS_ADD32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==addr32e32_16_noflags) {
        genArithRE_noflags(data, "+", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==addr32e32_32) {
        genArithRE(data, "+", "FLAGS_ADD32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==addr32e32_32_noflags) {
        genArithRE_noflags(data, "+", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL add8_reg(struct CPU* cpu, struct Op* op);
void OPCALL add8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen004(struct GenData* data, struct Op* op) {
    if (op->func==add8_reg) {
        genArithR(data, "+", "FLAGS_ADD8", "8", r8(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==add8_reg_noflags) {
        genArithR_noflags(data, "+", "8", r8(op->reg), op->imm, 0,"1");
    }
}
void OPCALL add16_reg(struct CPU* cpu, struct Op* op);
void OPCALL add16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen005(struct GenData* data, struct Op* op) {
    if (op->func==add16_reg) {
        genArithR(data, "+", "FLAGS_ADD16", "16", r16(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==add16_reg_noflags) {
        genArithR_noflags(data, "+", "16", r16(op->reg), op->imm, 0,"1");
    }
}
void OPCALL add32_reg(struct CPU* cpu, struct Op* op);
void OPCALL add32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen205(struct GenData* data, struct Op* op) {
    if (op->func==add32_reg) {
        genArithR(data, "+", "FLAGS_ADD32", "32", r32(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==add32_reg_noflags) {
        genArithR_noflags(data, "+", "32", r32(op->reg), op->imm, 0,"1");
    }
}
void OPCALL orr8r8(struct CPU* cpu, struct Op* op);
void OPCALL orr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ore8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL ore8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ore8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL ore8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen008(struct GenData* data, struct Op* op) {
    if (op->func==orr8r8) {
        genArithRR(data, "|", "FLAGS_OR8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==orr8r8_noflags) {
        genArithRR_noflags(data, "|", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==ore8r8_16) {
        genArithER(data, "|", "FLAGS_OR8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==ore8r8_16_noflags) {
        genArithER_noflags(data, "|", "8", geteaa(op), "b", r8(op->reg), 0,"3");
    } else if (op->func==ore8r8_32) {
        genArithER(data, "|", "FLAGS_OR8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==ore8r8_32_noflags) {
        genArithER_noflags(data, "|", "8", getEaa32(op), "b", r8(op->reg), 0,"3");
    }
}
void OPCALL orr16r16(struct CPU* cpu, struct Op* op);
void OPCALL orr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ore16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL ore16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ore16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL ore16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen009(struct GenData* data, struct Op* op) {
    if (op->func==orr16r16) {
        genArithRR(data, "|", "FLAGS_OR16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==orr16r16_noflags) {
        genArithRR_noflags(data, "|", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==ore16r16_16) {
        genArithER(data, "|", "FLAGS_OR16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==ore16r16_16_noflags) {
        genArithER_noflags(data, "|", "16", geteaa(op), "w", r16(op->reg), 0,"3");
    } else if (op->func==ore16r16_32) {
        genArithER(data, "|", "FLAGS_OR16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==ore16r16_32_noflags) {
        genArithER_noflags(data, "|", "16", getEaa32(op), "w", r16(op->reg), 0,"3");
    }
}
void OPCALL orr32r32(struct CPU* cpu, struct Op* op);
void OPCALL orr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ore32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL ore32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ore32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL ore32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen209(struct GenData* data, struct Op* op) {
    if (op->func==orr32r32) {
        genArithRR(data, "|", "FLAGS_OR32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==orr32r32_noflags) {
        genArithRR_noflags(data, "|", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==ore32r32_16) {
        genArithER(data, "|", "FLAGS_OR32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==ore32r32_16_noflags) {
        genArithER_noflags(data, "|", "32", geteaa(op), "d", r32(op->reg), 0,"3");
    } else if (op->func==ore32r32_32) {
        genArithER(data, "|", "FLAGS_OR32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==ore32r32_32_noflags) {
        genArithER_noflags(data, "|", "32", getEaa32(op), "d", r32(op->reg), 0,"3");
    }
}
void OPCALL orr8r8(struct CPU* cpu, struct Op* op);
void OPCALL orr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL orr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL orr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL orr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL orr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen00a(struct GenData* data, struct Op* op) {
    if (op->func==orr8r8) {
        genArithRR(data, "|", "FLAGS_OR8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==orr8r8_noflags) {
        genArithRR_noflags(data, "|", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==orr8e8_16) {
        genArithRE(data, "|", "FLAGS_OR8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==orr8e8_16_noflags) {
        genArithRE_noflags(data, "|", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==orr8e8_32) {
        genArithRE(data, "|", "FLAGS_OR8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==orr8e8_32_noflags) {
        genArithRE_noflags(data, "|", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL orr16r16(struct CPU* cpu, struct Op* op);
void OPCALL orr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL orr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL orr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL orr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL orr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen00b(struct GenData* data, struct Op* op) {
    if (op->func==orr16r16) {
        genArithRR(data, "|", "FLAGS_OR16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==orr16r16_noflags) {
        genArithRR_noflags(data, "|", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==orr16e16_16) {
        genArithRE(data, "|", "FLAGS_OR16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==orr16e16_16_noflags) {
        genArithRE_noflags(data, "|", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==orr16e16_32) {
        genArithRE(data, "|", "FLAGS_OR16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==orr16e16_32_noflags) {
        genArithRE_noflags(data, "|", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL orr32r32(struct CPU* cpu, struct Op* op);
void OPCALL orr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL orr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL orr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL orr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL orr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen20b(struct GenData* data, struct Op* op) {
    if (op->func==orr32r32) {
        genArithRR(data, "|", "FLAGS_OR32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==orr32r32_noflags) {
        genArithRR_noflags(data, "|", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==orr32e32_16) {
        genArithRE(data, "|", "FLAGS_OR32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==orr32e32_16_noflags) {
        genArithRE_noflags(data, "|", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==orr32e32_32) {
        genArithRE(data, "|", "FLAGS_OR32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==orr32e32_32_noflags) {
        genArithRE_noflags(data, "|", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL or8_reg(struct CPU* cpu, struct Op* op);
void OPCALL or8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen00c(struct GenData* data, struct Op* op) {
    if (op->func==or8_reg) {
        genArithR(data, "|", "FLAGS_OR8", "8", r8(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==or8_reg_noflags) {
        genArithR_noflags(data, "|", "8", r8(op->reg), op->imm, 0,"1");
    }
}
void OPCALL or16_reg(struct CPU* cpu, struct Op* op);
void OPCALL or16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen00d(struct GenData* data, struct Op* op) {
    if (op->func==or16_reg) {
        genArithR(data, "|", "FLAGS_OR16", "16", r16(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==or16_reg_noflags) {
        genArithR_noflags(data, "|", "16", r16(op->reg), op->imm, 0,"1");
    }
}
void OPCALL or32_reg(struct CPU* cpu, struct Op* op);
void OPCALL or32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen20d(struct GenData* data, struct Op* op) {
    if (op->func==or32_reg) {
        genArithR(data, "|", "FLAGS_OR32", "32", r32(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==or32_reg_noflags) {
        genArithR_noflags(data, "|", "32", r32(op->reg), op->imm, 0,"1");
    }
}
void OPCALL adcr8r8(struct CPU* cpu, struct Op* op);
void OPCALL adcr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adce8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL adce8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adce8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL adce8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen010(struct GenData* data, struct Op* op) {
    if (op->func==adcr8r8) {
        genArithRR(data, "+", "FLAGS_ADC8", "8", r8(op->reg), r8(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adcr8r8_noflags) {
        genArithRR_noflags(data, "+", "8", r8(op->reg), r8(op->rm), 1,"1");
    } else if (op->func==adce8r8_16) {
        genArithER(data, "+", "FLAGS_ADC8", "8", geteaa(op), "b", r8(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adce8r8_16_noflags) {
        genArithER_noflags(data, "+", "8", geteaa(op), "b", r8(op->reg), 1,"3");
    } else if (op->func==adce8r8_32) {
        genArithER(data, "+", "FLAGS_ADC8", "8", getEaa32(op), "b", r8(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adce8r8_32_noflags) {
        genArithER_noflags(data, "+", "8", getEaa32(op), "b", r8(op->reg), 1,"3");
    }
}
void OPCALL adcr16r16(struct CPU* cpu, struct Op* op);
void OPCALL adcr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adce16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL adce16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adce16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL adce16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen011(struct GenData* data, struct Op* op) {
    if (op->func==adcr16r16) {
        genArithRR(data, "+", "FLAGS_ADC16", "16", r16(op->reg), r16(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adcr16r16_noflags) {
        genArithRR_noflags(data, "+", "16", r16(op->reg), r16(op->rm), 1,"1");
    } else if (op->func==adce16r16_16) {
        genArithER(data, "+", "FLAGS_ADC16", "16", geteaa(op), "w", r16(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adce16r16_16_noflags) {
        genArithER_noflags(data, "+", "16", geteaa(op), "w", r16(op->reg), 1,"3");
    } else if (op->func==adce16r16_32) {
        genArithER(data, "+", "FLAGS_ADC16", "16", getEaa32(op), "w", r16(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adce16r16_32_noflags) {
        genArithER_noflags(data, "+", "16", getEaa32(op), "w", r16(op->reg), 1,"3");
    }
}
void OPCALL adcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL adcr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adce32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL adce32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adce32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL adce32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen211(struct GenData* data, struct Op* op) {
    if (op->func==adcr32r32) {
        genArithRR(data, "+", "FLAGS_ADC32", "32", r32(op->reg), r32(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adcr32r32_noflags) {
        genArithRR_noflags(data, "+", "32", r32(op->reg), r32(op->rm), 1,"1");
    } else if (op->func==adce32r32_16) {
        genArithER(data, "+", "FLAGS_ADC32", "32", geteaa(op), "d", r32(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adce32r32_16_noflags) {
        genArithER_noflags(data, "+", "32", geteaa(op), "d", r32(op->reg), 1,"3");
    } else if (op->func==adce32r32_32) {
        genArithER(data, "+", "FLAGS_ADC32", "32", getEaa32(op), "d", r32(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adce32r32_32_noflags) {
        genArithER_noflags(data, "+", "32", getEaa32(op), "d", r32(op->reg), 1,"3");
    }
}
void OPCALL adcr8r8(struct CPU* cpu, struct Op* op);
void OPCALL adcr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adcr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL adcr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adcr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL adcr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen012(struct GenData* data, struct Op* op) {
    if (op->func==adcr8r8) {
        genArithRR(data, "+", "FLAGS_ADC8", "8", r8(op->reg), r8(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adcr8r8_noflags) {
        genArithRR_noflags(data, "+", "8", r8(op->reg), r8(op->rm), 1,"1");
    } else if (op->func==adcr8e8_16) {
        genArithRE(data, "+", "FLAGS_ADC8", "8", geteaa(op), "b", r8(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adcr8e8_16_noflags) {
        genArithRE_noflags(data, "+", "8", geteaa(op), "b", r8(op->reg), 1,"2");
    } else if (op->func==adcr8e8_32) {
        genArithRE(data, "+", "FLAGS_ADC8", "8", getEaa32(op), "b", r8(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adcr8e8_32_noflags) {
        genArithRE_noflags(data, "+", "8", getEaa32(op), "b", r8(op->reg), 1,"2");
    }
}
void OPCALL adcr16r16(struct CPU* cpu, struct Op* op);
void OPCALL adcr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adcr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL adcr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adcr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL adcr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen013(struct GenData* data, struct Op* op) {
    if (op->func==adcr16r16) {
        genArithRR(data, "+", "FLAGS_ADC16", "16", r16(op->reg), r16(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adcr16r16_noflags) {
        genArithRR_noflags(data, "+", "16", r16(op->reg), r16(op->rm), 1,"1");
    } else if (op->func==adcr16e16_16) {
        genArithRE(data, "+", "FLAGS_ADC16", "16", geteaa(op), "w", r16(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adcr16e16_16_noflags) {
        genArithRE_noflags(data, "+", "16", geteaa(op), "w", r16(op->reg), 1,"2");
    } else if (op->func==adcr16e16_32) {
        genArithRE(data, "+", "FLAGS_ADC16", "16", getEaa32(op), "w", r16(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adcr16e16_32_noflags) {
        genArithRE_noflags(data, "+", "16", getEaa32(op), "w", r16(op->reg), 1,"2");
    }
}
void OPCALL adcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL adcr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adcr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL adcr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adcr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL adcr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen213(struct GenData* data, struct Op* op) {
    if (op->func==adcr32r32) {
        genArithRR(data, "+", "FLAGS_ADC32", "32", r32(op->reg), r32(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adcr32r32_noflags) {
        genArithRR_noflags(data, "+", "32", r32(op->reg), r32(op->rm), 1,"1");
    } else if (op->func==adcr32e32_16) {
        genArithRE(data, "+", "FLAGS_ADC32", "32", geteaa(op), "d", r32(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adcr32e32_16_noflags) {
        genArithRE_noflags(data, "+", "32", geteaa(op), "d", r32(op->reg), 1,"2");
    } else if (op->func==adcr32e32_32) {
        genArithRE(data, "+", "FLAGS_ADC32", "32", getEaa32(op), "d", r32(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adcr32e32_32_noflags) {
        genArithRE_noflags(data, "+", "32", getEaa32(op), "d", r32(op->reg), 1,"2");
    }
}
void OPCALL adc8_reg(struct CPU* cpu, struct Op* op);
void OPCALL adc8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen014(struct GenData* data, struct Op* op) {
    if (op->func==adc8_reg) {
        genArithR(data, "+", "FLAGS_ADC8", "8", r8(op->reg), op->imm, 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adc8_reg_noflags) {
        genArithR_noflags(data, "+", "8", r8(op->reg), op->imm, 1,"1");
    }
}
void OPCALL adc16_reg(struct CPU* cpu, struct Op* op);
void OPCALL adc16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen015(struct GenData* data, struct Op* op) {
    if (op->func==adc16_reg) {
        genArithR(data, "+", "FLAGS_ADC16", "16", r16(op->reg), op->imm, 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adc16_reg_noflags) {
        genArithR_noflags(data, "+", "16", r16(op->reg), op->imm, 1,"1");
    }
}
void OPCALL adc32_reg(struct CPU* cpu, struct Op* op);
void OPCALL adc32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen215(struct GenData* data, struct Op* op) {
    if (op->func==adc32_reg) {
        genArithR(data, "+", "FLAGS_ADC32", "32", r32(op->reg), op->imm, 1, 1,"1");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adc32_reg_noflags) {
        genArithR_noflags(data, "+", "32", r32(op->reg), op->imm, 1,"1");
    }
}
void OPCALL sbbr8r8(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbe8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL sbbe8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbe8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen018(struct GenData* data, struct Op* op) {
    if (op->func==sbbr8r8) {
        genArithRR(data, "-", "FLAGS_SBB8", "8", r8(op->reg), r8(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbbr8r8_noflags) {
        genArithRR_noflags(data, "-", "8", r8(op->reg), r8(op->rm), 1,"1");
    } else if (op->func==sbbe8r8_16) {
        genArithER(data, "-", "FLAGS_SBB8", "8", geteaa(op), "b", r8(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbbe8r8_16_noflags) {
        genArithER_noflags(data, "-", "8", geteaa(op), "b", r8(op->reg), 1,"3");
    } else if (op->func==sbbe8r8_32) {
        genArithER(data, "-", "FLAGS_SBB8", "8", getEaa32(op), "b", r8(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbbe8r8_32_noflags) {
        genArithER_noflags(data, "-", "8", getEaa32(op), "b", r8(op->reg), 1,"3");
    }
}
void OPCALL sbbr16r16(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbe16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL sbbe16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbe16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen019(struct GenData* data, struct Op* op) {
    if (op->func==sbbr16r16) {
        genArithRR(data, "-", "FLAGS_SBB16", "16", r16(op->reg), r16(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbbr16r16_noflags) {
        genArithRR_noflags(data, "-", "16", r16(op->reg), r16(op->rm), 1,"1");
    } else if (op->func==sbbe16r16_16) {
        genArithER(data, "-", "FLAGS_SBB16", "16", geteaa(op), "w", r16(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbbe16r16_16_noflags) {
        genArithER_noflags(data, "-", "16", geteaa(op), "w", r16(op->reg), 1,"3");
    } else if (op->func==sbbe16r16_32) {
        genArithER(data, "-", "FLAGS_SBB16", "16", getEaa32(op), "w", r16(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbbe16r16_32_noflags) {
        genArithER_noflags(data, "-", "16", getEaa32(op), "w", r16(op->reg), 1,"3");
    }
}
void OPCALL sbbr32r32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbe32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL sbbe32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbe32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbe32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen219(struct GenData* data, struct Op* op) {
    if (op->func==sbbr32r32) {
        genArithRR(data, "-", "FLAGS_SBB32", "32", r32(op->reg), r32(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbbr32r32_noflags) {
        genArithRR_noflags(data, "-", "32", r32(op->reg), r32(op->rm), 1,"1");
    } else if (op->func==sbbe32r32_16) {
        genArithER(data, "-", "FLAGS_SBB32", "32", geteaa(op), "d", r32(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbbe32r32_16_noflags) {
        genArithER_noflags(data, "-", "32", geteaa(op), "d", r32(op->reg), 1,"3");
    } else if (op->func==sbbe32r32_32) {
        genArithER(data, "-", "FLAGS_SBB32", "32", getEaa32(op), "d", r32(op->reg), 1, 1,"3");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbbe32r32_32_noflags) {
        genArithER_noflags(data, "-", "32", getEaa32(op), "d", r32(op->reg), 1,"3");
    }
}
void OPCALL sbbr8r8(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen01a(struct GenData* data, struct Op* op) {
    if (op->func==sbbr8r8) {
        genArithRR(data, "-", "FLAGS_SBB8", "8", r8(op->reg), r8(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbbr8r8_noflags) {
        genArithRR_noflags(data, "-", "8", r8(op->reg), r8(op->rm), 1,"1");
    } else if (op->func==sbbr8e8_16) {
        genArithRE(data, "-", "FLAGS_SBB8", "8", geteaa(op), "b", r8(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbbr8e8_16_noflags) {
        genArithRE_noflags(data, "-", "8", geteaa(op), "b", r8(op->reg), 1,"2");
    } else if (op->func==sbbr8e8_32) {
        genArithRE(data, "-", "FLAGS_SBB8", "8", getEaa32(op), "b", r8(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbbr8e8_32_noflags) {
        genArithRE_noflags(data, "-", "8", getEaa32(op), "b", r8(op->reg), 1,"2");
    }
}
void OPCALL sbbr16r16(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen01b(struct GenData* data, struct Op* op) {
    if (op->func==sbbr16r16) {
        genArithRR(data, "-", "FLAGS_SBB16", "16", r16(op->reg), r16(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbbr16r16_noflags) {
        genArithRR_noflags(data, "-", "16", r16(op->reg), r16(op->rm), 1,"1");
    } else if (op->func==sbbr16e16_16) {
        genArithRE(data, "-", "FLAGS_SBB16", "16", geteaa(op), "w", r16(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbbr16e16_16_noflags) {
        genArithRE_noflags(data, "-", "16", geteaa(op), "w", r16(op->reg), 1,"2");
    } else if (op->func==sbbr16e16_32) {
        genArithRE(data, "-", "FLAGS_SBB16", "16", getEaa32(op), "w", r16(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbbr16e16_32_noflags) {
        genArithRE_noflags(data, "-", "16", getEaa32(op), "w", r16(op->reg), 1,"2");
    }
}
void OPCALL sbbr32r32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL sbbr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen21b(struct GenData* data, struct Op* op) {
    if (op->func==sbbr32r32) {
        genArithRR(data, "-", "FLAGS_SBB32", "32", r32(op->reg), r32(op->rm), 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbbr32r32_noflags) {
        genArithRR_noflags(data, "-", "32", r32(op->reg), r32(op->rm), 1,"1");
    } else if (op->func==sbbr32e32_16) {
        genArithRE(data, "-", "FLAGS_SBB32", "32", geteaa(op), "d", r32(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbbr32e32_16_noflags) {
        genArithRE_noflags(data, "-", "32", geteaa(op), "d", r32(op->reg), 1,"2");
    } else if (op->func==sbbr32e32_32) {
        genArithRE(data, "-", "FLAGS_SBB32", "32", getEaa32(op), "d", r32(op->reg), 1, 1,"2");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbbr32e32_32_noflags) {
        genArithRE_noflags(data, "-", "32", getEaa32(op), "d", r32(op->reg), 1,"2");
    }
}
void OPCALL sbb8_reg(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen01c(struct GenData* data, struct Op* op) {
    if (op->func==sbb8_reg) {
        genArithR(data, "-", "FLAGS_SBB8", "8", r8(op->reg), op->imm, 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbb8_reg_noflags) {
        genArithR_noflags(data, "-", "8", r8(op->reg), op->imm, 1,"1");
    }
}
void OPCALL sbb16_reg(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen01d(struct GenData* data, struct Op* op) {
    if (op->func==sbb16_reg) {
        genArithR(data, "-", "FLAGS_SBB16", "16", r16(op->reg), op->imm, 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbb16_reg_noflags) {
        genArithR_noflags(data, "-", "16", r16(op->reg), op->imm, 1,"1");
    }
}
void OPCALL sbb32_reg(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen21d(struct GenData* data, struct Op* op) {
    if (op->func==sbb32_reg) {
        genArithR(data, "-", "FLAGS_SBB32", "32", r32(op->reg), op->imm, 1, 1,"1");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbb32_reg_noflags) {
        genArithR_noflags(data, "-", "32", r32(op->reg), op->imm, 1,"1");
    }
}
void OPCALL andr8r8(struct CPU* cpu, struct Op* op);
void OPCALL andr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ande8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL ande8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ande8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL ande8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen020(struct GenData* data, struct Op* op) {
    if (op->func==andr8r8) {
        genArithRR(data, "&", "FLAGS_AND8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==andr8r8_noflags) {
        genArithRR_noflags(data, "&", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==ande8r8_16) {
        genArithER(data, "&", "FLAGS_AND8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==ande8r8_16_noflags) {
        genArithER_noflags(data, "&", "8", geteaa(op), "b", r8(op->reg), 0,"3");
    } else if (op->func==ande8r8_32) {
        genArithER(data, "&", "FLAGS_AND8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==ande8r8_32_noflags) {
        genArithER_noflags(data, "&", "8", getEaa32(op), "b", r8(op->reg), 0,"3");
    }
}
void OPCALL andr16r16(struct CPU* cpu, struct Op* op);
void OPCALL andr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ande16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL ande16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ande16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL ande16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen021(struct GenData* data, struct Op* op) {
    if (op->func==andr16r16) {
        genArithRR(data, "&", "FLAGS_AND16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==andr16r16_noflags) {
        genArithRR_noflags(data, "&", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==ande16r16_16) {
        genArithER(data, "&", "FLAGS_AND16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==ande16r16_16_noflags) {
        genArithER_noflags(data, "&", "16", geteaa(op), "w", r16(op->reg), 0,"3");
    } else if (op->func==ande16r16_32) {
        genArithER(data, "&", "FLAGS_AND16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==ande16r16_32_noflags) {
        genArithER_noflags(data, "&", "16", getEaa32(op), "w", r16(op->reg), 0,"3");
    }
}
void OPCALL andr32r32(struct CPU* cpu, struct Op* op);
void OPCALL andr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ande32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL ande32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ande32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL ande32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen221(struct GenData* data, struct Op* op) {
    if (op->func==andr32r32) {
        genArithRR(data, "&", "FLAGS_AND32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==andr32r32_noflags) {
        genArithRR_noflags(data, "&", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==ande32r32_16) {
        genArithER(data, "&", "FLAGS_AND32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==ande32r32_16_noflags) {
        genArithER_noflags(data, "&", "32", geteaa(op), "d", r32(op->reg), 0,"3");
    } else if (op->func==ande32r32_32) {
        genArithER(data, "&", "FLAGS_AND32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==ande32r32_32_noflags) {
        genArithER_noflags(data, "&", "32", getEaa32(op), "d", r32(op->reg), 0,"3");
    }
}
void OPCALL andr8r8(struct CPU* cpu, struct Op* op);
void OPCALL andr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL andr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL andr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL andr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL andr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen022(struct GenData* data, struct Op* op) {
    if (op->func==andr8r8) {
        genArithRR(data, "&", "FLAGS_AND8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==andr8r8_noflags) {
        genArithRR_noflags(data, "&", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==andr8e8_16) {
        genArithRE(data, "&", "FLAGS_AND8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==andr8e8_16_noflags) {
        genArithRE_noflags(data, "&", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==andr8e8_32) {
        genArithRE(data, "&", "FLAGS_AND8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==andr8e8_32_noflags) {
        genArithRE_noflags(data, "&", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL andr16r16(struct CPU* cpu, struct Op* op);
void OPCALL andr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL andr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL andr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL andr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL andr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen023(struct GenData* data, struct Op* op) {
    if (op->func==andr16r16) {
        genArithRR(data, "&", "FLAGS_AND16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==andr16r16_noflags) {
        genArithRR_noflags(data, "&", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==andr16e16_16) {
        genArithRE(data, "&", "FLAGS_AND16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==andr16e16_16_noflags) {
        genArithRE_noflags(data, "&", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==andr16e16_32) {
        genArithRE(data, "&", "FLAGS_AND16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==andr16e16_32_noflags) {
        genArithRE_noflags(data, "&", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL andr32r32(struct CPU* cpu, struct Op* op);
void OPCALL andr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL andr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL andr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL andr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL andr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen223(struct GenData* data, struct Op* op) {
    if (op->func==andr32r32) {
        genArithRR(data, "&", "FLAGS_AND32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==andr32r32_noflags) {
        genArithRR_noflags(data, "&", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==andr32e32_16) {
        genArithRE(data, "&", "FLAGS_AND32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==andr32e32_16_noflags) {
        genArithRE_noflags(data, "&", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==andr32e32_32) {
        genArithRE(data, "&", "FLAGS_AND32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==andr32e32_32_noflags) {
        genArithRE_noflags(data, "&", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL and8_reg(struct CPU* cpu, struct Op* op);
void OPCALL and8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen024(struct GenData* data, struct Op* op) {
    if (op->func==and8_reg) {
        genArithR(data, "&", "FLAGS_AND8", "8", r8(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==and8_reg_noflags) {
        genArithR_noflags(data, "&", "8", r8(op->reg), op->imm, 0,"1");
    }
}
void OPCALL and16_reg(struct CPU* cpu, struct Op* op);
void OPCALL and16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen025(struct GenData* data, struct Op* op) {
    if (op->func==and16_reg) {
        genArithR(data, "&", "FLAGS_AND16", "16", r16(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==and16_reg_noflags) {
        genArithR_noflags(data, "&", "16", r16(op->reg), op->imm, 0,"1");
    }
}
void OPCALL and32_reg(struct CPU* cpu, struct Op* op);
void OPCALL and32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen225(struct GenData* data, struct Op* op) {
    if (op->func==and32_reg) {
        genArithR(data, "&", "FLAGS_AND32", "32", r32(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==and32_reg_noflags) {
        genArithR_noflags(data, "&", "32", r32(op->reg), op->imm, 0,"1");
    }
}
void OPCALL subr8r8(struct CPU* cpu, struct Op* op);
void OPCALL subr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sube8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL sube8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sube8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL sube8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen028(struct GenData* data, struct Op* op) {
    if (op->func==subr8r8) {
        genArithRR(data, "-", "FLAGS_SUB8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==subr8r8_noflags) {
        genArithRR_noflags(data, "-", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==sube8r8_16) {
        genArithER(data, "-", "FLAGS_SUB8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==sube8r8_16_noflags) {
        genArithER_noflags(data, "-", "8", geteaa(op), "b", r8(op->reg), 0,"3");
    } else if (op->func==sube8r8_32) {
        genArithER(data, "-", "FLAGS_SUB8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==sube8r8_32_noflags) {
        genArithER_noflags(data, "-", "8", getEaa32(op), "b", r8(op->reg), 0,"3");
    }
}
void OPCALL subr16r16(struct CPU* cpu, struct Op* op);
void OPCALL subr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sube16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL sube16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sube16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL sube16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen029(struct GenData* data, struct Op* op) {
    if (op->func==subr16r16) {
        genArithRR(data, "-", "FLAGS_SUB16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==subr16r16_noflags) {
        genArithRR_noflags(data, "-", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==sube16r16_16) {
        genArithER(data, "-", "FLAGS_SUB16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==sube16r16_16_noflags) {
        genArithER_noflags(data, "-", "16", geteaa(op), "w", r16(op->reg), 0,"3");
    } else if (op->func==sube16r16_32) {
        genArithER(data, "-", "FLAGS_SUB16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==sube16r16_32_noflags) {
        genArithER_noflags(data, "-", "16", getEaa32(op), "w", r16(op->reg), 0,"3");
    }
}
void OPCALL subr32r32(struct CPU* cpu, struct Op* op);
void OPCALL subr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sube32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL sube32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sube32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL sube32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen229(struct GenData* data, struct Op* op) {
    if (op->func==subr32r32) {
        genArithRR(data, "-", "FLAGS_SUB32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==subr32r32_noflags) {
        genArithRR_noflags(data, "-", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==sube32r32_16) {
        genArithER(data, "-", "FLAGS_SUB32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==sube32r32_16_noflags) {
        genArithER_noflags(data, "-", "32", geteaa(op), "d", r32(op->reg), 0,"3");
    } else if (op->func==sube32r32_32) {
        genArithER(data, "-", "FLAGS_SUB32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"3");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==sube32r32_32_noflags) {
        genArithER_noflags(data, "-", "32", getEaa32(op), "d", r32(op->reg), 0,"3");
    }
}
void OPCALL subr8r8(struct CPU* cpu, struct Op* op);
void OPCALL subr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL subr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL subr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL subr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL subr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen02a(struct GenData* data, struct Op* op) {
    if (op->func==subr8r8) {
        genArithRR(data, "-", "FLAGS_SUB8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==subr8r8_noflags) {
        genArithRR_noflags(data, "-", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==subr8e8_16) {
        genArithRE(data, "-", "FLAGS_SUB8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==subr8e8_16_noflags) {
        genArithRE_noflags(data, "-", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==subr8e8_32) {
        genArithRE(data, "-", "FLAGS_SUB8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==subr8e8_32_noflags) {
        genArithRE_noflags(data, "-", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL subr16r16(struct CPU* cpu, struct Op* op);
void OPCALL subr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL subr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL subr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL subr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL subr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen02b(struct GenData* data, struct Op* op) {
    if (op->func==subr16r16) {
        genArithRR(data, "-", "FLAGS_SUB16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==subr16r16_noflags) {
        genArithRR_noflags(data, "-", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==subr16e16_16) {
        genArithRE(data, "-", "FLAGS_SUB16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==subr16e16_16_noflags) {
        genArithRE_noflags(data, "-", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==subr16e16_32) {
        genArithRE(data, "-", "FLAGS_SUB16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==subr16e16_32_noflags) {
        genArithRE_noflags(data, "-", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL subr32r32(struct CPU* cpu, struct Op* op);
void OPCALL subr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL subr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL subr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL subr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL subr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen22b(struct GenData* data, struct Op* op) {
    if (op->func==subr32r32) {
        genArithRR(data, "-", "FLAGS_SUB32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==subr32r32_noflags) {
        genArithRR_noflags(data, "-", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==subr32e32_16) {
        genArithRE(data, "-", "FLAGS_SUB32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==subr32e32_16_noflags) {
        genArithRE_noflags(data, "-", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==subr32e32_32) {
        genArithRE(data, "-", "FLAGS_SUB32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==subr32e32_32_noflags) {
        genArithRE_noflags(data, "-", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL sub8_reg(struct CPU* cpu, struct Op* op);
void OPCALL sub8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen02c(struct GenData* data, struct Op* op) {
    if (op->func==sub8_reg) {
        genArithR(data, "-", "FLAGS_SUB8", "8", r8(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==sub8_reg_noflags) {
        genArithR_noflags(data, "-", "8", r8(op->reg), op->imm, 0,"1");
    }
}
void OPCALL sub16_reg(struct CPU* cpu, struct Op* op);
void OPCALL sub16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen02d(struct GenData* data, struct Op* op) {
    if (op->func==sub16_reg) {
        genArithR(data, "-", "FLAGS_SUB16", "16", r16(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==sub16_reg_noflags) {
        genArithR_noflags(data, "-", "16", r16(op->reg), op->imm, 0,"1");
    }
}
void OPCALL sub32_reg(struct CPU* cpu, struct Op* op);
void OPCALL sub32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen22d(struct GenData* data, struct Op* op) {
    if (op->func==sub32_reg) {
        genArithR(data, "-", "FLAGS_SUB32", "32", r32(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==sub32_reg_noflags) {
        genArithR_noflags(data, "-", "32", r32(op->reg), op->imm, 0,"1");
    }
}
void OPCALL xorr8r8(struct CPU* cpu, struct Op* op);
void OPCALL xorr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xore8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL xore8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xore8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL xore8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen030(struct GenData* data, struct Op* op) {
    if (op->func==xorr8r8) {
        genArithRR(data, "^", "FLAGS_XOR8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xorr8r8_noflags) {
        genArithRR_noflags(data, "^", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==xore8r8_16) {
        genArithER(data, "^", "FLAGS_XOR8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xore8r8_16_noflags) {
        genArithER_noflags(data, "^", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==xore8r8_32) {
        genArithER(data, "^", "FLAGS_XOR8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xore8r8_32_noflags) {
        genArithER_noflags(data, "^", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL xorr16r16(struct CPU* cpu, struct Op* op);
void OPCALL xorr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xore16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL xore16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xore16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL xore16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen031(struct GenData* data, struct Op* op) {
    if (op->func==xorr16r16) {
        genArithRR(data, "^", "FLAGS_XOR16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xorr16r16_noflags) {
        genArithRR_noflags(data, "^", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==xore16r16_16) {
        genArithER(data, "^", "FLAGS_XOR16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xore16r16_16_noflags) {
        genArithER_noflags(data, "^", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==xore16r16_32) {
        genArithER(data, "^", "FLAGS_XOR16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xore16r16_32_noflags) {
        genArithER_noflags(data, "^", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL xorr32r32(struct CPU* cpu, struct Op* op);
void OPCALL xorr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xore32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL xore32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xore32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL xore32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen231(struct GenData* data, struct Op* op) {
    if (op->func==xorr32r32) {
        genArithRR(data, "^", "FLAGS_XOR32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xorr32r32_noflags) {
        genArithRR_noflags(data, "^", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==xore32r32_16) {
        genArithER(data, "^", "FLAGS_XOR32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xore32r32_16_noflags) {
        genArithER_noflags(data, "^", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==xore32r32_32) {
        genArithER(data, "^", "FLAGS_XOR32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xore32r32_32_noflags) {
        genArithER_noflags(data, "^", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL xorr8r8(struct CPU* cpu, struct Op* op);
void OPCALL xorr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xorr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL xorr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xorr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL xorr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen032(struct GenData* data, struct Op* op) {
    if (op->func==xorr8r8) {
        genArithRR(data, "^", "FLAGS_XOR8", "8", r8(op->reg), r8(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xorr8r8_noflags) {
        genArithRR_noflags(data, "^", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==xorr8e8_16) {
        genArithRE(data, "^", "FLAGS_XOR8", "8", geteaa(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xorr8e8_16_noflags) {
        genArithRE_noflags(data, "^", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==xorr8e8_32) {
        genArithRE(data, "^", "FLAGS_XOR8", "8", getEaa32(op), "b", r8(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xorr8e8_32_noflags) {
        genArithRE_noflags(data, "^", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL xorr16r16(struct CPU* cpu, struct Op* op);
void OPCALL xorr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xorr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL xorr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xorr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL xorr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen033(struct GenData* data, struct Op* op) {
    if (op->func==xorr16r16) {
        genArithRR(data, "^", "FLAGS_XOR16", "16", r16(op->reg), r16(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xorr16r16_noflags) {
        genArithRR_noflags(data, "^", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==xorr16e16_16) {
        genArithRE(data, "^", "FLAGS_XOR16", "16", geteaa(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xorr16e16_16_noflags) {
        genArithRE_noflags(data, "^", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==xorr16e16_32) {
        genArithRE(data, "^", "FLAGS_XOR16", "16", getEaa32(op), "w", r16(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xorr16e16_32_noflags) {
        genArithRE_noflags(data, "^", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL xorr32r32(struct CPU* cpu, struct Op* op);
void OPCALL xorr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xorr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL xorr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xorr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL xorr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen233(struct GenData* data, struct Op* op) {
    if (op->func==xorr32r32) {
        genArithRR(data, "^", "FLAGS_XOR32", "32", r32(op->reg), r32(op->rm), 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xorr32r32_noflags) {
        genArithRR_noflags(data, "^", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==xorr32e32_16) {
        genArithRE(data, "^", "FLAGS_XOR32", "32", geteaa(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xorr32e32_16_noflags) {
        genArithRE_noflags(data, "^", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==xorr32e32_32) {
        genArithRE(data, "^", "FLAGS_XOR32", "32", getEaa32(op), "d", r32(op->reg), 1, 0,"2");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xorr32e32_32_noflags) {
        genArithRE_noflags(data, "^", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL xor8_reg(struct CPU* cpu, struct Op* op);
void OPCALL xor8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen034(struct GenData* data, struct Op* op) {
    if (op->func==xor8_reg) {
        genArithR(data, "^", "FLAGS_XOR8", "8", r8(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xor8_reg_noflags) {
        genArithR_noflags(data, "^", "8", r8(op->reg), op->imm, 0,"1");
    }
}
void OPCALL xor16_reg(struct CPU* cpu, struct Op* op);
void OPCALL xor16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen035(struct GenData* data, struct Op* op) {
    if (op->func==xor16_reg) {
        genArithR(data, "^", "FLAGS_XOR16", "16", r16(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xor16_reg_noflags) {
        genArithR_noflags(data, "^", "16", r16(op->reg), op->imm, 0,"1");
    }
}
void OPCALL xor32_reg(struct CPU* cpu, struct Op* op);
void OPCALL xor32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen235(struct GenData* data, struct Op* op) {
    if (op->func==xor32_reg) {
        genArithR(data, "^", "FLAGS_XOR32", "32", r32(op->reg), op->imm, 1, 0,"1");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xor32_reg_noflags) {
        genArithR_noflags(data, "^", "32", r32(op->reg), op->imm, 0,"1");
    }
}
void OPCALL cmpr8r8(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpe8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe8r8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpe8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpe8r8_32_noflags(struct CPU* cpu, struct Op* op);
void gen038(struct GenData* data, struct Op* op) {
    if (op->func==cmpr8r8) {
        genArithRR(data, "-", "FLAGS_CMP8", "8", r8(op->reg), r8(op->rm), 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmpr8r8_noflags) {
        genArithRR_noflags(data, "-", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==cmpe8r8_16) {
        genArithER(data, "-", "FLAGS_CMP8", "8", geteaa(op), "b", r8(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmpe8r8_16_noflags) {
        genArithER_noflags(data, "-", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==cmpe8r8_32) {
        genArithER(data, "-", "FLAGS_CMP8", "8", getEaa32(op), "b", r8(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmpe8r8_32_noflags) {
        genArithER_noflags(data, "-", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL cmpr16r16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpe16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe16r16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpe16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpe16r16_32_noflags(struct CPU* cpu, struct Op* op);
void gen039(struct GenData* data, struct Op* op) {
    if (op->func==cmpr16r16) {
        genArithRR(data, "-", "FLAGS_CMP16", "16", r16(op->reg), r16(op->rm), 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmpr16r16_noflags) {
        genArithRR_noflags(data, "-", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==cmpe16r16_16) {
        genArithER(data, "-", "FLAGS_CMP16", "16", geteaa(op), "w", r16(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmpe16r16_16_noflags) {
        genArithER_noflags(data, "-", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==cmpe16r16_32) {
        genArithER(data, "-", "FLAGS_CMP16", "16", getEaa32(op), "w", r16(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmpe16r16_32_noflags) {
        genArithER_noflags(data, "-", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL cmpr32r32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpe32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpe32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpe32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen239(struct GenData* data, struct Op* op) {
    if (op->func==cmpr32r32) {
        genArithRR(data, "-", "FLAGS_CMP32", "32", r32(op->reg), r32(op->rm), 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmpr32r32_noflags) {
        genArithRR_noflags(data, "-", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==cmpe32r32_16) {
        genArithER(data, "-", "FLAGS_CMP32", "32", geteaa(op), "d", r32(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmpe32r32_16_noflags) {
        genArithER_noflags(data, "-", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==cmpe32r32_32) {
        genArithER(data, "-", "FLAGS_CMP32", "32", getEaa32(op), "d", r32(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmpe32r32_32_noflags) {
        genArithER_noflags(data, "-", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL cmpr8r8(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8r8_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8e8_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8e8_32_noflags(struct CPU* cpu, struct Op* op);
void gen03a(struct GenData* data, struct Op* op) {
    if (op->func==cmpr8r8) {
        genArithRR(data, "-", "FLAGS_CMP8", "8", r8(op->reg), r8(op->rm), 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmpr8r8_noflags) {
        genArithRR_noflags(data, "-", "8", r8(op->reg), r8(op->rm), 0,"1");
    } else if (op->func==cmpr8e8_16) {
        genArithRE(data, "-", "FLAGS_CMP8", "8", geteaa(op), "b", r8(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmpr8e8_16_noflags) {
        genArithRE_noflags(data, "-", "8", geteaa(op), "b", r8(op->reg), 0,"2");
    } else if (op->func==cmpr8e8_32) {
        genArithRE(data, "-", "FLAGS_CMP8", "8", getEaa32(op), "b", r8(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmpr8e8_32_noflags) {
        genArithRE_noflags(data, "-", "8", getEaa32(op), "b", r8(op->reg), 0,"2");
    }
}
void OPCALL cmpr16r16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16r16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16e16_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16e16_32_noflags(struct CPU* cpu, struct Op* op);
void gen03b(struct GenData* data, struct Op* op) {
    if (op->func==cmpr16r16) {
        genArithRR(data, "-", "FLAGS_CMP16", "16", r16(op->reg), r16(op->rm), 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmpr16r16_noflags) {
        genArithRR_noflags(data, "-", "16", r16(op->reg), r16(op->rm), 0,"1");
    } else if (op->func==cmpr16e16_16) {
        genArithRE(data, "-", "FLAGS_CMP16", "16", geteaa(op), "w", r16(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmpr16e16_16_noflags) {
        genArithRE_noflags(data, "-", "16", geteaa(op), "w", r16(op->reg), 0,"2");
    } else if (op->func==cmpr16e16_32) {
        genArithRE(data, "-", "FLAGS_CMP16", "16", getEaa32(op), "w", r16(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmpr16e16_32_noflags) {
        genArithRE_noflags(data, "-", "16", getEaa32(op), "w", r16(op->reg), 0,"2");
    }
}
void OPCALL cmpr32r32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen23b(struct GenData* data, struct Op* op) {
    if (op->func==cmpr32r32) {
        genArithRR(data, "-", "FLAGS_CMP32", "32", r32(op->reg), r32(op->rm), 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmpr32r32_noflags) {
        genArithRR_noflags(data, "-", "32", r32(op->reg), r32(op->rm), 0,"1");
    } else if (op->func==cmpr32e32_16) {
        genArithRE(data, "-", "FLAGS_CMP32", "32", geteaa(op), "d", r32(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmpr32e32_16_noflags) {
        genArithRE_noflags(data, "-", "32", geteaa(op), "d", r32(op->reg), 0,"2");
    } else if (op->func==cmpr32e32_32) {
        genArithRE(data, "-", "FLAGS_CMP32", "32", getEaa32(op), "d", r32(op->reg), 0, 0,"2");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmpr32e32_32_noflags) {
        genArithRE_noflags(data, "-", "32", getEaa32(op), "d", r32(op->reg), 0,"2");
    }
}
void OPCALL cmp8_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmp8_reg_noflags(struct CPU* cpu, struct Op* op);
void gen03c(struct GenData* data, struct Op* op) {
    if (op->func==cmp8_reg) {
        genArithR(data, "-", "FLAGS_CMP8", "8", r8(op->reg), op->imm, 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmp8_reg_noflags) {
        genArithR_noflags(data, "-", "8", r8(op->reg), op->imm, 0,"1");
    }
}
void OPCALL cmp16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmp16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen03d(struct GenData* data, struct Op* op) {
    if (op->func==cmp16_reg) {
        genArithR(data, "-", "FLAGS_CMP16", "16", r16(op->reg), op->imm, 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmp16_reg_noflags) {
        genArithR_noflags(data, "-", "16", r16(op->reg), op->imm, 0,"1");
    }
}
void OPCALL cmp32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmp32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen23d(struct GenData* data, struct Op* op) {
    if (op->func==cmp32_reg) {
        genArithR(data, "-", "FLAGS_CMP32", "32", r32(op->reg), op->imm, 0, 0,"1");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmp32_reg_noflags) {
        genArithR_noflags(data, "-", "32", r32(op->reg), op->imm, 0,"1");
    }
}
void gen006(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, cpu->segValue[ES]);CYCLES(1);");
}
void gen206(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, cpu->segValue[ES]);CYCLES(1);");
}
void gen007(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[ES] = pop16(cpu); cpu->segAddress[ES] = cpu->ldt[cpu->segValue[ES] >> 3].base_addr;CYCLES(3);");
}
void gen207(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[ES] = pop32(cpu); cpu->segAddress[ES] = cpu->ldt[cpu->segValue[ES] >> 3].base_addr;CYCLES(3);");
}
void gen00e(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, cpu->segValue[CS]);CYCLES(1);");
}
void gen20e(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, cpu->segValue[CS]);CYCLES(1);");
}
void gen016(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, cpu->segValue[SS]);CYCLES(1);");
}
void gen216(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, cpu->segValue[SS]);CYCLES(1);");
}
void gen017(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[SS] = pop16(cpu); cpu->segAddress[SS] = cpu->ldt[cpu->segValue[SS] >> 3].base_addr;CYCLES(3);");
}
void gen217(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[SS] = pop32(cpu); cpu->segAddress[SS] = cpu->ldt[cpu->segValue[SS] >> 3].base_addr;CYCLES(3);");
}
void gen01e(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, cpu->segValue[DS]);CYCLES(1);");
}
void gen21e(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, cpu->segValue[DS]);CYCLES(1);");
}
void gen01f(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[DS] = pop16(cpu); cpu->segAddress[DS] = cpu->ldt[cpu->segValue[DS] >> 3].base_addr;CYCLES(3);");
}
void gen21f(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[DS] = pop32(cpu); cpu->segAddress[DS] = cpu->ldt[cpu->segValue[DS] >> 3].base_addr;CYCLES(3);");
}
void gen1a0(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, cpu->segValue[FS]);CYCLES(1);");
}
void gen3a0(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, cpu->segValue[FS]);CYCLES(1);");
}
void gen1a1(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[FS] = pop16(cpu); cpu->segAddress[FS] = cpu->ldt[cpu->segValue[FS] >> 3].base_addr;CYCLES(3);");
}
void gen3a1(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[FS] = pop32(cpu); cpu->segAddress[FS] = cpu->ldt[cpu->segValue[FS] >> 3].base_addr;CYCLES(3);");
}
void gen1a8(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, cpu->segValue[GS]);CYCLES(1);");
}
void gen3a8(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, cpu->segValue[GS]);CYCLES(1);");
}
void gen1a9(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[GS] = pop16(cpu); cpu->segAddress[GS] = cpu->ldt[cpu->segValue[GS] >> 3].base_addr;CYCLES(3);");
}
void gen3a9(struct GenData* data, struct Op* op) {
    out(data, "cpu->segValue[GS] = pop32(cpu); cpu->segAddress[GS] = cpu->ldt[cpu->segValue[GS] >> 3].base_addr;CYCLES(3);");
}
void OPCALL cmovO_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovO_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovO_16_mem32(struct CPU* cpu, struct Op* op);
void gen140(struct GenData* data, struct Op* op) {
    if (op->func == cmovO_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovO_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovO_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen140");
    }
}
void OPCALL cmovO_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovO_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovO_32_mem32(struct CPU* cpu, struct Op* op);
void gen340(struct GenData* data, struct Op* op) {
    if (op->func == cmovO_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovO_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovO_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen340");
    }
}
void OPCALL cmovNO_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNO_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNO_16_mem32(struct CPU* cpu, struct Op* op);
void gen141(struct GenData* data, struct Op* op) {
    if (op->func == cmovNO_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNO_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNO_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen141");
    }
}
void OPCALL cmovNO_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNO_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNO_32_mem32(struct CPU* cpu, struct Op* op);
void gen341(struct GenData* data, struct Op* op) {
    if (op->func == cmovNO_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNO_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNO_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen341");
    }
}
void OPCALL cmovB_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovB_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovB_16_mem32(struct CPU* cpu, struct Op* op);
void gen142(struct GenData* data, struct Op* op) {
    if (op->func == cmovB_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovB_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovB_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen142");
    }
}
void OPCALL cmovB_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovB_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovB_32_mem32(struct CPU* cpu, struct Op* op);
void gen342(struct GenData* data, struct Op* op) {
    if (op->func == cmovB_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovB_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovB_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen342");
    }
}
void OPCALL cmovNB_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNB_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNB_16_mem32(struct CPU* cpu, struct Op* op);
void gen143(struct GenData* data, struct Op* op) {
    if (op->func == cmovNB_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNB_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNB_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen143");
    }
}
void OPCALL cmovNB_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNB_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNB_32_mem32(struct CPU* cpu, struct Op* op);
void gen343(struct GenData* data, struct Op* op) {
    if (op->func == cmovNB_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNB_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNB_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen343");
    }
}
void OPCALL cmovZ_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovZ_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovZ_16_mem32(struct CPU* cpu, struct Op* op);
void gen144(struct GenData* data, struct Op* op) {
    if (op->func == cmovZ_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovZ_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovZ_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen144");
    }
}
void OPCALL cmovZ_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovZ_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovZ_32_mem32(struct CPU* cpu, struct Op* op);
void gen344(struct GenData* data, struct Op* op) {
    if (op->func == cmovZ_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovZ_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovZ_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen344");
    }
}
void OPCALL cmovNZ_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNZ_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNZ_16_mem32(struct CPU* cpu, struct Op* op);
void gen145(struct GenData* data, struct Op* op) {
    if (op->func == cmovNZ_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNZ_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNZ_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen145");
    }
}
void OPCALL cmovNZ_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNZ_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNZ_32_mem32(struct CPU* cpu, struct Op* op);
void gen345(struct GenData* data, struct Op* op) {
    if (op->func == cmovNZ_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNZ_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNZ_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen345");
    }
}
void OPCALL cmovBE_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovBE_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovBE_16_mem32(struct CPU* cpu, struct Op* op);
void gen146(struct GenData* data, struct Op* op) {
    if (op->func == cmovBE_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovBE_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovBE_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen146");
    }
}
void OPCALL cmovBE_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovBE_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovBE_32_mem32(struct CPU* cpu, struct Op* op);
void gen346(struct GenData* data, struct Op* op) {
    if (op->func == cmovBE_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovBE_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovBE_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen346");
    }
}
void OPCALL cmovNBE_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNBE_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNBE_16_mem32(struct CPU* cpu, struct Op* op);
void gen147(struct GenData* data, struct Op* op) {
    if (op->func == cmovNBE_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNBE_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNBE_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen147");
    }
}
void OPCALL cmovNBE_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNBE_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNBE_32_mem32(struct CPU* cpu, struct Op* op);
void gen347(struct GenData* data, struct Op* op) {
    if (op->func == cmovNBE_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNBE_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNBE_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen347");
    }
}
void OPCALL cmovS_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovS_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovS_16_mem32(struct CPU* cpu, struct Op* op);
void gen148(struct GenData* data, struct Op* op) {
    if (op->func == cmovS_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovS_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovS_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen148");
    }
}
void OPCALL cmovS_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovS_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovS_32_mem32(struct CPU* cpu, struct Op* op);
void gen348(struct GenData* data, struct Op* op) {
    if (op->func == cmovS_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovS_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovS_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen348");
    }
}
void OPCALL cmovNS_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNS_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNS_16_mem32(struct CPU* cpu, struct Op* op);
void gen149(struct GenData* data, struct Op* op) {
    if (op->func == cmovNS_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNS_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNS_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen149");
    }
}
void OPCALL cmovNS_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNS_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNS_32_mem32(struct CPU* cpu, struct Op* op);
void gen349(struct GenData* data, struct Op* op) {
    if (op->func == cmovNS_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNS_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNS_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen349");
    }
}
void OPCALL cmovP_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovP_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovP_16_mem32(struct CPU* cpu, struct Op* op);
void gen14a(struct GenData* data, struct Op* op) {
    if (op->func == cmovP_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovP_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovP_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen14a");
    }
}
void OPCALL cmovP_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovP_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovP_32_mem32(struct CPU* cpu, struct Op* op);
void gen34a(struct GenData* data, struct Op* op) {
    if (op->func == cmovP_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovP_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovP_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen34a");
    }
}
void OPCALL cmovNP_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNP_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNP_16_mem32(struct CPU* cpu, struct Op* op);
void gen14b(struct GenData* data, struct Op* op) {
    if (op->func == cmovNP_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNP_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNP_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen14b");
    }
}
void OPCALL cmovNP_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNP_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNP_32_mem32(struct CPU* cpu, struct Op* op);
void gen34b(struct GenData* data, struct Op* op) {
    if (op->func == cmovNP_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNP_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNP_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen34b");
    }
}
void OPCALL cmovL_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovL_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovL_16_mem32(struct CPU* cpu, struct Op* op);
void gen14c(struct GenData* data, struct Op* op) {
    if (op->func == cmovL_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovL_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovL_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen14c");
    }
}
void OPCALL cmovL_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovL_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovL_32_mem32(struct CPU* cpu, struct Op* op);
void gen34c(struct GenData* data, struct Op* op) {
    if (op->func == cmovL_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovL_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovL_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen34c");
    }
}
void OPCALL cmovNL_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNL_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNL_16_mem32(struct CPU* cpu, struct Op* op);
void gen14d(struct GenData* data, struct Op* op) {
    if (op->func == cmovNL_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNL_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNL_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen14d");
    }
}
void OPCALL cmovNL_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNL_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNL_32_mem32(struct CPU* cpu, struct Op* op);
void gen34d(struct GenData* data, struct Op* op) {
    if (op->func == cmovNL_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNL_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNL_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen34d");
    }
}
void OPCALL cmovLE_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovLE_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovLE_16_mem32(struct CPU* cpu, struct Op* op);
void gen14e(struct GenData* data, struct Op* op) {
    if (op->func == cmovLE_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovLE_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovLE_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen14e");
    }
}
void OPCALL cmovLE_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovLE_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovLE_32_mem32(struct CPU* cpu, struct Op* op);
void gen34e(struct GenData* data, struct Op* op) {
    if (op->func == cmovLE_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovLE_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovLE_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen34e");
    }
}
void OPCALL cmovNLE_16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNLE_16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNLE_16_mem32(struct CPU* cpu, struct Op* op);
void gen14f(struct GenData* data, struct Op* op) {
    if (op->func == cmovNLE_16_reg) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNLE_16_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNLE_16_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen14f");
    }
}
void OPCALL cmovNLE_32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmovNLE_32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmovNLE_32_mem32(struct CPU* cpu, struct Op* op);
void gen34f(struct GenData* data, struct Op* op) {
    if (op->func == cmovNLE_32_reg) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} CYCLES(1);");
    } else if (op->func == cmovNLE_32_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");} CYCLES(1);");
    } else if (op->func == cmovNLE_32_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");} CYCLES(1);");
    } else {
        kpanic("gen34f");
    }
}
void OPCALL setO_reg(struct CPU* cpu, struct Op* op);
void OPCALL setO_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setO_mem32(struct CPU* cpu, struct Op* op);
void gen190(struct GenData* data, struct Op* op) {
    if (op->func == setO_reg) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setO_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setO_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 0));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen190");
    }
}
void OPCALL setNO_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNO_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNO_mem32(struct CPU* cpu, struct Op* op);
void gen191(struct GenData* data, struct Op* op) {
    if (op->func == setNO_reg) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNO_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNO_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 1));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen191");
    }
}
void OPCALL setB_reg(struct CPU* cpu, struct Op* op);
void OPCALL setB_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setB_mem32(struct CPU* cpu, struct Op* op);
void gen192(struct GenData* data, struct Op* op) {
    if (op->func == setB_reg) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setB_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setB_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 2));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen192");
    }
}
void OPCALL setNB_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNB_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNB_mem32(struct CPU* cpu, struct Op* op);
void gen193(struct GenData* data, struct Op* op) {
    if (op->func == setNB_reg) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNB_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNB_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 3));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen193");
    }
}
void OPCALL setZ_reg(struct CPU* cpu, struct Op* op);
void OPCALL setZ_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setZ_mem32(struct CPU* cpu, struct Op* op);
void gen194(struct GenData* data, struct Op* op) {
    if (op->func == setZ_reg) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setZ_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setZ_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 4));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen194");
    }
}
void OPCALL setNZ_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNZ_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNZ_mem32(struct CPU* cpu, struct Op* op);
void gen195(struct GenData* data, struct Op* op) {
    if (op->func == setNZ_reg) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNZ_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNZ_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 5));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen195");
    }
}
void OPCALL setBE_reg(struct CPU* cpu, struct Op* op);
void OPCALL setBE_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setBE_mem32(struct CPU* cpu, struct Op* op);
void gen196(struct GenData* data, struct Op* op) {
    if (op->func == setBE_reg) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setBE_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setBE_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 6));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen196");
    }
}
void OPCALL setNBE_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNBE_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNBE_mem32(struct CPU* cpu, struct Op* op);
void gen197(struct GenData* data, struct Op* op) {
    if (op->func == setNBE_reg) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNBE_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNBE_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 7));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen197");
    }
}
void OPCALL setS_reg(struct CPU* cpu, struct Op* op);
void OPCALL setS_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setS_mem32(struct CPU* cpu, struct Op* op);
void gen198(struct GenData* data, struct Op* op) {
    if (op->func == setS_reg) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setS_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setS_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 8));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen198");
    }
}
void OPCALL setNS_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNS_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNS_mem32(struct CPU* cpu, struct Op* op);
void gen199(struct GenData* data, struct Op* op) {
    if (op->func == setNS_reg) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNS_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNS_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 9));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen199");
    }
}
void OPCALL setP_reg(struct CPU* cpu, struct Op* op);
void OPCALL setP_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setP_mem32(struct CPU* cpu, struct Op* op);
void gen19a(struct GenData* data, struct Op* op) {
    if (op->func == setP_reg) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setP_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setP_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 10));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen19a");
    }
}
void OPCALL setNP_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNP_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNP_mem32(struct CPU* cpu, struct Op* op);
void gen19b(struct GenData* data, struct Op* op) {
    if (op->func == setNP_reg) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNP_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNP_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 11));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen19b");
    }
}
void OPCALL setL_reg(struct CPU* cpu, struct Op* op);
void OPCALL setL_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setL_mem32(struct CPU* cpu, struct Op* op);
void gen19c(struct GenData* data, struct Op* op) {
    if (op->func == setL_reg) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setL_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setL_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 12));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen19c");
    }
}
void OPCALL setNL_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNL_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNL_mem32(struct CPU* cpu, struct Op* op);
void gen19d(struct GenData* data, struct Op* op) {
    if (op->func == setNL_reg) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNL_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNL_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 13));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen19d");
    }
}
void OPCALL setLE_reg(struct CPU* cpu, struct Op* op);
void OPCALL setLE_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setLE_mem32(struct CPU* cpu, struct Op* op);
void gen19e(struct GenData* data, struct Op* op) {
    if (op->func == setLE_reg) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setLE_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setLE_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 14));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen19e");
    }
}
void OPCALL setNLE_reg(struct CPU* cpu, struct Op* op);
void OPCALL setNLE_mem16(struct CPU* cpu, struct Op* op);
void OPCALL setNLE_mem32(struct CPU* cpu, struct Op* op);
void gen19f(struct GenData* data, struct Op* op) {
    if (op->func == setNLE_reg) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {");
        out(data, r8(op->r1));
        out(data, " = 1;} else {");
        out(data, r8(op->r1));
        out(data, " = 0;} CYCLES(2);");
    } else if (op->func == setNLE_mem16) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", 0);} CYCLES(2);");
    } else if (op->func == setNLE_mem32) {
        out(data, "if (");
        out(data, getCondition(data, 15));
        out(data, ") {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 1);} else {writeb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", 0);} CYCLES(2);");
    } else {
        kpanic("gen19f");
    }
}
