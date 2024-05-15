#include "boxedwine.h"
#include "decoder.h"
#include "../../util/ptrpool.h"

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)

const InstructionInfo instructionInfo[] = {
/*000*/    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR8E8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR8I8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR16E16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AddE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR8E8
/*010*/    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR8I8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR16E16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // OrE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR8E8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcE8R8
/*020*/    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR8I8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR16E16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // AdcE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR8E8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR8R8
/*030*/    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR8I8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR16E16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, CF, 0, 0}, // SbbE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR8E8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR8I8
/*040*/    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR16E16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // AndE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR8E8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR8I8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubE8I8
/*050*/    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR16E16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // SubE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR8E8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR8I8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR16E16
/*060*/    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR16I16
    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR32E32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorR32I32
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // XorE32I32

    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR8E8
    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR8I8
    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR16E16
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpE16R16
/*070*/    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR16I16
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR32E32
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpR32I32
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // CmpE32I32
    // CF, AF, OF are always 0
    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestE8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestR8R8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestR8I8
    {0, 8, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestE8I8
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestE16R16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestR16R16
/*080*/    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestR16I16
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestE16I16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestE32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestR32R32
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestR32I32
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // TestE32I32

    {0, 0, 0, 0, 0, 0, 0}, // NotR8
    {0, 8, 8, 0, 0, 0, 0}, // NotE8
    {0, 0, 0, 0, 0, 0, 0}, // NotR16 
    {0, 16, 16, 0, 0, 0, 0}, // NotE16
    {0, 0, 0, 0, 0, 0, 0}, // NotR32 
    {0, 16, 16, 0, 0, 0, 0}, // NotE32

    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // NegR8
    {0, 8, 8, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // NegE8
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // NegR16 
/*090*/    {0, 16, 16, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // NegE16
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // NegR32 
    {0, 32, 32, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // NegE32

    {0, 0, 0, CF|OF, 0, ZF|SF|AF|PF, 0}, // MulR8
    {0, 8, 8, CF|OF, 0, ZF|SF|AF|PF, 0}, // MulE8,
    {0, 0, 0, CF|OF, 0, ZF|SF|AF|PF, 0}, // MulR16 
    {0, 16, 16, CF|OF, 0, ZF|SF|AF|PF, 0}, // MulE16
    {0, 0, 0, CF|OF, 0, ZF|SF|AF|PF, 0}, // MulR32 
    {0, 32, 32, CF|OF, 0, ZF|SF|AF|PF, 0}, // MulE32

    {0, 0, 0, CF|OF, 0, 0}, // IMulR8, 
    {0, 8, 8, CF|OF, 0, 0}, // IMulE8,
    {0, 0, 0, CF|OF, 0, 0}, // IMulR16, 
    {0, 16, 16, CF|OF, 0, 0}, // IMulE16,
    {0, 0, 0, CF|OF, 0, 0}, // IMulR32, 
    {0, 32, 32, CF|OF, 0, 0}, // IMulE32,

    {0, 0, 0, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // DivR8
/*0a0*/    {0, 8, 8, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // DivE8
    {0, 0, 0, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // DivR16 
    {0, 16, 16, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // DivE16
    {0, 0, 0, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // DivR32 
    {0, 32, 32, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // DivE32

    {0, 0, 0, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // IDivR8 
    {0, 8, 8, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // IDivE8
    {0, 0, 0, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // IDivR16 
    {0, 16, 16, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // IDivE16
    {0, 0, 0, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // IDivR32 
    {0, 32, 32, 0, 0, ZF|OF|SF|CF|AF|PF, 1}, // IDivE32

    {0, 0, 0, 0, 0, 0, 0}, // XchgR8R8 
    {0, 8, 8, 0, 0, 0, 0}, // XchgE8R8
    {0, 0, 0, 0, 0, 0, 0}, // XchgR16R16 
    {0, 16, 16, 0, 0, 0, 0}, // XchgE16R16
    {0, 0, 0, 0, 0, 0, 0}, // XchgR32R32 
/*0b0*/    {0, 32, 32, 0, 0, 0, 0}, // XchgE32R32

    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtR16R16
    {0, 16, 0, CF, 0, OF|SF|AF|PF, 0}, // BtE16R16
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtR32R32
    {0, 32, 0, CF, 0, OF|SF|AF|PF, 0}, // BtE32R32

    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtsR16R16
    {0, 16, 16, CF, 0, OF|SF|AF|PF, 0}, // BtsE16R16
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtsR32R32
    {0, 32, 32, CF, 0, OF|SF|AF|PF, 0}, // BtsE32R32

    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtrR16R16 
    {0, 16, 16, CF, 0, OF|SF|AF|PF, 0}, // BtrE16R16
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtrR32R32 
    {0, 32, 32, CF, 0, OF|SF|AF|PF, 0}, // BtrE32R32

    {0, 0, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsfR16R16, 
    {0, 16, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsfR16E16,
    {0, 0, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsfR32R32, 
    {0, 32, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsfR32E32,

    {0, 0, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsrR16R16, 
    {0, 16, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsrR16E16,
    {0, 0, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsrR32R32, 
    {0, 32, 0, ZF, 0, OF|SF|CF|AF|PF, 0}, // BsrR32E32,

    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtcR16R16
    {0, 16, 16, CF, 0, OF|SF|AF|PF, 0}, // BtcE16R16
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtcR32R32
    {0, 32, 32, CF, 0, OF|SF|AF|PF, 0}, // BtcE32R32

    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtR16, 
    {0, 16, 0, CF, 0, OF|SF|AF|PF, 0}, // BtE16,
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtsR16, 
    {0, 16, 16, CF, 0, OF|SF|AF|PF, 0}, // BtsE16,
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtrR16, 
    {0, 16, 16, CF, 0, OF|SF|AF|PF, 0}, // BtrE16,
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtcR16, 
    {0, 16, 16, CF, 0, OF|SF|AF|PF, 0}, // BtcE16,

    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtR32, 
    {0, 32, 0, CF, 0, OF|SF|AF|PF, 0}, // BtE32,
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtsR32, 
    {0, 32, 32, CF, 0, OF|SF|AF|PF, 0}, // BtsE32,
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtrR32, 
    {0, 32, 32, CF, 0, OF|SF|AF|PF, 0}, // BtrE32,
    {0, 0, 0, CF, 0, OF|SF|AF|PF, 0}, // BtcR32, 
    {0, 32, 32, CF, 0, OF|SF|AF|PF, 0}, // BtcE32,

    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshlR16R16
    {0, 16, 16, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshlE16R16
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshlClR16R16
    {0, 16, 16, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshlClE16R16
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshrR16R16
    {0, 16, 16, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshrE16R16
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshrClR16R16
    {0, 16, 16, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshrClE16R16

    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshlR32R32
    {0, 32, 32, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshlE32R32
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshlClR32R32
    {0, 32, 32, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshlClE32R32
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshrR32R32
    {0, 32, 32, CF|ZF|SF|OF|PF, 0, AF, 0}, // DshrE32R32
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshrClR32R32
    {0, 32, 32, CF|ZF|SF|OF|PF|MAYBE, 0, AF, 0}, // DshrClE32R32

    {0, 0, 0, CF|OF, 0, AF|ZF|SF|PF, 0}, // DimulR16R16
    {0, 16, 0, CF|OF, 0, AF|ZF|SF|PF, 0}, // DimulR16E16
    {0, 0, 0, CF|OF, 0, AF|ZF|SF|PF, 0}, // DimulR32R32
    {0, 32, 0, CF|OF, 0, AF|ZF|SF|PF, 0}, // DimulR32E32

    {0, 0, 0, CF|OF|AF|ZF|SF|PF, 0, 0, 0}, // CmpXchgR8R8
    {0, 8, 8, CF|OF|AF|ZF|SF|PF, 0, 0, 0}, // CmpXchgE8R8
    {0, 0, 0, CF|OF|AF|ZF|SF|PF, 0, 0, 0}, // CmpXchgR16R16
    {0, 16, 16, CF|OF|AF|ZF|SF|PF, 0, 0, 0}, // CmpXchgE16R16
    {0, 0, 0, CF|OF|AF|ZF|SF|PF, 0, 0, 0}, // CmpXchgR32R32
    {0, 32, 32, CF|OF|AF|ZF|SF|PF, 0, 0, 0}, // CmpXchgE32R32
    // CF is preserved
    {0, 0, 0, AF|ZF|SF|OF|PF, 0, 0, 0}, // IncR8
    {0, 0, 0, AF|ZF|SF|OF|PF, 0, 0, 0}, // IncR16
    {0, 0, 0, AF|ZF|SF|OF|PF, 0, 0, 0}, // IncR32
    {0, 8, 8, AF|ZF|SF|OF|PF, 0, 0, 0}, // IncE8
    {0, 16, 16, AF|ZF|SF|OF|PF, 0, 0, 0}, // IncE16
    {0, 32, 32, AF|ZF|SF|OF|PF, 0, 0, 0}, // IncE32
    // CF is preserved
    {0, 0, 0, AF|ZF|SF|OF|PF, 0, 0, 0}, // DecR8
    {0, 0, 0, AF|ZF|SF|OF|PF, 0, 0, 0}, // DecR16
    {0, 0, 0, AF|ZF|SF|OF|PF, 0, 0, 0}, // DecR32
    {0, 8, 8, AF|ZF|SF|OF|PF, 0, 0, 0}, // DecE8
    {0, 16, 16, AF|ZF|SF|OF|PF, 0, 0, 0}, // DecE16
    {0, 32, 32, AF|ZF|SF|OF|PF, 0, 0, 0}, // DecE32

    {0, 0, 16, 0, 0, 0, 0}, // PushSeg16
    {0, 16, 0, 0, 0, 0, 1}, // PopSeg16
    {0, 0, 32, 0, 0, 0, 0}, // PushSeg32
    {0, 32, 0, 0, 0, 0, 1}, // PopSeg32

    {0, 0, 16, 0, 0, 0, 0}, // PushR16
    {0, 0, 32, 0, 0, 0, 0}, // PushR32
    {0, 16, 16, 0, 0, 0, 0}, // PushE16
    {0, 32, 32, 0, 0, 0, 0}, // PushE32

    {0, 16, 0, 0, 0, 0, 0}, // PopR16
    {0, 32, 0, 0, 0, 0, 0}, // PopR32
    {0, 16, 16, 0, 0, 0, 0}, // PopE16
    {0, 32, 32, 0, 0, 0, 0}, // PopE32

    {0, 0, 16*8, 0, 0, 0, 0}, // PushA16
    {0, 0, 32*8, 0, 0, 0, 0}, // PushA32
    {0, 16*8, 0, 0, 0, 0, 0}, // PopA16
    {0, 16*8, 0, 0, 0, 0, 0}, // PopA32

    {0, 0, 16, 0, 0, 0, 0}, // Push16
    {0, 0, 32, 0, 0, 0, 0}, // Push32

    {0, 0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0}, // PushF16
    {0, 0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0}, // PushF32
    {0, 16, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // PopF16
    {0, 32, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // PopF32

    {0, 32, 0, 0, 0, 0, 1}, // Bound16
    {0, 64, 0, 0, 0, 0, 1}, // Bound32

    {0, 0, 0, ZF, 0, 0, 0}, // ArplReg 
    {0, 16, 16, ZF, 0, 0, 0}, // ArplMem
    {0, 0, 0, ZF, 0, 0, 0}, // ArplReg32
    {0, 32, 32, ZF, 0, 0, 0}, // ArplMem32

    {0, 0, 0, CF|AF|ZF|SF|PF, CF|AF, OF, 0}, // Daa
    {0, 0, 0, CF|AF|ZF|SF|PF, CF|AF, OF, 0}, // Das
    {0, 0, 0, CF|AF,AF,ZF|SF|OF|PF, 0}, // Aaa
    {0, 0, 0, CF|AF,AF,ZF|SF|OF|PF, 0}, // Aas
    {0, 0, 0, SF|ZF|PF,0,CF|OF|AF, 1}, // Aam
    {0, 0, 0, SF|ZF|PF,0,CF|OF|AF, 0}, // Aad

    {0, 16, 0, CF|OF,0,AF|SF|PF|ZF, 0}, // ImulR16E16
    {0, 0, 0, CF|OF,0,AF|SF|PF|ZF, 0}, // ImulR16R16
    {0, 32, 0, CF|OF,0,AF|SF|PF|ZF, 0}, // ImulR32E32
    {0, 0, 0, CF|OF,0,AF|SF|PF|ZF, 0}, // ImulR32R32

    {0, 0, 8, 0, 0, 0, 0}, // Insb
    {0, 0, 16, 0, 0, 0, 0}, // Insw
    {0, 0, 16, 0, 0, 0, 0}, // Insd

    {0, 8, 0, 0, 0, 0, 0}, // Outsb
    {0, 16, 0, 0, 0, 0, 0}, // Outsw
    {0, 32, 0, 0, 0, 0, 0}, // Outsd

    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, OF, 0, 0}, // JumpO
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, OF, 0, 0}, // JumpNO
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, CF, 0, 0}, // JumpB
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, CF, 0, 0}, // JumpNB
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, ZF, 0, 0}, // JumpZ
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, ZF, 0, 0}, // JumpNZ
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, CF|ZF, 0, 0}, // JumpBE
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, CF|ZF, 0, 0}, // JumpNBE
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, SF, 0, 0}, // JumpS
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, SF, 0, 0}, // JumpNS
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, PF, 0, 0}, // JumpP
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, PF, 0, 0}, // JumpNP
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, SF|OF, 0, 0}, // JumpL
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, SF|OF, 0, 0}, // JumpNL
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, SF|OF|ZF, 0, 0}, // JumpLE
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, SF|OF|ZF, 0, 0}, // JumpNLE

    {0, 0, 0, 0, 0, 0, 0}, // MovR8R8
    {0, 0, 8, 0, 0, 0, 0}, // MovE8R8
    {0, 8, 0, 0, 0, 0, 0}, // MovR8E8
    {0, 0, 0, 0, 0, 0, 0}, // MovR8I8
    {0, 0, 8, 0, 0, 0, 0}, // MovE8I8
    {0, 0, 0, 0, 0, 0, 0}, // MovR16R16
    {0, 0, 16, 0, 0, 0, 0}, // MovE16R16
    {0, 16, 0, 0, 0, 0, 0}, // MovR16E16
    {0, 0, 0, 0, 0, 0, 0}, // MovR16I16
    {0, 0, 16, 0, 0, 0, 0}, // MovE16I16
    {0, 0, 0, 0, 0, 0, 0}, // MovR32R32 
    {0, 0, 32, 0, 0, 0, 0}, // MovE32R32
    {0, 32, 0, 0, 0, 0, 0}, // MovR32E32
    {0, 0, 0, 0, 0, 0, 0}, // MovR32I32
    {0, 0, 32, 0, 0, 0, 0}, // MovE32I32

    {0, 0, 0, 0, 0, 0, 0}, // MovR16S16
    {0, 0, 0, 0, 0, 0, 0}, // MovR32S16
    {0, 0, 16, 0, 0, 0, 0}, // MovE16S16
    {0, 0, 0, 0, 0, 0, 1}, // MovS16R16
    {0, 16, 0, 0, 0, 0, 1}, // MovS16E16

    {0, 8, 0, 0, 0, 0, 0}, // MovAlOb
    {0, 16, 0, 0, 0, 0, 0}, // MovAxOw
    {0, 32, 0, 0, 0, 0, 0}, // MovEaxOd
    {0, 0, 8, 0, 0, 0, 0}, // MovObAl
    {0, 0, 16, 0, 0, 0, 0}, // MovOwAx
    {0, 0, 32, 0, 0, 0, 0}, // MovOdEax

    {0, 0, 0, 0, 0, 0, 0}, // MovGwXzR8
    {0, 8, 0, 0, 0, 0, 0}, // MovGwXzE8
    {0, 0, 0, 0, 0, 0, 0}, // MovGwSxR8
    {0, 8, 0, 0, 0, 0, 0}, // MovGwSxE8

    {0, 0, 0, 0, 0, 0, 0}, // MovGdXzR8
    {0, 8, 0, 0, 0, 0, 0}, // MovGdXzE8
    {0, 0, 0, 0, 0, 0, 0}, // MovGdSxR8
    {0, 8, 0, 0, 0, 0, 0}, // MovGdSxE8

    {0, 0, 0, 0, 0, 0, 0}, // MovGdXzR16
    {0, 16, 0, 0, 0, 0, 0}, // MovGdXzE16
    {0, 0, 0, 0, 0, 0, 0}, // MovGdSxR16
    {0, 16, 0, 0, 0, 0, 0}, // MovGdSxE16

    {0, 0, 0, 0, 0, 0, 1}, // MovRdCRx,
    {0, 0, 0, 0, 0, 0, 1}, // MovCRxRd,

    {0, 0, 0, 0, 0, 0, 0}, // LeaR16E16
    {0, 0, 0, 0, 0, 0, 0}, // LeaR32E32

    {0, 0, 0, 0, 0, 0, 0}, // Nop
    {0, 0, 0, 0, 0, 0, 0}, // Cwd
    {0, 0, 0, 0, 0, 0, 0}, // Cwq
    {DECODE_BRANCH_NO_CACHE, 0, 32, 0, 0, 0, 1}, // CallAp
    {DECODE_BRANCH_NO_CACHE, 0, 64, 0, 0, 0, 1}, // CallFar
    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0, 1}, // JmpAp
    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0, 1}, // JmpFar
    {0, 0, 0, 0, 0, 0, 0}, // Wait
    {0, 0, 0, CF|AF|ZF|SF|PF, 0, 0, 0}, // Sahf - OF not part of lower 8-bits of flags
    {0, 0, 0, 0, CF|AF|ZF|SF|PF, 0, 0}, // Lahf - OF not part of lower 8-bits of flags
    {0, 0, 0, 0, CF, 0, 0}, // Salc
    {DECODE_BRANCH_NO_CACHE, 16, 0, 0, 0, 0, 0}, // Retn16Iw
    {DECODE_BRANCH_NO_CACHE, 32, 0, 0, 0, 0, 0}, // Retn32Iw
    {DECODE_BRANCH_NO_CACHE, 16, 0, 0, 0, 0, 0}, // Retn16
    {DECODE_BRANCH_NO_CACHE, 32, 0, 0, 0, 0, 0}, // Retn32
    {DECODE_BRANCH_NO_CACHE, 32, 0, 0, 0, 0, 1}, // Retf16
    {DECODE_BRANCH_NO_CACHE, 64, 0, 0, 0, 0, 1}, // Retf32
    // not sure about the read/write for this, these will run a signal which will push a bunch of stuff on the stack
    {DECODE_BRANCH_NO_CACHE, 0, 1, 0, 0, 0, 0}, // Invalid 
    {DECODE_BRANCH_NO_CACHE, 0, 1, 0, 0, 0, 0}, // Int3,
    {DECODE_BRANCH_NO_CACHE, 1, 1, 0, 0, 0, 0}, // Int80 Syscall
    {0, 1, 1, 0, 0, 0, 0}, // Int98 Wine callback
    {0, 1, 1, 0, 0, 0, 0}, // Int99 OpenGL callback
    {0, 1, 1, 0, 0, 0, 0}, // Int9A Vulkan callback
    {DECODE_BRANCH_NO_CACHE, 0, 1, 0, 0, 0, 0}, // IntIb,
    {DECODE_BRANCH_NO_CACHE, 0, 1, 0, OF, 0, 0}, // IntO,

    {DECODE_BRANCH_NO_CACHE, 48, 0, CF|SF|ZF|PF|OF|AF, 0, 0, 0}, // Iret
    {DECODE_BRANCH_NO_CACHE, 96, 0, CF|SF|ZF|PF|OF|AF, 0, 0, 0}, // Iret32
    {0, 1, 0, 0, 0, 0, 0}, // Xlat
    {DECODE_BRANCH_NO_CACHE, 0, 1, 0, 0, 0, 0}, // ICEBP
    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0, 0}, // Hlt
    {0, 0, 0, CF, CF, 0, 0}, // Cmc
    {0, 0, 0, CF, 0, 0, 0}, // Clc
    {0, 0, 0, CF, 0, 0, 0}, // Stc
    {0, 0, 0, IF, 0, 0, 0}, // Cli
    {0, 0, 0, IF, 0, 0, 0}, // Sti
    {0, 0, 0, DF, 0, 0, 0}, // Cld
    {0, 0, 0, DF, 0, 0, 0}, // Std
    {0, 0, 0, 0, 0, 0, 0}, // Rdtsc
    {0, 0, 0, 0, 0, 0, 0}, // CPUID

    {0, 0, 16, 0, 0, 0, 0}, // Enter16
    {0, 0, 32, 0, 0, 0, 0}, // Enter32
    {0, 16, 0, 0, 0, 0, 0}, // Leave16
    {0, 32, 0, 0, 0, 0, 0}, // Leave32
    
    {0, 32, 0, 0, 0, 0, 1}, // LoadSegment16
    {0, 48, 0, 0, 0, 0, 1}, // LoadSegment32

    {0, 8, 8, 0, 0, 0, 0}, // Movsb
    {0, 16, 16, 0, 0, 0, 0}, // Movsw,
    {0, 32, 32, 0, 0, 0, 0}, // Movsd,
    {0, 16, 0, CF|AF|ZF|SF|OF|PF|MAYBE, 0, 0, 0}, // Cmpsb
    {0, 32, 0, CF|AF|ZF|SF|OF|PF|MAYBE, 0, 0, 0}, // Cmpsw
    {0, 64, 0, CF|AF|ZF|SF|OF|PF|MAYBE, 0, 0, 0}, // Cmpsd
    {0, 0, 8, 0, 0, 0, 0}, // Stosb
    {0, 0, 16, 0, 0, 0, 0}, // Stosw
    {0, 0, 32, 0, 0, 0, 0}, // Stosd
    {0, 8, 0, 0, 0, 0, 0}, // Lodsb
    {0, 16, 0, 0, 0, 0, 0}, // Lodsw,
    {0, 32, 0, 0, 0, 0, 0}, // Lodsd,
    {0, 8, 0, CF|AF|ZF|SF|OF|PF|MAYBE, 0, 0, 0}, // Scasb
    {0, 16, 0, CF|AF|ZF|SF|OF|PF|MAYBE, 0, 0, 0}, // Scasw
    {0, 32, 0, CF|AF|ZF|SF|OF|PF|MAYBE, 0, 0, 0}, // Scasd

    {0, 0, 0, OF|CF, 0, 0, 0}, // RolR8I8
    {0, 8, 8, OF|CF, 0, 0, 0}, // RolE8I8
    {0, 0, 0, OF|CF, 0, 0, 0}, // RorR8I8 
    {0, 8, 8, OF|CF, 0, 0, 0}, // RorE8I8
    {0, 0, 0, OF|CF, CF, 0, 0}, // RclR8I8 
    {0, 8, 8, OF|CF, CF, 0, 0}, // RclE8I8
    {0, 0, 0, OF|CF, CF, 0, 0}, // RcrR8I8 
    {0, 8, 8, OF|CF, CF, 0, 0}, // RcrE8I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShlR8I8 
    {0, 8, 8, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShlE8I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShrR8I8 
    {0, 8, 8, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShrE8I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // SarR8I8 
    {0, 8, 8, CF|ZF|SF|OF|PF, 0, AF, 0}, // SarE8I8

    {0, 0, 0, OF|CF, 0, 0, 0}, // RolR16I8, 
    {0, 16, 16, OF|CF, 0, 0, 0}, // RolE16I8
    {0, 0, 0, OF|CF, 0, 0, 0}, // RorR16I8 
    {0, 16, 16, OF|CF, 0, 0, 0}, // RorE16I8
    {0, 0, 0, OF|CF, CF, 0, 0}, // RclR16I8 
    {0, 16, 16, OF|CF, CF, 0, 0}, // RclE16I8
    {0, 0, 0, OF|CF, CF, 0, 0}, // RcrR16I8 
    {0, 16, 16, OF|CF, CF, 0, 0}, // RcrE16I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShlR16I8 
    {0, 16, 16, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShlE16I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShrR16I8 
    {0, 16, 16, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShrE16I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // SarR16I8 
    {0, 16, 16, CF|ZF|SF|OF|PF, 0, AF, 0}, // SarE16I8

    {0, 0, 0, OF|CF, 0, 0, 0}, // RolR32I8 
    {0, 32, 32, OF|CF, 0, 0, 0}, // RolE32I8
    {0, 0, 0, OF|CF, 0, 0, 0}, // RorR32I8 
    {0, 32, 32, OF|CF, 0, 0, 0}, // RorE32I8
    {0, 0, 0, OF|CF, CF, 0, 0}, // RclR32I8 
    {0, 32, 32, OF|CF, CF, 0, 0}, // RclE32I8
    {0, 0, 0, OF|CF, CF, 0, 0}, // RcrR32I8 
    {0, 32, 32, OF|CF, CF, 0, 0}, // RcrE32I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShlR32I8 
    {0, 32, 32, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShlE32I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShrR32I8 
    {0, 32, 32, CF|ZF|SF|OF|PF, 0, AF, 0}, // ShrE32I8
    {0, 0, 0, CF|ZF|SF|OF|PF, 0, AF, 0}, // SarR32I8 
    {0, 32, 32, CF|ZF|SF|OF|PF, 0, AF, 0}, // SarE32I8

    {0, 0, 0, OF|CF, 0, 0, 0}, // RolR8Cl 
    {0, 8, 8, OF|CF, 0, 0, 0}, // RolE8Cl
    {0, 0, 0, OF|CF, 0, 0, 0}, // RorR8Cl 
    {0, 8, 8, OF|CF, 0, 0, 0}, // RorE8Cl
    {0, 0, 0, OF|CF, CF, 0, 0}, // RclR8Cl 
    {0, 8, 8, OF|CF, CF, 0, 0}, // RclE8Cl
    {0, 0, 0, OF|CF, CF, 0, 0}, // RcrR8Cl 
    {0, 8, 8, OF|CF, CF, 0, 0}, // RcrE8Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShlR8Cl 
    {0, 8, 8, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShlE8Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShrR8Cl 
    {0, 8, 8, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShrE8Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // SarR8Cl 
    {0, 8, 8, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // SarE8Cl

    {0, 0, 0, OF|CF, 0, 0, 0}, // RolR16Cl
    {0, 16, 16, OF|CF, 0, 0, 0}, // RolE16Cl
    {0, 0, 0, OF|CF, 0, 0, 0}, // RorR16Cl
    {0, 16, 16, OF|CF, 0, 0, 0}, // RorE16Cl
    {0, 0, 0, OF|CF, CF, 0, 0}, // RclR16Cl
    {0, 16, 16, OF|CF, CF, 0, 0}, // RclE16Cl
    {0, 0, 0, OF|CF, CF, 0, 0}, // RcrR16Cl
    {0, 16, 16, OF|CF, CF, 0, 0}, // RcrE16Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShlR16Cl
    {0, 16, 16, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShlE16Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShrR16Cl
    {0, 16, 16, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShrE16Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // SarR16Cl
    {0, 16, 16, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // SarE16Cl

    {0, 0, 0, OF|CF, 0, 0, 0}, // RolR32Cl
    {0, 32, 32, OF|CF, 0, 0, 0}, // RolE32Cl
    {0, 0, 0, OF|CF, 0, 0, 0}, // RorR32Cl
    {0, 32, 32, OF|CF, 0, 0, 0}, // RorE32Cl
    {0, 0, 0, OF|CF, CF, 0, 0}, // RclR32Cl
    {0, 32, 32, OF|CF, CF, 0, 0}, // RclE32Cl
    {0, 0, 0, OF|CF, CF, 0, 0}, // RcrR32Cl
    {0, 32, 32, OF|CF, CF, 0, 0}, // RcrE32Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShlR32Cl
    {0, 32, 32, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShlE32Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShrR32Cl
    {0, 32, 32, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // ShrE32Cl
    {0, 0, 0, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // SarR32Cl
    {0, 32, 32, CF|ZF|SF|OF|PF|MAYBE, 0, AF|MAYBE, 0}, // SarE32Cl

    {0, 0, 0, 0, 0, 0, 0}, // FADD_ST0_STj
    {0, 0, 0, 0, 0, 0, 0}, // FMUL_ST0_STj
    {0, 0, 0, 0, 0, 0, 0}, // FCOM_STi
    {0, 0, 0, 0, 0, 0, 0}, // FCOM_STi_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FSUB_ST0_STj
    {0, 0, 0, 0, 0, 0, 0}, // FSUBR_ST0_STj
    {0, 0, 0, 0, 0, 0, 0}, // FDIV_ST0_STj
    {0, 0, 0, 0, 0, 0, 0}, // FDIVR_ST0_STj
    {0, 32, 0, 0, 0, 0, 0}, // FADD_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FMUL_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FCOM_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FCOM_SINGLE_REAL_Pop
    {0, 32, 0, 0, 0, 0, 0}, // FSUB_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FSUBR_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FDIV_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FDIVR_SINGLE_REAL

    {0, 0, 0, 0, 0, 0, 0}, // FLD_STi
    {0, 0, 0, 0, 0, 0, 0}, // FXCH_STi
    {0, 0, 0, 0, 0, 0, 0}, // FNOP
    {0, 0, 0, 0, 0, 0, 0}, // FST_STi_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FCHS
    {0, 0, 0, 0, 0, 0, 0}, // FABS
    {0, 0, 0, 0, 0, 0, 0}, // FTST
    {0, 0, 0, 0, 0, 0, 0}, // FXAM
    {0, 0, 0, 0, 0, 0, 0}, // FLD1
    {0, 0, 0, 0, 0, 0, 0}, // FLDL2T
    {0, 0, 0, 0, 0, 0, 0}, // FLDL2E
    {0, 0, 0, 0, 0, 0, 0}, // FLDPI
    {0, 0, 0, 0, 0, 0, 0}, // FLDLG2
    {0, 0, 0, 0, 0, 0, 0}, // FLDLN2
    {0, 0, 0, 0, 0, 0, 0}, // FLDZ
    {0, 0, 0, 0, 0, 0, 0}, // F2XM1
    {0, 0, 0, 0, 0, 0, 0}, // FYL2X
    {0, 0, 0, 0, 0, 0, 0}, // FPTAN
    {0, 0, 0, 0, 0, 0, 0}, // FPATAN
    {0, 0, 0, 0, 0, 0, 0}, // FXTRACT
    {0, 0, 0, 0, 0, 0, 0}, // FPREM_nearest
    {0, 0, 0, 0, 0, 0, 0}, // FDECSTP
    {0, 0, 0, 0, 0, 0, 0}, // FINCSTP
    {0, 0, 0, 0, 0, 0, 0}, // FPREM
    {0, 0, 0, 0, 0, 0, 0}, // FYL2XP1
    {0, 0, 0, 0, 0, 0, 0}, // FSQRT
    {0, 0, 0, 0, 0, 0, 0}, // FSINCOS
    {0, 0, 0, 0, 0, 0, 0}, // FRNDINT
    {0, 0, 0, 0, 0, 0, 0}, // FSCALE
    {0, 0, 0, 0, 0, 0, 0}, // FSIN
    {0, 0, 0, 0, 0, 0, 0}, // FCOS
    {0, 32, 0, 0, 0, 0, 0}, // FLD_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FST_SINGLE_REAL
    {0, 32, 0, 0, 0, 0, 0}, // FST_SINGLE_REAL_Pop
    {0, 224, 0, 0, 0, 0, 0}, // FLDENV
    {0, 16, 0, 0, 0, 0, 0}, // FLDCW
    {0, 0, 224, 0, 0, 0, 0}, // FNSTENV
    {0, 0, 16, 0, 0, 0, 0}, // FNSTCW

    {0, 0, 0, 0, CF, 0, 0}, // FCMOV_ST0_STj_CF
    {0, 0, 0, 0, ZF, 0, 0}, // FCMOV_ST0_STj_ZF
    {0, 0, 0, 0, CF|ZF, 0, 0}, // FCMOV_ST0_STj_CF_OR_ZF
    {0, 0, 0, 0, PF, 0, 0}, // FCMOV_ST0_STj_PF
    {0, 0, 0, 0, 0, 0, 0}, // FUCOMPP
    {0, 32, 0, 0, 0, 0, 0}, // FIADD_DWORD_INTEGER
    {0, 32, 0, 0, 0, 0, 0}, // FIMUL_DWORD_INTEGER
    {0, 32, 0, 0, 0, 0, 0}, // FICOM_DWORD_INTEGER
    {0, 32, 0, 0, 0, 0, 0}, // FICOM_DWORD_INTEGER_Pop
    {0, 32, 0, 0, 0, 0, 0}, // FISUB_DWORD_INTEGER
    {0, 32, 0, 0, 0, 0, 0}, // FISUBR_DWORD_INTEGER
    {0, 32, 0, 0, 0, 0, 0}, // FIDIV_DWORD_INTEGER
    {0, 32, 0, 0, 0, 0, 0}, // FIDIVR_DWORD_INTEGER

    {0, 0, 0, 0, CF, 0, 0}, // FCMOV_ST0_STj_NCF
    {0, 0, 0, 0, ZF, 0, 0}, // FCMOV_ST0_STj_NZF
    {0, 0, 0, 0, CF|ZF, 0, 0}, // FCMOV_ST0_STj_NCF_AND_NZF
    {0, 0, 0, 0, PF, 0, 0}, // FCMOV_ST0_STj_NPF
    {0, 0, 0, 0, 0, 0, 0}, // FNCLEX
    {0, 0, 0, 0, 0, 0, 0}, // FNINIT
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // FUCOMI_ST0_STj
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // FCOMI_ST0_STj
    {0, 32, 0, 0, 0, 0, 0}, // FILD_DWORD_INTEGER
    {0, 0, 32, 0, 0, 0, 0}, // FISTTP32
    {0, 0, 32, 0, 0, 0, 0}, // FIST_DWORD_INTEGER
    {0, 0, 32, 0, 0, 0, 0}, // FIST_DWORD_INTEGER_Pop
    {0, 80, 0, 0, 0, 0, 0}, // FLD_EXTENDED_REAL
    {0, 0, 80, 0, 0, 0, 0}, // FSTP_EXTENDED_REAL

    {0, 0, 0, 0, 0, 0, 0}, // FADD_STi_ST0
    {0, 0, 0, 0, 0, 0, 0}, // FMUL_STi_ST0
    {0, 0, 0, 0, 0, 0, 0}, // FSUBR_STi_ST0
    {0, 0, 0, 0, 0, 0, 0}, // FSUB_STi_ST0
    {0, 0, 0, 0, 0, 0, 0}, // FDIVR_STi_ST0
    {0, 0, 0, 0, 0, 0, 0}, // FDIV_STi_ST0
    {0, 64, 0, 0, 0, 0, 0}, // FADD_DOUBLE_REAL
    {0, 64, 0, 0, 0, 0, 0}, // FMUL_DOUBLE_REAL
    {0, 64, 0, 0, 0, 0, 0}, // FCOM_DOUBLE_REAL
    {0, 64, 0, 0, 0, 0, 0}, // FCOM_DOUBLE_REAL_Pop
    {0, 64, 0, 0, 0, 0, 0}, // FSUB_DOUBLE_REAL
    {0, 64, 0, 0, 0, 0, 0}, // FSUBR_DOUBLE_REAL
    {0, 64, 0, 0, 0, 0, 0}, // FDIV_DOUBLE_REAL
    {0, 64, 0, 0, 0, 0, 0}, // FDIVR_DOUBLE_REAL

    {0, 0, 0, 0, 0, 0, 0}, // FFREE_STi
    {0, 0, 0, 0, 0, 0, 0}, // FST_STi
    {0, 0, 0, 0, 0, 0, 0}, // FUCOM_STi
    {0, 0, 0, 0, 0, 0, 0}, // FUCOM_STi_Pop
    {0, 64, 0, 0, 0, 0, 0}, // FLD_DOUBLE_REAL
    {0, 0, 64, 0, 0, 0, 0}, // FISTTP64
    {0, 0, 64, 0, 0, 0, 0}, // FST_DOUBLE_REAL
    {0, 0, 64, 0, 0, 0, 0}, // FST_DOUBLE_REAL_Pop
    {0, 668, 0, 0, 0, 0, 0}, // FRSTOR
    {0, 0, 668, 0, 0, 0, 0}, // FNSAVE
    {0, 0, 16, 0, 0, 0, 0}, // FNSTSW

    {0, 0, 0, 0, 0, 0, 0}, // FADD_STi_ST0_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FMUL_STi_ST0_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FCOMPP
    {0, 0, 0, 0, 0, 0, 0}, // FSUBR_STi_ST0_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FSUB_STi_ST0_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FDIVR_STi_ST0_Pop
    {0, 0, 0, 0, 0, 0, 0}, // FDIV_STi_ST0_Pop
    {0, 16, 0, 0, 0, 0, 0}, // FIADD_WORD_INTEGER
    {0, 16, 0, 0, 0, 0, 0}, // FIMUL_WORD_INTEGER
    {0, 16, 0, 0, 0, 0, 0}, // FICOM_WORD_INTEGER
    {0, 16, 0, 0, 0, 0, 0}, // FICOM_WORD_INTEGER_Pop
    {0, 16, 0, 0, 0, 0, 0}, // FISUB_WORD_INTEGER
    {0, 16, 0, 0, 0, 0, 0}, // FISUBR_WORD_INTEGER
    {0, 16, 0, 0, 0, 0, 0}, // FIDIV_WORD_INTEGER
    {0, 16, 0, 0, 0, 0, 0}, // FIDIVR_WORD_INTEGER

    {0, 0, 0, 0, 0, 0, 0}, // FFREEP_STi
    {0, 0, 0, 0, 0, 0, 0}, // FNSTSW_AX
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // FUCOMI_ST0_STj_Pop
    {0, 0, 0, CF|AF|ZF|SF|OF|PF, 0, 0, 0}, // FCOMI_ST0_STj_Pop
    {0, 16, 0, 0, 0, 0, 0}, // FILD_WORD_INTEGER
    {0, 0, 16, 0, 0, 0, 0}, // FISTTP16
    {0, 0, 16, 0, 0, 0, 0}, // FIST_WORD_INTEGER
    {0, 0, 16, 0, 0, 0, 0}, // FIST_WORD_INTEGER_Pop
    {0, 80, 0, 0, 0, 0, 0}, // FBLD_PACKED_BCD
    {0, 64, 0, 0, 0, 0, 0}, // FILD_QWORD_INTEGER
    {0, 0, 80, 0, 0, 0, 0}, // FBSTP_PACKED_BCD
    {0, 0, 64, 0, 0, 0, 0}, // FISTP_QWORD_INTEGER

    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, ZF, 0}, // LoopNZ
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, ZF, 0}, // LoopZ
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, 0, 0}, // Loop
    {DECODE_BRANCH_1|DECODE_BRANCH_2, 0, 0, 0, 0, 0}, // Jcxz

    {0, 0, 0, 0, 0, 0}, // InAlIb
    {0, 0, 0, 0, 0, 0}, // InAxIb
    {0, 0, 0, 0, 0, 0}, // InEaxIb
    {0, 0, 0, 0, 0, 0}, // OutIbAl
    {0, 0, 0, 0, 0, 0}, // OutIbAx
    {0, 0, 0, 0, 0, 0}, // OutIbEax
    {0, 0, 0, 0, 0, 0}, // InAlDx
    {0, 0, 0, 0, 0, 0}, // InAxDx
    {0, 0, 0, 0, 0, 0}, // InEaxDx
    {0, 0, 0, 0, 0, 0}, // OutDxAl
    {0, 0, 0, 0, 0, 0}, // OutDxAx
    {0, 0, 0, 0, 0, 0}, // OutDxEax

    {DECODE_BRANCH_1, 0, 16, 0, 0, 0, 0}, // CallJw
    {DECODE_BRANCH_1, 0, 32, 0, 0, 0, 0}, // CallJd
    {DECODE_BRANCH_1, 0, 0, 0, 0, 0, 0}, // JmpJw
    {DECODE_BRANCH_1, 0, 0, 0, 0, 0, 0}, // JmpJd
    {DECODE_BRANCH_1, 0, 0, 0, 0, 0, 0}, // JmpJb
    {DECODE_BRANCH_NO_CACHE, 0, 16, 0, 0, 0, 0}, // CallR16 
    {DECODE_BRANCH_NO_CACHE, 0, 32, 0, 0, 0, 0}, // CallR32 
    {DECODE_BRANCH_NO_CACHE, 16, 16, 0, 0, 0, 0}, // CallE16
    {DECODE_BRANCH_NO_CACHE, 32, 32, 0, 0, 0, 0}, // CallE32
    {DECODE_BRANCH_NO_CACHE, 32, 32, 0, 0, 0, 1}, // CallFarE16
    {DECODE_BRANCH_NO_CACHE, 64, 64, 0, 0, 0, 1}, // CallFarE32
    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0, 0}, // JmpR16 
    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0, 0}, // JmpR32 
    {DECODE_BRANCH_NO_CACHE, 16, 0, 0, 0, 0, 0}, // JmpE16
    {DECODE_BRANCH_NO_CACHE, 32, 0, 0, 0, 0, 0}, // JmpE32
    {DECODE_BRANCH_NO_CACHE, 32, 0, 0, 0, 0, 1}, // JmpFarE16
    {DECODE_BRANCH_NO_CACHE, 64, 0, 0, 0, 0, 1}, // JmpFarE32

    {0, 0, 0, ZF, 0, 0, 0}, // LarR16R16
    {0, 16, 0, ZF, 0, 0, 0}, // LarR16E16
    {0, 0, 0, ZF, 0, 0, 0}, // LslR16R16
    {0, 16, 0, ZF, 0, 0, 0}, // LslR16E16
    {0, 0, 0, ZF, 0, 0, 0}, // LslR32R32
    {0, 16, 0, ZF, 0, 0, 0}, // LslR32E32 (intentional 16-bit read)

    {0, 0, 0, 0, OF, 0, 0}, // CmovO_R16R16 
    {0, 16, 0, 0, OF, 0, 0}, // CmovO_R16E16
    {0, 0, 0, 0, OF, 0, 0}, // CmovNO_R16R16 
    {0, 16, 0, 0, OF, 0, 0}, // CmovNO_R16E16
    {0, 0, 0, 0, CF, 0, 0}, // CmovB_R16R16 
    {0, 16, 0, 0, CF, 0, 0}, // CmovB_R16E16
    {0, 0, 0, 0, CF, 0, 0}, // CmovNB_R16R16 
    {0, 16, 0, 0, CF, 0, 0}, // CmovNB_R16E16
    {0, 0, 0, 0, ZF, 0, 0}, // CmovZ_R16R16 
    {0, 16, 0, 0, ZF, 0, 0}, // CmovZ_R16E16
    {0, 0, 0, 0, ZF, 0, 0}, // CmovNZ_R16R16 
    {0, 16, 0, 0, ZF, 0, 0}, // CmovNZ_R16E16
    {0, 0, 0, 0, CF|ZF, 0, 0}, // CmovBE_R16R16 
    {0, 16, 0, 0, CF|ZF, 0, 0}, // CmovBE_R16E16
    {0, 0, 0, 0, CF|ZF, 0, 0}, // CmovNBE_R16R16 
    {0, 16, 0, 0, CF|ZF, 0, 0}, // CmovNBE_R16E16
    {0, 0, 0, 0, SF, 0, 0}, // CmovS_R16R16 
    {0, 16, 0, 0, SF, 0, 0}, // CmovS_R16E16
    {0, 0, 0, 0, SF, 0, 0}, // CmovNS_R16R16 
    {0, 16, 0, 0, SF, 0, 0}, // CmovNS_R16E16
    {0, 0, 0, 0, PF, 0, 0}, // CmovP_R16R16 
    {0, 16, 0, 0, PF, 0, 0}, // CmovP_R16E16
    {0, 0, 0, 0, PF, 0, 0}, // CmovNP_R16R16 
    {0, 16, 0, 0, PF, 0, 0}, // CmovNP_R16E16
    {0, 0, 0, 0, SF|OF, 0, 0}, // CmovL_R16R16 
    {0, 16, 0, 0, SF|OF, 0, 0}, // CmovL_R16E16
    {0, 0, 0, 0, SF|OF, 0, 0}, // CmovNL_R16R16 
    {0, 16, 0, 0, SF|OF, 0, 0}, // CmovNL_R16E16
    {0, 0, 0, 0, SF|OF|ZF, 0, 0}, // CmovLE_R16R16 
    {0, 16, 0, 0, SF|OF|ZF, 0, 0}, // CmovLE_R16E16
    {0, 0, 0, 0, SF|OF|ZF, 0, 0}, // CmovNLE_R16R16 
    {0, 16, 0, 0, SF|OF|ZF, 0, 0}, // CmovNLE_R16E16

    {0, 0, 0, 0, OF, 0, 0}, // CmovO_R32R32 
    {0, 32, 0, 0, OF, 0, 0}, // CmovO_R32E32
    {0, 0, 0, 0, OF, 0, 0}, // CmovNO_R32R32 
    {0, 32, 0, 0, OF, 0, 0}, // CmovNO_R32E32
    {0, 0, 0, 0, CF, 0, 0}, // CmovB_R32R32 
    {0, 32, 0, 0, CF, 0, 0}, // CmovB_R32E32
    {0, 0, 0, 0, CF, 0, 0}, // CmovNB_R32R32 
    {0, 32, 0, 0, CF, 0, 0}, // CmovNB_R32E32
    {0, 0, 0, 0, ZF, 0, 0}, // CmovZ_R32R32 
    {0, 32, 0, 0, ZF, 0, 0}, // CmovZ_R32E32
    {0, 0, 0, 0, ZF, 0, 0}, // CmovNZ_R32R32 
    {0, 32, 0, 0, ZF, 0, 0}, // CmovNZ_R32E32
    {0, 0, 0, 0, CF|ZF, 0, 0}, // CmovBE_R32R32 
    {0, 32, 0, 0, CF|ZF, 0, 0}, // CmovBE_R32E32
    {0, 0, 0, 0, CF|ZF, 0, 0}, // CmovNBE_R32R32 
    {0, 32, 0, 0, CF|ZF, 0, 0}, // CmovNBE_R32E32
    {0, 0, 0, 0, SF, 0, 0}, // CmovS_R32R32 
    {0, 32, 0, 0, SF, 0, 0}, // CmovS_R32E32
    {0, 0, 0, 0, SF, 0, 0}, // CmovNS_R32R32 
    {0, 32, 0, 0, SF, 0, 0}, // CmovNS_R32E32
    {0, 0, 0, 0, PF, 0, 0}, // CmovP_R32R32 
    {0, 32, 0, 0, PF, 0, 0}, // CmovP_R32E32
    {0, 0, 0, 0, PF, 0, 0}, // CmovNP_R32R32 
    {0, 32, 0, 0, PF, 0, 0}, // CmovNP_R32E32
    {0, 0, 0, 0, SF|OF, 0, 0}, // CmovL_R32R32 
    {0, 32, 0, 0, SF|OF, 0, 0}, // CmovL_R32E32
    {0, 0, 0, 0, SF|OF, 0, 0}, // CmovNL_R32R32 
    {0, 32, 0, 0, SF|OF, 0, 0}, // CmovNL_R32E32
    {0, 0, 0, 0, SF|OF|ZF, 0, 0}, // CmovLE_R32R32 
    {0, 32, 0, 0, SF|OF|ZF, 0, 0}, // CmovLE_R32E32
    {0, 0, 0, 0, SF|OF|ZF, 0, 0}, // CmovNLE_R32R32 
    {0, 32, 0, 0, SF|OF|ZF, 0, 0}, // CmovNLE_R32E32

    {0, 0, 0, 0, OF, 0, 0}, // SetO_R8 
    {0, 0, 8, 0, OF, 0, 0}, // SetO_E8
    {0, 0, 0, 0, OF, 0, 0}, // SetNO_R8 
    {0, 0, 8, 0, OF, 0, 0}, // SetNO_E8
    {0, 0, 0, 0, CF, 0, 0}, // SetB_R8 
    {0, 0, 8, 0, CF, 0, 0}, // SetB_E8
    {0, 0, 0, 0, CF, 0, 0}, // SetNB_R8 
    {0, 0, 8, 0, CF, 0, 0}, // SetNB_E8
    {0, 0, 0, 0, ZF, 0, 0}, // SetZ_R8 
    {0, 0, 8, 0, ZF, 0, 0}, // SetZ_E8
    {0, 0, 0, 0, ZF, 0, 0}, // SetNZ_R8 
    {0, 0, 8, 0, ZF, 0, 0}, // SetNZ_E8
    {0, 0, 0, 0, CF|ZF, 0, 0}, // SetBE_R8 
    {0, 0, 8, 0, CF|ZF, 0, 0}, // SetBE_E8
    {0, 0, 0, 0, CF|ZF, 0, 0}, // SetNBE_R8 
    {0, 0, 8, 0, CF|ZF, 0, 0}, // SetNBE_E8
    {0, 0, 0, 0, SF, 0, 0}, // SetS_R8 
    {0, 0, 8, 0, SF, 0, 0}, // SetS_E8
    {0, 0, 0, 0, SF, 0, 0}, // SetNS_R8 
    {0, 0, 8, 0, SF, 0, 0}, // SetNS_E8
    {0, 0, 0, 0, PF, 0, 0}, // SetP_R8 
    {0, 0, 8, 0, PF, 0, 0}, // SetP_E8
    {0, 0, 0, 0, PF, 0, 0}, // SetNP_R8 
    {0, 0, 8, 0, PF, 0, 0}, // SetNP_E8
    {0, 0, 0, 0, SF|OF, 0, 0}, // SetL_R8 
    {0, 0, 8, 0, SF|OF, 0, 0}, // SetL_E8
    {0, 0, 0, 0, SF|OF, 0, 0}, // SetNL_R8 
    {0, 0, 8, 0, SF|OF, 0, 0}, // SetNL_E8
    {0, 0, 0, 0, SF|OF|ZF, 0, 0}, // SetLE_R8 
    {0, 0, 8, 0, SF|OF|ZF, 0, 0}, // SetLE_E8
    {0, 0, 0, 0, SF|OF|ZF, 0, 0}, // SetNLE_R8 
    {0, 0, 8, 0, SF|OF|ZF, 0, 0}, // SetNLE_E8

    {0, 0, 0, 0, 0, 0, 0}, // SLDTReg 
    {0, 0, 16, 0, 0, 0, 0}, // SLDTE16
    {0, 0, 0, 0, 0, 0, 0}, // STRReg 
    {0, 0, 16, 0, 0, 0, 0}, // STRE16
    {0, 0, 0, 0, 0, 0, 0}, // LLDTR16 
    {0, 16, 0, 0, 0, 0, 0}, // LLDTE16
    {0, 0, 0, 0, 0, 0, 0}, // LTRR16 
    {0, 16, 0, 0, 0, 0, 0}, // LTRE16
    {0, 0, 0, 0, 0, 0, 0}, // VERRR16 
    {0, 16, 0, 0, 0, 0, 0}, // VERRE16
    {0, 0, 0, 0, 0, 0, 0}, // VERWR16 
    {0, 16, 0, 0, 0, 0, 0}, // VERWE16

    {0, 0, 48, 0, 0, 0, 0}, // SGDT
    {0, 0, 48, 0, 0, 0, 0}, // SIDT
    {0, 48, 0, 0, 0, 0, 0}, // LGDT
    {0, 48, 0, 0, 0, 0, 0}, // LIDT
    {0, 0, 0, 0, 0, 0, 0}, // SMSWRreg 
    {0, 0, 16, 0, 0, 0, 0}, // SMSW
    {0, 0, 0, 0, 0, 0, 1}, // LMSWRreg 
    {0, 16, 0, 0, 0, 0, 1}, // LMSW
    {0, 0, 0, 0, 0, 0, 0}, // INVLPG

    { 0, 0, 0, CF | AF | ZF | SF | OF | PF, 0, 0, 0 }, // XaddR8R8 
    { 0, 8, 8, CF | AF | ZF | SF | OF | PF, 0, 0, 0 }, // XaddR8E8
    { 0, 0, 0, CF | AF | ZF | SF | OF | PF, 0, 0, 0 }, // XaddR16R16 
    { 0, 16, 16, CF | AF | ZF | SF | OF | PF, 0, 0, 0 }, // XaddR16E16
    { 0, 0, 0, CF | AF | ZF | SF | OF | PF, 0, 0, 0 }, // XaddR32R32 
    { 0, 32, 32, CF | AF | ZF | SF | OF | PF, 0, 0, 0 }, // XaddR32E32
    {0, 64, 64, ZF, 0, 0, 0}, // CmpXchg8b
    {0, 0, 0, 0, 0, 0, 0}, // Bswap32

    {0, 0, 0, 0, 0, 0, 0}, // PunpcklbwMmx
    {0, 64, 0, 0, 0, 0, 0}, // PunpcklbwE64
    {0, 0, 0, 0, 0, 0, 0}, // PunpcklwdMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PunpcklwdE64
    {0, 0, 0, 0, 0, 0, 0}, // PunpckldqMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PunpckldqE64
    {0, 0, 0, 0, 0, 0, 0}, // PacksswbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PacksswbE64
    {0, 0, 0, 0, 0, 0, 0}, // PcmpgtbMmx
    {0, 64, 0, 0, 0, 0, 0}, // PcmpgtbE64
    {0, 0, 0, 0, 0, 0, 0}, // PcmpgtwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PcmpgtwE64
    {0, 0, 0, 0, 0, 0, 0}, // PcmpgtdMmx
    {0, 64, 0, 0, 0, 0, 0}, // PcmpgtdE64
    {0, 0, 0, 0, 0, 0, 0}, // PackuswbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PackuswbE64
    {0, 0, 0, 0, 0, 0, 0}, // PunpckhbwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PunpckhbwE64
    {0, 0, 0, 0, 0, 0, 0}, // PunpckhwdMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PunpckhwdE64
    {0, 0, 0, 0, 0, 0, 0}, // PunpckhdqMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PunpckhdqE64
    {0, 0, 0, 0, 0, 0, 0}, // PackssdwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PackssdwE64
    {0, 0, 0, 0, 0, 0, 0}, // MovPqR32, 
    {0, 32, 0, 0, 0, 0, 0}, // MovPqE32,
    {0, 0, 0, 0, 0, 0, 0}, // MovPqMmx, 
    {0, 64, 0, 0, 0, 0, 0}, // MovPqE64,
    {0, 0, 0, 0, 0, 0, 0}, // Psrlw 
    {0, 0, 0, 0, 0, 0, 0}, // Psraw 
    {0, 0, 0, 0, 0, 0, 0}, // Psllw
    {0, 0, 0, 0, 0, 0, 0}, // Psrld 
    {0, 0, 0, 0, 0, 0, 0}, // Psrad 
    {0, 0, 0, 0, 0, 0, 0}, // Pslld
    {0, 0, 0, 0, 0, 0, 0}, // Psrlq 
    {0, 0, 0, 0, 0, 0, 0}, // Psllq
    {0, 0, 0, 0, 0, 0, 0}, // PcmpeqbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PcmpeqbE64
    {0, 0, 0, 0, 0, 0, 0}, // PcmpeqwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PcmpeqwE64
    {0, 0, 0, 0, 0, 0, 0}, // PcmpeqdMmx
    {0, 64, 0, 0, 0, 0, 0}, // PcmpeqdE64
    {0, 0, 0, 0, 0, 0, 0}, // Emms
    {0, 0, 0, 0, 0, 0, 0}, // MovR32Pq 
    {0, 0, 32, 0, 0, 0, 0}, // MovE32Pq
    {0, 0, 0, 0, 0, 0, 0}, // MovMmxPq
    {0, 0, 64, 0, 0, 0, 0}, // MovE64Pq
    {0, 0, 0, 0, 0, 0, 0}, // PsrlwMmx
    {0, 64, 0, 0, 0, 0, 0}, // PsrlwE64
    {0, 0, 0, 0, 0, 0, 0}, // PsrldMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsrldE64
    {0, 0, 0, 0, 0, 0, 0}, // PsrlqMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsrlqE64
    {0, 0, 0, 0, 0, 0, 0}, // PmullwMmx
    {0, 64, 0, 0, 0, 0, 0}, // PmullwE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubusbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubusbE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubuswMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubuswE64
    {0, 0, 0, 0, 0, 0, 0}, // PandMmx
    {0, 64, 0, 0, 0, 0, 0}, // PandE64
    {0, 0, 0, 0, 0, 0, 0}, // PaddusbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PaddusbE64
    {0, 0, 0, 0, 0, 0, 0}, // PadduswMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PadduswE64
    {0, 0, 0, 0, 0, 0, 0}, // PandnMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PandnE64
    {0, 0, 0, 0, 0, 0, 0}, // PsrawMmx
    {0, 64, 0, 0, 0, 0, 0}, // PsrawE64
    {0, 0, 0, 0, 0, 0, 0}, // PsradMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsradE64
    {0, 0, 0, 0, 0, 0, 0}, // PmulhwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PmulhwE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubsbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubsbE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubswMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubswE64
    {0, 0, 0, 0, 0, 0, 0}, // PorMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PorE64
    {0, 0, 0, 0, 0, 0, 0}, // PaddsbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PaddsbE64
    {0, 0, 0, 0, 0, 0, 0}, // PaddswMmx
    {0, 64, 0, 0, 0, 0, 0}, // PaddswE64
    {0, 0, 0, 0, 0, 0, 0}, // PxorMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PxorE64
    {0, 0, 0, 0, 0, 0, 0}, // PsllwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsllwE64
    {0, 0, 0, 0, 0, 0, 0}, // PslldMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PslldE64
    {0, 0, 0, 0, 0, 0, 0}, // PsllqMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsllqE64
    {0, 0, 0, 0, 0, 0, 0}, // PmaddwdMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PmaddwdE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubbE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubwE64
    {0, 0, 0, 0, 0, 0, 0}, // PsubdMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PsubdE64
    {0, 0, 0, 0, 0, 0, 0}, // PaddbMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PaddbE64
    {0, 0, 0, 0, 0, 0, 0}, // PaddwMmx 
    {0, 64, 0, 0, 0, 0, 0}, // PaddwE64
    {0, 0, 0, 0, 0, 0, 0}, // PadddMmx 
    {0, 64, 0, 0, 0, 0}, // PadddE64

    {0, 0, 512*8, 0, 0, 0}, // Fxsave
    {0, 512*8, 0, 0, 0, 0}, // Fxrstor
    {0, 0, 32, 0, 0, 0}, // Ldmxcsr
    {0, 32, 0, 0, 0, 0}, // Stmxcsr
    {0, 0, 512*8, 0, 0, 0}, // Xsave :TODO: just a guess for the size
    {0, 0, 0, 0, 0, 0}, // Lfence
    {0, 512*8, 0, 0, 0, 0}, // Xrstor
    {0, 0, 0, 0, 0, 0}, // Mfence
    {0, 0, 0, 0, 0, 0}, // Sfence
    {0, 0, 0, 0, 0, 0}, // Clflush

    // SSE1
    {0, 0, 0, 0, 0, 0}, // AddpsXmm
    {0, 128, 0, 0, 0, 0}, // AddpsE128
    {0, 0, 0, 0, 0, 0}, // AddssXmm
    {0, 32, 0, 0, 0, 0}, // AddssE32
    {0, 0, 0, 0, 0, 0}, // SubpsXmm
    {0, 128, 0, 0, 0, 0}, // SubpsE128
    {0, 0, 0, 0, 0, 0}, // SubssXmm
    {0, 32, 0, 0, 0, 0}, // SubssE32
    {0, 0, 0, 0, 0, 0}, // MulpsXmm
    {0, 128, 0, 0, 0, 0}, // MulpsE128
    {0, 0, 0, 0, 0, 0}, // MulssXmm
    {0, 32, 0, 0, 0, 0}, // MulssE32
    {0, 0, 0, 0, 0, 0}, // DivpsXmm
    {0, 128, 0, 0, 0, 0}, // DivpsE128
    {0, 0, 0, 0, 0, 0}, // DivssXmm
    {0, 32, 0, 0, 0, 0}, // DivssE32
    {0, 0, 0, 0, 0, 0}, // RcppsXmm
    {0, 128, 0, 0, 0, 0}, // RcppsE128
    {0, 0, 0, 0, 0, 0}, // RcpssXmm
    {0, 32, 0, 0, 0, 0}, // RcpssE32
    {0, 0, 0, 0, 0, 0}, // SqrtpsXmm
    {0, 128, 0, 0, 0, 0}, // SqrtpsE128
    {0, 0, 0, 0, 0, 0}, // SqrtssXmm
    {0, 32, 0, 0, 0, 0}, // SqrtssE32
    {0, 0, 0, 0, 0, 0}, // RsqrtpsXmm
    {0, 128, 0, 0, 0, 0}, // RsqrtpsE128
    {0, 0, 0, 0, 0, 0}, // RsqrtssXmm
    {0, 32, 0, 0, 0, 0}, // RsqrtssE32
    {0, 0, 0, 0, 0, 0}, // MaxpsXmm
    {0, 128, 0, 0, 0, 0}, // MaxpsE128
    {0, 0, 0, 0, 0, 0}, // MaxssXmm
    {0, 32, 0, 0, 0, 0}, // MaxssE32
    {0, 0, 0, 0, 0, 0}, // MinpsXmm
    {0, 128, 0, 0, 0, 0}, // MinpsE128
    {0, 0, 0, 0, 0, 0}, // MinssXmm
    {0, 32, 0, 0, 0, 0}, // MinssE32
    {0, 0, 0, 0, 0, 0}, // PavgbMmxMmx
    {0, 64, 0, 0, 0, 0}, // PavgbMmxE64
    {0, 0, 0, 0, 0, 0}, // PavgbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PavgbXmmE128
    {0, 0, 0, 0, 0, 0}, // PavgwMmxMmx
    {0, 64, 0, 0, 0, 0}, // PavgwMmxE64
    {0, 0, 0, 0, 0, 0}, // PavgwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PavgwXmmE128
    {0, 0, 0, 0, 0, 0}, // PsadbwMmxMmx
    {0, 64, 0, 0, 0, 0}, // PsadbwMmxE64
    {0, 0, 0, 0, 0, 0}, // PsadbwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsadbwXmmE128
    {0, 0, 0, 0, 0, 0}, // PextrwR32Mmx
    {0, 0, 16, 0, 0, 0}, // PextrwE16Mmx
    {0, 0, 0, 0, 0, 0}, // PextrwR32Xmm
    {0, 0, 16, 0, 0, 0}, // PextrwE16Xmm
    {0, 0, 0, 0, 0, 0}, // PinsrwMmxR32
    {0, 16, 0, 0, 0, 0}, // PinsrwMmxE16
    {0, 0, 0, 0, 0, 0}, // PinsrwXmmR32
    {0, 16, 0, 0, 0, 0}, // PinsrwXmmE16
    {0, 0, 0, 0, 0, 0}, // PmaxswMmxMmx
    {0, 64, 0, 0, 0, 0}, // PmaxswMmxE64
    {0, 0, 0, 0, 0, 0}, // PmaxswXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmaxswXmmE128
    {0, 0, 0, 0, 0, 0}, // PmaxubMmxMmx
    {0, 64, 0, 0, 0, 0}, // PmaxubMmxE64
    {0, 0, 0, 0, 0, 0}, // PmaxubXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmaxubXmmE128
    {0, 0, 0, 0, 0, 0}, // PminswMmxMmx
    {0, 64, 0, 0, 0, 0}, // PminswMmxE64
    {0, 0, 0, 0, 0, 0}, // PminswXmmXmm
    {0, 128, 0, 0, 0, 0}, // PminswXmmE128
    {0, 0, 0, 0, 0, 0}, // PminubMmxMmx
    {0, 64, 0, 0, 0, 0}, // PminubMmxE64
    {0, 0, 0, 0, 0, 0}, // PminubXmmXmm
    {0, 128, 0, 0, 0, 0}, // PminubXmmE128
    {0, 0, 0, 0, 0, 0}, // PmovmskbR32Mmx
    {0, 0, 0, 0, 0, 0}, // PmovmskbR32Xmm
    {0, 0, 0, 0, 0, 0}, // PmulhuwMmxMmx
    {0, 64, 0, 0, 0, 0}, // PmulhuwMmxE64
    {0, 0, 0, 0, 0, 0}, // PmulhuwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmulhuwXmmE128
    {0, 0, 0, 0, 0, 0}, // PshufwMmxMmx
    {0, 64, 0, 0, 0, 0}, // PshufwMmxE64
    {0, 0, 0, 0, 0, 0}, // AndnpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // AndnpsXmmE128
    {0, 0, 0, 0, 0, 0}, // AndpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // AndpsXmmE128
    {0, 0, 0, 0, 0, 0}, // OrpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // OrpsXmmE128
    {0, 0, 0, 0, 0, 0}, // XorpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // XorpsXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtpi2psXmmMmx
    {0, 64, 0, 0, 0, 0}, // Cvtpi2psXmmE64
    {0, 0, 0, 0, 0, 0}, // Cvtps2piMmxXmm
    {0, 64, 0, 0, 0, 0}, // Cvtps2piMmxE64
    {0, 0, 0, 0, 0, 0}, // Cvtsi2ssXmmR32
    {0, 32, 0, 0, 0, 0}, // Cvtsi2ssXmmE32
    {0, 0, 0, 0, 0, 0}, // Cvtss2siR32Xmm
    {0, 32, 0, 0, 0, 0}, // Cvtss2siR32E32
    {0, 0, 0, 0, 0, 0}, // Cvttps2piMmxXmm
    {0, 64, 0, 0, 0, 0}, // Cvttps2piMmxE64
    {0, 0, 0, 0, 0, 0}, // Cvttss2siR32Xmm
    {0, 32, 0, 0, 0, 0}, // Cvttss2siR32E32
    {0, 0, 0, 0, 0, 0}, // MovapsXmmXmm
    {0, 128, 0, 0, 0, 0}, // MovapsXmmE128
    {0, 0, 128, 0, 0, 0}, // MovapsE128Xmm
    {0, 0, 0, 0, 0, 0}, // MovhlpsXmmXmm
    {0, 0, 0, 0, 0, 0}, // MovlhpsXmmXmm
    {0, 64, 0, 0, 0, 0}, // MovhpsXmmE64
    {0, 0, 64, 0, 0, 0}, // MovhpsE64Xmm
    {0, 64, 0, 0, 0, 0}, // MovlpsXmmE64
    {0, 0, 64, 0, 0, 0}, // MovlpsE64Xmm
    {0, 0, 0, 0, 0, 0}, // MovmskpsR32Xmm
    {0, 0, 0, 0, 0, 0}, // MovssXmmXmm
    {0, 32, 0, 0, 0, 0}, // MovssXmmE32
    {0, 0, 32, 0, 0, 0}, // MovssE32Xmm
    {0, 0, 0, 0, 0, 0}, // MovupsXmmXmm
    {0, 128, 0, 0, 0, 0}, // MovupsXmmE128
    {0, 0, 128, 0, 0, 0}, // MovupsE128Xmm
    {0, 0, 0, 0, 0, 0}, // MaskmovqEDIMmxMmx
    {0, 0, 128, 0, 0, 0}, // MovntpsE128Xmm
    {0, 0, 64, 0, 0, 0}, // MovntqE64Mmx
    {0, 0, 0, 0, 0, 0}, // ShufpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // ShufpsXmmE128
    {0, 0, 0, 0, 0, 0}, // UnpckhpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // UnpckhpsXmmE128
    {0, 0, 0, 0, 0, 0}, // UnpcklpsXmmXmm
    {0, 128, 0, 0, 0, 0}, // UnpcklpsXmmE128
    {0, 0, 0, 0, 0, 0}, // PrefetchT0
    {0, 0, 0, 0, 0, 0}, // PrefetchT1
    {0, 0, 0, 0, 0, 0}, // PrefetchT2
    {0, 0, 0, 0, 0, 0}, // PrefetchNTA
    {0, 0, 0, 0, 0, 0}, // CmppsXmmXmm
    {0, 128, 0, 0, 0, 0}, // CmppsXmmE128
    {0, 0, 0, 0, 0, 0}, // CmpssXmmXmm
    {0, 32, 0, 0, 0, 0}, // CmpssXmmE32
    {0, 0, 0, 0, 0, 0}, // ComissXmmXmm
    {0, 32, 0, 0, 0, 0}, // ComissXmmE32
    {0, 0, 0, 0, 0, 0}, // UcomissXmmXmm
    {0, 32, 0, 0, 0, 0}, // UcomissXmmE32

    {0, 0, 0, 0, 0, 0}, // AddpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // AddpdXmmE128
    {0, 0, 0, 0, 0, 0}, // AddsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // AddsdXmmE64
    {0, 0, 0, 0, 0, 0}, // SubpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // SubpdXmmE128
    {0, 0, 0, 0, 0, 0}, // SubsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // SubsdXmmE64
    {0, 0, 0, 0, 0, 0}, // MulpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // MulpdXmmE128
    {0, 0, 0, 0, 0, 0}, // MulsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // MulsdXmmE64
    {0, 0, 0, 0, 0, 0}, // DivpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // DivpdXmmE128
    {0, 0, 0, 0, 0, 0}, // DivsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // DivsdXmmE64
    {0, 0, 0, 0, 0, 0}, // MaxpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // MaxpdXmmE128
    {0, 0, 0, 0, 0, 0}, // MaxsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // MaxsdXmmE64
    {0, 0, 0, 0, 0, 0}, // MinpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // MinpdXmmE128
    {0, 0, 0, 0, 0, 0}, // MinsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // MinsdXmmE64
    {0, 0, 0, 0, 0, 0}, // PaddbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PaddbXmmE128
    {0, 0, 0, 0, 0, 0}, // PaddwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PaddwXmmE128
    {0, 0, 0, 0, 0, 0}, // PadddXmmXmm
    {0, 128, 0, 0, 0, 0}, // PadddXmmE128
    {0, 0, 0, 0, 0, 0}, // PaddqMmxMmx
    {0, 64, 0, 0, 0, 0}, // PaddqMmxE64
    {0, 0, 0, 0, 0, 0}, // PaddqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PaddqXmmE128
    {0, 0, 0, 0, 0, 0}, // PaddsbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PaddsbXmmE128
    {0, 0, 0, 0, 0, 0}, // PaddswXmmXmm
    {0, 128, 0, 0, 0, 0}, // PaddswXmmE128
    {0, 0, 0, 0, 0, 0}, // PaddusbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PaddusbXmmE128
    {0, 0, 0, 0, 0, 0}, // PadduswXmmXmm
    {0, 128, 0, 0, 0, 0}, // PadduswXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubbXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubwXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubdXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubqMmxMmx
    {0, 64, 0, 0, 0, 0}, // PsubqMmxE64
    {0, 0, 0, 0, 0, 0}, // PsubqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubqXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubsbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubsbXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubswXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubswXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubusbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubusbXmmE128
    {0, 0, 0, 0, 0, 0}, // PsubuswXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsubuswXmmE128
    {0, 0, 0, 0, 0, 0}, // PmaddwdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmaddwdXmmE128
    {0, 0, 0, 0, 0, 0}, // PmulhwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmulhwXmmE128
    {0, 0, 0, 0, 0, 0}, // PmullwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmullwXmmE128
    {0, 0, 0, 0, 0, 0}, // PmuludqMmxMmx
    {0, 64, 0, 0, 0, 0}, // PmuludqMmxE64
    {0, 0, 0, 0, 0, 0}, // PmuludqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PmuludqXmmE128
    {0, 0, 0, 0, 0, 0}, // SqrtpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // SqrtpdXmmE128
    {0, 0, 0, 0, 0, 0}, // SqrtsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // SqrtsdXmmE64
    {0, 0, 0, 0, 0, 0}, // AndnpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // AndnpdXmmE128
    {0, 0, 0, 0, 0, 0}, // AndpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // AndpdXmmE128
    {0, 0, 0, 0, 0, 0}, // PandXmmXmm
    {0, 128, 0, 0, 0, 0}, // PandXmmE128
    {0, 0, 0, 0, 0, 0}, // PandnXmmXmm
    {0, 128, 0, 0, 0, 0}, // PandnXmmE128
    {0, 0, 0, 0, 0, 0}, // PorXmmXmm
    {0, 128, 0, 0, 0, 0}, // PorXmmXmmE128
    {0, 0, 0, 0, 0, 0}, // PslldqXmm
    {0, 0, 0, 0, 0, 0}, // PsllqXmm
    {0, 0, 0, 0, 0, 0}, // PsllqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsllqXmmE128
    {0, 0, 0, 0, 0, 0}, // PslldXmm
    {0, 0, 0, 0, 0, 0}, // PslldXmmXmm
    {0, 128, 0, 0, 0, 0}, // PslldXmmE128
    {0, 0, 0, 0, 0, 0}, // PsllwXmm
    {0, 0, 0, 0, 0, 0}, // PsllwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsllwXmmE128
    {0, 0, 0, 0, 0, 0}, // PsradXmm
    {0, 0, 0, 0, 0, 0}, // PsradXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsradXmmE128
    {0, 0, 0, 0, 0, 0}, // PsrawXmm
    {0, 0, 0, 0, 0, 0}, // PsrawXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsrawXmmE128
    {0, 0, 0, 0, 0, 0}, // PsrldqXmm
    {0, 0, 0, 0, 0, 0}, // PsrlqXmm
    {0, 0, 0, 0, 0, 0}, // PsrlqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsrlqXmmE128
    {0, 0, 0, 0, 0, 0}, // PsrldXmm
    {0, 0, 0, 0, 0, 0}, // PsrldXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsrldXmmE128
    {0, 0, 0, 0, 0, 0}, // PsrlwXmm
    {0, 0, 0, 0, 0, 0}, // PsrlwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PsrlwXmmE128
    {0, 0, 0, 0, 0, 0}, // PxorXmmXmm
    {0, 128, 0, 0, 0, 0}, // PxorXmmE128
    {0, 0, 0, 0, 0, 0}, // OrpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // OrpdXmmE128
    {0, 0, 0, 0, 0, 0}, // XorpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // XorpdXmmE128
    {0, 0, 0, 0, 0, 0}, // CmppdXmmXmm
    {0, 128, 0, 0, 0, 0}, // CmppdXmmE128
    {0, 0, 0, 0, 0, 0}, // CmpsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // CmpsdXmmE64
    {0, 0, 0, 0, 0, 0}, // ComisdXmmXmm
    {0, 64, 0, 0, 0, 0}, // ComisdXmmE64
    {0, 0, 0, 0, 0, 0}, // UcomisdXmmXmm
    {0, 64, 0, 0, 0, 0}, // UcomisdXmmE64
    {0, 0, 0, 0, 0, 0}, // PcmpgtbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PcmpgtbXmmE128
    {0, 0, 0, 0, 0, 0}, // PcmpgtwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PcmpgtwXmmE128
    {0, 0, 0, 0, 0, 0}, // PcmpgtdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PcmpgtdXmmE128
    {0, 0, 0, 0, 0, 0}, // PcmpeqbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PcmpeqbXmmE128
    {0, 0, 0, 0, 0, 0}, // PcmpeqwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PcmpeqwXmmE128
    {0, 0, 0, 0, 0, 0}, // PcmpeqdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PcmpeqdXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtdq2pdXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvtdq2pdXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtdq2psXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvtdq2psXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtpd2piMmxXmm
    {0, 128, 0, 0, 0, 0}, // Cvtpd2piMmxE128
    {0, 0, 0, 0, 0, 0}, // Cvtpd2dqXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvtpd2dqXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtpd2psXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvtpd2psXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtpi2pdXmmMmx
    {0, 64, 0, 0, 0, 0}, // Cvtpi2pdXmmE64
    {0, 0, 0, 0, 0, 0}, // Cvtps2dqXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvtps2dqXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvtps2pdXmmXmm
    {0, 64, 0, 0, 0, 0}, // Cvtps2pdXmmE64
    {0, 0, 0, 0, 0, 0}, // Cvtsd2siR32Xmm
    {0, 64, 0, 0, 0, 0}, // Cvtsd2siR32E64
    {0, 0, 0, 0, 0, 0}, // Cvtsd2ssXmmXmm
    {0, 64, 0, 0, 0, 0}, // Cvtsd2ssXmmE64
    {0, 0, 0, 0, 0, 0}, // Cvtsi2sdXmmR32
    {0, 32, 0, 0, 0, 0}, // Cvtsi2sdXmmE32
    {0, 0, 0, 0, 0, 0}, // Cvtss2sdXmmXmm
    {0, 32, 0, 0, 0, 0}, // Cvtss2sdXmmE32
    {0, 0, 0, 0, 0, 0}, // Cvttpd2piMmxXmm
    {0, 128, 0, 0, 0, 0}, // Cvttpd2piMmE128
    {0, 0, 0, 0, 0, 0}, // Cvttpd2dqXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvttpd2dqXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvttps2dqXmmXmm
    {0, 128, 0, 0, 0, 0}, // Cvttps2dqXmmE128
    {0, 0, 0, 0, 0, 0}, // Cvttsd2siR32Xmm
    {0, 64, 0, 0, 0, 0}, // Cvttsd2siR32E64
    {0, 0, 0, 0, 0, 0}, // MovqXmmXmm
    {0, 0, 64, 0, 0, 0}, // MovqE64Xmm
    {0, 64, 0, 0, 0, 0}, // MovqXmmE64
    {0, 0, 0, 0, 0, 0}, // MovsdXmmXmm
    {0, 64, 0, 0, 0, 0}, // MovsdXmmE64
    {0, 0, 64, 0, 0, 0}, // MovsdE64Xmm
    {0, 0, 0, 0, 0, 0}, // MovapdXmmXmm
    {0, 128, 0, 0, 0, 0}, // MovapdXmmE128
    {0, 0, 128, 0, 0, 0}, // MovapdE128Xmm
    {0, 0, 0, 0, 0, 0}, // MovupdXmmXmm
    {0, 128, 0, 0, 0, 0}, // MovupdXmmE128
    {0, 0, 128, 0, 0, 0}, // MovupdE128Xmm
    {0, 64, 0, 0, 0, 0}, // MovhpdXmmE64
    {0, 0, 64, 0, 0, 0}, // MovhpdE64Xmm
    {0, 64, 0, 0, 0, 0}, // MovlpdXmmE64
    {0, 0, 64, 0, 0, 0}, // MovlpdE64Xmm
    {0, 0, 0, 0, 0, 0}, // MovmskpdR32Xmm
    {0, 0, 0, 0, 0, 0}, // MovdXmmR32
    {0, 32, 0, 0, 0, 0}, // MovdXmmE32
    {0, 0, 0, 0, 0, 0}, // MovdR32Xmm
    {0, 0, 32, 0, 0, 0}, // MovdE32Xmm
    {0, 0, 0, 0, 0, 0}, // MovdqaXmmXmm
    {0, 128, 0, 0, 0, 0}, // MovdqaXmmE128
    {0, 0, 128, 0, 0, 0}, // MovdqaE128Xmm
    {0, 0, 0, 0, 0, 0}, // MovdquXmmXmm
    {0, 128, 0, 0, 0, 0}, // MovdquXmmE128
    {0, 0, 128, 0, 0, 0}, // MovdquE128Xmm    
    {0, 0, 0, 0, 0, 0}, // Movdq2qMmxXmm
    {0, 0, 0, 0, 0, 0}, // Movq2dqXmmMmx
    {0, 0, 128, 0, 0, 0}, // MovntpdE128Xmm
    {0, 0, 128, 0, 0, 0}, // MovntdqE128Xmm
    {0, 0, 32, 0, 0, 0}, // MovntiE32R32
    {0, 0, 128, 0, 0, 0}, // MaskmovdquE128XmmXmm
    {0, 0, 0, 0, 0, 0}, // PshufdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PshufdXmmE128
    {0, 0, 0, 0, 0, 0}, // PshufhwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PshufhwXmmE128
    {0, 0, 0, 0, 0, 0}, // PshuflwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PshuflwXmmE128
    {0, 0, 0, 0, 0, 0}, // UnpckhpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // UnpckhpdXmmE128
    {0, 0, 0, 0, 0, 0}, // UnpcklpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // UnpcklpdXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpckhbwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpckhbwXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpckhwdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpckhwdXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpckhdqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpckhdqXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpckhqdqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpckhqdqXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpcklbwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpcklbwXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpcklwdXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpcklwdXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpckldqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpckldqXmmE128
    {0, 0, 0, 0, 0, 0}, // PunpcklqdqXmmXmm
    {0, 128, 0, 0, 0, 0}, // PunpcklqdqXmmE128
    {0, 0, 0, 0, 0, 0}, // PackssdwXmmXmm
    {0, 128, 0, 0, 0, 0}, // PackssdwXmmE128
    {0, 0, 0, 0, 0, 0}, // PacksswbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PacksswbXmmE128
    {0, 0, 0, 0, 0, 0}, // PackuswbXmmXmm
    {0, 128, 0, 0, 0, 0}, // PackuswbXmmE128
    {0, 0, 0, 0, 0, 0}, // ShufpdXmmXmm
    {0, 128, 0, 0, 0, 0}, // ShufpdXmmE128
    {0, 0, 0, 0, 0, 0}, // Pause

    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0}, // Callback
    {DECODE_BRANCH_NO_CACHE, 0, 0, 0, 0, 0}, // Done
    {0, 0, 0, 0, 0, 0} // Custom1
};

// for now, kept out of emscript to keep size down
#ifndef __EMSCRIPTEN__
struct LogInstruction;

typedef void (*LogFormat)(const LogInstruction* inst, DecodedOp* op, CPU* cpu);

struct LogInstruction {
    const char* name;
    S32 width;
    LogFormat pfnFormat;
    bool imm;
    const char* postfix;
};

static void outXMM(U32 r, CPU* cpu) {
    switch (r) {
    case 0: cpu->logFile.write("xmm0"); break;
    case 1: cpu->logFile.write("xmm1"); break;
    case 2: cpu->logFile.write("xmm2"); break;
    case 3: cpu->logFile.write("xmm3"); break;
    case 4: cpu->logFile.write("xmm4"); break;
    case 5: cpu->logFile.write("xmm5"); break;
    case 6: cpu->logFile.write("xmm6"); break;
    case 7: cpu->logFile.write("xmm7"); break;
    }
}

static void outR32(U32 r, CPU* cpu) {
    switch (r) {
        case 0: cpu->logFile.write("EAX"); break;
        case 1: cpu->logFile.write("ECX"); break;
        case 2: cpu->logFile.write("EDX"); break;
        case 3: cpu->logFile.write("EBX"); break;
        case 4: cpu->logFile.write("ESP"); break;
        case 5: cpu->logFile.write("EBP"); break;
        case 6: cpu->logFile.write("ESI"); break;
        case 7: cpu->logFile.write("EDI"); break;
    }
}

static void outR16(U32 r, CPU* cpu) {
    switch (r) {
        case 0: cpu->logFile.write("AX"); break;
        case 1: cpu->logFile.write("CX"); break;
        case 2: cpu->logFile.write("DX"); break;
        case 3: cpu->logFile.write("BX"); break;
        case 4: cpu->logFile.write("SP"); break;
        case 5: cpu->logFile.write("BP"); break;
        case 6: cpu->logFile.write("SI"); break;
        case 7: cpu->logFile.write("DI"); break;
    }
}

static void outR8(U32 r, CPU* cpu) {
    switch (r) {
        case 0: cpu->logFile.write("AL"); break;
        case 1: cpu->logFile.write("CL"); break;
        case 2: cpu->logFile.write("DL"); break;
        case 3: cpu->logFile.write("BL"); break;
        case 4: cpu->logFile.write("AH"); break;
        case 5: cpu->logFile.write("CH"); break;
        case 6: cpu->logFile.write("DH"); break;
        case 7: cpu->logFile.write("BH"); break;
    }
}

static void outS(U32 s, CPU* cpu) {
    switch (s) {
        case 0: cpu->logFile.write("ES"); break;
        case 1: cpu->logFile.write("CS"); break;
        case 2: cpu->logFile.write("SS"); break;
        case 3: cpu->logFile.write("DS"); break;
        case 4: cpu->logFile.write("FS"); break;
        case 5: cpu->logFile.write("GS"); break;
    }
}

static void outEA32(DecodedOp* op, CPU* cpu) {
    bool added = false;

    if (op->base!=SEG_ZERO) {
        outS(op->base, cpu);
        cpu->logFile.write(":");
    }
    if (op->rm!=8) {
        outR32(op->rm, cpu);
        added = true;
    }
    if (op->sibIndex!=8) {
        if (added) {
            cpu->logFile.write("+");
        }
        outR32(op->sibIndex, cpu);
        added = true;
        if (op->sibScale!=0) {
            cpu->logFile.write("<<");
            cpu->logFile.write((op->sibScale==1)?"1":"2");
        }
    }
    if (op->disp) {
        if (added) {
            cpu->logFile.write("+");
        }
        cpu->logFile.writeFormat("%X",op->disp);
    }
}

static void outEA16(DecodedOp* op, CPU* cpu) {
    bool added = false;

    if (op->base!=SEG_ZERO) {
        outS(op->base, cpu);
        cpu->logFile.write(":");
    }
    if (op->rm!=8) {
        outR16(op->rm, cpu);
        added = true;
    }
    if (op->sibIndex!=8) {
        if (added) {
            cpu->logFile.write("+");
        }
        outR16(op->sibIndex, cpu);
        added = true;
    }
    if (op->disp) {
        if (added) {
            cpu->logFile.write("+");
        }
        cpu->logFile.write("%X", op->disp);
    }
}

static void outE64(DecodedOp* op, CPU* cpu) {
    cpu->logFile.write("QWORD PTR [");
    if (op->ea16)
        outEA16(op, cpu);
    else
        outEA32(op, cpu);
    cpu->logFile.write("]");
}

static void outE32(DecodedOp* op, CPU* cpu) {
    cpu->logFile.write("DWORD PTR [");
    if (op->ea16)
        outEA16(op, cpu);
    else
        outEA32(op, cpu);
    cpu->logFile.write("]");
}

static void outE16(DecodedOp* op, CPU* cpu) {
    cpu->logFile.write("WORD PTR [");
    if (op->ea16)
        outEA16(op, cpu);
    else
        outEA32(op, cpu);
    cpu->logFile.write("]");
}

static void outE8(DecodedOp* op, CPU* cpu) {
    cpu->logFile.write("BYTE PTR [");
    if (op->ea16)
        outEA16(op, cpu);
    else
        outEA32(op, cpu);
    cpu->logFile.write("]");
}

static void logRR(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.writeFormat("%s ", inst->name);
    if (inst->width==32) {
        outR32(op->reg, cpu);
        cpu->logFile.write(",");
        outR32(op->rm, cpu);
    } else if (inst->width==16) {
        outR16(op->reg, cpu);
        cpu->logFile.write(",");
        outR16(op->rm, cpu);
    } else if (inst->width==8) {
        outR8(op->reg, cpu);
        cpu->logFile.write(",");
        outR8(op->rm, cpu);
    } else {
        kpanic("unknown width: %d in logRR", inst->width);
    }
}

static void logRE(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outR32(op->reg, cpu);
        cpu->logFile.write(",");
        outE32(op, cpu);
    } else if (inst->width==16) {
        outR16(op->reg, cpu);
        cpu->logFile.write(",");
        outE16(op, cpu);
    } else if (inst->width==8) {
        outR8(op->reg, cpu);
        cpu->logFile.write(",");
        outE8(op, cpu);
    } else {
        kpanic("unknown width: %d in logRE", inst->width);
    }
}

static void logER(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outE32(op, cpu);
        cpu->logFile.write(",");
        outR32(op->reg, cpu);        
    } else if (inst->width==16) {        
        outE16(op, cpu);
        cpu->logFile.write(",");
        outR16(op->reg, cpu);        
    } else if (inst->width==8) {        
        outE8(op, cpu);
        cpu->logFile.write(",");
        outR8(op->reg, cpu);
    } else {
        kpanic("unknown width: %d in logER", inst->width);
    }
}

static void logRE8(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outR32(op->reg, cpu);
        cpu->logFile.write(",");
        outE8(op, cpu);
    } else if (inst->width==16) {
        outR16(op->reg, cpu);
        cpu->logFile.write(",");
        outE8(op, cpu);
    } else if (inst->width==8) {
        outR8(op->reg, cpu);
        cpu->logFile.write(",");
        outE8(op, cpu);
    } else {
        kpanic("unknown width: %d in logRE8", inst->width);
    }
}

static void logRR8(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outR32(op->reg, cpu);
        cpu->logFile.write(",");
        outR8(op->rm, cpu);        
    } else if (inst->width==16) {        
        outR16(op->reg, cpu);
        cpu->logFile.write(",");
        outR8(op->rm, cpu);        
    } else if (inst->width==8) {        
        outR8(op->reg, cpu);
        cpu->logFile.write(",");
        outR8(op->rm, cpu);
    } else {
        kpanic("unknown width: %d in logRR8", inst->width);
    }
}

static void logRE16(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outR32(op->reg, cpu);
        cpu->logFile.write(",");
        outE16(op, cpu);
    } else if (inst->width==16) {
        outR16(op->reg, cpu);
        cpu->logFile.write(",");
        outE16(op, cpu);
    } else if (inst->width==8) {
        outR8(op->reg, cpu);
        cpu->logFile.write(",");
        outE16(op, cpu);
    } else {
        kpanic("unknown width: %d in logRE16", inst->width);
    }
}

static void logRR16(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outR32(op->reg, cpu);
        cpu->logFile.write(",");
        outR16(op->rm, cpu);        
    } else if (inst->width==16) {        
        outR16(op->reg, cpu);
        cpu->logFile.write(",");
        outR16(op->rm, cpu);        
    } else if (inst->width==8) {        
        outR8(op->reg, cpu);
        cpu->logFile.write(",");
        outR16(op->rm, cpu);
    } else {
        kpanic("unknown width: %d in logRR16", inst->width);
    }
}

static void logR(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outR32(op->reg, cpu);
    } else if (inst->width==16) {
        outR16(op->reg, cpu);
    } else if (inst->width==8) {
        outR8(op->reg, cpu);
    } else {
        kpanic("unknown width: %d in logR", inst->width);
    }
}

static void logE(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (inst->width==32) {
        outE32(op, cpu);
    } else if (inst->width==16) {
        outE16(op, cpu);
    } else if (inst->width==8) {
        outE8(op, cpu);
    } else if (inst->width==64) {
        outE64(op, cpu);
    } else {
        kpanic("unknown width: %d in logE", inst->width);
    }
}

static void logS(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logSR(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logRS(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logSE(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logES(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logName(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
}

static void logCsEip(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logMM(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logME(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logMR(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logM(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logRM(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logEM(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logXmmXmm(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    outXMM(op->reg, cpu);
    cpu->logFile.write(",");
    outXMM(op->rm, cpu);
}

static void logXmmI(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logXmmE(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");

    outXMM(op->reg, cpu);        
    cpu->logFile.write(",");
    if (op->ea16) {
        outE16(op, cpu);
    } else {
        outE32(op, cpu);
    }
}

static void logRXmm(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logEXmm(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
    cpu->logFile.write(" ");
    if (op->ea16) {
        outE16(op, cpu);
    } else {
        outE32(op, cpu);
    }
    cpu->logFile.write(",");
    outXMM(op->reg, cpu);
}

static void logXmmR(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logXmmM(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

static void logMXmm(const LogInstruction* inst, DecodedOp* op, CPU* cpu) {
    cpu->logFile.write(inst->name);
}

const LogInstruction instructionLog[] = {
    {"Add", 8, logRE},
    {"Add", 8, logER},
    {"Add", 8, logRR},
    {"Add", 8, logR, true},
    {"Add", 8, logE, true},
    {"Add", 16, logRE},
    {"Add", 16, logER},
    {"Add", 16, logRR},
    {"Add", 16, logR, true},
    {"Add", 16, logE, true},
    {"Add", 32, logRE},
    {"Add", 32, logER},
    {"Add", 32, logRR},
    {"Add", 32, logR, true},
    {"Add", 32, logE, true},

    {"Or", 8, logRE},
    {"Or", 8, logER},
    {"Or", 8, logRR},
    {"Or", 8, logR, true},
    {"Or", 8, logE, true},
    {"Or", 16, logRE},
    {"Or", 16, logER},
    {"Or", 16, logRR},
    {"Or", 16, logR, true},
    {"Or", 16, logE, true},
    {"Or", 32, logRE},
    {"Or", 32, logER},
    {"Or", 32, logRR},
    {"Or", 32, logR, true},
    {"Or", 32, logE, true},

    {"Adc", 8, logRE},
    {"Adc", 8, logER},
    {"Adc", 8, logRR},
    {"Adc", 8, logR, true},
    {"Adc", 8, logE, true},
    {"Adc", 16, logRE},
    {"Adc", 16, logER},
    {"Adc", 16, logRR},
    {"Adc", 16, logR, true},
    {"Adc", 16, logE, true},
    {"Adc", 32, logRE},
    {"Adc", 32, logER},
    {"Adc", 32, logRR},
    {"Adc", 32, logR, true},
    {"Adc", 32, logE, true},

    {"Sbb", 8, logRE},
    {"Sbb", 8, logER},
    {"Sbb", 8, logRR},
    {"Sbb", 8, logR, true},
    {"Sbb", 8, logE, true},
    {"Sbb", 16, logRE},
    {"Sbb", 16, logER},
    {"Sbb", 16, logRR},
    {"Sbb", 16, logR, true},
    {"Sbb", 16, logE, true},
    {"Sbb", 32, logRE},
    {"Sbb", 32, logER},
    {"Sbb", 32, logRR},
    {"Sbb", 32, logR, true},
    {"Sbb", 32, logE, true},

    {"And", 8, logRE},
    {"And", 8, logER},
    {"And", 8, logRR},
    {"And", 8, logR, true},
    {"And", 8, logE, true},
    {"And", 16, logRE},
    {"And", 16, logER},
    {"And", 16, logRR},
    {"And", 16, logR, true},
    {"And", 16, logE, true},
    {"And", 32, logRE},
    {"And", 32, logER},
    {"And", 32, logRR},
    {"And", 32, logR, true},
    {"And", 32, logE, true},

    {"Sub", 8, logRE},
    {"Sub", 8, logER},
    {"Sub", 8, logRR},
    {"Sub", 8, logR, true},
    {"Sub", 8, logE, true},
    {"Sub", 16, logRE},
    {"Sub", 16, logER},
    {"Sub", 16, logRR},
    {"Sub", 16, logR, true},
    {"Sub", 16, logE, true},
    {"Sub", 32, logRE},
    {"Sub", 32, logER},
    {"Sub", 32, logRR},
    {"Sub", 32, logR, true},
    {"Sub", 32, logE, true},

    {"Xor", 8, logRE},
    {"Xor", 8, logER},
    {"Xor", 8, logRR},
    {"Xor", 8, logR, true},
    {"Xor", 8, logE, true},
    {"Xor", 16, logRE},
    {"Xor", 16, logER},
    {"Xor", 16, logRR},
    {"Xor", 16, logR, true},
    {"Xor", 16, logE, true},
    {"Xor", 32, logRE},
    {"Xor", 32, logER},
    {"Xor", 32, logRR},
    {"Xor", 32, logR, true},
    {"Xor", 32, logE, true},

    {"Cmp", 8, logRE},
    {"Cmp", 8, logER},
    {"Cmp", 8, logRR},
    {"Cmp", 8, logR, true},
    {"Cmp", 8, logE, true},
    {"Cmp", 16, logRE},
    {"Cmp", 16, logER},
    {"Cmp", 16, logRR},
    {"Cmp", 16, logR, true},
    {"Cmp", 16, logE, true},
    {"Cmp", 32, logRE},
    {"Cmp", 32, logER},
    {"Cmp", 32, logRR},
    {"Cmp", 32, logR, true},
    {"Cmp", 32, logE, true},

    {"Test", 8, logER},
    {"Test", 8, logRR},
    {"Test", 8, logR, true},
    {"Test", 8, logE, true},
    {"Test", 16, logER},
    {"Test", 16, logRR},
    {"Test", 16, logR, true},
    {"Test", 16, logE, true},
    {"Test", 32, logER},
    {"Test", 32, logRR},
    {"Test", 32, logR, true},
    {"Test", 32, logE, true},

    {"Not", 8, logR},
    {"Not", 8, logE},
    {"Not", 16, logR},
    {"Not", 16, logE},
    {"Not", 32, logR},
    {"Not", 32, logE},

    {"Neg", 8, logR},
    {"Neg", 8, logE},
    {"Neg", 16, logR},
    {"Neg", 16, logE},
    {"Neg", 32, logR},
    {"Neg", 32, logE},

    {"Mul", 8, logR, false, ",AL"},
    {"Mul", 8, logE, false, ",AL"},
    {"Mul", 16, logR, false, ",AX"},
    {"Mul", 16, logE, false, ",AX"},
    {"Mul", 32, logR, false, ",EAX"},
    {"Mul", 32, logE, false, ",EAX"},

    {"IMul", 8, logR, false, ",AL"},
    {"IMul", 8, logE, false, ",AL"},
    {"IMul", 16, logR, false, ",AX"},
    {"IMul", 16, logE, false, ",AX"},
    {"IMul", 32, logR, false, ",EAX"},
    {"IMul", 32, logE, false, ",EAX"},

    {"Div", 8, logR, false, ",AL"},
    {"Div", 8, logE, false, ",AL"},
    {"Div", 16, logR, false, ",AX"},
    {"Div", 16, logE, false, ",AX"},
    {"Div", 32, logR, false, ",EAX"},
    {"Div", 32, logE, false, ",EAX"},

    {"IDiv", 8, logR, false, ",AL"},
    {"IDiv", 8, logE, false, ",AL"},
    {"IDiv", 16, logR, false, ",AX"},
    {"IDiv", 16, logE, false, ",AX"},
    {"IDiv", 32, logR, false, ",EAX"},
    {"IDiv", 32, logE, false, ",EAX"},

    {"Xchg", 8, logRR},
    {"Xchg", 8, logER},
    {"Xchg", 16, logRR},
    {"Xchg", 16, logER},
    {"Xchg", 32, logRR},
    {"Xchg", 32, logER},

    {"Bt", 16, logRR},
    {"Bt", 16, logER},
    {"Bt", 32, logRR},
    {"Bt", 32, logER},

    {"Bts", 16, logRR},
    {"Bts", 16, logER},
    {"Bts", 32, logRR},
    {"Bts", 32, logER},

    {"Btr", 16, logRR},
    {"Btr", 16, logER},
    {"Btr", 32, logRR},
    {"Btr", 32, logER},

    {"Bsf", 16, logRR},
    {"Bsf", 16, logER},
    {"Bsf", 32, logRR},
    {"Bsf", 32, logER},

    {"Bsr", 16, logRR},
    {"Bsr", 16, logER},
    {"Bsr", 32, logRR},
    {"Bsr", 32, logER},

    {"Btc", 16, logRR},
    {"Btc", 16, logER},
    {"Btc", 32, logRR},
    {"Btc", 32, logER},

    {"Bt", 16, logR, true}, 
    {"Bt", 16, logE, true},
    {"Bts", 16, logR, true}, 
    {"Bts", 16, logE, true},
    {"Btr", 16, logR, true}, 
    {"Btr", 16, logE, true},
    {"Btc", 16, logR, true}, 
    {"Btc", 16, logE, true},

    {"Bt", 32, logR, true}, 
    {"Bt", 32, logE, true},
    {"Bts", 32, logR, true}, 
    {"Bts", 32, logE, true},
    {"Btr", 32, logR, true}, 
    {"Btr", 32, logE, true},
    {"Btc", 32, logR, true}, 
    {"Btc", 32, logE, true},

    {"Dshl", 16, logRR, true},
    {"Dshl", 16, logER, true},
    {"Dshl", 16, logRR, false, ",CL"},
    {"Dshl", 16, logER, false, ",CL"},
    {"Dshr", 16, logRR, true},
    {"Dshr", 16, logER, true},
    {"Dshr", 16, logRR, false, ",CL"},
    {"Dshr", 16, logER, false, ",CL"},

    {"Dshl", 32, logRR, true},
    {"Dshl", 32, logER, true},
    {"Dshl", 32, logRR, false, ",CL"},
    {"Dshl", 32, logER, false, ",CL"},
    {"Dshr", 32, logRR, true},
    {"Dshr", 32, logER, true},
    {"Dshr", 32, logRR, false, ",CL"},
    {"Dshr", 32, logER, false, ",CL"},

    {"Mul", 16, logRR},
    {"Mul", 16, logRE},
    {"Mul", 32, logRR},
    {"Mul", 32, logRE},

    {"CmpXchg", 8, logRR},
    {"CmpXchg", 8, logER},
    {"CmpXchg", 16, logRR},
    {"CmpXchg", 16, logER},
    {"CmpXchg", 32, logRR},
    {"CmpXchg", 32, logER},

    {"Inc", 8, logR},
    {"Inc", 16, logR},
    {"Inc", 32, logR},
    {"Inc", 8, logE},
    {"Inc", 16, logE},
    {"Inc", 32, logE},

    {"Dec", 8, logR},
    {"Dec", 16, logR},
    {"Dec", 32, logR},
    {"Dec", 8, logE},
    {"Dec", 16, logE},
    {"Dec", 32, logE},

    {"Push", 16, logS},
    {"Pop", 16, logS},
    {"Push", 32, logS},
    {"Pop", 32, logS},

    {"Push", 16, logR},
    {"Push", 32, logR},
    {"Push", 16, logE},
    {"Push", 32, logE},

    {"Pop", 16, logR},
    {"Pop", 32, logR},
    {"Pop", 16, logE},
    {"Pop", 32, logE},

    {"PushA", 16, logName},
    {"PushA", 32, logName},
    {"PopA", 16, logName},
    {"PopA", 32, logName},

    {"Push", 16, logName, true},
    {"Push", 32, logName, true},

    {"PushF", 16, logName},
    {"PushF", 32, logName},
    {"PopF", 16, logName},
    {"PopF", 32, logName},

    {"Bound", 16, logRE},
    {"Bound", 32, logRE},

    {"Arpl", 16, logR},
    {"Arpl", 16, logE},
    {"Arpl", 32, logR},
    {"Arpl", 32, logE},

    {"Daa", 0, logName},
    {"Das", 0, logName},
    {"Aaa", 0, logName},
    {"Aas", 0, logName},
    {"Aam", 0, logName},
    {"Aad", 0, logName},

    {"Imul", 16, logRE},
    {"Imul", 16, logRR},
    {"Imul", 32, logRE},
    {"Imul", 32, logRR},

    {"Insb", 8, logName},
    {"Insw", 16, logName},
    {"Insd", 32, logName},

    {"Outsb", 8, logName},
    {"Outsw", 16, logName},
    {"Outsd", 32, logName},

    {"JO", -32, logName, true},
    {"JNO", -32, logName, true},
    {"JB", -32, logName, true},
    {"JNB", -32, logName, true},
    {"JZ", -32, logName, true},
    {"JNZ", -32, logName, true},
    {"JBE", -32, logName, true},
    {"JNBE", -32, logName, true},
    {"JS", -32, logName, true},
    {"JNS", -32, logName, true},
    {"JP", -32, logName, true},
    {"JNP", -32, logName, true},
    {"JL", -32, logName, true},
    {"JNL", -32, logName, true},
    {"JLE", -32, logName, true},
    {"JNLE", -32, logName, true},

    {"Mov", 8, logRR},
    {"Mov", 8, logER},
    {"Mov", 8, logRE},
    {"Mov", 8, logR, true},
    {"Mov", 8, logE, true},
    {"Mov", 16, logRR},
    {"Mov", 16, logER},
    {"Mov", 16, logRE},
    {"Mov", 16, logR, true},
    {"Mov", 16, logE, true},
    {"Mov", 32, logRR},
    {"Mov", 32, logER},
    {"Mov", 32, logRE},
    {"Mov", 32, logR, true},
    {"Mov", 32, logE, true},

    {"Mov", 16, logRS},
    {"Mov", 32, logRS},
    {"Mov", 16, logES},
    {"Mov", 16, logSR},
    {"Mov", 32, logSE},

    {"Mov Al,", 8, logE},
    {"Mov Ax,", 16, logE},
    {"Mov Eax,", 32, logE},
    {"Mov", 8, logE, false, ",Al"},
    {"Mov", 16, logE, false, ",Ax"},
    {"Mov", 32, logE, false, ",Eax"},

    {"Movzx", 16, logRR8},
    {"Movzx", 16, logRE8},
    {"Movsx", 16, logRR8},
    {"Movsx", 16, logRE8},

    {"Movzx", 32, logRR8},
    {"Movzx", 32, logRE8},
    {"Movsx", 32, logRR8},
    {"Movsx", 32, logRE8},

    {"Movzx", 32, logRR16},
    {"Movzx", 32, logRE16},
    {"Movsx", 32, logRR16},
    {"Movsx", 32, logRE16},

    {"MovRdCRx", 32, logName},
    {"MovCRxRd", 32, logName},

    {"Lea", 16, logRE},
    {"Lea", 32, logRE},

    {"Nop", 0, logName},
    {"Cwd", 0, logName},
    {"Cwq", 0, logName},
    {"Call", 16, logCsEip},
    {"Call", 32, logCsEip},
    {"Jmp", 32, logCsEip},
    {"Jmp", 32, logCsEip},
    {"Wait", 0, logName},
    {"Sahf", 0, logName},
    {"Lahf", 0, logName},
    {"Salc", 0, logName}, // Salc
    {"Ret", 16, logName, true},
    {"Ret", 32, logName, true},
    {"Ret", 16, logName},
    {"Ret", 32, logName},
    {"Retf", 16, logName},
    {"Retf", 32, logName},    
    {"Invalid", 0, logName},
    {"Int3", 0, logName},
    {"Int80 Syscall", 0, logName},
    {"Int98 Wine", 0, logName},
    {"Int99 OpenGL", 0, logName},
    {"Int9A Vulkan", 0, logName },
    {"Int", 0, logName, true},
    {"Int0", 0, logName},
    {"IRet", 16, logName},
    {"IRet", 32, logName},
    {"Xlat", 0, logName},
    {"ICEBP", 0, logName},
    {"Hlt", 0, logName},
    {"Cmc", 0, logName},
    {"Clc", 0, logName},
    {"Stc", 0, logName},
    {"Cli", 0, logName},
    {"Sti", 0, logName},
    {"Cld", 0, logName},
    {"Std", 0, logName},
    {"Rdtsc", 0, logName},
    {"CPUID", 0, logName},

    {"Enter", 16, logName},
    {"Enter", 32, logName},
    {"Leave", 16, logName},
    {"Leave", 32, logName},
    
    {"Ls", 16, logName}, // :TODO: should be les, lds, etc
    {"Ls", 16, logName},

    {"Movsb", 8, logName},
    {"Movsw", 16, logName},
    {"Movsd", 32, logName},
    {"Cmpsb", 8, logName},
    {"Cmpsw", 16, logName},
    {"Cmpsd", 32, logName},
    {"Stosb", 8, logName},
    {"Stosw", 16, logName},
    {"Stosd", 32, logName},
    {"Lodsb", 8, logName},
    {"Lodsw", 16, logName},
    {"Lodsd", 32, logName},
    {"Scasb", 8, logName},
    {"Scasw", 16, logName},
    {"Scasd", 32, logName},

    {"Rol", 8, logR, true},
    {"Rol", 8, logE, true},
    {"Ror", 8, logR, true},
    {"Ror", 8, logE, true},
    {"Rcl", 8, logR, true},
    {"Rcl", 8, logE, true},
    {"Rcr", 8, logR, true},
    {"Rcr", 8, logE, true},
    {"Shl", 8, logR, true},
    {"Shl", 8, logE, true},
    {"Shr", 8, logR, true},
    {"Shr", 8, logE, true},
    {"Sar", 8, logR, true},
    {"Sar", 8, logE, true},

    {"Rol", 16, logR, true},
    {"Rol", 16, logE, true},
    {"Ror", 16, logR, true},
    {"Ror", 16, logE, true},
    {"Rcl", 16, logR, true},
    {"Rcl", 16, logE, true},
    {"Rcr", 16, logR, true},
    {"Rcr", 16, logE, true},
    {"Shl", 16, logR, true},
    {"Shl", 16, logE, true},
    {"Shr", 16, logR, true},
    {"Shr", 16, logE, true},
    {"Sar", 16, logR, true},
    {"Sar", 16, logE, true},

    {"Rol", 32, logR, true},
    {"Rol", 32, logE, true},
    {"Ror", 32, logR, true},
    {"Ror", 32, logE, true},
    {"Rcl", 32, logR, true},
    {"Rcl", 32, logE, true},
    {"Rcr", 32, logR, true},
    {"Rcr", 32, logE, true},
    {"Shl", 32, logR, true},
    {"Shl", 32, logE, true},
    {"Shr", 32, logR, true},
    {"Shr", 32, logE, true},
    {"Sar", 32, logR, true},
    {"Sar", 32, logE, true},

    {"Rol", 8, logR, false, ",Cl"},
    {"Rol", 8, logE, false, ",Cl"},
    {"Ror", 8, logR, false, ",Cl"},
    {"Ror", 8, logE, false, ",Cl"},
    {"Rcl", 8, logR, false, ",Cl"},
    {"Rcl", 8, logE, false, ",Cl"},
    {"Rcr", 8, logR, false, ",Cl"},
    {"Rcr", 8, logE, false, ",Cl"},
    {"Shl", 8, logR, false, ",Cl"},
    {"Shl", 8, logE, false, ",Cl"},
    {"Shr", 8, logR, false, ",Cl"},
    {"Shr", 8, logE, false, ",Cl"},
    {"Sar", 8, logR, false, ",Cl"},
    {"Sar", 8, logE, false, ",Cl"},

    {"Rol", 16, logR, false, ",Cl"},
    {"Rol", 16, logE, false, ",Cl"},
    {"Ror", 16, logR, false, ",Cl"},
    {"Ror", 16, logE, false, ",Cl"},
    {"Rcl", 16, logR, false, ",Cl"},
    {"Rcl", 16, logE, false, ",Cl"},
    {"Rcr", 16, logR, false, ",Cl"},
    {"Rcr", 16, logE, false, ",Cl"},
    {"Shl", 16, logR, false, ",Cl"},
    {"Shl", 16, logE, false, ",Cl"},
    {"Shr", 16, logR, false, ",Cl"},
    {"Shr", 16, logE, false, ",Cl"},
    {"Sar", 16, logR, false, ",Cl"},
    {"Sar", 16, logE, false, ",Cl"},

    {"Rol", 32, logR, false, ",Cl"},
    {"Rol", 32, logE, false, ",Cl"},
    {"Ror", 32, logR, false, ",Cl"},
    {"Ror", 32, logE, false, ",Cl"},
    {"Rcl", 32, logR, false, ",Cl"},
    {"Rcl", 32, logE, false, ",Cl"},
    {"Rcr", 32, logR, false, ",Cl"},
    {"Rcr", 32, logE, false, ",Cl"},
    {"Shl", 32, logR, false, ",Cl"},
    {"Shl", 32, logE, false, ",Cl"},
    {"Shr", 32, logR, false, ",Cl"},
    {"Shr", 32, logE, false, ",Cl"},
    {"Sar", 32, logR, false, ",Cl"},
    {"Sar", 32, logE, false, ",Cl"},

    {"FADD_ST0_STj", 0, logName},
    {"FMUL_ST0_STj", 0, logName},
    {"FCOM_STi", 0, logName},
    {"FCOM_STi_Pop", 0, logName},
    {"FSUB_ST0_STj", 0, logName},
    {"FSUBR_ST0_STj", 0, logName},
    {"FDIV_ST0_STj", 0, logName},
    {"FDIVR_ST0_STj", 0, logName},
    {"FADD_SINGLE_REAL", 0, logName},
    {"FMUL_SINGLE_REAL", 0, logName},
    {"FCOM_SINGLE_REAL", 0, logName},
    {"FCOM_SINGLE_REAL_Pop", 0, logName},
    {"FSUB_SINGLE_REAL", 0, logName},
    {"FSUBR_SINGLE_REAL", 0, logName},
    {"FDIV_SINGLE_REAL", 0, logName},
    {"FDIVR_SINGLE_REAL", 0, logName},

    {"FLD_STi", 0, logName},
    {"FXCH_STi", 0, logName},
    {"FNOP", 0, logName},
    {"FST_STi_Pop", 0, logName},
    {"FCHS", 0, logName},
    {"FABS", 0, logName},
    {"FTST", 0, logName},
    {"FXAM", 0, logName},
    {"FLD1", 0, logName},
    {"FLDL2T", 0, logName},
    {"FLDL2E", 0, logName},
    {"FLDPI", 0, logName},
    {"FLDLG2", 0, logName},
    {"FLDLN2", 0, logName},
    {"FLDZ", 0, logName},
    {"F2XM1", 0, logName},
    {"FYL2X", 0, logName},
    {"FPTAN", 0, logName},
    {"FPATAN", 0, logName},
    {"FXTRACT", 0, logName},
    {"FPREM_nearest", 0, logName},
    {"FDECSTP", 0, logName},
    {"FINCSTP", 0, logName},
    {"FPREM", 0, logName},
    {"FYL2XP1", 0, logName},
    {"FSQRT", 0, logName},
    {"FSINCOS", 0, logName},
    {"FRNDINT", 0, logName},
    {"FSCALE", 0, logName},
    {"FSIN", 0, logName},
    {"FCOS", 0, logName},
    {"FLD_SINGLE_REAL", 0, logName},
    {"FST_SINGLE_REAL", 0, logName},
    {"FST_SINGLE_REAL_Pop", 0, logName},
    {"FLDENV", 0, logName},
    {"FLDCW", 0, logName},
    {"FNSTENV", 0, logName},
    {"FNSTCW", 0, logName},

    {"FCMOV_ST0_STj_CF", 0, logName},
    {"FCMOV_ST0_STj_ZF", 0, logName},
    {"FCMOV_ST0_STj_CF_OR_ZF", 0, logName},
    {"FCMOV_ST0_STj_PF", 0, logName},
    {"FUCOMPP", 0, logName},
    {"FIADD_DWORD_INTEGER", 0, logName},
    {"FIMUL_DWORD_INTEGER", 0, logName},
    {"FICOM_DWORD_INTEGER", 0, logName},
    {"FICOM_DWORD_INTEGER_Pop", 0, logName},
    {"FISUB_DWORD_INTEGER", 0, logName},
    {"FISUBR_DWORD_INTEGER", 0, logName},
    {"FIDIV_DWORD_INTEGER", 0, logName},
    {"FIDIVR_DWORD_INTEGER", 0, logName},

    {"FCMOV_ST0_STj_NCF", 0, logName},
    {"FCMOV_ST0_STj_NZF", 0, logName},
    {"FCMOV_ST0_STj_NCF_AND_NZF", 0, logName},
    {"FCMOV_ST0_STj_NPF", 0, logName},
    {"FNCLEX", 0, logName},
    {"FNINIT", 0, logName},
    {"FUCOMI_ST0_STj", 0, logName},
    {"FCOMI_ST0_STj", 0, logName},
    {"FILD_DWORD_INTEGER", 0, logName},
    {"FISTTP32", 0, logName},
    {"FIST_DWORD_INTEGER", 0, logName},
    {"FIST_DWORD_INTEGER_Pop", 0, logName},
    {"FLD_EXTENDED_REAL", 0, logName},
    {"FSTP_EXTENDED_REAL", 0, logName},

    {"FADD_STi_ST0", 0, logName},
    {"FMUL_STi_ST0", 0, logName},
    {"FSUBR_STi_ST0", 0, logName},
    {"FSUB_STi_ST0", 0, logName},
    {"FDIVR_STi_ST0", 0, logName},
    {"FDIV_STi_ST0", 0, logName},
    {"FADD_DOUBLE_REAL", 0, logName},
    {"FMUL_DOUBLE_REAL", 0, logName},
    {"FCOM_DOUBLE_REAL", 0, logName},
    {"FCOM_DOUBLE_REAL_Pop", 0, logName},
    {"FSUB_DOUBLE_REAL", 0, logName},
    {"FSUBR_DOUBLE_REAL", 0, logName},
    {"FDIV_DOUBLE_REAL", 0, logName},
    {"FDIVR_DOUBLE_REAL", 0, logName},

    {"FFREE_STi", 0, logName},
    {"FST_STi", 0, logName},
    {"FUCOM_STi", 0, logName},
    {"FUCOM_STi_Pop", 0, logName},
    {"FLD_DOUBLE_REAL", 0, logName},
    {"FISTTP64", 0, logName},
    {"FST_DOUBLE_REAL", 0, logName},
    {"FST_DOUBLE_REAL_Pop", 0, logName},
    {"FRSTOR", 0, logName},
    {"FNSAVE", 0, logName},
    {"FNSTSW", 0, logName},

    {"FADD_STi_ST0_Pop", 0, logName},
    {"FMUL_STi_ST0_Pop", 0, logName},
    {"FCOMPP", 0, logName},
    {"FSUBR_STi_ST0_Pop", 0, logName},
    {"FSUB_STi_ST0_Pop", 0, logName},
    {"FDIVR_STi_ST0_Pop", 0, logName},
    {"FDIV_STi_ST0_Pop", 0, logName},
    {"FIADD_WORD_INTEGER", 0, logName},
    {"FIMUL_WORD_INTEGER", 0, logName},
    {"FICOM_WORD_INTEGER", 0, logName},
    {"FICOM_WORD_INTEGER_Pop", 0, logName},
    {"FISUB_WORD_INTEGER", 0, logName},
    {"FISUBR_WORD_INTEGER", 0, logName},
    {"FIDIV_WORD_INTEGER", 0, logName},
    {"FIDIVR_WORD_INTEGER", 0, logName},

    {"FFREEP_STi", 0, logName},
    {"FNSTSW_AX", 0, logName},
    {"FUCOMI_ST0_STj_Pop", 0, logName},
    {"FCOMI_ST0_STj_Pop", 0, logName},
    {"FILD_WORD_INTEGER", 0, logName},
    {"FISTTP16", 0, logName},
    {"FIST_WORD_INTEGER", 0, logName},
    {"FIST_WORD_INTEGER_Pop", 0, logName},
    {"FBLD_PACKED_BCD", 0, logName},
    {"FILD_QWORD_INTEGER", 0, logName},
    {"FBSTP_PACKED_BCD", 0, logName},
    {"FISTP_QWORD_INTEGER", 0, logName},

    {"LoopNZ", 0, logName},
    {"LoopZ", 0, logName},
    {"Loop", 0, logName},
    {"Jcxz", 0, logName},

    {"In Al,", 8, logName, true},
    {"In Ax,", 16, logName, true},
    {"In Eax,", 32, logName, true},
    {"Out", 8, logName, true, ",Al"},
    {"Out", 16, logName, true, ",Ax"},
    {"Out", 32, logName, true, ",Eax"},
    {"In Al,Dx", 8, logName},
    {"In Ax,Dx", 16, logName},
    {"In Eax,Dx", 32, logName},
    {"Out Dx,Al", 8, logName},
    {"Out Dx,Ax", 16, logName},
    {"Out Dx,Eax", 32, logName},

    {"Call", -16, logName, true},
    {"Call", -32, logName, true},
    {"Jmp", -16, logName, true},
    {"Jmp", -32, logName, true},
    {"Jmp", -8, logName, true},
    {"Call", 16, logR},
    {"Call", 32, logR},
    {"Call", 16, logE},
    {"Call", 32, logE},
    {"Callf", 16, logE},
    {"Callf", 32, logE},
    {"Jmp", 16, logR},
    {"Jmp", 32, logR},
    {"Jmp", 16, logE},
    {"Jmp", 32, logE},
    {"Jmpf", 16, logE},
    {"Jmpf", 32, logE},

    {"Lar", 16, logRR},
    {"Lar", 16, logRE},
    {"Lsl", 16, logRR},
    {"Lsl", 16, logRE},
    {"Lsl", 32, logRR},
    {"Lsl", 32, logRE},

    {"CmovO", 16, logRR},
    {"CmovO", 16, logRE},
    {"CmovNO", 16, logRR},
    {"CmovNO", 16, logRE},    
    {"CmovB", 16, logRR},
    {"CmovB", 16, logRE},
    {"CmovNB", 16, logRR},
    {"CmovNB", 16, logRE},
    {"CmovZ", 16, logRR},
    {"CmovZ", 16, logRE},
    {"CmovNZ", 16, logRR},
    {"CmovNZ", 16, logRE},
    {"CmovBE", 16, logRR},
    {"CmovBE", 16, logRE},
    {"CmovNBE", 16, logRR},
    {"CmovNBE", 16, logRE},
    {"CmovS", 16, logRR},
    {"CmovS", 16, logRE},
    {"CmovNS", 16, logRR},
    {"CmovNS", 16, logRE},
    {"CmovP", 16, logRR},
    {"CmovP", 16, logRE},
    {"CmovNP", 16, logRR},
    {"CmovNP", 16, logRE},
    {"CmovL", 16, logRR},
    {"CmovL", 16, logRE},
    {"CmovNL", 16, logRR},
    {"CmovNL", 16, logRE},
    {"CmovLE", 16, logRR},
    {"CmovLE", 16, logRE},
    {"CmovNLE", 16, logRR},
    {"CmovNLE", 16, logRE},

    {"CmovO", 32, logRR},
    {"CmovO", 32, logRE},
    {"CmovNO", 32, logRR},
    {"CmovNO", 32, logRE},    
    {"CmovB", 32, logRR},
    {"CmovB", 32, logRE},
    {"CmovNB", 32, logRR},
    {"CmovNB", 32, logRE},
    {"CmovZ", 32, logRR},
    {"CmovZ", 32, logRE},
    {"CmovNZ", 32, logRR},
    {"CmovNZ", 32, logRE},
    {"CmovBE", 32, logRR},
    {"CmovBE", 32, logRE},
    {"CmovNBE", 32, logRR},
    {"CmovNBE", 32, logRE},
    {"CmovS", 32, logRR},
    {"CmovS", 32, logRE},
    {"CmovNS", 32, logRR},
    {"CmovNS", 32, logRE},
    {"CmovP", 32, logRR},
    {"CmovP", 32, logRE},
    {"CmovNP", 32, logRR},
    {"CmovNP", 32, logRE},
    {"CmovL", 32, logRR},
    {"CmovL", 32, logRE},
    {"CmovNL", 32, logRR},
    {"CmovNL", 32, logRE},
    {"CmovLE", 32, logRR},
    {"CmovLE", 32, logRE},
    {"CmovNLE", 32, logRR},
    {"CmovNLE", 32, logRE},

    {"SetO", 8, logR},
    {"SetO", 8, logE},
    {"SetNO", 8, logR},
    {"SetNO", 8, logE},    
    {"SetB", 8, logR},
    {"SetB", 8, logE},
    {"SetNB", 8, logR},
    {"SetNB", 8, logE},
    {"SetZ", 8, logR},
    {"SetZ", 8, logE},
    {"SetNZ", 8, logR},
    {"SetNZ", 8, logE},
    {"SetBE", 8, logR},
    {"SetBE", 8, logE},
    {"SetNBE", 8, logR},
    {"SetNBE", 8, logE},
    {"SetS", 8, logR},
    {"SetS", 8, logE},
    {"SetNS", 8, logR},
    {"SetNS", 8, logE},
    {"SetP", 8, logR},
    {"SetP", 8, logE},
    {"SetNP", 8, logR},
    {"SetNP", 8, logE},
    {"SetL", 8, logR},
    {"SetL", 8, logE},
    {"SetNL", 8, logR},
    {"SetNL", 8, logE},
    {"SetLE", 8, logR},
    {"SetLE", 8, logE},
    {"SetNLE", 8, logR},
    {"SetNLE", 8, logE},

    {"SLDT", 16, logR}, 
    {"SLDT", 16, logE},
    {"STR", 16, logR}, 
    {"STR", 16, logE},
    {"LLDT", 16, logR}, 
    {"LLDT", 16, logE},
    {"LTR", 16, logR}, 
    {"LTR", 16, logE},
    {"VERR", 16, logR}, 
    {"VERR", 16, logE},
    {"VERW", 16, logR}, 
    {"VERW", 16, logE},

    {"SGDT", 0, logName},
    {"SIDT", 0, logName},
    {"LGDT", 0, logName},
    {"LIDT", 0, logName},
    {"SMSW", 16, logR},
    {"SMSW", 16, logE},
    {"LMSW", 16, logR},
    {"LMSW", 16, logE},
    {"INVLPG", 0, logName},

    { "Xadd", 8, logRR },
    { "Xadd", 8, logRE },
    { "Xadd", 16, logRR },
    { "Xadd", 16, logRE },
    {"Xadd", 32, logRR},
    {"Xadd", 32, logRE},
    {"CmpXchg8b", 64, logE},
    {"Bswap32", 32, logR},

    {"Punpcklbw", 0, logMM},
    {"Punpcklbw", 0, logME},
    {"Punpcklwd", 0, logMM},
    {"Punpcklwd", 0, logME},
    {"Punpckldq", 0, logMM},
    {"Punpckldq", 0, logME},
    {"Packsswb", 0, logMM},
    {"Packsswb", 0, logME},
    {"Pcmpgtb", 0, logMM},
    {"Pcmpgtb", 0, logME},
    {"Pcmpgtw", 0, logMM},
    {"Pcmpgtw", 0, logME},
    {"Pcmpgtd", 0, logMM},
    {"Pcmpgtd", 0, logME},
    {"Packuswb", 0, logMM},
    {"Packuswb", 0, logME},
    {"Punpckhbw", 0, logMM},
    {"Punpckhbw", 0, logME},
    {"Punpckhwd", 0, logMM},
    {"Punpckhwd", 0, logME},
    {"Punpckhdq", 0, logMM},
    {"Punpckhdq", 0, logME},
    {"Packssdw", 0, logMM},
    {"Packssdw", 0, logME},
    {"MovPq", 32, logMR}, 
    {"MovPq", 0, logME},
    {"MovPq", 0, logMM}, 
    {"MovPq", 0, logME},

    {"Psrlw", 0, logM, true},
    {"Psraw", 0, logM, true},
    {"Psllw", 0, logM, true},
    {"Psrld", 0, logM, true},
    {"Psrad", 0, logM, true},
    {"Pslld", 0, logM, true},
    {"Psrlq", 0, logM, true},
    {"Psllq", 0, logM, true},
    {"Pcmpeqb", 0, logMM},
    {"Pcmpeqb", 0, logME},
    {"Pcmpeqw", 0, logMM},
    {"Pcmpeqw", 0, logME},
    {"Pcmpeqd", 0, logMM},
    {"Pcmpeqd", 0, logME},
    {"Emms", 0, logName},
    {"Mov", 32, logRM},
    {"Mov", 32, logEM},
    {"Mov", 64, logMM},
    {"Mov", 64, logEM },
    {"Psrlw", 0, logMM},
    {"Psrlw", 0, logME},
    {"Psrld", 0, logMM},
    {"Psrld", 0, logME},
    {"Psrlq", 0, logMM},
    {"Psrlq", 0, logME},
    {"Pmullw", 0, logMM},
    {"Pmullw", 0, logME},
    {"Psubusb", 0, logMM},
    {"Psubusb", 0, logME},
    {"Psubusw", 0, logMM},
    {"Psubusw", 0, logME},
    {"Pand", 0, logMM},
    {"Pand", 0, logME},
    {"Paddusb", 0, logMM},
    {"Paddusb", 0, logME},
    {"Paddusw", 0, logMM},
    {"Paddusw", 0, logME},
    {"Pandn", 0, logMM},
    {"Pandn", 0, logME},
    {"Psraw", 0, logMM},
    {"Psraw", 0, logME},
    {"Psrad", 0, logMM},
    {"Psrad", 0, logME},
    {"Pmulhw", 0, logMM},
    {"Pmulhw", 0, logME},
    {"Psubsb", 0, logMM},
    {"Psubsb", 0, logME},
    {"Psubsw", 0, logMM},
    {"Psubsw", 0, logME},
    {"Por", 0, logMM},
    {"Por", 0, logME},
    {"Paddsb", 0, logMM},
    {"Paddsb", 0, logME},
    {"Paddsw", 0, logMM},
    {"Paddsw", 0, logME},
    {"Pxor", 0, logMM},
    {"Pxor", 0, logME},
    {"Psllw", 0, logMM},
    {"Psllw", 0, logME},
    {"Pslld", 0, logMM},
    {"Pslld", 0, logME},
    {"Psllq", 0, logMM},
    {"Psllq", 0, logME},
    {"Pmaddwd", 0, logMM},
    {"Pmaddwd", 0, logME},
    {"Psubb", 0, logMM},
    {"Psubb", 0, logME},
    {"Psubw", 0, logMM},
    {"Psubw", 0, logME},
    {"Psubd", 0, logMM},
    {"Psubd", 0, logME},
    {"Paddb", 0, logMM},
    {"Paddb", 0, logME},
    {"Paddw", 0, logMM},
    {"Paddw", 0, logME},
    {"Paddd", 0, logMM},
    {"Paddd", 0, logME},

    {"FXSAVE", 0, logName},
    {"FXRSTOR", 0, logName},
    {"LDMXCSR", 0, logName},
    {"STMXCSR", 0, logName},
    {"XSAVE", 0, logName},
    {"Lfence", 0, logName},
    {"XRSTOR", 0, logName},
    {"MFENCE", 0, logName},
    {"SFENCE", 0, logName},
    {"CLFLUSH", 0, logName},

    // SSE1
    {"Addps", 0, logXmmXmm},
    {"Addps", 128, logXmmE},
    {"Addss", 0, logXmmXmm},
    {"Addss", 32, logXmmE},
    {"Subps", 0, logXmmXmm},
    {"Subps", 128, logXmmE},
    {"Subss", 0, logXmmXmm},
    {"Subss", 32, logXmmE},
    {"Mulps", 0, logXmmXmm},
    {"Mulps", 128, logXmmE},
    {"Mulss", 0, logXmmXmm},
    {"Mulss", 32, logXmmE},
    {"Divps", 0, logXmmXmm},
    {"Divps", 128, logXmmE},
    {"Divss", 0, logXmmXmm},
    {"Divss", 32, logXmmE},
    {"Rcpps", 0, logXmmXmm},
    {"Rcpps", 128, logXmmE},
    {"Rcpss", 0, logXmmXmm},
    {"Rcpss", 32, logXmmE},
    {"Sqrtps", 0, logXmmXmm},
    {"Sqrtps", 128, logXmmE},
    {"Sqrtss", 0, logXmmXmm},
    {"Sqrtss", 32, logXmmE},
    {"Rsqrtps", 0, logXmmXmm},
    {"Rsqrtps", 128, logXmmE},
    {"Rsqrtss", 0, logXmmXmm},
    {"Rsqrtss", 32, logXmmE},
    {"Maxps", 0, logXmmXmm},
    {"Maxps", 128, logXmmE},
    {"Maxss", 0, logXmmXmm},
    {"Maxss", 32, logXmmE},
    {"Minps", 0, logXmmXmm},
    {"Minps", 128, logXmmE},
    {"Minss", 0, logXmmXmm},
    {"Minss", 32, logXmmE},
    {"Pavgb", 0, logXmmXmm},
    {"Pavgb", 64, logXmmE},
    {"Pavgb", 0, logXmmXmm},
    {"Pavgb", 128, logXmmE},
    {"Pavgw", 0, logMM},
    {"Pavgw", 64, logXmmE},
    {"Pavgw", 0, logXmmXmm},
    {"Pavgw", 128, logXmmE},
    {"Psadbw", 0, logMM},
    {"Psadbw", 64, logXmmE},
    {"Psadbw", 0, logXmmXmm},
    {"Psadbw", 128, logXmmE},
    {"Pextrw", 0, logRM},
    {"Pextrw", 16, logEM},
    {"Pextrw", 0, logRXmm},
    {"Pextrw", 16, logEXmm},
    {"Pinsrw", 0, logMR},
    {"Pinsrw", 16, logME},
    {"Pinsrw", 0, logXmmR},
    {"Pinsrw", 16, logXmmE},
    {"Pmaxsw", 0, logMM},
    {"Pmaxsw", 64, logXmmE},
    {"Pmaxsw", 0, logXmmXmm},
    {"Pmaxsw", 128, logXmmE},
    {"Pmaxub", 0, logMM},
    {"Pmaxub", 64, logXmmE},
    {"Pmaxub", 0, logXmmXmm},
    {"Pmaxub", 128, logXmmE},
    {"Pminsw", 0, logMM},
    {"Pminsw", 64, logXmmE},
    {"Pminsw", 0, logXmmXmm},
    {"Pminsw", 128, logXmmE},
    {"Pminub", 0, logMM},
    {"Pminub", 64, logXmmE},
    {"Pminub", 0, logXmmXmm},
    {"Pminub", 128, logXmmE},
    {"Pmovmskb", 0, logRM},
    {"Pmovmskb", 0, logRXmm},
    {"Pmulhuw", 0, logMM},
    {"Pmulhuw", 64, logXmmE},
    {"Pmulhuw", 0, logXmmXmm},
    {"Pmulhuw", 128, logXmmE},
    {"Pshufw", 0, logMM},
    {"Pshufw", 64, logXmmE},
    {"Andnps", 0, logXmmXmm},
    {"Andnps", 128, logXmmE},
    {"Andps", 0, logXmmXmm},
    {"Andps", 128, logXmmE},
    {"Orps", 0, logXmmXmm},
    {"Orps", 128, logXmmE},
    {"Xorps", 0, logXmmXmm},
    {"Xorps", 128, logXmmE},
    {"Cvtpi2ps", 0, logXmmM},
    {"Cvtpi2ps", 64, logXmmE},
    {"Cvtps2pi", 0, logMXmm},
    {"Cvtps2pi", 64, logXmmE},
    {"Cvtsi2ss", 0, logXmmR},
    {"Cvtsi2ss", 32, logXmmE},
    {"Cvtss2si", 0, logRXmm},
    {"Cvtss2si", 32, logRE},
    {"Cvttps2pi", 0, logMXmm},
    {"Cvttps2pi", 64, logXmmE},
    {"Cvttss2si", 0, logRXmm},
    {"Cvttss2si", 32, logRE},
    {"Movaps", 0, logXmmXmm},
    {"Movaps", 128, logXmmE},
    {"Movaps", 128, logEXmm},
    {"Movhlps", 0, logXmmXmm},
    {"Movlhps", 0, logXmmXmm},
    {"Movhps", 64, logXmmE},
    {"Movhps", 64, logEXmm},
    {"Movlps", 64, logXmmE},
    {"Movlps", 64, logEXmm},
    {"Movmskps", 32, logRXmm},
    {"Movss", 0, logXmmXmm},
    {"Movss", 32, logXmmE},
    {"Movss", 32, logEXmm},
    {"Movups", 0, logXmmXmm},
    {"Movups", 128, logXmmE},
    {"Movups", 128, logEXmm},
    {"MaskmovqEDI", 0, logMM},
    {"Movntps", 128, logEXmm},
    {"Movntq", 64, logEM},
    {"Shufps", 0, logXmmXmm},
    {"Shufps", 128, logXmmE},
    {"Unpckhps", 0, logXmmXmm},
    {"Unpckhps", 64, logXmmE},
    {"Unpcklps", 0, logXmmXmm},
    {"Unpcklps", 64, logXmmE},
    {"PrefetchT0", 0, logName},
    {"PrefetchT1", 0, logName},
    {"PrefetchT2", 0, logName},
    {"PrefetchNTA", 0, logName},
    {"Cmpps", 0, logXmmXmm},
    {"Cmpps", 128, logXmmE},
    {"Cmpss", 0, logXmmXmm},
    {"Cmpss", 32, logXmmE},
    {"Comiss", 0, logXmmXmm},
    {"Comiss", 32, logXmmE},
    {"Ucomiss", 0, logXmmXmm},
    {"Ucomiss", 32, logXmmE},


    {"Addpd", 0, logXmmXmm},
    {"Addpd", 128, logXmmE},
    {"Addsd", 0, logXmmXmm},
    {"Addsd", 64, logXmmE},
    {"Subpd", 0, logXmmXmm},
    {"Subpd", 128, logXmmE},
    {"Subsd", 0, logXmmXmm},
    {"Subsd", 64, logXmmE},
    {"Mulpd", 0, logXmmXmm},
    {"Mulpd", 128, logXmmE},    
    {"Mulsd", 0, logXmmXmm},
    {"Mulsd", 64, logXmmE},
    {"Divpd", 0, logXmmXmm},
    {"Divpd", 128, logXmmE},
    {"Divsd", 0, logXmmXmm},
    {"Divsd", 64, logXmmE},
    {"Maxpd", 0, logXmmXmm},
    {"Maxpd", 128, logXmmE},
    {"Maxsd", 0, logXmmXmm},
    {"Maxsd", 64, logXmmE},
    {"Minpd", 0, logXmmXmm},
    {"Minpd", 128, logXmmE},
    {"Minsd", 0, logXmmXmm},
    {"Minsd", 64, logXmmE},
    {"Paddb", 0, logXmmXmm},
    {"Paddb", 128, logXmmE},
    {"Paddw", 0, logXmmXmm},
    {"Paddw", 128, logXmmE},
    {"Paddd", 0, logXmmXmm},
    {"Paddd", 128, logXmmE},
    {"Paddq", 0, logMM},
    {"Paddq", 64, logME},
    {"Paddq", 0, logXmmXmm},
    {"Paddq", 128, logXmmE},
    {"Paddsb", 0, logXmmXmm},
    {"Paddsb", 128, logXmmE},
    {"Paddsw", 0, logXmmXmm},
    {"Paddsw", 128, logXmmE},
    {"Paddusb", 0, logXmmXmm},
    {"Paddusb", 128, logXmmE},
    {"Paddusw", 0, logXmmXmm},
    {"Paddusw", 128, logXmmE},
    {"Psubb", 0, logXmmXmm},
    {"Psubb", 128, logXmmE},
    {"Psubw", 0, logXmmXmm},
    {"Psubw", 128, logXmmE},
    {"Psubd", 0, logXmmXmm},
    {"Psubd", 128, logXmmE},
    {"Psubq", 0, logMM},
    {"Psubq", 64, logME},
    {"Psubq", 0, logXmmXmm},
    {"Psubq", 128, logXmmE},
    {"Psubsb", 0, logXmmXmm},
    {"Psubsb", 128, logXmmE},
    {"Psubsw", 0, logXmmXmm},
    {"Psubsw", 128, logXmmE},
    {"Psubusb", 0, logXmmXmm},
    {"Psubusb", 128, logXmmE},
    {"Psubusw", 0, logXmmXmm},
    {"Psubusw", 128, logXmmE},
    {"Pmaddwd", 0, logXmmXmm},
    {"Pmaddwd", 128, logXmmE},
    {"Pmulhw", 0, logXmmXmm},
    {"Pmulhw", 128, logXmmE},
    {"Pmullw", 0, logXmmXmm},
    {"Pmullw", 128, logXmmE},
    {"Pmuludq", 0, logMM},
    {"Pmuludq", 64, logME},
    {"Pmuludq", 0, logXmmXmm},
    {"Pmuludq", 128, logXmmE},
    {"Sqrtpd", 0, logXmmXmm},
    {"Sqrtpd", 128, logXmmE},
    {"Sqrtsd", 0, logXmmXmm},
    {"Sqrtsd", 64, logXmmE},
    {"Andnpd", 0, logXmmXmm},
    {"Andnpd", 128, logXmmE},
    {"Andpd", 0, logXmmXmm},
    {"Andpd", 128, logXmmE},
    {"Pand", 0, logXmmXmm},
    {"Pand", 128, logXmmE},
    {"Pandn", 0, logXmmXmm},
    {"Pandn", 128, logXmmE},
    {"Por", 0, logXmmXmm},
    {"PorXmm", 128, logXmmE},
    {"Pslldq", 0, logXmmI},
    {"Psllq", 0, logXmmI},
    {"Psllq", 0, logXmmXmm},
    {"Psllq", 128, logXmmE},
    {"Pslld", 0, logXmmI},
    {"Pslld", 0, logXmmXmm},
    {"Pslld", 128, logXmmE},
    {"Psllw", 0, logXmmI},
    {"Psllw", 0, logXmmXmm},
    {"Psllw", 128, logXmmE},
    {"Psrad", 0, logXmmI},
    {"Psrad", 0, logXmmXmm},
    {"Psrad", 128, logXmmE},
    {"Psraw", 0, logXmmI},
    {"Psraw", 0, logXmmXmm},
    {"Psraw", 128, logXmmE},
    {"Psrldq", 0, logXmmI},
    {"Psrlq", 0, logXmmI},
    {"Psrlq", 0, logXmmXmm},
    {"Psrlq", 128, logXmmE},
    {"Psrld", 0, logXmmI},
    {"Psrld", 0, logXmmXmm},
    {"Psrld", 128, logXmmE},
    {"Psrlw", 0, logXmmI},
    {"Psrlw", 0, logXmmXmm},
    {"Psrlw", 128, logXmmE},
    {"Pxor", 0, logXmmXmm},
    {"Pxor", 128, logXmmE},
    {"Orpd", 0, logXmmXmm},
    {"Orpd", 128, logXmmE},
    {"Xorpd", 0, logXmmXmm},
    {"Xorpd", 128, logXmmE},
    {"Cmppd", 0, logXmmXmm},
    {"Cmppd", 128, logXmmE},
    {"Cmpsd", 0, logXmmXmm},
    {"Cmpsd", 64, logXmmE},
    {"Comisd", 0, logXmmXmm},
    {"Comisd", 64, logXmmE},
    {"Ucomisd", 0, logXmmXmm},
    {"Ucomisd", 64, logXmmE},
    {"Pcmpgtb", 0, logXmmXmm},
    {"Pcmpgtb", 128, logXmmE},
    {"Pcmpgtw", 0, logXmmXmm},
    {"Pcmpgtw", 128, logXmmE},
    {"Pcmpgtd", 0, logXmmXmm},
    {"Pcmpgtd", 128, logXmmE},
    {"Pcmpeqb", 0, logXmmXmm},
    {"Pcmpeqb", 128, logXmmE},
    {"Pcmpeqw", 0, logXmmXmm},
    {"Pcmpeqw", 128, logXmmE},
    {"Pcmpeqd", 0, logXmmXmm},
    {"Pcmpeqd", 128, logXmmE},
    {"Cvtdq2pd", 0, logXmmXmm},
    {"Cvtdq2pd", 128, logXmmE},
    {"Cvtdq2ps", 0, logXmmXmm},
    {"Cvtdq2ps", 128, logXmmE},
    {"Cvtpd2pi", 0, logMXmm},
    {"Cvtpd2pi", 128, logME},
    {"Cvtpd2dq", 0, logXmmXmm},
    {"Cvtpd2dq", 128, logXmmE},
    {"Cvtpd2ps", 0, logXmmXmm},
    {"Cvtpd2ps", 128, logXmmE},
    {"Cvtpi2pd", 0, logXmmM},
    {"Cvtpi2pd", 64, logXmmE},
    {"Cvtps2dq", 0, logXmmXmm},
    {"Cvtps2dq", 128, logXmmE},
    {"Cvtps2pd", 0, logXmmXmm},
    {"Cvtps2pd", 128, logXmmE},
    {"Cvtsd2si", 0, logRXmm},
    {"Cvtsd2si", 64, logRE},
    {"Cvtsd2ss", 0, logXmmXmm},
    {"Cvtsd2ss", 64, logXmmE},
    {"Cvtsi2sd", 0, logXmmR},
    {"Cvtsi2sd", 32, logXmmE},
    {"Cvtss2sd", 0, logXmmXmm},
    {"Cvtss2sd", 32, logXmmE},
    {"Cvttpd2pi", 0, logMXmm},
    {"Cvttpd2pi", 128, logME},
    {"Cvttpd2dq", 0, logXmmXmm},
    {"Cvttpd2dq", 128, logXmmE},
    {"Cvttps2dq", 0, logXmmXmm},
    {"Cvttps2dq", 128, logXmmE},
    {"Cvttsd2si", 0, logRXmm},
    {"Cvttsd2si", 32, logRE},
    {"Movq", 0, logXmmXmm},
    {"Movq", 64, logEXmm},
    {"Movq", 64, logXmmE},   
    {"Movsd", 0, logXmmXmm},
    {"Movsd", 64, logXmmE},
    {"Movsd", 64, logEXmm},
    {"Movapd", 0, logXmmXmm},
    {"Movapd", 128, logXmmE},
    {"Movapd", 128, logEXmm},
    {"Movupd", 0, logXmmXmm},
    {"Movupd", 128, logXmmE},
    {"Movupd", 128, logEXmm},
    {"Movhpd", 64, logXmmE},
    {"Movhpd", 64, logEXmm},
    {"Movlpd", 64, logXmmE},
    {"Movlpd", 64, logEXmm},
    {"Movmskpd", 0, logRXmm},
    {"Movd", 0, logXmmR},
    {"Movd", 32, logXmmE},
    {"Movd", 0, logRXmm},
    {"Movd", 32, logEXmm},
    {"Movdqa", 0, logXmmXmm},
    {"Movdqa", 128, logXmmE},
    {"Movdqa", 128, logEXmm},
    {"Movdqu", 0, logXmmXmm},
    {"Movdqu", 128, logXmmE},
    {"Movdqu", 128, logEXmm},     
    {"Movdq2q", 0, logMXmm},
    {"Movq2dq", 0, logXmmM},
    {"Movntpd", 128, logEXmm},
    {"Movntdq", 128, logEXmm},
    {"Movnti", 32, logER},
    {"MaskmovdquE128", 0, logXmmXmm},
    {"Pshufd", 0, logXmmXmm},
    {"Pshufd", 128, logXmmE},
    {"Pshufhw", 0, logXmmXmm},
    {"Pshufhw", 128, logXmmE},
    {"Pshuflw", 0, logXmmXmm},
    {"Pshuflw", 128, logXmmE},
    {"Unpckhpd", 0, logXmmXmm},
    {"Unpckhpd", 128, logXmmE},
    {"Unpcklpd", 0, logXmmXmm},
    {"Unpcklpd", 128, logXmmE},
    {"Punpckhbw", 0, logXmmXmm},
    {"Punpckhbw", 128, logXmmE},
    {"Punpckhwd", 0, logXmmXmm},
    {"Punpckhwd", 128, logXmmE},
    {"Punpckhdq", 0, logXmmXmm},
    {"Punpckhdq", 128, logXmmE},
    {"Punpckhqdq", 0, logXmmXmm},
    {"Punpckhqdq", 128, logXmmE},
    {"Punpcklbw", 0, logXmmXmm},
    {"Punpcklbw", 128, logXmmE},
    {"Punpcklwd", 0, logXmmXmm},
    {"Punpcklwd", 128, logXmmE},
    {"Punpckldq", 0, logXmmXmm},
    {"Punpckldq", 128, logXmmE},
    {"Punpcklqdq", 0, logXmmXmm},
    {"Punpcklqdq", 128, logXmmE},
    {"Packssdw", 0, logXmmXmm},
    {"Packssdw", 128, logXmmE},
    {"Packsswb", 0, logXmmXmm},
    {"Packsswb", 128, logXmmE},
    {"Packuswb", 0, logXmmXmm},
    {"Packuswb", 128, logXmmE},
    {"Shufpd", 0, logXmmXmm},
    {"Shufpd", 128, logXmmE},
    {"Pause", 0, logName}
};
#endif
class DecodeData {
public:  
    DecodeData() = default;

    U8 ds = 0;
    U8 ss = 0;
    bool ea16 = false;

    U32 opCode = 0;
    U32 inst = 0;

    U8 fetch8();
    U16 fetch16();
    U32 fetch32();

    pfnFetchByte fetchByte = nullptr;
    void* fetchByteData = nullptr;
    U32 eip = 0;
    U32 opCountSoFarInThisBlock = 0;
    U8 opLen = 0;
};

typedef void (*DECODER)(DecodeData* obj);

void decodeEa16(DecodeData* data, DecodedOp* op, U8 rm) {
    if (rm<0x40) {
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->rm=regBX; op->sibIndex=regSI; break;
        case 0x01: op->base=data->ds; op->rm=regBX; op->sibIndex=regDI; break;
        case 0x02: op->base=data->ss; op->rm=regBP; op->sibIndex=regSI; break;
        case 0x03: op->base=data->ss; op->rm=regBP; op->sibIndex=regDI; break;
        case 0x04: op->base=data->ds; op->rm=regSI; op->sibIndex=regZero; break;
        case 0x05: op->base=data->ds; op->rm=regDI; op->sibIndex=regZero; break;
        case 0x06: op->base=data->ds; op->rm=regZero;op->disp = (S16)data->fetch16(); op->sibIndex=regZero; op->sibIndex=regZero; break;
        case 0x07: op->base=data->ds; op->rm=regBX; op->sibIndex=regZero; break;
        }
    } else {
        if (rm<0x80) {
            op->disp = (S8)data->fetch8();
        } else {
            op->disp = (S16)data->fetch16();
        }
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->rm=regBX; op->sibIndex=regSI; break;
        case 0x01: op->base=data->ds; op->rm=regBX; op->sibIndex=regDI; break;
        case 0x02: op->base=data->ss; op->rm=regBP; op->sibIndex=regSI; break;
        case 0x03: op->base=data->ss; op->rm=regBP; op->sibIndex=regDI; break;
        case 0x04: op->base=data->ds; op->rm=regSI; op->sibIndex=regZero; break;
        case 0x05: op->base=data->ds; op->rm=regDI; op->sibIndex=regZero; break;
        case 0x06: op->base=data->ss; op->rm=regBP; op->sibIndex=regZero; break;
        case 0x07: op->base=data->ds; op->rm=regBX; op->sibIndex=regZero; break;
        }
    }
}

U32 SibE2Reg[8] = {regAX, regCX, regDX, regBX, regZero, regBP, regSI, regDI};

 void Sib0(DecodeData* data, DecodedOp* op) {
    U8 sib=data->fetch8();

    switch (sib&7) {
    case 0:
        op->base=data->ds; op->rm=regAX;break;
    case 1:
        op->base=data->ds; op->rm=regCX;break;
    case 2:
        op->base=data->ds; op->rm=regDX;break;
    case 3:
        op->base=data->ds; op->rm=regBX;break;
    case 4:
        op->base=data->ss; op->rm=regSP;break;
    case 5:
        op->base = data->ds; op->rm=regZero;op->disp = data->fetch32();break;
    case 6:
        op->base=data->ds; op->rm=regSI;break;
    case 7:
        op->base=data->ds; op->rm=regDI;break;
    }
    op->sibIndex = SibE2Reg[(sib >> 3) & 7];
    op->sibScale = sib >> 6;
} 

  void Sib1(DecodeData* data, DecodedOp* op) {
    U8 sib=data->fetch8();

    switch (sib&7) {
    case 0:
        op->base=data->ds; op->rm=regAX;break;
    case 1:
        op->base=data->ds; op->rm=regCX;break;
    case 2:
        op->base=data->ds; op->rm=regDX;break;
    case 3:
        op->base=data->ds; op->rm=regBX;break;
    case 4:
        op->base=data->ss; op->rm=regSP;break;
    case 5:
        op->base=data->ss; op->rm=regBP;break;
    case 6:
        op->base=data->ds; op->rm=regSI;break;
    case 7:
        op->base=data->ds; op->rm=regDI;break;
    }
    op->sibIndex = SibE2Reg[(sib >> 3) & 7];
    op->sibScale = sib >> 6;
} 

void decodeEa32(DecodeData* data, DecodedOp* op, U8 rm) {
    op->rm=regZero; 
    op->sibIndex=regZero; 
    op->sibScale = 0;
    if (rm<0x40) {
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->rm = regAX; break;
        case 0x01: op->base=data->ds; op->rm = regCX; break;
        case 0x02: op->base=data->ds; op->rm = regDX; break;
        case 0x03: op->base=data->ds; op->rm = regBX; break;
        case 0x04: Sib0(data, op); break;
        case 0x05: op->base=data->ds; op->rm = regZero; op->disp = data->fetch32(); break;
        case 0x06: op->base=data->ds; op->rm = regSI; break;
        case 0x07: op->base=data->ds; op->rm = regDI; break;
        }
    } else {		
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->rm = regAX; break;
        case 0x01: op->base=data->ds; op->rm = regCX; break;
        case 0x02: op->base=data->ds; op->rm = regDX; break;
        case 0x03: op->base=data->ds; op->rm = regBX; break;
        case 0x04: Sib1(data, op); break;
        case 0x05: op->base=data->ss; op->rm = regBP; break;
        case 0x06: op->base=data->ds; op->rm = regSI; break;
        case 0x07: op->base=data->ds; op->rm = regDI; break;
        }
        if (rm<0x80) {
            op->disp = (S8)data->fetch8();
        } else {
            op->disp = data->fetch32();
        }
    }
}

void decodeEa(DecodeData* data, DecodedOp* op, U8 rm) {
    if (data->ea16)
        decodeEa16(data, op, rm);
    else
        decodeEa32(data, op, rm);
}

class Decode {
public:
    virtual void decode(DecodeData* data, DecodedOp* op) const = 0;
    Decode() {
        this->immWidth = 0;
        this->signExtendWidth = 0;
    }
    Decode(U32 immWdith) {
        this->immWidth = immWdith;
        this->signExtendWidth = 0;
    }
    Decode(U32 immWdith, U32 signExtendWidth) {
        this->immWidth = immWdith;
        this->signExtendWidth = signExtendWidth;
    }
protected:
    void fetchImm(DecodeData* data, DecodedOp* op) const {
        if  (this->immWidth==8) {
            if (this->signExtendWidth==16)
                op->imm = (U16)((S8)data->fetch8());
            else if (this->signExtendWidth==32)
                op->imm = (U32)((S8)data->fetch8());
            else
                op->imm = data->fetch8();
        } else if  (immWidth==16)
            if (this->signExtendWidth==32)
                op->imm = (U32)((S16)data->fetch16());
            else
                op->imm = data->fetch16();
        else if  (immWidth==32)
            op->imm = data->fetch32();
    }
private:
    U32 immWidth;
    U32 signExtendWidth;
};

extern const Decode* const decoder[];

class DecodeRM : public Decode {
public:
    DecodeRM(Instruction reg, Instruction mem) : Decode() {
        this->reg = reg;
        this->mem = mem;
    }
    DecodeRM(Instruction reg, Instruction mem, U32 immWdith) : Decode(immWdith) {
        this->reg = reg;
        this->mem = mem;
    }
    DecodeRM(Instruction reg, Instruction mem, U32 immWdith, U32 signExtendWidth) : Decode(immWdith, signExtendWidth) {
        this->reg = reg;
        this->mem = mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            op->inst = this->reg;
            op->reg = E(rm);
            op->rm = G(rm);
        } else {
            op->inst = this->mem;
            op->reg = G(rm);
            decodeEa(data, op, rm);
        }        
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction mem;
};

class DecodeRMImm : public Decode {
public:
    DecodeRMImm(Instruction reg, Instruction mem, U32 imm) : Decode() {
        this->reg = reg;
        this->mem = mem;
        this->imm = imm;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        op->reg = G(rm);
        if (rm>=0xC0) {    
            op->inst = this->reg;
            op->rm = E(rm);
        } else {
            op->inst = this->mem;
            decodeEa(data, op, rm);
        }        
        op->imm = this->imm;
    }

private:
    U32 imm;
    Instruction reg;
    Instruction mem;
};

class DecodeMaskRM : public DecodeRM {
public:
    DecodeMaskRM(Instruction reg, Instruction mem, U32 immWdith, U32 mask) : DecodeRM(reg, mem, immWdith) {
        this->mask = mask;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        DecodeRM::decode(data, op);
        op->imm &= this->mask;
        if (op->imm==0)
            op->inst = Nop;
    }
private:
    U32 mask;
};

class DecodeE : public Decode {
public:
    DecodeE(Instruction reg, Instruction mem) : Decode() {
        this->reg = reg;
        this->mem = mem;
    }
    DecodeE(Instruction reg, Instruction mem, U32 immWdith) : Decode(immWdith) {
        this->reg = reg;
        this->mem = mem;
    }
    DecodeE(Instruction reg, Instruction mem, U32 immWdith, U32 signExtendWidth) : Decode(immWdith, signExtendWidth) {
        this->reg = reg;
        this->mem = mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            op->inst = this->reg;
            op->reg = E(rm);
        } else {
            op->inst = this->mem;
            decodeEa(data, op, rm);
        }        
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction mem;
};

class DecodeRMr : public Decode {
public:
    DecodeRMr(Instruction reg, Instruction mem) : Decode() {
        this->reg = reg;
        this->mem = mem;
    }
    DecodeRMr(Instruction reg, Instruction mem, U32 immWdith) : Decode(immWdith) {
        this->reg = reg;
        this->mem = mem;
    }
    DecodeRMr(Instruction reg, Instruction mem, U32 immWdith, U32 signExtendWidth) : Decode(immWdith, signExtendWidth) {
        this->reg = reg;
        this->mem = mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            op->inst = this->reg;
            op->reg = G(rm);
            op->rm = E(rm);
        } else {
            op->inst = this->mem;
            op->reg = G(rm);
            decodeEa(data, op, rm);
        }        
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction mem;
};

class DecodeMem : public Decode {
public:
    DecodeMem(Instruction mem) {
        this->mem = mem;
    }
    DecodeMem(Instruction mem, U32 immWidth) : Decode(immWidth) {
        this->mem = mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        op->reg = G(rm);
        op->inst = this->mem;
        decodeEa(data, op, rm);
        fetchImm(data, op);
    }

private:
    Instruction mem;
};

class DecodeLea : public Decode {
public:
    DecodeLea(Instruction mem) {
        this->mem = mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        op->reg = G(rm);
        op->inst = this->mem;
        decodeEa(data, op, rm);
        op->base = SEG_ZERO;
        fetchImm(data, op);
    }

private:
    Instruction mem;
};

class DecodeInstrMem : public Decode {
public:
    DecodeInstrMem(Instruction mem) {
        this->mem = mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->base = data->ds;
        op->inst = this->mem;
    }

private:
    Instruction mem;
};

class DecodeReg : public Decode {
public:
    DecodeReg(Instruction inst, U32 reg) : Decode() {
        this->reg = reg;
        this->inst = inst;
    }
    DecodeReg(Instruction inst, U32 reg, U32 immWidth) : Decode(immWidth) {
        this->reg = reg;
        this->inst = inst;
    }
    DecodeReg(Instruction inst, U32 reg, U32 immWidth, U32 signExtendWidth) : Decode(immWidth, signExtendWidth) {
        this->reg = reg;
        this->inst = inst;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = this->inst;
        op->reg = this->reg;
        fetchImm(data, op);
    }

private:
    U32 reg;
    Instruction inst;
};

class DecodeReg2 : public Decode {
public:
    DecodeReg2(Instruction inst, U32 reg, U32 reg2) : Decode() {
        this->reg = reg;
        this->reg2 = reg2;
        this->inst = inst;
    }
    DecodeReg2(Instruction inst, U32 reg, U32 reg2, U32 immWidth) : Decode(immWidth) {
        this->reg = reg;
        this->reg2 = reg2;
        this->inst = inst;
    }
    DecodeReg2(Instruction inst, U32 reg, U32 reg2, U32 immWidth, U32 signExtendWidth) : Decode(immWidth, signExtendWidth) {
        this->reg = reg;
        this->reg2 = reg2;
        this->inst = inst;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = this->inst;
        op->reg = this->reg;
        op->rm = this->reg2;
        fetchImm(data, op);
    }

private:
    U32 reg;
    U32 reg2;
    Instruction inst;
};

class DecodeInst : public Decode {
public:
    DecodeInst(Instruction reg) {
        this->reg = reg;
    }
    DecodeInst(Instruction reg, U32 immWidth) : Decode(immWidth) {
        this->reg = reg;
    }
    DecodeInst(Instruction reg, U32 immWidth, U32 signExtendWidth) : Decode(immWidth, signExtendWidth) {
        this->reg = reg;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = reg;
        fetchImm(data, op);
    }

private:
    Instruction reg;
};

class DecodeZeroInst : public Decode {
public:
    DecodeZeroInst(Instruction reg, Instruction f2Reg, Instruction f3Reg) {
        this->reg = reg;
        this->f2Reg = f2Reg;
        this->f3Reg = f3Reg;
    }
    DecodeZeroInst(Instruction reg, Instruction f2Reg, Instruction f3Reg, U32 immWidth) : Decode(immWidth) {
        this->reg = reg;
        this->f2Reg = f2Reg;
        this->f3Reg = f3Reg;
    }
    DecodeZeroInst(Instruction reg, Instruction f2Reg, Instruction f3Reg, U32 immWidth, U32 signExtendWidth) : Decode(immWidth, signExtendWidth) {
        this->reg = reg;
        this->f2Reg = f2Reg;
        this->f3Reg = f3Reg;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        if (op->repNotZero) {
            op->inst = f2Reg;
        } else if (op->repZero) {
            op->inst = f3Reg;
        } else {
            op->inst = reg;
        }
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction f2Reg;
    Instruction f3Reg;
};

class DecodeIntIb : public Decode {
public:
    DecodeIntIb() : Decode(8) {
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        fetchImm(data, op);
        if (op->imm==0x80)
            op->inst = Int80;
        else if (op->imm==0x98)
            op->inst = Int98;
        else if (op->imm==0x99)
            op->inst = Int99;
        else if (op->imm == 0x9a)
            op->inst = Int9A;
        else
            op->inst = IntIb;
    }
};

class DecodeDirect : public Decode {
public:
    DecodeDirect(Instruction reg) {
        this->reg = reg;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = reg;
        op->base = data->ds;
        op->rm=regZero; 
        op->sibIndex=regZero; 
        if (data->ea16) {
            op->disp = data->fetch16();
        } else {
            op->disp = data->fetch32();
        }
    }

private:
    Instruction reg;
};

class Decode2Byte : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        data->opCode+=0x100;
        data->inst = data->fetch8()+data->opCode;
        if (!decoder[data->inst]) {
            op->inst = Invalid;
            return;
        }
        decoder[data->inst]->decode(data, op);
    }
};

class Decode66 : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        data->opCode=0x200;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class Decode67 : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        data->ea16 = 0;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class Decode266 : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        data->opCode=0;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class Decode267 : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        data->ea16 = 1;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class DecodeLock : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->lock = 1;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class DecodeRepNZ : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->repNotZero = 1;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class DecodeRepZ : public Decode {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->repZero = 1;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }
};

class DecodeSeg : public Decode {
public:
    DecodeSeg(U32 seg) {
        this->seg = seg;
    }

    void decode(DecodeData* data, DecodedOp* op) const override {
        data->ds = seg;
        data->ss = seg;
        data->inst = data->fetch8()+data->opCode;
        decoder[data->inst]->decode(data, op);
    }

private:
    U32 seg;
};

class DecodeIwIw : public Decode {
public:
    DecodeIwIw(Instruction reg) {
        this->reg = reg;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = reg;
        op->disp = data->fetch16();
        op->imm = data->fetch16();
    }

private:
    Instruction reg;
};

class DecodeIdIw : public Decode {
public:
    DecodeIdIw(Instruction reg) {
        this->reg = reg;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = reg;
        op->disp = data->fetch32();
        op->imm = data->fetch16();
    }

private:
    Instruction reg;
};

class DecodeFunc : public Decode {
public:
    DecodeFunc() {
    }
    DecodeFunc(U32 immWidth) : Decode(immWidth) {
    }
    DecodeFunc(U32 immWidth, U32 signExtendWidth) : Decode(immWidth, signExtendWidth) {
    }
    void func(DecodeData* data, DecodedOp* op, U32 rm, Instruction reg, Instruction mem) const {
        if (rm>=0xC0) {    
            op->inst = reg;
            op->reg = E(rm);
        } else {
            op->inst = mem;
            decodeEa(data, op, rm);
        }
    }
};

class DecodeGrp1_8 : public DecodeFunc {
public:
    DecodeGrp1_8(U32 immWidth) : DecodeFunc(immWidth) {
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, AddR8I8, AddE8I8); break;
        case 1: func(data, op, rm, OrR8I8, OrE8I8); break;
        case 2: func(data, op, rm, AdcR8I8, AdcE8I8); break;
        case 3: func(data, op, rm, SbbR8I8, SbbE8I8); break;
        case 4: func(data, op, rm, AndR8I8, AndE8I8); break;
        case 5: func(data, op, rm, SubR8I8, SubE8I8); break;
        case 6: func(data, op, rm, XorR8I8, XorE8I8); break;
        case 7: func(data, op, rm, CmpR8I8, CmpE8I8); break;
        }	
        fetchImm(data, op);
    }
};

class DecodeGrp1_16 : public DecodeFunc {
public:
    DecodeGrp1_16(U32 immWidth) : DecodeFunc(immWidth) {
    }
    DecodeGrp1_16(U32 immWidth, U32 signExtendWidth) : DecodeFunc(immWidth, signExtendWidth) {
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, AddR16I16, AddE16I16); break;
        case 1: func(data, op, rm, OrR16I16, OrE16I16); break;
        case 2: func(data, op, rm, AdcR16I16, AdcE16I16); break;
        case 3: func(data, op, rm, SbbR16I16, SbbE16I16); break;
        case 4: func(data, op, rm, AndR16I16, AndE16I16); break;
        case 5: func(data, op, rm, SubR16I16, SubE16I16); break;
        case 6: func(data, op, rm, XorR16I16, XorE16I16); break;
        case 7: func(data, op, rm, CmpR16I16, CmpE16I16); break;
        }	
        fetchImm(data, op);
    }
};

class DecodeGrp1_32 : public DecodeFunc {
public:
    DecodeGrp1_32(U32 immWidth) : DecodeFunc(immWidth) {
    }
    DecodeGrp1_32(U32 immWidth, U32 signExtendWidth) : DecodeFunc(immWidth, signExtendWidth) {
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, AddR32I32, AddE32I32); break;
        case 1: func(data, op, rm, OrR32I32, OrE32I32); break;
        case 2: func(data, op, rm, AdcR32I32, AdcE32I32); break;
        case 3: func(data, op, rm, SbbR32I32, SbbE32I32); break;
        case 4: func(data, op, rm, AndR32I32, AndE32I32); break;
        case 5: func(data, op, rm, SubR32I32, SubE32I32); break;
        case 6: func(data, op, rm, XorR32I32, XorE32I32); break;
        case 7: func(data, op, rm, CmpR32I32, CmpE32I32); break;
        }	
        fetchImm(data, op);
    }
};

class DecodeGrp2_8 : public DecodeFunc {
public:
    DecodeGrp2_8(U32 immWidth, U32 imm) : DecodeFunc(immWidth) {
        this->imm = imm;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, RolR8I8, RolE8I8); break;
        case 1: func(data, op, rm, RorR8I8, RorE8I8); break;
        case 2: func(data, op, rm, RclR8I8, RclE8I8); break;
        case 3: func(data, op, rm, RcrR8I8, RcrE8I8); break;
        case 4: func(data, op, rm, ShlR8I8, ShlE8I8); break;
        case 5: func(data, op, rm, ShrR8I8, ShrE8I8); break;
        case 6: func(data, op, rm, ShlR8I8, ShlE8I8); break;
        case 7: func(data, op, rm, SarR8I8, SarE8I8); break;
        }	
        if (this->imm) {
            op->imm = this->imm;
        } else {
            fetchImm(data, op);            
            op->imm&=0x1f;
            if (op->imm==0)
                op->inst = Nop;
            switch (G(rm)) {
                case 2: op->imm = op->imm % 9; break;
                case 3: op->imm = op->imm % 9; break;
                default: break;
            }
        }
    }
private:
    U32 imm;
};

class DecodeGrp2_16 : public DecodeFunc {
public:
    DecodeGrp2_16(U32 immWidth, U32 imm) : DecodeFunc(immWidth) {
        this->imm = imm;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, RolR16I8, RolE16I8); break;
        case 1: func(data, op, rm, RorR16I8, RorE16I8); break;
        case 2: func(data, op, rm, RclR16I8, RclE16I8); break;
        case 3: func(data, op, rm, RcrR16I8, RcrE16I8); break;
        case 4: func(data, op, rm, ShlR16I8, ShlE16I8); break;
        case 5: func(data, op, rm, ShrR16I8, ShrE16I8); break;
        case 6: func(data, op, rm, ShlR16I8, ShlE16I8); break;
        case 7: func(data, op, rm, SarR16I8, SarE16I8); break;
        }	
        if (this->imm) {
            op->imm = this->imm;
        } else {
            fetchImm(data, op);
            op->imm&=0x1f;
            if (op->imm==0)
                op->inst = Nop;
            switch (G(rm)) {
                case 2: op->imm = op->imm % 17; break;
                case 3: op->imm = op->imm % 17; break;
                default: break;
            }
        }
    }
private:
    U32 imm;
};

class DecodeGrp2_32 : public DecodeFunc {
public:
    DecodeGrp2_32(U32 immWidth, U32 imm) : DecodeFunc(immWidth) {
        this->imm = imm;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, RolR32I8, RolE32I8); break;
        case 1: func(data, op, rm, RorR32I8, RorE32I8); break;
        case 2: func(data, op, rm, RclR32I8, RclE32I8); break;
        case 3: func(data, op, rm, RcrR32I8, RcrE32I8); break;
        case 4: func(data, op, rm, ShlR32I8, ShlE32I8); break;
        case 5: func(data, op, rm, ShrR32I8, ShrE32I8); break;
        case 6: func(data, op, rm, ShlR32I8, ShlE32I8); break;
        case 7: func(data, op, rm, SarR32I8, SarE32I8); break;
        }	
        if (this->imm) {
            op->imm = this->imm;
        } else {
            fetchImm(data, op);
            op->imm&=0x1f;
            if (op->imm==0)
                op->inst = Nop;
        }
    }
private:
    U32 imm;
};

class DecodeGrp2_Cl_8 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, RolR8Cl, RolE8Cl); break;
        case 1: func(data, op, rm, RorR8Cl, RorE8Cl); break;
        case 2: func(data, op, rm, RclR8Cl, RclE8Cl); break;
        case 3: func(data, op, rm, RcrR8Cl, RcrE8Cl); break;
        case 4: func(data, op, rm, ShlR8Cl, ShlE8Cl); break;
        case 5: func(data, op, rm, ShrR8Cl, ShrE8Cl); break;
        case 6: func(data, op, rm, ShlR8Cl, ShlE8Cl); break;
        case 7: func(data, op, rm, SarR8Cl, SarE8Cl); break;
        }	
    }
};

class DecodeGrp2_Cl_16 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, RolR16Cl, RolE16Cl); break;
        case 1: func(data, op, rm, RorR16Cl, RorE16Cl); break;
        case 2: func(data, op, rm, RclR16Cl, RclE16Cl); break;
        case 3: func(data, op, rm, RcrR16Cl, RcrE16Cl); break;
        case 4: func(data, op, rm, ShlR16Cl, ShlE16Cl); break;
        case 5: func(data, op, rm, ShrR16Cl, ShrE16Cl); break;
        case 6: func(data, op, rm, ShlR16Cl, ShlE16Cl); break;
        case 7: func(data, op, rm, SarR16Cl, SarE16Cl); break;
        }	
    }
};

class DecodeGrp2_Cl_32 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0: func(data, op, rm, RolR32Cl, RolE32Cl); break;
        case 1: func(data, op, rm, RorR32Cl, RorE32Cl); break;
        case 2: func(data, op, rm, RclR32Cl, RclE32Cl); break;
        case 3: func(data, op, rm, RcrR32Cl, RcrE32Cl); break;
        case 4: func(data, op, rm, ShlR32Cl, ShlE32Cl); break;
        case 5: func(data, op, rm, ShrR32Cl, ShrE32Cl); break;
        case 6: func(data, op, rm, ShlR32Cl, ShlE32Cl); break;
        case 7: func(data, op, rm, SarR32Cl, SarE32Cl); break;
        }	
    }
};

