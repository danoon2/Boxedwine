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

static JitWidth getWidthOfFlags(const LazyFlags* flags) {
    if (flags->width == 32)
        return JitWidth::b32;
    if (flags->width == 16)
        return JitWidth::b16;
    if (flags->width == 8)
        return JitWidth::b8;
    kpanic_fmt("getWidthOfCondition: invalid flag width: %d", flags->width);
    return JitWidth::b32;
}

void JitCodeGen::genOF(const LazyFlags* flags, RegPtr result) {
    if (flags == FLAGS_NONE) {
        readCPU(JitWidth::b32, offsetof(CPU, flags), result);
        shrValue(JitWidth::b32, result, 11);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_ADD8 || flags == FLAGS_ADC8) {
        // ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;        
        RegPtr src = getFlagSrcTmp();
        xorReg(JitWidth::b32, getFlagResultTmp(result), src);
        xorReg(JitWidth::b32, src, getFlagDestReadOnly());
        xorValue(JitWidth::b32, src, 0x80);        
        andReg(JitWidth::b32, result, src);
        movzx(JitWidth::b32, result, JitWidth::b8, result);
        shrValue(JitWidth::b32, result, 7);
    } else if (flags == FLAGS_ADD16 || flags == FLAGS_ADC16) {
        // ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;
        RegPtr src = getFlagSrcTmp();
        xorReg(JitWidth::b32, getFlagResultTmp(result), src);
        xorReg(JitWidth::b32, src, getFlagDestReadOnly());
        xorValue(JitWidth::b32, src, 0x8000);
        andReg(JitWidth::b32, result, src);
        movzx(JitWidth::b32, result, JitWidth::b16, result);
        shrValue(JitWidth::b32, result, 15);
    } else if (flags == FLAGS_ADD32 || flags == FLAGS_ADC32) {
        // ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;
        RegPtr src = getFlagSrcTmp();
        xorReg(JitWidth::b32, getFlagResultTmp(result), src);
        xorReg(JitWidth::b32, src, getFlagDestReadOnly());
        xorValue(JitWidth::b32, src, 0x80000000);
        andReg(JitWidth::b32, result, src);
        shrValue(JitWidth::b32, result, 31);
    } else if (flags == FLAGS_OR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_OR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_OR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_SUB8 || flags == FLAGS_SBB8 || flags == FLAGS_CMP8) {
        // ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;
        RegPtr dst = getFlagDestTmp();
        xorReg(JitWidth::b32, getFlagResultTmp(result), dst);
        xorReg(JitWidth::b32, dst, getFlagSrcReadOnly());
        andReg(JitWidth::b32, result, dst);
        movzx(JitWidth::b32, result, JitWidth::b8, result);
        shrValue(JitWidth::b32, result, 7);
    } else if (flags == FLAGS_SUB16 || flags == FLAGS_SBB16 || flags == FLAGS_CMP16) {
        // ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;
        RegPtr dst = getFlagDestTmp();
        xorReg(JitWidth::b32, getFlagResultTmp(result), dst);
        xorReg(JitWidth::b32, dst, getFlagSrcReadOnly());
        andReg(JitWidth::b32, result, dst);
        movzx(JitWidth::b32, result, JitWidth::b16, result);
        shrValue(JitWidth::b32, result, 15);
    } else if (flags == FLAGS_SUB32 || flags == FLAGS_SBB32 || flags == FLAGS_CMP32) {
        // ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;
        RegPtr dst = getFlagDestTmp();
        xorReg(JitWidth::b32, getFlagResultTmp(result), dst);
        xorReg(JitWidth::b32, dst, getFlagSrcReadOnly());
        andReg(JitWidth::b32, result, dst);
        shrValue(JitWidth::b32, result, 31);
    } else if (flags == FLAGS_XOR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_XOR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_XOR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_INC8) {
        // cpu->result.u8 == 0x80;
        compareValue(JitWidth::b8, getFlagResultReadOnly(), 0x80, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_INC16) {
        // cpu->result.u16 == 0x8000;
        compareValue(JitWidth::b16, getFlagResultReadOnly(), 0x8000, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_INC32) {
        // cpu->result.u32 == 0x80000000;
        compareValue(JitWidth::b32, getFlagResultReadOnly(), 0x80000000, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_DEC8) {
        // cpu->result.u8 == 0x7f;
        compareValue(JitWidth::b8, getFlagResultReadOnly(), 0x7f, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_DEC16) {
        // cpu->result.u16 == 0x7fff;
        compareValue(JitWidth::b16, getFlagResultReadOnly(), 0x7fff, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_DEC32) {
        // cpu->result.u32 == 0x7fffffff;
        compareValue(JitWidth::b32, getFlagResultReadOnly(), 0x7fffffff, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_SHL8) {
        // (cpu->result.u8 ^ cpu->dst.u8) & 0x80;
        xorReg(JitWidth::b32, getFlagResultTmp(result), getFlagDestReadOnly());
        movzx(JitWidth::b32, result, JitWidth::b8, result);
        shrValue(JitWidth::b32, result, 7);
    } else if (flags == FLAGS_SHL16 || flags == FLAGS_DSHL16 || flags == FLAGS_DSHR16) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        xorReg(JitWidth::b32, getFlagResultTmp(result), getFlagDestReadOnly());
        movzx(JitWidth::b32, result, JitWidth::b16, result);
        shrValue(JitWidth::b32, result, 15);
    } else if (flags == FLAGS_SHL32 || flags == FLAGS_DSHL32 || flags == FLAGS_DSHR32) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        xorReg(JitWidth::b32, getFlagResultTmp(result), getFlagDestReadOnly());
        shrValue(JitWidth::b32, result, 31);
    } else if (flags == FLAGS_SHR8) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;

        // undefined if src & 0x1f != 1, since its undefined, I don't see a reason to return 0 for that case
        getFlagSrcReadOnly(result);
        movzx(JitWidth::b32, result, JitWidth::b8, result);
        sarValue(JitWidth::b32, result, 7);        
    } else if (flags == FLAGS_SHR16) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;
        
        // undefined if src & 0x1f != 1, since its undefined, I don't see a reason to return 0 for that case
        getFlagSrcReadOnly(result);
        movzx(JitWidth::b32, result, JitWidth::b16, result);
        sarValue(JitWidth::b32, result, 15);
    } else if (flags == FLAGS_SHR32) {
        // if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;
        
        // undefined if src & 0x1f != 1, since its undefined, I don't see a reason to return 0 for that case
        getFlagSrcReadOnly(result);
        sarValue(JitWidth::b32, result, 31);
    } else if (flags == FLAGS_SAR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_SAR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_SAR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8 == 0x80;
        compareValue(JitWidth::b8, getFlagSrcReadOnly(), 0x80, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_NEG16) {
        // return cpu->src.u16 == 0x8000;
        compareValue(JitWidth::b16, getFlagSrcReadOnly(), 0x8000, JitEvaluate::EQUALS, result);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32 == 0x80000000;
        compareValue(JitWidth::b32, getFlagSrcReadOnly(), 0x80000000, JitEvaluate::EQUALS, result);
    } else {
        kpanic("genOF unknown flags");
    }
}

static U8 parity_lookup[256] = {
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

void JitCodeGen::genPF(const LazyFlags* flags, RegPtr result) {
    getFlagResultTmp(result);
    movzx(JitWidth::b32, result, JitWidth::b8, result);
    read(JitWidth::b8, result, result, 0, (U32)parity_lookup);    
}

void JitCodeGen::genCF(const LazyFlags* flags, RegPtr result) {    
    if (flags == FLAGS_NONE) {
        readCPU(JitWidth::b32, offsetof(CPU, flags), result);
        andValue(JitWidth::b32, result, CF);
    } else if (flags == FLAGS_ADD8) {
        // cpu->result.u8<cpu->dst.u8;
        compareReg(JitWidth::b8, getFlagResultReadOnly(result), getFlagDestReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_ADD16) {
        // cpu->result.u16<cpu->dst.u16;
        compareReg(JitWidth::b16, getFlagResultReadOnly(result), getFlagDestReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_ADD32) {
        // cpu->result.u32<cpu->dst.u32;
        compareReg(JitWidth::b32, getFlagResultReadOnly(result), getFlagDestReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_OR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_OR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_OR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_ADC8) {
        // (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8));
        RegPtr fResult = getFlagResultTmp();
        RegPtr fDest = getFlagDestReadOnly();
        compareReg(JitWidth::b8, fResult, fDest, JitEvaluate::LESS_THAN_UNSIGNED, result);
        compareReg(JitWidth::b8, fResult, fDest, JitEvaluate::EQUALS, fResult);
        fDest = nullptr;
        andReg(JitWidth::b32, fResult, getFlagCF());
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_ADC16) {
        // (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));
        RegPtr fResult = getFlagResultTmp();
        RegPtr fDest = getFlagDestReadOnly();
        compareReg(JitWidth::b16, fResult, fDest, JitEvaluate::LESS_THAN_UNSIGNED, result);
        compareReg(JitWidth::b16, fResult, fDest, JitEvaluate::EQUALS, fResult);
        fDest = nullptr;
        andReg(JitWidth::b32, fResult, getFlagCF());
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_ADC32) {
        // (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));
        RegPtr fResult = getFlagResultTmp();
        RegPtr fDest = getFlagDestReadOnly();        
        compareReg(JitWidth::b32, fResult, fDest, JitEvaluate::LESS_THAN_UNSIGNED, result);
        compareReg(JitWidth::b32, fResult, fDest, JitEvaluate::EQUALS, fResult);
        fDest = nullptr;
        andReg(JitWidth::b32, fResult, getFlagCF());
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_SBB8) {
        // (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8==0xff));
        compareReg(JitWidth::b8, getFlagDestReadOnly(), getFlagResultReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
        RegPtr fResult = compareValue(JitWidth::b8, getFlagSrcReadOnly(), 0xff, JitEvaluate::EQUALS);
        andReg(JitWidth::b32, fResult, getFlagCF());
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_SBB16) {
        // (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16==0xffff));
        compareReg(JitWidth::b16, getFlagDestReadOnly(), getFlagResultReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
        RegPtr fResult = compareValue(JitWidth::b16, getFlagSrcReadOnly(), 0xffff, JitEvaluate::EQUALS);
        andReg(JitWidth::b32, fResult, getFlagCF());
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_SBB32) {
        // (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32==0xffffffff));
        compareReg(JitWidth::b32, getFlagDestReadOnly(), getFlagResultReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
        RegPtr fResult = compareValue(JitWidth::b32, getFlagSrcReadOnly(), 0xffffffff, JitEvaluate::EQUALS);
        andReg(JitWidth::b32, fResult, getFlagCF());
        orReg(JitWidth::b32, result, fResult);
    } else if (flags == FLAGS_AND8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_AND32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_SUB8) {
        // cpu->dst.u8<cpu->src.u8;
        compareReg(JitWidth::b8, getFlagDestReadOnly(result), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_SUB16) {
        // cpu->dst.u16<cpu->src.u16;
        compareReg(JitWidth::b16, getFlagDestReadOnly(result), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_SUB32) {
        // cpu->dst.u32<cpu->src.u32;
        compareReg(JitWidth::b32, getFlagDestReadOnly(result), getFlagSrcReadOnly(), JitEvaluate::LESS_THAN_UNSIGNED, result);
    } else if (flags == FLAGS_XOR8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_XOR16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_XOR32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_INC8) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_INC16) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_INC32) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_DEC8) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_DEC16) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_DEC32) {
        // cpu->oldCF;
        getFlagCF(result);
    } else if (flags == FLAGS_SHL8) {
        // last bit that was shifted out
        // ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        movValue(JitWidth::b32, reg, 8);
        subReg(JitWidth::b32, reg, getFlagSrcReadOnly());
        shrReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SHL16 || flags == FLAGS_DSHL16) {
        // ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        movValue(JitWidth::b32, reg, 16);
        subReg(JitWidth::b32, reg, getFlagSrcReadOnly());
        shrReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SHL32 || flags == FLAGS_DSHL32) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        movValue(JitWidth::b32, reg, 32);
        subReg(JitWidth::b32, reg, getFlagSrcReadOnly());
        shrReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SHR8 || flags == FLAGS_SHR16 || flags == FLAGS_SHR32) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        subValue(JitWidth::b32, getFlagSrcTmp(reg), 1);            
        shrReg(getWidthOfFlags(flags), result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_DSHR16 || flags == FLAGS_DSHR32) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        subValue(JitWidth::b32, getFlagSrcTmp(reg), 1);
        shrReg(JitWidth::b32, result, reg); // intentional 32-bit
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SAR8) {
        // (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        subValue(JitWidth::b32, getFlagSrcTmp(reg), 1);
        sarReg(JitWidth::b8, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SAR16) {
        // (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        subValue(JitWidth::b32, getFlagSrcTmp(reg), 1);
        sarReg(JitWidth::b16, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_SAR32) {
        // (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;
        result = getFlagDestTmp(result);
        RegPtr reg = getTmpRegWithHint(1);
        subValue(JitWidth::b32, getFlagSrcTmp(reg), 1);
        sarReg(JitWidth::b32, result, reg);
        andValue(JitWidth::b32, result, 1);
    } else if (flags == FLAGS_TEST8) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST16) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_TEST32) {
        // 0
        xorReg(JitWidth::b32, result, result);
    } else if (flags == FLAGS_NEG8) {
        // cpu->src.u8!=0;
        compareValue(JitWidth::b8, getFlagSrcReadOnly(), 0, JitEvaluate::NOT_EQUALS, result);
    } else if (flags == FLAGS_NEG16) {
        // cpu->src.u16!=0;
        compareValue(JitWidth::b16, getFlagSrcReadOnly(), 0, JitEvaluate::NOT_EQUALS, result);
    } else if (flags == FLAGS_NEG32) {
        // cpu->src.u32!=0;
        compareValue(JitWidth::b32, getFlagSrcReadOnly(), 0, JitEvaluate::NOT_EQUALS, result);
    } else {
        kpanic("genCF unknown flags");
    }
}