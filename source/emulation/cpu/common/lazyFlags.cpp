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
};

static LazyFlagsNone flagsNone(0);
const LazyFlags* FLAGS_NONE = &flagsNone;

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
};

static LazyFlagsAdd8 flagsAdd8;
const LazyFlags* FLAGS_ADD8 = &flagsAdd8;

class LazyFlagsAdd16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->result.u16<cpu->dst.u16;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}
};

static LazyFlagsAdd16 flagsAdd16;
const LazyFlags* FLAGS_ADD16 = &flagsAdd16;

class LazyFlagsAdd32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->result.u32<cpu->dst.u32;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}
};

static LazyFlagsAdd32 flagsAdd32;
const LazyFlags* FLAGS_ADD32 = &flagsAdd32;

U32 get_0(CPU* cpu) {return 0;}

class LazyFlagsZero8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return 0;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

class LazyFlagsZero16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return 0;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

class LazyFlagsZero32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return 0;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

static LazyFlagsZero8 flags0_8;
const LazyFlags* FLAGS_OR8 = &flags0_8;
const LazyFlags* FLAGS_AND8 = &flags0_8;
const LazyFlags* FLAGS_XOR8 = &flags0_8;
const LazyFlags* FLAGS_TEST8 = &flags0_8;

static LazyFlagsZero16 flags0_16;
const LazyFlags* FLAGS_OR16 = &flags0_16;
const LazyFlags* FLAGS_AND16 = &flags0_16;
const LazyFlags* FLAGS_XOR16 = &flags0_16;
const LazyFlags* FLAGS_TEST16 = &flags0_16;

static LazyFlagsZero32 flags0_r32;
const LazyFlags* FLAGS_OR32 = &flags0_r32;
const LazyFlags* FLAGS_AND32 = &flags0_r32;
const LazyFlags* FLAGS_XOR32 = &flags0_r32;
const LazyFlags* FLAGS_TEST32 = &flags0_r32;

class LazyFlagsAdc8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8));}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}
};

static LazyFlagsAdc8 flagsAdc8;
const LazyFlags* FLAGS_ADC8 = &flagsAdc8;

class LazyFlagsAdc16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16));}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}
};

static LazyFlagsAdc16 flagsAdc16;
const LazyFlags* FLAGS_ADC16 = &flagsAdc16;

class LazyFlagsAdc32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}
};

static LazyFlagsAdc32 flagsAdc32;
const LazyFlags* FLAGS_ADC32 = &flagsAdc32;

class LazyFlagsSbb8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8==0xff));}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}
};

static LazyFlagsSbb8 flagsSbb8;
const LazyFlags* FLAGS_SBB8 = &flagsSbb8;

class LazyFlagsSbb16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16==0xffff));}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}
};

static LazyFlagsSbb16 flagsSbb16;
const LazyFlags* FLAGS_SBB16 = &flagsSbb16;

class LazyFlagsSbb32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32==0xffffffff));}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}
};

static LazyFlagsSbb32 flagsSbb32;
const LazyFlags* FLAGS_SBB32 = &flagsSbb32;

class LazyFlagsSub8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u8<cpu->src.u8;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;}
};

static LazyFlagsSub8 flagsSub8;
const LazyFlags* FLAGS_SUB8 = &flagsSub8;
const LazyFlags* FLAGS_CMP8 = &flagsSub8;

class LazyFlagsSub16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u16<cpu->src.u16;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;}
};

static LazyFlagsSub16 flagsSub16;
const LazyFlags* FLAGS_SUB16 = &flagsSub16;
const LazyFlags* FLAGS_CMP16 = &flagsSub16;

class LazyFlagsSub32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u32<cpu->src.u32;}
    U32 getOF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;}
};

static LazyFlagsSub32 flagsSub32;
const LazyFlags* FLAGS_SUB32 = &flagsSub32;
const LazyFlags* FLAGS_CMP32 = &flagsSub32;

class LazyFlagsInc8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u8 == 0x80;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u8 & 0x0f) == 0;}
};

static LazyFlagsInc8 flagsInc8;
const LazyFlags* FLAGS_INC8 = &flagsInc8;

class LazyFlagsInc16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u16 == 0x8000;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u16 & 0x0f) == 0;}
};

static LazyFlagsInc16 flagsInc16;
const LazyFlags* FLAGS_INC16 = &flagsInc16;

class LazyFlagsInc32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u32 == 0x80000000;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u32 & 0x0f) == 0;}
};

static LazyFlagsInc32 flagsInc32;
const LazyFlags* FLAGS_INC32 = &flagsInc32;

class LazyFlagsDec8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u8 == 0x7f;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u8 & 0x0f) == 0x0f;}
};

static LazyFlagsDec8 flagsDec8;
const LazyFlags* FLAGS_DEC8 = &flagsDec8;

class LazyFlagsDec16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u16 == 0x7fff;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u16 & 0x0f) == 0x0f;}
};

static LazyFlagsDec16 flagsDec16;
const LazyFlags* FLAGS_DEC16 = &flagsDec16;

class LazyFlagsDec32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->oldCF;}
    U32 getOF(CPU* cpu) const override {return cpu->result.u32 == 0x7fffffff;}
    U32 getAF(CPU* cpu) const override {return (cpu->result.u32 & 0x0f) == 0x0f;}
};

static LazyFlagsDec32 flagsDec32;
const LazyFlags* FLAGS_DEC32 = &flagsDec32;