class DecodeGrp3_8 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0:
        case 1: func(data, op, rm, TestR8I8, TestE8I8); op->imm = data->fetch8(); break;
        case 2: func(data, op, rm, NotR8, NotE8); break;
        case 3: func(data, op, rm, NegR8, NegE8); break;
        case 4: func(data, op, rm, MulR8, MulE8); break;
        case 5: func(data, op, rm, IMulR8, IMulE8); break;
        case 6: func(data, op, rm, DivR8, DivE8); break;
        case 7: func(data, op, rm, IDivR8, IDivE8); break;
        }	
    }
};

class DecodeGrp3_16 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0:
        case 1: func(data, op, rm, TestR16I16, TestE16I16); op->imm = data->fetch16(); break;
        case 2: func(data, op, rm, NotR16, NotE16); break;
        case 3: func(data, op, rm, NegR16, NegE16); break;
        case 4: func(data, op, rm, MulR16, MulE16); break;
        case 5: func(data, op, rm, IMulR16, IMulE16); break;
        case 6: func(data, op, rm, DivR16, DivE16); break;
        case 7: func(data, op, rm, IDivR16, IDivE16); break;
        }	
    }
};

class DecodeGrp3_32 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U32 rm = data->fetch8();

        switch (G(rm)) {
        case 0:
        case 1: func(data, op, rm, TestR32I32, TestE32I32); op->imm = data->fetch32(); break;
        case 2: func(data, op, rm, NotR32, NotE32); break;
        case 3: func(data, op, rm, NegR32, NegE32); break;
        case 4: func(data, op, rm, MulR32, MulE32); break;
        case 5: func(data, op, rm, IMulR32, IMulE32); break;
        case 6: func(data, op, rm, DivR32, DivE32); break;
        case 7: func(data, op, rm, IDivR32, IDivE32); break;
        }	
    }
};

