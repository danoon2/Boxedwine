#ifndef __X64_H__
#define __X64_H__

#include "platform.h"
#include <stddef.h>

#define HOST_TMP2         0
#define HOST_CPU          1
#define HOST_MEM          2
#define HOST_ESP          3
#define HOST_TMP3         4
#define HOST_TMP          5
#define HOST_SS           6
#define HOST_DS           7

#define RAX 0

#define FMASK_X64 (FMASK_TEST|DF)
#define CPU_OFFSET_ES (U32)(offsetof(struct CPU, segValue[ES]))
#define CPU_OFFSET_CS (U32)(offsetof(struct CPU, segValue[CS]))
#define CPU_OFFSET_SS (U32)(offsetof(struct CPU, segValue[SS]))
#define CPU_OFFSET_DS (U32)(offsetof(struct CPU, segValue[DS]))
#define CPU_OFFSET_FS (U32)(offsetof(struct CPU, segValue[FS]))
#define CPU_OFFSET_GS (U32)(offsetof(struct CPU, segValue[GS]))

#define CPU_OFFSET_EAX (U32)(offsetof(struct CPU, reg[0].u32))
#define CPU_OFFSET_ECX (U32)(offsetof(struct CPU, reg[1].u32))
#define CPU_OFFSET_EDX (U32)(offsetof(struct CPU, reg[2].u32))
#define CPU_OFFSET_EBX (U32)(offsetof(struct CPU, reg[3].u32))
#define CPU_OFFSET_ESP (U32)(offsetof(struct CPU, reg[4].u32))
#define CPU_OFFSET_EBP (U32)(offsetof(struct CPU, reg[5].u32))
#define CPU_OFFSET_ESI (U32)(offsetof(struct CPU, reg[6].u32))
#define CPU_OFFSET_EDI (U32)(offsetof(struct CPU, reg[7].u32))
#define CPU_OFFSET_FLAGS (U32)(offsetof(struct CPU, flags))

#define CPU_OFFSET_ES_ADDRESS (U32)(offsetof(struct CPU, segAddress[ES]))
#define CPU_OFFSET_CS_ADDRESS (U32)(offsetof(struct CPU, segAddress[CS]))
#define CPU_OFFSET_SS_ADDRESS (U32)(offsetof(struct CPU, segAddress[SS]))
#define CPU_OFFSET_DS_ADDRESS (U32)(offsetof(struct CPU, segAddress[DS]))
#define CPU_OFFSET_FS_ADDRESS (U32)(offsetof(struct CPU, segAddress[FS]))
#define CPU_OFFSET_GS_ADDRESS (U32)(offsetof(struct CPU, segAddress[GS]))

#define CPU_OFFSET_ES_NEG_ADDRESS (U32)(offsetof(struct CPU, negSegAddress[ES]))
#define CPU_OFFSET_CS_NEG_ADDRESS (U32)(offsetof(struct CPU, negSegAddress[CS]))
#define CPU_OFFSET_SS_NEG_ADDRESS (U32)(offsetof(struct CPU, negSegAddress[SS]))
#define CPU_OFFSET_DS_NEG_ADDRESS (U32)(offsetof(struct CPU, negSegAddress[DS]))
#define CPU_OFFSET_FS_NEG_ADDRESS (U32)(offsetof(struct CPU, negSegAddress[FS]))
#define CPU_OFFSET_GS_NEG_ADDRESS (U32)(offsetof(struct CPU, negSegAddress[GS]))

#define CPU_OFFSET_MEM (U32)(offsetof(struct CPU, memOffset))
#define CPU_OFFSET_NEG_MEM (U32)(offsetof(struct CPU, negMemOffset))

#define CPU_OFFSET_STACK_MASK  (U32)(offsetof(struct CPU, stackMask))
#define CPU_OFFSET_STACK_NOT_MASK (U32)(offsetof(struct CPU, stackNotMask))
#define CPU_OFFSET_HOST_ENTRY (U32)(offsetof(struct CPU, enterHost))
#define CPU_OFFSET_CMD (U32)(offsetof(struct CPU, cmd))
#define CPU_OFFSET_CMD_ARG (U32)(offsetof(struct CPU, cmdArg))
#define CPU_OFFSET_CMD_ARG2 (U32)(offsetof(struct CPU, cmdArg2))
#define CPU_OFFSET_CMD_EIPCOUNT (U32)(offsetof(struct CPU, cmdEipCount))
#define CPU_OFFSET_EIP (U32)(offsetof(struct CPU, eip.u32))
#define CPU_OFFSET_RSP (U32)(offsetof(struct CPU, rsp))

#define CPU_OFFSET_LOG (U32)(offsetof(struct CPU, log))
#define CPU_OFFSET_DONE (U32)(offsetof(struct CPU, done))

