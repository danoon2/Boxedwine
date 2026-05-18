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

#include "boxedwine.h"

#include "../decoder.h"

U8 parity_lookup[256] = {
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  0, PF, PF, 0, PF, 0, 0, PF, PF, 0, 0, PF, 0, PF, PF, 0,
  PF, 0, 0, PF, 0, PF, PF, 0, 0, PF, PF, 0, PF, 0, 0, PF
  };

class LazyFlagsNone : public LazyFlags {
public:
    LazyFlagsNone(U32 width) : LazyFlags(width) {}
    U32 getCF(CPU* cpu) const override {return cpu->flags & CF;}
    U32 getSF(CPU* cpu) const override {return cpu->flags & SF;}
    U32 getZF(CPU* cpu) const override {return cpu->flags & ZF;}
    U32 getOF(CPU* cpu) const override {return cpu->flags & OF;}
    U32 getAF(CPU* cpu) const override {return cpu->flags & AF;}
    U32 getPF(CPU* cpu) const override {return cpu->flags & PF;}
    bool usesResult(U32 mask) const override { return false; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsNone flagsNone(0);
static const LazyFlags* flagsNONE = &flagsNone;

class LazyFlagsDefault : public LazyFlags {
public:
    LazyFlagsDefault(U32 width) : LazyFlags(width) {}
    U32 getPF(CPU* cpu) const override {return parity_lookup[cpu->result.u8];}
};

class LazyFlagsDefault8 : public LazyFlagsDefault {
public:
    LazyFlagsDefault8() : LazyFlagsDefault(8) {}
    U32 getSF(CPU* cpu) const override {return cpu->result.u8 & 0x80;}
    U32 getZF(CPU* cpu) const override {return cpu->result.u8==0;}
};

class LazyFlagsDefault16 : public LazyFlagsDefault {
public:
    LazyFlagsDefault16() : LazyFlagsDefault(16) {}
    U32 getSF(CPU* cpu) const override {return cpu->result.u16 & 0x8000;}
    U32 getZF(CPU* cpu) const override {return cpu->result.u16==0;}
};

class LazyFlagsDefault32 : public LazyFlagsDefault {
public:
    LazyFlagsDefault32() : LazyFlagsDefault(32) {}
    U32 getSF(CPU* cpu) const override {return cpu->result.u32 & 0x80000000;}
    U32 getZF(CPU* cpu) const override {return cpu->result.u32==0;}
};

class LazyFlagsAdd8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->result.u8<cpu->dst.u8;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | CF | OF | AF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsAdd8 flagsAdd8;
static const LazyFlags* flagsADD8 = &flagsAdd8;

class LazyFlagsAdd16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->result.u16<cpu->dst.u16;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | CF | OF | AF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsAdd16 flagsAdd16;
static const LazyFlags* flagsADD16 = &flagsAdd16;

class LazyFlagsAdd32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->result.u32<cpu->dst.u32;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | CF | OF | AF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsAdd32 flagsAdd32;
static const LazyFlags* flagsADD32 = &flagsAdd32;

U32 get_0(CPU* cpu) {return 0;}

class LazyFlagsZero8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return 0;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

class LazyFlagsZero16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return 0;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

class LazyFlagsZero32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return 0;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsZero8 flags0_8;
static const LazyFlags* flagsOR8 = &flags0_8;
static const LazyFlags* flagsAND8 = &flags0_8;
static const LazyFlags* flagsXOR8 = &flags0_8;
static const LazyFlags* flagsTEST8 = &flags0_8;

static LazyFlagsZero16 flags0_16;
static const LazyFlags* flagsOR16 = &flags0_16;
static const LazyFlags* flagsAND16 = &flags0_16;
static const LazyFlags* flagsXOR16 = &flags0_16;
static const LazyFlags* flagsTEST16 = &flags0_16;

static LazyFlagsZero32 flags0_r32;
static const LazyFlags* flagsOR32 = &flags0_r32;
static const LazyFlags* flagsAND32 = &flags0_r32;
static const LazyFlags* flagsXOR32 = &flags0_r32;
static const LazyFlags* flagsTEST32 = &flags0_r32;

class LazyFlagsAdc8 : public LazyFlagsAdd8 {
    U32 getCF(CPU* cpu) const override {return (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8));}
    bool usesOldCF(U32 mask) const override { return (mask & CF) != 0; }
};

static LazyFlagsAdc8 flagsAdc8;
static const LazyFlags* flagsADC8 = &flagsAdc8;

class LazyFlagsAdc16 : public LazyFlagsAdd16 {
    U32 getCF(CPU* cpu) const override {return (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));}
    bool usesOldCF(U32 mask) const override { return (mask & CF) != 0; }
};

static LazyFlagsAdc16 flagsAdc16;
static const LazyFlags* flagsADC16 = &flagsAdc16;

class LazyFlagsAdc32 : public LazyFlagsAdd32 {
    U32 getCF(CPU* cpu) const override {return (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));}
    bool usesOldCF(U32 mask) const override { return (mask & CF) != 0; }
};

