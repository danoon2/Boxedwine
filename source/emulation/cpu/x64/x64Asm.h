#ifndef __X64ASM_H__
#define __X64ASM_H__

#ifdef BOXEDWINE_X64

#include "x64Data.h"

#define CPU_OFFSET_ES_ADDRESS (U32)(offsetof(CPU, seg[ES].address))
#define CPU_OFFSET_CS_ADDRESS (U32)(offsetof(CPU, seg[CS].address))
#define CPU_OFFSET_SS_ADDRESS (U32)(offsetof(CPU, seg[SS].address))
#define CPU_OFFSET_DS_ADDRESS (U32)(offsetof(CPU, seg[DS].address))
#define CPU_OFFSET_FS_ADDRESS (U32)(offsetof(CPU, seg[FS].address))
#define CPU_OFFSET_GS_ADDRESS (U32)(offsetof(CPU, seg[GS].address))

#define CPU_OFFSET_ES_NEG_ADDRESS (U32)(offsetof(x64CPU, negSegAddress[ES]))
#define CPU_OFFSET_CS_NEG_ADDRESS (U32)(offsetof(x64CPU, negSegAddress[CS]))
#define CPU_OFFSET_SS_NEG_ADDRESS (U32)(offsetof(x64CPU, negSegAddress[SS]))
#define CPU_OFFSET_DS_NEG_ADDRESS (U32)(offsetof(x64CPU, negSegAddress[DS]))
#define CPU_OFFSET_FS_NEG_ADDRESS (U32)(offsetof(x64CPU, negSegAddress[FS]))
#define CPU_OFFSET_GS_NEG_ADDRESS (U32)(offsetof(x64CPU, negSegAddress[GS]))

#define CPU_OFFSET_ES (U32)(offsetof(CPU, seg[ES].value))
#define CPU_OFFSET_CS (U32)(offsetof(CPU, seg[CS].value))
#define CPU_OFFSET_SS (U32)(offsetof(CPU, seg[SS].value))
#define CPU_OFFSET_DS (U32)(offsetof(CPU, seg[DS].value))
#define CPU_OFFSET_FS (U32)(offsetof(CPU, seg[FS].value))
#define CPU_OFFSET_GS (U32)(offsetof(CPU, seg[GS].value))

#define CPU_OFFSET_OP_PAGES (U32)(offsetof(x64CPU, eipToHostInstructionPages))
#define CPU_OFFSET_EIP (U32)(offsetof(x64CPU, eip.u32))
#define CPU_OFFSET_EIP_FROM (U32)(offsetof(x64CPU, fromEip))
#define CPU_OFFSET_EXIT_TO_START_LOOP (U32)(offsetof(x64CPU, exitToStartThreadLoop))
#define CPU_OFFSET_RETURN_ADDRESS (U32)(offsetof(x64CPU, returnToLoopAddress))

typedef void (*PFN_FPU_REG)(CPU* cpu, U32 reg);
typedef void (*PFN_FPU_ADDRESS)(CPU* cpu, U32 address);
typedef void (*PFN_FPU)(CPU* cpu);

class X64Asm : public X64Data {
public:  
    X64Asm(x64CPU* cpu);

    // from BtData
    void translateInstruction() override;
    void reset() override;
    void jumpTo(U32 eip) override;

    void translateRM(U8 rm, bool checkG, bool checkE, bool isG8bit, bool isE8bit, U8 immWidth, bool calculateHostAddress = true);
    void writeOp(bool isG8bit=false);
    void createCodeForDoSingleOp();
    void fpuRead(U8 rm, U32 len);
    void fpuWrite(U8 rm, U32 len);
	void saveNativeState();
	void restoreNativeState();
    void createCodeForRetranslateChunk(bool includeSetupFromR9=false);
    void createCodeForJmpAndTranslateIfNecessary(bool includeSetupFromR9 = false);
    void createCodeForSyncToHost();
    void createCodeForSyncFromHost();    
    void callRetranslateChunk();
#ifdef BOXEDWINE_POSIX
    void createCodeForRunSignal();
#endif

    void setImmediate8(U8 value);
    void setImmediate16(U16 value);
    void setImmediate32(U32 value);

