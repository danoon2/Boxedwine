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

#include "cpu.h"
#include "decoder.h"
#include "memory.h"
#include "op.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include "fpu.h"
#include "ops.h"
#include "kalloc.h"
#include "conditions.h"
#include "setcc.h"
#include "block.h"
#include "jit.h"
#include "ksystem.h"
#include "decodedata.h"

#define FETCH_S8(data) (S8)FETCH8(data)
#define FETCH_S16(data) (S16)FETCH16(data)
#define FETCH_S32(data) (S32)FETCH32(data)

#ifdef BOXEDWINE_VM
U32 OPCALL getWriteAddress32(struct CPU* cpu, struct Op* op) {
    return eaa32(cpu, op);
}

U32 OPCALL getWriteAddress16(struct CPU* cpu, struct Op* op) {
    return eaa16(cpu, op);
}

U32 OPCALL getWriteAddressDirect(struct CPU* cpu, struct Op* op) {
    return cpu->segAddress[op->base]+op->data1;
}

U32 OPCALL getWriteBytes1(struct CPU* cpu, struct Op* op) {
    return 1;
}

U32 OPCALL getWriteBytes2(struct CPU* cpu, struct Op* op) {
    return 2;
}

U32 OPCALL getWriteBytes4(struct CPU* cpu, struct Op* op) {
    return 4;
}

#endif

void log_pf(struct KThread* thread, U32 address);

struct Op* freeOps;
int totalOpCount;
int freeOpCount;
int allocatedOpMemory;

struct Op* allocOp() {
    struct Op* result;

    if (freeOps) {
        result = freeOps;
        freeOps = result->next;
        memset(result, 0, sizeof(struct Op));
        freeOpCount--;
    } else {
        U32 count = 1024*1023/sizeof(struct Op);
        U32 i;

        result = (struct Op*)kalloc(1024*1023, KALLOC_OP);
        for (i=0;i<count;i++) {
            result->next = freeOps;
            freeOps = result;
            result++;
        }
        freeOpCount+=count;
        totalOpCount+=count;
        allocatedOpMemory+=1024*1023;
        return allocOp();
    }	
    return result;
}

void freeOp(struct Op* op) {
    struct Op* next;
    while (op) {
        next = op->next;
        op->next = freeOps;
        freeOps = op;
        freeOpCount++;
        op = next;
    }
}

struct BlockNode {
    struct Block* block;
    struct BlockNode* next;
};

struct BlockNode* freeBlockNodes;

struct BlockNode* allocBlockNode() {
    struct BlockNode* result;

    if (freeBlockNodes) {
        result = freeBlockNodes;
        freeBlockNodes = result->next;
        memset(result, 0, sizeof(struct BlockNode));
    } else {
        U32 count = 1024*1023/sizeof(struct BlockNode);
        U32 i;

        result = (struct BlockNode*)kalloc(1024*1023, KALLOC_BLOCK);
        for (i=0;i<count;i++) {
            result->next = freeBlockNodes;
            freeBlockNodes = result;
            result++;
        }
        return allocBlockNode();
    }	
    return result;
}

void freeBlockNode(struct BlockNode* node) {
    node->next = freeBlockNodes;
    freeBlockNodes = node;
}

void addBlockNode(struct BlockNode** node, struct Block* block) {
    struct BlockNode* n = allocBlockNode();
    n->block = block;
    n->next = *node;
    *node = n;
}

struct Block* freeBlocks;

struct Block* allocBlock() {
    struct Block* result;

    if (freeBlocks) {
        result = freeBlocks;
        freeBlocks = result->block1;
        memset(result, 0, sizeof(struct Block));
    } else {
        U32 count = 1024*1023/sizeof(struct Block);
        U32 i;

        result = (struct Block*)kalloc(1024*1023, KALLOC_BLOCK);
        for (i=0;i<count;i++) {
            result->block1 = freeBlocks;
            freeBlocks = result;
            result++;
        }
        return allocBlock();
    }	
    return result;
}

void removeBlockFromBlockReference(struct Block* block, struct Block* reference) {
	if (block && reference) {
		struct BlockNode* from = reference->referencedFrom;
		struct BlockNode* prev = NULL;

		while (from) {
			if (from->block == block) {
				struct BlockNode* removed = from;

				if (prev) {					
					prev->next = from->next;					
				} else {
					reference->referencedFrom = from->next;
				}
				from = from->next;
				freeBlockNode(removed);
				continue;
			}
			prev = from;
			from = from->next;
		}
	}
}
void unlinkBlock(struct Block* block) {
    while (block->referencedFrom) {
        struct BlockNode* next = block->referencedFrom->next;

        // if any blocks reference this block, set that reference to NULL
        if (block->referencedFrom->block->block1 == block)
            block->referencedFrom->block->block1 = NULL;
        if (block->referencedFrom->block->block2 == block)
            block->referencedFrom->block->block2 = NULL;
        freeBlockNode(block->referencedFrom);
        block->referencedFrom = next;
    }
	removeBlockFromBlockReference(block, block->block1);
	removeBlockFromBlockReference(block, block->block2);
	block->block1 = NULL;
	block->block2 = NULL;
}

static struct Block* delayedFree;

void delayFreeBlock(struct Block* block) {
    if (delayedFree) {
        freeBlock(delayedFree);
    }
    unlinkBlock(block);
	block->count = 0; // make sure JIT won't run on it
    delayedFree = block;
}

// This will stop the execution of the current block while letting the 
// current op finish, we want to let the current op finish and not longjmp
// out because we don't want an unfinished op since we won't come back
// to it.  
void delayFreeBlockAndKillCurrentBlock(struct Block* block) {
    struct Op* op = block->ops;
    while (op) {
        op->func = emptyOp;
        op = op->next;
    }
    delayFreeBlock(block);
}

void freeBlock(struct Block* block) {
    unlinkBlock(block);
	if (block->ops) {
		freeOp(block->ops);
		block->ops = 0;
	}
    block->block1 = freeBlocks;
    freeBlocks = block;
}

#define FINISH_OP(data) data->op->eipCount=data->ip-data->start

typedef void (*DECODER)(struct DecodeData* obj);

extern DECODER decoder[1024];

// the op was discarded, but make sure we count the eip change
#define RESTART(data) if (data->cpu->big) { data->opCode = 0x200; data->ea16 = 0; } else { data->opCode = 0; data->ea16 = 1; } data->ds = DS; data->ss = SS; data->rep = 0

void RESTART_OP(struct DecodeData* data) {
    data->op->inst = FETCH8(data)+data->opCode;
    decoder[data->op->inst](data);
}

void NEXT_OP(struct DecodeData* data) {
    FINISH_OP(data);
    if (data->singleOp)
        return;
    data->ds = DS;
    data->ss = SS;
    data->rep = 0;    
    data->start = data->ip;
    data->op->next = allocOp();
    data->op = data->op->next;
    if (data->cpu->big) {
        data->opCode = 0x200;
        data->ea16 = 0;
    } else {
        data->opCode = 0;
        data->ea16 = 1;
    }
    data->count++;
    if (data->count>50) {
        data->op->func = emptyOp;
        data->op->inst = 1024;
        return;
    }
    data->op->inst = FETCH8(data)+data->opCode;
    decoder[data->op->inst](data);
}

#ifdef LOG_OPS
const char* RB(int r) {
    switch (r) {
    case 0: return "AL";
    case 1: return "CL";
    case 2: return "DL";
    case 3: return "BL";
    case 4: return "AH";
    case 5: return "CH";
    case 6: return "DH";
    case 7: return "BH";
    default: return "0";
    }
}

const char* RW(int r) {
    switch (r) {
    case 0: return "AX";
    case 1: return "CX";
    case 2: return "DX";
    case 3: return "BX";
    case 4: return "SP";
    case 5: return "BP";
    case 6: return "SI";
    case 7: return "DI";
    default: return "0";
    }
}

const char* RD(int r) {
    switch (r) {
    case 0: return "EAX";
    case 1: return "ECX";
    case 2: return "EDX";
    case 3: return "EBX";
    case 4: return "ESP";
    case 5: return "EBP";
    case 6: return "ESI";
    case 7: return "EDI";
    default: return "0";
    }
}

#define R8 RB
#define R16 RW
#define R32 RD

const char* EABASE(int base) {
    switch (base) {
    case ES: return "ES";
    case CS: return "CS";
    case SS: return "SS";
    case DS: return "DS";
    case FS: return "FS";
    case GS: return "GS";
    default: return "0";
    }
}

void SIB(char* s, struct Op* op) {
    BOOL added = FALSE;
    if (op->e1<8) {
        strcat(s, RD(op->e1));
        added = TRUE;
    }
    if (op->e2<8) {
        if (op->eSib>0) {
            if (added) {
                strcat(s, "+");
            }
            sprintf(s+strlen(s), "(%s<<%d)", RD(op->e2), op->eSib);
            added = TRUE;
        } else {
            if (added) {
                strcat(s, "+");
            }
            sprintf(s+strlen(s), "%s", RD(op->e2));
            added = TRUE;
        }
    }
    if (op->eData) {
        if (added) {
            strcat(s, "+");
        }
        sprintf(s+strlen(s), "%X", op->eData); 
    }
}

void EAA16(char* s, int rm, struct Op* op) {
    switch (rm & 7) {
    case 0x00: strcat(s, "BX+SI"); break;
    case 0x01: strcat(s, "BX+DI"); break;
    case 0x02: strcat(s, "BP+SI"); break;
    case 0x03: strcat(s, "BP+DI"); break;
    case 0x04: strcat(s, "SI"); break;
    case 0x05: strcat(s, "DI"); break;
    case 0x06: 
        if (rm<0x40) {
            sprintf(s+strlen(s), "%X", op->eData); 
        } else {
            strcat(s, "BP");
        }
        break;
    case 0x07: strcat(s, "BX"); break;
    }
    if (rm>0x40) {
        sprintf(s+strlen(s), "+%X", op->eData); 
    }
}

void EAA32(char* s, int rm, struct Op* op) {
    switch (rm & 7) {
    case 0x00: strcat(s, "EAX"); break;
    case 0x01: strcat(s, "ECX"); break;
    case 0x02: strcat(s, "EDX"); break;
    case 0x03: strcat(s, "EBX"); break;
    case 0x04: SIB(s, op); return;
    case 0x05: 
        if (rm<0x40) {
            sprintf(s+strlen(s), "%X", op->eData); 
        } else {
            strcat(s, "EBP");
        }
        break;
    case 0x06: strcat(s, "ESI"); break;
    case 0x07: strcat(s, "EDI"); break;
    }
    if (rm>0x40) {
        sprintf(s+strlen(s), "+%X", op->eData); 
    }	
}

const char* M16(struct DecodeData* data, int rm, struct Op* op) {
    strcpy(data->tmp, "WORD PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    if (data->ea16) {
        EAA16(data->tmp, rm, op);
    } else {
        EAA32(data->tmp, rm, op);
    }
    strcat(data->tmp, "]");
    return data->tmp;
}

