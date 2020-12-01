#ifndef __ARMV8BTOPS_SSE_SHUFFLE_H__
#define __ARMV8BTOPS_SSE_SHUFFLE_H__

void opPshufwMmxMmx(Armv8btAsm* data);
void opPshufwMmxE64(Armv8btAsm* data);
void opPshufdXmmXmm(Armv8btAsm* data);
void opPshufdXmmE128(Armv8btAsm* data);
void opPshufhwXmmXmm(Armv8btAsm* data);
void opPshufhwXmmE128(Armv8btAsm* data);
void opPshuflwXmmXmm(Armv8btAsm* data);
void opPshuflwXmmE128(Armv8btAsm* data);
void opShufpdXmmXmm(Armv8btAsm* data);
void opShufpdXmmE128(Armv8btAsm* data);
void opShufpsXmmXmm(Armv8btAsm* data);
void opShufpsXmmE128(Armv8btAsm* data);
#endif