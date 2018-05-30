/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "jit.h"
#include "cpu.h"
#include "decoder.h"
#include "decode_noflags.h"

#define MAYBE 0x0400

#define GRP1 0x1000
#define GRP2cl 0x2000
#define GRP2 0x3000
#define GRP3 0x4000
#define GRP4 0x5000
#define GRP5 0x6000
#define GRP6 0x7000
#define FPU2r 0x8000
#define FPU2m 0x9000
#define FPU3r 0xa000
#define FPU3m 0xb000
#define FPU7r 0xc000
#define FPU7m 0xd000

struct OpInfo {
    U16 setsFlags;
    U16 getsFlags;
};

struct OpInfo subOpInfo[][8] = {
    // index 0 is not used
    {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},},

    // Group 1 (ADD/OR/ADC/SBB/AND/SUB/XOR/CMP)
    {{CF|AF|ZF|SF|OF|PF, 0}, {CF|AF|ZF|SF|OF|PF, 0}, {CF|AF|ZF|SF|OF|PF, CF}, {CF|AF|ZF|SF|OF|PF, CF}, {CF|AF|ZF|SF|OF|PF, 0}, {CF|AF|ZF|SF|OF|PF, 0}, {CF|AF|ZF|SF|OF|PF, 0}, {CF|AF|ZF|SF|OF|PF, 0}},

    // Group 2 cl (ROL/ROR/RCL/RCR/SHL/SHR/SAL/SAR)
    {{CF|OF|MAYBE,0}, {CF|OF|MAYBE,0}, {CF|OF|MAYBE,CF}, {CF|OF|MAYBE,CF}, {CF|AF|ZF|SF|OF|PF|MAYBE,0}, {CF|AF|ZF|SF|OF|PF|MAYBE,0}, {CF|AF|ZF|SF|OF|PF|MAYBE,0}, {CF|AF|ZF|SF|OF|PF|MAYBE,0}},

    // Group 2 (ROL/ROR/RCL/RCR/SHL/SHR/SAL/SAR)
    {{CF|OF,0}, {CF|OF,0}, {CF|OF,CF}, {CF|OF,CF}, {CF|AF|ZF|SF|OF|PF,0}, {CF|AF|ZF|SF|OF|PF,0}, {CF|AF|ZF|SF|OF|PF,0}, {CF|AF|ZF|SF|OF|PF,0}},

    // Group 3 (TEST/TEST/NOT/NEG/MUL/IMUL/DIV/IDIV)
    {{CF|AF|ZF|SF|OF|PF, 0}, {CF|AF|ZF|SF|OF|PF,0}, {0,0}, {CF|AF|ZF|SF|OF|PF,0}, {CF|OF,0}, {CF|OF,0}, {0,0}, {0,0}},

    // Group 4 (INC/DEC)
    {{AF|ZF|SF|OF|PF, 0}, {AF|ZF|SF|OF|PF,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}},

    // Group 5 (INC/DEC)
    {{AF|ZF|SF|OF|PF, 0}, {AF|ZF|SF|OF|PF,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}},

    // Group 6 (SLDT/STR/Lldt/Ltr/Verr/Verw)
    {{0, 0}, {0,0}, {0,0}, {0,0}, {ZF,0}, {ZF,0}, {0,0}, {0,0}},

    // FPU 2 r (FCMOVB/FCMOVE/FCMOVBE/FCMOVU)
    {{0,CF}, {0,ZF}, {0,CF|ZF}, {0, PF}, {0,0}, {0,0}, {0,0}, {0,0}},

    // FPU 2 m
    {{0,0}, {0,0}, {0,0}, {0, 0}, {0,0}, {0,0}, {0,0}, {0,0}},

    // FPU 3 r (FCMOVNB/FCMOVNE/FCMOVNBE/FCMOVNU/ /FUCOMI/FCOMI)
    {{0,CF}, {0,ZF}, {0,CF|ZF}, {0, PF}, {0,0}, {CF|AF|ZF|SF|OF|PF,0}, {CF|AF|ZF|SF|OF|PF,0}, {0,0}},

    // FPU 3 m
    {{0,0}, {0,0}, {0,0}, {0, 0}, {0,0}, {0,0}, {0,0}, {0,0}},

    // FPU 7 r (FFREEP/FXCH/FST/FST/FNSTSW/FUCOMI/FCOMI)
    {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {CF|AF|ZF|SF|OF|PF,0}, {CF|AF|ZF|SF|OF|PF,0}, {0,0}},

    // FPU 7 m
    {{0,0}, {0,0}, {0,0}, {0, 0}, {0,0}, {0,0}, {0,0}, {0,0}},
};

