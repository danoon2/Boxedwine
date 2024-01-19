#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT
#include "armv8btAsm.h"
#include "arm8btFlags.h"
#include "../decoder.h"

U8 arm_parity_lookup[256] = {
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

class Arm8BtFlagsDefault : public Arm8BtFlags {
public:
    Arm8BtFlagsDefault(U32 width) : Arm8BtFlags(width) {}
    virtual void setPF(Armv8btAsm* data, U8 reg) {
        // xFlags &~ PF
        // xFlags |= parity_lookup[xResult.u8]

        U8 tmp = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->zeroExtend(tmp, xResult, 8);
        // :TODO: put arm_parity_lookup on cpu so that we load the pointer with a single call, instead of the 3 or so it takes to load this pointer
        data->loadConst(tmp2, (U64)arm_parity_lookup);
        data->readMem8RegOffset(tmp, tmp, tmp2);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 2, 1); // PF is 0x4 (bit 2)
        data->releaseTmpReg(tmp);
        data->releaseTmpReg(tmp2);
    }
    // override with cseteq like add32 and sub32 if the flags support it
    virtual void setZF(Armv8btAsm* data, U8 reg) {
        U8 tmp = data->getTmpReg();
        data->clz32(tmp, xResult); // will be 32 if result == 0
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1, true); // bit 5 is set for the number 32, which means all bits are set to 0
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 6, 1); // ZF is 0x40 (bit 6)
        data->releaseTmpReg(tmp);
    }
};

class Arm8BtFlagsDefault8 : public Arm8BtFlagsDefault {
public:
    Arm8BtFlagsDefault8() : Arm8BtFlagsDefault(8) {}
    virtual void setPF(Armv8btAsm* data, U8 reg) {
        // xFlags &~ PF
        // xFlags |= parity_lookup[xResult.u8]

        U8 tmp = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->zeroExtend(tmp, xResult, 8);
        data->loadConst(tmp2, (U64)arm_parity_lookup);
        data->readMem8RegOffset(tmp, tmp2, tmp);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 2, 1); // PF is 0x4 (bit 2)
        data->releaseTmpReg(tmp);
        data->releaseTmpReg(tmp2);
    }
    virtual void setSF(Armv8btAsm* data, U8 reg) {
        U8 tmp = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmp, xResult, 7, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 7, 1); // SF is 0x80 (bit 7)
        data->releaseTmpReg(tmp);
    }
};

class Arm8BtFlagsDefault16 : public Arm8BtFlagsDefault {
public:
    Arm8BtFlagsDefault16() : Arm8BtFlagsDefault(16) {}
    virtual void setSF(Armv8btAsm* data, U8 reg) {
        U8 tmp = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmp, xResult, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 7, 1); // SF is 0x80 (bit 7)
        data->releaseTmpReg(tmp);
    }
};

class Arm8BtFlagsDefault32 : public Arm8BtFlagsDefault {
public:
    Arm8BtFlagsDefault32() : Arm8BtFlagsDefault(32) {}
    virtual void setSF(Armv8btAsm* data, U8 reg) {
        U8 tmp = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmp, xResult, 31, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 7, 1); // SF is 0x80 (bit 7)
        data->releaseTmpReg(tmp);
    }  
};

class Arm8BtFlagsAdd8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u8 < cpu->dst.u8; 
        U8 tmp = data->getTmpReg();
        data->zeroExtend(xResult, xResult, 8);
        data->subRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 8, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80; 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->xorRegs32(tmp1, xDst, xSrc);
        data->xorValue32(tmp1, tmp1, 0x80);
        data->xorRegs32(tmp2, xResult, xSrc);
        data->andRegs32(tmp1, tmp1, tmp2);
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 7, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp1, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp, tmp, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);        
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (ZF | PF | SF | CF | OF | AF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
};

static Arm8BtFlagsAdd8 arm8BtFlagsAdd8;
Arm8BtFlags* ARM8BT_FLAGS_ADD8 = &arm8BtFlagsAdd8;