class DecodeGrp4_8 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x00: func(data, op, rm, IncR8, IncE8); break;
        case 0x01: func(data, op, rm, DecR8, DecE8); break;
        case 0x07: {
#ifdef BOXEDWINE_64
                U64 address = data->fetch32();
                address |= ((U64)data->fetch32()) << 32;
                op->pfn = (OpCallback)address;
#else
                op->pfn = (OpCallback)(uintptr_t)data->fetch32();
#endif
            op->inst = Callback;
            break;
        }
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
    }
};

class DecodeGrp5_16 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x00: func(data, op, rm, IncR16, IncE16); break;
        case 0x01: func(data, op, rm, DecR16, DecE16); break;
        case 0x02: func(data, op, rm, CallR16, CallE16); break;
        case 0x03: func(data, op, rm, Invalid, CallFarE16); break;
        case 0x04: func(data, op, rm, JmpR16, JmpE16); break;
        case 0x05: func(data, op, rm, Invalid, JmpFarE16); break;
        case 0x06: func(data, op, rm, PushR16, PushE16); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
    }
};

class DecodeGrp5_32 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x00: func(data, op, rm, IncR32, IncE32); break;
        case 0x01: func(data, op, rm, DecR32, DecE32); break;
        case 0x02: func(data, op, rm, CallR32, CallE32); break;
        case 0x03: func(data, op, rm, Invalid, CallFarE32); break;
        case 0x04: func(data, op, rm, JmpR32, JmpE32); break;
        case 0x05: func(data, op, rm, Invalid, JmpFarE32); break;
        case 0x06: func(data, op, rm, PushR32, PushE32); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
    }
};