struct OpInfo opInfo[] = {
    {CF|AF|ZF|SF|OF|PF,0},			// 000 ADD Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 000 ADD Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 002 ADD Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 003 ADD Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 004 ADD Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 005 ADD Ax,Iw
    {0,0},							// 006 PUSH ES
    {0,0},							// 007 POP ES
    {CF|AF|ZF|SF|OF|PF,0},			// 008 OR Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 009 OR Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 00a OR Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 00b OR Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 00c OR Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 00d OR Ax,Iw
    {0,0},							// 00e PUSH CS
    {0,0},							// 00f 2 byte instruction
    {CF|AF|ZF|SF|OF|PF,CF},			// 010 ADC Eb,Gb
    {CF|AF|ZF|SF|OF|PF,CF},			// 010 ADC Ew,Gw
    {CF|AF|ZF|SF|OF|PF,CF},			// 012 ADC Gb,Eb
    {CF|AF|ZF|SF|OF|PF,CF},			// 013 ADC Gw,Ew
    {CF|AF|ZF|SF|OF|PF,CF},			// 014 ADC Al,Ib
    {CF|AF|ZF|SF|OF|PF,CF},			// 015 ADC Ax,Iw
    {0,0},							// 016 PUSH SS
    {0,0},							// 017 POP SS
    {CF|AF|ZF|SF|OF|PF,CF},			// 018 SBB Eb,Gb
    {CF|AF|ZF|SF|OF|PF,CF},			// 019 SBB Ew,Gw
    {CF|AF|ZF|SF|OF|PF,CF},			// 01a SBB Gb,Eb
    {CF|AF|ZF|SF|OF|PF,CF},			// 01b SBB Gw,Ew
    {CF|AF|ZF|SF|OF|PF,CF},			// 01c SBB Al,Ib
    {CF|AF|ZF|SF|OF|PF,CF},			// 01d SBB Ax,Iw
    {0,0},							// 01e PUSH DS
    {0,0},							// 01f POP DS
    {CF|AF|ZF|SF|OF|PF,0},			// 020 AND Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 021 AND Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 022 AND Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 023 AND Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 024 AND Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 025 AND Ax,Iw
    {0,0},							// 026 SEG ES
    {CF|AF|ZF|SF|PF,CF|AF},			// 027 DAA, OF is undefined
    {CF|AF|ZF|SF|OF|PF,0},			// 028 SUB Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 029 SUB Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 02a SUB Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 02b SUB Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 02c SUB Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 02d SUB Ax,Iw
    {0,0},							// 02e SEG CS
    {CF|AF|ZF|SF|OF|PF,CF|AF},		// 02f DAS
    {CF|AF|ZF|SF|OF|PF,0},			// 030 XOR Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 031 XOR Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 032 XOR Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 033 XOR Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 034 XOR Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 035 XOR Ax,Iw
    {0,0},							// 036 SEG SS
    {CF|AF|ZF|SF|OF|PF,AF},			// 037 AAA
    {CF|AF|ZF|SF|OF|PF,0},			// 038 CMP Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 039 CMP Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 03a CMP Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 03b CMP Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 03c CMP Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 03d CMP Ax,Iw
    {0,0},							// 03e SEG DS
    {CF|AF|ZF|SF|OF|PF,AF},			// 03f AAS
    {AF|ZF|SF|OF|PF,0},				// 040 INC AX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 041 INC CX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 042 INC DX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 043 INC BX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 044 INC SP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 045 INC BP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 046 INC SI, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 047 INC DI, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 048 DEC AX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 049 DEC CX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 04a DEC DX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 04b DEC BX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 04c DEC SP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 04d DEC BP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 04e DEC SI, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 04f DEC DI, CF is preserved
    {0,0},							// 050 PUSH AX
    {0,0},							// 051 PUSH CX
    {0,0},							// 052 PUSH DX
    {0,0},							// 053 PUSH BX
    {0,0},							// 054 PUSH SP
    {0,0},							// 055 PUSH BP
    {0,0},							// 056 PUSH SI
    {0,0},							// 057 PUSH DI
    {0,0},							// 058 POP AX
    {0,0},							// 059 POP CX
    {0,0},							// 05a POP DX
    {0,0},							// 05b POP BX
    {0,0},							// 05c POP SP
    {0,0},							// 05d POP BP
    {0,0},							// 05e POP SI
    {0,0},							// 05f POP DI
    {0,0},							// 060 PUSHA
    {0,0},							// 061 POPA
    {0,0},							// 062 BOUND
    {ZF,0},							// 063 ARPL Ew,Rw
    {0,0},							// 064 SEG FS
    {0,0},							// 065 SEG GS
    {0,0},							// 066 Operand Size Prefix
    {0,0},							// 067 Address Size Prefix
    {0,0},							// 068 PUSH Iw
    {CF|OF,0},						// 069 IMUL Gw,Ew,Iw: SF, ZF, PF, AF are undefined
    {0,0},							// 06a PUSH Ib
    {CF|OF,0},						// 06b IMUL Gw,Ew,Ib: SF, ZF, PF, AF are undefined
    {0,0},							// 06c INSB
    {0,0},							// 06d INSW
    {0,0},							// 06e OUTSB
    {0,0},							// 06f OUTSW
    {0,OF},							// 070 JO
    {0,OF},							// 071 JNO
    {0,CF},							// 072 JB
    {0,CF},							// 073 JNB
    {0,ZF},							// 074 JZ
    {0,ZF},							// 075 JNZ
    {0,CF|ZF},						// 076 JBE
    {0,CF|ZF},						// 077 JNBE
    {0,SF},							// 078 JS
    {0,SF},							// 079 JNS
    {0,PF},							// 07a JP
    {0,PF},							// 07b JNP
    {0,SF|OF},						// 07c JL
    {0,SF|OF},						// 07d JNL
    {0,SF|OF|ZF},					// 07e JLE
    {0,SF|OF|ZF},					// 07f JNLE
    {GRP1, GRP1},					// 080 Grpl Eb,Ib
    {GRP1, GRP1},					// 081 Grpl Ew,Iw
    {GRP1, GRP1},					// 082 Grpl Eb,Ib
    {GRP1, GRP1},					// 083 Grpl Ew,Ix
    {CF|AF|ZF|SF|OF|PF,0},			// 084 TEST Eb,Gb: CF, AF, OF are always 0
    {CF|AF|ZF|SF|OF|PF,0},			// 085 TEST Ew,Gw: CF, AF, OF are always 0
    {0,0},							// 086 XCHG Eb,Gb
    {0,0},							// 087 XCHG Ew,Gw
    {0,0},							// 088 MOV Eb,Gb
    {0,0},							// 089 MOV Ew,Gw
    {0,0},							// 08a MOV Gb,Eb
    {0,0},							// 08b MOV Gw,Ew
    {0,0},							// 08c Mov Ew,Sw
    {0,0},							// 08d LEA Gw
    {0,0},							// 08e MOV Sw,Ew
    {0,0},							// 08f POP Ew
    {0,0},							// 090 NOP
    {0,0},							// 091 XCHG CX,AX
    {0,0},							// 092 XCHG DX,AX
    {0,0},							// 093 XCHG BX,AX
    {0,0},							// 094 XCHG SP,AX
    {0,0},							// 095 XCHG BP,AX
    {0,0},							// 096 XCHG SI,AX
    {0,0},							// 097 XCHG DI,AX
    {0,0},							// 098 CBW
    {0,0},							// 099 CWD
    {0,0},							// 09a CALL Ap
    {0,0},							// 09b WAIT
    {0,CF|AF|ZF|SF|OF|PF},			// 09c PUSHF
    {CF|AF|ZF|SF|OF|PF,0},			// 09d POPF
    {CF|AF|ZF|SF|PF,0},				// 09e SAHF: OF not part of lower 8 bits
    {0,CF|AF|ZF|SF|PF},				// 09f LAHF: OF not part of lower 8 bits
    {0,0},							// 0a0 MOV AL,Ob
    {0,0},							// 0a1 MOV AX,Ow
    {0,0},							// 0a2 MOV Ob,AL
    {0,0},							// 0a3 MOV Ow,AX
    {0,0},							// 0a4 MOVSB
    {0,0},							// 0a5 MOVSW
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 0a6 CMPSB
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 0a7 CMPSW
    {CF|AF|ZF|SF|OF|PF,0},	        // 0a8 TEST AL,Ib: CF, AF, OF are always 0
    {CF|AF|ZF|SF|OF|PF,0},	        // 0a9 TEST AX,Iw: CF, AF, OF are always 0
    {0,0},							// 0aa STOSB
    {0,0},							// 0ab STOSW
    {0,0},							// 0ac LODSB
    {0,0},							// 0ad LODSW
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 0ae SCASB
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 0af SCASW
    {0,0},							// 0b0 MOV AL,Ib
    {0,0},							// 0b1 MOV CL,Ib
    {0,0},							// 0b2 MOV DL,Ib
    {0,0},							// 0b3 MOV BL,Ib
    {0,0},							// 0b4 MOV AH,Ib
    {0,0},							// 0b5 MOV CH,Ib
    {0,0},							// 0b6 MOV DH,Ib
    {0,0},							// 0b7 MOV BH,Ib
    {0,0},							// 0b8 MOV AX,Iw
    {0,0},							// 0b9 MOV CX,Iw
    {0,0},							// 0ba MOV DX,Iw
    {0,0},							// 0bb MOV BX,Iw
    {0,0},							// 0bc MOV SP,Iw
    {0,0},							// 0bd MOV BP,Iw
    {0,0},							// 0be MOV SI,Iw
    {0,0},							// 0bf MOV DI,Iw
    {GRP2,GRP2},					// 0c0 GRP2 Eb,Ib
    {GRP2,GRP2},					// 0c1 GRP2 Ew,Ib
    {0,0},							// 0c2 RETN Iw
    {0,0},							// 0c3 RETN
    {0,0},							// 0c4 LES
    {0,0},							// 0c5 LDS
    {0,0},							// 0c6 MOV Eb,Ib
    {0,0},							// 0c7 MOV EW,Iw
    {0,0},							// 0c8 ENTER Iw,Ib
    {0,0},							// 0c9 LEAVE
    {0,0},							// 0ca RETF Iw
    {0,0},							// 0cb RETF
    {0,0},							// 0cc INT3
    {0,0},							// 0cd INT Ib
    {0,OF},							// 0ce INTO
    {0,0},							// 0cf IRET
    {GRP2,GRP2},					// 0d0 GRP2 Eb,1
    {GRP2,GRP2},					// 0d1 GRP2 Ew,1
    {GRP2cl,GRP2cl},				// 0d2 GRP2 Eb,CL
    {GRP2cl,GRP2cl},				// 0d3 GRP2 Ew,CL
    {CF|AF|ZF|SF|OF|PF,0},			// 0d4 AAM Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 0d5 AAD Ib
    {0, CF},     					// 0d6 SALC
    {0,0},							// 0d7 XLAT
    {0,0},							// 0d8 FPU ESC 0
    {0,0},							// 0d9 FPU ESC 1
    {FPU2r,FPU2m},					// 0da FPU ESC 2
    {FPU3r,FPU3m},					// 0db FPU ESC 3
    {0,0},							// 0dc FPU ESC 4
    {0,0},							// 0dd FPU ESC 5
    {0,0},							// 0de FPU ESC 6
    {FPU7r,FPU7m},					// 0df FPU ESC 7
    {0,ZF},							// 0e0 LOOPNZ
    {0,ZF},							// 0e1 LOOPZ
    {0,0},							// 0e2 LOOP
    {0,0},							// 0e3 JCXZ
    {0,0},							// 0e4 IN AL,Ib
    {0,0},							// 0e5 IN AX,Ib
    {0,0},							// 0e6 OUT Ib,AL
    {0,0},							// 0e7 OUT Ib,AX
    {0,0},							// 0e8 CALL Jw
    {0,0},							// 0e9 JMP Jw
    {0,0},							// 0ea JMP Ap
    {0,0},							// 0eb JMP Jb
    {0,0},							// 0ec IN AL,DX
    {0,0},							// 0ed IN AX,DX
    {0,0},							// 0ee OUT DX,AL
    {0,0},							// 0ef OUT DX,AX
    {0,0},							// 0f0 LOCK
    {0,0},							// 0f1 ICEBP
    {0,0},							// 0f2 REPNZ
    {0,0},							// 0f3 REPZ
    {0,0},							// 0f4 HLT
    {CF,CF},						// 0f5 CMC
    {GRP3,GRP3},					// 0f6 GRP3 Eb(,Ib)
    {GRP3,GRP3},					// 0f7 GRP3 Ew(,Iw)
    {CF,0},							// 0f8 CLC
    {CF,0},							// 0f9 STC
    {0,0},							// 0fa CLI
    {0,0},							// 0fb STI
    {0,0},							// 0fc CLD
    {0,0},							// 0fd STD
    {GRP4,GRP4},					// 0fe GRP4 Eb
    {GRP5,GRP5},					// 0ff GRP5 Ew
    {GRP6,GRP6},					// 100 GRP 6 Exxx
    {0,0},							// 101 Group 7 Ew
    {ZF,0},							// 102 LAR Gw,Ew
    {ZF,0},							// 103 LSL Gw,Ew
    {0,0},							// 104
    {0,0},							// 105
    {0,0},							// 106 CLTS
    {0,0},							// 107
    {0,0},							// 108 INVD
    {0,0},							// 109 WBINVD
    {0,0},							// 10a
    {0,0},							// 10b
    {0,0},							// 10c
    {0,0},							// 10d
    {0,0},							// 10e
    {0,0},							// 10f
    {0,0},							// 110
    {0,0},							// 111
    {0,0},							// 112
    {0,0},							// 113
    {0,0},							// 114
    {0,0},							// 115
    {0,0},							// 116
    {0,0},							// 117
    {0,0},							// 118
    {0,0},							// 119
    {0,0},							// 11a
    {0,0},							// 11b
    {0,0},							// 11c
    {0,0},							// 11d
    {0,0},							// 11e
    {0,0},							// 11f
    {0,0},							// 120 MOV Rd.CRx
    {0,0},							// 121 MOV Rd,DRx
    {0,0},							// 122 MOV CRx,Rd
    {0,0},							// 123 MOV DRx,Rd
    {0,0},							// 124 MOV Rd,TRx
    {0,0},							// 125
    {0,0},							// 126 MOV TRx,Rd
    {0,0},							// 127
    {0,0},							// 128
    {0,0},							// 129
    {0,0},							// 12a
    {0,0},							// 12b
    {0,0},							// 12c
    {0,0},							// 12d
    {0,0},							// 12e
    {0,0},							// 12f
    {0,0},							// 130
    {0,0},							// 131 RDTSC
    {0,0},							// 132
    {0,0},							// 133
    {0,0},							// 134
    {0,0},							// 135
    {0,0},							// 136
    {0,0},							// 137
    {0,0},							// 138
    {0,0},							// 139
    {0,0},							// 13a
    {0,0},							// 13b
    {0,0},							// 13c
    {0,0},							// 13d
    {0,0},							// 13e
    {0,0},							// 13f
    {0,OF},							// 140 CMOVO
    {0,OF},							// 141 CMOVNO
    {0,CF},							// 142 CMOVB
    {0,CF},							// 143 CMOVNB
    {0,ZF},							// 144 CMOVZ
    {0,ZF},							// 145 CMOVNZ
    {0,CF|ZF},						// 146 CMOVBE
    {0,CF|ZF},						// 147 CMOVNBE
    {0,SF},							// 148 CMOVS
    {0,SF},							// 149 CMOVNS
    {0,PF},							// 14a CMOVP
    {0,PF},							// 14b CMOVNP
    {0,SF|OF},						// 14c CMOVL
    {0,SF|OF},						// 14d CMOVNL
    {0,SF|OF|ZF},					// 14e CMOVLE
    {0,SF|OF|ZF},					// 14f CMOVNLE
    {0,0},							// 150
    {0,0},							// 151
    {0,0},							// 152
    {0,0},							// 153
    {0,0},							// 154
    {0,0},							// 155
    {0,0},							// 156
    {0,0},							// 157
    {0,0},							// 158
    {0,0},							// 159
    {0,0},							// 15a
    {0,0},							// 15b
    {0,0},							// 15c
    {0,0},							// 15d
    {0,0},							// 15e
    {0,0},							// 15f
    {0,0},							// 160
    {0,0},							// 161
    {0,0},							// 162
    {0,0},							// 163
    {0,0},							// 164
    {0,0},							// 165
    {0,0},							// 166
    {0,0},							// 167
    {0,0},							// 168
    {0,0},							// 169
    {0,0},							// 16a
    {0,0},							// 16b
    {0,0},							// 16c
    {0,0},							// 16d
    {0,0},							// 16e
    {0,0},							// 16f
    {0,0},							// 170
    {0,0},							// 171
    {0,0},							// 172
    {0,0},							// 173
    {0,0},							// 174
    {0,0},							// 175
    {0,0},							// 176
    {0,0},							// 177
    {0,0},							// 178
    {0,0},							// 179
    {0,0},							// 17a
    {0,0},							// 17b
    {0,0},							// 17c
    {0,0},							// 17d
    {0,0},							// 17e
    {0,0},							// 17f
    {0,OF},							// 180 JO
    {0,OF},							// 181 JNO
    {0,CF},							// 182 JB
    {0,CF},							// 183 JNB
    {0,ZF},							// 184 JZ
    {0,ZF},							// 185 JNZ
    {0,CF|ZF},						// 186 JBE
    {0,CF|ZF},						// 187 JNBE
    {0,SF},							// 188 JS
    {0,SF},							// 189 JNS
    {0,PF},							// 18a JP
    {0,PF},							// 18b JNP
    {0,SF|OF},						// 18c JL
    {0,SF|OF},						// 18d JNL
    {0,SF|OF|ZF},					// 18e JLE
    {0,SF|OF|ZF},					// 18f JNLE
    {0,OF},							// 190 SETO
    {0,OF},							// 191 SETNO
    {0,CF},							// 192 SETB
    {0,CF},							// 193 SETNB
    {0,ZF},							// 194 SETZ
    {0,ZF},							// 195 SETNZ
    {0,CF|ZF},						// 196 SETBE
    {0,CF|ZF},						// 197 SETNBE
    {0,SF},							// 198 SETS
    {0,SF},							// 199 SETNS
    {0,PF},							// 19a SETP
    {0,PF},							// 19b SETNP
    {0,SF|OF},						// 19c SETL
    {0,SF|OF},						// 19d SETNL
    {0,SF|OF|ZF},					// 19e SETLE
    {0,SF|OF|ZF},					// 19f SETNLE
    {0,0},							// 1a0 PUSH FS
    {0,0},							// 1a1 POP FS
    {0,0},							// 1a2 CPUID
    {CF,0},							// 1a3 BT Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 1a4 SHLD Ew,Gw,Ib: AF is alway 0
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 1a5 SHLD Ew,Gw,CL: AF is alway 0
    {0,0},							// 1a6
    {0,0},							// 1a7
    {0,0},							// 1a8 PUSH GS
    {0,0},							// 1a9 POP GS
    {0,0},							// 1aa
    {CF,0},							// 1ab BTS Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 1ac SHRD Ew,Gw,Ib: AF is alway 0
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 1ad SHRD Ew,Gw,CL: AF is alway 0
    {0,0},							// 1ae
    {CF|OF,0},						// 1af IMUL Gw,Ew: SF, ZF, PF, AF are undefined
    {CF|AF|ZF|SF|OF|PF,0},			// 1b0 cmpxchg Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 1b1 cmpxchg Ew,Gw
    {0,0},							// 1b2 LSS Ew
    {CF,0},							// 1b3 BTR Ew,Gw
    {0,0},							// 1b4 LFS Ew
    {0,0},							// 1b5 LGS Ew
    {0,0},							// 1b6 MOVZX Gw,Eb
    {0,0},							// 1b7 MOVZX Gw,Ew
    {0,0},							// 1b8
    {0,0},							// 1b9
    {CF,0},							// 1ba GRP8 Ew,Ib
    {CF,0},							// 1bb BTC Ew,Gw
    {ZF,0},							// 1bc BSF Gw,Ew: other flags are undefined
    {ZF,0},							// 1bd BSR Gw,Ew: other flags are undefined
    {0,0},							// 1be MOVSX Gw,Eb
    {0,0},							// 1bf MOVSX Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 1c0 XADD Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 1c1 XADD Gw,Ew
    {0,0},							// 1c2
    {0,0},							// 1c3
    {0,0},							// 1c4
    {0,0},							// 1c5
    {0,0},							// 1c6
    {0,0},							// 1c7
    {0,0},							// 1c8 BSWAP AX
    {0,0},							// 1c9 BSWAP CX
    {0,0},							// 1ca BSWAP DX
    {0,0},							// 1cb BSWAP BX
    {0,0},							// 1cc BSWAP SP
    {0,0},							// 1cd BSWAP BP
    {0,0},							// 1ce BSWAP SI
    {0,0},							// 1cf BSWAP DI
    {0,0},							// 1d0
    {0,0},							// 1d1
    {0,0},							// 1d2
    {0,0},							// 1d3
    {0,0},							// 1d4
    {0,0},							// 1d5
    {0,0},							// 1d6
    {0,0},							// 1d7
    {0,0},							// 1d8
    {0,0},							// 1d9
    {0,0},							// 1da
    {0,0},							// 1db
    {0,0},							// 1dc
    {0,0},							// 1dd
    {0,0},							// 1de
    {0,0},							// 1df
    {0,0},							// 1e0
    {0,0},							// 1e1
    {0,0},							// 1e2
    {0,0},							// 1e3
    {0,0},							// 1e4
    {0,0},							// 1e5
    {0,0},							// 1e6
    {0,0},							// 1e7
    {0,0},							// 1e8
    {0,0},							// 1e9
    {0,0},							// 1ea
    {0,0},							// 1eb
    {0,0},							// 1ec
    {0,0},							// 1ed
    {0,0},							// 1ee
    {0,0},							// 1ef
    {0,0},							// 1f0
    {0,0},							// 1f1
    {0,0},							// 1f2
    {0,0},							// 1f3
    {0,0},							// 1f4
    {0,0},							// 1f5
    {0,0},							// 1f6
    {0,0},							// 1f7
    {0,0},							// 1f8
    {0,0},							// 1f9
    {0,0},							// 1fa
    {0,0},							// 1fb
    {0,0},							// 1fc
    {0,0},							// 1fd
    {0,0},							// 1fe
    {0,0},							// 1ff
    {CF|AF|ZF|SF|OF|PF,0},			// 200 ADD Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 201 ADD Ed,Gd
    {CF|AF|ZF|SF|OF|PF,0},			// 202 ADD Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 203 ADD Gd,Ed
    {CF|AF|ZF|SF|OF|PF,0},			// 204 ADD Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 205 ADD Eax,Id
    {0,0},							// 206 PUSH ES
    {0,0},							// 207 POP ES
    {CF|AF|ZF|SF|OF|PF,0},			// 208 OR Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 209 OR Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 20a OR Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 20b OR Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 20c OR Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 20d OR Ax,Iw
    {0,0},							// 20e PUSH CS
    {0,0},							// 20f 2 byte instruction
    {CF|AF|ZF|SF|OF|PF,CF},			// 210 ADC Eb,Gb
    {CF|AF|ZF|SF|OF|PF,CF},			// 210 ADC Ew,Gw
    {CF|AF|ZF|SF|OF|PF,CF},			// 212 ADC Gb,Eb
    {CF|AF|ZF|SF|OF|PF,CF},			// 213 ADC Gw,Ew
    {CF|AF|ZF|SF|OF|PF,CF},			// 214 ADC Al,Ib
    {CF|AF|ZF|SF|OF|PF,CF},			// 215 ADC Ax,Iw
    {0,0},							// 216 PUSH SS
    {0,0},							// 217 POP SS
    {CF|AF|ZF|SF|OF|PF,CF},			// 218 SBB Eb,Gb
    {CF|AF|ZF|SF|OF|PF,CF},			// 219 SBB Ew,Gw
    {CF|AF|ZF|SF|OF|PF,CF},			// 21a SBB Gb,Eb
    {CF|AF|ZF|SF|OF|PF,CF},			// 21b SBB Gw,Ew
    {CF|AF|ZF|SF|OF|PF,CF},			// 21c SBB Al,Ib
    {CF|AF|ZF|SF|OF|PF,CF},			// 21d SBB Ax,Iw
    {0,0},							// 21e PUSH DS
    {0,0},							// 21f POP DS
    {CF|AF|ZF|SF|OF|PF,0},			// 220 AND Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 221 AND Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 222 AND Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 223 AND Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 224 AND Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 225 AND Ax,Iw
    {0,0},							// 226 SEG ES
    {CF|AF|ZF|SF|PF,CF|AF},			// 227 DAA, OF is undefined
    {CF|AF|ZF|SF|OF|PF,0},			// 228 SUB Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 229 SUB Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 22a SUB Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 22b SUB Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 22c SUB Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 22d SUB Ax,Iw
    {0,0},							// 22e SEG CS
    {CF|AF|ZF|SF|OF|PF,CF|AF},		// 22f DAS
    {CF|AF|ZF|SF|OF|PF,0},			// 230 XOR Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 231 XOR Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 232 XOR Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 233 XOR Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 234 XOR Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 235 XOR Ax,Iw
    {0,0},							// 236 SEG SS
    {CF|AF|ZF|SF|OF|PF,AF},			// 237 AAA
    {CF|AF|ZF|SF|OF|PF,0},			// 238 CMP Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 239 CMP Ew,Gw
    {CF|AF|ZF|SF|OF|PF,0},			// 23a CMP Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 23b CMP Gw,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 23c CMP Al,Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 23d CMP Ax,Iw
    {0,0},							// 23e SEG DS
    {CF|AF|ZF|SF|OF|PF,AF},			// 23f AAS
    {AF|ZF|SF|OF|PF,0},				// 240 INC EAX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 241 INC ECX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 242 INC EDX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 243 INC EBX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 244 INC ESP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 245 INC EBP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 246 INC ESI, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 247 INC EDI, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 248 DEC EAX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 249 DEC ECX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 24a DEC EDX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 24b DEC EBX, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 24c DEC ESP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 24d DEC EBP, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 24e DEC ESI, CF is preserved
    {AF|ZF|SF|OF|PF,0},				// 24f DEC EDI, CF is preserved
    {0,0},							// 250 PUSH EAX
    {0,0},							// 251 PUSH ECX
    {0,0},							// 252 PUSH EDX
    {0,0},							// 253 PUSH EBX
    {0,0},							// 254 PUSH ESP
    {0,0},							// 255 PUSH EBP
    {0,0},							// 256 PUSH ESI
    {0,0},							// 257 PUSH EDI
    {0,0},							// 258 POP EAX
    {0,0},							// 259 POP ECX
    {0,0},							// 25a POP EDX
    {0,0},							// 25b POP EBX
    {0,0},							// 25c POP ESP
    {0,0},							// 25d POP EBP
    {0,0},							// 25e POP ESI
    {0,0},							// 25f POP EDI
    {0,0},							// 260 PUSHA
    {0,0},							// 261 POPA
    {0,0},							// 262 BOUND
    {ZF,0},							// 263 ARPL Ed,Rd
    {0,0},							// 264 SEG FS
    {0,0},							// 265 SEG GS
    {0,0},							// 266 Operand Size Prefix
    {0,0},							// 267 Address Size Prefix
    {0,0},							// 268 PUSH Id
    {CF|OF,0},						// 269 IMUL Gd,Ed,Id: SF, ZF, PF, AF are undefined
    {0,0},							// 26a PUSH Ib
    {CF|OF,0},						// 26b IMUL Gd,Ed,Ib: SF, ZF, PF, AF are undefined
    {0,0},							// 26c INSB
    {0,0},							// 26d INSD
    {0,0},							// 26e OUTSB
    {0,0},							// 26f OUTSD
    {0,OF},							// 270 JO
    {0,OF},							// 271 JNO
    {0,CF},							// 272 JB
    {0,CF},							// 273 JNB
    {0,ZF},							// 274 JZ
    {0,ZF},							// 275 JNZ
    {0,CF|ZF},						// 276 JBE
    {0,CF|ZF},						// 277 JNBE
    {0,SF},							// 278 JS
    {0,SF},							// 279 JNS
    {0,PF},							// 27a JP
    {0,PF},							// 27b JNP
    {0,SF|OF},						// 27c JL
    {0,SF|OF},						// 27d JNL
    {0,SF|OF|ZF},					// 27e JLE
    {0,SF|OF|ZF},					// 27f JNLE
    {GRP1,GRP1},					// 280 Grpl Eb,Ib
    {GRP1,GRP1},					// 281 Grpl Ed,Id
    {GRP1,GRP1},					// 282 Grpl Eb,Ib
    {GRP1,GRP1},					// 283 Grpl Ed,Ix
    {CF|AF|ZF|SF|OF|PF,0},			// 284 TEST Eb,Gb: CF, AF, OF are always 0
    {CF|AF|ZF|SF|OF|PF,0},			// 285 TEST Ed,Gd: CF, AF, OF are always 0
    {0,0},							// 286 XCHG Eb,Gb
    {0,0},							// 287 XCHG Ed,Gd
    {0,0},							// 288 MOV Eb,Gb
    {0,0},							// 289 MOV Ed,Gd
    {0,0},							// 28a MOV Gb,Eb
    {0,0},							// 28b MOV Gd,Ed
    {0,0},							// 28c Mov Ew,Sw
    {0,0},							// 28d LEA Gd
    {0,0},							// 28e MOV Sw,Ew
    {0,0},							// 28f POP Ed
    {0,0},							// 290 NOP
    {0,0},							// 291 XCHG ECX,EAX
    {0,0},							// 292 XCHG EDX,EAX
    {0,0},							// 293 XCHG EBX,EAX
    {0,0},							// 294 XCHG ESP,EAX
    {0,0},							// 295 XCHG EBP,EAX
    {0,0},							// 296 XCHG ESI,EAX
    {0,0},							// 297 XCHG EDI,EAX
    {0,0},							// 298 CWDE
    {0,0},							// 299 CDQ
    {0,0},							// 29a CALL FAR Ad
    {0,0},							// 29b WAIT
    {0,CF|AF|ZF|SF|OF|PF},			// 29c PUSHFD
    {CF|AF|ZF|SF|OF|PF,0},			// 29d POPFD
    {CF|AF|ZF|SF|PF,0},				// 29e SAHF: OF not part of lower 8 bits
    {0,CF|AF|ZF|SF|PF},				// 29f LAHF: OF not part of lower 8 bits
    {0,0},							// 2a0 MOV AL,Ob
    {0,0},							// 2a1 MOV EAX,Od
    {0,0},							// 2a2 MOV Ob,AL
    {0,0},							// 2a3 MOV Od,EAX
    {0,0},							// 2a4 MOVSB
    {0,0},							// 2a5 MOVSD
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 2a6 CMPSB
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 2a7 CMPSD
    {CF|AF|ZF|SF|OF|PF,0},			// 2a8 TEST AL,Ib: CF, AF, OF are always 0
    {CF|AF|ZF|SF|OF|PF,0},			// 2a9 TEST EAX,Id: CF, AF, OF are always 0
    {0,0},							// 2aa STOSB
    {0,0},							// 2ab STOSD
    {0,0},							// 2ac LODSB
    {0,0},							// 2ad LODSD
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 2ae SCASB
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 2af SCASD
    {0,0},							// 2b0 MOV AL,Ib
    {0,0},							// 2b1 MOV CL,Ib
    {0,0},							// 2b2 MOV DL,Ib
    {0,0},							// 2b3 MOV BL,Ib
    {0,0},							// 2b4 MOV AH,Ib
    {0,0},							// 2b5 MOV CH,Ib
    {0,0},							// 2b6 MOV DH,Ib
    {0,0},							// 2b7 MOV BH,Ib
    {0,0},							// 2b8 MOV EAX,Id
    {0,0},							// 2b9 MOV ECX,Id
    {0,0},							// 2ba MOV EDX,Id
    {0,0},							// 2bb MOV EBX,Id
    {0,0},							// 2bc MOV ESP,Id
    {0,0},							// 2bd MOV EBP,Id
    {0,0},							// 2be MOV ESI,Id
    {0,0},							// 2bf MOV EDI,Id
    {GRP2,GRP2},					// 2c0 GRP2 Eb,Ib
    {GRP2,GRP2},					// 2c1 GRP2 Ed,Ib
    {0,0},							// // 2c2 RETN Iw
    {0,0},							// // 2c3 RETN
    {0,0},							// // 2c4 LES
    {0,0},							// // 2c5 LDS
    {0,0},							// // 2c6 MOV Eb,Ib
    {0,0},							// // 2c7 MOV Ed,Id
    {0,0},							// // 2c8 ENTER Iw,Ib
    {0,0},							// // 2c9 LEAVE
    {0,0},							// // 2ca RETF Iw
    {0,0},							// // 2cb RETF
    {0,0},							// // 2cc INT3
    {0,0},							// // 2cd INT Ib
    {0,OF},							// 2ce INTO
    {0,0},							// 2cf IRET
    {GRP2,GRP2},					// 2d0 GRP2 Eb,1
    {GRP2,GRP2},					// 2d1 GRP2 Ed,1
    {GRP2,GRP2},					// 2d2 GRP2 Eb,CL
    {GRP2,GRP2},					// 2d3 GRP2 Ed,CL
    {CF|AF|ZF|SF|OF|PF,0},			// 2d4 AAM Ib
    {CF|AF|ZF|SF|OF|PF,0},			// 2d5 AAD Ib
    {0, CF},    					// 2d6 SALC
    {0,0},							// 2d7 XLAT
    {0,0},							// 2d8 FPU ESC 0
    {0,0},							// 2d9 FPU ESC 1
    {FPU2r,FPU2m},					// 2da FPU ESC 2
    {FPU3r,FPU3m},					// 2db FPU ESC 3
    {0,0},							// 2dc FPU ESC 4
    {0,0},							// 2dd FPU ESC 5
    {0,0},							// 2de FPU ESC 6
    {FPU7r,FPU7m},					// 2df FPU ESC 7
    {0,ZF},							// 2e0 LOOPNZ
    {0,ZF},							// 2e1 LOOPZ
    {0,0},							// 2e2 LOOP
    {0,0},							// 2e3 JCXZ
    {0,0},							// 2e4 IN AL,Ib
    {0,0},							// 2e5 IN EAX,Ib
    {0,0},							// 2e6 OUT Ib,AL
    {0,0},							// 2e7 OUT Ib,EAX
    {0,0},							// 2e8 CALL Jd
    {0,0},							// 2e9 JMP Jd
    {0,0},							// 2ea JMP Ad
    {0,0},							// 2eb JMP Jb
    {0,0},							// 2ec IN AL,DX
    {0,0},							// 2ed IN EAX,DX
    {0,0},							// 2ee OUT DX,AL
    {0,0},							// 2ef OUT DX,EAX
    {0,0},							// 2f0 LOCK
    {0,0},							// 2f1 ICEBP
    {0,0},							// 2f2 REPNZ
    {0,0},							// 2f3 REPZ
    {0,0},							// 2f4 HLT
    {CF,CF},						// 2f5 CMC
    {GRP3,GRP3},					// 2f6 GRP3 Eb(,Ib)
    {GRP3,GRP3},					// 2f7 GRP3 Ed(,Id)
    {CF,0},							// 2f8 CLC
    {CF,0},							// 2f9 STC
    {0,0},							// 2fa CLI
    {0,0},							// 2fb STI
    {0,0},							// 2fc CLD
    {0,0},							// 2fd STD
    {GRP4,GRP4},					// 2fe GRP4 Eb
    {GRP5,GRP5},					// 2ff GRP5 Ed
    {GRP6,GRP6},					// 300 GRP 6 Exxx
    {0,0},							// 301 Group 7 Ed
    {ZF,0},							// 302 LAR Gd,Ed
    {ZF,0},							// 303 LSL Gd,Ew
    {0,0},							// 304
    {0,0},							// 305
    {0,0},							// 306 CLTS
    {0,0},							// 307
    {0,0},							// 308 INVD
    {0,0},							// 309 WBINVD
    {0,0},							// 30a
    {0,0},							// 30b
    {0,0},							// 30c
    {0,0},							// 30d
    {0,0},							// 30e
    {0,0},							// 30f
    {0,0},							// 310
    {0,0},							// 311
    {0,0},							// 312
    {0,0},							// 313
    {0,0},							// 314
    {0,0},							// 315
    {0,0},							// 316
    {0,0},							// 317
    {0,0},							// 318
    {0,0},							// 319
    {0,0},							// 31a
    {0,0},							// 31b
    {0,0},							// 31c
    {0,0},							// 31d
    {0,0},							// 31e
    {0,0},							// 31f
    {0,0},							// 320 MOV Rd.CRx
    {0,0},							// 321 MOV Rd,DRx
    {0,0},							// 322 MOV CRx,Rd
    {0,0},							// 323 MOV DRx,Rd
    {0,0},							// 324 MOV Rd,TRx
    {0,0},							// 325
    {0,0},							// 326 MOV TRx,Rd
    {0,0},							// 327
    {0,0},							// 328
    {0,0},							// 329
    {0,0},							// 32a
    {0,0},							// 32b
    {0,0},							// 32c
    {0,0},							// 32d
    {0,0},							// 32e
    {0,0},							// 32f
    {0,0},							// 330 WRMSR
    {0,0},							// 331 RDTSC
    {0,0},							// 332 RDMSR
    {0,0},							// 333
    {0,0},							// 334
    {0,0},							// 335
    {0,0},							// 336
    {0,0},							// 337
    {0,0},							// 338
    {0,0},							// 339
    {0,0},							// 33a
    {0,0},							// 33b
    {0,0},							// 33c
    {0,0},							// 33d
    {0,0},							// 33e
    {0,0},							// 33f
    {0,OF},							// 340 CMOVO
    {0,OF},							// 341 CMOVNO
    {0,CF},							// 342 CMOVB
    {0,CF},							// 343 CMOVNB
    {0,ZF},							// 344 CMOVZ
    {0,ZF},							// 345 CMOVNZ
    {0,CF|ZF},						// 346 CMOVBE
    {0,CF|ZF},						// 347 CMOVNBE
    {0,SF},							// 348 CMOVS
    {0,SF},							// 349 CMOVNS
    {0,PF},							// 34a CMOVP
    {0,PF},							// 34b CMOVNP
    {0,SF|OF},						// 34c CMOVL
    {0,SF|OF},						// 34d CMOVNL
    {0,SF|OF|ZF},					// 34e CMOVLE
    {0,SF|OF|ZF},					// 34f CMOVNLE
    {0,0},							// 350
    {0,0},							// 351
    {0,0},							// 352
    {0,0},							// 353
    {0,0},							// 354
    {0,0},							// 355
    {0,0},							// 356
    {0,0},							// 357
    {0,0},							// 358
    {0,0},							// 359
    {0,0},							// 35a
    {0,0},							// 35b
    {0,0},							// 35c
    {0,0},							// 35d
    {0,0},							// 35e
    {0,0},							// 35f
    {0,0},							// 360
    {0,0},							// 361
    {0,0},							// 362
    {0,0},							// 363
    {0,0},							// 364
    {0,0},							// 365
    {0,0},							// 366
    {0,0},							// 367
    {0,0},							// 368
    {0,0},							// 369
    {0,0},							// 36a
    {0,0},							// 36b
    {0,0},							// 36c
    {0,0},							// 36d
    {0,0},							// 36e
    {0,0},							// 36f
    {0,0},							// 370
    {0,0},							// 371
    {0,0},							// 372
    {0,0},							// 373
    {0,0},							// 374
    {0,0},							// 375
    {0,0},							// 376
    {0,0},							// 377
    {0,0},							// 378
    {0,0},							// 379
    {0,0},							// 37a
    {0,0},							// 37b
    {0,0},							// 37c
    {0,0},							// 37d
    {0,0},							// 37e
    {0,0},							// 37f
    {0,OF},							// 380 JO
    {0,OF},							// 381 JNO
    {0,CF},							// 382 JB
    {0,CF},							// 383 JNB
    {0,ZF},							// 384 JZ
    {0,ZF},							// 385 JNZ
    {0,CF|ZF},						// 386 JBE
    {0,CF|ZF},						// 387 JNBE
    {0,SF},							// 388 JS
    {0,SF},							// 389 JNS
    {0,PF},							// 38a JP
    {0,PF},							// 38b JNP
    {0,SF|OF},						// 38c JL
    {0,SF|OF},						// 38d JNL
    {0,SF|OF|ZF},					// 38e JLE
    {0,SF|OF|ZF},					// 38f JNLE
    {0,OF},							// 390 SETO
    {0,OF},							// 391 SETNO
    {0,CF},							// 392 SETB
    {0,CF},							// 393 SETNB
    {0,ZF},							// 394 SETZ
    {0,ZF},							// 395 SETNZ
    {0,CF|ZF},						// 396 SETBE
    {0,CF|ZF},						// 397 SETNBE
    {0,SF},							// 398 SETS
    {0,SF},							// 399 SETNS
    {0,PF},							// 39a SETP
    {0,PF},							// 39b SETNP
    {0,SF|OF},						// 39c SETL
    {0,SF|OF},						// 39d SETNL
    {0,SF|OF|ZF},					// 39e SETLE
    {0,SF|OF|ZF},					// 39f SETNLE
    {0,0},							// 3a0 PUSH FS
    {0,0},							// 3a1 POP FS
    {0,0},							// 3a2 CPUID
    {CF,0},							// 3a3 BT Ed,Gd
    {CF|AF|ZF|SF|OF|PF,0},			// 3a4 SHLD Ed,Gd,Ib: AF is alway 0
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 3a5 SHLD Ed,Gd,CL: AF is alway 0
    {0,0},							// 3a6
    {0,0},							// 3a7
    {0,0},							// 3a8 PUSH GS
    {0,0},							// 3a9 POP GS
    {0,0},							// 3aa
    {CF,0},							// 3ab BTS Ed,Gd
    {CF|AF|ZF|SF|OF|PF,0},			// 3ac SHRD Ed,Gd,Ib: AF is alway 0
    {CF|AF|ZF|SF|OF|PF|MAYBE,0},	// 3ad SHRD Ed,Gd,CL: AF is alway 0
    {0,0},							// 3ae
    {CF|OF,0},						// 3af IMUL Gd,Ed: SF, ZF, PF, AF are undefined
    {CF|AF|ZF|SF|OF|PF,0},			// 3b0 cmpxchg Eb,Gb
    {CF|AF|ZF|SF|OF|PF,0},			// 3b1 cmpxchg Ed,Gd
    {0,0},							// 3b2 LSS Ed
    {CF,0},							// 3b3 BTR Ed,Gd
    {0,0},							// 3b4 LFS Ed
    {0,0},							// 3b5 LGS Ed
    {0,0},							// 3b6 MOVZX Gd,Eb
    {0,0},							// 3b7 MOVZX Gd,Ew
    {0,0},							// 3b8
    {0,0},							// 3b9
    {CF,0},							// 3ba GRP8 Ed,Ib
    {CF,0},							// 3bb BTC Ed,Gd
    {ZF,0},							// 3bc BSF Gd,Ed: other flags are undefined
    {ZF,0},							// 3bd BSR Gd,Ed: other flags are undefined
    {0,0},							// 3be MOVSX Gd,Eb
    {0,0},							// 3bf MOVSX Gd,Ew
    {CF|AF|ZF|SF|OF|PF,0},			// 3c0 XADD Gb,Eb
    {CF|AF|ZF|SF|OF|PF,0},			// 3c1 XADD Gd,Ew
    {0,0},							// 3c2
    {0,0},							// 3c3
    {0,0},							// 3c4
    {0,0},							// 3c5
    {0,0},							// 3c6
    {0,0},							// 3c7
    {0,0},							// 3c8 BSWAP EAX
    {0,0},							// 3c9 BSWAP ECX
    {0,0},							// 3ca BSWAP EDX
    {0,0},							// 3cb BSWAP EBX
    {0,0},							// 3cc BSWAP ESP
    {0,0},							// 3cd BSWAP EBP
    {0,0},							// 3ce BSWAP ESI
    {0,0},							// 3cf BSWAP EDI
    {0,0},							// 3d0
    {0,0},							// 3d1
    {0,0},							// 3d2
    {0,0},							// 3d3
    {0,0},							// 3d4
    {0,0},							// 3d5
    {0,0},							// 3d6
    {0,0},							// 3d7
    {0,0},							// 3d8
    {0,0},							// 3d9
    {0,0},							// 3da
    {0,0},							// 3db
    {0,0},							// 3dc
    {0,0},							// 3dd
    {0,0},							// 3de
    {0,0},							// 3df
    {0,0},							// 3e0
    {0,0},							// 3e1
    {0,0},							// 3e2
    {0,0},							// 3e3
    {0,0},							// 3e4
    {0,0},							// 3e5
    {0,0},							// 3e6
    {0,0},							// 3e7
    {0,0},							// 3e8
    {0,0},							// 3e9
    {0,0},							// 3ea
    {0,0},							// 3eb
    {0,0},							// 3ec
    {0,0},							// 3ed
    {0,0},							// 3ee
    {0,0},							// 3ef
    {0,0},							// 3f0
    {0,0},							// 3f1
    {0,0},							// 3f2
    {0,0},							// 3f3
    {0,0},							// 3f4
    {0,0},							// 3f5
    {0,0},							// 3f6
    {0,0},							// 3f7
    {0,0},							// 3f8
    {0,0},							// 3f9
    {0,0},							// 3fa
    {0,0},							// 3fb
    {0,0},							// 3fc
    {0,0},							// 3fd
    {0,0},							// 3fe
    {0,0},							// 3ff
    {0,0},                          // empty op
};