class Arm8BtFlagsAdd16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u16 < cpu->dst.u16; 
        U8 tmp = data->getTmpReg();
        data->zeroExtend(xResult, xResult, 16);
        data->subRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 16, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000;
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->xorRegs32(tmp1, xDst, xSrc);
        data->xorValue32(tmp1, tmp1, 0x8000);
        data->xorRegs32(tmp2, xResult, xSrc);
        data->andRegs32(tmp1, tmp1, tmp2);
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp1, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp, tmp, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (ZF | PF | SF | CF | OF | AF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
};

static Arm8BtFlagsAdd16 arm8BtFlagsAdd16;
Arm8BtFlags* ARM8BT_FLAGS_ADD16 = &arm8BtFlagsAdd16;

class Arm8BtFlagsAdd32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u32 < cpu->dst.u32;

        // using the hardware flags would require 3 instructions instead of 2 because the carry flag is inverted
        /*
        U8 tmp = data->getTmpReg();
        csetCc(data, tmp);
        data->xorValue32(tmp, tmp, 0);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 0, 1); // CF is 0x1 (bit 0)
        data->releaseTmpReg(tmp);
        */

        U8 tmp = data->getTmpReg();
        data->subRegs64(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest64(reg, tmp, 32, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000;
        U8 tmp = data->getTmpReg();
        data->csetVs(tmp);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setZF(Armv8btAsm* data, U8 reg) {
        U8 tmp = data->getTmpReg();
        data->csetEq(tmp);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 6, 1); // ZF is 0x40 (bit 6)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp, tmp, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesHardwareFlags(U32 mask) {
        return (mask & (OF | ZF)) != 0;
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | CF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & AF) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (AF | CF)) != 0;
    }
};

static Arm8BtFlagsAdd32 arm8BtFlagsAdd32;
Arm8BtFlags* ARM8BT_FLAGS_ADD32 = &arm8BtFlagsAdd32;

class Arm8BtFlagsZero8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 0, 1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 11, 1);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 4, 1);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & ( PF | SF | ZF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

class Arm8BtFlagsZero16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 0, 1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 11, 1);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 4, 1);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | SF | ZF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

class Arm8BtFlagsZero32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 0, 1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 11, 1);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->clearBits(reg, 4, 1);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | SF | ZF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsZero8 arm8BtFlags0_8;
Arm8BtFlags* ARM8BT_FLAGS_OR8 = &arm8BtFlags0_8;
Arm8BtFlags* ARM8BT_FLAGS_AND8 = &arm8BtFlags0_8;
Arm8BtFlags* ARM8BT_FLAGS_XOR8 = &arm8BtFlags0_8;
Arm8BtFlags* ARM8BT_FLAGS_TEST8 = &arm8BtFlags0_8;

static Arm8BtFlagsZero16 arm8BtFlags0_16;
Arm8BtFlags* ARM8BT_FLAGS_OR16 = &arm8BtFlags0_16;
Arm8BtFlags* ARM8BT_FLAGS_AND16 = &arm8BtFlags0_16;
Arm8BtFlags* ARM8BT_FLAGS_XOR16 = &arm8BtFlags0_16;
Arm8BtFlags* ARM8BT_FLAGS_TEST16 = &arm8BtFlags0_16;

static Arm8BtFlagsZero32 arm8BtFlags0_r32;
Arm8BtFlags* ARM8BT_FLAGS_OR32 = &arm8BtFlags0_r32;
Arm8BtFlags* ARM8BT_FLAGS_AND32 = &arm8BtFlags0_r32;
Arm8BtFlags* ARM8BT_FLAGS_XOR32 = &arm8BtFlags0_r32;
Arm8BtFlags* ARM8BT_FLAGS_TEST32 = &arm8BtFlags0_r32;

class Arm8BtFlagsAdc8 : public Arm8BtFlagsAdd8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u8 < cpu->dst.u8) || (cpu->oldCF && (cpu->result.u8 == cpu->dst.u8)); 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        // check if cpu->result.u8 < cpu->dst.u8
        data->zeroExtend(xResult, xResult, 8);
        data->subRegs32(tmp1, xResult, xDst);
        data->clz32(tmp2, tmp1); // will be 32 if result == dst
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 8, 1, true); // CF is 0x01 (bit 0)

        // check if cpu->oldCF && cpu->result.u8 == cpu->dst.u8        
        data->copyBitsFromSourceAtPositionToDest(tmp2, tmp2, 5, 1, true); // bit 5 is set for the number 32, which means all bits are set to 0
        data->andRegs32(tmp2, tmp2, xFLAGS);
        
        data->orRegs32(tmp1, tmp1, tmp2);

        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1);

        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
};

