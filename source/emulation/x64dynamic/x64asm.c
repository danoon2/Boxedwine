#ifdef BOXEDWINE_VM

#include "x64.h"
#include "cpu.h"
#include "kalloc.h"
#include "kthread.h"
#include "kprocess.h"
#include "kscheduler.h"
#include "decoder.h"
#include "../cpu/strings.h"
#include "../hardmmu/hard_memory.h"

#define REX_BASE 0x40
#define REX_MOD_RM 0x1
#define REX_SIB_INDEX 0x2
#define REX_MOD_REG 0x4
#define REX_64 0x8

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)

static void write8(struct x64_Data* data, U8 value) {
    if (!data->availableMem) {
        struct Memory* memory = data->cpu->memory;
        allocExecutable64kBlock(memory);
        data->availableMem = 64*1024;
    }
    data->memStart[data->memPos++] = value;
    data->availableMem--;
}

static void write16(struct x64_Data* data, U16 value) {
    write8(data, (U8)value);
    write8(data, (U8)(value >> 8));
}

static void write32(struct x64_Data* data, U32 value) {
    write8(data, value);
    write8(data, value >> 8);
    write8(data, value >> 16);
    write8(data, value >> 24);
}

static void write64(struct x64_Data* data, U64 value) {
    write8(data, (U8)value);
    write8(data, (U8)(value >> 8));
    write8(data, (U8)(value >> 16));
    write8(data, (U8)(value >> 24));
    write8(data, (U8)(value >> 32));
    write8(data, (U8)(value >> 40));
    write8(data, (U8)(value >> 48));
    write8(data, (U8)(value >> 56));
}

void x64_mapAddressInternal(struct x64_Data* data, U32 ip, void* address) {
    void** table = data->cpu->opToAddressPages[ip >> PAGE_SHIFT];
    U32* eipTable = data->cpu->thread->process->hostToEip[((U32)address) >> PAGE_SHIFT];

    if (!table) {
        table = kalloc(sizeof(void*)*PAGE_SIZE, KALLOC_IP_CACHE);
        data->cpu->opToAddressPages[ip >> PAGE_SHIFT] = table;
    }
    table[ip & PAGE_MASK] = address;

    if (!eipTable) {
        eipTable = kalloc(sizeof(U32)*PAGE_SIZE, KALLOC_IP_CACHE);
        data->cpu->thread->process->hostToEip[((U32)address) >> PAGE_SHIFT] = eipTable;
    }
    eipTable[((U32)address) & PAGE_MASK] = ip;
}

void x64_commitMappedAddresses(struct x64_Data* data) {
    S32 i;
    S32 pageCount = (data->ip-data->start+PAGE_MASK) >> PAGE_SHIFT;
#ifndef __TEST
    for (i=0;i<pageCount;i++) {
        makeCodePageReadOnly(data->cpu->memory, ((data->start+data->cpu->segAddress[CS])>>PAGE_SHIFT)+i);
    }
#endif
    for (i=data->ipAddressCount-1;i>=0;i--) {
        x64_mapAddressInternal(data, data->ipAddress[i], data->hostAddress[i]);
    }
    if (data->ipAddress!=data->ipAddressBuffer) {
        free(data->ipAddress);
        data->ipAddress = data->ipAddressBuffer;
    }
    if (data->hostAddress!=data->hostAddressBuffer) {
        free(data->hostAddress);
        data->hostAddress = data->hostAddressBuffer;
    }
    data->ipAddressCount = 0;
    data->ipAddressBufferSize = sizeof(data->ipAddressBuffer)/sizeof(data->ipAddressBuffer[0]);
}

void x64_mapAddress(struct x64_Data* data, U32 ip, void* address) {
    if (data->ipAddressCount>=data->ipAddressBufferSize) {
        U32* ipAddressOld = data->ipAddress;
        void** hostAddressOld = data->hostAddress;

        data->ipAddress = malloc(sizeof(U32)*data->ipAddressBufferSize*2);
        data->hostAddress = malloc(sizeof(void*)*data->ipAddressBufferSize*2);

        memcpy(data->ipAddress, ipAddressOld, sizeof(U32)*data->ipAddressBufferSize);
        memcpy(data->hostAddress, hostAddressOld, sizeof(void*)*data->ipAddressBufferSize);

        data->ipAddressBufferSize*=2;
        if (ipAddressOld!=data->ipAddressBuffer) {
            free(ipAddressOld);
        }
        if (hostAddressOld!=data->hostAddressBuffer) {
            free(hostAddressOld);
        }
    }
    data->ipAddress[data->ipAddressCount] = ip;
    data->hostAddress[data->ipAddressCount++] = address;        
}

void x64_writeOp(struct x64_Data* data) {
    x64_writeOp8(data, FALSE, FALSE);
}

void x64_writeOp8(struct x64_Data* data, BOOL isG8bit, BOOL isGWritten) {
    U32 g8 = 0;
    U32 g8Temp = 0;
    U32 tmpReg = x64_getTmpReg(data);

    if (data->rex && data->has_rm && isG8bit && G(data->rm)>=4) {        
        g8=G(data->rm);
        g8Temp = (g8==4?1:0);

        // push rax
        x64_pushNative(data, g8Temp, FALSE);

        // mov al, g8
        x64_writeToRegFromReg(data, g8Temp, FALSE, g8, FALSE, 1);

        // mov HOST_TMP2, eax
        x64_writeToRegFromReg(data, tmpReg, TRUE, g8Temp, FALSE, 4);

        // pop rax
        x64_popNative(data, g8Temp, FALSE);

        data->rex |= REX_BASE | REX_MOD_REG;
        data->rm &=~ (0x7 << 3);
        data->rm |= (tmpReg << 3);        
    }

    if (data->lockPrefix)
        write8(data, 0xF0);
    if (data->cpu->big && data->operandPrefix)
        write8(data, 0x66);
    else if (!data->cpu->big && !data->operandPrefix)
        write8(data, 0x66);
    if (data->repZeroPrefix)
        write8(data, 0xF3);
    if (data->repNotZeroPrefix)
        write8(data, 0xF2);

    if (data->rex)
        write8(data, data->rex);           
    
    if (data->multiBytePrefix)
        write8(data, 0x0F);

    write8(data, data->op);
    if (data->has_rm) {
        write8(data, data->rm);
    }
    if (data->has_sib) {
        write8(data, data->sib);
    }
    if (data->has_disp8) {
        write8(data, data->disp8);
    } else if (data, data->has_disp16) {
        write16(data, data->disp16);
    } else if (data, data->has_disp32) {
        write32(data, data->disp32);
    }
    if (data->has_im8) {
        write8(data, data->im8);
    } else if (data, data->has_im16) {
        write16(data, data->im16);
    } else if (data, data->has_im32) {
        write32(data, data->im32);
    }
    if (isGWritten && g8) {
        // push rax
        x64_pushNative(data, g8Temp, FALSE);

        // mov eax, HOST_TMP2
        x64_writeToRegFromReg(data, g8Temp, FALSE, tmpReg, TRUE, 4);

        // mov g8, al
        x64_writeToRegFromReg(data, g8, FALSE, g8Temp, FALSE, 1);

        // pop rax
        x64_popNative(data, g8Temp, FALSE);
    }
    x64_releaseTmpReg(data, tmpReg);
}

void x64_setRM(struct x64_Data* data, U8 rm, BOOL checkG, BOOL checkE, U32 isG8bit, U32 isE8bit) {
    if (checkG && G(rm) == 4 && !isG8bit) {
        data->rex |= REX_BASE | REX_MOD_REG;
        rm = (rm & ~0x38) | (HOST_ESP << 3);
        if (checkE && E(rm)>=4 && isE8bit) {
            kpanic("x64_setRM unhandled E");
        }
    }
    if (checkE && E(rm)== 4 && !isE8bit) {
        data->rex |= REX_BASE | REX_MOD_RM;
        rm = (rm & ~0x07) | HOST_ESP;
        if (checkG && G(rm)>=4 && isG8bit) {
            kpanic("x64_setRM unhandled G");
        }
    }
    data->has_rm = 1;
    data->rm = rm;
}

void x64_setSib(struct x64_Data* data, U8 sib, BOOL checkBase) {
    if (checkBase && (sib & 7)==4) {
        data->rex |= REX_BASE | REX_MOD_RM;
        sib = (sib & ~7) | HOST_ESP;
    }
    // will never convert index from ESP to HOST_ESP, because ESP indicates a 0 value
    /*
    if (checkIndex && ((sib >> 3) & 7)==4) {
        data->rex |= REX_BASE | REX_SIB_INDEX;
        sib = (sib & ~0x38) | (HOST_ESP << 3);
    }
    */
    data->has_sib = 1;
    data->sib = sib;
}

void x64_setDisplacement32(struct x64_Data* data, U32 disp32) {
    data->has_disp32 = 1;
    data->disp32 = disp32;
}

void x64_setDisplacement8(struct x64_Data* data, U8 disp8) {
    data->has_disp8 = 1;
    data->disp8 = disp8;
}

void x64_setImmediate8(struct x64_Data* data, U8 value) {
    data->has_im8 = 1;
    data->im8 = value;
}

void x64_setImmediate16(struct x64_Data* data, U16 value) {
    data->has_im16 = 1;
    data->im16 = value;
}

void x64_setImmediate32(struct x64_Data* data, U32 value) {
    data->has_im32 = 1;
    data->im32 = value;
}

U32 x64_getRegForSeg(struct x64_Data* data, U32 base, U32 tmpReg) {
    if (base == ES) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ES_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == SS) {return HOST_SS;}
    if (base == GS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_GS_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == FS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_FS_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == DS) {return HOST_DS;}
    if (base == CS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_CS_ADDRESS, 4, FALSE); return tmpReg;}
    kpanic("unknown base in x64dynamic.c getRegForSeg");
    return 0;
}