class LazyFlagsNeg8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->src.u8!=0;}
    U32 getOF(CPU* cpu) const override {return cpu->src.u8 == 0x80;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x0f;}
};

static LazyFlagsNeg8 flagsNeg8;
const LazyFlags* FLAGS_NEG8 = &flagsNeg8;

class LazyFlagsNeg16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->src.u16!=0;}
    U32 getOF(CPU* cpu) const override {return cpu->src.u16 == 0x8000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x0f;}
};

static LazyFlagsNeg16 flagsNeg16;
const LazyFlags* FLAGS_NEG16 = &flagsNeg16;

class LazyFlagsNeg32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->src.u32!=0;}
    U32 getOF(CPU* cpu) const override {return cpu->src.u32 == 0x80000000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x0f;}
};

static LazyFlagsNeg32 flagsNeg32;
const LazyFlags* FLAGS_NEG32 = &flagsNeg32;

class LazyFlagsShl8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return ((cpu->dst.u8 << (cpu->src.u8-1)) & 0x80) >> 7;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u8 ^ cpu->dst.u8) & 0x80;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
};

static LazyFlagsShl8 flagsShl8;
const LazyFlags* FLAGS_SHL8 = &flagsShl8;

class LazyFlagsShl16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return ((cpu->dst.u16 << (cpu->src.u8-1)) & 0x8000)>>15;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
};

static LazyFlagsShl16 flagsShl16;
const LazyFlags* FLAGS_SHL16 = &flagsShl16;

class LazyFlagsShl32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
};

static LazyFlagsShl32 flagsShl32;
const LazyFlags* FLAGS_SHL32 = &flagsShl32;

class LazyFlagsDshl16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u16 >> (16-cpu->src.u8)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

static LazyFlagsDshl16 flagsDshl16;
const LazyFlags* FLAGS_DSHL16 = &flagsDshl16;

class LazyFlagsDshl32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

static LazyFlagsDshl32 flagsDshl32;
const LazyFlags* FLAGS_DSHL32 = &flagsDshl32;

class LazyFlagsDshr16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;} // dst is intentionally 32 bit
    U32 getOF(CPU* cpu) const override {return (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

static LazyFlagsDshr16 flagsDshr16;
const LazyFlags* FLAGS_DSHR16 = &flagsDshr16;

class LazyFlagsDshr32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;}
    U32 getAF(CPU* cpu) const override {return 0;}
};

static LazyFlagsDshr32 flagsDshr32;
const LazyFlags* FLAGS_DSHR32 = &flagsDshr32;

class LazyFlagsShr8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u8 >= 0x80); else return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
};

static LazyFlagsShr8 flagsShr8;
const LazyFlags* FLAGS_SHR8 = &flagsShr8;

class LazyFlagsShr16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u16 >= 0x8000); else return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
};

static LazyFlagsShr16 flagsShr16;
const LazyFlags* FLAGS_SHR16 = &flagsShr16;

class LazyFlagsShr32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {if ((cpu->src.u8&0x1f)==1) return (cpu->dst.u32 >= 0x80000000); else return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
};

static LazyFlagsShr32 flagsShr32;
const LazyFlags* FLAGS_SHR32 = &flagsShr32;

class LazyFlagsShr8_1 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u8 & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->dst.u8 >= 0x80);}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
};

static LazyFlagsShr8_1 flagsShr8_1;
const LazyFlags* FLAGS_SHR8_1 = &flagsShr8_1;

class LazyFlagsShr16_1 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u16 & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->dst.u16 >= 0x8000);}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
};

static LazyFlagsShr16_1 flagsShr16_1;
const LazyFlags* FLAGS_SHR16_1 = &flagsShr16_1;

class LazyFlagsShr32_1 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return cpu->dst.u32 & 1;}
    U32 getOF(CPU* cpu) const override {return (cpu->dst.u32 >= 0x80000000);}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
};

static LazyFlagsShr32_1 flagsShr32_1;
const LazyFlags* FLAGS_SHR32_1 = &flagsShr32_1;

class LazyFlagsShr8_N1 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
};

static LazyFlagsShr8_N1 flagsShr8_N1;
const LazyFlags* FLAGS_SHR8_N1 = &flagsShr8_N1;

class LazyFlagsShr16_N1 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
};

static LazyFlagsShr16_N1 flagsShr16_N1;
const LazyFlags* FLAGS_SHR16_N1 = &flagsShr16_N1;

class LazyFlagsShr32_N1 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
};

static LazyFlagsShr32_N1 flagsShr32_N1;
const LazyFlags* FLAGS_SHR32_N1 = &flagsShr32_N1;

class LazyFlagsSar8 : public LazyFlagsDefault8 {
    U32 getCF(CPU* cpu) const override {return (((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u8 & 0x1f;}
};

static LazyFlagsSar8 flagsSar8;
const LazyFlags* FLAGS_SAR8 = &flagsSar8;

class LazyFlagsSar16 : public LazyFlagsDefault16 {
    U32 getCF(CPU* cpu) const override {return (((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u16 & 0x1f;}
};

static LazyFlagsSar16 flagsSar16;
const LazyFlags* FLAGS_SAR16 = &flagsSar16;

class LazyFlagsSar32 : public LazyFlagsDefault32 {
    U32 getCF(CPU* cpu) const override {return (((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;}
    U32 getOF(CPU* cpu) const override {return 0;}
    U32 getAF(CPU* cpu) const override {return cpu->src.u32 & 0x1f;}
};

static LazyFlagsSar32 flagsSar32;
const LazyFlags* FLAGS_SAR32 = &flagsSar32;