void OPCALL add8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL add8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL or8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL or8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL adc8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL adc8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL and8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL and8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sub8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sub8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL xor8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL xor8_mem16(struct CPU* cpu, struct Op* op);
void decode080_noflags(struct Op* op) {
    if (op->func == add8_reg) {
        op->func = add8_reg_noflags;
    } else if (op->func == add8_mem32) {
        op->func = add8_mem32_noflags;
    } else if (op->func == add8_mem16) {
        op->func = add8_mem16_noflags;
    } 
    
    else if (op->func == or8_reg) {
        op->func = or8_reg_noflags;
    } else if (op->func == or8_mem32) {
        op->func = or8_mem32_noflags;
    } else if (op->func == or8_mem16) {
        op->func = or8_mem16_noflags;
    } 
    
    else if (op->func == adc8_reg) {
        op->func = adc8_reg_noflags;
    } else if (op->func == adc8_mem32) {
        op->func = adc8_mem32_noflags;
    } else if (op->func == adc8_mem16) {
        op->func = adc8_mem16_noflags;
    }

    else if (op->func == sbb8_reg) {
        op->func = sbb8_reg_noflags;
    } else if (op->func == sbb8_mem32) {
        op->func = sbb8_mem32_noflags;
    } else if (op->func == sbb8_mem16) {
        op->func = sbb8_mem16_noflags;
    }

    else if (op->func == and8_reg) {
        op->func = and8_reg_noflags;
    } else if (op->func == and8_mem32) {
        op->func = and8_mem32_noflags;
    } else if (op->func == and8_mem16) {
        op->func = and8_mem16_noflags;
    }

    else if (op->func == sub8_reg) {
        op->func = sub8_reg_noflags;
    } else if (op->func == sub8_mem32) {
        op->func = sub8_mem32_noflags;
    } else if (op->func == sub8_mem16) {
        op->func = sub8_mem16_noflags;
    }

    else if (op->func == xor8_reg) {
        op->func = xor8_reg_noflags;
    } else if (op->func == xor8_mem32) {
        op->func = xor8_mem32_noflags;
    } else if (op->func == xor8_mem16) {
        op->func = xor8_mem16_noflags;
    }

    else {
        kpanic("decode080_noflags error");
    }
}

