#ifndef __CPU_H__
#define __CPU_H__

#include "lazyFlags.h"
#include "fpu.h"
#include "../decoder.h"

typedef void (*Int99Callback)(CPU* cpu);
extern Int99Callback* int99Callback;
extern U32 int99CallbackSize;
extern Int99Callback* wine_callback;
extern U32 wine_callbackSize;

#define EXCEPTION_DIVIDE 0
#define EXCEPTION_BOUND 5
#define EXCEPTION_PERMISSION 13
#define EXCEPTION_PAGE_FAULT 14

#define AL cpu->reg[0].u8
#define AH cpu->reg[0].h8
#define CL cpu->reg[1].u8
#define CH cpu->reg[1].h8
#define DL cpu->reg[2].u8
#define DH cpu->reg[2].h8
#define BL cpu->reg[3].u8
#define BH cpu->reg[3].h8

#define AX cpu->reg[0].u16
#define CX cpu->reg[1].u16
#define DX cpu->reg[2].u16
#define BX cpu->reg[3].u16
#define SP cpu->reg[4].u16
#define BP cpu->reg[5].u16
#define SI cpu->reg[6].u16
#define DI cpu->reg[7].u16

#define EAX cpu->reg[0].u32
#define ECX cpu->reg[1].u32
#define EDX cpu->reg[2].u32
#define EBX cpu->reg[3].u32
#define ESP cpu->reg[4].u32
#define EBP cpu->reg[5].u32
#define ESI cpu->reg[6].u32
#define EDI cpu->reg[7].u32

#define THIS_EAX this->reg[0].u32
#define THIS_ECX this->reg[1].u32
#define THIS_EDX this->reg[2].u32
#define THIS_EBX this->reg[3].u32
#define THIS_ESP this->reg[4].u32
#define THIS_EBP this->reg[5].u32
#define THIS_ESI this->reg[6].u32
#define THIS_EDI this->reg[7].u32

#define THIS_AX this->reg[0].u16
#define THIS_CX this->reg[1].u16
#define THIS_DX this->reg[2].u16
#define THIS_BX this->reg[3].u16
#define THIS_SP this->reg[4].u16
#define THIS_BP this->reg[5].u16
#define THIS_SI this->reg[6].u16
#define THIS_DI this->reg[7].u16

#define DF		0x00000400

#define TF		0x00000100
#define IF		0x00000200

#define IOPL	0x00003000
#define NT		0x00004000
#define VM		0x00020000
#define AC		0x00040000
#define ID		0x00200000

#define FMASK_TEST (CF | PF | AF | ZF | SF | OF)
#define FMASK_NORMAL (FMASK_TEST | DF | TF | IF | ID | AC)
#define FMASK_ALL (FMASK_NORMAL | IOPL | NT)

#define EXCEPTION_DIVIDE 0
#define EXCEPTION_BOUND 5
#define EXCEPTION_UD 6
#define EXCEPTION_NM 7
#define EXCEPTION_TS 10
#define EXCEPTION_NP 11
#define EXCEPTION_SS 12
#define EXCEPTION_GP 13
#define EXCEPTION_PF 14
#define EXCEPTION_MF 16

#define CR0_PROTECTION          0x00000001
#define CR0_MONITORPROCESSOR    0x00000002
#define CR0_FPUEMULATION        0x00000004
#define CR0_TASKSWITCH          0x00000008
#define CR0_FPUPRESENT          0x00000010
#define CR0_NUMERICERROR        0x00000020
#define CR0_WRITEPROTECT        0x00010000
#define CR0_PAGING              0x80000000

#define ES 0
#define CS 1
#define SS 2
#define DS 3
#define FS 4
#define GS 5
#define SEG_ZERO 6


#define u16 word[0]
#define h16 word[1]
#define u8 byte[0]
#define h8 byte[1]

union Reg {
	U32 u32;
	U16 word[2];
	U8  byte[4];
};

typedef union {

	U64 q;

#ifndef WORDS_BIGENDIAN
	struct {
		U32 d0,d1;
	} ud;

	struct {
		S32 d0,d1;
	} sd;

	struct {
		U16 w0,w1,w2,w3;
	} uw;

	struct {
		S16 w0,w1,w2,w3;
	} sw;

	struct {
		U8 b0,b1,b2,b3,b4,b5,b6,b7;
	} ub;

	struct {
		S8 b0,b1,b2,b3,b4,b5,b6,b7;
	} sb;
#else
	struct {
		U32 d1,d0;
	} ud;

	struct {
		S32 d1,d0;
	} sd;

	struct {
		U16 w3,w2,w1,w0;
	} uw;

	struct {
		S16 w3,w2,w1,w0;
	} sw;

	struct {
		U8 b7,b6,b5,b4,b3,b2,b1,b0;
	} ub;

	struct {
		S8 b7,b6,b5,b4,b3,b2,b1,b0;
	} sb;
#endif

} MMX_reg;

class Seg {
public:
    U32 address;
    U32 value;
};

#define LDT_ENTRIES 8192

struct user_desc {
    U32  entry_number;
    U32 base_addr;
    U32  limit;
    union {
        struct {
            U32  seg_32bit:1;
            U32  contents:2;
            U32  read_exec_only:1;
            U32  limit_in_pages:1;
            U32  seg_not_present:1;
            U32  useable:1;
        };
        U32 flags;
    };
};

class KThread;
class Memory;

class CPU {
public:
    CPU();

    Reg reg[9];
    Seg seg[7];
    U32 flags;
    Reg eip;
    U32 big;
    U8* reg8[9];
    MMX_reg reg_mmx[8];

    Reg  src;
    Reg  dst;
    Reg  dst2;
    Reg  result;
    LazyFlags const * lazyFlags;
    U32	        df;
    U32         oldCF;
    FPU         fpu;
    U64		    instructionCount;
    U32         blockInstructionCount;
    bool        yield;
    U32         cpl;    
    U32 cr0;
    U32 stackNotMask;
    U32 stackMask;

    KThread* thread;
    Memory* memory;
    void* logFile;

    U32 getCF();
    U32 getSF();
    U32 getZF();
    U32 getOF();
    U32 getAF();
    U32 getPF();

    void setCF(U32 value);
    void setOF(U32 value);
    void setSF(U32 value);
    void setZF(U32 value);
    void setPFonValue(U32 value);
    void fillFlagsNoCFOF();
    void fillFlagsNoOF();
    void fillFlagsNoCF();
    void fillFlagsNoZF();
    void fillFlags();
    void setFlags(U32 flags, U32 mask);

    void addFlag(U32 flags);
    void removeFlag(U32 flags);
    void addZF();
    void removeZF();
    void addCF();
    void removeCF();
    void addAF();
    void removeAF();
    void addOF();
    void removeOF();

    U32 pop32();
    U16 pop16();
    U32 peek32(U32 index);
    U16 peek16(U32 index);
    void push16(U16 value);
    void push32(U32 value);

    U32 setSegment(U32 seg, U32 value);
    void prepareException(int code, int error);
    void call(U32 big, U32 selector, U32 offset, U32 oldEip);
    void jmp(U32 big, U32 selector, U32 offset, U32 oldEip);
    void ret(U32 big, U32 bytes);
    void iret(U32 big, U32 oldEip);
    void cpuid();
    void enter(U32 big, U32 bytes, U32 level);
    U32 lsl(U32 selector, U32 limit);
    U32 lar(U32 selector, U32 ar);
    void walkStack(U32 eip, U32 ebp, U32 indent);
    void clone(CPU* from);
    void reset();

    virtual void run()=0;
};

#endif