class DecodeGrp6_32 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x00: func(data, op, rm, SLDTReg, SLDTE16); break;
        case 0x01: func(data, op, rm, STRReg, STRE16); break;
        case 0x02: func(data, op, rm, LLDTR16, LLDTE16); break;
        case 0x03: func(data, op, rm, LTRR16, LTRE16); break;
        case 0x04: func(data, op, rm, VERRR16, VERRE16); break;
        case 0x05: func(data, op, rm, VERWR16, VERWE16); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
    }
};

class DecodeGrp6_16 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x00: func(data, op, rm, SLDTReg, SLDTE16); break;
        case 0x01: func(data, op, rm, STRReg, STRE16); break;
        case 0x02: func(data, op, rm, LLDTR16, LLDTE16); break;
        case 0x03: func(data, op, rm, LTRR16, LTRE16); break;
        case 0x04: func(data, op, rm, VERRR16, VERRE16); break;
        case 0x05: func(data, op, rm, VERWR16, VERWE16); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
    }
};

class DecodeGrp7_32 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x00: func(data, op, rm, Invalid, SGDT); break;
        case 0x01: func(data, op, rm, Invalid, SIDT); break;
        case 0x02: func(data, op, rm, Invalid, LGDT); break;
        case 0x03: func(data, op, rm, Invalid, LIDT); break;
        case 0x04: func(data, op, rm, SMSWRreg, SMSW); break;
        case 0x06: func(data, op, rm, LMSWRreg, LMSW); break;
        case 0x07: func(data, op, rm, Invalid, INVLPG); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
    }
};

