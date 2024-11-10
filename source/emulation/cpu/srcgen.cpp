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

#include "boxedwine.h"
#ifdef GENERATE_SOURCE
#include "op.h"
#include "block.h"
#include "cpu.h"
#include "kalloc.h"
#include "jit.h"
#include "decoder.h"

#include <stdio.h>
#include <stdlib.h>

#define sFLAGS_NONE 0
#define sFLAGS_ADD8 1
#define sFLAGS_ADD16 2
#define sFLAGS_ADD32 3
#define sFLAGS_OR8 4
#define sFLAGS_OR16 5
#define sFLAGS_OR32 6
#define sFLAGS_ADC8 7
#define sFLAGS_ADC16 8
#define sFLAGS_ADC32 9
#define sFLAGS_SBB8 10
#define sFLAGS_SBB16 11
#define sFLAGS_SBB32 12
#define sFLAGS_AND8 13
#define sFLAGS_AND16 14
#define sFLAGS_AND32 15
#define sFLAGS_SUB8 16
#define sFLAGS_SUB16 17
#define sFLAGS_SUB32 18
#define sFLAGS_XOR8 19
#define sFLAGS_XOR16 20
#define sFLAGS_XOR32 21
#define sFLAGS_INC8 22
#define sFLAGS_INC16 23
#define sFLAGS_INC32 24
#define sFLAGS_DEC8 25
#define sFLAGS_DEC16 26
#define sFLAGS_DEC32 27
#define sFLAGS_SHL8 28
#define sFLAGS_SHL16 29
#define sFLAGS_SHL32 30
#define sFLAGS_SHR8 31
#define sFLAGS_SHR16 32
#define sFLAGS_SHR32 33
#define sFLAGS_SAR8 34
#define sFLAGS_SAR16 35
#define sFLAGS_SAR32 36
#define sFLAGS_CMP8 37
#define sFLAGS_CMP16 38
#define sFLAGS_CMP32 39
#define sFLAGS_TEST8 40
#define sFLAGS_TEST16 41
#define sFLAGS_TEST32 42
#define sFLAGS_DSHL16 43
#define sFLAGS_DSHL32 44
#define sFLAGS_DSHR16 45
#define sFLAGS_DSHR32 46
#define sFLAGS_NEG8 47
#define sFLAGS_NEG16 48
#define sFLAGS_NEG32 49
#define sFLAGS_UNKNOWN 50

struct GenData {
    struct CPU* cpu;
    struct Block* block;
    U32 lazyFlags;
    struct Op* inlinedLazyFlagOp;
    U32 inlinedLazyFlagOpAssignedToCPU;
    U32 eip;
    U32 inlinedBlock;
    char* sourceBuffer;
    U32 sourceBufferLen;
    U32 sourceBufferPos;
    U32 indent;
    char* ops;
    U32 opPos;
    U32 maxOpPos;
    U32 ip;
}; 

void writeBlock(struct GenData* data, struct Block* block);
void writeBlockWithEipCount(struct GenData* data, struct Block* block, S32 eipCount);

U32 needsToSetFlag(struct CPU* cpu, struct Block* block, U32 blockEIP, struct Op* op, U32 flags, struct Op** opThatUsesFlag);

void outfp(FILE* fp, const char* str) {
    fwrite(str, strlen(str), 1, fp);
}

void out(struct GenData* data, const char* str) {
    U32 len = (U32)strlen(str);
    if (data->sourceBufferPos+len>data->sourceBufferLen) {
        char* tmp = kalloc(data->sourceBufferLen*2, KALLOC_SRCGENBUFFER);
        strcpy(tmp, data->sourceBuffer);
        kfree(data->sourceBuffer, KALLOC_SRCGENBUFFER);
        data->sourceBuffer = tmp;
        data->sourceBufferLen*=2;
    }
    strcpy(data->sourceBuffer+data->sourceBufferPos, str);
    data->sourceBufferPos+=len;
}

const char* srcGetZF(struct GenData* data) {
    switch (data->lazyFlags) {
    case sFLAGS_NONE:
        return "(cpu->flags & ZF)";
    case sFLAGS_ADD8:
    case sFLAGS_OR8:
    case sFLAGS_ADC8:
    case sFLAGS_SBB8:
    case sFLAGS_AND8:
    case sFLAGS_SUB8:
    case sFLAGS_XOR8:
    case sFLAGS_INC8:
    case sFLAGS_DEC8:
    case sFLAGS_SHL8:
    case sFLAGS_SHR8:
    case sFLAGS_SAR8:
    case sFLAGS_CMP8:
    case sFLAGS_TEST8:
    case sFLAGS_NEG8:
        return "(cpu->result.u8 == 0)";

    case sFLAGS_ADD16:
    case sFLAGS_OR16:
    case sFLAGS_ADC16:
    case sFLAGS_SBB16:
    case sFLAGS_AND16:
    case sFLAGS_SUB16:
    case sFLAGS_XOR16:
    case sFLAGS_INC16:
    case sFLAGS_DEC16:
    case sFLAGS_SHL16:
    case sFLAGS_SHR16:
    case sFLAGS_SAR16:
    case sFLAGS_CMP16:
    case sFLAGS_TEST16:
    case sFLAGS_DSHL16:
    case sFLAGS_DSHR16:
    case sFLAGS_NEG16:
        return "(cpu->result.u16 == 0)";

    case sFLAGS_ADD32:
    case sFLAGS_OR32:
    case sFLAGS_ADC32:
    case sFLAGS_SBB32:
    case sFLAGS_AND32:
    case sFLAGS_SUB32:
    case sFLAGS_XOR32:
    case sFLAGS_INC32:
    case sFLAGS_DEC32:
    case sFLAGS_SHL32:
    case sFLAGS_SHR32:
    case sFLAGS_SAR32:
    case sFLAGS_CMP32:
    case sFLAGS_TEST32:
    case sFLAGS_DSHL32:
    case sFLAGS_DSHR32:
    case sFLAGS_NEG32:
        return "(cpu->result.u32 == 0)";
    case sFLAGS_UNKNOWN:
        return "getZF(cpu)";
    }
    kpanic("srcGetZF %d", data->lazyFlags);
    return "";
}

const char* srcGetSF(struct GenData* data) {
    switch (data->lazyFlags) {
    case sFLAGS_NONE:
        return "(cpu->flags & SF)";
    case sFLAGS_ADD8:
    case sFLAGS_OR8:
    case sFLAGS_ADC8:
    case sFLAGS_SBB8:
    case sFLAGS_AND8:
    case sFLAGS_SUB8:
    case sFLAGS_XOR8:
    case sFLAGS_INC8:
    case sFLAGS_DEC8:
    case sFLAGS_SHL8:
    case sFLAGS_SHR8:
    case sFLAGS_SAR8:
    case sFLAGS_CMP8:
    case sFLAGS_TEST8:
    case sFLAGS_NEG8:
        return "(cpu->result.u8 & 0x80)";

    case sFLAGS_ADD16:
    case sFLAGS_OR16:
    case sFLAGS_ADC16:
    case sFLAGS_SBB16:
    case sFLAGS_AND16:
    case sFLAGS_SUB16:
    case sFLAGS_XOR16:
    case sFLAGS_INC16:
    case sFLAGS_DEC16:
    case sFLAGS_SHL16:
    case sFLAGS_SHR16:
    case sFLAGS_SAR16:
    case sFLAGS_CMP16:
    case sFLAGS_TEST16:
    case sFLAGS_DSHL16:
    case sFLAGS_DSHR16:
    case sFLAGS_NEG16:
        return "(cpu->result.u16 & 0x8000)";

    case sFLAGS_ADD32:
    case sFLAGS_OR32:
    case sFLAGS_ADC32:
    case sFLAGS_SBB32:
    case sFLAGS_AND32:
    case sFLAGS_SUB32:
    case sFLAGS_XOR32:
    case sFLAGS_INC32:
    case sFLAGS_DEC32:
    case sFLAGS_SHL32:
    case sFLAGS_SHR32:
    case sFLAGS_SAR32:
    case sFLAGS_CMP32:
    case sFLAGS_TEST32:
    case sFLAGS_DSHL32:
    case sFLAGS_DSHR32:
    case sFLAGS_NEG32:
        return "(cpu->result.u32 & 0x80000000)";
    case sFLAGS_UNKNOWN:
        return "getSF(cpu)";
    }
    kpanic("srcGetSF %d", data->lazyFlags);
    return "";
}

const char* srcGetCF(struct GenData* data) {
    switch (data->lazyFlags) {
    case sFLAGS_NONE:
        return "(cpu->flags & CF)";
    case sFLAGS_ADD8:
        return "(cpu->result.u8<cpu->dst.u8)";
    case sFLAGS_OR8:
        return "0";
    case sFLAGS_ADC8:
        return "((cpu->result.u8 < cpu->dst.u8) || (cpu->oldcf && (cpu->result.u8 == cpu->dst.u8)))";
    case sFLAGS_SBB8:
        return "((cpu->dst.u8 < cpu->result.u8) || (cpu->oldcf && (cpu->src.u8==0xff)))";
    case sFLAGS_AND8:
        return "0";
    case sFLAGS_SUB8:
        return "(cpu->dst.u8<cpu->src.u8)";
    case sFLAGS_XOR8:
        return "0";
    case sFLAGS_INC8:
        return "cpu->oldcf";
    case sFLAGS_DEC8:
        return "cpu->oldcf";
    case sFLAGS_SHL8:
        return "((cpu->src.u8>8)?0:(cpu->dst.u8 >> (8-cpu->src.u8)) & 1)";
    case sFLAGS_SHR8:
        return "((cpu->dst.u8 >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_SAR8:
        return "((((S8) cpu->dst.u8) >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_CMP8:
        return "(cpu->dst.u8<cpu->src.u8)";
    case sFLAGS_TEST8:
        return "0";
    case sFLAGS_NEG8:
        return "(cpu->dst.u8!=0)";

    case sFLAGS_ADD16:
        return "(cpu->result.u16<cpu->dst.u16)";
    case sFLAGS_OR16:
        return "0";
    case sFLAGS_ADC16:
        return "((cpu->result.u16 < cpu->dst.u16) || (cpu->oldcf && (cpu->result.u16 == cpu->dst.u16)))";
    case sFLAGS_SBB16:
        return "((cpu->dst.u16 < cpu->result.u16) || (cpu->oldcf && (cpu->src.u16==0xffff)))";
    case sFLAGS_AND16:
        return "0";
    case sFLAGS_SUB16:
        return "(cpu->dst.u16<cpu->src.u16)";
    case sFLAGS_XOR16:
        return "0";
    case sFLAGS_INC16:
        return "cpu->oldcf";
    case sFLAGS_DEC16:
        return "cpu->oldcf";
    case sFLAGS_SHL16:
        return "((cpu->src.u8>16)?0:(cpu->dst.u16 >> (16-cpu->src.u8)) & 1)";
    case sFLAGS_SHR16:
        return "((cpu->dst.u16 >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_SAR16:
        return "((((S16) cpu->dst.u16) >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_CMP16:
        return "(cpu->dst.u16<cpu->src.u16)";
    case sFLAGS_TEST16:
        return "0";
    case sFLAGS_DSHL16:
        return "((cpu->src.u8>16) ? (cpu->dst2.u16 >> (32-cpu->src.u8)) & 1 : (cpu->dst.u16 >> (16-cpu->src.u8)) & 1)";
    case sFLAGS_DSHR16:
        return "((cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_NEG16:
        return "(cpu->dst.u16!=0)";

    case sFLAGS_ADD32:
        return "(cpu->result.u32<cpu->dst.u32)";
    case sFLAGS_OR32:
        return "0";
    case sFLAGS_ADC32:
        return "((cpu->result.u32 < cpu->dst.u32) || (cpu->oldcf && (cpu->result.u32 == cpu->dst.u32)))";
    case sFLAGS_SBB32:
        return "((cpu->dst.u32 < cpu->result.u32) || (cpu->oldcf && (cpu->src.u32==0xffffffff)))";
    case sFLAGS_AND32:
        return "0";
    case sFLAGS_SUB32:
        return "(cpu->dst.u32<cpu->src.u32)";
    case sFLAGS_XOR32:
        return "0";
    case sFLAGS_INC32:
        return "cpu->oldcf";
    case sFLAGS_DEC32:
        return "cpu->oldcf";
    case sFLAGS_SHL32:
        return "((cpu->dst.u32 >> (32 - cpu->src.u8)) & 1)";
    case sFLAGS_SHR32:
        return "((cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_SAR32:
        return "((((S32) cpu->dst.u32) >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_CMP32:
        return "(cpu->dst.u32<cpu->src.u32)";
    case sFLAGS_TEST32:
        return "0";
    case sFLAGS_DSHL32:
        return "((cpu->dst.u32 >> (32 - cpu->src.u8)) & 1)";
    case sFLAGS_DSHR32:
        return "((cpu->dst.u32 >> (cpu->src.u8 - 1)) & 1)";
    case sFLAGS_NEG32:
        return "(cpu->dst.u32!=0)";
    case sFLAGS_UNKNOWN:
        return "getCF(cpu)";
    }
    kpanic("srcGetCF %d", data->lazyFlags);
    return "";
}

const char* srcGetOF(struct GenData* data) {
    switch (data->lazyFlags) {
    case sFLAGS_NONE:
        return "(cpu->flags & OF)";
    case sFLAGS_ADD8:
        return "(((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80)";
    case sFLAGS_OR8:
        return "0";
    case sFLAGS_ADC8:
        return "(((cpu->dst.u8 ^ cpu->src.u8 ^ 0x80) & (cpu->result.u8 ^ cpu->src.u8)) & 0x80)";
    case sFLAGS_SBB8:
        return "(((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80)";
    case sFLAGS_AND8:
        return "0";
    case sFLAGS_SUB8:
        return "(((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80)";
    case sFLAGS_XOR8:
        return "0";
    case sFLAGS_INC8:
        return "(cpu->result.u8 == 0x80)";
    case sFLAGS_DEC8:
        return "(cpu->result.u8 == 0x7f)";
    case sFLAGS_SHL8:
        return "((cpu->result.u8 ^ cpu->dst.u8) & 0x80)";
    case sFLAGS_SHR8:
        return "(((cpu->src.u8&0x1f)==1) ? (cpu->dst.u8 >= 0x80); : 0)";
    case sFLAGS_SAR8:
        return "0";
    case sFLAGS_CMP8:
        return "(((cpu->dst.u8 ^ cpu->src.u8) & (cpu->dst.u8 ^ cpu->result.u8)) & 0x80)";
    case sFLAGS_TEST8:
        return "0";
    case sFLAGS_NEG8:
        return "(cpu->dst.u8 == 0x80)";

    case sFLAGS_ADD16:
        return "(((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000)";
    case sFLAGS_OR16:
        return "0";
    case sFLAGS_ADC16:
        return "(((cpu->dst.u16 ^ cpu->src.u16 ^ 0x8000) & (cpu->result.u16 ^ cpu->src.u16)) & 0x8000)";
    case sFLAGS_SBB16:
        return "(((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000)";
    case sFLAGS_AND16:
        return "0";
    case sFLAGS_SUB16:
        return "(((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000)";
    case sFLAGS_XOR16:
        return "0";
    case sFLAGS_INC16:
        return "(cpu->result.u16 == 0x8000)";
    case sFLAGS_DEC16:
        return "(cpu->result.u16 == 0x7fff)";
    case sFLAGS_SHL16:
        return "((cpu->result.u16 ^ cpu->dst.u16) & 0x8000)";
    case sFLAGS_SHR16:
        return "(((cpu->src.u8&0x1f)==1) ? (cpu->dst.u16 >= 0x8000) : 0)";
    case sFLAGS_SAR16:
        return "0";
    case sFLAGS_CMP16:
        return "(((cpu->dst.u16 ^ cpu->src.u16) & (cpu->dst.u16 ^ cpu->result.u16)) & 0x8000)";
    case sFLAGS_TEST16:
        return "0";
    case sFLAGS_DSHL16:
        return "((cpu->result.u16 ^ cpu->dst.u16) & 0x8000)";
    case sFLAGS_DSHR16:
        return "((cpu->result.u16 ^ cpu->dst.u16) & 0x8000)";
    case sFLAGS_NEG16:
        return "(cpu->dst.u16 == 0x8000)";

    case sFLAGS_ADD32:
        return "(((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000)";
    case sFLAGS_OR32:
        return "0";
    case sFLAGS_ADC32:
        return "(((cpu->dst.u32 ^ cpu->src.u32 ^ 0x80000000) & (cpu->result.u32 ^ cpu->src.u32)) & 0x80000000)";
    case sFLAGS_SBB32:
        return "(((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000)";
    case sFLAGS_AND32:
        return "0";
    case sFLAGS_SUB32:
        return "(((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000)";
    case sFLAGS_XOR32:
        return "0";
    case sFLAGS_INC32:
        return "(cpu->result.u32 == 0x80000000)";
    case sFLAGS_DEC32:
        return "(cpu->result.u32 == 0x7fffffff)";
    case sFLAGS_SHL32:
        return "((cpu->result.u32 ^ cpu->dst.u32) & 0x80000000)";
    case sFLAGS_SHR32:
        return "(((cpu->src.u8&0x1f)==1) ? (cpu->dst.u32 >= 0x80000000) : 0)";
    case sFLAGS_SAR32:
        return "0";
    case sFLAGS_CMP32:
        return "(((cpu->dst.u32 ^ cpu->src.u32) & (cpu->dst.u32 ^ cpu->result.u32)) & 0x80000000)";
    case sFLAGS_TEST32:
        return "0";
    case sFLAGS_DSHL32:
        return "((cpu->result.u32 ^ cpu->dst.u32) & 0x80000000)";
    case sFLAGS_DSHR32:
        return "((cpu->result.u32 ^ cpu->dst.u32) & 0x80000000)";
    case sFLAGS_NEG32:
        return "(cpu->dst.u32 == 0x80000000)";
    case sFLAGS_UNKNOWN:
        return "getOF(cpu)";
    }
    kpanic("srcGetOF %d", data->lazyFlags);
    return "";
}

const char* getFlag(struct GenData* data, int flag) {
    if (flag == OF)
        return srcGetOF(data);
    else if (flag == CF)
        return srcGetCF(data);
    else if (flag == SF)
        return srcGetSF(data);
    else if (flag == PF)
        return "parity_lookup[cpu->result.u8]";
    else if (flag == ZF)
        return srcGetZF(data);
    else if (flag == AF)
        return "getAF(cpu)";
    else
        kpanic("outGetFlag %d", flag);
    return "";
}

const char* getCondition(struct GenData* data, int condition);

const char* r8(int r) {
    switch (r) {
    case 0: return "AL";
    case 1: return "CL";
    case 2: return "DL";
    case 3: return "BL";
    case 4: return "AH";
    case 5: return "CH";
    case 6: return "DH";
    case 7: return "BH";
    }
    return "r8";
}

const char* r16(int r) {
    switch (r) {
    case 0: return "AX";
    case 1: return "CX";
    case 2: return "DX";
    case 3: return "BX";
    case 4: return "SP";
    case 5: return "BP";
    case 6: return "SI";
    case 7: return "DI";
    }
    return "r16";
}

const char* r32(int r) {
    switch (r) {
    case 0: return "EAX";
    case 1: return "ECX";
    case 2: return "EDX";
    case 3: return "EBX";
    case 4: return "ESP";
    case 5: return "EBP";
    case 6: return "ESI";
    case 7: return "EDI";
    }
    return "r32";
}

const char* getBase(int base) {
    switch(base) {
        case ES: return "ES";
        case CS: return "CS";
        case SS: return "SS";
        case DS: return "DS";
        case FS: return "FS";
        case GS: return "GS";
    }
    return "getBase";
}

char eaaStr[256];

const char* getEaa16(struct Op* op) {
    eaaStr[0]=0;

    if (op->base<6) {
        strcat(eaaStr, "cpu->segAddress[");
        strcat(eaaStr, getBase(op->base));
        strcat(eaaStr, "] + ");
    }
    strcat(eaaStr, "(U16)(");
    strcat(eaaStr, r16(op->e1));
    if (op->e2<8) {
        strcat(eaaStr, " + (S16)");
        strcat(eaaStr, r16(op->e2));
    }
    if (op->eData) {
        char tmp[32];
        strcat(eaaStr, " + 0x");
        itoa(op->eData, tmp, 16);
        strcat(eaaStr, tmp);
    }
    strcat(eaaStr, ")");
    return eaaStr;
}

const char* getEaa32(struct Op* op) {
    eaaStr[0]=0;

    if (op->base<6) {
        strcat(eaaStr, "cpu->segAddress[");
        strcat(eaaStr, getBase(op->base));
        strcat(eaaStr, "]");
    }
    if (op->e1<8) {
        if (eaaStr[0])
            strcat(eaaStr, " + ");
        strcat(eaaStr, r32(op->e1));
    }    
    if (op->e2<8) {
        char tmp[32];

        if (eaaStr[0])
            strcat(eaaStr, " + ");
        if (op->eSib)
            strcat(eaaStr, "(");
        strcat(eaaStr, r32(op->e2));
        if (op->eSib) {
            strcat(eaaStr, " << ");
            itoa(op->eSib, tmp, 10);
            strcat(eaaStr, tmp);
            strcat(eaaStr, ")");
        }
    }
    if (op->eData) {
        char tmp[32];

        if (eaaStr[0])
            strcat(eaaStr, " + ");
        strcat(eaaStr, "0x");
        itoa(op->eData, tmp, 16);
        strcat(eaaStr, tmp);
    }
    return eaaStr;
}

U16 flagsThatOpUses(struct Op* op);
U32 isConditionalJump(struct Op* op);
U32 getTestLeftRight(struct Op* op, char* left, char* right);

U32 inlineTestJump(struct GenData* data, struct Op* op, U32 lazyFlag, const char* cycles) {
    struct Op* nextOpToUseFlags = op->next;

    if (!needsToSetFlag(data->cpu, data->block, data->eip, op, CF|ZF|SF|OF|AF|PF, NULL)) {
        out(data, "// removed unecessary TEST\n");
        out(data, "CYCLES(");
        out(data, cycles);
        out(data, ");");
        return 1;
    }
    while(nextOpToUseFlags) {
        U16 flags = flagsThatOpUses(nextOpToUseFlags);
        if (flags!=0)
            break;
        nextOpToUseFlags = nextOpToUseFlags->next;
    }
    if (!nextOpToUseFlags) {
        // ran into something else besides a jump that used this test or maybe a retn
        return 0;
    }
    if (isConditionalJump(nextOpToUseFlags)) {        
        if (!needsToSetFlag(data->cpu, data->block, data->eip, nextOpToUseFlags, CF|ZF|SF|OF|AF|PF, NULL)) {
            out(data, "// inlined TEST\n    ");
            // TODO maybe in the future, the code will know what registers are set between these two instructions and whether or not it will cause a problem
            data->inlinedLazyFlagOpAssignedToCPU = 0;
            if (op->next != nextOpToUseFlags) {
                char left[256];
                char right[256];
                U32 same = getTestLeftRight(op, left, right);
                if (lazyFlag == sFLAGS_TEST8) {
                    out(data, "cpu->dst.u8 = ");
                    out(data, left);
                    if (!same) {
                        out(data, "; cpu->src.u8 = ");
                        out(data, right);
                    }
                } else if (lazyFlag == sFLAGS_TEST16) {
                    out(data, "cpu->dst.u16 = ");
                    out(data, left);
                    if (!same) {
                        out(data, "; cpu->src.u16 = ");
                        out(data, right);
                    }
                } else if (lazyFlag == sFLAGS_TEST32) {
                    out(data, "cpu->dst.u32 = ");
                    out(data, left);
                    if (!same) {
                        out(data, "; cpu->src.u32 = ");
                        out(data, right);
                    }
                } else {
                    kpanic("inlineTestJump: lazyFlag unknown %d", lazyFlag);
                }
                out(data, "; ");
                data->inlinedLazyFlagOpAssignedToCPU = 1;
            }
            out(data, "CYCLES(");
            out(data, cycles);
            out(data, ");");
            data->lazyFlags = lazyFlag;
            data->inlinedLazyFlagOp = op;
            return 1;
        }
    }
    return 0;
}

void gen027(struct GenData* data, struct Op* op) {
    out(data, "instruction_daa(cpu);CYCLES(3);");
}

void gen02f(struct GenData* data, struct Op* op) {
    out(data, "instruction_das(cpu);CYCLES(3);");
}

void gen037(struct GenData* data, struct Op* op) {
    out(data, "instruction_aaa(cpu);CYCLES(3);");
}

void gen03f(struct GenData* data, struct Op* op) {
    out(data, "instruction_aas(cpu);CYCLES(3);");
}

void OPCALL inc16_reg(struct CPU* cpu, struct Op* op);
void OPCALL inc16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen040(struct GenData* data, struct Op* op) {
    if (op->func==inc16_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->result.u16=cpu->dst.u16 + 1; cpu->lazyFlags = FLAGS_INC16; ");
        data->lazyFlags = sFLAGS_INC16;
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16;CYCLES(1);");
    } else if (op->func==inc16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " + 1;CYCLES(1);");
    } else {
        kpanic("gen040");
    }
}

void OPCALL dec16_reg(struct CPU* cpu, struct Op* op);
void OPCALL dec16_reg_noflags(struct CPU* cpu, struct Op* op);
void gen048(struct GenData* data, struct Op* op) {
    if (op->func==dec16_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->result.u16=cpu->dst.u16 - 1; cpu->lazyFlags = FLAGS_DEC16; ");
        data->lazyFlags = sFLAGS_DEC16;
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16;CYCLES(1);");
    } else if (op->func==dec16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " - 1;CYCLES(1);");
    } else {
        kpanic("gen048");
    }
}

void OPCALL inc32_reg(struct CPU* cpu, struct Op* op);
void OPCALL inc32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen240(struct GenData* data, struct Op* op) {
    if (op->func==inc32_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=cpu->dst.u32 + 1; cpu->lazyFlags = FLAGS_INC32; ");
        data->lazyFlags = sFLAGS_INC32;
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32;CYCLES(1);");
    } else if (op->func==inc32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " + 1;CYCLES(1);");
    } else {
        kpanic("gen240");
    }
}

void OPCALL dec32_reg(struct CPU* cpu, struct Op* op);
void OPCALL dec32_reg_noflags(struct CPU* cpu, struct Op* op);
void gen248(struct GenData* data, struct Op* op) {
    if (op->func==dec32_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=cpu->dst.u32 - 1; cpu->lazyFlags = FLAGS_DEC32; ");
        data->lazyFlags = sFLAGS_DEC32;
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32;CYCLES(1);");
    } else if (op->func==dec32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " - 1;CYCLES(1);");
    } else {
        kpanic("gen248");
    }

}

void gen050(struct GenData* data, struct Op* op) {
    out(data, "push16(cpu, ");
    out(data, r16(op->r1));
    out(data, ");CYCLES(1);");
}

void gen058(struct GenData* data, struct Op* op) {
    out(data, r16(op->r1));
    out(data, " = pop16(cpu);CYCLES(1);");
}

void gen250(struct GenData* data, struct Op* op) {
    out(data, "push32(cpu, ");
    out(data, r32(op->r1));
    out(data, ");CYCLES(1);");
}

void gen258(struct GenData* data, struct Op* op) {
    out(data, r32(op->r1));
    out(data, " = pop32(cpu);CYCLES(1);");
}

void gen260(struct GenData* data, struct Op* op) {
    out(data, "tmp32 = ESP; push32(cpu, EAX); push32(cpu, ECX); push32(cpu, EDX); push32(cpu, EBX);push32(cpu, tmp32); push32(cpu, EBP); push32(cpu, ESI); push32(cpu, EDI); CYCLES(5);");
}

void gen261(struct GenData* data, struct Op* op) {
    out(data, "EDI = pop32(cpu); ESI = pop32(cpu); EBP = pop32(cpu); pop32(cpu); EBX = pop32(cpu); EDX = pop32(cpu); ECX = pop32(cpu); EAX = pop32(cpu); CYCLES(5);");
}

void gen068(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "push16(cpu, 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, ");CYCLES(1);");
}

void gen268(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "push32(cpu, 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, ");CYCLES(1);");
}

void OPCALL dimulcr16r16(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr16e16_32(struct CPU* cpu, struct Op* op);
void gen069(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);
    out(data, "tmp32=(S16)(");
    if (op->func == dimulcr16r16) {
        out(data, r16(op->r2));        
    } else if (op->func == dimulcr16e16_16) {
        out(data, "readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")");
    } else if (op->func == dimulcr16e16_32) {
        out(data, "readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")");
    }
    
    out(data, ") * (S32)0x");
    out(data, tmp);
    out(data, "; fillFlagsNoCFOF(cpu); if ((tmp32 >= -32767) && (tmp32 <= 32767)) { removeFlag(CF|OF); } else { addFlag(CF|OF); } ");
    data->lazyFlags = sFLAGS_NONE;
    out(data, r16(op->r1));
    out(data, " = tmp32;CYCLES(10);");
}

void OPCALL dimulcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dimulcr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen269(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    if (op->func == dimulcr32r32_noflags) {
        out(data, r32(op->r1));
        out(data, " = (S32)");
        out(data, r32(op->r2));
        out(data, " * ((S32)0x");
        out(data, tmp);
        out(data, ");CYCLES(10);");
        return;
    } else if (op->func == dimulcr32e32_16_noflags) {
        out(data, r32(op->r1));
        out(data, " = (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ") * ((S32)0x");
        out(data, tmp);
        out(data, ");CYCLES(10);");
        return;
    } else if (op->func == dimulcr32e32_32_noflags) {
        out(data, r32(op->r1));
        out(data, " = (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ") * ((S32)0x");
        out(data, tmp);
        out(data, ");CYCLES(10);");
        return;
    }
    out(data, "tmp64=(S32)(");
    if (op->func == dimulcr32r32) {
        out(data, r32(op->r2));    
    } else if (op->func == dimulcr32e32_16) {
        out(data, "readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")");
    } else if (op->func == dimulcr32e32_32) {
        out(data, "readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")");
    }
    out(data, ") * (S64)((S32)0x");
    out(data, tmp);
    out(data, "); fillFlagsNoCFOF(cpu); if ((tmp32 >= -2147483647l) && (tmp32 <= 2147483647l)) { removeFlag(CF|OF); } else { addFlag(CF|OF); } ");
    data->lazyFlags = sFLAGS_NONE;
    out(data, r32(op->r1));
    out(data, " = (S32)tmp64;CYCLES(10);");
}

U32 getBlockEipCount(struct Block* block);

struct Op* getLastOp(struct Block* block) {
    struct Op* result = block->ops;
    while (result->next) {
        result = result->next;
    }
    return result;
}

void OPCALL jump(struct CPU* cpu, struct Op* op);
void OPCALL firstOp(struct CPU* cpu, struct Op* op);
void OPCALL restoreOps(struct CPU* cpu, struct Op* op);

void addBlockToData(struct GenData* data, struct Block* block) {
    struct Op* op = block->ops;
    U32 i;

    if (op->func==firstOp)
        op = op->next;
    while (op) {
        for (i=0;i<op->eipCount;i++) {
            if (data->opPos>=data->maxOpPos)
                kpanic("Block is too big");
            data->ops[data->opPos++] = readb(data->cpu->thread, data->ip++);
        }
        op = op->next;
    }
}

void genJump(struct GenData* data, struct Op* op, int condition) {
    char tmp[16];
    itoa(op->data1+op->eipCount, tmp, 10);
    
    if ((S32)op->data1>0 && data->block->block2 && data->block->block1 && getBlockEipCount(data->block->block1) > op->data1) {
        struct Block* block = data->block;
        U32 eip = data->eip;
        struct Op* first;

        if (block->block1->ops->func == restoreOps) {
            decodeBlockWithBlock(data->cpu, data->eip+getBlockEipCount(data->block), block->block1);
        }
        first = block->block1->ops;
        if (first->func == firstOp)
            first = first->next;
        if ((S32)op->data1 < first->eipCount) {
            // this if statement is for a prefix, like lock
            if (op->data1==1) {
                U8 inst = readb(data->cpu->thread, data->eip+getBlockEipCount(data->block));
                if (inst != 0xF0)
                    kpanic("genJump wasn't prepared for non lock prefix");
                // we can ignore a lock prefix thuse we can ignore this entire conditional jump
                out(data, "// removed conditional jump / lock instruction\n");
                out(data, "    cpu->eip.u32+=");
                itoa(op->eipCount+op->data1, tmp, 10);
                out(data, tmp);
                out(data, ";\n");
            } else {
                kpanic("genJump wasn't prepared for more than a 1 byte prefix");
            }
        } else {
            out(data, "if (");
            out(data, getCondition(data, condition));           
            out(data, ") {\n        cpu->eip.u32+=");
            out(data, tmp); 
            out(data, ";CYCLES(1);\n    } else {\n        cpu->eip.u32+=");
            itoa(op->eipCount, tmp, 10);
            out(data, tmp);
            out(data, ";CYCLES(1);\n");
            data->block = data->block->block1;
            data->eip += getBlockEipCount(block);
            data->inlinedBlock = 1;
            data->indent++;
            writeBlockWithEipCount(data, block->block1, op->data1);
            addBlockToData(data, block->block1);
            data->indent--;
            data->inlinedBlock = 0;
            data->block = block;
            data->eip = eip;
            out (data, "    }\n");
        }
        data->eip += getBlockEipCount(block)+op->data1;
        data->block = block->block2;
        writeBlock(data, block->block2);
        addBlockToData(data, block->block2);
    } else if ((S32)op->data1>0 && data->block->block2 && data->block->block1 && data->block->block1 == data->block->block2->block1 && getLastOp(data->block->block2)->func==jump) {
        struct Block* block = data->block;
        U32 eip = data->eip;

        out(data, "if (");
        out(data, getCondition(data, condition));           
        out(data, ") {\n        cpu->eip.u32+=");
        out(data, tmp); 
        out(data, ";CYCLES(1);\n");
        data->block = data->block->block2;
        data->eip += getBlockEipCount(block);
        data->inlinedBlock = 1;
        data->indent++;
        writeBlock(data, block->block2);
        addBlockToData(data, block->block2);
        data->indent--;
        data->inlinedBlock = 0;
        data->block = block;
        data->eip = eip;
        out(data, "\n    } else {\n        cpu->eip.u32+=");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, ";CYCLES(1);\n    }\n");
        data->eip += getBlockEipCount(block);
        data->block = block->block1;
        writeBlock(data, block->block1);
        addBlockToData(data, block->block1);
    } else {    
        out(data, "if (");
        out(data, getCondition(data, condition));           
        out(data, ") {cpu->eip.u32+=");
        out(data, tmp); 
        out(data, "; cpu->nextBlock = getBlock2(cpu);} else {cpu->eip.u32+=");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, ";cpu->nextBlock = getBlock1(cpu);}CYCLES(1);");
    }    
}

