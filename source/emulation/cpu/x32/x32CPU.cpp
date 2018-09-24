#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC32

#include "x32CPU.h"
#include "../common/lazyFlags.h"

// cdecl calling convention states EAX, ECX, and EDX are caller saved
// to prevent unnecessary push/pop, it would be nice to use esi/edi instead of ecx/edx
// but then 8 bit math operations would be very hard because you can't reference esi/edi from 8-bit


/********************************************************/
/* Following is required to be defined for dynamic code */
/********************************************************/

#define INCREMENT_EIP(x) incrementEip(x)

#define OFFSET_REG8(x) (x>=4?offsetof(CPU, reg[x-4].h8):offsetof(CPU, reg[x].u8))

// DynReg is a required type, but the values inside are local to this function
enum DynReg {
    DYN_EAX=0,
    DYN_ECX=1,
    DYN_EDX=2,
    DYN_EBX=3,    
};

#define DYN_READ_RESULT DYN_EAX
#define DYN_CALL_RESULT DYN_EAX
#define DYN_SRC DYN_ECX
#define DYN_DEST DYN_EDX
#define DYN_ADDRESS DYN_EBX
#define DYN_ANY DYN_DEST

#define DYN_PTR_SIZE U32

enum DynWidth {
    DYN_8bit,
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

#define Dyn_PtrSize DYN_32bit

// helper, can be done with multiple other calls
void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg);
void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg);
void calculateEaa(DecodedOp* op, DynReg reg);

// REG to REG
void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth);
void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth);

// to Reg
void movToReg(DynReg reg, DynWidth width, U32 imm);

// to CPU
void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width);
void movToCpu(U32 dstOffset, DynWidth dstWidth, U32 imm);

// from CPU
void movToRegFromCpu(DynReg reg, U32 srcOffset, DynWidth width);

// from Mem to DYN_READ_RESULT
void movFromMem(DynWidth width, DynReg addressReg);

// to Mem
void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width);
void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm);

// arith
void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm);
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth);
void instReg(char inst, DynReg reg, DynWidth regWidth);

// conditional, didn't want to create more complicated generic routines to handle these, so they are special
void movCC(void* condition, DecodedOp* op, DynWidth width, bool useAddress=false);
void setCC(void* condition, DecodedOp* op, bool useAddress=false);

// call into emulator, like setFlags, getCF, etc
void callHostFunction(void* address, bool hasReturn=false, bool returnIfTrue=false, bool returnIfFalse=false, U32 argCount=0, U32 arg1=0, DynCallParamType arg1Type=DYN_PARAM_CONST_32, U32 arg2=0, DynCallParamType arg2Type=DYN_PARAM_CONST_32, U32 arg3=0, DynCallParamType arg3Type=DYN_PARAM_CONST_32, U32 arg4=0, DynCallParamType arg4Type=DYN_PARAM_CONST_32, U32 arg5=0, DynCallParamType arg5Type=DYN_PARAM_CONST_32);

/********************************************************/
/* End required for dynamic code                        */
/********************************************************/

// referenced in macro above
void incrementEip(U32 inc);

#include "../normal/instructions.h"
#include "../common/common_arith.h"
#include "../common/common_pushpop.h"
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
#include "../dynamic/dynamic_fpu.h"

U8* outBuffer;
U32 outBufferSize;
U32 outBufferPos;