#define CMD_SET_ES 0
#define CMD_SET_CS 1
#define CMD_SET_SS 2
#define CMD_SET_DS 3
#define CMD_SET_FS 4
#define CMD_SET_GS 5
#define CMD_CALL_16 6
#define CMD_LOAD_ES 7
#define CMD_LOAD_CS 8
#define CMD_LOAD_SS 9
#define CMD_LOAD_DS 10
#define CMD_LOAD_FS 11
#define CMD_LOAD_GS 12
#define CMD_SYSCALL 13
#define CMD_PRINT 14
#define CMD_CPUID 15
#define CMD_INVALID_OP 16
#define CMD_WINE 17
#define CMD_OPENGL 18
#define CMD_SELF_MODIFYING 19
#define CMD_RETF_16 20
#define CMD_RETFIw_16 21
#define CMD_ENTER32 22
#define CMD_ENTER16 23
#define CMD_INT 24
#define CMD_CALL_FAR 25
#define CMD_RETFIw_32 26
#define CMD_RETF_32 27
#define CMD_JUMP_16 28
#define CMD_MOVSB 29
#define CMD_MOVSW 30
#define CMD_MOVSD 31
#define CMD_CMPSB 32
#define CMD_CMPSW 33
#define CMD_CMPSD 34
#define CMD_STOSB 35
#define CMD_STOSW 36
#define CMD_STOSD 37
#define CMD_SCASB 38
#define CMD_SCASW 39
#define CMD_SCASD 40
#define CMD_LODSB 41
#define CMD_LODSW 42
#define CMD_LODSD 43
#define CMD_EXIT_SIGNAL 44
#define CMD_IRET32 45
#define CMD_POP16_ES 46
#define CMD_POP16_CS 47
#define CMD_POP16_SS 48
#define CMD_POP16_DS 49
#define CMD_POP16_FS 50
#define CMD_POP16_GS 51
#define CMD_POP32_ES 52
#define CMD_POP32_CS 53
#define CMD_POP32_SS 54
#define CMD_POP32_DS 55
#define CMD_POP32_FS 56
#define CMD_POP32_GS 57
#define CMD_IRET16 58
#define CMD_LAR 59
#define CMD_LSL 60

struct x64_Data {
    U32 baseOp;
    U32 inst;
    U32 ea16;
    U32 ds;
    U32 ss;
    U32 ip;
    U32 start;
    U32 done;
    U32 startOfOpIp;

    // Prefixes + Mandatory Prefix + REX Prefix + Opcode Bytes + ModR/M + SIB + Displacement (1,2 or 4 bytes) + Immediate (1,2 or 4 bytes)

    U8 multiBytePrefix;
    U8 operandPrefix;
    U8 addressPrefix;
    U8 lockPrefix;
    U8 repNotZeroPrefix;
    U8 repZeroPrefix;

    U32 opIp;
    U8 rex;
    U8 op;

    U8 rm;
    U8 has_rm;

    U8 sib;
    U8 has_sib;

    U8 disp8;
    U8 has_disp8;

    U16 disp16;
    U8 has_disp16;

    U32 disp32;
    U8 has_disp32;

    U8 im8;
    U8 has_im8;

    U16 im16;
    U8 has_im16;

    U32 im32;
    U8 has_im32;

    struct CPU* cpu;

    U8* memStart;
    U32 memPos;
    U32 availableMem;

    U32* jmpTodoEip;
    void** jmpTodoAddress;
    U8*  jmpTodoOffsetSize;
    U32 jmpTodoCount;
    U32 jmpTodoEipBuffer[128];
    void* jmpTodoAddressBuffer[128];
    U8 jmpTodoOffsetSizeBuffer[128];
    U32 jmpTodoSize;

    U32* ipAddress;
    void** hostAddress;
    U32 ipAddressCount;
    U32 ipAddressBufferSize;
    U32 ipAddressBuffer[64];
    void* hostAddressBuffer[64];
    struct x64_Data* parent;

    BOOL tmp1InUse;
    BOOL tmp2InUse;
    BOOL tmp3InUse;
};

typedef U32 (*DECODER)(struct x64_Data* data);

extern DECODER x64Decoder[1024];

void x64_mapAddress(struct x64_Data* data, U32 ip, void* address);
void x64_commitMappedAddresses(struct x64_Data* data);
void x64_addSignalExit(struct CPU* cpu);
U64 x64_generateInt(struct CPU* cpu);
void x64_writeInt(struct x64_Data* data);

