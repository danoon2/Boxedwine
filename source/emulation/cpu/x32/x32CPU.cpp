#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC32

#include "x32CPU.h"
#include "../common/lazyFlags.h"
#include "../dynamic/dynamic.h"
#include "../dynamic/dynamic_memory.h"
#include "../../softmmu/kmemory_soft.h"

// cdecl calling convention states EAX, ECX, and EDX are caller saved

/********************************************************/
/* Following is required to be defined for dynamic code */
/********************************************************/

#define INCREMENT_EIP(x, y) incrementEip(x, y)

#define CPU_OFFSET_OF(x) offsetof(CPU, x)

// DynReg is a required type, but the values inside are local to this file
// Used only these 4 because it is possible to use 8-bit calls with them, like add al, cl
enum DynReg {
    DYN_EAX=0,
    DYN_ECX=1,
    DYN_EDX=2,
    DYN_EBX=3,  
    DYN_NOT_SET=0xff
};

enum DynCondition {
    DYN_EQUALS_ZERO,
    DYN_NOT_EQUALS_ZERO
};

enum DynConditionEvaluate {
    DYN_EQUALS,
    DYN_NOT_EQUALS,
    DYN_LESS_THAN_UNSIGNED,
    DYN_LESS_THAN_EQUAL_UNSIGNED,
    DYN_GREATER_THAN_EQUAL_UNSIGNED,
    DYN_LESS_THAN_SIGNED,
    DYN_LESS_THAN_EQUAL_SIGNED,
};

#define DYN_CALL_RESULT DYN_EAX
#define DYN_SRC DYN_ECX
#define DYN_DEST DYN_EDX
#define DYN_ADDRESS DYN_EBX
#define DYN_ANY DYN_DEST

#define DYN_PTR_SIZE U32

enum DynWidth {
    DYN_8bit=0,
    DYN_16bit,
    DYN_32bit,
};

enum DynCallParamType {
    DYN_PARAM_REG_8,    
    DYN_PARAM_REG_16,
    DYN_PARAM_REG_32,
    DYN_PARAM_CONST_8,
    DYN_PARAM_CONST_16,
    DYN_PARAM_CONST_32,
    DYN_PARAM_CONST_PTR,
    DYN_PARAM_ABSOLUTE_ADDRESS_8,
    DYN_PARAM_ABSOLUTE_ADDRESS_16,
    DYN_PARAM_ABSOLUTE_ADDRESS_32,
    DYN_PARAM_CPU_ADDRESS_8,
    DYN_PARAM_CPU_ADDRESS_16,
    DYN_PARAM_CPU_ADDRESS_32,
    DYN_PARAM_CPU,
};

enum DynConditional {
    O,
    NO,
    B,
    NB,
    Z,
    NZ,
    BE,
    NBE,
    S,
    NS,
    P,
    NP,
    L,
    NL,
    LE,
    NLE
};

#define Dyn_PtrSize DYN_32bit

// helper, can be done with multiple other calls
void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult);
void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg);
void calculateEaa(DecodedOp* op, DynReg reg);

void byteSwapReg32(DynReg reg);

// REG to REG
void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);
void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg);

// to Reg
void movToReg(DynReg reg, DynWidth width, U32 imm);

// to CPU
void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg);
void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm);
void movToCpuPtr(U32 dstOffset, DYN_PTR_SIZE imm) {movToCpu(dstOffset, DYN_32bit, imm);}

// from CPU
void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width);

// from Mem to DYN_READ_RESULT
void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg);

// to Mem
void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg);
void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg);

// arith
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg);
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg);
void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg);

void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg);
void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm);

void instReg(char inst, DynReg reg, DynWidth regWidth);
void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg);
void instCPU(char inst, U32 dstOffset, DynWidth regWidth);

// if conditions
void startIf(DynReg reg, DynCondition condition, bool doneWithReg);
void startElse();
void endIf();
void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg);
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition);
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg);

// call into emulator, like setFlags, getCF, etc
void callHostFunction(void* address, bool hasReturn=false, U32 argCount=0, U32 arg1=0, DynCallParamType arg1Type=DYN_PARAM_CONST_32, bool doneWithArg1=true, U32 arg2=0, DynCallParamType arg2Type=DYN_PARAM_CONST_32, bool doneWithArg2=true, U32 arg3=0, DynCallParamType arg3Type=DYN_PARAM_CONST_32, bool doneWithArg3=true, U32 arg4=0, DynCallParamType arg4Type=DYN_PARAM_CONST_32, bool doneWithArg4=true, U32 arg5=0, DynCallParamType arg5Type=DYN_PARAM_CONST_32, bool doneWithArg5=true);

// set up the cpu to the correct next block

// this is called for cases where we don't know ahead of time where the next block will be, so we need to look it up
void blockDone();
// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1();
void blockNext2();

/********************************************************/
/* End required for dynamic code                        */
/********************************************************/

// referenced in macro above
void incrementEip(DynamicData* data, DecodedOp* op);
void incrementEip(DynamicData* data, U32 len);

#include "../normal/instructions.h"
#include "../common/common_arith.h"
#include "../common/common_pushpop.h"
#include "../dynamic/dynamic_func.h"
#include "../dynamic/dynamic_arith.h"
#include "../dynamic/dynamic_mov.h"
#include "../dynamic/dynamic_incdec.h"
#include "../dynamic/dynamic_jump.h"
#include "../dynamic/dynamic_pushpop.h"
#include "../dynamic/dynamic_strings.h"
#include "../dynamic/dynamic_shift.h"
#include "../dynamic/dynamic_conditions.h"
#include "../dynamic/dynamic_setcc.h"
#include "../dynamic/dynamic_xchg.h"
#include "../dynamic/dynamic_bit.h"
#include "../dynamic/dynamic_other.h"
#include "../dynamic/dynamic_mmx.h"
#include "../dynamic/dynamic_sse.h"
#include "../dynamic/dynamic_sse2.h"
#include "../dynamic/dynamic_fpu.h"

static U8* outBuffer;
static U32 outBufferSize;
static U32 outBufferPos;

static std::vector<U32> patch;
static std::vector<U32> ifJump;

// per instruction, not per block.  
// will allow us to determine if ecx or edx needs to be saved before calling an external function
bool regUsed[4]; 

void ensureBufferSize(U32 grow) {
    if (!outBuffer) {
        outBuffer = new U8[256];
        outBufferSize = 256;
    }
    if (outBufferSize-outBufferPos<grow) {
        U8* t =  new U8[outBufferSize*2];
        memcpy(t, outBuffer, outBufferSize);
        delete[] outBuffer;
        outBuffer = t;
        outBufferSize = outBufferSize*2;
    }
}

void outb(U8 b) {
    ensureBufferSize(1);
    outBuffer[outBufferPos++]=b;
}

void outw(U16 w) {
    outb((U8)w);
    outb((U8)(w>>8));
}

void outd(U32 d) {
    outb((U8)d);
    outb((U8)(d>>8));
    outb((U8)(d>>16));
    outb((U8)(d>>24));
}

void calculateEaa(DecodedOp* op, DynReg reg) {
    regUsed[reg]=true;

    if (op->ea16) {
        // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)

        // xor eax
        outb(0x31);
        outb(0xc0|reg|(reg<<3));

        // add ax, op->disp 
        if (op->disp) {
            outb(0x66);
            outb(0x81);
            outb(0xC0+reg);
            outw(op->disp);
        }
        // add ax, [edi+cpu->reg[op->rm].u16]
        if (op->rm != 8) {
            outb(0x66);
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg16(op->rm));
        }

        // add ax, [cpu->reg[op->sibIndex].u16]
        if (op->sibIndex != 8) {
            outb(0x66);
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg16(op->sibIndex));
        }

        // seg[6] is always 0
        if (op->base<6) { 
            // add eax, [cpu->seg[op->base].address]
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofSegAddress(op->base));
        }
    } else {
        // cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->disp
        bool initiallized = false;

        if (op->sibIndex!=8) {
            initiallized = true;
            // mov eax, [cpu->reg[op->sibIndex].u32];
            outb(0x8b);
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg32(op->sibIndex));

            if (op->sibScale) {                
                // shl eax, op->sibScale
                if (op->sibScale==1) {
                    outb(0xd1);
                    outb(0xe0 | reg);
                } else {
                    outb(0xc1);
                    outb(0xe0 | reg);
                    outb(op->sibScale);
                }
            }
            // seg[6] is always 0
            if (op->base<6 && KThread::currentThread()->process->hasSetSeg[op->base]) { 
                // add eax, [cpu->seg[op->base].address]
                outb(0x03);
                outb(0x47 | (reg << 3));
                outb(CPU::offsetofSegAddress(op->base));
            }
        } else {
            // seg[6] is always 0
            if (op->base<6 && KThread::currentThread()->process->hasSetSeg[op->base]) { 
                initiallized = true;
                // mov eax, [cpu->seg[op->base].address]
                outb(0x8b);
                outb(0x47 | (reg << 3));
                outb(CPU::offsetofSegAddress(op->base));
            }
        }
        // add eax, [cpu->reg[op->rm].u32]
        if (op->rm != 8) {
            if (!initiallized) {
                initiallized = true;
                outb(0x8b); // mov
            } else {
                outb(0x03); // add
            }
            outb(0x47 | (reg << 3));
            outb(CPU::offsetofReg32(op->rm));
        }

        // add eax, op->disp 
        if (op->disp) {
            if (!initiallized) {
                initiallized = true;
                outb(0xb8+reg); // mov
            } else {
                outb(0x81); // add
                outb(0xc0 | reg);
            }            
            outd(op->disp);
        }       
        if (!initiallized) {
            // xor reg, reg
            outb(0x31);
            outb(0xc0|reg|(reg<<3));
        }
    }
}