const char* O16(struct DecodeData* data, struct Op* op) {
    strcpy(data->tmp, "WORD PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    sprintf(data->tmp+strlen(data->tmp), "+%X", op->data1); 
    strcat(data->tmp, "]");
    return data->tmp;
}

const char* M8(struct DecodeData* data, int rm, struct Op* op) {
    strcpy(data->tmp, "BYTE PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    if (data->ea16) {
        EAA16(data->tmp, rm, op);
    } else {
        EAA32(data->tmp, rm, op);
    }
    strcat(data->tmp, "]");
    return data->tmp;
}

const char* O8(struct DecodeData* data, struct Op* op) {
    strcpy(data->tmp, "BYTE PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    sprintf(data->tmp+strlen(data->tmp), "+%X", op->data1); 
    strcat(data->tmp, "]");
    return data->tmp;
}

const char* M32(struct DecodeData* data, int rm, struct Op* op) {
    strcpy(data->tmp, "DWORD PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    if (data->ea16) {
        EAA16(data->tmp, rm, op);
    } else {
        EAA32(data->tmp, rm, op);
    }
    strcat(data->tmp, "]");
    return data->tmp;
}

const char* M64(struct DecodeData* data, int rm, struct Op* op) {
    strcpy(data->tmp, "QWORD PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    if (data->ea16) {
        EAA16(data->tmp, rm, op);
    } else {
        EAA32(data->tmp, rm, op);
    }
    strcat(data->tmp, "]");
    return data->tmp;
}

const char* O32(struct DecodeData* data, struct Op* op) {
    strcpy(data->tmp, "DWORD PTR [");
    if (op->base != SEG_ZERO) {
        strcat(data->tmp, EABASE(op->base));
        strcat(data->tmp, ":");
    }

    sprintf(data->tmp+strlen(data->tmp), "+%X", op->data1); 
    strcat(data->tmp, "]");
    return data->tmp;
}
#endif
void logOpCsEip(char* buffer, const char* name, U32 cs, U32 eip) {
    sprintf(buffer, "%s %04X:%04X", name, cs, eip);
}

#define regAX 0
#define regCX 1
#define regDX 2
#define regBX 3
#define regSP 4
#define regBP 5
#define regSI 6
#define regDI 7
#define regZero 8

void decodeEa16(struct DecodeData* data, int rm) {
    struct Op* op = data->op;

    if (rm<0x40) {
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->e1=regBX; op->e2=regSI; break;
        case 0x01: op->base=data->ds; op->e1=regBX; op->e2=regDI; break;
        case 0x02: op->base=data->ss; op->e1=regBP; op->e2=regSI; break;
        case 0x03: op->base=data->ss; op->e1=regBP; op->e2=regDI; break;
        case 0x04: op->base=data->ds; op->e1=regSI; op->e2=regZero; break;
        case 0x05: op->base=data->ds; op->e1=regDI; op->e2=regZero; break;
        case 0x06: op->base=data->ds; op->eData = FETCH16(data); op->e1=regZero; op->e2=regZero; break;
        case 0x07: op->base=data->ds; op->e1=regBX; op->e2=regZero; break;
        }
    } else {
        if (rm<0x80) {
            op->eData = FETCH_S8(data);
        } else {
            op->eData = FETCH_S16(data);
        }
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->e1=regBX; op->e2=regSI; break;
        case 0x01: op->base=data->ds; op->e1=regBX; op->e2=regDI; break;
        case 0x02: op->base=data->ss; op->e1=regBP; op->e2=regSI; break;
        case 0x03: op->base=data->ss; op->e1=regBP; op->e2=regDI; break;
        case 0x04: op->base=data->ds; op->e1=regSI; op->e2=regZero; break;
        case 0x05: op->base=data->ds; op->e1=regDI; op->e2=regZero; break;
        case 0x06: op->base=data->ss; op->e1=regBP; op->e2=regZero; break;
        case 0x07: op->base=data->ds; op->e1=regBX; op->e2=regZero; break;
        }
    }
}

U32 SibE2Reg[8] = {regAX, regCX, regDX, regBX, regZero, regBP, regSI, regDI};

 void Sib0(struct DecodeData* data) {
    int sib=FETCH8(data);
    struct Op* op = data->op;

    switch (sib&7) {
    case 0:
        op->base=data->ds; op->e1=regAX;break;
    case 1:
        op->base=data->ds; op->e1=regCX;break;
    case 2:
        op->base=data->ds; op->e1=regDX;break;
    case 3:
        op->base=data->ds; op->e1=regBX;break;
    case 4:
        op->base=data->ss; op->e1=regSP;break;
    case 5:
        op->base = data->ds; op->eData = FETCH32(data);break;
    case 6:
        op->base=data->ds; op->e1=regSI;break;
    case 7:
        op->base=data->ds; op->e1=regDI;break;
    }
    op->e2 = SibE2Reg[(sib >> 3) & 7];
    op->eSib = sib >> 6;
} 

  void Sib1(struct DecodeData* data) {
    int sib=FETCH8(data);
    struct Op* op = data->op;

    switch (sib&7) {
    case 0:
        op->base=data->ds; op->e1=regAX;break;
    case 1:
        op->base=data->ds; op->e1=regCX;break;
    case 2:
        op->base=data->ds; op->e1=regDX;break;
    case 3:
        op->base=data->ds; op->e1=regBX;break;
    case 4:
        op->base=data->ss; op->e1=regSP;break;
    case 5:
        op->base=data->ss; op->e1=regBP;break;
    case 6:
        op->base=data->ds; op->e1=regSI;break;
    case 7:
        op->base=data->ds; op->e1=regDI;break;
    }
    op->e2 = SibE2Reg[(sib >> 3) & 7];
    op->eSib = sib >> 6;
} 

void decodeEa32(struct DecodeData* data, int rm) {
    struct Op* op = data->op;

    op->e1=regZero; 
    op->e2=regZero; 
    op->eSib = 0;
    if (rm<0x40) {
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->e1 = regAX; break;
        case 0x01: op->base=data->ds; op->e1 = regCX; break;
        case 0x02: op->base=data->ds; op->e1 = regDX; break;
        case 0x03: op->base=data->ds; op->e1 = regBX; break;
        case 0x04: Sib0(data); break;
        case 0x05: op->base=data->ds; op->eData = FETCH32(data); break;
        case 0x06: op->base=data->ds; op->e1 = regSI; break;
        case 0x07: op->base=data->ds; op->e1 = regDI; break;
        }
    } else {		
        switch (rm & 7) {
        case 0x00: op->base=data->ds; op->e1 = regAX; break;
        case 0x01: op->base=data->ds; op->e1 = regCX; break;
        case 0x02: op->base=data->ds; op->e1 = regDX; break;
        case 0x03: op->base=data->ds; op->e1 = regBX; break;
        case 0x04: Sib1(data); break;
        case 0x05: op->base=data->ss; op->e1 = regBP; break;
        case 0x06: op->base=data->ds; op->e1 = regSI; break;
        case 0x07: op->base=data->ds; op->e1 = regDI; break;
        }
        if (rm<0x80) {
            op->eData = FETCH_S8(data);
        } else {
            op->eData = FETCH32(data);
        }
    }
}

#define DECODE_MEMORY(m16, m32)		\
if (data->ea16) {					\
    data->op->func = m16;			\
    decodeEa16(data, rm);			\
    WRITE_ADDRESS_16(data->op);     \
} else {							\
    data->op->func = m32;			\
    decodeEa32(data, rm);			\
    WRITE_ADDRESS_32(data->op);     \
}

#define DECODE_E(r, m16, m32)		\
if (rm >= 0xc0 ) {					\
    data->op->func = r;				\
    data->op->r1 = E(rm);			\
} else {							\
    DECODE_MEMORY(m16, m32);		\
}

#include "mmx.h"

#ifdef LOG_OPS

void LOG_E8(const char* name, int rm, struct DecodeData* data) {
    if (rm >= 0xc0 ) {
        LOG_OP1(name, RB(data->op->r1));
    } else {
        LOG_OP1(name, M8(data, rm, data->op));
    }
}

void LOG_E8C(const char* name, int rm, struct DecodeData* data) {
    sprintf(data->tmp, "%X", data->op->data1);
    if (rm >= 0xc0 ) {
        LOG_OP2(name, RB(data->op->r1), data->tmp);
    } else {
        LOG_OP2(name, M8(data, rm, data->op), data->tmp);
    }
}

void LOG_E8Cl(const char* name, int rm, struct DecodeData* data) {
    if (rm >= 0xc0 ) {
        LOG_OP2(name, RB(data->op->r1), "CL");
    } else {
        LOG_OP2(name, M8(data, rm, data->op), "CL");
    }
}

void LOG_E16(const char* name, int rm, struct DecodeData* data) {
    if (rm >= 0xc0 ) {
        LOG_OP1(name, RW(data->op->r1));
    } else {
        LOG_OP1(name, M16(data, rm, data->op));
    }
}

void LOG_E16C(const char* name, int rm, struct DecodeData* data) {
    sprintf(data->tmp, "%X", data->op->data1);
    if (rm >= 0xc0 ) {
        LOG_OP2(name, RW(data->op->r1), data->tmp);
    } else {
        LOG_OP2(name, M16(data, rm, data->op), data->tmp);
    }
}

void LOG_E16Cl(const char* name, int rm, struct DecodeData* data) {
    if (rm >= 0xc0 ) {
        LOG_OP2(name, RW(data->op->r1), "CL");
    } else {
        LOG_OP2(name, M16(data, rm, data->op), "CL");
    }
}

void LOG_E32(const char* name, int rm, struct DecodeData* data) {
    if (rm >= 0xc0 ) {
        LOG_OP1(name, RD(data->op->r1));
    } else {
        LOG_OP1(name, M32(data, rm, data->op));
    }
}

void LOG_E32C(const char* name, int rm, struct DecodeData* data) {
    sprintf(data->tmp, "%X", data->op->data1);
    if (rm >= 0xc0 ) {
        LOG_OP2(name, RD(data->op->r1), data->tmp);
    } else {
        LOG_OP2(name, M32(data, rm, data->op), data->tmp);
    }
}

void LOG_E32Cl(const char* name, int rm, struct DecodeData* data) {
    if (rm >= 0xc0 ) {
        LOG_OP2(name, RD(data->op->r1), "CL");
    } else {
        LOG_OP2(name, M32(data, rm, data->op), "CL");
    }
}
#endif

#include "decode.h"

// 2 byte opcodes
void decode00f(struct DecodeData* data) {
    data->opCode+=0x100;
    RESTART_OP(data);
}

// BOUND
void decode062(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (data->ea16) {
        data->op->func = bound_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
    } else {
        data->op->func = bound_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
    }
    LOG_OP2("BOUND", RW(data->op->r1), M16(data, rm, data->op));
    NEXT_OP(data);
}

// BOUND
void decode262(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (data->ea16) {
        data->op->func = boundd_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
    } else {
        data->op->func = boundd_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
    }
    LOG_OP2("BOUND", RD(data->op->r1), M32(data, rm, data->op));
    NEXT_OP(data);
}

void invalidOp(struct DecodeData* data) {
    printf("Invalid instruction %x\n", data->op->inst);    
    log_pf(data->cpu->thread, data->ip);
}

// Operand Size Prefix
void decode066(struct DecodeData* data) {
    data->opCode = 0x200;
    RESTART_OP(data);
}

// Operand Size Prefix
void decode266(struct DecodeData* data) {
    data->opCode = 0;
    RESTART_OP(data);
}

// Address Size Prefix
void decode067(struct DecodeData* data) {
    data->ea16 = 0;
    RESTART_OP(data);
}

// Address Size Prefix
void decode267(struct DecodeData* data) {
    data->ea16 = 1;
    RESTART_OP(data);
}

// Grpl Eb,Ib
void decode080(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(add8_reg, add8_mem16, add8_mem32); break;
    case 1: DECODE_E(or8_reg, or8_mem16, or8_mem32); break;
    case 2: DECODE_E(adc8_reg, adc8_mem16, adc8_mem32); break;
    case 3: DECODE_E(sbb8_reg, sbb8_mem16, sbb8_mem32); break;
    case 4: DECODE_E(and8_reg, and8_mem16, and8_mem32); break;
    case 5: DECODE_E(sub8_reg, sub8_mem16, sub8_mem32); break;
    case 6: DECODE_E(xor8_reg, xor8_mem16, xor8_mem32); break;
    case 7: DECODE_E(cmp8_reg, cmp8_mem16, cmp8_mem32); break;
    }	
    WRITE_BYTES_1(data->op);
    data->op->data1 = FETCH8(data);			
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E8C("ADD", rm, data); break;
    case 1: LOG_E8C("OR", rm, data); break;
    case 2: LOG_E8C("ADC", rm, data); break;
    case 3: LOG_E8C("SBB", rm, data); break;
    case 4: LOG_E8C("AND", rm, data); break;
    case 5: LOG_E8C("SUB", rm, data); break;
    case 6: LOG_E8C("XOR", rm, data); break;
    case 7: LOG_E8C("CMP", rm, data); break;
    }
#endif
    NEXT_OP(data);
}

