#ifndef __ARMV8BTOPS_SSE_MINMAX_H__
#define __ARMV8BTOPS_SSE_MINMAX_H__

void opMaxpsXmm(Armv8btAsm* data);
void opMaxpsE128(Armv8btAsm* data);
void opMaxssXmm(Armv8btAsm* data);
void opMaxssE32(Armv8btAsm* data);
void opMinpsXmm(Armv8btAsm* data);
void opMinpsE128(Armv8btAsm* data);
void opMinssXmm(Armv8btAsm* data);
void opMinssE32(Armv8btAsm* data);
void opMaxpdXmmXmm(Armv8btAsm* data);
void opMaxpdXmmE128(Armv8btAsm* data);
void opMaxsdXmmXmm(Armv8btAsm* data);
void opMaxsdXmmE64(Armv8btAsm* data);
void opMinpdXmmXmm(Armv8btAsm* data);
void opMinpdXmmE128(Armv8btAsm* data);
void opMinsdXmmXmm(Armv8btAsm* data);
void opMinsdXmmE64(Armv8btAsm* data);

#endif