class DecodeGrp8_16 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x04: func(data, op, rm, BtR16, BtE16); break;
        case 0x05: func(data, op, rm, BtsR16, BtsE16); break;
        case 0x06: func(data, op, rm, BtrR16, BtrE16); break;
        case 0x07: func(data, op, rm, BtcR16, BtcE16); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        op->extra = data->fetch8() & 15;
        op->imm = 1 << op->extra;
#else
        op->imm = 1 << (data->fetch8() & 15);
#endif
    }
};

class DecodeGrp8_32 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
        case 0x04: func(data, op, rm, BtR32, BtE32); break;
        case 0x05: func(data, op, rm, BtsR32, BtsE32); break;
        case 0x06: func(data, op, rm, BtrR32, BtrE32); break;
        case 0x07: func(data, op, rm, BtcR32, BtcE32); break;
        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
        }	
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        op->extra = data->fetch8() & 31;
        op->imm = 1 << op->extra;
#else
        op->imm = 1 << (data->fetch8() & 31);
#endif
    }
};

class DecodeEnter : public Decode {
public:
    public:
    DecodeEnter(Instruction reg) {
        this->reg = reg;
    }

    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = this->reg;
        op->imm = data->fetch16();
        op->disp = data->fetch8() & 0x1f;
    }

private:
    Instruction reg;
};

class DecodeFPU0 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);
            switch (G(rm)) {
            case 0: op->inst = FADD_ST0_STj; break;
            case 1: op->inst = FMUL_ST0_STj; break;
            case 2: op->inst = FCOM_STi; break;
            case 3: op->inst = FCOM_STi_Pop; break;
            case 4: op->inst = FSUB_ST0_STj; break;
            case 5: op->inst = FSUBR_ST0_STj; break;
            case 6: op->inst = FDIV_ST0_STj; break;
            case 7: op->inst = FDIVR_ST0_STj; break;
            }
        } else {
            decodeEa(data, op, rm);
            switch (G(rm)) {
                case 0: op->inst = FADD_SINGLE_REAL; break;
                case 1: op->inst = FMUL_SINGLE_REAL; break;
                case 2: op->inst = FCOM_SINGLE_REAL; break;
                case 3: op->inst = FCOM_SINGLE_REAL_Pop; break;
                case 4: op->inst = FSUB_SINGLE_REAL; break;
                case 5: op->inst = FSUBR_SINGLE_REAL; break;
                case 6: op->inst = FDIV_SINGLE_REAL; break;
                case 7: op->inst = FDIVR_SINGLE_REAL; break;
            }
        }
    }
};

class DecodeFPU1 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {				
            switch ((rm >> 3) & 7) {
                case 0: op->reg = E(rm); op->inst = FLD_STi; break;
                case 1: op->reg = E(rm); op->inst = FXCH_STi; break;
                case 2: op->inst = FNOP; break;
                case 3: op->reg = E(rm); op->inst = FST_STi_Pop; break;
                case 4:
                {
                    switch (rm & 7) {
                        case 0: op->inst = FCHS; break;
                        case 1: op->inst = FABS; break;
                        case 4: op->inst = FTST; break;
                        case 5: op->inst = FXAM; break;
                        default: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
                    }
                    break;
                }
                case 5:
                {
                    switch (rm & 7) {
                        case 0: op->inst = FLD1; break;
                        case 1: op->inst = FLDL2T; break;
                        case 2: op->inst = FLDL2E; break;
                        case 3: op->inst = FLDPI; break;
                        case 4: op->inst = FLDLG2; break;
                        case 5: op->inst = FLDLN2; break;
                        case 6: op->inst = FLDZ; break;
                        case 7: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
                    }
                    break;
                }
                case 6:
                {
                    switch (rm & 7) {
                        case 0: op->inst = F2XM1; break;
                        case 1: op->inst = FYL2X; break;
                        case 2: op->inst = FPTAN; break;
                        case 3: op->inst = FPATAN; break;
                        case 4: op->inst = FXTRACT; break;
                        case 5: op->inst = FPREM_nearest; break;
                        case 6: op->inst = FDECSTP; break;
                        case 7: op->inst = FINCSTP; break;
                    }
                    break;
                }
                case 7:
                {
                    switch (rm & 7) {
                        case 0: op->inst = FPREM; break;
                        case 1: op->inst = FYL2XP1; break;
                        case 2: op->inst = FSQRT; break;
                        case 3: op->inst = FSINCOS; break;
                        case 4: op->inst = FRNDINT; break;
                        case 5: op->inst = FSCALE; break;
                        case 6: op->inst = FSIN; break;
                        case 7: op->inst = FCOS; break;
                    }
                    break;
                }
            }
        } else {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FLD_SINGLE_REAL; break;
                case 1: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
                case 2: op->inst = FST_SINGLE_REAL; break;
                case 3: op->inst = FST_SINGLE_REAL_Pop; break;
                case 4: op->inst = FLDENV; break;
                case 5: op->inst = FLDCW; break;
                case 6: op->inst = FNSTENV; break;
                case 7: op->inst = FNSTCW; break;
            }
        }
    }
};

class DecodeFPU2 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);         
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FCMOV_ST0_STj_CF; break;
                case 1: op->inst = FCMOV_ST0_STj_ZF; break;
                case 2: op->inst = FCMOV_ST0_STj_CF_OR_ZF; break;
                case 3: op->inst = FCMOV_ST0_STj_PF; break;
                case 5:
                    if ((rm & 7)==1) {
                        op->inst = FUCOMPP;
                        break;
                    }
                // intentional fall through
                    [[fallthrough]];
                default:
                    op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
            }
        } else {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FIADD_DWORD_INTEGER; break;
                case 1: op->inst = FIMUL_DWORD_INTEGER; break;
                case 2: op->inst = FICOM_DWORD_INTEGER; break;
                case 3: op->inst = FICOM_DWORD_INTEGER_Pop; break;
                case 4: op->inst = FISUB_DWORD_INTEGER; break;
                case 5: op->inst = FISUBR_DWORD_INTEGER; break;
                case 6: op->inst = FIDIV_DWORD_INTEGER; break;
                case 7: op->inst = FIDIVR_DWORD_INTEGER; break;
            }
        }
    }
};

class DecodeFPU3 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FCMOV_ST0_STj_NCF; break;
                case 1: op->inst = FCMOV_ST0_STj_NZF; break;
                case 2: op->inst = FCMOV_ST0_STj_NCF_AND_NZF; break;
                case 3: op->inst = FCMOV_ST0_STj_NPF; break;
                case 4:
                {
                    switch (rm & 7) {
                        case 2:op->inst = FNCLEX; break;
                        case 3:op->inst = FNINIT; break;
                        default:op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
                    }
                    break;
                }
                case 5: op->inst = FUCOMI_ST0_STj; break;
                case 6: op->inst = FCOMI_ST0_STj; break;
                default:op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
            }
        } else {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FILD_DWORD_INTEGER; break;
                case 1: op->inst = FISTTP32; break;
                case 2: op->inst = FIST_DWORD_INTEGER; break;
                case 3: op->inst = FIST_DWORD_INTEGER_Pop; break;
                case 5: op->inst = FLD_EXTENDED_REAL; break;
                case 7: op->inst = FSTP_EXTENDED_REAL; break;
                default:op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
            }
        }
    }
};

class DecodeFPU4 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FADD_STi_ST0; break;
                case 1: op->inst = FMUL_STi_ST0; break;
                case 2: op->inst = FCOM_STi; break;
                case 3: op->inst = FCOM_STi_Pop; break;
                case 4: op->inst = FSUBR_STi_ST0; break;
                case 5: op->inst = FSUB_STi_ST0; break;
                case 6: op->inst = FDIVR_STi_ST0; break;
                case 7: op->inst = FDIV_STi_ST0; break;
            }
        } else  {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FADD_DOUBLE_REAL; break;
                case 1: op->inst = FMUL_DOUBLE_REAL; break;
                case 2: op->inst = FCOM_DOUBLE_REAL; break;
                case 3: op->inst = FCOM_DOUBLE_REAL_Pop; break;
                case 4: op->inst = FSUB_DOUBLE_REAL; break;
                case 5: op->inst = FSUBR_DOUBLE_REAL; break;
                case 6: op->inst = FDIV_DOUBLE_REAL; break;
                case 7: op->inst = FDIVR_DOUBLE_REAL; break;
            }
        }
    }
};

class DecodeFPU5 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FFREE_STi; break;
                case 1: op->inst = FXCH_STi; break;
                case 2: op->inst = FST_STi; break;
                case 3: op->inst = FST_STi_Pop; break;
                case 4: op->inst = FUCOM_STi; break;
                case 5: op->inst = FUCOM_STi_Pop; break;
                default:op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
            }
        } else {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FLD_DOUBLE_REAL; break;
                case 1: op->inst = FISTTP64; break;
                case 2: op->inst = FST_DOUBLE_REAL; break;
                case 3: op->inst = FST_DOUBLE_REAL_Pop; break;
                case 4: op->inst = FRSTOR; break;
                case 5: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
                case 6: op->inst = FNSAVE; break;
                case 7: op->inst = FNSTSW; break;
            }
        }
    }
};

class DecodeFPU6 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FADD_STi_ST0_Pop; break;
                case 1: op->inst = FMUL_STi_ST0_Pop; break;
                case 2: op->inst = FCOM_STi_Pop; break;
                case 3:
                    if ((rm & 7) == 1)
                        op->inst = FCOMPP;
                    else {
                        op->inst = Invalid; op->reg = rm; op->imm = data->inst;
                    }
                    break;
                break;
                case 4: op->inst = FSUBR_STi_ST0_Pop; break;
                case 5: op->inst = FSUB_STi_ST0_Pop; break;
                case 6: op->inst = FDIVR_STi_ST0_Pop; break;
                case 7: op->inst = FDIV_STi_ST0_Pop; break;
            }
        } else {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FIADD_WORD_INTEGER; break;
                case 1: op->inst = FIMUL_WORD_INTEGER; break;
                case 2: op->inst = FICOM_WORD_INTEGER; break;
                case 3: op->inst = FICOM_WORD_INTEGER_Pop; break;
                case 4: op->inst = FISUB_WORD_INTEGER; break;
                case 5: op->inst = FISUBR_WORD_INTEGER; break;
                case 6: op->inst = FIDIV_WORD_INTEGER; break;
                case 7: op->inst = FIDIVR_WORD_INTEGER; break;
            }
        }
    }
};