static Arm8BtFlagsAdc8 arm8BtFlagsAdc8;
Arm8BtFlags* ARM8BT_FLAGS_ADC8 = &arm8BtFlagsAdc8;

class Arm8BtFlagsAdc16 : public Arm8BtFlagsAdd16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u16 < cpu->dst.u16) || (cpu->oldCF && (cpu->result.u16 == cpu->dst.u16)); 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        // check if cpu->result.u16 < cpu->dst.u16
        data->zeroExtend(xResult, xResult, 16);
        data->subRegs32(tmp1, xResult, xDst);
        data->clz32(tmp2, tmp1); // will be 32 if result == dst
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 16, 1, true); // CF is 0x01 (bit 0)

        // check if cpu->oldCF && cpu->result.u16 == cpu->dst.u16        
        data->copyBitsFromSourceAtPositionToDest(tmp2, tmp2, 5, 1, true); // bit 5 is set for the number 32, which means all bits are set to 0
        data->andRegs32(tmp2, tmp2, xFLAGS);

        data->orRegs32(tmp1, tmp1, tmp2);

        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1);

        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
};

static Arm8BtFlagsAdc16 arm8BtFlagsAdc16;
Arm8BtFlags* ARM8BT_FLAGS_ADC16 = &arm8BtFlagsAdc16;

class Arm8BtFlagsAdc32 : public Arm8BtFlagsAdd32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u32 < cpu->dst.u32) || (cpu->oldCF && (cpu->result.u32 == cpu->dst.u32));
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        // check if cpu->result.u32 < cpu->dst.u32
        data->subRegs64(tmp1, xResult, xDst);
        data->clz64(tmp2, tmp1); // will be 32 if result == dst
        data->copyBitsFromSourceAtPositionToDest64(tmp1, tmp1, 32, 1, true); // CF is 0x01 (bit 0)

        // check if cpu->oldCF && cpu->result.u32 == cpu->dst.u32        
        data->copyBitsFromSourceAtPositionToDest(tmp2, tmp2, 6, 1, true); // bit 6 is set for the number 64, which means all bits are set to 0
        data->andRegs32(tmp2, tmp2, xFLAGS);

        data->orRegs32(tmp1, tmp1, tmp2);

        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1);

        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
};

static Arm8BtFlagsAdc32 arm8BtFlagsAdc32;
Arm8BtFlags* ARM8BT_FLAGS_ADC32 = &arm8BtFlagsAdc32;

class Arm8BtFlagsSub8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->dst.u8 < cpu->src.u8; 
        U8 tmp = data->getTmpReg();
        data->subRegs32(tmp, xDst, xSrc);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 8, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) { 
        // ((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80; 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->xorRegs32(tmp1, xDst, xSrc);
        data->xorRegs32(tmp2, xDst, xResult);
        data->andRegs32(tmp1, tmp1, tmp2);
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 7, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp1, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) { 
        // ((cpu->dst.u8 ^ cpu->src.u8) ^ cpu->result.u8) & 0x10; 
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp, tmp, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (ZF | PF | SF | OF | AF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
};

static Arm8BtFlagsSub8 arm8BtFlagsSub8;
Arm8BtFlags* ARM8BT_FLAGS_SUB8 = &arm8BtFlagsSub8;
Arm8BtFlags* ARM8BT_FLAGS_CMP8 = &arm8BtFlagsSub8;

class Arm8BtFlagsSub16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->dst.u16 < cpu->src.u16;
        U8 tmp = data->getTmpReg();
        data->subRegs32(tmp, xDst, xSrc);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 16, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000; 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->xorRegs32(tmp1, xDst, xSrc);
        data->xorRegs32(tmp2, xDst, xResult);
        data->andRegs32(tmp1, tmp1, tmp2);
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp1, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u16 ^ cpu->src.u16) ^ cpu->result.u16) & 0x10;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp, tmp, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (ZF | PF | SF | OF | AF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
};

static Arm8BtFlagsSub16 arm8BtFlagsSub16;
Arm8BtFlags* ARM8BT_FLAGS_SUB16 = &arm8BtFlagsSub16;
Arm8BtFlags* ARM8BT_FLAGS_CMP16 = &arm8BtFlagsSub16;