void OPCALL add32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL add32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL or32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL or32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL adc32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL adc32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL and32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL and32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sub32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sub32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL xor32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL xor32_mem16(struct CPU* cpu, struct Op* op);
void decode281_noflags(struct Op* op) {
    if (op->func == add32_reg) {
        op->func = add32_reg_noflags;
    } else if (op->func == add32_mem32) {
        op->func = add32_mem32_noflags;
    } else if (op->func == add32_mem16) {
        op->func = add32_mem16_noflags;
    } 
    
    else if (op->func == or32_reg) {
        op->func = or32_reg_noflags;
    } else if (op->func == or32_mem32) {
        op->func = or32_mem32_noflags;
    } else if (op->func == or32_mem16) {
        op->func = or32_mem16_noflags;
    } 
    
    else if (op->func == adc32_reg) {
        op->func = adc32_reg_noflags;
    } else if (op->func == adc32_mem32) {
        op->func = adc32_mem32_noflags;
    } else if (op->func == adc32_mem16) {
        op->func = adc32_mem16_noflags;
    }

    else if (op->func == sbb32_reg) {
        op->func = sbb32_reg_noflags;
    } else if (op->func == sbb32_mem32) {
        op->func = sbb32_mem32_noflags;
    } else if (op->func == sbb32_mem16) {
        op->func = sbb32_mem16_noflags;
    }

    else if (op->func == and32_reg) {
        op->func = and32_reg_noflags;
    } else if (op->func == and32_mem32) {
        op->func = and32_mem32_noflags;
    } else if (op->func == and32_mem16) {
        op->func = and32_mem16_noflags;
    }

    else if (op->func == sub32_reg) {
        op->func = sub32_reg_noflags;
    } else if (op->func == sub32_mem32) {
        op->func = sub32_mem32_noflags;
    } else if (op->func == sub32_mem16) {
        op->func = sub32_mem16_noflags;
    }

    else if (op->func == xor32_reg) {
        op->func = xor32_reg_noflags;
    } else if (op->func == xor32_mem32) {
        op->func = xor32_mem32_noflags;
    } else if (op->func == xor32_mem16) {
        op->func = xor32_mem16_noflags;
    }

    else {
        kpanic("decode281_noflags error");
    }
}

