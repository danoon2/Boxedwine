/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
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
void callX11(CPU* cpu, U32 index);
extern U32 lastGlCallTime;
extern U32 int99CallbackSize;

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

class Seg {
public:
    U32 address;
    U32 value;
};

#define LDT_ENTRIES 8192
#define BOXEDWINE_INTERNAL_USER_CODE_SELECTOR 0x0F
#define BOXEDWINE_INTERNAL_USER_DATA_SELECTOR 0x17
#define BOXEDWINE_VISIBLE_USER_CODE_SELECTOR 0x0B
#define BOXEDWINE_VISIBLE_USER_DATA_SELECTOR 0x13

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

#ifndef JIT_RUN_COUNT
#if defined(BOXEDWINE_WASM_JIT) && defined(__EMSCRIPTEN__)
#define JIT_RUN_COUNT 200
#else
#define JIT_RUN_COUNT 50
#endif
#endif

#if JIT_RUN_COUNT > 254
#error "JIT_RUN_COUNT must fit in DecodedOp::runCount (U8) and leave room for the JIT_RUN_COUNT + 1 sentinel"
#endif

class CPU: public DecodeBlockCallback {
public:
    static CPU* allocCPU(KMemory* memory);

    CPU(KMemory* memory);
    virtual ~CPU();
    
    Reg reg[9];
    Seg seg[7];    
    U32 flags = 0;
    Reg  src;
    Reg  dst;
    Reg  result;
    Reg eip;
#ifdef BOXEDWINE_JIT
    U32 tmpReg;
#endif
#ifdef BOXEDWINE_WASM_JIT
    // Scratch fields used only by the WASM JIT to pass
    // address/value to its per-width memory helpers without trampling
    // lazy-flag state in src.u32/dst.u32.
    U32 memHelperAddr = 0;
    U32 memHelperValue = 0;
    // Self-modifying-code support: each JIT block call clears these fields.
    // Before a checked memory write, generated code records the active block's
    // first DecodedOp. If removeCodeBlock clears that block while it is active
    // (or a post-write helper observes that its pfnJitCode was cleared), it
    // sets wasmJitBailout=1 so generated bailout checks can exit before stale
    // compiled bytes keep running.
    DecodedOp* wasmJitActiveBlock = nullptr;
    U32 wasmJitBailout = 0;
    // Inline TLB fast-path: cached pointers to the per-page host-base
    // arrays in KMemoryData. Set up by wasmHelper_blockEnter so the JIT
    // codegen can do `wasmReadPageBaseArray[page] -> entry; if (entry)
    // direct-load`, skipping the full helper round-trip on cache hits.
    // Encoded as `(U32)(uintptr_t)wasmReadPageBase`/`wasmWritePageBase`
    // (32-bit linear-memory offsets under emcc).
    U32 wasmJitMemoryData = 0;
    U32 wasmReadPageBaseArray  = 0;
    U32 wasmWritePageBaseArray = 0;
#ifdef BOXEDWINE_WASM_JIT_PROFILE
    U32 wasmJitProfileSampleCounter = 0;
#endif
#endif
    U8* reg8[9];
    ALIGN(SSE xmm[8], 16);    

    U32 sseControlStateTmp = 0;
    U32 mxcsr = 0x1F80; // sse control register
    
    LazyFlagType lazyFlagType = FLAGS_NONE;
    LazyFlagType lazyFlagTypePrev = FLAGS_NONE;
    U32         oldCF = 0;
#if defined(BOXEDWINE_JIT_ARMV8)
    U64 storedRegs[12];
#endif
    FPU         fpu;
    U64		    instructionCount = 0;
    U32         blockInstructionCount = 0;
    bool        yield = false;
    U32         cpl = 0;
    U32 cr0 = 0;
    U32 stackNotMask = 0;
    U32 stackMask = 0;
    U32 fpuDirtyFlags = 0;
    bool debugTrapOnNextInstruction = false;
    bool pendingDebugTrap = false;
    bool debugTrapActive = false;
    U32 pendingDebugTrapCode = 0;
    U32 pendingDebugTrapDr6 = 0;
    DecodedOp*** opCache = nullptr;

    U64 fAbs = 0x7fffffffffffffffl;
    U64 fNeg = 0x8000000000000000l;
    U64 fZero = 0l;
    U64 fOne = 0x3FF0000000000000l;
    U64 fL2T = 0x400A934F0979A371;
    U64 fL2E = 0x3FF71547652B82FE;
    U64 fPi = 0x400921FB54442D18;
    U64 fLG2 = 0x3FD34413509F79FF;
    U64 fLN2 = 0x3FE62E42FEFA39EF;