class Arm8BtFlagsSub32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->dst.u32 < cpu->src.u32;

        // using the hardware flags would require 3 instructions instead of 2 because the carry flag is inverted
        /*
        U8 tmp = data->getTmpReg();
        csetCc(data, tmp);
        data->xorValue32(tmp, tmp, 0);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 0, 1); // CF is 0x1 (bit 0)
        data->releaseTmpReg(tmp);
        */

        U8 tmp = data->getTmpReg();
        data->subRegs64(tmp, xDst, xSrc);
        data->copyBitsFromSourceAtPositionToDest64(reg, tmp, 32, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000;
        U8 tmp = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp2, xDst, xResult);
        data->andRegs32(tmp, tmp, tmp2);
        data->shiftRegRightWithValue32(tmp, tmp, 31);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
        data->releaseTmpReg(tmp2);
    }
    virtual void setZF(Armv8btAsm* data, U8 reg) {
        U8 tmp = data->getTmpReg();
        data->csetEq(tmp);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 6, 1); // ZF is 0x40 (bit 6)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u32 ^ cpu->src.u32) ^ cpu->result.u32) & 0x10;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xDst, xSrc);
        data->xorRegs32(tmp, tmp, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesHardwareFlags(U32 mask) {
        return (mask & ZF) != 0;
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | OF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | CF | OF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (AF | CF | OF)) != 0;
    }
};

static Arm8BtFlagsSub32 arm8BtFlagsSub32;
Arm8BtFlags* ARM8BT_FLAGS_SUB32 = &arm8BtFlagsSub32;
Arm8BtFlags* ARM8BT_FLAGS_CMP32 = &arm8BtFlagsSub32;

class Arm8BtFlagsSbb8 : public Arm8BtFlagsSub8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u8 < cpu->result.u8) || (cpu->oldCF && (cpu->src.u8 == 0xff));
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        // check if cpu->dst.u8 < cpu->result.u8
        data->zeroExtend(xResult, xResult, 8);
        data->subRegs32(tmp1, xDst, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 8, 1, true); // CF is 0x01 (bit 0)

        // check if cpu->oldCF && cpu->src.u8 == 0xff
        data->xorValue32(tmp2, xSrc, 0xFF); // will cause tmp2 to be 0 if xSrc == 0xFF
        data->clz32(tmp2, tmp2); // will be 32 if xSrc == 0xFF
        data->copyBitsFromSourceAtPositionToDest(tmp2, tmp2, 5, 1, true); // bit 5 is set for the number 32, which means all bits are set to 0
        data->andRegs32(tmp2, tmp2, xFLAGS);

        data->orRegs32(tmp1, tmp1, tmp2);

        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1);

        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
};

static Arm8BtFlagsSbb8 arm8BtFlagsSbb8;
Arm8BtFlags* ARM8BT_FLAGS_SBB8 = &arm8BtFlagsSbb8;

class Arm8BtFlagsSbb16 : public Arm8BtFlagsSub16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u16 < cpu->result.u16) || (cpu->oldCF && (cpu->src.u16 == 0xffff)); 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        // check if cpu->dst.u16 < cpu->result.u16
        data->zeroExtend(xResult, xResult, 16);
        data->subRegs32(tmp1, xDst, xResult);
        data->copyBitsFromSourceAtPositionToDest(tmp1, tmp1, 16, 1, true); // CF is 0x01 (bit 0)

        // check if cpu->oldCF && cpu->src.u16 == 0xffff
        data->xorValue32(tmp2, xSrc, 0xFFFF); // will cause tmp2 to be 0 if xSrc == 0xFFFF
        data->clz32(tmp2, tmp2); // will be 32 if xSrc == 0xFFFF
        data->copyBitsFromSourceAtPositionToDest(tmp2, tmp2, 5, 1, true); // bit 5 is set for the number 32, which means all bits are set to 0
        data->andRegs32(tmp2, tmp2, xFLAGS);

        data->orRegs32(tmp1, tmp1, tmp2);

        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1);

        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
};