void byteSwapReg32(DynReg reg) {
    outb(0x0f);
    outb(0xc8 + reg);
}

void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {    
    regUsed[dst] = true;
    if (dstWidth<=srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth, doneWithSrcReg);
    } else {
        if (dstWidth==DYN_32bit) {
            if (srcWidth==DYN_16bit) {
                outb(0x0f);
                outb(0xbf);
                outb(0xC0 | (src << 3) | dst);
            } else if (srcWidth==DYN_8bit) {
                outb(0x0f);
                outb(0xbe);
                outb(0xC0 | (src << 3) | dst);
            } else {
                kpanic("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth==DYN_16bit) {
            if (srcWidth==DYN_8bit) {
                outb(0x66);
                outb(0x0f);
                outb(0xbe);
                outb(0xC0 | (src << 3) | dst);
            } else {
                kpanic("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
            }           
        } else {
            kpanic("unknown width in x32CPU::movToRegFromRegSignExtend %d <= %d", dstWidth, srcWidth);
        }
        if (doneWithSrcReg) {
            regUsed[src] = false;
        }
    }
}

void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth, bool doneWithSrcReg) {
    regUsed[dst] = true;
    if (dstWidth<=srcWidth) {
        if (dst==src) // downsizing doesn't need anything
            return;
        if (dstWidth==DYN_32bit) {
            outb(0x89);
            outb(0xC0 | (src << 3) | dst);
        } else if (dstWidth==DYN_16bit) {
            outb(0x66);
            outb(0x89);
            outb(0xC0 | (src << 3) | dst);
        } else if (dstWidth==DYN_8bit) {
            outb(0x88);
            if (src>=4 || dst>=4) {
                kpanic("x32CPU::movToRegFromReg invalid code, only first 4 regs allowed");
            }
            outb(0xC0 | (src << 3) | dst);
        }  else {
            kpanic("unknown dstWidth in x32CPU::movToRegFromReg %d", dstWidth);
        }
    } else {
        if (dstWidth==DYN_32bit) {
            if (srcWidth==DYN_16bit) {
                outb(0x0f);
                outb(0xb7);
                outb(0xC0 | (dst << 3) | src);
            } else if (srcWidth==DYN_8bit) {
                outb(0x0f);
                outb(0xb6);
                outb(0xC0 | (dst << 3) | src);
            } else {
                kpanic("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth==DYN_16bit) {
            if (srcWidth==DYN_8bit) {
                outb(0x66);
                outb(0x0f);
                outb(0xb6);
                outb(0xC0 | (dst << 3) | src);
            } else {
                kpanic("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }           
        } else {
            kpanic("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
        }
    }
    if (doneWithSrcReg) {
        regUsed[src] = false;
    }
}

void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width) {    
    regUsed[reg] = true;
    // mov reg, [edi+srcOffset]    
    if (width == DYN_32bit) {
        outb(0x8b);
    } else if (width == DYN_16bit) {
        outb(0x66);
        outb(0x8b);
    } else if (width == DYN_8bit) {
        outb(0x8a);
    } else {
        kpanic("unknown dstWidth in x32CPU::movToRegFromCpu %d", width);
    }

    if (srcOffset<=127) {
        outb(0x47|(reg << 3));
        outb((U8)srcOffset);
    } else {
        outb(0x87|(reg << 3));
        outd(srcOffset);
    }
}

void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width, bool doneWithReg) {
    // mov [edi+dstOffset], reg
    if (width == DYN_32bit) {
        outb(0x89);
    } else if (width == DYN_16bit) {
        outb(0x66);
        outb(0x89);
    } else if (width == DYN_8bit) {
        outb(0x88);
    } else {
        kpanic("unknown dstWidth in x32CPU::movToCpuFromReg %d", width);
    }
    if (dstOffset<=127) {
        outb(0x47|(reg << 3));
        outb((U8)dstOffset);
    } else {
        outb(0x87|(reg << 3));
        outd(dstOffset);
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg, bool doneWithTmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(tmpReg, srcOffset, width);

    // mov [cpu+dstOffset], tmpReg
    movToCpuFromReg(dstOffset, tmpReg, width, doneWithTmpReg);    
}

void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm) {
    // mov [cpu+dstOffset], imm
    if (dstWidth == DYN_32bit) {
        outb(0xc7);
    } else if (dstWidth == DYN_16bit) {
        outb(0x66);
        outb(0xc7);
    } else if (dstWidth == DYN_8bit) {
        outb(0xc6);
    } else {
        kpanic("unknown dstWidth in x32CPU::movToCpu %d", dstWidth);
    }

    if (dstOffset<=127) {
        outb(0x47);
        outb((U8)dstOffset);
    } else {
        outb(0x87);
        outd(dstOffset);
    }

    if (dstWidth==DYN_32bit) {
        outd(imm);
    } else if (dstWidth==DYN_16bit) {
        outw(imm);
    } else if (dstWidth == DYN_8bit) {
        outb(imm);
    } else {
        kpanic("unknown width in x32CPU::movToCpu %d", dstWidth);
    }
}

void movToReg(DynReg reg, DynWidth width, U32 imm) {
    regUsed[reg] = true;
    if (imm==0) {
        outb(0x31);
        outb(0xc0|reg|(reg << 3));
    } else {
        outb(0xb8+reg);
        outd(imm);
    }
}

static U32 readd(U32 address) {
    return KThread::currentThread()->memory->readd(address);
}

static U32 readw(U32 address) {
    return KThread::currentThread()->memory->readw(address);
}

static U32 readb(U32 address) {
    return KThread::currentThread()->memory->readb(address);
}

static void writed(U32 address, U32 value) {
    KThread::currentThread()->memory->writed(address, value);
}

static void writew(U32 address, U16 value) {
    KThread::currentThread()->memory->writew(address, value);
}

static void writeb(U32 address, U8 value) {
    KThread::currentThread()->memory->writeb(address, value);
}

void movFromMem(DynWidth width, DynReg addressReg, bool doneWithAddressReg) {
    regUsed[DYN_EAX] = true;
    U32 firstCheckPos=0;

    // make sure we only use the fast path if the entire read will take place on the same page
    if (width==DYN_16bit) {
        // if ((address & 0xFFF) < 0xFFF)
    
        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3));

        // and eax, 0xfff
        outb(0x25);
        outd(0xFFF);

        // cmp eax, 0xfff
        outb(0x3D);
        outd(0xfff);

        // jnb
        outb(0x73);
        firstCheckPos = outBufferPos;
        outb(0);

    } else if (width==DYN_32bit) {
        // if ((address & 0xFFF) < 0xFFD)

        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3));

        // and eax, 0xfff
        outb(0x25);
        outd(0xFFF);

        // cmp eax, 0xffd
        outb(0x3D);
        outd(0xffd);

        // jnb
        outb(0x73);
        firstCheckPos = outBufferPos;
        outb(0);
    }

    // int index = address >> 12;
    // if (Memory::currentMMUReadPtr[index])
    //     return *(U32*)(&Memory::currentMMUReadPtr[index][address & 0xFFF]);
    // else
    //     return readd(address);

    // mov eax, addressReg
    outb(0x89);
    outb(0xc0 | (addressReg<<3));

    // address >> 12
    // shr eax, 12
    outb(0xc1);
    outb(0xe8);
    outb(0x0c);

    // mov eax, [currentMMUReadPtr+sizeof(U8*)*index];
    outb(0x8b);
    outb(0x04);
    outb(0x85);
    outd((U32)getMemData(KThread::currentThread()->memory)->mmuReadPtr);

    // test eax, eax
    outb(0x85);
    outb(0xc0);

    // jz
    outb(0x74);
    U32 jzPos = outBufferPos;
    outb(0); // skip over cached read

    // mov eax, [eax+(address & 0xFFF)]
    U32 reg;
    bool pushedReg = false;
    if (doneWithAddressReg) {
        reg = addressReg;
    } else {
        if (!regUsed[DYN_ECX]) {
            reg = DYN_ECX;
        } else if (!regUsed[DYN_EDX]) {
            reg = DYN_EDX;
        } else {
    #ifdef _DEBUG
            klog("movFromMem ran out of regs");
    #endif
            reg = DYN_ECX;
            pushedReg = true;
            outb(0x51);
        }

        // mov reg, addressReg
        outb(0x89);
        outb(0xc0|reg|(addressReg<<3));
    }
    // and reg, 0xfff
    outb(0x81);
    outb(0xe0+reg);
    outd(0xfff);

    // mov eax, [eax+reg]
    if (width==DYN_8bit) {
        outb(0x8a);
        outb(0x04);
        outb(reg << 3);
    } else if (width==DYN_16bit) {
        outb(0x66);
        outb(0x8b);
        outb(0x04);
        outb(reg << 3);
    } else if (width==DYN_32bit) {
        outb(0x8b);
        outb(0x04);
        outb(reg << 3);
    }

    if (pushedReg) {
        outb(0x59);
    }

    // jmp over slow read
    outb(0xeb);
    U32 slowPos = outBufferPos;
    outb(0);

    outBuffer[jzPos] = (U8)(outBufferPos-jzPos-1);
    if (firstCheckPos)
        outBuffer[firstCheckPos] = (U8)(outBufferPos-firstCheckPos-1);

    // will set EAX so don't push it then clobber the result with a pop

    if (regUsed[DYN_ECX] && addressReg!=DYN_ECX)
        outb(0x51);
    if (regUsed[DYN_EDX] && addressReg!=DYN_EDX)
        outb(0x52);

    // push addressReg
    outb(0x50+addressReg);

    void* address;

    // call read
    if (width == DYN_32bit) {
        address = readd;
    } else if (width == DYN_16bit) {
        address = readw;
    } else if (width == DYN_8bit) {
        address = readb;
    } else {
        kpanic("unknown width in x32CPU::movFromMem %d", width);
    }

    outb(0xe8);
    patch.push_back(outBufferPos);
    outd((U32)address);

    // add esp, 4
    outb(0x83);
    outb(0xc4);
    outb(0x04);
    
    if (regUsed[DYN_EDX] && addressReg!=DYN_EDX)
        outb(0x5a);
    if (regUsed[DYN_ECX] && addressReg!=DYN_ECX)
        outb(0x59);

    outBuffer[slowPos] =  (U8)(outBufferPos-slowPos-1);
    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
}