void gen070(struct GenData* data, struct Op* op) {
    genJump(data, op, 0);
}

void gen071(struct GenData* data, struct Op* op) {
    genJump(data, op, 1);
}

void gen072(struct GenData* data, struct Op* op) {
    genJump(data, op, 2);
}

void gen073(struct GenData* data, struct Op* op) {
    genJump(data, op, 3);
}

void gen074(struct GenData* data, struct Op* op) {
    genJump(data, op, 4);
}

void gen075(struct GenData* data, struct Op* op) {
    genJump(data, op, 5);
}

void gen076(struct GenData* data, struct Op* op) {
    genJump(data, op, 6);
}

void gen077(struct GenData* data, struct Op* op) {
    genJump(data, op, 7);
}

void gen078(struct GenData* data, struct Op* op) {
    genJump(data, op, 8);
}

void gen079(struct GenData* data, struct Op* op) {
    genJump(data, op, 9);
}

void gen07a(struct GenData* data, struct Op* op) {
    genJump(data, op, 10);
}

void gen07b(struct GenData* data, struct Op* op) {
    genJump(data, op, 11);
}

void gen07c(struct GenData* data, struct Op* op) {
    genJump(data, op, 12);
}

void gen07d(struct GenData* data, struct Op* op) {
    genJump(data, op, 13);
}

void gen07e(struct GenData* data, struct Op* op) {
    genJump(data, op, 14);
}

void gen07f(struct GenData* data, struct Op* op) {
    genJump(data, op, 15);
}

#include "gensrc.h"