static Arm8BtFlagsSbb16 arm8BtFlagsSbb16;
Arm8BtFlags* ARM8BT_FLAGS_SBB16 = &arm8BtFlagsSbb16;

class Arm8BtFlagsSbb32 : public Arm8BtFlagsSub32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 < cpu->result.u32) || (cpu->oldCF && (cpu->src.u32 == 0xffffffff)); 
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        // check if cpu->dst.u32 < cpu->result.u32
        data->subRegs64(tmp1, xDst, xResult);
        data->copyBitsFromSourceAtPositionToDest64(tmp1, tmp1, 32, 1, true); // CF is 0x01 (bit 0)

        // check if cpu->oldCF && cpu->src.u32 == 0xffffffff
        data->xorValue32(tmp2, xSrc, 0xFFFFFFFF); // will cause tmp2 to be 0 if xSrc == 0xFFFF
        data->clz32(tmp2, tmp2); // will be 32 if xSrc == 0xFFFF
        data->copyBitsFromSourceAtPositionToDest(tmp2, tmp2, 5, 1, true); // bit 5 is set for the number 32, which means all bits are set to 0
        data->andRegs32(tmp2, tmp2, xFLAGS);

        data->orRegs32(tmp1, tmp1, tmp2);

        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1);

        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
};

static Arm8BtFlagsSbb32 arm8BtFlagsSbb32;
Arm8BtFlags* ARM8BT_FLAGS_SBB32 = &arm8BtFlagsSbb32;

class Arm8BtFlagsInc8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // not change
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u8 == 0x80; 
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xResult, 0x80); // will cause tmp to be 0 if xResult == 0x80
        data->clz32(tmp, tmp); // will be 32 if xResult == 0x80
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u8 & 0x0f) == 0; 
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xResult, 0xF);
        data->clz32(tmp, tmp); // will be 32 if xSrc == 0xFFFF
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | ZF | SF | OF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsInc8 arm8BtFlagsInc8;
Arm8BtFlags* ARM8BT_FLAGS_INC8 = &arm8BtFlagsInc8;

class Arm8BtFlagsInc16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // not change
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u16 == 0x8000; 
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xResult, 0x8000); // will cause tmp to be 0 if xResult == 0x8000
        data->clz32(tmp, tmp); // will be 32 if xResult == 0x8000
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u16 & 0x0f) == 0; 
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xResult, 0xF);
        data->clz32(tmp, tmp); // will be 32 if (xResult & 0x0f) == 0
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | ZF | SF | OF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsInc16 arm8BtFlagsInc16;
Arm8BtFlags* ARM8BT_FLAGS_INC16 = &arm8BtFlagsInc16;

class Arm8BtFlagsInc32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // not change
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u32 == 0x80000000; 
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xResult, 0x80000000); // will cause tmp to be 0 if xResult == 0x80000000
        data->clz32(tmp, tmp); // will be 32 if xResult == 0x80000000
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u32 & 0x0f) == 0; 
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xResult, 0xF);
        data->clz32(tmp, tmp); // will be 32 if (xResult & 0x0f) == 0
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | ZF | SF | OF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsInc32 arm8BtFlagsInc32;
Arm8BtFlags* ARM8BT_FLAGS_INC32 = &arm8BtFlagsInc32;

class Arm8BtFlagsDec8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // not change
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u8 == 0x7f; 
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xResult, 0x7f); // will cause tmp to be 0 if xResult == 0x7f
        data->clz32(tmp, tmp); // will be 32 if xResult == 0x7f
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u8 & 0x0f) == 0x0f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xResult, 0xF);
        data->xorValue32(tmp, tmp, 0xf);
        data->clz32(tmp, tmp);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | ZF | SF | OF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsDec8 arm8BtFlagsDec8;
Arm8BtFlags* ARM8BT_FLAGS_DEC8 = &arm8BtFlagsDec8;

class Arm8BtFlagsDec16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // not change
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u16 == 0x7fff; 
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xResult, 0x7fff); // will cause tmp to be 0 if xResult == 0x7fff
        data->clz32(tmp, tmp); // will be 32 if xResult == 0x7fff
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u16 & 0x0f) == 0x0f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xResult, 0xF);
        data->xorValue32(tmp, tmp, 0xf);
        data->clz32(tmp, tmp);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | ZF | SF | OF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsDec16 arm8BtFlagsDec16;