void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg, bool doneWithAddressReg, bool doneWithCallResult) {
    movFromMem(dstWidth, addressReg, doneWithAddressReg);
    // mov [cpu+srcOffset], eax
    movToCpuFromReg(dstOffset, DYN_CALL_RESULT, dstWidth, doneWithCallResult);
}

void pushValue(U32 arg, DynCallParamType argType) {
    switch (argType) {
    case DYN_PARAM_REG_8:
        // movzx
        outb(0x0f);
        outb(0xb6);
        if (arg>=4) {
            kpanic("x32CPU: invalid arg: %d for DYN_PARAM_REG_8", arg);
        }
        outb(0xC0 | (arg) | (arg<<3));

        outb(0x50+arg);
        break;
    case DYN_PARAM_REG_16:
        // movzx
        outb(0x0f);
        outb(0xb7);
        outb(0xC0 | (arg) | (arg<<3));

        // push
        outb(0x50+arg);
        break;
    case DYN_PARAM_REG_32:
        outb(0x50+arg);
        break;
    case DYN_PARAM_CPU:
        outb(0x57); // cpu should be in edi
        break;
    case DYN_PARAM_CONST_8:
        outb(0x6a);
        outb((U8)arg);
        break;
    case DYN_PARAM_CONST_16:
        outb(0x68);
        outd(arg & 0xFFFF);
        break;
    case DYN_PARAM_CONST_32:
        outb(0x68);
        outd(arg);
        break;
    case DYN_PARAM_CONST_PTR:
        outb(0x68);
        outd(arg);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_8:
        // :TODO: enforce that EAX wasn't used in the callHostFunction
        // mov al, [arg]
        outb(0xa0);
        outd(arg);
        // movzx eax, al
        outb(0x0f);
        outb(0xb6);
        outb(0xc0);
        // push eax
        outb(0x50);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_16:
        // :TODO: enforce that EAX wasn't used in the callHostFunction
        // mov ax, [arg]
        outb(0x66);
        outb(0xa1);
        outd(arg);
        // movzx eax, ax
        outb(0x0f);
        outb(0xb7);
        outb(0xc0);
        // push eax
        outb(0x50);
        break;
    case DYN_PARAM_ABSOLUTE_ADDRESS_32:
        outb(0xff);
        outb(0x35);
        outd(arg);
        break;
    case DYN_PARAM_CPU_ADDRESS_8:
        // mov al, [edi+arg] (edi contains cpu)
        outb(0x8a);
        outb(0x87);
        outd(arg);
        // movzx eax, al
        outb(0x0f);
        outb(0xb6);
        outb(0xc0);
        // push eax
        outb(0x50);
        break;
    case DYN_PARAM_CPU_ADDRESS_16:
        // mov ax, [edi+arg] (edi contains cpu)
        outb(0x66);
        outb(0x8b);
        outb(0x87);
        outd(arg);
        // movzx eax, ax
        outb(0x0f);
        outb(0xb7);
        outb(0xc0);
        // push eax
        outb(0x50);
        break;
    case DYN_PARAM_CPU_ADDRESS_32:
        // mov eax, [edi+arg] (edi contains cpu)
        outb(0x8b);
        outb(0x87);
        outd(arg);
        // push eax
        outb(0x50);
        break;
    default:
        kpanic("x32CPU: unknown argType: %d", argType);
        break;
    }
}

bool isParamTypeReg(DynCallParamType paramType) {
    return paramType==DYN_PARAM_REG_8 || paramType==DYN_PARAM_REG_16 || paramType==DYN_PARAM_REG_32;
}