static LazyFlagsAdc32 flagsAdc32;
static const LazyFlags* flagsADC32 = &flagsAdc32;

class LazyFlagsSub8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u8<cpu->src.u8;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | OF | AF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsSub8 flagsSub8;
static const LazyFlags* flagsSUB8 = &flagsSub8;
static const LazyFlags* flagsCMP8 = &flagsSub8;

class LazyFlagsSub16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u16<cpu->src.u16;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | OF | AF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsSub16 flagsSub16;
static const LazyFlags* flagsSUB16 = &flagsSub16;
static const LazyFlags* flagsCMP16 = &flagsSub16;

class LazyFlagsSub32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u32<cpu->src.u32;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | OF | AF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsSub32 flagsSub32;
static const LazyFlags* flagsSUB32 = &flagsSub32;
static const LazyFlags* flagsCMP32 = &flagsSub32;

class LazyFlagsSbb8 : public LazyFlagsSub8 {
    U32 getCF(CPU* cpu) const override { return (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8 == 0xff)); }
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | OF | AF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return (mask & CF) != 0; }
};

static LazyFlagsSbb8 flagsSbb8;
static const LazyFlags* flagsSBB8 = &flagsSbb8;

class LazyFlagsSbb16 : public LazyFlagsSub16 {
    U32 getCF(CPU* cpu) const override { return (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16 == 0xffff)); }
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | OF | AF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return (mask & CF) != 0; }
};

static LazyFlagsSbb16 flagsSbb16;
static const LazyFlags* flagsSBB16 = &flagsSbb16;

class LazyFlagsSbb32 : public LazyFlagsSub32 {
    U32 getCF(CPU* cpu) const override { return (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32 == 0xffffffff)); }
    bool usesResult(U32 mask) const override { return (mask & (ZF | PF | SF | OF | AF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return (mask & CF) != 0; }
};

static LazyFlagsSbb32 flagsSbb32;
static const LazyFlags* flagsSBB32 = &flagsSbb32;

class LazyFlagsInc8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u8 == 0x80;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u8 & 0x0f) == 0;}
    bool usesResult(U32 mask) const override { return (mask & (AF | PF | ZF | SF | OF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    // getCF() returns cpu->oldCF unconditionally; oldCF must always be saved
    // so fillFlags() preserves CF across block boundaries (INC/DEC don't set CF).
    bool usesOldCF(U32 mask) const override { return true; }
};

static LazyFlagsInc8 flagsInc8;
static const LazyFlags* flagsINC8 = &flagsInc8;

class LazyFlagsInc16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u16 == 0x8000;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u16 & 0x0f) == 0;}
    bool usesResult(U32 mask) const override { return (mask & (AF | PF | ZF | SF | OF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return true; }
};

static LazyFlagsInc16 flagsInc16;
static const LazyFlags* flagsINC16 = &flagsInc16;

class LazyFlagsInc32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u32 == 0x80000000;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u32 & 0x0f) == 0;}
    bool usesResult(U32 mask) const override { return (mask & (AF | PF | ZF | SF | OF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return true; }
};

static LazyFlagsInc32 flagsInc32;
static const LazyFlags* flagsINC32 = &flagsInc32;

class LazyFlagsDec8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u8 == 0x7f;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u8 & 0x0f) == 0x0f;}
    bool usesResult(U32 mask) const override { return (mask & (AF | PF | ZF | SF | OF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return true; }
};

static LazyFlagsDec8 flagsDec8;
static const LazyFlags* flagsDEC8 = &flagsDec8;

class LazyFlagsDec16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u16 == 0x7fff;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u16 & 0x0f) == 0x0f;}
    bool usesResult(U32 mask) const override { return (mask & (AF | PF | ZF | SF | OF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return true; }
};

static LazyFlagsDec16 flagsDec16;
static const LazyFlags* flagsDEC16 = &flagsDec16;

class LazyFlagsDec32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u32 == 0x7fffffff;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u32 & 0x0f) == 0x0f;}
    bool usesResult(U32 mask) const override { return (mask & (AF | PF | ZF | SF | OF)) != 0; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return true; }
};

static LazyFlagsDec32 flagsDec32;
static const LazyFlags* flagsDEC32 = &flagsDec32;

class LazyFlagsNeg8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->src.u8!=0;}
    U32 getOF(CPU* cpu) const override {return cpu->src.u8 == 0x80;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x0f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsNeg8 flagsNeg8;
static const LazyFlags* flagsNEG8 = &flagsNeg8;

class LazyFlagsNeg16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->src.u16!=0;}
    U32 getOF(CPU* cpu) const override {return cpu->src.u16 == 0x8000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x0f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsNeg16 flagsNeg16;
static const LazyFlags* flagsNEG16 = &flagsNeg16;

class LazyFlagsNeg32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->src.u32!=0;}
    U32 getOF(CPU* cpu) const override {return cpu->src.u32 == 0x80000000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x0f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (AF | OF | CF)) != 0; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsNeg32 flagsNeg32;
