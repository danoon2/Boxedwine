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

void dynamic_jump(DynamicData* data, DecodedOp* op, DynReg reg, U32 inst, U32 len1, U32 len2) {
    startIf(reg, (inst==JumpZ?DYN_NOT_EQUALS_ZERO:DYN_EQUALS_ZERO), true);

    INCREMENT_EIP(data, len2);
    blockNext2(data, op);
    startElse();
    INCREMENT_EIP(data, len1);
    blockNext1(data, op);
    endIf();
    data->done = true;
}

enum DynArg {
    DYN_Mem=0,
    DYN_Reg,
    DYN_Const,
    DYN_None,
};

U32 cpuOffset(U32 r, DynWidth width) {
    if (width==DYN_8bit)
        return CPU::offsetofReg8(r);
    else if (width==DYN_16bit)
        return CPU::offsetofReg16(r);
    else if (width==DYN_32bit)
        return CPU::offsetofReg16(r);
    else {
        kpanic_fmt("dynamic cpuOffset unexpected width: %d", width);
        return 0;
    }
}

U32 cpuOffsetResult(DynWidth width) {
    if (width==DYN_8bit)
        return CPU_OFFSET_OF(result.u8);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(result.u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(result.u32);
    else {
        kpanic_fmt("dynamic cpuOffsetResult unexpected width: %d", width);
        return 0;
    }
}

U32 cpuOffsetDst(DynWidth width) {
    if (width==DYN_8bit)
        return CPU_OFFSET_OF(dst.u8);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(dst.u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(dst.u32);
    else {
        kpanic_fmt("dynamic cpuOffsetDst unexpected width: %d", width);
        return 0;
    }
}

U32 cpuOffsetSrc(DynWidth width) {
    if (width==DYN_8bit)
        return CPU_OFFSET_OF(src.u8);
    else if (width==DYN_16bit)
        return CPU_OFFSET_OF(src.u16);
    else if (width==DYN_32bit)
        return CPU_OFFSET_OF(src.u32);
    else {
        kpanic_fmt("dynamic cpuOffsetSrc unexpected width: %d", width);
        return 0;
    }
}
void dynamic_getCF(DynamicData* data);

void dynamic_arith(DynamicData* data, DecodedOp* op, DynArg src, DynArg dst, DynWidth width, char inst, bool cf, bool store, const LazyFlags* flags) {
    bool needsToSetFlags = op->needsToSetFlags(data->cpu);

    if (cf) {
        dynamic_getCF(data);
    }
    if (!needsToSetFlags && !store) {
        // I've seen a test followed by a cmp when running Quake 2.  Very weird
        INCREMENT_EIP(data, op);
        return;
    }
    if (!needsToSetFlags) {
        if (src == DYN_Reg) {
            if (dst == DYN_Reg) {
                movToRegFromCpu(DYN_SRC, cpuOffset(op->rm, width), width);
                if (cf) {
                    if (width != DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                instCPUReg(inst, cpuOffset(op->reg, width), DYN_SRC, width, true);
            } else if (dst == DYN_Mem) {
                calculateEaa(op, DYN_ADDRESS);
                movToRegFromCpu(DYN_SRC, cpuOffset(op->reg, width), width);
                if (cf) {
                    if (width != DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                instMemReg(inst, DYN_ADDRESS, DYN_SRC, width, true, true);
            }
        } else if (src == DYN_Mem) {
            if (dst == DYN_Reg) {
                calculateEaa(op, DYN_ADDRESS);
                if (cf) {
                    movToRegFromReg(DYN_DEST, width, DYN_CALL_RESULT, DYN_32bit, false);
                    movFromMem(width, DYN_ADDRESS, true);
                    instRegReg('+', DYN_CALL_RESULT, DYN_DEST, width, true);
                } else {
                    movFromMem(width, DYN_ADDRESS, true);
                }
                instCPUReg(inst, cpuOffset(op->reg, width), DYN_CALL_RESULT, width, true);
            }
        } else if (src == DYN_Const) {
            if (dst == DYN_Reg) {
                if (cf) {
                    if (width != DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegImm('+', DYN_CALL_RESULT, width, op->imm);
                    instCPUReg(inst, cpuOffset(op->reg, width), DYN_CALL_RESULT, width, true);
                } else {
                    instCPUImm(inst, cpuOffset(op->reg, width), width, op->imm);
                }
            } else if (dst == DYN_Mem) {
                calculateEaa(op, DYN_ADDRESS);
                if (cf) {
                    movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
                    instRegImm('+', DYN_SRC, width, op->imm);
                    instMemReg(inst, DYN_ADDRESS, DYN_SRC, width, true, true);
                } else {
                    instMemImm(inst, DYN_ADDRESS, width, op->imm, true);
                }
            }
        }
    } else {
        if (cf) {
            movToCpuFromReg(CPU_OFFSET_OF(oldCF), DYN_CALL_RESULT, DYN_32bit, false);
        }
        if (src == DYN_Reg) {
            if (dst == DYN_Reg) {
                movToCpuFromCpu(cpuOffsetSrc(width), cpuOffset(op->rm, width), width, DYN_SRC, false);
                movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(op->reg, width), width, DYN_DEST, false);

                if (cf) {
                    if (width != DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                instRegReg(inst, DYN_DEST, DYN_SRC, width, true);
                movToCpuFromReg(cpuOffsetResult(width), DYN_DEST, width, !store);
                if (store) {
                    movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, true);
                }
            } else if (dst == DYN_Mem) {
                movToCpuFromCpu(cpuOffsetSrc(width), cpuOffset(op->reg, width), width, DYN_SRC, false);
                if (cf) {
                    if (width != DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, true);
                    }
                    instRegReg('+', DYN_SRC, DYN_CALL_RESULT, width, true);
                }
                calculateEaa(op, DYN_ADDRESS);
                movToCpuFromMem(cpuOffsetDst(width), width, DYN_ADDRESS, !store, false);
                instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
                movToCpuFromReg(cpuOffsetResult(width), DYN_CALL_RESULT, width, !store);
                if (store) {
                    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true);
                }
            }
        } else if (src == DYN_Mem) {
            if (dst == DYN_Reg) {
                if (cf) {
                    movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
                }
                calculateEaa(op, DYN_ADDRESS);
                movToCpuFromMem(cpuOffsetSrc(width), width, DYN_ADDRESS, true, false);
                movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(op->reg, width), width, DYN_DEST, false);
                if (cf) {
                    instRegReg('+', DYN_CALL_RESULT, DYN_SRC, width, true);
                }
                instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
                movToCpuFromReg(cpuOffsetResult(width), DYN_DEST, width, !store);
                if (store) {
                    movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, true);
                }
            }
        } else if (src == DYN_Const) {
            if (dst == DYN_Reg) {
                movToCpu(cpuOffsetSrc(width), width, op->imm);
                movToCpuFromCpu(cpuOffsetDst(width), cpuOffset(op->reg, width), width, DYN_DEST, false);
                instRegImm(inst, DYN_DEST, width, op->imm);
                if (cf) {
                    if (width != DYN_32bit) {
                        movToRegFromReg(DYN_CALL_RESULT, width, DYN_CALL_RESULT, DYN_32bit, false);
                    }
                    instRegReg(inst, DYN_DEST, DYN_CALL_RESULT, width, true);
                }
                movToCpuFromReg(cpuOffsetResult(width), DYN_DEST, width, !store);
                if (store) {
                    movToCpuFromReg(cpuOffset(op->reg, width), DYN_DEST, width, true);
                }
            } else if (dst == DYN_Mem) {
                calculateEaa(op, DYN_ADDRESS);
                movToCpu(cpuOffsetSrc(width), width, op->imm);
                if (cf) {
                    movToRegFromReg(DYN_SRC, width, DYN_CALL_RESULT, DYN_32bit, true);
                    movToCpuFromMem(cpuOffsetDst(width), width, DYN_ADDRESS, !store, false);
                    instRegImm('+', DYN_SRC, width, op->imm);
                    instRegReg(inst, DYN_CALL_RESULT, DYN_SRC, width, true);
                } else {
                    movToCpuFromMem(cpuOffsetDst(width), width, DYN_ADDRESS, !store, false);
                    instRegImm(inst, DYN_CALL_RESULT, width, op->imm);
                }
                movToCpuFromReg(cpuOffsetResult(width), DYN_CALL_RESULT, width, !store);
                if (store) {
                    movToMemFromReg(DYN_ADDRESS, DYN_CALL_RESULT, width, true, true);
                }
            }
        }
        movToCpuPtr(CPU_OFFSET_OF(lazyFlags), (DYN_PTR_SIZE)flags);
        data->currentLazyFlags = flags;
    }

    INCREMENT_EIP(data, op);
}

void genCF(const LazyFlags* flags, DynReg reg) {
    if (reg==DYN_SRC || reg == DYN_DEST) {
        kpanic("genCF expects reg not to be DYN_SRC or DYN_DEST");
    }
    if (regUsed[DYN_SRC] || regUsed[DYN_DEST]) {
        kpanic("genCF expects DYN_SRC and DYN_DEST to be available");
    }
    if (flags == FLAGS_NONE) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(flags), DYN_32bit);
        instRegImm('&', reg, DYN_32bit, CF);
    } else if (flags == FLAGS_ADD8) {
        // cpu->result.u8<cpu->dst.u8;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_ADD16) {
        // cpu->result.u16<cpu->dst.u16;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_ADD32) {
        // cpu->result.u32<cpu->dst.u32;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_OR8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_ADC8) {
        // (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8));
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_EQUALS, false, false);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, false, false);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(oldCF), DYN_32bit);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_ADC16) {
        // (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_EQUALS, false, false);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, false, false);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(oldCF), DYN_32bit);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_ADC32) {
        // (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_EQUALS, false, false);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, false, false);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(oldCF), DYN_32bit);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB8) {
        // (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8==0xff));
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xff, DYN_8bit, DYN_EQUALS, true, false);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(oldCF), DYN_32bit);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        instRegReg('&', reg, DYN_SRC, DYN_32bit, true);

        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);        
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, false);

        instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB16) {
        // (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16==0xffff));
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xffff, DYN_16bit, DYN_EQUALS, true, false);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(oldCF), DYN_32bit);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        instRegReg('&', reg, DYN_SRC, DYN_32bit, true);

        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);        
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, false);

        instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB32) {
        // (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32==0xffffffff));
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xffffffff, DYN_32bit, DYN_EQUALS, true, false);
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(oldCF), DYN_32bit);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        instRegReg('&', reg, DYN_SRC, DYN_32bit, true);

        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(result.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);        
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, false);

        instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_AND8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SUB8) {
        // cpu->dst.u8<cpu->src.u8;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_SUB16) {
        // cpu->dst.u16<cpu->src.u16;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_SUB32) {
        // cpu->dst.u32<cpu->src.u32;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_XOR8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_INC8) {
        // cpu->oldCF;
        movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
    } else if (flags == FLAGS_INC16) {
        // cpu->oldCF;
        movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
    } else if (flags == FLAGS_INC32) {
        // cpu->oldCF;
        movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
    } else if (flags == FLAGS_DEC8) {
        // cpu->oldCF;
        movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
    } else if (flags == FLAGS_DEC16) {
        // cpu->oldCF;
        movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
    } else if (flags == FLAGS_DEC32) {
        // cpu->oldCF;
        movToRegFromCpu(reg, CPU_OFFSET_OF(oldCF), DYN_32bit);
    } else if (flags == FLAGS_SHL8) {
        // ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        instRegImm('-', DYN_SRC, DYN_8bit, 1);
        instRegReg('<', reg, DYN_SRC, DYN_8bit, true);
        instRegImm('&', reg, DYN_8bit, 0x80);
        instRegImm('>', reg, DYN_8bit, 7);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('-', DYN_SRC, DYN_16bit, 1);
        instRegReg('<', reg, DYN_SRC, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        instRegImm('>', reg, DYN_16bit, 15);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(src.u32), DYN_32bit); // ok to use src.u32 instead of src.u8
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        movToReg(DYN_SRC, DYN_32bit, 32);
        instRegReg('-', DYN_SRC, DYN_DEST, DYN_32bit, true);
        instRegReg('>', reg, DYN_SRC, DYN_32bit, true); // on x86, shift by reg, reg must be cl
        instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SHR8 || flags == FLAGS_SHR8_N1) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        instRegImm('-', DYN_SRC, DYN_8bit, 1);
        instRegReg('>', reg, DYN_SRC, DYN_8bit, true);
        instRegImm('&', reg, DYN_8bit, 0x1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16 || flags == FLAGS_SHR16_N1) {
        // (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('-', DYN_SRC, DYN_16bit, 1);
        instRegReg('>', reg, DYN_SRC, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32 || flags == FLAGS_SHR32_N1) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit); // ok to use src.u32 instead of src.u8
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        instRegImm('-', DYN_SRC, DYN_32bit, 1);
        instRegReg('>', reg, DYN_SRC, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SHR8_1) {
        // cpu->dst.u8 & 1;
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        instRegImm('&', reg, DYN_8bit, 0x1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16_1) {
        // cpu->dst.u16 & 1;
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('&', reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32_1) {
        // cpu->dst.u32  & 1;
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SAR8) {
        // (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        instRegImm('-', DYN_SRC, DYN_8bit, 1);
        instRegReg(')', reg, DYN_SRC, DYN_8bit, true);
        instRegImm('&', reg, DYN_8bit, 0x1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SAR16) {
        // (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('-', DYN_SRC, DYN_16bit, 1);
        instRegReg(')', reg, DYN_SRC, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SAR32) {
        // (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit); // ok to use src.u32 instead of src.u8
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        instRegImm('-', DYN_SRC, DYN_32bit, 1);
        instRegReg(')', reg, DYN_SRC, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_TEST8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_DSHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('-', DYN_SRC, DYN_16bit, 1);
        instRegReg('<', reg, DYN_SRC, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        instRegImm('>', reg, DYN_16bit, 15);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(src.u32), DYN_32bit); // ok to use src.u32 instead of src.u8
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        movToReg(DYN_SRC, DYN_32bit, 32);
        instRegReg('-', DYN_SRC, DYN_DEST, DYN_32bit, true); // on x86, shift by reg, reg must be cl
        instRegReg('>', reg, DYN_SRC, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_DSHR16) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('-', DYN_SRC, DYN_16bit, 1);
        instRegReg('>', reg, DYN_SRC, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHR32) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit); // ok to use src.u32 instead of src.u8
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        instRegImm('-', DYN_SRC, DYN_32bit, 1);
        instRegReg('>', reg, DYN_SRC, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 1);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8!=0;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_8bit, DYN_NOT_EQUALS, true, false);
    } else if (flags == FLAGS_NEG16) {
        // cpu->src.u16!=0;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_16bit, DYN_NOT_EQUALS, true, false);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32!=0;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, true, false);
    } else {
        kpanic("genCF unknown flags");
    }
}

void genOF(const LazyFlags* flags, DynReg reg) {
    if (reg == DYN_SRC || reg == DYN_DEST) {
        kpanic("genOF expects reg not to be DYN_SRC or DYN_DEST");
    }
    if (regUsed[DYN_SRC] || regUsed[DYN_DEST]) {
        kpanic("genOF expects DYN_SRC and DYN_DEST to be available");
    }
    if (flags == FLAGS_NONE) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(flags), DYN_32bit);
        instRegImm('&', reg, DYN_32bit, CF);
    } else if (flags == FLAGS_ADD8 || flags == FLAGS_ADC8) {
        // ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;        
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        instRegReg('^', DYN_DEST, DYN_SRC, DYN_8bit, false);
        instRegImm('^', DYN_DEST, DYN_8bit, 0x80);
        instRegReg('^', reg, DYN_SRC, DYN_8bit, true);
        instRegReg('&', reg, DYN_DEST, DYN_8bit, true);
        instRegImm('&', reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_ADD16 || flags == FLAGS_ADC16) {
        // ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        instRegReg('^', DYN_DEST, DYN_SRC, DYN_16bit, false);
        instRegImm('^', DYN_DEST, DYN_16bit, 0x8000);
        instRegReg('^', reg, DYN_SRC, DYN_16bit, true);
        instRegReg('&', reg, DYN_DEST, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_ADD32 || flags == FLAGS_ADC32) {
        // ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        instRegReg('^', DYN_DEST, DYN_SRC, DYN_32bit, false);
        instRegImm('^', DYN_DEST, DYN_32bit, 0x80000000);
        instRegReg('^', reg, DYN_SRC, DYN_32bit, true);
        instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 0x80000000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_32bit, false);
    } else if (flags == FLAGS_OR8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_OR32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_AND32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SUB8 || flags == FLAGS_SBB8 || flags == FLAGS_CMP8) {
        // ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        instRegReg('^', DYN_SRC, DYN_DEST, DYN_8bit, false);
        instRegReg('^', reg, DYN_DEST, DYN_8bit, true);
        instRegReg('&', reg, DYN_SRC, DYN_8bit, true);
        instRegImm('&', reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SUB16 || flags == FLAGS_SBB16 || flags == FLAGS_CMP16) {
        // ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        instRegReg('^', DYN_SRC, DYN_DEST, DYN_16bit, false);
        instRegReg('^', reg, DYN_DEST, DYN_16bit, true);
        instRegReg('&', reg, DYN_SRC, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SUB32 || flags == FLAGS_SBB32 || flags == FLAGS_CMP32) {
        // ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        instRegReg('^', DYN_SRC, DYN_DEST, DYN_32bit, false);
        instRegReg('^', reg, DYN_DEST, DYN_32bit, true);
        instRegReg('&', reg, DYN_SRC, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_XOR8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_XOR32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_INC8) {
        // cpu->result.u8 == 0x80;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_INC16) {
        // cpu->result.u16 == 0x8000;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_INC32) {
        // cpu->result.u32 == 0x80000000;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC8) {
        // cpu->result.u8 == 0x7f;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7f, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC16) {
        // cpu->result.u16 == 0x7fff;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7fff, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC32) {
        // cpu->result.u32 == 0x7fffffff;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7fffffff, DYN_32bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_SHL8) {
        // (cpu->result.u8 ^ cpu->dst.u8) & 0x80;
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        instRegReg('^', reg, DYN_DEST, DYN_8bit, true);
        instRegImm('&', reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHL16) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        instRegReg('^', reg, DYN_DEST, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHL32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        instRegReg('^', reg, DYN_DEST, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_SHR8) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        instRegImm('&', DYN_SRC, DYN_8bit, 0x1f);
        evaluateToReg(reg, DYN_8bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_8bit, DYN_EQUALS, true, false);
        instRegImm('>', DYN_DEST, DYN_8bit, 7); // mov top bit to bottom
        instRegReg('&', reg, DYN_DEST, DYN_8bit, true);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegImm('&', DYN_SRC, DYN_16bit, 0x1f);
        evaluateToReg(reg, DYN_16bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_16bit, DYN_EQUALS, true, false);
        instRegImm('>', DYN_DEST, DYN_16bit, 15); // mov top bit to bottom
        instRegReg('&', reg, DYN_DEST, DYN_16bit, true);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;
        movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        instRegImm('&', DYN_SRC, DYN_32bit, 0x1f);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_32bit, DYN_EQUALS, true, false);
        instRegImm('>', DYN_DEST, DYN_32bit, 31); // mov top bit to bottom
        instRegReg('&', reg, DYN_DEST, DYN_32bit, true);
    } else if (flags == FLAGS_SHR8_1) {
        // (cpu->dst.u8 >= 0x80);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR16_1) {
        // (cpu->dst.u16 >= 0x8000);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR32_1) {
        // (cpu->dst.u32 >= 0x80000000);
        movToRegFromCpu(reg, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR8_N1) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SHR16_N1) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SHR32_N1) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SAR8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SAR16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_SAR32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST8) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST16) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_TEST32) {
        // 0
        movToReg(reg, DYN_32bit, 0);
    } else if (flags == FLAGS_DSHL16 || flags == FLAGS_DSHR16) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
        instRegReg('^', reg, DYN_DEST, DYN_16bit, true);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHL32 || flags == FLAGS_DSHR32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
        instRegReg('^', reg, DYN_DEST, DYN_32bit, true);
        instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8 == 0x80;
        movToRegFromCpu(reg, CPU_OFFSET_OF(src.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_NEG16) {
        // return cpu->src.u16 == 0x8000;
        movToRegFromCpu(reg, CPU_OFFSET_OF(src.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32 == 0x80000000;
        movToRegFromCpu(reg, CPU_OFFSET_OF(src.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_EQUALS, false, false);
    }
}

DynWidth getWidthOfCondition(const LazyFlags* flags) {
    if (flags->width==32)
        return DYN_32bit;
    if (flags->width==16)
        return DYN_16bit;
    if (flags->width==8)
        return DYN_8bit;
    kpanic_fmt("getWidthOfCondition: invalid flag width: %d", flags->width);
    return DYN_32bit;
}

void genNZ(const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
    } else if (width==DYN_16bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (width==DYN_8bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

void genZ(const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
    } else if (width==DYN_16bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_16bit, DYN_EQUALS, false, false);
    } else if (width==DYN_8bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_8bit, DYN_EQUALS, false, false);
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

void genS(const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u32), DYN_32bit);
        instRegImm('&', reg, DYN_32bit, 0x80000000);
    } else if (width==DYN_16bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u16), DYN_16bit);
        instRegImm('&', reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);            
    } else if (width==DYN_8bit) {
        movToRegFromCpu(reg, CPU_OFFSET_OF(result.u8), DYN_8bit);
        instRegImm('&', reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);            
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

bool getFlagInReg(DynamicData* data, DynConditional condition, DynReg reg) {
    U32 flag = 0;
    bool notFlag = false;

    switch (condition) {
    case O: flag = OF; break;
    case NO: flag = OF; notFlag = true; break;
    case B: flag = CF; break;
    case NB: flag = CF; notFlag = true; break;
    case Z: flag = ZF; break;
    case NZ: flag = ZF; notFlag = true; break;
    case S: flag = SF; break;
    case NS: flag = SF; notFlag = true; break;
    case P: flag = PF; break;
    case NP: flag = PF; notFlag = true; break;
    default: 
        return false;
    }

    movToRegFromCpu(reg, CPU_OFFSET_OF(flags), DYN_32bit);
    if (notFlag)
        instReg('~', reg, DYN_32bit);
    instRegImm('&', reg, DYN_32bit, flag);
    return true;
}

void getCondition(DynamicData* data, DynConditional condition, DynReg reg) {
    if (regUsed[DYN_CALL_RESULT] && reg != DYN_CALL_RESULT) {
        kpanic("getCondition expects DYN_CALL_RESULT to be available");
    }
    switch (condition) {
    case O: callHostFunction((void*)common_condition_o, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NO: callHostFunction((void*)common_condition_no, true, 1, 0, DYN_PARAM_CPU, false); break;
    case B: callHostFunction((void*)common_condition_b, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NB: callHostFunction((void*)common_condition_nb, true, 1, 0, DYN_PARAM_CPU, false); break;
    case Z: callHostFunction((void*)common_condition_z, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NZ: callHostFunction((void*)common_condition_nz, true, 1, 0, DYN_PARAM_CPU, false); break;
    case BE: callHostFunction((void*)common_condition_be, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NBE: callHostFunction((void*)common_condition_nbe, true, 1, 0, DYN_PARAM_CPU, false); break;
    case S: callHostFunction((void*)common_condition_s, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NS: callHostFunction((void*)common_condition_ns, true, 1, 0, DYN_PARAM_CPU, false); break;
    case P: callHostFunction((void*)common_condition_p, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NP: callHostFunction((void*)common_condition_np, true, 1, 0, DYN_PARAM_CPU, false); break;
    case L: callHostFunction((void*)common_condition_l, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NL: callHostFunction((void*)common_condition_nl, true, 1, 0, DYN_PARAM_CPU, false); break;
    case LE: callHostFunction((void*)common_condition_le, true, 1, 0, DYN_PARAM_CPU, false); break;
    case NLE: callHostFunction((void*)common_condition_nle, true, 1, 0, DYN_PARAM_CPU, false); break;
    default:
        kpanic_fmt("setConditionInReg: unknown condition %d", condition);
    }
    if (reg != DYN_CALL_RESULT) {
        movToRegFromReg(reg, DYN_32bit, DYN_CALL_RESULT, DYN_32bit, true);
    }
}

// by inlining flag calculations, we save a virtual function call or 2.  This really helps performance for Quake 2, around 10%.
// The if statement to check if the lazyFlags are still what we thought they would be are for when code in another location
// jumps here with a different lazyFlags, its rare so the hardware branch prediction can handle this check for free.  I have
// seen this happen with iexplore.
void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg) {
    if ((regUsed[DYN_DEST] && reg != DYN_DEST) || (regUsed[DYN_SRC] && reg != DYN_SRC)) {
        kpanic("setConditionInReg expects DYN_DEST to be available");
    }
    if (!data->currentLazyFlags || condition == P || condition == NP) {
        getCondition(data, condition, reg);
        return;
    }    
    movToRegFromCpuPtr(reg, offsetof(CPU, lazyFlags));
    startIfCmpPtr(reg, (DYN_PTR_SIZE)data->currentLazyFlags, true, false);
    if (data->currentLazyFlags==FLAGS_NONE) {
        if (!getFlagInReg(data, condition, reg)) {
            getCondition(data, condition, reg);
        }
    } else if (condition==NZ) {
        genNZ(data->currentLazyFlags, reg);
    } else if (condition==Z) {
        genZ(data->currentLazyFlags, reg);
    } else if (condition==S) {
        genS(data->currentLazyFlags, reg);
    } else if (condition==B) {
        genCF(data->currentLazyFlags, reg);
    } else if (condition==O) {
        genOF(data->currentLazyFlags, reg);
    } else if (condition==BE) {
        if (data->currentLazyFlags==FLAGS_SUB8) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB16) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB32) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        }  else {
            // cpu->getZF() || cpu->getCF()
            genCF(data->currentLazyFlags, reg);
            // must come after genCF, since genCF can clobber DYN_SRC
            genZ(data->currentLazyFlags, DYN_SRC);
            instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
        }
    } else if (condition==L) {
        if (data->currentLazyFlags==FLAGS_SUB8) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB16) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB32) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_SIGNED, true, true);
        } else {
            // cpu->getSF()!=cpu->getOF()        
            genOF(data->currentLazyFlags, reg);
            genS(data->currentLazyFlags, DYN_SRC);
            evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(reg, DYN_32bit, reg, false, DYN_SRC, 0, DYN_32bit, DYN_NOT_EQUALS, false, true);
        }
    } else if (condition==LE) {
        if (data->currentLazyFlags==FLAGS_SUB8) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u8), DYN_8bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u8), DYN_8bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB16) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u16), DYN_16bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u16), DYN_16bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else if (data->currentLazyFlags==FLAGS_SUB32) {
            movToRegFromCpu(DYN_SRC, CPU_OFFSET_OF(src.u32), DYN_32bit);
            movToRegFromCpu(DYN_DEST, CPU_OFFSET_OF(dst.u32), DYN_32bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else {
            // cpu->getZF() || cpu->getSF()!=cpu->getOF()       
            genOF(data->currentLazyFlags, reg);
            genS(data->currentLazyFlags, DYN_SRC);
            evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(reg, DYN_32bit, reg, false, DYN_SRC, 0, DYN_32bit, DYN_NOT_EQUALS, false, true);
            genZ(data->currentLazyFlags, DYN_SRC);
            instRegReg('|', reg, DYN_SRC, DYN_32bit, true);
        }
    } else {
        kpanic("setConditionInReg unhandled condition");
    }
    startElse();
    getCondition(data, condition, reg);
    endIf();
}