void OPCALL add16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL add16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL or16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL or16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL adc16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL adc16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL and16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL and16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sub16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sub16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL xor16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL xor16_mem16(struct CPU* cpu, struct Op* op);
void decode081_noflags(struct Op* op) {
    if (op->func == add16_reg) {
        op->func = add16_reg_noflags;
    } else if (op->func == add16_mem32) {
        op->func = add16_mem32_noflags;
    } else if (op->func == add16_mem16) {
        op->func = add16_mem16_noflags;
    } 
    
    else if (op->func == or16_reg) {
        op->func = or16_reg_noflags;
    } else if (op->func == or16_mem32) {
        op->func = or16_mem32_noflags;
    } else if (op->func == or16_mem16) {
        op->func = or16_mem16_noflags;
    } 
    
    else if (op->func == adc16_reg) {
        op->func = adc16_reg_noflags;
    } else if (op->func == adc16_mem32) {
        op->func = adc16_mem32_noflags;
    } else if (op->func == adc16_mem16) {
        op->func = adc16_mem16_noflags;
    }

    else if (op->func == sbb16_reg) {
        op->func = sbb16_reg_noflags;
    } else if (op->func == sbb16_mem32) {
        op->func = sbb16_mem32_noflags;
    } else if (op->func == sbb16_mem16) {
        op->func = sbb16_mem16_noflags;
    }

    else if (op->func == and16_reg) {
        op->func = and16_reg_noflags;
    } else if (op->func == and16_mem32) {
        op->func = and16_mem32_noflags;
    } else if (op->func == and16_mem16) {
        op->func = and16_mem16_noflags;
    }

    else if (op->func == sub16_reg) {
        op->func = sub16_reg_noflags;
    } else if (op->func == sub16_mem32) {
        op->func = sub16_mem32_noflags;
    } else if (op->func == sub16_mem16) {
        op->func = sub16_mem16_noflags;
    }

    else if (op->func == xor16_reg) {
        op->func = xor16_reg_noflags;
    } else if (op->func == xor16_mem32) {
        op->func = xor16_mem32_noflags;
    } else if (op->func == xor16_mem16) {
        op->func = xor16_mem16_noflags;
    }

    else {
        kpanic("decode081_noflags error");
    }
}

void OPCALL rol8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rol8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rol8_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL ror8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL ror8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL ror8_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl8_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr8_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shl8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shl8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shl8_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shr8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shr8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shr8_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL sar8_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL sar8_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL sar8_mem32_op(struct CPU* cpu, struct Op* op);
void decode0c0_noflags(struct Op* op) {
    if (op->func == rol8_reg_op) {
        op->func = rol8_reg_noflags;
    } else if (op->func == rol8_mem32_op) {
        op->func = rol8_mem32_noflags;
    } else if (op->func == rol8_mem16_op) {
        op->func = rol8_mem16_noflags;
    } 
    
    else if (op->func == ror8_reg_op) {
        op->func = ror8_reg_noflags;
    } else if (op->func == ror8_mem32_op) {
        op->func = ror8_mem32_noflags;
    } else if (op->func == ror8_mem16_op) {
        op->func = ror8_mem16_noflags;
    } 
    
    else if (op->func == rcl8_reg_op) {
        op->func = rcl8_reg_noflags;
    } else if (op->func == rcl8_mem32_op) {
        op->func = rcl8_mem32_noflags;
    } else if (op->func == rcl8_mem16_op) {
        op->func = rcl8_mem16_noflags;
    }

    else if (op->func == rcr8_reg_op) {
        op->func = rcr8_reg_noflags;
    } else if (op->func == rcr8_mem32_op) {
        op->func = rcr8_mem32_noflags;
    } else if (op->func == rcr8_mem16_op) {
        op->func = rcr8_mem16_noflags;
    }

    else if (op->func == shl8_reg_op) {
        op->func = shl8_reg_noflags;
    } else if (op->func == shl8_mem32_op) {
        op->func = shl8_mem32_noflags;
    } else if (op->func == shl8_mem16_op) {
        op->func = shl8_mem16_noflags;
    }

    else if (op->func == shr8_reg_op) {
        op->func = shr8_reg_noflags;
    } else if (op->func == shr8_mem32_op) {
        op->func = shr8_mem32_noflags;
    } else if (op->func == shr8_mem16_op) {
        op->func = shr8_mem16_noflags;
    }

    else if (op->func == sar8_reg_op) {
        op->func = sar8_reg_noflags;
    } else if (op->func == sar8_mem32_op) {
        op->func = sar8_mem32_noflags;
    } else if (op->func == sar8_mem16_op) {
        op->func = sar8_mem16_noflags;
    }

    else {
        kpanic("decode0c0_noflags error");
    }
}

void OPCALL rol8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rol8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rol8cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL ror8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL ror8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL ror8cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl8cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr8cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shl8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shl8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shl8cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shr8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shr8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shr8cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL sar8cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL sar8cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL sar8cl_mem32_op(struct CPU* cpu, struct Op* op);
void decode0d2_noflags(struct Op* op) {
    if (op->func == rol8cl_reg_op) {
        op->func = rol8cl_reg_noflags;
    } else if (op->func == rol8cl_mem32_op) {
        op->func = rol8cl_mem32_noflags;
    } else if (op->func == rol8cl_mem16_op) {
        op->func = rol8cl_mem16_noflags;
    } 
    
    else if (op->func == ror8cl_reg_op) {
        op->func = ror8cl_reg_noflags;
    } else if (op->func == ror8cl_mem32_op) {
        op->func = ror8cl_mem32_noflags;
    } else if (op->func == ror8cl_mem16_op) {
        op->func = ror8cl_mem16_noflags;
    } 
    
    else if (op->func == rcl8cl_reg_op) {
        op->func = rcl8cl_reg_noflags;
    } else if (op->func == rcl8cl_mem32_op) {
        op->func = rcl8cl_mem32_noflags;
    } else if (op->func == rcl8cl_mem16_op) {
        op->func = rcl8cl_mem16_noflags;
    }

    else if (op->func == rcr8cl_reg_op) {
        op->func = rcr8cl_reg_noflags;
    } else if (op->func == rcr8cl_mem32_op) {
        op->func = rcr8cl_mem32_noflags;
    } else if (op->func == rcr8cl_mem16_op) {
        op->func = rcr8cl_mem16_noflags;
    }

    else if (op->func == shl8cl_reg_op) {
        op->func = shl8cl_reg_noflags;
    } else if (op->func == shl8cl_mem32_op) {
        op->func = shl8cl_mem32_noflags;
    } else if (op->func == shl8cl_mem16_op) {
        op->func = shl8cl_mem16_noflags;
    }

    else if (op->func == shr8cl_reg_op) {
        op->func = shr8cl_reg_noflags;
    } else if (op->func == shr8cl_mem32_op) {
        op->func = shr8cl_mem32_noflags;
    } else if (op->func == shr8cl_mem16_op) {
        op->func = shr8cl_mem16_noflags;
    }

    else if (op->func == sar8cl_reg_op) {
        op->func = sar8cl_reg_noflags;
    } else if (op->func == sar8cl_mem32_op) {
        op->func = sar8cl_mem32_noflags;
    } else if (op->func == sar8cl_mem16_op) {
        op->func = sar8cl_mem16_noflags;
    }

    else {
        kpanic("decode0d2_noflags error");
    }
}

void OPCALL rol32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rol32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rol32_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL ror32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL ror32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL ror32_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl32_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr32_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shl32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shl32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shl32_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shr32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shr32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shr32_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL sar32_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL sar32_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL sar32_mem32_op(struct CPU* cpu, struct Op* op);
void decode2c1_noflags(struct Op* op) {
    if (op->func == rol32_reg_op) {
        op->func = rol32_reg_noflags;
    } else if (op->func == rol32_mem32_op) {
        op->func = rol32_mem32_noflags;
    } else if (op->func == rol32_mem16_op) {
        op->func = rol32_mem16_noflags;
    } 
    
    else if (op->func == ror32_reg_op) {
        op->func = ror32_reg_noflags;
    } else if (op->func == ror32_mem32_op) {
        op->func = ror32_mem32_noflags;
    } else if (op->func == ror32_mem16_op) {
        op->func = ror32_mem16_noflags;
    } 
    
    else if (op->func == rcl32_reg_op) {
        op->func = rcl32_reg_noflags;
    } else if (op->func == rcl32_mem32_op) {
        op->func = rcl32_mem32_noflags;
    } else if (op->func == rcl32_mem16_op) {
        op->func = rcl32_mem16_noflags;
    }

    else if (op->func == rcr32_reg_op) {
        op->func = rcr32_reg_noflags;
    } else if (op->func == rcr32_mem32_op) {
        op->func = rcr32_mem32_noflags;
    } else if (op->func == rcr32_mem16_op) {
        op->func = rcr32_mem16_noflags;
    }

    else if (op->func == shl32_reg_op) {
        op->func = shl32_reg_noflags;
    } else if (op->func == shl32_mem32_op) {
        op->func = shl32_mem32_noflags;
    } else if (op->func == shl32_mem16_op) {
        op->func = shl32_mem16_noflags;
    }

    else if (op->func == shr32_reg_op) {
        op->func = shr32_reg_noflags;
    } else if (op->func == shr32_mem32_op) {
        op->func = shr32_mem32_noflags;
    } else if (op->func == shr32_mem16_op) {
        op->func = shr32_mem16_noflags;
    }

    else if (op->func == sar32_reg_op) {
        op->func = sar32_reg_noflags;
    } else if (op->func == sar32_mem32_op) {
        op->func = sar32_mem32_noflags;
    } else if (op->func == sar32_mem16_op) {
        op->func = sar32_mem16_noflags;
    }

    else {
        kpanic("decode2c1_noflags error");
    }
}