// will inline the following code snippet (this code is for 32-bit, but it will do a similiar thing for 16-bit and 8-bit)
//
// if ((address & 0xFFF) < 0xFFD) {
//      int index = address >> 12;
//      if (Memory::currentMMUWritePtr[index])
//          *(U32*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
//      else
//          Memory::currentMMU[index]->writed(address, value);		
//  } else {
//      Memory::currentMMU[index]->writed(address, value);		
//  }
void movToMem(DynReg addressReg, DynWidth width, U32 value, DynCallParamType paramType, bool doneWithValueReg) {    
    U32 firstCheckPos=0;

    U32 reg1;
    bool pushedReg1 = false;
    bool isParamReg = isParamTypeReg(paramType);
    if (!regUsed[DYN_EAX] && !(isParamReg && value == DYN_EAX)) {
        reg1 = DYN_EAX;
    } else if (!regUsed[DYN_ECX] && !(isParamReg && value == DYN_ECX)) {
        reg1 = DYN_ECX;
    } else if (!regUsed[DYN_EDX] && !(isParamReg && value == DYN_EDX)) {
        reg1 = DYN_EDX;
    } else if (!regUsed[DYN_EBX] && !(isParamReg && value == DYN_EBX)) {
        reg1 = DYN_EBX;
    } else {
        if (isParamReg && value == DYN_EAX) {
            reg1 = DYN_EDX;
        } else {
            reg1 = DYN_EAX;
        }
        outb(0x50+reg1);
        pushedReg1 = true;
    }

    // make sure we only use the fast path if the entire read will take place on the same page
    if (width==DYN_16bit) {
        // if ((address & 0xFFF) < 0xFFF)
    
        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3) | reg1);

        // and eax, 0xfff
        if (reg1==DYN_EAX) {
            outb(0x25);
            outd(0xFFF);
        } else {
            outb(0x81);
            outb(0xe0|reg1);
            outd(0xfff);
        }

        // cmp eax, 0xfff
        if (reg1==DYN_EAX) {
            outb(0x3D);
            outd(0xfff);
        } else {
            outb(0x81);
            outb(0xf8|reg1);
            outd(0xfff);
        }

        // jnb
        outb(0x73);
        firstCheckPos = outBufferPos;
        outb(0);

    } else if (width==DYN_32bit) {
        // if ((address & 0xFFF) < 0xFFD)

        // mov eax, addressReg
        outb(0x89);
        outb(0xc0 | (addressReg<<3) | reg1);

        // and eax, 0xfff
        if (reg1==DYN_EAX) {
            outb(0x25);
            outd(0xFFF);
        } else {
            outb(0x81);
            outb(0xe0|reg1);
            outd(0xfff);
        }

        // cmp eax, 0xffd
        if (reg1==DYN_EAX) {
            outb(0x3D);
            outd(0xffd);
        } else {
            outb(0x81);
            outb(0xf8|reg1);
            outd(0xffd);
        }

        // jnb
        outb(0x73);
        firstCheckPos = outBufferPos;
        outb(0);
    }

    // int index = address >> 12;
    // if (Memory::currentMMUWritePtr[index])
    //     *(U32*)(&Memory::currentMMUWritePtr[index][address & 0xFFF]) = value;
    // else
    //     Memory::currentMMU[index]->writed(address, value);	

    // mov reg1, addressReg
    outb(0x89);
    outb(0xc0 | (addressReg<<3) | reg1);

    // address >> 12
    // shr reg1, 10
    outb(0xc1);
    outb(0xe8 | reg1);
    outb(0x0c);

    // mov reg1, [currentMMUWritePtr+sizeof(U8*)*index];
    outb(0x8b);
    outb(0x04|(reg1<<3));
    outb(0x85|(reg1<<3));
    outd((U32)getMemData(KThread::currentThread()->memory)->mmuWritePtr);

    // test reg1, reg1
    outb(0x85);
    outb(0xc0 | reg1 | (reg1 << 3));

    // jz
    outb(0x74);
    U32 jzPos = outBufferPos;
    outb(0); // skip over cached read

    // mov eax, [eax+(address & 0xFFF)]
    U32 reg2;
    bool pushedReg2 = false;
    if (!regUsed[DYN_ECX] && reg1!=DYN_ECX && !(isParamReg && value == DYN_ECX)) {
        reg2 = DYN_ECX;
    } else if (!regUsed[DYN_EDX] && reg1!=DYN_EDX && !(isParamReg && value == DYN_EDX)) {
        reg2 = DYN_EDX;
    } else if (!regUsed[DYN_EBX] && reg1!=DYN_EBX && !(isParamReg && value == DYN_EBX)) {
        reg2 = DYN_EBX;
    } else {
        if ((isParamReg && value == DYN_ECX) || reg1 == DYN_ECX) {
            if (reg1==DYN_EAX || (isParamReg && value == DYN_EAX)) {
                reg2 = DYN_EDX;
            } else {
                reg2 = DYN_EAX;
            }
        } else {
            reg2 = DYN_ECX;
        }
        pushedReg2 = true;
        outb(0x50+reg2);
    }

    // mov reg2, addressReg
    outb(0x89);
    outb(0xc0|reg2|(addressReg<<3));

    // and reg, 0xfff
    outb(0x81);
    outb(0xe0+reg2);
    outd(0xfff);

    // mov [eax+reg2], value
    if (paramType == DYN_PARAM_CONST_8 || paramType == DYN_PARAM_CONST_16 || paramType == DYN_PARAM_CONST_32) {
        if (width==DYN_8bit) {
            outb(0xc6);
            outb(0x04);
            outb(reg1 | (reg2 << 3));
            outb((U8)value);
        } else if (width==DYN_16bit) {
            outb(0x66);
            outb(0xc7);
            outb(0x04);
            outb(reg1 | (reg2 << 3));
            outw((U16)value);
        } else if (width==DYN_32bit) {
            outb(0xc7);
            outb(0x04);
            outb(reg1 | (reg2 << 3));
            outd((U32)value);
        }
    } else if (paramType == DYN_PARAM_REG_8 || paramType == DYN_PARAM_REG_16 || paramType == DYN_PARAM_REG_32) {
        if (width==DYN_8bit) {
            outb(0x88);
            outb(0x04|(value<<3));
            outb(reg1 | (reg2 << 3));
        } else if (width==DYN_16bit) {
            outb(0x66);
            outb(0x89);
            outb(0x04|(value<<3));
            outb(reg1 | (reg2 << 3));
        } else if (width==DYN_32bit) {
            outb(0x89);
            outb(0x04|(value<<3));
            outb(reg1 | (reg2 << 3));
        }
    } else {
        kpanic("x32CPU::movToMem unknown param type: %d", paramType);
    }
    if (pushedReg2) {
        outb(0x58+reg2);
    }

    // jmp over slow path
    outb(0xeb);
    U32 slowPos = outBufferPos;
    outb(0);

    outBuffer[jzPos] = (U8)(outBufferPos-jzPos-1);
    if (firstCheckPos)
        outBuffer[firstCheckPos] = (U8)(outBufferPos-firstCheckPos-1);

    if (regUsed[DYN_EAX] && (reg1!=DYN_EAX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EAX))
        outb(0x50);  
    if (regUsed[DYN_ECX] && (reg1!=DYN_ECX || !pushedReg1) && (!doneWithValueReg || value!=DYN_ECX))
        outb(0x51);
    if (regUsed[DYN_EDX] && (reg1!=DYN_EDX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EDX))
        outb(0x52);

    pushValue(value, paramType);

    // push addressReg
    outb(0x50+addressReg);

    void* address;

    // call write
    if (width == DYN_32bit) {
        address = writed;        
    } else if (width == DYN_16bit) {
        address = writew;
    } else if (width == DYN_8bit) {
        address = writeb;
    } else {
        kpanic("unknown width in x32CPU::movToMem %d", width);
    }    

    outb(0xe8);
    patch.push_back(outBufferPos);
    outd((U32)address);

    // add esp, 8
    outb(0x83);
    outb(0xc4);
    outb(0x08);

    if (regUsed[DYN_EDX] && (reg1!=DYN_EDX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EDX))
        outb(0x5a);
    if (regUsed[DYN_ECX] && (reg1!=DYN_ECX || !pushedReg1) && (!doneWithValueReg || value!=DYN_ECX))
        outb(0x59);
    if (regUsed[DYN_EAX] && (reg1!=DYN_EAX || !pushedReg1) && (!doneWithValueReg || value!=DYN_EAX))
        outb(0x58);  

    outBuffer[slowPos] =  (U8)(outBufferPos-slowPos-1);
    if (pushedReg1) {
        outb(0x58+reg1);
    }
}

void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width, bool doneWithAddressReg, bool doneWithReg) {
    // push reg
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_REG_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_REG_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_REG_32;
    else
        kpanic("unknown width %d in x32CPU::movToMemFromReg", width);

    movToMem(addressReg, width, reg, paramType, doneWithReg);   
    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
    if (doneWithReg) {
        regUsed[reg] = false;
    }
}

void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm, bool doneWithAddressReg) {    
    DynCallParamType paramType;

    if (width==DYN_8bit)
        paramType = DYN_PARAM_CONST_8;
    else if (width==DYN_16bit)
        paramType = DYN_PARAM_CONST_16;
    else if (width==DYN_32bit)
        paramType = DYN_PARAM_CONST_32;
    else
        kpanic("unknown width %d in x32CPU::movToMemFromImm", width);

    movToMem(addressReg, width, imm, paramType, false);    
    if (doneWithAddressReg) {
        regUsed[addressReg] = false;
    }
}