void x64_writeToRegFromMem(struct x64_Data* data, U32 toReg, U32 isToRegRex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes, U32 translateToHost);
void x64_writeToMemFromValue(struct x64_Data* data, U64 value, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes, U32 translateToHost);
void x64_writeToMemFromReg(struct x64_Data* data, U32 reg1, U32 isReg1Rex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes, U32 translateToHost);
void x64_writeToRegFromValue(struct x64_Data* data, U32 reg, U32 isRexReg, U64 value, U32 bytes);
void x64_writeToRegFromReg(struct x64_Data* data, U32 toReg, U32 isToReg1Rex, U32 fromReg, U32 isFromRegRex, U32 bytes);
void x64_addWithLea(struct x64_Data* data, U32 reg1, U32 isReg1Rex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes);
void x64_pushw(struct x64_Data* data, U16 value);
void x64_pushd(struct x64_Data* data, U32 value);
U32 x64_getRegForSeg(struct x64_Data* data, U32 base, U32 tmpReg);
U32 x64_getRegForNegSeg(struct x64_Data* data, U32 base, U32 tmpReg);
void x64_translateRM(struct x64_Data* data, U32 rm, U32 checkG, U32 checkE, U32 isG8bit, U32 isE8bit, U32 tmpReg);
void x64_leaToReg(struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes);
void x64_writeToRegFromE(struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes);
void x64_writeToEFromReg(struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes);
void x64_cmpRegToValue(struct x64_Data* data, U32 reg, U32 isRegRex, S32 value, U32 bytes);
void x64_jcxz(struct x64_Data* data, S8 offset, BOOL ea16);
void x64_loop(struct x64_Data* data, S8 offset, BOOL ea16);
void x64_loopz(struct x64_Data* data, S8 offset, BOOL ea16);
void x64_loopnz(struct x64_Data* data, S8 offset, BOOL ea16);
void x64_bswapEsp(struct x64_Data* data);
void x64_retn(struct x64_Data* data);
void x64_salc(struct x64_Data* data);
void x64_xlat(struct x64_Data* data);
void x64_jmpJd(struct x64_Data* data, U32 offset);
void x64_andWriteToRegFromCPU(struct x64_Data* data, U32 reg, U32 isRegRex, U32 offset);
void x64_orRegReg(struct x64_Data* data, U32 dst, U32 isDstRex, U32 src, U32 isSrcRex);

void x64_writeOp(struct x64_Data* data);
void x64_writeOp8(struct x64_Data* data, BOOL isG8bit, BOOL isGWritten);
void x64_setFlags(struct x64_Data* data, U32 flags, U32 mask);

U32 x64_getTmpReg(struct x64_Data* data);
void x64_releaseTmpReg(struct x64_Data* data, U32 tmpReg);
BOOL x64_isTmpReg(struct x64_Data* data, U32 tmpReg);

void x64_setRM(struct x64_Data* data, U8 rm, BOOL checkG, BOOL checkE, U32 isG8bit, U32 isE8bit);
void x64_setSib(struct x64_Data* data, U8 sib, BOOL checkBase);
void x64_setDisplacement32(struct x64_Data* data, U32 disp32);
void x64_setDisplacement8(struct x64_Data* data, U8 disp8);
void x64_setImmediate8(struct x64_Data* data, U8 value);
void x64_setImmediate16(struct x64_Data* data, U16 value);
void x64_setImmediate32(struct x64_Data* data, U32 value);

void x64_pushCpuOffset32(struct x64_Data* data, U32 offset);
void x64_pushCpuOffset16(struct x64_Data* data, U32 offset);
void x64_pushReg16(struct x64_Data* data, U32 reg, U32 isRegRex);
void x64_popReg16(struct x64_Data* data, U32 reg, U32 isRegRex);
void x64_pushReg32(struct x64_Data* data, U32 reg, U32 isRegRex);
void x64_popReg32(struct x64_Data* data, U32 reg, U32 isRegRex);
void x64_incReg(struct x64_Data* data, U32 reg, U32 isRegRex, U32 bytes);
void x64_decReg(struct x64_Data* data, U32 reg, U32 isRegRex, U32 bytes);
void x64_daa(struct x64_Data* data);

void x64_writeCmd(struct x64_Data* data, U32 cmd, U32 eip, U32 eipCount);
void x64_jumpConditional(struct x64_Data* data, U32 condition, U32 eip);
void x64_jumpTo(struct x64_Data* data,  U32 eip);
void x64_callTo(struct x64_Data* data,  U32 eip);
void x64_jmpReg(struct x64_Data* data, U32 reg, U32 isRex);

void x64_pushNative(struct x64_Data* data, U32 reg, U32 isRegRex);
void x64_popNative(struct x64_Data* data, U32 reg, U32 isRegRex);
void x64_popNativeFlags(struct x64_Data* data);
void x64_pushNativeFlags(struct x64_Data* data);

void x64_writeXchgSpAx(struct x64_Data* data);
void x64_writeXchgEspEax(struct x64_Data* data);

void x64_write32Buffer(U8* buffer, U32 value);
void x64_write16Buffer(U8* buffer, U16 value);
void x64_write8Buffer(U8* buffer, U8 value);
U32 x64_read32Buffer(U8* buffer);

U8 x64_fetch8(struct x64_Data* data);
U16 x64_fetch16(struct x64_Data* data);
U32 x64_fetch32(struct x64_Data* data);

void x64_translateInstruction(struct x64_Data* data);
void x64_initData(struct x64_Data* data, struct CPU* cpu, U32 eip);
void x64_link(struct x64_Data* data);

#endif