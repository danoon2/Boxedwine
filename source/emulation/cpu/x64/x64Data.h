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
#define HOST_TMP2         0
#define HOST_TMP          1
#define HOST_TMP3         2
#define HOST_ESP          3
#define HOST_MEM          4
#define HOST_CPU          5
#define HOST_SS           6
#define HOST_DS           7

class X64Data {
public:
    X64Data(x64CPU* cpu);
    virtual ~X64Data();
    
    void mapAddress(U32 ip, U32 bufferPos);

    U8 fetch8();
    U16 fetch16();
    U32 fetch32();    

    void write8(U8 data);
    void write16(U16 data);
    void write32(U32 data);
    void write64(U64 data);

    void resetForNewOp();
    void* commit();

    U32 ip;
    U32 startOfDataIp;
    U32 startOfOpIp;
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

    static class TodoJump {
    public:
        TodoJump() : eip(0), bufferPos(0), offsetSize(0) {}
        TodoJump(U32 eip, U32 bufferPos, U8 offsetSize) : eip(eip), bufferPos(bufferPos), offsetSize(offsetSize) {}
        U32 eip;
        U32 bufferPos;
        U8 offsetSize;
    };
    std::vector<TodoJump> todoJump;
  
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

    bool skipWriteOp;
    bool isG8bitWritten;
};
#endif
#endif