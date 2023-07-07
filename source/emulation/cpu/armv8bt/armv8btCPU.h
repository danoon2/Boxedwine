#ifndef __ARMV8BT_CPU_H__
#define __ARMV8BT_CPU_H__

#ifdef BOXEDWINE_ARMV8BT
#include "../binaryTranslation/btCpu.h"

class Armv8btAsm;

class Armv8btCPU : public BtCPU {
public:
    Armv8btCPU();
    
    virtual void restart();
    virtual void* init();
    virtual void* translateEipInternal(U32 ip);   	

    U32 destEip;    
    U8* parity_lookup;                
	void*** eipToHostInstructionPages;    

    SSE sseConstants[6];

    ALIGN(U8 fpuState[512], 16);
	ALIGN(U8 originalFpuState[512], 16);
	U64 originalCpuRegs[16];	
    void* reTranslateChunkAddress;
    void* reTranslateChunkAddressFromReg;
    // for use with exceptions caused by jumping to an eip that isn't translated yet
    U64 regPage;
    U64 regOffset;
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
    void* jmpAndTranslateIfNecessary;
#endif
#ifdef _DEBUG
    U32 fromEip;
    U64 exceptionRegs[32];
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif

    void translateInstruction(Armv8btAsm* data, Armv8btAsm* firstPass);
    void link(Armv8btAsm* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk=0);
    S32 preLinkCheck(Armv8btAsm* data); // returns the index of the jump that failed    
    void translateData(Armv8btAsm* data, Armv8btAsm* firstPass=NULL);
    std::shared_ptr<BtCodeChunk> translateChunk(Armv8btAsm* parent, U32 ip);

    U64 handleCodePatch(U64 rip, U32 address);
    U64 handleMissingCode(U64 r8, U64 r9, U32 inst);
    U64 handleAccessException(U64 ip, U64 address, bool readAddress); // returns new ip, if 0 then don't set ip, but continue execution

    virtual std::shared_ptr<BtCodeChunk> translateChunk(U32 ip);

    virtual void setSeg(U32 index, U32 address, U32 value);

#ifdef __TEST
    virtual void postTestRun() {};
#endif
private:      
    void* translateEipInternal(Armv8btAsm* parent, U32 ip);            
    void writeJumpAmount(Armv8btAsm* data, U32 pos, U32 toLocation, U8* offset);
    U64 getIpFromEip();
    bool isStringOp(DecodedOp* op);
};
#endif
#endif