void OPCALL rol32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rol32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rol32cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL ror32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL ror32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL ror32cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl32cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr32cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shl32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shl32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shl32cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shr32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shr32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shr32cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL sar32cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL sar32cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL sar32cl_mem32_op(struct CPU* cpu, struct Op* op);
void decode2d3_noflags(struct Op* op) {
    if (op->func == rol32cl_reg_op) {
        op->func = rol32cl_reg_noflags;
    } else if (op->func == rol32cl_mem32_op) {
        op->func = rol32cl_mem32_noflags;
    } else if (op->func == rol32cl_mem16_op) {
        op->func = rol32cl_mem16_noflags;
    } 
    
    else if (op->func == ror32cl_reg_op) {
        op->func = ror32cl_reg_noflags;
    } else if (op->func == ror32cl_mem32_op) {
        op->func = ror32cl_mem32_noflags;
    } else if (op->func == ror32cl_mem16_op) {
        op->func = ror32cl_mem16_noflags;
    } 
    
    else if (op->func == rcl32cl_reg_op) {
        op->func = rcl32cl_reg_noflags;
    } else if (op->func == rcl32cl_mem32_op) {
        op->func = rcl32cl_mem32_noflags;
    } else if (op->func == rcl32cl_mem16_op) {
        op->func = rcl32cl_mem16_noflags;
    }

    else if (op->func == rcr32cl_reg_op) {
        op->func = rcr32cl_reg_noflags;
    } else if (op->func == rcr32cl_mem32_op) {
        op->func = rcr32cl_mem32_noflags;
    } else if (op->func == rcr32cl_mem16_op) {
        op->func = rcr32cl_mem16_noflags;
    }

    else if (op->func == shl32cl_reg_op) {
        op->func = shl32cl_reg_noflags;
    } else if (op->func == shl32cl_mem32_op) {
        op->func = shl32cl_mem32_noflags;
    } else if (op->func == shl32cl_mem16_op) {
        op->func = shl32cl_mem16_noflags;
    }

    else if (op->func == shr32cl_reg_op) {
        op->func = shr32cl_reg_noflags;
    } else if (op->func == shr32cl_mem32_op) {
        op->func = shr32cl_mem32_noflags;
    } else if (op->func == shr32cl_mem16_op) {
        op->func = shr32cl_mem16_noflags;
    }

    else if (op->func == sar32cl_reg_op) {
        op->func = sar32cl_reg_noflags;
    } else if (op->func == sar32cl_mem32_op) {
        op->func = sar32cl_mem32_noflags;
    } else if (op->func == sar32cl_mem16_op) {
        op->func = sar32cl_mem16_noflags;
    }

    else {
        kpanic("decode2d3_noflags error");
    }
}

void OPCALL rol16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rol16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rol16_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL ror16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL ror16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL ror16_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl16_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr16_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shl16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shl16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shl16_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shr16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shr16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shr16_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL sar16_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL sar16_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL sar16_mem32_op(struct CPU* cpu, struct Op* op);
void decode0c1_noflags(struct Op* op) {
    if (op->func == rol16_reg_op) {
        op->func = rol16_reg_noflags;
    } else if (op->func == rol16_mem32_op) {
        op->func = rol16_mem32_noflags;
    } else if (op->func == rol16_mem16_op) {
        op->func = rol16_mem16_noflags;
    } 
    
    else if (op->func == ror16_reg_op) {
        op->func = ror16_reg_noflags;
    } else if (op->func == ror16_mem32_op) {
        op->func = ror16_mem32_noflags;
    } else if (op->func == ror16_mem16_op) {
        op->func = ror16_mem16_noflags;
    } 
    
    else if (op->func == rcl16_reg_op) {
        op->func = rcl16_reg_noflags;
    } else if (op->func == rcl16_mem32_op) {
        op->func = rcl16_mem32_noflags;
    } else if (op->func == rcl16_mem16_op) {
        op->func = rcl16_mem16_noflags;
    }

    else if (op->func == rcr16_reg_op) {
        op->func = rcr16_reg_noflags;
    } else if (op->func == rcr16_mem32_op) {
        op->func = rcr16_mem32_noflags;
    } else if (op->func == rcr16_mem16_op) {
        op->func = rcr16_mem16_noflags;
    }

    else if (op->func == shl16_reg_op) {
        op->func = shl16_reg_noflags;
    } else if (op->func == shl16_mem32_op) {
        op->func = shl16_mem32_noflags;
    } else if (op->func == shl16_mem16_op) {
        op->func = shl16_mem16_noflags;
    }

    else if (op->func == shr16_reg_op) {
        op->func = shr16_reg_noflags;
    } else if (op->func == shr16_mem32_op) {
        op->func = shr16_mem32_noflags;
    } else if (op->func == shr16_mem16_op) {
        op->func = shr16_mem16_noflags;
    }

    else if (op->func == sar16_reg_op) {
        op->func = sar16_reg_noflags;
    } else if (op->func == sar16_mem32_op) {
        op->func = sar16_mem32_noflags;
    } else if (op->func == sar16_mem16_op) {
        op->func = sar16_mem16_noflags;
    }

    else {
        kpanic("decode0c1_noflags error");
    }
}

void OPCALL rol16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rol16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rol16cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL ror16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL ror16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL ror16cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcl16cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL rcr16cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shl16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shl16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shl16cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL shr16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL shr16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL shr16cl_mem32_op(struct CPU* cpu, struct Op* op);
void OPCALL sar16cl_reg_op(struct CPU* cpu, struct Op* op);
void OPCALL sar16cl_mem16_op(struct CPU* cpu, struct Op* op);
void OPCALL sar16cl_mem32_op(struct CPU* cpu, struct Op* op);
void decode0d3_noflags(struct Op* op) {
    if (op->func == rol16cl_reg_op) {
        op->func = rol16cl_reg_noflags;
    } else if (op->func == rol16cl_mem32_op) {
        op->func = rol16cl_mem32_noflags;
    } else if (op->func == rol16cl_mem16_op) {
        op->func = rol16cl_mem16_noflags;
    } 
    
    else if (op->func == ror16cl_reg_op) {
        op->func = ror16cl_reg_noflags;
    } else if (op->func == ror16cl_mem32_op) {
        op->func = ror16cl_mem32_noflags;
    } else if (op->func == ror16cl_mem16_op) {
        op->func = ror16cl_mem16_noflags;
    } 
    
    else if (op->func == rcl16cl_reg_op) {
        op->func = rcl16cl_reg_noflags;
    } else if (op->func == rcl16cl_mem32_op) {
        op->func = rcl16cl_mem32_noflags;
    } else if (op->func == rcl16cl_mem16_op) {
        op->func = rcl16cl_mem16_noflags;
    }

    else if (op->func == rcr16cl_reg_op) {
        op->func = rcr16cl_reg_noflags;
    } else if (op->func == rcr16cl_mem32_op) {
        op->func = rcr16cl_mem32_noflags;
    } else if (op->func == rcr16cl_mem16_op) {
        op->func = rcr16cl_mem16_noflags;
    }

    else if (op->func == shl16cl_reg_op) {
        op->func = shl16cl_reg_noflags;
    } else if (op->func == shl16cl_mem32_op) {
        op->func = shl16cl_mem32_noflags;
    } else if (op->func == shl16cl_mem16_op) {
        op->func = shl16cl_mem16_noflags;
    }

    else if (op->func == shr16cl_reg_op) {
        op->func = shr16cl_reg_noflags;
    } else if (op->func == shr16cl_mem32_op) {
        op->func = shr16cl_mem32_noflags;
    } else if (op->func == shr16cl_mem16_op) {
        op->func = shr16cl_mem16_noflags;
    }

    else if (op->func == sar16cl_reg_op) {
        op->func = sar16cl_reg_noflags;
    } else if (op->func == sar16cl_mem32_op) {
        op->func = sar16cl_mem32_noflags;
    } else if (op->func == sar16cl_mem16_op) {
        op->func = sar16cl_mem16_noflags;
    }

    else {
        kpanic("decode0d3_noflags error");
    }
}

void OPCALL inc16_reg(struct CPU* cpu, struct Op* op);
void decode040_noflags(struct Op* op) {
    if (op->func == inc16_reg) {
        op->func = inc16_reg_noflags;
    } else {
        kpanic("decode040_noflags error");
    }
}

void OPCALL dec16_reg(struct CPU* cpu, struct Op* op);
void decode048_noflags(struct Op* op) {
    if (op->func == dec16_reg) {
        op->func = dec16_reg_noflags;
    } else {
        kpanic("decode048_noflags error");
    }
}

void OPCALL inc32_reg(struct CPU* cpu, struct Op* op);
void decode240_noflags(struct Op* op) {
    if (op->func == inc32_reg) {
        op->func = inc32_reg_noflags;
    } else {
        kpanic("decode240_noflags error");
    }
}

void OPCALL dec32_reg(struct CPU* cpu, struct Op* op);
void decode248_noflags(struct Op* op) {
    if (op->func == dec32_reg) {
        op->func = dec32_reg_noflags;
    } else {
        kpanic("decode240_noflags error");
    }
}

void OPCALL neg32_reg_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 =  0-cpu->reg[op->r1].u32;
    CYCLES(1);
    NEXT();
}

void OPCALL neg32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    writed(cpu->thread, eaa, 0-readd(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL neg32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    writed(cpu->thread, eaa, 0-readd(cpu->thread, eaa));
    CYCLES(3);
    NEXT();
}

void OPCALL mul32_reg_noflags(struct CPU* cpu, struct Op* op) {
    U64 result = (U64)EAX * cpu->reg[op->r1].u32;
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    CYCLES(10);
    NEXT();
}

void OPCALL mul32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    U64 result = (U64)EAX * readd(cpu->thread, eaa16(cpu, op));
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    CYCLES(10);
    NEXT();
}

void OPCALL mul32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    U64 result = (U64)EAX * readd(cpu->thread, eaa32(cpu, op));
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
    CYCLES(10);
    NEXT();
}

void OPCALL imul32_reg_noflags(struct CPU* cpu, struct Op* op) {
    S64 result = (S64)((S32)EAX) * (S32)cpu->reg[op->r1].u32;
    EAX = (S32)result;
    EDX = (S32)(result >> 32);
    CYCLES(10);
    NEXT();
}

void OPCALL imul32_mem16_noflags(struct CPU* cpu, struct Op* op) {
    S64 result = (S64)((S32)EAX) * (S32)readd(cpu->thread, eaa16(cpu, op));
    EAX = (S32)result;
    EDX = (S32)(result >> 32);
    CYCLES(10);
    NEXT();
}

void OPCALL imul32_mem32_noflags(struct CPU* cpu, struct Op* op) {
    S64 result = (S64)((S32)EAX) * (S32)readd(cpu->thread, eaa32(cpu, op));
    EAX = (S32)result;
    EDX = (S32)(result >> 32);
    CYCLES(10);
    NEXT();
}

void OPCALL test32_reg(struct CPU* cpu, struct Op* op);
void OPCALL test32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL test32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL neg32_reg(struct CPU* cpu, struct Op* op);
void OPCALL neg32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL neg32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL mul32_reg(struct CPU* cpu, struct Op* op);
void OPCALL mul32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mul32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL imul32_reg(struct CPU* cpu, struct Op* op);
void OPCALL imul32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL imul32_mem32(struct CPU* cpu, struct Op* op);
void decode2f7_noflags(struct Op* op) {
    if (op->func == test32_reg) {
        op->func = test32_reg_noflags;
    } else if (op->func == test32_mem32) {
        op->func = test32_mem32_noflags;
    } else if (op->func == test32_mem16) {
        op->func = test32_mem16_noflags;
    } else if (op->func == neg32_reg) {
        op->func = neg32_reg_noflags;
    } else if (op->func == neg32_mem16) {
        op->func = neg32_mem16_noflags;
    } else if (op->func == neg32_mem32) {
        op->func = neg32_mem32_noflags;
    } else if (op->func == mul32_reg) {
        op->func = mul32_reg_noflags;
    } else if (op->func == mul32_mem16) {
        op->func = mul32_mem16_noflags;
    } else if (op->func == mul32_mem32) {
        op->func = mul32_mem32_noflags;
    } else if (op->func == imul32_reg) {
        op->func = imul32_reg_noflags;
    } else if (op->func == imul32_mem16) {
        op->func = imul32_mem16_noflags;
    } else if (op->func == imul32_mem32) {
        op->func = imul32_mem32_noflags;
    } else {
        kpanic("decode2f7_noflags error");
    }
}

void OPCALL inc8_reg(struct CPU* cpu, struct Op* op);
void OPCALL inc8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL inc8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL dec8_reg(struct CPU* cpu, struct Op* op);
void OPCALL dec8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL dec8_mem16(struct CPU* cpu, struct Op* op);
void decode0fe_noflags(struct Op* op) {
    if (op->func == inc8_reg) {
        op->func = inc8_reg_noflags;
    } else if (op->func == inc8_mem32) {
        op->func = inc8_mem32_noflags;
    } else if (op->func == inc8_mem16) {
        op->func = inc8_mem16_noflags;
    } else if (op->func == dec8_reg) {
        op->func = dec8_reg_noflags;
    } else if (op->func == dec8_mem32) {
        op->func = dec8_mem32_noflags;
    } else if (op->func == dec8_mem16) {
        op->func = dec8_mem16_noflags;
    } else {
        kpanic("decode0fe_noflags error");
    }
}