class DecodeFPU7 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm >= 0xc0) {
            op->reg = E(rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FFREEP_STi; break;
                case 1: op->inst = FXCH_STi; break;
                case 2:
                case 3: op->inst = FST_STi_Pop; break;
                case 4:
                    if ((rm & 7)==0)
                        op->inst = FNSTSW_AX;
                    else {
                        op->inst = Invalid; op->reg = rm; op->imm = data->inst;
                    }
                    break;
                case 5: op->inst = FUCOMI_ST0_STj_Pop; break;
                case 6: op->inst = FCOMI_ST0_STj_Pop; break;
                case 7: op->inst = Invalid; op->reg = rm; op->imm = data->inst; break;
            }
        } else  {
            decodeEa(data, op, rm);
            switch ((rm >> 3) & 7) {
                case 0: op->inst = FILD_WORD_INTEGER; break;
                case 1: op->inst = FISTTP16; break;
                case 2: op->inst = FIST_WORD_INTEGER; break;
                case 3: op->inst = FIST_WORD_INTEGER_Pop; break;
                case 4: op->inst = FBLD_PACKED_BCD; break;
                case 5: op->inst = FILD_QWORD_INTEGER; break;
                case 6: op->inst = FBSTP_PACKED_BCD; break;
                case 7: op->inst = FISTP_QWORD_INTEGER; break;
            }
        }
    }
};

class DecodeGroup : public Decode {
public:
    DecodeGroup(Instruction i1, Instruction i2, Instruction i3, Instruction i4, Instruction i5, Instruction i6, Instruction i7, Instruction i8) {
        this->inst[0] = i1;
        this->inst[1] = i2;
        this->inst[2] = i3;
        this->inst[3] = i4;
        this->inst[4] = i5;
        this->inst[5] = i6;
        this->inst[6] = i7;
        this->inst[7] = i8;
    }
    DecodeGroup(Instruction i1, Instruction i2, Instruction i3, Instruction i4, Instruction i5, Instruction i6, Instruction i7, Instruction i8, U32 immWidth) : Decode(immWidth) {
        this->inst[0] = i1;
        this->inst[1] = i2;
        this->inst[2] = i3;
        this->inst[3] = i4;
        this->inst[4] = i5;
        this->inst[5] = i6;
        this->inst[6] = i7;
        this->inst[7] = i8;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();
        if (rm >= 0xc0) {
            op->reg = E(rm);
        } else {
            decodeEa(data, op, rm);
        }
        op->inst = inst[G(rm)];
        fetchImm(data, op);
    }

private:
    Instruction inst[8];
};

class Decode3AE : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
            case 0: func(data, op, rm, Invalid, Fxsave); break;
            case 1: func(data, op, rm, Invalid, Fxrstor); break;
            case 2: func(data, op, rm, Invalid, Ldmxcsr); break;
            case 3: func(data, op, rm, Invalid, Stmxcsr); break;
            case 4: func(data, op, rm, Invalid, Xsave); break;
            case 5: func(data, op, rm, Lfence, Xrstor); break;
            case 6: func(data, op, rm, Mfence, Invalid); break;
            case 7: func(data, op, rm, Sfence, Clflush); break;
        }
    }
};

class Decode318 : public DecodeFunc {
public:
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        switch (G(rm)) {
            case 0: func(data, op, rm, Invalid, PrefetchNTA); break;
            case 1: func(data, op, rm, Invalid, PrefetchT0); break;
            case 2: func(data, op, rm, Invalid, PrefetchT1); break;
            case 3: func(data, op, rm, Invalid, PrefetchT2); break;
            case 4: func(data, op, rm, Invalid, Invalid); break;
            case 5: func(data, op, rm, Invalid, Invalid); break;
            case 6: func(data, op, rm, Invalid, Invalid); break;
            case 7: func(data, op, rm, Invalid, Invalid); break;
        }
    }
};

class DecodeSSE : public Decode {
public:
    DecodeSSE(Instruction reg1, Instruction mem1, Instruction reg2, Instruction mem2) {
        this->reg1 = reg1;
        this->mem1 = mem1;
        this->reg2 = reg2;
        this->mem2 = mem2;
    }
    DecodeSSE(Instruction reg1, Instruction mem1, Instruction reg2, Instruction mem2, U32 imm) : Decode(imm) {
        this->reg1 = reg1;
        this->mem1 = mem1;
        this->reg2 = reg2;
        this->mem2 = mem2;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            op->inst = op->repZero?this->reg2:this->reg1;
            op->reg = G(rm);
            op->rm = E(rm);
        } else {
            op->inst = op->repZero?this->mem2:this->mem1;
            op->reg = G(rm);
            decodeEa(data, op, rm);
        } 
        op->repZero = false;
        fetchImm(data, op);
    }

private:
    Instruction reg1;
    Instruction mem1;
    Instruction reg2;
    Instruction mem2;
};

class DecodeSSE2 : public Decode {
public:
    DecodeSSE2(Instruction reg, Instruction mem, Instruction f2Reg, Instruction f2Mem, Instruction f3Reg, Instruction f3Mem) {
        this->reg = reg;
        this->mem = mem;
        this->f2Reg = f2Reg;
        this->f2Mem = f2Mem;
        this->f3Reg = f3Reg;
        this->f3Mem = f3Mem;
    }
    DecodeSSE2(Instruction reg, Instruction mem, Instruction f2Reg, Instruction f2Mem, Instruction f3Reg, Instruction f3Mem, U32 imm) : Decode(imm) {
        this->reg = reg;
        this->mem = mem;
        this->f2Reg = f2Reg;
        this->f2Mem = f2Mem;
        this->f3Reg = f3Reg;
        this->f3Mem = f3Mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            if (op->repZero) {
                op->inst = this->f3Reg;
            } else if (op->repNotZero) {
                op->inst = this->f2Reg;
            } else {
                op->inst = this->reg;
            }
            
            op->reg = G(rm);
            op->rm = E(rm);
        } else {
            if (op->repZero) {
                op->inst = this->f3Mem;
            } else if (op->repNotZero) {
                op->inst = this->f2Mem;
            } else {
                op->inst = this->mem;
            }
            op->reg = G(rm);
            decodeEa(data, op, rm);
        } 
        op->repZero = false;
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction mem;
    Instruction f2Reg;
    Instruction f2Mem;
    Instruction f3Reg;
    Instruction f3Mem;
};

class DecodeSSE2r : public Decode {
public:
    DecodeSSE2r(Instruction reg, Instruction mem, Instruction f2Reg, Instruction f2Mem, Instruction f3Reg, Instruction f3Mem) {
        this->reg = reg;
        this->mem = mem;
        this->f2Reg = f2Reg;
        this->f2Mem = f2Mem;
        this->f3Reg = f3Reg;
        this->f3Mem = f3Mem;
    }
    DecodeSSE2r(Instruction reg, Instruction mem, Instruction f2Reg, Instruction f2Mem, Instruction f3Reg, Instruction f3Mem, U32 imm) : Decode(imm) {
        this->reg = reg;
        this->mem = mem;
        this->f2Reg = f2Reg;
        this->f2Mem = f2Mem;
        this->f3Reg = f3Reg;
        this->f3Mem = f3Mem;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            if (op->repZero) {
                op->inst = this->f3Reg;
            } else if (op->repNotZero) {
                op->inst = this->f2Reg;
            } else {
                op->inst = this->reg;
            }
            
            op->reg = E(rm);
            op->rm = G(rm);
        } else {
            if (op->repZero) {
                op->inst = this->f3Mem;
            } else if (op->repNotZero) {
                op->inst = this->f2Mem;
            } else {
                op->inst = this->mem;
            }
            op->reg = G(rm);
            decodeEa(data, op, rm);
        } 
        op->repZero = false;
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction mem;
    Instruction f2Reg;
    Instruction f2Mem;
    Instruction f3Reg;
    Instruction f3Mem;
};

class DecodeSSE2custom : public Decode {
public:
    DecodeSSE2custom(Instruction reg, Instruction mem, bool reversed1, Instruction f2Reg, Instruction f2Mem, bool reversed2, Instruction f3Reg, Instruction f3Mem, bool reversed3) {
        this->reg = reg;
        this->mem = mem;
        this->reversed1 = reversed1;
        this->f2Reg = f2Reg;
        this->f2Mem = f2Mem;
        this->reversed2 = reversed2;
        this->f3Reg = f3Reg;
        this->f3Mem = f3Mem;
        this->reversed3 = reversed3;
    }
    DecodeSSE2custom(Instruction reg, Instruction mem, bool reversed1, Instruction f2Reg, Instruction f2Mem, bool reversed2, Instruction f3Reg, Instruction f3Mem, bool reversed3, U32 imm) : Decode(imm) {
        this->reg = reg;
        this->mem = mem;
        this->reversed1 = reversed1;
        this->f2Reg = f2Reg;
        this->f2Mem = f2Mem;
        this->reversed2 = reversed2;
        this->f3Reg = f3Reg;
        this->f3Mem = f3Mem;
        this->reversed3 = reversed3;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        U8 rm = data->fetch8();

        if (rm>=0xC0) {    
            bool reversed = false;

            if (op->repZero) {
                op->inst = this->f3Reg;
                reversed = this->reversed3;
            } else if (op->repNotZero) {
                op->inst = this->f2Reg;
                reversed = this->reversed2;
            } else {
                op->inst = this->reg;
                reversed = this->reversed1;
            }
            if (reversed) {
                op->reg = G(rm);
                op->rm = E(rm);
            } else {
                op->reg = E(rm);
                op->rm = G(rm);
            }
        } else {
            if (op->repZero) {
                op->inst = this->f3Mem;
            } else if (op->repNotZero) {
                op->inst = this->f2Mem;
            } else {
                op->inst = this->mem;
            }
            op->reg = G(rm);
            decodeEa(data, op, rm);
        } 
        op->repZero = false;
        fetchImm(data, op);
    }

private:
    Instruction reg;
    Instruction mem;
    bool reversed1;
    Instruction f2Reg;
    Instruction f2Mem;
    bool reversed2;
    Instruction f3Reg;
    Instruction f3Mem;
    bool reversed3;
};


class DecodeRdtsc : public Decode {
public:
    DecodeRdtsc(Instruction reg) {
        this->reg = reg;
    }
    void decode(DecodeData* data, DecodedOp* op) const override {
        op->inst = reg;
        op->imm = data->opCountSoFarInThisBlock;
    }

private:
    Instruction reg;
};

DecodeRM decodeAddEbGb(AddR8R8, AddE8R8);         // ADD Eb,Gb
DecodeRM decodeAddEwGw(AddR16R16, AddE16R16);   // ADD Ew,Gw
DecodeRM decodeAddEdGd(AddR32R32, AddE32R32);   // ADD Ed,Gd
DecodeRMr decodeAddGbEb(AddR8R8, AddR8E8);        // ADD Gb,Eb
DecodeRMr decodeAddGwEw(AddR16R16, AddR16E16);  // ADD Gw,Ew
DecodeRMr decodeAddGdEd(AddR32R32, AddR32E32);  // ADD Gd,Ed
DecodeReg decodeAddAlIb(AddR8I8, 0, 8);                          // ADD Al,Ib
DecodeReg decodeAddAxIw(AddR16I16, 0, 16);                       // ADD Ax,Iw
DecodeReg decodeAddEaxId(AddR32I32, 0, 32);                       // ADD Eax,Id

DecodeRM decodeOrEbGb(OrR8R8, OrE8R8);             // OR Eb,Gb
DecodeRM decodeOrEwGw(OrR16R16, OrE16R16);       // OR Ew,Gw
DecodeRM decodeOrEdGd(OrR32R32, OrE32R32);       // OR Ed,Gd
DecodeRMr decodeOrGbEb(OrR8R8, OrR8E8);            // OR Gb,Eb
DecodeRMr decodeOrGwEw(OrR16R16, OrR16E16);      // OR Gw,Ew
DecodeRMr decodeOrGdEd(OrR32R32, OrR32E32);      // OR Gd,Ed
DecodeReg decodeOrAlIb(OrR8I8, 0, 8);                            // OR Al,Ib
DecodeReg decodeOrAxIw(OrR16I16, 0, 16);                         // OR Ax,Iw
DecodeReg decodeOrEaxId(OrR32I32, 0, 32);                         // OR Eax,Id

DecodeRM decodeAdcEbGb(AdcR8R8, AdcE8R8);         // ADC Eb,Gb
DecodeRM decodeAdcEwGw(AdcR16R16, AdcE16R16);   // ADC Ew,Gw
DecodeRM decodeAdcEdGd(AdcR32R32, AdcE32R32);   // ADC Ed,Gd
DecodeRMr decodeAdcGbEb(AdcR8R8, AdcR8E8);        // ADC Gb,Eb
DecodeRMr decodeAdcGwEw(AdcR16R16, AdcR16E16);  // ADC Gw,Ew
DecodeRMr decodeAdcGdEd(AdcR32R32, AdcR32E32);  // ADC Gd,Ed
DecodeReg decodeAdcAlIb(AdcR8I8, 0, 8);                          // ADC Al,Ib
DecodeReg decodeAdcAxIw(AdcR16I16, 0, 16);                       // ADC Ax,Iw
DecodeReg decodeAdcEaxId(AdcR32I32, 0, 32);                         // ADC Eax,Id

DecodeRM decodeSbbEbGb(SbbR8R8, SbbE8R8);         // SBB Eb,Gb
DecodeRM decodeSbbEwGw(SbbR16R16, SbbE16R16);   // SBB Ew,Gw
DecodeRM decodeSbbEdGd(SbbR32R32, SbbE32R32);   // SBB Ed,Gd
DecodeRMr decodeSbbGbEb(SbbR8R8, SbbR8E8);        // SBB Gb,Eb
DecodeRMr decodeSbbGwEw(SbbR16R16, SbbR16E16);  // SBB Gw,Ew
DecodeRMr decodeSbbGdEd(SbbR32R32, SbbR32E32);  // SBB Gd,Ed
DecodeReg decodeSbbAlIb(SbbR8I8, 0, 8);                          // SBB Al,Ib
DecodeReg decodeSbbAxIw(SbbR16I16, 0, 16);                       // SBB Ax,Iw
DecodeReg decodeSbbEaxId(SbbR32I32, 0, 32);                         // SBB Eax,Id

DecodeRM decodeAndEbGb(AndR8R8, AndE8R8);         // AND Eb,Gb
DecodeRM decodeAndEwGw(AndR16R16, AndE16R16);   // AND Ew,Gw
DecodeRM decodeAndEdGd(AndR32R32, AndE32R32);   // AND Ed,Gd
DecodeRMr decodeAndGbEb(AndR8R8, AndR8E8);        // AND Gb,Eb
DecodeRMr decodeAndGwEw(AndR16R16, AndR16E16);  // AND Gw,Ew
DecodeRMr decodeAndGdEd(AndR32R32, AndR32E32);  // AND Gd,Ed
DecodeReg decodeAndAlIb(AndR8I8, 0, 8);                          // AND Al,Ib
DecodeReg decodeAndAxIw(AndR16I16, 0, 16);                       // AND Ax,Iw
DecodeReg decodeAndEaxId(AndR32I32, 0, 32);                         // AND Eax,Id

DecodeRM decodeSubEbGb(SubR8R8, SubE8R8);         // SUB Eb,Gb
DecodeRM decodeSubEwGw(SubR16R16, SubE16R16);   // SUB Ew,Gw
DecodeRM decodeSubEdGd(SubR32R32, SubE32R32);   // SUB Ed,Gd
DecodeRMr decodeSubGbEb(SubR8R8, SubR8E8);        // SUB Gb,Eb
DecodeRMr decodeSubGwEw(SubR16R16, SubR16E16);  // SUB Gw,Ew
DecodeRMr decodeSubGdEd(SubR32R32, SubR32E32);  // SUB Gd,Ed
DecodeReg decodeSubAlIb(SubR8I8, 0, 8);                          // SUB Al,Ib
DecodeReg decodeSubAxIw(SubR16I16, 0, 16);                       // SUB Ax,Iw
DecodeReg decodeSubEaxId(SubR32I32, 0, 32);                         // SUB Eax,Id

DecodeRM decodeXorEbGb(XorR8R8, XorE8R8);         // XOR Eb,Gb
DecodeRM decodeXorEwGw(XorR16R16, XorE16R16);   // XOR Ew,Gw
DecodeRM decodeXorEdGd(XorR32R32, XorE32R32);   // XOR Ed,Gd
DecodeRMr decodeXorGbEb(XorR8R8, XorR8E8);        // XOR Gb,Eb
DecodeRMr decodeXorGwEw(XorR16R16, XorR16E16);  // XOR Gw,Ew
DecodeRMr decodeXorGdEd(XorR32R32, XorR32E32);  // XOR Gd,Ed
DecodeReg decodeXorAlIb(XorR8I8, 0, 8);                          // XOR Al,Ib
DecodeReg decodeXorAxIw(XorR16I16, 0, 16);                       // XOR Ax,Iw
DecodeReg decodeXorEaxId(XorR32I32, 0, 32);                         // XOR Eax,Id

DecodeRM decodeCmpEbGb(CmpR8R8, CmpE8R8);         // CMP Eb,Gb
DecodeRM decodeCmpEwGw(CmpR16R16, CmpE16R16);   // CMP Ew,Gw
DecodeRM decodeCmpEdGd(CmpR32R32, CmpE32R32);   // CMP Ed,Gd
DecodeRMr decodeCmpGbEb(CmpR8R8, CmpR8E8);        // CMP Gb,Eb
DecodeRMr decodeCmpGwEw(CmpR16R16, CmpR16E16);  // CMP Gw,Ew
DecodeRMr decodeCmpGdEd(CmpR32R32, CmpR32E32);  // CMP Gd,Ed
DecodeReg decodeCmpAlIb(CmpR8I8, 0, 8);                          // CMP Al,Ib
DecodeReg decodeCmpAxIw(CmpR16I16, 0, 16);                       // CMP Ax,Iw
DecodeReg decodeCmpEaxId(CmpR32I32, 0, 32);                         // CMP Eax,Id

DecodeReg decodePushEs16(PushSeg16, ES);                            // PUSH ES
DecodeReg decodePushCs16(PushSeg16, CS);                            // PUSH CS
DecodeReg decodePushSs16(PushSeg16, SS);                            // PUSH SS
DecodeReg decodePushDs16(PushSeg16, DS);                            // PUSH DS
DecodeReg decodePushFs16(PushSeg16, FS);                            // PUSH FS
DecodeReg decodePushGs16(PushSeg16, GS);                            // PUSH GS
DecodeReg decodePopEs16(PopSeg16, ES);                              // POP ES
DecodeReg decodePopSs16(PopSeg16, SS);                              // POP SS
DecodeReg decodePopDs16(PopSeg16, DS);                              // POP DS
DecodeReg decodePopFs16(PopSeg16, FS);                              // POP FS
DecodeReg decodePopGs16(PopSeg16, GS);                              // POP GS

DecodeReg decodePushEs32(PushSeg32, ES);                            // PUSH ES
DecodeReg decodePushCs32(PushSeg32, CS);                            // PUSH CS
DecodeReg decodePushSs32(PushSeg32, SS);                            // PUSH SS
DecodeReg decodePushDs32(PushSeg32, DS);                            // PUSH DS
DecodeReg decodePushFs32(PushSeg32, FS);                            // PUSH FS
DecodeReg decodePushGs32(PushSeg32, GS);                            // PUSH GS
DecodeReg decodePopEs32(PopSeg32, ES);                              // POP ES
DecodeReg decodePopSs32(PopSeg32, SS);                              // POP SS
DecodeReg decodePopDs32(PopSeg32, DS);                              // POP DS
DecodeReg decodePopFs32(PopSeg32, FS);                              // POP FS
DecodeReg decodePopGs32(PopSeg32, GS);                              // POP GS

DecodeSeg decodeSegEs(ES);                                      // SEG ES
DecodeSeg decodeSegCs(CS);                                      // SEG CS
DecodeSeg decodeSegSs(SS);                                      // SEG SS
DecodeSeg decodeSegDs(DS);                                      // SEG DS
DecodeSeg decodeSegFs(FS);                                      // SEG FS
DecodeSeg decodeSegGs(GS);                                      // SEG GS

Decode2Byte decode2Byte;
Decode66 decode66;
Decode67 decode67;
Decode266 decode266;
Decode267 decode267;

DecodeInst decodeDaa(Daa);                                      // DAA
DecodeInst decodeDas(Das);                                      // DAS
DecodeInst decodeAaa(Aaa);                                      // AAA
DecodeInst decodeAas(Aas);                                      // AAS
DecodeInst decodeAam(Aam, 8);                                   // AAM
DecodeInst decodeAad(Aad, 8);                                   // AAD

DecodeReg decodeIncAx(IncR16, 0);                                // INC AX
DecodeReg decodeIncCx(IncR16, 1);                                // INC CX
DecodeReg decodeIncDx(IncR16, 2);                                // INC DX
DecodeReg decodeIncBx(IncR16, 3);                                // INC BX
DecodeReg decodeIncSp(IncR16, 4);                                // INC SP
DecodeReg decodeIncBp(IncR16, 5);                                // INC BP
DecodeReg decodeIncSi(IncR16, 6);                                // INC SI
DecodeReg decodeIncDi(IncR16, 7);                                // INC DI

DecodeReg decodeDecAx(DecR16, 0);                                // DEC AX
DecodeReg decodeDecCx(DecR16, 1);                                // DEC CX
DecodeReg decodeDecDx(DecR16, 2);                                // DEC DX
DecodeReg decodeDecBx(DecR16, 3);                                // DEC BX
DecodeReg decodeDecSp(DecR16, 4);                                // DEC SP
DecodeReg decodeDecBp(DecR16, 5);                                // DEC BP
DecodeReg decodeDecSi(DecR16, 6);                                // DEC SI
DecodeReg decodeDecDi(DecR16, 7);                                // DEC DI

DecodeReg decodeIncEax(IncR32, 0);                                // INC EAX
DecodeReg decodeIncEcx(IncR32, 1);                                // INC ECX
DecodeReg decodeIncEdx(IncR32, 2);                                // INC EDX
DecodeReg decodeIncEbx(IncR32, 3);                                // INC EBX
DecodeReg decodeIncEsp(IncR32, 4);                                // INC ESP
DecodeReg decodeIncEbp(IncR32, 5);                                // INC EBP
DecodeReg decodeIncEsi(IncR32, 6);                                // INC ESI
DecodeReg decodeIncEdi(IncR32, 7);                                // INC EDI

DecodeReg decodeDecEax(DecR32, 0);                                // DEC EAX
DecodeReg decodeDecEcx(DecR32, 1);                                // DEC ECX
DecodeReg decodeDecEdx(DecR32, 2);                                // DEC EDX
DecodeReg decodeDecEbx(DecR32, 3);                                // DEC EBX
DecodeReg decodeDecEsp(DecR32, 4);                                // DEC ESP
DecodeReg decodeDecEbp(DecR32, 5);                                // DEC EBP
DecodeReg decodeDecEsi(DecR32, 6);                                // DEC ESI
DecodeReg decodeDecEdi(DecR32, 7);                                // DEC EDI

DecodeReg decodePushAx(PushR16, 0);                              // PUSH AX
DecodeReg decodePushCx(PushR16, 1);                              // PUSH CX
DecodeReg decodePushDx(PushR16, 2);                              // PUSH DX
DecodeReg decodePushBx(PushR16, 3);                              // PUSH BX
DecodeReg decodePushSp(PushR16, 4);                              // PUSH SP
DecodeReg decodePushBp(PushR16, 5);                              // PUSH BP
DecodeReg decodePushSi(PushR16, 6);                              // PUSH SI
DecodeReg decodePushDi(PushR16, 7);                              // PUSH DI

DecodeReg decodePopAx(PopR16, 0);                                // POP AX
DecodeReg decodePopCx(PopR16, 1);                                // POP CX
DecodeReg decodePopDx(PopR16, 2);                                // POP DX
DecodeReg decodePopBx(PopR16, 3);                                // POP BX
DecodeReg decodePopSp(PopR16, 4);                                // POP SP
DecodeReg decodePopBp(PopR16, 5);                                // POP BP
DecodeReg decodePopSi(PopR16, 6);                                // POP SI
DecodeReg decodePopDi(PopR16, 7);                                // POP DI

DecodeReg decodePushEax(PushR32, 0);                              // PUSH EAX
DecodeReg decodePushEcx(PushR32, 1);                              // PUSH ECX
DecodeReg decodePushEdx(PushR32, 2);                              // PUSH EDX
DecodeReg decodePushEbx(PushR32, 3);                              // PUSH EBX
DecodeReg decodePushEsp(PushR32, 4);                              // PUSH ESP
DecodeReg decodePushEbp(PushR32, 5);                              // PUSH EBP
DecodeReg decodePushEsi(PushR32, 6);                              // PUSH ESI
DecodeReg decodePushEdi(PushR32, 7);                              // PUSH EDI

DecodeReg decodePopEax(PopR32, 0);                                // POP EAX
DecodeReg decodePopEcx(PopR32, 1);                                // POP ECX
DecodeReg decodePopEdx(PopR32, 2);                                // POP EDX
DecodeReg decodePopEbx(PopR32, 3);                                // POP EBX
DecodeReg decodePopEsp(PopR32, 4);                                // POP ESP
DecodeReg decodePopEbp(PopR32, 5);                                // POP EBP
DecodeReg decodePopEsi(PopR32, 6);                                // POP ESI
DecodeReg decodePopEdi(PopR32, 7);                                // POP EDI

DecodeInst decodePushA16(PushA16);                               // PUSHA
DecodeInst decodePopA16(PopA16);                                 // POPA
DecodeInst decodePushA32(PushA32);                               // PUSHA
DecodeInst decodePopA32(PopA32);                                 // POPA

DecodeInst decodePushIw16(Push16, 16);                           // PUSH Iw
DecodeInst decodePushIb16(Push16, 8, 16);                        // PUSH Ib
DecodeInst decodePushId32(Push32, 32);                           // PUSH Id
DecodeInst decodePushIb32(Push32, 8, 32);                        // PUSH Ib

DecodeE decodePopEw(PopR16, PopE16);               // POP Ew
DecodeE decodePopEd(PopR32, PopE32);               // POP Ed

DecodeInst decodePushF16(PushF16);                               // PUSHF
DecodeInst decodePopF16(PopF16);                                 // POPF
DecodeInst decodePushF32(PushF32);                               // PUSHF
DecodeInst decodePopF32(PopF32);                                 // POPF

DecodeMem decodeBound16(Bound16);                 // BOUND
DecodeMem decodeBound32(Bound32);                 // BOUND
DecodeRM decodeArpl(ArplReg, ArplMem);                     // ARPL
DecodeRM decodeArpl32(ArplReg32, ArplMem32);                     // ARPL

DecodeRMr decodeImulGwEwIw(ImulR16R16, ImulR16E16, 16); // IMUL Gw,Ew,Iw
DecodeRMr decodeImulGwEwIb(ImulR16R16, ImulR16E16, 8, 16); // IMUL Gw,Ew,Ib
DecodeRMr decodeImulGdEdId(ImulR32R32, ImulR32E32, 32); // IMUL Gd,Ed,Id
DecodeRMr decodeImulGdEdIb(ImulR32R32, ImulR32E32, 8, 32); // IMUL Gd,Ed,Ib

DecodeMem decodeInsb(Insb);                          // INSB
DecodeMem decodeInsw(Insw);                          // INSW
DecodeMem decodeInsd(Insd);                          // INSD

DecodeMem decodeOutsb(Outsb);                       // OUTSB
DecodeMem decodeOutsw(Outsw);                       // OUTSW
DecodeMem decodeOutsd(Outsd);                       // OUTSD

DecodeInst decodeJo8(JumpO, 8, 32);                              // JO
DecodeInst decodeJno8(JumpNO, 8, 32);                            // JNO
DecodeInst decodeJb8(JumpB, 8, 32);                              // JB
DecodeInst decodeJnb8(JumpNB, 8, 32);                            // JNB
DecodeInst decodeJz8(JumpZ, 8, 32);                              // JZ
DecodeInst decodeJnz8(JumpNZ, 8, 32);                            // JNZ
DecodeInst decodeJbe8(JumpBE, 8, 32);                            // JBE
DecodeInst decodeJnbe8(JumpNBE, 8, 32);                          // JNBE
DecodeInst decodeJs8(JumpS, 8, 32);                              // JS
DecodeInst decodeJns8(JumpNS, 8, 32);                            // JNS
DecodeInst decodeJp8(JumpP, 8, 32);                              // JP
DecodeInst decodeJnp8(JumpNP, 8, 32);                            // JNP
DecodeInst decodeJl8(JumpL, 8, 32);                              // JL
DecodeInst decodeJnl8(JumpNL, 8, 32);                            // JNL
DecodeInst decodeJle8(JumpLE, 8, 32);                             // JLE
DecodeInst decodeJnle8(JumpNLE, 8, 32);                          // JNLE

DecodeInst decodeJo16(JumpO, 16, 32);                            // JO
DecodeInst decodeJno16(JumpNO, 16, 32);                          // JNO
DecodeInst decodeJb16(JumpB, 16, 32);                            // JB
DecodeInst decodeJnb16(JumpNB, 16, 32);                          // JNB
DecodeInst decodeJz16(JumpZ, 16, 32);                            // JZ
DecodeInst decodeJnz16(JumpNZ, 16, 32);                          // JNZ
DecodeInst decodeJbe16(JumpBE, 16, 32);                          // JBE
DecodeInst decodeJnbe16(JumpNBE, 16, 32);                        // JNBE
DecodeInst decodeJs16(JumpS, 16, 32);                            // JS
DecodeInst decodeJns16(JumpNS, 16, 32);                          // JNS
DecodeInst decodeJp16(JumpP, 16, 32);                            // JP
DecodeInst decodeJnp16(JumpNP, 16, 32);                          // JNP
DecodeInst decodeJl16(JumpL, 16, 32);                            // JL
DecodeInst decodeJnl16(JumpNL, 16, 32);                          // JNL
DecodeInst decodeJle16(JumpLE, 16, 32);                           // JLE
DecodeInst decodeJnle16(JumpNLE, 16, 32);                        // JNLE

DecodeInst decodeJo32(JumpO, 32);                            // JO
DecodeInst decodeJno32(JumpNO, 32);                          // JNO
DecodeInst decodeJb32(JumpB, 32);                            // JB
DecodeInst decodeJnb32(JumpNB, 32);                          // JNB
DecodeInst decodeJz32(JumpZ, 32);                            // JZ
DecodeInst decodeJnz32(JumpNZ, 32);                          // JNZ
DecodeInst decodeJbe32(JumpBE, 32);                          // JBE
DecodeInst decodeJnbe32(JumpNBE, 32);                        // JNBE
DecodeInst decodeJs32(JumpS, 32);                            // JS
DecodeInst decodeJns32(JumpNS, 32);                          // JNS
DecodeInst decodeJp32(JumpP, 32);                            // JP
DecodeInst decodeJnp32(JumpNP, 32);                          // JNP
DecodeInst decodeJl32(JumpL, 32);                            // JL
DecodeInst decodeJnl32(JumpNL, 32);                          // JNL
DecodeInst decodeJle32(JumpLE, 32);                           // JLE
DecodeInst decodeJnle32(JumpNLE, 32);                        // JNLE

DecodeGrp1_8 decodeGroup1_8_Ib(8);                               // Grpl Eb,Ib
DecodeGrp1_16 decodeGroup1_16_Iw(16);                            // Grpl Ew,Iw
DecodeGrp1_32 decodeGroup1_32_Id(32);                            // Grpl Ed,Id
DecodeGrp1_16 decodeGroup1_16_Ix(8, 16);                         // Grpl Ew,Ix
DecodeGrp1_32 decodeGroup1_32_Ix(8, 32);                         // Grpl Ed,Ix
DecodeGrp2_8 decodeGroup2_8_1(0, 1);                             // GRP2 Eb,1
DecodeGrp2_16 decodeGroup2_16_1(0, 1);                           // GRP2 Ew,1
DecodeGrp2_32 decodeGroup2_32_1(0, 1);                           // GRP2 Ed,1
DecodeGrp2_8 decodeGroup2_8_Ib(8, 0);                            // GRP2 Eb,Ib
DecodeGrp2_16 decodeGroup2_16_Ib(8, 0);                          // GRP2 Ew,Ib
DecodeGrp2_32 decodeGroup2_32_Ib(8, 0);                          // GRP2 Ed,Ib
DecodeGrp2_Cl_8 decodeGroup2_8_Cl;                            // GRP2 Eb,Cl
DecodeGrp2_Cl_16 decodeGroup2_16_Cl;                          // GRP2 Ew,Cl
DecodeGrp2_Cl_32 decodeGroup2_32_Cl;                          // GRP2 Ed,Cl
DecodeGrp3_8 decodeGroup3_8;                                     // GRP3 Eb(,Ib)
DecodeGrp3_16 decodeGroup3_16;                                   // GRP3 Ew(,Iw)
DecodeGrp3_32 decodeGroup3_32;                                   // GRP3 Ed(,Id)
DecodeGrp4_8 decodeGroup4_8;                                     // GRP4 Eb
DecodeGrp5_16 decodeGroup5_16;                                   // GRP5 Ew
DecodeGrp5_32 decodeGroup5_32;                                   // GRP5 Ed
DecodeGrp6_16 decodeGroup6_16;                                   // GRP6
DecodeGrp6_32 decodeGroup6_32;                                   // GRP6
DecodeGrp7_32 decodeGroup7_32;                                   // GRP7
DecodeGrp8_16 decodeGroup8_16;
DecodeGrp8_32 decodeGroup8_32;

DecodeRM decodeTestEbGb(TestR8R8, TestE8R8);     // TEST Eb,Gb
DecodeRM decodeTestEwGw(TestR16R16, TestE16R16);// TEST Ew,Gw
DecodeRM decodeTestEdGd(TestR32R32, TestE32R32);// TEST Ed,Gd
DecodeReg decodeTestAlIb(TestR8I8, 0, 8);                        // TEST AL,Ib
DecodeReg decodeTestAxIw(TestR16I16, 0, 16);                     // TEST AX,Iw
DecodeReg decodeTestEaxId(TestR32I32, 0, 32);                     // TEST EAX,Id

DecodeRM decodeXchgEbGb(XchgR8R8, XchgE8R8);     // XCHG Eb,Gb
DecodeRM decodeXchgEwGw(XchgR16R16, XchgE16R16);     // XCHG Ew,Gw
DecodeRM decodeXchgEdGd(XchgR32R32, XchgE32R32);     // XCHG Ed,Gd
DecodeReg2 decodeXchgCxAx(XchgR16R16, 0, 1);                       // XCHG CX,AX
DecodeReg2 decodeXchgDxAx(XchgR16R16, 0, 2);                       // XCHG DX,AX
DecodeReg2 decodeXchgBxAx(XchgR16R16, 0, 3);                       // XCHG BX,AX
DecodeReg2 decodeXchgSpAx(XchgR16R16, 0, 4);                       // XCHG SP,AX
DecodeReg2 decodeXchgBpAx(XchgR16R16, 0, 5);                       // XCHG BP,AX
DecodeReg2 decodeXchgSiAx(XchgR16R16, 0, 6);                       // XCHG SI,AX
DecodeReg2 decodeXchgDiAx(XchgR16R16, 0, 7);                       // XCHG DI,AX
DecodeReg2 decodeXchgEcxEax(XchgR32R32, 0, 1);                       // XCHG ECX,EAX
DecodeReg2 decodeXchgEdxEax(XchgR32R32, 0, 2);                       // XCHG EDX,EAX
DecodeReg2 decodeXchgEbxEax(XchgR32R32, 0, 3);                       // XCHG EBX,EAX
DecodeReg2 decodeXchgEspEax(XchgR32R32, 0, 4);                       // XCHG ESP,EAX
DecodeReg2 decodeXchgEbpEax(XchgR32R32, 0, 5);                       // XCHG EBP,EAX
DecodeReg2 decodeXchgEsiEax(XchgR32R32, 0, 6);                       // XCHG ESI,EAX
DecodeReg2 decodeXchgEdiEax(XchgR32R32, 0, 7);                       // XCHG EDI,EAX

DecodeRM decodeBtEwGw(BtR16R16, BtE16R16);                       // BT Ew,Gw
DecodeRM decodeBtEdGd(BtR32R32, BtE32R32);                       // BT Ed,Gd
DecodeRM decodeBtsEwGw(BtsR16R16, BtsE16R16);                    // BTS Ew,Gw
DecodeRM decodeBtsEdGd(BtsR32R32, BtsE32R32);                    // BTS Ed,Gd
DecodeRM decodeBtrEwGw(BtrR16R16, BtrE16R16);                    // BTR Ew,Gw
DecodeRM decodeBtrEdGd(BtrR32R32, BtrE32R32);                    // BTR Ed,Gd
DecodeRMr decodeBsfGwEw(BsfR16R16, BsfR16E16);                   // BSF Gw,Ew
DecodeRMr decodeBsfGdEd(BsfR32R32, BsfR32E32);                   // BSF Gd,Ed
DecodeRMr decodeBsrGwEw(BsrR16R16, BsrR16E16);                   // BSR Gw,Ew
DecodeRMr decodeBsrGdEd(BsrR32R32, BsrR32E32);                   // BSR Gd,Ed
DecodeRM decodeBtcEwGw(BtcR16R16, BtcE16R16);                    // BTC Ew,Gw
DecodeRM decodeBtcEdGd(BtcR32R32, BtcE32R32);                    // BTC Ed,Gd

DecodeMaskRM decodeDshlEwGw(DshlR16R16, DshlE16R16, 8, 0x1f);    // DSHL Ew,Gw
DecodeRM decodeDshlClEwGw(DshlClR16R16, DshlClE16R16);             // DSHLCL Ew,Gw
DecodeMaskRM decodeDshrEwGw(DshrR16R16, DshrE16R16, 8, 0x1f);    // DSHR Ew,Gw
DecodeRM decodeDshrClEwGw(DshrClR16R16, DshrClE16R16);             // DSHRCL Ew,Gw