// per instruction, not per block.  
// will allow us to determin if ecx or edx needs to be saved before calling an external function
// :TODO: optimize so that unnecessary push/pop of ecx/edx don't happen if the op is done with these regs
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
            outb(offsetof(CPU, reg[op->rm].u16));
        }

        // add ax, [cpu->reg[op->sibIndex].u16]
        if (op->sibIndex != 8) {
            outb(0x66);
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(offsetof(CPU, reg[op->sibIndex].u16));
        }

        // seg[6] is always 0
        if (op->base<6) { 
            // add eax, [cpu->seg[op->base].address]
            outb(0x03);
            outb(0x47 | (reg << 3));
            outb(offsetof(CPU, seg[op->base].address));
        }
    } else {
        // cpu->seg[op->base].address + cpu->reg[op->rm].u32 + (cpu->reg[op->sibIndex].u32 << + op->sibScale) + op->disp
        bool initiallized = false;

        if (op->sibIndex!=8) {
            initiallized = true;
            // mov eax, [cpu->reg[op->sibIndex].u32];
            outb(0x8b);
            outb(0x47 | (reg << 3));
            outb(offsetof(CPU, reg[op->sibIndex].u32));

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
            if (op->base<6) { 
                // add eax, [cpu->seg[op->base].address]
                outb(0x03);
                outb(0x47 | (reg << 3));
                outb(offsetof(CPU, seg[op->base].address));
            }
        } else {
            // seg[6] is always 0
            if (op->base<6) { 
                initiallized = true;
                // mov eax, [cpu->seg[op->base].address]
                outb(0x8b);
                outb(0x47 | (reg << 3));
                outb(offsetof(CPU, seg[op->base].address));
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
            outb(offsetof(CPU, reg[op->rm].u32));
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

void movToRegFromRegSignExtend(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth) {
    regUsed[dst] = true;
    if (dstWidth<=srcWidth) {
        movToRegFromReg(dst, dstWidth, src, srcWidth);
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
    }
}

void movToRegFromReg(DynReg dst, DynWidth dstWidth, DynReg src, DynWidth srcWidth) {
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
                outb(0xC0 | (src << 3) | dst);
            } else if (srcWidth==DYN_8bit) {
                outb(0x0f);
                outb(0xb6);
                outb(0xC0 | (src << 3) | dst);
            } else {
                kpanic("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }
        } else if (dstWidth==DYN_16bit) {
            if (srcWidth==DYN_8bit) {
                outb(0x66);
                outb(0x0f);
                outb(0xb6);
                outb(0xC0 | (src << 3) | dst);
            } else {
                kpanic("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
            }           
        } else {
            kpanic("unknown width in x32CPU::movToRegFromReg %d <= %d", dstWidth, srcWidth);
        }
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

void movToCpuFromReg(U32 dstOffset, DynReg reg, DynWidth width) {
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
}

void movToCpuFromCpu(U32 dstOffset, U32 srcOffset, DynWidth width, DynReg tmpReg) {
    // mov tmpReg, [cpu+srcOffset]
    movToRegFromCpu(tmpReg, srcOffset, width);

    // mov [cpu+dstOffset], tmpReg
    movToCpuFromReg(dstOffset, tmpReg, width);    
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
    outb(0xb8+reg);
    outd(imm);
}

void movFromMem(DynWidth width, DynReg addressReg) {
    regUsed[DYN_EAX] = true;

    // set EAX so don't push it then clobber the result with a pop

    if (regUsed[DYN_ECX])
        outb(0x51);
    if (regUsed[DYN_EDX])
        outb(0x52);

    // push addressReg
    outb(0x50+addressReg);

    // call read
    if (width == DYN_32bit) {
        outb(0xb8);
        outd((U32)readd);
        outb(0xff);
        outb(0xd0);
    } else if (width == DYN_16bit) {
        outb(0xb8);
        outd((U32)readw);
        outb(0xff);
        outb(0xd0);
    } else if (width == DYN_8bit) {
        outb(0xb8);
        outd((U32)readb);
        outb(0xff);
        outb(0xd0);
    } else {
        kpanic("unknown width in x32CPU::movFromMem %d", width);
    }

    // add esp, 4
    outb(0x83);
    outb(0xc4);
    outb(0x04);
    
    if (regUsed[DYN_EDX])
        outb(0x5a);
    if (regUsed[DYN_ECX])
        outb(0x59);
}

void movToCpuFromMem(U32 dstOffset, DynWidth dstWidth, DynReg addressReg) {
    movFromMem(dstWidth, addressReg);
    // mov [cpu+srcOffset], eax
    movToCpuFromReg(dstOffset, DYN_EAX, dstWidth);
}

void movToMem(DynReg addressReg, DynWidth width) {    
    // push addressReg
    outb(0x50+addressReg);

    // call write
    if (width == DYN_32bit) {
        outb(0xb8);
        outd((U32)writed);
        outb(0xff);
        outb(0xd0);
    } else if (width == DYN_16bit) {
        outb(0xb8);
        outd((U32)writew);
        outb(0xff);
        outb(0xd0);
    } else if (width == DYN_8bit) {
        outb(0xb8);
        outd((U32)writeb);
        outb(0xff);
        outb(0xd0);
    } else {
        kpanic("unknown width in x32CPU::movToMemFromReg %d", width);
    }    
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

void movToMemFromReg(DynReg addressReg, DynReg reg, DynWidth width) {
    if (regUsed[DYN_EAX])
        outb(0x50);
    if (regUsed[DYN_ECX])
        outb(0x51);
    if (regUsed[DYN_EDX])
        outb(0x52);

    // push reg
    if (width==DYN_8bit)
        pushValue(reg, DYN_PARAM_REG_8);
    else if (width==DYN_16bit)
        pushValue(reg, DYN_PARAM_REG_16);
    else if (width==DYN_32bit)
        pushValue(reg, DYN_PARAM_REG_32);
    else
        kpanic("unknown width %d in x32CPU::movToMemFromReg", width);

    movToMem(addressReg, width);

    // add esp, 8
    outb(0x83);
    outb(0xc4);
    outb(0x08);

    if (regUsed[DYN_EDX])
        outb(0x5a);
    if (regUsed[DYN_ECX])
        outb(0x59);
    if (regUsed[DYN_EAX])
        outb(0x58);    
}

void movToMemFromImm(DynReg addressReg, DynWidth width, U32 imm) {
    if (regUsed[DYN_EAX])
        outb(0x50);  
    if (regUsed[DYN_ECX])
        outb(0x51);
    if (regUsed[DYN_EDX])
        outb(0x52);

    // push imm
    outb(0x68);
    outd(imm);

    movToMem(addressReg, width);    

    // add esp, 8
    outb(0x83);
    outb(0xc4);
    outb(0x08);
    
    if (regUsed[DYN_EDX])
        outb(0x5a);
    if (regUsed[DYN_ECX])
        outb(0x59);
    if (regUsed[DYN_EAX])
        outb(0x58);

}

void callHostFunction(void* address, bool hasReturn, bool returnIfTrue, bool returnIfFalse, U32 argCount, U32 arg1, DynCallParamType arg1Type, U32 arg2, DynCallParamType arg2Type, U32 arg3, DynCallParamType arg3Type, U32 arg4, DynCallParamType arg4Type, U32 arg5, DynCallParamType arg5Type) {
    if (regUsed[DYN_EAX] && !hasReturn)
        outb(0x50);
    if (regUsed[DYN_ECX])
        outb(0x51);
    if (regUsed[DYN_EDX])
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

    // mov eax, address
    outb(0xb8);
    outd((U32)address);

    // call eax
    outb(0xff);
    outb(0xd0);

    // sub esp, 4*argCount
    if (argCount) {
        outb(0x83);
        outb(0xc4);
        outb(0x04*argCount);
    }
    if (regUsed[DYN_EDX])
        outb(0x5a);
    if (regUsed[DYN_ECX])
        outb(0x59);    
    if (regUsed[DYN_EAX] && !hasReturn)
        outb(0x58);

    if (hasReturn && (returnIfTrue || returnIfFalse)) {
        // test eax, eax
        outb(0x85);
        outb(0xc0);

        if (returnIfTrue) {
            outb(0x74); // jz
        } else if (returnIfFalse) {
            outb(0x75); // jnz
        }        
        outb(0x03); // jump 3, skip ret    
        outb(0x5f); // pop edi
        outb(0x5b); // pop ebx
        outb(0xc3); // ret, assumes we don't clean up stack (calling convention or fast call)
    }
}

// (useAddress)
//     if (condition) cpu->reg[op->reg].u32 = readd(eaa(cpu, op));
// else
//     if(condition) cpu->reg[op->reg].u32 = cpu->reg[op->rm].u32;
void movCC(void* condition, DecodedOp* op, DynWidth width, bool useAddress) {    
    if (useAddress)
        calculateEaa( op, DYN_ADDRESS);
    callHostFunction(condition, true, false, false, 1, 0, DYN_PARAM_CPU);

    // test eax, eax
    outb(0x85);
    outb(0xc0);
    outb(0x74); // jz, jump over if not true

    U32 pos = outBufferPos;
    outb(0); // jump over amount  

    if (useAddress) {
        movFromMem(width, DYN_ADDRESS);        
    } else {              

        // mov eax, cpu->reg[op->rm].u32
        if (width == DYN_32bit) {
            movToRegFromCpu(DYN_EAX, offsetof(CPU, reg[op->rm].u32), DYN_32bit);
        } else {
            movToRegFromCpu(DYN_EAX, offsetof(CPU, reg[op->rm].u16), DYN_16bit);
        }        
    }

    if (width == DYN_32bit) {
        movToCpuFromReg(offsetof(CPU, reg[op->reg].u32), DYN_EAX, DYN_32bit);
    } else {
        movToCpuFromReg(offsetof(CPU, reg[op->reg].u16), DYN_EAX, DYN_16bit);
    }

    outBuffer[pos] = (U8)(outBufferPos-pos);
}

void setCC(void* condition, DecodedOp* op, bool useAddress) {
    if (useAddress)
        calculateEaa( op, DYN_ADDRESS);
    callHostFunction(condition, true, false, false, 1, 0, DYN_PARAM_CPU);

    // test eax, eax
    outb(0x85);
    outb(0xc0);
    
    //setnz al
    outb(0x0f);
    outb(0x95);
    outb(0xc0);

    if (useAddress) {
        movToMemFromReg(DYN_ADDRESS, DYN_EAX, DYN_8bit);
    } else {    
        movToCpuFromReg( OFFSET_REG8(op->reg), DYN_EAX, DYN_8bit);
    }
}

// inst can be +, |, - , &, ^
void instRegImm(U32 inst, DynReg reg, DynWidth regWidth, U32 imm) {
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

// inst can be +, |, -, &, ^
void instRegReg(char inst, DynReg reg, DynReg rm, DynWidth regWidth) {
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
            kpanic("unhandled op in x32CPU::instRegReg %c", inst);
            break;
    }
    // add reg, imm
    if (regWidth==DYN_32bit) {            
        outb(i);
        outb(0xC0 | rm << 3 | reg);
    } else if (regWidth == DYN_16bit) {
        outb(0x66);
        outb(i);
        outb(0xC0 | rm << 3 | reg);
    } else if (regWidth == DYN_8bit) {
        outb(i-1);
        outb(0xC0 | rm << 3 | reg);
    } else {
        kpanic("unknown regWidth in x32CPU::instRegImm + %d", regWidth);
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

void OPCALL x32_sidt(CPU* cpu, DecodedOp* op) {
}

void OPCALL x32_invalid_op(CPU* cpu, DecodedOp* op) {
    kpanic("Invalid instruction %x\n", op->inst);
}

static OpCallback x32Ops[NUMBER_OF_OPS];
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
    x32Ops[VERRE16] = 0;
    x32Ops[VERWR16] = 0; 
    x32Ops[VERWE16] = 0;
    x32Ops[SGDT] = 0;
    x32Ops[SIDT] = x32_sidt;
    x32Ops[LGDT] = 0;
    x32Ops[LIDT] = 0;
    x32Ops[SMSWRreg] = 0; 
    x32Ops[SMSW] = 0;
    x32Ops[LMSWRreg] = 0; 
    x32Ops[LMSW] = 0;
    x32Ops[INVLPG] = 0;
    x32Ops[Callback] = 0;
}

void OPCALL firstX32Op(CPU* cpu, DecodedOp* op) {
#ifdef __TEST
    if (DecodedBlock::currentBlock->runCount == 0) {
#else
    if (DecodedBlock::currentBlock->runCount == 50) {
#endif
        initX32Ops();
        DecodedOp* o = op->next;
        outBufferPos = 0;
        outb(0x53); // push ebx
        outb(0x57); // push edi , will hold cpu
        // on win32 cx contains cpu
        // mov edi, ecx
        outb(0x89);
        outb(0xcf);
        movToCpu(offsetof(CPU, nextBlock), DYN_32bit, 0);
        while (o) {
            memset(regUsed, 0, sizeof(regUsed));
#ifndef __TEST
#ifdef _DEBUG
            callHostFunction(common_log, false, false, false, 2, 0, DYN_PARAM_CPU, (DYN_PTR_SIZE)o, DYN_PARAM_CONST_PTR);
#endif
#endif
            x32Ops[o->inst](cpu, o);            
            o = o->next;
        }
        outb(0x5f); // pop edi
        outb(0x5b); // pop ebx
        outb(0xc3); // ret
        Memory* memory = cpu->thread->process->memory;
        void* mem = NULL;

        if (memory->dynamicExecutableMemory.size()==0) {
            mem = allocExecutable64kBlock();
            memory->dynamicExecutableMemoryPos = 0;
            memory->dynamicExecutableMemory.push_back(mem);
        } else {
            mem = memory->dynamicExecutableMemory[memory->dynamicExecutableMemory.size()-1];
            if (memory->dynamicExecutableMemoryPos+outBufferPos>=64*1024) {
                mem = allocExecutable64kBlock();
                memory->dynamicExecutableMemoryPos = 0;
                memory->dynamicExecutableMemory.push_back(mem);
            }
        }
        void* begin = (U8*)mem+memory->dynamicExecutableMemoryPos;
        memcpy(begin, outBuffer, outBufferPos);
        memory->dynamicExecutableMemoryPos+=outBufferPos;
        

        bool b= false;
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