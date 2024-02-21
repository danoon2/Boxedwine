#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Ops.h"
#include "x64Data.h"
#include "x64Asm.h"
#include "../normal/normal_strings.h"
#include "x64CPU.h"

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)

static U32 segOffset[] = {CPU_OFFSET_ES, CPU_OFFSET_CS, CPU_OFFSET_SS, CPU_OFFSET_DS, CPU_OFFSET_FS, CPU_OFFSET_GS, 0, 0};

// push es 0x06 removed
// pop es 0x07 removed
// push cs 0x0e removed
// push ss 0x16 removed
// pop ss 0x17 removed
// push ds 0x1e removed
// pop ds 0x1f removed
// DAA 0x27 removed
// DAS 0x2f removed
// AAA 0x37 removed
// AAS 0x3f removed
// inc/dev 0x40-0x4f removed
// pusha 0x60 removed
// popa 0x61 removed
// bound 0x62 removed
// Grpl Eb,Ib 0x82 mirror of 0x80
// call Ap 0x9a removed
// LES 0xc4 removed
// LDS 0xc5 removed
// AAM Ib 0xd4 removed
// AAD Ib 0xd5 removed
// SALC 0xd6 removed
// JMP Ap 0xea removed

// ADD Eb,Gb
// OR Eb,Gb
// ADC Eb,Gb
// SBB Eb,Gb
// AND Eb,Gb
// SUB Eb,Gb
// XOR Eb,Gb
// CMP Eb,Gb
// CMP Gb,Eb
// TEST Eb,Gb
// MOV Eb,Gb
// SETO
// SETNO
// SETB
// SETNB
// SETZ
// SETNZ
// SETBE
// SETNBE
// SETS
// SETNS
// SETP
// SETNP
// SETL
// SETNL
// SETLE
// SETNLE
static U32 inst8RM(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, true, true, 0);
    return 0;
}

// ADD Gb,Eb
// OR Gb,Eb
// ADC Gb,Eb
// SBB Gb,Eb
// AND Gb,Eb
// SUB Gb,Eb
// XOR Gb,Eb
// MOV Gb,Eb
// XCHG Eb,Gb
static U32 inst8RMGWritten(X64Asm* data) {
    data->isG8bitWritten = true;
    data->translateRM(data->fetch8(), true, true, true, true, 0);
    return 0;
}

// ADD Ew,Gw
// ADD Gw,Ew
// OR Ew,Gw
// OR Gw,Ew
// ADC Ew,Gw
// ADC Gw,Ew
// SBB Ew,Gw
// SBB Gw,Ew
// AND Ew,Gw
// AND Gw,Ew
// SUB Ew,Gw
// SUB Gw,Ew
// XOR Ew,Gw
// XOR Gw,Ew
// CMP Ew,Gw
// CMP Gw,Ew
// TEST Ew,Gw
// XCHG Ew,Gw
// MOV Ew,Gw
// MOV Gw,Ew
// CMOVO
// CMOVNO
// CMOVB
// CMOVNB
// CMOVZ
// CMOVNZ
// CMOVBE
// CMOVNBE
// CMOVS
// CMOVNS
// CMOVP
// CMOVNP
// CMOVL
// CMOVNL
// CMOVLE
// CMOVNLE
// DSHLCL Ew,Gw
// BTS Ew,Gw
// DSHRCL Ew,Gw
// DIMUL Gw,Ew
// CMPXCHG Ew,Gw
static U32 inst16RM(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, false, 0);
    return 0;
}

// BT Ew,Gw
// BSR Gw,Ew
// BTS Ew,Gw (Roller Coaster Tycoon demo)
// BSF Gw,Ew (Roller Coaster Tycoon demo)
static U32 inst16RMBit(X64Asm* data) {
    U8 rm = data->fetch8();

#ifndef __TEST
    if (rm < 0xC0 && data->ea16) {
        static bool hasWarned = false;
        if (!hasWarned) {
            // see common_bte16r16 for how this should be implemented
            // 
            // I couldn't find any app/game that needed this, so I delayed fixing it because it would cause a refacting in how memory addresses are calculated
            klog("X64: 16-bit memory bit instructions might not be handled correctly.  Please report which app/game caused this");
            hasWarned = true;
        }
    }
#endif
    data->translateRM(rm, true, true, false, false, 0);    
    return 0;
}

// GRP2 Eb,1
// GRP2 Eb,CL
static U32 inst8RMSafeG(X64Asm* data) {
    U8 rm = data->fetch8();
    data->translateRM(rm, false, true, false, true, 0);
    return 0;
}

// GRP4 Eb
static U32 instGrp4(X64Asm* data) {
    U8 rm = data->fetch8();
    if (G(rm)==7) {        
        void* pfn = (void*)data->fetch64();
        data->emulateSingleOp(data->currentOp);
        data->done = true;
        return 0;
    }
    data->translateRM(rm, false, true, false, true, 0);
    return 0;
}

// MOVXZ8 Gw,Eb
// MOVSX8 Gw,Eb
static U32 inst16E8RM(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, true, 0);
    return 0;
}

// MOVXZ8 Gd,Eb
// MOVSX8 Gd,Eb
static U32 inst32E8RM(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, true, 0);
    return 0;
}

// MOVXZ16 Gd,Ew
// MOVXS16 Gd,Ew
static U32 inst32E16RM(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, false, 0);
    return 0;
}

// IMUL Gw,Ew,Iw
static U32 inst16RMimm16(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, false, 16);
    return 0;
}

// Grpl Ew,Iw
static U32 inst16RMimm16SafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 16);
    return 0;
}

// IMUL Gd,Ed,Id
// MOV ED,ID
static U32 inst32RMimm32(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, false, 32);
    return 0;
}

// Grpl Ed,Id
static U32 inst32RMimm32SafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 32);
    return 0;
}

// Grpl Eb,Ib
// GRP2 Eb,Ib
static U32 inst8RMimm8SafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, true, 8);
    return 0;
}

// MOV EB,IB
static U32 inst8RMimm8(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, true, 8);
    return 0;
}

// IMUL Gw,Ew,Ib
// DSHL Ew,Gw
// DSHR Ew,Gw
static U32 inst16RMimm8(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, false, 8);
    return 0;
}

// Grpl Ew,Ix
// GRP2 Ew,Ib
static U32 inst16RMimm8SafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 8);
    return 0;
}

// IMUL Gd,Ed,Ib
// DSHL Ed,Gd
// DSHR Ed,Gd
static U32 inst32RMimm8(X64Asm* data) {
    return inst16RMimm8(data);
}

// Grpl Ed,Ix
// GRP2 Ed,Ib
// GRP8 Ed,Ib
static U32 inst32RMimm8SafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 8);
    return 0;
}

// GRP2 Ed,1
// GRP2 Ed,CL
static U32 inst32RMSafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 0);
    return 0;
}

// ADD Al,Ib
// OR Al,Ib
// ADC Al,Ib
// SBB Al,Ib
// AND Al,Ib
// SUB Al,Ib
// XOR Al,Ib
// CMP Al,Ib
static U32 arithR8Ib(X64Asm* data) {
    data->setImmediate8(data->fetch8());
    data->writeOp();
    return 0;
}

// ADD Ax,Iw
// OR Ax,Iw
// ADC Ax,Iw
// SBB Ax,Iw
// AND Ax,Iw
// SUB Ax,Iw
// XOR Ax,Iw
// CMP Ax,Iw
static U32 arithR16Iw(X64Asm* data) {
    data->setImmediate16(data->fetch16());
    data->writeOp();
    return 0;
}

// ADD Eax,Id
// OR Eax,Id
// ADC Eax,Id
// SBB Eax,Id
// AND Eax,Id
// SUB Eax,Id
// XOR Eax,Id
// CMP Eax,Id
static U32 arithR32Id(X64Asm* data) {
    data->setImmediate32(data->fetch32());
    data->writeOp();
    return 0;
}

// XLAT
static U32 xlat(X64Asm* data) {
    data->xlat();
    return 0;
}

// PUSH ES
static U32 push16ES(X64Asm* data) {    
    data->pushCpuOffset16(CPU_OFFSET_ES);
    return 0;
}

static void popSeg16(X64Asm* data, U32 seg) {
    // setSegment might throw an exception so we need to sync the regs
    data->popSeg(seg, 2);
    data->cpu->thread->process->hasSetSeg[seg] = true;
}