    void xlat();
    void pushCpuOffset16(U32 offset);
    void pushCpuOffset32(U32 offset);
    void popSeg(U8 seg, U8 bytes);
    void daa();
    void das();
    void aaa();
    void aas();
    void aam(U8 ib);
    void aad(U8 ib);
    void salc();
    void incReg(U8 reg, bool isRegRex, U8 bytes);
    void decReg(U8 reg, bool isRegRex, U8 bytes);
    void pushReg16(U8 reg, bool isRegRex);
    void popReg16(U8 reg, bool isRegRex);
    void pushReg32(U8 reg, bool isRegRex);
    void pushw(U16 value);
    void popw(U8 rm);
    void pushd(U32 value);
    void popd(U8 rm);
    void popReg32(U8 reg, bool isRegRex);
    void pushA16();
    void popA16();
    void pushA32();
    void popA32();
    void pushfw();
    void pushfd();
    void popfw();
    void popfd();
    void pushE16(U8 rm);
    void pushE32(U8 rm);
    void jumpConditional(U8 condition, U32 eip);
    void jcxz(U32 eip, bool ea16);
    void loop(U32 eip, bool ea16);
    void loopz(U32 eip, bool ea16);
    void loopnz(U32 eip, bool ea16);    
    void jmp(bool big, U32 sel, U32 offset, U32 oldEip);
    void call(bool big, U32 sel, U32 offset, U32 oldEip);
    void retn16(U32 bytes);
    void retn32(U32 bytes);
    void retf(U32 big, U32 bytes);
    void iret(U32 big, U32 oldEip);
    void signalIllegalInstruction(int code);
    void signalTrap(int code);
    void syscall(U32 opLen);
    void int98(U32 opLen);
    void int99(U32 opLen);
    void int9A(U32 opLen);
    void writeToEFromCpuOffset(U8 rm, U32 offset, U8 fromBytes, U8 toBytes);    
    void writeToRegFromMemAddress(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes);
    void writeToMemAddressFromReg(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes);
    void writeToMemFromValue(U64 value, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost, bool canUseReg2AsTmp = false, bool skipAlignmentCheck = false);
    void setSeg(U8 seg, U8 rm);
    void loadSeg(U8 seg, U8 rm, bool b32);
    void writeXchgEspEax();
    void writeXchgSpAx();
    void bswapEsp();
    void bswapSp();
    void writeToRegFromValue(U8 reg, bool isRexReg, U64 value, U8 bytes);
    void enter(bool big, U32 bytes, U32 level);
    void leave(bool big);
    void callE(bool big, U8 rm);
    void callFar(bool big, U8 rm);
    void jmpE(bool big, U8 rm);
    void jmpFar(bool big, U8 rm);  
    void callCallback(void* pfn);
    void lsl(bool big, U8 rm);
    void lar(bool big, U8 rm);
    void verw(U8 rm);
    void verr(U8 rm);
    void invalidOp(U32 op);
    void errorMsg(const char* msg);
    void cpuid();
    void setNativeFlags(U32 flags, U32 mask);    
    void pushNativeFlags();
    void popNativeFlags();
    void logOp(U32 eip);
    std::vector<U8> autoReleaseTmpAfterWriteOp;
    bool tmp1InUse;
    bool tmp2InUse;
    bool tmp3InUse;
    bool tmp4InUse;
    bool param1InUse;
    bool param2InUse;
    bool param3InUse;
    bool param4InUse;
    void doJmp(bool mightNeedCS);
    void bound32(U8 rm);
    void bound16(U8 rm);
    void movRdCrx(U32 which, U32 reg);
    void movCrxRd(U32 which, U32 reg);

    void fpu0(U8 rm);
    void fpu1(U8 rm);
    void fpu2(U8 rm);
    void fpu3(U8 rm);
    void fpu4(U8 rm);
    void fpu5(U8 rm);
    void fpu6(U8 rm);
    void fpu7(U8 rm);

    void DsEdiMmxOrSSE(U8 rm);
    void emulateSingleOp(DecodedOp* op, bool isDynamic = false);
#ifdef __TEST
    void addReturnFromTest();
#endif
private:
    U8 getTmpReg();
    bool isTmpRegAvailable();
    void releaseTmpReg(U8 reg);
    bool isTmpReg(U8 tmpReg);
    void lockParamReg(U8 paramReg, bool paramRex);
    void unlockParamReg(U8 paramReg, bool paramRex);
    U8 getParamSafeTmpReg();

    void push(S32 reg, bool isRegRex, U32 value, S32 bytes);
    void pushNativeReg(U8 reg, bool isRegRex);
    void pushNativeValue32(U32 value);
    void pushNativeValue8(U8 value);
    void popNativeReg(U8 reg, bool isRegRex);
    void orRegReg(U8 dst, bool isDstRex, U8 src, bool isSrcRex);
    void andWriteToRegFromCPU(U8 reg, bool isRegRex, U32 offset);
    void writeToMemFromReg(U8 src, bool isSrcRex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost, bool skipAlignmentCheck = false, bool releaseReg3 = false);
    void writeToRegFromMem(U8 dst, bool isDstRex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost, bool skipAlignmentCheck = false, bool releaseReg3 = false);
    void writeToRegFromReg(U8 toReg, bool isToReg1Rex, U8 fromReg, bool isFromRegRex, U8 bytes);    
    void popReg(U8 reg, bool isRegRex, S8 bytes, bool commit);
    void syncRegsFromHost(bool eipInR9=false);
    void syncRegsFromHostCall();
    void syncRegsToHostCall();
    void syncRegsToHost(S8 excludeReg=-1);
    void minSyncRegsFromHost();
    void minSyncRegsToHost();
    void adjustStack(U8 tmpReg, S32 bytes);
    void doIf(U8 reg, bool isRexReg, U32 equalsValue, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock, bool keepFlags = false, bool generateCmp = true, bool releaseRegAfterCmp = false);
    void setPF_onAL(U8 flagReg);
    void setZF_onAL(U8 flagReg);
    void setSF_onAL(U8 flagReg);

