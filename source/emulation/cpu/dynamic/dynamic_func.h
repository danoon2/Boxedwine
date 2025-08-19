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
    }

    INCREMENT_EIP(data, op);
}

void setConditionInReg(DynamicData* data, DynConditional condition, DynReg reg) {
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
}

void dynamic_getCF(DynamicData* data) {
    callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
}