void callHostFunction(void* address, bool hasReturn, U32 argCount, U32 arg1, DynCallParamType arg1Type, bool doneWithArg1, U32 arg2, DynCallParamType arg2Type, bool doneWithArg2, U32 arg3, DynCallParamType arg3Type, bool doneWithArg3, U32 arg4, DynCallParamType arg4Type, bool doneWithArg4, U32 arg5, DynCallParamType arg5Type, bool doneWithArg5) {
    bool regDone[4]={false, false, false, false};

    if (argCount>=5) {
        if (isParamTypeReg(arg5Type) && doneWithArg5) {
            if (arg5>=4)
                kpanic("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg5, arg5Type);
            regDone[arg5] = true;
        }
    }
    if (argCount>=4) {
        if (isParamTypeReg(arg4Type) && doneWithArg4) {
            if (arg4>=4)
                kpanic("x32CPU::callHostFunction bad param 4: arg=%d argType=%d", arg4, arg4Type);
            regDone[arg4] = true;
        }
    }
    if (argCount>=3) {
        if (isParamTypeReg(arg3Type) && doneWithArg3) {
            if (arg3>=4)
                kpanic("x32CPU::callHostFunction bad param 3: arg=%d argType=%d", arg3, arg3Type);
            regDone[arg3] = true;
        }
    }
    if (argCount>=2) {
        if (isParamTypeReg(arg2Type) && doneWithArg2) {
            if (arg2>=4)
                kpanic("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg2, arg2Type);
            regDone[arg2] = true;
        }
    }
    if (argCount>=1) {
        if (isParamTypeReg(arg1Type) && doneWithArg1) {
            if (arg1>=4)
                kpanic("x32CPU::callHostFunction bad param 5: arg=%d argType=%d", arg1, arg1Type);
            regDone[arg1] = true;
        }
    } 

    if (hasReturn) {
        regUsed[DYN_EAX]=true;
    } else if (regUsed[DYN_EAX] && !hasReturn && !regDone[DYN_EAX]) {
        outb(0x50);
    }
    if (regUsed[DYN_ECX] && !regDone[DYN_ECX])
        outb(0x51);
    if (regUsed[DYN_EDX] && !regDone[DYN_EDX])
        outb(0x52);
    if (argCount>=5) {
        pushValue(arg5, arg5Type);
    }
    if (argCount>=4) {
        pushValue(arg4, arg4Type);
    }
    if (argCount>=3) {
        pushValue(arg3, arg3Type);
    }
    if (argCount>=2) {
        pushValue(arg2, arg2Type);
    }
    if (argCount>=1) {
        pushValue(arg1, arg1Type);
    }                

    // call address
    outb(0xe8);
    patch.push_back(outBufferPos);
    outd((U32)address);

    // sub esp, 4*argCount
    if (argCount) {
        outb(0x83);
        outb(0xc4);
        outb(0x04*argCount);
    }
    if (regUsed[DYN_EDX] && !regDone[DYN_EDX])
        outb(0x5a);
    if (regUsed[DYN_ECX] && !regDone[DYN_ECX])
        outb(0x59);    
    if (regUsed[DYN_EAX] && !hasReturn && !regDone[DYN_EAX])
        outb(0x58);

    for (int i=0;i<4;i++) {
        if (regDone[i])
            regUsed[i] = false;
    }
}

// inst can be +, |, - , &, ^, <, >, ) right parens is for signed right shift
void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
    if (inst == '<' || inst == '>' || inst == ')') {
        U8 group;
        if (inst == '<')
            group = 0xe0;
        else if (inst == '>')
            group = 0xe8;
        else if (inst == ')')
            group = 0xf8;

        if (regWidth==DYN_32bit) {
            if (imm==1) {
                outb(0xd1);
                outb(group+reg);
            } else {
                outb(0xc1);
                outb(group+reg);
                outb((U8)imm);
            }
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            if (imm==1) {
                outb(0xd1);
                outb(group+reg);
            } else {
                outb(0xc1);
                outb(group+reg);
                outb((U8)imm);
            }
        } if (regWidth==DYN_8bit) {
            if (imm==1) {
                outb(0xd0);
                outb(group+reg);
            } else {
                outb(0xc0);
                outb(group+reg);
                outb((U8)imm);
            }
        }
        return;
    }

    S32 s = (S32)imm;
    bool oneByte = s>=-128 && s<=127;

    U32 i=0;
    switch (inst) {
        case '+':
            i=0;
            break;
        case '-':
            i=5;
            break;
        case '|':
            i=1;
            break;
        case '&':
            i=4;
            break;
        case '^':
            i=6;
            break;
        default:
            kpanic("unhandled op in x32CPU::instRegIMM %c", inst);
            break;
    }
    // add reg, imm
    if (regWidth==DYN_32bit) {            
        outb(oneByte?0x83:0x81);
        outb(0xC0 | i << 3 | reg);
        if (oneByte) {
            outb((U8)imm);
        } else {
            outd(imm);
        }
    } else if (regWidth == DYN_16bit) {
        outb(0x66);
        outb(oneByte?0x83:0x81);
        outb(0xC0 | i << 3 | reg);
        if (oneByte) {
            outb((U8)imm);
        } else {
            outw((U16)imm);
        }
    } else if (regWidth == DYN_8bit) {
        outb(0x80);
        outb(0xC0 | i << 3 | reg);
        outb((U8)imm);
    } else {
        kpanic("unknown regWidth in x32CPU::instRegImm + %d", regWidth);
    }
}
void instCPUReg(char inst, U32 dstOffset, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (dstOffset>127)
        kpanic("x32CPU::instCPUReg register offset expected to be less than 128: %d", dstOffset);

    if (inst == '<' || inst == '>' || inst == ')') {
        U8 group;
        if (inst == '<')
            group = 0x67;
        else if (inst == '>')
            group = 0x6f;
        else if (inst == ')')
            group = 0x7f;

        if (regWidth==DYN_32bit) {
            outb(0xd3);
            outb(group);
            outb((U8)dstOffset);
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0xd3);
            outb(group);
            outb((U8)dstOffset);
        } if (regWidth==DYN_8bit) {
            outb(0xd2);
            outb(group);
            outb((U8)dstOffset);
        }
        return;
    }

    U8 i=0;
    switch (inst) {
        case '+':
            i=0x01;
            break;
        case '-':
            i=0x29;
            break;
        case '|':
            i=0x09;
            break;
        case '&':
            i=0x21;
            break;
        case '^':
            i=0x31;
            break;
        default:
            kpanic("unhandled op in x32CPU::instCPUReg %c", inst);
            break;
    }
    // add [offset], rm
    if (regWidth==DYN_32bit) {            
        outb(i);
        outb(0x47 | rm << 3);
    } else if (regWidth == DYN_16bit) {
        outb(0x66);
        outb(i);
        outb(0x47 | rm << 3);
    } else if (regWidth == DYN_8bit) {
        outb(i-1);
        outb(0x47 | rm << 3);
    } else {
        kpanic("unknown regWidth in x32CPU::instCPUReg + %d", regWidth);
    }    
    outb((U8)dstOffset);
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}
void instCPUImm(char inst, U32 dstOffset, DynWidth regWidth, U32 imm) {
    if (dstOffset>127)
        kpanic("x32CPU::instCPUImm register offset expected to be less than 128: %d", dstOffset);

    if (inst == '<' || inst == '>' || inst == ')') {
        U8 group;
        if (inst == '<')
            group = 0x67;
        else if (inst == '>')
            group = 0x6f;
        else if (inst == ')')
            group = 0x7f;

        if (regWidth==DYN_32bit) {
            if (imm==1) {
                outb(0xd1);
                outb(group);
                outb((U8)dstOffset);
            } else {
                outb(0xc1);
                outb(group);
                outb((U8)dstOffset);
                outb((U8)imm);
            }
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            if (imm==1) {
                outb(0xd1);
                outb(group);
                outb((U8)dstOffset);
            } else {
                outb(0xc1);
                outb(group);
                outb((U8)dstOffset);
                outb((U8)imm);
            }
        } if (regWidth==DYN_8bit) {
            if (imm==1) {
                outb(0xd0);
                outb(group);
                outb((U8)dstOffset);
            } else {
                outb(0xc0);
                outb(group);
                outb((U8)dstOffset);
                outb((U8)imm);
            }
        }
        return;
    }

    S32 s = (S32)imm;
    bool oneByte = s>=-128 && s<=127;

    U32 i=0;
    switch (inst) {
        case '+':
            i=0;
            break;
        case '-':
            i=5;
            break;
        case '|':
            i=1;
            break;
        case '&':
            i=4;
            break;
        case '^':
            i=6;
            break;
        default:
            kpanic("unhandled op in x32CPU::instCPUImm %c", inst);
            break;
    }
    // add [reg], imm
    if (regWidth==DYN_32bit) {            
        outb(oneByte?0x83:0x81);
        outb(0x47 | (i<<3));
        outb((U8)dstOffset);

        if (oneByte) {
            outb((U8)imm);
        } else {
            outd(imm);
        }
    } else if (regWidth == DYN_16bit) {
        outb(0x66);
        outb(oneByte?0x83:0x81);
        outb(0x47 | (i<<3));
        outb((U8)dstOffset);
        if (oneByte) {
            outb((U8)imm);
        } else {
            outw((U16)imm);
        }
    } else if (regWidth == DYN_8bit) {
        outb(0x80);
        outb(0x47 | (i<<3));
        outb((U8)dstOffset);
        outb((U8)imm);
    } else {
        kpanic("unknown regWidth in x32CPU::instCPUImm + %d", regWidth);
    }    
}