static void popSeg32(X64Asm* data, U32 seg) {
    data->popSeg(seg, 4);
}

// POP ES
static U32 pop16ES(X64Asm* data) {
    popSeg16(data, ES);
    return 0;
}

// PUSH ES
static U32 push32ES(X64Asm* data) {
    data->pushCpuOffset32(CPU_OFFSET_ES);
    return 0;
}

// POP ES
static U32 pop32ES(X64Asm* data) {
    popSeg32(data, ES);
    return 0;
}

// PUSH CS
static U32 push16CS(X64Asm* data) {
    data->pushCpuOffset16(CPU_OFFSET_CS);
    return 0;
}

// PUSH CS
static U32 push32CS(X64Asm* data) {
    data->pushCpuOffset32(CPU_OFFSET_CS);
    return 0;
}

// PUSH SS
static U32 push16SS(X64Asm* data) {
    data->pushCpuOffset16(CPU_OFFSET_SS);
    return 0;
}

// POP SS
static U32 pop16SS(X64Asm* data) {
    popSeg16(data, SS);
    return 0;
}

// PUSH SS
static U32 push32SS(X64Asm* data) {
    data->pushCpuOffset32(CPU_OFFSET_SS);
    return 0;
}

// POP SS
static U32 pop32SS(X64Asm* data) {
    popSeg32(data, SS);
    return 0;
}

// PUSH DS
static U32 push16DS(X64Asm* data) {
    data->pushCpuOffset16(CPU_OFFSET_DS);
    return 0;
}

// POP DS
static U32 pop16DS(X64Asm* data) {
    popSeg16(data, DS);
    return 0;
}

// PUSH DS
static U32 push32DS(X64Asm* data) {
    data->pushCpuOffset32(CPU_OFFSET_DS);
    return 0;
}

// POP DS
static U32 pop32DS(X64Asm* data) {
    popSeg32(data, DS);
    return 0;
}

// PUSH FS
static U32 push16FS(X64Asm* data) {
    data->pushCpuOffset16(CPU_OFFSET_FS);
    return 0;
}

// POP FS
static U32 pop16FS(X64Asm* data) {
    popSeg16(data, FS);
    return 0;
}

// PUSH FS
static U32 push32FS(X64Asm* data) {
    data->pushCpuOffset32(CPU_OFFSET_FS);
    return 0;
}

// POP FS
static U32 pop32FS(X64Asm* data) {
    popSeg32(data, FS);
    return 0;
}

// PUSH GS
static U32 push16GS(X64Asm* data) {
    data->pushCpuOffset16(CPU_OFFSET_GS);
    return 0;
}

// POP GS
static U32 pop16GS(X64Asm* data) {
    popSeg16(data, GS);
    return 0;
}

// PUSH GS
static U32 push32GS(X64Asm* data) {
    data->pushCpuOffset32(CPU_OFFSET_GS);
    return 0;
}

// POP GS
static U32 pop32GS(X64Asm* data) {
    popSeg32(data, GS);
    return 0;
}

// DAA
static U32 daa(X64Asm* data) {
    data->daa();
    return 0;
}

// DAS
static U32 das(X64Asm* data) {
    data->das();
    return 0;
}

// AAA
static U32 aaa(X64Asm* data) {
    data->aaa();
    return 0;
}

// AAS
static U32 aas(X64Asm* data) {
    data->aas();
    return 0;
}

// AAM Ib
static U32 aam(X64Asm* data) {
    data->aam(data->fetch8());
    return 0;
}

// AAD Ib
static U32 aad(X64Asm* data) {
    data->aad(data->fetch8());
    return 0;
}

// SALC
static U32 salc(X64Asm* data) {
    data->salc();
    return 0;
}

// INC AX
static U32 incAX(X64Asm* data) {
    data->incReg(0, false, 2);
    return 0;
}

// INC CX
static U32 incCX(X64Asm* data) {
    data->incReg(1, false, 2);
    return 0;
}

// INC DX
static U32 incDX(X64Asm* data) {
    data->incReg(2, false, 2);
    return 0;
}

// INC BX
static U32 incBX(X64Asm* data) {
    data->incReg(3, false, 2);
    return 0;
}

// INC SP
static U32 incSP(X64Asm* data) {
    data->incReg(HOST_ESP, true, 2);
    return 0;
}

// INC BP
static U32 incBP(X64Asm* data) {
    data->incReg(5, false, 2);
    return 0;
}

// INC SI
static U32 incSI(X64Asm* data) {
    data->incReg(6, false, 2);
    return 0;
}

// INC DI
static U32 incDI(X64Asm* data) {
    data->incReg(7, false, 2);
    return 0;
}

// DEC AX
static U32 decAX(X64Asm* data) {
    data->decReg(0, false, 2);
    return 0;
}

// DEC CX
static U32 decCX(X64Asm* data) {
    data->decReg(1, false, 2);
    return 0;
}

// DEC DX
static U32 decDX(X64Asm* data) {
    data->decReg(2, false, 2);
    return 0;
}

// DEC BX
static U32 decBX(X64Asm* data) {
    data->decReg(3, false, 2);
    return 0;
}

// DEC SP
static U32 decSP(X64Asm* data) {
    data->decReg(HOST_ESP, true, 2);
    return 0;
}

// DEC BP
static U32 decBP(X64Asm* data) {
    data->decReg(5, false, 2);
    return 0;
}

// DEC SI
static U32 decSI(X64Asm* data) {
    data->decReg(6, false, 2);
    return 0;
}

// DEC DI
static U32 decDI(X64Asm* data) {
    data->decReg(7, false, 2);
    return 0;
}

// INC EAX
static U32 incEAX(X64Asm* data) {
    data->incReg(0, false, 4);
    return 0;
}

// INC ECX
static U32 incECX(X64Asm* data) {
    data->incReg(1, false, 4);
    return 0;
}

// INC EDX
static U32 incEDX(X64Asm* data) {
    data->incReg(2, false, 4);
    return 0;
}

// INC EBX
static U32 incEBX(X64Asm* data) {
    data->incReg(3, false, 4);
    return 0;
}

// INC ESP
static U32 incESP(X64Asm* data) {
    data->incReg(HOST_ESP, true, 4);
    return 0;
}

// INC EBP
static U32 incEBP(X64Asm* data) {
    data->incReg(5, false, 4);
    return 0;
}

// INC ESI
static U32 incESI(X64Asm* data) {
    data->incReg(6, false, 4);
    return 0;
}

// INC EDI
static U32 incEDI(X64Asm* data) {
    data->incReg(7, false, 4);
    return 0;
}

// DEC EAX
static U32 decEAX(X64Asm* data) {
    data->decReg(0, false, 4);
    return 0;
}

// DEC ECX
static U32 decECX(X64Asm* data) {
    data->decReg(1, false, 4);
    return 0;
}

// DEC EDX
static U32 decEDX(X64Asm* data) {
    data->decReg(2, false, 4);
    return 0;
}

// DEC EBX
static U32 decEBX(X64Asm* data) {
    data->decReg(3, false, 4);
    return 0;
}

// DEC ESP
static U32 decESP(X64Asm* data) {
    data->decReg(HOST_ESP, true, 4);
    return 0;
}

// DEC EBP
static U32 decEBP(X64Asm* data) {
    data->decReg(5, false, 4);
    return 0;
}

// DEC ESI
static U32 decESI(X64Asm* data) {
    data->decReg(6, false, 4);
    return 0;
}

// DEC EDI
static U32 decEDI(X64Asm* data) {
    data->decReg(7, false, 4);
    return 0;
}

// PUSH AX
static U32 pushAX(X64Asm* data) {
    data->pushReg16(0, false);
    return 0;
}

// PUSH CX
static U32 pushCX(X64Asm* data) {
    data->pushReg16(1, false);
    return 0;
}

// PUSH DX
static U32 pushDX(X64Asm* data) {
    data->pushReg16(2, false);
    return 0;
}

// PUSH BX
static U32 pushBX(X64Asm* data) {
    data->pushReg16(3, false);
    return 0;
}

// PUSH SP
static U32 pushSP(X64Asm* data) {
    data->pushReg16(HOST_ESP, true);
    return 0;
}

// PUSH BP
static U32 pushBP(X64Asm* data) {
    data->pushReg16(5, false);
    return 0;
}

// PUSH SI
static U32 pushSI(X64Asm* data) {
    data->pushReg16(6, false);
    return 0;
}