    KThread* thread = nullptr;
    KMemory* memory = nullptr;
    BWriteFile logFile;

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
    void lazyFlagsCFOF(bool fillCF = false);
    void fillFlagsNoCF();
    void fillFlagsNoZF();
    void fillFlags();
    void setFlags(U32 flags, U32 mask);
    void updateDebugTrapActive();
    bool startDebugInstruction();
    bool finishDebugInstruction();

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
    bool isNullSegment(U32 seg) const;
    bool checkSegmentAccess(U32 seg);
    void prepareException(int code, int error);
    void prepareFpuException(int code, int trapNo=16, int error=0);
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
    virtual void reset();
    void verr(U32 selector);
    void verw(U32 selector);
    U32 readCrx(U32 which, U32 reg);
    U32 writeCrx(U32 which, U32 value);

    U32 getEipAddress();

    DecodedOp* nextOp = nullptr;

    virtual void run()=0;
    virtual void restart() {}
    virtual void setSeg(U32 index, U32 address, U32 value);
    U32 getSegValue(U32 index) const;
    static U32 makeSegmentVisible(U32 value);
    static U32 makeSegmentInternal(U32 value);

    bool isBig() {return this->big!=0;}
    virtual void setIsBig(U32 value);

    DecodedOp* getNextOp(U32 jumpTargetFlags = 0);

    // from DecodeBlockCallback
    U8 fetchByte(U32* eip) override;
    bool shouldContinue(U32 eip) override;
    DecodedOp** getOpLocation(U32 eip) override;        
    void runNextSingleOp();

#ifdef BOXEDWINE_MULTI_THREADED
    U64 nativeHandle = 0;
    U32 tmpLockAddress = 0;
#endif

#ifdef BOXEDWINE_HOST_EXCEPTIONS
    bool inException = false;

    // for linux/mac
    U64 exceptionAddress = 0;
    bool exceptionReadAddress = false;
    void* returnHostAddress = 0; // after returning from the signalHandler, this will contain the host address we should jump to
    int exceptionSigNo = 0;
    int exceptionSigCode = 0;
    U64 exceptionIp = 0;

    void* handleAccessException(DecodedOp* op);
    void* startException(U32 address, bool readAddress);

#endif

#ifdef BOXEDWINE_JIT
    static U32 offsetofReg32(U32 index);
    static U32 offsetofReg16(U32 index);
    static U32 offsetofReg8(U32 index);
    static U32 offsetofSegAddress(U32 index);
    static U32 offsetofSegValue(U32 index);

    void* calculateCF[FLAGS_NULL] = {};
#ifdef BOXEDWINE_JIT_ARMV8
#define SSE_MAX_INT32_PLUS_ONE_AS_DOUBLE 0
#define SSE_MIN_INT32_MINUS_ONE_AS_DOUBLE 1
#define SSE_MAX_INT32_PLUS_ONE_AS_FLOAT 2
#define SSE_MIN_INT32_MINUS_ONE_AS_FLOAT 3
#define SSE_INT32_BIT_MASK 4
#define SSE_BYTE8_BIT_MASK 5    
    SSE storedNeonRegs[32];
    SSE sseConstants[6];
#endif
#ifdef BOXEDWINE_JIT_X64
    U64 storedRegs[16];    
    SSE storedXMMRegs[16];
#elif defined (BOXEDWINE_JIT_X86)
    SSE storedXMMRegs[8];
#endif
#endif
    U32 flagZeroMask[52] = {
    0, // FLAGS_NONE,
    0xFF, // FLAGS_ADD8,
    0xFFFF, // FLAGS_ADD16,
    0xFFFFFFFF, // FLAGS_ADD32,
    0xFF, // FLAGS_OR8,
    0xFFFF, // FLAGS_OR16,
    0xFFFFFFFF, // FLAGS_OR32,
    0xFF, // FLAGS_ADC8,
    0xFFFF, // FLAGS_ADC16,
    0xFFFFFFFF, // FLAGS_ADC32,
    0xFF, // FLAGS_SBB8,
    0xFFFF, // FLAGS_SBB16,
    0xFFFFFFFF, // FLAGS_SBB32,
    0xFF, // FLAGS_AND8,
    0xFFFF, // FLAGS_AND16,
    0xFFFFFFFF, // FLAGS_AND32,
    0xFF, // FLAGS_SUB8,
    0xFFFF, // FLAGS_SUB16,
    0xFFFFFFFF, // FLAGS_SUB32,
    0xFF, // FLAGS_XOR8,
    0xFFFF, // FLAGS_XOR16,
    0xFFFFFFFF, // FLAGS_XOR32,
    0xFF, // FLAGS_INC8,
    0xFFFF, // FLAGS_INC16,
    0xFFFFFFFF, // FLAGS_INC32,
    0xFF, // FLAGS_DEC8,
    0xFFFF, // FLAGS_DEC16,
    0xFFFFFFFF, // FLAGS_DEC32,
    0xFF, // FLAGS_SHL8,
    0xFFFF, // FLAGS_SHL16,
    0xFFFFFFFF, // FLAGS_SHL32,
    0xFF, // FLAGS_SHR8,
    0xFFFF, // FLAGS_SHR16,
    0xFFFFFFFF, // FLAGS_SHR32,
    0xFF, // FLAGS_SAR8,
    0xFFFF, // FLAGS_SAR16,
    0xFFFFFFFF, // FLAGS_SAR32,
    0xFF, // FLAGS_CMP8,
    0xFFFF, // FLAGS_CMP16,
    0xFFFFFFFF, // FLAGS_CMP32,
    0xFF, // FLAGS_TEST8,
    0xFFFF, // FLAGS_TEST16,
    0xFFFFFFFF, // FLAGS_TEST32,
    0xFFFF, // FLAGS_DSHL16,
    0xFFFFFFFF, // FLAGS_DSHL32,
    0xFFFF, // FLAGS_DSHR16,
    0xFFFFFFFF, // FLAGS_DSHR32,
    0xFF, // FLAGS_NEG8,
    0xFFFF, // FLAGS_NEG16,
    0xFFFFFFFF, // FLAGS_NEG32,
    0, // FLAGS_CFOF
    0 // FLAGS_NULL
    };