void instMemImm(char inst, DynReg addressReg, DynWidth regWidth, U32 imm, bool doneWithAddressReg) {
    DynReg reg;
    bool pushed = false;

    if (addressReg==DYN_EAX) {
        kpanic("x32CPU::instMemImm doesn't handle passing the address reg in EAX");
    }
    if (!regUsed[DYN_EAX] && addressReg != DYN_EAX) {
        reg = DYN_EAX;
    } else {
        outb(0x50);
        reg = DYN_EAX;
        pushed = true;
#ifdef _DEBUG
        klog("TODO: x32CPU::instMem pushed EAX.");
#endif
    }    
    movFromMem(regWidth, addressReg, false);
    instRegImm(inst, DYN_EAX, regWidth, imm);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true);
    if (pushed) {
        outb(0x58);
    }
}
void instMemReg(char inst, DynReg addressReg, DynReg rm, DynWidth regWidth, bool doneWithAddressReg, bool doneWithRmReg) {
    DynReg reg;
    bool pushed = false;

    if (addressReg==DYN_EAX) {
        kpanic("x32CPU::instMemReg doesn't handle passing the address reg in EAX");
    }
    if (rm==DYN_EAX) {
        kpanic("x32CPU::instMemReg doesn't handle passing the rm reg in EAX");
    }
    if (!regUsed[DYN_EAX] && addressReg != DYN_EAX && rm != DYN_EAX) {
        reg = DYN_EAX;
    } else {
        outb(0x50);
        reg = DYN_EAX;
        pushed = true;
#ifdef _DEBUG
        klog("TODO: x32CPU::instMem pushed EAX.");
#endif
    }    
    movFromMem(regWidth, addressReg, false);
    instRegReg(inst, DYN_EAX, rm, regWidth, doneWithRmReg);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true);
    if (pushed) {
        outb(0x58);
    }
}

// inst can be +, |, -, &, ^, <, >, ) right parens is for signed right shift
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth, bool doneWithRmReg) {
    if (inst == '<' || inst == '>' || inst == ')') {
        U8 group;
        if (inst == '<')
            group = 0xe0;
        else if (inst == '>')
            group = 0xe8;
        else if (inst == ')')
            group = 0xf8;

        if (regWidth==DYN_32bit) {
            outb(0xd3);
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0xd3);
        } if (regWidth==DYN_8bit) {
            outb(0xd2);
        }
        outb(group | reg);
    }
    else {
        U8 i = 0;
        switch (inst) {
        case '+':
            i = 0x01;
            break;
        case '-':
            i = 0x29;
            break;
        case '|':
            i = 0x09;
            break;
        case '&':
            i = 0x21;
            break;
        case '^':
            i = 0x31;
            break;
        default:
            kpanic("unhandled op in x32CPU::instRegReg %c", inst);
            break;
        }
        // add reg, imm
        if (regWidth == DYN_32bit) {
            outb(i);
            outb(0xC0 | rm << 3 | reg);
        }
        else if (regWidth == DYN_16bit) {
            outb(0x66);
            outb(i);
            outb(0xC0 | rm << 3 | reg);
        }
        else if (regWidth == DYN_8bit) {
            outb(i - 1);
            outb(0xC0 | rm << 3 | reg);
        }
        else {
            kpanic("unknown regWidth in x32CPU::instRegImm + %d", regWidth);
        }
    }
    if (doneWithRmReg) {
        regUsed[rm] = false;
    }
}

// inst can be: ~ or -
void instReg(char inst, DynReg reg, DynWidth regWidth) {
    switch (inst) {
    case '~':
        if (regWidth==DYN_32bit) {
            outb(0xf7);
            outb(0xd0+reg);            
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0xf7);
            outb(0xd0+reg);            
        } else if (regWidth==DYN_8bit) {
            outb(0xf6);
            outb(0xd0+reg);   
        } else {
            kpanic("unhandled regWidth in x32CPU::instReg %d", regWidth);
        }
        break;
    case '-':
        if (regWidth==DYN_32bit) {
            outb(0xf7);
            outb(0xd8+reg);            
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0xf7);
            outb(0xd8+reg);            
        } else if (regWidth==DYN_8bit) {
            outb(0xf6);
            outb(0xd8+reg);   
        } else {
            kpanic("unhandled regWidth in x32CPU::instReg %d", regWidth);
        }
        break;
    default:
        kpanic("unhandled op in x32CPU::instReg %c", inst);
        break;
    }
}

void instMem(char inst, DynReg addressReg, DynWidth regWidth, bool doneWithAddressReg) {
    DynReg reg;
    bool pushed = false;

    if (addressReg==DYN_EAX) {
        kpanic("x32CPU::instMem doesn't handle passing the address reg in EAX");
    }
    if (!regUsed[DYN_EAX] && addressReg != DYN_EAX) {
        reg = DYN_EAX;
    } else {
        outb(0x50);
        reg = DYN_EAX;
        pushed = true;
#ifdef _DEBUG
        klog("TODO: x32CPU::instMem pushed EAX.");
#endif
    }    
    movFromMem(regWidth, addressReg, false);
    instReg(inst, DYN_EAX, regWidth);
    movToMemFromReg(addressReg, DYN_EAX, regWidth, true, true);   
    if (pushed) {
        outb(0x58);
    }
}

void instCPU(char inst, U32 dstOffset, DynWidth regWidth) {
    switch (inst) {
    case '~':
        if (regWidth==DYN_32bit) {
            outb(0xf7);
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0xf7);
        } else if (regWidth==DYN_8bit) {
            outb(0xf6); 
        } else {
            kpanic("unhandled regWidth in x32CPU::instCPU %d", regWidth);
        }
        if (dstOffset<128) {
            outb(0x57);
            outb((U8)dstOffset);
        } else {
            outb(0x97);
            outd(dstOffset);
        }
        break;
    case '-':
        if (regWidth==DYN_32bit) {
            outb(0xf7);          
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0xf7);          
        } else if (regWidth==DYN_8bit) {
            outb(0xf6);  
        } else {
            kpanic("unhandled regWidth in x32CPU::instCPU %d", regWidth);
        }
        if (dstOffset<128) {
            outb(0x5f);
            outb((U8)dstOffset);
        } else {
            outb(0x9f);
            outd(dstOffset);
        }
        break;
    default:
        kpanic("unhandled op in x32CPU::instCPU %c", inst);
        break;
    }
}

void startIf(DynReg reg, DynCondition condition, bool doneWithReg) {
    // test reg, reg
    outb(0x85);
    outb(0xc0 | reg | (reg << 3));

    if (condition==DYN_NOT_EQUALS_ZERO) {
        outb(0x74); // jz, jump over if not true
    } else if (condition==DYN_EQUALS_ZERO) {
        outb(0x75); // jnz, jump over not true
    } else {
        kpanic("x32CPU::startIf unknown condition %d", condition);
    }

    ifJump.push_back(outBufferPos);
    outb(0); // jump over amount
    if (doneWithReg)
        regUsed[reg] = false;
}

void startElse() {    
    outb(0xeb); // previous block should jump over else statement
    U32 pos = outBufferPos;
    outb(0); // jump over amount

    // if statement will jump here if it wasn't true
    endIf();

    ifJump.push_back(pos);
}

void endIf() {
    U32 pos = ifJump.back();
    U32 amount = outBufferPos-pos-1;
    if (amount>127) {
        kpanic("x32CPU::endIf large if/else blocks not supported: %d", amount);
    }
    ifJump.pop_back();
    outBuffer[pos] = (U8)(amount);
}

