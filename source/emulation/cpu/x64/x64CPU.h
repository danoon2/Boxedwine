#ifndef __X64_CPU_H__
#define __X64_CPU_H__

#ifdef BOXEDWINE_X64
#include "../common/cpu.h"
#include "x64CodeChunk.h"
#include "../binaryTranslation/btCpu.h"

class X64Asm;

class x64CPU : public BtCPU {
public:
    x64CPU();
    
    virtual void restart();
    virtual void* init();

    U32 negSegAddress[6];

    U64 negMemOffset;
    U64 exceptionRSP;
    U64 exceptionRSI;
    U64 exceptionRDI;
    U64 exceptionR8;
    U64 exceptionR9;
    U64 exceptionR10;	
	void*** eipToHostInstructionPages;    
    U32 stringRepeat;
    U32 stringWritesToDi;
    U32 arg5;
    ALIGN(U8 fpuState[512], 16);
	ALIGN(U8 originalFpuState[512], 16);
	U64 originalCpuRegs[16];
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromReg;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    void* jmpAndTranslateIfNecessary;
#endif
    static bool hasBMI2;

#ifdef _DEBUG
    U32 fromEip;
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    void translateInstruction(X64Asm* data, X64Asm* firstPass);    
    void link(X64Asm* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk=0);
    virtual std::shared_ptr<BtCodeChunk> translateChunk(U32 ip);
    void translateData(X64Asm* data, X64Asm* firstPass=NULL);    
    
    U64 handleCodePatch(U64 rip, U32 address, U64 rsi, U64 rdi, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    U64 handleMissingCode(U64 r8, U64 r9, U32 inst);
    U64 handleAccessException(U64 ip, U64 address, bool readAddress, std::function<U64(U32 reg)>getReg, std::function<void(U32 reg, U64 value)>setReg, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo); // returns new ip, if 0 then don't set ip, but continue execution
    bool fixStringOp(DecodedOp* op, U64 rsi, U64 rdi);

    virtual void setSeg(U32 index, U32 address, U32 value);
#ifdef __TEST
    virtual void postTestRun();
#endif           
};
#endif
#endif
