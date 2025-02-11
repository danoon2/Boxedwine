#include "boxedwine.h"
void common_cmpxchgr8r8(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->dst.u8 = AL;
    cpu->src.u8 = *cpu->reg8[dstReg];
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    if (AL == cpu->src.u8) {
        *cpu->reg8[dstReg] = *cpu->reg8[srcReg];
    } else {
        AL = cpu->src.u8;
    }
}
void common_cmpxchge8r8(CPU* cpu, U32 address, U32 srcReg){
    cpu->dst.u8 = AL;
    cpu->src.u8 = cpu->memory->readb(address);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;
    if (AL == cpu->src.u8) {
        cpu->memory->writeb(address, *cpu->reg8[srcReg]);
    } else {
        AL = cpu->src.u8;
    }
}
void common_cmpxchgr16r16(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->reg[dstReg].u16;
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->reg[dstReg].u16 = cpu->reg[srcReg].u16;
    } else {
        AX = cpu->src.u16;
    }
}
void common_cmpxchge16r16(CPU* cpu, U32 address, U32 srcReg){
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->memory->readw(address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;
    if (AX == cpu->src.u16) {
        cpu->memory->writew(address, cpu->reg[srcReg].u16);
    } else {
        AX = cpu->src.u16;
    }
}
void common_cmpxchgr32r32(CPU* cpu, U32 dstReg, U32 srcReg){
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->reg[dstReg].u32;
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->reg[dstReg].u32 = cpu->reg[srcReg].u32;
    } else {
        EAX = cpu->src.u32;
    }
}
void common_cmpxchge32r32(CPU* cpu, U32 address, U32 srcReg){
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->memory->readd(address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;
    if (EAX == cpu->src.u32) {
        cpu->memory->writed(address, cpu->reg[srcReg].u32);
    } else {
        EAX = cpu->src.u32;
    }
}

#ifdef BOXEDWINE_MULTI_THREADED
void common_cmpxchge32r32_lock(CPU* cpu, U32 address, U32 srcReg) {
    cpu->dst.u32 = EAX;
    cpu->src.u32 = cpu->memory->readd(address);
    cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    cpu->lazyFlags = FLAGS_CMP32;

    LockData32* p = (LockData32*)cpu->memory->getRamPtr(address, 4, true);
    std::atomic_ref<U32> mem(p->data);
    U32 expected = EAX;
    U32 desired = cpu->reg[srcReg].u32;

    if (!mem.compare_exchange_strong(expected, desired)) {
        EAX = expected;
        cpu->src.u32 = expected;
        cpu->result.u32 = cpu->dst.u32 - cpu->src.u32;
    }
}
void common_cmpxchge16r16_lock(CPU* cpu, U32 address, U32 srcReg) {
    cpu->dst.u16 = AX;
    cpu->src.u16 = cpu->memory->readw(address);
    cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    cpu->lazyFlags = FLAGS_CMP16;

    LockData16* p = (LockData16*)cpu->memory->getRamPtr(address, 2, true);
    std::atomic_ref<U16> mem(p->data);

    U16 expected = AX;
    U16 desired = cpu->reg[srcReg].u16;

    if (!mem.compare_exchange_strong(expected, desired)) {
        AX = expected;
        cpu->src.u16 = expected;
        cpu->result.u16 = cpu->dst.u16 - cpu->src.u16;
    }
}
void common_cmpxchge8r8_lock(CPU* cpu, U32 address, U32 srcReg) {
    cpu->dst.u8 = AL;
    cpu->src.u8 = cpu->memory->readb(address);
    cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    cpu->lazyFlags = FLAGS_CMP8;

    LockData8* p = (LockData8*)cpu->memory->getRamPtr(address, 1, true);
    std::atomic_ref<U8> mem(p->data);
    U8 expected = AL;
    U8 desired = *cpu->reg8[srcReg];

    if (!mem.compare_exchange_strong(expected, desired)) {
        AL = expected;
        cpu->src.u8 = expected;
        cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
    }
}
#endif