// PUSH DI
static U32 pushDI(X64Asm* data) {
    data->pushReg16(7, false);
    return 0;
}

// POP AX
static U32 popAX(X64Asm* data) {
    data->popReg16(0, false);
    return 0;
}

// POP CX
static U32 popCX(X64Asm* data) {
    data->popReg16(1, false);
    return 0;
}

// POP DX
static U32 popDX(X64Asm* data) {
    data->popReg16(2, false);
    return 0;
}

// POP BX
static U32 popBX(X64Asm* data) {
    data->popReg16(3, false);
    return 0;
}

// POP SP
static U32 popSP(X64Asm* data) {
    data->popReg16(HOST_ESP, true);
    return 0;
}

// POP BP
static U32 popBP(X64Asm* data) {
    data->popReg16(5, false);
    return 0;
}

// POP SI
static U32 popSI(X64Asm* data) {
    data->popReg16(6, false);
    return 0;
}

// POP DI
static U32 popDI(X64Asm* data) {
    data->popReg16(7, false);
    return 0;
}

// PUSH EAX
static U32 pushEAX(X64Asm* data) {
    data->pushReg32(0, false);
    return 0;
}

// PUSH ECX
static U32 pushECX(X64Asm* data) {
    data->pushReg32(1, false);
    return 0;
}

// PUSH EDX
static U32 pushEDX(X64Asm* data) {
    data->pushReg32(2, false);
    return 0;
}

// PUSH EBX
static U32 pushEBX(X64Asm* data) {
    data->pushReg32(3, false);
    return 0;
}

// PUSH ESP
static U32 pushESP(X64Asm* data) {
    data->pushReg32(HOST_ESP, true);
    return 0;
}

// PUSH EBP
static U32 pushEBP(X64Asm* data) {
    data->pushReg32(5, false);
    return 0;
}

// PUSH ESI
static U32 pushESI(X64Asm* data) {
    data->pushReg32(6, false);
    return 0;
}

// PUSH EDI
static U32 pushEDI(X64Asm* data) {
    data->pushReg32(7, false);
    return 0;
}

// POP EAX
static U32 popEAX(X64Asm* data) {
    data->popReg32(0, false);
    return 0;
}

// POP ECX
static U32 popECX(X64Asm* data) {
    data->popReg32(1, false);
    return 0;
}

// POP EDX
static U32 popEDX(X64Asm* data) {
    data->popReg32(2, false);
    return 0;
}

// POP EBX
static U32 popEBX(X64Asm* data) {
    data->popReg32(3, false);
    return 0;
}

// POP ESP
static U32 popESP(X64Asm* data) {
    data->popReg32(HOST_ESP, true);
    return 0;
}

// POP EBP
static U32 popEBP(X64Asm* data) {
    data->popReg32(5, false);
    return 0;
}

// POP ESI
static U32 popESI(X64Asm* data) {
    data->popReg32(6, false);
    return 0;
}

// POP EDI
static U32 popEDI(X64Asm* data) {
    data->popReg32(7, false);
    return 0;
}

// PUSHA
static U32 pushA16(X64Asm* data) {
    data->pushA16();
    return 0;
}

// POPA
static U32 popA16(X64Asm* data) {
    data->popA16();
    return 0;
}

// PUSHAD
static U32 pushA32(X64Asm* data) {
    data->pushA32();
    return 0;
}

// POPAD
static U32 popA32(X64Asm* data) {
    data->popA32();
    return 0;
}

// PUSH Iw
static U32 pushIw16(X64Asm* data) {
    data->pushw(data->fetch16());
    return 0;
}

// PUSH Ib
static U32 pushIb16(X64Asm* data) {
    data->pushw((S8)data->fetch8());
    return 0;
}

// PUSH Id
static U32 pushId32(X64Asm* data) {
    data->pushd(data->fetch32());
    return 0;
}

// PUSH Ib
static U32 pushIb32(X64Asm* data) {
    data->pushd((S8)data->fetch8());
    return 0;
}

// BOUND
static U32 bound16(X64Asm* data) {
    data->bound16(data->fetch8());
    return 0;
}

// BOUND
static U32 bound32(X64Asm* data) {   
    data->bound32(data->fetch8());
    return 0;
}

// JO, JNO, JB, JNB, JZ, JNZ, JBE, JNBE, JS, JNS, JP, JNP, JL, JNL, JNLE
static U32 jump8(X64Asm* data) {
    S8 offset = (S8)data->fetch8();
    data->jumpConditional(data->op & 0xf, data->ip+offset);    
    return 0;
}

// JO, JNO, JB, JNB, JZ, JNZ, JBE, JNBE, JS, JNS, JP, JNP, JL, JNL, JNLE
static U32 jump16(X64Asm* data) {
    S16 offset = (S16)data->fetch16();
    data->jumpConditional(data->op & 0xf, data->ip+offset);    
    return 0;
}

static U32 jump32(X64Asm* data) {
    S32 offset = (S32)data->fetch32();
    data->jumpConditional(data->op & 0xf, data->ip+offset);    
    return 0;
}

// LOOPNZ
static U32 x64loopnz(X64Asm* data) {
    S8 offset = (S8)data->fetch8();
    data->loopnz(data->ip+offset, data->ea16);
    return 0;
}

// LOOPZ
static U32 x64loopz(X64Asm* data) {
    S8 offset = (S8)data->fetch8();
    data->loopz(data->ip+offset, data->ea16);
    return 0;
}

// LOOP
static U32 x64loop(X64Asm* data) {
    S8 offset = (S8)data->fetch8();
    data->loop(data->ip+offset, data->ea16);
    return 0;
}

// JCXZ
static U32 x64jcxz(X64Asm* data) {
    S8 offset = (S8)data->fetch8();
    data->jcxz(data->ip+offset, data->ea16);
    return 0;
}

// CALL Jw 
static U32 callJw(X64Asm* data) {
    U16 offset = data->fetch16();
    U32 eip = (data->ip+offset) & 0xFFFF;    
    data->pushw(data->ip); // will return to next instruction
    data->jumpTo(eip);
    data->done = true;
    return 0;
}

// CALL Jd 
static U32 callJd(X64Asm* data) {
    S32 offset = data->fetch32();
    U32 eip = data->ip+offset;    
    data->pushd(data->ip); // will return to next instruction
    data->jumpTo(eip);
    data->done = true;
    return 0;
}

// JMP Jw 
static U32 jmpJw(X64Asm* data) {
    S16 offset = (S16)data->fetch16();
    data->jumpTo(data->ip+offset);
    data->done = true;
    return 0;
}

// JMP Jb
static U32 jmpJb(X64Asm* data) {
    S8 offset = (S8)data->fetch8();
    data->jumpTo(data->ip+offset);
    data->done = true;
    return 0;
}

// JMP Jd 
static U32 jmpJd(X64Asm* data) {
    S32 offset = (S32)data->fetch32();
    data->jumpTo(data->ip+offset);
    data->done = true;
    return 0;
}

// JMP Ap
static U32 jmpAp(X64Asm* data) {
    U16 offset = data->fetch16();
    U16 sel = data->fetch16();
    data->jmp(false, sel, offset, data->ip);
    data->done = true;
    return 0;
}

// CALL Ap
static U32 callAp(X64Asm* data) {
    U16 offset = data->fetch16();
    U16 sel = data->fetch16();
    data->call(false, sel, offset, data->ip);
    data->done = true;
    return 0;
}

static U32 callFar32(X64Asm* data) {
    U32 offset = data->fetch32();
    U16 sel = data->fetch16();
    data->call(true, sel, offset, data->ip);
    data->done = true;
    return 0;
}

static U32 jmpFar32(X64Asm* data) {
    U32 offset = data->fetch32();
    U16 sel = data->fetch16();
    data->jmp(true, sel, offset, data->ip);
    data->done = true;
    return 0;
}

// RETN Iw
static U32 retn16Iw(X64Asm* data) {
    data->retn16(data->fetch16());
    data->done = true;
    return 0;
}

// RETN Iw
static U32 retn32Iw(X64Asm* data) {
    data->retn32(data->fetch16());
    data->done = true;
    return 0;
}

// RETN16
static U32 retn16(X64Asm* data) {
    data->retn16(0);
    data->done = true;
    return 0;
}

// RETN32
static U32 retn32(X64Asm* data) {
    data->retn32(0);
    data->done = true;
    return 0;
}

