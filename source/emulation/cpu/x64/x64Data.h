#ifndef __X64OP_H__
#define __X64OP_H__

#ifdef BOXEDWINE_X64

#include "x64CPU.h"

#define REX_BASE 0x40
#define REX_MOD_RM 0x1
#define REX_SIB_INDEX 0x2
#define REX_MOD_REG 0x4
#define REX_64 0x8

// R8 and R9 are caller saved for Windows
// R10 and R11 are caller saved for Linux
//
// calling convention RCX, RDX, R8, R9 for first 4 parameters on Windows, X64Asm::callHost is hard coded to use HOST_TMP3, this can not overlap with parameters
// calling convention RDI, RSI, RDX, RCX, R8, R9 for first 6 parameters on other platforms

// be careful of overlapping the use of these when setting up parameters for a call
#define HOST_TMP2         0
#define HOST_TMP          1

// tmp3 is used by callHost and should not overlap with params above
// getParamSafeTmpReg assumes this does not overlap with params
#define HOST_TMP3         2

// popseg assumes these won't overlap with params
#define HOST_ESP          3
#define HOST_MEM          4

// R13 is good for this one, because R13 can not be used in sib memory encoding, it will translate to 0 instead of the reg, and HOST_CPU will never be used to encode memory
#define HOST_CPU          5

// popseg assumes this won't overlap with params
#define HOST_LARGE_ADDRESS_SPACE_MAPPING           6
#define HOST_SMALL_ADDRESS_SPACE_SS                6

#define HOST_TMP4         7

class TodoJump {
public:
    TodoJump() : eip(0), bufferPos(0), offsetSize(0), sameChunk(true) {}
    TodoJump(U32 eip, U32 bufferPos, U8 offsetSize, bool sameChunk, U32 opIndex) : eip(eip), bufferPos(bufferPos), offsetSize(offsetSize), sameChunk(sameChunk), opIndex(opIndex) {}
    U32 eip;
    U32 bufferPos;
    U8 offsetSize;
    bool sameChunk;
    U32 opIndex;
};

class X64Data {
public:
    X64Data(x64CPU* cpu);
    virtual ~X64Data();
    
    void mapAddress(U32 ip, U32 bufferPos);

    U8 fetch8();
    U16 fetch16();
    U32 fetch32();    
    U64 fetch64();

    void write8(U8 data);
    void write16(U16 data);
    void write32(U32 data);
    void write64(U64 data);

    void resetForNewOp();
    std::shared_ptr<X64CodeChunk> commit(bool makeLive);

    U32 ip;
    U32 startOfDataIp;
    U32 startOfOpIp;
    U32 calculatedEipLen;
    bool done;
    U32 op;
    U32 inst; // full op, like 0x200 while op would be 0x00
    bool addressPrefix;
    bool operandPrefix;
    bool multiBytePrefix;
    bool ea16;
    bool lockPrefix;
    bool repNotZeroPrefix;
    bool repZeroPrefix;
    U32 baseOp;
    U8 ds;
    U8 ss;
    U8 rex;

    U8 rm;
    bool has_rm;
    //bool checkG;
    //bool checkE;
    //bool isG8bit;
    //bool isE8bit;
    //bool isE8bitWritten;
    //bool isG8bitWritten;

    U8 sib;
    bool has_sib;

    U8 dispSize;
    U32 disp;

    U8 immSize;
    U32 imm;

    x64CPU* cpu;

    std::vector<TodoJump> todoJump;
    S32 stopAfterInstruction;

    U8 calculateEipLen(U32 eip);

    U32* ipAddress;
    U32* ipAddressBufferPos;
    U32 ipAddressCount;
    U32 ipAddressBufferSize;
    U32 ipAddressBuffer[64];
    U32 ipAddressBufferPosBuffer[64];
    U8* buffer;
    U32 bufferSize;
    U32 bufferPos;
    U8 bufferInternal[256];
    bool dynamic;
    bool useSingleMemOffset;
    bool skipWriteOp;
    bool isG8bitWritten;
};
#endif
#endif