void OPCALL inc32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL inc32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL dec32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL dec32_mem16(struct CPU* cpu, struct Op* op);
void decode2ff_noflags(struct Op* op) {
    if (op->func == inc32_reg) {
        op->func = inc32_reg_noflags;
    } else if (op->func == inc32_mem32) {
        op->func = inc32_mem32_noflags;
    } else if (op->func == inc32_mem16) {
        op->func = inc32_mem16_noflags;
    } else if (op->func == dec32_reg) {
        op->func = dec32_reg_noflags;
    } else if (op->func == dec32_mem32) {
        op->func = dec32_mem32_noflags;
    } else if (op->func == dec32_mem16) {
        op->func = dec32_mem16_noflags;
    } else {
        kpanic("decode2ff_noflags error");
    }
}

void OPCALL dimulr32r32_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S32)(cpu->reg[op->r2].u32) * ((S32)cpu->reg[op->r1].u32);
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr32e32_16_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S32)(readd(cpu->thread, eaa16(cpu, op))) * ((S32)cpu->reg[op->r1].u32);
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr32e32_32_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S32)(readd(cpu->thread, eaa32(cpu, op))) * ((S32)cpu->reg[op->r1].u32);
    CYCLES(10);
    NEXT();
}

void OPCALL dimulr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32e32_32(struct CPU* cpu, struct Op* op);
void decode3af_noflags(struct Op* op) {
    if (op->func == dimulr32r32) {
        op->func = dimulr32r32_noflags;
    } else if (op->func == dimulr32e32_32) {
        op->func = dimulr32e32_32_noflags;
    } else if (op->func == dimulr32e32_16) {
        op->func = dimulr32e32_16_noflags;
    } else {
        kpanic("decode3af_noflags error");
    }
}

void OPCALL dimulcr32r32_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S32)(cpu->reg[op->r2].u32) * ((S32)op->data1);
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr32e32_16_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S32)(readd(cpu->thread, eaa16(cpu, op))) * ((S32)op->data1);
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr32e32_32_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (S32)(readd(cpu->thread, eaa32(cpu, op))) * ((S32)op->data1);
    CYCLES(10);
    NEXT();
}

void OPCALL dimulcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32e32_32(struct CPU* cpu, struct Op* op);
void decode269_noflags(struct Op* op) {
    if (op->func == dimulcr32r32) {
        op->func = dimulcr32r32_noflags;
    } else if (op->func == dimulcr32e32_32) {
        op->func = dimulcr32e32_32_noflags;
    } else if (op->func == dimulcr32e32_16) {
        op->func = dimulcr32e32_16_noflags;
    } else {
        kpanic("decode269_noflags error");
    }
}

void OPCALL cmpxchgr32r32_noflags(struct CPU* cpu, struct Op* op) {
    if (EAX == cpu->reg[op->r1].u32) {
        cpu->reg[op->r1].u32 = cpu->reg[op->r2].u32;
    } else {
        EAX = cpu->reg[op->r1].u32;
    }
    CYCLES(5);
    NEXT();
}

void OPCALL cmpxchge32r32_16_noflags(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    U32 dst = readd(cpu->thread, address);
    if (EAX == dst) {
        writed(cpu->thread, address, cpu->reg[op->r1].u32);
    } else {
        EAX = dst;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL cmpxchge32r32_32_noflags(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    U32 dst = readd(cpu->thread, address);
    if (EAX == dst) {
        writed(cpu->thread, address, cpu->reg[op->r1].u32);
    } else {
        EAX = dst;
    }
    CYCLES(6);
    NEXT();
}

void OPCALL cmpxchgr32r32(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge32r32_32(struct CPU* cpu, struct Op* op);
void decode3b1_noflags(struct Op* op) {
    if (op->func == cmpxchgr32r32) {
        op->func = cmpxchgr32r32_noflags;
    } else if (op->func == cmpxchge32r32_32) {
        op->func = cmpxchge32r32_32_noflags;
    } else if (op->func == cmpxchge32r32_16) {
        op->func = cmpxchge32r32_16_noflags;
    } else {
        kpanic("decode269_noflags error");
    }
}

void OPCALL dshrr32r32_noflags(struct CPU* cpu, struct Op* op) {
    cpu->reg[op->r1].u32 = (cpu->reg[op->r1].u32 >> op->data1) | (cpu->reg[op->r2].u32 << (32-op->data1));	
    CYCLES(4);
    NEXT();
}

void OPCALL dshre32r32_16_noflags(struct CPU* cpu, struct Op* op) {
    U32 address = eaa16(cpu, op);
    writed(cpu->thread, address, (readd(cpu->thread, address) >> op->data1) | (cpu->reg[op->r1].u32 << (32-op->data1)));
    CYCLES(4);
    NEXT();
}

void OPCALL dshre32r32_32_noflags(struct CPU* cpu, struct Op* op) {
    U32 address = eaa32(cpu, op);
    writed(cpu->thread, address, (readd(cpu->thread, address) >> op->data1) | (cpu->reg[op->r1].u32 << (32-op->data1)));
    CYCLES(4);
    NEXT();
}

void OPCALL dshrr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dshre32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL dshre32r32_32(struct CPU* cpu, struct Op* op);
void decode3ac_noflags(struct Op* op) {
    if (op->func == dshrr32r32) {
        op->func = dshrr32r32_noflags;
    } else if (op->func == dshre32r32_32) {
        op->func = dshre32r32_32_noflags;
    } else if (op->func == dshre32r32_16) {
        op->func = dshre32r32_16_noflags;
    } else {
        kpanic("decode3ac_noflags error");
    }
}

void OPCALL xadd32r32r32_noflags(struct CPU* cpu, struct Op* op) {
    U32 tmp = cpu->reg[op->r2].u32;
    cpu->reg[op->r2].u32 = cpu->reg[op->r1].u32+cpu->reg[op->r2].u32;
    cpu->reg[op->r1].u32 = tmp;
    CYCLES(3);
    NEXT();
}

void OPCALL xadd32r32e32_16_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa16(cpu, op);
    U32 tmp = readd(cpu->thread, eaa);
    writed(cpu->thread, eaa, tmp + cpu->reg[op->r1].u32);
    cpu->reg[op->r1].u32 = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xadd32r32e32_32_noflags(struct CPU* cpu, struct Op* op) {
    U32 eaa = eaa32(cpu, op);
    U32 tmp = readd(cpu->thread, eaa);
    writed(cpu->thread, eaa, tmp + cpu->reg[op->r1].u32);
    cpu->reg[op->r1].u32 = tmp;
    CYCLES(4);
    NEXT();
}

void OPCALL xadd32r32r32(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32e32_32(struct CPU* cpu, struct Op* op);
void decode3c1_noflags(struct Op* op) {
    if (op->func == xadd32r32r32) {
        op->func = xadd32r32r32_noflags;
    } else if (op->func == xadd32r32e32_32) {
        op->func = xadd32r32e32_32_noflags;
    } else if (op->func == xadd32r32e32_16) {
        op->func = xadd32r32e32_16_noflags;
    } else {
        kpanic("decode3c1_noflags error");
    }
}

typedef void (*FAST_DECODER)(struct Op* op);

FAST_DECODER fastDecoder[1024] = {
    decode000_noflags, decode001_noflags, decode002_noflags, decode003_noflags, decode004_noflags, decode005_noflags, 0, 0,
    decode008_noflags, decode009_noflags, decode00a_noflags, decode00b_noflags, decode00c_noflags, decode00d_noflags, 0, 0,
    decode010_noflags, decode011_noflags, decode012_noflags, decode013_noflags, decode014_noflags, decode015_noflags, 0, 0,
    decode018_noflags, decode019_noflags, decode01a_noflags, decode01b_noflags, decode01c_noflags, decode01d_noflags, 0, 0,
    decode020_noflags, decode021_noflags, decode022_noflags, decode023_noflags, decode024_noflags, decode025_noflags, 0, 0,
    decode028_noflags, decode029_noflags, decode02a_noflags, decode02b_noflags, decode02c_noflags, decode02d_noflags, 0, 0,
    decode030_noflags, decode031_noflags, decode032_noflags, decode033_noflags, decode034_noflags, decode035_noflags, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    decode040_noflags, decode040_noflags, decode040_noflags, decode040_noflags, decode040_noflags, decode040_noflags, decode040_noflags, decode040_noflags,
    decode048_noflags, decode048_noflags, decode048_noflags, decode048_noflags, decode048_noflags, decode048_noflags, decode048_noflags, decode048_noflags,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 0x080
    decode080_noflags, decode081_noflags, decode080_noflags, decode081_noflags, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    decode0c0_noflags, decode0c1_noflags, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    decode0c0_noflags, decode0c1_noflags, decode0d2_noflags, decode0d3_noflags, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, decode0fe_noflags, 0,
    // 0x100
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 0x180
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 0x200
    decode000_noflags, decode201_noflags, decode002_noflags, decode203_noflags, decode004_noflags, decode205_noflags, 0, 0,
    decode008_noflags, decode209_noflags, decode00a_noflags, decode20b_noflags, decode00c_noflags, decode20d_noflags, 0, 0,
    decode010_noflags, decode211_noflags, decode012_noflags, decode213_noflags, decode014_noflags, decode215_noflags, 0, 0,
    decode018_noflags, decode219_noflags, decode01a_noflags, decode21b_noflags, decode01c_noflags, decode21d_noflags, 0, 0,
    decode020_noflags, decode221_noflags, decode022_noflags, decode223_noflags, decode024_noflags, decode225_noflags, 0, 0,
    decode028_noflags, decode229_noflags, decode02a_noflags, decode22b_noflags, decode02c_noflags, decode22d_noflags, 0, 0,
    decode030_noflags, decode231_noflags, decode032_noflags, decode233_noflags, decode034_noflags, decode235_noflags, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    decode240_noflags, decode240_noflags, decode240_noflags, decode240_noflags, decode240_noflags, decode240_noflags, decode240_noflags, decode240_noflags,
    decode248_noflags, decode248_noflags, decode248_noflags, decode248_noflags, decode248_noflags, decode248_noflags, decode248_noflags, decode248_noflags,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, decode269_noflags, 0, decode269_noflags, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 0x280
    decode080_noflags, decode281_noflags, decode080_noflags, decode281_noflags, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    decode0c0_noflags, decode2c1_noflags, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    decode0c0_noflags, decode2c1_noflags, decode0d2_noflags, decode2d3_noflags, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, decode2f7_noflags,
    0, 0, 0, 0, 0, 0, decode0fe_noflags, decode2ff_noflags,
    // 0x300
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 0x380
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, decode3ac_noflags, 0, 0, decode3af_noflags,
    0, decode3b1_noflags, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, decode3c1_noflags, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

void OPCALL pushReg32(struct CPU* cpu, struct Op* op);
void OPCALL push2Reg32(struct CPU* cpu, struct Op* op){
    push32(cpu, cpu->reg[op->r1].u32);
    push32(cpu, cpu->reg[op->r2].u32);
    CYCLES(2);
    NEXT();
}
void OPCALL popReg32(struct CPU* cpu, struct Op* op);
void OPCALL pop2Reg32(struct CPU * cpu, struct Op * op){
    cpu->reg[op->r1].u32 = pop32(cpu);
    cpu->reg[op->r2].u32 = pop32(cpu);
    CYCLES(2);
    NEXT();
}

U16 flagsThatOpUses(struct Op* op) {
    U16 gFlag = opInfo[op->inst].getsFlags;                
    U16 index = gFlag >> 12;

    if (index) {
        U32 subIndex = (op->rm >> 3) & 7;
        if (op->rm>=0xC0) {
            index = opInfo[op->inst].setsFlags >> 12;
        }
        gFlag = subOpInfo[index][subIndex].getsFlags;
    }
    return gFlag;
}

U32 isConditionalJump(struct Op* op) {
    return ((op->inst>=0x70 && op->inst<0x80) || (op->inst>=0x180 && op->inst<0x190) || (op->inst>=0x270 && op->inst<0x280) || (op->inst>=0x380 && op->inst<0x390));
}

U32 getBlockEipCount(struct Block* block) {
    struct Op* op = block->ops;
    U32 result = 0;

    while(op) {
        result += op->eipCount;
        op = op->next;
    }
    return result;
}

void OPCALL restoreOps(struct CPU* cpu, struct Op* op);
void OPCALL jump(struct CPU* cpu, struct Op* op);
void OPCALL firstOp(struct CPU* cpu, struct Op* op);

U32 needsToSetFlag_r(struct CPU* cpu, struct Block* block, U32 blockEIP, struct Op* op, U32 flags, U32 depth, struct Op** opThatUsesFlag) {
    if (depth==0)
        return 1;    

    while (1) {
        U16 sFlag;
        U16 index;
        U16 gFlag;

        if (op == 0) {
            op = block->ops;
        } else if (op->next) {
            op = op->next;
        } else {
            break;
        }
        if (op->func==firstOp)
            op = op->next;

        sFlag = opInfo[op->inst].setsFlags;
        index = sFlag >> 12;
        gFlag = flagsThatOpUses(op);

        // an op needs a flag
        if (gFlag & flags) {
			if (opThatUsesFlag)
				*opThatUsesFlag = op;
            return 1;
        }

        sFlag = opInfo[op->inst].setsFlags;        
        index = sFlag >> 12;
        if (index) {
            U32 subIndex = (op->rm >> 3) & 7;
            if (op->rm<0xC0) {
                index = opInfo[op->inst].getsFlags >> 12;
            }
            sFlag = subOpInfo[index][(op->rm >> 3) & 7].setsFlags;
        }

        // an op sets the flag before it is used so the op being tested doesn't need to set it
        if (sFlag & flags && !(sFlag & MAYBE)) {
            flags &=~ sFlag;
            if (!flags)
                return 0;
        }
    }

    if (isConditionalJump(op)) {
        U32 blockCount = getBlockEipCount(block);

        if (!block->block1) {
            U32 oldEIP = cpu->eip.u32;
            cpu->eip.u32 = blockEIP + blockCount;
            block->block1 = getBlock(cpu, cpu->eip.u32);
            cpu->eip.u32 = oldEIP;
        }   
        if (block->block1->ops->func == restoreOps) {
            decodeBlockWithBlock(cpu, blockEIP + blockCount, block->block1);
        }

        if (!block->block2) {
            U32 oldEIP = cpu->eip.u32;
            cpu->eip.u32 = blockEIP + blockCount + op->data1;
            block->block2 = getBlock(cpu, cpu->eip.u32);
            cpu->eip.u32 = oldEIP;
        }
        if (block->block2->ops->func == restoreOps) {
            decodeBlockWithBlock(cpu, blockEIP + blockCount + op->data1, block->block2);
        }
		if (!needsToSetFlag_r(cpu, block->block1, blockEIP + blockCount, 0, flags, depth - 1, opThatUsesFlag) &&
			!needsToSetFlag_r(cpu, block->block2, blockEIP + blockCount + op->data1, 0, flags, depth - 1, opThatUsesFlag))
        {
            return 0;
        }
    } else if (op->func == jump) {
        U32 blockCount = getBlockEipCount(block);
        
        if (!block->block1) {
            U32 oldEIP = cpu->eip.u32;
            cpu->eip.u32 = blockEIP + blockCount + op->data1;
            block->block1 = getBlock(cpu, cpu->eip.u32);
            cpu->eip.u32 = oldEIP;
        }   
        if (block->block1->ops->func == restoreOps) {
            decodeBlockWithBlock(cpu, blockEIP + blockCount + op->data1, block->block1);
        }
		if (!needsToSetFlag_r(cpu, block->block1, blockEIP + blockCount + op->data1, 0, flags, depth - 1, opThatUsesFlag)) {
            return 0;
        }
    } 
    return 1; // this is the last instruction in this block that sets this flag, we should keep it
}

U32 needsToSetFlag(struct CPU* cpu, struct Block* block, U32 blockEIP, struct Op* op, U32 flags, struct Op** opThatUsesFlag) {
    return needsToSetFlag_r(cpu, block, blockEIP, op, flags, 3, opThatUsesFlag);
}

void OPCALL jumpO_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpO_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpO_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNO_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (!(((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000)) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpNO_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (!(((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000)) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpNO_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (!(((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80)) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpB_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u32<cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpB_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u16<cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpB_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u8<cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpNB_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u32>=cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpNB_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u16 >= cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}	
}

void OPCALL jumpNB_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u8 >= cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpZ_32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u32 == 0) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpZ_16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u16 == 0) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpZ_8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u8 == 0) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNZ_32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u32 != 0) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNZ_16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u16 != 0) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNZ_8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u8 != 0) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpBE_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u32 <= cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpBE_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u16 <= cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpBE_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u8 <= cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNBE_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u32 > cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNBE_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u16 > cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNBE_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->dst.u8 > cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpS_32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u32 & 0x80000000) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpS_16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u16 & 0x8000) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpS_8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (cpu->result.u8 & 0x80) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNS_32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (!(cpu->result.u32 & 0x80000000)) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNS_16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (!(cpu->result.u16 & 0x8000)) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNS_8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if (!(cpu->result.u8 & 0x80)) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpL_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S32)cpu->dst.u32 < (S32)cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpL_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S16)cpu->dst.u16 < (S16)cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpL_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S8)cpu->dst.u8 < (S8)cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNL_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S32)cpu->dst.u32 >= (S32)cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNL_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S16)cpu->dst.u16 >= (S16)cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNL_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S8)cpu->dst.u8 >= (S8)cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpLE_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S32)cpu->dst.u32 <= (S32)cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpLE_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S16)cpu->dst.u16 <= (S16)cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpLE_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S8)cpu->dst.u8 <= (S8)cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNLE_cmp32(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S32)cpu->dst.u32 > (S32)cpu->src.u32) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNLE_cmp16(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S16)cpu->dst.u16 > (S16)cpu->src.u16) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL jumpNLE_cmp8(struct CPU* cpu, struct Op* op) {
	DONE();
	cpu->eip.u32 += op->eipCount;
    CYCLES(1);
	if ((S8)cpu->dst.u8 > (S8)cpu->src.u8) {
		cpu->eip.u32 += op->data1;
		cpu->nextBlock = getBlock2(cpu);
        if ((int)op->data1 > 0)
            runBlock(cpu, cpu->nextBlock);
	}
	else {
		cpu->nextBlock = getBlock1(cpu);
        runBlock(cpu, cpu->nextBlock);
	}
}