void evaluateToReg(DynReg reg, DynWidth dstWidth, DynReg left, bool isRightConst, DynReg right, U32 rightConst, DynWidth regWidth, DynConditionEvaluate condition, bool doneWithLeftReg, bool doneWithRightReg) {
    if (reg>=4) {
        kpanic("x32CPU::evaluateToRegFromRegs doesn't support reg %d", reg);
    }
    // cmp left, right
    if (isRightConst) {
        S32 sRightConst = (S32)rightConst;
        bool isOneByte = sRightConst>=-128 && sRightConst<=127;
        if (isOneByte && (regWidth==DYN_32bit || regWidth==DYN_16bit)) {
            if (regWidth==DYN_32bit) {
                outb(0x83);
            } else if (regWidth==DYN_16bit) {
                outb(0x66);
                outb(0x83);
            } else {
                kpanic("x32CPU::evaluateToRegFromRegs reg width %d", regWidth);
            }
            outb(0xf8 | left);
            outb((U8)rightConst);
        } else {
            if (left==DYN_EAX) {
                if (regWidth==DYN_32bit) {
                    outb(0x3d);
                    outd(rightConst);
                } else if (regWidth==DYN_16bit) {
                    outb(0x66);
                    outb(0x3d);
                    outw((U16)rightConst);
                } else if (regWidth==DYN_8bit) {
                    outb(0x3c);
                    outb((U8)rightConst);
                } else {
                    kpanic("x32CPU::evaluateToRegFromRegs reg width %d", regWidth);
                }
            } else {
                if (regWidth==DYN_32bit) {
                    outb(0x81);
                    outb(0xf8 | left);
                    outd(rightConst);
                } else if (regWidth==DYN_16bit) {
                    outb(0x66);
                    outb(0x81);
                    outb(0xf8 | left);
                    outw((U16)rightConst);
                } else if (regWidth==DYN_8bit) {
                    outb(0x80);
                    outb(0xf8 | left);
                    outb((U8)rightConst);
                } else {
                    kpanic("x32CPU::evaluateToRegFromRegs reg width %d", regWidth);
                }                                
            }            
        }
    } else {
        if (regWidth==DYN_32bit) {
            outb(0x39);
        } else if (regWidth==DYN_16bit) {
            outb(0x66);
            outb(0x39);
        } else if (regWidth==DYN_8bit) {
            outb(0x38);
        } else {
            kpanic("x32CPU::evaluateToRegFromRegs reg width %d", regWidth);
        }
        outb(0xc0 | right | (left << 3));
    }

    switch (condition) {
    case DYN_EQUALS:        
        // setz reg
        outb(0x0f);
        outb(0x94);
        outb(0xc0+reg);
        break;
    case DYN_NOT_EQUALS:
        // setnz reg
        outb(0x0f);
        outb(0x95);
        outb(0xc0+reg);
        break;
    case DYN_LESS_THAN_UNSIGNED:
        // setnbe reg
        outb(0x0f);
        outb(0x97);
        outb(0xc0+reg);
        break;
    case DYN_LESS_THAN_EQUAL_UNSIGNED:
        // setnb reg
        outb(0x0f);
        outb(0x93);
        outb(0xc0+reg);
        break;
    case DYN_GREATER_THAN_EQUAL_UNSIGNED:
        // setbe reg
        outb(0x0f);
        outb(0x96);
        outb(0xc0+reg);
        break;
    case DYN_LESS_THAN_SIGNED:
        // setnle reg
        outb(0x0f);
        outb(0x9f);
        outb(0xc0+reg);
        break;
    case DYN_LESS_THAN_EQUAL_SIGNED:
        // setnl reg
        outb(0x0f);
        outb(0x9d);
        outb(0xc0+reg);
        break;
    default:
        kpanic("x32CPU::evaluateToRegFromRegs unknown condition %d", condition);
    }
    if (dstWidth!=DYN_8bit) {
        movToRegFromReg(reg, dstWidth, reg, DYN_8bit, false);
    }
    if (doneWithLeftReg)
        regUsed[left] = false;
    if (doneWithRightReg)
        regUsed[right] = false;
    regUsed[reg] = true;
}

// this is good generic code to copy for other implementations that don't have something like setcc
/*
void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToCpu(offset, regWidth, 0);
    startElse();
    movToCpu(offset, regWidth, 1);
    endIf();
}
*/

void setCPU(DynamicData* data, U32 offset, DynWidth regWidth, DynConditional condition) {
    U8 setnz = 0x95;
    // changing conditions to ones that are optimized
    if (condition==Z) {
        condition = NZ;
        setnz = 0x94; // change to setz
    } else if (condition == NS) {
        condition = S;
        setnz = 0x94; // change to setz
    } else if (condition == NB) {
        condition = B;
        setnz = 0x94; // change to setz
    } else if (condition == NO) {
        condition = O;
        setnz = 0x94; // change to setz
    } else if (condition == NBE) {
        condition = BE;
        setnz = 0x94; // change to setz
    } else if (condition == NLE) {
        condition = LE;
        setnz = 0x94; // change to setz
    } else if (condition == NL) {
        condition = L;
        setnz = 0x94; // change to setz
    }
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    
    // test reg, reg
    outb(0x85);
    outb(0xc0 | DYN_CALL_RESULT | (DYN_CALL_RESULT << 3));

    // setnz al
    outb(0x0f);
    outb(setnz);
    outb(0xc0);

    if (regWidth!=DYN_8bit) {
        movToRegFromReg(DYN_EAX, regWidth, DYN_EAX, DYN_8bit, false);
    }
    movToCpuFromReg(offset, DYN_EAX, regWidth, true);
}

/*
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    startIf(DYN_CALL_RESULT, DYN_EQUALS_ZERO, true);
    movToReg(DYN_SRC, regWidth, 0);
    startElse();
    movToReg(DYN_SRC, regWidth, 1);
    endIf();
    // don't put this movToMem in the if statement because it is big and will be inlines once in each block of the if statement
    movToMemFromReg(DYN_ADDRESS, DYN_SRC, regWidth, doneWithAddressReg, true);
}
*/
void setMem(DynamicData* data, DynReg addressReg, DynWidth regWidth, DynConditional condition, bool doneWithAddressReg) {
    U8 setnz = 0x95;
    if (condition==Z) {
        condition = NZ;
        setnz = 0x94; // change to setz
    } else if (condition == NS) {
        condition = S;
        setnz = 0x94; // change to setz
    } else if (condition == NB) {
        condition = B;
        setnz = 0x94; // change to setz
    } else if (condition == NO) {
        condition = O;
        setnz = 0x94; // change to setz
    } else if (condition == NBE) {
        condition = BE;
        setnz = 0x94; // change to setz
    } else if (condition == NLE) {
        condition = LE;
        setnz = 0x94; // change to setz
    } else if (condition == NL) {
        condition = L;
        setnz = 0x94; // change to setz
    }
    setConditionInReg(data, condition, DYN_CALL_RESULT);
    
    // test reg, reg
    outb(0x85);
    outb(0xc0 | DYN_CALL_RESULT | (DYN_CALL_RESULT << 3));

    // setnz al
    outb(0x0f);
    outb(setnz);
    outb(0xc0);

    if (regWidth!=DYN_8bit) {
        movToRegFromReg(DYN_EAX, regWidth, DYN_EAX, DYN_8bit, false);
    }
    movToMemFromReg(addressReg, DYN_EAX, regWidth, doneWithAddressReg, true);
}

void incrementEip(U32 inc) {
    S32 d = (S32)inc;
    if (d>=-128 && d<=127) {
        outb(0x83);
        outb(0x47);
        outb(offsetof(CPU, eip.u32));
        outb((U8)inc);
    } else {
        outb(0x81);
        outb(0x47);
        outb(offsetof(CPU, eip.u32));
        outd(inc);
    }
}

void incrementEip(DynamicData* data, DecodedOp* op) {
    if (op->next) {
        const InstructionInfo& info = instructionInfo[op->next->inst];
        if (!info.branch && !info.readMemWidth && !info.writeMemWidth && !info.throwsException) {
            data->skipEipUpdateLen += op->len;
            return;
        }
    }
    U32 len = op->len + data->skipEipUpdateLen;
    data->skipEipUpdateLen = 0;
    incrementEip(len);
}

void incrementEip(DynamicData* data, U32 len) {
    if (data->skipEipUpdateLen) {
        kpanic("incrementEip had an unexpected update");
    }
    incrementEip(len);
}

void blockDone() {
    // cpu->nextBlock = cpu->getNextBlock();
    callHostFunction(common_getNextBlock, true, 1, 0, DYN_PARAM_CPU, false);
    movToCpuFromReg(offsetof(CPU, nextBlock), DYN_CALL_RESULT, DYN_32bit, true);
    outb(0x5f); // pop edi
    outb(0x5b); // pop ebx
    outb(0xc3); // ret
}

static DecodedBlock* updateNext1(CPU* cpu) {
    DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); 
    DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);
    return DecodedBlock::currentBlock->next1;
}

// next block is also set in common_other.cpp for loop instructions, so don't use this as a hook for something else
void blockNext1() {
    // if (!DecodedBlock::currentBlock->next1) {
    //    DecodedBlock::currentBlock->next1 = cpu->getNextBlock(); 
    //    DecodedBlock::currentBlock->next1->addReferenceFrom(DecodedBlock::currentBlock);
    // } 
    // cpu->nextBlock = DecodedBlock::currentBlock->next1
    
    // mov edx, DecodedBlock::currentBlock
    outb(0x8b);
    outb(0x15);
    outd((U32)&DecodedBlock::currentBlock);

    // mov eax, DecodedBlock::currentBlock->next1
    outb(0x8b);    
    if (offsetof(DecodedBlock, next1)<128) {
        outb(0x42);
        outb(offsetof(DecodedBlock, next1));
    } else {
        outb(0x82);
        outd(offsetof(DecodedBlock, next1));
    }

    // test eax, eax
    outb(0x85);
    outb(0xc0);

    // jnz 
    outb(0x75);
    U32 pos = outBufferPos;
    outb(0);
    
    callHostFunction(updateNext1, true, 1, 0, DYN_PARAM_CPU);

    outBuffer[pos] = (U8)(outBufferPos-pos-1);

    // cpu->nextBlock = DecodedBlock::currentBlock->next1
    movToCpuFromReg(offsetof(CPU, nextBlock), DYN_CALL_RESULT, DYN_32bit, true);
    
}

