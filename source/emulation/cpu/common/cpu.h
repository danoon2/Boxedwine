#ifndef __CPU_H__
#define __CPU_H__

#include "lazyFlags.h"
#include "fpu.h"
#include "../decoder.h"
#ifndef SIMDE_NO_NATIVE
#define SIMDE_NO_NATIVE
#endif
#ifndef SIMDE_NO_VECTOR
#define SIMDE_NO_VECTOR
#endif
#ifndef SIMDE_NO_CHECK_IMMEDIATE_CONSTANT
#define SIMDE_NO_CHECK_IMMEDIATE_CONSTANT
#endif

#include "simde/x86/sse.h"
#include "simde/x86/sse2.h"

typedef void (*Int99Callback)(CPU* cpu);
extern Int99Callback* int99Callback;
void callOpenGL(CPU* cpu, U32 index);
void callVulkan(CPU* cpu, U32 index);
extern U32 lastGlCallTime;
extern U32 int99CallbackSize;
extern Int99Callback* wine_callback;
extern Int99Callback* wine_audio_callback;
extern U32 wine_callbackSize;
extern U32 wine_audio_callback_size;
extern U32 wine_audio_callback_base;

#define EXCEPTION_DIVIDE 0
#define EXCEPTION_BOUND 5
#define EXCEPTION_GPF 13
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

union SSE {
    simde__m128 ps;
    simde__m128d pd;
    simde__m128i pi;
};

class CPU {
public:
    static CPU* allocCPU(KMemory* memory);

    CPU(KMemory* memory);
    virtual ~CPU() {}
    
    Reg reg[9];
    Seg seg[7];    
    U32 flags;
    Reg eip;
    U8* reg8[9];  
#ifdef BOXEDWINE_BINARY_TRANSLATOR
    DecodedOp* currentSingleOp;
#endif
    MMX_reg reg_mmx[8];
    ALIGN(SSE xmm[8], 16);

#if defined(BOXEDWINE_BINARY_TRANSLATOR) && defined(BOXEDWINE_X64)
    U64 memcheckq[K_PAGE_SIZE];
    U64 memcheckd[K_PAGE_SIZE];
    U64 memcheckw[K_PAGE_SIZE];
    U64 memcheckqq[K_PAGE_SIZE];
#endif

    Reg  src;
    Reg  dst;
    Reg  dst2;
    Reg  result;
    LazyFlags const * lazyFlags;
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
    KMemory* memory;
    BWriteFile logFile;

    DecodedBlock* nextBlock;
    DecodedBlock* delayedFreeBlock;

    bool getCF();
    bool getSF();
    bool getZF();
    bool getOF();
    bool getAF();
    bool getPF();

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
    int getDirection() {return (this->flags & DF) ? -1 : 1;}
    U32 pop32();
    U16 pop16();
    U32 peek32(U32 index);
    U16 peek16(U32 index);
    void push16(U16 value);
    void push32(U32 value);

    U32 push32_r(U32 esp, U32 value);
    U32 push16_r(U32 esp, U16 value);

    U32 setSegment(U32 seg, U32 value);
    void prepareException(int code, int error);
    void prepareFpuException(int code, int error=0);
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
    void verr(U32 selector);
    void verw(U32 selector);
    U32 readCrx(U32 which, U32 reg);
    U32 writeCrx(U32 which, U32 value);
    void resetMMX();
    bool isMMXinUse();

    U32 getEipAddress();

    virtual void run()=0;
    virtual DecodedBlock* getNextBlock() = 0;
    virtual void restart() {}
    virtual void setSeg(U32 index, U32 address, U32 value);

    bool isBig() {return this->big!=0;}
    virtual void setIsBig(U32 value);

#ifdef BOXEDWINE_DYNAMIC
    static U32 offsetofReg32(U32 index);
    static U32 offsetofReg16(U32 index);
    static U32 offsetofReg8(U32 index);
    static U32 offsetofSegAddress(U32 index);
    static U32 offsetofSegValue(U32 index);
#endif

#ifndef __TEST
protected:    
#endif
    U32 big;
};

void common_prepareException(CPU* cpu, int code, int error);

// until I can figure out how to call cpp function directly from asm
U32 common_getCF(CPU* cpu);
U32 common_condition_o(CPU* cpu);
U32 common_condition_no(CPU* cpu);
U32 common_condition_b(CPU* cpu);
U32 common_condition_nb(CPU* cpu);
U32 common_condition_z(CPU* cpu);
U32 common_condition_nz(CPU* cpu);
U32 common_condition_be(CPU* cpu);
U32 common_condition_nbe(CPU* cpu);
U32 common_condition_s(CPU* cpu);
U32 common_condition_ns(CPU* cpu);
U32 common_condition_p(CPU* cpu);
U32 common_condition_np(CPU* cpu);
U32 common_condition_l(CPU* cpu);
U32 common_condition_nl(CPU* cpu);
U32 common_condition_le(CPU* cpu);
U32 common_condition_nle(CPU* cpu);

U32 common_pop32(CPU* cpu);
U16 common_pop16(CPU* cpu);
void common_push16(CPU* cpu, U16 value);
void common_push32(CPU* cpu, U32 value);
U32 common_peek32(CPU* cpu, U32 index);
U16 common_peek16(CPU* cpu, U32 index);
U32 common_setSegment(CPU* cpu, U32 seg, U32 value);
void common_setFlags(CPU* cpu, U32 flags, U32 mask);
void common_fillFlags(CPU* cpu);
void common_call(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip);
void common_jmp(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip);
void common_ret(CPU* cpu, U32 big, U32 bytes);
void common_iret(CPU* cpu, U32 big, U32 oldEip);
void common_enter(CPU* cpu, U32 big, U32 bytes, U32 level);
void common_rdtsc(CPU* cpu, U32 extra);
void common_log(CPU* cpu, DecodedOp* op);
DecodedBlock* common_getNextBlock(CPU* cpu);
U32 common_readCrx(CPU* cpu, U32 which, U32 reg);
U32 common_writeCrx(CPU* cpu, U32 which, U32 value);
#endif