static const LazyFlags* flagsNEG32 = &flagsNeg32;

class LazyFlagsShl8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u8 ^ cpu->dst.u8) & 0x80;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsShl8 flagsShl8;
static const LazyFlags* flagsSHL8 = &flagsShl8;

class LazyFlagsShl16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsShl16 flagsShl16;
static const LazyFlags* flagsSHL16 = &flagsShl16;

class LazyFlagsShl32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsShl32 flagsShl32;
static const LazyFlags* flagsSHL32 = &flagsShl32;

class LazyFlagsDshl16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u16 >> (16-cpu->src.u8)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & CF) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsDshl16 flagsDshl16;
static const LazyFlags* flagsDSHL16 = &flagsDshl16;

class LazyFlagsDshl32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & CF) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsDshl32 flagsDshl32;
static const LazyFlags* flagsDSHL32 = &flagsDshl32;

class LazyFlagsDshr16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;} // dst is intentionally 32 bit
    U32 getOF(CPU* cpu) const override {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & CF) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsDshr16 flagsDshr16;
static const LazyFlags* flagsDSHR16 = &flagsDshr16;

class LazyFlagsDshr32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return 0;}
    bool usesResult(U32 mask) const override { return (mask & (OF | PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & CF) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsDshr32 flagsDshr32;
static const LazyFlags* flagsDSHR32 = &flagsDshr32;

class LazyFlagsShr8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {if ((cpu->src.u8 & 0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (OF | CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsShr8 flagsShr8;
static const LazyFlags* flagsSHR8 = &flagsShr8;

class LazyFlagsShr16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (OF | CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsShr16 flagsShr16;
static const LazyFlags* flagsSHR16 = &flagsShr16;

class LazyFlagsShr32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {if ((cpu->src.u8 & 0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (OF | CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (OF | CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsShr32 flagsShr32;
static const LazyFlags* flagsSHR32 = &flagsShr32;

class LazyFlagsSar8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsSar8 flagsSar8;
static const LazyFlags* flagsSAR8 = &flagsSar8;

class LazyFlagsSar16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsSar16 flagsSar16;
static const LazyFlags* flagsSAR16 = &flagsSar16;

class LazyFlagsSar32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
    bool usesResult(U32 mask) const override { return (mask & (PF | ZF | SF)) != 0; }
    bool usesSrc(U32 mask) const override { return (mask & (CF | AF)) != 0; }
    bool usesDst(U32 mask) const override { return (mask & (CF)) != 0; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsSar32 flagsSar32;
static const LazyFlags* flagsSAR32 = &flagsSar32;

class LazyFlagsCFOF : public LazyFlags {
public:
    LazyFlagsCFOF() : LazyFlags(0) {}
    U32 getCF(CPU* cpu) const override { return cpu->flags & CF; }
    U32 getOF(CPU* cpu) const override { return cpu->flags & OF; }

    U32 getSF(CPU* cpu) const override { return lazyFlags[cpu->lazyFlagTypePrev]->getSF(cpu); }
    U32 getZF(CPU* cpu) const override { return lazyFlags[cpu->lazyFlagTypePrev]->getZF(cpu); }
    U32 getAF(CPU* cpu) const override { return lazyFlags[cpu->lazyFlagTypePrev]->getAF(cpu); }
    U32 getPF(CPU* cpu) const override { return lazyFlags[cpu->lazyFlagTypePrev]->getPF(cpu); }

    bool usesResult(U32 mask) const override { return false; }
    bool usesSrc(U32 mask) const override { return false; }
    bool usesDst(U32 mask) const override { return false; }
    bool usesOldCF(U32 mask) const override { return false; }
};

static LazyFlagsCFOF flagsCFOF;

const LazyFlags* lazyFlags[] = {
    flagsNONE,
    flagsADD8,
    flagsADD16,
    flagsADD32,
    flagsOR8,
    flagsOR16,
    flagsOR32,
    flagsADC8,
    flagsADC16,
    flagsADC32,
    flagsSBB8,
    flagsSBB16,
    flagsSBB32,
    flagsAND8,
    flagsAND16,
    flagsAND32,
    flagsSUB8,
    flagsSUB16,
    flagsSUB32,
    flagsXOR8,
    flagsXOR16,
    flagsXOR32,
    flagsINC8,
    flagsINC16,
    flagsINC32,
    flagsDEC8,
    flagsDEC16,
    flagsDEC32,
    flagsSHL8,
    flagsSHL16,
    flagsSHL32,
    flagsSHR8,
    flagsSHR16,
    flagsSHR32,
    flagsSAR8,
    flagsSAR16,
    flagsSAR32,
    flagsCMP8,
    flagsCMP16,
    flagsCMP32,
    flagsTEST8,
    flagsTEST16,
    flagsTEST32,
    flagsDSHL16,
    flagsDSHL32,
    flagsDSHR16,
    flagsDSHR32,
    flagsNEG8,
    flagsNEG16,
    flagsNEG32,
    &flagsCFOF,
    nullptr
};