void OPCALL cmpr8r8(struct CPU* cpu, struct Op* op);
void OPCALL cmpe8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL cmp8_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmp8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmp8_mem32(struct CPU* cpu, struct Op* op);

void OPCALL cmpr16r16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL cmp16_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmp16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmp16_mem32(struct CPU* cpu, struct Op* op);

void OPCALL cmpr32r32(struct CPU* cpu, struct Op* op);
void OPCALL cmpe32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpe32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL cmp32_reg(struct CPU* cpu, struct Op* op);
void OPCALL cmp32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmp32_mem32(struct CPU* cpu, struct Op* op);

void OPCALL testr8r8(struct CPU* cpu, struct Op* op);
void OPCALL teste8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL teste8r8_32(struct CPU* cpu, struct Op* op);
void OPCALL testr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL testr8e8_32(struct CPU* cpu, struct Op* op);
void OPCALL test8_reg(struct CPU* cpu, struct Op* op);
void OPCALL test8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL test8_mem32(struct CPU* cpu, struct Op* op);

void OPCALL testr16r16(struct CPU* cpu, struct Op* op);
void OPCALL teste16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL teste16r16_32(struct CPU* cpu, struct Op* op);
void OPCALL testr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL testr16e16_32(struct CPU* cpu, struct Op* op);
void OPCALL test16_reg(struct CPU* cpu, struct Op* op);
void OPCALL test16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL test16_mem32(struct CPU* cpu, struct Op* op);

void OPCALL testr32r32(struct CPU* cpu, struct Op* op);
void OPCALL teste32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL teste32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL testr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL testr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL test32_reg(struct CPU* cpu, struct Op* op);
void OPCALL test32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL test32_mem32(struct CPU* cpu, struct Op* op);

void OPCALL jumpO(struct CPU* cpu, struct Op* op);
void OPCALL jumpNO(struct CPU* cpu, struct Op* op);
void OPCALL jumpB(struct CPU* cpu, struct Op* op);
void OPCALL jumpNB(struct CPU* cpu, struct Op* op);
void OPCALL jumpZ(struct CPU* cpu, struct Op* op);
void OPCALL jumpNZ(struct CPU* cpu, struct Op* op);
void OPCALL jumpBE(struct CPU* cpu, struct Op* op);
void OPCALL jumpNBE(struct CPU* cpu, struct Op* op);
void OPCALL jumpS(struct CPU* cpu, struct Op* op);
void OPCALL jumpNS(struct CPU* cpu, struct Op* op);
void OPCALL jumpL(struct CPU* cpu, struct Op* op);
void OPCALL jumpNL(struct CPU* cpu, struct Op* op);
void OPCALL jumpLE(struct CPU* cpu, struct Op* op);
void OPCALL jumpNLE(struct CPU* cpu, struct Op* op);

#define IS_CMP8() op->func == cmpr8r8 || op->func == cmpe8r8_16 || op->func == cmpe8r8_32 || op->func == cmpr8e8_16 || op->func == cmpr8e8_32 || op->func == cmp8_reg || op->func == cmp8_mem16 || op->func == cmp8_mem32
#define IS_CMP16() op->func == cmpr16r16 || op->func == cmpe16r16_16 || op->func == cmpe16r16_32 || op->func == cmpr16e16_16 || op->func == cmpr16e16_32 || op->func == cmp16_reg || op->func == cmp16_mem16 || op->func == cmp16_mem32
#define IS_CMP32() op->func == cmpr32r32 || op->func == cmpe32r32_16 || op->func == cmpe32r32_32 || op->func == cmpr32e32_16 || op->func == cmpr32e32_32 || op->func == cmp32_reg || op->func == cmp32_mem16 || op->func == cmp32_mem32
#define IS_TEST8() op->func == testr8r8 || op->func == teste8r8_16 || op->func == teste8r8_32 || op->func == testr8e8_16 || op->func == testr8e8_32 || op->func == test8_reg || op->func == test8_mem16 || op->func == test8_mem32
#define IS_TEST16() op->func == testr16r16 || op->func == teste16r16_16 || op->func == teste16r16_32 || op->func == testr16e16_16 || op->func == testr16e16_32 || op->func == test16_reg || op->func == test16_mem16 || op->func == test16_mem32
#define IS_TEST32() op->func == testr32r32 || op->func == teste32r32_16 || op->func == teste32r32_32 || op->func == testr32e32_16 || op->func == testr32e32_32 || op->func == test32_reg || op->func == test32_mem16 || op->func == test32_mem32

void jit(struct CPU* cpu, struct Block* block, U32 blockEIP) {
    struct Op* op;
    
    if (block->jit)
        return;
    block->jit = 1;    
    op = block->ops;
    if (op->func == firstOp)
        op = op->next;
    while (op) {
        U16 sFlags = opInfo[op->inst].setsFlags;
        U16 index = sFlags >> 12;
        if (index) {
            U32 subIndex = (op->rm >> 3) & 7;
            if (op->rm<0xC0) {
                index = opInfo[op->inst].getsFlags >> 12;
            }
            sFlags = subOpInfo[index][(op->rm >> 3) & 7].setsFlags;
        }
        if (sFlags) {
			struct Op* opThatUsesFlag = 0;

            if (!needsToSetFlag(cpu, block, blockEIP, op, sFlags, &opThatUsesFlag)) {
                if (fastDecoder[op->inst]) {
                    fastDecoder[op->inst](op);
                }
			} else if (opThatUsesFlag) {
				if (opThatUsesFlag->func == jumpO) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpO_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpO_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpO_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpNO) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpNO_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpNO_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpNO_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpB) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpB_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpB_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpB_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpNB) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpNB_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpNB_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpNB_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpZ) {
					if (IS_CMP32() || IS_TEST32()) {
						opThatUsesFlag->func = jumpZ_32;
					} else if (IS_CMP16() || IS_TEST16()) {
						opThatUsesFlag->func = jumpZ_16;
					}
					else if (IS_CMP8() || IS_TEST8()) {
						opThatUsesFlag->func = jumpZ_8;
					}
				}
				else if (opThatUsesFlag->func == jumpNZ) {
					if (IS_CMP32() || IS_TEST32()) {
						opThatUsesFlag->func = jumpNZ_32;
					}
					else if (IS_CMP16() || IS_TEST16()) {
						opThatUsesFlag->func = jumpNZ_16;
					}
					else if (IS_CMP8() || IS_TEST8()) {
						opThatUsesFlag->func = jumpNZ_8;
					}
				}
				else if (opThatUsesFlag->func == jumpBE) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpBE_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpBE_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpBE_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpNBE) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpNBE_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpNBE_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpNBE_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpS) {
					if (IS_CMP32() || IS_TEST32()) {
						opThatUsesFlag->func = jumpS_32;
					}
					else if (IS_CMP16() || IS_TEST16()) {
						opThatUsesFlag->func = jumpS_16;
					}
					else if (IS_CMP8() || IS_TEST8()) {
						opThatUsesFlag->func = jumpS_8;
					}
				}
				else if (opThatUsesFlag->func == jumpNS) {
					if (IS_CMP32() || IS_TEST32()) {
						opThatUsesFlag->func = jumpNS_32;
					}
					else if (IS_CMP16() || IS_TEST16()) {
						opThatUsesFlag->func = jumpNS_16;
					}
					else if (IS_CMP8() || IS_TEST8()) {
						opThatUsesFlag->func = jumpNS_8;
					}
				}
				else if (opThatUsesFlag->func == jumpL) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpL_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpL_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpL_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpNL) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpNL_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpNL_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpNL_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpLE) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpLE_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpLE_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpLE_cmp8;
					}
				}
				else if (opThatUsesFlag->func == jumpNLE) {
					if (IS_CMP32()) {
						opThatUsesFlag->func = jumpNLE_cmp32;
					}
					else if (IS_CMP16()) {
						opThatUsesFlag->func = jumpNLE_cmp16;
					}
					else if (IS_CMP8()) {
						opThatUsesFlag->func = jumpNLE_cmp8;
					}
				}
			}
        }
        op = op->next;
    }
}