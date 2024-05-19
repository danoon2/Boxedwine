#ifndef __X64OP_H__
#define __X64OP_H__

#ifdef BOXEDWINE_X64

#include "../binaryTranslation/btData.h"

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

#define HOST_MEM_READ 4
#define HOST_MEM_WRITE 6

// R13 is good for this one, because R13 can not be used in sib memory encoding, it will translate to 0 instead of the reg, and HOST_CPU will never be used to encode memory
#define HOST_CPU          5

#define HOST_TMP4         7

class x64CPU;

class X64Data : public BtData {
public:
    X64Data(x64CPU* cpu);
    
    // from BtData
    void resetForNewOp() override;
protected:
    std::shared_ptr<BtCodeChunk> createChunk(U32 instructionCount, U32* eipInstructionAddress, U32* hostInstructionIndex, U8* hostInstructionBuffer, U32 hostInstructionBufferLen, U32 eip, U32 eipLen, bool dynamic) override;

public:
    U8 fetch8();
    U16 fetch16();
    U32 fetch32();    
    U64 fetch64();    

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
    
    bool skipWriteOp;
    bool isG8bitWritten;
    bool flagsWrittenToStringFlags;
};
#endif
#endif