DecodeMaskRM decodeDshlEdGd(DshlR32R32, DshlE32R32, 8, 0x1f);    // DSHL Ed,Gd
DecodeRM decodeDshlClEdGd(DshlClR32R32, DshlClE32R32);             // DSHLCL Ed,Gd
DecodeMaskRM decodeDshrEdGd(DshrR32R32, DshrE32R32, 8, 0x1f);    // DSHR Ed,Gd
DecodeRM decodeDshrClEdGd(DshrClR32R32, DshrClE32R32);             // DSHRCL Ed,Gd

DecodeRMr decodeDimulGwEw(DimulR16R16, DimulR16E16);              // DIMUL Gw,Ew
DecodeRMr decodeDimulGdEd(DimulR32R32, DimulR32E32);              // DIMUL Gd,Ed
DecodeRM decodeCmpXchgEbGb(CmpXchgR8R8, CmpXchgE8R8);        // CMPXCHG Eb,Gb
DecodeRM decodeCmpXchgEwGw(CmpXchgR16R16, CmpXchgE16R16);        // CMPXCHG Ew,Gw
DecodeRM decodeCmpXchgEdGd(CmpXchgR32R32, CmpXchgE32R32);        // CMPXCHG Ed,Gd

DecodeRM decodeMovEbGb(MovR8R8, MovE8R8);         // MOV Eb,Gb
DecodeRM decodeMovEwGw(MovR16R16, MovE16R16);   // MOV Ew,Gw
DecodeRM decodeMovEdGd(MovR32R32, MovE32R32);   // MOV Ed,Gd
DecodeRMr decodeMovGbEb(MovR8R8, MovR8E8);        // MOV Gb,Eb
DecodeRMr decodeMovGwEw(MovR16R16, MovR16E16);  // MOV Gw,Ew
DecodeRMr decodeMovGdEd(MovR32R32, MovR32E32);  // MOV Gd,Ed

DecodeReg decodeMovAlIb(MovR8I8, 0, 8);                          // MOV AL,Ib
DecodeReg decodeMovClIb(MovR8I8, 1, 8);                          // MOV CL,Ib
DecodeReg decodeMovDlIb(MovR8I8, 2, 8);                          // MOV DL,Ib
DecodeReg decodeMovBlIb(MovR8I8, 3, 8);                          // MOV BL,Ib
DecodeReg decodeMovAhIb(MovR8I8, 4, 8);                          // MOV AH,Ib
DecodeReg decodeMovChIb(MovR8I8, 5, 8);                          // MOV CH,Ib
DecodeReg decodeMovDhIb(MovR8I8, 6, 8);                          // MOV DH,Ib
DecodeReg decodeMovBhIb(MovR8I8, 7, 8);                          // MOV BH,Ib

DecodeReg decodeMovAxIw(MovR16I16, 0, 16);                       // MOV AX,Iw
DecodeReg decodeMovCxIw(MovR16I16, 1, 16);                       // MOV CX,Iw
DecodeReg decodeMovDxIw(MovR16I16, 2, 16);                       // MOV DX,Iw
DecodeReg decodeMovBxIw(MovR16I16, 3, 16);                       // MOV BX,Iw
DecodeReg decodeMovSpIw(MovR16I16, 4, 16);                       // MOV SP,Iw
DecodeReg decodeMovBpIw(MovR16I16, 5, 16);                       // MOV BP,Iw
DecodeReg decodeMovSiIw(MovR16I16, 6, 16);                       // MOV SI,Iw
DecodeReg decodeMovDiIw(MovR16I16, 7, 16);                       // MOV DI,Iw

DecodeReg decodeMovEaxId(MovR32I32, 0, 32);                       // MOV EAX,Id
DecodeReg decodeMovEcxId(MovR32I32, 1, 32);                       // MOV ECX,Id
DecodeReg decodeMovEdxId(MovR32I32, 2, 32);                       // MOV EDX,Id
DecodeReg decodeMovEbxId(MovR32I32, 3, 32);                       // MOV EBX,Id
DecodeReg decodeMovEspId(MovR32I32, 4, 32);                       // MOV ESP,Id
DecodeReg decodeMovEbpId(MovR32I32, 5, 32);                       // MOV EBP,Id
DecodeReg decodeMovEsiId(MovR32I32, 6, 32);                       // MOV ESI,Id
DecodeReg decodeMovEdiId(MovR32I32, 7, 32);                       // MOV eEDI,Id

DecodeRM decodeMovEbIb(MovR8I8, MovE8I8, 8);         // MOV EB,IB
DecodeRM decodeMovEwIw(MovR16I16, MovE16I16, 16);   // MOV EW,IW
DecodeRM decodeMovEdId(MovR32I32, MovE32I32, 32);   // MOV ED,ID

DecodeRM decodeMovEwSw(MovR16S16, MovE16S16);   // Mov Ew,Sw
DecodeRM decodeMovSwEw(MovS16R16, MovS16E16);   // MOV Sw,Ew
DecodeRM decodeMovEdSw(MovR32S16, MovE16S16);   // Mov Ed,Sw

DecodeDirect decodeMovAlOb(MovAlOb);                             // MOV AL,Ob
DecodeDirect decodeMovAxOw(MovAxOw);                             // MOV AX,Ow
DecodeDirect decodeMovEaxOd(MovEaxOd);                             // MOV EAX,Od
DecodeDirect decodeMovObAl(MovObAl);                             // MOV Ob,AL
DecodeDirect decodeMovOwAx(MovOwAx);                             // MOV Ow,AX
DecodeDirect decodeMovOdEax(MovOdEax);                             // MOV Od,EAX

DecodeRMr decodeMovGwXz8(MovGwXzR8, MovGwXzE8);                        // MOVXZ8 Gw,Ew
DecodeRMr decodeMovGwSx8(MovGwSxR8, MovGwSxE8);                        // MOVSX8 Gw,Ew
DecodeRMr decodeMovGdXz8(MovGdXzR8, MovGdXzE8);                        // MOVXZ8 Gd,Ed
DecodeRMr decodeMovGdSx8(MovGdSxR8, MovGdSxE8);                        // MOVSX8 Gd,Ed
DecodeRMr decodeMovGdXz16(MovGdXzR16, MovGdXzE16);                        // MOVXZ16 Gd,Ed
DecodeRMr decodeMovGdSx16(MovGdSxR16, MovGdSxE16);                        // MOVSX16 Gd,Ed

DecodeRM decodeMovRdCrx(MovRdCRx, Invalid);
DecodeRM decodeMovCrxRd(MovCRxRd, Invalid);

DecodeLea decodeLeaGw(LeaR16);               // LEA Gw
DecodeLea decodeLeaGd(LeaR32);               // LEA Gd

DecodeRM decodeHintNop(Nop, Nop);
DecodeZeroInst decodeNop(Nop, None, Pause);
DecodeReg2 decodeCbw(MovGwSxR8, 0, 0);                           // CBW
DecodeReg2 decodeCwde(MovGdSxR16, 0, 0);                         // CWDE
DecodeInst decodeCwd(Cwd);                                       // CWD
DecodeInst decodeCwq(Cwq);                                       // CWQ
DecodeIwIw decodeCallAp(CallAp);                                 // CALL Ap 
DecodeIdIw decodeCallFar(CallFar);                               // CALL Far 
DecodeIwIw decodeJmpAp(JmpAp);                                   // JMP Ap 
DecodeIdIw decodeJmpFar(JmpFar);                                   // JMP Far 
DecodeInst decodeWait(Wait);                                     // Wait
DecodeInst decodeSahf(Sahf);                                     // SAHF
DecodeInst decodeLahf(Lahf);                                     // LAHF
DecodeInst decodeSalc(Salc);                                     // SALC
DecodeInst decodeRetn16Iw(Retn16Iw, 16);                             // RETN Iw
DecodeInst decodeRetn32Iw(Retn32Iw, 16);                             // RETN Iw
DecodeInst decodeRetn16(Retn16);                                 // RETN
DecodeInst decodeRetn32(Retn32);                                 // RETN
DecodeInst decodeRetfIw16(Retf16, 16);                         // RETF Iw
DecodeInst decodeRetfIw32(Retf32, 16);                         // RETF Iw
DecodeInst decodeRetf16(Retf16);                                 // RETF
DecodeInst decodeRetf32(Retf32);                                 // RETF
DecodeEnter decodeEnter16(Enter16);                              // ENTER
DecodeEnter decodeEnter32(Enter32);                              // ENTER
DecodeInst decodeLeave16(Leave16);                               // LEAVE
DecodeInst decodeLeave32(Leave32);                               // LEAVE
DecodeInst decodeInt3(Int3);                                     // INT 3
DecodeIntIb decodeIntIb;                                         // INT Ib
DecodeInst decodeIntO(IntO);                                     // INTO
DecodeInst decodeIret(Iret);                                     // IRET
DecodeInst decodeIret32(Iret32);                                 // IRET
DecodeInstrMem decodeXlat(Xlat);                     // XLAT
DecodeInst decodeICEBP(ICEBP);                                   // ICEBP
DecodeInst decodeHlt(Hlt);                                       // HLT
DecodeInst decodeCmc(Cmc);                                       // CMC
DecodeInst decodeClc(Clc);                                       // CLC
DecodeInst decodeStc(Stc);                                       // STC
DecodeInst decodeCli(Cli);                                       // CLI
DecodeInst decodeSti(Sti);                                       // STI
DecodeInst decodeCld(Cld);                                       // CLD
DecodeInst decodeStd(Std);                                       // STD
DecodeRdtsc decodeRdtsc(Rdtsc);                                   // RDTSC
DecodeInst decodeCPUID(CPUID);                                   // CPUID

DecodeInstrMem decodeMovsb(Movsb);                  // MOVSB
DecodeInstrMem decodeMovsw(Movsw);                  // MOVSW
DecodeInstrMem decodeMovsd(Movsd);                  // MOVSD
DecodeInstrMem decodeCmpsb(Cmpsb);                  // CMPSB
DecodeInstrMem decodeCmpsw(Cmpsw);                  // CMPSW
DecodeInstrMem decodeCmpsd(Cmpsd);                  // CMPSD
DecodeInstrMem decodeStosb(Stosb);                  // STOSB
DecodeInstrMem decodeStosw(Stosw);                  // STOSW
DecodeInstrMem decodeStosd(Stosd);                  // STOSD
DecodeInstrMem decodeLodsb(Lodsb);                  // LODSB
DecodeInstrMem decodeLodsw(Lodsw);                  // LODSW
DecodeInstrMem decodeLodsd(Lodsd);                  // LODSD
DecodeInstrMem decodeScasb(Scasb);                  // SCASB
DecodeInstrMem decodeScasw(Scasw);                  // SCASW
DecodeInstrMem decodeScasd(Scasd);                  // SCASD

DecodeRMImm decodeLes(Invalid, LoadSegment16, ES);                     // LES
DecodeRMImm decodeLds(Invalid, LoadSegment16, DS);                     // LDS
DecodeRMImm decodeLss(Invalid, LoadSegment16, SS);                     // LSS
DecodeRMImm decodeLfs(Invalid, LoadSegment16, FS);                     // LFS
DecodeRMImm decodeLgs(Invalid, LoadSegment16, GS);                     // LGS

DecodeRMImm decodeLes32(Invalid, LoadSegment32, ES);                     // LES
DecodeRMImm decodeLds32(Invalid, LoadSegment32, DS);                     // LDS
DecodeRMImm decodeLss32(Invalid, LoadSegment32, SS);                     // LSS
DecodeRMImm decodeLfs32(Invalid, LoadSegment32, FS);                     // LFS
DecodeRMImm decodeLgs32(Invalid, LoadSegment32, GS);                     // LGS

DecodeFPU0 decodeFpu0;
DecodeFPU1 decodeFpu1;
DecodeFPU2 decodeFpu2;
DecodeFPU3 decodeFpu3;
DecodeFPU4 decodeFpu4;
DecodeFPU5 decodeFpu5;
DecodeFPU6 decodeFpu6;
DecodeFPU7 decodeFpu7;

DecodeInst decodeLoopNZ(LoopNZ, 8, 32);                 // LOOPNZ
DecodeInst decodeLoopZ(LoopZ, 8, 32);                    // LOOPZ
DecodeInst decodeLoop(Loop, 8, 32);                       // LOOP
DecodeInst decodeJcxz(Jcxz, 8, 32);                       // JCXZ

DecodeInst decodeInAlIb(InAlIb, 8);                              // IN AL,Ib
DecodeInst decodeInAxIb(InAxIb, 8);                              // IN AX,Ib
DecodeInst decodeInEaxIb(InEaxIb, 8);                              // IN EAX,Ib
DecodeInst decodeOutIbAl(OutIbAl, 8);                            // OUT Ib,AL
DecodeInst decodeOutIbAx(OutIbAx, 8);                            // OUT Ib,AX
DecodeInst decodeOutIbEax(OutIbEax, 8);                            // OUT Ib,EAX
DecodeInst decodeInAlDx(InAlDx);                                 // IN AL,Dx
DecodeInst decodeInAxDx(InAxDx);                                 // IN AX,Dx
DecodeInst decodeInEaxDx(InEaxDx);                                 // IN EAX,Dx
DecodeInst decodeOutDxAl(OutDxAl);                               // OUT Dx,AL
DecodeInst decodeOutDxAx(OutDxAx);                               // OUT Dx,AX
DecodeInst decodeOutDxEax(OutDxEax);                               // OUT Dx,EAX

DecodeInst decodeCallJw(CallJw, 16);                             // CALL Jw 
DecodeInst decodeCallJd(CallJd, 32);                             // CALL Jd 
DecodeInst decodeJmpJd(JmpJd, 32);                               // JMP Jd 
DecodeInst decodeJmpJw(JmpJw, 16);                               // JMP Jw 
DecodeInst decodeJmpJb(JmpJb, 8);                                // JMP Jb 

DecodeLock decodeLock;                                           // LOCK
DecodeRepNZ decodeRepNZ;                                         // REPNZ
DecodeRepZ decodeRepZ;                                           // REPZ

DecodeRMr decodeLar16(LarR16R16, LarR16E16);                      // LAR
DecodeRMr decodeLsl16(LslR16R16, LslR16E16);                      // LSL
DecodeRMr decodeLsl32(LslR32R32, LslR32E32);                      // LSL

DecodeRMr decodeCmovO_16(CmovO_R16R16, CmovO_R16E16);             // CMOVO
DecodeRMr decodeCmovNO_16(CmovNO_R16R16, CmovNO_R16E16);          // CMOVNO
DecodeRMr decodeCmovB_16(CmovB_R16R16, CmovB_R16E16);             // CMOVB
DecodeRMr decodeCmovNB_16(CmovNB_R16R16, CmovNB_R16E16);          // CMOVNB
DecodeRMr decodeCmovZ_16(CmovZ_R16R16, CmovZ_R16E16);             // CMOVZ
DecodeRMr decodeCmovNZ_16(CmovNZ_R16R16, CmovNZ_R16E16);          // CMOVNZ
DecodeRMr decodeCmovBE_16(CmovBE_R16R16, CmovBE_R16E16);          // CMOVBE
DecodeRMr decodeCmovNBE_16(CmovNBE_R16R16, CmovNBE_R16E16);       // CMOVNBE
DecodeRMr decodeCmovS_16(CmovS_R16R16, CmovS_R16E16);             // CMOVS
DecodeRMr decodeCmovNS_16(CmovNS_R16R16, CmovNS_R16E16);          // CMOVNS
DecodeRMr decodeCmovP_16(CmovP_R16R16, CmovP_R16E16);             // CMOVP
DecodeRMr decodeCmovNP_16(CmovNP_R16R16, CmovNP_R16E16);          // CMOVNP
DecodeRMr decodeCmovL_16(CmovL_R16R16, CmovL_R16E16);             // CMOVL
DecodeRMr decodeCmovNL_16(CmovNL_R16R16, CmovNL_R16E16);          // CMOVNL
DecodeRMr decodeCmovLE_16(CmovLE_R16R16, CmovLE_R16E16);          // CMOVLE
DecodeRMr decodeCmovNLE_16(CmovNLE_R16R16, CmovNLE_R16E16);       // CMOVNLE

DecodeRMr decodeCmovO_32(CmovO_R32R32, CmovO_R32E32);             // CMOVO
DecodeRMr decodeCmovNO_32(CmovNO_R32R32, CmovNO_R32E32);          // CMOVNO
DecodeRMr decodeCmovB_32(CmovB_R32R32, CmovB_R32E32);             // CMOVB
DecodeRMr decodeCmovNB_32(CmovNB_R32R32, CmovNB_R32E32);          // CMOVNB
DecodeRMr decodeCmovZ_32(CmovZ_R32R32, CmovZ_R32E32);             // CMOVZ
DecodeRMr decodeCmovNZ_32(CmovNZ_R32R32, CmovNZ_R32E32);          // CMOVNZ
DecodeRMr decodeCmovBE_32(CmovBE_R32R32, CmovBE_R32E32);          // CMOVBE
DecodeRMr decodeCmovNBE_32(CmovNBE_R32R32, CmovNBE_R32E32);       // CMOVNBE
DecodeRMr decodeCmovS_32(CmovS_R32R32, CmovS_R32E32);             // CMOVS
DecodeRMr decodeCmovNS_32(CmovNS_R32R32, CmovNS_R32E32);          // CMOVNS
DecodeRMr decodeCmovP_32(CmovP_R32R32, CmovP_R32E32);             // CMOVP
DecodeRMr decodeCmovNP_32(CmovNP_R32R32, CmovNP_R32E32);          // CMOVNP
DecodeRMr decodeCmovL_32(CmovL_R32R32, CmovL_R32E32);             // CMOVL
DecodeRMr decodeCmovNL_32(CmovNL_R32R32, CmovNL_R32E32);          // CMOVNL
DecodeRMr decodeCmovLE_32(CmovLE_R32R32, CmovLE_R32E32);          // CMOVLE
DecodeRMr decodeCmovNLE_32(CmovNLE_R32R32, CmovNLE_R32E32);       // CMOVNLE

DecodeRM decodeSetO(SetO_R8, SetO_E8);                           // SETO
DecodeRM decodeSetNO(SetNO_R8, SetNO_E8);                        // SETNO
DecodeRM decodeSetB(SetB_R8, SetB_E8);                           // SETB
DecodeRM decodeSetNB(SetNB_R8, SetNB_E8);                        // SETNB
DecodeRM decodeSetZ(SetZ_R8, SetZ_E8);                           // SETZ
DecodeRM decodeSetNZ(SetNZ_R8, SetNZ_E8);                        // SETNZ
DecodeRM decodeSetBE(SetBE_R8, SetBE_E8);                        // SETBE
DecodeRM decodeSetNBE(SetNBE_R8, SetNBE_E8);                     // SETNBE
DecodeRM decodeSetS(SetS_R8, SetS_E8);                           // SETS
DecodeRM decodeSetNS(SetNS_R8, SetNS_E8);                        // SETNS
DecodeRM decodeSetP(SetP_R8, SetP_E8);                           // SETP
DecodeRM decodeSetNP(SetNP_R8, SetNP_E8);                        // SETNP
DecodeRM decodeSetL(SetL_R8, SetL_E8);                           // SETL
DecodeRM decodeSetNL(SetNL_R8, SetNL_E8);                        // SETNL
DecodeRM decodeSetLE(SetLE_R8, SetLE_E8);                        // SETLE
DecodeRM decodeSetNLE(SetNLE_R8, SetNLE_E8);                     // SETNLE

DecodeRMr decodeXadd8(XaddR8R8, XaddR8E8); // XADD
DecodeRMr decodeXadd16(XaddR16R16, XaddR16E16); // XADD
DecodeRMr decodeXadd32(XaddR32R32, XaddR32E32); // XADD
DecodeGroup decodeCmpXchg8b(Invalid, CmpXchg8b, Invalid, Invalid, Invalid, Invalid, Invalid, Invalid); // CMPXCHG8B

DecodeReg decodeBswapEAX(Bswap32, 0);
DecodeReg decodeBswapECX(Bswap32, 1);
DecodeReg decodeBswapEDX(Bswap32, 2);
DecodeReg decodeBswapEBX(Bswap32, 3);
DecodeReg decodeBswapESP(Bswap32, 4);
DecodeReg decodeBswapEBP(Bswap32, 5);
DecodeReg decodeBswapESI(Bswap32, 6);
DecodeReg decodeBswapEDI(Bswap32, 7);

// for mmx, E (mem or reg) is a source, not a dest for most instructions, which is why DecodeRMr is used
DecodeRMr decodePunpcklbw(PunpcklbwMmx, PunpcklbwE64);
DecodeRMr decodePunpcklwd(PunpcklwdMmx, PunpcklwdE64);
DecodeRMr decodePunpckldq(PunpckldqMmx, PunpckldqE64);
DecodeRMr decodePacksswb(PacksswbMmx, PacksswbE64);
DecodeRMr decodePcmpgtb(PcmpgtbMmx, PcmpgtbE64);
DecodeRMr decodePcmpgtw(PcmpgtwMmx, PcmpgtwE64);
DecodeRMr decodePcmpgtd(PcmpgtdMmx, PcmpgtdE64);
DecodeRMr decodePackuswb(PackuswbMmx, PackuswbE64);
DecodeRMr decodePunpckhbw(PunpckhbwMmx, PunpckhbwE64);
DecodeRMr decodePunpckhwd(PunpckhwdMmx, PunpckhwdE64);
DecodeRMr decodePunpckhdq(PunpckhdqMmx, PunpckhdqE64);
DecodeRMr decodePackssdw(PackssdwMmx, PackssdwE64);
DecodeRMr decodeMovPqEd(MovPqR32, MovPqE32);
DecodeSSE2 decodeMovPqQq(MovPqMmx, MovPqE64, Invalid, Invalid, MovdquXmmXmm, MovdquXmmE128);
DecodeGroup decode371MMX(Invalid, Invalid, Psrlw, Invalid, Psraw, Invalid, Psllw, Invalid, 8);
DecodeGroup decode372MMX(Invalid, Invalid, Psrld, Invalid, Psrad, Invalid, Pslld, Invalid, 8);
DecodeGroup decode373MMX(Invalid, Invalid, Psrlq, Invalid, Invalid, Invalid, Psllq, Invalid, 8);
DecodeRMr decodePcmpeqb(PcmpeqbMmx, PcmpeqbE64);
DecodeRMr decodePcmpeqw(PcmpeqwMmx, PcmpeqwE64);
DecodeRMr decodePcmpeqd(PcmpeqdMmx, PcmpeqdE64);
DecodeInst decodeEmms(Emms);
DecodeSSE2r decodeMovQqPq(MovPqMmx, MovE64Pq, Invalid, Invalid, MovdquXmmXmm, MovdquE128Xmm);
DecodeRMr decodePsrlw(PsrlwMmx, PsrlwE64);
DecodeRMr decodePsrld(PsrldMmx, PsrldE64);
DecodeRMr decodePsrlq(PsrlqMmx, PsrlqE64);
DecodeRMr decodePmullw(PmullwMmx, PmullwE64);
DecodeRMr decodePsubusb(PsubusbMmx, PsubusbE64);
DecodeRMr decodePsubusw(PsubuswMmx, PsubuswE64);
DecodeRMr decodePand(PandMmx, PandE64);
DecodeRMr decodePaddusb(PaddusbMmx, PaddusbE64);
DecodeRMr decodePaddusw(PadduswMmx, PadduswE64);
DecodeRMr decodePandn(PandnMmx, PandnE64);
DecodeRMr decodePsraw(PsrawMmx, PsrawE64);
DecodeRMr decodePsrad(PsradMmx, PsradE64);
DecodeRMr decodePmulhw(PmulhwMmx, PmulhwE64);
DecodeRMr decodePsubsb(PsubsbMmx, PsubsbE64);
DecodeRMr decodePsubsw(PsubswMmx, PsubswE64);
DecodeRMr decodePor(PorMmx, PorE64);
DecodeRMr decodePaddsb(PaddsbMmx, PaddsbE64);
DecodeRMr decodePaddsw(PaddswMmx, PaddswE64);
DecodeRMr decodePxor(PxorMmx, PxorE64);
DecodeRMr decodePsllw(PsllwMmx, PsllwE64);
DecodeRMr decodePslld(PslldMmx, PslldE64);
DecodeRMr decodePsllq(PsllqMmx, PsllqE64);
DecodeRMr decodePmaddwd(PmaddwdMmx, PmaddwdE64);
DecodeRMr decodePsubb(PsubbMmx, PsubbE64);
DecodeRMr decodePsubw(PsubwMmx, PsubwE64);
DecodeRMr decodePsubd(PsubdMmx, PsubdE64);
DecodeRMr decodePaddb(PaddbMmx, PaddbE64);
DecodeRMr decodePaddw(PaddwMmx, PaddwE64);
DecodeRMr decodePaddd(PadddMmx, PadddE64);

Decode3AE decode3EA;

DecodeSSE2 sseAdd(AddpsXmm, AddpsE128, AddsdXmmXmm, AddsdXmmE64, AddssXmm, AddssE32);
DecodeSSE2 sseSub(SubpsXmm, SubpsE128, SubsdXmmXmm, SubsdXmmE64, SubssXmm, SubssE32);
DecodeSSE2 sseMul(MulpsXmm, MulpsE128, MulsdXmmXmm, MulsdXmmE64, MulssXmm, MulssE32);
DecodeSSE2 sseDiv(DivpsXmm, DivpsE128, DivsdXmmXmm, DivsdXmmE64, DivssXmm, DivssE32);
DecodeSSE sseRpc(RcppsXmm, RcppsE128, RcpssXmm, RcpssE32);
DecodeSSE2 sseSqrt(SqrtpsXmm, SqrtpsE128, SqrtsdXmmXmm, SqrtsdXmmE64, SqrtssXmm, SqrtssE32);
DecodeSSE sseRsqrt(RsqrtpsXmm, RsqrtpsE128, RsqrtssXmm, RsqrtssE32);
DecodeSSE2 sseMin(MinpsXmm, MinpsE128, MinsdXmmXmm, MinsdXmmE64, MinssXmm, MinssE32);
DecodeSSE2 sseMax(MaxpsXmm, MaxpsE128, MaxsdXmmXmm, MaxsdXmmE64, MaxssXmm, MaxssE32);
DecodeRMr ssePavgbMmx(PavgbMmxMmx, PavgbMmxE64);
DecodeRMr ssePavgbXmm(PavgbXmmXmm, PavgbXmmE128);
DecodeRMr ssePavgwMmx(PavgwMmxMmx, PavgwMmxE64);
DecodeRMr ssePavgwXmm(PavgwXmmXmm, PavgwXmmE128);
DecodeRMr ssePsadbwMmx(PsadbwMmxMmx, PsadbwMmxE64);
DecodeRMr ssePsadbwXmm(PsadbwXmmXmm, PsadbwXmmE128);
DecodeRMr ssePextrwMmx(PextrwR32Mmx, PextrwE16Mmx, 8);
DecodeRMr ssePextrwXmm(PextrwR32Xmm, PextrwE16Xmm, 8);
DecodeRMr ssePinsrwMmx(PinsrwMmxR32, PinsrwMmxE16, 8);
DecodeRMr ssePinsrwXmm(PinsrwXmmR32, PinsrwXmmE16, 8);
DecodeRMr ssePmaxswMmx(PmaxswMmxMmx, PmaxswMmxE64);
DecodeRMr ssePmaxswXmm(PmaxswXmmXmm, PmaxswXmmE128);
DecodeRMr ssePmaxubMmx(PmaxubMmxMmx, PmaxubMmxE64);
DecodeRMr ssePmaxubXmm(PmaxubXmmXmm, PmaxubXmmE128);
DecodeRMr ssePminswMmx(PminswMmxMmx, PminswMmxE64);
DecodeRMr ssePminswXmm(PminswXmmXmm, PminswXmmE128);
DecodeRMr ssePminubMmx(PminubMmxMmx, PminubMmxE64);
DecodeRMr ssePminubXmm(PminubXmmXmm, PminubXmmE128);
DecodeRMr ssePmovmskbMmx(PmovmskbR32Mmx, Invalid);
DecodeRMr ssePmovmskbXmm(PmovmskbR32Xmm, Invalid);
DecodeRMr ssePmulhuwMmx(PmulhuwMmxMmx, PmulhuwMmxE64);
DecodeRMr ssePmulhuwXmm(PmulhuwXmmXmm, PmulhuwXmmE128);
DecodeSSE2 ssePshufwMmx(PshufwMmxMmx, PshufwMmxE64, PshuflwXmmXmm, PshuflwXmmE128, PshufhwXmmXmm, PshufhwXmmE128, 8);
DecodeRMr sseAndnpsXmm(AndnpsXmmXmm, AndnpsXmmE128);
DecodeRMr sseAndpsXmm(AndpsXmmXmm, AndpsXmmE128);
DecodeRMr sseOrpsXmm(OrpsXmmXmm, OrpsXmmE128);
DecodeRMr sseXorpsXmm(XorpsXmmXmm, XorpsXmmE128);
DecodeSSE2 sseCvt2a(Cvtpi2psXmmMmx, Cvtpi2psXmmE64, Cvtsi2sdXmmR32, Cvtsi2sdXmmE32, Cvtsi2ssXmmR32, Cvtsi2ssXmmE32);
DecodeSSE2 sseCvt2c(Cvttps2piMmxXmm, Cvttps2piMmxE64, Cvttsd2siR32Xmm, Cvttsd2siR32E64, Cvttss2siR32Xmm, Cvttss2siR32E32);
DecodeSSE2 sseCvt2d(Cvtps2piMmxXmm, Cvtps2piMmxE64, Cvtsd2siR32Xmm, Cvtsd2siR32E64, Cvtss2siR32Xmm, Cvtss2siR32E32);
DecodeRMr sseMovapsXE(MovapsXmmXmm, MovapsXmmE128);
DecodeRM sseMovapsEX(MovapsXmmXmm, MovapsE128Xmm);
DecodeSSE2 sseMov0x310(MovupsXmmXmm, MovupsXmmE128, MovsdXmmXmm, MovsdXmmE64, MovssXmmXmm, MovssXmmE32);
DecodeSSE2r sseMov0x311(MovupsXmmXmm, MovupsE128Xmm, MovsdXmmXmm, MovsdE64Xmm, MovssXmmXmm, MovssE32Xmm);
DecodeRMr sseMov0x312(MovhlpsXmmXmm, MovlpsXmmE64);
DecodeRMr sseMov0x112(Invalid, MovlpdXmmE64);
DecodeRMr sseMov0x113(Invalid, MovlpdE64Xmm);
DecodeRMr sseMov0x114(UnpcklpdXmmXmm, UnpcklpdXmmE128);
DecodeRMr sseMov0x115(UnpckhpdXmmXmm, UnpckhpdXmmE128);
DecodeRMr sseMov0x116(Invalid, MovhpdXmmE64);
DecodeRM sseMov0x117(Invalid, MovhpdE64Xmm);
DecodeRMr sseMov0x128(MovapdXmmXmm, MovapdXmmE128);
DecodeRM sseMov0x129(MovapdXmmXmm, MovapdE128Xmm);
DecodeRMr sseMov0x12a(Cvtpi2pdXmmMmx, Cvtpi2pdXmmE64);
DecodeRMr sseMov0x12b(Invalid, MovntpdE128Xmm);
DecodeRMr sseMov0x12c(Cvttpd2piMmxXmm, Cvttpd2piMmE128);
DecodeRMr sseMov0x12d(Cvtpd2piMmxXmm, Cvtpd2piMmxE128);
DecodeRMr sseMov0x12e(UcomisdXmmXmm, UcomisdXmmE64);
DecodeRMr sseMov0x12f(ComisdXmmXmm, ComisdXmmE64);
DecodeRMr sseMov0x150(MovmskpdR32Xmm, Invalid);
DecodeRMr sseMov0x151(SqrtpdXmmXmm, SqrtpdXmmE128);
DecodeRMr sseAnd0x154(AndpdXmmXmm, AndpdXmmE128);
DecodeRMr sseAndNot0x155(AndnpdXmmXmm, AndnpdXmmE128);
DecodeRMr sseOr0x156(OrpdXmmXmm, OrpdXmmE128);
DecodeRMr sseXor0x157(XorpdXmmXmm, XorpdXmmE128);
DecodeRMr sseAdd0x158(AddpdXmmXmm, AddpdXmmE128);
DecodeRMr sseMul0x159(MulpdXmmXmm, MulpdXmmE128);
DecodeRMr sse0x15a(Cvtpd2psXmmXmm, Cvtpd2psXmmE128);
DecodeSSE2 sse0x35a(Cvtps2pdXmmXmm, Cvtps2pdXmmE64, Cvtsd2ssXmmXmm, Cvtsd2ssXmmE64, Cvtss2sdXmmXmm, Cvtss2sdXmmE32);
DecodeRMr sse0x15b(Cvtps2dqXmmXmm, Cvtps2dqXmmE128);
DecodeSSE2 sse0x35b(Cvtdq2psXmmXmm, Cvtdq2psXmmE128, Invalid, Invalid, Cvttps2dqXmmXmm, Cvttps2dqXmmE128);
DecodeRMr sse0x15c(SubpdXmmXmm, SubpdXmmE128);
DecodeRMr sse0x15d(MinpdXmmXmm, MinpdXmmE128);
DecodeRMr sse0x15e(DivpdXmmXmm, DivpdXmmE128);
DecodeRMr sse0x15f(MaxpdXmmXmm, MaxpdXmmE128);
DecodeRMr sse0x160(PunpcklbwXmmXmm, PunpcklbwXmmE128);
DecodeRMr sse0x161(PunpcklwdXmmXmm, PunpcklwdXmmE128);
DecodeRMr sse0x162(PunpckldqXmmXmm, PunpckldqXmmE128);
DecodeRMr sse0x163(PacksswbXmmXmm, PacksswbXmmE128);
DecodeRMr sse0x164(PcmpgtbXmmXmm, PcmpgtbXmmE128);
DecodeRMr sse0x165(PcmpgtwXmmXmm, PcmpgtwXmmE128);
DecodeRMr sse0x166(PcmpgtdXmmXmm, PcmpgtdXmmE128);
DecodeRMr sse0x167(PackuswbXmmXmm, PackuswbXmmE128);
DecodeRMr sse0x168(PunpckhbwXmmXmm, PunpckhbwXmmE128);
DecodeRMr sse0x169(PunpckhwdXmmXmm, PunpckhwdXmmE128);
DecodeRMr sse0x16a(PunpckhdqXmmXmm, PunpckhdqXmmE128);
DecodeRMr sse0x16b(PackssdwXmmXmm, PackssdwXmmE128);
DecodeRMr sse0x16c(PunpcklqdqXmmXmm, PunpcklqdqXmmE128);
DecodeRMr sse0x16d(PunpckhqdqXmmXmm, PunpckhqdqXmmE128);
DecodeRMr sse0x16e(MovdXmmR32, MovdXmmE32);
DecodeRMr sse0x16f(MovdqaXmmXmm, MovdqaXmmE128);
DecodeRMr sse0x170(PshufdXmmXmm, PshufdXmmE128, 8);
DecodeGroup sse0x171(Invalid, Invalid, PsrlwXmm, Invalid, PsrawXmm, Invalid, PsllwXmm, Invalid, 8);
DecodeGroup sse0x172(Invalid, Invalid, PsrldXmm, Invalid, PsradXmm, Invalid, PslldXmm, Invalid, 8);
DecodeGroup sse0x173(Invalid, Invalid, PsrlqXmm, PsrldqXmm, Invalid, Invalid, PsllqXmm, PslldqXmm, 8);
DecodeRMr sse0x174(PcmpeqbXmmXmm, PcmpeqbXmmE128);
DecodeRMr sse0x175(PcmpeqwXmmXmm, PcmpeqwXmmE128);
DecodeRMr sse0x176(PcmpeqdXmmXmm, PcmpeqdXmmE128);
DecodeRM sse0x17e(MovdR32Xmm, MovdE32Xmm);
DecodeSSE2custom sse0x37e(MovR32Pq, MovE32Pq, false, Invalid, Invalid, false, MovqXmmXmm, MovqXmmE64, true);
DecodeRM sse0x17f(MovdqaXmmXmm, MovdqaE128Xmm);
DecodeRMr sse0x1c2(CmppdXmmXmm, CmppdXmmE128, 8);
DecodeRM sse0x3c3(Invalid, MovntiE32R32);
DecodeRMr sse0x1c6(ShufpdXmmXmm, ShufpdXmmE128, 8);
DecodeRMr sse0x1d1(PsrlwXmmXmm, PsrlwXmmE128);
DecodeRMr sse0x1d2(PsrldXmmXmm, PsrldXmmE128);
DecodeRMr sse0x1d3(PsrlqXmmXmm, PsrlqXmmE128);
DecodeRMr sse0x1d4(PaddqXmmXmm, PaddqXmmE128);
DecodeRMr sse0x3d4(PaddqMmxMmx, PaddqMmxE64);
DecodeRMr sse0x1d5(PmullwXmmXmm, PmullwXmmE128);
DecodeRM sse0x1d6(MovqXmmXmm, MovqE64Xmm);
DecodeSSE2 sse0x3d6(Invalid, Invalid, Movdq2qMmxXmm, Invalid, Movq2dqXmmMmx, Invalid);
DecodeRMr sse0x1d8(PsubusbXmmXmm, PsubusbXmmE128);
DecodeRMr sse0x1d9(PsubuswXmmXmm, PsubuswXmmE128);
DecodeRMr sse0x1db(PandXmmXmm, PandXmmE128);
DecodeRMr sse0x1dc(PaddusbXmmXmm, PaddusbXmmE128);
DecodeRMr sse0x1dd(PadduswXmmXmm, PadduswXmmE128);
DecodeRMr sse0x1df(PandnXmmXmm, PandnXmmE128);
DecodeRMr sse0x1e1(PsrawXmmXmm, PsrawXmmE128);
DecodeRMr sse0x1e2(PsradXmmXmm, PsradXmmE128);
DecodeRMr sse0x1e5(PmulhwXmmXmm, PmulhwXmmE128);
DecodeRMr sse0x1e6(Cvttpd2dqXmmXmm, Cvttpd2dqXmmE128);
DecodeSSE2 sse0x3e6(Invalid, Invalid, Cvtpd2dqXmmXmm, Cvtpd2dqXmmE128, Cvtdq2pdXmmXmm, Cvtdq2pdXmmE128);
DecodeRM sse0x1e7(Invalid, MovntdqE128Xmm);
DecodeRMr sse0x1e8(PsubsbXmmXmm, PsubsbXmmE128);
DecodeRMr sse0x1e9(PsubswXmmXmm, PsubswXmmE128);
DecodeRMr sse0x1eb(PorXmmXmm, PorXmmXmmE128);
DecodeRMr sse0x1ec(PaddsbXmmXmm, PaddsbXmmE128);
DecodeRMr sse0x1ed(PaddswXmmXmm, PaddswXmmE128);
DecodeRMr sse0x1ef(PxorXmmXmm, PxorXmmE128);
DecodeRMr sse0x1f1(PsllwXmmXmm, PsllwXmmE128);
DecodeRMr sse0x1f2(PslldXmmXmm, PslldXmmE128);
DecodeRMr sse0x1f3(PsllqXmmXmm, PsllqXmmE128);
DecodeRMr sse0x1f4(PmuludqXmmXmm, PmuludqXmmE128);
DecodeRMr sse0x3f4(PmuludqMmxMmx, PmuludqMmxE64);
DecodeRMr sse0x1f5(PmaddwdXmmXmm, PmaddwdXmmE128);
DecodeRMr sse0x1f7(MaskmovdquE128XmmXmm, Invalid);
DecodeRMr sse0x1f8(PsubbXmmXmm, PsubbXmmE128);
DecodeRMr sse0x1f9(PsubwXmmXmm, PsubwXmmE128);
DecodeRMr sse0x1fa(PsubdXmmXmm, PsubdXmmE128);
DecodeRMr sse0x1fb(PsubqXmmXmm, PsubqXmmE128);
DecodeRMr sse0x3fb(PsubqMmxMmx, PsubqMmxE64);
DecodeRMr sse0x1fc(PaddbXmmXmm, PaddbXmmE128);
DecodeRMr sse0x1fd(PaddwXmmXmm, PaddwXmmE128);
DecodeRMr sse0x1fe(PadddXmmXmm, PadddXmmE128);