// RETF Iw
static U32 retf16Iw(X64Asm* data) {
    data->retf(0, data->fetch16());
    data->done = true;
    return 0;
}

// RETF Iw
static U32 retf32Iw(X64Asm* data) {
    data->retf(1, data->fetch16());
    data->done = true;
    return 0;
}

// RETF
static U32 retf16(X64Asm* data) {
    data->retf(0, 0);
    data->done = true;
    return 0;
}

// RETF
static U32 retf32(X64Asm* data) {
    data->retf(1, 0);
    data->done = true;
    return 0;
}

// IRET
static U32 iret(X64Asm* data) {
    data->iret(0, data->ip);
    data->done = true;
    return 0;
}

// IRET
static U32 iret32(X64Asm* data) {
    data->iret(1, data->ip);
    data->done = true;
    return 0;
}

// INT 3
static U32 int3(X64Asm* data) {
    data->signalTrap(1); // 1=TRAP_BRKPT
    data->done = true;
    return 0;
}

// INT Ib
static U32 intIb(X64Asm* data) {
    U8 i = data->fetch8();
    if (i==0x80) {
        data->syscall(data->ip-data->startOfOpIp);
    } else if (i==0x98) {
        data->int98(data->ip-data->startOfOpIp);
    } else if (i==0x99) {
        data->int99(data->ip-data->startOfOpIp);
    } else if (i == 0x9A) {
        data->int9A(data->ip - data->startOfOpIp);
    }
#ifdef __TEST
    else if (i==0x97) {
        data->addReturnFromTest();
        data->done = true;
    }
#endif
    else {
        data->signalIllegalInstruction(5);  // 5=ILL_PRVOPC
        data->done = true;
    }
    return 0;
}

// Mov Ew,Sw
static U32 movEwSw(X64Asm* data) {
    U8 rm = data->fetch8();
    data->writeToEFromCpuOffset(rm, segOffset[((rm >> 3) & 7)], 4, 2);
    return 0;
}

// Mov Ed,Sw
static U32 movEdSw(X64Asm* data) {
    U8 rm = data->fetch8();
    data->writeToEFromCpuOffset(rm, segOffset[((rm >> 3) & 7)], 4, 4);
    return 0;
}

// MOV Sw,Ew
static U32 movSwEw(X64Asm* data) {
    U8 rm = data->fetch8();    
    U8 seg = (rm >> 3) & 7;
    data->setSeg(seg, rm);
    return 0;
}

// LEA Gw
static U32 leaGw(X64Asm* data) {
    data->ds = SEG_ZERO;
    data->ss = SEG_ZERO;
    data->translateRM(data->fetch8(), true, true, false, false, 0);
    return 0;
}

// LEA Gd
static U32 leaGd(X64Asm* data) {
    data->ds = SEG_ZERO;
    data->ss = SEG_ZERO;
    data->translateRM(data->fetch8(), true, true, false, false, 0);
    return 0;
}

// POP Ew
static U32 popEw(X64Asm* data) {
    U8 rm = data->fetch8();
    data->popw(rm);
    return 0;
}

// POP Ed
static U32 popEd(X64Asm* data) {
    U8 rm = data->fetch8();
    data->popd(rm);
    return 0;
}

// NOP
// XCHG CX,AX
// XCHG DX,AX
// XCHG BX,AX
// XCHG BP,AX
// XCHG SI,AX
// XCHG DI,AX
// CBW
// CWD
// Wait
// SAHF
// LAHF
// CMC
// CLC
// STC
// CLI
// STI
// CLD
// STD
// RDTSC
// XCHG ECX,EAX
// XCHG EDX,EAX
// XCHG EBX,EAX
// XCHG ESP,EAX
// XCHG EBP,EAX
// XCHG ESI,EAX
// XCHG EDI,EAX
// CBWE
// CWQ
static U32 keepSame(X64Asm* data) {
    data->writeOp();
    return 0;
}

static U32 rdtsc(X64Asm* data) {
#ifdef LOG_OPS
    x64_writeToRegFromValue(data, 0, FALSE, 1, 4);
    x64_writeToRegFromValue(data, 2, FALSE, 0, 4);
#else
    data->writeOp();
#endif
    return 0;
}

// TEST AL,Ib
// MOV AL,Ib
// MOV CL,Ib
// MOV DL,Ib
// MOV BL,Ib
// MOV AH,Ib
// MOV CH,Ib
// MOV DH,Ib
// MOV BH,Ib
static U32 keepSameImm8(X64Asm* data) {
    data->setImmediate8(data->fetch8());
    data->writeOp();
    return 0;
}

// TEST AX,Iw
// MOV AX,Iw
// MOV CX,Iw
// MOV DX,Iw
// MOV BX,Iw
// MOV BP,Iw
// MOV SI,Iw
// MOV DI,Iw
static U32 keepSameImm16(X64Asm* data) {
    data->setImmediate16(data->fetch16());
    data->writeOp();
    return 0;
}

// TEST EAX,Id
// MOV EAX,Id
// MOV ECX,Id
// MOV EDX,Id
// MOV EBX,Id
// MOV EBP,Id
// MOV ESI,Id
// MOV EDI,Id
static U32 keepSameImm32(X64Asm* data) {
    data->setImmediate32(data->fetch32());
    data->writeOp();
    return 0;
}

// XCHG SP,AX
static U32 xchgSpAx(X64Asm* data) {
    data->writeXchgSpAx();
    return 0;
}

static U32 xchgEspEax(X64Asm* data) {
    data->writeXchgEspEax();    
    return 0;
}

// PUSHF16
static U32 pushFlags16(X64Asm* data) {
    data->pushfw();
    return 0;
}

// PUSHF32
static U32 pushFlags32(X64Asm* data) {
    data->pushfd();
    return 0;
}

// POPF16
static U32 popFlags16(X64Asm* data) {
    data->popfw();
    return 0;
}

// POPF32
static U32 popFlags32(X64Asm* data) {
    data->popfd();
    return 0;
}

// MOV AL,Ob
static U32 movAlOb(X64Asm* data) {
    U32 disp = 0;
    if (data->ea16) {
        disp = data->fetch16();
    } else {
        disp = data->fetch32();
    }
    data->writeToRegFromMemAddress(data->ds, 0, false, disp, 1);
    return 0;
}

// MOV AX,Ow
static U32 movAxOw(X64Asm* data) {
    U32 disp = 0;
    if (data->ea16) {
        disp = data->fetch16();
    } else {
        disp = data->fetch32();
    }
    if ((disp & 1) != 0) {
        data->emulateSingleOp(data->currentOp);
        data->done = true;
    } else {
        data->writeToRegFromMemAddress(data->ds, 0, false, disp, 2);
    }
    return 0;
}

// MOV EAX,Od
static U32 movEaxOd(X64Asm* data) {
    U32 disp = 0;
    if (data->ea16) {
        disp = data->fetch16();
    } else {
        disp = data->fetch32();
    }
    if ((disp & 3) != 0) {
        data->emulateSingleOp(data->currentOp);
        data->done = true;
    } else {
        data->writeToRegFromMemAddress(data->ds, 0, false, disp, 4);
    }
    return 0;
}

// MOV Ob,Al
static U32 movObAl(X64Asm* data) {
    U32 disp = 0;
    if (data->ea16) {
        disp = data->fetch16();
    } else {
        disp = data->fetch32();
    }    
    data->writeToMemAddressFromReg(data->ds, 0, false, disp, 1);
    return 0;
}

// MOV Ow,Ax
static U32 movOwAx(X64Asm* data) {
    U32 disp = 0;
    if (data->ea16) {
        disp = data->fetch16();
    } else {
        disp = data->fetch32();
    }
    if ((disp & 1) != 0) {
        data->emulateSingleOp(data->currentOp);
        data->done = true;
    } else {
        data->writeToMemAddressFromReg(data->ds, 0, false, disp, 2);
    }
    return 0;
}

// MOV Od,Eax
static U32 movOdEax(X64Asm* data) {
    U32 disp = 0;
    if (data->ea16) {
        disp = data->fetch16();
    } else {
        disp = data->fetch32();
    }
    if ((disp & 3) != 0) {
        data->emulateSingleOp(data->currentOp);
        data->done = true;
    } else {
        data->writeToMemAddressFromReg(data->ds, 0, false, disp, 4);
    }
    return 0;
}

static U32 bswapSp(X64Asm* data) {
    data->bswapEsp();
    return 0;
}