void OPCALL add8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL add8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL add8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL add8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL or8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL or8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL or8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL or8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adc8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL adc8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL adc8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adc8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbb8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL and8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL and8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL and8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL and8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sub8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sub8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sub8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sub8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xor8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL xor8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL xor8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xor8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmp8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmp8_mem32(struct CPU* cpu, struct Op* op);
void gen080(struct GenData* data, struct Op* op) {
    if (op->func==add8_reg) {
        genArithR(data, "+", "FLAGS_ADD8", "8", r8(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==add8_mem16) {
        genArithE(data, "+", "FLAGS_ADD8", "8", getEaa16(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==add8_mem32) {
        genArithE(data, "+", "FLAGS_ADD8", "8", getEaa32(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_ADD8;
    } else if (op->func==add8_reg_noflags) {
        genArithR_noflags(data, "+", "8", r8(op->r1), op->data1, 0, "1");
    } else if (op->func==add8_mem16_noflags) {
        genArithE_noflags(data, "+", "8", getEaa16(op), "b", op->data1, 0, "3");
    } else if (op->func==add8_mem32_noflags) {
        genArithE_noflags(data, "+", "8", getEaa32(op), "b", op->data1, 0, "3");
    }

    else if (op->func==or8_reg) {
        genArithR(data, "|", "FLAGS_OR8", "8", r8(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==or8_mem16) {
        genArithE(data, "|", "FLAGS_OR8", "8", getEaa16(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==or8_mem32) {
        genArithE(data, "|", "FLAGS_OR8", "8", getEaa32(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_OR8;
    } else if (op->func==or8_reg_noflags) {
        genArithR_noflags(data, "|", "8", r8(op->r1), op->data1, 0, "1");
    } else if (op->func==or8_mem16_noflags) {
        genArithE_noflags(data, "|", "8", getEaa16(op), "b", op->data1, 0, "3");
    } else if (op->func==or8_mem32_noflags) {
        genArithE_noflags(data, "|", "8", getEaa32(op), "b", op->data1, 0, "3");
    }

    else if (op->func==adc8_reg) {
        genArithR(data, "+", "FLAGS_ADC8", "8", r8(op->r1), op->data1, 1, 1, "1");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adc8_mem16) {
        genArithE(data, "+", "FLAGS_ADC8", "8", getEaa16(op), "b", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adc8_mem32) {
        genArithE(data, "+", "FLAGS_ADC8", "8", getEaa32(op), "b", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_ADC8;
    } else if (op->func==adc8_reg_noflags) {
        genArithR_noflags(data, "+", "8", r8(op->r1), op->data1, 1, "1");
    } else if (op->func==adc8_mem16_noflags) {
        genArithE_noflags(data, "+", "8", getEaa16(op), "b", op->data1, 1, "3");
    } else if (op->func==adc8_mem32_noflags) {
        genArithE_noflags(data, "+", "8", getEaa32(op), "b", op->data1, 1, "3");
    }

    else if (op->func==sbb8_reg) {
        genArithR(data, "-", "FLAGS_SBB8", "8", r8(op->r1), op->data1, 1, 1, "1");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbb8_mem16) {
        genArithE(data, "-", "FLAGS_SBB8", "8", getEaa16(op), "b", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbb8_mem32) {
        genArithE(data, "-", "FLAGS_SBB8", "8", getEaa32(op), "b", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_SBB8;
    } else if (op->func==sbb8_reg_noflags) {
        genArithR_noflags(data, "-", "8", r8(op->r1), op->data1, 1, "1");
    } else if (op->func==sbb8_mem16_noflags) {
        genArithE_noflags(data, "-", "8", getEaa16(op), "b", op->data1, 1, "3");
    } else if (op->func==sbb8_mem32_noflags) {
        genArithE_noflags(data, "-", "8", getEaa32(op), "b", op->data1, 1, "3");
    }

    else if (op->func==and8_reg) {
        genArithR(data, "&", "FLAGS_AND8", "8", r8(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==and8_mem16) {
        genArithE(data, "&", "FLAGS_AND8", "8", getEaa16(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==and8_mem32) {
        genArithE(data, "&", "FLAGS_AND8", "8", getEaa32(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_AND8;
    } else if (op->func==and8_reg_noflags) {
        genArithR_noflags(data, "&", "8", r8(op->r1), op->data1, 0, "1");
    } else if (op->func==and8_mem16_noflags) {
        genArithE_noflags(data, "&", "8", getEaa16(op), "b", op->data1, 0, "3");
    } else if (op->func==and8_mem32_noflags) {
        genArithE_noflags(data, "&", "8", getEaa32(op), "b", op->data1, 0, "3");
    }

    else if (op->func==sub8_reg) {
        genArithR(data, "-", "FLAGS_SUB8", "8", r8(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==sub8_mem16) {
        genArithE(data, "-", "FLAGS_SUB8", "8", getEaa16(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==sub8_mem32) {
        genArithE(data, "-", "FLAGS_SUB8", "8", getEaa32(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_SUB8;
    } else if (op->func==sub8_reg_noflags) {
        genArithR_noflags(data, "-", "8", r8(op->r1), op->data1, 0, "1");
    } else if (op->func==sub8_mem16_noflags) {
        genArithE_noflags(data, "-", "8", getEaa16(op), "b", op->data1, 0, "3");
    } else if (op->func==sub8_mem32_noflags) {
        genArithE_noflags(data, "-", "8", getEaa32(op), "b", op->data1, 0, "3");
    }

    else if (op->func==xor8_reg) {
        genArithR(data, "^", "FLAGS_XOR8", "8", r8(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xor8_mem16) {
        genArithE(data, "^", "FLAGS_XOR8", "8", getEaa16(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xor8_mem32) {
        genArithE(data, "^", "FLAGS_XOR8", "8", getEaa32(op), "b", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_XOR8;
    } else if (op->func==xor8_reg_noflags) {
        genArithR_noflags(data, "^", "8", r8(op->r1), op->data1, 0, "1");
    } else if (op->func==xor8_mem16_noflags) {
        genArithE_noflags(data, "^", "8", getEaa16(op), "b", op->data1, 0, "3");
    } else if (op->func==xor8_mem32_noflags) {
        genArithE_noflags(data, "^", "8", getEaa32(op), "b", op->data1, 0, "3");
    }

    else if (op->func==cmp8_reg) {
        genArithR(data, "-", "FLAGS_CMP8", "8", r8(op->r1), op->data1, 0, 0, "1");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmp8_mem16) {
        genArithE(data, "-", "FLAGS_CMP8", "8", getEaa16(op), "b", op->data1, 0, 0, "2");
        data->lazyFlags = sFLAGS_CMP8;
    } else if (op->func==cmp8_mem32) {
        genArithE(data, "-", "FLAGS_CMP8", "8", getEaa32(op), "b", op->data1, 0, 0, "2");
        data->lazyFlags = sFLAGS_CMP8;
    } else {
        kpanic("gen080");
    }
}

void OPCALL add16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL add16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL add16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL add16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL or16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL or16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL or16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL or16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adc16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL adc16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL adc16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adc16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbb16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL and16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL and16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL and16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL and16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sub16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sub16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sub16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sub16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xor16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL xor16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL xor16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xor16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmp16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmp16_mem32(struct CPU* cpu, struct Op* op);
void gen081(struct GenData* data, struct Op* op) {
    if (op->func==add16_reg) {
        genArithR(data, "+", "FLAGS_ADD16", "16", r16(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==add16_mem16) {
        genArithE(data, "+", "FLAGS_ADD16", "16", getEaa16(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==add16_mem32) {
        genArithE(data, "+", "FLAGS_ADD16", "16", getEaa32(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_ADD16;
    } else if (op->func==add16_reg_noflags) {
        genArithR_noflags(data, "+", "16", r16(op->r1), op->data1, 0, "1");
    } else if (op->func==add16_mem16_noflags) {
        genArithE_noflags(data, "+", "16", getEaa16(op), "w", op->data1, 0, "3");
    } else if (op->func==add16_mem32_noflags) {
        genArithE_noflags(data, "+", "16", getEaa32(op), "w", op->data1, 0, "3");
    }

    else if (op->func==or16_reg) {
        genArithR(data, "|", "FLAGS_OR16", "16", r16(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==or16_mem16) {
        genArithE(data, "|", "FLAGS_OR16", "16", getEaa16(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==or16_mem32) {
        genArithE(data, "|", "FLAGS_OR16", "16", getEaa32(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_OR16;
    } else if (op->func==or16_reg_noflags) {
        genArithR_noflags(data, "|", "16", r16(op->r1), op->data1, 0, "1");
    } else if (op->func==or16_mem16_noflags) {
        genArithE_noflags(data, "|", "16", getEaa16(op), "w", op->data1, 0, "3");
    } else if (op->func==or16_mem32_noflags) {
        genArithE_noflags(data, "|", "16", getEaa32(op), "w", op->data1, 0, "3");
    }

    else if (op->func==adc16_reg) {
        genArithR(data, "+", "FLAGS_ADC16", "16", r16(op->r1), op->data1, 1, 1, "1");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adc16_mem16) {
        genArithE(data, "+", "FLAGS_ADC16", "16", getEaa16(op), "w", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adc16_mem32) {
        genArithE(data, "+", "FLAGS_ADC16", "16", getEaa32(op), "w", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_ADC16;
    } else if (op->func==adc16_reg_noflags) {
        genArithR_noflags(data, "+", "16", r16(op->r1), op->data1, 1, "1");
    } else if (op->func==adc16_mem16_noflags) {
        genArithE_noflags(data, "+", "16", getEaa16(op), "w", op->data1, 1, "3");
    } else if (op->func==adc16_mem32_noflags) {
        genArithE_noflags(data, "+", "16", getEaa32(op), "w", op->data1, 1, "3");
    }

    else if (op->func==sbb16_reg) {
        genArithR(data, "-", "FLAGS_SBB16", "16", r16(op->r1), op->data1, 1, 1, "1");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbb16_mem16) {
        genArithE(data, "-", "FLAGS_SBB16", "16", getEaa16(op), "w", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbb16_mem32) {
        genArithE(data, "-", "FLAGS_SBB16", "16", getEaa32(op), "w", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_SBB16;
    } else if (op->func==sbb16_reg_noflags) {
        genArithR_noflags(data, "-", "16", r16(op->r1), op->data1, 1, "1");
    } else if (op->func==sbb16_mem16_noflags) {
        genArithE_noflags(data, "-", "16", getEaa16(op), "w", op->data1, 1, "3");
    } else if (op->func==sbb16_mem32_noflags) {
        genArithE_noflags(data, "-", "16", getEaa32(op), "w", op->data1, 1, "3");
    }

    else if (op->func==and16_reg) {
        genArithR(data, "&", "FLAGS_AND16", "16", r16(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==and16_mem16) {
        genArithE(data, "&", "FLAGS_AND16", "16", getEaa16(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==and16_mem32) {
        genArithE(data, "&", "FLAGS_AND16", "16", getEaa32(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_AND16;
    } else if (op->func==and16_reg_noflags) {
        genArithR_noflags(data, "&", "16", r16(op->r1), op->data1, 0, "1");
    } else if (op->func==and16_mem16_noflags) {
        genArithE_noflags(data, "&", "16", getEaa16(op), "w", op->data1, 0, "3");
    } else if (op->func==and16_mem32_noflags) {
        genArithE_noflags(data, "&", "16", getEaa32(op), "w", op->data1, 0, "3");
    }

    else if (op->func==sub16_reg) {
        genArithR(data, "-", "FLAGS_SUB16", "16", r16(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==sub16_mem16) {
        genArithE(data, "-", "FLAGS_SUB16", "16", getEaa16(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==sub16_mem32) {
        genArithE(data, "-", "FLAGS_SUB16", "16", getEaa32(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_SUB16;
    } else if (op->func==sub16_reg_noflags) {
        genArithR_noflags(data, "-", "16", r16(op->r1), op->data1, 0, "1");
    } else if (op->func==sub16_mem16_noflags) {
        genArithE_noflags(data, "-", "16", getEaa16(op), "w", op->data1, 0, "3");
    } else if (op->func==sub16_mem32_noflags) {
        genArithE_noflags(data, "-", "16", getEaa32(op), "w", op->data1, 0, "3");
    }

    else if (op->func==xor16_reg) {
        genArithR(data, "^", "FLAGS_XOR16", "16", r16(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xor16_mem16) {
        genArithE(data, "^", "FLAGS_XOR16", "16", getEaa16(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xor16_mem32) {
        genArithE(data, "^", "FLAGS_XOR16", "16", getEaa32(op), "w", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_XOR16;
    } else if (op->func==xor16_reg_noflags) {
        genArithR_noflags(data, "^", "16", r16(op->r1), op->data1, 0, "1");
    } else if (op->func==xor16_mem16_noflags) {
        genArithE_noflags(data, "^", "16", getEaa16(op), "w", op->data1, 0, "3");
    } else if (op->func==xor16_mem32_noflags) {
        genArithE_noflags(data, "^", "16", getEaa32(op), "w", op->data1, 0, "3");
    }

    else if (op->func==cmp16_reg) {
        genArithR(data, "-", "FLAGS_CMP16", "16", r16(op->r1), op->data1, 0, 0, "1");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmp16_mem16) {
        genArithE(data, "-", "FLAGS_CMP16", "16", getEaa16(op), "w", op->data1, 0, 0, "2");
        data->lazyFlags = sFLAGS_CMP16;
    } else if (op->func==cmp16_mem32) {
        genArithE(data, "-", "FLAGS_CMP16", "16", getEaa32(op), "w", op->data1, 0, 0, "2");
        data->lazyFlags = sFLAGS_CMP16;
    } else {
        kpanic("gen081");
    }
}

void OPCALL add32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL add32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL add32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL add32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL or32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL or32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL or32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL or32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adc32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL adc32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL adc32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL adc32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sbb32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL and32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL and32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL and32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL and32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sub32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL sub32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL sub32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sub32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xor32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL xor32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL xor32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xor32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmp32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL cmp32_mem32(struct CPU* cpu, struct Op* op);
void gen281(struct GenData* data, struct Op* op) {
    if (op->func==add32_reg) {
        genArithR(data, "+", "FLAGS_ADD32", "32", r32(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==add32_mem16) {
        genArithE(data, "+", "FLAGS_ADD32", "32", getEaa16(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==add32_mem32) {
        genArithE(data, "+", "FLAGS_ADD32", "32", getEaa32(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_ADD32;
    } else if (op->func==add32_reg_noflags) {
        genArithR_noflags(data, "+", "32", r32(op->r1), op->data1, 0, "1");
    } else if (op->func==add32_mem16_noflags) {
        genArithE_noflags(data, "+", "32", getEaa16(op), "d", op->data1, 0, "3");
    } else if (op->func==add32_mem32_noflags) {
        genArithE_noflags(data, "+", "32", getEaa32(op), "d", op->data1, 0, "3");
    }

    else if (op->func==or32_reg) {
        genArithR(data, "|", "FLAGS_OR32", "32", r32(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==or32_mem16) {
        genArithE(data, "|", "FLAGS_OR32", "32", getEaa16(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==or32_mem32) {
        genArithE(data, "|", "FLAGS_OR32", "32", getEaa32(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_OR32;
    } else if (op->func==or32_reg_noflags) {
        genArithR_noflags(data, "|", "32", r32(op->r1), op->data1, 0, "1");
    } else if (op->func==or32_mem16_noflags) {
        genArithE_noflags(data, "|", "32", getEaa16(op), "d", op->data1, 0, "3");
    } else if (op->func==or32_mem32_noflags) {
        genArithE_noflags(data, "|", "32", getEaa32(op), "d", op->data1, 0, "3");
    }

    else if (op->func==adc32_reg) {
        genArithR(data, "+", "FLAGS_ADC32", "32", r32(op->r1), op->data1, 1, 1, "1");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adc32_mem16) {
        genArithE(data, "+", "FLAGS_ADC32", "32", getEaa16(op), "d", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adc32_mem32) {
        genArithE(data, "+", "FLAGS_ADC32", "32", getEaa32(op), "d", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_ADC32;
    } else if (op->func==adc32_reg_noflags) {
        genArithR_noflags(data, "+", "32", r32(op->r1), op->data1, 1, "1");
    } else if (op->func==adc32_mem16_noflags) {
        genArithE_noflags(data, "+", "32", getEaa16(op), "d", op->data1, 1, "3");
    } else if (op->func==adc32_mem32_noflags) {
        genArithE_noflags(data, "+", "32", getEaa32(op), "d", op->data1, 1, "3");
    }

    else if (op->func==sbb32_reg) {
        genArithR(data, "-", "FLAGS_SBB32", "32", r32(op->r1), op->data1, 1, 1, "1");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbb32_mem16) {
        genArithE(data, "-", "FLAGS_SBB32", "32", getEaa16(op), "d", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbb32_mem32) {
        genArithE(data, "-", "FLAGS_SBB32", "32", getEaa32(op), "d", op->data1, 1, 1, "3");
        data->lazyFlags = sFLAGS_SBB32;
    } else if (op->func==sbb32_reg_noflags) {
        genArithR_noflags(data, "-", "32", r32(op->r1), op->data1, 1, "1");
    } else if (op->func==sbb32_mem16_noflags) {
        genArithE_noflags(data, "-", "32", getEaa16(op), "d", op->data1, 1, "3");
    } else if (op->func==sbb32_mem32_noflags) {
        genArithE_noflags(data, "-", "32", getEaa32(op), "d", op->data1, 1, "3");
    }

    else if (op->func==and32_reg) {
        genArithR(data, "&", "FLAGS_AND32", "32", r32(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==and32_mem16) {
        genArithE(data, "&", "FLAGS_AND32", "32", getEaa16(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==and32_mem32) {
        genArithE(data, "&", "FLAGS_AND32", "32", getEaa32(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_AND32;
    } else if (op->func==and32_reg_noflags) {
        genArithR_noflags(data, "&", "32", r32(op->r1), op->data1, 0, "1");
    } else if (op->func==and32_mem16_noflags) {
        genArithE_noflags(data, "&", "32", getEaa16(op), "d", op->data1, 0, "3");
    } else if (op->func==and32_mem32_noflags) {
        genArithE_noflags(data, "&", "32", getEaa32(op), "d", op->data1, 0, "3");
    }

    else if (op->func==sub32_reg) {
        genArithR(data, "-", "FLAGS_SUB32", "32", r32(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==sub32_mem16) {
        genArithE(data, "-", "FLAGS_SUB32", "32", getEaa16(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==sub32_mem32) {
        genArithE(data, "-", "FLAGS_SUB32", "32", getEaa32(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_SUB32;
    } else if (op->func==sub32_reg_noflags) {
        genArithR_noflags(data, "-", "32", r32(op->r1), op->data1, 0, "1");
    } else if (op->func==sub32_mem16_noflags) {
        genArithE_noflags(data, "-", "32", getEaa16(op), "d", op->data1, 0, "3");
    } else if (op->func==sub32_mem32_noflags) {
        genArithE_noflags(data, "-", "32", getEaa32(op), "d", op->data1, 0, "3");
    }

    else if (op->func==xor32_reg) {
        genArithR(data, "^", "FLAGS_XOR32", "32", r32(op->r1), op->data1, 1, 0, "1");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xor32_mem16) {
        genArithE(data, "^", "FLAGS_XOR32", "32", getEaa16(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xor32_mem32) {
        genArithE(data, "^", "FLAGS_XOR32", "32", getEaa32(op), "d", op->data1, 1, 0, "3");
        data->lazyFlags = sFLAGS_XOR32;
    } else if (op->func==xor32_reg_noflags) {
        genArithR_noflags(data, "^", "32", r32(op->r1), op->data1, 0, "1");
    } else if (op->func==xor32_mem16_noflags) {
        genArithE_noflags(data, "^", "32", getEaa16(op), "d", op->data1, 0, "3");
    } else if (op->func==xor32_mem32_noflags) {
        genArithE_noflags(data, "^", "32", getEaa32(op), "d", op->data1, 0, "3");
    }

    else if (op->func==cmp32_reg) {
        genArithR(data, "-", "FLAGS_CMP32", "32", r32(op->r1), op->data1, 0, 0, "1");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmp32_mem16) {
        genArithE(data, "-", "FLAGS_CMP32", "32", getEaa16(op), "d", op->data1, 0, 0, "2");
        data->lazyFlags = sFLAGS_CMP32;
    } else if (op->func==cmp32_mem32) {
        genArithE(data, "-", "FLAGS_CMP32", "32", getEaa32(op), "d", op->data1, 0, 0, "2");
        data->lazyFlags = sFLAGS_CMP32;
    } else {
        kpanic("gen0320");
    }
}

void OPCALL testr8r8(struct CPU* cpu, struct Op* op);
void OPCALL teste8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL teste8r8_32(struct CPU* cpu, struct Op* op);
void gen084(struct GenData* data, struct Op* op) {    
    if (inlineTestJump(data, op, sFLAGS_TEST8, op->func == testr8r8?"1":"2"))
        return;

    if (op->func == testr8r8) {
        genArithRR(data, "&", "FLAGS_TEST8", "8", r8(op->r1), r8(op->r2), 0, 0, "1");
    } else if (op->func==teste8r8_16) {
        genArithER(data, "&", "FLAGS_TEST8", "8", getEaa16(op), "b", r8(op->r1), 0, 0,"2");
    } else if (op->func==teste8r8_32) {
        genArithER(data, "&", "FLAGS_TEST8", "8", getEaa32(op), "b", r8(op->r1), 0, 0,"2");
    } else {
        kpanic("gen084");
    }
    data->lazyFlags = sFLAGS_TEST8;
}

void OPCALL testr16r16(struct CPU* cpu, struct Op* op);
void OPCALL teste16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL teste16r16_32(struct CPU* cpu, struct Op* op);
void gen085(struct GenData* data, struct Op* op) {
    if (inlineTestJump(data, op, sFLAGS_TEST16, op->func == testr16r16?"1":"2"))
        return;

    if (op->func == testr16r16) {
        genArithRR(data, "&", "FLAGS_TEST16", "16", r16(op->r1), r16(op->r2), 0, 0, "1");
    } else if (op->func==teste16r16_16) {
        genArithER(data, "&", "FLAGS_TEST16", "16", getEaa16(op), "w", r16(op->r1), 0, 0,"2");
    } else if (op->func==teste16r16_32) {
        genArithER(data, "&", "FLAGS_TEST16", "16", getEaa32(op), "w", r16(op->r1), 0, 0,"2");
    } else {
        kpanic("gen085");
    }
    data->lazyFlags = sFLAGS_TEST16;
}

void OPCALL testr32r32(struct CPU* cpu, struct Op* op);
void OPCALL teste32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL teste32r32_32(struct CPU* cpu, struct Op* op);
void gen285(struct GenData* data, struct Op* op) {
    if (inlineTestJump(data, op, sFLAGS_TEST32, op->func == testr32r32?"1":"2"))
        return;

    if (op->func == testr32r32) {
        genArithRR(data, "&", "FLAGS_TEST32", "32", r32(op->r1), r32(op->r2), 0, 0, "1");        
    } else if (op->func==teste32r32_16) {
        genArithER(data, "&", "FLAGS_TEST32", "32", getEaa16(op), "d", r32(op->r1), 0, 0,"2");
    } else if (op->func==teste32r32_32) {
        genArithER(data, "&", "FLAGS_TEST32", "32", getEaa32(op), "d", r32(op->r1), 0, 0,"2");
    } else {
        kpanic("gen285");
    }
    data->lazyFlags = sFLAGS_TEST32;
}

void OPCALL xchgr8r8(struct CPU* cpu, struct Op* op);
void OPCALL xchge8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL xchge8r8_32(struct CPU* cpu, struct Op* op);
void gen086(struct GenData* data, struct Op* op) {
    if (op->func == xchgr8r8) {
        out(data, "tmp8 = ");
        out(data, r8(op->r1));
        out(data, "; ");
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r2));
        out(data, "; ");
        out(data, r8(op->r2));
        out(data, " = tmp8; CYCLES(3);");
    } else {        
        out(data, "eaa = ");
        if (op->func == xchge8r8_16) {
            out(data, getEaa16(op));
        } else if (op->func == xchge8r8_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen086");
        }
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, ");
        out(data, r8(op->r1));
        out(data, "); ");
        out(data, r8(op->r1));
        out(data, " = tmp8; CYCLES(4);");
    }
}

void OPCALL xchgr16r16(struct CPU* cpu, struct Op* op);
void OPCALL xchge16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL xchge16r16_32(struct CPU* cpu, struct Op* op);
void gen087(struct GenData* data, struct Op* op) {
    if (op->func == xchgr16r16) {
        out(data, "tmp16 = ");
        out(data, r16(op->r1));
        out(data, "; ");
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, "; ");
        out(data, r16(op->r2));
        out(data, " = tmp16; CYCLES(3);");
    } else {        
        out(data, "eaa = ");
        if (op->func == xchge16r16_16) {
            out(data, getEaa16(op));
        } else if (op->func == xchge16r16_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen087");
        }
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, ");
        out(data, r16(op->r1));
        out(data, "); ");
        out(data, r16(op->r1));
        out(data, " = tmp16; CYCLES(4);");
    }
}

void OPCALL xchgr32r32(struct CPU* cpu, struct Op* op);
void OPCALL xchge32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL xchge32r32_32(struct CPU* cpu, struct Op* op);
void gen287(struct GenData* data, struct Op* op) {
    if (op->func == xchgr32r32) {
        out(data, "tmp32 = ");
        out(data, r32(op->r1));
        out(data, "; ");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, "; ");
        out(data, r32(op->r2));
        out(data, " = tmp32; CYCLES(3);");
    } else {        
        out(data, "eaa = ");
        if (op->func == xchge32r32_16) {
            out(data, getEaa16(op));
        } else if (op->func == xchge32r32_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen287");
        }
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, ");
        out(data, r32(op->r1));
        out(data, "); ");
        out(data, r32(op->r1));
        out(data, " = tmp32; CYCLES(4);");
    }
}


void OPCALL movr8r8(struct CPU* cpu, struct Op* op);
void OPCALL move8r8_16(struct CPU* cpu, struct Op* op);
void OPCALL move8r8_32(struct CPU* cpu, struct Op* op);
void gen088(struct GenData* data, struct Op* op) {
    if (op->func == movr8r8) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r2));
        out(data, "; CYCLES(1);");
    } else {                
        out(data, "writeb(cpu->thread, ");
        if (op->func == move8r8_16) {
            out(data, getEaa16(op));
        } else if (op->func == move8r8_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen088");
        }
        out(data, ", ");
        out(data, r8(op->r1));
        out(data, ");CYCLES(1);");
    }
}

void OPCALL movr16r16(struct CPU* cpu, struct Op* op);
void OPCALL move16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL move16r16_32(struct CPU* cpu, struct Op* op);
void gen089(struct GenData* data, struct Op* op) {
    if (op->func == movr16r16) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, "; CYCLES(1);");
    } else {        
        out(data, "writew(cpu->thread, ");
        if (op->func == move16r16_16) {
            out(data, getEaa16(op));
        } else if (op->func == move16r16_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen089");
        }
        out(data, ", ");
        out(data, r16(op->r1));
        out(data, ");CYCLES(1);");
    }
}

void OPCALL movr32r32(struct CPU* cpu, struct Op* op);
void OPCALL move32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL move32r32_32(struct CPU* cpu, struct Op* op);
void gen289(struct GenData* data, struct Op* op) {
    if (op->func == movr32r32) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, "; CYCLES(1);");
    } else {        
        out(data, "writed(cpu->thread, ");
        if (op->func == move32r32_16) {
            out(data, getEaa16(op));
        } else if (op->func == move32r32_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen289");
        }
        out(data, ", ");
        out(data, r32(op->r1));
        out(data, ");CYCLES(1);");
    }
}

void OPCALL movr8e8_16(struct CPU* cpu, struct Op* op);
void OPCALL movr8e8_32(struct CPU* cpu, struct Op* op);
void gen08a(struct GenData* data, struct Op* op) {
    if (op->func == movr8r8) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r2));
        out(data, "; CYCLES(1);");
    } else {                
        out(data, r8(op->r1));
        out(data, " = readb(cpu->thread, ");
        if (op->func == movr8e8_16) {
            out(data, getEaa16(op));
        } else if (op->func == movr8e8_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen08a");
        }
        out(data, ");CYCLES(1);");
    }
}

void OPCALL movr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL movr16e16_32(struct CPU* cpu, struct Op* op);
void gen08b(struct GenData* data, struct Op* op) {
    if (op->func == movr16r16) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, "; CYCLES(1);");
    } else {                
        out(data, r16(op->r1));
        out(data, " = readw(cpu->thread, ");
        if (op->func == movr16e16_16) {
            out(data, getEaa16(op));
        } else if (op->func == movr16e16_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen08b");
        }
        out(data, ");CYCLES(1);");
    }
}

void OPCALL movr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL movr32e32_32(struct CPU* cpu, struct Op* op);
void gen28b(struct GenData* data, struct Op* op) {
    if (op->func == movr32r32) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, "; CYCLES(1);");
    } else {                
        out(data, r32(op->r1));
        out(data, " = readd(cpu->thread, ");
        if (op->func == movr32e32_16) {
            out(data, getEaa16(op));
        } else if (op->func == movr32e32_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen28b");
        }
        out(data, ");CYCLES(1);");
    }
}

void OPCALL movr16s16(struct CPU* cpu, struct Op* op);
void OPCALL movr32s16(struct CPU* cpu, struct Op* op);
void OPCALL move16s16_16(struct CPU* cpu, struct Op* op);
void OPCALL move16s16_32(struct CPU* cpu, struct Op* op);
void gen08c(struct GenData* data, struct Op* op) {
    if (op->func == movr16s16) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, "cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "]; CYCLES(1);");
    } else if (op->func == movr32s16) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, "cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "]; CYCLES(1);");
    } else {                
        out(data, "writew(cpu->thread, ");
        if (op->func == move16s16_16) {
            out(data, getEaa16(op));
        } else if (op->func == move16s16_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen08b");
        }
        out(data, ", ");
        out(data, "cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "]);CYCLES(1);");
    }
}

void OPCALL lear16_32(struct CPU* cpu, struct Op* op);
void OPCALL lear16_16(struct CPU* cpu, struct Op* op);
void gen08d(struct GenData* data, struct Op* op) {
    out(data, r16(op->r1));
    out(data, " = ");
    if (op->func == lear16_16) {
        out(data, getEaa16(op));        
    } else if (op->func == lear16_32) {        
        out(data, getEaa32(op));
    } else {
        kpanic("gen08d");
    }
    out(data, "; CYCLES(1);");
}

void OPCALL lear32_16(struct CPU* cpu, struct Op* op);
void OPCALL lear32_32(struct CPU* cpu, struct Op* op);
void gen28d(struct GenData* data, struct Op* op) {
    out(data, r32(op->r1));
    out(data, " = ");
    if (op->func == lear32_16) {
        out(data, getEaa16(op));        
    } else if (op->func == lear32_32) {        
        out(data, getEaa32(op));
    } else {
        kpanic("gen28d");
    }
    out(data, "; CYCLES(1);");
}

void OPCALL movs16r16(struct CPU* cpu, struct Op* op);
void OPCALL movs16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL movs16e16_32(struct CPU* cpu, struct Op* op);
void gen08e(struct GenData* data, struct Op* op) {
    if (op->func == movs16r16) {
        out(data, "cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "] = ");
        out(data, r16(op->r1));
        out(data, "; cpu->segAddress[");
        out(data, getBase(op->r2));
        out(data, "] = cpu->ldt[cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "] >> 3].base_addr; CYCLES(2);");
    } else {                
        out(data, "cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "] = readw(cpu->thread, ");
        if (op->func == movs16e16_16) {
            out(data, getEaa16(op));
        } else if (op->func == movs16e16_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen08e");
        }
        out(data, "); cpu->segAddress[");
        out(data, getBase(op->r2));
        out(data, "] = cpu->ldt[cpu->segValue[");
        out(data, getBase(op->r2));
        out(data, "] >> 3].base_addr; CYCLES(3);");
    }
}

void OPCALL popReg16(struct CPU* cpu, struct Op* op);
void OPCALL pope16_16(struct CPU* cpu, struct Op* op);
void OPCALL pope16_32(struct CPU* cpu, struct Op* op);
void gen08f(struct GenData* data, struct Op* op) {
    if (op->func == popReg16) {
        out(data, r16(op->r1));
        out(data, " = pop16(cpu); CYCLES(1);");
    } else {                
        out(data, "writew(cpu->thread, ");
        if (op->func == pope16_16) {
            out(data, getEaa16(op));
        } else if (op->func == pope16_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen08f");
        }
        out(data, ", pop16(cpu)); CYCLES(3);");
    }
}

void OPCALL popReg32(struct CPU* cpu, struct Op* op);
void OPCALL pope32_16(struct CPU* cpu, struct Op* op);
void OPCALL pope32_32(struct CPU* cpu, struct Op* op);
void gen28f(struct GenData* data, struct Op* op) {
    if (op->func == popReg32) {
        out(data, r32(op->r1));
        out(data, " = pop32(cpu); CYCLES(1);");
    } else {                
        out(data, "writed(cpu->thread, ");
        if (op->func == pope32_16) {
            out(data, getEaa16(op));
        } else if (op->func == pope32_32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen28f");
        }
        out(data, ", pop32(cpu)); CYCLES(3);");
    }
}

void gen090(struct GenData* data, struct Op* op) {
    out(data, "CYCLES(1);");
}

void gen091(struct GenData* data, struct Op* op) {
    out(data, "tmp16 = ");
    out(data, r16(op->r1));
    out(data, "; ");
    out(data, r16(op->r1));
    out(data, " = ");
    out(data, r16(op->r2));
    out(data, "; ");
    out(data, r16(op->r2));
    out(data, " = tmp16; CYCLES(3);");
}

void gen291(struct GenData* data, struct Op* op) {
    out(data, "tmp32 = ");
    out(data, r32(op->r1));
    out(data, "; ");
    out(data, r32(op->r1));
    out(data, " = ");
    out(data, r32(op->r2));
    out(data, "; ");
    out(data, r32(op->r2));
    out(data, " = tmp32; CYCLES(3);");
}

void gen098(struct GenData* data, struct Op* op) {
    out(data, "AX = (S8)AL; CYCLES(3);");
}

void gen298(struct GenData* data, struct Op* op) {
    out(data, "EAX = (S16)AX; CYCLES(3);");
}

void gen099(struct GenData* data, struct Op* op) {
    out(data, "if (((S16)AX) < 0) DX = 0xFFFF; else DX = 0; CYCLES(2);");
}

void gen299(struct GenData* data, struct Op* op) {
    out(data, "if (((S32)EAX) < 0) EDX = 0xFFFFFFFF; else EDX = 0; CYCLES(2);");
}

void gen09a(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "cpu->eip.u32+=");
    itoa(op->eipCount, tmp, 10);
    out(data, tmp);
    out(data, ";cpu_call(cpu, 0, 0x");
    itoa(op->eData, tmp, 16);
    out(data, tmp);
    out(data, ", 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, ", cpu->eip.u32); CYCLES(4);");
}

void gen09c(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); push16(cpu, cpu->flags|2); CYCLES(3);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen29c(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); push32(cpu, (cpu->flags|2) & 0xFCFFFF); CYCLES(3);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen09d(struct GenData* data, struct Op* op) {
    out(data, "cpu->lazyFlags = FLAGS_NONE; setFlags(cpu, pop16(cpu), FMASK_ALL & 0xFFFF); CYCLES(4);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen29d(struct GenData* data, struct Op* op) {
    out(data, "cpu->lazyFlags = FLAGS_NONE; setFlags(cpu, pop32(cpu), FMASK_ALL); CYCLES(4);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen09e(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); setFlags(cpu, AH, FMASK_ALL & 0xFF); CYCLES(2);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen09f(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); AH = cpu->flags & (SF|ZF|AF|PF|CF); CYCLES(2);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen0a0(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "AL = readb(cpu->thread, cpu->segAddress[");
    out(data, getBase(op->base));
    out(data, "] + 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, "); CYCLES(1);");
}

void gen0a1(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "AX = readw(cpu->thread, cpu->segAddress[");
    out(data, getBase(op->base));
    out(data, "] + 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, "); CYCLES(1);");
}

void gen2a1(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "EAX = readd(cpu->thread, cpu->segAddress[");
    out(data, getBase(op->base));
    out(data, "] + 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, "); CYCLES(1);");
}

void gen0a2(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "writeb(cpu->thread, cpu->segAddress[");
    out(data, getBase(op->base));
    out(data, "] + 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, ", AL); CYCLES(1);");
}

void gen0a3(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "writew(cpu->thread, cpu->segAddress[");
    out(data, getBase(op->base));
    out(data, "] + 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, ", AX); CYCLES(1);");
}

void gen2a3(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "writed(cpu->thread, cpu->segAddress[");
    out(data, getBase(op->base));
    out(data, "] + 0x");
    itoa(op->data1, tmp, 16);
    out(data, tmp);
    out(data, ", EAX); CYCLES(1);");
}

void OPCALL movsb16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsb16_op(struct CPU* cpu, struct Op* op);
void OPCALL movsb32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsb32_op(struct CPU* cpu, struct Op* op);
void gen0a4(struct GenData* data, struct Op* op) {
    if (op->func==movsb16_r_op) {
        out(data, "movsb16_r(cpu, ");        
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==movsb16_op) {
        out(data, "writeb(cpu->thread, cpu->segAddress[ES]+DI, readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI)); DI+=cpu->df; SI+=cpu->df; CYCLES(4);");
    } else if (op->func==movsb32_r_op) {
        out(data, "movsb32_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==movsb32_op) {
        out(data, "writeb(cpu->thread, cpu->segAddress[ES]+EDI, readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI)); EDI+=cpu->df; ESI+=cpu->df; CYCLES(4);");
    } else {
        kpanic("gen0a4");
    }    
}

void OPCALL movsw16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsw16_op(struct CPU* cpu, struct Op* op);
void OPCALL movsw32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsw32_op(struct CPU* cpu, struct Op* op);
void gen0a5(struct GenData* data, struct Op* op) {
    if (op->func==movsw16_r_op) {
        out(data, "movsw16_r(cpu, ");        
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==movsw16_op) {
        out(data, "writew(cpu->thread, cpu->segAddress[ES]+DI, readw(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI)); DI+=cpu->df<<1; SI+=cpu->df<<1; CYCLES(4);");
    } else if (op->func==movsw32_r_op) {
        out(data, "movsw32_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==movsw32_op) {
        out(data, "writew(cpu->thread, cpu->segAddress[ES]+EDI, readw(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI)); EDI+=cpu->df<<1; ESI+=cpu->df<<1; CYCLES(4);");
    } else {
        kpanic("gen0a5");
    }    
}

void OPCALL movsd16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsd16_op(struct CPU* cpu, struct Op* op);
void OPCALL movsd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsd32_op(struct CPU* cpu, struct Op* op);
void gen2a5(struct GenData* data, struct Op* op) {
    if (op->func==movsd16_r_op) {
        out(data, "movsd16_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==movsd16_op) {
        out(data, "writed(cpu->thread, cpu->segAddress[ES]+DI, readd(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI)); DI+=cpu->df<<2; SI+=cpu->df<<2; CYCLES(4);");
    } else if (op->func==movsd32_r_op) {
        out(data, "movsd32_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==movsd32_op) {
        out(data, "writed(cpu->thread, cpu->segAddress[ES]+EDI, readd(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI)); EDI+=cpu->df<<2; ESI+=cpu->df<<2; CYCLES(4);");
    } else {
        kpanic("gen2a5");
    }
}

void OPCALL cmpsb16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsb16_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsb32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsb32_op(struct CPU* cpu, struct Op* op);
void gen0a6(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    if (op->func==cmpsb16_r_op) {
        out(data, "cmpsb16_r(cpu, ");        
        out(data, tmp);
        out(data, ", ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==cmpsb16_op) {
        out(data, "cpu->dst.u8 = readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI); cpu->src.u8 = readb(cpu->thread, cpu->segAddress[ES]+DI); cpu->result.u8 = cpu->dst.u8 - cpu->src.u8; cpu->lazyFlags = FLAGS_SUB8; DI+=cpu->df; SI+=cpu->df; CYCLES(5);");        
    } else if (op->func==cmpsb32_r_op) {
        out(data, "cmpsb32_r(cpu, ");
        out(data, tmp);
        out(data, ", ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==cmpsb32_op) {
        out(data, "cpu->dst.u8 = readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI); cpu->src.u8 = readb(cpu->thread, cpu->segAddress[ES]+EDI); cpu->result.u8 = cpu->dst.u8 - cpu->src.u8; cpu->lazyFlags = FLAGS_SUB8; EDI+=cpu->df; ESI+=cpu->df; CYCLES(5);");
    } else {
        kpanic("gen0a6");
    }   
    data->lazyFlags = sFLAGS_SUB8;
}

void OPCALL cmpsw16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsw16_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsw32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsw32_op(struct CPU* cpu, struct Op* op);
void gen0a7(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    if (op->func==cmpsw16_r_op) {
        out(data, "cmpsw16_r(cpu, ");
        out(data, tmp);
        out(data, ", ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==cmpsw16_op) {
        out(data, "cpu->dst.u16 = readw(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI); cpu->src.u16 = readw(cpu->thread, cpu->segAddress[ES]+DI); cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_SUB16; DI+=cpu->df<<1; SI+=cpu->df<<1; CYCLES(5);");
    } else if (op->func==cmpsw32_r_op) {
        out(data, "cmpsw32_r(cpu, ");
        out(data, tmp);
        out(data, ", ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==cmpsw32_op) {
        out(data, "cpu->dst.u16 = readw(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI); cpu->src.u16 = readw(cpu->thread, cpu->segAddress[ES]+EDI); cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_SUB16; EDI+=cpu->df<<1; ESI+=cpu->df<<1; CYCLES(5);");
    } else {
        kpanic("gen0a7");
    }    
    data->lazyFlags = sFLAGS_SUB16;
}

void OPCALL cmpsd16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsd16_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL cmpsd32_op(struct CPU* cpu, struct Op* op);
void gen2a7(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    if (op->func==cmpsd16_r_op) {
        out(data, "cmpsd16_r(cpu, ");
        out(data, tmp);
        out(data, ", ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==cmpsd16_op) {
        out(data, "cpu->dst.u32 = readd(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI); cpu->src.u32 = readd(cpu->thread, cpu->segAddress[ES]+DI); cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_SUB32; DI+=cpu->df<<2; SI+=cpu->df<<2; CYCLES(5);");
    } else if (op->func==cmpsd32_r_op) {
        out(data, "cmpsd32_r(cpu, ");
        out(data, tmp);
        out(data, ", ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==cmpsd32_op) {
        out(data, "cpu->dst.u32 = readd(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI); cpu->src.u32 = readd(cpu->thread, cpu->segAddress[ES]+EDI); cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_SUB32; EDI+=cpu->df<<2; ESI+=cpu->df<<2; CYCLES(5);");
    } else {
        kpanic("gen2a7");
    }   
    data->lazyFlags = sFLAGS_SUB32;
}

void OPCALL stosb16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosb16_op(struct CPU* cpu, struct Op* op);
void OPCALL stosb32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosb32_op(struct CPU* cpu, struct Op* op);
void gen0aa(struct GenData* data, struct Op* op) {
    if (op->func==stosb16_r_op) {
        out(data, "stosb16_r(cpu);");        
    } else if (op->func==stosb16_op) {
        out(data, "writeb(cpu->thread, cpu->segAddress[ES]+DI, AL); DI+=cpu->df; CYCLES(3);");
    } else if (op->func==stosb32_r_op) {
        out(data, "stosb32_r(cpu);");
    } else if (op->func==stosb32_op) {
        out(data, "writeb(cpu->thread, cpu->segAddress[ES]+EDI, AL); EDI+=cpu->df; CYCLES(3);");
    } else {
        kpanic("gen0aa");
    }
}

void OPCALL stosw16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosw16_op(struct CPU* cpu, struct Op* op);
void OPCALL stosw32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosw32_op(struct CPU* cpu, struct Op* op);
void gen0ab(struct GenData* data, struct Op* op) {
    if (op->func==stosw16_r_op) {
        out(data, "stosw16_r(cpu);");
    } else if (op->func==stosw16_op) {
        out(data, "writew(cpu->thread, cpu->segAddress[ES]+DI, AX); DI+=cpu->df<<1; CYCLES(3);");
    } else if (op->func==stosw32_r_op) {
        out(data, "stosw32_r(cpu);");
    } else if (op->func==stosw32_op) {
        out(data, "writew(cpu->thread, cpu->segAddress[ES]+EDI, AX); EDI+=cpu->df<<1; CYCLES(3);");
    } else {
        kpanic("gen0ab");
    }
}

void OPCALL stosd16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosd16_op(struct CPU* cpu, struct Op* op);
void OPCALL stosd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosd32_op(struct CPU* cpu, struct Op* op);
void gen2ab(struct GenData* data, struct Op* op) {
    if (op->func==stosd16_r_op) {
        out(data, "stosd16_r(cpu);");
    } else if (op->func==stosd16_op) {
        out(data, "writed(cpu->thread, cpu->segAddress[ES]+DI, EAX); DI+=cpu->df<<2; CYCLES(3);");
    } else if (op->func==stosd32_r_op) {
        out(data, "stosd32_r(cpu);");
    } else if (op->func==stosd32_op) {
        out(data, "writed(cpu->thread, cpu->segAddress[ES]+EDI, EAX); EDI+=cpu->df<<2; CYCLES(3);");
    } else {
        kpanic("gen2ab");
    }
}

void OPCALL lodsb16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsb16_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsb32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsb32_op(struct CPU* cpu, struct Op* op);
void gen0ac(struct GenData* data, struct Op* op) {
    if (op->func==lodsb16_r_op) {
        out(data, "lodsb16_r(cpu, ");        
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==lodsb16_op) {
        out(data, "AL = readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI); SI+=cpu->df; CYCLES(2);");
    } else if (op->func==lodsb32_r_op) {
        out(data, "lodsb32_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==lodsb32_op) {
        out(data, "AL = readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI); ESI+=cpu->df; CYCLES(2);");
    } else {
        kpanic("gen0ac");
    }    
}

void OPCALL lodsw16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsw16_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsw32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsw32_op(struct CPU* cpu, struct Op* op);
void gen0ad(struct GenData* data, struct Op* op) {
    if (op->func==lodsw16_r_op) {
        out(data, "lodsw16_r(cpu, ");        
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==lodsw16_op) {
        out(data, "AX = readw(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI); SI+=cpu->df<<1; CYCLES(2);");
    } else if (op->func==lodsw32_r_op) {
        out(data, "lodsw32_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==lodsw32_op) {
        out(data, "AX = readw(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI); ESI+=cpu->df<<1; CYCLES(2);");
    } else {
        kpanic("gen0ad");
    }    
}

void OPCALL lodsd16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsd16_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL lodsd32_op(struct CPU* cpu, struct Op* op);
void gen2ad(struct GenData* data, struct Op* op) {
    if (op->func==lodsd16_r_op) {
        out(data, "lodsd16_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==lodsd16_op) {
        out(data, "EAX = readd(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+SI); SI+=cpu->df<<2; CYCLES(2);");
    } else if (op->func==lodsd32_r_op) {
        out(data, "lodsd32_r(cpu, ");
        out(data, getBase(op->base));
        out(data, ");");
    } else if (op->func==lodsd32_op) {
        out(data, "EAX = readd(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "]+ESI); ESI+=cpu->df<<2; CYCLES(2);");
    } else {
        kpanic("gen2ad");
    }    
}

void OPCALL scasb16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL scasb16_op(struct CPU* cpu, struct Op* op);
void OPCALL scasb32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL scasb32_op(struct CPU* cpu, struct Op* op);
void gen0ae(struct GenData* data, struct Op* op) {
    if (op->func==scasb16_r_op) {
        out(data, "scasb16_r(cpu, ");        
        out(data, getBase(op->data1));
        out(data, ");");
    } else if (op->func==scasb16_op) {
        out(data, "cpu->dst.u8 = AL; cpu->src.u8 = readb(cpu->thread,  cpu->segAddress[ES]+DI); cpu->result.u8 = cpu->dst.u8 - cpu->src.u8; cpu->lazyFlags = FLAGS_SUB8; DI+=cpu->df; CYCLES(4);");
    } else if (op->func==scasb32_r_op) {
        out(data, "scasb32_r(cpu, ");
        out(data, getBase(op->data1));
        out(data, ");");
    } else if (op->func==scasb32_op) {
        out(data, "cpu->dst.u8 = AL; cpu->src.u8 = readb(cpu->thread,  cpu->segAddress[ES]+EDI); cpu->result.u8 = cpu->dst.u8 - cpu->src.u8; cpu->lazyFlags = FLAGS_SUB8; EDI+=cpu->df; CYCLES(4);");
    } else {
        kpanic("gen0a4");
    }   
    data->lazyFlags = sFLAGS_SUB8;
}

void OPCALL scasw16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL scasw16_op(struct CPU* cpu, struct Op* op);
void OPCALL scasw32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL scasw32_op(struct CPU* cpu, struct Op* op);
void gen0af(struct GenData* data, struct Op* op) {
    if (op->func==scasw16_r_op) {
        out(data, "scasw16_r(cpu, ");        
        out(data, getBase(op->data1));
        out(data, ");");
    } else if (op->func==scasw16_op) {
        out(data, "cpu->dst.u16 = AX; cpu->src.u16 = readw(cpu->thread,  cpu->segAddress[ES]+DI); cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_SUB16; DI+=cpu->df<<1; CYCLES(4);");
    } else if (op->func==scasw32_r_op) {
        out(data, "scasw32_r(cpu, ");
        out(data, getBase(op->data1));
        out(data, ");");
    } else if (op->func==scasw32_op) {
        out(data, "cpu->dst.u16 = AX; cpu->src.u16 = readw(cpu->thread,  cpu->segAddress[ES]+EDI); cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_SUB16; EDI+=cpu->df<<1; CYCLES(4);");
    } else {
        kpanic("gen0a5");
    }   
    data->lazyFlags = sFLAGS_SUB16;
}

void OPCALL scasd16_r_op(struct CPU* cpu, struct Op* op);
void OPCALL scasd16_op(struct CPU* cpu, struct Op* op);
void OPCALL scasd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL scasd32_op(struct CPU* cpu, struct Op* op);
void gen2af(struct GenData* data, struct Op* op) {
    if (op->func==scasd16_r_op) {
        out(data, getBase(op->data1));
        out(data, ");");
    } else if (op->func==scasd16_op) {
        out(data, "cpu->dst.u32 = EAX; cpu->src.u32 = readd(cpu->thread,  cpu->segAddress[ES]+DI); cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_SUB32; DI+=cpu->df<<2; CYCLES(4);");
    } else if (op->func==scasd32_r_op) {
        out(data, getBase(op->data1));
        out(data, ");");
    } else if (op->func==scasd32_op) {
        out(data, "cpu->dst.u32 = EAX; cpu->src.u32 = readd(cpu->thread,  cpu->segAddress[ES]+EDI); cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_SUB32; EDI+=cpu->df<<2; CYCLES(4);");
    } else {
        kpanic("gen2a5");
    }   
    data->lazyFlags = sFLAGS_SUB32;
}


void gen0a8(struct GenData* data, struct Op* op) {
    if (inlineTestJump(data, op, sFLAGS_TEST8, "1"))
        return;
    genArithR(data, "&", "FLAGS_TEST8", "8", r8(0), op->data1, 0, 0, "1");
    data->lazyFlags = sFLAGS_TEST8;
}

void gen0a9(struct GenData* data, struct Op* op) {
    if (inlineTestJump(data, op, sFLAGS_TEST16, "1"))
        return;

    genArithR(data, "&", "FLAGS_TEST16", "16", r16(0), op->data1, 0, 0, "1");
    data->lazyFlags = sFLAGS_TEST16;
}

void gen2a9(struct GenData* data, struct Op* op) {
    if (inlineTestJump(data, op, sFLAGS_TEST32, "1"))
        return;

    genArithR(data, "&", "FLAGS_TEST32", "32", r32(0), op->data1, 0, 0, "1");
    data->lazyFlags = sFLAGS_TEST32;
}

void gen0b0(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    out(data, r8(op->r1));
    out(data, " = 0x");
    out(data, tmp);
    out(data, "; CYCLES(1);");
}

void gen0b8(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    out(data, r16(op->r1));
    out(data, " = 0x");
    out(data, tmp);
    out(data, "; CYCLES(1);");
}

void gen2b8(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    out(data, r32(op->r1));
    out(data, " = 0x");
    out(data, tmp);
    out(data, "; CYCLES(1);");
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
void OPCALL rol8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar8_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen0c0(struct GenData* data, struct Op* op) {
    char reg[16];
    char value[16];
    itoa(op->r1, reg, 10);
    itoa(op->data1, value, 10);

    if (op->func == rol8_reg_op) {
        out(data, "rol8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rol8_mem16_op) {
        out(data, "rol8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rol8_mem32_op) {
        out(data, "rol8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror8_reg_op) {
        out(data, "ror8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror8_mem16_op) {
        out(data, "rol8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror8_mem32_op) {
        out(data, "rol8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl8_reg_op) {
        out(data, "rcl8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl8_mem16_op) {
        out(data, "rcl8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl8_mem32_op) {
        out(data, "rcl8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr8_reg_op) {
        out(data, "rcr8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr8_mem16_op) {
        out(data, "rcl8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr8_mem32_op) {
        out(data, "rcl8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl8_reg_op) {
        data->lazyFlags = sFLAGS_SHL8;
        out(data, "shl8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl8_mem16_op) {
        data->lazyFlags = sFLAGS_SHL8;
        out(data, "shl8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl8_mem32_op) {
        data->lazyFlags = sFLAGS_SHL8;
        out(data, "shl8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr8_reg_op) {
        data->lazyFlags = sFLAGS_SHR8;
        out(data, "shr8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr8_mem16_op) {
        data->lazyFlags = sFLAGS_SHR8;
        out(data, "shr8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr8_mem32_op) {
        data->lazyFlags = sFLAGS_SHR8;
        out(data, "shr8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar8_reg_op) {
        data->lazyFlags = sFLAGS_SAR8;
        out(data, "sar8_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar8_mem16_op) {
        data->lazyFlags = sFLAGS_SAR8;
        out(data, "sar8_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar8_mem32_op) {
        data->lazyFlags = sFLAGS_SAR8;
        out(data, "sar8_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    }

    else if (op->func == rol8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " << ");
        out(data, value);
        out(data, ") | (");
        out(data, r8(op->r1));
        out(data, " >> (8 - ");
        out(data, value);
        out(data, ")); CYCLES(1);");
    } else if (op->func == rol8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << ");
        out(data, value);
        out(data, ") | (tmp8 >> (8 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == rol8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << ");
        out(data, value);
        out(data, ") | (tmp8 >> (8 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == ror8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, ") | (");
        out(data, r8(op->r1));
        out(data, " << (8 - ");
        out(data, value);
        out(data, ")); CYCLES(1);");
    } else if (op->func == ror8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> ");
        out(data, value);
        out(data, ") | (tmp8 << (8 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == ror8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> ");
        out(data, value);
        out(data, ") | (tmp8 << (8 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == rcl8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " << ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(8);");
        } else {
            out(data, ") | (");
            out(data, r8(op->r1));
            out(data, " >> (9 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1)); CYCLES(8);");
        }
    } else if (op->func == rcl8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r8(op->r1));
            out(data, " >> (9 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1)); CYCLES(10);");
        }
    } else if (op->func == rcl8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r8(op->r1));
            out(data, " >> (9 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1)); CYCLES(10);");
        }
    } else if (op->func == rcr8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " >> ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 7); CYCLES(8);");
        } else {
            out(data, ") | (");
            out(data, r8(op->r1));
            out(data, " << (9 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (8 - ");
            out(data, value);
            out(data, ")); CYCLES(8);");
        }
    } else if (op->func == rcr8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 7); CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r8(op->r1));
            out(data, " << (9 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (8 - ");
            out(data, value);
            out(data, ")); CYCLES(10);");
        }       
    } else if (op->func == rcr8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 7); CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r8(op->r1));
            out(data, " << (9 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (8 - ");
            out(data, value);
            out(data, ")); CYCLES(10);");
        } 
    } else if (op->func == shl8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r1));
        out(data, " << ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == shl8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) << ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shl8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) << ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shr8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == shr8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shr8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == sar8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (S8)");
        out(data, r8(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == sar8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, (S8)readb(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == sar8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, (S8)readb(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else {
        kpanic("gen0c0");
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
void OPCALL rol16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar16_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen0c1(struct GenData* data, struct Op* op) {
    char reg[16];
    char value[16];
    itoa(op->r1, reg, 10);
    itoa(op->data1, value, 10);

    if (op->func == rol16_reg_op) {
        out(data, "rol16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rol16_mem16_op) {
        out(data, "rol16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rol16_mem32_op) {
        out(data, "rol16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror16_reg_op) {
        out(data, "ror16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror16_mem16_op) {
        out(data, "rol16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror16_mem32_op) {
        out(data, "rol16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl16_reg_op) {
        out(data, "rcl16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl16_mem16_op) {
        out(data, "rcl16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl16_mem32_op) {
        out(data, "rcl16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr16_reg_op) {
        out(data, "rcr16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr16_mem16_op) {
        out(data, "rcl16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr16_mem32_op) {
        out(data, "rcl16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl16_reg_op) {
        data->lazyFlags = sFLAGS_SHL16;
        out(data, "shl16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl16_mem16_op) {
        data->lazyFlags = sFLAGS_SHL16;
        out(data, "shl16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl16_mem32_op) {
        data->lazyFlags = sFLAGS_SHL16;
        out(data, "shl16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr16_reg_op) {
        data->lazyFlags = sFLAGS_SHR16;
        out(data, "shr16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr16_mem16_op) {
        data->lazyFlags = sFLAGS_SHR16;
        out(data, "shr16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr16_mem32_op) {
        data->lazyFlags = sFLAGS_SHL16;
        out(data, "shr16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar16_reg_op) {
        data->lazyFlags = sFLAGS_SAR16;
        out(data, "sar16_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar16_mem16_op) {
        data->lazyFlags = sFLAGS_SAR16;
        out(data, "sar16_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar16_mem32_op) {
        data->lazyFlags = sFLAGS_SAR16;
        out(data, "sar16_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    }

    else if (op->func == rol16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " << ");
        out(data, value);
        out(data, ") | (");
        out(data, r16(op->r1));
        out(data, " >> (16 - ");
        out(data, value);
        out(data, ")); CYCLES(1);");
    } else if (op->func == rol16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << ");
        out(data, value);
        out(data, ") | (tmp16 >> (16 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == rol16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << ");
        out(data, value);
        out(data, ") | (tmp16 >> (16 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == ror16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, ") | (");
        out(data, r16(op->r1));
        out(data, " << (16 - ");
        out(data, value);
        out(data, ")); CYCLES(1);");
    } else if (op->func == ror16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> ");
        out(data, value);
        out(data, ") | (tmp16 << (16 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == ror16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> ");
        out(data, value);
        out(data, ") | (tmp16 << (16 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == rcl16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " << ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(8);");
        } else {
            out(data, ") | (");
            out(data, r16(op->r1));
            out(data, " >> (17 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1)); CYCLES(8);");
        }
    } else if (op->func == rcl16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r16(op->r1));
            out(data, " >> (17 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1)); CYCLES(10);");
        }
    } else if (op->func == rcl16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r16(op->r1));
            out(data, " >> (17 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1)); CYCLES(10);");
        }
    } else if (op->func == rcr16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " >> ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 15); CYCLES(8);");
        } else {
            out(data, ") | (");
            out(data, r16(op->r1));
            out(data, " << (17 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (16 - ");
            out(data, value);
            out(data, ")); CYCLES(8);");
        }
    } else if (op->func == rcr16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 15); CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r16(op->r1));
            out(data, " << (17 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (16 - ");
            out(data, value);
            out(data, ")); CYCLES(10);");
        }       
    } else if (op->func == rcr16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> ");
        out(data, value);
        if (op->data1 == 1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 15); CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r16(op->r1));
            out(data, " << (17 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (16 - ");
            out(data, value);
            out(data, ")); CYCLES(10);");
        }
    } else if (op->func == shl16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " << ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == shl16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) << ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shl16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) << ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shr16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == shr16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shr16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == sar16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (S16)");
        out(data, r16(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == sar16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, (S16)readw(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == sar16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, (S16)readw(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else {
        kpanic("gen0c1");
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
void OPCALL rol32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar32_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen2c1(struct GenData* data, struct Op* op) {
    char reg[16];
    char value[16];
    itoa(op->r1, reg, 10);
    itoa(op->data1, value, 10);

    if (op->func == rol32_reg_op) {
        out(data, "rol32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rol32_mem16_op) {
        out(data, "rol32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rol32_mem32_op) {
        out(data, "rol32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror32_reg_op) {
        out(data, "ror32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror32_mem16_op) {
        out(data, "rol32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == ror32_mem32_op) {
        out(data, "rol32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl32_reg_op) {
        out(data, "rcl32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl32_mem16_op) {
        out(data, "rcl32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcl32_mem32_op) {
        out(data, "rcl32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr32_reg_op) {
        out(data, "rcr32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr32_mem16_op) {
        out(data, "rcl32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == rcr32_mem32_op) {
        out(data, "rcl32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl32_reg_op) {
        data->lazyFlags = sFLAGS_SHL32;
        out(data, "shl32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl32_mem16_op) {
        data->lazyFlags = sFLAGS_SHL32;
        out(data, "shl32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shl32_mem32_op) {
        data->lazyFlags = sFLAGS_SHL32;
        out(data, "shl32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr32_reg_op) {
        data->lazyFlags = sFLAGS_SHR32;
        out(data, "shr32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr32_mem16_op) {
        data->lazyFlags = sFLAGS_SHR32;
        out(data, "shr32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == shr32_mem32_op) {
        data->lazyFlags = sFLAGS_SHR32;
        out(data, "shr32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar32_reg_op) {
        data->lazyFlags = sFLAGS_SAR32;
        out(data, "sar32_reg(cpu, ");
        out(data, reg);
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar32_mem16_op) {
        data->lazyFlags = sFLAGS_SAR32;
        out(data, "sar32_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    } else if (op->func == sar32_mem32_op) {
        data->lazyFlags = sFLAGS_SAR32;
        out(data, "sar32_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", ");
        out(data, value);
        out(data, ");");
    }

    else if (op->func == rol32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " << ");
        out(data, value);
        out(data, ") | (");
        out(data, r32(op->r1));
        out(data, " >> (32 - ");
        out(data, value);
        out(data, ")); CYCLES(1);");
    } else if (op->func == rol32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << ");
        out(data, value);
        out(data, ") | (tmp32 >> (32 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == rol32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << ");
        out(data, value);
        out(data, ") | (tmp32 >> (32 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == ror32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, ") | (");
        out(data, r32(op->r1));
        out(data, " << (32 - ");
        out(data, value);
        out(data, ")); CYCLES(1);");
    } else if (op->func == ror32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> ");
        out(data, value);
        out(data, ") | (tmp32 << (32 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == ror32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> ");
        out(data, value);
        out(data, ") | (tmp32 << (32 - ");
        out(data, value);
        out(data, ")); CYCLES(3);");
    } else if (op->func == rcl32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " << ");
        out(data, value);
        if (op->data1==1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(8);");
        } else {
            out(data, ") | (");
            out(data, r32(op->r1));
            out(data, " >> (33 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1))); CYCLES(8);");
        }
    } else if (op->func == rcl32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << ");
        out(data, value);
        if (op->data1==1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r32(op->r1));
            out(data, " >> (33 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1))); CYCLES(10);");
        }
    } else if (op->func == rcl32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << ");
        out(data, value);
        if (op->data1==1) {
            out(data, ") | ");
            out(data, getFlag(data, CF));
            out(data, "; CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r32(op->r1));
            out(data, " >> (33 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (");
            out(data, value);
            out(data, " - 1))); CYCLES(10);");
        }
    } else if (op->func == rcr32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " >> ");
        out(data, value);
        if (op->data1==1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 31); CYCLES(8);");
        } else {
            out(data, ") | (");
            out(data, r32(op->r1));
            out(data, " << (33 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (32 - ");
            out(data, value);
            out(data, ")); CYCLES(8);");
        }
    } else if (op->func == rcr32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> ");
        out(data, value);
        if (op->data1==1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 31); CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r32(op->r1));
            out(data, " << (33 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (32 - ");
            out(data, value);
            out(data, ")); CYCLES(10);");
        }      
    } else if (op->func == rcr32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> ");
        out(data, value);
        if (op->data1==1) {
            out(data, ") | (");
            out(data, getFlag(data, CF));
            out(data, " << 31); CYCLES(10);");
        } else {
            out(data, ") | (");
            out(data, r32(op->r1));
            out(data, " << (33 - ");
            out(data, value);
            out(data, ")) | (");
            out(data, getFlag(data, CF));
            out(data, " << (32 - ");
            out(data, value);
            out(data, ")); CYCLES(10);");
        }
    } else if (op->func == shl32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " << ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == shl32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) << ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shl32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) << ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shr32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == shr32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == shr32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == sar32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (S32)");
        out(data, r32(op->r1));
        out(data, " >> ");
        out(data, value);
        out(data, "; CYCLES(1);");
    } else if (op->func == sar32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, (S32)readd(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else if (op->func == sar32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, (S32)readd(cpu->thread, eaa) >> ");
        out(data, value);
        out(data, "); CYCLES(3);"); 
    } else {
        kpanic("gen2c1");
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
void OPCALL rol8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar8cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar8cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar8cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen0d2(struct GenData* data, struct Op* op) {
    char reg[16];
    itoa(op->r1, reg, 10);

    if (op->func == rol8cl_reg_op) {
        out(data, "rol8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rol8cl_mem16_op) {
        out(data, "rol8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rol8cl_mem32_op) {
        out(data, "rol8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror8cl_reg_op) {
        out(data, "ror8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror8cl_mem16_op) {
        out(data, "rol8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror8cl_mem32_op) {
        out(data, "rol8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl8cl_reg_op) {
        out(data, "rcl8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl8cl_mem16_op) {
        out(data, "rcl8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl8cl_mem32_op) {
        out(data, "rcl8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr8cl_reg_op) {
        out(data, "rcr8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr8cl_mem16_op) {
        out(data, "rcl8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr8cl_mem32_op) {
        out(data, "rcl8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl8cl_reg_op) {
        out(data, "shl8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl8cl_mem16_op) {
        out(data, "shl8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl8cl_mem32_op) {
        out(data, "shl8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr8cl_reg_op) {
        out(data, "shr8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr8cl_mem16_op) {
        out(data, "shr8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr8cl_mem32_op) {
        out(data, "shr8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar8cl_reg_op) {
        out(data, "sar8cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar8cl_mem16_op) {
        out(data, "sar8cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar8cl_mem32_op) {
        out(data, "sar8cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    }

    else if (op->func == rol8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " << (CL & 0x1F) | (");
        out(data, r8(op->r1));
        out(data, " >> (8 - (CL & 7))); CYCLES(1);");
    } else if (op->func == rol8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << (CL & 7)) | (tmp8 >> (8 - (CL & 7))); CYCLES(3);");
    } else if (op->func == rol8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << (CL & 7)) | (tmp8 >> (8 - (CL & 7))); CYCLES(3);");
    } else if (op->func == ror8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " >> (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " << (8 - (CL & 7))); CYCLES(1);");
    } else if (op->func == ror8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> (CL & 7)) | (tmp8 << (8 - (CL & 7))); CYCLES(3);");
    } else if (op->func == ror8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> (CL & 7)) | (tmp8 << (8 - (CL & 7))); CYCLES(3);");
    } else if (op->func == rcl8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " << (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " >> (9 - (CL & 7))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 7) - 1)); CYCLES(8);");
    } else if (op->func == rcl8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " >> (9 - (CL & 7))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 7) - 1))); CYCLES(10);");
    } else if (op->func == rcl8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 << (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " >> (9 - (CL & 7))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 7) - 1))); CYCLES(10);");
    } else if (op->func == rcr8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (");
        out(data, r8(op->r1));
        out(data, " >> (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " << (9 - (CL & 7))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (8 - (CL & 7))); CYCLES(8);");
    } else if (op->func == rcr8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " << (9 - (CL & 7))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (8 - (CL & 7)))); CYCLES(10);");        
    } else if (op->func == rcr8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp8 = readb(cpu->thread, eaa); writeb(cpu->thread, eaa, (tmp8 >> (CL & 7)) | (");
        out(data, r8(op->r1));
        out(data, " << (9 - (CL & 7))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (8 - (CL & 7)))); CYCLES(10);"); 
    } else if (op->func == shl8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r1));
        out(data, " << (CL & 7); CYCLES(1);");
    } else if (op->func == shl8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) << (CL & 7)); CYCLES(3);"); 
    } else if (op->func == shl8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) << (CL & 7)); CYCLES(3);"); 
    } else if (op->func == shr8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r1));
        out(data, " >> (CL & 7); CYCLES(1);");
    } else if (op->func == shr8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) >> (CL & 7)); CYCLES(3);"); 
    } else if (op->func == shr8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) >> (CL & 7)); CYCLES(3);"); 
    } else if (op->func == sar8cl_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = (S8)");
        out(data, r8(op->r1));
        out(data, " >> (CL & 7); CYCLES(1);");
    } else if (op->func == sar8cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, (S8)readb(cpu->thread, eaa) >> (CL & 7)); CYCLES(3);"); 
    } else if (op->func == sar8cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, (S8)readb(cpu->thread, eaa) >> (CL & 7)); CYCLES(3);"); 
    } else {
        kpanic("gen0d2");
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
void OPCALL rol16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar16cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar16cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar16cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen0d3(struct GenData* data, struct Op* op) {
    char reg[16];
    itoa(op->r1, reg, 10);

    if (op->func == rol16cl_reg_op) {
        out(data, "rol16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rol16cl_mem16_op) {
        out(data, "rol16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rol16cl_mem32_op) {
        out(data, "rol16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror16cl_reg_op) {
        out(data, "ror16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror16cl_mem16_op) {
        out(data, "rol16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror16cl_mem32_op) {
        out(data, "rol16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl16cl_reg_op) {
        out(data, "rcl16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl16cl_mem16_op) {
        out(data, "rcl16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl16cl_mem32_op) {
        out(data, "rcl16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr16cl_reg_op) {
        out(data, "rcr16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr16cl_mem16_op) {
        out(data, "rcl16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr16cl_mem32_op) {
        out(data, "rcl16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl16cl_reg_op) {
        out(data, "shl16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl16cl_mem16_op) {
        out(data, "shl16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl16cl_mem32_op) {
        out(data, "shl16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr16cl_reg_op) {
        out(data, "shr16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr16cl_mem16_op) {
        out(data, "shr16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr16cl_mem32_op) {
        out(data, "shr16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar16cl_reg_op) {
        out(data, "sar16cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar16cl_mem16_op) {
        out(data, "sar16cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar16cl_mem32_op) {
        out(data, "sar16cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    }

    else if (op->func == rol16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " << (CL & 0x1F) | (");
        out(data, r16(op->r1));
        out(data, " >> (16 - (CL & 15))); CYCLES(1);");
    } else if (op->func == rol16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << (CL & 15)) | (tmp16 >> (16 - (CL & 15))); CYCLES(3);");
    } else if (op->func == rol16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << (CL & 15)) | (tmp16 >> (16 - (CL & 15))); CYCLES(3);");
    } else if (op->func == ror16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " >> (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " << (16 - (CL & 15))); CYCLES(1);");
    } else if (op->func == ror16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> (CL & 15)) | (tmp16 << (16 - (CL & 15))); CYCLES(3);");
    } else if (op->func == ror16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> (CL & 15)) | (tmp16 << (16 - (CL & 15))); CYCLES(3);");
    } else if (op->func == rcl16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " << (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " >> (17 - (CL & 15))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 15) - 1)); CYCLES(8);");
    } else if (op->func == rcl16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " >> (17 - (CL & 15))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 15) - 1))); CYCLES(10);");
    } else if (op->func == rcl16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 << (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " >> (17 - (CL & 15))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 15) - 1))); CYCLES(10);");
    } else if (op->func == rcr16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (");
        out(data, r16(op->r1));
        out(data, " >> (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " << (17 - (CL & 15))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (16 - (CL & 15))); CYCLES(8);");
    } else if (op->func == rcr16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " << (17 - (CL & 15))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (16 - (CL & 15)))); CYCLES(10);");        
    } else if (op->func == rcr16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp16 = readw(cpu->thread, eaa); writew(cpu->thread, eaa, (tmp16 >> (CL & 15)) | (");
        out(data, r16(op->r1));
        out(data, " << (17 - (CL & 15))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (16 - (CL & 15)))); CYCLES(10);"); 
    } else if (op->func == shl16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " << (CL & 15); CYCLES(1);");
    } else if (op->func == shl16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) << (CL & 15)); CYCLES(3);"); 
    } else if (op->func == shl16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) << (CL & 15)); CYCLES(3);"); 
    } else if (op->func == shr16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " >> (CL & 15); CYCLES(1);");
    } else if (op->func == shr16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) >> (CL & 15)); CYCLES(3);"); 
    } else if (op->func == shr16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) >> (CL & 15)); CYCLES(3);"); 
    } else if (op->func == sar16cl_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = (S16)");
        out(data, r16(op->r1));
        out(data, " >> (CL & 15); CYCLES(1);");
    } else if (op->func == sar16cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, (S16)readw(cpu->thread, eaa) >> (CL & 15)); CYCLES(3);"); 
    } else if (op->func == sar16cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, (S16)readw(cpu->thread, eaa) >> (CL & 15)); CYCLES(3);"); 
    } else {
        kpanic("gen0d3");
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
void OPCALL rol32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rol32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL ror32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcl32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL rcr32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shl32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL shr32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar32cl_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar32cl_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL sar32cl_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen2d3(struct GenData* data, struct Op* op) {
    char reg[16];
    itoa(op->r1, reg, 10);

    if (op->func == rol32cl_reg_op) {
        out(data, "rol32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rol32cl_mem16_op) {
        out(data, "rol32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rol32cl_mem32_op) {
        out(data, "rol32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror32cl_reg_op) {
        out(data, "ror32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror32cl_mem16_op) {
        out(data, "rol32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == ror32cl_mem32_op) {
        out(data, "rol32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl32cl_reg_op) {
        out(data, "rcl32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl32cl_mem16_op) {
        out(data, "rcl32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcl32cl_mem32_op) {
        out(data, "rcl32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr32cl_reg_op) {
        out(data, "rcr32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr32cl_mem16_op) {
        out(data, "rcl32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == rcr32cl_mem32_op) {
        out(data, "rcl32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl32cl_reg_op) {
        out(data, "shl32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl32cl_mem16_op) {
        out(data, "shl32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shl32cl_mem32_op) {
        out(data, "shl32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr32cl_reg_op) {
        out(data, "shr32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr32cl_mem16_op) {
        out(data, "shr32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == shr32cl_mem32_op) {
        out(data, "shr32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar32cl_reg_op) {
        out(data, "sar32cl_reg(cpu, ");
        out(data, reg);
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar32cl_mem16_op) {
        out(data, "sar32cl_mem16(cpu, ");
        out(data, getEaa16(op));
        out(data, ", CL & 0x1F);");
    } else if (op->func == sar32cl_mem32_op) {
        out(data, "sar32cl_mem32(cpu, ");
        out(data, getEaa32(op));
        out(data, ", CL & 0x1F);");
    }

    else if (op->func == rol32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " << (CL & 0x1F)) | (");
        out(data, r32(op->r1));
        out(data, " >> (32 - (CL & 31))); CYCLES(1);");
    } else if (op->func == rol32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << (CL & 31)) | (tmp32 >> (32 - (CL & 31))); CYCLES(3);");
    } else if (op->func == rol32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << (CL & 31)) | (tmp32 >> (32 - (CL & 31))); CYCLES(3);");
    } else if (op->func == ror32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " >> (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " << (32 - (CL & 31))); CYCLES(1);");
    } else if (op->func == ror32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> (CL & 31)) | (tmp32 << (32 - (CL & 31))); CYCLES(3);");
    } else if (op->func == ror32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> (CL & 31)) | (tmp32 << (32 - (CL & 31))); CYCLES(3);");
    } else if (op->func == rcl32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " << (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " >> (33 - (CL & 31))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 31) - 1)); CYCLES(8);");
    } else if (op->func == rcl32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " >> (33 - (CL & 31))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 31) - 1))); CYCLES(10);");
    } else if (op->func == rcl32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 << (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " >> (33 - (CL & 31))) | (");
        out(data, getFlag(data, CF));
        out(data, " << ((CL & 31) - 1))); CYCLES(10);");
    } else if (op->func == rcr32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " >> (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " << (33 - (CL & 31))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (32 - (CL & 31))); CYCLES(8);");
    } else if (op->func == rcr32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " << (33 - (CL & 31))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (32 - (CL & 31)))); CYCLES(10);");        
    } else if (op->func == rcr32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, (tmp32 >> (CL & 31)) | (");
        out(data, r32(op->r1));
        out(data, " << (33 - (CL & 31))) | (");
        out(data, getFlag(data, CF));
        out(data, " << (32 - (CL & 31)))); CYCLES(10);"); 
    } else if (op->func == shl32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " << (CL & 31); CYCLES(1);");
    } else if (op->func == shl32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) << (CL & 31)); CYCLES(3);"); 
    } else if (op->func == shl32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) << (CL & 31)); CYCLES(3);"); 
    } else if (op->func == shr32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " >> (CL & 31); CYCLES(1);");
    } else if (op->func == shr32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) >> (CL & 31)); CYCLES(3);"); 
    } else if (op->func == shr32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) >> (CL & 31)); CYCLES(3);"); 
    } else if (op->func == sar32cl_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = (S32)");
        out(data, r32(op->r1));
        out(data, " >> (CL & 31); CYCLES(1);");
    } else if (op->func == sar32cl_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, (S32)readd(cpu->thread, eaa) >> (CL & 31)); CYCLES(3);"); 
    } else if (op->func == sar32cl_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, (S32)readd(cpu->thread, eaa) >> (CL & 31)); CYCLES(3);"); 
    } else {
        kpanic("gen2d3");
    }
}

void gen0c2(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);

    out(data, "cpu->eip.u32 = pop16(cpu); SP = SP + 0x");
    out(data, tmp);
    out(data, "; CYCLES(3); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
}

void gen2c2(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);

    out(data, "cpu->eip.u32 = pop32(cpu); ESP = ESP + 0x");
    out(data, tmp);
    out(data, "; CYCLES(3); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
}

void gen0c3(struct GenData* data, struct Op* op) {
    out(data, "cpu->eip.u32 = pop16(cpu); CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
}

void gen2c3(struct GenData* data, struct Op* op) {
    out(data, "cpu->eip.u32 = pop32(cpu); CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
}

void OPCALL mov8_reg(struct CPU* cpu, struct Op* op);
void OPCALL mov8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mov8_mem32(struct CPU* cpu, struct Op* op);
void gen0c6(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);

    if (op->func == mov8_reg) {
        out(data, r8(op->r1));
        out(data, " = 0x");
        out(data, tmp);
        out(data, "; CYCLES(1);");
    } else {
        out(data, "writeb(cpu->thread, ");
        if (op->func == mov8_mem16) {
            out(data, getEaa16(op));
        } else if (op->func == mov8_mem32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen0c6");
        }
        out(data, ", 0x");
        out(data, tmp);
        out(data, "); CYCLES(1);");
    }
}

void OPCALL mov16_reg(struct CPU* cpu, struct Op* op);
void OPCALL mov16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mov16_mem32(struct CPU* cpu, struct Op* op);
void gen0c7(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);

    if (op->func == mov16_reg) {
        out(data, r16(op->r1));
        out(data, " = 0x");
        out(data, tmp);
        out(data, "; CYCLES(1);");
    } else {
        out(data, "writew(cpu->thread, ");
        if (op->func == mov16_mem16) {
            out(data, getEaa16(op));
        } else if (op->func == mov16_mem32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen0c7");
        }
        out(data, ", 0x");
        out(data, tmp);
        out(data, "); CYCLES(1);");
    }
}

void OPCALL mov32_reg(struct CPU* cpu, struct Op* op);
void OPCALL mov32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mov32_mem32(struct CPU* cpu, struct Op* op);
void gen2c7(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);

    if (op->func == mov32_reg) {
        out(data, r32(op->r1));
        out(data, " = 0x");
        out(data, tmp);
        out(data, "; CYCLES(1);");
    } else {
        out(data, "writed(cpu->thread, ");
        if (op->func == mov32_mem16) {
            out(data, getEaa16(op));
        } else if (op->func == mov32_mem32) {
            out(data, getEaa32(op));
        } else {
            kpanic("gen2c7");
        }
        out(data, ", 0x");
        out(data, tmp);
        out(data, "); CYCLES(1);");
    }
}

void gen0c8(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "cpu_enter16(cpu, ");
    itoa(op->data1, tmp, 10);
    out(data, tmp);
    out(data, ", ");
    itoa(op->r1, tmp, 10);
    out(data, tmp);
    out(data, "); CYCLES(15);");
}

void gen2c8(struct GenData* data, struct Op* op) {
    char tmp[16];

    out(data, "cpu_enter32(cpu, ");
    itoa(op->data1, tmp, 10);
    out(data, tmp);
    out(data, ", ");
    itoa(op->r1, tmp, 10);
    out(data, tmp);
    out(data, "); CYCLES(15);");
}

void gen0c9(struct GenData* data, struct Op* op) {
    out(data, "SP = BP; BP = pop16(cpu); CYCLES(3);");
}

void gen2c9(struct GenData* data, struct Op* op) {
    out(data, "ESP = EBP; EBP = pop32(cpu); CYCLES(3);");
}

void gen0cb(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->eipCount, tmp, 10);
    out(data, "fillFlags(cpu); cpu->eip.u32+=");
    data->lazyFlags = sFLAGS_NONE;
    out(data, tmp);
    out(data, "; cpu_ret(cpu, 0, 0, cpu->eip.u32); CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
}

void OPCALL syscall_op(struct CPU* cpu, struct Op* op);
void OPCALL int99(struct CPU* cpu, struct Op* op);
void gen0cd(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->eipCount, tmp, 10);
    if (op->func == syscall_op) {        
        out(data, "ksyscall(cpu, ");
        out(data, tmp);
        out(data, ");");
        // syscall will set nextBlock
    } else if (op->func == int99) {
        out(data, "int99Callback[peek32(cpu, 0)](cpu);");
    } else {
        kpanic("gen0cd");
    }
}

void gen0d4(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    out(data, "instruction_aam(cpu, ");
    out(data, tmp);
    out(data, "); CYCLES(18);");
}

void gen0d5(struct GenData* data, struct Op* op) {
    char tmp[16];

    itoa(op->data1, tmp, 16);
    out(data, "instruction_aad(cpu, ");
    out(data, tmp);
    out(data, "); CYCLES(10);");
}

void gen0d6(struct GenData* data, struct Op* op) {
    out(data, "if (");
    out(data, getFlag(data, CF));
    out(data, ") AL = 0xFF; else AL = 0; CYCLES(2);");
}

void OPCALL xlat16(struct CPU* cpu, struct Op* op);
void OPCALL xlat32(struct CPU* cpu, struct Op* op);
void gen0d7(struct GenData* data, struct Op* op) {
    if (op->func == xlat16) {
        out(data, "AL = readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "] + (U16)(BX + AL)); CYCLES(4);");
    } else if (op->func == xlat32) {
        out(data, "AL = readb(cpu->thread, cpu->segAddress[");
        out(data, getBase(op->base));
        out(data, "] + EBX + AL); CYCLES(4);");
    } else {
        kpanic("gen0d7");
    }
}

void OPCALL FADD_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_STi(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FADD_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_SINGLE_REAL_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FADD_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_SINGLE_REAL_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void gen0d8(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);
    if (op->func == FADD_ST0_STj) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d += cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d; CYCLES(1);");
    } else if (op->func == FMUL_ST0_STj) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d *= cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d; CYCLES(1);");
    } else if (op->func == FCOM_STi) {
        out(data, "FPU_FCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "));CYCLES(1);");
    } else if (op->func == FCOM_STi_Pop) {
        out(data, "FPU_FCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "));FPU_FPOP(cpu);CYCLES(1);");
    } else if (op->func == FSUB_ST0_STj) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d -= cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d; CYCLES(1);");
    } else if (op->func == FSUBR_ST0_STj) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(1);");
    } else if (op->func == FDIV_ST0_STj) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d /= cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d; CYCLES(39);");
    } else if (op->func == FDIVR_ST0_STj) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FADD_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d += f2i.f; CYCLES(1);");
    } else if (op->func == FMUL_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d *= f2i.f; CYCLES(1);");
    } else if (op->func == FCOM_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[8].d = f2i.f; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(1);");
    } else if (op->func == FCOM_SINGLE_REAL_32_Pop) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[8].d = f2i.f; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FSUB_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d -= f2i.f; CYCLES(1);");
    } else if (op->func == FSUBR_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = f2i.f - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FDIV_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d /= f2i.f; CYCLES(1);");
    } else if (op->func == FDIVR_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = f2i.f / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FADD_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d += f2i.f; CYCLES(1);");
    } else if (op->func == FMUL_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d *= f2i.f; CYCLES(1);");
    } else if (op->func == FCOM_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[8].d = f2i.f; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(1);");
    } else if (op->func == FCOM_SINGLE_REAL_16_Pop) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[8].d = f2i.f; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FSUB_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d -= f2i.f; CYCLES(1);");
    } else if (op->func == FSUBR_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = f2i.f - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FDIV_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d /= f2i.f; CYCLES(1);");
    } else if (op->func == FDIVR_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = f2i.f / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else {
        kpanic("gen0d8");
    }
}

void OPCALL FLD_STi(struct CPU* cpu, struct Op* op);
void OPCALL FXCH_STi(struct CPU* cpu, struct Op* op);
void OPCALL FNOP(struct CPU* cpu, struct Op* op);
void OPCALL FST_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FCHS(struct CPU* cpu, struct Op* op);
void OPCALL FABS(struct CPU* cpu, struct Op* op);
void OPCALL FTST(struct CPU* cpu, struct Op* op);
void OPCALL FXAM(struct CPU* cpu, struct Op* op);
void OPCALL FLD1(struct CPU* cpu, struct Op* op);
void OPCALL FLDL2T(struct CPU* cpu, struct Op* op);
void OPCALL FLDL2E(struct CPU* cpu, struct Op* op);
void OPCALL FLDPI(struct CPU* cpu, struct Op* op);
void OPCALL FLDLG2(struct CPU* cpu, struct Op* op);
void OPCALL FLDLN2(struct CPU* cpu, struct Op* op);
void OPCALL FLDZ(struct CPU* cpu, struct Op* op);
void OPCALL F2XM1(struct CPU* cpu, struct Op* op);
void OPCALL FYL2X(struct CPU* cpu, struct Op* op);
void OPCALL FPTAN(struct CPU* cpu, struct Op* op);
void OPCALL FPATAN(struct CPU* cpu, struct Op* op);
void OPCALL FXTRACT(struct CPU* cpu, struct Op* op);
void OPCALL FPREM_nearest(struct CPU* cpu, struct Op* op);
void OPCALL FDECSTP(struct CPU* cpu, struct Op* op);
void OPCALL FINCSTP(struct CPU* cpu, struct Op* op);
void OPCALL FPREM(struct CPU* cpu, struct Op* op);
void OPCALL FYL2XP1(struct CPU* cpu, struct Op* op);
void OPCALL FSQRT(struct CPU* cpu, struct Op* op);
void OPCALL FSINCOS(struct CPU* cpu, struct Op* op);
void OPCALL FRNDINT(struct CPU* cpu, struct Op* op);
void OPCALL FSCALE(struct CPU* cpu, struct Op* op);
void OPCALL FSIN(struct CPU* cpu, struct Op* op);
void OPCALL FCOS(struct CPU* cpu, struct Op* op);
void OPCALL FLD_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FST_SINGLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FST_SINGLE_REAL_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FLDENV_16(struct CPU* cpu, struct Op* op);
void OPCALL FLDCW_16(struct CPU* cpu, struct Op* op);
void OPCALL FNSTENV_16(struct CPU* cpu, struct Op* op);
void OPCALL FNSTCW_16(struct CPU* cpu, struct Op* op);
void OPCALL FLD_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FST_SINGLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FST_SINGLE_REAL_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FLDENV_32(struct CPU* cpu, struct Op* op);
void OPCALL FLDCW_32(struct CPU* cpu, struct Op* op);
void OPCALL FNSTENV_32(struct CPU* cpu, struct Op* op);
void OPCALL FNSTCW_32(struct CPU* cpu, struct Op* op);
void gen0d9(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FLD_STi) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.top = (cpu->fpu.top - 1) & 7; cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d; CYCLES(1);");
    } else if (op->func == FXCH_STi) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); tmp32_2 = cpu->fpu.tags[tmp32]; tmpd = cpu->fpu.regs[tmp32].d; cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d; cpu->fpu.tags[cpu->fpu.top] = tmp32_2; cpu->fpu.regs[cpu->fpu.top].d = tmpd; CYCLES(1);");
    } else if (op->func == FNOP) {
        out(data, "CYCLES(1);");
    } else if (op->func == FST_STi_Pop) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d;FPU_FPOP(cpu);CYCLES(1);");
    } else if (op->func == FCHS) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = -1.0 * (cpu->fpu.regs[cpu->fpu.top].d);CYCLES(1);");
    } else if (op->func == FABS) {
        // don't inline because of calling convention of fabs
        out(data, "FPU_FABS(&cpu->fpu);CYCLES(1);");
    } else if (op->func == FTST) {
        out(data, "cpu->fpu.regs[8].d = 0.0;FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8);CYCLES(1);");
    } else if (op->func == FXAM) {
        out(data, "FPU_FXAM(&cpu->fpu); CYCLES(21);");
    } else if (op->func == FLD1) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = 1.0; CYCLES(2);");
    } else if (op->func == FLDL2T) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = 3.3219280948873623; CYCLES(3);");
    } else if (op->func == FLDL2E) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = 1.4426950408889634; CYCLES(3);");
    } else if (op->func == FLDPI) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = 3.14159265358979323846; CYCLES(3);");
    } else if (op->func == FLDLG2) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = 0.3010299956639812; CYCLES(3);");
    } else if (op->func == FLDLN2) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = 0.69314718055994531; CYCLES(3);");
    } else if (op->func == FLDZ) {
        out(data, "cpu->fpu.top = (cpu->fpu.top - 1) & 7; cpu->fpu.regs[cpu->fpu.top].d = 0.0; cpu->fpu.tags[cpu->fpu.top] = TAG_Zero; CYCLES(2);");
    } else if (op->func == F2XM1) {
        out(data, "FPU_F2XM1(&cpu->fpu); CYCLES(13);");
    } else if (op->func == FYL2X) {
        out(data, "FPU_FYL2X(&cpu->fpu); CYCLES(22);");
    } else if (op->func == FPTAN) {
        out(data, "FPU_FPTAN(&cpu->fpu); CYCLES(17);");
    } else if (op->func == FPATAN) {
        out(data, "FPU_FPATAN(&cpu->fpu); CYCLES(17);");
    } else if (op->func == FXTRACT) {
        out(data, "FPU_FXTRACT(&cpu->fpu); CYCLES(13);");
    } else if (op->func == FPREM_nearest) {
        out(data, "FPU_FPREM1(&cpu->fpu); CYCLES(17);");
    } else if (op->func == FDECSTP) {
        out(data, "cpu->fpu.top = (cpu->fpu.top - 1) & 7; CYCLES(1);");
    } else if (op->func == FINCSTP) {
        out(data, "cpu->fpu.top = (cpu->fpu.top + 1) & 7; CYCLES(1);");
    } else if (op->func == FPREM) {
        out(data, "FPU_FPREM(&cpu->fpu); CYCLES(16);");
    } else if (op->func == FYL2XP1) {
        out(data, "FPU_FYL2XP1(&cpu->fpu);CYCLES(22);");
    } else if (op->func == FSQRT) {
        out(data, "FPU_FSQRT(&cpu->fpu);CYCLES(70);");
    } else if (op->func == FSINCOS) {
        out(data, "FPU_FSINCOS(&cpu->fpu); CYCLES(17);");
    } else if (op->func == FRNDINT) {
        out(data, "FPU_FRNDINT(&cpu->fpu); CYCLES(9);");
    } else if (op->func == FSCALE) {
        out(data, "FPU_FSCALE(&cpu->fpu); CYCLES(20);");
    } else if (op->func == FSIN) {
        out(data, "FPU_FSIN(&cpu->fpu); CYCLES(16);");
    } else if (op->func == FCOS) {
        out(data, "FPU_FCOS(&cpu->fpu); CYCLES(16);");
    } else if (op->func == FLD_SINGLE_REAL_16) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = f2i.f; CYCLES(1);");	    
    } else if (op->func == FST_SINGLE_REAL_16) {
        out(data, "f2i.f = (float)cpu->fpu.regs[cpu->fpu.top].d; writed(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", f2i.i); CYCLES(2);");
    } else if (op->func == FST_SINGLE_REAL_16_Pop) {
        out(data, "f2i.f = (float)cpu->fpu.regs[cpu->fpu.top].d; writed(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", f2i.i); FPU_FPOP(cpu); CYCLES(2);");
    } else if (op->func == FLDENV_16) {
        out(data, "FPU_FLDENV(cpu, "); 
        out(data, getEaa16(op));
        out(data, "); CYCLES(32);");
    } else if (op->func == FLDCW_16) {
        out(data, "FPU_SetCW(&cpu->fpu, readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")); CYCLES(7);");
    } else if (op->func == FNSTENV_16) {
        out(data, "FPU_FSTENV(cpu, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(48);");
    } else if (op->func == FNSTCW_16) {
        out(data, "writew(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", cpu->fpu.cw);CYCLES(2);");
    } else if (op->func == FLD_SINGLE_REAL_32) {
        out(data, "f2i.i = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = f2i.f; CYCLES(1);");	    
    } else if (op->func == FST_SINGLE_REAL_32) {
        out(data, "f2i.f = (float)cpu->fpu.regs[cpu->fpu.top].d; writed(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", f2i.i); CYCLES(2);");
    } else if (op->func == FST_SINGLE_REAL_32_Pop) {
        out(data, "f2i.f = (float)cpu->fpu.regs[cpu->fpu.top].d; writed(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", f2i.i); FPU_FPOP(cpu); CYCLES(2);");
    } else if (op->func == FLDENV_32) {
        out(data, "FPU_FLDENV(cpu, "); 
        out(data, getEaa32(op));
        out(data, "); CYCLES(32);");
    } else if (op->func == FLDCW_32) {
        out(data, "FPU_SetCW(&cpu->fpu, readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")); CYCLES(7);");
    } else if (op->func == FNSTENV_32) {
        out(data, "FPU_FSTENV(cpu, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(48);");
    } else if (op->func == FNSTCW_32) {
        out(data, "writew(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", cpu->fpu.cw);CYCLES(2);");
    } else {
        kpanic("gen0d9");
    }
}

void OPCALL FCMOV_ST0_STj_CF(struct CPU* cpu, struct Op* op);
void OPCALL FCMOV_ST0_STj_ZF(struct CPU* cpu, struct Op* op);
void OPCALL FCMOV_ST0_STj_CF_OR_ZF(struct CPU* cpu, struct Op* op);
void OPCALL FCMOV_ST0_STj_PF(struct CPU* cpu, struct Op* op);
void OPCALL FUCOMPP(struct CPU* cpu, struct Op* op);
void OPCALL FIADD_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIMUL_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_DWORD_INTEGER_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FISUB_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FISUBR_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIDIV_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIDIVR_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIADD_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIMUL_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_DWORD_INTEGER_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FISUB_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FISUBR_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIDIV_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIDIVR_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void gen0da(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FCMOV_ST0_STj_CF) {
        out(data, "if (");
        out(data, getFlag(data, CF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FCMOV_ST0_STj_ZF) {
        out(data, "if (");
        out(data, getFlag(data, ZF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FCMOV_ST0_STj_CF_OR_ZF) {
        out(data, "if (");
        out(data, getFlag(data, CF));
        out(data, " || ");
        out(data, getFlag(data, ZF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FCMOV_ST0_STj_PF) {
        out(data, "if (");
        out(data, getFlag(data, PF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FUCOMPP) {
        out(data, "FPU_FUCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, 1)); FPU_FPOP(cpu); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FIADD_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d += (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FIMUL_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d *= (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FICOM_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[8].d = (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(4);");
    } else if (op->func == FICOM_DWORD_INTEGER_16_Pop) {
        out(data, "cpu->fpu.regs[8].d = (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(4);");
    } else if (op->func == FISUB_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d -= (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FISUBR_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ") - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(4);");
    } else if (op->func == FIDIV_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d /= (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(42);");
    } else if (op->func == FIDIVR_DWORD_INTEGER_16) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ") / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(42);");
    } else if (op->func == FIADD_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d += (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FIMUL_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d *= (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FICOM_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[8].d = (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(4);");
    } else if (op->func == FICOM_DWORD_INTEGER_32_Pop) {
        out(data, "cpu->fpu.regs[8].d = (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(4);");
    } else if (op->func == FISUB_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d -= (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FISUBR_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ") - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(4);");
    } else if (op->func == FIDIV_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d /= (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(42);");
    } else if (op->func == FIDIVR_DWORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ") / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(42);");
    } else {
        kpanic("gen0da");
    }
}

void OPCALL FCMOV_ST0_STj_NCF(struct CPU* cpu, struct Op* op);
void OPCALL FCMOV_ST0_STj_NZF(struct CPU* cpu, struct Op* op);
void OPCALL FCMOV_ST0_STj_NCF_AND_NZF(struct CPU* cpu, struct Op* op);
void OPCALL FCMOV_ST0_STj_NPF(struct CPU* cpu, struct Op* op);
void OPCALL FNCLEX(struct CPU* cpu, struct Op* op);
void OPCALL FNINIT(struct CPU* cpu, struct Op* op);
void OPCALL FUCOMI_ST0_STj(struct CPU* cpu, struct Op* op);
void OPCALL FCOMI_ST0_STj_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FILD_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FISTTP32_16(struct CPU* cpu, struct Op* op);
void OPCALL FIST_DWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIST_DWORD_INTEGER_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FLD_EXTENDED_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FSTP_EXTENDED_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FILD_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FISTTP32_32(struct CPU* cpu, struct Op* op);
void OPCALL FIST_DWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIST_DWORD_INTEGER_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FLD_EXTENDED_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FSTP_EXTENDED_REAL_32(struct CPU* cpu, struct Op* op);
void gen0db(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FCMOV_ST0_STj_NCF) {
        out(data, "if (!");
        out(data, getFlag(data, CF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FCMOV_ST0_STj_NZF) {
        out(data, "if (!");
        out(data, getFlag(data, ZF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FCMOV_ST0_STj_NCF_AND_NZF) {
        out(data, "if (!");
        out(data, getFlag(data, CF));
        out(data, " && !");
        out(data, getFlag(data, ZF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FCMOV_ST0_STj_NPF) {
        out(data, "if (!");
        out(data, getFlag(data, PF));
        out(data, ") {tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[cpu->fpu.top] = cpu->fpu.tags[tmp32]; cpu->fpu.regs[cpu->fpu.top].d = cpu->fpu.regs[tmp32].d;} CYCLES(2);");
    } else if (op->func == FNCLEX) {
        out(data, "cpu->fpu.sw &= 0x7f00; CYCLES(9);");
    } else if (op->func == FNINIT) {
        out(data, "FPU_FINIT(&cpu->fpu); CYCLES(12);");
    } else if (op->func == FUCOMI_ST0_STj) {
        out(data, "FPU_FCOMI(cpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); CYCLES(1);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == FCOMI_ST0_STj_Pop) {
        out(data, "FPU_FCOMI(cpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); FPU_FPOP(cpu); CYCLES(1);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == FILD_DWORD_INTEGER_16) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(1);");
    } else if (op->func == FISTTP32_16) {
        out(data, "writed(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", (S32)cpu->fpu.regs[cpu->fpu.top].d); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FIST_DWORD_INTEGER_16) {
        out(data, "FPU_FST_I32(cpu, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(6);");
    } else if (op->func == FIST_DWORD_INTEGER_16_Pop) {
        out(data, "FPU_FST_I32(cpu, ");
        out(data, getEaa16(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FLD_EXTENDED_REAL_16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = FPU_FLD80(readq(cpu->thread, eaa), readw(cpu->thread, eaa + 8)); CYCLES(3);");
    } else if (op->func == FSTP_EXTENDED_REAL_16) {
        out(data, "FPU_ST80(cpu, ");
        out(data, getEaa16(op));
        out(data, ", cpu->ftp.top); FPU_FPOP(cpu); CYCLES(3);");
    } else if (op->func == FILD_DWORD_INTEGER_32) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(1);");
    } else if (op->func == FISTTP32_32) {
        out(data, "writed(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", (S32)cpu->fpu.regs[cpu->fpu.top].d); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FIST_DWORD_INTEGER_32) {
        out(data, "FPU_FST_I32(cpu, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(6);");
    } else if (op->func == FIST_DWORD_INTEGER_32_Pop) {
        out(data, "FPU_FST_I32(cpu, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FLD_EXTENDED_REAL_32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = FPU_FLD80(readq(cpu->thread, eaa), readw(cpu->thread, eaa + 8)); CYCLES(3);");
    } else if (op->func == FSTP_EXTENDED_REAL_32) {
        out(data, "FPU_ST80(cpu, ");
        out(data, getEaa32(op));
        out(data, ", cpu->ftp.top); FPU_FPOP(cpu); CYCLES(3);");
    } else {
        kpanic("gen0db");
    }
}

void OPCALL FADD_STi_ST0(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_STi_ST0(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_STi(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_STi_ST0(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_STi_ST0(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_STi_ST0(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_STi_ST0(struct CPU* cpu, struct Op* op);
void OPCALL FADD_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_DOUBLE_REAL_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FADD_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_DOUBLE_REAL_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void gen0dc(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FADD_STi_ST0) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d += cpu->fpu.regs[cpu->fpu.top].d; CYCLES(1);");
    } else if (op->func == FMUL_STi_ST0) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d *= cpu->fpu.regs[cpu->fpu.top].d; CYCLES(1);");
    } else if (op->func == FCOM_STi) {
        out(data, "FPU_FCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "));CYCLES(1);");
    } else if (op->func == FCOM_STi_Pop) {
        out(data, "FPU_FCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "));FPU_FPOP(cpu);CYCLES(1);");
    } else if (op->func == FSUBR_STi_ST0) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d - cpu->fpu.regs[tmp32].d; CYCLES(1);");
    } else if (op->func == FSUB_STi_ST0) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d -= cpu->fpu.regs[cpu->fpu.top].d; CYCLES(1);");
    } else if (op->func == FDIVR_STi_ST0) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d / cpu->fpu.regs[tmp32].d; CYCLES(39);");
    } else if (op->func == FDIV_STi_ST0) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d /= cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FADD_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d += d2l.d; CYCLES(1);");
    } else if (op->func == FMUL_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d *= d2l.d; CYCLES(1);");
    } else if (op->func == FCOM_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[8].d = d2l.d; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(1);");
    } else if (op->func == FCOM_DOUBLE_REAL_16_Pop) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[8].d = d2l.d; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FSUB_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d -= d2l.d; CYCLES(1);");
    } else if (op->func == FSUBR_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = d2l.d - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FDIV_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d /= d2l.d; CYCLES(1);");
    } else if (op->func == FDIVR_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = d2l.d / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FADD_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d += d2l.d; CYCLES(1);");
    } else if (op->func == FMUL_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d *= d2l.d; CYCLES(1);");
    } else if (op->func == FCOM_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[8].d = d2l.d; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(1);");
    } else if (op->func == FCOM_DOUBLE_REAL_32_Pop) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[8].d = d2l.d; FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FSUB_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d -= d2l.d; CYCLES(1);");
    } else if (op->func == FSUBR_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = d2l.d - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else if (op->func == FDIV_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d /= d2l.d; CYCLES(1);");
    } else if (op->func == FDIVR_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");cpu->fpu.regs[cpu->fpu.top].d = d2l.d / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(39);");
    } else {
        kpanic("gen0dc");
    }
}

void OPCALL FFREE_STi(struct CPU* cpu, struct Op* op);
void OPCALL FXCH_STi(struct CPU* cpu, struct Op* op);
void OPCALL FST_STi(struct CPU* cpu, struct Op* op);
void OPCALL FST_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FUCOM_STi(struct CPU* cpu, struct Op* op);
void OPCALL FUCOM_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FLD_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FISTTP64_16(struct CPU* cpu, struct Op* op);
void OPCALL FST_DOUBLE_REAL_16(struct CPU* cpu, struct Op* op);
void OPCALL FST_DOUBLE_REAL_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FRSTOR_16(struct CPU* cpu, struct Op* op);
void OPCALL FNSAVE_16(struct CPU* cpu, struct Op* op);
void OPCALL FNSTSW_16(struct CPU* cpu, struct Op* op);
void OPCALL FLD_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FISTTP64_32(struct CPU* cpu, struct Op* op);
void OPCALL FST_DOUBLE_REAL_32(struct CPU* cpu, struct Op* op);
void OPCALL FST_DOUBLE_REAL_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FRSTOR_32(struct CPU* cpu, struct Op* op);
void OPCALL FNSAVE_32(struct CPU* cpu, struct Op* op);
void OPCALL FNSTSW_32(struct CPU* cpu, struct Op* op);
void gen0dd(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FFREE_STi) {
        out(data, "cpu->fpu.tags[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")] = TAG_Empty; CYCLES(1);");
    } else if (op->func == FXCH_STi) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); tmp32_2 = cpu->fpu.tags[tmp32]; tmpd = cpu->fpu.regs[tmp32].d; cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d; cpu->fpu.tags[cpu->fpu.top] = tmp32_2; cpu->fpu.regs[cpu->fpu.top].d = tmpd; CYCLES(1);");
    } else if (op->func == FST_STi) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d;CYCLES(1);");
    } else if (op->func == FST_STi_Pop) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d;FPU_FPOP(cpu);CYCLES(1);");
    } else if (op->func == FUCOM_STi) {
        out(data, "FPU_FUCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); CYCLES(1);");
    } else if (op->func == FUCOM_STi_Pop) {
        out(data, "FPU_FUCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FLD_DOUBLE_REAL_16) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = d2l.d; CYCLES(1);");	    
    } else if (op->func == FISTTP64_16) {
        out(data, "writeq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", (S64)cpu->fpu.regs[cpu->fpu.top].d); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FST_DOUBLE_REAL_16) {
        out(data, "writeq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", cpu->fpu.regs[cpu->fpu.top].l); CYCLES(2);");
    } else if (op->func == FST_DOUBLE_REAL_16_Pop) {
        out(data, "writeq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", cpu->fpu.regs[cpu->fpu.top].l); FPU_FPOP(cpu); CYCLES(2);");
    } else if (op->func == FRSTOR_16) {
        out(data, "FPU_FRSTOR(cpu, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(75);");
    } else if (op->func == FNSAVE_16) {
        out(data, "FPU_FSAVE(cpu, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(127);");
    } else if (op->func == FNSTSW_16) {
        out(data, "FPU_SET_TOP(&cpu->fpu, cpu->fpu.top); writew(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", cpu->fpu.sw); CYCLES(2);");
    } else if (op->func == FLD_DOUBLE_REAL_32) {
        out(data, "d2l.l = readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = d2l.d; CYCLES(1);");	    
    } else if (op->func == FISTTP64_32) {
        out(data, "writeq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", (S64)cpu->fpu.regs[cpu->fpu.top].d); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FST_DOUBLE_REAL_32) {
        out(data, "writeq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", cpu->fpu.regs[cpu->fpu.top].l); CYCLES(2);");
    } else if (op->func == FST_DOUBLE_REAL_32_Pop) {
        out(data, "writeq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", cpu->fpu.regs[cpu->fpu.top].l); FPU_FPOP(cpu); CYCLES(2);");
    } else if (op->func == FRSTOR_32) {
        out(data, "FPU_FRSTOR(cpu, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(75);");
    } else if (op->func == FNSAVE_32) {
        out(data, "FPU_FSAVE(cpu, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(127);");
    } else if (op->func == FNSTSW_32) {
        out(data, "FPU_SET_TOP(&cpu->fpu, cpu->fpu.top); writew(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", cpu->fpu.sw); CYCLES(2);");
    } else {
        kpanic("gen0dd");
    }
}

void OPCALL FADD_STi_ST0_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FMUL_STi_ST0_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FCOM_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FCOMPP(struct CPU* cpu, struct Op* op);
void OPCALL FSUBR_STi_ST0_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FSUB_STi_ST0_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FDIVR_STi_ST0_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FDIV_STi_ST0_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FIADD_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIMUL_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_WORD_INTEGER_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FISUB_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FISUBR_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIDIV_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIDIVR_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIADD_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIMUL_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FICOM_WORD_INTEGER_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FISUB_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FISUBR_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIDIV_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIDIVR_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void gen0de(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FADD_STi_ST0_Pop) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d += cpu->fpu.regs[cpu->fpu.top].d; FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FMUL_STi_ST0_Pop) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d *= cpu->fpu.regs[cpu->fpu.top].d; FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FCOM_STi_Pop) {
        out(data, "FPU_FCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FCOMPP) {
        out(data, "FPU_FCOM(&cpu->fpu, cpu->fpu.top, STV(&cpu->fpu, 1));FPU_FPOP(cpu);FPU_FPOP(cpu);CYCLES(1);");
    } else if (op->func == FSUBR_STi_ST0_Pop) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d - cpu->fpu.regs[tmp32].d; FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FSUB_STi_ST0_Pop) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d -= cpu->fpu.regs[cpu->fpu.top].d; FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FDIVR_STi_ST0_Pop) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d / cpu->fpu.regs[tmp32].d; FPU_FPOP(cpu); CYCLES(39);");
    } else if (op->func == FDIV_STi_ST0_Pop) {
        out(data, "cpu->fpu.regs[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")].d /= cpu->fpu.regs[cpu->fpu.top].d; FPU_FPOP(cpu); CYCLES(39);");
    } else if (op->func == FIADD_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d += (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FIMUL_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d *= (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FICOM_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[8].d = (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); CYCLES(4);");
    } else if (op->func == FICOM_WORD_INTEGER_32_Pop) {
        out(data, "cpu->fpu.regs[8].d = (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FCOM(&cpu->fpu, cpu->fpu.top, 8); FPU_FPOP(cpu); CYCLES(4);");
    } else if (op->func == FISUB_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d -= (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(4);");
    } else if (op->func == FISUBR_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ") - cpu->fpu.regs[cpu->fpu.top].d; CYCLES(4);");
    } else if (op->func == FIDIV_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d /= (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(42);");
    } else if (op->func == FIDIVR_WORD_INTEGER_32) {
        out(data, "cpu->fpu.regs[cpu->fpu.top].d = (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ") / cpu->fpu.regs[cpu->fpu.top].d; CYCLES(42);");
    }  else {
        kpanic("gen0de");
    }
}

void OPCALL FFREEP_STi(struct CPU* cpu, struct Op* op);
void OPCALL FXCH_STi(struct CPU* cpu, struct Op* op);
void OPCALL FST_STi_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FNSTSW_AX(struct CPU* cpu, struct Op* op);
void OPCALL FUCOMI_ST0_STj_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FCOMI_ST0_STj_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FILD_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FISTTP16_32(struct CPU* cpu, struct Op* op);
void OPCALL FIST_WORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FIST_WORD_INTEGER_32_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FBLD_PACKED_BCD_32(struct CPU* cpu, struct Op* op);
void OPCALL FILD_QWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FBSTP_PACKED_BCD_32(struct CPU* cpu, struct Op* op);
void OPCALL FISTP_QWORD_INTEGER_32(struct CPU* cpu, struct Op* op);
void OPCALL FILD_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FISTTP16_16(struct CPU* cpu, struct Op* op);
void OPCALL FIST_WORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FIST_WORD_INTEGER_16_Pop(struct CPU* cpu, struct Op* op);
void OPCALL FBLD_PACKED_BCD_16(struct CPU* cpu, struct Op* op);
void OPCALL FILD_QWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void OPCALL FBSTP_PACKED_BCD_16(struct CPU* cpu, struct Op* op);
void OPCALL FISTP_QWORD_INTEGER_16(struct CPU* cpu, struct Op* op);
void gen0df(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->r1, tmp, 10);

    if (op->func == FFREEP_STi) {
        out(data, "cpu->fpu.tags[STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "] = TAG_Empty; FPU_FPOP(cpu); CYCLES(1);");
    } else if (op->func == FXCH_STi) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); tmp32_2 = cpu->fpu.tags[tmp32]; tmpd = cpu->fpu.regs[tmp32].d; cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d; cpu->fpu.tags[cpu->fpu.top] = tmp32_2; cpu->fpu.regs[cpu->fpu.top].d = tmpd; CYCLES(1);");
    } else if (op->func == FNSTSW_AX) {
        out(data, "FPU_SET_TOP(&cpu->fpu, cpu->fpu.top); AX = cpu->fpu.sw; CYCLES(2);");
    } else if (op->func == FST_STi_Pop) {
        out(data, "tmp32 = STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, "); cpu->fpu.tags[tmp32] = cpu->fpu.tags[cpu->fpu.top]; cpu->fpu.regs[tmp32].d = cpu->fpu.regs[cpu->fpu.top].d;FPU_FPOP(cpu);CYCLES(1);");
    } else if (op->func == FUCOMI_ST0_STj_Pop) {
        out(data, "FPU_FCOMI(cpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); FPU_FPOP(cpu); CYCLES(1);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == FCOMI_ST0_STj_Pop) {
        out(data, "FPU_FCOMI(cpu, cpu->fpu.top, STV(&cpu->fpu, ");
        out(data, tmp);
        out(data, ")); FPU_FPOP(cpu); CYCLES(1);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == FILD_WORD_INTEGER_16) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = (S16)readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(1);");
    } else if (op->func == FISTTP16_16) {
        out(data, "writew(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ", (S16)cpu->fpu.regs[cpu->fpu.top].d); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FIST_WORD_INTEGER_16) {
        out(data, "FPU_FST_I16(cpu, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(6);");
    } else if (op->func == FIST_WORD_INTEGER_16_Pop) {
        out(data, "FPU_FST_I16(cpu, ");
        out(data, getEaa16(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FBLD_PACKED_BCD_16) {
        out(data, "FBLD_PACKED_BCD(cpu, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(48);");
    } else if (op->func == FILD_QWORD_INTEGER_16) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = (double)((S64)readq(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")); CYCLES(1);");
    } else if (op->func == FBSTP_PACKED_BCD_16) {
        out(data, "FPU_FBST(cpu, ");
        out(data, getEaa16(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(148);");
    } else if (op->func == FISTP_QWORD_INTEGER_16) {
        out(data, "FPU_FST_I64(cpu, ");
        out(data, getEaa16(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FILD_WORD_INTEGER_32) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(1);");
    } else if (op->func == FISTTP16_32) {
        out(data, "writew(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ", (S16)cpu->fpu.regs[cpu->fpu.top].d); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FIST_WORD_INTEGER_32) {
        out(data, "FPU_FST_I16(cpu, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(6);");
    } else if (op->func == FIST_WORD_INTEGER_32_Pop) {
        out(data, "FPU_FST_I16(cpu, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(6);");
    } else if (op->func == FBLD_PACKED_BCD_32) {
        out(data, "FBLD_PACKED_BCD(cpu, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(48);");
    } else if (op->func == FILD_QWORD_INTEGER_32) {
        out(data, "FPU_PREP_PUSH(cpu); cpu->fpu.regs[cpu->fpu.top].d = (double)((S64)readq(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")); CYCLES(1);");
    } else if (op->func == FBSTP_PACKED_BCD_32) {
        out(data, "FPU_FBST(cpu, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(148);");
    } else if (op->func == FISTP_QWORD_INTEGER_32) {
        out(data, "FPU_FST_I64(cpu, ");
        out(data, getEaa32(op));
        out(data, "); FPU_FPOP(cpu); CYCLES(6);");
    } else {
        kpanic("gen0df");
    }
}

void OPCALL loopnz16(struct CPU* cpu, struct Op* op);
void OPCALL loopnz32(struct CPU* cpu, struct Op* op);
void gen0e0(struct GenData* data, struct Op* op) {
    char tmp[16];

    if (op->func == loopnz16) {
        out(data, "CX--; if (CX!=0 && !");
        out(data, getFlag(data, ZF));
        out(data, ") {cpu->eip.u32+=");
    } else if (op->func == loopnz32) {
        out(data, "ECX--; if (ECX!=0 && !");
        out(data, getFlag(data, ZF));
        out(data, ") {cpu->eip.u32+=");
    } else {
        kpanic("gen0e0");
    }
    itoa(op->eipCount+op->data1, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock2(cpu);} else {cpu->eip.u32+=");
    itoa(op->eipCount, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock1(cpu);} CYCLES(7);");
}

void OPCALL loopz16(struct CPU* cpu, struct Op* op);
void OPCALL loopz32(struct CPU* cpu, struct Op* op);
void gen0e1(struct GenData* data, struct Op* op) {
    char tmp[16];

    if (op->func == loopz16) {
        out(data, "CX--; if (CX!=0 && ");
        out(data, getFlag(data, ZF));
        out(data, ") {cpu->eip.u32+=");
    } else if (op->func == loopz32) {
        out(data, "ECX--; if (ECX!=0 && ");
        out(data, getFlag(data, ZF));
        out(data, ") {cpu->eip.u32+=");
    } else {
        kpanic("gen0e1");
    }
    itoa(op->eipCount+op->data1, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock2(cpu);} else {cpu->eip.u32+=");
    itoa(op->eipCount, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock1(cpu);} CYCLES(7);");
}

void OPCALL loop16(struct CPU* cpu, struct Op* op);
void OPCALL loop32(struct CPU* cpu, struct Op* op);
void gen0e2(struct GenData* data, struct Op* op) {
    char tmp[16];

    if (op->func == loop16) {
        out(data, "CX--; if (CX!=0) {cpu->eip.u32+=");
    } else if (op->func == loop32) {
        out(data, "ECX--; if (ECX!=0) {cpu->eip.u32+=");
    } else {
        kpanic("gen0e2");
    }
    itoa(op->eipCount+op->data1, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock2(cpu);} else {cpu->eip.u32+=");
    itoa(op->eipCount, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock1(cpu);} CYCLES(7);");
}

void OPCALL jcxz16(struct CPU* cpu, struct Op* op);
void OPCALL jcxz32(struct CPU* cpu, struct Op* op);
void gen0e3(struct GenData* data, struct Op* op) {
    char tmp[16];

    if (op->func == jcxz16) {
        out(data, "if (CX==0) {cpu->eip.u32+=");
    } else if (op->func == jcxz32) {
        out(data, "if (ECX==0) {cpu->eip.u32+=");
    } else {
        kpanic("gen0e3");
    }
    itoa(op->eipCount+op->data1, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock2(cpu);} else {cpu->eip.u32+=");
    itoa(op->eipCount, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock1(cpu);} CYCLES(7);");
}

void gen0e8(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->eipCount, tmp, 10);
    out(data, "push16(cpu, cpu->eip.u32 + ");
    out(data, tmp);
    out(data, "); cpu->eip.u32 += ");
    out(data, tmp);
    out(data, " + ");
    itoa(op->data1, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock1(cpu); CYCLES(1);");
}

void gen2e8(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->eipCount, tmp, 10);
    out(data, "push32(cpu, cpu->eip.u32 + ");
    out(data, tmp);
    out(data, "); cpu->eip.u32 += ");
    out(data, tmp);
    out(data, " + ");
    itoa(op->data1, tmp, 10);
    out(data, tmp);
    out(data, "; cpu->nextBlock = getBlock1(cpu); CYCLES(1);");
}

void gen0e9(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->eipCount, tmp, 10);
    out(data, "cpu->eip.u32 += ");
    out(data, tmp);
    out(data, " + ");
    itoa(op->data1, tmp, 10);
    out(data, tmp);
    out(data, ";");
    if (!data->inlinedBlock)
         out(data, " cpu->nextBlock = getBlock1(cpu);");
    out(data, "CYCLES(1);");
}

void gen0f5(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); setCF(cpu, !(cpu->flags & CF)); CYCLES(2);");
    data->lazyFlags = sFLAGS_NONE;
}

void OPCALL test8_reg(struct CPU* cpu, struct Op* op);
void OPCALL test8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL test8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL not8_reg(struct CPU* cpu, struct Op* op);
void OPCALL not8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL not8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL neg8_reg(struct CPU* cpu, struct Op* op);
void OPCALL neg8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL neg8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL mul8_reg(struct CPU* cpu, struct Op* op);
void OPCALL mul8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mul8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL imul8_reg(struct CPU* cpu, struct Op* op);
void OPCALL imul8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL imul8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL div8_reg(struct CPU* cpu, struct Op* op);
void OPCALL div8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL div8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL idiv8_reg(struct CPU* cpu, struct Op* op);
void OPCALL idiv8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL idiv8_mem32(struct CPU* cpu, struct Op* op);
void gen0f6(struct GenData* data, struct Op* op) {
    char d[16];

    itoa(op->data1, d, 16);

    if (op->func == test8_reg) {
        if (inlineTestJump(data, op, sFLAGS_TEST8, "1"))
            return;
        out(data, "cpu->dst.u8 = ");
        out(data, r8(op->r1));
        out(data, "; cpu->src.u8 = 0x");
        out(data, d);
        out(data, "; cpu->result.u8 = cpu->dst.u8 & cpu->src.u8; cpu->lazyFlags = FLAGS_TEST8; CYCLES(1);");
        data->lazyFlags = sFLAGS_TEST8;
    } else if (op->func == test8_mem16) {
        if (inlineTestJump(data, op, sFLAGS_TEST8, "2"))
            return;
        out(data, "cpu->dst.u8 = readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); cpu->src.u8 = 0x");
        out(data, d);
        out(data, "; cpu->result.u8 = cpu->dst.u8 & cpu->src.u8; cpu->lazyFlags = FLAGS_TEST8; CYCLES(2);");
        data->lazyFlags = sFLAGS_TEST8;
    } else if (op->func == test8_mem32) {
        if (inlineTestJump(data, op, sFLAGS_TEST8, "2"))
            return;
        out(data, "cpu->dst.u8 = readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); cpu->src.u8 = 0x");
        out(data, d);
        out(data, "; cpu->result.u8 = cpu->dst.u8 & cpu->src.u8; cpu->lazyFlags = FLAGS_TEST8; CYCLES(2);");
        data->lazyFlags = sFLAGS_TEST8;
    } else if (op->func == not8_reg) {
        out(data, r8(op->r1));
        out(data, " = ~ ");
        out(data, r8(op->r1));
        out(data, "; CYCLES(1);");
    } else if (op->func == not8_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, ~readb(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == not8_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, ~readb(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == neg8_reg) {
        out(data, "cpu->dst.u8 = ");
        out(data, r8(op->r1));
        out(data, "; cpu->result.u8 = 0-cpu->dst.u8; ");
        out(data, r8(op->r1));
        out(data, " = cpu->result.u8; cpu->lazyFlags = FLAGS_NEG8; CYCLES(1);");
        data->lazyFlags = sFLAGS_NEG8;
    } else if (op->func == neg8_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->dst.u8 = readb(cpu->thread, eaa); cpu->result.u8 = 0-cpu->dst.u8; writeb(cpu->thread, eaa, cpu->result.u8); cpu->lazyFlags = FLAGS_NEG8; CYCLES(3);");
        data->lazyFlags = sFLAGS_NEG8;
    } else if (op->func == neg8_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->dst.u8 = readb(cpu->thread, eaa); cpu->result.u8 = 0-cpu->dst.u8; writeb(cpu->thread, eaa, cpu->result.u8); cpu->lazyFlags = FLAGS_NEG8; CYCLES(3);");
        data->lazyFlags = sFLAGS_NEG8;
    } else if (op->func == mul8_reg) {
        out(data, "AX = AL * ");
        out(data, r8(op->r1));
        out(data, "; fillFlagsNoCFOF(cpu); if (AX>0xFF) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul8_mem16) {
        out(data, "AX = AL * readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); fillFlagsNoCFOF(cpu); if (AX>0xFF) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul8_mem32) {
        out(data, "AX = AL * readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); fillFlagsNoCFOF(cpu); if (AX>0xFF) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul8_reg) {
        out(data, "AX = (S16)((S8)AL) * (S8)(");
        out(data, r8(op->r1));
        out(data, "); fillFlagsNoCFOF(cpu);if ((S16)AX<-128 || (S16)AX>127) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul8_mem16) {
        out(data, "AX = (S16)((S8)AL) * (S8)readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); fillFlagsNoCFOF(cpu); if ((S16)AX<-128 || (S16)AX>127) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul8_mem32) {
        out(data, "AX = (S16)((S8)AL) * (S8)readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); fillFlagsNoCFOF(cpu); if ((S16)AX<-128 || (S16)AX>127) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == div8_reg) {
        out(data, "if (!div8(cpu, ");
        out(data, r8(op->r1));
        out(data, ")) return; CYCLES(17);");
    } else if (op->func == div8_mem16) {
        out(data, "if (!div8(cpu, readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "))) return; CYCLES(17);");
    } else if (op->func == div8_mem32) {
        out(data, "if (!div8(cpu, readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "))) return; CYCLES(17);");
    } else if (op->func == idiv8_reg) {
        out(data, "if (!idiv8(cpu, (S8)");
        out(data, r8(op->r1));
        out(data, ")) return; CYCLES(22);");
    } else if (op->func == idiv8_mem16) {
        out(data, "if (!idiv8(cpu, (S8)readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "))) return; CYCLES(22);");
    } else if (op->func == idiv8_mem32) {
        out(data, "if (!idiv8(cpu, (S8)readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "))) return; CYCLES(22);");
    } else {
        kpanic("gen0f6");
    }
}

void OPCALL test16_reg(struct CPU* cpu, struct Op* op);
void OPCALL test16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL test16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL not16_reg(struct CPU* cpu, struct Op* op);
void OPCALL not16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL not16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL neg16_reg(struct CPU* cpu, struct Op* op);
void OPCALL neg16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL neg16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL mul16_reg(struct CPU* cpu, struct Op* op);
void OPCALL mul16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mul16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL imul16_reg(struct CPU* cpu, struct Op* op);
void OPCALL imul16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL imul16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL div16_reg(struct CPU* cpu, struct Op* op);
void OPCALL div16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL div16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL idiv16_reg(struct CPU* cpu, struct Op* op);
void OPCALL idiv16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL idiv16_mem32(struct CPU* cpu, struct Op* op);
void gen0f7(struct GenData* data, struct Op* op) {
    char d[16];

    itoa(op->data1, d, 16);

    if (op->func == test16_reg) {
        if (inlineTestJump(data, op, sFLAGS_TEST16, "1"))
            return;
        out(data, "cpu->dst.u16 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->src.u16 = 0x");
        out(data, d);
        out(data, "; cpu->result.u16 = cpu->dst.u16 & cpu->src.u16; cpu->lazyFlags = FLAGS_TEST16; CYCLES(1);");
        data->lazyFlags = sFLAGS_TEST16;
    } else if (op->func == test16_mem16) {
        if (inlineTestJump(data, op, sFLAGS_TEST16, "2"))
            return;
        out(data, "cpu->dst.u16 = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); cpu->src.u16 = 0x");
        out(data, d);
        out(data, "; cpu->result.u16 = cpu->dst.u16 & cpu->src.u16; cpu->lazyFlags = FLAGS_TEST16; CYCLES(2);");
        data->lazyFlags = sFLAGS_TEST16;
    } else if (op->func == test16_mem32) {
        if (inlineTestJump(data, op, sFLAGS_TEST16, "2"))
        return;
        out(data, "cpu->dst.u16 = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); cpu->src.u16 = 0x");
        out(data, d);
        out(data, "; cpu->result.u16 = cpu->dst.u16 & cpu->src.u16; cpu->lazyFlags = FLAGS_TEST16; CYCLES(2);");
        data->lazyFlags = sFLAGS_TEST16;
    } else if (op->func == not16_reg) {
        out(data, r16(op->r1));
        out(data, " = ~ ");
        out(data, r16(op->r1));
        out(data, "; CYCLES(1);");
    } else if (op->func == not16_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, ~readw(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == not16_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, ~readw(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == neg16_reg) {
        out(data, "cpu->dst.u16 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->result.u16 = 0-cpu->dst.u16; ");
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; cpu->lazyFlags = FLAGS_NEG16; CYCLES(1);");
        data->lazyFlags = sFLAGS_NEG16;
    } else if (op->func == neg16_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->dst.u16 = readw(cpu->thread, eaa); cpu->result.u16 = 0-cpu->dst.u16; writew(cpu->thread, eaa, cpu->result.u16); cpu->lazyFlags = FLAGS_NEG16; CYCLES(3);");
        data->lazyFlags = sFLAGS_NEG16;
    } else if (op->func == neg16_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->dst.u16 = readw(cpu->thread, eaa); cpu->result.u16 = 0-cpu->dst.u16; writew(cpu->thread, eaa, cpu->result.u16); cpu->lazyFlags = FLAGS_NEG16; CYCLES(3);");
        data->lazyFlags = sFLAGS_NEG16;
    } else if (op->func == mul16_reg) {
        out(data, "tmp32 = (U32)AX * ");
        out(data, r16(op->r1));
        out(data, "; AX = (U16)tmp32; DX = (U16)(tmp32 >> 16); fillFlagsNoCFOF(cpu); if (DX) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul16_mem16) {
        out(data, "tmp32 = (U32)AX * readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); AX = (U16)tmp32; DX = (U16)(tmp32 >> 16); fillFlagsNoCFOF(cpu); if (DX) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul16_mem32) {
        out(data, "tmp32 = (U32)AX * readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); AX = (U16)tmp32; DX = (U16)(tmp32 >> 16); fillFlagsNoCFOF(cpu); if (DX) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul16_reg) {
        out(data, "tmps32 = (S32)((S16)AX) * (S16)");
        out(data, r16(op->r1));
        out(data, "; AX = (S16)tmps32; DX = (S16)(tmps32 >> 16); fillFlagsNoCFOF(cpu); if (tmps32>32767 || tmps32<-32768) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF); } CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul16_mem16) {
        out(data, "tmps32 = (S32)((S16)AX) * (S16)readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); AX = (S16)tmps32; DX = (S16)(tmps32 >> 16); fillFlagsNoCFOF(cpu); if (tmps32>32767 || tmps32<-32768) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF); } CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul16_mem32) {
        out(data, "tmps32 = (S32)((S16)AX) * (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); AX = (S16)tmps32; DX = (S16)(tmps32 >> 16); fillFlagsNoCFOF(cpu); if (tmps32>32767 || tmps32<-32768) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF); } CYCLES(11);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == div16_reg) {
        out(data, "if (!div16(cpu, ");
        out(data, r16(op->r1));
        out(data, ")) return; CYCLES(25);");
    } else if (op->func == div16_mem16) {
        out(data, "if (!div16(cpu, readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "))) return; CYCLES(25);");
    } else if (op->func == div16_mem32) {
        out(data, "if (!div16(cpu, readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "))) return; CYCLES(25);");
    } else if (op->func == idiv16_reg) {
        out(data, "if (!idiv16(cpu, (S16)");
        out(data, r16(op->r1));
        out(data, ")) return; CYCLES(30);");
    } else if (op->func == idiv16_mem16) {
        out(data, "if (!idiv16(cpu, (S16)readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "))) return; CYCLES(30);");
    } else if (op->func == idiv16_mem32) {
        out(data, "if (!idiv16(cpu, (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "))) return; CYCLES(30);");
    } else {
        kpanic("gen0f7");
    }
}

void OPCALL test32_reg(struct CPU* cpu, struct Op* op);
void OPCALL test32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL test32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL not32_reg(struct CPU* cpu, struct Op* op);
void OPCALL not32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL not32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL neg32_reg(struct CPU* cpu, struct Op* op);
void OPCALL neg32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL neg32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL mul32_reg(struct CPU* cpu, struct Op* op);
void OPCALL mul32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL mul32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL imul32_reg(struct CPU* cpu, struct Op* op);
void OPCALL imul32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL imul32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL div32_reg(struct CPU* cpu, struct Op* op);
void OPCALL div32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL div32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL idiv32_reg(struct CPU* cpu, struct Op* op);
void OPCALL idiv32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL idiv32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL neg32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL neg32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL neg32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL mul32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL mul32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL mul32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL imul32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL imul32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL imul32_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen2f7(struct GenData* data, struct Op* op) {
    char d[16];

    itoa(op->data1, d, 16);

    if (op->func == test32_reg) {
        if (inlineTestJump(data, op, sFLAGS_TEST32, "1"))
            return;
        out(data, "cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->src.u32 = 0x");
        out(data, d);
        out(data, "; cpu->result.u32 = cpu->dst.u32 & cpu->src.u32; cpu->lazyFlags = FLAGS_TEST32; CYCLES(1);");
        data->lazyFlags = sFLAGS_TEST32;
    } else if (op->func == test32_mem16) {
        if (inlineTestJump(data, op, sFLAGS_TEST32, "2"))
            return;
        out(data, "cpu->dst.u32 = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); cpu->src.u32 = 0x");
        out(data, d);
        out(data, "; cpu->result.u32 = cpu->dst.u32 & cpu->src.u32; cpu->lazyFlags = FLAGS_TEST32; CYCLES(2);");
        data->lazyFlags = sFLAGS_TEST32;
    } else if (op->func == test32_mem32) {
        if (inlineTestJump(data, op, sFLAGS_TEST32, "2"))
            return;
        out(data, "cpu->dst.u32 = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); cpu->src.u32 = 0x");
        out(data, d);
        out(data, "; cpu->result.u32 = cpu->dst.u32 & cpu->src.u32; cpu->lazyFlags = FLAGS_TEST32; CYCLES(2);");
        data->lazyFlags = sFLAGS_TEST32;
    } else if (op->func == not32_reg) {
        out(data, r32(op->r1));
        out(data, " = ~ ");
        out(data, r32(op->r1));
        out(data, "; CYCLES(1);");
    } else if (op->func == not32_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, ~readd(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == not32_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, ~readd(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == neg32_reg) {
        out(data, "cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32 = 0-cpu->dst.u32; ");
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; cpu->lazyFlags = FLAGS_NEG32; CYCLES(1);");
        data->lazyFlags = sFLAGS_NEG32;
    } else if (op->func == neg32_reg_noflags) {
        out(data, "cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, " = 0 - ");
        out(data, r32(op->r1));
        out(data, "; CYCLES(1);");
    } else if (op->func == neg32_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32 = 0-cpu->dst.u32; writed(cpu->thread, eaa, cpu->result.u32); cpu->lazyFlags = FLAGS_NEG32; CYCLES(3);");
        data->lazyFlags = sFLAGS_NEG32;
    } else if (op->func == neg32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, 0-readd(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == neg32_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32 = 0-cpu->dst.u32; writed(cpu->thread, eaa, cpu->result.u32); cpu->lazyFlags = FLAGS_NEG32; CYCLES(3);");
        data->lazyFlags = sFLAGS_NEG32;
    } else if (op->func == neg32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, 0-readd(cpu->thread, eaa)); CYCLES(3);");
    } else if (op->func == mul32_reg) {
        out(data, "tmp64 = (U64)EAX * ");
        out(data, r32(op->r1));
        out(data, "; EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); fillFlagsNoCFOF(cpu); if (EDX) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(10);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul32_reg_noflags) {
        out(data, "tmp64 = (U64)EAX * ");
        out(data, r32(op->r1));
        out(data, "; EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); CYCLES(10);");
    } else if (op->func == mul32_mem16) {
        out(data, "tmp64 = (U64)EAX * readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); fillFlagsNoCFOF(cpu); if (EDX) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(10);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul32_mem16_noflags) {
        out(data, "tmp64 = (U64)EAX * readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); CYCLES(10);");
    } else if (op->func == mul32_mem32) {
        out(data, "tmp64 = (U64)EAX * readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); fillFlagsNoCFOF(cpu); if (EDX) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(10);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == mul32_mem32_noflags) {
        out(data, "tmp64 = (U64)EAX * readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); CYCLES(10);");
    } else if (op->func == imul32_reg) {
        out(data, "tmps64 = (S64)((S32)EAX) * (S32)");
        out(data, r32(op->r1));
        out(data, "; EAX = (S32)tmps64; EDX = (S32)(tmps64 >> 32); fillFlagsNoCFOF(cpu); if (tmps64>0x7fffffffl || tmps64<-0x7fffffffl) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(10);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul32_reg_noflags) {
        out(data, "tmps64 = (S64)((S32)EAX) * (S32)");
        out(data, r32(op->r1));
        out(data, "; EAX = (S32)tmps64; EDX = (S32)(tmps64 >> 32); CYCLES(10);");
    } else if (op->func == imul32_mem16) {
        out(data, "tmps64 = (S64)((S32)EAX) * (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); EAX = (S32)tmps64; EDX = (S32)(tmps64 >> 32); fillFlagsNoCFOF(cpu); if (tmps64>0x7fffffffl || tmps64<-0x7fffffffl) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(10);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul32_mem16_noflags) {
        out(data, "tmps64 = (S64)((S32)EAX) * (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); EAX = (S32)tmps64; EDX = (S32)(tmps64 >> 32); CYCLES(10);");
    } else if (op->func == imul32_mem32) {
        out(data, "tmps64 = (S64)((S32)EAX) * (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); EAX = (S32)tmps64; EDX = (S32)(tmps64 >> 32); fillFlagsNoCFOF(cpu); if (tmps64>0x7fffffffl || tmps64<-0x7fffffffl) {cpu->flags|=CF|OF;} else {cpu->flags&=~(CF|OF);} CYCLES(10);");
        data->lazyFlags = sFLAGS_NONE;
    } else if (op->func == imul32_mem32_noflags) {
        out(data, "tmps64 = (S64)((S32)EAX) * (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); EAX = (S32)tmps64; EDX = (S32)(tmps64 >> 32); CYCLES(10);");
    } else if (op->func == div32_reg) {
        out(data, "if (!div32(cpu, ");
        out(data, r32(op->r1));
        out(data, ")) return; CYCLES(41);");
    } else if (op->func == div32_mem16) {
        out(data, "if (!div32(cpu, readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "))) return; CYCLES(41);");
    } else if (op->func == div32_mem32) {
        out(data, "if (!div32(cpu, readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "))) return; CYCLES(41);");
    } else if (op->func == idiv32_reg) {
        out(data, "if (!idiv32(cpu, (S32)");
        out(data, r32(op->r1));
        out(data, ")) return; CYCLES(46);");
    } else if (op->func == idiv32_mem16) {
        out(data, "if (!idiv32(cpu, (S32)readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "))) return; CYCLES(46);");
    } else if (op->func == idiv32_mem32) {
        out(data, "if (!idiv32(cpu, (S32)readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "))) return; CYCLES(46);");
    } else {
        kpanic("gen2f7");
    }
}

void gen0f8(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); cpu->flags &= ~CF; CYCLES(2);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen0f9(struct GenData* data, struct Op* op) {
    out(data, "fillFlags(cpu); cpu->flags |= CF; CYCLES(2);");
    data->lazyFlags = sFLAGS_NONE;
}

void gen0fc(struct GenData* data, struct Op* op) {
    out(data, "removeFlag(DF); cpu->df = 1; CYCLES(2);");
}

void gen0fd(struct GenData* data, struct Op* op) {
    out(data, "addFlag(DF); cpu->df = -1; CYCLES(2);");
}

void OPCALL inc8_reg(struct CPU* cpu, struct Op* op);
void OPCALL inc8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL inc8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL dec8_reg(struct CPU* cpu, struct Op* op);
void OPCALL dec8_mem16(struct CPU* cpu, struct Op* op);
void OPCALL dec8_mem32(struct CPU* cpu, struct Op* op);
void OPCALL inc8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL inc8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL inc8_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec8_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec8_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec8_mem32_noflags(struct CPU* cpu, struct Op* op);
void gen0fe(struct GenData* data, struct Op* op) {
    if (op->func == inc8_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u8=");
        out(data, r8(op->r1));
        out(data, "; cpu->result.u8=cpu->dst.u8 + 1; cpu->lazyFlags = FLAGS_INC8; ");
        data->lazyFlags = sFLAGS_INC8;
        out(data, r8(op->r1));
        out(data, " = cpu->result.u8; CYCLES(1);");
    } else if (op->func == inc8_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u8 = readb(cpu->thread, eaa); cpu->result.u8=cpu->dst.u8 + 1; cpu->lazyFlags = FLAGS_INC8; writeb(cpu->thread, eaa, cpu->result.u8); CYCLES(3);");
        data->lazyFlags = sFLAGS_INC8;
    } else if (op->func == inc8_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u8 = readb(cpu->thread, eaa); cpu->result.u8=cpu->dst.u8 + 1; cpu->lazyFlags = FLAGS_INC8; writeb(cpu->thread, eaa, cpu->result.u8); CYCLES(3);");
        data->lazyFlags = sFLAGS_INC8;
    } else if (op->func == dec8_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u8=");
        out(data, r8(op->r1));
        out(data, "; cpu->result.u8=cpu->dst.u8 - 1; cpu->lazyFlags = FLAGS_DEC8; ");
        data->lazyFlags = sFLAGS_DEC8;
        out(data, r8(op->r1));
        out(data, " = cpu->result.u8; CYCLES(1);");
    } else if (op->func == dec8_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u8 = readb(cpu->thread, eaa); cpu->result.u8=cpu->dst.u8 - 1; cpu->lazyFlags = FLAGS_DEC8; writeb(cpu->thread, eaa, cpu->result.u8); CYCLES(3);");
        data->lazyFlags = sFLAGS_DEC8;
    } else if (op->func == dec8_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u8 = readb(cpu->thread, eaa); cpu->result.u8=cpu->dst.u8 - 1; cpu->lazyFlags = FLAGS_DEC8; writeb(cpu->thread, eaa, cpu->result.u8); CYCLES(3);");
        data->lazyFlags = sFLAGS_DEC8;
    } else if (op->func == inc8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r1));
        out(data, " + 1; CYCLES(1);");
    } else if (op->func == inc8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) + 1); CYCLES(3);");
    } else if (op->func == inc8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) + 1); CYCLES(3);");
    } else if (op->func == dec8_reg_noflags) {
        out(data, r8(op->r1));
        out(data, " = ");
        out(data, r8(op->r1));
        out(data, " - 1; CYCLES(1);");
    } else if (op->func == dec8_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - 1); CYCLES(3);");
    } else if (op->func == dec8_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writeb(cpu->thread, eaa, readb(cpu->thread, eaa) - 1); CYCLES(3);");
    } else {
        kpanic("gen0fe");
    }
}

void OPCALL inc16_reg(struct CPU* cpu, struct Op* op);
void OPCALL inc16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL inc16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL dec16_reg(struct CPU* cpu, struct Op* op);
void OPCALL dec16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL dec16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL inc16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL inc16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL inc16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec16_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec16_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec16_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL callEv16_reg(struct CPU* cpu, struct Op* op);
void OPCALL callEv16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL callEv16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL jmpEv16_reg(struct CPU* cpu, struct Op* op);
void OPCALL jmpEv16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL jmpEv16_mem32(struct CPU* cpu, struct Op* op);
void OPCALL pushEv16_reg(struct CPU* cpu, struct Op* op);
void OPCALL pushEv16_mem16(struct CPU* cpu, struct Op* op);
void OPCALL pushEv16_mem32(struct CPU* cpu, struct Op* op);
void gen0ff(struct GenData* data, struct Op* op) {
    if (op->func == inc16_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16=");
        out(data, r16(op->r1));
        out(data, "; cpu->result.u16=cpu->dst.u16 + 1; cpu->lazyFlags = FLAGS_INC16; ");
        data->lazyFlags = sFLAGS_INC16;
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; CYCLES(1);");
    } else if (op->func == inc16_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16 = readw(cpu->thread, eaa); cpu->result.u16=cpu->dst.u16 + 1; cpu->lazyFlags = FLAGS_INC16; writew(cpu->thread, eaa, cpu->result.u16); CYCLES(3);");
        data->lazyFlags = sFLAGS_INC16;
    } else if (op->func == inc16_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16 = readw(cpu->thread, eaa); cpu->result.u16=cpu->dst.u16 + 1; cpu->lazyFlags = FLAGS_INC16; writew(cpu->thread, eaa, cpu->result.u16); CYCLES(3);");
        data->lazyFlags = sFLAGS_INC16;
    } else if (op->func == dec16_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16=");
        out(data, r16(op->r1));
        out(data, "; cpu->result.u16=cpu->dst.u16 - 1; cpu->lazyFlags = FLAGS_DEC16; ");
        data->lazyFlags = sFLAGS_DEC16;
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; CYCLES(1);");
    } else if (op->func == dec16_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16 = readw(cpu->thread, eaa); cpu->result.u16=cpu->dst.u16 - 1; cpu->lazyFlags = FLAGS_DEC16; writew(cpu->thread, eaa, cpu->result.u16); CYCLES(3);");
        data->lazyFlags = sFLAGS_DEC16;
    } else if (op->func == dec16_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u16 = readw(cpu->thread, eaa); cpu->result.u16=cpu->dst.u16 - 1; cpu->lazyFlags = FLAGS_DEC16; writew(cpu->thread, eaa, cpu->result.u16); CYCLES(3);");
        data->lazyFlags = sFLAGS_DEC16;
    } else if (op->func == inc16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " + 1; CYCLES(1);");
    } else if (op->func == inc16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) + 1); CYCLES(3);");
    } else if (op->func == inc16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) + 1); CYCLES(3);");
    } else if (op->func == dec16_reg_noflags) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r1));
        out(data, " - 1; CYCLES(1);");
    } else if (op->func == dec16_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) - 1); CYCLES(3);");
    } else if (op->func == dec16_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writew(cpu->thread, eaa, readw(cpu->thread, eaa) - 1); CYCLES(3);");
    } else if (op->func == callEv16_reg) {
        char tmp[16];

        out(data, "push16(cpu, cpu->eip.u32 + ");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, "); cpu->eip.u32 = ");
        out(data, r16(op->r1));
        out(data, "; CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == callEv16_mem16) {
        char tmp[16];

        out(data, "tmp32 = cpu->eip.u32 + ");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, "; tmp16 = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); push16(cpu, tmp32); cpu->eip.u32 = tmp16; CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == callEv16_mem32) {
        char tmp[16];

        out(data, "tmp32 = cpu->eip.u32 + ");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, "; tmp16 = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); push16(cpu, tmp32); cpu->eip.u32 = tmp16; CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == jmpEv16_reg) {
        out(data, "cpu->eip.u32 = ");
        out(data, r16(op->r1));
        out(data, "; CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == jmpEv16_mem16) {
        out(data, "cpu->eip.u32 = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == jmpEv16_mem32) {
        out(data, "cpu->eip.u32 = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == pushEv16_reg) {
        out(data, "push16(cpu, ");
        out(data, r16(op->r1));
        out(data, "); CYCLES(1);");
    } else if (op->func == pushEv16_mem16) {
        out(data, "push16(cpu, readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")); CYCLES(2);");
    } else if (op->func == pushEv16_mem32) {
        out(data, "push16(cpu, readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")); CYCLES(2);");
    } else {
        kpanic("gen0ff");
    }
}

void OPCALL inc32_reg(struct CPU* cpu, struct Op* op);
void OPCALL inc32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL inc32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL dec32_reg(struct CPU* cpu, struct Op* op);
void OPCALL dec32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL dec32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL inc32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL inc32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL inc32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec32_reg_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec32_mem16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dec32_mem32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL callNear32_reg(struct CPU* cpu, struct Op* op);
void OPCALL callNear32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL callNear32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL jmpNear32_reg(struct CPU* cpu, struct Op* op);
void OPCALL jmpNear32_mem16(struct CPU* cpu, struct Op* op);
void OPCALL jmpNear32_mem32(struct CPU* cpu, struct Op* op);
void OPCALL pushEd_reg(struct CPU* cpu, struct Op* op);
void OPCALL pushEd_mem16(struct CPU* cpu, struct Op* op);
void OPCALL pushEd_mem32(struct CPU* cpu, struct Op* op);
void gen2ff(struct GenData* data, struct Op* op) {
    if (op->func == inc32_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32=");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=cpu->dst.u32 + 1; cpu->lazyFlags = FLAGS_INC32; ");
        data->lazyFlags = sFLAGS_INC32;
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; CYCLES(1);");
    } else if (op->func == inc32_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=cpu->dst.u32 + 1; cpu->lazyFlags = FLAGS_INC32; writed(cpu->thread, eaa, cpu->result.u32); CYCLES(3);");
        data->lazyFlags = sFLAGS_INC32;
    } else if (op->func == inc32_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=cpu->dst.u32 + 1; cpu->lazyFlags = FLAGS_INC32; writed(cpu->thread, eaa, cpu->result.u32); CYCLES(3);");
        data->lazyFlags = sFLAGS_INC32;
    } else if (op->func == dec32_reg) {
        out(data, "cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32=");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=cpu->dst.u32 - 1; cpu->lazyFlags = FLAGS_DEC32; ");
        data->lazyFlags = sFLAGS_DEC32;
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; CYCLES(1);");
    } else if (op->func == dec32_mem16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=cpu->dst.u32 - 1; cpu->lazyFlags = FLAGS_DEC32; writed(cpu->thread, eaa, cpu->result.u32); CYCLES(3);");
        data->lazyFlags = sFLAGS_DEC32;
    } else if (op->func == dec32_mem32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->oldcf=");
        out(data, getFlag(data, CF));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=cpu->dst.u32 - 1; cpu->lazyFlags = FLAGS_DEC32; writed(cpu->thread, eaa, cpu->result.u32); CYCLES(3);");
        data->lazyFlags = sFLAGS_DEC32;
    } else if (op->func == inc32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " + 1; CYCLES(1);");
    } else if (op->func == inc32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) + 1); CYCLES(3);");
    } else if (op->func == inc32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) + 1); CYCLES(3);");
    } else if (op->func == dec32_reg_noflags) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r1));
        out(data, " - 1; CYCLES(1);");
    } else if (op->func == dec32_mem16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) - 1); CYCLES(3);");
    } else if (op->func == dec32_mem32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; writed(cpu->thread, eaa, readd(cpu->thread, eaa) - 1); CYCLES(3);");
    } else if (op->func == callNear32_reg) {
        char tmp[16];

        out(data, "push32(cpu, cpu->eip.u32 + ");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, "); cpu->eip.u32 = ");
        out(data, r32(op->r1));
        out(data, "; CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == callNear32_mem16) {
        char tmp[16];

        out(data, "tmp32 = cpu->eip.u32 + ");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, "; tmp32_2 = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); push32(cpu, tmp32); cpu->eip.u32 = tmp32_2; CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == callNear32_mem32) {
        char tmp[16];

        out(data, "tmp32 = cpu->eip.u32 + ");
        itoa(op->eipCount, tmp, 10);
        out(data, tmp);
        out(data, "; tmp32_2 = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); push32(cpu, tmp32); cpu->eip.u32 = tmp32_2; CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == jmpNear32_reg) {
        out(data, "cpu->eip.u32 = ");
        out(data, r32(op->r1));
        out(data, "; CYCLES(2); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == jmpNear32_mem16) {
        out(data, "cpu->eip.u32 = readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ");");
        out(data, " CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == jmpNear32_mem32) {
        out(data, "cpu->eip.u32 = readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ");");
        out(data, " CYCLES(4); cpu->nextBlock = getBlock(cpu, cpu->eip.u32);");
    } else if (op->func == pushEd_reg) {
        out(data, "push32(cpu, ");
        out(data, r32(op->r1));
        out(data, "); CYCLES(1);");
    } else if (op->func == pushEd_mem16) {
        out(data, "push32(cpu, readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")); CYCLES(2);");
    } else if (op->func == pushEd_mem32) {
        out(data, "push32(cpu, readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")); CYCLES(2);");
    } else {
        kpanic("gen2ff");
    }
}

void gen131(struct GenData* data, struct Op* op) {
#ifdef INCLUDE_CYCLES
    out(data, "tmp64 = cpu->timeStampCounter+cpu->blockCounter; EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); CYCLES(1);");
#else
    out(data, "tmp64 = cpu->timeStampCounter+cpu->blockInstructionCount; EAX = (U32)tmp64; EDX = (U32)(tmp64 >> 32); CYCLES(1);");
#endif
}

void gen1a2(struct GenData* data, struct Op* op) {
    out(data, "cpuid(cpu); CYCLES(14);");
}

void OPCALL btr16r16(struct CPU* cpu, struct Op* op);
void OPCALL bte16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL bte16r16_32(struct CPU* cpu, struct Op* op);
void gen1a3(struct GenData* data, struct Op* op) {
    data->lazyFlags = sFLAGS_NONE;
    if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
         out(data, "fillFlagsNoCF(cpu); ");         
    } else {
        out(data, "cpu->lazyFlags = FLAGS_NONE; ");
    }
    if (op->func == btr16r16) {
        out(data, "setCF(cpu, ");
        out(data, r16(op->r1));
        out(data, " & (1 << (");
        out(data, r16(op->r2));
        out(data, " & 15))); CYCLES(4);");
    } else if (op->func == bte16r16_16) {
        out(data, "setCF(cpu, (readw(cpu->thread, (");
        out(data, getEaa16(op));
        out(data, ")+(((S16)");
        out(data, r16(op->r1));
        out(data, ")>>4)*2) & (1 << (");
        out(data, r16(op->r1));
        out(data, " & 15)))); CYCLES(9)");
    } else if (op->func == bte16r16_32) {
        out(data, "setCF(cpu, (readw(cpu->thread, (");
        out(data, getEaa32(op));
        out(data, ")+(((S16)");
        out(data, r16(op->r1));
        out(data, ")>>4)*2) & (1 << (");
        out(data, r16(op->r1));
        out(data, " & 15)))); CYCLES(9)");
    } else {
        kpanic("gen1a3");
    }
} 

void OPCALL btr32r32(struct CPU* cpu, struct Op* op);
void OPCALL bte32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL bte32r32_32(struct CPU* cpu, struct Op* op);
void gen3a3(struct GenData* data, struct Op* op) {
    data->lazyFlags = sFLAGS_NONE;
    if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
         out(data, "fillFlagsNoCF(cpu); ");         
    } else {
        out(data, "cpu->lazyFlags = FLAGS_NONE; ");
    }
    if (op->func == btr32r32) {
        out(data, "setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & (1 << (");
        out(data, r32(op->r2));
        out(data, " & 31))); CYCLES(4);");
    } else if (op->func == bte32r32_16) {
        out(data, "setCF(cpu, (readd(cpu->thread, (");
        out(data, getEaa16(op));
        out(data, ")+(((S32)");
        out(data, r32(op->r1));
        out(data, ")>>5)*4) & (1 << (");
        out(data, r32(op->r1));
        out(data, " & 31)))); CYCLES(9)");
    } else if (op->func == bte32r32_32) {
        out(data, "setCF(cpu, (readd(cpu->thread, (");
        out(data, getEaa32(op));
        out(data, ")+(((S32)");
        out(data, r32(op->r1));
        out(data, ")>>5)*4) & (1 << (");
        out(data, r32(op->r1));
        out(data, " & 31)))); CYCLES(9)");
    } else {
        kpanic("gen3a3");
    }
} 

void OPCALL dshlr16r16(struct CPU* cpu, struct Op* op);
void OPCALL dshle16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL dshle16r16_32(struct CPU* cpu, struct Op* op);
void gen1a4(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 10);

    if (op->func == dshlr16r16) {
        out(data, "cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->dst2.u32 = ");
        out(data, r16(op->r2));
        out(data, "; tmp32=(((cpu->dst.u32<<16)|cpu->dst2.u32) << cpu->src.u8)");
        if (op->data1>16) {
            out(data, "|((U32)(");
            out(data, r16(op->r2));
            out(data, ") << (");
            out(data, tmp);
            out(data, " - 16))");
        }
        out(data, "; cpu->result.u16=(U16)(tmp32 >> 16); ");
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; cpu->lazyFlags=FLAGS_DSHL16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHL16;
    } else {
        out(data, "eaa = ");
        if (op->func == dshle16r16_16)
            out(data, getEaa16(op));
        else if (op->func == dshle16r16_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen1a4");
        out(data, "; cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = readw(cpu->thread, eaa); cpu->dst2.u32 = ");
        out(data, r16(op->r1));
        out(data, "; tmp32=(((cpu->dst.u32<<16)|cpu->dst2.u32) << cpu->src.u8)");
        if (op->data1>16) {
            out(data, "|((U32)(");
            out(data, r16(op->r1));
            out(data, ") << (");
            out(data, tmp);
            out(data, " - 16))");
        }
        out(data, "; cpu->result.u16=(U16)(tmp32 >> 16); writew(cpu->thread, eaa, cpu->result.u16); cpu->lazyFlags=FLAGS_DSHL16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHL16;
    }
} 

void OPCALL dshlr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dshle32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL dshle32r32_32(struct CPU* cpu, struct Op* op);
void gen3a4(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 10);

    if (op->func == dshlr32r32) {
        out(data, "cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=(");
        out(data, r32(op->r1));
        out(data, " << ");
        out(data, tmp);
        out(data, ") | (");
        out(data, r32(op->r2));
        out(data, " >> (32 - ");
        out(data, tmp);
        out(data, ")); ");
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; cpu->lazyFlags=FLAGS_DSHL32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHL32;
    } else {
        out(data, "eaa = ");
        if (op->func == dshle32r32_16)
            out(data, getEaa16(op));
        else if (op->func == dshle32r32_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen3a4");
        out(data, "; cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=(cpu->dst.u32 << ");
        out(data, tmp);
        out(data, ") | (");
        out(data, r32(op->r1));
        out(data, " >> (32 - ");
        out(data, tmp);
        out(data, ")); writed(cpu->thread, eaa, cpu->result.u32); cpu->lazyFlags=FLAGS_DSHL32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHL32;
    }
} 

void OPCALL dshlclr16r16(struct CPU* cpu, struct Op* op);
void OPCALL dshlcle16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL dshlcle16r16_32(struct CPU* cpu, struct Op* op);
void gen1a5(struct GenData* data, struct Op* op) {
    if (op->func == dshlclr16r16) {
        out(data, "cpu->src.u32 = CL; cpu->dst.u32 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->dst2.u32 = ");
        out(data, r16(op->r2));
        out(data, "; tmp32=(((cpu->dst.u32<<16)|cpu->dst2.u32) << cpu->src.u8)");
        if (op->data1>16) {
            out(data, "|((U32)(");
            out(data, r16(op->r2));
            out(data, ") << (CL - 16))");
        }
        out(data, "; cpu->result.u16=(U16)(tmp32 >> 16); ");
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; cpu->lazyFlags=FLAGS_DSHL16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHL16;
    } else {
        out(data, "eaa = ");
        if (op->func == dshlcle16r16_16)
            out(data, getEaa16(op));
        else if (op->func == dshlcle16r16_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen1a5");
        out(data, "; cpu->src.u32 = CL; cpu->dst.u32 = readw(cpu->thread, eaa); cpu->dst2.u32 = ");
        out(data, r16(op->r1));
        out(data, "; tmp32=(((cpu->dst.u32<<16)|cpu->dst2.u32) << cpu->src.u8)");
        if (op->data1>16) {
            out(data, "|((U32)(");
            out(data, r16(op->r1));
            out(data, ") << (cpu->src.u32 - 16))");
        }
        out(data, "; cpu->result.u16=(U16)(tmp32 >> 16); writew(cpu->thread, eaa, cpu->result.u16); cpu->lazyFlags=FLAGS_DSHL16; CYCLES(5);");
        data->lazyFlags = sFLAGS_DSHL16;
    }
} 

void OPCALL dshlclr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dshlcle32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL dshlcle32r32_32(struct CPU* cpu, struct Op* op);
void gen3a5(struct GenData* data, struct Op* op) {
    if (op->func == dshlclr32r32) {
        out(data, "cpu->src.u32 = CL; cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=(");
        out(data, r32(op->r1));
        out(data, " << CL) | (");
        out(data, r32(op->r2));
        out(data, " >> (32 - CL)); ");
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; cpu->lazyFlags=FLAGS_DSHL32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHL32;
    } else {
        out(data, "eaa = ");
        if (op->func == dshlcle32r32_16)
            out(data, getEaa16(op));
        else if (op->func == dshlcle32r32_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen3a5");
        out(data, "; cpu->src.u32 = CL; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=(cpu->dst.u32 << CL) | (");
        out(data, r32(op->r1));
        out(data, " >> (32 - CL)); writed(cpu->thread, eaa, cpu->result.u32); cpu->lazyFlags=FLAGS_DSHL32; CYCLES(5);");
        data->lazyFlags = sFLAGS_DSHL32;
    }
}

void OPCALL btsr16r16(struct CPU* cpu, struct Op* op);
void OPCALL btse16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL btse16r16_32(struct CPU* cpu, struct Op* op);
void gen1ab(struct GenData* data, struct Op* op) {
    data->lazyFlags = sFLAGS_NONE;
    if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
         out(data, "fillFlagsNoCF(cpu); ");         
    } else {
        out(data, "cpu->lazyFlags = FLAGS_NONE; ");
    }
    if (op->func == btsr16r16) {
        out(data, "tmp16 = (1 << (");
        out(data, r16(op->r2));
        out(data, " & 15))); setCF(cpu, ");
        out(data, r16(op->r1));
        out(data, " & tmp16); ");
        out(data, r16(op->r1));
        out(data, "|=tmp16; CYCLES(7);");
    } else if (op->func == btse16r16_16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, ")+((S16)");
        out(data, r16(op->r1));
        out(data, ")>>4)*2; tmp16 = (1 << (");
        out(data, r16(op->r1));
        out(data, " & 15))); tmp16_2 = readw(cpu->thread, eaa); setCF(cpu, tmp16_2 & tmp16); writew(cpu->thread, eaa, tmp16_2 | tmp16); CYCLES(13);");
    } else if (op->func == btse16r16_32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, ")+((S16)");
        out(data, r16(op->r1));
        out(data, ")>>4)*2; tmp16 = (1 << (");
        out(data, r16(op->r1));
        out(data, " & 15))); tmp16_2 = readw(cpu->thread, eaa); setCF(cpu, tmp16_2 & tmp16); writew(cpu->thread, eaa, tmp16_2 | tmp16); CYCLES(13);");
    } else {
        kpanic("gen1ab");
    }
} 

void OPCALL btsr32r32(struct CPU* cpu, struct Op* op);
void OPCALL btse32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL btse32r32_32(struct CPU* cpu, struct Op* op);
void gen3ab(struct GenData* data, struct Op* op) {
    data->lazyFlags = sFLAGS_NONE;
    if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
         out(data, "fillFlagsNoCF(cpu); ");         
    } else {
        out(data, "cpu->lazyFlags = FLAGS_NONE; ");
    }
    if (op->func == btsr32r32) {
        out(data, "tmp32 = (1 << (");
        out(data, r32(op->r2));
        out(data, " & 31))); setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & tmp32); ");
        out(data, r32(op->r1));
        out(data, "|=tmp32; CYCLES(7);");
    } else if (op->func == btse32r32_16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "+(((S32)");
        out(data, r32(op->r1));
        out(data, ")>>5)*4; tmp32 = (1 << (");
        out(data, r32(op->r1));
        out(data, " & 31)); tmp32_2 = readd(cpu->thread, eaa); setCF(cpu, tmp32_2 & tmp32); writed(cpu->thread, eaa, tmp32_2 | tmp32); CYCLES(13);");
    } else if (op->func == btse32r32_32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "+(((S32)");
        out(data, r32(op->r1));
        out(data, ")>>5)*4; tmp32 = (1 << (");
        out(data, r32(op->r1));
        out(data, " & 31)); tmp32_2 = readd(cpu->thread, eaa); setCF(cpu, tmp32_2 & tmp32); writed(cpu->thread, eaa, tmp32_2 | tmp32); CYCLES(13);");
    } else {
        kpanic("gen3ab");
    }
} 

void OPCALL dshrr16r16(struct CPU* cpu, struct Op* op);
void OPCALL dshre16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL dshre16r16_32(struct CPU* cpu, struct Op* op);
void gen1ac(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 10);

    if (op->func == dshrr16r16) {
        out(data, "cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = (");
        out(data, r16(op->r1));
        out(data, ")|((U32)(");
        out(data, r16(op->r2));
        out(data, ")<<16); tmp32 = (cpu->dst.u32 >> cpu->src.u8)");
        if (op->data1>16) {
            out(data, " | ((U32)(");
            out(data, r16(op->r2));
            out(data, ") << (32 - ");
            out(data, tmp);
            out(data, "))");
        }
        out(data, "; cpu->result.u16=(U16)tmp32;");
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; cpu->lazyFlags=FLAGS_DSHR16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR16;
    } else {
        out(data, "eaa = ");
        if (op->func == dshre16r16_16)
            out(data, getEaa16(op));
        else if (op->func == dshre16r16_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen1ac");
        out(data, "; cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = readw(cpu->thread, eaa) | ((U32)(");
        out(data, r16(op->r1));
        out(data, ")<<16); tmp32=(cpu->dst.u32 >> cpu->src.u8)");
        if (op->data1>16) {
            out(data, " | ((U32)(");
            out(data, r16(op->r1));
            out(data, ") << (32 - ");
            out(data, tmp);
            out(data, "))");
        }
        out(data, ";cpu->result.u16=(U16)result; writew(cpu->thread, address, cpu->result.u16); cpu->lazyFlags=FLAGS_DSHR16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR16;
    }
} 

void OPCALL dshrr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dshre32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL dshre32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL dshrr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dshre32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dshre32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen3ac(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 10);

    if (op->func == dshrr32r32) {
        out(data, "cpu->src.u32 = ");
        out(data, tmp);
        out(data, "; cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=(");
        out(data, r32(op->r1));
        out(data, " >> ");
        out(data, tmp);
        out(data, ") | (");
        out(data, r32(op->r2));
        out(data, " << (32 - ");
        out(data, tmp);
        out(data, ")); ");
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; cpu->lazyFlags=FLAGS_DSHR32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR32;
    } else if (op->func == dshrr32r32_noflags) {
        out(data, r32(op->r1));
        out(data, " = (");
        out(data, r32(op->r1));
        out(data, " >> ");
        out(data, tmp);
        out(data, ") | (");
        out(data, r32(op->r2));
        out(data, " << (32 - ");
        out(data, tmp);
        out(data, ")); CYCLES(4);");
    } else if (op->func == dshre32r32_16_noflags || op->func == dshre32r32_32_noflags) {
        out(data, "eaa = ");
        if (op->func == dshre32r32_16_noflags)
            out(data, getEaa16(op));
        else
            out(data, getEaa32(op));
        out(data, "writed(cpu->thread, eaa, (readd(cpu->thread, eaa) >> ");
        out(data, tmp);
        out(data, ") | (");
        out(data, r32(op->r1));
        out(data, " << (32 - ");
        out(data, tmp);
        out(data, "))); CYCLES(4);");
    } else {
        out(data, "eaa = ");
        if (op->func == dshre32r32_16)
            out(data, getEaa16(op));
        else if (op->func == dshre32r32_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen3ac");
        out(data, "; cpu->src.u32 = ");
        out(data, tmp);
        out(data, ";  cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=(cpu->dst.u32 >> ");
        out(data, tmp);
        out(data, ") | (");
        out(data, r32(op->r1));
        out(data, " << (32 - ");
        out(data, tmp);
        out(data, ")); writed(cpu->thread, eaa, cpu->result.u32); cpu->lazyFlags=FLAGS_DSHR32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR32;
    }
} 

void OPCALL dshrclr16r16(struct CPU* cpu, struct Op* op);
void OPCALL dshrcle16r16_16(struct CPU* cpu, struct Op* op);
void OPCALL dshrcle16r16_32(struct CPU* cpu, struct Op* op);
void gen1ad(struct GenData* data, struct Op* op) {
    if (op->func == dshrclr16r16) {
        out(data, "cpu->src.u32 = CL; cpu->dst.u32 = (");
        out(data, r16(op->r1));
        out(data, ")|((U32)(");
        out(data, r16(op->r2));
        out(data, ")<<16); tmp32 = (cpu->dst.u32 >> cpu->src.u8)");
        if (op->data1>16) {
            out(data, " | ((U32)(");
            out(data, r16(op->r2));
            out(data, ") << (32 - CL))");
        }
        out(data, "; cpu->result.u16=(U16)tmp32;");
        out(data, r16(op->r1));
        out(data, " = cpu->result.u16; cpu->lazyFlags=FLAGS_DSHR16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR16;
    } else {
        out(data, "eaa = ");
        if (op->func == dshrcle16r16_16)
            out(data, getEaa16(op));
        else if (op->func == dshrcle16r16_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen1ac");
        out(data, "; cpu->src.u32 = CL; cpu->dst.u32 = readw(cpu->thread, eaa) | ((U32)(");
        out(data, r16(op->r1));
        out(data, ")<<16); tmp32=(cpu->dst.u32 >> cpu->src.u8)");
        if (op->data1>16) {
            out(data, " | ((U32)(");
            out(data, r16(op->r1));
            out(data, ") << (32 - CL))");
        }
        out(data, ";cpu->result.u16=(U16)result; writew(cpu->thread, address, cpu->result.u16); cpu->lazyFlags=FLAGS_DSHR16; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR16;
    }
} 

void OPCALL dshrclr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dshrcle32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL dshrcle32r32_32(struct CPU* cpu, struct Op* op);
void gen3ad(struct GenData* data, struct Op* op) {
    if (op->func == dshrclr32r32) {
        out(data, "cpu->src.u32 = CL; cpu->dst.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->result.u32=(");
        out(data, r32(op->r1));
        out(data, " >> CL) | (");
        out(data, r32(op->r2));
        out(data, " << (32 - CL)); ");
        out(data, r32(op->r1));
        out(data, " = cpu->result.u32; cpu->lazyFlags=FLAGS_DSHR32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR32;
    } else {
        out(data, "eaa = ");
        if (op->func == dshrcle32r32_16)
            out(data, getEaa16(op));
        else if (op->func == dshrcle32r32_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen3ad");
        out(data, "; cpu->src.u32 = CL;  cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32=(cpu->dst.u32 >> CL) | (");
        out(data, r32(op->r1));
        out(data, " << (32 - CL)); writed(cpu->thread, eaa, cpu->result.u32); cpu->lazyFlags=FLAGS_DSHR32; CYCLES(4);");
        data->lazyFlags = sFLAGS_DSHR32;
    }
}

void OPCALL dimulr16r16(struct CPU* cpu, struct Op* op);
void OPCALL dimulr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL dimulr16e16_32(struct CPU* cpu, struct Op* op);
void gen1af(struct GenData* data, struct Op* op) {
    if (op->func == dimulr16r16) {
        out(data, "tmps32 = (S16)(");
        out(data, r16(op->r2));
        out(data, ") * (S32)");
        out(data, r16(op->r1));
        out(data, "; fillFlagsNoCFOF(cpu); if ((tmps32 >= -32767) && (tmps32 <= 32767)) {removeFlag(CF|OF);} else {addFlag(CF|OF);}");
        data->lazyFlags = sFLAGS_NONE;
        out(data, r16(op->r1));
        out(data, " = tmps32; CYCLES(10);");
    } else {
        out(data, "tmps32 = (S16)(readw(cpu->thread, ");
        if (op->func == dimulr16e16_16)
            out(data, getEaa16(op));
        else if (op->func == dimulr16e16_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen1af");

        out(data, ")) * (S32)");
        out(data, r16(op->r1));
        out(data, "; fillFlagsNoCFOF(cpu); if ((tmps32 >= -32767) && (tmps32 <= 32767)) {removeFlag(CF|OF);} else {addFlag(CF|OF);}");
        data->lazyFlags = sFLAGS_NONE;
        out(data, r16(op->r1));
        out(data, " = tmps32; CYCLES(10);");
    }
}

void OPCALL dimulr32r32(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL dimulr32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen3af(struct GenData* data, struct Op* op) {
    if (op->func == dimulr32r32) {
        out(data, "tmps64 = (S32)(");
        out(data, r32(op->r2));
        out(data, ") * (S64)");
        out(data, r32(op->r1));
        out(data, "; fillFlagsNoCFOF(cpu); if ((tmps64 >= -2147483647l) && (tmps64 <= 2147483647l)) {removeFlag(CF|OF);} else {addFlag(CF|OF);}");
        data->lazyFlags = sFLAGS_NONE;
        out(data, r32(op->r1));
        out(data, " = (S32)tmps64; CYCLES(10);");
    } else if (op->func == dimulr32r32_noflags) {
        out(data, r32(op->r1));
        out(data, " = (S32)(");
        out(data, r32(op->r2));
        out(data, ") * ((S32)");
        out(data, r32(op->r1));
        out(data, ");CYCLES(10);");
    } else if (op->func == dimulr32e32_16_noflags || op->func == dimulr32e32_32_noflags) {
        out(data, r32(op->r1));
        out(data, "= (S32)(readd(cpu->thread, ");
        if (op->func == dimulr32e32_16_noflags)
            out(data, getEaa16(op));
        else
            out(data, getEaa32(op));
        out(data, ")) * ((S32)");
        out(data, r32(op->r1));
        out(data, "); CYCLES(10);");
    } else {
        out(data, "tmps64 = (S32)(readw(cpu->thread, ");
        if (op->func == dimulr32e32_16)
            out(data, getEaa16(op));
        else if (op->func == dimulr32e32_32) 
            out(data, getEaa32(op));
        else
            kpanic("gen3af");

        out(data, ")) * (S64)");
        out(data, r32(op->r1));
        out(data, "; fillFlagsNoCFOF(cpu); if ((tmps64 >= -2147483647l) && (tmps64 <= 2147483647l)) {removeFlag(CF|OF);} else {addFlag(CF|OF);}");
        data->lazyFlags = sFLAGS_NONE;
        out(data, r32(op->r1));
        out(data, " = (S32)tmps64; CYCLES(10);");
    }
}

void OPCALL cmpxchgr16r16(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchgr16r16(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge16r16_32(struct CPU* cpu, struct Op* op);
void gen1b1(struct GenData* data, struct Op* op) {
    if (op->func == cmpxchgr16r16) {
        out(data, "cpu->src.u16 = ");
        out(data, r16(op->r1));
        out(data, "; cpu->dst.u16 = AX; cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_CMP16; if (AX == cpu->src.u16) {");        
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, "; } else {AX = cpu->src.u16;} CYCLES(5);");
    } else if (op->func == cmpxchgr16r16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->src.u16 = readw(cpu->thread, eaa); cpu->dst.u16 = AX; cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_CMP16; if (AX == cpu->src.u16) {writew(cpu->thread, eaa, ");
        out(data, r16(op->r1));
        out(data, ");} else {AX = cpu->dst.u16;} CYCLES(6);");
    } else if (op->func == cmpxchge16r16_32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->src.u16 = readw(cpu->thread, eaa); cpu->dst.u16 = AX; cpu->result.u16 = cpu->dst.u16 - cpu->src.u16; cpu->lazyFlags = FLAGS_CMP16; if (AX == cpu->src.u16) {writew(cpu->thread, eaa, ");
        out(data, r16(op->r1));
        out(data, ");} else {AX = cpu->src.u16;} CYCLES(6);");
    }
    data->lazyFlags = sFLAGS_CMP16;
}

void OPCALL cmpxchgr32r32(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge32r32_32(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchgr32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge32r32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL cmpxchge32r32_32_noflags(struct CPU* cpu, struct Op* op);
void gen3b1(struct GenData* data, struct Op* op) {
    if (op->func == cmpxchgr32r32) {
        out(data, "cpu->src.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->dst.u32 = EAX; cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_CMP32; if (EAX == cpu->src.u32) {");
        data->lazyFlags = sFLAGS_CMP32;
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} else {EAX = cpu->src.u32;} CYCLES(5);");
    } else if (op->func == cmpxchgr32r32_noflags) {
        out(data, "if (EAX == ");
        out(data, r32(op->r1));
        out(data, ") {");
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r32(op->r2));
        out(data, ";} else {EAX = ");
        out(data, r32(op->r1));
        out(data, ";} CYCLES(5);");
    } else if (op->func == cmpxchge32r32_16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->src.u32 = readd(cpu->thread, eaa); cpu->dst.u32 = EAX; cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_CMP32; if (EAX == cpu->src.u32) { writed(cpu->thread, eaa, ");
        data->lazyFlags = sFLAGS_CMP32;
        out(data, r32(op->r1));
        out(data, "); } else {EAX = cpu->src.u32;} CYCLES(6);");
    } else if (op->func == cmpxchge32r32_16_noflags) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); if (EAX == tmp32) {writed(cpu->thread, eaa, ");
        out(data, r32(op->r1));
        out(data, ");} else {EAX = tmp32;} CYCLES(6);");
    } else if (op->func == cmpxchge32r32_32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->src.u32 = readd(cpu->thread, eaa); cpu->dst.u32 = EAX; cpu->result.u32 = cpu->dst.u32 - cpu->src.u32; cpu->lazyFlags = FLAGS_CMP32; if (EAX == cpu->src.u32) { writed(cpu->thread, eaa, ");
        data->lazyFlags = sFLAGS_CMP32;
        out(data, r32(op->r1));
        out(data, "); } else {EAX = cpu->src.u32;} CYCLES(6);");
    } else if (op->func == cmpxchge32r32_32_noflags) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); if (EAX == tmp32) {writed(cpu->thread, eaa, ");
        out(data, r32(op->r1));
        out(data, ");} else {EAX = tmp32;} CYCLES(6);");
    } 
}

void OPCALL movxz8r16r16(struct CPU* cpu, struct Op* op);
void OPCALL movxz8r16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL movxz8r16e16_32(struct CPU* cpu, struct Op* op);
void gen1b6(struct GenData* data, struct Op* op) {
    if (op->func == movxz8r16r16) {
        out(data, r16(op->r1));
        out(data, " = ");
        out(data, r8(op->r2));
        out(data, "; CYCLES(3);");
    } else if (op->func == movxz8r16e16_16) {
        out(data, r16(op->r1));
        out(data, " = readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(3);");
    } else if (op->func == movxz8r16e16_32) {
        out(data, r16(op->r1));
        out(data, " = readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(3);");
    } else {
        kpanic("gen1b6");
    }
}

void OPCALL movxz8r32r32(struct CPU* cpu, struct Op* op);
void OPCALL movxz8r32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL movxz8r32e32_32(struct CPU* cpu, struct Op* op);
void gen3b6(struct GenData* data, struct Op* op) {
    if (op->func == movxz8r32r32) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r8(op->r2));
        out(data, "; CYCLES(3);");
    } else if (op->func == movxz8r32e32_16) {
        out(data, r32(op->r1));
        out(data, " = readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(3);");
    } else if (op->func == movxz8r32e32_32) {
        out(data, r32(op->r1));
        out(data, " = readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(3);");
    } else {
        kpanic("gen3b6");
    }
}

void OPCALL movxz16r32r32(struct CPU* cpu, struct Op* op);
void OPCALL movxz16r32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL movxz16r32e32_32(struct CPU* cpu, struct Op* op);
void gen3b7(struct GenData* data, struct Op* op) {
    if (op->func == movxz16r32r32) {
        out(data, r32(op->r1));
        out(data, " = ");
        out(data, r16(op->r2));
        out(data, "; CYCLES(3);");
    } else if (op->func == movxz16r32e32_16) {
        out(data, r32(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(3);");
    } else if (op->func == movxz16r32e32_32) {
        out(data, r32(op->r1));
        out(data, " = readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(3);");
    } else {
        kpanic("gen3b7");
    }
}

void OPCALL bt_reg(struct CPU* cpu, struct Op* op);
void OPCALL bt_mem16(struct CPU* cpu, struct Op* op);
void OPCALL bt_mem32(struct CPU* cpu, struct Op* op);
void OPCALL bts_reg(struct CPU* cpu, struct Op* op);
void OPCALL bts_mem16(struct CPU* cpu, struct Op* op);
void OPCALL bts_mem32(struct CPU* cpu, struct Op* op);
void OPCALL btr_reg(struct CPU* cpu, struct Op* op);
void OPCALL btr_mem16(struct CPU* cpu, struct Op* op);
void OPCALL btr_mem32(struct CPU* cpu, struct Op* op);
void OPCALL btc_reg(struct CPU* cpu, struct Op* op);
void OPCALL btc_mem16(struct CPU* cpu, struct Op* op);
void OPCALL btc_mem32(struct CPU* cpu, struct Op* op);
void gen3ba(struct GenData* data, struct Op* op) {
    char tmp[16];
    itoa(op->data1, tmp, 16);
    if (op->func == bt_reg) {       
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & 0x");
        out(data, tmp);
        out(data, "); CYCLES(4);");
    } else if (op->func == bt_mem16) {  
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "setCF(cpu, readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ") & 0x");
        out(data, tmp);
        out(data, "); CYCLES(4);");
    } else if (op->func == bt_mem32) {
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "setCF(cpu, readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ") & 0x");
        out(data, tmp);
        out(data, "); CYCLES(4);");
    } else if (op->func == bts_reg) {  
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & 0x");
        out(data, tmp);
        out(data, "); ");
        out(data, r32(op->r1));
        out(data, " |= 0x");
        out(data, tmp);
        out(data, "; CYCLES(7);");
    } else if (op->func == bts_mem16) {      
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "eaa = ");
        getEaa16(op);
        out(data, ";tmp32 = readd(cpu->thread, eaa); setCF(cpu, tmp32 & 0x");
        out(data, tmp);
        out(data, "); writed(cpu->thread, eaa, value | 0x");
        out(data, tmp);
        out(data, "); CYCLES(8);");
    } else if (op->func == bts_mem32) {
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "eaa = ");
        getEaa32(op);
        out(data, ";tmp32 = readd(cpu->thread, eaa); setCF(cpu, tmp32 & 0x");
        out(data, tmp);
        out(data, "); writed(cpu->thread, eaa, value | 0x");
        out(data, tmp);
        out(data, "); CYCLES(8);");
    } else if (op->func == btr_reg) {        
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & 0x");
        out(data, tmp);
        out(data, "); ");
        out(data, r32(op->r1));
        out(data, " &= ~0x");
        out(data, tmp);
        out(data, "; CYCLES(7);");
    } else if (op->func == btr_mem16) {   
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "eaa = ");
        getEaa16(op);
        out(data, ";tmp32 = readd(cpu->thread, eaa); setCF(cpu, tmp32 & 0x");
        out(data, tmp);
        out(data, "); writed(cpu->thread, eaa, value & ~0x");
        out(data, tmp);
        out(data, "); CYCLES(8);");
    } else if (op->func == btr_mem32) {
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "eaa = ");
        getEaa32(op);
        out(data, ";tmp32 = readd(cpu->thread, eaa); setCF(cpu, tmp32 & 0x");
        out(data, tmp);
        out(data, "); writed(cpu->thread, eaa, value & ~0x");
        out(data, tmp);
        out(data, "); CYCLES(8);");
    } else if (op->func == btc_reg) {   
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & 0x");
        out(data, tmp);
        out(data, "); ");
        out(data, r32(op->r1));
        out(data, " ^= 0x");
        out(data, tmp);
        out(data, "; CYCLES(7);");
    } else if (op->func == btc_mem16) {      
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "eaa = ");
        getEaa16(op);
        out(data, ";tmp32 = readd(cpu->thread, eaa); setCF(cpu, tmp32 & 0x");
        out(data, tmp);
        out(data, "); writed(cpu->thread, eaa, value ^ 0x");
        out(data, tmp);
        out(data, "); CYCLES(8);");
    } else if (op->func == btc_mem32) {
        data->lazyFlags = sFLAGS_NONE;
        if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
            out(data, "fillFlagsNoCF(cpu);");            
        } else {
            out(data, "cpu->lazyFlags = FLAGS_NONE; ");
        }
        out(data, "eaa = ");
        getEaa32(op);
        out(data, ";tmp32 = readd(cpu->thread, eaa); setCF(cpu, tmp32 & 0x");
        out(data, tmp);
        out(data, "); writed(cpu->thread, eaa, value ^ 0x");
        out(data, tmp);
        out(data, "); CYCLES(8);");
    } else {
        kpanic("gen3ba");
    }
}

void OPCALL btcr32r32(struct CPU* cpu, struct Op* op);
void OPCALL btce32r32_16(struct CPU* cpu, struct Op* op);
void OPCALL btce32r32_32(struct CPU* cpu, struct Op* op);
void gen3bb(struct GenData* data, struct Op* op) {
    data->lazyFlags = sFLAGS_NONE;
    if (needsToSetFlag(data->cpu, data->block, data->eip, op, ZF|SF|OF|PF|AF, NULL)) {
        out(data, "fillFlagsNoCF(cpu);");        
    } else {
        out(data, "cpu->lazyFlags = FLAGS_NONE; ");
    }
    if (op->func == btcr32r32) {        
        out(data, "tmp32 = 1 << ");
        out(data, r32(op->r2));
        out(data, " & 31; setCF(cpu, ");
        out(data, r32(op->r1));
        out(data, " & tmp32); ");
        out(data, r32(op->r1));
        out(data, " ^= tmp32; CYCLES(7);");
    } else if (op->func == btce32r32_16) {        
        out(data, "tmp32 = 1 << ");
        out(data, r32(op->r1));
        out(data, " & 31; eaa = ");
        out(data, getEaa16(op));
        out(data, "+((((S32)");
        out(data, r32(op->r1));
        out(data, ")>>3) & ~3); tmp32_2 = readd(cpu->thread, eaa); setCF(cpu, tmp32_2 & tmp32); writed(cpu->thread, eaa, tmp32_2 ^ tmp32); CYCLES(13);");
    } else if (op->func == btce32r32_32) {
        out(data, "tmp32 = 1 << ");
        out(data, r32(op->r1));
        out(data, " & 31; eaa = ");
        out(data, getEaa32(op));
        out(data, "+((((S32)");
        out(data, r32(op->r1));
        out(data, ")>>3) & ~3); tmp32_2 = readd(cpu->thread, eaa); setCF(cpu, tmp32_2 & tmp32); writed(cpu->thread, eaa, tmp32_2 ^ tmp32); CYCLES(13);");
    } else {
        kpanic("gen3bb");
    }
}

void OPCALL bsrr16r16(struct CPU* cpu, struct Op* op);
void OPCALL bsrr16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL bsrr16e16_32(struct CPU* cpu, struct Op* op);
void gen1bd(struct GenData* data, struct Op* op) {
    out(data, "tmp16 = ");
    if (op->func == bsrr16r16) {        
        out(data, r16(op->r2));        
    } else if (op->func == bsrr16e16_16) {        
        out(data, "readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")");
    } else if (op->func == bsrr16e16_32) {
        out(data, "readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")");
    } else {
        kpanic("gen3bb");
    }
    out(data, "; if (tmp16==0) {addFlag(ZF);} else {tmp32 = 15; while ((tmp16 & 0x8000)==0) { tmp32--; tmp16<<=1; } removeFlag(ZF); ");
    out(data, r16(op->r1));
    out(data, " = result;} cpu->lazyFlags = FLAGS_NONE; CYCLES(7);");
    data->lazyFlags = sFLAGS_NONE;
}

void OPCALL bsfr32r32(struct CPU* cpu, struct Op* op);
void OPCALL bsfr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL bsfr32e32_32(struct CPU* cpu, struct Op* op);
void gen3bc(struct GenData* data, struct Op* op) {
    data->lazyFlags = sFLAGS_NONE;
    if (needsToSetFlag(data->cpu, data->block, data->eip, op, CF|SF|OF|PF|AF, NULL)) {
        out(data, "fillFlagsNoZF(cpu);");        
    } else {
        out(data, "cpu->lazyFlags = FLAGS_NONE; ");
    }
    out(data, "tmp32 = ");
    if (op->func == bsfr32r32) {                
        out(data, r32(op->r2));        
    } else if (op->func == bsfr32e32_16) {        
        out(data, "readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")");
    } else if (op->func == bsfr32e32_32) {
        out(data, "readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")");
    } else {
        kpanic("gen3bc");
    }    
    out(data, "; if (tmp32==0) {addFlag(ZF);} else {tmp32_2 = 0; while ((tmp32 & 0x01)==0) { tmp32_2++; tmp32>>=1; } removeFlag(ZF); ");
    out(data, r32(op->r1));
    out(data, "= tmp32_2;} CYCLES(6);");
}

void OPCALL bsrr32r32(struct CPU* cpu, struct Op* op);
void OPCALL bsrr32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL bsrr32e32_32(struct CPU* cpu, struct Op* op);
void gen3bd(struct GenData* data, struct Op* op) {
    out(data, "tmp32 = ");
    if (op->func == bsrr32r32) {                
        out(data, r32(op->r2));        
    } else if (op->func == bsrr32e32_16) {        
        out(data, "readd(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, ")");
    } else if (op->func == bsrr32e32_32) {
        out(data, "readd(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, ")");
    } else {
        kpanic("gen3bd");
    }    
    out(data, "; if (tmp32==0) {addFlag(ZF);} else {tmp32_2 = 31; while ((tmp32 & 0x80000000)==0) { tmp32_2--; tmp32<<=1; } removeFlag(ZF); ");
    out(data, r32(op->r1));
    out(data, " = tmp32_2; } cpu->lazyFlags = FLAGS_NONE; CYCLES(7);");
    data->lazyFlags = sFLAGS_NONE;
}

void OPCALL movsx8r16r16(struct CPU* cpu, struct Op* op);
void OPCALL movsx8r16e16_16(struct CPU* cpu, struct Op* op);
void OPCALL movsx8r16e16_32(struct CPU* cpu, struct Op* op);
void gen1be(struct GenData* data, struct Op* op) {

    if (op->func == movsx8r16r16) {        
        out(data, r16(op->r1));
        out(data, " = (S8)");
        out(data, r8(op->r2));
        out(data, "; CYCLES(3);");	      
    } else if (op->func == movsx8r16e16_16) {        
        out(data, r16(op->r1));
        out(data, " = (S8)readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(3);");	      
    } else if (op->func == movsx8r16e16_32) {
        out(data, r16(op->r1));
        out(data, " = (S8)readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(3);");	      
    } else {
        kpanic("gen1be");
    }
}

void OPCALL movsx8r32r32(struct CPU* cpu, struct Op* op);
void OPCALL movsx8r32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL movsx8r32e32_32(struct CPU* cpu, struct Op* op);
void gen3be(struct GenData* data, struct Op* op) {
    if (op->func == movsx8r32r32) {        
        out(data, r32(op->r1));
        out(data, " = (S8)");
        out(data, r8(op->r2));
        out(data, "; CYCLES(3);");	      
    } else if (op->func == movsx8r32e32_16) {        
        out(data, r32(op->r1));
        out(data, " = (S8)readb(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(3);");	      
    } else if (op->func == movsx8r32e32_32) {
        out(data, r32(op->r1));
        out(data, " = (S8)readb(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(3);");	      
    } else {
        kpanic("gen3be");
    }
}

void OPCALL movsx16r32r32(struct CPU* cpu, struct Op* op);
void OPCALL movsx16r32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL movsx16r32e32_32(struct CPU* cpu, struct Op* op);
void gen3bf(struct GenData* data, struct Op* op) {
    if (op->func == movsx16r32r32) {        
        out(data, r32(op->r1));
        out(data, " = (S16)");
        out(data, r16(op->r2));
        out(data, "; CYCLES(3);");	      
    } else if (op->func == movsx16r32e32_16) {        
        out(data, r32(op->r1));
        out(data, " = (S16)readw(cpu->thread, ");
        out(data, getEaa16(op));
        out(data, "); CYCLES(3);");	      
    } else if (op->func == movsx16r32e32_32) {
        out(data, r32(op->r1));
        out(data, " = (S16)readw(cpu->thread, ");
        out(data, getEaa32(op));
        out(data, "); CYCLES(3);");	      
    } else {
        kpanic("gen3bf");
    }
}

void OPCALL xadd32r32r32(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32e32_16(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32e32_32(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32r32_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32e32_16_noflags(struct CPU* cpu, struct Op* op);
void OPCALL xadd32r32e32_32_noflags(struct CPU* cpu, struct Op* op);
void gen3c1(struct GenData* data, struct Op* op) {
    if (op->func == xadd32r32r32) {        
        out(data, "cpu->src.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->dst.u32 = ");
        out(data, r32(op->r2));
        out(data, "; cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32; ");
        data->lazyFlags = sFLAGS_ADD32;
        out(data, r32(op->r1));
        out(data, " = cpu->dst.u32; ");
        out(data, r32(op->r2));
        out(data, " =  cpu->result.u32; CYCLES(3);");
    } else if (op->func == xadd32r32r32_noflags) {        
      out(data, "tmp32 = ");
      out(data, r32(op->r2));
      out(data, "; ");
      out(data, r32(op->r2));
      out(data, " = ");
      out(data, r32(op->r1));
      out(data, " + ");
      out(data, r32(op->r2));
      out(data, "; ");
      out(data, r32(op->r1));
      out(data, " = tmp32; CYCLES(3);");
    } else if (op->func == xadd32r32e32_32) {
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; cpu->src.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32; ");
        data->lazyFlags = sFLAGS_ADD32;
        out(data, r32(op->r1));
        out(data, " = cpu->dst.u32;  writed(cpu->thread, eaa, cpu->result.u32); CYCLES(4);");
    } else if (op->func == xadd32r32e32_32_noflags) {        
        out(data, "eaa = ");
        out(data, getEaa32(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, tmp32 + ");
        out(data, r32(op->r1));
        out(data, "); ");
        out(data, r32(op->r1));
        out(data, " = tmp32; CYCLES(4);");
    } else if (op->func == xadd32r32e32_16) {
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; cpu->src.u32 = ");
        out(data, r32(op->r1));
        out(data, "; cpu->dst.u32 = readd(cpu->thread, eaa); cpu->result.u32 = cpu->dst.u32 + cpu->src.u32; cpu->lazyFlags = FLAGS_ADD32; ");
        data->lazyFlags = sFLAGS_ADD32;
        out(data, r32(op->r1));
        out(data, " = cpu->dst.u32;  writed(cpu->thread, eaa, cpu->result.u32); CYCLES(4);");
    } else if (op->func == xadd32r32e32_16_noflags) {        
        out(data, "eaa = ");
        out(data, getEaa16(op));
        out(data, "; tmp32 = readd(cpu->thread, eaa); writed(cpu->thread, eaa, tmp32 + ");
        out(data, r32(op->r1));
        out(data, "); ");
        out(data, r32(op->r1));
        out(data, " = tmp32; CYCLES(4);");
    } else {
        kpanic("gen3c1");
    }
}

void OPCALL cmpxchgg8b_16(struct CPU* cpu, struct Op* op);
void gen3c7(struct GenData* data, struct Op* op) {
    out(data, "eaa = ");
    if (op->func == cmpxchgg8b_16) {
        out(data, getEaa16(op));
    } else {
        out(data, getEaa32(op));
    }
    out(data, "; tmp64 = readq(cpu->thread, eaa); fillFlags(cpu); if (tmp64 == (((U64)EDX) << 32 | EAX)) { addFlag(ZF); writed(cpu->thread, eaa, EBX); writed(cpu->thread, eaa + 4, ECX); } else { removeFlag(ZF); EDX = (U32)(tmp64 >> 32); EAX = (U32)tmp64; } CYCLES(10);");
}

void gen3c8(struct GenData* data, struct Op* op) {
    out(data, "tmp32 = ");
    out(data, r32(op->r1));
    out(data, "; ");
    out(data, r32(op->r1));
    out(data, " = (((tmp32 & 0xff000000) >> 24) | ((tmp32 & 0x00ff0000) >>  8) | ((tmp32 & 0x0000ff00) <<  8) | ((tmp32 & 0x000000ff) << 24)); CYCLES(1);");
}

void gen400(struct GenData* data, struct Op* op) {
    struct Op* first;
    U32 blockCount = getBlockEipCount(data->block);

    data->block = data->block->block1;
    first = data->block->ops;    
    if (first->func == firstOp) {
        data->block->ops = data->block->ops->next;
        first->next = 0;
        freeOp(first);
    }

    data->eip+=blockCount;
    jit(data->cpu, data->block, data->eip);    
    writeBlock(data, data->block);
    addBlockToData(data, data->block);
}

U32 getTestLeftRight(struct Op* op, char* left, char* right) {
    U32 same = 0;

    if (op->func == testr8r8) {
        strcpy(left, r8(op->r1));
        strcpy(right, r8(op->r2));
        if (op->r1 == op->r2)
            same = 1;
    } else if (op->func == teste8r8_16) {
        strcpy(left, "readb(cpu->thread, ");
        strcat(left, getEaa16(op));
        strcat(left, ")");
        strcpy(right, r8(op->r1));
    } else if (op->func == teste8r8_32) {
        strcpy(left, "readb(cpu->thread, ");
        strcat(left, getEaa32(op));
        strcat(left, ")");
        strcpy(right, r8(op->r1));
    } else if (op->func == test8_reg) {
        strcpy(left, r8(op->r1));
        strcpy(right, "0x");
        itoa(op->data1 & 0xFF, right+2, 16);
    } else if (op->func == test8_mem16) {
        strcpy(left, "readb(cpu->thread, ");
        strcat(left, getEaa16(op));
        strcat(left, ")");
        strcpy(right, "0x");
        itoa(op->data1 & 0xFF, right+2, 16);
    } else if (op->func == test8_mem32) {
        strcpy(left, "readb(cpu->thread, ");
        strcat(left, getEaa32(op));
        strcat(left, ")");
        strcpy(right, "0x");
        itoa(op->data1 & 0xFF, right+2, 16);
    } else if (op->func == testr16r16) {
        strcpy(left, r16(op->r1));
        strcpy(right, r16(op->r2));
        if (op->r1 == op->r2)
            same = 1;
    } else if (op->func == teste16r16_16) {
        strcpy(left, "readw(cpu->thread, ");
        strcat(left, getEaa16(op));
        strcat(left, ")");
        strcpy(right, r16(op->r1));
    } else if (op->func == teste16r16_32) {
        strcpy(left, "readw(cpu->thread, ");
        strcat(left, getEaa32(op));
        strcat(left, ")");
        strcpy(right, r16(op->r1));
    } else if (op->func == test16_reg) {
        strcpy(left, r16(op->r1));
        strcpy(right, "0x");
        itoa(op->data1 & 0xFFFF, right+2, 16);
    } else if (op->func == test16_mem16) {
        strcpy(left, "readw(cpu->thread, ");
        strcat(left, getEaa16(op));
        strcat(left, ")");
        strcpy(right, "0x");
        itoa(op->data1 & 0xFFFF, right+2, 16);
    } else if (op->func == test16_mem32) {
        strcpy(left, "readw(cpu->thread, ");
        strcat(left, getEaa32(op));
        strcat(left, ")");
        strcpy(right, "0x");
        itoa(op->data1 & 0xFFFF, right+2, 16);
    } else if (op->func == testr32r32) {
        strcpy(left, r32(op->r1));
        strcpy(right, r32(op->r2));
        if (op->r1 == op->r2)
            same = 1;
    } else if (op->func == teste32r32_16) {
        strcpy(left, "readd(cpu->thread, ");
        strcat(left, getEaa16(op));
        strcat(left, ")");
        strcpy(right, r32(op->r1));
    } else if (op->func == teste32r32_32) {
        strcpy(left, "readd(cpu->thread, ");
        strcat(left, getEaa32(op));
        strcat(left, ")");
        strcpy(right, r32(op->r1));
    } else if (op->func == test32_reg) {
        strcpy(left, r32(op->r1));
        strcpy(right, "0x");
        itoa(op->data1, right+2, 16);
    } else if (op->func == test32_mem16) {
        strcpy(left, "readd(cpu->thread, ");
        strcat(left, getEaa16(op));
        strcat(left, ")");
        strcpy(right, "0x");
        itoa(op->data1, right+2, 16);
    } else if (op->func == test32_mem32) {
        strcpy(left, "readd(cpu->thread, ");
        strcat(left, getEaa32(op));
        strcat(left, ")");
        strcpy(right, "0x");
        itoa(op->data1, right+2, 16);
    } else {
        kpanic("getTestLeftRight: unknown op");
    }
    return same;
}

char tmpCondition[256];
const char* getCondition(struct GenData* data, int condition) {
    if (data->inlinedLazyFlagOp) {
        if (data->lazyFlags == sFLAGS_TEST8 || data->lazyFlags == sFLAGS_TEST16 || data->lazyFlags == sFLAGS_TEST32) {
            char left[256];
            char right[256];
            U32 same = 0;
            const char* signMask;
            const char* signCast;
            const char* src;
            const char* dst;

            if (data->lazyFlags == sFLAGS_TEST8) {
                signMask = "0x80";
                signCast = "(S8)";
                src = "cpu->src.u8";
                dst = "cpu->dst.u8";
            } else if (data->lazyFlags == sFLAGS_TEST16) {
                signMask = "0x8000";
                signCast = "(S16)";
                src = "cpu->src.u16";
                dst = "cpu->dst.u16";
            } else if (data->lazyFlags == sFLAGS_TEST32) {
                signMask = "0x80000000";
                signCast = "(S32)";
                src = "cpu->src.u32";
                dst = "cpu->dst.u32";
            } else {
                kpanic("getCondition: oops");
            }

            same = getTestLeftRight(data->inlinedLazyFlagOp, left, right);

            if (data->inlinedLazyFlagOpAssignedToCPU) {
                strcpy(left, dst);
                strcpy(right, src);
            }
            data->inlinedLazyFlagOp = 0;

            switch (condition) {                    
                case 0: // OF
                    return "0";                    
                case 1: // !OF
                    return "1";                    
                case 2: // CF
                    return "0";                    
                case 3: // !CF
                    return "1";                    
                case 4: // ZF                    
                case 6: // ZF or CF
                    if (same) {
                        strcpy(tmpCondition, "!"); strcat(tmpCondition, left); return tmpCondition;                        
                    }                    
                    strcpy(tmpCondition, "!("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ")"); return tmpCondition;                    
                case 5: // !ZF
                case 7: // !ZF and !CF
                    if (same) {
                        strcpy(tmpCondition, left); return tmpCondition;
                    }
                    strcpy(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); return tmpCondition;
                case 8: // SF
                case 12: // SF != OF
                    if (same) {
                        strcpy(tmpCondition, ""); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, signMask); return tmpCondition;
                    }
                    strcpy(tmpCondition, "("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ") & "); strcat(tmpCondition, signMask); return tmpCondition;                    
                case 9: // !SF
                case 13: // SF == OF
                    if (same) {
                        strcpy(tmpCondition, "!("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, signMask); strcat(tmpCondition, ")"); return tmpCondition;                    
                    }
                    strcpy(tmpCondition, "!(("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ") & "); strcat(tmpCondition, signMask); strcat(tmpCondition, ")"); return tmpCondition;                    
                case 10: // PF
                    if (same) {
                        strcpy(tmpCondition, "parity_lookup["); strcat(tmpCondition, left); strcat(tmpCondition, " & 0xFF]"); 
                    }
                    strcpy(tmpCondition, "parity_lookup[("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ") & 0xFF]");                    
                case 11: // !PF
                    if (same) {
                        strcpy(tmpCondition, "!parity_lookup["); strcat(tmpCondition, left); strcat(tmpCondition, " & 0xFF]");                    
                    }
                    strcpy(tmpCondition, "!parity_lookup[("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ") & 0xFF]");                    
                case 14: // ZF || SF != OF       
                    if (same) {
                        strcpy(tmpCondition, signCast);
                        strcat(tmpCondition, left);
                        strcat(tmpCondition, " <= 0");
                    } else {                   
                        strcpy(tmpCondition, "!("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ")"); return tmpCondition;                    
                        strcpy(tmpCondition, "|| (("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ") & "); strcat(tmpCondition, signMask); strcat(tmpCondition, ")"); return tmpCondition;                    
                    }
                    return tmpCondition;
                case 15: // !ZF && SF == OF
                    if (same) {
                        strcpy(tmpCondition, signCast);
                        strcat(tmpCondition, left);
                        strcat(tmpCondition, " > 0");
                    } else {                   
                        strcpy(tmpCondition, "("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ")"); return tmpCondition;                    
                        strcpy(tmpCondition, "&& !(("); strcat(tmpCondition, left); strcat(tmpCondition, " & "); strcat(tmpCondition, right); strcat(tmpCondition, ") & "); strcat(tmpCondition, signMask); strcat(tmpCondition, ")"); return tmpCondition;                    
                    }
                    return tmpCondition;
                default: kpanic("getCondition %d", condition); return "";
                }
        } else {
            kpanic("getCondition: unknown data->lazyFlags %d", data->lazyFlags);
        }        
    }
    switch (condition) {
    case 0: return getFlag(data, OF);
    case 1: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, OF)); return tmpCondition;
    case 2: return getFlag(data, CF);
    case 3: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, CF)); return tmpCondition;
    case 4: return getFlag(data, ZF);
    case 5: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, ZF)); return tmpCondition;
    case 6: strcpy(tmpCondition, getFlag(data, ZF)); strcat(tmpCondition, " || "); strcat(tmpCondition, getFlag(data, CF)); return tmpCondition;
    case 7: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, ZF)); strcat(tmpCondition, " && !"); strcat(tmpCondition, getFlag(data, CF)); return tmpCondition;
    case 8: return getFlag(data, SF);
    case 9: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, SF)); return tmpCondition;
    case 10: return getFlag(data, PF);
    case 11: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, PF)); return tmpCondition;
    case 12: strcpy(tmpCondition, getFlag(data, SF)); strcat(tmpCondition, " != "); strcat(tmpCondition, getFlag(data, OF)); return tmpCondition;
    case 13: strcpy(tmpCondition, getFlag(data, SF)); strcat(tmpCondition, " == "); strcat(tmpCondition, getFlag(data, OF)); return tmpCondition;
    case 14: strcpy(tmpCondition, getFlag(data, ZF)); strcat(tmpCondition, " || "), strcat(tmpCondition, getFlag(data, SF)); strcat(tmpCondition, " != "); strcat(tmpCondition, getFlag(data, OF)); return tmpCondition;
    case 15: strcpy(tmpCondition, "!"); strcat(tmpCondition, getFlag(data, ZF)); strcat(tmpCondition, " && "), strcat(tmpCondition, getFlag(data, SF)); strcat(tmpCondition, " == "); strcat(tmpCondition, getFlag(data, OF)); return tmpCondition;
    default: kpanic("getCondition %d", condition); return "";
    }
}

typedef void (*SRC_GEN)(struct GenData* data, struct Op* op);

SRC_GEN srcgen[] = {
    // 000
    gen000, gen001, gen002, gen003, gen004, gen005, gen006, gen007,
    gen008, gen009, gen00a, gen00b, gen00c, gen00d, gen00e, 0,
    gen010, gen011, gen012, gen013, gen014, gen015, gen016, gen017,
    gen018, gen019, gen01a, gen01b, gen01c, gen01d, gen01e, gen01f,
    gen020, gen021, gen022, gen023, gen024, gen025, 0, gen027,
    gen028, gen029, gen02a, gen02b, gen02c, gen02d, 0, gen02f,
    gen030, gen031, gen032, gen033, gen034, gen035, 0, gen037,
    gen038, gen039, gen03a, gen03b, gen03c, gen03d, 0, gen03f,
    gen040, gen040, gen040, gen040, gen040, gen040, gen040, gen040,
    gen048, gen048, gen048, gen048, gen048, gen048, gen048, gen048,
    gen050, gen050, gen050, gen050, gen050, gen050, gen050, gen050,
    gen058, gen058, gen058, gen058, gen058, gen058, gen058, gen058,
    0, 0, 0, 0, 0, 0, 0, 0,
    gen068, gen069, gen068, gen069, 0, 0, 0, 0,
    gen070, gen071, gen072, gen073, gen074, gen075, gen076, gen077,
    gen078, gen079, gen07a, gen07b, gen07c, gen07d, gen07e, gen07f,
    // 080
    gen080, gen081, gen080, gen081, gen084, gen085, gen086, gen087,
    gen088, gen089, gen08a, gen08b, gen08c, gen08d, gen08e, gen08f,
    gen090, gen091, gen091, gen091, gen091, gen091, gen091, gen091,
    gen098, gen099, gen09a, gen090, gen09c, gen09d, gen09e, gen09f,
    gen0a0, gen0a1, gen0a2, gen0a3, gen0a4, gen0a5, gen0a6, gen0a7,
    gen0a8, gen0a9, gen0aa, gen0ab, gen0ac, gen0ad, gen0ae, gen0af,
    gen0b0, gen0b0, gen0b0, gen0b0, gen0b0, gen0b0, gen0b0, gen0b0,
    gen0b8, gen0b8, gen0b8, gen0b8, gen0b8, gen0b8, gen0b8, gen0b8,
    gen0c0, gen0c1, gen0c2, gen0c3, 0, 0, gen0c6, gen0c7,
    0, gen0c9, 0, gen0cb, 0, gen0cd, 0, 0,
    gen0c0, gen0c1, gen0d2, gen0d3, gen0d4, gen0d5, gen0d6, gen0d7,
    gen0d8, gen0d9, gen0da, gen0db, gen0dc, gen0dd, gen0de, gen0df,
    gen0e0, gen0e1, gen0e2, gen0e3, 0, 0, 0, 0,
    gen0e8, gen0e9, 0, gen0e9, 0, 0, 0, 0,
    0, 0, 0, 0, 0, gen0f5, gen0f6, gen0f7,
    gen0f8, gen0f9, 0, 0, gen0fc, gen0fd, gen0fe, gen0ff,
    // 100
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, gen131, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    gen140, gen141, gen142, gen143, gen144, gen145, gen146, gen147,
    gen148, gen149, gen14a, gen14b, gen14c, gen14d, gen14e, gen14f,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 180
    gen070, gen071, gen072, gen073, gen074, gen075, gen076, gen077,
    gen078, gen079, gen07a, gen07b, gen07c, gen07d, gen07e, gen07f,
    gen190, gen191, gen192, gen193, gen194, gen195, gen196, gen197,
    gen198, gen199, gen19a, gen19b, gen19c, gen19d, gen19e, gen19f,
    gen1a0, gen1a1, gen1a2, gen1a3, gen1a4, gen1a5, 0, 0,
    gen1a8, gen1a9, 0, gen1ab, gen1ac, gen1ad, 0, gen1af,
    0, gen1b1, 0, 0, 0, 0, gen1b6, 0,
    0, 0, 0, 0, 0, gen1bd, gen1be, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 200
    gen000, gen201, gen002, gen203, gen004, gen205, gen206, gen207,
    gen008, gen209, gen00a, gen20b, gen00c, gen20d, gen20e, 0,
    gen010, gen211, gen012, gen213, gen014, gen215, gen216, gen217,
    gen018, gen219, gen01a, gen21b, gen01c, gen21d, gen21e, gen21f,
    gen020, gen221, gen022, gen223, gen024, gen225, 0, gen027,
    gen028, gen229, gen02a, gen22b, gen02c, gen22d, 0, gen02f,
    gen030, gen231, gen032, gen233, gen034, gen235, 0, gen037,
    gen038, gen239, gen03a, gen23b, gen03c, gen23d, 0, gen03f,
    gen240, gen240, gen240, gen240, gen240, gen240, gen240, gen240,
    gen248, gen248, gen248, gen248, gen248, gen248, gen248, gen248,
    gen250, gen250, gen250, gen250, gen250, gen250, gen250, gen250,
    gen258, gen258, gen258, gen258, gen258, gen258, gen258, gen258,
    gen260, gen261, 0, 0, 0, 0, 0, 0,
    gen268, gen269, gen268, gen269, 0, 0, 0, 0,
    gen070, gen071, gen072, gen073, gen074, gen075, gen076, gen077,
    gen078, gen079, gen07a, gen07b, gen07c, gen07d, gen07e, gen07f,
    // 280
    gen080, gen281, gen080, gen281, gen084, gen285, gen086, gen287,
    gen088, gen289, gen08a, gen28b, gen08c, gen28d, gen08e, gen28f,
    gen090, gen291, gen291, gen291, gen291, gen291, gen291, gen291,
    gen298, gen299, 0, gen090, gen29c, gen29d, gen09e, gen09f,
    gen0a0, gen2a1, gen0a2, gen2a3, gen0a4, gen2a5, gen0a6, gen2a7,
    gen0a8, gen2a9, gen0aa, gen2ab, gen0ac, gen2ad, gen0ae, gen2af,
    gen0b0, gen0b0, gen0b0, gen0b0, gen0b0, gen0b0, gen0b0, gen0b0,
    gen2b8, gen2b8, gen2b8, gen2b8, gen2b8, gen2b8, gen2b8, gen2b8,
    gen0c0, gen2c1, gen2c2, gen2c3, 0, 0, gen0c6, gen2c7,
    0, gen2c9, 0, 0, 0, gen0cd, 0, 0,
    gen0c0, gen2c1, gen0d2, gen2d3, gen0d4, gen0d5, gen0d6, gen0d7,
    gen0d8, gen0d9, gen0da, gen0db, gen0dc, gen0dd, gen0de, gen0df,
    gen0e0, gen0e1, gen0e2, gen0e3, 0, 0, 0, 0,
    gen2e8, gen0e9, 0, gen0e9, 0, 0, 0, 0,
    0, 0, 0, 0, 0, gen0f5, gen0f6, gen2f7,
    gen0f8, gen0f9, 0, 0, gen0fc, gen0fd, gen0fe, gen2ff,
    // 300
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, gen131, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    gen340, gen341, gen342, gen343, gen344, gen345, gen346, gen347,
    gen348, gen349, gen34a, gen34b, gen34c, gen34d, gen34e, gen34f,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    // 380
    gen070, gen071, gen072, gen073, gen074, gen075, gen076, gen077,
    gen078, gen079, gen07a, gen07b, gen07c, gen07d, gen07e, gen07f,
    gen190, gen191, gen192, gen193, gen194, gen195, gen196, gen197,
    gen198, gen199, gen19a, gen19b, gen19c, gen19d, gen19e, gen19f,
    gen3a0, gen3a1, gen1a2, gen3a3, gen3a4, gen3a5, 0, 0,
    gen3a8, gen3a9, 0, gen3ab, gen3ac, gen3ad, 0, gen3af,
    0, gen3b1, 0, 0, 0, 0, gen3b6, gen3b7,
    0, 0, gen3ba, gen3bb, gen3bc, gen3bd, gen3be, gen3bf,
    0, gen3c1, 0, 0, 0, 0, 0, gen3c7,
    gen3c8, gen3c8, gen3c8, gen3c8, gen3c8, gen3c8, gen3c8, gen3c8,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    gen400,
};

static int generatedBlocks;

#include "kprocess.h"
#include "crc.h"

#define STR(x) #x
#define OUT_DEFINE(v) out(data, "#define ");out(data, #v);out(data, " ");out(data, STR(v));out(data, "\n")

struct CompiledBlocks {
    U32 crc;
    char func[16];
    char* bytes;
    U32 byteLen;
};

struct CompiledBlocks compiledBlocks[32*1024];
U32 compiledBlockCount;

void addBlock(char* func, U32 crc, char* bytes, U32 byteLen) {
    int i = compiledBlockCount;  
  
    while (i>0 && crc<compiledBlocks[i-1].crc) {
        compiledBlocks[i] = compiledBlocks[i-1];
        i--;
    }
    compiledBlocks[i].crc = crc;
    compiledBlocks[i].bytes = bytes;
    compiledBlocks[i].byteLen = byteLen;
    strcpy(compiledBlocks[i].func, func);
    compiledBlockCount++;
}

BOOL doesBlockExist(U32 crc, char* bytes, U32 byteLen) {
    U32 i;

    for (i=0;i<compiledBlockCount;i++) {
        if (compiledBlocks[i].crc==crc) {
            while (compiledBlocks[i].crc==crc) {
                if (compiledBlocks[i].byteLen == byteLen && !memcmp(bytes, compiledBlocks[i].bytes, byteLen)) {
                    return 1;
                }
                i++;
            }
            break;
        }
        if (compiledBlocks[i].crc>crc)
            break;
    }
    return 0;
}

static struct GenData d;

void writeSource() {
    FILE* fp = fopen("gen.c", "w");  
    U32 i,j;
    char tmp[1024];

    fwrite(d.sourceBuffer, d.sourceBufferPos, 1, fp);

    for (i=0;i<compiledBlockCount;i++) {
        outfp(fp, "const unsigned char data");
        itoa(i, tmp, 10);
        outfp(fp, tmp);
        outfp(fp, "[] = {");
        for (j=0;j<compiledBlocks[i].byteLen;j++) {
            if (j>0)
                outfp(fp, ", ");
            sprintf(tmp, "0x%.02X", ((U32)compiledBlocks[i].bytes[j]) & 0xFF);
            outfp(fp, tmp);
        }
        outfp(fp, "};\n");
    }


    outfp(fp, "typedef void (OPCALL *OpCallback)(struct CPU* cpu, struct Op* op);");
    outfp(fp, "struct CompiledCode {U32 crc; OpCallback func; const unsigned char* bytes; unsigned int byteLen;};\n");
    outfp(fp, "static struct CompiledCode compiledCode[] = {\n");
        
    for (i=0;i<compiledBlockCount;i++) {
        outfp(fp, "{0x");
        itoa(compiledBlocks[i].crc, tmp, 16);
        outfp(fp, tmp);
        outfp(fp, ", ");
        outfp(fp, compiledBlocks[i].func);
        outfp(fp, ", data");
        itoa(i, tmp, 10);
        outfp(fp, tmp);
        outfp(fp, ", ");
        itoa(compiledBlocks[i].byteLen, tmp, 10);
        outfp(fp, tmp);
        outfp(fp, "},\n");
    }
    outfp(fp, "};\n");        

    outfp(fp, "#include <string.h>\n");
    outfp(fp, "// :TODO: compiledCode is sorted, use a binary search\n");
    outfp(fp, "OpCallback getCompiledFunction(U32 crc, const unsigned char* bytes, U32 byteLen, struct KThread* thread, U32 ip) {\n");
    outfp(fp, "    int i;\n");
    outfp(fp, "    int count = sizeof(compiledCode) / sizeof(struct CompiledCode);\n");
    outfp(fp, "    for (i=0;i<count;i++) {\n");
    outfp(fp, "        if (compiledCode[i].crc==crc) {\n");
    outfp(fp, "            while (compiledCode[i].crc==crc) {\n");
    outfp(fp, "                if (compiledCode[i].byteLen >= byteLen && !memcmp(bytes, compiledCode[i].bytes, byteLen)) {\n");
    outfp(fp, "                    U32 count = compiledCode[i].byteLen - byteLen;\n");
    outfp(fp, "                    U32 p = ip;\n");
    outfp(fp, "                    U32 pos = byteLen;\n");
    outfp(fp, "                    while (count) {\n");
    outfp(fp, "                        if (compiledCode[i].bytes[pos++]!=readb(thread, p++))\n");
    outfp(fp, "                            break;\n");
    outfp(fp, "                        count--;\n");
    outfp(fp, "                    }\n");
    outfp(fp, "                    if (count==0)\n");
    outfp(fp, "                        return compiledCode[i].func;\n");
    outfp(fp, "                }\n");
    outfp(fp, "                i++;\n");
    outfp(fp, "            }\n");
    outfp(fp, "            break;\n");
    outfp(fp, "        }\n");
    outfp(fp, "    }\n");
    outfp(fp, "    return 0;\n");
    outfp(fp, "}\n");

    fflush(fp);
    fclose(fp);
}

void writeBlock(struct GenData* data, struct Block* block) {
    char tmp[16];
    U32 i;
    struct Op* op;

    if (block->ops->func == restoreOps) {
        jit(data->cpu, block, data->eip);
        decodeBlockWithBlock(data->cpu, data->eip, block);
    }
    op = block->ops;
    if (op->func == firstOp)
        op = op->next;
    jit(data->cpu, block, data->eip);
    while (op) {
        for (i=0;i<data->indent;i++) {
            out(data, "    ");
        }
        srcgen[op->inst](data, op);
        // the last op is responsible for handling the eip adjustment
        if (op->next && op->eipCount) {
            out(data, " cpu->eip.u32+=");
            itoa(op->eipCount, tmp, 10);
            out(data, tmp);    
            out(data, ";\n");
        } 
        op = op->next;
    }
}

void writeBlockWithEipCount(struct GenData* data, struct Block* block, S32 eipCount) {
    char tmp[16];
    U32 i;
    struct Op* op;

    if (block->ops->func == restoreOps) {
        decodeBlockWithBlock(data->cpu, data->eip, block);
    }
    op = block->ops;
    if (op->func == firstOp) {
        op = op->next;
    }
    
    jit(data->cpu, block, data->eip);
    while (op && eipCount>0) {
        for (i=0;i<data->indent;i++) {
            out(data, "    ");
        }
        srcgen[op->inst](data, op);
        // the last op is responsible for handling the eip adjustment
        if (op->next && op->eipCount) {
            out(data, " cpu->eip.u32+=");
            itoa(op->eipCount, tmp, 10);
            out(data, tmp);    
            out(data, ";\n");
        } 
        eipCount-=op->eipCount;
        op = op->next;
    }
     if (eipCount!=0)
         kpanic("writeBlockWithEipCount error");
}

U32 writeFunction(struct GenData* data, struct Op* op) {
    return 0;
}

static unsigned char opsBuffer[16384];

void generateSource(struct CPU* cpu, U32 eip, struct Block* block) {
    struct Op* op = block->ops;
    char name[256];
    char tmp[1024];    
    int i;
    U32 crc;    
    struct GenData* data = &d;

    // if the block has only one op then don't compile it
    if (!block->ops->next) {
        return;
    }

    data->eip = eip;
    data->cpu = cpu;
    data->block = block;
    data->lazyFlags = sFLAGS_UNKNOWN;
    data->ops = opsBuffer;
    data->opPos = 0;
    data->maxOpPos = sizeof(opsBuffer);
    data->ip = eip;

    while (op) {
        if (!srcgen[op->inst]) {
            klog("missing instruction for recompiler: %X", op->inst);
            return;
        }
        for (i=0;i<op->eipCount;i++) {
            if (!isValidReadAddress(cpu->thread, data->ip))
                return;
            data->ops[data->opPos++] = readb(cpu->thread, data->ip++);
        }
        op = op->next;
    }
    crc = crc32b(data->ops, data->opPos);

    if (doesBlockExist(crc, data->ops, data->opPos))
        return;

    op = block->ops;
    if (!data->sourceBuffer) {
        data->sourceBufferLen = 1024*1024*10;
        data->sourceBuffer = kalloc(data->sourceBufferLen, KALLOC_SRCGENBUFFER);
        data->sourceBufferPos = 0;
        out(data, "#include \"platform.h\"\n");
        out(data, "#include \"cpu.h\"\n");
        out(data, "#include \"../../../../source/emulation/cpu/shift.h\"\n");
        out(data, "#include \"../../../../source/emulation/cpu/strings.h\"\n");
        out(data, "#include \"decoder.h\"\n");
        out(data, "void FPU_FCOM(struct FPU* fpu, int st, int other);\n");
        out(data, "void FPU_FUCOM(struct FPU* fpu, int st, int other);\n");
        out(data, "void FPU_FABS(struct FPU* fpu);\n");
        out(data, "void FPU_FXAM(struct FPU* fpu);\n");
        out(data, "void FPU_F2XM1(struct FPU* fpu);\n");
        out(data, "void FPU_FYL2X(struct FPU* fpu);\n");
        out(data, "void FPU_FPTAN(struct FPU* fpu);\n");
        out(data, "void FPU_FPATAN(struct FPU* fpu);\n");
        out(data, "void FPU_FXTRACT(struct FPU* fpu);\n");
        out(data, "void FPU_FPREM1(struct FPU* fpu);\n");
        out(data, "void FPU_FPREM(struct FPU* fpu);\n");
        out(data, "void FPU_FYL2XP1(struct FPU* fpu);\n");
        out(data, "void FPU_FSQRT(struct FPU* fpu);\n");
        out(data, "void FPU_FSINCOS(struct FPU* fpu);\n");
        out(data, "void FPU_FRNDINT(struct FPU* fpu);\n");
        out(data, "void FPU_FSCALE(struct FPU* fpu);\n");
        out(data, "void FPU_FSIN(struct FPU* fpu);\n");
        out(data, "void FPU_FCOS(struct FPU* fpu);\n");
        out(data, "void FPU_FLDENV(struct CPU* cpu, int addr);\n");
        out(data, "void FPU_SetCW(struct FPU* fpu, U16 word);\n");
        out(data, "void FPU_FSTENV(struct CPU* cpu, int addr);\n");        
        out(data, "void FPU_FINIT(struct FPU* fpu);\n");
        out(data, "void FPU_FCOMI(struct CPU* cpu, int st, int other);\n");
        out(data, "void FPU_FST_I32(struct CPU* cpu, int addr);\n");
        out(data, "double FPU_FLD80(U64 eind, U32 begin);\n");
        out(data, "void FPU_ST80(struct CPU* cpu, int addr, int reg);\n");
        out(data, "void FPU_FRSTOR(struct CPU* cpu, int addr);\n");
        out(data, "void FPU_FSAVE(struct CPU* cpu, int addr);\n");
        out(data, "void FPU_FST_I16(struct CPU* cpu, int addr);\n");
        out(data, "void FBLD_PACKED_BCD(struct CPU* cpu, U32 address);\n");
        out(data, "void FPU_FBST(struct CPU* cpu, int addr);\n");
        out(data, "void FPU_FST_I64(struct CPU* cpu, int addr);\n");
        out(data, "U32 div8(struct CPU* cpu, U8 src);\n");
        out(data, "U32 idiv8(struct CPU* cpu, S8 src);\n");
        out(data, "U32 div16(struct CPU* cpu, U16 src);\n");
        out(data, "U32 idiv16(struct CPU* cpu, S16 src);\n");
        out(data, "U32 div32(struct CPU* cpu, U32 src);\n");
        out(data, "U32 idiv32(struct CPU* cpu, S32 src);\n");
        out(data, "void ksyscall(struct CPU* cpu, U32 eipCount);\n");
        out(data, "void cpuid(struct CPU* cpu);\n");
        out(data, "typedef void (*Int99Callback)(struct CPU* cpu);\n");
        out(data, "extern Int99Callback* int99Callback;\n");
        out(data, "extern Int99Callback* wine_callback;\n");
        out(data, "U8 parity_lookup[256] ;\n");
        out(data, "#define TAG_Valid 0\n");
        out(data, "#define TAG_Zero 1\n");
        out(data, "#define TAG_Empty 3\n");
        out(data, "#define FPU_FPOP(cpu) cpu->fpu.tags[cpu->fpu.top] = TAG_Empty; cpu->fpu.top = ((cpu->fpu.top + 1) & 7)\n");
        out(data, "#define STV(fpu, i) (((fpu)->top + (i)) & 7)\n");
        out(data, "#define FPU_PREP_PUSH(cpu) cpu->fpu.top = (cpu->fpu.top - 1) & 7; cpu->fpu.tags[cpu->fpu.top] = TAG_Valid\n");
        out(data, "#define FPU_SET_TOP(fpu, val) (fpu)->sw &= ~0x3800; (fpu)->sw |= (val & 7) << 11\n");

        out(data, "static U32 eaa;\n");
        out(data, "static U8 tmp8;\n");
        out(data, "static U16 tmp16;\n");
        out(data, "static U32 tmp32;\n");        
        out(data, "static U32 tmp32_2;\n");
        out(data, "static S32 tmps32;\n");
        out(data, "static U64 tmp64;\n\n");
        out(data, "static S64 tmps64;\n\n");
        out(data, "static double tmpd;\n\n");

        out(data, "struct F2I {union {float f;U32 i;};};\n");
        out(data, "struct F2I f2i;");
        out(data, "struct D2L {union {double d;U64 l;};};\n");
        out(data, "struct D2L d2l;");
    }
    sprintf(tmp, "// 0x%.8x CRC=%.08X %s at 0x%.8x\n", eip, crc, getModuleName(cpu, eip), getModuleEip(cpu, eip));
    out(data, tmp);
    sprintf(name, "generated%X", compiledBlockCount);
    out(data, "void OPCALL ");
    out(data, name);
    out(data, "(struct CPU* cpu, struct Op* op) {\n");    
    data->indent=1;
    if (!block->startFunction || !writeFunction(data, op))
        writeBlock(data, block);
    out(data, "\n");
    out(data, "}\n\n");    

    {
        char* bytes = (char*)kalloc(data->opPos, KALLOC_SRCGENBYTES);
        memcpy(bytes, data->ops, data->opPos);
        addBlock(name, crc, bytes, data->opPos);
    }
}
#endif