Arm8BtFlags* ARM8BT_FLAGS_DEC16 = &arm8BtFlagsDec16;

class Arm8BtFlagsDec32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // not change
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->result.u32 == 0x7fffffff; 
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xResult, 0x7fffffff); // will cause tmp to be 0 if xResult == 0x7fffffff
        data->clz32(tmp, tmp); // will be 32 if xResult == 0x7fffffff
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u32 & 0x0f) == 0x0f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xResult, 0xF);
        data->xorValue32(tmp, tmp, 0xf);
        data->clz32(tmp, tmp);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (AF | PF | ZF | SF | OF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsDec32 arm8BtFlagsDec32;
Arm8BtFlags* ARM8BT_FLAGS_DEC32 = &arm8BtFlagsDec32;

class Arm8BtFlagsNeg8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 != 0;
        U8 tmp = data->getTmpReg();
        data->clz32(tmp, xSrc); // will be 32 if xSrc == 0
        data->notReg32(tmp, tmp);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 5, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 == 0x80;
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xSrc, 0x80); // will cause tmp to be 0 if xSrc == 0x80
        data->clz32(tmp, tmp); // will be 32 if xSrc == 0x80
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 & 0x0f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x0f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x0f) is 0 else if will be 28-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1); 
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsNeg8 arm8BtFlagsNeg8;
Arm8BtFlags* ARM8BT_FLAGS_NEG8 = &arm8BtFlagsNeg8;

class Arm8BtFlagsNeg16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u16 != 0;
        U8 tmp = data->getTmpReg();
        data->clz32(tmp, xSrc); // will be 32 if xSrc == 0
        data->notReg32(tmp, tmp);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 5, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u16 == 0x8000;
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xSrc, 0x8000); // will cause tmp to be 0 if xSrc == 0x8000
        data->clz32(tmp, tmp); // will be 32 if xSrc == 0x8000
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u16 & 0x0f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x0f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x0f) is 0 else if will be 28-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsNeg16 arm8BtFlagsNeg16;
Arm8BtFlags* ARM8BT_FLAGS_NEG16 = &arm8BtFlagsNeg16;

class Arm8BtFlagsNeg32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u32 != 0;
        U8 tmp = data->getTmpReg();
        data->clz32(tmp, xSrc); // will be 32 if xSrc == 0
        data->notReg32(tmp, tmp);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp, 5, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u32 == 0x80000000;
        U8 tmp = data->getTmpReg();
        data->xorValue32(tmp, xSrc, 0x80000000); // will cause tmp to be 0 if xSrc == 0x80000000
        data->clz32(tmp, tmp); // will be 32 if xSrc == 0x80000000
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 5, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u32 & 0x0f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x0f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x0f) is 0 else if will be 28-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (AF | OF | CF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return false;
    }
};

static Arm8BtFlagsNeg32 arm8BtFlagsNeg32;
Arm8BtFlags* ARM8BT_FLAGS_NEG32 = &arm8BtFlagsNeg32;

class Arm8BtFlagsShl8 : public Arm8BtFlagsDefault8 {
    // :TODO: could be faster if cpu->src is a constant
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u8 << (cpu->src.u8 - 1)) & 0x80) >> 7;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegLeftWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 7, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u8 ^ cpu->dst.u8) & 0x80;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 7, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsShl8 arm8BtFlagsShl8;
Arm8BtFlags* ARM8BT_FLAGS_SHL8 = &arm8BtFlagsShl8;

class Arm8BtFlagsShl16 : public Arm8BtFlagsDefault16 {
    // :TODO: could be faster if cpu->src is a constant
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u16 << (cpu->src.u8 - 1)) & 0x8000) >> 15;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegLeftWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 15, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u16 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsShl16 arm8BtFlagsShl16;
Arm8BtFlags* ARM8BT_FLAGS_SHL16 = &arm8BtFlagsShl16;

