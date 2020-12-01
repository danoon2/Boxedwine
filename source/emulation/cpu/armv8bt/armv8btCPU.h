#ifndef __ARMV8BT_CPU_H__
#define __ARMV8BT_CPU_H__

#ifdef BOXEDWINE_ARMV8BT
#include "../binaryTranslation/btCpu.h"

class Armv8btAsm;

class Armv8btCPU : public BtCPU {
public:
    Armv8btCPU();

    virtual void run();
    virtual DecodedBlock* getNextBlock();
    virtual void restart();
    void* init();
    void* translateEip(U32 ip);
    
	jmp_buf* jmpBuf;

    U64 memOffset;
    U8* parity_lookup;            
    U64 exceptionRSP;
    U64 exceptionRSI;
    U64 exceptionRDI;
    U64 exceptionR8;
    U64 exceptionR9;
    U64 exceptionR10;
	int exitToStartThreadLoop; // this will be checked after a syscall, if set to 1 then then Armv8btCPU.returnToLoopAddress will be called    
	void*** eipToHostInstructionPages;
    DecodedOp* getOp(U32 eip, bool existing);
    U32 stringRepeat;
    U32 stringWritesToDi;
    U32 arg5;   
    // 0 is vMaxInt32PlusOneAsDouble: 2147483648.0, 2147483648.0
    // 1 is vMinInt32MinusOneAsDouble: -2147483649.0, -2147483649.0
    // 2 is vMaxInt32PlusOneAsFloat: 2147483648.0, 2147483648.0, 2147483648.0, 2147483648.0
    // 3 is vMinInt32MinusOneAsFloat: -2147483649.0, -2147483649.0, -2147483649.0, -2147483649.0
    // 4 is vInt32BitMask: 1, 2, 3, 4
    SSE sseConstants[5];

    ALIGN(U8 fpuState[512], 16);
	ALIGN(U8 originalFpuState[512], 16);
	U64 originalCpuRegs[16];
	void* returnToLoopAddress;
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromR9;
    void* jmpAndTranslateIfNecessaryToR9;

#ifdef _DEBUG
    U32 fromEip;
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    void translateInstruction(Armv8btAsm* data, Armv8btAsm* firstPass);
    void link(Armv8btAsm* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk=0);
    S32 preLinkCheck(Armv8btAsm* data); // returns the index of the jump that failed
    void makePendingCodePagesReadOnly();
    void translateData(Armv8btAsm* data, Armv8btAsm* firstPass=NULL);
    std::shared_ptr<BtCodeChunk> translateChunk(Armv8btAsm* parent, U32 ip);

    U64 reTranslateChunk();
    U64 handleChangedUnpatchedCode(U64 rip);
    U64 handleCodePatch(U64 rip, U32 address, U64 rsi, U64 rdi, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    U64 handleMissingCode(U64 r8, U64 r9, U32 inst);

    virtual U64 handleIllegalInstruction(U64 rip);
    virtual U64 handleAccessException(U64 ip, U64 address, bool readAddress, std::function<U64(U32 reg)>getReg, std::function<void(U32 reg, U64 value)>setReg, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo); // returns new ip, if 0 then don't set ip, but continue execution
    virtual void startThread();
    void wakeThreadIfWaiting();
    virtual U64 startException(U64 address, bool readAddress, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    virtual U64 handleFpuException(int code, std::function<void(DecodedOp*)> doSyncFrom, std::function<void(DecodedOp*)> doSyncTo);
    virtual std::shared_ptr<BtCodeChunk> translateChunk(U32 ip);

    virtual void setSeg(U32 index, U32 address, U32 value);

#ifdef __TEST
    virtual void postTestRun() {};
#endif
private:      
    void* translateEipInternal(Armv8btAsm* parent, U32 ip);            
    void markCodePageReadOnly(Armv8btAsm* data);
    void writeJumpAmount(Armv8btAsm* data, U32 pos, U32 toLocation);

    std::vector<U32> pendingCodePages;
};
#endif
#endif