static U32 bswapEsp(X64Asm* data) {
    data->bswapEsp();
    return 0;
}

static U32 movsb(X64Asm* data) {
    data->movs(1);
    return 0;
}

static U32 movsw(X64Asm* data) {
    data->movs(2);
    return 0;
}

static U32 movsd(X64Asm* data) {
    data->movs(4);
    return 0;
}

static U32 cmpsb(X64Asm* data) {
    data->cmps(1);
    return 0;
}

static U32 cmpsw(X64Asm* data) {
    data->cmps(2);
    return 0;
}

static U32 cmpsd(X64Asm* data) {
    data->cmps(4);
    return 0;
}

static U32 stosb(X64Asm* data) {
    data->stos(1);
    return 0;
}

static U32 stosw(X64Asm* data) {
    data->stos(2);
    return 0;
}

static U32 stosd(X64Asm* data) {
    data->stos(4);
    return 0;
}

static U32 lodsb(X64Asm* data) {
    data->lods(1);
    return 0;
}

static U32 lodsw(X64Asm* data) {
    data->lods(2);
    return 0;
}

static U32 lodsd(X64Asm* data) {
    data->lods(4);
    return 0;
}

static U32 scasb(X64Asm* data) {
    data->scas(1);
    return 0;
}

static U32 scasw(X64Asm* data) {
    data->scas(2);
    return 0;
}

static U32 scasd(X64Asm* data) {
    data->scas(4);
    return 0;
}

// MOV SP,Iw
static U32 movSpIw(X64Asm* data) {
    data->writeToRegFromValue(HOST_ESP, true, data->fetch16(), 2);
    return 0;
}

// MOV ESP,Id
static U32 movEspId(X64Asm* data) {
    data->writeToRegFromValue(HOST_ESP, true, data->fetch32(), 4);
    return 0;
}

// LES
static U32 les16(X64Asm* data) {
    data->loadSeg(ES, data->fetch8(), false);
    return 0;
}
static U32 les32(X64Asm* data) {
    data->loadSeg(ES, data->fetch8(), true);
    return 0;
}

// LSS
static U32 lss16(X64Asm* data) {
    data->loadSeg(SS, data->fetch8(), false);
    return 0;
}
static U32 lss32(X64Asm* data) {
    data->loadSeg(SS, data->fetch8(), true);
    return 0;
}


// LFS
static U32 lfs16(X64Asm* data) {
    data->loadSeg(FS, data->fetch8(), false);
    return 0;
}
static U32 lfs32(X64Asm* data) {
    data->loadSeg(FS, data->fetch8(), true);
    return 0;
}

// LGS
static U32 lgs16(X64Asm* data) {
    data->loadSeg(GS, data->fetch8(), false);
    return 0;
}
static U32 lgs32(X64Asm* data) {
    data->loadSeg(GS, data->fetch8(), true);
    return 0;
}

// LDS
static U32 lds16(X64Asm* data) {
    data->loadSeg(DS, data->fetch8(), false);
    return 0;
}
static U32 lds32(X64Asm* data) {
    data->loadSeg(DS, data->fetch8(), true);
    return 0;
}

// ENTER
static U32 enter16(X64Asm* data) {
    // push bp
    // mov  bp, sp
    // sub  sp, bytes

    S32 bytes = (S16)data->fetch16();
    U32 level = data->fetch8() & 0x1f;

    data->enter(false, (U32)bytes, level);
    return 0;
}

// ENTER
static U32 enter32(X64Asm* data) {
    // push ebp
    // mov  ebp, esp
    // sub  esp, bytes

    S32 bytes = (S16)data->fetch16();
    U32 level = data->fetch8() & 0x1f;

    data->enter(true, (U32)bytes, level);
    return 0;
}

// LEAVE16
static U32 leave16(X64Asm* data) {
    data->leave(false);
    return 0;
}

// LEAVE32
static U32 leave32(X64Asm* data) {
    data->leave(true);
    return 0;
}

// GRP3 Eb(,Ib)
static U32 grp3b(X64Asm* data) {
    U8 rm = data->fetch8();
    U8 g = (rm >> 3) & 7;   

    data->translateRM(rm, false, true, false, true, (g==0||g==1)?8:0);
    return 0;
}

// GRP3 Ew(,Iw)
static U32 grp3w(X64Asm* data) {
    U8 rm = data->fetch8();
    U8 g = (rm >> 3) & 7;   

    data->translateRM(rm, false, true, false, false, (g==0||g==1)?16:0);
    return 0;
}

// GRP3 Ed(,Id)
static U32 grp3d(X64Asm* data) {
    U8 rm = data->fetch8();
    U8 g = (rm >> 3) & 7;   

    data->translateRM(rm, false, true, false, false, (g==0||g==1)?32:0);
    return 0;
}

// GRP5 Ew
static U32 grp5w(X64Asm* data) {
    U8 rm = data->fetch8();
    U8 g = (rm >> 3) & 7;        
    if (g==0 || g==1) { // inc/dec
        data->translateRM(rm, false, true, false, false, 0);
    } else if (g==2) { // call Ev
        data->callE(false, rm);    
        data->done = true;
    } else if (g==3) { // call Ep
        data->callFar(false, rm);
        data->done = true;
    } else if (g==4) { // jmp Ev
        data->jmpE(false, rm);
        data->done = true;
    } else if (g==5) { // jmp Ep
        data->jmpFar(false, rm);
        data->done = true;
    } else if (g==6) { // push Ev
        data->pushE16(rm);
    } else {
        kpanic("invalid grp5w");
    }    
    return 0;
}

// GRP5 Ed
static U32 grp5d(X64Asm* data) {
    U8 rm = data->fetch8();
    U8 g = (rm >> 3) & 7;
    if (g==0 || g==1) { // inc/dec
        data->translateRM(rm, false, true, false, false, 0);
    } else if (g==2) { // call near Ed
        data->callE(true, rm);     
        data->done = true;
    } else if (g==3) { // call far Ed
        data->callFar(true, rm);
        data->done = true;
    } else if (g==4) { // jmp near Ed
        data->jmpE(true, rm);
        data->done = true;
    } else if (g==5) { // jmp far Ed
        data->jmpFar(true, rm);
        data->done = true;
    } else if (g==6) { // push Ed
        data->pushE32(rm);
    } else {
        // kpanic("invalid grp5d");
        // we will just hope this doesn't get used and that we just translated more than we needed, this is the case for "kknd 2 krossfire demo"
        data->done = true;
        data->errorMsg("invalid grp5d");
    }    
    return 0;
}

// LSL
static U32 lsl(X64Asm* data) {
    data->lsl(false, data->fetch8());
    return 0;
}
static U32 lsl32(X64Asm* data) {
    data->lsl(true, data->fetch8());
    return 0;
}

// LAR
static U32 lar(X64Asm* data) {
    data->lar(false, data->fetch8());
    return 0;
}

// HLT
static U32 hlt(X64Asm* data) {
    // requires ring 0 access
    data->signalIllegalInstruction(5); // 5=ILL_PRVOPC
    data->done = true;
    return 0;
}

static U32 grp6_16(X64Asm* data) {
    U8 rm = data->fetch8();

    switch (G(rm)) {
    case 0x00:
        kpanic("SLDT not implemented");
        break;
    case 0x01:
        kpanic("STR not implemented");
        break;
    case 0x02:
        kpanic("LLDT not implemented");
        break;
    case 0x03:
        kpanic("LTR not implemented");
        break;
    case 0x04:
        data->verr(rm);
        break;
    case 0x05:
        data->verw(rm);
        break;
    default: 
        kpanic("invalid grp6");
        break;
    }	
    return 0;
}

static U32 cli(X64Asm* data) {
    data->write8(0x90); // nop
    return 0;
}

static U32 sti(X64Asm* data) {
    data->write8(0x90); // nop
    return 0;
}

static U32 movRdCrx(X64Asm* data) {
    U8 rm = data->fetch8();
    data->movRdCrx(G(rm), E(rm));
    return 0;
}

static U32 movCrxRd(X64Asm* data) {
    U8 rm = data->fetch8();
    data->movCrxRd(G(rm), E(rm));
    return 0;
}

static U32 invalidOp(X64Asm* data) {
    data->invalidOp(data->inst);
    data->done = true;
    return 0;
}

static U32 outb(X64Asm* data) {
    /*U8 port =*/ data->fetch8();
    return 0;
}

