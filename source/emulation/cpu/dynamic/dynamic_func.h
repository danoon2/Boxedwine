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

void DynamicData::genCF(const LazyFlags* flags, DynReg reg) {
    if (reg==DYN_SRC || reg == DYN_DEST) {
        kpanic("genCF expects reg not to be DYN_SRC or DYN_DEST");
    }
    if (regUsed[DYN_SRC] || regUsed[DYN_DEST]) {
        kpanic("genCF expects DYN_SRC and DYN_DEST to be available");
    }
    if (flags == FLAGS_NONE) {
        loadCPUFlags(reg);
        andRegImm(reg, DYN_32bit, CF);
    } else if (flags == FLAGS_ADD8) {
        // cpu->result.u8<cpu->dst.u8;
        loadLazyFlagsResult(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_ADD16) {
        // cpu->result.u16<cpu->dst.u16;
        loadLazyFlagsResult(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_ADD32) {
        // cpu->result.u32<cpu->dst.u32;
        loadLazyFlagsResult(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
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
        loadLazyFlagsResult(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_EQUALS, false, false);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, false, false);
        loadLazyFlagsOldCF(DYN_DEST);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        andRegReg(reg, DYN_DEST, DYN_32bit, true);
        orRegReg(reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_ADC16) {
        // (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));
        loadLazyFlagsResult(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_EQUALS, false, false);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, false, false);
        loadLazyFlagsOldCF(DYN_DEST);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        andRegReg(reg, DYN_DEST, DYN_32bit, true);
        orRegReg(reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_ADC32) {
        // (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));
        loadLazyFlagsResult(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_EQUALS, false, false);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, false, DYN_DEST, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, false, false);
        loadLazyFlagsOldCF(DYN_DEST);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        andRegReg(reg, DYN_DEST, DYN_32bit, true);
        orRegReg(reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB8) {
        // (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8==0xff));
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xff, DYN_8bit, DYN_EQUALS, true, false);
        loadLazyFlagsOldCF(DYN_SRC);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        andRegReg(reg, DYN_SRC, DYN_32bit, true);

        loadLazyFlagsResult(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, false);

        orRegReg(reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB16) {
        // (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16==0xffff));
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xffff, DYN_16bit, DYN_EQUALS, true, false);
        loadLazyFlagsOldCF(DYN_SRC);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        andRegReg(reg, DYN_SRC, DYN_32bit, true);

        loadLazyFlagsResult(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, false);

        orRegReg(reg, DYN_SRC, DYN_32bit, true);
    } else if (flags == FLAGS_SBB32) {
        // (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32==0xffffffff));
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0xffffffff, DYN_32bit, DYN_EQUALS, true, false);
        loadLazyFlagsOldCF(DYN_SRC);
        // shortcut, we know oldCF will be 0 or 1, we also know that evaluateToReg will be 0 or 1
        andRegReg(reg, DYN_SRC, DYN_32bit, true);

        loadLazyFlagsResult(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        evaluateToReg(DYN_SRC, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_UNSIGNED, true, false);

        orRegReg(reg, DYN_SRC, DYN_32bit, true);
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
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_SUB16) {
        // cpu->dst.u16<cpu->src.u16;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_UNSIGNED, true, true);
    } else if (flags == FLAGS_SUB32) {
        // cpu->dst.u32<cpu->src.u32;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
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
        loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_INC16) {
        // cpu->oldCF;
        loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_INC32) {
        // cpu->oldCF;
        loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_DEC8) {
        // cpu->oldCF;
        loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_DEC16) {
        // cpu->oldCF;
        loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_DEC32) {
        // cpu->oldCF;
        loadLazyFlagsOldCF(reg);
    } else if (flags == FLAGS_SHL8) {
        // ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(reg, DYN_8bit);
        subRegImm(DYN_SRC, DYN_8bit, 1);
        shlRegReg(reg, DYN_SRC, DYN_8bit, true);
        andRegImm(reg, DYN_8bit, 0x80);
        shrRegImm(reg, DYN_8bit, 7);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(reg, DYN_16bit);
        subRegImm(DYN_SRC, DYN_16bit, 1);
        shlRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 0x8000);
        shrRegImm(reg, DYN_16bit, 15);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        loadLazyFlagsSrc(DYN_DEST, DYN_32bit); // ok to use src.u32 instead of src.u8
        loadLazyFlagsDst(reg, DYN_32bit);
        movToReg(DYN_SRC, DYN_32bit, 32);
        subRegReg(DYN_SRC, DYN_DEST, DYN_32bit, true);
        shrRegReg(reg, DYN_SRC, DYN_32bit, true); // on x86, shift by reg, reg must be cl
        andRegImm(reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SHR8 || flags == FLAGS_SHR8_N1) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(reg, DYN_8bit);
        subRegImm(DYN_SRC, DYN_8bit, 1);
        shrRegReg(reg, DYN_SRC, DYN_8bit, true);
        andRegImm(reg, DYN_8bit, 0x1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16 || flags == FLAGS_SHR16_N1) {
        // (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(reg, DYN_16bit);
        subRegImm(DYN_SRC, DYN_16bit, 1);
        shrRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32 || flags == FLAGS_SHR32_N1) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit); // ok to use src.u32 instead of src.u8
        loadLazyFlagsDst(reg, DYN_32bit);
        subRegImm(DYN_SRC, DYN_32bit, 1);
        shrRegReg(reg, DYN_SRC, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SHR8_1) {
        // cpu->dst.u8 & 1;
        loadLazyFlagsDst(reg, DYN_8bit);
        andRegImm(reg, DYN_8bit, 0x1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16_1) {
        // cpu->dst.u16 & 1;
        loadLazyFlagsDst(reg, DYN_16bit);
        andRegImm(reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32_1) {
        // cpu->dst.u32  & 1;
        loadLazyFlagsDst(reg, DYN_32bit);
        andRegImm(reg, DYN_32bit, 1);
    } else if (flags == FLAGS_SAR8) {
        // (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(reg, DYN_8bit);
        subRegImm(DYN_SRC, DYN_8bit, 1);
        sarRegReg(reg, DYN_SRC, DYN_8bit, true);
        andRegImm(reg, DYN_8bit, 0x1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SAR16) {
        // (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(reg, DYN_16bit);
        subRegImm(DYN_SRC, DYN_16bit, 1);
        sarRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SAR32) {
        // (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit); // ok to use src.u32 instead of src.u8
        loadLazyFlagsDst(reg, DYN_32bit);
        subRegImm(DYN_SRC, DYN_32bit, 1);
        sarRegReg(reg, DYN_SRC, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 1);
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
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(reg, DYN_16bit);
        subRegImm(DYN_SRC, DYN_16bit, 1);
        shlRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 0x8000);
        shrRegImm(reg, DYN_16bit, 15);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        loadLazyFlagsSrc(DYN_DEST, DYN_32bit); // ok to use src.u32 instead of src.u8
        loadLazyFlagsDst(reg, DYN_32bit);
        movToReg(DYN_SRC, DYN_32bit, 32);
        subRegReg(DYN_SRC, DYN_DEST, DYN_32bit, true); // on x86, shift by reg, reg must be cl
        shrRegReg(reg, DYN_SRC, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 1);
    } else if (flags == FLAGS_DSHR16) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(reg, DYN_16bit);
        subRegImm(DYN_SRC, DYN_16bit, 1);
        shrRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 1);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHR32) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit); // ok to use src.u32 instead of src.u8
        loadLazyFlagsDst(reg, DYN_32bit);
        subRegImm(DYN_SRC, DYN_32bit, 1);
        shrRegReg(reg, DYN_SRC, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 1);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8!=0;
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_8bit, DYN_NOT_EQUALS, true, false);
    } else if (flags == FLAGS_NEG16) {
        // cpu->src.u16!=0;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_16bit, DYN_NOT_EQUALS, true, false);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32!=0;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_NOT_EQUALS, true, false);
    } else {
        kpanic("genCF unknown flags");
    }
}

