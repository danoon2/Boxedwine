void common_cmpxchgr8r8(CPU* cpu, U32 dstReg, U32 srcReg);
void common_cmpxchge8r8(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchgr16r16(CPU* cpu, U32 dstReg, U32 srcReg);
void common_cmpxchge16r16(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchgr32r32(CPU* cpu, U32 dstReg, U32 srcReg);
void common_cmpxchge32r32(CPU* cpu, U32 address, U32 srcReg);

#ifdef BOXEDWINE_MULTI_THREADED
void common_cmpxchge32r32_lock(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchge16r16_lock(CPU* cpu, U32 address, U32 srcReg);
void common_cmpxchge8r8_lock(CPU* cpu, U32 address, U32 srcReg);
#endif