U32 x64_getRegForNegSeg(struct x64_Data* data, U32 base, U32 tmpReg) {
    if (base == ES) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ES_NEG_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == SS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_SS_NEG_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == DS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_DS_NEG_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == FS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_FS_NEG_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == GS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_GS_NEG_ADDRESS, 4, FALSE); return tmpReg;}
    if (base == CS) {x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_CS_NEG_ADDRESS, 4, FALSE); return tmpReg;}
    kpanic("unknown base in x64dynamic.c getRegForNegSeg");
    return 0;
}

#define SWAP_U32(x, y) {U32 t=y;y=x;x=t;}

static void doMemoryInstruction(U8 op, struct x64_Data* data, U32 reg1, U32 isReg1Rex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes) {
    U32 oneByteDisplacement = (displacement>=-128 && displacement<=127);
    U8 rex = 0;
    U8 rm = 0;
    U8 displacementBytes = 0;

    if (displacement) {
        if (oneByteDisplacement)
            displacementBytes = 1;
        else
            displacementBytes = 4;
    }

    if (reg1==4 && !isReg1Rex) {
        reg1 = HOST_ESP;
        isReg1Rex = TRUE;
    }

    if (reg2==4 && !isReg2Rex) {
        reg2 = HOST_ESP;
        isReg2Rex = TRUE;
    }

    if (reg3==4 && !isReg3Rex) {
        reg3 = HOST_ESP;
        isReg3Rex = TRUE;
    }

    if (reg3==4 && reg2!=4 && reg3Shift==0) {
        // sp as an index is replaced with a value of 0
        SWAP_U32(reg2, reg3);
        SWAP_U32(isReg2Rex, isReg3Rex);
    } else if (reg2==5 && reg3!=-1 && reg3!=5 && reg3Shift==0) {
        // sib0, [ebp+reg<<shift] does not exist
        SWAP_U32(reg2, reg3);
        SWAP_U32(isReg2Rex, isReg3Rex);
    }

    if (isReg1Rex)
        rex |= REX_BASE | REX_MOD_REG;
    if (isReg2Rex)
        rex |= REX_BASE | REX_MOD_RM;
    if (isReg3Rex)
        rex |= REX_BASE | REX_SIB_INDEX;
    if (bytes == 8)
        rex |= REX_64;
    if (bytes == 2) 
        write8(data, 0x66);
    if (rex)
        write8(data, rex);
    write8(data, op);

    rm|=reg1 << 3;
    if (reg2!=4 && reg3==-1) {        
        if (reg2==5 && displacement==0) {            
            displacementBytes = 1; // [EBP] is not valid, it is reserved for [EIP+disp32], so change it to [EBP+0]
        }         
        if (displacementBytes==1)
            rm|=0x40;
        else if (displacementBytes==4)
            rm|=0x80;        
        rm|=reg2;
        write8(data, rm);
    } else {
        U8 sib = 0;

        if (reg2==4 && reg3==4) {
            kpanic("Wasn't expecting [ESP+ESP*n] memory access");
        }        
        
        if (displacementBytes==0 && reg2==5) {
            // sib0, [ebp+reg<<shift] does not exist, replace with sib1, [ebp+reg<<shift+0]
            displacementBytes=1;
        }
              
        if (displacementBytes==1)
            rm|=0x40;
        else if (displacementBytes==4)
            rm|=0x80;
        rm|=0x4; // sib

        if (reg3==-1) {
            reg3=4; // index=ESP, the value will always be 0
        }
                
        write8(data, rm);
        write8(data, (reg3Shift<<6) | (reg3 << 3) | reg2); // sib
    }

    if (reg1==HOST_ESP && isReg1Rex && bytes == 2) {
        // :TODO: should top 16-bits be cleared?
    }
       
    if (displacementBytes==1)
        write8(data, (U8)displacement);
    else if (displacementBytes==4)
        write32(data, (U32)displacement);
}

void x64_zeroReg(struct x64_Data* data, U32 reg, BOOL isRexReg) {
    // xor would use less bytes, but it changes the flags
    if (isRexReg)
        write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0xb8 + reg);
    write32(data, 0);
}