    U32 flagSignMask[52] = {
        0, // FLAGS_NONE,
        0x80, // FLAGS_ADD8,
        0x8000, // FLAGS_ADD16,
        0x80000000, // FLAGS_ADD32,
        0x80, // FLAGS_OR8,
        0x8000, // FLAGS_OR16,
        0x80000000, // FLAGS_OR32,
        0x80, // FLAGS_ADC8,
        0x8000, // FLAGS_ADC16,
        0x80000000, // FLAGS_ADC32,
        0x80, // FLAGS_SBB8,
        0x8000, // FLAGS_SBB16,
        0x80000000, // FLAGS_SBB32,
        0x80, // FLAGS_AND8,
        0x8000, // FLAGS_AND16,
        0x80000000, // FLAGS_AND32,
        0x80, // FLAGS_SUB8,
        0x8000, // FLAGS_SUB16,
        0x80000000, // FLAGS_SUB32,
        0x80, // FLAGS_XOR8,
        0x8000, // FLAGS_XOR16,
        0x80000000, // FLAGS_XOR32,
        0x80, // FLAGS_INC8,
        0x8000, // FLAGS_INC16,
        0x80000000, // FLAGS_INC32,
        0x80, // FLAGS_DEC8,
        0x8000, // FLAGS_DEC16,
        0x80000000, // FLAGS_DEC32,
        0x80, // FLAGS_SHL8,
        0x8000, // FLAGS_SHL16,
        0x80000000, // FLAGS_SHL32,
        0x80, // FLAGS_SHR8,
        0x8000, // FLAGS_SHR16,
        0x80000000, // FLAGS_SHR32,
        0x80, // FLAGS_SAR8,
        0x8000, // FLAGS_SAR16,
        0x80000000, // FLAGS_SAR32,
        0x80, // FLAGS_CMP8,
        0x8000, // FLAGS_CMP16,
        0x80000000, // FLAGS_CMP32,
        0x80, // FLAGS_TEST8,
        0x8000, // FLAGS_TEST16,
        0x80000000, // FLAGS_TEST32,
        0x8000, // FLAGS_DSHL16,
        0x80000000, // FLAGS_DSHL32,
        0x8000, // FLAGS_DSHR16,
        0x80000000, // FLAGS_DSHR32,
        0x80, // FLAGS_NEG8,
        0x8000, // FLAGS_NEG16,
        0x80000000, // FLAGS_NEG32,
        0, // FLAGS_CFOF
        0, // FLAGS_NULL
    };

    U8 flagParityLookup[256] = {
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
      1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
    };

#ifndef __TEST
protected:
#endif
    U32 big;

#if defined(BOXEDWINE_WASM_JIT) && defined(BOXEDWINE_MULTI_THREADED)
public:
    // Keep owner-hazard bookkeeping after every pre-existing CPU field. Raw
    // WASM JIT modules embed CPU member offsets, so inserting cold MT state
    // into the established layout changes their generated-code ABI.
    U32 wasmJitActiveTableIndex = 0;
    U32 wasmJitActiveTableIndexLocal = 0;
    U32 wasmJitCallsUntilQuiescence = 0;
    U32 wasmJitInCompiledCall = 0;
    U32 wasmJitReapRetiredOnExit = 0;
    U32 wasmJitHazardRegistered = 0;
#endif
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
DecodedOp* common_getNextOp(CPU* cpu);
U32 common_readCrx(CPU* cpu, U32 which, U32 reg);
U32 common_writeCrx(CPU* cpu, U32 which, U32 value);
#endif