class Arm8BtFlagsShl32 : public Arm8BtFlagsDefault32 {
    // :TODO: could be faster if cpu->src is a constant
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // ((cpu->dst.u32 << (cpu->src.u8 - 1)) & 0x80000000) >> 31;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegLeftWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 31, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 31, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u32 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsShl32 arm8BtFlagsShl32;
Arm8BtFlags* ARM8BT_FLAGS_SHL32 = &arm8BtFlagsShl32;

class Arm8BtFlagsDshl16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u16 >> (16 - cpu->src.u8)) & 1;
        U8 tmp1 = data->getTmpReg();
        data->shiftRegRightWithValue32(tmp1, xDst, 16 - data->currentOp->imm);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->andRegs32(reg, reg, ~AF);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsDshl16 arm8BtFlagsDshl16;
Arm8BtFlags* ARM8BT_FLAGS_DSHL16 = &arm8BtFlagsDshl16;

class Arm8BtFlagsDshl32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        U8 tmp1 = data->getTmpReg();
        data->shiftRegRightWithValue32(tmp1, xDst, 32 - data->currentOp->imm);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 31, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->andRegs32(reg, reg, ~AF);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsDshl32 arm8BtFlagsDshl32;
Arm8BtFlags* ARM8BT_FLAGS_DSHL32 = &arm8BtFlagsDshl32;

class Arm8BtFlagsDshl16Cl : public Arm8BtFlagsDshl16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u16 >> (16 - cpu->src.u8)) & 1;
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subRegFromValue32(tmp2, xSrc, 16);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & CF) != 0;
    }
};

static Arm8BtFlagsDshl16Cl arm8BtFlagsDshl16Cl;
Arm8BtFlags* ARM8BT_FLAGS_DSHL16_CL = &arm8BtFlagsDshl16Cl;

class Arm8BtFlagsDshl32Cl : public Arm8BtFlagsDshl32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (32 - cpu->src.u8)) & 1;
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subRegFromValue32(tmp2, xSrc, 32);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & CF) != 0;
    }
};

static Arm8BtFlagsDshl32Cl arm8BtFlagsDshl32Cl;
Arm8BtFlags* ARM8BT_FLAGS_DSHL32_CL = &arm8BtFlagsDshl32Cl;

class Arm8BtFlagsDshr16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f
        U8 amount = data->currentOp->imm & 0xF;
        U8 tmp1 = data->getTmpReg();
        data->shiftRegRightWithValue32(tmp1, xDst, amount - 1);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u16 ^ cpu->dst.u16) & 0x8000;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->andRegs32(reg, reg, ~AF);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsDshr16 arm8BtFlagsDshr16;
Arm8BtFlags* ARM8BT_FLAGS_DSHR16 = &arm8BtFlagsDshr16;

class Arm8BtFlagsDshr16Cl : public Arm8BtFlagsDshr16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & CF) != 0;
    }
};

static Arm8BtFlagsDshr16Cl arm8BtFlagsDshr16Cl;
Arm8BtFlags* ARM8BT_FLAGS_DSHR16_CL = &arm8BtFlagsDshr16Cl;

class Arm8BtFlagsDshr32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        data->shiftRegRightWithValue32(tmp1, xDst, data->currentOp->imm - 1);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        // (cpu->result.u32 ^ cpu->dst.u32) & 0x80000000;
        U8 tmp = data->getTmpReg();
        data->xorRegs32(tmp, xResult, xDst);
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 31, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        data->andRegs32(reg, reg, ~AF);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (OF | PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return false;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

class Arm8BtFlagsDshr32Cl : public Arm8BtFlagsDshr32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;
        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & CF) != 0;
    }
};

static Arm8BtFlagsDshr32Cl arm8BtFlagsDshr32Cl;
Arm8BtFlags* ARM8BT_FLAGS_DSHR32_CL = &arm8BtFlagsDshr32Cl;

static Arm8BtFlagsDshr32 arm8BtFlagsDshr32;
Arm8BtFlags* ARM8BT_FLAGS_DSHR32 = &arm8BtFlagsDshr32;

class Arm8BtFlagsShr8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1; 

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        //if ((cpu->src.u8 & 0x1f) == 1) return (cpu->dst.u8 >= 0x80); else return 0;

        // :TODO: is the ((cpu->src.u8 & 0x1f) == 1) necessary
        U8 tmp = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmp, xDst, 7, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsShr8 arm8BtFlagsShr8;
Arm8BtFlags* ARM8BT_FLAGS_SHR8 = &arm8BtFlagsShr8;

