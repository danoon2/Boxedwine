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

    virtual void link(const std::shared_ptr<BtData>& data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk=0);    
    virtual void translateData(const std::shared_ptr<BtData>& data, const std::shared_ptr<BtData>& firstPass = nullptr);
        
    virtual bool handleStringOp(DecodedOp* op);

    virtual void setSeg(U32 index, U32 address, U32 value);
#ifdef __TEST
    virtual void postTestRun();
#endif    
protected:
    virtual std::shared_ptr<BtData> createData();
};
#endif
#endif