// Grpl Ew,Iw
void decode081(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(add16_reg, add16_mem16, add16_mem32); break;
    case 1: DECODE_E(or16_reg, or16_mem16, or16_mem32); break;
    case 2: DECODE_E(adc16_reg, adc16_mem16, adc16_mem32); break;
    case 3: DECODE_E(sbb16_reg, sbb16_mem16, sbb16_mem32); break;
    case 4: DECODE_E(and16_reg, and16_mem16, and16_mem32); break;
    case 5: DECODE_E(sub16_reg, sub16_mem16, sub16_mem32); break;
    case 6: DECODE_E(xor16_reg, xor16_mem16, xor16_mem32); break;
    case 7: DECODE_E(cmp16_reg, cmp16_mem16, cmp16_mem32); break;
    }			
    WRITE_BYTES_2(data->op);
    data->op->data1 = FETCH16(data);			
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E16C("ADD", rm, data); break;
    case 1: LOG_E16C("OR", rm, data); break;
    case 2: LOG_E16C("ADC", rm, data); break;
    case 3: LOG_E16C("SBB", rm, data); break;
    case 4: LOG_E16C("AND", rm, data); break;
    case 5: LOG_E16C("SUB", rm, data); break;
    case 6: LOG_E16C("XOR", rm, data); break;
    case 7: LOG_E16C("CMP", rm, data); break;
    }
#endif
    NEXT_OP(data);
}

// Grpl Ed,Id
void decode281(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(add32_reg, add32_mem16, add32_mem32); break;
    case 1: DECODE_E(or32_reg, or32_mem16, or32_mem32); break;
    case 2: DECODE_E(adc32_reg, adc32_mem16, adc32_mem32); break;
    case 3: DECODE_E(sbb32_reg, sbb32_mem16, sbb32_mem32); break;
    case 4: DECODE_E(and32_reg, and32_mem16, and32_mem32); break;
    case 5: DECODE_E(sub32_reg, sub32_mem16, sub32_mem32); break;
    case 6: DECODE_E(xor32_reg, xor32_mem16, xor32_mem32); break;
    case 7: DECODE_E(cmp32_reg, cmp32_mem16, cmp32_mem32); break;
    }		
    WRITE_BYTES_4(data->op);
    data->op->data1 = FETCH32(data);			
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E32C("ADD", rm, data); break;
    case 1: LOG_E32C("OR", rm, data); break;
    case 2: LOG_E32C("ADC", rm, data); break;
    case 3: LOG_E32C("SBB", rm, data); break;
    case 4: LOG_E32C("AND", rm, data); break;
    case 5: LOG_E32C("SUB", rm, data); break;
    case 6: LOG_E32C("XOR", rm, data); break;
    case 7: LOG_E32C("CMP", rm, data); break;
    }
#endif
    NEXT_OP(data);
}

// Grpl Ew,Ix
void decode083(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(add16_reg, add16_mem16, add16_mem32); break;
    case 1: DECODE_E(or16_reg, or16_mem16, or16_mem32); break;
    case 2: DECODE_E(adc16_reg, adc16_mem16, adc16_mem32); break;
    case 3: DECODE_E(sbb16_reg, sbb16_mem16, sbb16_mem32); break;
    case 4: DECODE_E(and16_reg, and16_mem16, and16_mem32); break;
    case 5: DECODE_E(sub16_reg, sub16_mem16, sub16_mem32); break;
    case 6: DECODE_E(xor16_reg, xor16_mem16, xor16_mem32); break;
    case 7: DECODE_E(cmp16_reg, cmp16_mem16, cmp16_mem32); break;
    }		
    WRITE_BYTES_2(data->op);
    data->op->data1 = FETCH_S8(data);			
    data->op->data1 &= 0xFFFF;
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E16C("ADD", rm, data); break;
    case 1: LOG_E16C("OR", rm, data); break;
    case 2: LOG_E16C("ADC", rm, data); break;
    case 3: LOG_E16C("SBB", rm, data); break;
    case 4: LOG_E16C("AND", rm, data); break;
    case 5: LOG_E16C("SUB", rm, data); break;
    case 6: LOG_E16C("XOR", rm, data); break;
    case 7: LOG_E16C("CMP", rm, data); break;
    }
#endif
    NEXT_OP(data);
}

// Grpl Ed,Ix
void decode283(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(add32_reg, add32_mem16, add32_mem32); break;
    case 1: DECODE_E(or32_reg, or32_mem16, or32_mem32); break;
    case 2: DECODE_E(adc32_reg, adc32_mem16, adc32_mem32); break;
    case 3: DECODE_E(sbb32_reg, sbb32_mem16, sbb32_mem32); break;
    case 4: DECODE_E(and32_reg, and32_mem16, and32_mem32); break;
    case 5: DECODE_E(sub32_reg, sub32_mem16, sub32_mem32); break;
    case 6: DECODE_E(xor32_reg, xor32_mem16, xor32_mem32); break;
    case 7: DECODE_E(cmp32_reg, cmp32_mem16, cmp32_mem32); break;
    }		
    WRITE_BYTES_4(data->op);
    data->op->data1 = FETCH_S8(data);			
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E32C("ADD", rm, data); break;
    case 1: LOG_E32C("OR", rm, data); break;
    case 2: LOG_E32C("ADC", rm, data); break;
    case 3: LOG_E32C("SBB", rm, data); break;
    case 4: LOG_E32C("AND", rm, data); break;
    case 5: LOG_E32C("SUB", rm, data); break;
    case 6: LOG_E32C("XOR", rm, data); break;
    case 7: LOG_E32C("CMP", rm, data); break;
    }
#endif
    NEXT_OP(data);
}

// Mov Ew,Sw
void decode08c(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr16s16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", RW(data->op->r1), EABASE(data->op->r2));
    } else if (data->ea16) {
        data->op->func = move16s16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("MOV", M16(data, rm, data->op), EABASE(data->op->r2));
    } else {
        data->op->func = move16s16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP2("MOV", M16(data, rm, data->op), EABASE(data->op->r2));
    }
    NEXT_OP(data);
}

// Mov Ed,Sw
void decode28c(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movr32s16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", RD(data->op->r1), EABASE(data->op->r2));
    } else if (data->ea16) {
        data->op->func = move16s16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", M16(data, rm, data->op), EABASE(data->op->r2));
    } else {
        data->op->func = move16s16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", M16(data, rm, data->op), EABASE(data->op->r2));
    }
    NEXT_OP(data);
}

// LEA Gw
void decode08d(struct DecodeData* data) {
    U8 rm = FETCH8(data);	
    if (data->ea16) {
        data->op->func =lear16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);		
    } else {
        data->op->func = lear16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
    }	
    data->op->base = SEG_ZERO;
    LOG_OP2("LEA", RW(data->op->r1), M16(data, rm, data->op));
    NEXT_OP(data);
}

// LEA Gd
void decode28d(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (data->ea16) {
        data->op->func =lear32_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
    } else {
        data->op->func = lear32_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
    }
    data->op->base = SEG_ZERO;
    LOG_OP2("LEA", RD(data->op->r1), M32(data, rm, data->op));
    NEXT_OP(data);
}

// MOV Sw,Ew
void decode08e(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = movs16r16;
        data->op->r1 = E(rm);
        data->op->r2 = G(rm);
        LOG_OP2("MOV", EABASE(data->op->r2), RW(data->op->r1));
    } else if (data->ea16) {
        data->op->func = movs16e16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("MOV", EABASE(data->op->r1), M16(data, rm, data->op));
    } else {
        data->op->func = movs16e16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("MOV", EABASE(data->op->r1), M16(data, rm, data->op));
    }
    NEXT_OP(data);
}

// POP Ew
void decode08f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = popReg16;
        data->op->r1 = E(rm);		
    } else if (data->ea16) {
        data->op->func = pope16_16;
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP1("POP", M16(data, rm, data->op));
    } else {
        data->op->func = pope16_32;
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_2(data->op);
        LOG_OP1("POP", M16(data, rm, data->op));
    }
    NEXT_OP(data);
}

// POP Ed
void decode28f(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = popReg32;
        data->op->r1 = E(rm);
    } else if (data->ea16) {
        data->op->func = pope32_16;
        decodeEa16(data, rm);
        WRITE_ADDRESS_16(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP1("POP", M32(data, rm, data->op));
    } else {
        data->op->func = pope32_32;
        decodeEa32(data, rm);
        WRITE_ADDRESS_32(data->op);
        WRITE_BYTES_4(data->op);
        LOG_OP1("POP", M32(data, rm, data->op));
    }
    NEXT_OP(data);
}

// NOP
void decode090(struct DecodeData* data) {
    data->op->func = nop;
    LOG_OP("NOP");
    NEXT_OP(data);
}

// CALL Ap 
void decode09a(struct DecodeData* data) {
    data->op->func = callAp;
    data->op->data1 = FETCH16(data);
    data->op->eData = FETCH16(data);
    LOG_OP_CS_EIP("CALL Ap", data->op->eData, data->op->data1);
    FINISH_OP(data);
}

// Wait
void decode09b(struct DecodeData* data) {
    data->op->func = nop;
    LOG_OP("WAIT");
    NEXT_OP(data);
}

// GRP2 Eb,Ib
void decode0c0(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol8_reg_op, rol8_mem16_op, rol8_mem32_op); break;
    case 1: DECODE_E(ror8_reg_op, ror8_mem16_op, ror8_mem32_op); break;
    case 2: DECODE_E(rcl8_reg_op, rcl8_mem16_op, rcl8_mem32_op); break;
    case 3: DECODE_E(rcr8_reg_op, rcr8_mem16_op, rcr8_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl8_reg_op, shl8_mem16_op, shl8_mem32_op); break;
    case 5: DECODE_E(shr8_reg_op, shr8_mem16_op, shr8_mem32_op); break;
    case 7: DECODE_E(sar8_reg_op, sar8_mem16_op, sar8_mem32_op); break;
    }	
    WRITE_BYTES_1(data->op);
    data->op->data1 = FETCH8(data) & 0x1F;
    if (data->op->data1==0) {
        RESTART(data);	
        RESTART_OP(data);
        return;
    }
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E8C("ROL", rm, data); break;
    case 1: LOG_E8C("ROR", rm, data); break;
    case 2: LOG_E8C("RCL", rm, data); break;
    case 3: LOG_E8C("RCR", rm, data); break;	
    case 4: LOG_E8C("SHL", rm, data); break;
    case 5: LOG_E8C("SHR", rm, data); break;
    case 6: LOG_E8C("SAL", rm, data); break;
    case 7: LOG_E8C("SAR", rm, data); break;
    }	
#endif
    switch (G(rm)) {
        case 0: data->op->data1 &= 0x7; break;
        case 1: data->op->data1 &= 0x7; break;
        case 2: data->op->data1 = data->op->data1 % 9; break;
        case 3: data->op->data1 = data->op->data1 % 9; break;
        default: break;
    }
    NEXT_OP(data);
}

// GRP2 Ew,Ib
void decode0c1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol16_reg_op, rol16_mem16_op, rol16_mem32_op); break;
    case 1: DECODE_E(ror16_reg_op, ror16_mem16_op, ror16_mem32_op); break;
    case 2: DECODE_E(rcl16_reg_op, rcl16_mem16_op, rcl16_mem32_op); break;
    case 3: DECODE_E(rcr16_reg_op, rcr16_mem16_op, rcr16_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl16_reg_op, shl16_mem16_op, shl16_mem32_op); break;
    case 5: DECODE_E(shr16_reg_op, shr16_mem16_op, shr16_mem32_op); break;
    case 7: DECODE_E(sar16_reg_op, sar16_mem16_op, sar16_mem32_op); break;
    }		
    WRITE_BYTES_2(data->op);
    data->op->data1 = FETCH8(data) & 0x1F;
    if (data->op->data1==0) {
        RESTART(data);	
        RESTART_OP(data);
        return;
    }
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E16C("ROL", rm, data); break;
    case 1: LOG_E16C("ROR", rm, data); break;
    case 2: LOG_E16C("RCL", rm, data); break;
    case 3: LOG_E16C("RCR", rm, data); break;	
    case 4: LOG_E16C("SHL", rm, data); break;
    case 5: LOG_E16C("SHR", rm, data); break;
    case 6: LOG_E16C("SAL", rm, data); break;
    case 7: LOG_E16C("SAR", rm, data); break;
    }	