class Arm8BtFlagsShr16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        //if ((cpu->src.u8 & 0x1f) == 1) return (cpu->dst.u16 >= 0x8000); else return 0;

        // :TODO: is the ((cpu->src.u8 & 0x1f) == 1) necessary
        U8 tmp = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmp, xDst, 15, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u16 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsShr16 arm8BtFlagsShr16;
Arm8BtFlags* ARM8BT_FLAGS_SHR16 = &arm8BtFlagsShr16;

class Arm8BtFlagsShr32 : public Arm8BtFlagsDefault32 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        //if ((cpu->src.u8 & 0x1f) == 1) return (cpu->dst.u32 >= 0x80000000); else return 0;

        // :TODO: is the ((cpu->src.u8 & 0x1f) == 1) necessary
        U8 tmp = data->getTmpReg();
        data->copyBitsFromSourceAtPositionToDest(tmp, xDst, 31, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 11, 1); // OF is 0x800 (bit 11)
        data->releaseTmpReg(tmp);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u32 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (OF | CF)) != 0;
    }
};

static Arm8BtFlagsShr32 arm8BtFlagsShr32;
Arm8BtFlags* ARM8BT_FLAGS_SHR32 = &arm8BtFlagsShr32;

class Arm8BtFlagsSar8 : public Arm8BtFlagsDefault8 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (((S8)cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->signExtend(tmp1, xDst, 8);
        data->shiftSignedRegRightWithReg32(tmp1, tmp1, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        data->andValue32(reg, reg, ~OF);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (CF)) != 0;
    }
};

static Arm8BtFlagsSar8 arm8BtFlagsSar8;
Arm8BtFlags* ARM8BT_FLAGS_SAR8 = &arm8BtFlagsSar8;

class Arm8BtFlagsSar16 : public Arm8BtFlagsDefault16 {
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (((S16)cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->signExtend(tmp1, xDst, 16);
        data->shiftSignedRegRightWithReg32(tmp1, tmp1, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        data->andValue32(reg, reg, ~OF);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (CF)) != 0;
    }
};

static Arm8BtFlagsSar16 arm8BtFlagsSar16;
Arm8BtFlags* ARM8BT_FLAGS_SAR16 = &arm8BtFlagsSar16;

class Arm8BtFlagsSar32 : public Arm8BtFlagsDefault32 {
    U32 getCF(CPU* cpu) { return (((S32)cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1; }
    virtual void setCF(Armv8btAsm* data, U8 reg) {
        // (((S32)cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1;

        // src.u8 should already have been masked to 0x1f

        U8 tmp1 = data->getTmpReg();
        U8 tmp2 = data->getTmpReg();
        data->subValue32(tmp2, xSrc, 1);
        data->shiftSignedRegRightWithReg32(tmp1, xDst, tmp2);
        data->copyBitsFromSourceAtPositionToDest(reg, tmp1, 0, 1); // CF is 0x01 (bit 0)
        data->releaseTmpReg(tmp1);
        data->releaseTmpReg(tmp2);
    }
    virtual void setOF(Armv8btAsm* data, U8 reg) {
        data->andValue32(reg, reg, ~OF);
    }
    virtual void setAF(Armv8btAsm* data, U8 reg) {
        // cpu->src.u8 & 0x1f;
        U8 tmp = data->getTmpReg();
        data->andValue32(tmp, xSrc, 0x1f);
        data->clz32(tmp, tmp); // will be 32 if (cpu->src.u8 & 0x1f) is 0 else if will be 27-31, which means bit 4 will be set
        data->copyBitsFromSourceAtPositionToDest(tmp, tmp, 4, 1);
        data->copyBitsFromSourceToDestAtPosition(reg, tmp, 4, 1); // AF is 0x10 (bit 4)
        data->releaseTmpReg(tmp);
    }
    virtual bool usesResult(U32 mask) {
        return (mask & (PF | ZF | SF)) != 0;
    }
    virtual bool usesSrc(U32 mask) {
        return (mask & (CF | AF)) != 0;
    }
    virtual bool usesDst(U32 mask) {
        return (mask & (CF)) != 0;
    }
};

static Arm8BtFlagsSar32 arm8BtFlagsSar32;
Arm8BtFlags* ARM8BT_FLAGS_SAR32 = &arm8BtFlagsSar32;

#endif