static U32 outb_dx(X64Asm* data) {
    return 0;
}

static U32 outw(X64Asm* data) {
    /*U8 port =*/ data->fetch8();
    return 0;
}

static U32 outw_dx(X64Asm* data) {
    return 0;
}

static U32 outd(X64Asm* data) {
    /*U8 port =*/ data->fetch8();
    return 0;
}

static U32 outd_dx(X64Asm* data) {
    return 0;
}

static U32 inb(X64Asm* data) {
    /*U8 port =*/ data->fetch8();
    data->writeToRegFromValue(0, false, 0xFF, 1);
    return 0;
}

static U32 inb_dx(X64Asm* data) {
    data->writeToRegFromValue(0, false, 0xFF, 1);
    return 0;
}

static U32 inw(X64Asm* data) {
    /*U8 port =*/ data->fetch8();
    data->writeToRegFromValue(0, false, 0xFFFF, 2);
    return 0;
}

static U32 inw_dx(X64Asm* data) {
    data->writeToRegFromValue(0, false, 0xFFFF, 2);
    return 0;
}

static U32 ind(X64Asm* data) {
    /*U8 port =*/ data->fetch8();
    data->writeToRegFromValue(0, false, 0xFFFFFFFF, 4);
    return 0;
}

static U32 ind_dx(X64Asm* data) {
    data->writeToRegFromValue(0, false, 0xFFFFFFFF, 4);
    return 0;
}

// SEG ES
static U32 segES(X64Asm* data) {
    data->ds = ES;
    data->ss = ES;
    return 1;
}

// SEG CS
static U32 segCS(X64Asm* data) {
    data->ds = CS;
    data->ss = CS;
    return 1;
}

// SEG SS
static U32 segSS(X64Asm* data) {
    data->ds = SS;
    data->ss = SS;
    return 1;
}

// SEG DS
static U32 segDS(X64Asm* data) {
    data->ds = DS;
    data->ss = DS;
    return 1;
}

// SEG FS
static U32 segFS(X64Asm* data) {
    data->ds = FS;
    data->ss = FS;
    return 1;
}

// SEG GS
static U32 segGS(X64Asm* data) {
    data->ds = GS;
    data->ss = GS;
    return 1;
}

// CPUID
static U32 x64cpuid(X64Asm* data) {
    data->cpuid();
    return 0;
}

// 2 byte opcodes
static U32 instruction0f(X64Asm* data) {
    data->baseOp+=0x100;
    data->multiBytePrefix = true;
    return 1; // continue decoding current instruction
}

// Operand Size Prefix
static U32 instruction66(X64Asm* data) {
    data->baseOp+=0x200;
    data->operandPrefix = 1;
    return 1; // continue decoding current instruction
}

// Operand Size Prefix
static U32 instruction266(X64Asm* data) {
    data->baseOp-=0x200;
    data->operandPrefix = 1;
    return 1; // continue decoding current instruction
}

static U32 instruction82(X64Asm* data) {
    data->op = 0x80;
    return inst8RMimm8SafeG(data);
}

// Address Size Prefix
static U32 addressSize32(X64Asm* data) {
    data->ea16 = 0;
    return 1;
}

// Address Size Prefix
static U32 addressSize16(X64Asm* data) {
    data->ea16 = 1;
    return 1;
}

// LOCK
static U32 lock(X64Asm* data) {
    data->lockPrefix = true;
    // add a mapping to the host instruction so that the lock can be skipped.  libc seems to do this.
    if (data->ip-1 == data->startOfOpIp) {
        data->mapAddress(data->ip, data->bufferPos);
    }
    return 1;
}

// REPNZ
static U32 repnz(X64Asm* data) {
    data->repNotZeroPrefix = true;
    return 1;
}

// REPZ
static U32 repz(X64Asm* data) {
    data->repZeroPrefix = true;
    return 1;
}

static U32 mmx(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 0);
    return 0;
}

static U32 mmxImm8(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 8);
    return 0;
}

static U32 mmxRegE(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 0);
    return 0;
}

// FPU ESC 0
// FPU ESC 1
// FPU ESC 2
// FPU ESC 3
// FPU ESC 4
// FPU ESC 5
// FPU ESC 6
// FPU ESC 7

static U32 instFPU(X64Asm* data, U8 rm) {
    bool isBig = data->currentOp->originalOp >= 0x200;    
    if (!isBig) {
        kpanic("instFPU softmmu doesn't support 16-bit, fpu emulation should have been enabled");
    }
    if (data->currentOp->inst == FNSAVE) {
        data->fpuWrite(rm, isBig ? 108 : 94);
        return 0;
    } else if (data->currentOp->inst == FNSTENV) {
        data->fpuWrite(rm, isBig ? 28 : 14);
        return 0;
    } else if (data->currentOp->inst == Fxsave) {
        data->fpuWrite(rm, 512);
        return 0;
    } else if (data->currentOp->inst == FLDENV) {
        data->fpuRead(rm, isBig ? 28 : 14);
        return 0;
    } else if (data->currentOp->inst == FRSTOR) {
        data->fpuRead(rm, isBig ? 108 : 94);
        return 0;
    } else if (data->currentOp->inst == Fxrstor) {
        data->fpuRead(rm, 512);
        return 0;
    }
    // don't check G, because G is a function not a reg
    // don't check E, we never load or store to ESP
    data->translateRM(rm, false, false, false, false, 0);
    return 0;
}

static U32 wait(X64Asm* data) {
    if (!data->cpu->thread->process->emulateFPU) {
        keepSame(data);
    }
    return 0;
}