void DynamicData::genOF(const LazyFlags* flags, DynReg reg) {
    if (reg == DYN_SRC || reg == DYN_DEST) {
        kpanic("genOF expects reg not to be DYN_SRC or DYN_DEST");
    }
    if (regUsed[DYN_SRC] || regUsed[DYN_DEST]) {
        kpanic("genOF expects DYN_SRC and DYN_DEST to be available");
    }
    if (flags == FLAGS_NONE) {
        loadCPUFlags(reg);
        andRegImm(reg, DYN_32bit, OF);
    } else if (flags == FLAGS_ADD8 || flags == FLAGS_ADC8) {
        // ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;        
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        loadLazyFlagsResult(reg, DYN_8bit);
        xorRegReg(DYN_DEST, DYN_SRC, DYN_8bit, false);
        xorRegImm(DYN_DEST, DYN_8bit, 0x80);
        xorRegReg(reg, DYN_SRC, DYN_8bit, true);
        andRegReg(reg, DYN_DEST, DYN_8bit, true);
        andRegImm(reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_ADD16 || flags == FLAGS_ADC16) {
        // ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        loadLazyFlagsResult(reg, DYN_16bit);
        xorRegReg(DYN_DEST, DYN_SRC, DYN_16bit, false);
        xorRegImm(DYN_DEST, DYN_16bit, 0x8000);
        xorRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegReg(reg, DYN_DEST, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_ADD32 || flags == FLAGS_ADC32) {
        // ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        loadLazyFlagsResult(reg, DYN_32bit);
        xorRegReg(DYN_DEST, DYN_SRC, DYN_32bit, false);
        xorRegImm(DYN_DEST, DYN_32bit, 0x80000000);
        xorRegReg(reg, DYN_SRC, DYN_32bit, true);
        andRegReg(reg, DYN_DEST, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 0x80000000);
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
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        loadLazyFlagsResult(reg, DYN_8bit);
        xorRegReg(DYN_SRC, DYN_DEST, DYN_8bit, false);
        xorRegReg(reg, DYN_DEST, DYN_8bit, true);
        andRegReg(reg, DYN_SRC, DYN_8bit, true);
        andRegImm(reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SUB16 || flags == FLAGS_SBB16 || flags == FLAGS_CMP16) {
        // ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        loadLazyFlagsResult(reg, DYN_16bit);
        xorRegReg(DYN_SRC, DYN_DEST, DYN_16bit, false);
        xorRegReg(reg, DYN_DEST, DYN_16bit, true);
        andRegReg(reg, DYN_SRC, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SUB32 || flags == FLAGS_SBB32 || flags == FLAGS_CMP32) {
        // ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        loadLazyFlagsResult(reg, DYN_32bit);
        xorRegReg(DYN_SRC, DYN_DEST, DYN_32bit, false);
        xorRegReg(reg, DYN_DEST, DYN_32bit, true);
        andRegReg(reg, DYN_SRC, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 0x80000000);
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
        loadLazyFlagsResult(reg, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_INC16) {
        // cpu->result.u16 == 0x8000;
        loadLazyFlagsResult(reg, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_INC32) {
        // cpu->result.u32 == 0x80000000;
        loadLazyFlagsResult(reg, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC8) {
        // cpu->result.u8 == 0x7f;
        loadLazyFlagsResult(reg, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7f, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC16) {
        // cpu->result.u16 == 0x7fff;
        loadLazyFlagsResult(reg, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7fff, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_DEC32) {
        // cpu->result.u32 == 0x7fffffff;
        loadLazyFlagsResult(reg, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x7fffffff, DYN_32bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_SHL8) {
        // (cpu->result.u8 ^ cpu->dst.u8) & 0x80;
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        loadLazyFlagsResult(reg, DYN_8bit);
        xorRegReg(reg, DYN_DEST, DYN_8bit, true);
        andRegImm(reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHL16) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        loadLazyFlagsResult(reg, DYN_16bit);
        xorRegReg(reg, DYN_DEST, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHL32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        loadLazyFlagsResult(reg, DYN_32bit);
        xorRegReg(reg, DYN_DEST, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_SHR8) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;
        loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
        loadLazyFlagsDst(DYN_DEST, DYN_8bit);
        andRegImm(DYN_SRC, DYN_8bit, 0x1f);
        evaluateToReg(reg, DYN_8bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_8bit, DYN_EQUALS, true, false);
        shrRegImm(DYN_DEST, DYN_8bit, 7); // mov top bit to bottom
        andRegReg(reg, DYN_DEST, DYN_8bit, true);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else if (flags == FLAGS_SHR16) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;
        loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        andRegImm(DYN_SRC, DYN_16bit, 0x1f);
        evaluateToReg(reg, DYN_16bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_16bit, DYN_EQUALS, true, false);
        shrRegImm(DYN_DEST, DYN_16bit, 15); // mov top bit to bottom
        andRegReg(reg, DYN_DEST, DYN_16bit, true);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_SHR32) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;
        loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        andRegImm(DYN_SRC, DYN_32bit, 0x1f);
        evaluateToReg(reg, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 1, DYN_32bit, DYN_EQUALS, true, false);
        shrRegImm(DYN_DEST, DYN_32bit, 31); // mov top bit to bottom
        andRegReg(reg, DYN_DEST, DYN_32bit, true);
    } else if (flags == FLAGS_SHR8_1) {
        // (cpu->dst.u8 >= 0x80);
        loadLazyFlagsDst(reg, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR16_1) {
        // (cpu->dst.u16 >= 0x8000);
        loadLazyFlagsDst(reg, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_GREATER_THAN_EQUAL_UNSIGNED, false, false);
    } else if (flags == FLAGS_SHR32_1) {
        // (cpu->dst.u32 >= 0x80000000);
        loadLazyFlagsDst(reg, DYN_32bit);
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
        loadLazyFlagsResult(reg, DYN_16bit);
        loadLazyFlagsDst(DYN_DEST, DYN_16bit);
        xorRegReg(reg, DYN_DEST, DYN_16bit, true);
        andRegImm(reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (flags == FLAGS_DSHL32 || flags == FLAGS_DSHR32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        loadLazyFlagsResult(reg, DYN_32bit);
        loadLazyFlagsDst(DYN_DEST, DYN_32bit);
        xorRegReg(reg, DYN_DEST, DYN_32bit, true);
        andRegImm(reg, DYN_32bit, 0x80000000);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8 == 0x80;
        loadLazyFlagsSrc(reg, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80, DYN_8bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_NEG16) {
        // return cpu->src.u16 == 0x8000;
        loadLazyFlagsSrc(reg, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x8000, DYN_16bit, DYN_EQUALS, false, false);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32 == 0x80000000;
        loadLazyFlagsSrc(reg, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0x80000000, DYN_32bit, DYN_EQUALS, false, false);
    }
}

static DynWidth getWidthOfCondition(const LazyFlags* flags) {
    if (flags->width==32)
        return DYN_32bit;
    if (flags->width==16)
        return DYN_16bit;
    if (flags->width==8)
        return DYN_8bit;
    kpanic_fmt("getWidthOfCondition: invalid flag width: %d", flags->width);
    return DYN_32bit;
}

void DynamicData::genNZ(const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        loadLazyFlagsResult(reg, DYN_32bit);
    } else if (width==DYN_16bit) {
        loadLazyFlagsResult(reg, DYN_16bit);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);
    } else if (width==DYN_8bit) {
        loadLazyFlagsResult(reg, DYN_8bit);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

void DynamicData::genZ(const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        loadLazyFlagsResult(reg, DYN_32bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
    } else if (width==DYN_16bit) {
        loadLazyFlagsResult(reg, DYN_16bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_16bit, DYN_EQUALS, false, false);
    } else if (width==DYN_8bit) {
        loadLazyFlagsResult(reg, DYN_8bit);
        evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_8bit, DYN_EQUALS, false, false);
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

void DynamicData::genS(const LazyFlags* flags, DynReg reg) {
    DynWidth width = getWidthOfCondition(flags);
    if (width==DYN_32bit) {
        loadLazyFlagsResult(reg, DYN_32bit);
        andRegImm(reg, DYN_32bit, 0x80000000);
    } else if (width==DYN_16bit) {
        loadLazyFlagsResult(reg, DYN_16bit);
        andRegImm(reg, DYN_16bit, 0x8000);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_16bit, false);            
    } else if (width==DYN_8bit) {
        loadLazyFlagsResult(reg, DYN_8bit);
        andRegImm(reg, DYN_8bit, 0x80);
        movToRegFromReg(reg, DYN_32bit, reg, DYN_8bit, false);            
    } else {
        kpanic_fmt("setConditionInReg: unknown condition width: %d", width);
    }
}

bool DynamicData::getFlagInReg(DynConditional condition, DynReg reg) {
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

    loadCPUFlags(reg);
    if (notFlag) {
        notReg(reg, DYN_32bit);
    }
    andRegImm(reg, DYN_32bit, flag);
    return true;
}

void DynamicData::getCondition( DynConditional condition, DynReg reg) {
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
void DynamicData::setConditionInReg(DynConditional condition, DynReg reg) {
    if ((regUsed[DYN_DEST] && reg != DYN_DEST) || (regUsed[DYN_SRC] && reg != DYN_SRC)) {
        kpanic("setConditionInReg expects DYN_DEST to be available");
    }
    if (!currentLazyFlags || condition == P || condition == NP) {
        getCondition(condition, reg);
        return;
    }    
    loadLazyFlags(reg);
    IfPtrEqual(reg, (DYN_PTR_SIZE)currentLazyFlags, false);
    if (currentLazyFlags==FLAGS_NONE) {
        if (!getFlagInReg(condition, reg)) {
            getCondition(condition, reg);
        }
    } else if (condition==NZ) {
        genNZ(currentLazyFlags, reg);
    } else if (condition==Z) {
        genZ(currentLazyFlags, reg);
    } else if (condition==S) {
        genS(currentLazyFlags, reg);
    } else if (condition==B) {
        genCF(currentLazyFlags, reg);
    } else if (condition==O) {
        genOF(currentLazyFlags, reg);
    } else if (condition==BE) {
        if (currentLazyFlags==FLAGS_SUB8) {
            loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
            loadLazyFlagsDst(DYN_DEST, DYN_8bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        } else if (currentLazyFlags==FLAGS_SUB16) {
            loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
            loadLazyFlagsDst(DYN_DEST, DYN_16bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        } else if (currentLazyFlags==FLAGS_SUB32) {
            loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
            loadLazyFlagsDst(DYN_DEST, DYN_32bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_EQUAL_UNSIGNED, true, true);
        }  else {
            // cpu->getZF() || cpu->getCF()
            genCF(currentLazyFlags, reg);
            // must come after genCF, since genCF can clobber DYN_SRC
            genZ(currentLazyFlags, DYN_SRC);
            orRegReg(reg, DYN_SRC, DYN_32bit, true);
        }
    } else if (condition==L) {
        if (currentLazyFlags==FLAGS_SUB8) {
            loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
            loadLazyFlagsDst(DYN_DEST, DYN_8bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_SIGNED, true, true);
        } else if (currentLazyFlags==FLAGS_SUB16) {
            loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
            loadLazyFlagsDst(DYN_DEST, DYN_16bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_SIGNED, true, true);
        } else if (currentLazyFlags==FLAGS_SUB32) {
            loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
            loadLazyFlagsDst(DYN_DEST, DYN_32bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_SIGNED, true, true);
        } else {
            // cpu->getSF()!=cpu->getOF()        
            genOF(currentLazyFlags, reg);
            genS(currentLazyFlags, DYN_SRC);
            evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(reg, DYN_32bit, reg, false, DYN_SRC, 0, DYN_32bit, DYN_NOT_EQUALS, false, true);
        }
    } else if (condition==LE) {
        if (currentLazyFlags==FLAGS_SUB8) {
            loadLazyFlagsSrc(DYN_SRC, DYN_8bit);
            loadLazyFlagsDst(DYN_DEST, DYN_8bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_8bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else if (currentLazyFlags==FLAGS_SUB16) {
            loadLazyFlagsSrc(DYN_SRC, DYN_16bit);
            loadLazyFlagsDst(DYN_DEST, DYN_16bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_16bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else if (currentLazyFlags==FLAGS_SUB32) {
            loadLazyFlagsSrc(DYN_SRC, DYN_32bit);
            loadLazyFlagsDst(DYN_DEST, DYN_32bit);
            evaluateToReg(reg, DYN_32bit, DYN_DEST, false, DYN_SRC, 0, DYN_32bit, DYN_LESS_THAN_EQUAL_SIGNED, true, true);
        } else {
            // cpu->getZF() || cpu->getSF()!=cpu->getOF()       
            genOF(currentLazyFlags, reg);
            genS(currentLazyFlags, DYN_SRC);
            evaluateToReg(reg, DYN_32bit, reg, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(DYN_SRC, DYN_32bit, DYN_SRC, true, DYN_NOT_SET, 0, DYN_32bit, DYN_EQUALS, false, false);
            evaluateToReg(reg, DYN_32bit, reg, false, DYN_SRC, 0, DYN_32bit, DYN_NOT_EQUALS, false, true);
            genZ(currentLazyFlags, DYN_SRC);
            orRegReg(reg, DYN_SRC, DYN_32bit, true);
        }
    } else {
        kpanic("setConditionInReg unhandled condition");
    }
    StartElse();
    getCondition(condition, reg);
    EndIf();
}

void DynamicData::dynamic_pushReg32(DynReg reg, bool doneWithReg) {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        loadReg(4, DYN_ADDRESS, DYN_32bit, true); // need ESP in tmp reg so that we don't commit it until after write
        subRegImm(DYN_ADDRESS, DYN_32bit, 4);
        movToMemFromReg(DYN_ADDRESS, reg, DYN_32bit, true, doneWithReg, DYN_DEST); // need to discard DYN_ADDRESS, otherwise will be out of regs
        subCPUImm(4, DYN_32bit, 4, DYN_ADDRESS);
    } else {
        callHostFunction((void*)common_push32, false, 2, 0, DYN_PARAM_CPU, false, DYN_SRC, DYN_PARAM_REG_32, doneWithReg);
    }
}

void DynamicData::dynamic_pop32() {
    if (!cpu->thread->process->hasSetStackMask && !cpu->thread->process->hasSetSeg[SS]) {
        DynReg reg = loadReg(4, DYN_ADDRESS, DYN_32bit);
        movFromMem(DYN_32bit, reg, true);
        addCPUImm(4, DYN_32bit, 4, DYN_DEST);
    } else {
        callHostFunction((void*)common_pop32, true, 1, 0, DYN_PARAM_CPU, false);
    }    
}

void DynamicData::dynamic_fillFlags() {
    callHostFunction((void*)common_fillFlags, false, 1, 0, DYN_PARAM_CPU, false);
    currentLazyFlags=FLAGS_NONE;
}

void DynamicData::dynamic_getCF() {
    if (currentLazyFlags) {
        loadLazyFlags(DYN_DEST);
        IfPtrEqual(DYN_DEST, (DYN_PTR_SIZE)currentLazyFlags, true);
        genCF(currentLazyFlags, DYN_CALL_RESULT);
        StartElse();
        callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
        EndIf();
    } else {
        callHostFunction((void*)common_getCF, true, 1, 0, DYN_PARAM_CPU, false);
    }
}
