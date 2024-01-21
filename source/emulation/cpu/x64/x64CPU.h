#ifndef __X64_CPU_H__
#define __X64_CPU_H__

#ifdef BOXEDWINE_X64
#include "../common/cpu.h"
#include "x64CodeChunk.h"
#include "../binaryTranslation/btCpu.h"

class X64Asm;

union X87OrMMXRegister {
    using X87Register = uint8_t[10];
    union {
        struct {
            X87Register st;
            uint8_t st_reserved[6];
        };
        struct {
            uint8_t mm_value[8];
            uint8_t mm_reserved[8];
        };
        struct {
            uint64_t low;
            uint64_t high;
        };
    };
};

union XMMRegister {
    uint8_t bytes[16];
    struct {
        uint64_t low;
        uint64_t high;
    };
};


struct FxsaveStruct {
    uint16_t fcw;  // FPU control word
    uint16_t fsw;  // FPU status word
    uint8_t ftw;  // abridged FPU tag word
    uint8_t reserved_1;
    uint16_t fop;  // FPU opcode
    uint32_t fpu_ip;  // FPU instruction pointer offset
    uint16_t fpu_cs;  // FPU instruction pointer segment selector
    uint16_t reserved_2;
    uint32_t fpu_dp;  // FPU data pointer offset
    uint16_t fpu_ds;  // FPU data pointer segment selector
    uint16_t reserved_3;
    uint32_t mxcsr;  // multimedia extensions status and control register
    uint32_t mxcsr_mask;  // valid bits in mxcsr
    X87OrMMXRegister st_mm[8];
    XMMRegister xmm[8];
    uint8_t reserved_4[176];
    uint8_t available[48];
};

class x64CPU : public BtCPU {
public:
    x64CPU(KMemory* memory);
    
    virtual void restart();
    virtual void* init();

    U32 negSegAddress[6];

#ifdef BOXEDWINE_64BIT_MMU
    U64 negMemOffset;
    virtual bool handleStringOp(DecodedOp* op);
#endif
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
    ALIGN(FxsaveStruct fpuState, 16);
	ALIGN(FxsaveStruct originalFpuState, 16);
    ALIGN(U8 fpuBuffer[512], 16);
	U64 originalCpuRegs[16];
    void* reTranslateChunkAddress;    
#ifdef BOXEDWINE_64BIT_MMU
    void* reTranslateChunkAddressFromReg;
#endif
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

    virtual void setSeg(U32 index, U32 address, U32 value);
#ifdef __TEST
    virtual void postTestRun();
#endif    
protected:
    virtual std::shared_ptr<BtData> createData();
};
#endif
#endif
