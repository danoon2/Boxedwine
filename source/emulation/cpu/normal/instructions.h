#ifndef __INSTRUCTIONS_H__
#define __INSTRUCTIONS_H__

// returns 0 if it eip was changed because of an exception
U32 div8(CPU* cpu, U8 src);
U32 idiv8(CPU* cpu, S8 src);
U32 div16(CPU* cpu, U16 src);
U32 idiv16(CPU* cpu, S16 src);
U32 div32(CPU* cpu, U32 src);
U32 idiv32(CPU* cpu, S32 src);

void dshlr16r16(CPU* cpu, U32 reg, U32 rm, U32 imm);
void dshle16r16(CPU* cpu, U32 reg, U32 address, U32 imm);
void dshlr32r32(CPU* cpu, U32 reg, U32 rm, U32 imm);
void dshle32r32(CPU* cpu, U32 reg, U32 address, U32 imm);
void dshlclr16r16(CPU* cpu, U32 reg, U32 rm);
void dshlcle16r16(CPU* cpu, U32 reg, U32 address);
void dshlclr32r32(CPU* cpu, U32 reg, U32 rm);
void dshlcle32r32(CPU* cpu, U32 reg, U32 address);
void dshrr16r16(CPU* cpu, U32 reg, U32 rm, U32 imm);
void dshre16r16(CPU* cpu, U32 reg, U32 address, U32 imm);
void dshrr32r32(CPU* cpu, U32 reg, U32 rm, U32 imm);
void dshre32r32(CPU* cpu, U32 reg, U32 address, U32 imm);
void dshrclr16r16(CPU* cpu, U32 reg, U32 rm);
void dshrcle16r16(CPU* cpu, U32 reg, U32 address);
void dshrclr32r32(CPU* cpu, U32 reg, U32 rm);
void dshrcle32r32(CPU* cpu, U32 reg, U32 address);

void daa(CPU* cpu);
void das(CPU* cpu);
void aaa(CPU* cpu);
void aas(CPU* cpu);
void aad(CPU* cpu, U32 value);
U32 aam(CPU* cpu, U32 value);
#endif