// reg1 = reg2 + reg3<<shift (can be 0,1,2 or 3) + displacement
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void x64_addWithLea(struct x64_Data* data, U32 reg1, U32 isReg1Rex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes) {
    doMemoryInstruction(0x8d, data, reg1, isReg1Rex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
}


// [reg2 + (reg3 << reg3Shift) + displacement] = reg1
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void x64_writeToMemFromReg(struct x64_Data* data, U32 reg1, U32 isReg1Rex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes, U32 translateToHost) {
    if (translateToHost) {
        U32 tmpReg = x64_getTmpReg(data);
        x64_addWithLea(data, tmpReg, TRUE, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
        x64_writeToMemFromReg(data, reg1, isReg1Rex, tmpReg, TRUE, HOST_MEM, TRUE, 0, 0, bytes, FALSE);
        x64_releaseTmpReg(data, tmpReg);
    } else {
        // reg 1 will be ignored
        doMemoryInstruction(bytes==1?0x88:0x89, data, reg1, isReg1Rex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
    }
}

void x64_writeToMemFromValue(struct x64_Data* data, U64 value, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes, U32 translateToHost) {
    if (translateToHost) {
        U32 tmpReg = x64_getTmpReg(data);
        x64_addWithLea(data, tmpReg, TRUE, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
        x64_writeToMemFromValue(data, value, tmpReg, TRUE, HOST_MEM, TRUE, 0, 0, bytes, FALSE);
        x64_releaseTmpReg(data, tmpReg);
    } else {
        // reg 1 will be ignored
        doMemoryInstruction(bytes==1?0xc6:0xc7, data, 0, FALSE, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
        if (bytes == 1)
            write8(data, (U8)value);
        else if (bytes == 2)
            write16(data, (U16)value);
        else if (bytes == 4) 
            write32(data, (U32)value);
        else if (bytes == 8)
            kpanic("writeToMemFromValue 64-bit not implemented");
    }
}

// reg1 = [reg2 + (reg3 << reg3Shift) + displacement]
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void x64_writeToRegFromMem(struct x64_Data* data, U32 toReg, U32 isToRegRex, U32 reg2, U32 isReg2Rex, S32 reg3, U32 isReg3Rex, U32 reg3Shift, S32 displacement, U32 bytes, U32 translateToHost) {
    if (translateToHost) {
        U32 tmpReg = x64_getTmpReg(data);
        x64_addWithLea(data, tmpReg, TRUE, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
        x64_writeToRegFromMem(data, toReg, isToRegRex, tmpReg, TRUE, HOST_MEM, TRUE, 0, 0, bytes, FALSE);
        x64_releaseTmpReg(data, tmpReg);
    } else {
        doMemoryInstruction(bytes==1?0x8a:0x8b, data, toReg, isToRegRex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
    }
}

void x64_writeToRegFromValue(struct x64_Data* data, U32 reg, U32 isRexReg, U64 value, U32 bytes) {
    U8 rex = 0;

    if (reg==4 && !isRexReg) {
        reg = HOST_ESP;
        isRexReg = TRUE;
    }
    if (isRexReg)
        rex = REX_BASE | REX_MOD_RM;
    if (bytes==8)
        rex |= REX_BASE | REX_64;

    if (bytes==2)
        write8(data, 0x66);
    if (rex)
        write8(data, rex);
    if (bytes==1) {
        write8(data, 0xb0+reg);
    } else {
        write8(data, 0xb8+reg);
    }
    if (bytes == 1) {
        write8(data, (U8)value);
    } else if (bytes == 2) {
        write16(data, (U16)value);
    } else if (bytes == 4) {
        write32(data, (U32)value);
    } else if (bytes == 8) {
        write64(data, value);
    }
}

void x64_writeToRegFromReg(struct x64_Data* data, U32 toReg, U32 isToReg1Rex, U32 fromReg, U32 isFromRegRex, U32 bytes) {
    U8 rex = 0;

    if (toReg==4 && !isToReg1Rex && bytes!=1) {
        toReg = HOST_ESP;
        isToReg1Rex = TRUE;
    }
    if (fromReg==4 && !isFromRegRex && bytes!=1) {
        fromReg = HOST_ESP;
        isFromRegRex = TRUE;
    }

    if (isToReg1Rex)
        rex = REX_BASE | REX_MOD_RM;
    if (isFromRegRex)
        rex |= REX_BASE | REX_MOD_REG;
    if (bytes==8)
        rex |= REX_BASE | REX_64;

    if (bytes == 2)
        write8(data, 0x66);
    if (rex)
        write8(data, rex);
    if (bytes == 1) 
        write8(data, 0x88);
    else
        write8(data, 0x89);
    write8(data, 0xC0 | toReg | (fromReg << 3));
}

static void shiftRightReg(struct x64_Data* data, U32 reg, U32 isRegRex, U32 shiftAmount) {
    if (isRegRex)
        write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0xC1);
    write8(data, 0xE8 | reg);
    write8(data, shiftAmount);
}

static void andReg(struct x64_Data* data, U32 reg, U32 isRegRex, U32 mask) {
    if (isRegRex)
        write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0x81);
    write8(data, 0xE0 | reg);
    write32(data, mask);
}

void x64_pushNative(struct x64_Data* data, U32 reg, U32 isRegRex) {
    if (isRegRex)
        write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0x50+reg);
}

void x64_pushNativeFlags(struct x64_Data* data) {
    write8(data, 0x9c);
}

void x64_popNative(struct x64_Data* data, U32 reg, U32 isRegRex) {
    if (isRegRex)
        write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0x58+reg);
}

void x64_popNativeFlags(struct x64_Data* data) {
    write8(data, 0x9d);
}

static void jmpNativeReg(struct x64_Data* data, U32 reg, U32 isRegRex) {
    if (isRegRex)
        write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0xff);
    write8(data, (0x04 << 3) | 0xC0 | reg);
}

void ksyscall(struct CPU* cpu, U32 eipCount);
void cpuid(struct CPU* cpu);
extern Int99Callback* wine_callback;
extern U32 wine_callbackSize;

static SDL_mutex* printMutex;
void OPCALL stosd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsd32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL stosb32_r_op(struct CPU* cpu, struct Op* op);
void OPCALL movsb32_r_op(struct CPU* cpu, struct Op* op);

void log_pf(struct KThread* thread, U32 address);
void OPCALL firstOp(struct CPU* cpu, struct Op* op);

void x64_cmdEntry(struct CPU* cpu) {
#ifdef _DEBUG
    if (cpu->lastRSP==0) {
        cpu->lastRSP = cpu->rsp;
    } else if (cpu->lastRSP!=cpu->rsp) {
        klog("x64asm: RSP was modified: %s at %.8X\n", getModuleName(cpu, cpu->segAddress[CS]+cpu->eip.u32), getModuleEip(cpu, cpu->segAddress[CS]+cpu->eip.u32));
    }
#endif
    //klog("%d/%d cmdEntry: cmd=%d eax=%X", cpu->thread->process->id, cpu->thread->id, cpu->cmd, EAX);
    switch (cpu->cmd) {
    case CMD_PRINT: 
#ifdef LOG_OPS
        if (cpu->log) {
            if (!printMutex) {
                printMutex = SDL_CreateMutex();
            }
            BOXEDWINE_LOCK(cpu->thread, printMutex);
            if (!cpu->logFile) {
                char buffer[MAX_FILEPATH_LEN];
                sprintf(buffer, "log_%d_%d.txt", cpu->thread->process->id, cpu->thread->id);
                cpu->logFile = fopen(buffer, "w");
            }
            fprintf(cpu->logFile, "%.08X/%.06X EAX=%.08X ECX=%.08X EDX=%.08X EBX=%.08X ESP=%.08X EBP=%.08X ESI=%.08X EDI=%.08X\n", cpu->eip.u32, cpu->cmdArg, EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI); 
            fflush(cpu->logFile);
            BOXEDWINE_UNLOCK(cpu->thread, printMutex);            
        }
#endif
        return; // don't run signal because of this
    case CMD_SET_ES:
    case CMD_SET_CS:
    case CMD_SET_SS:
    case CMD_SET_DS:
    case CMD_SET_FS:
    case CMD_SET_GS:
        if (cpu_setSegment(cpu, cpu->cmd - CMD_SET_ES, cpu->cmdArg))
            cpu->eip.u32+=cpu->cmdEipCount;
        return; // don't run signal because of this
    case CMD_POP16_ES:
    case CMD_POP16_CS:
    case CMD_POP16_SS:
    case CMD_POP16_DS:
    case CMD_POP16_FS:
    case CMD_POP16_GS:
        if (cpu_setSegment(cpu, cpu->cmd - CMD_POP16_ES, peek16(cpu, 0))) {
            ESP = (ESP & cpu->stackNotMask) | ((ESP + 2 ) & cpu->stackMask);
            cpu->eip.u32+=cpu->cmdEipCount;
        }
        return; // don't run signal because of this
    case CMD_POP32_ES:
    case CMD_POP32_CS:
    case CMD_POP32_SS:
    case CMD_POP32_DS:
    case CMD_POP32_FS:
    case CMD_POP32_GS:
        if (cpu_setSegment(cpu, cpu->cmd - CMD_POP32_ES, peek32(cpu, 0))) {
            ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);
            cpu->eip.u32+=cpu->cmdEipCount;
        }
        return; // don't run signal because of this
    case CMD_LOAD_ES:
    case CMD_LOAD_CS:
    case CMD_LOAD_SS:
    case CMD_LOAD_DS:
    case CMD_LOAD_FS:
    case CMD_LOAD_GS:
        if (cpu_setSegment(cpu, cpu->cmd - CMD_LOAD_ES, cpu->cmdArg>>16)) {
            cpu->reg[cpu->cmdArg2].u16 = (U16)cpu->cmdArg;
            cpu->eip.u32+=cpu->cmdEipCount;
        }
        return; // don't run signal because of this
    case CMD_CALL_16:
        cpu_call(cpu, 0, cpu->cmdArg >> 16, cpu->cmdArg & 0xFFFF, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_JUMP_16:
        cpu_jmp(cpu, 0, cpu->cmdArg >> 16, cpu->cmdArg & 0xFFFF, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_CALL_FAR:
        cpu_call(cpu, 1, cpu->cmdArg2, cpu->cmdArg, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_RETF_16:
        cpu_ret(cpu, 0, 0, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_RETF_32:
        cpu_ret(cpu, 1, 0, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_RETFIw_16:
        cpu_ret(cpu, 0, cpu->cmdArg, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_RETFIw_32:
        cpu_ret(cpu, 1, cpu->cmdArg, cpu->eip.u32 + cpu->cmdEipCount);
        break;
    case CMD_IRET32:
        cpu_iret(cpu, 1, cpu->eip.u32 + cpu->cmdEipCount);
        return;
    case CMD_IRET16:
        cpu_iret(cpu, 0, cpu->eip.u32 + cpu->cmdEipCount);
        return;
    case CMD_EXIT_SIGNAL:
        onExitSignal(cpu, 0);
        return;
    case CMD_SYSCALL:
    {
        jmp_buf jmpBuf;
        if (!setjmp(jmpBuf)) {
            cpu->thread->syscallJmpBuf = &jmpBuf;
            ksyscall(cpu, cpu->cmdEipCount);        
        }
    }
        break;
    case CMD_CPUID:
        cpuid(cpu);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_ENTER32:
        cpu_enter32(cpu, cpu->cmdArg, cpu->cmdArg2);
        break;
    case CMD_ENTER16:
        cpu_enter16(cpu, cpu->cmdArg, cpu->cmdArg2);
        break;
    case CMD_INVALID_OP:
        kpanic("invalid op: %X", cpu->cmdArg);
        break;
    case CMD_INT:
        signalIllegalInstruction(cpu->thread, 5);// 5=ILL_PRVOPC  // :TODO: just a guess
        return;
    case CMD_LAR:
        cpu->reg[cpu->cmdArg2].u16 = cpu_lar(cpu, cpu->cmdArg, cpu->reg[cpu->cmdArg2].u16);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_LSL:
        cpu->reg[cpu->cmdArg2].u16 = cpu_lsl(cpu, cpu->cmdArg, cpu->reg[cpu->cmdArg2].u16);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_MOVSB:
        if (cpu->cmdArg2)
            movsb16_r(cpu, cpu->cmdArg);
        else
            movsb16(cpu, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_MOVSW:
        if (cpu->cmdArg2)
            movsw16_r(cpu, cpu->cmdArg);
        else
            movsw16(cpu, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_MOVSD:
        if (cpu->cmdArg2)
            movsd16_r(cpu, cpu->cmdArg);
        else
            movsd16(cpu, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_CMPSB:
        if (cpu->cmdArg2 & 0x1)
            cmpsb16_r(cpu, 1, cpu->cmdArg);
        else if (cpu->cmdArg2)
            cmpsb16_r(cpu, 0, cpu->cmdArg);
        else
            cmpsb16(cpu, 0, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_CMPSW:
        if (cpu->cmdArg2 & 0x1)
            cmpsw16_r(cpu, 1, cpu->cmdArg);
        else if (cpu->cmdArg2)
            cmpsw16_r(cpu, 0, cpu->cmdArg);
        else
            cmpsw16(cpu, 0, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_CMPSD:
        if (cpu->cmdArg2 & 0x1)
            cmpsd16_r(cpu, 1, cpu->cmdArg);
        else if (cpu->cmdArg2)
            cmpsd16_r(cpu, 0, cpu->cmdArg);
        else
            cmpsd16(cpu, 0, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_STOSB:
        if (cpu->cmdArg2)
            stosb16_r(cpu);
        else
            stosb16(cpu);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_STOSW:
        if (cpu->cmdArg2)
            stosw16_r(cpu);
        else
            stosw16(cpu);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_STOSD:
        if (cpu->cmdArg2)
            stosd16_r(cpu);
        else
            stosd16(cpu);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_SCASB:
        if (cpu->cmdArg2 & 0x1)
            scasb16_r(cpu, 1);
        else if (cpu->cmdArg2)
            scasb16_r(cpu, 0);
        else
            scasb16(cpu, 0);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_SCASW:
        if (cpu->cmdArg2 & 0x1)
            scasw16_r(cpu, 1);
        else if (cpu->cmdArg2)
            scasw16_r(cpu, 0);
        else
            scasw16(cpu, 0);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_SCASD:
        if (cpu->cmdArg2 & 0x1)
            scasd16_r(cpu, 1);
        else if (cpu->cmdArg2)
            scasd16_r(cpu, 0);
        else
            scasd16(cpu, 0);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_LODSB:
        if (cpu->cmdArg2)
            lodsb16_r(cpu, cpu->cmdArg);
        else
            lodsb16(cpu, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_LODSW:
        if (cpu->cmdArg2)
            lodsw16_r(cpu, cpu->cmdArg);
        else
            lodsw16(cpu, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_LODSD:
        if (cpu->cmdArg2)
            lodsd16_r(cpu, cpu->cmdArg);
        else
            lodsd16(cpu, cpu->cmdArg);
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    case CMD_WINE:
    {
        U32 index = peek32(cpu, 0);
        if (index<wine_callbackSize && wine_callback[index]) {
            wine_callback[index](cpu);
        }
        else {
            kpanic("Uknown int 98 call: %d", index);
        }
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    }
    case CMD_OPENGL:
    {
        U32 index = peek32(cpu, 0);

        if (index<int99CallbackSize && int99Callback[index]) {
            int99Callback[index](cpu);
        } else {
            kpanic("Uknown int 99 call: %d", index);
        }
        cpu->eip.u32+=cpu->cmdEipCount;
        break;
    }
    case CMD_SELF_MODIFYING: {
        SDL_LockMutex(cpu->memory->executableMemoryMutex);
        {
            struct Block* block = decodeSingleOp(cpu, cpu->eip.u32);
            struct Op* op = block->ops;
            U32 page;
            U32 pageCount = 1;
            U32 address;
            U32 bytes;
            U32 i;
            U8 wasReadOnly[256];
            U32 targetEip = 0;
            U32 targetFound = FALSE;

            cpu->lazyFlags = FLAGS_NONE;
            if (op->getWriteAddress) {
                address = op->getWriteAddress(cpu, op);
                bytes = op->getWriteBytes(cpu, op);
            } else if (op->func==stosd32_r_op || op->func==movsd32_r_op) {           
                cpu->df=1-((cpu->flags & DF) >> 9);
                address = cpu->segAddress[ES]+EDI;
                if (cpu->df<0)
                    address-=4*(ECX-1);
                bytes = 4*ECX;
            } else if (op->func==stosb32_r_op || op->func==movsb32_r_op) {           
                cpu->df=1-((cpu->flags & DF) >> 9);
                address = cpu->segAddress[ES]+EDI;
                if (cpu->df<0)
                    address-=(ECX-1);
                bytes = ECX;
            } else {
                block = decodeBlock(cpu, cpu->eip.u32);
                log_pf(cpu->thread, cpu->eip.u32);
                kpanic("unhandled self-modifying instruction: %X", op->inst);
            }
            page = address >> PAGE_SHIFT;
            pageCount = ((address+bytes-1) >> PAGE_SHIFT) - page + 1;
            for (i=0;i<pageCount;i++) {
                wasReadOnly[i]=clearCodePageReadOnly(cpu->memory, page+i);
            }

            op->func(cpu, op);
            freeBlock(block);
            for (i=0;i<pageCount;i++) {
                if (wasReadOnly[i])
                    makeCodePageReadOnly(cpu->memory, page+i);
            }
            fillFlags(cpu);        
            targetEip = address;
            for (i=0;i<16;i++) {
                if (cpu->thread->process->opToAddressPages[targetEip>>PAGE_SHIFT] && cpu->thread->process->opToAddressPages[targetEip>>PAGE_SHIFT][targetEip & PAGE_MASK]) {
                    targetFound = TRUE;
                    break;
                }
                targetEip--;
            }
            if (!targetFound) {
                targetEip = address+1; // +1 because we already checked adddress above
                for (i=1;i<bytes;i++) {
                    if (cpu->thread->process->opToAddressPages[targetEip>>PAGE_SHIFT] && cpu->thread->process->opToAddressPages[targetEip>>PAGE_SHIFT][targetEip & PAGE_MASK]) {
                        targetFound = TRUE;
                        break;
                    }
                    targetEip++;
                }
            }
            if (targetFound) {            
                struct Block* block = decodeBlock(cpu, targetEip);
                struct Op* op = block->ops->next;
                // is the address actually used in an instruction
                while (op && targetEip+op->eipCount>address && targetEip<address+bytes) {
                    struct x64_Data data;
                    U32 nextTargetEip = targetEip+op->eipCount;
                    U32 targetLen = 0;
                    U64 hostAddress = (U64)cpu->thread->process->opToAddressPages[targetEip>>PAGE_SHIFT][targetEip & PAGE_MASK];
                    U64 nextHostAddress = (U64)cpu->thread->process->opToAddressPages[nextTargetEip>>PAGE_SHIFT][nextTargetEip & PAGE_MASK];

                    if (nextHostAddress) {
                        targetLen = (U32)(nextHostAddress-hostAddress);
                    }
                    x64_initData(&data, cpu, targetEip);
                    data.memStart = (char*)hostAddress;

                    x64_translateInstruction(&data);
                    x64_link(&data);
                    if (targetLen && data.memPos>targetLen) {
                        kpanic("x64: couldn't patch self modifying code");
                    }
                    for (i=data.memPos;i<targetLen;i++) {
                        ((char*)hostAddress)[i]=0x90; // nop
                    }
                    // look to see of any other instructions where affected by the bytes that were changed
                    targetEip+=op->eipCount;
                    op = op->next;
                }
                freeBlock(block);
            }
        // :TODO: check if code exists at addresses
        }
        SDL_UnlockMutex(cpu->memory->executableMemoryMutex);
        // eip adjusted when op was run
        // cpu->eip.u32+=cpu->cmdEipCount;
    }
        break;
    default:
        kpanic("Unknow x64dynamic cmd: %d", cpu->cmd);
        break;
    }
    if (cpu->done && !cpu->inException) {
        longjmp(*cpu->jmpBuf, 0);
    }
    if (cpu->thread->pendingSignals || cpu->thread->process->pendingSignals) {        
        runSignals(cpu->thread, cpu->thread);
    }
}

void x64_writeInt(struct x64_Data* data) {
    write8(data, 0xcd);
    write8(data, 0x53);   
}

void x64_writeCmd(struct x64_Data* data, U32 cmd, U32 eip, U32 eipCount) {   
    U32 tmpReg;

    x64_writeToMemFromValue(data, cmd, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_CMD, 4, FALSE);
    x64_writeToMemFromValue(data, eip, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EIP, 4, FALSE);
    x64_writeToMemFromValue(data, eipCount, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_CMD_EIPCOUNT, 4, FALSE);

#ifdef _DEBUG
    // dword ptr [r9+offset],esp 
    write8(data, REX_BASE | REX_MOD_RM | REX_64);
    write8(data, 0x89);
    write8(data, 0xa1);
    write32(data, CPU_OFFSET_RSP);
#endif
    if (cmd==CMD_SYSCALL) {
        // cmp eax, 11 (__NR_execve)
        write8(data, 0x83);
        write8(data, 0xf8);
        write8(data, 0x0b);

        // jnz 2
        write8(data, 0x75);
        write8(data, 0x02);

        write8(data, 0xcd);
        write8(data, 0x53);    

        // cmp eax, 252 (__NR_exit_group)
        write8(data, 0x3d);
        write32(data, 252);

        // jnz 2
        write8(data, 0x75);
        write8(data, 0x02);

        write8(data, 0xcd);
        write8(data, 0x53); 

        // cmp eax, 1 (__NR_exit)
        write8(data, 0x83);
        write8(data, 0xf8);
        write8(data, 0x01);

        // jnz 2
        write8(data, 0x75);
        write8(data, 0x02);

        write8(data, 0xcd);
        write8(data, 0x53); 


        write8(data, 0x41);
        write8(data, 0x83);
        write8(data, 0xb9);
        write32(data, CPU_OFFSET_DONE);
        write8(data, 0x0);

        // jz 2
        write8(data, 0x74);
        write8(data, 0x02);

        write8(data, 0xcd);
        write8(data, 0x53); 

        // will not return so it doesn't matter what comes after this
    }

    x64_pushNativeFlags(data);
    tmpReg = x64_getTmpReg(data);
    x64_popNative(data, tmpReg, 1);        
    x64_pushNative(data, HOST_CPU, TRUE);

    x64_writeToMemFromReg(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_FLAGS, 4, FALSE);
    x64_releaseTmpReg(data, tmpReg);

    x64_writeToMemFromReg(data, 0, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EAX, 4, FALSE);
    x64_writeToMemFromReg(data, 1, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ECX, 4, FALSE);
    x64_writeToMemFromReg(data, 2, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EDX, 4, FALSE);
    x64_writeToMemFromReg(data, 3, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EBX, 4, FALSE);
    x64_writeToMemFromReg(data, HOST_ESP, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ESP, 4, FALSE);
    x64_writeToMemFromReg(data, 5, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EBP, 4, FALSE);
    x64_writeToMemFromReg(data, 6, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ESI, 4, FALSE);
    x64_writeToMemFromReg(data, 7, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EDI, 4, FALSE);

    // first parameter to function being called
    // mov rcx, HOST_CPU
    x64_writeToRegFromReg(data, 1, FALSE, HOST_CPU, TRUE, 8);

    // part of the x64 windows ABI, shadow store
    // sub rsp, 32
    write8(data, REX_BASE | REX_64);
    write8(data, 0x83);
    write8(data, 0xEC);
    write8(data, 0x20);    

    // call cpu->enterHost
    write8(data, REX_BASE | REX_MOD_RM);
    write8(data, 0xFF);
    write8(data, 0x91);
    write32(data, CPU_OFFSET_HOST_ENTRY);

    // add rsp, 20
    write8(data, REX_BASE | REX_64);
    write8(data, 0x83);
    write8(data, 0xC4);
    write8(data, 0x20);

    x64_popNative(data, HOST_CPU, TRUE);

    x64_writeToRegFromMem(data, 0, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EAX, 4, FALSE);
    x64_writeToRegFromMem(data, 1, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ECX, 4, FALSE);
    x64_writeToRegFromMem(data, 2, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EDX, 4, FALSE);
    x64_writeToRegFromMem(data, 3, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EBX, 4, FALSE);
    x64_writeToRegFromMem(data, HOST_ESP, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ESP, 4, FALSE);
    x64_writeToRegFromMem(data, 5, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EBP, 4, FALSE);
    x64_writeToRegFromMem(data, 6, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_ESI, 4, FALSE);
    x64_writeToRegFromMem(data, 7, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EDI, 4, FALSE);

    tmpReg = x64_getTmpReg(data);
    x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_FLAGS, 4, FALSE);
    x64_pushNative(data, tmpReg, TRUE);
    x64_releaseTmpReg(data, tmpReg);

    x64_popNativeFlags(data);

    // RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile
    x64_writeToRegFromMem(data, HOST_MEM, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_MEM, 8, FALSE);        

    // :TODO: maybe only sync this if it is possible it could have changed
    x64_writeToRegFromMem(data, HOST_SS, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_SS_ADDRESS, 4, FALSE);
    x64_writeToRegFromMem(data, HOST_DS, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_DS_ADDRESS, 4, FALSE);

    if (eipCount) {        
        tmpReg = x64_getTmpReg(data);
        x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, CPU_OFFSET_EIP, 4, FALSE);        
        x64_jmpReg(data, tmpReg, TRUE);
        x64_releaseTmpReg(data, tmpReg);        
    }
}

void x64_orRegReg(struct x64_Data* data, U32 dst, U32 isDstRex, U32 src, U32 isSrcRex) {
    U8 rex = 0;
    if (isDstRex) {
        rex |= REX_BASE | REX_MOD_REG;
    }
    if (isSrcRex) {
        rex |= REX_BASE | REX_MOD_RM;
    }
    if (rex)
        write8(data, rex);
    write8(data, 0x09);
    write8(data, 0xC0 | (src<<3) | dst);
}

void x64_andWriteToRegFromCPU(struct x64_Data* data, U32 reg, U32 isRegRex, U32 offset) {
    if (isRegRex) {
        write8(data, REX_BASE | REX_MOD_REG | REX_MOD_RM);
    } else {
        write8(data, REX_BASE | REX_MOD_RM);
    }
    write8(data, 0x23);
    if (offset>127) {
        write8(data, 0x80 | (reg<<3) | HOST_CPU);
        write32(data, offset);
    } else {
        write8(data, 0x40 | (reg<<3) | HOST_CPU);
        write8(data, offset);
    }

}

// :TODO: is there a way to do this without changing flags so that I don't have to save them, 
// plus it will mess up the stack if an exception is thrown
//
// could also store flags in a tmpReg if that stack becomes a problem
//
// If this function changes, apply it to enter16/enter32

static void x64_popReg(struct x64_Data* data, U32 reg, U32 isRegRex, S32 bytes) {
    U32 tmpReg;
    BOOL allocatedTmpReg = FALSE;

    if (reg==4 && !isRegRex) {
        reg = HOST_ESP;
        isRegRex = TRUE;
    }

    if (isRegRex && x64_isTmpReg(data, reg)) {
        tmpReg = reg;
    } else {
        tmpReg = x64_getTmpReg(data);
        allocatedTmpReg = TRUE;
    }

    // store flags because the &, - instructions will affect flags
    x64_pushNativeFlags(data);

    // c code
    // reg = readd(cpu->thread, cpu->segAddress[SS] + (ESP & cpu->stackMask));
    // ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);


    // tmpReg = HOST_ESP
    x64_writeToRegFromReg(data, tmpReg, TRUE, HOST_ESP, TRUE, 4);

    // tmpReg &= cpu->stackMask
    x64_andWriteToRegFromCPU(data, tmpReg, TRUE, CPU_OFFSET_STACK_MASK);

    // reg = [ss:tmpReg]
    x64_writeToRegFromMem(data, reg, isRegRex, tmpReg, TRUE, HOST_SS, TRUE, 0, FALSE, bytes, TRUE);
    
    if (reg!=HOST_ESP || !isRegRex) {
        if (!allocatedTmpReg) {
            allocatedTmpReg = TRUE;
            tmpReg = x64_getTmpReg(data);
        }
        // tmpReg = HOST_ESP + bytes
        x64_addWithLea(data, tmpReg, TRUE, HOST_ESP, TRUE, -1, FALSE, 0, bytes, 4);

        // tmpReg &= cpu->stackMask
        x64_andWriteToRegFromCPU(data, tmpReg, TRUE, CPU_OFFSET_STACK_MASK);

        // HOST_ESP = HOST_ESP & cpu->stackNotMask
        x64_andWriteToRegFromCPU(data, HOST_ESP, TRUE, CPU_OFFSET_STACK_NOT_MASK);

        // HOST_ESP = HOST_ESP | tmpReg
        x64_orRegReg(data, HOST_ESP, TRUE, tmpReg, TRUE);
    }
    if (allocatedTmpReg)
        x64_releaseTmpReg(data, tmpReg);

    // restore original flags
    x64_popNativeFlags(data);
}

// 9C                   pushfq  
// 45 8D 43 FC          lea         r8d,[r11-4]  
// 45 23 41 2C          and         r8d,dword ptr [r9+2Ch]  
// 47 8D 04 30          lea         r8d,[r8+r14]  
// 43 89 2C 10          mov         dword ptr [r8+r10],ebp  
// 45 23 59 30          and         r11d,dword ptr [r9+30h]  
// 45 09 C3             or          r11d,r8d  
// 9D                   popfq 
static void x64_pushReg(struct x64_Data* data, S32 reg, U32 isRegRex, U32 value, S32 bytes) {
    U32 tmpReg = x64_getTmpReg(data);

    if (reg==4 && !isRegRex) {
        reg = HOST_ESP;
        isRegRex = TRUE;
    }

    // store flags because the &, - instructions will affect flags
    x64_pushNativeFlags(data);

    // c code
    // U32 new_esp=(ESP & cpu->stackNotMask) | ((ESP - bytes) & cpu->stackMask);
    // writew(cpu->thread, cpu->segAddress[SS] + (new_esp & cpu->stackMask) ,value);
    // ESP = new_esp;

    // tmpReg = HOST_ESP - bytes
    x64_addWithLea(data, tmpReg, TRUE, HOST_ESP, TRUE, -1, FALSE, 0, -bytes, 4);

    // tmpReg &= cpu->stackMask
    x64_andWriteToRegFromCPU(data, tmpReg, TRUE, CPU_OFFSET_STACK_MASK);

    // [ss:tmpReg] = reg
    if (reg>=0) {
        x64_writeToMemFromReg(data, reg, isRegRex, tmpReg, TRUE, HOST_SS, TRUE, 0, FALSE, bytes, TRUE);
    } else {
        x64_writeToMemFromValue(data, value, tmpReg, TRUE, HOST_SS, TRUE, 0, FALSE, bytes, TRUE);
    }

    // HOST_ESP = HOST_ESP & cpu->stackNotMask
    x64_andWriteToRegFromCPU(data, HOST_ESP, TRUE, CPU_OFFSET_STACK_NOT_MASK);

    // HOST_ESP = HOST_ESP | tmpReg
    x64_orRegReg(data, HOST_ESP, TRUE, tmpReg, TRUE);

    x64_releaseTmpReg(data, tmpReg);

    // restore original flags
    x64_popNativeFlags(data);
}

void x64_pushReg16(struct x64_Data* data, U32 reg, U32 isRegRex) {
    x64_pushReg(data, reg, isRegRex, 0, 2);
}

void x64_popReg16(struct x64_Data* data, U32 reg, U32 isRegRex) {
    x64_popReg(data, reg, isRegRex, 2);  
}

void x64_pushReg32(struct x64_Data* data, U32 reg, U32 isRegRex) {
    x64_pushReg(data, reg, isRegRex, 0, 4);
}

void x64_pushw(struct x64_Data* data, U16 value) {
    x64_pushReg(data, -1, FALSE, value, 2);    
}

void x64_pushd(struct x64_Data* data, U32 value) {
    x64_pushReg(data, -1, FALSE, value, 4);
}

void x64_popReg32(struct x64_Data* data, U32 reg, U32 isRegRex) {
    x64_popReg(data, reg, isRegRex, 4);
}

void x64_pushCpuOffset16(struct x64_Data* data, U32 offset) {
    U32 tmpReg = x64_getTmpReg(data);
    x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, offset, 4, FALSE);
    x64_pushReg16(data, tmpReg, TRUE);
    x64_releaseTmpReg(data, tmpReg);
}

void x64_pushCpuOffset32(struct x64_Data* data, U32 offset) {
    U32 tmpReg = x64_getTmpReg(data);
    x64_writeToRegFromMem(data, tmpReg, TRUE, HOST_CPU, TRUE, -1, FALSE, 0, offset, 4, FALSE);
    x64_pushReg32(data, tmpReg, TRUE);
    x64_releaseTmpReg(data, tmpReg);
}

// don't use x64_getTmpReg here, it is import that the exact reg is used for each instruction since
// the exception handler will look for it

void x64_jmpReg(struct x64_Data* data, U32 reg, U32 isRex) {    
    if (reg==HOST_TMP2 && isRex) {
        if (data->cpu->mightSetReg[CS]) {
            x64_getRegForSeg(data, CS, HOST_TMP);
            x64_addWithLea(data, reg, TRUE, reg, TRUE, HOST_TMP, TRUE, 0, 0, 4);
        }
        // mov HOST_TMP, HOST_TMP2
        x64_writeToRegFromReg(data, HOST_TMP, TRUE, reg, isRex, FALSE);
    } else {
        if (reg==HOST_TMP && isRex) {
            if (data->cpu->mightSetReg[CS]) {
                x64_getRegForSeg(data, CS, HOST_TMP2);
                x64_addWithLea(data, HOST_TMP, TRUE, HOST_TMP, TRUE, HOST_TMP2, TRUE, 0, 0, 4);
            }
            // mov HOST_TMP2, HOST_TMP
            x64_writeToRegFromReg(data, HOST_TMP2, TRUE, HOST_TMP, TRUE, FALSE);
        } else {        
            // mov HOST_TMP, reg
            x64_writeToRegFromReg(data, HOST_TMP, TRUE, reg, isRex, FALSE);

            if (data->cpu->mightSetReg[CS]) {
                x64_getRegForSeg(data, CS, HOST_TMP2);
                x64_addWithLea(data, HOST_TMP, TRUE, HOST_TMP, TRUE, HOST_TMP2, TRUE, 0, 0, 4);
            }

            // mov HOST_TMP2, HOST_TMP
            x64_writeToRegFromReg(data, HOST_TMP2, TRUE, HOST_TMP, TRUE, FALSE);
        }
    }
    x64_pushNativeFlags(data);

    // shr HOST_TMP2, 12
    shiftRightReg(data, HOST_TMP2, TRUE, PAGE_SHIFT);    

    // and HOST_TMP, 0xFFF
    andReg(data, HOST_TMP, TRUE, PAGE_MASK);

    // :TODO: maybe use HOST_TMP3 instead of RAX
    // push rax
    x64_pushNative(data, RAX, FALSE);        
    
    // rax=cpu->opToAddressPages
    // mov rax, [HOST_CPU];
    x64_writeToRegFromMem(data, RAX, FALSE, HOST_CPU, TRUE, -1, FALSE, 0, 0, 8, FALSE);
    
    // rax = cpu->opToAddressPages[page]
    // mov RAX, [RAX+HOST_TEMP2<<3] // 3 sizeof(void*)
    x64_writeToRegFromMem(data, RAX, FALSE, RAX, FALSE, HOST_TMP2, TRUE, 3, 0, 8, FALSE); 

    // will move address to RAX and test that it exists, if it doesn't then we
    // will catch the exception.  We leave the address/index we need in HOST_TMP
    // and HOST_TMP2

    // mov RAX, [RAX + HOST_TMP << 3]
    x64_writeToRegFromMem(data, RAX, FALSE, RAX, FALSE, HOST_TMP, TRUE, 3, 0, 8, FALSE); 

    // This will test that the value we are about to jump to exists
    // mov HOST_TMP, [RAX]
    x64_writeToRegFromMem(data, HOST_TMP, TRUE, RAX, FALSE, -1, FALSE, 0, 0, 2, FALSE);

    // mov HOST_TMP, RAX
    x64_writeToRegFromReg(data, HOST_TMP, TRUE, RAX, FALSE, 8);

    // pop rax
    x64_popNative(data, RAX, FALSE);    

    x64_popNativeFlags(data);

    // jmp HOST_TMP
    jmpNativeReg(data, HOST_TMP, TRUE);
}

static void writeHostPlusTmp(struct x64_Data* data, U32 rm, BOOL checkG, U32 isG8bit, U32 isE8bit, U32 tmpReg) {
    data->rex |= REX_BASE | REX_SIB_INDEX|REX_MOD_RM;    
    x64_setRM(data, rm, checkG, FALSE, isG8bit, isE8bit);
    x64_setSib(data, HOST_MEM | (tmpReg << 3), TRUE); 
}

void x64_translateMemory(struct x64_Data* data, U32 rm, BOOL checkG, U32 isG8bit, U32 isE8bit, U32 tmpReg) {
    if (data->ea16) {
        S16 disp = 0;
        U32 leaDisp = 0;

        if (rm<0x40) {
            // no disp
        } else if (rm<0x80) {
            disp = (S8)x64_fetch8(data);
            leaDisp = 0x40;
        } else {
            disp = (S16)x64_fetch16(data);
            leaDisp = 0x80;
        }

        // if changing this, make sure you take into account lea eax, [bx+si]
        switch (E(rm)) {
        case 0x00:             
            if (data->ds==SEG_ZERO) {
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x04, checkG, FALSE, isG8bit, FALSE);
                x64_setSib(data, 0x33, FALSE); // rbx+rsi
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {                
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = BX+SI+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 3, FALSE, 6, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + DS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        case 0x01:            
            if (data->ds==SEG_ZERO) {
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x04, checkG, FALSE, isG8bit, FALSE);
                x64_setSib(data, 0x3b, FALSE); // rbx+rdi
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {          
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = BX+DI+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 3, FALSE, 7, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + DS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        case 0x02:            
            if (data->ds==SEG_ZERO) {
                if (!leaDisp) {
                    leaDisp = 0x40;
                }
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x04, checkG, FALSE, isG8bit, FALSE);
                x64_setSib(data, 0x35, FALSE); // rbp+rsi
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {          
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = BP+SI+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 5, FALSE, 6, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + SS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ss, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        case 0x03:            
            if (data->ds==SEG_ZERO) {
                if (!leaDisp) {
                    leaDisp = 0x40;
                }
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x04, checkG, FALSE, isG8bit, FALSE);
                x64_setSib(data, 0x3d, FALSE); // rbp+rdi
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {             
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = BP+DI+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 5, FALSE, 7, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + SS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ss, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        case 0x04:              
            if (data->ds==SEG_ZERO) {
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x06, checkG, FALSE, isG8bit, FALSE);
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {            
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = SI+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 6, FALSE, -1, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + DS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        case 0x05:              
            if (data->ds==SEG_ZERO) {
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x07, checkG, FALSE, isG8bit, FALSE);
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {            
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = DI+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 7, FALSE, -1, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + DS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        case 0x06:  
        {
            U32 seg = data->ss;
            
            if (seg==SEG_ZERO) {
                if (leaDisp==0) {
                    x64_setRM(data, (G(rm) << 3) | 0x04, checkG, FALSE, isG8bit, FALSE);
                    x64_setSib(data, 0x25, FALSE);
                    x64_setDisplacement32(data, x64_fetch16(data));
                } else {
                    x64_setRM(data, leaDisp | (G(rm) << 3) | 0x05, checkG, FALSE, isG8bit, FALSE);
                    if (leaDisp==0x40) {
                        x64_setDisplacement8(data, (U8)disp);
                    } else if (leaDisp==0x80) {
                        x64_setDisplacement32(data, disp);
                    }
                }
            } else {  
                U32 tmpReg2 = x64_getTmpReg(data);

                x64_zeroReg(data, tmpReg, TRUE);
                if (rm<0x40) {
                    x64_writeToRegFromValue(data, tmpReg, TRUE, (S16)x64_fetch16(data), 2);
                    seg = data->ds;
                } else {
                    // HOST_TMP = BP+disp                    
                    x64_addWithLea(data, tmpReg, TRUE, 5, FALSE, -1, FALSE, 0, disp, 2);
                }
                // HOST_TMP = HOST_TMP + SS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, seg, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        }
        case 0x07:              
            if (data->ds==SEG_ZERO) {
                x64_setRM(data, leaDisp | (G(rm) << 3) | 0x03, checkG, FALSE, isG8bit, FALSE);
                if (leaDisp==0x40) {
                    x64_setDisplacement8(data, (U8)disp);
                } else if (leaDisp==0x80) {
                    x64_setDisplacement32(data, disp);
                }
            } else {        
                U32 tmpReg2 = x64_getTmpReg(data);

                // HOST_TMP = BX+disp
                x64_zeroReg(data, tmpReg, TRUE);
                x64_addWithLea(data, tmpReg, TRUE, 3, FALSE, -1, FALSE, 0, disp, 2);

                // HOST_TMP = HOST_TMP + DS
                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg2), TRUE, 0, 0, 4);

                // [HOST_MEM + HOST_TMP]
                writeHostPlusTmp(data, (rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

                x64_releaseTmpReg(data, tmpReg2);
            }
            break;
        }
    } else {
        if (rm<0x40) {
            switch (E(rm)) {
            case 0x00: 
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x06: 
            case 0x07:                 
                if (data->ds == SEG_ZERO) {
                    x64_setRM(data, rm, checkG, FALSE, isG8bit, isE8bit);
                } else {                    
                    // converts [reg] to HOST_TMP = reg+SEG; [HOST_TMP+HOST_MEM]
                
                    // don't need to worry about E(rm) == 5, that is handled below
                    // HOST_TMP = reg + SEG
                    x64_addWithLea(data, tmpReg, TRUE, E(rm), FALSE, x64_getRegForSeg(data, data->ds, tmpReg), TRUE, 0, 0, 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp(data, (rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                }
                break;
            case 0x05:
                // on x64 this is RIP/EIP + disp32 instead of DS:disp32
                if (data->ds == SEG_ZERO) {                    
                    // converts [disp32] to [disp32]
                    x64_setRM(data, (rm & ~(7)) | 4, checkG, FALSE, isG8bit, isE8bit); // 4=sib
                    x64_setSib(data, 5 | (4 << 3), FALSE); // 5 is for [disp32], 4 is for 0 value index
                    x64_setDisplacement32(data, x64_fetch32(data));
                } else {
                    // converts [disp32] to HOST_TMP = [SEG + disp32]; [HOST_TMP+HOST_MEM]

                    // HOST_TMP = SEG + disp32
                    x64_addWithLea(data, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg), TRUE, -1, FALSE, 0, x64_fetch32(data), 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp(data, (rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                }                
                break;
            case 0x04: {
                    U8 sib = x64_fetch8(data);
                    U8 base = (sib & 7);
                    U8 index = (sib >> 3) & 7;

                    if (base==5) { // no base, [index << shift + disp32]
                        if (data->ds == SEG_ZERO) {
                            x64_setRM(data, rm, checkG, FALSE, isG8bit, isE8bit);
                            x64_setSib(data, sib, TRUE); 
                            x64_setDisplacement32(data, x64_fetch32(data));
                        } else {
                            // convert [index << shift + disp32] to  HOST_TMP = SEG + index << shift + disp32; [HOST_MEM+HOST_TMP];

                            // HOST_TMP = SEG + index << shift + disp32;
                            x64_addWithLea(data, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg), TRUE, (index==4?-1:index), FALSE, sib >> 6, x64_fetch32(data), 4);

                            // [HOST_MEM + HOST_TMP]
                            writeHostPlusTmp(data, (rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                        }                        
                    } else { // [base + index << shift]
                        if (data->ds == SEG_ZERO) {
                            // keep the same, but convert ESP to HOST_ESP
                            x64_setRM(data, rm, checkG, FALSE, isG8bit, isE8bit);
                            x64_setSib(data, sib, TRUE);
                        } else {
                            // convert [base + index << shift] to HOST_TMP=[base + index << shift];HOST_TMP=[HOST_TMP+SEG];[HOST_TMP+MEM]

                            // HOST_TMP=[base + SEG]
                            x64_addWithLea(data, tmpReg, TRUE, base, FALSE,  x64_getRegForSeg(data, base==4?data->ss:data->ds, tmpReg), TRUE, 0, 0, 4);

                            // HOST_TMP=[HOST_TMP+index<<shift];
                            if (index!=4) {
                                x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, index, FALSE, sib >> 6, 0, 4);
                            }

                            // [HOST_MEM + HOST_TMP]
                            writeHostPlusTmp(data, (rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                        }
                    }    
                }
                break;
            }
        } else {		
            switch (E(rm)) {
            case 0x00: 
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x05:
            case 0x06:
            case 0x07:
                if (data->ds == SEG_ZERO) {
                    x64_setRM(data, rm, checkG, FALSE, isG8bit, isE8bit);
                    if (rm<0x80) {
                        x64_setDisplacement8(data, x64_fetch8(data));
                    } else {
                        x64_setDisplacement32(data, x64_fetch32(data));
                    }
                } else {
                    // converts [reg + disp] to HOST_TMP = [reg + SEG + disp]; [HOST_TMP+HOST_MEM];

                    // HOST_TMP = [reg + SEG + disp]
                    x64_addWithLea(data, tmpReg, TRUE, E(rm), FALSE, x64_getRegForSeg(data, E(rm)==5?data->ss:data->ds, tmpReg), TRUE, 0, (rm<0x80?(S8)x64_fetch8(data):x64_fetch32(data)), 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp(data, (rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                }
                break;
            case 0x04: {                    
                    U8 sib = x64_fetch8(data);                    

                    if (data->ds == SEG_ZERO) {
                        x64_setRM(data, rm, checkG, FALSE, isG8bit, isE8bit);
                        x64_setSib(data, sib, TRUE);
                        if (rm<0x80) {
                            x64_setDisplacement8(data, x64_fetch8(data));
                        } else {
                            x64_setDisplacement32(data, x64_fetch32(data));
                        }
                    } else {
                        // convert [base + index << shift + disp] to HOST_TMP=SEG+base+disp;HOST_TMP = HOST_TMP + index << shift;[HOST_TMP+MEM]
                        U8 base = (sib & 7);
                        U8 index = (sib >> 3) & 7;

                        // HOST_TMP=SEG+base+disp
                        x64_addWithLea(data, tmpReg, TRUE, base, FALSE, x64_getRegForSeg(data, base==4 || base==5?data->ss:data->ds, tmpReg), TRUE, 0, (rm<0x80?(S8)x64_fetch8(data):x64_fetch32(data)), 4);

                        // HOST_TMP = HOST_TMP + index << shift
                        if (index!=4) {
                            x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, index , FALSE, sib >> 6, 0, 4);
                        }

                        // [HOST_MEM + HOST_TMP]
                        writeHostPlusTmp(data, (rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                    }
                }
                break;
            }            
        }
    }
}

void x64_incReg(struct x64_Data* data, U32 reg, U32 isRegRex, U32 bytes) {
    U32 rex = 0;
    if (bytes == 2) {
        write8(data, 0x66);
    }
    if (isRegRex) 
        rex |= REX_BASE | REX_MOD_RM;
    if (bytes == 8)
        rex |= REX_BASE | REX_64;
    if (rex)
        write8(data, rex);
    if (bytes == 1) 
        write8(data, 0xFE);
    else
        write8(data, 0xFF);
    write8(data, 0xC0+reg);
}

void x64_decReg(struct x64_Data* data, U32 reg, U32 isRegRex, U32 bytes) {
    U32 rex = 0;
    if (bytes == 2) {
        write8(data, 0x66);
    }
    if (isRegRex) 
        rex |= REX_BASE | REX_MOD_RM;
    if (bytes == 8)
        rex |= REX_BASE | REX_64;
    if (rex)
        write8(data, rex);
    if (bytes == 1) 
        write8(data, 0xFE);
    else
        write8(data, 0xFF);
    write8(data, 0xC8+reg);
}

static void addTodoLinkJump(struct x64_Data* data, U32 memPos, U32 eip, U8 offsetSize, BOOL doneIfDoesNotExist) {
    if (data->cpu->opToAddressPages[eip >> PAGE_SHIFT] && data->cpu->opToAddressPages[eip >> PAGE_SHIFT][eip & PAGE_MASK]) {
        U8* translatedOffset = data->cpu->opToAddressPages[eip >> PAGE_SHIFT][eip & PAGE_MASK];
        U8* offset = &data->memStart[memPos];

        if (offsetSize==4) {
            x64_write32Buffer(offset, (U32)(translatedOffset - offset) - 4);
        } else if (offsetSize==2) {
            x64_write16Buffer(offset, (U32)(translatedOffset - offset) - 2);
        } else if (offsetSize==1) {
            x64_write8Buffer(offset, (U32)(translatedOffset - offset) - 1);
        }
        return;
    }
    if (data->jmpTodoCount>=data->jmpTodoSize) {
        U32 count = data->jmpTodoSize*2;
        U32* jmpTodoEip = malloc(sizeof(U32)*count);
        void** jmpTodoAddress = malloc(sizeof(void*)*count);
        U8* jmpTodoOffsetSize = malloc(sizeof(U8)*count);

        memcpy(jmpTodoEip, data->jmpTodoEip, data->jmpTodoSize*sizeof(U32));
        memcpy(jmpTodoAddress, data->jmpTodoAddress, data->jmpTodoSize*sizeof(void*));
        memcpy(jmpTodoOffsetSize, data->jmpTodoOffsetSize, data->jmpTodoSize*sizeof(U8));

        if (data->jmpTodoEip!=data->jmpTodoEipBuffer) {
            free(data->jmpTodoEip);
        }
        if (data->jmpTodoAddress!=data->jmpTodoAddressBuffer) {
            free(data->jmpTodoAddress);
        }
        if (data->jmpTodoOffsetSize!=data->jmpTodoOffsetSizeBuffer) {
            free(data->jmpTodoOffsetSize);
        }

        data->jmpTodoEip = jmpTodoEip;
        data->jmpTodoAddress = jmpTodoAddress;
        data->jmpTodoOffsetSize = jmpTodoOffsetSize;
        data->jmpTodoSize*=2;
    }
    data->jmpTodoAddress[data->jmpTodoCount] = &data->memStart[memPos];
    data->jmpTodoEip[data->jmpTodoCount] = eip;
    data->jmpTodoOffsetSize[data->jmpTodoCount++] = offsetSize;
}

void x64_jumpConditional(struct x64_Data* data, U32 condition, U32 eip) {    
    write8(data, 0x0F);
    write8(data, 0x80+condition);
    write32(data, 0);
    addTodoLinkJump(data, data->memPos-4, eip, 4, FALSE);
}

void x64_jumpTo(struct x64_Data* data,  U32 eip) {    
    write8(data, 0xE9);
    write32(data, 0);
    addTodoLinkJump(data, data->memPos-4, eip, 4, FALSE);
}

void x64_callTo(struct x64_Data* data,  U32 eip) {    
    write8(data, 0xE8);
    write32(data, 0);
    addTodoLinkJump(data, data->memPos-4, eip, 4, FALSE);
}

// rm could be set to use the rexReg in tmpReg
void x64_translateRM(struct x64_Data* data, U32 rm, U32 checkG, U32 checkE, U32 isG8bit, U32 isE8bit, U32 tmpReg) {
    if (rm<0xC0) {
        x64_translateMemory(data, rm, checkG, isG8bit, isE8bit, tmpReg);
    } else {
        x64_setRM(data, rm, checkG, checkE, isG8bit, isE8bit);
    }
}

static void pushData(struct x64_Data* data, struct x64_Data* next) {
    memset(next, 0, sizeof(struct x64_Data));
    next->ip = data->ip;
    next->memPos = data->memPos;
    next->memStart = data->memStart;
    next->availableMem = data->availableMem;
    next->cpu = data->cpu;
    next->ss = data->ss;
    next->ds = data->ds;
    next->tmp1InUse = data->tmp1InUse;
    next->tmp2InUse = data->tmp2InUse;
    next->tmp3InUse = data->tmp3InUse;
    if (!data->cpu->big)
        next->ea16 = 1;
}

static void popData(struct x64_Data* data, struct x64_Data* prev) {
    data->memPos = prev->memPos;
    data->availableMem = prev->availableMem;
    data->memStart = prev->memStart;
    data->ip = prev->ip;
    if (data->tmp1InUse!=prev->tmp1InUse || data->tmp2InUse!=prev->tmp2InUse || data->tmp3InUse!=prev->tmp3InUse) {
        kpanic("x64: popData failed");
    }
}

static void doInstructionWithE(U32 op, U32 oneByteOp, struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes, BOOL regIsSrc) {
    struct x64_Data tmpData;
    BOOL pushedTmp2 = FALSE;
    U32 tmpReg = x64_getTmpReg(data);

    pushData(data, &tmpData);
    if (bytes==1) {
        tmpData.op = oneByteOp;
    } else if (bytes==2) {
        if (data->cpu->big)
            tmpData.operandPrefix = TRUE;
        tmpData.op = op;
    } else if (bytes==4) {
        if (!data->cpu->big)
            tmpData.operandPrefix = TRUE;
        tmpData.op = op;
    } else if (bytes==8) {
        tmpData.op = op;
        tmpData.rex |= REX_BASE | REX_64;
    }
    if (isRegRex)
        tmpData.rex |= REX_BASE | REX_MOD_REG;
    rm &= ~0x38;
    rm |= (reg << 3);

    x64_translateMemory(&tmpData, rm, TRUE, bytes==1, bytes==1, tmpReg);

    x64_writeOp(&tmpData);
    popData(data, &tmpData);
    x64_releaseTmpReg(data, tmpReg);    
}

void x64_leaToReg(struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes) {
    if (bytes == 1) {
        kpanic("One byte leaToReg not supported");
    }
    doInstructionWithE(0x8b, 0x8a, data, rm, reg, isRegRex, bytes, FALSE);    
}

void x64_writeToRegFromE(struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes) {
    doInstructionWithE(0x8b, 0x8a, data, rm, reg, isRegRex, bytes, FALSE);
}

void x64_writeToEFromReg(struct x64_Data* data, U32 rm, U32 reg, U32 isRegRex, U32 bytes) {
    doInstructionWithE(0x89, 0x88, data, rm, reg, isRegRex, bytes, TRUE);
}

void x64_writeXchgEspEax(struct x64_Data* data) {
    write8(data, REX_MOD_RM|REX_BASE);
    write8(data, 0x90+HOST_ESP);
}

void x64_writeXchgSpAx(struct x64_Data* data) {
    write8(data, 0x66);
    write8(data, REX_MOD_RM|REX_BASE);    
    write8(data, 0x90+HOST_ESP);
}

void x64_cmpRegToValue(struct x64_Data* data, U32 reg, U32 isRegRex, S32 value, U32 bytes) {
    if (reg == 4 && !isRegRex) {
        reg = HOST_ESP;
        isRegRex = TRUE;
    }    
    if (bytes == 2) {
        write8(data, 0x66);
    }
    if (isRegRex) {
        write8(data, REX_BASE|REX_MOD_RM);
    }
    if (value>=-128 && value<=127) {
        write8(data, 0x83);
        write8(data, 0xC0 | (7 << 3) | reg);
        write8(data, (U8)value);
    } else {
        write8(data, 0x81);
        write8(data, 0xC0 | (7 << 3) | reg);
        if (bytes == 4) {
            write32(data, (U32)value);
        } else if (bytes == 2) {
            write16(data, (U16)value);
        }
    }    

}

// double jumps are to work around the fact that the offset we need in x64 may not fit in 1 signed byte
void x64_jcxz(struct x64_Data* data, S8 offset, BOOL ea16) {
    write8(data, 0xe3);
    write8(data, 5);
    x64_jumpTo(data, data->ip);   
    x64_jumpTo(data, data->ip+offset);
}

void x64_loop(struct x64_Data* data, S8 offset, BOOL ea16) {
    write8(data, 0xe2);
    write8(data, 5);
    x64_jumpTo(data, data->ip);   
    x64_jumpTo(data, data->ip+offset);
}

void x64_loopz(struct x64_Data* data, S8 offset, BOOL ea16) {
    write8(data, 0xe1);
    write8(data, 5);
    x64_jumpTo(data, data->ip);   
    x64_jumpTo(data, data->ip+offset);
}

void x64_loopnz(struct x64_Data* data, S8 offset, BOOL ea16) {
    write8(data, 0xe0);
    write8(data, 5);
    x64_jumpTo(data, data->ip);   
    x64_jumpTo(data, data->ip+offset);
}

void x64_bswapEsp(struct x64_Data* data) {
    data->rex = REX_BASE | REX_MOD_RM;
    data->op = 0xC8+HOST_ESP;
    x64_writeOp(data);
}

void x64_retn(struct x64_Data* data) {
    write8(data, 0xc3);
}

void x64_jmpJd(struct x64_Data* data, U32 offset) {
    write8(data, 0xe9);
    write32(data, offset);
}

void x64_setFlags(struct x64_Data* data, U32 flags, U32 mask) {
    x64_pushNative(data, 0, FALSE);
    x64_pushNativeFlags(data);
    x64_popNative(data, 0, FALSE);

    write8(data, 0x25);
    write32(data, ~mask);

    write8(data, 0x0d);
    write32(data, (flags & mask));

    x64_pushNative(data, 0, FALSE);
    x64_popNativeFlags(data);
    x64_popNative(data, 0, FALSE);
}

void x64_salc(struct x64_Data* data) {
    x64_pushNativeFlags(data);
    // sbb al, al
    write8(data, 0x18);
    write8(data, 0xc0);
    x64_popNativeFlags(data);
}

void x64_xlat(struct x64_Data* data) {
    U32 tmpReg = x64_getTmpReg(data);
    U32 tmpReg2;

    // movzx HOST_TMP, al
    write8(data, REX_BASE | REX_MOD_REG);
    write8(data, 0xf);
    write8(data, 0xb6);
    write8(data, 0xC0 | (tmpReg<<3));
    
    if (data->ea16) {
        // AL = readb(cpu->thread, cpu->segAddress[op->base] + (U16)(BX + AL));
        x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, 3, FALSE, 0, 0, 2);
    } else {
        // AL = readb(cpu->thread, cpu->segAddress[op->base] + EBX + AL);
        x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, 3, FALSE, 0, 0, 4);
    }
    // HOST_TMP = HOST_TMP + DS
    tmpReg2 = x64_getTmpReg(data);
    x64_addWithLea(data, tmpReg, TRUE, tmpReg, TRUE, x64_getRegForSeg(data, data->ds, tmpReg2), TRUE, 0, 0, 4);
    x64_releaseTmpReg(data, tmpReg2);

    // [HOST_MEM + HOST_TMP]
    writeHostPlusTmp(data, 4, FALSE, TRUE, TRUE, tmpReg);
    x64_releaseTmpReg(data, tmpReg);

    // mov al, [HOST_MEM + HOST_TMP]
    data->op = 0x8a;
    x64_writeOp(data);
}

static void x64_setSF_onAL(struct x64_Data* data, U32 flagReg) {
    /*
    if (AL & 0x80)
        flags|=SF;
    else
        flags&=~SF;
    */
    // test AL, 0x80
    write8(data, 0xa8);
    write8(data, (U8)SF);

    // jz :1
    write8(data, 0x74);
    write8(data, 6);

    // or HOST_TMPb, SF
    write8(data, REX_BASE|REX_MOD_RM);
    write8(data, 0x80);
    write8(data, 0xC0 | (1<<3) | flagReg);
    write8(data, (U8)SF);
        
    // jmp Jb
    write8(data, 0xeb);
    write8(data, 4);

    // and HOST_TMPb, ~SF
    write8(data, REX_BASE|REX_MOD_RM);
    write8(data, 0x80);
    write8(data, 0xC0 | (4<<3) | flagReg);
    write8(data, (U8)~(SF));
}

static void x64_setZF_onAL(struct x64_Data* data, U32 flagReg) {
    /*
    if (AL == 0)
        flags|=ZF;
    else
        flags&=~ZF;
    */
    // test AL, AL
    write8(data, 0x84);
    write8(data, 0xc0);

    // jnz :1
    write8(data, 0x75);
    write8(data, 6);

    // or HOST_TMPb, ZF
    write8(data, REX_BASE|REX_MOD_RM);
    write8(data, 0x80);
    write8(data, 0xC0 | (1<<3) | flagReg);
    write8(data, (U8)ZF);
        
    // jmp Jb
    write8(data, 0xeb);
    write8(data, 4);

    // and HOST_TMPb, ~ZF
    write8(data, REX_BASE|REX_MOD_RM);
    write8(data, 0x80);
    write8(data, 0xC0 | (4<<3) | flagReg);
    write8(data, (U8)~(ZF));
}

void x64_setPF_onAL(struct x64_Data* data, U32 flagReg) {
    U32 tmpReg = x64_getTmpReg(data);

    /*
    flags &= ~PF;
    flags |= parity_lookup[AL];
    */

    // and HOST_TMPb, ~PF
    write8(data, REX_BASE|REX_MOD_RM);
    write8(data, 0x80);
    write8(data, 0xC0 | (4<<3) | flagReg);
    write8(data, (U8)~(PF));

    // mov HOST_TMP2, parity_lookup
    write8(data, REX_BASE | REX_64 | REX_MOD_RM);
    write8(data, 0xb8+tmpReg);
    write64(data, (U64)parity_lookup);
    
    // or HOST_TMPb, byte ptr [HOST_TMP2]
    write8(data, REX_BASE | REX_MOD_REG | REX_MOD_RM);
    write8(data, 0x0a);
    write8(data, (flagReg << 3) | tmpReg);

    x64_releaseTmpReg(data, tmpReg);
}

void x64_daa(struct x64_Data* data) {
    U32 tmpReg = x64_getTmpReg(data);
    U32 tmpReg2 = x64_getTmpReg(data);

    x64_pushNativeFlags(data);
    x64_popNative(data, tmpReg, TRUE);

    /*
    if (flags & CF) {
        AL+=0x60;
    } else if (AL > 0x99) {
        AL+=0x60;
        addFlag(CF);
    }
    */
        // test HOST_TMP, CF
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0xF6);
        write8(data, 0xC0 | tmpReg);
        write8(data, CF);

        // jnz :1
        write8(data, 0x75);
        write8(data, 8);

        // cmp al, 0x99
        write8(data, 0x3c);
        write8(data, 0x99);
        
        // JBE :2
        write8(data, 0x76);
        write8(data, 6);

        // OR HOST_TMP, CF
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0x80);
        write8(data, 0xC0 | (1<<3) | tmpReg);
        write8(data, CF);
    
        // 1:
        // add al, 0x60
        write8(data, 0x04);
        write8(data, 0x60);

        // 2:

    /*
    if (flags & AF) {
        AL+=0x06;
    } else if ((AL & 0x0F)>0x09) {        
        AL+=0x06;
        addFlag(AF);
    }
    */

        // test HOST_TMP, AF
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0xF6);
        write8(data, 0xC0 | tmpReg);
        write8(data, (U8)AF);

        // jnz :1
        write8(data, 0x75);
        write8(data, 17);

        // mov HOST_TMP2, eax
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0x89);
        write8(data, 0xC0 | tmpReg2);

        // and HOST_TMP2, 0x0F
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0x80);
        write8(data, 0xC0 | (4<<3) | tmpReg2);
        write8(data, 0xf);

        // cmp HOST_TMP2b, 0x09
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0x80);
        write8(data, 0xC0 | (7 << 3) | tmpReg2);
        write8(data, 0x09);

        // jbe :2
        write8(data, 0x76);
        write8(data, 6);

        // OR HOST_TMP, AF
        write8(data, REX_BASE | REX_MOD_RM);
        write8(data, 0x80);
        write8(data, 0xC0 | (1<<3) | tmpReg);
        write8(data, (U8)AF);
    
        // 1:
        // add al, 0x06
        write8(data, 0x04);
        write8(data, 0x06);

        // 2:
    
    x64_releaseTmpReg(data, tmpReg2);

    x64_setSF_onAL(data, tmpReg);
    x64_setZF_onAL(data, tmpReg);
    x64_setPF_onAL(data, tmpReg);
    
    x64_pushNative(data, tmpReg, TRUE);
    x64_releaseTmpReg(data, tmpReg);

    x64_popNativeFlags(data);
}
#endif