static U32 instFPU0(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu0(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU1(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu1(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU2(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu2(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU3(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu3(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU4(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu4(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU5(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu5(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU6(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu6(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

static U32 instFPU7(X64Asm* data) {
    if (data->cpu->thread->process->emulateFPU) {
        data->fpu7(data->fetch8());
    } else {
        return instFPU(data, data->fetch8());
    }
    return 0;
}

// GRP2 Ew,1
// GRP2 Ew,CL
static U32 inst16RMSafeG(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 0);
    return 0;
}

// ADD Ed,Gd
// ADD Gd,Ed
// OR Ed,Gd
// OR Gd,Ed
// ADC Ed,Gd
// ADC Gd,Ed
// SBB Ed,Gd
// SBB Gd,Ed
// AND Ed,Gd
// AND Gd,Ed
// SUB Ed,Gd
// SUB Gd,Ed
// XOR Ed,Gd
// XOR Gd,Ed
// CMP Ed,Gd
// CMP Gd,Ed
// TEST Ed,Gd
// XCHG Ed,Gd
// MOV Ed,Gd
// MOV Gd,Ed
// BT Ed,Gd
// DSHLCL Ed,Gd
// BTS Ed,Gd
// DSHRCL Ed,Gd
// DIMUL Gd,Ed
// CMPXCHG Ed,Gd
// BTR Ed,Gd
// BTC Ed,Gd
// BSF Gd,Ed
// BSR Gd,Ed
static U32 inst32RM(X64Asm* data) {
    data->translateRM(data->fetch8(), true, true, false, false, 0);
    return 0;
}

static void emulateRM(X64Asm* data, U8 rm) {
    // advance eip correctly
    U32 pos = data->bufferPos;
    data->translateRM(rm, false, false, false, false, 0);
    data->bufferPos;
    data->emulateSingleOp(data->currentOp);
}

static U32 sseOp3AE(X64Asm* data) {
    U8 rm = data->fetch8();
    switch (G(rm)) {
    case 0: // FXSAVE
        if (data->cpu->thread->process->emulateFPU) {
            emulateRM(data, rm);
        } else {
            instFPU(data, rm);
        }
        break;
    case 1: // FXRSTOR
        if (data->cpu->thread->process->emulateFPU) {
            emulateRM(data, rm);
        } else {
            instFPU(data, rm);
        }
        break;
    case 2: // LDMXCSR
        data->translateRM(rm, false, true, false, false, 0);
        break;
    case 3: // STMXCSR
        data->translateRM(rm, false, true, false, false, 0);
        break;
    case 4: // XSAVE
        data->invalidOp(data->currentOp->originalOp);
        data->done = true;
        break;
    case 5:         
        if (rm>=0xC0) { // LFENCE
            data->has_rm = true;
            data->rm = rm;
            data->writeOp(); // keep same
        } else { // XRSTOR
            data->invalidOp(data->currentOp->originalOp);
            data->done = true;
        }
        break;
    case 6: // MFENCE
        data->has_rm = true;
        data->rm = rm;
        data->writeOp(); // keep same
        break;
    case 7:
        if (rm>=0xC0) { // SFENCE
            data->has_rm = true;
            data->rm = rm;
            data->writeOp(); // keep same
        } else { // CLFLUSH
            data->translateRM(rm, false, true, false, false, 0);
        }
    }
    return 0;
}

static U32 sseMmxErI8(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 8); // check E because it could be a reg
    return 0;
}

static U32 sseXmmErI8(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 8); // check E because it could be a reg
    return 0;
}

static U32 sseRegMmxI8(X64Asm* data) {
    data->translateRM(data->fetch8(), true, false, false, false, 8); // check E because it could be a reg
    return 0;
}

static U32 sseRegXmmI8(X64Asm* data) {
    data->translateRM(data->fetch8(), true, false, false, false, 8); // G is Reg
    return 0;
}

static U32 sseXmmExI8(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 8);
    return 0;
}

static U32 sseErMmx(X64Asm* data) {
    data->translateRM(data->fetch8(), true, false, false, false, 0); // check G because it could be a reg
    return 0;
}

static U32 sseRegXmm(X64Asm* data) {
    data->translateRM(data->fetch8(), true, false, false, false, 0); // G is Reg
    return 0;
}

static U32 sseXmmEx(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 0);
    return 0;
}

static U32 sseExXmm(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 0);
    return 0;
}

static U32 sseMmxEm(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 0);
    return 0;
}

static U32 sseDsEdiMmxOrSSE(X64Asm* data) {    
    data->DsEdiMmxOrSSE(data->fetch8());
    return 0;
}

static U32 sseOp318(X64Asm* data) {
    U8 rm = data->fetch8();
    switch (G(rm)) {
    case 0: // PREFETCHNTA
        data->translateRM(rm, false, true, false, false, 0);
        break;
    case 1: // PREFETCHT0
        data->translateRM(rm, false, true, false, false, 0);
        break;
    case 2: // PREFETCHT1
        data->translateRM(rm, false, true, false, false, 0);
        break;
    case 3: // PREFETCHT12
        data->translateRM(rm, false, true, false, false, 0);
        break;
    default:
        data->invalidOp(data->inst);
        break;
    }
    return 0;
}

static U32 sseOp32a(X64Asm* data) {
    if (data->repZeroPrefix || data->repNotZeroPrefix) {
        // read reg or address
        data->translateRM(data->fetch8(), false, true, false, false, 0);
    } else {
        // read mmx or address
        data->translateRM(data->fetch8(), false, false, false, false, 0);
    }
    return 0;
}

static U32 sseOp32c(X64Asm* data) {
    if (data->repZeroPrefix || data->repNotZeroPrefix) {
        // read xmm or address into reg
        data->translateRM(data->fetch8(), true, false, false, false, 0);
    } else {
        // read xmm or address into mmx
        data->translateRM(data->fetch8(), false, false, false, false, 0);
    }
    return 0;
}

static U32 sseOp32d(X64Asm* data) {
    if (data->repZeroPrefix || data->repNotZeroPrefix) {
        // read xmm or address into reg
        data->translateRM(data->fetch8(), true, false, false, false, 0);
    } else {
        // read xmm or address into mmx
        data->translateRM(data->fetch8(), false, false, false, false, 0);
    }
    return 0;
}

static U32 sseOp7e(X64Asm* data) {
    if (data->repZeroPrefix || data->repNotZeroPrefix) {
        data->translateRM(data->fetch8(), false, false, false, false, 0);
    } else {
        data->translateRM(data->fetch8(), false, true, false, false, 0);
    }
    return 0;
}

static U32 sse2(X64Asm* data) {
    data->translateRM(data->fetch8() | 0xC0, false, false, false, false, 0);
    return 0;
}

static U32 sse2E(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 0);
    return 0;
}

static U32 sse2Ed(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 0);
    return 0;
}

static U32 sse2Imm8(X64Asm* data) {
    data->translateRM(data->fetch8(), false, false, false, false, 8);
    return 0;
}

static U32 sse2RegE(X64Asm* data) {
    data->translateRM(data->fetch8(), true, false, false, false, 0);
    return 0;
}

static U32 op1f(X64Asm* data) {
    data->translateRM(data->fetch8(), false, true, false, false, 0);
    return 0;
}

X64Decoder x64Decoder[1024] = {
    // 00
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, push16ES, pop16ES,
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, push16CS, instruction0f,
    // 10
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, push16SS, pop16SS,
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, push16DS, pop16DS,
    // 20
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, segES, daa,
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, segCS, das,
    // 30
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, arithR8Ib, arithR16Iw, segSS, aaa,
    inst8RM, inst16RM, inst8RM, inst16RM, arithR8Ib, arithR16Iw, segDS, aas,
    // 40
    incAX, incCX, incDX, incBX, incSP, incBP, incSI, incDI,
    decAX, decCX, decDX, decBX, decSP, decBP, decSI, decDI,
    // 50
    pushAX, pushCX, pushDX, pushBX, pushSP, pushBP, pushSI, pushDI,
    popAX, popCX, popDX, popBX, popSP, popBP, popSI, popDI,
    // 60
    pushA16, popA16, bound16, invalidOp, segFS, segGS, instruction66, addressSize32,
    pushIw16, inst16RMimm16, pushIb16, inst16RMimm8, invalidOp, invalidOp, invalidOp, invalidOp,
    // 70
    jump8, jump8, jump8, jump8, jump8, jump8, jump8, jump8,
    jump8, jump8, jump8, jump8, jump8, jump8, jump8, jump8,
    // 80
    inst8RMimm8SafeG, inst16RMimm16SafeG, instruction82, inst16RMimm8SafeG, inst8RM, inst16RM, inst8RMGWritten, inst16RM,
    inst8RM, inst16RM, inst8RMGWritten, inst16RM, movEwSw, leaGw, movSwEw, popEw,
    // 90
    keepSame, keepSame, keepSame, keepSame, xchgSpAx, keepSame, keepSame, keepSame,
    keepSame, keepSame, callAp, wait, pushFlags16, popFlags16, keepSame, keepSame,
    // A0
    movAlOb, movAxOw, movObAl, movOwAx, movsb, movsw, cmpsb, cmpsw,
    keepSameImm8, keepSameImm16, stosb, stosw, lodsb, lodsw, scasb, scasw,
    // B0
    keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8,
    keepSameImm16, keepSameImm16, keepSameImm16, keepSameImm16, movSpIw, keepSameImm16, keepSameImm16, keepSameImm16,
    // C0
    inst8RMimm8SafeG, inst16RMimm8SafeG, retn16Iw, retn16, les16, lds16, inst8RMimm8, inst16RMimm16,
    enter16, leave16, retf16Iw, retf16, int3, intIb, invalidOp, iret,
    // D0
    inst8RMSafeG, inst16RMSafeG, inst8RMSafeG, inst16RMSafeG, aam, aad, salc, xlat,
    instFPU0, instFPU1, instFPU2, instFPU3, instFPU4, instFPU5, instFPU6, instFPU7,
    // E0
    x64loopnz, x64loopz, x64loop, x64jcxz, inb, inw, outb, outw,
    callJw, jmpJw, jmpAp, jmpJb, inb_dx, inw_dx, outb_dx, outw_dx,
    // F0
    lock, invalidOp, repnz, repz, hlt, keepSame, grp3b, grp3w,
    keepSame, keepSame, cli, sti, keepSame, keepSame, instGrp4, grp5w,

    // 100
    grp6_16, invalidOp, lar, lsl, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 110
    sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, &op1f,
    // 120
    movRdCrx, invalidOp, movCrxRd, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E,
    // 130
    invalidOp, rdtsc, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 140
    inst16RM, inst16RM, inst16RM, inst16RM, inst16RM, inst16RM, inst16RM, inst16RM,
    inst16RM, inst16RM, inst16RM, inst16RM, inst16RM, inst16RM, inst16RM, inst16RM,
    // 150
    sse2RegE, sse2E, invalidOp, invalidOp, sse2E, sse2E, sse2E, sse2E,
    sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E,
    // 160
    sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E,
    sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2Ed, sse2E,
    // 170
    sseXmmExI8, sse2Imm8, sse2Imm8, sse2Imm8, sse2E, sse2E, sse2E, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, sseOp7e, sse2E,
    // 180
    jump16, jump16, jump16, jump16, jump16, jump16, jump16, jump16,
    jump16, jump16, jump16, jump16, jump16, jump16, jump16, jump16,
    // 190
    inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM,
    inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM,
    // 1a0
    push16FS, pop16FS, x64cpuid, inst16RMBit, inst16RMimm8, inst16RM, invalidOp, invalidOp,
    push16GS, pop16GS, invalidOp, inst16RMBit, inst16RMimm8, inst16RM, invalidOp, inst16RM,
    // 1b0
    inst8RM, inst16RM, lss16, inst16RMBit, lfs16, lgs16, inst16E8RM, inst16RM,
    invalidOp, invalidOp, inst16RMimm8SafeG, inst16RMBit, inst16RM, inst16RM, inst16E8RM, invalidOp,
    // 1c0
    inst8RMGWritten, inst16RM, sse2Imm8, invalidOp, sseXmmErI8, sseRegXmmI8, sse2Imm8, invalidOp,
    keepSame, keepSame, keepSame, keepSame, bswapSp, keepSame, keepSame, keepSame,
    // 1d0
    invalidOp, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sseRegXmm,
    sse2E, sse2E, sseXmmEx, sse2E, sse2E, sse2E, sseXmmEx, sse2E,
    // 1e0
    sseXmmEx, sse2E, sse2E, sseXmmEx, sseXmmEx, sse2E, sse2E, sse2E,
    sse2E, sse2E, sseXmmEx, sse2E, sse2E, sse2E, sseXmmEx, sse2E,
    // 1f0
    invalidOp, sse2E, sse2E, sse2E, sse2E, sse2E, sseXmmEx, sseDsEdiMmxOrSSE,
    sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, sse2E, invalidOp,

    // 200
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, push32ES, pop32ES,
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, push32CS, instruction0f,
    // 210
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, push32SS, pop32SS,
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, push32DS, pop32DS,
    // 220
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, segES, daa,
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, segCS, das,
    // 230
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, arithR8Ib, arithR32Id, segSS, aaa,
    inst8RM, inst32RM, inst8RM, inst32RM, arithR8Ib, arithR32Id, segDS, aas,
    // 240
    incEAX, incECX, incEDX, incEBX, incESP, incEBP, incESI, incEDI,
    decEAX, decECX, decEDX, decEBX, decESP, decEBP, decESI, decEDI,
    // 250
    pushEAX, pushECX, pushEDX, pushEBX, pushESP, pushEBP, pushESI, pushEDI,
    popEAX, popECX, popEDX, popEBX, popESP, popEBP, popESI, popEDI,
    // 260
    pushA32, popA32, bound32, invalidOp, segFS, segGS, instruction266, addressSize16,
    pushId32, inst32RMimm32, pushIb32, inst32RMimm8, invalidOp, invalidOp, invalidOp, invalidOp,
    // 270
    jump8, jump8, jump8, jump8, jump8, jump8, jump8, jump8,
    jump8, jump8, jump8, jump8, jump8, jump8, jump8, jump8,
    // 280
    inst8RMimm8SafeG, inst32RMimm32SafeG, instruction82, inst32RMimm8SafeG, inst8RM, inst32RM, inst8RMGWritten, inst32RM,
    inst8RM, inst32RM, inst8RMGWritten, inst32RM, movEdSw, leaGd, movSwEw, popEd,
    // 290
    keepSame, keepSame, keepSame, keepSame, xchgEspEax, keepSame, keepSame, keepSame,
    keepSame, keepSame, callFar32, keepSame, pushFlags32, popFlags32, keepSame, keepSame,
    // 2a0
    movAlOb, movEaxOd, movObAl, movOdEax, movsb, movsd, cmpsb, cmpsd,
    keepSameImm8, keepSameImm32, stosb, stosd, lodsb, lodsd, scasb, scasd,
    // 2b0
    keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8, keepSameImm8,
    keepSameImm32, keepSameImm32, keepSameImm32, keepSameImm32, movEspId, keepSameImm32, keepSameImm32, keepSameImm32,
    // 2c0
    inst8RMimm8SafeG, inst32RMimm8SafeG, retn32Iw, retn32, les32, lds32, inst8RMimm8, inst32RMimm32,
    enter32, leave32, retf32Iw, retf32, int3, intIb, invalidOp, iret32,
    // 2d0
    inst8RMSafeG, inst32RMSafeG, inst8RMSafeG, inst32RMSafeG, aam, aad, salc, xlat,
    instFPU0, instFPU1, instFPU2, instFPU3, instFPU4, instFPU5, instFPU6, instFPU7,
    // 2e0
    x64loopnz, x64loopz, x64loop, x64jcxz, inb, ind, outb, outd,
    callJd, jmpJd, jmpFar32, jmpJb, inb_dx, ind_dx, outb_dx, outd_dx,
    // 2f0
    lock, keepSame, repnz, repz, hlt, keepSame, grp3b, grp3d,
    keepSame, keepSame, cli, sti, keepSame, keepSame, instGrp4, grp5d,

    // 300
    invalidOp, inst32RMSafeG, invalidOp, lsl32, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, keepSame, invalidOp, invalidOp, invalidOp, invalidOp,
    // 310
    sseXmmEx, sseExXmm, sseXmmEx, sseExXmm, sseXmmEx, sseXmmEx, sseXmmEx, sseExXmm,
    sseOp318, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, &op1f,
    // 320
    movRdCrx, invalidOp, movCrxRd, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    sseXmmEx, sseExXmm, sseOp32a, sseExXmm, sseOp32c, sseOp32d, sseXmmEx, sseXmmEx,
    // 330
    invalidOp, rdtsc, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 340
    inst32RM, inst32RM, inst32RM, inst32RM, inst32RM, inst32RM, inst32RM, inst32RM,
    inst32RM, inst32RM, inst32RM, inst32RM, inst32RM, inst32RM, inst32RM, inst32RM,
    // 350
    sseRegXmm, sseXmmEx, sseXmmEx, sseXmmEx, sseXmmEx, sseXmmEx, sseXmmEx, sseXmmEx,
    sseXmmEx, sseXmmEx, sse2E, sse2E, sseXmmEx, sseXmmEx, sseXmmEx, sseXmmEx,
    // 360
    mmx, mmx, mmx, mmx, mmx, mmx, mmx, mmx,
    mmx, mmx, mmx, mmx, invalidOp, invalidOp, mmxRegE, mmx,
    // 370
    sseXmmExI8, mmxImm8, mmxImm8, mmxImm8, mmx, mmx, mmx, keepSame,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, sseOp7e, mmx,
    // 380
    jump32, jump32, jump32, jump32, jump32, jump32, jump32, jump32,
    jump32, jump32, jump32, jump32, jump32, jump32, jump32, jump32,
    // 390
    inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM,
    inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM, inst8RM,
    // 3a0
    push32FS, pop32FS, x64cpuid, inst32RM, inst32RMimm8, inst32RM, invalidOp, invalidOp,
    push32GS, pop32GS, invalidOp, inst32RM, inst32RMimm8, inst32RM, sseOp3AE, inst32RM,
    // 3b0
    inst8RM, inst32RM, lss32, inst32RM, lfs32, lgs32, inst32E8RM, inst32E16RM,
    invalidOp, invalidOp, inst32RMimm8SafeG, inst32RM, inst32RM, inst32RM, inst32E8RM, inst32E16RM,
    // 3c0
    inst8RMGWritten, inst32RM, sseXmmExI8, sse2RegE, sseMmxErI8, sseRegMmxI8, sseXmmExI8, inst32RMSafeG,
    keepSame, keepSame, keepSame, keepSame, bswapEsp, keepSame, keepSame, keepSame,
    // 3d0
    invalidOp, mmx, mmx, mmx, sse2E, mmx, sse2, sseErMmx,
    mmx, mmx, sseMmxEm, mmx, mmx, mmx, sseMmxEm, mmx,
    // 3e0
    sseMmxEm, mmx, mmx, sseMmxEm, sseMmxEm, mmx, sse2E, sseMmxEm,
    mmx, mmx, sseMmxEm, mmx, mmx, mmx, sseMmxEm, mmx,
    // 3f0
    invalidOp, mmx, mmx, mmx, sse2E, mmx, sseMmxEm, sseDsEdiMmxOrSSE,
    mmx, mmx, mmx, sse2E, mmx, mmx, mmx, invalidOp,
};

#endif