DecodeRM sseMov0x313(Invalid, MovlpsE64Xmm);
DecodeRMr sseMov0x316(MovlhpsXmmXmm, MovhpsXmmE64);
DecodeRMr sseMov0x317(Invalid, MovhpsE64Xmm);
DecodeRMr sseMovmsk(MovmskpsR32Xmm, Invalid);
DecodeRMr sseMaskmov(MaskmovqEDIMmxMmx, Invalid);
DecodeRM sseMovnt(Invalid, MovntpsE128Xmm);
DecodeRM sseMovntq(Invalid, MovntqE64Mmx);
DecodeRMr sseShufp(ShufpsXmmXmm, ShufpsXmmE128, 8);
DecodeRMr sseUnpckhp(UnpckhpsXmmXmm, UnpckhpsXmmE128);
DecodeRMr sseUnpcklp(UnpcklpsXmmXmm, UnpcklpsXmmE128);
Decode318 seePrefetch;
DecodeSSE2 sseCmp(CmppsXmmXmm, CmppsXmmE128, CmpsdXmmXmm, CmpsdXmmE64, CmpssXmmXmm, CmpssXmmE32, 8);
DecodeRMr sseComis(ComissXmmXmm, ComissXmmE32);
DecodeRMr sseUcomis(UcomissXmmXmm, UcomissXmmE32);

DecodeRMr sse2Movupd(MovupdXmmXmm, MovupdXmmE128);
DecodeRM sse2Movupd_r(MovupdXmmXmm, MovupdE128Xmm);
DecodeRMr sse2Movapd(MovapdXmmXmm, MovapdXmmE128);
DecodeRM sse2Movapd_r(MovapdXmmXmm, MovapdE128Xmm);

const Decode* const decoder[] = {
    // 0x000
    &decodeAddEbGb, &decodeAddEwGw, &decodeAddGbEb, &decodeAddGwEw, &decodeAddAlIb, &decodeAddAxIw, &decodePushEs16, &decodePopEs16,
    &decodeOrEbGb, &decodeOrEwGw, &decodeOrGbEb, &decodeOrGwEw, &decodeOrAlIb, &decodeOrAxIw, &decodePushCs16, &decode2Byte,
    // 0x010
    &decodeAdcEbGb, &decodeAdcEwGw, &decodeAdcGbEb, &decodeAdcGwEw, &decodeAdcAlIb, &decodeAdcAxIw, &decodePushSs16, &decodePopSs16,
    &decodeSbbEbGb, &decodeSbbEwGw, &decodeSbbGbEb, &decodeSbbGwEw, &decodeSbbAlIb, &decodeSbbAxIw, &decodePushDs16, &decodePopDs16,
    // 0x020
    &decodeAndEbGb, &decodeAndEwGw, &decodeAndGbEb, &decodeAndGwEw, &decodeAndAlIb, &decodeAndAxIw, &decodeSegEs, &decodeDaa,
    &decodeSubEbGb, &decodeSubEwGw, &decodeSubGbEb, &decodeSubGwEw, &decodeSubAlIb, &decodeSubAxIw, &decodeSegCs, &decodeDas,
    // 0x030
    &decodeXorEbGb, &decodeXorEwGw, &decodeXorGbEb, &decodeXorGwEw, &decodeXorAlIb, &decodeXorAxIw, &decodeSegSs, &decodeAaa,
    &decodeCmpEbGb, &decodeCmpEwGw, &decodeCmpGbEb, &decodeCmpGwEw, &decodeCmpAlIb, &decodeCmpAxIw, &decodeSegDs, &decodeAas,
    // 0x040
    &decodeIncAx, &decodeIncCx, &decodeIncDx, &decodeIncBx, &decodeIncSp, &decodeIncBp, &decodeIncSi, &decodeIncDi,
    &decodeDecAx, &decodeDecCx, &decodeDecDx, &decodeDecBx, &decodeDecSp, &decodeDecBp, &decodeDecSi, &decodeDecDi,
    // 0x050
    &decodePushAx, &decodePushCx, &decodePushDx, &decodePushBx, &decodePushSp, &decodePushBp, &decodePushSi, &decodePushDi,
    &decodePopAx, &decodePopCx, &decodePopDx, &decodePopBx, &decodePopSp, &decodePopBp, &decodePopSi, &decodePopDi,
    // 0x060
    &decodePushA16, &decodePopA16, &decodeBound16, &decodeArpl, &decodeSegFs, &decodeSegGs, &decode66, &decode67,
    &decodePushIw16, &decodeImulGwEwIw, &decodePushIb16, &decodeImulGwEwIb, &decodeInsb, &decodeInsw, &decodeOutsb, &decodeOutsw,
    // 0x070
    &decodeJo8, &decodeJno8, &decodeJb8, &decodeJnb8, &decodeJz8, &decodeJnz8, &decodeJbe8, &decodeJnbe8,
    &decodeJs8, &decodeJns8, &decodeJp8, &decodeJnp8, &decodeJl8, &decodeJnl8, &decodeJle8, &decodeJnle8,
    // 0x080
    &decodeGroup1_8_Ib, &decodeGroup1_16_Iw, &decodeGroup1_8_Ib, &decodeGroup1_16_Ix, &decodeTestEbGb, &decodeTestEwGw, &decodeXchgEbGb, &decodeXchgEwGw,
    &decodeMovEbGb, &decodeMovEwGw, &decodeMovGbEb, &decodeMovGwEw, &decodeMovEwSw, &decodeLeaGw, &decodeMovSwEw, &decodePopEw,
    // 0x090
    &decodeNop, &decodeXchgCxAx, &decodeXchgDxAx, &decodeXchgBxAx, &decodeXchgSpAx, &decodeXchgBpAx, &decodeXchgSiAx, &decodeXchgDiAx,
    &decodeCbw, &decodeCwd, &decodeCallAp, &decodeWait, &decodePushF16, &decodePopF16, &decodeSahf, &decodeLahf,
    // 0x0a0
    &decodeMovAlOb, &decodeMovAxOw, &decodeMovObAl, &decodeMovOwAx, &decodeMovsb, &decodeMovsw, &decodeCmpsb, &decodeCmpsw,
    &decodeTestAlIb, &decodeTestAxIw, &decodeStosb, &decodeStosw, &decodeLodsb, &decodeLodsw, &decodeScasb, &decodeScasw,
    // 0x0b0
    &decodeMovAlIb, &decodeMovClIb, &decodeMovDlIb, &decodeMovBlIb, &decodeMovAhIb, &decodeMovChIb, &decodeMovDhIb, &decodeMovBhIb,
    &decodeMovAxIw, &decodeMovCxIw, &decodeMovDxIw, &decodeMovBxIw, &decodeMovSpIw, &decodeMovBpIw, &decodeMovSiIw, &decodeMovDiIw,
    // 0x0c0
    &decodeGroup2_8_Ib, &decodeGroup2_16_Ib, &decodeRetn16Iw, &decodeRetn16, &decodeLes, &decodeLds, &decodeMovEbIb, &decodeMovEwIw,
    &decodeEnter16, &decodeLeave16, &decodeRetfIw16, &decodeRetf16, &decodeInt3, &decodeIntIb, &decodeIntO, &decodeIret,
    // 0x0d0
    &decodeGroup2_8_1, &decodeGroup2_16_1, &decodeGroup2_8_Cl, &decodeGroup2_16_Cl, &decodeAam, &decodeAad, &decodeSalc, &decodeXlat,
    &decodeFpu0, &decodeFpu1, &decodeFpu2, &decodeFpu3, &decodeFpu4, &decodeFpu5, &decodeFpu6, &decodeFpu7,
    // 0x0e0
    &decodeLoopNZ, &decodeLoopZ, &decodeLoop, &decodeJcxz, &decodeInAlIb, &decodeInAxIb, &decodeOutIbAl, &decodeOutIbAx,
    &decodeCallJw, &decodeJmpJw, &decodeJmpAp, &decodeJmpJb, &decodeInAlDx, &decodeInAxDx, &decodeOutDxAl, &decodeOutDxAx,
    // 0x0f0
    &decodeLock, &decodeICEBP, &decodeRepNZ, &decodeRepZ, &decodeHlt, &decodeCmc, &decodeGroup3_8, &decodeGroup3_16,
    &decodeClc, &decodeStc, &decodeCli, &decodeSti, &decodeCld, &decodeStd, &decodeGroup4_8, &decodeGroup5_16,
    // 0x100
    &decodeGroup6_16, nullptr, &decodeLar16, &decodeLsl16, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    // 0x110
    &sse2Movupd, &sse2Movupd_r, &sseMov0x112, &sseMov0x113, &sseMov0x114, &sseMov0x115, &sseMov0x116, &sseMov0x117,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &decodeHintNop,
    // 0x120
    &decodeMovRdCrx, nullptr, &decodeMovCrxRd, nullptr, nullptr, nullptr, nullptr, nullptr,
    &sse2Movapd, &sse2Movapd_r, &sseMov0x12a, &sseMov0x12b, &sseMov0x12c, &sseMov0x12d, &sseMov0x12e, &sseMov0x12f,
    // 0x130
    nullptr, &decodeRdtsc, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    // 0x140
    &decodeCmovO_16, &decodeCmovNO_16, &decodeCmovB_16, &decodeCmovNB_16, &decodeCmovZ_16, &decodeCmovNZ_16, &decodeCmovBE_16, &decodeCmovNBE_16,
    &decodeCmovS_16, &decodeCmovNS_16, &decodeCmovP_16, &decodeCmovNP_16, &decodeCmovL_16, &decodeCmovNL_16, &decodeCmovLE_16, &decodeCmovNLE_16,
    // 0x150
    &sseMov0x150, &sseMov0x151, nullptr, nullptr, &sseAnd0x154, &sseAndNot0x155, &sseOr0x156, &sseXor0x157,
    &sseAdd0x158, &sseMul0x159, &sse0x15a, &sse0x15b, &sse0x15c, &sse0x15d, &sse0x15e, &sse0x15f,
    // 0x160
    &sse0x160, &sse0x161, &sse0x162, &sse0x163, &sse0x164, &sse0x165, &sse0x166, &sse0x167,
    &sse0x168, &sse0x169, &sse0x16a, &sse0x16b, &sse0x16c, &sse0x16d, &sse0x16e, &sse0x16f,
    // 0x170
    &sse0x170, &sse0x171, &sse0x172, &sse0x173, &sse0x174, &sse0x175, &sse0x176, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &sse0x17e, &sse0x17f,
    // 0x180
    &decodeJo16, &decodeJno16, &decodeJb16, &decodeJnb16, &decodeJz16, &decodeJnz16, &decodeJbe16, &decodeJnbe16,
    &decodeJs16, &decodeJns16, &decodeJp16, &decodeJnp16, &decodeJl16, &decodeJnl16, &decodeJle16, &decodeJnle16,
    // 0x190
    &decodeSetO, &decodeSetNO, &decodeSetB, &decodeSetNB, &decodeSetZ, &decodeSetNZ, &decodeSetBE, &decodeSetNBE,
    &decodeSetS, &decodeSetNS, &decodeSetP, &decodeSetNP, &decodeSetL, &decodeSetNL, &decodeSetLE, &decodeSetNLE,
    // 0x1a0
    &decodePushFs16, &decodePopFs16, &decodeCPUID, &decodeBtEwGw, &decodeDshlEwGw, &decodeDshlClEwGw, nullptr, nullptr,
    &decodePushGs16, &decodePopGs16, nullptr, &decodeBtsEwGw, &decodeDshrEwGw, &decodeDshrClEwGw, nullptr, &decodeDimulGwEw,
    // 0x1b0
    &decodeCmpXchgEbGb, &decodeCmpXchgEwGw, &decodeLss, &decodeBtrEwGw, &decodeLfs, &decodeLgs, &decodeMovGwXz8, &decodeMovEwGw,
    nullptr, nullptr, &decodeGroup8_16, &decodeBtcEwGw, &decodeBsfGwEw, &decodeBsrGwEw, &decodeMovGwSx8, nullptr,
    // 0x1c0
    &decodeXadd8, &decodeXadd16, &sse0x1c2, nullptr, &ssePinsrwXmm, &ssePextrwXmm, &sse0x1c6, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    // 0x1d0
    nullptr, &sse0x1d1, &sse0x1d2, &sse0x1d3, &sse0x1d4, &sse0x1d5, &sse0x1d6, &ssePmovmskbXmm,
    &sse0x1d8, &sse0x1d9, &ssePminubXmm, &sse0x1db, &sse0x1dc, &sse0x1dd, &ssePmaxubXmm, &sse0x1df,
    // 0x1e0
    &ssePavgbXmm, &sse0x1e1, &sse0x1e2, &ssePavgwXmm, &ssePmulhuwXmm, &sse0x1e5, &sse0x1e6, &sse0x1e7,
    &sse0x1e8, &sse0x1e9, &ssePminswXmm, &sse0x1eb, &sse0x1ec, &sse0x1ed, &ssePmaxswXmm, &sse0x1ef,
    // 0x1f0
    nullptr, &sse0x1f1, &sse0x1f2, &sse0x1f3, &sse0x1f4, &sse0x1f5, &ssePsadbwXmm, &sse0x1f7,
    &sse0x1f8, &sse0x1f9, &sse0x1fa, &sse0x1fb, &sse0x1fc, &sse0x1fd, &sse0x1fe, nullptr,
    // 0x200
    &decodeAddEbGb, &decodeAddEdGd, &decodeAddGbEb, &decodeAddGdEd, &decodeAddAlIb, &decodeAddEaxId, &decodePushEs32, &decodePopEs32,
    &decodeOrEbGb, &decodeOrEdGd, &decodeOrGbEb, &decodeOrGdEd, &decodeOrAlIb, &decodeOrEaxId, &decodePushCs32, &decode2Byte,
    // 0x210
    &decodeAdcEbGb, &decodeAdcEdGd, &decodeAdcGbEb, &decodeAdcGdEd, &decodeAdcAlIb, &decodeAdcEaxId, &decodePushSs32, &decodePopSs32,
    &decodeSbbEbGb, &decodeSbbEdGd, &decodeSbbGbEb, &decodeSbbGdEd, &decodeSbbAlIb, &decodeSbbEaxId, &decodePushDs32, &decodePopDs32,
    // 0x220
    &decodeAndEbGb, &decodeAndEdGd, &decodeAndGbEb, &decodeAndGdEd, &decodeAndAlIb, &decodeAndEaxId, &decodeSegEs, &decodeDaa,
    &decodeSubEbGb, &decodeSubEdGd, &decodeSubGbEb, &decodeSubGdEd, &decodeSubAlIb, &decodeSubEaxId, &decodeSegCs, &decodeDas,
    // 0x230
    &decodeXorEbGb, &decodeXorEdGd, &decodeXorGbEb, &decodeXorGdEd, &decodeXorAlIb, &decodeXorEaxId, &decodeSegSs, &decodeAaa,
    &decodeCmpEbGb, &decodeCmpEdGd, &decodeCmpGbEb, &decodeCmpGdEd, &decodeCmpAlIb, &decodeCmpEaxId, &decodeSegDs, &decodeAas,
    // 0x240
    &decodeIncEax, &decodeIncEcx, &decodeIncEdx, &decodeIncEbx, &decodeIncEsp, &decodeIncEbp, &decodeIncEsi, &decodeIncEdi,
    &decodeDecEax, &decodeDecEcx, &decodeDecEdx, &decodeDecEbx, &decodeDecEsp, &decodeDecEbp, &decodeDecEsi, &decodeDecEdi,
    // 0x250
    &decodePushEax, &decodePushEcx, &decodePushEdx, &decodePushEbx, &decodePushEsp, &decodePushEbp, &decodePushEsi, &decodePushEdi,
    &decodePopEax, &decodePopEcx, &decodePopEdx, &decodePopEbx, &decodePopEsp, &decodePopEbp, &decodePopEsi, &decodePopEdi,
    // 0x260
    &decodePushA32, &decodePopA32, &decodeBound32, &decodeArpl32, &decodeSegFs, &decodeSegGs, &decode266, &decode267,
    &decodePushId32, &decodeImulGdEdId, &decodePushIb32, &decodeImulGdEdIb, &decodeInsb, &decodeInsd, &decodeOutsb, &decodeOutsd,
    // 0x270
    &decodeJo8, &decodeJno8, &decodeJb8, &decodeJnb8, &decodeJz8, &decodeJnz8, &decodeJbe8, &decodeJnbe8,
    &decodeJs8, &decodeJns8, &decodeJp8, &decodeJnp8, &decodeJl8, &decodeJnl8, &decodeJle8, &decodeJnle8,
    // 0x280
    &decodeGroup1_8_Ib, &decodeGroup1_32_Id, &decodeGroup1_8_Ib, &decodeGroup1_32_Ix, &decodeTestEbGb, &decodeTestEdGd, &decodeXchgEbGb, &decodeXchgEdGd,
    &decodeMovEbGb, &decodeMovEdGd, &decodeMovGbEb, &decodeMovGdEd, &decodeMovEdSw, &decodeLeaGd, &decodeMovSwEw, &decodePopEd,
    // 0x290
    &decodeNop, &decodeXchgEcxEax, &decodeXchgEdxEax, &decodeXchgEbxEax, &decodeXchgEspEax, &decodeXchgEbpEax, &decodeXchgEsiEax, &decodeXchgEdiEax,
    &decodeCwde, &decodeCwq, &decodeCallFar, &decodeWait, &decodePushF32, &decodePopF32, &decodeSahf, &decodeLahf,
    // 0x2a0
    &decodeMovAlOb, &decodeMovEaxOd, &decodeMovObAl, &decodeMovOdEax, &decodeMovsb, &decodeMovsd, &decodeCmpsb, &decodeCmpsd,
    &decodeTestAlIb, &decodeTestEaxId, &decodeStosb, &decodeStosd, &decodeLodsb, &decodeLodsd, &decodeScasb, &decodeScasd,
    // 0x2b0
    &decodeMovAlIb, &decodeMovClIb, &decodeMovDlIb, &decodeMovBlIb, &decodeMovAhIb, &decodeMovChIb, &decodeMovDhIb, &decodeMovBhIb,
    &decodeMovEaxId, &decodeMovEcxId, &decodeMovEdxId, &decodeMovEbxId, &decodeMovEspId, &decodeMovEbpId, &decodeMovEsiId, &decodeMovEdiId,
    // 0x2c0
    &decodeGroup2_8_Ib, &decodeGroup2_32_Ib, &decodeRetn32Iw, &decodeRetn32, &decodeLes32, &decodeLds32, &decodeMovEbIb, &decodeMovEdId,
    &decodeEnter32, &decodeLeave32, &decodeRetfIw32, &decodeRetf32, &decodeInt3, &decodeIntIb, &decodeIntO, &decodeIret32,
    // 0x2d0
    &decodeGroup2_8_1, &decodeGroup2_32_1, &decodeGroup2_8_Cl, &decodeGroup2_32_Cl, &decodeAam, &decodeAad, &decodeSalc, &decodeXlat,
    &decodeFpu0, &decodeFpu1, &decodeFpu2, &decodeFpu3, &decodeFpu4, &decodeFpu5, &decodeFpu6, &decodeFpu7,
    // 0x2e0
    &decodeLoopNZ, &decodeLoopZ, &decodeLoop, &decodeJcxz, &decodeInAlIb, &decodeInEaxIb, &decodeOutIbAl, &decodeOutIbEax,
    &decodeCallJd, &decodeJmpJd, &decodeJmpFar, &decodeJmpJb, &decodeInAlDx, &decodeInEaxDx, &decodeOutDxAl, &decodeOutDxEax,
    // 0x2f0
    &decodeLock, &decodeICEBP, &decodeRepNZ, &decodeRepZ, &decodeHlt, &decodeCmc, &decodeGroup3_8, &decodeGroup3_32,
    &decodeClc, &decodeStc, &decodeCli, &decodeSti, &decodeCld, &decodeStd, &decodeGroup4_8, &decodeGroup5_32,
    // 0x300
    &decodeGroup6_32, &decodeGroup7_32, nullptr, &decodeLsl32, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    // 0x310
    &sseMov0x310, &sseMov0x311, &sseMov0x312, &sseMov0x313, &sseUnpcklp, &sseUnpckhp, &sseMov0x316, &sseMov0x317,
    &seePrefetch, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &decodeHintNop,
    // 0x320
    &decodeMovRdCrx, nullptr, &decodeMovCrxRd, nullptr, nullptr, nullptr, nullptr, nullptr,
    &sseMovapsXE, &sseMovapsEX, &sseCvt2a, &sseMovnt, &sseCvt2c, &sseCvt2d, &sseUcomis, &sseComis,
    // 0x330
    nullptr, &decodeRdtsc, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    // 0x340
    &decodeCmovO_32, &decodeCmovNO_32, &decodeCmovB_32, &decodeCmovNB_32, &decodeCmovZ_32, &decodeCmovNZ_32, &decodeCmovBE_32, &decodeCmovNBE_32,
    &decodeCmovS_32, &decodeCmovNS_32, &decodeCmovP_32, &decodeCmovNP_32, &decodeCmovL_32, &decodeCmovNL_32, &decodeCmovLE_32, &decodeCmovNLE_32,
    // 0x350
    &sseMovmsk, &sseSqrt, &sseRsqrt, &sseRpc, &sseAndpsXmm, &sseAndnpsXmm, &sseOrpsXmm, &sseXorpsXmm,
    &sseAdd, &sseMul, &sse0x35a, &sse0x35b, &sseSub, &sseMin, &sseDiv, &sseMax,
    // 0x360
    &decodePunpcklbw, &decodePunpcklwd, &decodePunpckldq, &decodePacksswb, &decodePcmpgtb, &decodePcmpgtw, &decodePcmpgtd, &decodePackuswb,
    &decodePunpckhbw, &decodePunpckhwd, &decodePunpckhdq, &decodePackssdw, nullptr, nullptr, &decodeMovPqEd, &decodeMovPqQq,
    // 0x370
    &ssePshufwMmx, &decode371MMX, &decode372MMX, &decode373MMX, &decodePcmpeqb, &decodePcmpeqw, &decodePcmpeqd, &decodeEmms,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &sse0x37e, &decodeMovQqPq,
    // 0x380
    &decodeJo32, &decodeJno32, &decodeJb32, &decodeJnb32, &decodeJz32, &decodeJnz32, &decodeJbe32, &decodeJnbe32,
    &decodeJs32, &decodeJns32, &decodeJp32, &decodeJnp32, &decodeJl32, &decodeJnl32, &decodeJle32, &decodeJnle32,
    // 0x390
    &decodeSetO, &decodeSetNO, &decodeSetB, &decodeSetNB, &decodeSetZ, &decodeSetNZ, &decodeSetBE, &decodeSetNBE,
    &decodeSetS, &decodeSetNS, &decodeSetP, &decodeSetNP, &decodeSetL, &decodeSetNL, &decodeSetLE, &decodeSetNLE,
    // 0x3a0
    &decodePushFs32, &decodePopFs32, &decodeCPUID, &decodeBtEdGd, &decodeDshlEdGd, &decodeDshlClEdGd, nullptr, nullptr,
    &decodePushGs32, &decodePopGs32, nullptr, &decodeBtsEdGd, &decodeDshrEdGd, &decodeDshrClEdGd, &decode3EA, &decodeDimulGdEd,
    // 0x3b0
    &decodeCmpXchgEbGb, &decodeCmpXchgEdGd, &decodeLss32, &decodeBtrEdGd, &decodeLfs32, &decodeLgs32, &decodeMovGdXz8, &decodeMovGdXz16,
    nullptr, nullptr, &decodeGroup8_32, &decodeBtcEdGd, &decodeBsfGdEd, &decodeBsrGdEd, &decodeMovGdSx8, &decodeMovGdSx16,
    // 0x3c0
    &decodeXadd8, &decodeXadd32, &sseCmp, &sse0x3c3, &ssePinsrwMmx, &ssePextrwMmx, &sseShufp, &decodeCmpXchg8b,
    &decodeBswapEAX, &decodeBswapECX, &decodeBswapEDX, &decodeBswapEBX, &decodeBswapESP, &decodeBswapEBP, &decodeBswapESI, &decodeBswapEDI,
    // 0x3d0
    nullptr, &decodePsrlw, &decodePsrld, &decodePsrlq, &sse0x3d4, &decodePmullw, &sse0x3d6, &ssePmovmskbMmx,
    &decodePsubusb, &decodePsubusw, &ssePminubMmx, &decodePand, &decodePaddusb, &decodePaddusw, &ssePmaxubMmx, &decodePandn,
    // 0x3e0
    &ssePavgbMmx, &decodePsraw, &decodePsrad, &ssePavgwMmx, &ssePmulhuwMmx, &decodePmulhw, &sse0x3e6, &sseMovntq,
    &decodePsubsb, &decodePsubsw, &ssePminswMmx, &decodePor, &decodePaddsb, &decodePaddsw, &ssePmaxswMmx, &decodePxor,
    // 0x3f0
    nullptr, &decodePsllw, &decodePslld, &decodePsllq, &sse0x3f4, &decodePmaddwd, &ssePsadbwMmx, &sseMaskmov,
    &decodePsubb, &decodePsubw, &decodePsubd, &sse0x3fb, &decodePaddb, &decodePaddw, &decodePaddd, nullptr,
};

U8 DecodeData::fetch8() {
    this->opLen++;
    return this->fetchByte(this->fetchByteData, &this->eip);
}

U16 DecodeData::fetch16() {
    return ((U16)this->fetch8()) | (((U16)this->fetch8()) << 8);
}

U32 DecodeData::fetch32() {
    return ((U32)this->fetch16()) | (((U32)this->fetch16()) << 16);
}

static PtrPool<DecodedOp> freeOps;

DecodedOp::DecodedOp() {
    this->reset();
}

void DecodedOp::clearCache() {
    freeOps.deleteAll();
}

void DecodedOp::reset() {
    this->next = nullptr;
    this->disp = 0;
    this->imm = 0;
    this->reg = 0;
    this->rm = 0;
    this->base = DS;
    this->sibIndex = 0;
    this->sibScale = 0;
    this->len = 0;
    this->lock = 0;
    this->repZero = 0;
    this->repNotZero = 0;
    this->pfn = nullptr;
}
DecodedOp* DecodedOp::alloc() {
    return freeOps.get();   
}

void DecodedOp::dealloc(bool deallocNext) {
#ifdef _DEBUG
    if (this->inst == InstructionCount) {
        kpanic("tried to dealloc a DecodedOp that was already deallocated");
    }
#endif
    if (deallocNext && this->next) {
        this->next->dealloc(deallocNext);
    }
    this->next = nullptr;
    this->inst = InstructionCount;
    freeOps.put(this);
}

bool DecodedOp::isStringOp() {
    if (this->inst == Lodsb || this->inst == Lodsw || this->inst == Lodsd ||
        this->inst == Stosb || this->inst == Stosw || this->inst == Stosd ||
        this->inst == Scasb || this->inst == Scasw || this->inst == Scasd ||
        this->inst == Movsb || this->inst == Movsw || this->inst == Movsd ||
        this->inst == Cmpsb || this->inst == Cmpsw || this->inst == Cmpsd) {

        return true;
    }
    return false;
}

bool DecodedOp::isFpuOp() {
   return (this->inst>=FADD_ST0_STj && this->inst<=FISTP_QWORD_INTEGER);
}

bool DecodedOp::needsToSetFlags() {
    U32 needsToSet = instructionInfo[this->inst].flagsSets & ~MAYBE;
    return DecodedOp::getNeededFlags(DecodedBlock::currentBlock, this, needsToSet)!=0;
}

U32 DecodedOp::getNeededFlags(DecodedBlock* block, DecodedOp* op, U32 flags, U32 depth) {
    DecodedOp* n = op->next;
    DecodedOp* lastOp = op;

    while (n && flags) {
        if (instructionInfo[n->inst].flagsUsed & flags) {
            U32 result = instructionInfo[n->inst].flagsUsed & flags;
            flags &= ~ instructionInfo[n->inst].flagsSets;
            flags &= ~ instructionInfo[n->inst].flagsUndefined;
            flags &= ~result;
            if (flags && op->next) {
                result |= DecodedOp::getNeededFlags(block, op->next, flags, depth);
            }
            return result;
        }
        if (!(instructionInfo[n->inst].flagsSets & MAYBE)) {
            flags &= ~ instructionInfo[n->inst].flagsSets;
            flags &= ~ instructionInfo[n->inst].flagsUndefined;
        }
#ifdef BOXEDWINE_BINARY_TRANSLATOR
        if (instructionInfo[n->inst].branch) {
            break;
        }
#endif
        lastOp = n;
        n = n->next;
    }
    if (flags && (instructionInfo[lastOp->inst].branch & DECODE_BRANCH_1) && depth>0) {
        // :TODO: maybe decode the missing branch?
        if (block->next1 && (block->next2 || !(instructionInfo[lastOp->inst].branch & DECODE_BRANCH_2))) {
            U32 needsToSet1 = DecodedOp::getNeededFlags(block->next1, block->next1->op, flags, depth-1);          

            U32 needsToSet2 = 0;
            if ((instructionInfo[lastOp->inst].branch & DECODE_BRANCH_2)) {
                needsToSet2 = flags;
                // :TODO: maybe decode the missing branch?
                if (block->next2) {
                    needsToSet2 = (DecodedOp::getNeededFlags(block->next2, block->next2->op, flags, depth-1));
                }
            }
            flags = needsToSet1 | needsToSet2;
        }
    }
    return flags;
}

static DecodedBlockFromNode* freeFromNodes;
DecodedBlockFromNode* DecodedBlockFromNode::alloc() {
    DecodedBlockFromNode* result = nullptr;

    if (freeFromNodes) {
        result = freeFromNodes;
        freeFromNodes = freeFromNodes->next;
    } else {
        DecodedBlockFromNode* nodes = new DecodedBlockFromNode[1024];

        freeFromNodes = &nodes[1];
        freeFromNodes->next = nullptr;
        for (int i=2;i<1024;i++) {
            nodes[i].next = freeFromNodes;
            freeFromNodes = &nodes[i];            
        }
        result = &nodes[0];
    }
    result->next = nullptr;
    result->block = nullptr;
    return result;
}
void DecodedBlockFromNode::dealloc() {
    this->next = freeFromNodes;
    this->block = nullptr;
    freeFromNodes = this;
}

void DecodedBlock::addReferenceFrom(DecodedBlock* block) {
    DecodedBlockFromNode* node = DecodedBlockFromNode::alloc();
    node->block = block;
    node->next = this->referencedFrom;
    this->referencedFrom = node;
}

DecodedOp* DecodedBlock::getOp(U32 eip) {
    DecodedOp* op = this->op;
    if (op->len == 0) {
        op = op->next;
    }
    U32 opEip = this->address;
    while (op) {
        if (opEip == eip) {
            return op;
        }
        opEip += op->len;
        op = op->next;
    }
    return nullptr;
}

void DecodedBlock::removeReferenceFrom(DecodedBlock* block) {
    DecodedBlockFromNode* from = this->referencedFrom;
	DecodedBlockFromNode* prev = nullptr;

	while (from) {
		if (from->block == block) {
			DecodedBlockFromNode* removed = from;

			if (prev) {					
				prev->next = from->next;					
			} else {
				this->referencedFrom = from->next;
			}
			from = from->next;
			removed->dealloc();
			continue;
		}
		prev = from;
		from = from->next;
	}
}

thread_local DecodedBlock* DecodedBlock::currentBlock;

void decodeBlock(pfnFetchByte fetchByte, void* fetchByteData, U32 eip, bool isBig, U32 maxInstructions, U32 maxLen, U32 stopIfThrowsException, DecodedBlock* block) {
    DecodeData d;    
    DecodedOp* op = DecodedOp::alloc();

    d.fetchByte = fetchByte;
    d.fetchByteData = fetchByteData;
    d.eip = eip;
    d.opCountSoFarInThisBlock = 0;

    block->op = op;
    block->bytes = 0;
    block->opCount = 0;
    while (1) {
        d.opLen = 0;
        d.ds = DS;
        d.ss = SS;
        if (isBig) {
            d.opCode = 0x200;
            d.ea16 = 0;
        } else {
            d.opCode = 0;
            d.ea16 = 1;
        }
        d.inst = d.opCode+d.fetch8(); 
        if (!decoder[d.inst]) {
            op->inst = Invalid;
        } else {
            decoder[d.inst]->decode(&d, op);
        }
        if (op->inst == Invalid) {
#if defined _DEBUG || defined BOXEDWINE_BINARY_TRANSLATOR
            op->originalOp = d.inst;
#endif
            break;
        }
        d.opCountSoFarInThisBlock++;
        block->opCount++;
        if (maxLen && d.opLen+block->bytes>maxLen) {
            op->inst = Done;
            op->len = 0;
            break;
        }
        op->len = d.opLen;
        op->ea16 = d.ea16;
        block->bytes += d.opLen;        
#if defined _DEBUG || defined BOXEDWINE_BINARY_TRANSLATOR
        op->originalOp = d.inst;
#endif
        if ((maxInstructions && maxInstructions<=block->opCount) || instructionInfo[op->inst].branch || (stopIfThrowsException && instructionInfo[op->inst].throwsException))
            break;
        op->next = DecodedOp::alloc();
        op = op->next;
    }
}

const char* DecodedOp::name() {
#ifdef __EMSCRIPTEN__
    return "unknown";
#else
    return instructionLog[this->inst].name;
#endif
}

void DecodedOp::log(CPU* cpu) {
#ifdef _DEBUG
    if (cpu->logFile.isOpen() && this->inst >= 0 && this->inst < None) {
        BOXEDWINE_CRITICAL_SECTION;
        U64 pos = cpu->logFile.getPos();
        cpu->logFile.writeFormat("%04X %08X ", cpu->thread->id, cpu->eip.u32);
        instructionLog[this->inst].pfnFormat(&instructionLog[this->inst], this, cpu);
        if (instructionLog[this->inst].imm) {
            if (instructionLog[this->inst].pfnFormat!=logName)
                cpu->logFile.write(",");
            switch (instructionLog[this->inst].width) {
            case -16:
                cpu->logFile.writeFormat("%X", (S32)((S16)((U16)this->imm)));
                break;
            case -32:
                cpu->logFile.writeFormat("%X", this->imm);
                break;
            case -8:
                cpu->logFile.writeFormat("%X", (S32)((S8)((U8)this->imm)));
                break;
            default:
                cpu->logFile.writeFormat("%X", this->imm);
                break;
            }        
        }
        U64 endPos = cpu->logFile.getPos();
        if (endPos-pos<55) {
            cpu->logFile.write(B("                                                       ").substr(0, (U32)(55-(endPos-pos))));
        }
        cpu->logFile.writeFormat(" EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X SS=%.8X DS=%.8X FLAGS=%.8X\n", EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, cpu->seg[SS].address, cpu->seg[DS].address, cpu->flags);
        cpu->logFile.flush();
    }
#endif
}