static DecodedBlock* updateNext2(CPU* cpu) {
    DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); 
    DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);
    return DecodedBlock::currentBlock->next2;
}

void blockNext2() {
    // if (!DecodedBlock::currentBlock->next2) {
    //    DecodedBlock::currentBlock->next2 = cpu->getNextBlock(); 
    //    DecodedBlock::currentBlock->next2->addReferenceFrom(DecodedBlock::currentBlock);
    // } 
    // cpu->nextBlock = DecodedBlock::currentBlock->next2
    
    // mov edx, DecodedBlock::currentBlock
    outb(0x8b);
    outb(0x15);
    outd((U32)&DecodedBlock::currentBlock);

    // mov eax, DecodedBlock::currentBlock->next1
    outb(0x8b);    
    if (offsetof(DecodedBlock, next2)<128) {
        outb(0x42);
        outb(offsetof(DecodedBlock, next2));
    } else {
        outb(0x82);
        outd(offsetof(DecodedBlock, next2));
    }

    // test eax, eax
    outb(0x85);
    outb(0xc0);

    // jnz 
    outb(0x75);
    U32 pos = outBufferPos;
    outb(1);
    
    callHostFunction(updateNext2, true, 1, 0, DYN_PARAM_CPU);

    outBuffer[pos] = (U8)(outBufferPos-pos-1);

    // cpu->nextBlock = DecodedBlock::currentBlock->next2
    movToCpuFromReg(offsetof(CPU, nextBlock), DYN_CALL_RESULT, DYN_32bit, true);
}

void x32_sidt(DynamicData* data, DecodedOp* op) {
}

void x32_onExitSignal(CPU* cpu) {
    onExitSignal(cpu, NULL);
}

void x32_callback(DynamicData* data, DecodedOp* op) {
    if (op->pfn == onExitSignal) {
        callHostFunction(x32_onExitSignal, false, 1, 0, DYN_PARAM_CPU);
    } else {
        kpanic("x32CPU::x32_callback unhandled callback");
    }
}

void x32_invalid_op(DynamicData* data, DecodedOp* op) {
    kpanic("Invalid instruction %x\n", op->inst);
}

static pfnDynamicOp x32Ops[NUMBER_OF_OPS];
static U32 x32OpsInitialized;

static void initX32Ops() {
    if (x32OpsInitialized)
        return;
    if (offsetof(CPU, eip.u32)>127)
        kpanic("x32CPU::initX32Ops wasn't expecting eip offset to be greater than 127");

    if (offsetof(CPU, reg[8].u32)>127)
        kpanic("x32CPU::initX32Ops wasn't expecting reg[8] offset to be greater than 127");

    if (offsetof(CPU, seg[6].address)>127)
        kpanic("x32CPU::initX32Ops wasn't expecting reg[8] offset to be greater than 127");

    x32OpsInitialized = 1;   
    for (int i=0;i<InstructionCount;i++) {
        x32Ops[i] = x32_invalid_op;
    }
#define INIT_CPU(e, f) x32Ops[e] = dynamic_##f;
#include "../common/cpu_init.h"
#include "../common/cpu_init_mmx.h"
#include "../common/cpu_init_sse.h"
#include "../common/cpu_init_sse2.h"
#include "../common/cpu_init_fpu.h"
#undef INIT_CPU    
    
    x32Ops[SLDTReg] = 0; 
    x32Ops[SLDTE16] = 0;
    x32Ops[STRReg] = 0; 
    x32Ops[STRE16] = 0;
    x32Ops[LLDTR16] = 0; 
    x32Ops[LLDTE16] = 0;
    x32Ops[LTRR16] = 0; 
    x32Ops[LTRE16] = 0;
    x32Ops[VERRR16] = 0; 
    x32Ops[VERWR16] = 0; 
    x32Ops[SGDT] = 0;
    x32Ops[SIDT] = x32_sidt;
    x32Ops[LGDT] = 0;
    x32Ops[LIDT] = 0;
    x32Ops[SMSWRreg] = 0; 
    x32Ops[SMSW] = 0;
    x32Ops[LMSWRreg] = 0; 
    x32Ops[LMSW] = 0;
    x32Ops[INVLPG] = 0;
    x32Ops[Callback] = x32_callback;
}

void OPCALL firstDynamicOp(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    if (DecodedBlock::currentBlock->runCount == 0) {
#else
    if (DecodedBlock::currentBlock->runCount == 50) {
#endif
        DynamicData data;
        data.cpu = cpu;
        data.block = DecodedBlock::currentBlock;

        initX32Ops();
        DecodedOp* o = op->next;
        outBufferPos = 0;
        patch.clear();
        outb(0x53); // push ebx
        outb(0x57); // push edi , will hold cpu
        // on win32 ecx contains cpu
        // mov edi, ecx
        outb(0x89);
        outb(0xcf);
        while (o) {
            memset(regUsed, 0, sizeof(regUsed));
#ifndef __TEST
#ifdef _DEBUG
            callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)o, DYN_PARAM_CONST_PTR, false);
#endif
#endif
            x32Ops[o->inst](&data, o); 
            if (ifJump.size()) {
                kpanic("x32CPU::firstDynamicOp if statement was not closed in instruction: %d", op->inst);
            }
            if (data.skipToOp) {
                o = data.skipToOp;
                data.skipToOp = NULL;
            } else if (data.done) {
#ifndef __TEST
#ifdef _DEBUG
                if (o->next)
                    callHostFunction(common_log, false, 2, 0, DYN_PARAM_CPU, false, (DYN_PTR_SIZE)o->next, DYN_PARAM_CONST_PTR, false);
#endif
#endif
                break;
            } else {
                o = o->next;
            }
        }
        outb(0x5f); // pop edi
        outb(0x5b); // pop ebx
        outb(0xc3); // ret
        DynamicMemory* memory = cpu->memory->dynamicMemory;
        if (!memory) {
            memory = new DynamicMemory();
            cpu->memory->dynamicMemory = memory;
        }
        void* mem = NULL;

        if (memory->dynamicExecutableMemory.size()==0) {
            int blocks = (outBufferPos+0xffff)/0x10000;
            memory->dynamicExecutableMemoryLen = blocks*0x10000;
            mem = Platform::alloc64kBlock(blocks, true);
            memory->dynamicExecutableMemoryPos = 0;
            memory->dynamicExecutableMemory.push_back(DynamicMemoryData(mem, blocks * 0x10000));
        } else {
            mem = memory->dynamicExecutableMemory[memory->dynamicExecutableMemory.size()-1].p;
            if (memory->dynamicExecutableMemoryPos+outBufferPos>=memory->dynamicExecutableMemoryLen) {
                int blocks = (outBufferPos+0xffff)/0x10000;
                memory->dynamicExecutableMemoryLen = blocks*0x10000;
                mem = Platform::alloc64kBlock(blocks, true);
                memory->dynamicExecutableMemoryPos = 0;
                memory->dynamicExecutableMemory.push_back(DynamicMemoryData(mem, blocks * 0x10000));
            }
        }
        U8* begin = (U8*)mem+memory->dynamicExecutableMemoryPos;
        memcpy(begin, outBuffer, outBufferPos);
        memory->dynamicExecutableMemoryPos+=outBufferPos;
        
        for (U32 i=0;i<patch.size();i++) {
            U32 pos = patch[i];
            U32* value = (U32*)(&begin[pos]);
            *value = *value - (U32)(begin+pos+4);
        }
        bool b = false;
        if (b) {
            printf("\n");
            for (U32 i=0;i<outBufferPos;i++) {
                printf("%0.2X ", outBuffer[i]);
            }
            printf("\n");
        }
#ifndef _DEBUG
        //op->next->dealloc(true);
        //op->next = NULL;
#endif
        op->pfn = (OpCallback)begin; // :TODO: if function is expected to pop stack because of the two passed params, then this will not work        
        op->pfn(cpu, op);
    } else {
        op->next->pfn(cpu, op->next);
    }
}

#endif