void dynamic_pushReg32(DynamicData* data, DynReg reg, bool doneWithReg) {    
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        instRegImm('-', DYN_ADDRESS, DYN_32bit, 4);
        movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, false, doneWithReg);
        movToCpuFromReg(CPU_OFFSET_OF(reg[4].u32), DYN_ADDRESS, DYN_32bit, true);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, doneWithReg);
    }
}

void dynamic_pop32(DynamicData* data) {
    if (!data->cpu->thread->process->hasSetStackMask && !data->cpu->thread->process->hasSetSeg[SS]) {
        movToRegFromCpu(DYN_ADDRESS, CPU_OFFSET_OF(reg[4].u32), DYN_32bit);
        movFromMem(DYN_32bit, DYN_ADDRESS, true);
        instCPUImm('+', CPU_OFFSET_OF(reg[4].u32), DYN_32bit, 4);
    } else {
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    }    
}

void dynamic_fillFlags(DynamicData* data) {
    callHostFunction((void*)common_fillFlags, false, 1, 0, DYN_PARAM_CPU, false);
    data->currentLazyFlags=FLAGS_NONE;
}

void dynamic_getCF(DynamicData* data) {
    if (data->currentLazyFlags) {
        movToRegFromCpuPtr(DYN_DEST, offsetof(CPU, lazyFlags));
        startIfCmpPtr(DYN_DEST, (DYN_PTR_SIZE)data->currentLazyFlags, true, true);
        genCF(data->currentLazyFlags, DYN_CALL_RESULT);
        startElse();
        callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
        endIf();
    } else {
        callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
    }
}
