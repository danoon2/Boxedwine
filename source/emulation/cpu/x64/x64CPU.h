#ifndef __X64_CPU_H__
#define __X64_CPU_H__

#ifdef BOXEDWINE_X64
#include "../common/cpu.h"
#include "x64CodeChunk.h"
#include "../binaryTranslation/btCpu.h"
#include "x64Asm.h"

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
    
    // from CPU
    void restart() override;
    void setSeg(U32 index, U32 address, U32 value) override;

    // from BtCPU
    void* init() override;
    void link(BtData* data, std::shared_ptr<BtCodeChunk>& fromChunk, U32 offsetIntoChunk = 0) override;
    void translateData(BtData* data, BtData* firstPass = nullptr) override;
protected:
    BtData* getData1() override { data1.reset(); return &data1; }
    BtData* getData2() override { data2.reset(); return &data2; }
    X64Asm data1;
    X64Asm data2;

public:

    U32 negSegAddress[6] = { 0 };
	U8*** eipToHostInstructionPages = nullptr;
    U32 arg5 = 0;
    U32 stringFlags = 0;
    ALIGN(FxsaveStruct fpuState, 16) = { 0 };
    ALIGN(FxsaveStruct originalFpuState, 16) = { 0 };
    ALIGN(U8 fpuBuffer[512], 16) = { 0 };
    U64 originalCpuRegs[16] = { 0 };
    void* reTranslateChunkAddress = nullptr;    
    void* jmpAndTranslateIfNecessary = nullptr;
    static bool hasBMI2;

#ifdef _DEBUG
    U32 fromEip = 0;
#endif
#ifdef __TEST
    void addReturnFromTest();
#endif
        
#ifdef __TEST
    virtual void postTestRun();
#endif    
};
#endif
#endif
