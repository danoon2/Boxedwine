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

    virtual void run();
    virtual DecodedBlock* getNextBlock();
    virtual void restart();
    void* init();
    virtual void* translateEip(U32 ip);

	jmp_buf* jmpBuf;

    U32 negSegAddress[6];

    U64 memOffset;
    U64 negMemOffset;
    U64 exceptionRip;
    U64 exceptionRSP;
    U64 exceptionRSI;
    U64 exceptionRDI;
    U64 exceptionR8;
    U64 exceptionR9;
    U64 exceptionR10;
	int exitToStartThreadLoop; // this will be checked after a syscall, if set to 1 then then x64CPU.returnToLoopAddress will be called
	void*** eipToHostInstructionPages;
    DecodedOp* getOp(U32 eip, bool existing);
    U32 stringRepeat;
    U32 stringWritesToDi;
    U32 arg5;
    ALIGN(U8 fpuState[512], 16);
	ALIGN(U8 originalFpuState[512], 16);
	U64 originalCpuRegs[16];
	void* returnToLoopAddress;
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromR9;
    void* jmpAndTranslateIfNecessaryToR9;
    static bool hasBMI2;

#ifdef _DEBUG
    U32 fromEip;
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    void translateInstruction(X64Asm* data, X64Asm* firstPass);    
    void link(X64Asm* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk=0);
    S32 preLinkCheck(X64Asm* data); // returns the index of the jump that failed
    virtual void makePendingCodePagesReadOnly();
    virtual std::shared_ptr<BtCodeChunk> translateChunk(U32 ip);
    void translateData(X64Asm* data, X64Asm* firstPass=NULL);
    std::shared_ptr<BtCodeChunk> translateChunk(X64Asm* parent, U32 ip);

    U64 reTranslateChunk();
    U64 handleChangedUnpatchedCode(U64 rip);
    U64 handleCodePatch(U64 rip, U32 address, U64 rsi, U64 rdi, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    U64 handleMissingCode(U64 r8, U64 r9, U32 inst);

    virtual U64 handleIllegalInstruction(U64 rip);
    virtual U64 handleAccessException(U64 rip, U64 address, bool readAddress, std::function<U64(U32 reg)>getReg, std::function<void(U32 reg, U64 value)>setReg, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo); // returns new rip, if 0 then don't set rip, but continue execution
    virtual void startThread();
    void wakeThreadIfWaiting();
    virtual U64 startException(U64 address, bool readAddress, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    virtual U64 handleFpuException(int code, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);

    virtual void setSeg(U32 index, U32 address, U32 value);
#ifdef __TEST
    virtual void postTestRun();
#endif
private:      
    void* translateEipInternal(X64Asm* parent, U32 ip);            
    void markCodePageReadOnly(X64Asm* data);

    std::vector<U32> pendingCodePages;
};
#endif
#endif