#endif
    switch (G(rm)) {
        case 0: data->op->data1 &= 0xf; break;
        case 1: data->op->data1 &= 0xf; break;
        case 2: data->op->data1 = data->op->data1 % 17; break;
        case 3: data->op->data1 = data->op->data1 % 17; break;
        default: break;
    }
    NEXT_OP(data);
}

// GRP2 Ed,Ib
void decode2c1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol32_reg_op, rol32_mem16_op, rol32_mem32_op); break;
    case 1: DECODE_E(ror32_reg_op, ror32_mem16_op, ror32_mem32_op); break;
    case 2: DECODE_E(rcl32_reg_op, rcl32_mem16_op, rcl32_mem32_op); break;
    case 3: DECODE_E(rcr32_reg_op, rcr32_mem16_op, rcr32_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl32_reg_op, shl32_mem16_op, shl32_mem32_op); break;
    case 5: DECODE_E(shr32_reg_op, shr32_mem16_op, shr32_mem32_op); break;
    case 7: DECODE_E(sar32_reg_op, sar32_mem16_op, sar32_mem32_op); break;
    }		
    WRITE_BYTES_4(data->op);
    data->op->data1 = FETCH8(data) & 0x1F;
    if (data->op->data1==0) {
        RESTART(data);	
        RESTART_OP(data);
        return;
    }
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E32C("ROL", rm, data); break;
    case 1: LOG_E32C("ROR", rm, data); break;
    case 2: LOG_E32C("RCL", rm, data); break;
    case 3: LOG_E32C("RCR", rm, data); break;	
    case 4: LOG_E32C("SHL", rm, data); break;
    case 5: LOG_E32C("SHR", rm, data); break;
    case 6: LOG_E32C("SAL", rm, data); break;
    case 7: LOG_E32C("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// ENTER
void decode0c8(struct DecodeData* data) {
    data->op->func = enter16;
    data->op->data1 = FETCH16(data);
    data->op->r1 = FETCH8(data) & 0x1f;
    LOG_OP("ENTER");
    NEXT_OP(data);
}

void decode2c8(struct DecodeData* data) {
    data->op->func = enter32;
    data->op->data1 = FETCH16(data);
    data->op->r1 = FETCH8(data) & 0x1f;
    LOG_OP("ENTER");
    NEXT_OP(data);
}

// RETF
void decode2cb(struct DecodeData* data) {
    data->op->func = retf32;
    LOG_OP("RETF");
    FINISH_OP(data);
}

void decode0cb(struct DecodeData* data) {
    data->op->func = retf16;
    LOG_OP("RETF");
    FINISH_OP(data);
}

// INT Ib
void decode0cd(struct DecodeData* data) {
    U8 i = FETCH8(data);
    if (i==0x80) {
        data->op->func = syscall_op;
        LOG_OP("INT 80");
        FINISH_OP(data);		
    } else if (i==0x99) {
        data->op->func = int99;
        LOG_OP("INT 99");
        NEXT_OP(data);
    } else if (i == 0x98) {
        data->op->func = int98;
        LOG_OP("INT 98");
        NEXT_OP(data);
    } else {
        data->op->func = intOp;
        data->op->data1 = i;
        LOG_OP("INT");
        FINISH_OP(data);
    }
}

// IRET
void decode2cf(struct DecodeData* data) {
    data->op->func = iret32;
    LOG_OP("IRET");
    FINISH_OP(data);
}

// GRP2 Eb,1
void decode0d0(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol8_reg_op, rol8_mem16_op, rol8_mem32_op); break;
    case 1: DECODE_E(ror8_reg_op, ror8_mem16_op, ror8_mem32_op); break;
    case 2: DECODE_E(rcl8_reg_op, rcl8_mem16_op, rcl8_mem32_op); break;
    case 3: DECODE_E(rcr8_reg_op, rcr8_mem16_op, rcr8_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl8_reg_op, shl8_mem16_op, shl8_mem32_op); break;
    case 5: DECODE_E(shr8_reg_op, shr8_mem16_op, shr8_mem32_op); break;
    case 7: DECODE_E(sar8_reg_op, sar8_mem16_op, sar8_mem32_op); break;
    }	
    WRITE_BYTES_1(data->op);
    data->op->data1 = 1;
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E8C("ROL", rm, data); break;
    case 1: LOG_E8C("ROR", rm, data); break;
    case 2: LOG_E8C("RCL", rm, data); break;
    case 3: LOG_E8C("RCR", rm, data); break;	
    case 4: LOG_E8C("SHL", rm, data); break;
    case 5: LOG_E8C("SHR", rm, data); break;
    case 6: LOG_E8C("SAL", rm, data); break;
    case 7: LOG_E8C("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// GRP2 Ew,1
void decode0d1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol16_reg_op, rol16_mem16_op, rol16_mem32_op); break;
    case 1: DECODE_E(ror16_reg_op, ror16_mem16_op, ror16_mem32_op); break;
    case 2: DECODE_E(rcl16_reg_op, rcl16_mem16_op, rcl16_mem32_op); break;
    case 3: DECODE_E(rcr16_reg_op, rcr16_mem16_op, rcr16_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl16_reg_op, shl16_mem16_op, shl16_mem32_op); break;
    case 5: DECODE_E(shr16_reg_op, shr16_mem16_op, shr16_mem32_op); break;
    case 7: DECODE_E(sar16_reg_op, sar16_mem16_op, sar16_mem32_op); break;
    }		
    WRITE_BYTES_2(data->op);
    data->op->data1 = 1;
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E16C("ROL", rm, data); break;
    case 1: LOG_E16C("ROR", rm, data); break;
    case 2: LOG_E16C("RCL", rm, data); break;
    case 3: LOG_E16C("RCR", rm, data); break;	
    case 4: LOG_E16C("SHL", rm, data); break;
    case 5: LOG_E16C("SHR", rm, data); break;
    case 6: LOG_E16C("SAL", rm, data); break;
    case 7: LOG_E16C("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// GRP2 Ed,1
void decode2d1(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol32_reg_op, rol32_mem16_op, rol32_mem32_op); break;
    case 1: DECODE_E(ror32_reg_op, ror32_mem16_op, ror32_mem32_op); break;
    case 2: DECODE_E(rcl32_reg_op, rcl32_mem16_op, rcl32_mem32_op); break;
    case 3: DECODE_E(rcr32_reg_op, rcr32_mem16_op, rcr32_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl32_reg_op, shl32_mem16_op, shl32_mem32_op); break;
    case 5: DECODE_E(shr32_reg_op, shr32_mem16_op, shr32_mem32_op); break;
    case 7: DECODE_E(sar32_reg_op, sar32_mem16_op, sar32_mem32_op); break;
    }	
    WRITE_BYTES_4(data->op);
    data->op->data1 = 1;
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E32C("ROL", rm, data); break;
    case 1: LOG_E32C("ROR", rm, data); break;
    case 2: LOG_E32C("RCL", rm, data); break;
    case 3: LOG_E32C("RCR", rm, data); break;	
    case 4: LOG_E32C("SHL", rm, data); break;
    case 5: LOG_E32C("SHR", rm, data); break;
    case 6: LOG_E32C("SAL", rm, data); break;
    case 7: LOG_E32C("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// GRP2 Eb,CL
void decode0d2(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol8cl_reg_op, rol8cl_mem16_op, rol8cl_mem32_op); break;
    case 1: DECODE_E(ror8cl_reg_op, ror8cl_mem16_op, ror8cl_mem32_op); break;
    case 2: DECODE_E(rcl8cl_reg_op, rcl8cl_mem16_op, rcl8cl_mem32_op); break;
    case 3: DECODE_E(rcr8cl_reg_op, rcr8cl_mem16_op, rcr8cl_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl8cl_reg_op, shl8cl_mem16_op, shl8cl_mem32_op); break;
    case 5: DECODE_E(shr8cl_reg_op, shr8cl_mem16_op, shr8cl_mem32_op); break;
    case 7: DECODE_E(sar8cl_reg_op, sar8cl_mem16_op, sar8cl_mem32_op); break;
    }	
    WRITE_BYTES_1(data->op);
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E8Cl("ROL", rm, data); break;
    case 1: LOG_E8Cl("ROR", rm, data); break;
    case 2: LOG_E8Cl("RCL", rm, data); break;
    case 3: LOG_E8Cl("RCR", rm, data); break;	
    case 4: LOG_E8Cl("SHL", rm, data); break;
    case 5: LOG_E8Cl("SHR", rm, data); break;
    case 6: LOG_E8Cl("SAL", rm, data); break;
    case 7: LOG_E8Cl("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// GRP2 Ew,CL
void decode0d3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol16cl_reg_op, rol16cl_mem16_op, rol16cl_mem32_op); break;
    case 1: DECODE_E(ror16cl_reg_op, ror16cl_mem16_op, ror16cl_mem32_op); break;
    case 2: DECODE_E(rcl16cl_reg_op, rcl16cl_mem16_op, rcl16cl_mem32_op); break;
    case 3: DECODE_E(rcr16cl_reg_op, rcr16cl_mem16_op, rcr16cl_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl16cl_reg_op, shl16cl_mem16_op, shl16cl_mem32_op); break;
    case 5: DECODE_E(shr16cl_reg_op, shr16cl_mem16_op, shr16cl_mem32_op); break;
    case 7: DECODE_E(sar16cl_reg_op, sar16cl_mem16_op, sar16cl_mem32_op); break;
    }	
    WRITE_BYTES_2(data->op);
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E16Cl("ROL", rm, data); break;
    case 1: LOG_E16Cl("ROR", rm, data); break;
    case 2: LOG_E16Cl("RCL", rm, data); break;
    case 3: LOG_E16Cl("RCR", rm, data); break;	
    case 4: LOG_E16Cl("SHL", rm, data); break;
    case 5: LOG_E16Cl("SHR", rm, data); break;
    case 6: LOG_E16Cl("SAL", rm, data); break;
    case 7: LOG_E16Cl("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// GRP2 Ed,CL
void decode2d3(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    switch (G(rm)) {
    case 0: DECODE_E(rol32cl_reg_op, rol32cl_mem16_op, rol32cl_mem32_op); break;
    case 1: DECODE_E(ror32cl_reg_op, ror32cl_mem16_op, ror32cl_mem32_op); break;
    case 2: DECODE_E(rcl32cl_reg_op, rcl32cl_mem16_op, rcl32cl_mem32_op); break;
    case 3: DECODE_E(rcr32cl_reg_op, rcr32cl_mem16_op, rcr32cl_mem32_op); break;
    case 6:
    case 4: DECODE_E(shl32cl_reg_op, shl32cl_mem16_op, shl32cl_mem32_op); break;
    case 5: DECODE_E(shr32cl_reg_op, shr32cl_mem16_op, shr32cl_mem32_op); break;
    case 7: DECODE_E(sar32cl_reg_op, sar32cl_mem16_op, sar32cl_mem32_op); break;
    }
    WRITE_BYTES_4(data->op);
#ifdef LOG_OPS
    switch (G(rm)) {
    case 0: LOG_E32Cl("ROL", rm, data); break;
    case 1: LOG_E32Cl("ROR", rm, data); break;
    case 2: LOG_E32Cl("RCL", rm, data); break;
    case 3: LOG_E32Cl("RCR", rm, data); break;	
    case 4: LOG_E32Cl("SHL", rm, data); break;
    case 5: LOG_E32Cl("SHR", rm, data); break;
    case 6: LOG_E32Cl("SAL", rm, data); break;
    case 7: LOG_E32Cl("SAR", rm, data); break;
    }	
#endif
    NEXT_OP(data);
}

// AAM Ib
void decode0d4(struct DecodeData* data) {
    data->op->data1 = FETCH8(data);
    data->op->func = aam;
    LOG_OP("AAM");
    NEXT_OP(data);
}

// AAD Ib
void decode0d5(struct DecodeData* data) {
    data->op->data1 = FETCH8(data);
    data->op->func = aad;
    LOG_OP("AAD");
    NEXT_OP(data);
}

// XLAT
void decode0d7(struct DecodeData* data) {
    if (data->ea16)
        data->op->func = xlat16;
    else
        data->op->func = xlat32;
    data->op->base = data->ds;
    LOG_OP("XLAT");
    NEXT_OP(data);
}

// FPU ESC 0
void decode0d8(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) {
        data->op->r1 = E(rm);
        switch (G(rm)) {
        case 0: data->op->func = FADD_ST0_STj; break;
        case 1: data->op->func = FMUL_ST0_STj; break;
        case 2: data->op->func = FCOM_STi; break;
        case 3: data->op->func = FCOM_STi_Pop; break;
        case 4: data->op->func = FSUB_ST0_STj; break;
        case 5: data->op->func = FSUBR_ST0_STj; break;
        case 6: data->op->func = FDIV_ST0_STj; break;
        case 7: data->op->func = FDIVR_ST0_STj; break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch (G(rm)) {
            case 0: data->op->func = FADD_SINGLE_REAL_16; break;
            case 1: data->op->func = FMUL_SINGLE_REAL_16; break;
            case 2: data->op->func = FCOM_SINGLE_REAL_16; break;
            case 3: data->op->func = FCOM_SINGLE_REAL_16_Pop; break;
            case 4: data->op->func = FSUB_SINGLE_REAL_16; break;
            case 5: data->op->func = FSUBR_SINGLE_REAL_16; break;
            case 6: data->op->func = FDIV_SINGLE_REAL_16; break;
            case 7: data->op->func = FDIVR_SINGLE_REAL_16; break;
        }					
    } else {
        decodeEa32(data, rm);
        switch (G(rm)) {
            case 0: data->op->func = FADD_SINGLE_REAL_32; break;
            case 1: data->op->func = FMUL_SINGLE_REAL_32; break;
            case 2: data->op->func = FCOM_SINGLE_REAL_32; break;
            case 3: data->op->func = FCOM_SINGLE_REAL_32_Pop; break;
            case 4: data->op->func = FSUB_SINGLE_REAL_32; break;
            case 5: data->op->func = FSUBR_SINGLE_REAL_32; break;
            case 6: data->op->func = FDIV_SINGLE_REAL_32; break;
            case 7: data->op->func = FDIVR_SINGLE_REAL_32; break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 1
void decode0d9(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) {				
        switch ((rm >> 3) & 7) {
            case 0: data->op->r1 = E(rm); data->op->func = FLD_STi; break;
            case 1: data->op->r1 = E(rm); data->op->func = FXCH_STi; break;
            case 2: data->op->func = FNOP; break;
            case 3: data->op->r1 = E(rm); data->op->func = FST_STi_Pop; break;
            case 4:
            {
                switch (rm & 7) {
                    case 0: data->op->func = FCHS; break;
                    case 1: data->op->func = FABS; break;
                    case 4: data->op->func = FTST; break;
                    case 5: data->op->func = FXAM; break;
                    default: kpanic("ESC 1:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
                }
                break;
            }
            case 5:
            {
                switch (rm & 7) {
                    case 0: data->op->func = FLD1; break;
                    case 1: data->op->func = FLDL2T; break;
                    case 2: data->op->func = FLDL2E; break;
                    case 3: data->op->func = FLDPI; break;
                    case 4: data->op->func = FLDLG2; break;
                    case 5: data->op->func = FLDLN2; break;
                    case 6: data->op->func = FLDZ; break;
                    case 7: kpanic("ESC 1:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
                }
                break;
            }
            case 6:
            {
                switch (rm & 7) {
                    case 0: data->op->func = F2XM1; break;
                    case 1: data->op->func = FYL2X; break;
                    case 2: data->op->func = FPTAN; break;
                    case 3: data->op->func = FPATAN; break;
                    case 4: data->op->func = FXTRACT; break;
                    case 5: data->op->func = FPREM_nearest; break;
                    case 6: data->op->func = FDECSTP; break;
                    case 7: data->op->func = FINCSTP; break;
                }
                break;
            }
            case 7:
            {
                switch (rm & 7) {
                    case 0: data->op->func = FPREM; break;
                    case 1: data->op->func = FYL2XP1; break;
                    case 2: data->op->func = FSQRT; break;
                    case 3: data->op->func = FSINCOS; break;
                    case 4: data->op->func = FRNDINT; break;
                    case 5: data->op->func = FSCALE; break;
                    case 6: data->op->func = FSIN; break;
                    case 7: data->op->func = FCOS; break;
                }
                break;
            }
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FLD_SINGLE_REAL_16; break;
            case 1: kpanic("ESC 1 EA:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
            case 2: data->op->func = FST_SINGLE_REAL_16; break;
            case 3: data->op->func = FST_SINGLE_REAL_16_Pop; break;
            case 4: data->op->func = FLDENV_16; break;
            case 5: data->op->func = FLDCW_16; break;
            case 6: data->op->func = FNSTENV_16; break;
            case 7: data->op->func = FNSTCW_16; break;
        }
    } else {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FLD_SINGLE_REAL_32; break;
            case 1: kpanic("ESC 1 EA:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
            case 2: data->op->func = FST_SINGLE_REAL_32; break;
            case 3: data->op->func = FST_SINGLE_REAL_32_Pop; break;
            case 4: data->op->func = FLDENV_32; break;
            case 5: data->op->func = FLDCW_32; break;
            case 6: data->op->func = FNSTENV_32; break;
            case 7: data->op->func = FNSTCW_32; break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 2
void decode0da(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    if (rm >= 0xc0) {
        data->op->r1 = E(rm);         
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FCMOV_ST0_STj_CF; break;
            case 1: data->op->func = FCMOV_ST0_STj_ZF; break;
            case 2: data->op->func = FCMOV_ST0_STj_CF_OR_ZF; break;
            case 3: data->op->func = FCMOV_ST0_STj_PF; break;
            case 5:
                if ((rm & 7)==1) {
                    data->op->func = FUCOMPP;
                    break;
                }
            // intentional fall through
            default:
                kpanic("ESC 1 EA:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FIADD_DWORD_INTEGER_16; break;
            case 1: data->op->func = FIMUL_DWORD_INTEGER_16; break;
            case 2: data->op->func = FICOM_DWORD_INTEGER_16; break;
            case 3: data->op->func = FICOM_DWORD_INTEGER_16_Pop; break;
            case 4: data->op->func = FISUB_DWORD_INTEGER_16; break;
            case 5: data->op->func = FISUBR_DWORD_INTEGER_16; break;
            case 6: data->op->func = FIDIV_DWORD_INTEGER_16; break;
            case 7: data->op->func = FIDIVR_DWORD_INTEGER_16; break;
        }
    } else {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FIADD_DWORD_INTEGER_32; break;
            case 1: data->op->func = FIMUL_DWORD_INTEGER_32; break;
            case 2: data->op->func = FICOM_DWORD_INTEGER_32; break;
            case 3: data->op->func = FICOM_DWORD_INTEGER_32_Pop; break;
            case 4: data->op->func = FISUB_DWORD_INTEGER_32; break;
            case 5: data->op->func = FISUBR_DWORD_INTEGER_32; break;
            case 6: data->op->func = FIDIV_DWORD_INTEGER_32; break;
            case 7: data->op->func = FIDIVR_DWORD_INTEGER_32; break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 3
void decode0db(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    if (rm >= 0xc0) {
        data->op->r1 = E(rm); 
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FCMOV_ST0_STj_NCF; break;
            case 1: data->op->func = FCMOV_ST0_STj_NZF; break;
            case 2: data->op->func = FCMOV_ST0_STj_NCF_AND_NZF; break;
            case 3: data->op->func = FCMOV_ST0_STj_NPF; break;
            case 4:
            {
                switch (rm & 7) {
                    case 2:data->op->func = FNCLEX; break;
                    case 3:data->op->func = FNINIT; break;
                    default:kpanic("ESC 3 :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
                }
                break;
            }
            case 5: data->op->func = FUCOMI_ST0_STj; break;
            case 6: data->op->func = FCOMI_ST0_STj_Pop; break;
            default:kpanic("ESC 3 :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FILD_DWORD_INTEGER_16; break;
            case 1: data->op->func = FISTTP32_16; break;
            case 2: data->op->func = FIST_DWORD_INTEGER_16; break;
            case 3: data->op->func = FIST_DWORD_INTEGER_16_Pop; break;
            case 5: data->op->func = FLD_EXTENDED_REAL_16; break;
            case 7: data->op->func = FSTP_EXTENDED_REAL_16; break;
            default:kpanic("ESC 3 EA :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
        }
    } else {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FILD_DWORD_INTEGER_32; break;
            case 1: data->op->func = FISTTP32_32; break;
            case 2: data->op->func = FIST_DWORD_INTEGER_32; break;
            case 3: data->op->func = FIST_DWORD_INTEGER_32_Pop; break;
            case 5: data->op->func = FLD_EXTENDED_REAL_32; break;
            case 7: data->op->func = FSTP_EXTENDED_REAL_32; break;
            default:kpanic("ESC 3 EA :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 4
void decode0dc(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) {
        data->op->r1 = E(rm); 
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FADD_STi_ST0; break;
            case 1: data->op->func = FMUL_STi_ST0; break;
            case 2: data->op->func = FCOM_STi; break;
            case 3: data->op->func = FCOM_STi_Pop; break;
            case 4: data->op->func = FSUBR_STi_ST0; break;
            case 5: data->op->func = FSUB_STi_ST0; break;
            case 6: data->op->func = FDIVR_STi_ST0; break;
            case 7: data->op->func = FDIV_STi_ST0; break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FADD_DOUBLE_REAL_16; break;
            case 1: data->op->func = FMUL_DOUBLE_REAL_16; break;
            case 2: data->op->func = FCOM_DOUBLE_REAL_16; break;
            case 3: data->op->func = FCOM_DOUBLE_REAL_16_Pop; break;
            case 4: data->op->func = FSUB_DOUBLE_REAL_16; break;
            case 5: data->op->func = FSUBR_DOUBLE_REAL_16; break;
            case 6: data->op->func = FDIV_DOUBLE_REAL_16; break;
            case 7: data->op->func = FDIVR_DOUBLE_REAL_16; break;
        }
    } else  {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FADD_DOUBLE_REAL_32; break;
            case 1: data->op->func = FMUL_DOUBLE_REAL_32; break;
            case 2: data->op->func = FCOM_DOUBLE_REAL_32; break;
            case 3: data->op->func = FCOM_DOUBLE_REAL_32_Pop; break;
            case 4: data->op->func = FSUB_DOUBLE_REAL_32; break;
            case 5: data->op->func = FSUBR_DOUBLE_REAL_32; break;
            case 6: data->op->func = FDIV_DOUBLE_REAL_32; break;
            case 7: data->op->func = FDIVR_DOUBLE_REAL_32; break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 5
void decode0dd(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) {
        data->op->r1 = E(rm); 
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FFREE_STi; break;
            case 1: data->op->func = FXCH_STi; break;
            case 2: data->op->func = FST_STi; break;
            case 3: data->op->func = FST_STi_Pop; break;
            case 4: data->op->func = FUCOM_STi; break;
            case 5: data->op->func = FUCOM_STi_Pop; break;
            default:kpanic("ESC 5 :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FLD_DOUBLE_REAL_16; break;
            case 1: data->op->func = FISTTP64_16; break;
            case 2: data->op->func = FST_DOUBLE_REAL_16; break;
            case 3: data->op->func = FST_DOUBLE_REAL_16_Pop; break;
            case 4: data->op->func = FRSTOR_16; break;
            case 5:kpanic("ESC 5 EA:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
            case 6: data->op->func = FNSAVE_16; break;
            case 7: data->op->func = FNSTSW_16; break;
        }
    } else {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FLD_DOUBLE_REAL_32; break;
            case 1: data->op->func = FISTTP64_32; break;
            case 2: data->op->func = FST_DOUBLE_REAL_32; break;
            case 3: data->op->func = FST_DOUBLE_REAL_32_Pop; break;
            case 4: data->op->func = FRSTOR_32; break;
            case 5:kpanic("ESC 5 EA:Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
            case 6: data->op->func = FNSAVE_32; break;
            case 7: data->op->func = FNSTSW_32; break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 6
void decode0de(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) {
        data->op->r1 = E(rm); 
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FADD_STi_ST0_Pop; break;
            case 1: data->op->func = FMUL_STi_ST0_Pop; break;
            case 2: data->op->func = FCOM_STi_Pop; break;
            case 3:
                if ((rm & 7) == 1)
                    data->op->func = FCOMPP;
                else {
                    kpanic("ESC 6 :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7));
                }
                break;
            break;
            case 4: data->op->func = FSUBR_STi_ST0_Pop; break;
            case 5: data->op->func = FSUB_STi_ST0_Pop; break;
            case 6: data->op->func = FDIVR_STi_ST0_Pop; break;
            case 7: data->op->func = FDIV_STi_ST0_Pop; break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FIADD_WORD_INTEGER_16; break;
            case 1: data->op->func = FIMUL_WORD_INTEGER_16; break;
            case 2: data->op->func = FICOM_WORD_INTEGER_16; break;
            case 3: data->op->func = FICOM_WORD_INTEGER_16_Pop; break;
            case 4: data->op->func = FISUB_WORD_INTEGER_16; break;
            case 5: data->op->func = FISUBR_WORD_INTEGER_16; break;
            case 6: data->op->func = FIDIV_WORD_INTEGER_16; break;
            case 7: data->op->func = FIDIVR_WORD_INTEGER_16; break;
        }
    } else {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FIADD_WORD_INTEGER_32; break;
            case 1: data->op->func = FIMUL_WORD_INTEGER_32; break;
            case 2: data->op->func = FICOM_WORD_INTEGER_32; break;
            case 3: data->op->func = FICOM_WORD_INTEGER_32_Pop; break;
            case 4: data->op->func = FISUB_WORD_INTEGER_32; break;
            case 5: data->op->func = FISUBR_WORD_INTEGER_32; break;
            case 6: data->op->func = FIDIV_WORD_INTEGER_32; break;
            case 7: data->op->func = FIDIVR_WORD_INTEGER_32; break;
        }
    }
    NEXT_OP(data);
}

// FPU ESC 7
void decode0df(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    data->op->rm = rm;
    if (rm >= 0xc0) {
        data->op->r1 = E(rm); 
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FFREEP_STi; break;
            case 1: data->op->func = FXCH_STi; break;
            case 2:
            case 3: data->op->func = FST_STi_Pop; break;
            case 4:
                if ((rm & 7)==0)
                    data->op->func = FNSTSW_AX;
                else {
                    kpanic("ESC 7 :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7));
                }
                break;
            case 5: data->op->func = FUCOMI_ST0_STj_Pop; break;
            case 6: data->op->func = FCOMI_ST0_STj_Pop; break;
            case 7: kpanic("ESC 7 :Unhandled group %d subfunction %d", ((rm >> 3) & 7), + (rm & 7)); break;
        }
    } else if (data->ea16) {
        decodeEa16(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FILD_WORD_INTEGER_16; break;
            case 1: data->op->func = FISTTP16_16; break;
            case 2: data->op->func = FIST_WORD_INTEGER_16; break;
            case 3: data->op->func = FIST_WORD_INTEGER_16_Pop; break;
            case 4: data->op->func = FBLD_PACKED_BCD_16; break;
            case 5: data->op->func = FILD_QWORD_INTEGER_16; break;
            case 6: data->op->func = FBSTP_PACKED_BCD_16; break;
            case 7: data->op->func = FISTP_QWORD_INTEGER_16; break;
        }
    } else  {
        decodeEa32(data, rm);
        switch ((rm >> 3) & 7) {
            case 0: data->op->func = FILD_WORD_INTEGER_32; break;
            case 1: data->op->func = FISTTP16_32; break;
            case 2: data->op->func = FIST_WORD_INTEGER_32; break;
            case 3: data->op->func = FIST_WORD_INTEGER_32_Pop; break;
            case 4: data->op->func = FBLD_PACKED_BCD_32; break;
            case 5: data->op->func = FILD_QWORD_INTEGER_32; break;
            case 6: data->op->func = FBSTP_PACKED_BCD_32; break;
            case 7: data->op->func = FISTP_QWORD_INTEGER_32; break;
        }
    }
    NEXT_OP(data);
}

// LOOPNZ
void decode0e0(struct DecodeData* data) {
    if (data->ea16)
        data->op->func = loopnz16;
    else
        data->op->func = loopnz32;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("LOOPNZ", itoa(data->op->data1, data->tmp, 16));
    FINISH_OP(data);
}

// LOOPZ
void decode0e1(struct DecodeData* data) {
    if (data->ea16)
        data->op->func = loopz16;
    else
        data->op->func = loopz32;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("LOOPZ", itoa(data->op->data1, data->tmp, 16));
    FINISH_OP(data);
}

// LOOP
void decode0e2(struct DecodeData* data) {
    if (data->ea16)
        data->op->func = loop16;
    else
        data->op->func = loop32;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("LOOP", itoa(data->op->data1, data->tmp, 10));
    FINISH_OP(data);
}

// JCXZ
void decode0e3(struct DecodeData* data) {
    if (data->ea16)
        data->op->func = jcxz16;
    else
        data->op->func = jcxz32;
    data->op->data1 = FETCH_S8(data);
    LOG_OP1("JCXZ", itoa(data->op->data1, data->tmp, 10));
    FINISH_OP(data);
}

// LOCK
void decode0f0(struct DecodeData* data) {
    RESTART_OP(data);
}

// REPNZ
void decode0f2(struct DecodeData* data) {
    data->rep = 1;
    data->rep_zero = 0;
    RESTART_OP(data);
}

// REPZ
void decode0f3(struct DecodeData* data) {
    data->rep = 1;
    data->rep_zero = 1;
    RESTART_OP(data);
}

// HLT
void decode0f4(struct DecodeData* data) {
    kpanic("HLT");
}

// GRP4 Eb
void decode0fe(struct DecodeData* data) {
    U8 rm=FETCH8(data);
    data->op->rm = rm;
    switch ((rm>>3)&7) {
        case 0x00:										// INC Eb 
            DECODE_E(inc8_reg, inc8_mem16, inc8_mem32);
            WRITE_BYTES_1(data->op);
            LOG_E8("INC", rm, data);
            break;
        case 0x01:										// DEC Eb 
            DECODE_E(dec8_reg, dec8_mem16, dec8_mem32);
            WRITE_BYTES_1(data->op);
            LOG_E8("DEC", rm, data);
            break;
        case 0x07:
            if (sizeof(data->op->func)==8) {
                U64 address = FETCH32(data);
                address |= ((U64)FETCH32(data)) << 32;
                data->op->func = (OpCallback)address;
            } else {
                data->op->func = (OpCallback)FETCH32(data);
            }
            return;
        default:
            kpanic("Illegal GRP4 Call %d, ",((rm>>3) & 7));
            break;
    }
    NEXT_OP(data);
}

// GRP5 Ew
void decode0ff(struct DecodeData* data) {
    U8 rm=FETCH8(data);
    data->op->rm = rm;
    switch ((rm>>3)&7) {
        case 0x00:										// INC Ew 
            DECODE_E(inc16_reg, inc16_mem16, inc16_mem32);
            WRITE_BYTES_2(data->op);
            LOG_E16("INC", rm, data);
            break;
        case 0x01:										// DEC Ew 
            DECODE_E(dec16_reg, dec16_mem16, dec16_mem32);
            WRITE_BYTES_2(data->op);
            LOG_E16("DEC", rm, data);
            break;
        case 0x02:										// CALL Ev 
            DECODE_E(callEv16_reg, callEv16_mem16, callEv16_mem32);
            LOG_E16("CALL", rm, data);
            FINISH_OP(data);
            return;
        case 0x03:										// CALL Ep 
            DECODE_MEMORY(callEp16_mem16, callEp16_mem32);
            LOG_E16("CALL", rm, data);
            FINISH_OP(data);
            break;
        case 0x04:										// JMP Ev 
            DECODE_E(jmpEv16_reg, jmpEv16_mem16, jmpEv16_mem32);
            LOG_E16("JMP", rm, data);
            FINISH_OP(data);
            return;
        case 0x05:										// JMP Ep 
            if (rm>=0xC0) {
                kpanic("Jmp Ep (0xFF) illegal RM");
            } else {
                DECODE_MEMORY(jmpEp16_mem16, jmpEp16_mem32);
                LOG_E16("JMP Ep", rm, data);
                FINISH_OP(data);
            }            
            break;
        case 0x06:										// PUSH Ev 
            DECODE_E(pushEv16_reg, pushEv16_mem16, pushEv16_mem32);
            LOG_E16("PUSH", rm, data);
            break;
        default:
            kpanic("CPU:GRP5:Illegal Call %d", (rm>>3)&7);
            break;
    }
    NEXT_OP(data);
}

// GRP5 Ed
void decode2ff(struct DecodeData* data) {
    U8 rm=FETCH8(data);
    data->op->rm = rm;
    switch ((rm>>3)&7) {
        case 0x00:											// INC Ed 
            DECODE_E(inc32_reg, inc32_mem16, inc32_mem32);
            WRITE_BYTES_4(data->op);
            LOG_E32("INC", rm, data);
            break;
        case 0x01:											// DEC Ed 
            DECODE_E(dec32_reg, dec32_mem16, dec32_mem32);
            WRITE_BYTES_4(data->op);
            LOG_E32("DEC", rm, data);
            break;
        case 0x02:											// CALL NEAR Ed 
            DECODE_E(callNear32_reg, callNear32_mem16, callNear32_mem32);
            LOG_E32("CALL", rm, data);
            FINISH_OP(data);
            return;
        case 0x03:											// CALL FAR Ed 
            kpanic("CALL FAR Ed not implemented");
            break;
        case 0x04:											// JMP NEAR Ed 
            DECODE_E(jmpNear32_reg, jmpNear32_mem16, jmpNear32_mem32);
            LOG_E32("JMP", rm, data);
            FINISH_OP(data);
            return;
        case 0x05:											// JMP FAR Ed 
            kpanic("JMP FAR Ed not implemented");
            break;
        case 0x06:											// Push Ed 
            DECODE_E(pushEd_reg, pushEd_mem16, pushEd_mem32);
            LOG_E32("PUSH", rm, data);
            break;
        default:
            data->cpu>walkStack(data->cpu->eip.u32, data->cpu->reg[5].u32, 2);
            kpanic("CPU:66:GRP5:Illegal call %d", (rm>>3)&7);
            break;
    }
    NEXT_OP(data);
}

// LES
void decode0c4(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) invalidOp(data);
    if (data->ea16) {
        data->op->func = loadSegment16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LES", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = loadSegment16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LES", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = ES;
    NEXT_OP(data);
}

// LDS
void decode0c5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) invalidOp(data);
    if (data->ea16) {
        data->op->func = loadSegment16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LDS", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = loadSegment16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LDS", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = DS;
    NEXT_OP(data);
}

// LSS
void decode1b2(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) invalidOp(data);
    if (data->ea16) {
        data->op->func = loadSegment16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LSS", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = loadSegment16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LSS", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = SS;
    NEXT_OP(data);
}

// LFS
void decode1b4(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) invalidOp(data);
    if (data->ea16) {
        data->op->func = loadSegment16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LGS", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = loadSegment16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LGS", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = FS;
    NEXT_OP(data);
}

// LGS
void decode1b5(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm >= 0xc0) invalidOp(data);
    if (data->ea16) {
        data->op->func = loadSegment16_mem16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LGS", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = loadSegment16_mem32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LGS", M16(data, rm, data->op),R16(data->op->r1));
    }
    data->op->data1 = GS;
    NEXT_OP(data);
}

void decode29a(struct DecodeData* data) {
    data->op->data1 = FETCH32(data);
    data->op->eData = FETCH16(data);
    data->op->func = callFar;
    LOG_OP("CALL");
    FINISH_OP(data);
}

// RETF Iw
void decode2ca(struct DecodeData* data) {
    data->op->data1 = FETCH16(data);
    data->op->func = retf32;
    LOG_OP("RETF");
    FINISH_OP(data);
}

// RETF Iw
void decode0ca(struct DecodeData* data) {
    data->op->data1 = FETCH16(data);
    data->op->func = retf16;
    LOG_OP("RETF");
    FINISH_OP(data);
}

// LAR
void decode102(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = larr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("LAR", R16(data->op->r2),R16(data->op->r1));
    } else if (data->ea16) {
        data->op->func = lare16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LAR", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = lare16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LAR", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}

// LSL
void decode103(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if (rm>=0xC0) {
        data->op->func = lslr16r16;
        data->op->r1 = G(rm);
        data->op->r2 = E(rm);
        LOG_OP2("LSL", R16(data->op->r2),R16(data->op->r1));
    } else if (data->ea16) {
        data->op->func = lsle16r16_16;
        data->op->r1 = G(rm);
        decodeEa16(data, rm);
        LOG_OP2("LSL", M16(data, rm, data->op),R16(data->op->r1));
    } else {
        data->op->func = lsle16r16_32;
        data->op->r1 = G(rm);
        decodeEa32(data, rm);
        LOG_OP2("LSL", M16(data, rm, data->op),R16(data->op->r1));
    }
    NEXT_OP(data);
}

#define DECODE_BT(r, m16, m32) DECODE_E(r, m16, m32); data->op->data1 = 1 << (FETCH8(data) & 31)

// GRP8 Ed,Ib
void decode3ba(struct DecodeData* data) {
    U8 rm=FETCH8(data);
    switch ((rm>>3)&7) {
        case 0x04: // BT 
            DECODE_BT(bt_reg, bt_mem16, bt_mem32);
            LOG_E32("BT", rm, data);
            break;
        case 0x05: // BTS
            DECODE_BT(bts_reg, bts_mem16, bts_mem32);
            LOG_E32("BTS", rm, data);
            break;
        case 0x06: // BTR
            DECODE_BT(btr_reg, btr_mem16, btr_mem32);
            LOG_E32("BTR", rm, data);
            break;
        case 0x07: // BTC
            DECODE_BT(btc_reg, btc_mem16, btc_mem32);
            LOG_E32("BTC", rm, data);
            break;
        default:
            kpanic("GRP8 illegal call %d", (rm>>3)&7);
            break;
    }
    WRITE_BYTES_4(data->op);
    NEXT_OP(data);
}

#define DECODE_BT16(r, m16, m32) DECODE_E(r, m16, m32); data->op->data1 = 1 << (FETCH8(data) & 15)

void decode1ba(struct DecodeData* data) {
    U8 rm=FETCH8(data);
    switch ((rm>>3)&7) {
        case 0x04: // BT 
            DECODE_BT(bt16_reg, bt16_mem16, bt16_mem32);
            LOG_E32("BT", rm, data);
            break;
        case 0x05: // BTS
            DECODE_BT(bts16_reg, bts16_mem16, bts16_mem32);
            LOG_E32("BTS", rm, data);
            break;
        case 0x06: // BTR
            DECODE_BT(btr16_reg, btr16_mem16, btr16_mem32);
            LOG_E32("BTR", rm, data);
            break;
        case 0x07: // BTC
            DECODE_BT(btc16_reg, btc16_mem16, btc16_mem32);
            LOG_E32("BTC", rm, data);
            break;
        default:
            kpanic("GRP8 illegal call %d", (rm>>3)&7);
            break;
    }
    WRITE_BYTES_2(data->op);
    NEXT_OP(data);
}

/* JMP Ap */
void decode0ea(struct DecodeData* data) {
    data->op->func = jmpAp;
    data->op->data1 = FETCH16(data);
    data->op->eData = FETCH16(data);
    LOG_OP_CS_EIP("JMP Ap", data->op->eData, data->op->data1);
    FINISH_OP(data);
};

void decode301(struct DecodeData* data) {
    U8 rm=FETCH8(data);
    U32 which=(rm>>3)&7;

    if (rm < 0xc0)	{
        switch (which) {
        case 0x00:										/* SGDT */
            DECODE_MEMORY(sgdt_mem16, sgdt_mem32);
            NEXT_OP(data);
            break;
        case 0x01:										/* SIDT */
            DECODE_MEMORY(sidt_mem16, sidt_mem32);
            NEXT_OP(data);
            break;
        case 0x04:										/* SMSW */
            DECODE_MEMORY(smsw_mem16, smsw_mem32);
            NEXT_OP(data);
            break;
        case 0x06:										/* LMSW */
            DECODE_MEMORY(lmsw_mem16, lmsw_mem32);
            NEXT_OP(data);
            break;
        default:
            invalidOp(data);
            break;
        }
    } else {
        switch (which) {
        case 0x04: // SMSW
            data->op->func = smsw_reg;
            data->op->r1 = E(rm);
            NEXT_OP(data);
            break;
        case 0x06: // LMSW
            data->op->func = lmsw_reg;
            data->op->r1 = E(rm);
            NEXT_OP(data);
            break;
        default:
            invalidOp(data);
            break;
        }
    }
}

void OPCALL writePort(struct CPU* cpu, struct Op* op) {
    NEXT();
}

void OPCALL readPort8(struct CPU* cpu, struct Op* op) {
    AL=0xFF;
    NEXT();
}

void OPCALL readPort16(struct CPU* cpu, struct Op* op) {
    AX=0xFFFF;
    NEXT();
}

void OPCALL readPort32(struct CPU* cpu, struct Op* op) {
    EAX=0xFFFFFFFF;
    NEXT();
}

void decode0e4(struct DecodeData* data) {
    data->op->data1=FETCH8(data);
    data->op->func = writePort;
    NEXT_OP(data);
}

void decode0e5(struct DecodeData* data) {
    data->op->data1=FETCH8(data);
    data->op->func = readPort8;
    NEXT_OP(data);
}

void decode2e5(struct DecodeData* data) {
    data->op->data1=FETCH8(data);
    data->op->func = readPort32;
    NEXT_OP(data);
}

void decode0e6(struct DecodeData* data) {
    data->op->data1=FETCH8(data);
    data->op->func = readPort16;
    NEXT_OP(data);
}

void decode0e7(struct DecodeData* data) {
    data->op->data1=FETCH8(data);
    data->op->func = writePort;
    NEXT_OP(data);
}

void decode2e7(struct DecodeData* data) {
    data->op->data1=FETCH8(data);
    data->op->func = writePort;
    NEXT_OP(data);
}

// IN AL,DX
void decode0ec(struct DecodeData* data) {    
    data->op->func = readPort8;
    NEXT_OP(data);
}

// IN AX,DX
void decode0ed(struct DecodeData* data) {
    data->op->func = readPort16;
    NEXT_OP(data);
}

// IN EAX,DX
void decode2ed(struct DecodeData* data) {
    data->op->func = readPort32;
    NEXT_OP(data);
}

// OUT DX,AL
void decode0ee(struct DecodeData* data) {
    data->op->func = writePort;
    NEXT_OP(data);
}

// OUT DX,AX 
void decode0ef(struct DecodeData* data) {
    data->op->func = writePort;
    NEXT_OP(data);
}

// OUT DX,EAX 
void decode2ef(struct DecodeData* data) {
    data->op->func = writePort;
    NEXT_OP(data);
}

void decode3c7(struct DecodeData* data) {
    U8 rm = FETCH8(data);
    if ((rm & 0x38) == 8) { // CMPXCHG8B
        if (data->ea16) {
            data->op->func = cmpxchgg8b_16;
            decodeEa16(data, rm);
            LOG_OP1("CMPXCHG8B", M32(data, rm, data->op));
        }
        else {
            data->op->func = cmpxchgg8b_32;
            decodeEa32(data, rm);
            LOG_OP1("CMPXCHG8B", M32(data, rm, data->op));
        }
    } else {
        printf("Invalid instruction %x\n", data->op->inst);
        log_pf(data->cpu->thread, data->ip);
    }	
    NEXT_OP(data);
};

// CLI
void decode0fa(struct DecodeData* data) {
    data->op->func = cli;
    LOG_OP("CLI");
    NEXT_OP(data);
}
// STI
void decode0fb(struct DecodeData* data) {
    data->op->func = sti;
    LOG_OP("STI");
    NEXT_OP(data);
}

DECODER decoder[1024] = {
    decode000, decode001, decode002, decode003, decode004, decode005, decode006, decode007,
    decode008, decode009, decode00a, decode00b, decode00c, decode00d, decode00e, decode00f,
    decode010, decode011, decode012, decode013, decode014, decode015, decode016, decode017,
    decode018, decode019, decode01a, decode01b, decode01c, decode01d, decode01e, decode01f,
    decode020, decode021, decode022, decode023, decode024, decode025, decode026, decode027,
    decode028, decode029, decode02a, decode02b, decode02c, decode02d, decode02e, decode02f,
    decode030, decode031, decode032, decode033, decode034, decode035, decode036, decode037,
    decode038, decode039, decode03a, decode03b, decode03c, decode03d, decode03e, decode03f,
    decode040, decode041, decode042, decode043, decode044, decode045, decode046, decode047,
    decode048, decode049, decode04a, decode04b, decode04c, decode04d, decode04e, decode04f,
    decode050, decode051, decode052, decode053, decode054, decode055, decode056, decode057,
    decode058, decode059, decode05a, decode05b, decode05c, decode05d, decode05e, decode05f,
    decode060, decode061, decode062, invalidOp, decode064, decode065, decode066, decode067,
    decode068, decode069, decode06a, decode06b, decode06c, decode06d, invalidOp, invalidOp,
    decode070, decode071, decode072, decode073, decode074, decode075, decode076, decode077,
    decode078, decode079, decode07a, decode07b, decode07c, decode07d, decode07e, decode07f,
    decode080, decode081, decode080, decode083, decode084, decode085, decode086, decode087,
    decode088, decode089, decode08a, decode08b, decode08c, decode08d, decode08e, decode08f,
    decode090, decode091, decode092, decode093, decode094, decode095, decode096, decode097,
    decode098, decode099, decode09a, decode09b, decode09c, decode09d, decode09e, decode09f,
    decode0a0, decode0a1, decode0a2, decode0a3, decode0a4, decode0a5, decode0a6, decode0a7,
    decode0a8, decode0a9, decode0aa, decode0ab, decode0ac, decode0ad, decode0ae, decode0af,
    decode0b0, decode0b1, decode0b2, decode0b3, decode0b4, decode0b5, decode0b6, decode0b7,
    decode0b8, decode0b9, decode0ba, decode0bb, decode0bc, decode0bd, decode0be, decode0bf,
    decode0c0, decode0c1, decode0c2, decode0c3, decode0c4, decode0c5, decode0c6, decode0c7,
    decode0c8, decode0c9, decode0ca, decode0cb, invalidOp, decode0cd, invalidOp, invalidOp,
    decode0d0, decode0d1, decode0d2, decode0d3, decode0d4, decode0d5, decode0d6, decode0d7,
    decode0d8, decode0d9, decode0da, decode0db, decode0dc, decode0dd, decode0de, decode0df,
    decode0e0, decode0e1, decode0e2, decode0e3, decode0e4, decode0e5, decode0e6, decode0e7,
    decode0e8, decode0e9, decode0ea, decode0eb, decode0ec, decode0ed, decode0ee, decode0ef,
    decode0f0, invalidOp, decode0f2, decode0f3, decode0f4, decode0f5, decode0f6, decode0f7,
    decode0f8, decode0f9, decode0fa, decode0fb, decode0fc, decode0fd, decode0fe, decode0ff,

    // 100
    invalidOp, invalidOp, decode102, decode103, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 110
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 120
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 130
    invalidOp, decode131, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 140
    decode140, decode141, decode142, decode143, decode144, decode145, decode146, decode147,
    decode148, decode149, decode14a, decode14b, decode14c, decode14d, decode14e, decode14f,
    // 150
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 160
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 170
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 180
    decode180, decode181, decode180, decode183, decode184, decode185, decode186, decode187,
    decode188, decode189, decode18a, decode18b, decode18c, decode18d, decode18e, decode18f,
    // 190
    decode190, decode191, decode192, decode193, decode194, decode195, decode196, decode197,
    decode198, decode199, decode19a, decode19b, decode19c, decode19d, decode19e, decode19f,
    // 1a0
    decode1a0, decode1a1, decode1a2, decode1a3, decode1a4, decode1a5, invalidOp, invalidOp,
    decode1a8, decode1a9, invalidOp, decode1ab, decode1ac, decode1ad, invalidOp, decode1af,
    // 1b0
    invalidOp, decode1b1, decode1b2, decode1b3, decode1b4, decode1b5, decode1b6, invalidOp,
    invalidOp, invalidOp, decode1ba, invalidOp, decode1bc, decode1bd, decode1be, invalidOp,
    // 1c0
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 1d0
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 1e0
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 1f0
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,

    decode000, decode201, decode002, decode203, decode004, decode205, decode206, decode207,
    decode008, decode209, decode00a, decode20b, decode00c, decode20d, decode20e, decode00f,
    decode010, decode211, decode012, decode213, decode014, decode215, decode216, decode217,
    decode018, decode219, decode01a, decode21b, decode01c, decode21d, decode21e, decode21f,
    decode020, decode221, decode022, decode223, decode024, decode225, decode026, decode027,
    decode028, decode229, decode02a, decode22b, decode02c, decode22d, decode02e, decode02f,
    decode030, decode231, decode032, decode233, decode034, decode235, decode036, decode037,
    decode038, decode239, decode03a, decode23b, decode03c, decode23d, decode03e, decode03f,
    decode240, decode241, decode242, decode243, decode244, decode245, decode246, decode247,
    decode248, decode249, decode24a, decode24b, decode24c, decode24d, decode24e, decode24f,
    decode250, decode251, decode252, decode253, decode254, decode255, decode256, decode257,
    decode258, decode259, decode25a, decode25b, decode25c, decode25d, decode25e, decode25f,
    decode260, decode261, decode262, invalidOp, decode064, decode065, decode266, decode267,
    decode268, decode269, decode26a, decode26b, decode06c, decode26d, invalidOp, invalidOp,
    decode070, decode071, decode072, decode073, decode074, decode075, decode076, decode077,
    decode078, decode079, decode07a, decode07b, decode07c, decode07d, decode07e, decode07f,
    decode080, decode281, decode080, decode283, decode084, decode285, decode086, decode287,
    decode088, decode289, decode08a, decode28b, decode28c, decode28d, decode08e, decode28f,
    decode090, decode291, decode292, decode293, decode294, decode295, decode296, decode297,
    decode298, decode299, decode29a, decode09b, decode29c, decode29d, decode09e, decode09f,
    decode0a0, decode2a1, decode0a2, decode2a3, decode0a4, decode2a5, decode0a6, decode2a7,
    decode0a8, decode2a9, decode0aa, decode2ab, decode0ac, decode2ad, decode0ae, decode2af,
    decode0b0, decode0b1, decode0b2, decode0b3, decode0b4, decode0b5, decode0b6, decode0b7,
    decode2b8, decode2b9, decode2ba, decode2bb, decode2bc, decode2bd, decode2be, decode2bf,
    decode0c0, decode2c1, decode2c2, decode2c3, invalidOp, invalidOp, decode0c6, decode2c7,
    decode2c8, decode2c9, decode2ca, decode2cb, invalidOp, decode0cd, invalidOp, decode2cf,
    decode0d0, decode2d1, decode0d2, decode2d3, decode0d4, decode0d5, decode0d6, decode0d7,
    decode0d8, decode0d9, decode0da, decode0db, decode0dc, decode0dd, decode0de, decode0df,
    decode0e0, decode0e1, decode0e2, decode0e3, decode0e4, decode2e5, decode0e6, decode2e7,
    decode2e8, decode2e9, invalidOp, decode0eb, decode0ec, decode2ed, decode0ee, decode2ef,
    decode0f0, invalidOp, decode0f2, decode0f3, decode0f4, decode0f5, decode0f6, decode2f7,
    decode0f8, decode0f9, decode0fa, decode0fb, decode0fc, decode0fd, decode0fe, decode2ff,

    // 300
    invalidOp, decode301, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 310
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 320
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 330
    invalidOp, decode131, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 340
    decode340, decode341, decode342, decode343, decode344, decode345, decode346, decode347,
    decode348, decode349, decode34a, decode34b, decode34c, decode34d, decode34e, decode34f,
    // 350
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp,
    // 360
    decode360, decode361, decode362, decode363, decode364, decode365, decode366, decode367,
    decode368, decode369, decode36a, decode36b, invalidOp, invalidOp, decode36e, decode36f,
    // 370
    invalidOp, decode371, decode372, decode373, decode374, decode375, decode376, decode377,
    invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, decode37e, decode37f,
    // 380
    decode380, decode381, decode382, decode383, decode384, decode385, decode386, decode387,
    decode388, decode389, decode38a, decode38b, decode38c, decode38d, decode38e, decode38f,
    // 390
    decode190, decode191, decode192, decode193, decode194, decode195, decode196, decode197,
    decode198, decode199, decode19a, decode19b, decode19c, decode19d, decode19e, decode19f,
    // 3a0
    decode3a0, decode3a1, decode1a2, decode3a3, decode3a4, decode3a5, invalidOp, invalidOp,
    decode3a8, decode3a9, invalidOp, decode3ab, decode3ac, decode3ad, invalidOp, decode3af,
    // 3b0
    invalidOp, decode3b1, invalidOp, decode3b3, invalidOp, invalidOp, decode3b6, decode3b7,
    invalidOp, invalidOp, decode3ba, decode3bb, decode3bc, decode3bd, decode3be, decode3bf,
    // 3c0
    invalidOp, decode3c1, invalidOp, invalidOp, invalidOp, invalidOp, invalidOp, decode3c7,
    decode3c8, decode3c9, decode3ca, decode3cb, decode3cc, decode3cd, decode3ce, decode3cf,
    // 3d0
    invalidOp, decode3d1, decode3d2, decode3d3, invalidOp, decode3d5, invalidOp, invalidOp,
    decode3d8, decode3d9, invalidOp, decode3db, decode3dc, decode3dd, invalidOp, decode3df,
    // 3e0
    invalidOp, decode3e1, decode3e2, invalidOp, invalidOp, decode3e5, invalidOp, invalidOp,
    decode3e8, decode3e9, invalidOp, decode3eb, decode3ec, decode3ed, invalidOp, decode3ef,
    // 3f0
    invalidOp, decode3f1, decode3f2, decode3f3, invalidOp, decode3f5, invalidOp, invalidOp,
    decode3f8, decode3f9, decode3fa, invalidOp, decode3fc, decode3fd, decode3fe, invalidOp,
};

struct Block* decodeBlock(struct CPU* cpu, U32 eip) {	
    struct Block* result;
    result = allocBlock();
    decodeBlockWithBlock(cpu, eip, result);
    return result;	
}

void OPCALL restoreOps(struct CPU* cpu, struct Op* op) {
    op->next = 0;
    freeOp(op);
    decodeBlockWithBlock(cpu, cpu->eip.u32, cpu->currentBlock);
    cpu->currentBlock->ops->func(cpu, cpu->currentBlock->ops);    
}

#ifdef GENERATE_SOURCE
void generateSource(struct CPU* cpu, U32 eip, struct Block* block);
extern U32 gensrc;
#endif
#ifdef AOT
OpCallback getCompiledFunction(U32 crc, const unsigned char* bytes, U32 byteLen, struct KThread* thread, U32 ip);
#include "crc.h"
U32 aot(struct CPU* cpu, struct Block* block, U32 eip) {
    U32 i;
    U32 opPos=0;
    unsigned char ops[1024];
    U32 ip = eip;
    U32 crc;
    OpCallback func = 0;
    struct Op* op = block->ops;

    while (op) {
        for (i=0;i<op->eipCount;i++) {
            // don't generate pf's from AOT
#ifndef BOXEDWINE_64BIT_MMU
            if (!cpu->memory->read[ip>>12])
                return 0;
#endif
            ops[opPos++] = readb(cpu->thread, ip++);
        }
        op = op->next;
    }
    crc = crc32b(ops, opPos);
    func = getCompiledFunction(crc, ops, opPos, cpu->thread, ip);
    if (func) {
        block->ops->func = func;
        freeOp(block->ops->next);
        block->ops->next = 0;     
        // blocks might have been combined, so we need to recache them
        block->block1 = 0;
        block->block2 = 0;
        return 1;
    }
    return 0;
}
#endif
extern struct Block emptyBlock;
void OPCALL firstOp(struct CPU* cpu, struct Op* op) {
    struct Block* block = cpu->currentBlock;
    U32 eip = cpu->eip.u32;

    op->next->func(cpu, op->next);
    if (cpu->thread->process->terminated)
        return;

    // :TODO: should experiment more with different values here
    // Keeping code that isn't used much from being cached reduces memory, it also improves the start time since
    // malloc won't have to be called as much
    if (block->count<20 && block->ops) {
        freeOp(block->ops->next);
        block->ops->func = restoreOps;
        block->ops->next = 0;
    }
    if (block->count==500) {        
        U32 needJIT = 1;
        block->ops = block->ops->next;
        op->next = 0;
        freeOp(op);

#ifdef GENERATE_SOURCE
        if (gensrc) {
            jit(cpu, block, eip);
            generateSource(cpu, eip, block);
            return; // uncompiled block are necessary for source generation, so don't use AOT			
        }
#endif
#ifdef AOT
        if (aot(cpu, block, eip))
            needJIT = 0;
#endif

        if (needJIT) {
            //jit(cpu, block, eip);
        }
    }
}

void decodeBlockWithBlock(struct CPU* cpu, U32 eip, struct Block* block) {	
    struct DecodeData data;
    struct DecodeData* pData = &data;
    struct Op* op;

    block->ops = allocOp();
    block->ops->func = firstOp;
    data.op = allocOp();
    block->ops->next = data.op;

    data.start = data.ip = eip;
    if (cpu->big) {
        data.opCode = 0x200;
        data.ea16 = 0;
    } else {
        data.opCode = 0;
        data.ea16 = 1;
    }

    data.ds = DS;
    data.ss = SS;
    data.rep = 0;
    data.rep_zero = 0;
    data.cpu = cpu;
    data.count = 0;
    data.memory = cpu->memory;
    data.singleOp = 0;
    initDecodeData(pData);    
    data.op->inst = FETCH8(pData)+data.opCode;
    decoder[data.op->inst](pData);
    block->eipCount = 0;
	block->instructionCount = 0;
    op = block->ops;
    while (op) {
        block->eipCount+=op->eipCount;
		block->instructionCount++;
        op = op->next;
    }
}

void OPCALL x64_done_op(struct CPU* cpu, struct Op* op) {
    cpu->eip.u32+=op->eipCount;
}

struct Block* decodeSingleOp(struct CPU* cpu, U32 eip) {	
    struct Block* block = allocBlock();
    struct DecodeData data;
    struct DecodeData* pData = &data;
    struct Op* op;

    block->ops = allocOp();
    data.op = block->ops;

    data.singleOp = 1;
    data.start = data.ip = eip;
    if (cpu->big) {
        data.opCode = 0x200;
        data.ea16 = 0;
    } else {
        data.opCode = 0;
        data.ea16 = 1;
    }

    data.ds = DS;
    data.ss = SS;
    data.rep = 0;
    data.rep_zero = 0;
    data.cpu = cpu;
    data.count = 0;
    data.memory = cpu->memory;
    initDecodeData(pData);    
    data.op->inst = FETCH8(pData)+data.opCode;
    decoder[data.op->inst](pData);
    block->eipCount = 0;
	block->instructionCount = 0;
    op = block->ops;
    while (op) {
        block->eipCount+=op->eipCount;
		block->instructionCount++;
        if (!op->next) {
            op->next = allocOp();
            op->next->func = x64_done_op;
            return block;
        }
        op = op->next;
    }
    kpanic("decodeSingleOp failed");
    return block;
}