    void callHost(void* pfn, U8 tmp = 0xFF, bool internal = false);
    void callJmp(bool big, U8 rm, bool jmp);

    void translateMemory(U32 rm, bool checkG, bool isG8bit, bool isE8bit, bool calculateHostAddress, U8 hostMem=0xff);
    void setRM(U8 rm, bool checkG, bool checkE, bool isG8bit, bool isE8bit);
    void setSib(U8 sib, bool checkBase);

    void addWithLea(U8 reg1, bool isReg1Rex, U8 reg2, bool isReg2Rex, S32 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U32 bytes);
    void zeroReg(U8 reg, bool isRexReg, bool keepFlags);
    void doMemoryInstruction(U8 op, U8 reg1, bool isReg1Rex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes);
    void writeHostPlusTmp(U8 rm, bool checkG, bool isG8bit, bool isE8bit, U8 tmpReg, bool calculateHostAddress, U8 hostMem);
  
    U8 getHostMem(U8 regEmulatedAddress, bool isRex);
    void releaseHostMem(U8 reg);

    U8 getRegForSeg(U8 base, U8 tmpReg);
    void translateMemory16(U32 rm, bool checkG, bool isG8bit, bool isE8bit, S8 r1, S8 r2, S16 disp, U8 seg, bool calculateHostAddress, U8 hostMem);

    void setDisplacement32(U32 disp32);
    void setDisplacement8(U8 disp8);  

    void addTodoLinkJump(U32 eip);       
    void doLoop(U32 eip);
    void doLoop16(U8 inst, U32 eip);
    void jmpReg(U8 reg, bool isRex, bool mightNeedCS);
    void jmpNativeReg(U8 reg, bool isRegRex);
    void shiftRightReg(U8 reg, bool isRegRex, U8 shiftAmount, bool arith = false);
    void shiftLeftReg(U8 reg, bool isRegRex, U8 shiftAmount);
    void bmi2ShiftRightReg(U8 dstReg, U8 srcReg, bool isSrcRex, U8 amountReg);
    void andReg(U8 reg, bool isRegRex, U32 mask);
    void orReg(U8 reg, bool isRegRex, U32 mask);
    void subRegs(U8 dst, bool isDstRex, U8 src, bool isSrcRex, bool is64);
    void writeToEFromReg(U8 rm, U8 reg, bool isRegRex, U8 bytes); // will trash current op data
    void writeToRegFromE(U8 reg, bool isRegRex, U8 rm, U8 bytes); // will trash current op data
    void getAddressInRegFromE(U8 reg, bool isRegRex, U8 rm, bool calculateHostAddress = false); // will trash current op data    
    void zeroExtend16to32(U8 reg, bool isRegRex, U8 fromReg, bool isFromRex);

    void pushFlagsToReg(U8 reg, bool isRexReg, bool includeOF);
    void popFlagsFromReg(U8 reg, bool isRexReg, bool includeOF);
    void xchange4(U8 reg1, bool isRexReg1, U8 reg2, bool isRexReg2);

    void callFpuNoArg(PFN_FPU pfn);
    void callFpuWithAddress(PFN_FPU_ADDRESS pfn, U8 rm);
    void callFpuWithAddressWrite(PFN_FPU_ADDRESS pfn, U8 rm, U32 len);
    void callFpuWithArg(PFN_FPU_REG pfn, U32 arg);

    void calculateMemory16(U8 reg, bool isRex, U32 rm, S8 r1, S8 r2, S16 disp, U8 seg);
    void calculateMemory(U8 reg, bool isRex, U32 rm);

    void shiftRightNoFlags(U8 src, bool isSrcRex, U8 dst, U32 value, U8 tmpReg);    

    void checkMemory(U8 reg, bool isRex, bool isWrite, U32 width, U8 memReg = 0xFF, bool writeHostMemToReg = false, bool skipAlignmentCheck = false, bool releaseReg = false);
public:
    void lods(U32 width) {
        string(width, true, false);
    }
    void movs(U32 width) {
        string(width, true, true);
    }
    void stos(U32 width) {
        string(width, false, true);
    }
    void cmps(U32 width) {
        cmps(width, true);
    }
    void scas(U32 width) {
        cmps(width, false);
    }
private:
    void string(U32 width, bool hasSrc, bool hasDst);
    void cmps(U32 width, bool hasSrc);
};
#endif
#endif