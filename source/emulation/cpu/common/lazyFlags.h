#ifndef __LAZY_FLAGS_H__
#define __LAZY_FLAGS_H__

class CPU;

class LazyFlags {
public:
    virtual U32 getCF(CPU* cpu) const=0;
    virtual U32 getSF(CPU* cpu) const=0;
    virtual U32 getZF(CPU* cpu) const=0;
    virtual U32 getOF(CPU* cpu) const=0;
    virtual U32 getAF(CPU* cpu) const=0;
    virtual U32 getPF(CPU* cpu) const=0;
};

extern const LazyFlags* FLAGS_NONE;
extern const LazyFlags* FLAGS_ADD8;
extern const LazyFlags* FLAGS_ADD16;
extern const LazyFlags* FLAGS_ADD32;
extern const LazyFlags* FLAGS_OR8;
extern const LazyFlags* FLAGS_OR16;
extern const LazyFlags* FLAGS_OR32;
extern const LazyFlags* FLAGS_ADC8;
extern const LazyFlags* FLAGS_ADC16;
extern const LazyFlags* FLAGS_ADC32;
extern const LazyFlags* FLAGS_SBB8;
extern const LazyFlags* FLAGS_SBB16;
extern const LazyFlags* FLAGS_SBB32;
extern const LazyFlags* FLAGS_AND8;
extern const LazyFlags* FLAGS_AND16;
extern const LazyFlags* FLAGS_AND32;
extern const LazyFlags* FLAGS_SUB8;
extern const LazyFlags* FLAGS_SUB16;
extern const LazyFlags* FLAGS_SUB32;
extern const LazyFlags* FLAGS_XOR8;
extern const LazyFlags* FLAGS_XOR16;
extern const LazyFlags* FLAGS_XOR32;
extern const LazyFlags* FLAGS_INC8;
extern const LazyFlags* FLAGS_INC16;
extern const LazyFlags* FLAGS_INC32;
extern const LazyFlags* FLAGS_DEC8;
extern const LazyFlags* FLAGS_DEC16;
extern const LazyFlags* FLAGS_DEC32;
extern const LazyFlags* FLAGS_SHL8;
extern const LazyFlags* FLAGS_SHL16;
extern const LazyFlags* FLAGS_SHL32;
extern const LazyFlags* FLAGS_SHR8;
extern const LazyFlags* FLAGS_SHR16;
extern const LazyFlags* FLAGS_SHR32;
extern const LazyFlags* FLAGS_SAR8;
extern const LazyFlags* FLAGS_SAR16;
extern const LazyFlags* FLAGS_SAR32;
extern const LazyFlags* FLAGS_CMP8;
extern const LazyFlags* FLAGS_CMP16;
extern const LazyFlags* FLAGS_CMP32;
extern const LazyFlags* FLAGS_TEST8;
extern const LazyFlags* FLAGS_TEST16;
extern const LazyFlags* FLAGS_TEST32;
extern const LazyFlags* FLAGS_DSHL16;
extern const LazyFlags* FLAGS_DSHL32;
extern const LazyFlags* FLAGS_DSHR16;
extern const LazyFlags* FLAGS_DSHR32;
extern const LazyFlags* FLAGS_NEG8;
extern const LazyFlags* FLAGS_NEG16;
extern const LazyFlags* FLAGS_NEG32;

#endif