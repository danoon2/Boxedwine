#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Asm.h"
#include "x64Ops.h"
#include "../common/common_other.h"
#include "../../../../source/emulation/softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"
#include "../normal/instructions.h"

#include "../common/common_fpu.h"
#include "x64CPU.h"

#define G(rm) ((rm >> 3) & 7)
#define E(rm) (rm & 7)

#define CPU_OFFSET_STACK_MASK  (U32)(offsetof(CPU, stackMask))
#define CPU_OFFSET_STACK_NOT_MASK (U32)(offsetof(CPU, stackNotMask))

#define CPU_OFFSET_EAX (U32)(offsetof(CPU, reg[0].u32))
#define CPU_OFFSET_ECX (U32)(offsetof(CPU, reg[1].u32))
#define CPU_OFFSET_EDX (U32)(offsetof(CPU, reg[2].u32))
#define CPU_OFFSET_EBX (U32)(offsetof(CPU, reg[3].u32))
#define CPU_OFFSET_ESP (U32)(offsetof(CPU, reg[4].u32))
#define CPU_OFFSET_EBP (U32)(offsetof(CPU, reg[5].u32))
#define CPU_OFFSET_ESI (U32)(offsetof(CPU, reg[6].u32))
#define CPU_OFFSET_EDI (U32)(offsetof(CPU, reg[7].u32))
#define CPU_OFFSET_FLAGS (U32)(offsetof(CPU, flags))

#define CPU_OFFSET_STRING_REPEAT (U32)(offsetof(x64CPU, stringRepeat))
#define CPU_OFFSET_STRING_WRITES_DI (U32)(offsetof(x64CPU, stringWritesToDi))
#define CPU_OFFSET_ARG5 (U32)(offsetof(x64CPU, arg5))
#define CPU_OFFSET_STRING_FLAGS (U32)(offsetof(x64CPU, stringFlags))
#define CPU_OFFSET_FPU_STATE (U32)(offsetof(x64CPU, fpuState))
#define CPU_OFFSET_FPU_BUFFER (U32)(offsetof(x64CPU, fpuBuffer))
#define CPU_OFFSET_RETURN_HOST_ADDRESS (U32)(offsetof(x64CPU, returnHostAddress))
#define CPU_OFFSET_RETRANSLATE_CHUNK_ADDRESS (U32)(offsetof(x64CPU, reTranslateChunkAddress))
#define CPU_OFFSET_SYNC_TO_HOST_ADDRESS (U32)(offsetof(x64CPU, syncToHostAddress))
#define CPU_OFFSET_SYNC_FROM_HOST_ADDRESS (U32)(offsetof(x64CPU, syncFromHostAddress))
#define CPU_OFFSET_DO_SINGLE_OP_ADDRESS (U32)(offsetof(x64CPU, doSingleOpAddress))
#define CPU_OFFSET_JMP_AND_TRANSLATE_IF_NECESSARY (U32)(offsetof(x64CPU, jmpAndTranslateIfNecessary))
#define CPU_MEMOFFSET (U32)(offsetof(BtCPU, memOffsets))

#ifdef BOXEDWINE_MSVC
// RCX
#define PARAM_1_REG 1
#define PARAM_1_REX false
// RDX
#define PARAM_2_REG 2
#define PARAM_2_REX false
// R8
#define PARAM_3_REG 0
#define PARAM_3_REX true
// R9
#define PARAM_4_REG 1
#define PARAM_4_REX true
#else 
// RDI
#define PARAM_1_REG 7
#define PARAM_1_REX false
// RSI
#define PARAM_2_REG 6
#define PARAM_2_REX false
// RDX
#define PARAM_3_REG 2
#define PARAM_3_REX false
// RCX
#define PARAM_4_REG 1
#define PARAM_4_REX false
#endif

X64Asm::X64Asm(x64CPU* cpu) : X64Data(cpu), tmp1InUse(false), tmp2InUse(false), tmp3InUse(false), tmp4InUse(false), param1InUse(false), param2InUse(false), param3InUse(false), param4InUse(false)  {
}

void X64Asm::reset() {
    tmp1InUse = false;
    tmp2InUse = false;
    tmp3InUse = false;
    tmp4InUse = false;
    param1InUse = false;
    param2InUse = false;
    param3InUse = false;
    param4InUse = false;
    resetForNewOp();
    BtData::reset();
}

void X64Asm::setDisplacement32(U32 disp32) {
    this->dispSize = 32;
    this->disp = disp32;
}

void X64Asm::setDisplacement8(U8 disp8) {
    this->dispSize = 8;
    this->disp = disp8;
}

void X64Asm::setImmediate8(U8 value) {
    this->immSize = 8;
    this->imm = value;
}

void X64Asm::setImmediate16(U16 value) {
    this->immSize = 16;
    this->imm = value;
}

void X64Asm::setImmediate32(U32 value) {
    this->immSize = 32;
    this->imm = value;
}

void X64Asm::setSib(U8 sib, bool checkBase) {
    if (checkBase && (sib & 7)==4) {
        this->rex |= REX_BASE | REX_MOD_RM;
        sib = (sib & ~7) | HOST_ESP;
    }
    // will never convert index from ESP to HOST_ESP, because ESP indicates a 0 value
    /*
    if (checkIndex && ((sib >> 3) & 7)==4) {
        this->rex |= REX_BASE | REX_SIB_INDEX;
        sib = (sib & ~0x38) | (HOST_ESP << 3);
    }
    */
    this->has_sib = true;
    this->sib = sib;
}

void X64Asm::zeroReg(U8 reg, bool isRexReg, bool keepFlags) {
    if (keepFlags) {
        if (isRexReg)
            this->write8(REX_BASE | REX_MOD_RM);
        this->write8(0xb8 + reg);
        this->write32(0);
    } else {
        if (isRexReg) {
            this->write8(REX_BASE | REX_MOD_RM | REX_MOD_REG);
        }
        this->write8(0x31);
        this->write8(0xC0 | reg | (reg << 3));
    }
}

#define SWAP_U32(x, y) {U32 t=y;y=x;x=t;}
#define SWAP_BOOL(x, y) {bool t=y;y=x;x=t;}

void X64Asm::doMemoryInstruction(U8 op, U8 reg1, bool isReg1Rex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes) {
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
        isReg1Rex = true;
    }

    if (reg2==4 && !isReg2Rex) {
        reg2 = HOST_ESP;
        isReg2Rex = true;
    }

    if (reg3==4 && !isReg3Rex) {
        reg3 = HOST_ESP;
        isReg3Rex = true;
    }

    if (reg3==4 && reg2!=4 && reg3Shift==0) {
        // sp as an index is replaced with a value of 0
        SWAP_U32(reg2, reg3);
        SWAP_BOOL(isReg2Rex, isReg3Rex);
    } else if (reg2==5 && reg3!=-1 && reg3!=5 && reg3Shift==0) {
        // sib0, [ebp+reg<<shift] does not exist
        SWAP_U32(reg2, reg3);
        SWAP_BOOL(isReg2Rex, isReg3Rex);
    }

    if (isReg1Rex)
        rex |= REX_BASE | REX_MOD_REG;
    if (isReg2Rex)
        rex |= REX_BASE | REX_MOD_RM;
    if (isReg3Rex && reg3!=-1)
        rex |= REX_BASE | REX_SIB_INDEX;
    if (bytes == 8)
        rex |= REX_64;
    if (bytes == 2) 
        this->write8(0x66);
    if (rex)
        this->write8(rex);
    this->write8(op);

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
        this->write8(rm);
    } else {
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
                
        this->write8(rm);
        this->write8((reg3Shift<<6) | (reg3 << 3) | reg2); // sib
    }
       
    if (displacementBytes==1)
        this->write8((U8)displacement);
    else if (displacementBytes==4)
        this->write32((U32)displacement);
}

// reg1 = reg2 + reg3<<shift (can be 0,1,2 or 3) + displacement
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void X64Asm::addWithLea(U8 reg1, bool isReg1Rex, U8 reg2, bool isReg2Rex, S32 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U32 bytes) {
    doMemoryInstruction(0x8d, reg1, isReg1Rex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
}

void X64Asm::writeToRegFromValue(U8 reg, bool isRexReg, U64 value, U8 bytes) {
    U8 rex = 0;

    if (reg==4 && !isRexReg) {
        reg = HOST_ESP;
        isRexReg = true;
    }
    if (isRexReg)
        rex = REX_BASE | REX_MOD_RM;
    if (bytes==8)
        rex |= REX_BASE | REX_64;

    if (bytes==2)
        this->write8(0x66);
    if (rex)
        this->write8(rex);
    if (bytes==1) {
        this->write8(0xb0+reg);
    } else {
        this->write8(0xb8+reg);
    }
    if (bytes == 1) {
        this->write8((U8)value);
    } else if (bytes == 2) {
        this->write16((U16)value);
    } else if (bytes == 4) {
        this->write32((U32)value);
    } else if (bytes == 8) {
        this->write64(value);
    }
}

void X64Asm::writeHostPlusTmp(U8 rm, bool checkG, bool isG8bit, bool isE8bit, U8 tmpReg, bool calculateHostAddress, U8 hostMem) {
    this->rex |= REX_BASE | REX_SIB_INDEX|REX_MOD_RM;    
    setRM(rm, checkG, false, isG8bit, isE8bit);
    U8 hostReg = 0xff;
    if (calculateHostAddress) {
        if (hostMem != 0xFF) {
            hostReg = hostMem;
        } else {
            hostReg = getHostMem(tmpReg, true);
        }
    } else {
        hostReg = getTmpReg();
        zeroReg(hostReg, true, true);
    }
    setSib(hostReg | (tmpReg << 3), false);

    if (!calculateHostAddress || hostMem == 0xff) {
        autoReleaseTmpAfterWriteOp.push_back(hostReg);
    }
    autoReleaseTmpAfterWriteOp.push_back(tmpReg);
}

// BMI2
// 0000000002940317  mov         r15d, 0Ch
// 000000000294031D  shrx        r10, r8, r15
// 0000000002940322  mov         r15d, dword ptr[r13 + 408h]
// 0000000002940329  mov         r10, qword ptr[r15 + r10 * 8]

// Without BMI2
// 0000000002840317  mov         r15d, eax
// 000000000284031A  seto        al
// 000000000284031D  lahf
// 000000000284031E  xchg        eax, r15d
// 0000000002840320  mov         r10d, r8d
// 0000000002840323  shr         r10d, 0Ch
// 0000000002840327  xchg        eax, r15d
// 0000000002840329  add         al, 7Fh
// 000000000284032B  sahf
// 000000000284032C  xchg        eax, r15d
// 000000000284032E  mov         r15d, dword ptr[r13 + 408h]
// 0000000002840335  mov         r10, qword ptr[r15 + r10 * 8]

void X64Asm::shiftRightNoFlags(U8 src, bool isSrcRex, U8 dst, U32 value, U8 tmpReg) {
    if (tmpReg == dst) {
        kpanic("X64Asm::shiftRightNoFlags logic error");
    }
    if (x64CPU::hasBMI2) {
        writeToRegFromValue(tmpReg, true, K_PAGE_SHIFT, 4);
        bmi2ShiftRightReg(dst, src, isSrcRex, tmpReg);
    } else {
        if (dst != src) {
            writeToRegFromReg(dst, true, src, isSrcRex, 4);
        }
        pushFlagsToReg(tmpReg, true, true); // since shiftRightReg will change flags
        shiftRightReg(dst, true, K_PAGE_SHIFT); // get page
        popFlagsFromReg(tmpReg, true, true);
    }
}

U8 X64Asm::getHostMem(U8 regEmulatedAddress, bool isRex) {
    U8 resultReg = getTmpReg();
    U8 tmpReg = 0xff;
        
    if (isTmpRegAvailable()) {
        tmpReg = getTmpReg();
    } else {
        tmpReg = (instructionInfo[this->currentOp->inst].writeMemWidth ? HOST_MEM_READ : HOST_MEM_WRITE);
        pushNativeReg(tmpReg, true);            
    }
    if (regEmulatedAddress == 4 && !isRex) {
        regEmulatedAddress = HOST_ESP;
        isRex = true;
    }
    if (x64CPU::hasBMI2) {
        writeToRegFromValue(tmpReg, true, K_PAGE_SHIFT, 4);
        bmi2ShiftRightReg(resultReg, regEmulatedAddress, isRex, tmpReg);
    } else {            
        pushFlagsToReg(tmpReg, true, true); // since shiftRightReg will change flags
        writeToRegFromReg(resultReg, true, regEmulatedAddress, isRex, 4);
        shiftRightReg(resultReg, true, K_PAGE_SHIFT); // get page
        popFlagsFromReg(tmpReg, true, true);            
    }
    writeToRegFromMem(resultReg, true, (instructionInfo[this->currentOp->inst].writeMemWidth ? HOST_MEM_WRITE : HOST_MEM_READ), true, resultReg, true, 3, 0, 8, false); // shift page << 3 (page*8), since sizeof(U64)==8 to get the value in memOffsets[page]
        
    if (isTmpReg(tmpReg)) {
        releaseTmpReg(tmpReg);            
    } else {
        popNativeReg(tmpReg, true);
    }
    return resultReg;
}

void X64Asm::releaseHostMem(U8 reg) {
    releaseTmpReg(reg);
}

void X64Asm::bmi2ShiftRightReg(U8 dstReg, U8 srcReg, bool isSrcRex, U8 amountReg) {
    // shrx tmpReg,regEmulatedAddress,12 
    U8 index = 7 - amountReg;
    write8(0xc4); // vex
    write8(isSrcRex?0x42:0x62);
    write8(0x83 | (index << 3));
    write8(0xf7);
    write8(0xc0 | (dstReg << 3) | srcReg);
}

// reg1 = [reg2 + (reg3 << reg3Shift) + displacement]
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void X64Asm::writeToRegFromMem(U8 dst, bool isDstRex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost, bool skipAlignmentCheck, bool releaseReg3) {
    if (translateToHost) {             
        if (reg3 >= 0 || displacement) {
            U8 tmpReg = getTmpReg();
            addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            if (releaseReg3) {
                releaseTmpReg(reg3);
            }
            checkMemory(tmpReg, true, false, bytes, 0xff, true, skipAlignmentCheck);
            writeToRegFromMem(dst, isDstRex, tmpReg, true, -1, false, 0, 0, bytes, false);
            releaseTmpReg(tmpReg);
        } else {
            U8 hostMemReg = getTmpReg();
            checkMemory(reg2, isReg2Rex, false, bytes, hostMemReg, false, skipAlignmentCheck);
            writeToRegFromMem(dst, isDstRex, reg2, isReg2Rex, hostMemReg, true, 0, 0, bytes, false);
            releaseTmpReg(hostMemReg);
        }        
    } else {
        doMemoryInstruction(bytes==1?0x8a:0x8b, dst, isDstRex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
        if (releaseReg3) {
            releaseTmpReg(reg3);
        }
    }
}

U8 X64Asm::getRegForSeg(U8 base, U8 tmpReg) {
    if (base == ES) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ES_ADDRESS, 4, false); return tmpReg;}
    if (base == SS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SS_ADDRESS, 4, false); return tmpReg;}
    if (base == GS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_GS_ADDRESS, 4, false); return tmpReg;}
    if (base == FS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FS_ADDRESS, 4, false); return tmpReg;}
    if (base == DS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_ADDRESS, 4, false); return tmpReg;}
    if (base == CS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_CS_ADDRESS, 4, false); return tmpReg;}
    kpanic("unknown base in x64dynamic.c getRegForSeg");
    return 0;
}

void X64Asm::translateMemory16(U32 rm, bool checkG, bool isG8bit, bool isE8bit, S8 r1, S8 r2, S16 disp, U8 seg, bool calculateHostAddress, U8 hostMem) {
    U32 tmpReg = getTmpReg();
    U32 tmpReg2 = getTmpReg();

    // tmpReg = r1+r2+disp
    zeroReg(tmpReg, true, true);
    if (r1>=0) {        
        addWithLea(tmpReg, true, r1, false, r2, false, 0, disp, 2);
    } else {
        writeToRegFromValue(tmpReg, true, disp, 2);
    }

    // tmpReg = tmpReg + seg
    addWithLea(tmpReg, true, tmpReg, true, getRegForSeg(seg, tmpReg2), true, 0, 0, 4);
    releaseTmpReg(tmpReg2);

    // [HOST_MEM + tmpReg]
    writeHostPlusTmp((rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);  
}

void X64Asm::translateMemory(U32 rm, bool checkG, bool isG8bit, bool isE8bit, bool calculateHostAddress, U8 hostMem) {
    if (this->ea16) {
        S16 disp = 0;
        U32 leaDisp = 0;

        if (rm<0x40) {
            // no disp
        } else if (rm<0x80) {
            disp = (S8)this->fetch8();
            leaDisp = 0x40;
        } else {
            disp = (S16)this->fetch16();
            leaDisp = 0x80;
        }

        // if changing this, make sure you take into account lea eax, [bx+si]
        switch (E(rm)) {
        case 0x00: // bx+si
            if (this->ds==SEG_ZERO) { // This is a LEA instrunction
                addWithLea(G(rm), false, 3, false, 6, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {                
                translateMemory16(rm, checkG, isG8bit, isE8bit, 3, 6, disp, this->ds, calculateHostAddress, hostMem);
            }
            break;
        case 0x01: // bx + di           
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 3, false, 7, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {        
                translateMemory16(rm, checkG, isG8bit, isE8bit, 3, 7, disp, this->ds, calculateHostAddress, hostMem);
            }
            break;
        case 0x02: // bp+si     
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 5, false, 6, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {          
                translateMemory16(rm, checkG, isG8bit, isE8bit, 5, 6, disp, this->ss, calculateHostAddress, hostMem);
            }
            break;
        case 0x03: // bp+di             
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 5, false, 7, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {             
                translateMemory16(rm, checkG, isG8bit, isE8bit, 5, 7, disp, this->ss, calculateHostAddress, hostMem);
            }
            break;
        case 0x04: // si
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 6, false, -1, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {    
                translateMemory16(rm, checkG, isG8bit, isE8bit, 6, -1, disp, this->ds, calculateHostAddress, hostMem);
            }
            break;
        case 0x05: // di
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 7, false, -1, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {            
                translateMemory16(rm, checkG, isG8bit, isE8bit, 7, -1, disp, this->ds, calculateHostAddress, hostMem);
            }
            break;
        case 0x06: // disp16 or bp
        {
            U32 seg = this->ss;
            
            if (seg==SEG_ZERO) {
                if (leaDisp==0) {
                    writeToRegFromValue(G(rm), false, this->fetch16(), 2);
                } else {
                    addWithLea(G(rm), false, 5, false, -1, false, 0, disp, 2);
                }
                this->skipWriteOp = true;
            } else {  
                if (leaDisp==0) {
                    translateMemory16(rm, checkG, isG8bit, isE8bit, -1, -1, (S16)this->fetch16(), this->ds, calculateHostAddress, hostMem);
                } else {
                    translateMemory16(rm, checkG, isG8bit, isE8bit, 5, -1, disp, this->ss, calculateHostAddress, hostMem);
                }
            }
            break;
        }
        case 0x07: // bx
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 3, false, -1, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {   
                translateMemory16(rm, checkG, isG8bit, isE8bit, 3, -1, disp, this->ds, calculateHostAddress, hostMem);
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
                if (this->ds == SEG_ZERO) {
                    setRM(rm, checkG, false, isG8bit, isE8bit);
                } else {                    
                    // converts [reg] to HOST_TMP = reg+SEG; [HOST_TMP+HOST_MEM]
                                    
                    // don't need to worry about E(rm) == 5, that is handled below
                    if (!this->cpu->thread->process->hasSetSeg[this->ds]) {
                        // [HOST_MEM + reg]
                        this->rex |= REX_BASE | REX_MOD_RM;   
                        U8 hostReg = getHostMem(E(rm), false);
                        setRM((rm & ~(7)) | 4, checkG, false, isG8bit, isE8bit);
                        setSib(hostReg | (E(rm) << 3), false);
                        if (isTmpReg(hostReg)) {
                            autoReleaseTmpAfterWriteOp.push_back(hostReg);
                        }
                    } else {
                        U32 tmpReg = getTmpReg();

                        // HOST_TMP = reg + SEG
                        addWithLea(tmpReg, true, E(rm), false, getRegForSeg(this->ds, tmpReg), true, 0, 0, 4);

                        // [HOST_MEM + HOST_TMP]
                        writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);
                    }                    
                }
                break;
            case 0x05:
                // on x64 this is RIP/EIP + disp32 instead of DS:disp32
                if (this->ds == SEG_ZERO) {                    
                    // converts [disp32] to [disp32]
                    setRM((rm & ~(7)) | 4, checkG, false, isG8bit, isE8bit); // 4=sib
                    setSib(5 | (4 << 3), false); // 5 is for [disp32], 4 is for 0 value index
                    setDisplacement32(this->fetch32());
                } else {
                    U32 disp = this->fetch32();
                    // converts [disp32] to HOST_TMP = [SEG + disp32]; [HOST_TMP+HOST_MEM]

                    U32 tmpReg = getTmpReg();
                    // HOST_TMP = SEG + disp32
                    addWithLea(tmpReg, true, getRegForSeg(this->ds, tmpReg), true, -1, false, 0, disp, 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);
                }                
                break;
            case 0x04: {
                    U8 sib = this->fetch8();
                    U8 base = (sib & 7);
                    U8 index = (sib >> 3) & 7;

                    if (base==5) { // no base, [index << shift + disp32]
                        if (this->ds == SEG_ZERO) {
                            setRM(rm, checkG, false, isG8bit, isE8bit);
                            setSib(sib, true); 
                            setDisplacement32(this->fetch32());
                        } else {
                            // convert [index << shift + disp32] to  HOST_TMP = SEG + index << shift + disp32; [HOST_MEM+HOST_TMP];

                            U32 tmpReg = getTmpReg();
                            // HOST_TMP = SEG + index << shift + disp32;
                            addWithLea(tmpReg, true, getRegForSeg(this->ds, tmpReg), true, (index==4?-1:index), false, sib >> 6, this->fetch32(), 4);

                            // [HOST_MEM + HOST_TMP]
                            writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);
                        }                        
                    } else { // [base + index << shift]
                        if (this->ds == SEG_ZERO) {
                            // keep the same, but convert ESP to HOST_ESP
                            setRM(rm, checkG, false, isG8bit, isE8bit);
                            setSib(sib, true);
                        } else {
                            U8 seg = base==4?this->ss:this->ds;
                            // convert [base + index << shift] to HOST_TMP=[base + index << shift];HOST_TMP=[HOST_TMP+SEG];[HOST_TMP+MEM]
                            U32 tmpReg = getTmpReg();
                            // HOST_TMP=[base + SEG]
                            addWithLea(tmpReg, true, base, false,  getRegForSeg(seg, tmpReg), true, 0, 0, 4);

                            // HOST_TMP=[HOST_TMP+index<<shift];
                            if (index!=4) {
                                addWithLea(tmpReg, true, tmpReg, true, index, false, sib >> 6, 0, 4);
                            }

                            // [HOST_MEM + HOST_TMP]
                            writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);
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
            {
                U8 seg = E(rm)==5?this->ss:this->ds;

                if (this->ds == SEG_ZERO) {
                    setRM(rm, checkG, false, isG8bit, isE8bit);
                    if (rm<0x80) {
                        setDisplacement8(this->fetch8());
                    } else {
                        setDisplacement32(this->fetch32());
                    }
                } else {
                    // converts [reg + disp] to HOST_TMP = [reg + SEG + disp]; [HOST_TMP+HOST_MEM];
                    U32 tmpReg = getTmpReg();

                    if (!this->cpu->thread->process->hasSetSeg[seg]) {
                        // HOST_TMP = [reg + disp]
                        addWithLea(tmpReg, true, E(rm), false, -1, false, 0, (rm<0x80?(S8)this->fetch8():this->fetch32()), 4);
                    } else {                        
                        // HOST_TMP = [reg + SEG + disp]
                        addWithLea(tmpReg, true, E(rm), false, getRegForSeg(seg, tmpReg), true, 0, (rm<0x80?(S8)this->fetch8():this->fetch32()), 4);                        
                    }
                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp((rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);
                }
                break;
            }
            case 0x04: {                    
                    U8 sib = this->fetch8();                    

                    if (this->ds == SEG_ZERO) {
                        setRM(rm, checkG, false, isG8bit, isE8bit);
                        setSib(sib, true);
                        if (rm<0x80) {
                            setDisplacement8(this->fetch8());
                        } else {
                            setDisplacement32(this->fetch32());
                        }
                    } else {
                        // convert [base + index << shift + disp] to HOST_TMP=SEG+base+disp;HOST_TMP = HOST_TMP + index << shift;[HOST_TMP+MEM]
                        U8 base = (sib & 7);
                        U8 index = (sib >> 3) & 7;
                        U8 seg = (base==4 || base==5)?this->ss:this->ds;

                        U32 tmpReg = getTmpReg();
                        U32 disp = (rm<0x80?(S8)this->fetch8():this->fetch32());
                        if (!this->cpu->thread->process->hasSetSeg[seg]) {
                            if (index==4) {
                                // HOST_TMP = base + disp
                                addWithLea(tmpReg, true, base, false, -1 , false, 0, disp, 4);
                            } else {
                                // HOST_TMP = base + index << shift + disp
                                addWithLea(tmpReg, true, base, false, index , false, sib >> 6, disp, 4);
                            }
                        } else {
                            // HOST_TMP=SEG+base+disp
                            addWithLea(tmpReg, true, base, false, getRegForSeg(seg, tmpReg), true, 0, disp, 4);

                            // HOST_TMP = HOST_TMP + index << shift
                            if (index!=4) {
                                addWithLea(tmpReg, true, tmpReg, true, index , false, sib >> 6, 0, 4);
                            }
                        }
                        // [HOST_MEM + HOST_TMP]
                        writeHostPlusTmp((rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg, calculateHostAddress, hostMem);
                    }
                }
                break;
            }            
        }
    }
}

void X64Asm::calculateMemory16(U8 reg, bool isRex, U32 rm, S8 r1, S8 r2, S16 disp, U8 seg) {    
    // tmpReg = r1+r2+disp
    if (isRex || seg != SEG_ZERO) {
        zeroReg(reg, isRex, true);
    }
    if (r1 >= 0) {
        addWithLea(reg, isRex, r1, false, r2, false, 0, disp, 2);
    } else {
        writeToRegFromValue(reg, isRex, disp, 2);
    }

    // tmpReg = tmpReg + seg
    if (seg != SEG_ZERO) {
        U32 tmpReg = getTmpReg();
        addWithLea(reg, isRex, reg, isRex, getRegForSeg(seg, tmpReg), true, 0, 0, 4);
        releaseTmpReg(tmpReg);
    }
}

void X64Asm::calculateMemory(U8 reg, bool isRex, U32 rm) {
    if (this->ea16) {
        S16 disp = 0;
        U32 leaDisp = 0;

        if (rm < 0x40) {
            // no disp
        } else if (rm < 0x80) {
            disp = (S8)this->fetch8();
            leaDisp = 0x40;
        } else {
            disp = (S16)this->fetch16();
            leaDisp = 0x80;
        }

        // if changing this, make sure you take into account lea eax, [bx+si]
        switch (E(rm)) {
        case 0x00: // bx+si
            calculateMemory16(reg, isRex, rm, 3, 6, disp, this->ds);
            break;
        case 0x01: // bx + di           
            calculateMemory16(reg, isRex, rm, 3, 7, disp, this->ds);
            break;
        case 0x02: // bp+si     
            calculateMemory16(reg, isRex, rm, 5, 6, disp, this->ss);
            break;
        case 0x03: // bp+di             
            calculateMemory16(reg, isRex, rm, 5, 7, disp, this->ss);
            break;
        case 0x04: // si
            calculateMemory16(reg, isRex, rm, 6, -1, disp, this->ds);
            break;
        case 0x05: // di
            calculateMemory16(reg, isRex, rm, 7, -1, disp, this->ds);
            break;
        case 0x06: // disp16 or bp
        {
            if (leaDisp == 0) {
                calculateMemory16(reg, isRex, rm, -1, -1, (S16)this->fetch16(), this->ds);
            } else {
                calculateMemory16(reg, isRex, rm, 5, -1, disp, this->ss);
            }
            break;
        }
        case 0x07: // bx
            calculateMemory16(reg, isRex, rm, 3, -1, disp, this->ds);
            break;
        }
    } else {
        if (rm < 0x40) {
            switch (E(rm)) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x06:
            case 0x07:
                // converts [reg] to HOST_TMP = reg+SEG; [HOST_TMP+HOST_MEM]

                // don't need to worry about E(rm) == 5, that is handled below
                if (!this->cpu->thread->process->hasSetSeg[this->ds] || this->ds == SEG_ZERO) {
                    writeToRegFromReg(reg, isRex, E(rm), false, 4);
                } else {
                    U32 tmpReg = getTmpReg();

                    // reg + SEG
                    addWithLea(reg, isRex, E(rm), false, getRegForSeg(this->ds, tmpReg), true, 0, 0, 4);
                    releaseTmpReg(tmpReg);
                }
                break;
            case 0x05:
                // DS:disp32
                if (this->ds == SEG_ZERO) {
                    writeToRegFromValue(reg, isRex, this->fetch32(), 4);
                } else {
                    U32 tmpReg = getTmpReg();
                    addWithLea(reg, isRex, getRegForSeg(this->ds, tmpReg), true, -1, false, 0, this->fetch32(), 4);
                    releaseTmpReg(tmpReg);
                }
                break;
            case 0x04:
            {
                U8 sib = this->fetch8();
                U8 base = (sib & 7);
                U8 index = (sib >> 3) & 7;

                if (base == 5) { // no base, [index << shift + disp32]
                    if (this->ds == SEG_ZERO) {
                        U8 shift = sib >> 6;
                        U32 disp = this->fetch32();
                        if (shift || index == 4) {
                            U32 tmpReg = getTmpReg();
                            zeroReg(tmpReg, true, true);
                            addWithLea(reg, isRex, tmpReg, true, (index == 4 ? -1 : index), false, sib >> 6, disp, 4);
                        } else if (disp) {
                            addWithLea(reg, isRex, index, false, -1, false, 0, disp, 4);
                        } else if (index != reg || isRex) {
                            writeToRegFromReg(reg, isRex, index, false, 4);
                        } else {
                            int ii = 0;
                        }
                    } else {
                        U32 tmpReg = getTmpReg();
                        addWithLea(reg, isRex, getRegForSeg(this->ds, tmpReg), true, (index == 4 ? -1 : index), false, sib >> 6, this->fetch32(), 4);
                        releaseTmpReg(tmpReg);
                    }
                } else { // [base + index << shift]
                    U8 seg = base == 4 ? this->ss : this->ds;

                    if (base == SEG_ZERO || !this->cpu->thread->process->hasSetSeg[seg]) {
                        addWithLea(reg, isRex, base, false, (index == 4 ? -1 : index), false, sib >> 6, 0, 4);
                    } else {                        
                        U32 tmpReg = getTmpReg();                        
                        if (index == 4) {
                            addWithLea(reg, isRex, base, false, getRegForSeg(seg, tmpReg), true, 0, 0, 4);
                        } else {
                            addWithLea(tmpReg, true, base, false, getRegForSeg(seg, tmpReg), true, 0, 0, 4);
                            addWithLea(reg, isRex, tmpReg, true, index, false, sib >> 6, 0, 4);
                        }
                        releaseTmpReg(tmpReg);
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
            {
                U8 seg = E(rm) == 5 ? this->ss : this->ds;

                if (this->ds == SEG_ZERO || !this->cpu->thread->process->hasSetSeg[seg]) {
                    addWithLea(reg, isRex, E(rm), false, -1, false, 0, (rm < 0x80 ? (U32)(S32)(S8)this->fetch8() : this->fetch32()), 4);
                } else {
                    // converts [reg + disp] to HOST_TMP = [reg + SEG + disp]; [HOST_TMP+HOST_MEM];                    
                    U32 tmpReg = getTmpReg();
                    addWithLea(reg, isRex, E(rm), false, getRegForSeg(seg, tmpReg), true, 0, (rm < 0x80 ? (U32)(S32)(S8)this->fetch8() : this->fetch32()), 4);
                    releaseTmpReg(tmpReg);
                }
                break;
            }
            case 0x04:
            {
                U8 sib = this->fetch8();
                U32 disp = (rm < 0x80 ? (U32)(S32)(S8)this->fetch8() : this->fetch32());
                U8 base = (sib & 7);
                U8 index = (sib >> 3) & 7;
                U8 seg = (base == 4 || base == 5) ? this->ss : this->ds;

                if (seg == SEG_ZERO || !this->cpu->thread->process->hasSetSeg[seg]) {
                    if (index == 4) {
                        // HOST_TMP = base + disp
                        addWithLea(reg, isRex, base, false, -1, false, 0, disp, 4);
                    } else {
                        // HOST_TMP = base + index << shift + disp
                        addWithLea(reg, isRex, base, false, index, false, sib >> 6, disp, 4);
                    }
                } else {
                    // reg=SEG+base+disp
                    U8 tmpReg = getTmpReg();
                    // reg = reg + index << shift
                    if (index == 4) {
                        addWithLea(reg, isRex, base, false, getRegForSeg(seg, tmpReg), true, 0, disp, 4);
                    } else  {
                        addWithLea(tmpReg, true, base, false, getRegForSeg(seg, tmpReg), true, 0, disp, 4);
                        addWithLea(reg, isRex, tmpReg, true, index, false, sib >> 6, 0, 4);
                    }
                    releaseTmpReg(tmpReg);
                }
            }
            break;
            }
        }
    }
}

void X64Asm::setRM(U8 rm, bool checkG, bool checkE, bool isG8bit, bool isE8bit) {
    if (checkG && G(rm) == 4 && !isG8bit) {
        this->rex |= REX_BASE | REX_MOD_REG;
        rm = (rm & ~0x38) | (HOST_ESP << 3);
        if (checkE && E(rm)>=4 && isE8bit) {
            kpanic("X64Asm::setRM unhandled E");
        }
    }
    if (checkE && E(rm)== 4 && !isE8bit) {
        this->rex |= REX_BASE | REX_MOD_RM;
        rm = (rm & ~0x07) | HOST_ESP;
        if (checkG && G(rm)>=4 && isG8bit) {
            kpanic("X64Asm::setRM unhandled G");
        }
    }
    this->has_rm = true;
    this->rm = rm;
}

void common_runSingleOp(x64CPU* cpu);

void X64Asm::checkMemory(U8 reg, bool isRex, bool isWrite, U32 width, U8 memReg, bool writeHostMemToReg, bool skipAlignmentCheck, bool releaseReg) {
    bool memRegNeedsRelease = false;
    //bool fpuMustBeNative = currentOp->inst == FNSAVE || currentOp->inst == FRSTOR || currentOp->inst == FLDENV || currentOp->inst == FNSTENV || currentOp->inst == Fxrstor || currentOp->inst == Fxsave;

    bool needFlags = false;

    if (!flagsWrittenToStringFlags) {
        needFlags = currentOp ? (DecodedOp::getNeededFlags(currentBlock, currentOp, CF | PF | SF | ZF | AF | OF) != 0 || instructionInfo[currentOp->inst].flagsUsed != 0) : true;
    }
    if (!skipAlignmentCheck && width != 1 && width != 2 && width != 4 && width != 8 && width != 16) {
        //needFlags = true;
    }

    if (needFlags) {
        U8 flagsReg = getTmpReg();
        pushFlagsToReg(flagsReg, true, true);
        pushNativeReg(flagsReg, true);
        releaseTmpReg(flagsReg);
    }

    if (memReg == 0xff) {
        memRegNeedsRelease = true;
        memReg = getTmpReg();
    }     

    // get page
    writeToRegFromReg(memReg, true, reg, isRex, 4);
    shiftRightReg(memReg, true, K_PAGE_SHIFT);

    // if hostMemReg is 0, then we don't have permission            
    writeToRegFromMem(memReg, true, (isWrite ? HOST_MEM_WRITE : HOST_MEM_READ), true, memReg, true, 3, 0, 8, false); // shift page << 3 (page*8), since sizeof(U64)==8 to get the value in memOffsets[page]            

    U8 testReg = memReg;
    U8 testRegReleaseAfterCmp = memRegNeedsRelease;

    if (writeHostMemToReg) {
        // ram is guaranteed to have 4k alignment, so it's ok that we write this now and test the offset below
        addWithLea(reg, isRex, reg, isRex, memReg, true, 0, 0, 8);
    }

    if (width > 1 && !skipAlignmentCheck) {        
        // 
        // 5 or 6 instructions to check for page splitting, I feel like this could be better
        // mov         r8d, r9d
        // and         r8d, 0FFFh
        // cmp         r8, 0FFCh
        // mov         r8d, 0
        // cmova       r10, r8

        if (!memRegNeedsRelease) {
            testReg = getTmpReg();
            writeToRegFromReg(testReg, true, memReg, true, 8);
            testRegReleaseAfterCmp = true;
        }

        U8 tmp = 0xff;
        bool hostMemPushed = false;
        if (releaseReg) {
            tmp = reg;
        } else {
            if (!isTmpRegAvailable()) {
                pushNativeReg(HOST_ESP, true);
                hostMemPushed = true;
                tmp = HOST_ESP;
            } else {
                tmp = getTmpReg();
            }
            writeToRegFromReg(tmp, true, reg, isRex, 4);
        }
        andReg(tmp, true, 0xFFF);

        // 2-4% faster
        if (width == 2 || width == 4 || width == 8 || width == 16) {
            // and testReg, [HOST_CPU + tmp + offset4b];
            write8(0x4f);
            write8(0x23);
            write8(0x84 | (testReg << 3));
            write8(0xC0 | HOST_CPU | (tmp << 3));
            if (width == 2) {
                write32(offsetof(CPU, memcheckw));
            } else if (width == 4) {
                write32(offsetof(CPU, memcheckd));
            } else if (width == 8) {
                write32(offsetof(CPU, memcheckq));
            } else if (width == 16) {
                write32(offsetof(CPU, memcheckqq));
            } else {
                kpanic("oops");
            }
        } else {
            // cmp tmp, 0x1000 - width
            write8(0x49);
            write8(0x81);
            write8(0xf8 | tmp);
            write32(0x1000 - width);

            zeroReg(tmp, true, true);

            // nbe = tmp > 0x1000 - width
            // cmovnbe memReg, zeroReg
            write8(0x4d);
            write8(0x0f);
            write8(0x47);
            write8(0xC0 | (testReg << 3) | tmp);
        }
        if (hostMemPushed) {
            popNativeReg(HOST_ESP, true);
        } else {
            releaseTmpReg(tmp);
        }
    } else {
        if (releaseReg) {
            releaseTmpReg(reg);
        }
    }
    
    doIf(testReg, true, 0, [=, this] {
        if (needFlags) {
            U8 flagsReg = getTmpReg();
            popNativeReg(flagsReg, true);
            popFlagsFromReg(flagsReg, true, true);
            releaseTmpReg(flagsReg);
        }
        emulateSingleOp(currentOp);
        }, nullptr, false, true, testRegReleaseAfterCmp);

    if (needFlags) {
        U8 flagsReg = getTmpReg();
        popNativeReg(flagsReg, true);
        popFlagsFromReg(flagsReg, true, true);
        releaseTmpReg(flagsReg);
    }

    //releaseTmpReg(flagsReg);
}

// rm could be set to use the rexReg in tmpReg
void X64Asm::translateRM(U8 rm, bool checkG, bool checkE, bool isG8bit, bool isE8bit, U8 immWidth, bool calculateHostAddress) {
    if (rm < 0xC0) {
        bool isWrite = instructionInfo[this->currentOp->inst].writeMemWidth;
        U32 width = (isWrite ? instructionInfo[this->currentOp->inst].writeMemWidth : instructionInfo[this->currentOp->inst].readMemWidth) / 8;

        if (calculateHostAddress && width) {
            U8 emulatedAddressReg = getTmpReg();
            U8 memReg = getTmpReg();

            U32 ip = this->ip;
            calculateMemory(emulatedAddressReg, true, rm);
            this->ip = ip;

            // :TODO: use this address in the actual op below so it doesn't have to calculate it again (retrieved from checkMemory, last optional param)
            checkMemory(emulatedAddressReg, true, isWrite, width, memReg, false, false, true);

            translateMemory(rm, checkG, isG8bit, isE8bit, calculateHostAddress, memReg);            
        } else {
            translateMemory(rm, checkG, isG8bit, isE8bit, calculateHostAddress);
        }
    } else {
        setRM(rm, checkG, checkE, isG8bit, isE8bit);
    }
    if (immWidth==8) {
        setImmediate8(this->fetch8());
    } else if (immWidth==16) {
        setImmediate16(this->fetch16());
    } else if (immWidth==32) {
        setImmediate32(this->fetch32());
    }
    if (!this->skipWriteOp) {
        this->writeOp(isG8bit);
    }
    for (auto & tmpReg : this->autoReleaseTmpAfterWriteOp) {
        releaseTmpReg(tmpReg);
    }
    this->autoReleaseTmpAfterWriteOp.clear();
}

void X64Asm::writeToEFromCpuOffset(U8 rm, U32 offset, U8 fromBytes, U8 toBytes) {
    U8 tmpReg = getTmpReg();
    if (fromBytes==2 && toBytes==4) {
        zeroReg(tmpReg, true, true);
    }
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, offset, fromBytes, false);
    writeToEFromReg(rm, tmpReg, true, toBytes);
    releaseTmpReg(tmpReg);
}

// it is ok to trash current op data
void X64Asm::writeToEFromReg(U8 rm, U8 reg, bool isRegRex, U8 bytes) {
    this->operandPrefix = false;
    if (isRegRex) {
        this->rex = REX_BASE | REX_MOD_REG;
    } else {
        this->rex = 0;
    }
    if (bytes==2) {
        if (this->cpu->isBig()) {
            this->operandPrefix = true;
        }
        this->op = 0x89;
    } else if (bytes==4) {
        this->op = 0x89;
    } else {
        kpanic("writeToEFromReg didn't handle toBytes: %d", bytes);
    }
    rm = (rm & ~0x38) | (reg << 3);
    translateRM(rm, false, true, false, false, 0);
}

void X64Asm::writeToRegFromE(U8 reg, bool isRegRex, U8 rm, U8 bytes) {
    if (rm < 0xC0) {                
        getAddressInRegFromE(reg, isRegRex, rm, false);
        checkMemory(reg, isRegRex, false, bytes, 0xff, true);
        writeToRegFromMem(reg, isRegRex, reg, isRegRex, -1, false, 0, 0, bytes, false);
    } else {
        writeToRegFromReg(reg, isRegRex, E(rm), false, bytes);
    }
}

void X64Asm::getAddressInRegFromE(U8 reg, bool isRegRex, U8 rm, bool calculateHostAddress) {
    calculateMemory(reg, isRegRex, rm);
    if (calculateHostAddress) {
        U8 memReg = getHostMem(reg, isRegRex);
        addWithLea(reg, isRegRex, memReg, true, reg, isRegRex, 0, 0, 8);
        releaseHostMem(memReg);
    }
}

void X64Asm::xlat() {
    U8 tmpReg = getTmpReg();

    // movzx tmp, al
    write8(REX_BASE | REX_MOD_REG);
    write8(0xf);
    write8(0xb6);
    write8(0xC0 | (tmpReg<<3));
    
    if (this->ea16) {
        // AL = readb(cpu->thread, cpu->segAddress[op->base] + (U16)(BX + AL));
        addWithLea(tmpReg, true, tmpReg, true, 3, false, 0, 0, 2);
    } else {
        // AL = readb(cpu->thread, cpu->segAddress[op->base] + EBX + AL);
        addWithLea(tmpReg, true, tmpReg, true, 3, false, 0, 0, 4);
    }
    // tmp = tmp + DS
    U8 tmpReg2 = getTmpReg();
    addWithLea(tmpReg, true, tmpReg, true, getRegForSeg(this->ds, tmpReg2), true, 0, 0, 4);
    releaseTmpReg(tmpReg2);

    // [HOST_MEM + HOST_TMP]
    writeHostPlusTmp(4, false, true, true, tmpReg, true, 0xFF);
    for (auto& tmpReg : this->autoReleaseTmpAfterWriteOp) {
        releaseTmpReg(tmpReg);
    }
    this->autoReleaseTmpAfterWriteOp.clear();

    // mov al, [HOST_MEM + HOST_TMP]
    this->op = 0x8a;
    writeOp();
}

void X64Asm::pushCpuOffset16(U32 offset) {
    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, offset, 4, false);
    pushReg16(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::pushCpuOffset32(U32 offset) {
    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, offset, 4, false);
    pushReg32(tmpReg, true);
    releaseTmpReg(tmpReg);
}

// c code
// U32 new_esp=(ESP & cpu->stackNotMask) | ((ESP - bytes) & cpu->stackMask);
// writew(cpu->thread, cpu->segAddress[SS] + (new_esp & cpu->stackMask) ,value);
// ESP = new_esp;

// 9C                   pushfq  
// 45 8D 43 FC          lea         r8d,[r11-4]  
// 45 23 41 2C          and         r8d,dword ptr [r9+2Ch]  
// 47 8D 04 30          lea         r8d,[r8+r14]  
// 43 89 2C 10          mov         dword ptr [r8+r10],ebp  
// 45 23 59 30          and         r11d,dword ptr [r9+30h]  
// 45 09 C3             or          r11d,r8d  
// 9D                   popfq 

// 32-bit push when no stack segment is set
// 43 89 6C 1C FC       mov         dword ptr [r12+r11-4],ebp  
// 45 8D 5B FC          lea         r11d,[r11-4] 

void X64Asm::push(S32 reg, bool isRegRex, U32 value, S32 bytes) {    
    if (reg==4 && !isRegRex) {
        reg = HOST_ESP;
        isRegRex = true;
    }      
    bool skipAlignmentCheck = true;

    if (cpu->thread->process->hasSetSeg[SS] && bytes == 4) {
        skipAlignmentCheck = false;
    }
	if (this->cpu->thread->process->hasSetSeg[SS]) {
        U8 tmpReg = getTmpReg();
        // tmpReg = HOST_ESP - bytes
        addWithLea(tmpReg, true, HOST_ESP, true, -1, false, 0, -bytes, 4);

        U8 tmpReg1 = getTmpReg();
        pushFlagsToReg(tmpReg1, true, true);
    	andWriteToRegFromCPU(tmpReg, true, CPU_OFFSET_STACK_MASK);
    	popFlagsFromReg(tmpReg1, true, true);
        releaseTmpReg(tmpReg1);

        // [ss:tmpReg] = reg     
        tmpReg1 = getTmpReg();
        addWithLea(tmpReg, true, tmpReg, true, getRegForSeg(SS, tmpReg1), true, 0, 0, 4);
        releaseTmpReg(tmpReg1);
        if (reg >= 0) {
            writeToMemFromReg(reg, isRegRex, tmpReg, true, -1, false, 0, 0, bytes, true, skipAlignmentCheck);
        } else {
            writeToMemFromValue(value, tmpReg, true, -1, false, 0, 0, bytes, true, true, skipAlignmentCheck);
        }
        addWithLea(tmpReg, true, HOST_ESP, true, -1, false, 0, -bytes, 4); // need to refetch, didn't have enough tmp variable to hold on to it

        tmpReg1 = getTmpReg();
        pushFlagsToReg(tmpReg1, true, true);
    	// HOST_ESP = HOST_ESP & cpu->stackNotMask
    	andWriteToRegFromCPU(HOST_ESP, true, CPU_OFFSET_STACK_NOT_MASK);

    	// HOST_ESP = HOST_ESP | tmpReg
    	orRegReg(HOST_ESP, true, tmpReg, true);
        releaseTmpReg(tmpReg);
		// restore original flags
    	popFlagsFromReg(tmpReg1, true, true);
        releaseTmpReg(tmpReg1);
	} else {
        // [ss:tmpReg] = reg
        if (reg>=0) {
            writeToMemFromReg(reg, isRegRex, HOST_ESP, true, -1, false, 0, -bytes, bytes, true, skipAlignmentCheck);
        } else {
            writeToMemFromValue(value, HOST_ESP, true, -1, false, 0, -bytes, bytes, true, false, skipAlignmentCheck);
        }
        addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -bytes, 4);
	}    
}

void X64Asm::pushfw() {
    pushNativeFlags();
    U8 tmpReg = getTmpReg();
    popNativeReg(tmpReg, true);
    pushReg16(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::pushfd() {
    pushNativeFlags();
    U8 tmpReg = getTmpReg();
    popNativeReg(tmpReg, true);
    pushReg32(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::popfw() {
    U8 tmpReg = getTmpReg();
    popReg16(tmpReg, true);
    pushNativeReg(tmpReg, true);
    releaseTmpReg(tmpReg);
    popNativeFlags();
}

void X64Asm::popfd() {
    U8 tmpReg = getTmpReg();
    popReg32(tmpReg, true);
    pushNativeReg(tmpReg, true);
    releaseTmpReg(tmpReg);
    popNativeFlags();
}

// only used for direct reads, alignment was checked before calling this
void X64Asm::writeToRegFromMemAddress(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes) {    
    U8 tmpReg = getTmpReg();
    U8 flagsReg = getTmpReg();
    U8 addressReg = getTmpReg();

    bool needFlags = currentOp ? (DecodedOp::getNeededFlags(currentBlock, currentOp, CF | PF | SF | ZF | AF | OF) != 0 || instructionInfo[currentOp->inst].flagsUsed != 0) : true;

    if (needFlags) {
        pushFlagsToReg(flagsReg, true, true);
    }
    // custom check memory (hard coded page, no alignment checks)
    if (this->cpu->thread->process->hasSetSeg[seg]) {
        addWithLea(addressReg, true, getRegForSeg(seg, tmpReg), true, -1, false, 0, disp, 4);
        U8 pageReg = getTmpReg();
        writeToRegFromReg(pageReg, true, addressReg, true, 4);
        shiftRightReg(pageReg, true, K_PAGE_SHIFT);
        writeToRegFromMem(tmpReg, true, HOST_MEM_READ, true, pageReg, true, 3, 0, 8, false);
        releaseTmpReg(pageReg);
    } else {
        writeToRegFromMem(tmpReg, true, HOST_MEM_READ, true, -1, false, 0, (disp >> K_PAGE_SHIFT) << 3, 8, false);
    }

    doIf(tmpReg, true, 0, [=, this] {
        if (needFlags) {
            popFlagsFromReg(flagsReg, true, true);
        }
        emulateSingleOp(currentOp);
        }, nullptr);

    if (needFlags) {
        popFlagsFromReg(flagsReg, true, true);
    }
    // if disp is a 32-bit negative, then it would be subtracted from the 64-bit tmpReg
    if (this->cpu->thread->process->hasSetSeg[seg]) {
        writeToRegFromMem(reg, isRegRex, tmpReg, true, addressReg, true, 0, 0, bytes, false);
    } else  if (disp >= 0x80000000) {
        U8 dispReg = getTmpReg();
        writeToRegFromValue(dispReg, true, disp, 4);
        writeToRegFromMem(reg, isRegRex, tmpReg, true, dispReg, true, 0, 0, bytes, false);
        releaseTmpReg(dispReg);
    } else {
        writeToRegFromMem(reg, isRegRex, tmpReg, true, -1, false, 0, disp, bytes, false);
    }
    releaseTmpReg(addressReg);
    releaseTmpReg(tmpReg);
    releaseTmpReg(flagsReg);
}

// only used for direct writes, alignment was checked before calling this
void X64Asm::writeToMemAddressFromReg(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes) {     
    U8 tmpReg = getTmpReg();
    U8 flagsReg = getTmpReg();        
    U8 addressReg = getTmpReg();

    bool needFlags = currentOp ? (DecodedOp::getNeededFlags(currentBlock, currentOp, CF | PF | SF | ZF | AF | OF) != 0 || instructionInfo[currentOp->inst].flagsUsed != 0) : true;

    if (needFlags) {
        pushFlagsToReg(flagsReg, true, true);
    }        
    // custom check memory (hard coded page, no alignment checks)
    if (this->cpu->thread->process->hasSetSeg[seg]) {
        addWithLea(addressReg, true, getRegForSeg(seg, tmpReg), true, -1, false, 0, disp, 4);
        U8 pageReg = getTmpReg();
        writeToRegFromReg(pageReg, true, addressReg, true, 4);
        shiftRightReg(pageReg, true, K_PAGE_SHIFT);
        writeToRegFromMem(tmpReg, true, HOST_MEM_WRITE, true, pageReg, true, 3, 0, 8, false);
        releaseTmpReg(pageReg);
    } else {
        writeToRegFromMem(tmpReg, true, HOST_MEM_WRITE, true, -1, false, 0, (disp >> K_PAGE_SHIFT) << 3, 8, false);
    }

    doIf(tmpReg, true, 0, [=, this] {
        if (needFlags) {
            popFlagsFromReg(flagsReg, true, true);                
        }
        emulateSingleOp(currentOp);
        }, nullptr);

    if (needFlags) {
        popFlagsFromReg(flagsReg, true, true);
    }
    // if disp is a 32-bit negative, then it would be subtracted from the 64-bit tmpReg
    if (this->cpu->thread->process->hasSetSeg[seg]) {
        writeToMemFromReg(reg, isRegRex, tmpReg, true, addressReg, true, 0, 0, bytes, false);
    } else  if (disp >= 0x80000000) {
        U8 dispReg = getTmpReg();
        writeToRegFromValue(dispReg, true, disp, 4);
        writeToMemFromReg(reg, isRegRex, tmpReg, true, dispReg, true, 0, 0, bytes, false);
        releaseTmpReg(dispReg);
    } else {
        writeToMemFromReg(reg, isRegRex, tmpReg, true, -1, false, 0, disp, bytes, false);
    }
    releaseTmpReg(addressReg);
    releaseTmpReg(tmpReg);
    releaseTmpReg(flagsReg);  
}

void X64Asm::pushReg16(U8 reg, bool isRegRex) {
    push(reg, isRegRex, 0, 2);
}

void X64Asm::popReg16(U8 reg, bool isRegRex) {
    popReg(reg, isRegRex, 2, true);  
}

void X64Asm::pushReg32(U8 reg, bool isRegRex) {
    push(reg, isRegRex, 0, 4);
}

void X64Asm::pushw(U16 value) {
    push(-1, false, value, 2);    
}

void X64Asm::pushd(U32 value) {
    push(-1, false, value, 4);
}

void X64Asm::pushE16(U8 rm) {
    if (rm<0xC0) {
        U8 tmpReg = getTmpReg();
        writeToRegFromE(tmpReg, true, rm, 2);
        pushReg16(tmpReg, true);
        releaseTmpReg(tmpReg);
    } else {
        pushReg16(rm & 7, false);
    }
}

void X64Asm::pushE32(U8 rm) {
    if (rm<0xC0) {
        U8 tmpReg = getTmpReg();
        writeToRegFromE(tmpReg, true, rm, 4);
        pushReg32(tmpReg, true);
        releaseTmpReg(tmpReg);
    } else {
        pushReg32(rm & 7, false);
    }
}

void X64Asm::popw(U8 rm) {
    if (rm<0xC0) {     
        U32 tmpReg = getTmpReg();
        popReg(tmpReg, true, 2, false); // just peek, we don't want to modify ESP until after the write succeeds incase there is an exception
        writeToEFromReg(rm, tmpReg, true, 2);
        adjustStack(tmpReg, 2);
        releaseTmpReg(tmpReg);
    } else {
        popReg16(rm & 7, false);
    }
}

void X64Asm::popd(U8 rm) {
    if (rm<0xC0) {     
        U32 tmpReg = getTmpReg();
        popReg(tmpReg, true, 4, false); // just peek, we don't want to modify ESP until after the write succeeds incase there is an exception
        writeToEFromReg(rm, tmpReg, true, 4);
        adjustStack(tmpReg, 4);
        releaseTmpReg(tmpReg);
    } else {
        popReg32(rm & 7, false);
    }
}

void X64Asm::popReg32(U8 reg, bool isRegRex) {
    popReg(reg, isRegRex, 4, true);
}

void X64Asm::pushNativeReg(U8 reg, bool isRegRex) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0x50+reg);
}

void X64Asm::pushNativeValue32(U32 value) {
    write8(0x68);
    write32(value);
}

void X64Asm::pushNativeValue8(U8 value) {
    write8(0x6a);
    write8(value);
}

void X64Asm::pushNativeFlags() {
    write8(0x9c);
}

void X64Asm::popNativeReg(U8 reg, bool isRegRex) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0x58+reg);
}

void X64Asm::popNativeFlags() {
    write8(0x9d);
}

void X64Asm::orRegReg(U8 dst, bool isDstRex, U8 src, bool isSrcRex) {
    U8 rex = 0;
    if (isDstRex) {
        rex |= REX_BASE | REX_MOD_RM;
    }
    if (isSrcRex) {
        rex |= REX_BASE | REX_MOD_REG;
    }
    if (rex)
        write8(rex);
    write8(0x09);
    write8(0xC0 | (src<<3) | dst);
}

void X64Asm::andWriteToRegFromCPU(U8 reg, bool isRegRex, U32 offset) {
    if (isRegRex) {
        write8(REX_BASE | REX_MOD_REG | REX_MOD_RM);
    } else {
        write8(REX_BASE | REX_MOD_RM);
    }
    write8(0x23);
    if (offset>127) {
        write8(0x80 | (reg<<3) | HOST_CPU);
        write32(offset);
    } else {
        write8(0x40 | (reg<<3) | HOST_CPU);
        write8(offset);
    }
}

// [reg2 + (reg3 << reg3Shift) + displacement] = reg1
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void X64Asm::writeToMemFromReg(U8 src, bool isSrcRex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost, bool skipAlignmentCheck, bool releaseReg3) {
    if (translateToHost) {                      
        if (reg3 >= 0 || displacement) {
            U8 tmpReg = getTmpReg();
            addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            if (releaseReg3) {
                releaseTmpReg(reg3);
            }
            checkMemory(tmpReg, true, true, bytes, 0xff, true, skipAlignmentCheck);
            writeToMemFromReg(src, isSrcRex, tmpReg, true, -1, true, 0, 0, bytes, false);
            releaseTmpReg(tmpReg);
        } else {
            U8 hostMemReg = getTmpReg();
            checkMemory(reg2, isReg2Rex, true, bytes, hostMemReg, false, skipAlignmentCheck);
            writeToMemFromReg(src, isSrcRex, reg2, isReg2Rex, hostMemReg, true, 0, 0, bytes, false);
            releaseTmpReg(hostMemReg);
        }        
    } else {
        // reg 1 will be ignored
        doMemoryInstruction(bytes==1?0x88:0x89, src, isSrcRex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
        if (reg3 >= 0 && releaseReg3) {
            releaseTmpReg(reg3);
        }
    }
}

void X64Asm::writeToMemFromValue(U64 value, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost, bool canUseReg2AsTmp, bool skipAlignmentCheck) {
    if (translateToHost) {
        if (canUseReg2AsTmp) {
            U8 hostMemReg = getTmpReg();
            addWithLea(reg2, isReg2Rex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            checkMemory(reg2, isReg2Rex, true, bytes, hostMemReg, false, skipAlignmentCheck);
            writeToMemFromValue(value, reg2, isReg2Rex, hostMemReg, true, 0, 0, bytes, false);
            releaseTmpReg(hostMemReg);
        } else {
            U8 tmpReg = getTmpReg();
            U8 hostMemReg = getTmpReg();
            addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            checkMemory(tmpReg, true, true, bytes, hostMemReg, false, skipAlignmentCheck);
            writeToMemFromValue(value, tmpReg, true, hostMemReg, true, 0, 0, bytes, false);
            releaseTmpReg(tmpReg);
            releaseTmpReg(hostMemReg);
        }
    } else {
        // reg 1 will be ignored
        doMemoryInstruction(bytes==1?0xc6:0xc7, 0, false, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
        if (bytes == 1)
            write8((U8)value);
        else if (bytes == 2)
            write16((U16)value);
        else if (bytes == 4) 
            write32((U32)value);
        else if (bytes == 8)
            kpanic("writeToMemFromValue 64-bit not implemented");
    }
}

void X64Asm::writeToRegFromReg(U8 toReg, bool isToReg1Rex, U8 fromReg, bool isFromRegRex, U8 bytes) {
    U8 rex = 0;

    if (toReg==4 && !isToReg1Rex && bytes!=1) {
        toReg = HOST_ESP;
        isToReg1Rex = true;
    }
    if (fromReg==4 && !isFromRegRex && bytes!=1) {
        fromReg = HOST_ESP;
        isFromRegRex = true;
    }

    if (isToReg1Rex)
        rex = REX_BASE | REX_MOD_RM;
    if (isFromRegRex)
        rex |= REX_BASE | REX_MOD_REG;
    if (bytes==8)
        rex |= REX_BASE | REX_64;

    if (bytes == 2)
        write8(0x66);
    if (rex)
        write8(rex);
    if (bytes == 1) 
        write8(0x88);
    else
        write8(0x89);
    write8(0xC0 | toReg | (fromReg << 3));
}

void X64Asm::adjustStack(U8 tmpReg, S32 bytes) {
	if (!this->cpu->thread->process->hasSetSeg[SS]) {
        addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, bytes, 4);
	} else {
        // tmpReg = HOST_ESP + bytes
        addWithLea(tmpReg, true, HOST_ESP, true, -1, false, 0, bytes, 4);

        U8 tmpReg2 = getTmpReg();
        pushFlagsToReg(tmpReg2, true, true);

        // tmpReg &= cpu->stackMask
        andWriteToRegFromCPU(tmpReg, true, CPU_OFFSET_STACK_MASK);

        // HOST_ESP = HOST_ESP & cpu->stackNotMask
        andWriteToRegFromCPU(HOST_ESP, true, CPU_OFFSET_STACK_NOT_MASK);

        // HOST_ESP = HOST_ESP | tmpReg
        orRegReg(HOST_ESP, true, tmpReg, true);
        
        popFlagsFromReg(tmpReg2, true, true);
        releaseTmpReg(tmpReg2);
    }
}

// could also store flags in a tmpReg if that stack becomes a problem
//
// If this function changes, apply it to enter16/enter32

// c code
// reg = readd(cpu->thread, cpu->segAddress[SS] + (ESP & cpu->stackMask));
// ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);

// pop esi if no stack segment set
// 43 8B 34 1C          mov         esi,dword ptr [r12+r11]  
// 45 8D 5B 04          lea         r11d,[r11+4] 

void X64Asm::popReg(U8 reg, bool isRegRex, S8 bytes, bool commit) {
    U8 tmpReg = 0xff;
    bool allocatedTmpReg = false;

    if (reg==4 && !isRegRex) {
        reg = HOST_ESP;
        isRegRex = true;
    }

    if (isRegRex && isTmpReg(reg)) {
        tmpReg = reg;
    } else {
        tmpReg = getTmpReg();
        allocatedTmpReg = true;
    }    

    // tmpReg &= cpu->stackMask
    if (this->cpu->thread->process->hasSetSeg[SS]) {
        // tmpReg = HOST_ESP
        writeToRegFromReg(tmpReg, true, HOST_ESP, true, 4);

        U8 tmpReg1 = getTmpReg();
        pushFlagsToReg(tmpReg1, true, true);
        andWriteToRegFromCPU(tmpReg, true, CPU_OFFSET_STACK_MASK);
        popFlagsFromReg(tmpReg1, true, true); // the following write can throw an exception, so make sure out stack is fine in case the rest of this instruction is skipped                

        // reg = [ss:tmpReg] 
        // could remove this add and put it in the write function, but I need the tmp reg
        addWithLea(tmpReg, true, tmpReg, true, getRegForSeg(SS, tmpReg1), true, 0, 0, 4);
        releaseTmpReg(tmpReg1);
        writeToRegFromMem(reg, isRegRex, tmpReg, true, -1, false, 0, false, bytes, true, bytes==2);        
    } else {
        // reg = [ss:tmpReg]    
        writeToRegFromMem(reg, isRegRex, HOST_ESP, true, -1, false, 0, false, bytes, true, true);
    }
    
    
    if (commit && (reg!=HOST_ESP || !isRegRex)) {
        if (!allocatedTmpReg) {
            allocatedTmpReg = true;
            tmpReg = getTmpReg();
        }
        adjustStack(tmpReg, bytes);
    }
    if (allocatedTmpReg)
        releaseTmpReg(tmpReg);
}

void X64Asm::lockParamReg(U8 paramReg, bool paramRex) {
    if (paramRex && ((paramReg == HOST_TMP && this->tmp1InUse) || (paramReg == HOST_TMP2 && this->tmp2InUse) || (paramReg == HOST_TMP3 && this->tmp3InUse))) {
        kpanic("X64Asm::lockParamReg tried to lock a param that is being used as a tmp reg");
    }
    if (paramRex && paramReg == HOST_TMP) {
        this->tmp1InUse = true;
    }
    if (paramRex && paramReg == HOST_TMP2) {
        this->tmp2InUse = true;
    }
    if (paramRex && paramReg == HOST_TMP3) {
        this->tmp3InUse = true;
    }
    if (paramReg==PARAM_1_REG && paramRex==PARAM_1_REX) {
        if (this->param1InUse) {
            kpanic("X64Asm::lockParamReg param 1 already locked");
        }
        this->param1InUse = true;        
    } else if (paramReg==PARAM_2_REG && paramRex==PARAM_2_REX) {
        if (this->param2InUse) {
            kpanic("X64Asm::lockParamReg param 2 already locked");
        }
        this->param2InUse = true;        
    } else if (paramReg==PARAM_3_REG && paramRex==PARAM_3_REX) {
        if (this->param3InUse) {
            kpanic("X64Asm::lockParamReg param 3 already locked");
        }
        this->param3InUse = true;        
    } else if (paramReg==PARAM_4_REG && paramRex==PARAM_4_REX) {
        if (this->param4InUse) {
            kpanic("X64Asm::lockParamReg param 4 already locked");
        }
        this->param4InUse = true;        
    } else {
        kpanic("X64Asm::lockParamReg unknown param %d", paramReg);
    }
}

void X64Asm::unlockParamReg(U8 paramReg, bool paramRex) {
    bool used = false;
    if (paramReg==PARAM_1_REG && paramRex==PARAM_1_REX) {
        if (this->param1InUse) {
            this->param1InUse = false;        
            used = true;
        }
    } else if (paramReg==PARAM_2_REG && paramRex==PARAM_2_REX) {
        if (this->param2InUse) {
            this->param2InUse = false;        
            used = true;
        }
    } else if (paramReg==PARAM_3_REG && paramRex==PARAM_3_REX) {
        if (this->param3InUse) {
            this->param3InUse = false;        
            used = true;
        }
    } else if (paramReg==PARAM_4_REG && paramRex==PARAM_4_REX) {
        if (this->param4InUse) {
            this->param4InUse = false;        
            used = true;
        }
    } else {
        kpanic("X64Asm::lockParamReg unknown param %d", paramReg);
    }
    if (used) {
        if (paramRex && paramReg == HOST_TMP) {
            this->tmp1InUse = false;
        }
        if (paramRex && paramReg == HOST_TMP2) {
            this->tmp2InUse = false;
        }
        if (paramRex && paramReg == HOST_TMP3) {
            this->tmp3InUse = false;
        }
    }
}

U8 X64Asm::getParamSafeTmpReg() {
    if (this->tmp3InUse) {
        kpanic("X64Asm::getParamSafeTmpReg tmp 3 already in use");
    }
    this->tmp3InUse = true;
    return HOST_TMP3;
}

bool X64Asm::isTmpRegAvailable() {
    if (!this->tmp1InUse) {
        return true;
    }
    if (!this->tmp2InUse) {
        return true;
    }
    if (!this->tmp3InUse) {
        return true;
    }
    if (!this->tmp4InUse) {
        return true;
    }
    return false;
}

U8 X64Asm::getTmpReg() {
    if (!this->tmp1InUse) {
        this->tmp1InUse = true;
        return HOST_TMP;
    }
    if (!this->tmp2InUse) {
        this->tmp2InUse = true;
        return HOST_TMP2;
    }
    if (!this->tmp3InUse) {
        this->tmp3InUse = true;
        return HOST_TMP3;
    }
    if (!this->tmp4InUse) {
        this->tmp4InUse = true;
        return HOST_TMP4;
    }
    kpanic("x64: ran out of tmp variables");
    return 0;
}


void X64Asm::releaseTmpReg(U8 tmpReg) {
    if (tmpReg==HOST_TMP) {
        if (!this->tmp1InUse) {
            kpanic("x64: tried to release HOST_TMP but it was not in use");
        }
        this->tmp1InUse = false;
    } else if (tmpReg==HOST_TMP2) {
        if (!this->tmp2InUse) {
            kpanic("x64: tried to release HOST_TMP2 but it was not in use");
        }
        this->tmp2InUse = false;
    } else if (tmpReg==HOST_TMP3) {
        if (!this->tmp3InUse) {
            kpanic("x64: tried to release HOST_TMP3 but it was not in use");
        }
        this->tmp3InUse = false;
    } else if (tmpReg == HOST_TMP4) {
        if (!this->tmp4InUse) {
            kpanic("x64: tried to release HOST_TMP4 but it was not in use");
        }
        this->tmp4InUse = false;
    } else {
        kpanic("x64: tried to release an unknown reg");
    }
}

bool X64Asm::isTmpReg(U8 tmpReg) {
    return tmpReg == HOST_TMP || tmpReg == HOST_TMP2 || tmpReg == HOST_TMP3 || tmpReg == HOST_TMP4;
}

// calling convention, these regs can be modified 
// Microsft: RAX, RCX, RDX, R8, R9, R10, R11 volitile
// Other: RAX, RCX, RDX, RSP, RDI, RSI, R8, R9, R10, R11 are volitile

void X64Asm::minSyncRegsFromHost() {
    writeToMemFromReg(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EAX, 4, false);
    writeToMemFromReg(1, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ECX, 4, false);
    writeToMemFromReg(2, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDX, 4, false);
    writeToMemFromReg(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESP, 4, false);

#ifndef BOXEDWINE_MSVC
    writeToMemFromReg(6, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESI, 4, false);
    writeToMemFromReg(7, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDI, 4, false);
#endif
}

void X64Asm::minSyncRegsToHost() {
    writeToRegFromMem(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EAX, 4, false); // RAX is volitile
    writeToRegFromMem(1, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ECX, 4, false); // RCX is volitile
    writeToRegFromMem(2, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDX, 4, false); // RDX is volitile
    writeToRegFromMem(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESP, 4, false); // R11 is volitile    
#ifndef BOXEDWINE_MSVC      
    writeToRegFromMem(6, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESI, 4, false); // RSI is volitile
    writeToRegFromMem(7, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDI, 4, false); // RDI is volitile
#endif
}

void X64Asm::syncRegsFromHost(bool eipInR9) {    
    if (eipInR9) {
#ifdef _DEBUG
        writeToMemFromReg(1, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP_FROM, 4, false);
#endif
        writeToMemFromReg(1, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    } else {
        writeToMemFromValue(this->startOfOpIp, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    }
    syncRegsFromHostCall();
}

void X64Asm::createCodeForDoSingleOp() {
    syncRegsFromHost(true);
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8);    
    callHost((void*)common_runSingleOp);
    writeToRegFromReg(0, true, 0, false, 8);
    syncRegsToHost();
    doJmp(true);
}

static void fpuWriteMem(x64CPU* cpu, U32 address, U32 len) {
    if (len > sizeof(cpu->fpuState)) {
        kpanic("fpuReadMem");
    }
    cpu->memory->memcpy(address, cpu->fpuBuffer, len);
}

static void fpuReadMem(x64CPU* cpu, U32 address, U32 len) {
    if (len > sizeof(cpu->fpuState)) {
        kpanic("fpuReadMem");
    }
    cpu->memory->memcpy(cpu->fpuBuffer, address, len);
}

void X64Asm::fpuWrite(U8 rm, U32 len) {
    U8 emulatedAddressReg = getParamSafeTmpReg();    

    calculateMemory(emulatedAddressReg, true, rm);

    // fnsave [HOST_CPU + offset]
    write8(0x41);
    if (currentOp->originalOp >= 0x300) {
        write8(0x0f);
    }    
    write8((U8)currentOp->originalOp);
    write8((rm & 0x38) | HOST_CPU | 0x80);
    write32(CPU_OFFSET_FPU_BUFFER);

    syncRegsFromHost();

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromReg(PARAM_2_REG, PARAM_2_REX, emulatedAddressReg, true, 4);

    releaseTmpReg(emulatedAddressReg);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8);    

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, len, 4);
    
    callHost((void*)fpuWriteMem);
    syncRegsToHost();
}

void X64Asm::fpuRead(U8 rm, U32 len) {
    U8 emulatedAddressReg = getParamSafeTmpReg();

    calculateMemory(emulatedAddressReg, true, rm);    

    syncRegsFromHost();

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromReg(PARAM_2_REG, PARAM_2_REX, emulatedAddressReg, true, 4);

    releaseTmpReg(emulatedAddressReg);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, len, 4);
    
    callHost((void*)fpuReadMem);
    syncRegsToHost();

    // fldenv [HOST_CPU + offset]
    write8(0x41);
    if (currentOp->originalOp >= 0x300) {
        write8(0x0f);
    }    
    write8((U8)currentOp->originalOp);
    write8((rm & 0x38) | HOST_CPU | 0x80);
    write32(CPU_OFFSET_FPU_BUFFER);
}

void X64Asm::createCodeForSyncFromHost() {
    writeToMemFromReg(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EAX, 4, false);
    writeToMemFromReg(1, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ECX, 4, false);
    writeToMemFromReg(2, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDX, 4, false);
    writeToMemFromReg(3, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBX, 4, false);
    writeToMemFromReg(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESP, 4, false);
    writeToMemFromReg(5, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBP, 4, false);
    writeToMemFromReg(6, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESI, 4, false);
    writeToMemFromReg(7, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDI, 4, false);
    U8 tmpReg = getTmpReg();
    pushNativeFlags();
    popNativeReg(tmpReg, true);
    writeToMemFromReg(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FLAGS, 4, false);
    releaseTmpReg(tmpReg);

    write8(0x41);
    write8(0x0f);
    write8(0xae);
    write8(0x80 | HOST_CPU);
    write32(CPU_OFFSET_FPU_STATE);

    write8(0xc3); // ret
}

void X64Asm::createCodeForSyncToHost() {
    writeToRegFromMem(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EAX, 4, false);
    writeToRegFromMem(1, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ECX, 4, false);
    writeToRegFromMem(2, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDX, 4, false);
    writeToRegFromMem(3, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBX, 4, false);
    writeToRegFromMem(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESP, 4, false);
    writeToRegFromMem(5, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBP, 4, false);
    writeToRegFromMem(6, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESI, 4, false);
    writeToRegFromMem(7, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDI, 4, false);

    //writeToRegFromMem(HOST_DS, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_ADDRESS, 4, false);

    // R12-R15 are non volitile on windows and linux

    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FLAGS, 4, false);
    //popFlagsFromReg(tmpReg, true, true);
    pushNativeReg(tmpReg, true);
    popNativeFlags();
    releaseTmpReg(tmpReg);

    write8(0x41);
    write8(0x0f);
    write8(0xae);
    write8(0x88 | HOST_CPU);
    write32(CPU_OFFSET_FPU_STATE);

    write8(0xc3); // ret
}

void X64Asm::syncRegsToHostCall() {
    U8 tmp = getParamSafeTmpReg();
    writeToRegFromMem(tmp, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SYNC_TO_HOST_ADDRESS, 8, false);
    callHost(nullptr, tmp, true);
}

void X64Asm::syncRegsFromHostCall() {
    U8 tmp = getTmpReg();
    writeToRegFromMem(tmp, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SYNC_FROM_HOST_ADDRESS, 8, false);
    callHost(nullptr, tmp, true);
}

void X64Asm::syncRegsToHost(S8 excludeReg) {
    if (excludeReg == -1) {
        syncRegsToHostCall();
        return;
    }
    if (excludeReg!=0)
        writeToRegFromMem(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EAX, 4, false);
    if (excludeReg!=1)
        writeToRegFromMem(1, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ECX, 4, false);
    if (excludeReg!=2)
        writeToRegFromMem(2, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDX, 4, false);
    if (excludeReg!=3)
        writeToRegFromMem(3, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBX, 4, false);
    if (excludeReg!=4)
        writeToRegFromMem(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESP, 4, false);
    if (excludeReg!=5)
        writeToRegFromMem(5, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBP, 4, false);
    if (excludeReg!=6)
        writeToRegFromMem(6, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESI, 4, false);
    if (excludeReg!=7)
        writeToRegFromMem(7, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDI, 4, false);

    //writeToRegFromMem(HOST_DS, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_ADDRESS, 4, false);

    // R12-R15 are non volitile on windows and linux

    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FLAGS, 4, false);
    pushNativeReg(tmpReg, true);
    popNativeFlags();
    releaseTmpReg(tmpReg);

    // fxrstor
    write8(0x41);
    write8(0x0f);
    write8(0xae);
    write8(0x88 | HOST_CPU);
    write32(CPU_OFFSET_FPU_STATE);  
}

void badStack(CPU* cpu) {
    kpanic("native stack is unaligned");
}

void X64Asm::callHost(void* pfn, U8 tmp, bool internal) {    
    if (tmp == 0xFF) {
        tmp = getParamSafeTmpReg();
    }
#ifdef _DEBUG
    if (!internal) {
        // test rsp,0xf
        write8(0x48);
        write8(0xf7);
        write8(0xc4);
        write32(0xf);

        // jz :1
        write8(0x74);
        U32 pos = this->bufferPos;
        write8(0);
        writeToRegFromValue(tmp, true, (U64)badStack, 8);
        write8(REX_BASE | REX_64);
        write8(0x83);
        write8(0xEC);
        write8(0x20);

        // call tmp
        write8(REX_BASE | REX_MOD_RM);
        write8(0xFF);
        write8(0xd0 | tmp);
        this->buffer[pos] = this->bufferPos - pos - 1;
    }
#endif
    if (!internal) {
        write8(0xfc); // cld
    }
    if (pfn) {
        writeToRegFromValue(tmp, true, (U64)pfn, 8);
    }

#ifdef BOXEDWINE_MSVC
    if (!internal) {
        // part of the x64 windows ABI, shadow store
        // sub rsp, 32
        write8(REX_BASE | REX_64);
        write8(0x83);
        write8(0xEC);
        write8(0x20);
    }
#endif

    // call tmp
    write8(REX_BASE | REX_MOD_RM);
    write8(0xFF);
    write8(0xd0 | tmp);

#ifdef BOXEDWINE_MSVC
    if (!internal) {
        // add rsp, 20
        write8(REX_BASE | REX_64);
        write8(0x83);
        write8(0xC4);
        write8(0x20);
    }
#endif

    releaseTmpReg(tmp);
    unlockParamReg(PARAM_1_REG, PARAM_1_REX);
    unlockParamReg(PARAM_2_REG, PARAM_2_REX);
    unlockParamReg(PARAM_3_REG, PARAM_3_REX);
    unlockParamReg(PARAM_4_REG, PARAM_4_REX);
}

void X64Asm::doIf(U8 reg, bool isRexReg, U32 equalsValue, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock, bool keepFlags, bool generateCmp, bool releaseRegAfterCmp) {
    U8 tmpReg = 0xFF;
    if (keepFlags) {
        tmpReg = getTmpReg();
        pushFlagsToReg(tmpReg, true, true);
    }
    if (generateCmp) {
        // cmp reg, value
        if (isRexReg) {
            write8(REX_BASE | REX_MOD_RM);
        }
        if (equalsValue > 255) {
            write8(0x81);
        } else {
            write8(0x83);
        }
        write8(0xf8 | reg);
        if (equalsValue > 255) {
            write32(equalsValue);
        } else {
            write8((U8)equalsValue);
        }
    }
    if (releaseRegAfterCmp) {
        releaseTmpReg(reg);
    }
    if (keepFlags && elseBlock == nullptr) {
        elseBlock = [] {};
    }
    if (elseBlock == nullptr) {        
        // jnz
        write8(0x75);
        U32 pos = this->bufferPos;
        write8(0);        
        ifBlock();
        if (this->bufferPos - pos - 1 > 127) {
            kpanic("doIf needs some work");
        }
        this->buffer[pos] = this->bufferPos - pos - 1; // else block jumps over if block
    } else {
        // jz 
        write8(0x74);
        U32 pos = this->bufferPos;
        write8(0);

        // ELSE BLOCK
        if (keepFlags) {
            popFlagsFromReg(tmpReg, true, true);
        }
        elseBlock();
        // jmp jb
        bool large = false;
        if (this->firstPass && this->firstPass->needLargeIfJmpReg) {
            large = true;
        }
        write8(large?0xe9:0xeb);
        U32 pos2 = this->bufferPos;
        if (large) {
            write32(0);
        } else {
            write8(0);
        }
        if (this->bufferPos - pos - 1 > 127) {
            kpanic("doIf needs some work");
        }
        this->buffer[pos] = this->bufferPos - pos - 1; // jump to if block
        // IF BLOCK
        if (keepFlags) {
            popFlagsFromReg(tmpReg, true, true);
            releaseTmpReg(tmpReg);
        }
        ifBlock();
        if (!large && this->bufferPos - pos2 - 1 > 127) {
            if (this->firstPass != nullptr) {
                kpanic("doIf needs some work");
            } else {
                this->needLargeIfJmpReg = true;
            }
        }
        if (large) {
            write32Buffer(&this->buffer[pos2], this->bufferPos - pos2 - 4);
        } else {
            this->buffer[pos2] = this->bufferPos - pos2 - 1; // else block jumps over if block
        }
    }
}

void X64Asm::doJmp(bool mightNeedCS) {
    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    jmpReg(tmpReg, true, mightNeedCS);
    releaseTmpReg(tmpReg);
}

// c code
//if (cpu->setSegment(op->reg, cpu->peek16(0))) {
//    ESP = (ESP & cpu->stackNotMask) | ((ESP + 2 ) & cpu->stackMask); 
//    NEXT();
//} else {
//    NEXT_DONE();
//}
void X64Asm::popSeg(U8 seg, U8 bytes) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::setSeg(U8 seg, U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::setSF_onAL(U8 flagReg) {
    /*
    if (AL & 0x80)
        flags|=SF;
    else
        flags&=~SF;
    */
    // test AL, 0x80
    write8(0xa8);
    write8((U8)SF);

    // jz :1
    write8(0x74);
    write8(6);

    // or HOST_TMPb, SF
    write8(REX_BASE|REX_MOD_RM);
    write8(0x80);
    write8(0xC0 | (1<<3) | flagReg);
    write8((U8)SF);
        
    // jmp Jb
    write8(0xeb);
    write8(4);

    // and HOST_TMPb, ~SF
    write8(REX_BASE|REX_MOD_RM);
    write8(0x80);
    write8(0xC0 | (4<<3) | flagReg);
    write8((U8)~(SF));
}

void X64Asm::setZF_onAL(U8 flagReg) {
    /*
    if (AL == 0)
        flags|=ZF;
    else
        flags&=~ZF;
    */
    // test AL, AL
    write8(0x84);
    write8(0xc0);

    // jnz :1
    write8(0x75);
    write8(6);

    // or HOST_TMPb, ZF
    write8(REX_BASE|REX_MOD_RM);
    write8(0x80);
    write8(0xC0 | (1<<3) | flagReg);
    write8((U8)ZF);
        
    // jmp Jb
    write8(0xeb);
    write8(4);

    // and HOST_TMPb, ~ZF
    write8(REX_BASE|REX_MOD_RM);
    write8(0x80);
    write8(0xC0 | (4<<3) | flagReg);
    write8((U8)~(ZF));
}

extern U8 parity_lookup[256];

void X64Asm::setPF_onAL(U8 flagReg) {
    U8 tmpReg = getTmpReg();

    /*
    flags &= ~PF;
    flags |= parity_lookup[AL];
    */

    // and HOST_TMPb, ~PF
    write8(REX_BASE|REX_MOD_RM);
    write8(0x80);
    write8(0xC0 | (4<<3) | flagReg);
    write8((U8)~(PF));

    // mov HOST_TMP2, parity_lookup
    write8(REX_BASE | REX_64 | REX_MOD_RM);
    write8(0xb8+tmpReg);
    write64((U64)parity_lookup);
    
    // or HOST_TMPb, byte ptr [HOST_TMP2]
    write8(REX_BASE | REX_MOD_REG | REX_MOD_RM);
    write8(0x0a);
    write8((flagReg << 3) | tmpReg);

    releaseTmpReg(tmpReg);
}

void X64Asm::daa() {
    U32 tmpReg = getTmpReg();
    U32 tmpReg2 = getTmpReg();

    // pushFlagsToReg can not be used if the flags need to be modified
    pushNativeFlags();
    popNativeReg(tmpReg, true);

    /*
    if (flags & CF) {
        AL+=0x60;
    } else if (AL > 0x99) {
        AL+=0x60;
        addFlag(CF);
    }
    */
        // test HOST_TMP, CF
        write8(REX_BASE | REX_MOD_RM);
        write8(0xF6);
        write8(0xC0 | tmpReg);
        write8(CF);

        // jnz :1
        write8(0x75);
        write8(8);

        // cmp al, 0x99
        write8(0x3c);
        write8(0x99);
        
        // JBE :2
        write8(0x76);
        write8(6);

        // OR HOST_TMP, CF
        write8(REX_BASE | REX_MOD_RM);
        write8(0x80);
        write8(0xC0 | (1<<3) | tmpReg);
        write8(CF);
    
        // 1:
        // add al, 0x60
        write8(0x04);
        write8(0x60);

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
        write8(REX_BASE | REX_MOD_RM);
        write8(0xF6);
        write8(0xC0 | tmpReg);
        write8((U8)AF);

        // jnz :1
        write8(0x75);
        write8(17);

        // mov HOST_TMP2, eax
        write8(REX_BASE | REX_MOD_RM);
        write8(0x89);
        write8(0xC0 | tmpReg2);

        // and HOST_TMP2, 0x0F
        write8(REX_BASE | REX_MOD_RM);
        write8(0x80);
        write8(0xC0 | (4<<3) | tmpReg2);
        write8(0xf);

        // cmp HOST_TMP2b, 0x09
        write8(REX_BASE | REX_MOD_RM);
        write8(0x80);
        write8(0xC0 | (7 << 3) | tmpReg2);
        write8(0x09);

        // jbe :2
        write8(0x76);
        write8(6);

        // OR HOST_TMP, AF
        write8(REX_BASE | REX_MOD_RM);
        write8(0x80);
        write8(0xC0 | (1<<3) | tmpReg);
        write8((U8)AF);
    
        // 1:
        // add al, 0x06
        write8(0x04);
        write8(0x06);

        // 2:
    
    releaseTmpReg(tmpReg2);

    setSF_onAL(tmpReg);
    setZF_onAL(tmpReg);
    setPF_onAL(tmpReg);
    
    pushNativeReg(tmpReg, true);
    releaseTmpReg(tmpReg);
    popNativeFlags();
}

// :TODO: maybe make native versions
void X64Asm::das() {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::aaa() {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::aas() {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::aad(U8 value) {
    emulateSingleOp(currentOp);
}

/*
AH = AL / value;
AL = AL % value;
cpu->lazyFlags = FLAGS_NONE;
cpu->setSF(AL & 0x80);
cpu->setZF(AL == 0);		
cpu->setPFonValue(AL);
cpu->removeCF();
cpu->removeOF();
cpu->removeAF();   
*/
void X64Asm::aam(U8 value) {
    U32 tmpReg = getTmpReg();
    U32 tmpReg2 = getTmpReg();

    if (value==0) {
        kpanic("X64Asm::aam divide by 0 not handled");
    }
    pushFlagsToReg(tmpReg, true, true);

    // mov tmpReg2, value
    write8(0x66);
    write8(0x41);
    write8(0xb8+tmpReg2);
    write16(value);

    // movzx  ax,al 
    write8(0x66);
    write8(0x0f);
    write8(0xb6);
    write8(0xc0);

    // div tmpReg
    write8(0x41);
    write8(0xf6);
    write8(0xf0 | tmpReg2);

    releaseTmpReg(tmpReg2);

    // xchg al, ah
    write8(0x86);
    write8(0xe0);

    setSF_onAL(tmpReg);
    setZF_onAL(tmpReg);
    setPF_onAL(tmpReg);
    
    // AND HOST_TMP, ~(CF|AF|OF)
    write8(REX_BASE | REX_MOD_RM);
    write8(0x80);
    write8(0xC0 | (4<<3) | tmpReg);
    write8((U8)(~(CF|AF|OF)));

    popFlagsFromReg(tmpReg, true, true);
    releaseTmpReg(tmpReg);    
}

void X64Asm::salc() {
    // save flags since we don't want sbb to affect them
    U8 tmpReg1 = getTmpReg();
    pushFlagsToReg(tmpReg1, true, true);
    
    // sbb al, al
    write8(0x18);
    write8(0xc0);
    
    popFlagsFromReg(tmpReg1, true, true);
    releaseTmpReg(tmpReg1);
}

void X64Asm::incReg(U8 reg, bool isRegRex, U8 bytes) {
    U8 rex = 0;
    if (bytes == 2) {
        write8(0x66);
    }
    if (isRegRex) 
        rex |= REX_BASE | REX_MOD_RM;
    if (bytes == 8)
        rex |= REX_BASE | REX_64;
    if (rex)
        write8(rex);
    if (bytes == 1) 
        write8(0xFE);
    else
        write8(0xFF);
    write8(0xC0+reg);
}

void X64Asm::decReg(U8 reg, bool isRegRex, U8 bytes) {
    U8 rex = 0;
    if (bytes == 2) {
        write8(0x66);
    }
    if (isRegRex) 
        rex |= REX_BASE | REX_MOD_RM;
    if (bytes == 8)
        rex |= REX_BASE | REX_64;
    if (rex)
        write8(rex);
    if (bytes == 1) 
        write8(0xFE);
    else
        write8(0xFF);
    write8(0xC8+reg);
}

void X64Asm::pushA16() {
    U8 tmpReg = getTmpReg();
    U8 ssReg = getRegForSeg(SS, tmpReg);
    writeToMemFromReg(0, false, HOST_ESP, true, ssReg, true, 0, -2, 2, true, true);
    writeToMemFromReg(1, false, HOST_ESP, true, ssReg, true, 0, -4, 2, true, true);
    writeToMemFromReg(2, false, HOST_ESP, true, ssReg, true, 0, -6, 2, true, true);
    writeToMemFromReg(3, false, HOST_ESP, true, ssReg, true, 0, -8, 2, true, true);
    writeToMemFromReg(HOST_ESP, true, HOST_ESP, true, ssReg, true, 0, -10, 2, true, true);
    writeToMemFromReg(5, false, HOST_ESP, true, ssReg, true, 0, -12, 2, true, true);
    writeToMemFromReg(6, false, HOST_ESP, true, ssReg, true, 0, -14, 2, true, true);
    writeToMemFromReg(7, false, HOST_ESP, true, ssReg, true, 0, -16, 2, true, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -16, 2);
    releaseTmpReg(tmpReg);
}

// POPA
void X64Asm::popA16() {
    U8 tmpReg = getTmpReg();
    U8 ssReg = getRegForSeg(SS, tmpReg);
    writeToRegFromMem(7, false, HOST_ESP, true, ssReg, true, 0, 0, 2, true, true);
    writeToRegFromMem(6, false, HOST_ESP, true, ssReg, true, 0, 2, 2, true, true);
    writeToRegFromMem(5, false, HOST_ESP, true, ssReg, true, 0, 4, 2, true, true);
    // SP isn't pop, but the stack will be adjusted
    // writeToRegFromMem(4, false, HOST_ESP, true, ssReg, true, 0, 6, 2, true);
    writeToRegFromMem(3, false, HOST_ESP, true, ssReg, true, 0, 8, 2, true, true);
    writeToRegFromMem(2, false, HOST_ESP, true, ssReg, true, 0, 10, 2, true, true);
    writeToRegFromMem(1, false, HOST_ESP, true, ssReg, true, 0, 12, 2, true, true);
    writeToRegFromMem(0, false, HOST_ESP, true, ssReg, true, 0, 14, 2, true, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, 16, 2);
    releaseTmpReg(tmpReg);
}

// PUSHAD
void X64Asm::pushA32() {
    S8 seg = 0;
    U8 tmpReg = getTmpReg();

    if (this->cpu->thread->process->hasSetSeg[SS]) {
        seg = getRegForSeg(SS, tmpReg);
    } else {
        seg = -1;
    }
    writeToMemFromReg(0, false, HOST_ESP, true, seg, true, 0, -4, 4, true, true);
    writeToMemFromReg(1, false, HOST_ESP, true, seg, true, 0, -8, 4, true, true);
    writeToMemFromReg(2, false, HOST_ESP, true, seg, true, 0, -12, 4, true, true);
    writeToMemFromReg(3, false, HOST_ESP, true, seg, true, 0, -16, 4, true, true);
    writeToMemFromReg(HOST_ESP, true, HOST_ESP, true, seg, true, 0, -20, 4, true, true);
    writeToMemFromReg(5, false, HOST_ESP, true, seg, true, 0, -24, 4, true, true);
    writeToMemFromReg(6, false, HOST_ESP, true, seg, true, 0, -28, 4, true, true);
    writeToMemFromReg(7, false, HOST_ESP, true, seg, true, 0, -32, 4, true, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -32, 4);

    releaseTmpReg(tmpReg);
}

// POPAD
void X64Asm::popA32() {
    S8 seg = 0;
    U8 tmpReg = getTmpReg();

    if (this->cpu->thread->process->hasSetSeg[SS]) {
        seg = getRegForSeg(SS, tmpReg);
    } else {
        seg = -1;
    }
    writeToRegFromMem(7, false, HOST_ESP, true, seg, true, 0, 0, 4, true, true);
    writeToRegFromMem(6, false, HOST_ESP, true, seg, true, 0, 4, 4, true, true);
    writeToRegFromMem(5, false, HOST_ESP, true, seg, true, 0, 8, 4, true, true);
    // SP isn't pop, but the stack will be adjusted
    // writeToRegFromMem(4, false, HOST_ESP, true, HOST_SS, true, 0, 12, 4, true);
    writeToRegFromMem(3, false, HOST_ESP, true, seg, true, 0, 16, 4, true, true);
    writeToRegFromMem(2, false, HOST_ESP, true, seg, true, 0, 20, 4, true, true);
    writeToRegFromMem(1, false, HOST_ESP, true, seg, true, 0, 24, 4, true, true);
    writeToRegFromMem(0, false, HOST_ESP, true, seg, true, 0, 28, 4, true, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, 32, 4);

    releaseTmpReg(tmpReg);
}

void X64Asm::jumpConditional(U8 condition, U32 eip) {    
    if (this->stopAfterInstruction!=(S32)this->ipAddressCount && (this->calculatedEipLen==0 || (eip>=this->startOfDataIp && eip<this->startOfDataIp+this->calculatedEipLen))) {
        write8(0x0F);
        write8(0x80+condition);
        write32(0);
        addTodoLinkJump(eip);
    } else {
        write8(0x70+condition);
        doLoop(eip);
    }
}

void X64Asm::addTodoLinkJump(U32 eip) {
    this->todoJump.push_back(TodoJump(eip, this->bufferPos-4, this->ipAddressCount));
}

void X64Asm::jumpTo(U32 eip) {  

    if (!this->cpu->isBig()) {
        eip = eip & 0xffff;
    }
#ifdef _DEBUG
    //this->writeToMemFromValue(this->startOfOpIp, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP_FROM, 4, false);
#endif
    // :TODO: is this necessary?  who uses it?
    //this->writeToMemFromValue(eip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    if (this->stopAfterInstruction!=(S32)this->ipAddressCount && (this->calculatedEipLen==0 || (eip>=this->startOfDataIp && eip<this->startOfDataIp+this->calculatedEipLen))) {
        write8(0xE9);
        write32(0);
        addTodoLinkJump(eip);
    } else {
        writeToRegFromValue(HOST_TMP, true, eip, 4);
        jmpReg(HOST_TMP, true, true);
    }
}

void x64_changed(x64CPU* cpu) {
    kpanic("Self modifying code was not trapped");
}

void X64Asm::xchange4(U8 reg1, bool isRexReg1, U8 reg2, bool isRexReg2) {
    if (isRexReg1) {
        write8(REX_BASE|REX_MOD_RM);
    }
    if (isRexReg2) {
        kpanic("X64Asm::xchange4 doesn't support isRexReg2");
    }
    if (reg2==0 && !isRexReg2) {
        write8(0x90|reg1);
    } else {
        kpanic("X64Asm::xchange4 reg2 must be eax");
    }
}

void X64Asm::pushFlagsToReg(U8 reg, bool isRexReg, bool includeOF) {
    if (includeOF) {
        writeToRegFromReg(reg, isRexReg, 0, false, 4);
        // seto al
        write8(0x0f);
        write8(0x90);
        write8(0xc0);
        // lahf
        write8(0x9f);
        xchange4(reg, isRexReg, 0, false);
    } else  {
        writeToRegFromReg(reg, isRexReg, 0, false, 4);
        // lahf
        write8(0x9f);
        xchange4(reg, isRexReg, 0, false);
    }
}

void X64Asm::popFlagsFromReg(U8 reg, bool isRexReg, bool includeOF) {
    if (includeOF) {
        xchange4(reg, isRexReg, 0, false);
        // add al, 127 (will restore OF)
        write8(0x04);
        write8(0x7f);
        // sahf
        write8(0x9e);
        xchange4(reg, isRexReg, 0, false);
    } else  {
        xchange4(reg, isRexReg, 0, false);
        // sahf
        write8(0x9e);
        xchange4(reg, isRexReg, 0, false);
    }
}

void X64Asm::doLoop(U32 eip) {
    // :TODO: maybe find one byte offset
    U32 pos = this->bufferPos;
    write8(0); // skip over the next jmp
    jumpTo(this->ip); // jump to next instruction after this because condition was not met
    if (this->bufferPos - pos - 1 > 127) {
        kpanic("X64Asm::doLoop bad jmp");
    }
    this->buffer[pos] = this->bufferPos-pos-1;
    jumpTo(eip); // jmp to offset because condition was met
}

void X64Asm::jcxz(U32 eip, bool ea16) {
    if (!ea16) {
        write8(0xe3);    
        doLoop(eip);
    } else {
        // test cx, cx
        write8(0x66);
        write8(0x85);
        write8(0xc9);
        // jz 
        write8(0x75);
        U32 pos = this->bufferPos;
        write8(0);
        jumpTo(eip);
        if (this->bufferPos-pos-1>127) {
            kpanic("X64Asm::jcxz tried to jump too far");
        }
        this->buffer[pos] = this->bufferPos-pos-1;
    }   
}

void X64Asm::loop(U32 eip, bool ea16) {
    if (!ea16) {
        write8(0xe2);    
        doLoop(eip);
    } else {
        U8 tmpReg = getTmpReg();
        pushFlagsToReg(tmpReg, true, true);
        decReg(1, false, 2);
        // test cx, cx
        write8(0x66);
        write8(0x85);
        write8(0xc9);
        // jz 
        write8(0x74);
        U32 pos = this->bufferPos;
        write8(0);
        popFlagsFromReg(tmpReg, true, true);
        releaseTmpReg(tmpReg);
        jumpTo(eip);
        if (this->bufferPos-pos-1>127) {
            kpanic("X64Asm::loop tried to jump too far");
        }
        this->buffer[pos] = this->bufferPos-pos-1;
        popFlagsFromReg(tmpReg, true, true);
    }
}

void X64Asm::loopz(U32 eip, bool ea16) {
    if (ea16) {
        doLoop16(0xe1, eip);
    } else {
        write8(0xe1);    
        doLoop(eip);
    }
}

void X64Asm::doLoop16(U8 inst, U32 eip) {
    // mov r8d, ECX
    write8(0x41);
    write8(0x89);
    write8(0xc8);

    // movzx ECX, CX
    zeroExtend16to32(1, false, 1, false);

    // loopnz 1:
    write8(inst); 
    write8(2); 

    // jmp 2:
    write8(0xeb);
    U32 pos2 = this->bufferPos;
    write8(0);

    // 1:
    // mov r8w, CX
    write8(0x66);
    write8(0x41);
    write8(0x89);
    write8(0xc8);

    // mov ECX, r8d
    write8(0x44);
    write8(0x89);
    write8(0xc1);

    // jmp eip
    jumpTo(eip);

    // 2: 
    this->buffer[pos2] = this->bufferPos - pos2 -1;

    // mov r8w, CX
    write8(0x66);
    write8(0x41);
    write8(0x89);
    write8(0xc8);

    // mov ECX, r8d
    write8(0x44);
    write8(0x89);
    write8(0xc1);

    // continue to next instruction
}
void X64Asm::loopnz(U32 eip, bool ea16) {
    if (ea16) {
        doLoop16(0xe0, eip);
    } else {
        write8(0xe0);    
        doLoop(eip);
    }    
}

void x64_jmp(x64CPU* cpu, U32 big, U32 selector, U32 offset) {
    cpu->jmp(big, selector, offset, cpu->arg5);
}

void X64Asm::jmp(bool big, U32 sel, U32 offset, U32 oldEip) {
    emulateSingleOp(currentOp);
    done = true;
}

void x64_call(x64CPU* cpu, U32 big, U32 selector, U32 offset) {
    cpu->call(big, selector, offset, cpu->arg5);
}

// common_jmp(cpu, false, sel, offset, oldEip);
void X64Asm::call(bool big, U32 sel, U32 offset, U32 oldEip) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::shiftRightReg(U8 reg, bool isRegRex, U8 shiftAmount, bool arith) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0xC1);
    if (arith) {
        write8(0xF8 | reg);
    } else {
        write8(0xE8 | reg);
    }
    write8(shiftAmount);
}

void X64Asm::shiftLeftReg(U8 reg, bool isRegRex, U8 shiftAmount) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0xC1);
    write8(0xE0 | reg);
    write8(shiftAmount);
}

void X64Asm::andReg(U8 reg, bool isRegRex, U32 mask) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0x81);
    write8(0xE0 | reg);
    write32(mask);
}

void X64Asm::orReg(U8 reg, bool isRegRex, U32 mask) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0x81);
    write8(0xC8 | reg);
    write32(mask);
}

void X64Asm::subRegs(U8 dst, bool isDstRex, U8 src, bool isSrcRex, bool is64) {
    if (isDstRex || isSrcRex || is64) {
        U8 rex = REX_BASE;
        if (isDstRex) {
            rex |= REX_MOD_RM;
        }
        if (isSrcRex) {
            rex |= REX_MOD_REG;
        }
        if (is64) {
            rex |= REX_64;
        }
        write8(rex);
    }
    write8(0x29);
    write8(0xC0 | dst | (src << 3));
}

// :TODO: what about making call/ret pairs
// call will push eip/rip into circular buffer
// call will goto to a prelogue and adjust the stack so we don't worry about it
// ret will push the location we are going to after checking the eip/rip buffer

// don't use x64_getTmpReg here, it is important that the exact reg is used for each instruction since
// the exception handler will look for it

// with BMI2
//41 BA 0C 00 00 00    mov         r10d,0Ch  
//C4 42 AB F7 C1       shrx        r8,r9,r10                 // get page
//41 BA FF 0F 00 00    mov         r10d,0FFFh  
//C4 42 B2 F5 CA       pext        r9,r9,r10                 // get offset
//4D 8B 95 98 04 00 00 mov         r10,qword ptr [r13+498h]  // mov eipToHostInstruction to a reg
//4F 8B 14 C2          mov         r10,qword ptr [r10+r8*8]  // get the page from eipToHostInstruction
//4F 8B 14 CA          mov         r10,qword ptr [r10+r9*8]  // get the host location from page[offset]
//66 45 8B 0A          mov         r9w,word ptr [r10]        // just to make sure the host location is not 0
//41 FF E2             jmp         r10  

// 43 FF 24 CE          jmp         qword ptr[r14 + r9 * 8]

void X64Asm::jmpReg(U8 reg, bool isRex, bool mightNeedCS) {     
    if (reg != 7 || !isRex) {
        writeToRegFromReg(7, true, reg, isRex, 4);
    }
    bool needFlags = true; // need to figure out a way to look at both branches we might jump to

    // HOST_TMP2 will hold the page
    // HOST_TMP will hold the offset
    if (x64CPU::hasBMI2) {
        if (!this->cpu->thread->process->hasSetSeg[CS] && !mightNeedCS) {
            if (reg == HOST_TMP2 && isRex) {
                writeToRegFromValue(HOST_TMP3, true, K_PAGE_MASK, 4);
                // PEXT HOST_TMP, HOST_TMP2, HOST_TMP3
                write8(0xc4);
                write8(0x42);
                write8(0xba);
                write8(0xf5);
                write8(0xca);

                writeToRegFromValue(HOST_TMP3, true, 12, 4);
                // shrx HOST_TMP2,HOST_TMP2,12 
                write8(0xc4);
                write8(0x42);
                write8(0xab); // :TODO: how does this encode r10 ?
                write8(0xf7);
                write8(0xc0 | (HOST_TMP2 << 3) | HOST_TMP2);
            } else if (reg == HOST_TMP && isRex) {
                writeToRegFromValue(HOST_TMP3, true, 12, 4);
                // shrx HOST_TMP2,HOST_TMP,12 
                write8(0xc4);
                write8(0x42);
                write8(0xab); // :TODO: how does this encode r10 ?
                write8(0xf7);
                write8(0xc0 | (HOST_TMP2 << 3) | HOST_TMP);

                writeToRegFromValue(HOST_TMP3, true, K_PAGE_MASK, 4);
                // PEXT HOST_TMP, HOST_TMP, HOST_TMP3
                write8(0xc4);
                write8(0x42);
                write8(0xb2);
                write8(0xf5);
                write8(0xca);
            } else {
                // mov HOST_TMP2, reg
                writeToRegFromReg(HOST_TMP2, true, reg, isRex, false);

                writeToRegFromValue(HOST_TMP3, true, K_PAGE_MASK, 4);
                // PEXT HOST_TMP, HOST_TMP2, HOST_TMP3
                write8(0xc4);
                write8(0x42);
                write8(0xba);
                write8(0xf5);
                write8(0xca);

                writeToRegFromValue(HOST_TMP3, true, 12, 4);
                // shrx HOST_TMP2,HOST_TMP2,12 
                write8(0xc4);
                write8(0x42);
                write8(0xab); // :TODO: how does this encode r10 ?
                write8(0xf7);
                write8(0xc0 | (HOST_TMP2 << 3) | HOST_TMP2);
            }
        } else {
            writeToRegFromValue(HOST_TMP3, true, K_PAGE_MASK, 4);
            if (reg == HOST_TMP2 && isRex) {
                getRegForSeg(CS, HOST_TMP);
                addWithLea(HOST_TMP2, true, HOST_TMP2, true, HOST_TMP, true, 0, 0, 4);
            } else if (reg == HOST_TMP && isRex) {
                getRegForSeg(CS, HOST_TMP2);
                addWithLea(HOST_TMP2, true, reg, isRex, HOST_TMP2, true, 0, 0, 4);
            }

            // PEXT HOST_TMP, HOST_TMP2, HOST_TMP3
            write8(0xc4);
            write8(0x42);
            write8(0xba);
            write8(0xf5);
            write8(0xca);

            writeToRegFromValue(HOST_TMP3, true, 12, 4);
            // shrx HOST_TMP2,HOST_TMP2,12 
            write8(0xc4);
            write8(0x42);
            write8(0xab); // :TODO: how does this encode r10 ?
            write8(0xf7);
            write8(0xc0 | (HOST_TMP2 << 3) | HOST_TMP2);
        }
    } else {
        if (!this->cpu->thread->process->hasSetSeg[CS] && !mightNeedCS) {
            if (reg == HOST_TMP2 && isRex) {
                writeToRegFromReg(HOST_TMP, true, HOST_TMP2, true, 4);
            } else if (reg == HOST_TMP && isRex) {
                writeToRegFromReg(HOST_TMP2, true, HOST_TMP, true, 4);
            } else {
                writeToRegFromReg(HOST_TMP, true, reg, isRex, 4);
                writeToRegFromReg(HOST_TMP2, true, reg, isRex, 4);
            }
        } else {
            if (reg == HOST_TMP2 && isRex) {
                getRegForSeg(CS, HOST_TMP);
                addWithLea(HOST_TMP2, true, HOST_TMP2, true, HOST_TMP, true, 0, 0, 4);
            } else {
                getRegForSeg(CS, HOST_TMP2);
                addWithLea(HOST_TMP2, true, reg, isRex, HOST_TMP2, true, 0, 0, 4);
            }
            writeToRegFromReg(HOST_TMP, true, HOST_TMP2, true, 4);
        }

        // Breakdown setup will not work without preserving these flags
        if (needFlags) {
            pushFlagsToReg(HOST_TMP3, true, true);
        }
        // shr HOST_TMP2, 12
        shiftRightReg(HOST_TMP2, true, K_PAGE_SHIFT);

        // and HOST_TMP, 0xFFF
        andReg(HOST_TMP, true, K_PAGE_MASK);
        if (needFlags) {
            popFlagsFromReg(HOST_TMP3, true, true);
        }
    }

    // HOST_TMP3=cpu->opToAddressPages
    // mov HOST_TMP3, [HOST_CPU + CPU_OFFSET_OP_PAGES];
    writeToRegFromMem(HOST_TMP3, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_OP_PAGES, 8, false);

    // HOST_TMP3 = cpu->opToAddressPages[page]
    // mov HOST_TMP3, [HOST_TMP3+HOST_TEMP2<<3] // 3 sizeof(void*)
    writeToRegFromMem(HOST_TMP3, true, HOST_TMP3, true, HOST_TMP2, true, 3, 0, 8, false);

    if (needFlags) {
        pushFlagsToReg(HOST_TMP2, true, true);
    }
    doIf(HOST_TMP3, true, 0, [=, this] {          
        if (needFlags) {
            popFlagsFromReg(HOST_TMP2, true, true);
        }
        writeToRegFromMem(0, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_JMP_AND_TRANSLATE_IF_NECESSARY, 8, false);
        jmpNativeReg(0, true);
        }, nullptr);

    // will move address to HOST_TMP3 and test that it exists, if it doesn't then we
    // will catch the exception.  We leave the address/index we need in HOST_TMP
    // and HOST_TMP2

    // HOST_TMP3 = cpu->opToAddressPages[page][offset]
    // mov HOST_TMP3, [HOST_TMP3 + HOST_TMP << 3]
    writeToRegFromMem(HOST_TMP3, true, HOST_TMP3, true, HOST_TMP, true, 3, 0, 8, false);

    doIf(HOST_TMP3, true, 0, [=, this] {
        if (needFlags) {
            popFlagsFromReg(HOST_TMP2, true, true);
        }
        writeToRegFromMem(0, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_JMP_AND_TRANSLATE_IF_NECESSARY, 8, false);
        jmpNativeReg(0, true);
        }, nullptr);
    if (needFlags) {
        popFlagsFromReg(HOST_TMP2, true, true);
    }      

    // jmp HOST_TMP
    jmpNativeReg(HOST_TMP3, true);
}

void X64Asm::jmpNativeReg(U8 reg, bool isRegRex) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0xff);
    write8((0x04 << 3) | 0xC0 | reg);
}

void X64Asm::zeroExtend16to32(U8 reg, bool isRegRex, U8 fromReg, bool isFromRex) {
    if (isFromRex || isRegRex) {
        write8(REX_BASE | (isFromRex ? REX_MOD_RM : 0) | (isRegRex ? REX_MOD_REG : 0));
    }
    write8(0x0f);
    write8(0xb7);
    write8(0xc0 | (reg << 3) | fromReg);
}

void X64Asm::retn16(U32 bytes) {
    U32 tmpReg = getTmpReg();
    popReg16(tmpReg, true);
    zeroExtend16to32(tmpReg, true, tmpReg, true);

    if (bytes) {
        addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, bytes, 2);
    }
    jmpReg(tmpReg, true, false);
    releaseTmpReg(tmpReg);
}

void X64Asm::retn32(U32 bytes) {
    U32 tmpReg = getTmpReg();
    popReg32(tmpReg, true);
    if (bytes) {
        addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, bytes, 4);
    }
    jmpReg(tmpReg, true, false);
    releaseTmpReg(tmpReg);
}

void X64Asm::retf(U32 big, U32 bytes) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::iret(U32 big, U32 oldEip) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::signalIllegalInstruction(int code) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::signalTrap(int code) {
    emulateSingleOp(currentOp);
    done = true;
}

static U8 fetchByte(void* p, U32 *eip) {
    KMemory* memory = (KMemory*)p;
    return memory->readb((*eip)++);
}

static void x64log(CPU* cpu) {
    if (!cpu->logFile.isOpen())
        return;
    thread_local static DecodedBlock* block = new DecodedBlock();
    decodeBlock(fetchByte, cpu->memory, cpu->eip.u32+cpu->seg[CS].address, cpu->isBig(), 1, K_PAGE_SIZE, 0, block);
    block->op->log(cpu);
    block->op->dealloc(false);
}

void X64Asm::logOp(U32 eip) {
    syncRegsFromHost(); 

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param    

    callHost((void*)x64log);

    syncRegsToHost();
}

void X64Asm::syscall(U32 opLen) {
    syncRegsFromHost();  

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)currentOp, 8);
    writeToMemFromReg(PARAM_2_REG, PARAM_2_REX, HOST_CPU, true, -1, false, 0, CPU_OFFSET_CURRENT_OP, 8, false);
    writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);
    callHost((void*)common_runSingleOp);

    syncRegsToHost();
	
	U8 tmpReg = getTmpReg();
	writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EXIT_TO_START_LOOP, 4, false);
	doIf(tmpReg, true, 1, [this]() {
		// jmp [HOST_CPU+returnToLoopAddress]
		write8(0x41);
		write8(0xff);
		write8(0xa0 | HOST_CPU);
		write32(CPU_OFFSET_RETURN_ADDRESS);
		}, []() {
		}
	);
	releaseTmpReg(tmpReg);
    doJmp(true);
}

void X64Asm::int98(U32 opLen) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::int99(U32 opLen) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::int9A(U32 opLen) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::writeXchgEspEax() {
    write8(REX_MOD_RM|REX_BASE);
    write8(0x90+HOST_ESP);
}

void X64Asm::writeXchgSpAx() {
    write8(0x66);
    write8(REX_MOD_RM|REX_BASE);    
    write8(0x90+HOST_ESP);
}

void X64Asm::bswapEsp() {
    write8(REX_BASE | REX_MOD_RM);
    write8(0xC8+HOST_ESP);
}

void X64Asm::bswapSp() {
    write8(0x66);
    write8(REX_BASE | REX_MOD_RM);
    write8(0xC8+HOST_ESP);
}

void X64Asm::DsEdiMmxOrSSE(U8 rm) {
    emulateSingleOp(currentOp);
}

// U16 val = readw(eaa);
// U32 selector = readw(eaa+2);
// if (cpu->setSegment(op->imm, selector)) {
//    cpu->reg[op->reg].u16 = val;
void X64Asm::loadSeg(U8 seg, U8 rm, bool b32) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::enter(bool big, U32 bytes, U32 level) {
    if (level!=0) {
        emulateSingleOp(currentOp);
        done = true;
    } else {
        if (big) {
            // push EBP
            pushReg32(5, false);

            //EBP = ESP;
            writeToRegFromReg(5, false, HOST_ESP, true, 4);
        } else {
            // push BP
            pushReg16(5, false);

            //BP = SP;
            writeToRegFromReg(5, false, HOST_ESP, true, 2);
        }        
    
        //sub  esp, bytes
        if (bytes) {
            U8 tmpReg = getTmpReg();
            adjustStack(tmpReg, -((S32)bytes));
            releaseTmpReg(tmpReg);
        }
    }
}

void X64Asm::leave(bool big) {
    //SP = BP;
    writeToRegFromReg(HOST_ESP, true, 5, false, (big?4:2));
    
    //pop BP
    popReg(5, false, (big?4:2), true);  
}

void X64Asm::callE(bool big, U8 rm) {
    U8 tmpReg = getTmpReg();
    writeToRegFromE(tmpReg, true, rm, (big?4:2));
    if (!big) {
        zeroExtend16to32(tmpReg, true, tmpReg, true);
    }
    push(-1, false, this->ip, (big?4:2)); 
    jmpReg(tmpReg, true, false);
    releaseTmpReg(tmpReg);
}

void X64Asm::callJmp(bool big, U8 rm, bool jmp) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::callFar(bool big, U8 rm) {    
    callJmp(big, rm, false);   
}

void X64Asm::jmpE(bool big, U8 rm) {
    U8 tmpReg = getTmpReg();
    writeToRegFromE(tmpReg, true, rm, (big?4:2));
    if (!big) {
        zeroExtend16to32(tmpReg, true, tmpReg, true);
    }
    jmpReg(tmpReg, true, false);
    releaseTmpReg(tmpReg);
}

void X64Asm::jmpFar(bool big, U8 rm) {
    callJmp(big, rm, true);       
}

// call back signature
// void OPCALL onExitSignal(CPU* cpu, DecodedOp* op) 
void X64Asm::callCallback(void* pfn) {
    syncRegsFromHost();

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, 0, 4);

    callHost((void*)pfn);
    syncRegsToHost();
    doJmp(true);
}

void X64Asm::lsl(bool big, U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::lar(bool big, U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::verw(U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::verr(U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

static void x64_invalidOp(CPU* cpu, U32 op) {
    klog("x64_invalidOp: 0x%X", op);
    BString name = cpu->thread->process->getModuleName(cpu->seg[CS].address + cpu->eip.u32);
    klog("%.8X EAX=%.8X ECX=%.8X EDX=%.8X EBX=%.8X ESP=%.8X EBP=%.8X ESI=%.8X EDI=%.8X %s at %.8X\n", cpu->seg[CS].address + cpu->eip.u32, cpu->reg[0].u32, cpu->reg[1].u32, cpu->reg[2].u32, cpu->reg[3].u32, cpu->reg[4].u32, cpu->reg[5].u32, cpu->reg[6].u32, cpu->reg[7].u32, name.c_str(), cpu->thread->process->getModuleEip(cpu->seg[CS].address + cpu->eip.u32));
    cpu->thread->signalIllegalInstruction(5);
}

void X64Asm::invalidOp(U32 op) {
    emulateSingleOp(currentOp);
    done = true;
}

static void x64_errorMsg(const char* msg) {
    kpanic("%s", msg);
}

void X64Asm::errorMsg(const char* msg) {
    //syncRegsFromHost(); 
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromValue(PARAM_1_REG, PARAM_1_REX, (U64)msg, 8);
    callHost((void*)x64_errorMsg);
    //syncRegsToHost();
    //doJmp();
}

void X64Asm::cpuid() {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::setNativeFlags(U32 flags, U32 mask) {
    U8 tmpReg = getTmpReg();
    pushNativeFlags();
    popNativeReg(tmpReg, true);

    // and tmpReg, mask
    write8(REX_BASE | REX_MOD_RM);
    write8(0x81);
    write8(0xe0 | tmpReg);
    write32(~mask);

    // or tmpReg, (flags & mask)
    write8(REX_BASE | REX_MOD_RM);
    write8(0x81);
    write8(0xc8 | tmpReg);
    write32((flags & mask));

    pushNativeReg(tmpReg, true);
    popNativeFlags();
    releaseTmpReg(tmpReg);
}

void X64Asm::writeOp(bool isG8bit) {
    U8 g8 = 0xFF;
    U8 g8Temp = 0;
    U8 tmpReg=0;
    bool isTmpRegAllocated = false;
    
    if (this->rex && this->has_rm && isG8bit && G(this->rm)>=4) {        
        tmpReg = getTmpReg();
        isTmpRegAllocated = true;

        g8=G(this->rm);
        g8Temp = (g8==4?1:0);

        // push rax
        pushNativeReg(g8Temp, false);

        // mov al, g8
        writeToRegFromReg(g8Temp, false, g8, false, 1);

        // mov HOST_TMP2, eax
        writeToRegFromReg(tmpReg, true, g8Temp, false, 4);

        // pop rax
        popNativeReg(g8Temp, false);

        this->rex |= REX_BASE | REX_MOD_REG;
        this->rm &=~ (0x7 << 3);
        this->rm |= (tmpReg << 3);        
    }

    if (this->lockPrefix)
        write8(0xF0);
    if (this->cpu->isBig() && this->operandPrefix)
        write8(0x66);
    else if (!this->cpu->isBig() && !this->operandPrefix)
        write8(0x66);
    if (this->repZeroPrefix)
        write8(0xF3);
    if (this->repNotZeroPrefix)
        write8(0xF2);

    if (this->rex)
        write8(this->rex);           
    
    if (this->multiBytePrefix)
        write8(0x0F);

    write8(this->op);
    if (this->has_rm) {
        write8(this->rm);
    }
    if (this->has_sib) {
        write8(this->sib);
    }
    if (this->dispSize==8) {
        write8((U8)this->disp);
    } else if (this->dispSize==16) {
        write16((U16)this->disp);
    } else if (this->dispSize==32) {
        write32(this->disp);
    }
    if (this->immSize==8) {
        write8((U8)this->imm);
    } else if (this->immSize==16) {
        write16((U16)this->imm);
    } else if (this->immSize==32) {
        write32(this->imm);
    }
    if (this->isG8bitWritten && g8!=0xFF) {
        // push rax
        pushNativeReg(g8Temp, false);

        // mov eax, HOST_TMP2
        writeToRegFromReg(g8Temp, false, tmpReg, true, 4);

        // mov g8, al
        writeToRegFromReg(g8, false, g8Temp, false, 1);

        // pop rax
        popNativeReg(g8Temp, false);
    }
    if (isTmpRegAllocated) {
        releaseTmpReg(tmpReg);
    }
}


typedef void (*pfnStringNoArgs)(CPU* cpu);

void x64_stringNoArgs(x64CPU* cpu, pfnStringNoArgs pfn, U32 size) {
    pfn(cpu);
    cpu->fillFlags();
}

void X64Asm::emulateSingleOp(DecodedOp* op, bool dynamic) {
    writeToRegFromValue(HOST_TMP, true, (U64)op, 8);
    writeToMemFromReg(HOST_TMP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_CURRENT_OP, 8, false);
    writeToRegFromValue(HOST_TMP, true, startOfOpIp, 4);
    writeToMemFromValue(dynamic ? 1 : 0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);    
    writeToRegFromMem(HOST_TMP2, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DO_SINGLE_OP_ADDRESS, 8, false);
    jmpNativeReg(HOST_TMP2, true);
}

// U32 dBase = cpu->seg[ES].address;
// U32 sBase = cpu->seg[base].address;
// S32 inc = cpu->getDirection();
// U32 count = CX;
// U32 i;
// for (i = 0; i < count; i++) {
//     cpu->memory->writeb(dBase + DI, cpu->memory->readb(sBase + SI));
//     DI += inc;
//     SI += inc;
//     CX--;
// }
void X64Asm::string(U32 width, bool hasSrc, bool hasDst) {
    bool repeat = (currentOp->repZero || currentOp->repNotZero);
    bool needFlags = currentOp ? (DecodedOp::getNeededFlags(currentBlock, currentOp, CF | PF | SF | ZF | AF | OF) != 0 || instructionInfo[currentOp->inst].flagsUsed != 0) : true;
    U32 skipPos = 0;

    if (needFlags) {
        U8 flagsReg = getTmpReg();
        pushFlagsToReg(flagsReg, true, true);
        writeToMemFromReg(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_FLAGS, 4, false);
        releaseTmpReg(flagsReg);
        flagsWrittenToStringFlags = true;
    }
    if (repeat) {
        // test (e)cx, (e)cx
        if (ea16) {
            write8(0x66);
        }
        write8(0x85);
        write8(0xc9);

        // jz
        write8(0x0f);
        write8(0x84);

        skipPos = bufferPos;
        write32(0x0);
    }
   
    // get direction (DF)
    {
        U8 tmp = getTmpReg();
        pushNativeFlags();
        popNativeReg(tmp, true);
        shiftLeftReg(tmp, true, 21);
        shiftRightReg(tmp, true, 31, true);
        orReg(tmp, true, width);
        if (width == 2) {
            andReg(tmp, true, 0xfffffffe);
        } else if (width == 4) {
            andReg(tmp, true, 0xfffffffc);
        }
        writeToMemFromReg(tmp, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);
        releaseTmpReg(tmp);
    }

    U32 pos = bufferPos;
    if (hasSrc && hasDst) {
        U8 tmpReg = getTmpReg();
        U8 srcReg = 6;
        bool srcRegIsRex = false;
        
        if (this->cpu->thread->process->hasSetSeg[ds]) {
            if (ea16) {
                srcReg = getTmpReg();
                zeroExtend16to32(srcReg, true, 6, false);
                srcRegIsRex = true;
            }
            writeToRegFromMem(tmpReg, true, srcReg, srcRegIsRex, getRegForSeg(ds, tmpReg), true, 0, 0, width, true);
            if (ea16) {
                releaseTmpReg(srcReg);
            }
        } else {
            if (ea16) {
                zeroExtend16to32(tmpReg, true, 6, false);
                srcReg = tmpReg;
                srcRegIsRex = true;
            }
            writeToRegFromMem(tmpReg, true, srcReg, srcRegIsRex, -1, false, 0, 0, width, true);
        }

        U8 dstReg = 7;
        bool dstRegIsRex = false;
        U8 tmpDst = 0xff;

        if (ea16) {
            tmpDst = getTmpReg();
            zeroExtend16to32(tmpDst, true, 7, false);
            dstReg = tmpDst;
            dstRegIsRex = true;
        }
        if (this->cpu->thread->process->hasSetSeg[ES]) {
            U8 segReg = getTmpReg();
            writeToMemFromReg(tmpReg, true, dstReg, dstRegIsRex, getRegForSeg(ES, segReg), true, 0, 0, width, true, false, true);
        } else {
            writeToMemFromReg(tmpReg, true, dstReg, dstRegIsRex, -1, false, 0, 0, width, true);
        }
        if (tmpDst != 0xff) {
            releaseTmpReg(tmpDst);
        }
        releaseTmpReg(tmpReg);
    } else if (hasSrc) {
        U8 srcReg = 6;
        bool srcRegIsRex = false;

        if (ea16) {
            srcReg = getTmpReg();
            zeroExtend16to32(srcReg, true, 6, false);
            srcRegIsRex = true;
        }

        if (this->cpu->thread->process->hasSetSeg[ds]) {
            U8 segReg = getTmpReg();
            writeToRegFromMem(0, false, srcReg, srcRegIsRex, getRegForSeg(ds, segReg), true, 0, 0, width, true);
            releaseTmpReg(segReg);
        } else {
            writeToRegFromMem(0, false, srcReg, srcRegIsRex, -1, false, 0, 0, width, true);
        }
        if (ea16) {
            releaseTmpReg(srcReg);
        }
    } else if (hasDst) {
        U8 dstReg = 7;
        bool dstRegIsRex = false;

        if (ea16) {
            dstReg = getTmpReg();
            zeroExtend16to32(dstReg, true, 7, false);
            dstRegIsRex = true;
        }

        if (this->cpu->thread->process->hasSetSeg[ES]) {
            U8 segReg = getTmpReg();
            writeToMemFromReg(0, false, dstReg, dstRegIsRex, getRegForSeg(ES, segReg), true, 0, 0, width, true, false, true);
        } else {
            writeToMemFromReg(0, false, dstReg, dstRegIsRex, -1, false, 0, 0, width, true);
        }
        if (ea16) {
            releaseTmpReg(dstReg);
        }
    } else {
        kpanic("X64Asm::string oops");
    }
      
    {
        U8 tmpReg = getTmpReg();
        writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);
        if (hasSrc) {
            addWithLea(6, false, 6, false, tmpReg, true, 0, 0, ea16 ? 2 : 4);
        }
        if (hasDst) {
            addWithLea(7, false, 7, false, tmpReg, true, 0, 0, ea16 ? 2 : 4);
        }
        releaseTmpReg(tmpReg);
    }

    if (repeat) {
        S32 diff = (S32)pos - (S32)bufferPos;
        U32 startPos = bufferPos;

        // dec (e)cx
        if (ea16) {
            write8(0x66);
        }
        write8(0xff);
        write8(0xc9);

        // cmp (e)cx, 0
        if (ea16) {
            write8(0x66);
        }
        write8(0x83);
        write8(0xf9);
        write8(0x00);

        // jz
        write8(0x74);
        write8(5);

        // jmp back to start and do another loop
        write8(0xe9);
        write32(diff - (bufferPos - startPos + 4));

        write32Buffer(buffer + skipPos, bufferPos - skipPos - 4);
    }
    if (needFlags) {
        U8 flagsReg = getTmpReg();
        writeToRegFromMem(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_FLAGS, 4, false);
        popFlagsFromReg(flagsReg, true, true);
        releaseTmpReg(flagsReg);
    }
}

void X64Asm::cmps(U32 width, bool hasSrc) {
    bool repeat = (currentOp->repZero || currentOp->repNotZero);
    U32 skipPos = 0;
    bool needFlags = currentOp ? (DecodedOp::getNeededFlags(currentBlock, currentOp, CF | PF | SF | ZF | AF | OF) != 0 || instructionInfo[currentOp->inst].flagsUsed != 0) : true;

    if (needFlags) {
        U8 flagsReg = getTmpReg();
        pushFlagsToReg(flagsReg, true, true);
        writeToMemFromReg(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_FLAGS, 4, false);
        releaseTmpReg(flagsReg);
        flagsWrittenToStringFlags = true;
    }    
    if (repeat) {        
        // test (e)cx, (e)cx
        if (ea16) {
            write8(0x66);
        }
        write8(0x85);
        write8(0xc9);

        // jz
        write8(0x0f);
        write8(0x84);

        skipPos = bufferPos;
        write32(0x0);
    }
    DecodedOp* op = currentBlock->op;
    int direction = 0;

    while (op && op != currentOp) {
        if (op->inst == Cld) {
            direction = 1;
        } else if (op->inst == Std) {
            direction = -1;
        }
        op = op->next;
    }
    if (direction == 0) {
        direction = cpu->getDirection();
    }
    U32 pos = bufferPos;
    U8 tmpSrc = 0xff;
    bool tmpSrcRex = true;

    if (!hasSrc) {
        tmpSrc = 0;
        tmpSrcRex = false;
    } else {
        tmpSrc = getTmpReg();
        U8 srcReg = 6;
        bool srcRegIsRex = false;

        if (this->cpu->thread->process->hasSetSeg[ds]) {
            if (ea16) {
                srcReg = getTmpReg();
                zeroExtend16to32(srcReg, true, 6, false);
                srcRegIsRex = true;
            }
            writeToRegFromMem(tmpSrc, true, srcReg, srcRegIsRex, getRegForSeg(ds, tmpSrc), true, 0, 0, width, true);
            if (ea16) {
                releaseTmpReg(srcReg);
            }
        } else {
            if (ea16) {
                zeroExtend16to32(tmpSrc, true, 6, false);
                srcReg = tmpSrc;
                srcRegIsRex = true;
            }
            writeToRegFromMem(tmpSrc, true, srcReg, srcRegIsRex, -1, false, 0, 0, width, true);
        }
    }

    U8 dstReg = 7;
    bool dstRegIsRex = false;
    U8 tmpDst = getTmpReg();

    if (this->cpu->thread->process->hasSetSeg[ES]) {
        U8 segReg = getTmpReg();
        if (ea16) {
            dstReg = tmpDst;
            zeroExtend16to32(dstReg, true, 7, false);
            dstRegIsRex = true;
        }
        writeToRegFromMem(tmpDst, true, dstReg, dstRegIsRex, getRegForSeg(ES, segReg), true, 0, 0, width, true, false, true);
    } else {
        if (ea16) {
            zeroExtend16to32(tmpDst, true, 7, false);
            dstReg = tmpDst;
            dstRegIsRex = true;
        }
        writeToRegFromMem(tmpDst, true, dstReg, dstRegIsRex, -1, false, 0, 0, width, true);
    }

    if (width == 2) {
        write8(0x66);
    }
    write8(REX_BASE | REX_MOD_REG | (tmpSrcRex?REX_MOD_RM:0));
    write8(width==1?0x38:0x39);
    write8(0xc0 | (tmpDst << 3) | tmpSrc);

    releaseTmpReg(tmpDst);
    if (hasSrc) {
        releaseTmpReg(tmpSrc);
        addWithLea(6, false, 6, false, -1, false, 0, direction * width, ea16 ? 2 : 4);
    }    
    addWithLea(7, false, 7, false, -1, false, 0, direction * width, ea16 ? 2 : 4);

    if (repeat) {
        S32 diff = (S32)pos - (S32)bufferPos;
        U32 startPos = bufferPos;

        // dec (e)cx
        addWithLea(1, false, 1, false, -1, false, 0, 0xffffffff, ea16 ? 2 : 4);

        // condition tmpDst - tmpSrc met? then exit loop with that last cmp flags intact
        if (this->repZeroPrefix) {
            // jnz
            write8(0x75);
            write8(5);
        } else {
            // jz
            write8(0x74);
            write8(5);
        }
        U32 jmpOutFromEcxPos = bufferPos;

        U8 flagsReg = getTmpReg();
        pushFlagsToReg(flagsReg, true, true);
        // technically correct, the flags should be set so that they are correct for the next loop, but I'm not sure if its really necessary just in case an exception happens
        writeToMemFromReg(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_FLAGS, 4, false);

        // cmp (e)cx, 0
        if (ea16) {
            write8(0x66);
        }
        write8(0x83);
        write8(0xf9);
        write8(0x00);

        // jz
        write8(0x74);
        write8(5);        

        // jmp back to start and do another loop
        write8(0xe9);
        write32(diff - (bufferPos - startPos + 4));

        // write this skip pos before we restore flags in case we need the flags
        write32Buffer(buffer + skipPos, bufferPos - skipPos - 4);                
        
        popFlagsFromReg(flagsReg, true, true);
        releaseTmpReg(flagsReg);
        
        if (needFlags) {
            // jmp, this will jump over the ecx==0 case of restoring original flags so that we can exit
            write8(0xeb);
            U32 pos = bufferPos;
            write8(0x0);
            
            U8 flagsReg = getTmpReg();
            writeToRegFromMem(flagsReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_FLAGS, 4, false);
            popFlagsFromReg(flagsReg, true, true);
            releaseTmpReg(flagsReg);
            buffer[pos] = bufferPos - pos - 1;
        }
        buffer[jmpOutFromEcxPos - 1] = bufferPos - jmpOutFromEcxPos; // jmp here if ecx was 0 and we skip cmps
    }
}

// :TODO: could be inlined
// U32 common_bound16(CPU* cpu, U32 reg, U32 address)
void X64Asm::bound16(U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

// U32 common_bound32(CPU* cpu, U32 reg, U32 address)
void X64Asm::bound32(U8 rm) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::movRdCrx(U32 which, U32 reg) {
    emulateSingleOp(currentOp);
    done = true;
}

void X64Asm::movCrxRd(U32 which, U32 reg) {
    emulateSingleOp(currentOp);
    done = true;
}

void common_fpu_write_address(x64CPU* cpu, PFN_FPU_ADDRESS pfn, U32 address, U32 len) {
    pfn(cpu, address);
}

void X64Asm::callFpuNoArg(PFN_FPU pfn) {
    syncRegsFromHost();

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)pfn);
    syncRegsToHost();
}

void X64Asm::callFpuWithAddress(PFN_FPU_ADDRESS pfn, U8 rm) {
    syncRegsFromHost();

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    getAddressInRegFromE(PARAM_2_REG, PARAM_2_REX, rm);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)pfn);
    syncRegsToHost();
}

void X64Asm::callFpuWithAddressWrite(PFN_FPU_ADDRESS pfn, U8 rm, U32 len) {
    syncRegsFromHost();
    getAddressInRegFromE(PARAM_3_REG, PARAM_4_REX, rm);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, len, 4);

    callHost((void*)common_fpu_write_address);
    syncRegsToHost();
}

void X64Asm::callFpuWithArg(PFN_FPU_REG pfn, U32 arg) {
    syncRegsFromHost();

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U32)arg, 4);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)pfn);
    syncRegsToHost();
}

void X64Asm::saveNativeState() {
	for (int i = 0; i < 8; i++) {
		if (i != 4) { // don't save RSP
			pushNativeReg(i, false);
		}
	}
	for (int i = 0; i < 8; i++) {
		pushNativeReg(i, true);
	}

	writeToRegFromValue(HOST_CPU, true, (U64)this->cpu, 8);

	// fxsave cpu->fpuState
	write8(0x41);
	write8(0x0f);
	write8(0xae);
	write8(0x80 | HOST_CPU);
	write32((U32)(offsetof(x64CPU, fpuState)));	
}

void X64Asm::restoreNativeState() {
	// fxrstor
	write8(0x41);
	write8(0x0f);
	write8(0xae);
	write8(0x88 | HOST_CPU);
	write32((U32)(offsetof(x64CPU, fpuState)));
	
	for (int i = 7; i >= 0; i--) {		
		popNativeReg(i, true);
	}
	for (int i = 7; i >= 0; i--) {
		if (i != 4) { // don't pop RSP
			popNativeReg(i, false);
		}
	}
}

static void x64_retranslateChunk() {
    x64CPU* cpu = ((x64CPU*)KThread::currentThread()->cpu);
    cpu->returnHostAddress = cpu->reTranslateChunk();
}

static void x64_retranslateChunkAdjustForCS() {
    x64CPU* cpu = ((x64CPU*)KThread::currentThread()->cpu);
    cpu->eip.u32 -= cpu->seg[CS].address;
    if (!cpu->isBig()) {
        cpu->eip.u32 = cpu->eip.u32 & 0xFFFF;
    }
    cpu->returnHostAddress = cpu->reTranslateChunk();
}

void X64Asm::callRetranslateChunk() {
    syncRegsFromHost();
    writeToRegFromMem(HOST_TMP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_RETRANSLATE_CHUNK_ADDRESS, 8, false);
    jmpNativeReg(HOST_TMP, true);
}

void X64Asm::createCodeForRetranslateChunk(bool includeSetupFromR9) {
    if (includeSetupFromR9) {
        syncRegsFromHost(true);
        callHost((void*)x64_retranslateChunkAdjustForCS);
    } else {        
        callHost((void*)x64_retranslateChunk);
    }
    syncRegsToHost();
    writeToRegFromMem(HOST_TMP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_RETURN_HOST_ADDRESS, 8, false);
    jmpNativeReg(HOST_TMP, true);
}

static void x64_jmpAndTranslateIfNecessary() {
    x64CPU* cpu = ((x64CPU*)KThread::currentThread()->cpu);
    KMemoryData* mem = getMemData(cpu->memory);
    try {
        while (true) {
            U32 address = cpu->eip.u32 + cpu->seg[CS].address;
            DecodedOp* op = NormalCPU::decodeSingleOp(cpu, address);
            bool wasDynamic = false;
            if (mem->isAddressDynamic(cpu->eip.u32, op->len)) {
                cpu->arg5 = 1; // signal to runSingleOp that it is dynamic
                common_runSingleOp(cpu);
                wasDynamic = true;
            } else {
                CodeBlock block = cpu->memory->findCodeBlockContaining(address, op->len);
                if (block) {
                    if (block->getEip() == address + 1 && op->lock != 0) {
                        // the current block was created by skipping the lock, lets replace it
                        cpu->memory->removeCodeBlock(block->getEip(), block->getEipLen());
                    }
                }
            }
            op->dealloc(true);
            if (wasDynamic) {
                continue;
            }
            break;
        }
    } catch (...) {
    }
    cpu->returnHostAddress = (U64)cpu->translateEip(cpu->eip.u32);
}

static void x64_jmpAndTranslateIfNecessaryAdjustForCS() {
    x64CPU* cpu = ((x64CPU*)KThread::currentThread()->cpu);
    cpu->eip.u32 -= cpu->seg[CS].address;
    if (!cpu->isBig()) {
        cpu->eip.u32 = cpu->eip.u32 & 0xFFFF;
    }
    cpu->returnHostAddress = (U64)cpu->translateEip(cpu->eip.u32);
}

void X64Asm::createCodeForJmpAndTranslateIfNecessary(bool includeSetupFromR9) {
    if (includeSetupFromR9) {        
        writeToMemFromReg(7, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
        syncRegsFromHostCall();
        callHost((void*)x64_jmpAndTranslateIfNecessary);
    } else {
        syncRegsFromHost(false);
        callHost((void*)x64_jmpAndTranslateIfNecessary);
    }
    syncRegsToHost();
    writeToRegFromMem(HOST_TMP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_RETURN_HOST_ADDRESS, 8, false);
    jmpNativeReg(HOST_TMP, true);
}

#ifdef BOXEDWINE_POSIX
void signalHandler();

void X64Asm::createCodeForRunSignal() {
    callHost((void*)signalHandler);
    syncRegsToHost();

    write8(REX_BASE | REX_MOD_RM);
    write8(0xff);
    write8(0xa0 | HOST_CPU);
    write32(CPU_OFFSET_RETURN_HOST_ADDRESS);
}
#endif

void X64Asm::fpu0(U8 rm) {
    if (rm >= 0xc0) {
        switch (G(rm)) {
        case 0: callFpuWithArg(common_FADD_ST0_STj, E(rm)); break;
        case 1: callFpuWithArg(common_FMUL_ST0_STj, E(rm)); break;
        case 2: callFpuWithArg(common_FCOM_STi, E(rm)); break;
        case 3: callFpuWithArg(common_FCOM_STi_Pop, E(rm)); break;
        case 4: callFpuWithArg(common_FSUB_ST0_STj, E(rm)); break;
        case 5: callFpuWithArg(common_FSUBR_ST0_STj, E(rm)); break;
        case 6: callFpuWithArg(common_FDIV_ST0_STj, E(rm)); break;
        case 7: callFpuWithArg(common_FDIVR_ST0_STj, E(rm)); break;
        }
    } else {
        switch (G(rm)) {
            case 0: callFpuWithAddress(common_FADD_SINGLE_REAL, rm); break;
            case 1: callFpuWithAddress(common_FMUL_SINGLE_REAL, rm); break;
            case 2: callFpuWithAddress(common_FCOM_SINGLE_REAL, rm); break;
            case 3: callFpuWithAddress(common_FCOM_SINGLE_REAL_Pop, rm); break;
            case 4: callFpuWithAddress(common_FSUB_SINGLE_REAL, rm); break;
            case 5: callFpuWithAddress(common_FSUBR_SINGLE_REAL, rm); break;
            case 6: callFpuWithAddress(common_FDIV_SINGLE_REAL, rm); break;
            case 7: callFpuWithAddress(common_FDIVR_SINGLE_REAL, rm); break;
        }
    }
}

void X64Asm::fpu1(U8 rm) {
    if (rm >= 0xc0) {	
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FLD_STi, E(rm)); break;
            case 1: callFpuWithArg(common_FXCH_STi, E(rm)); break;
            case 2: callFpuNoArg(common_FNOP); break;
            case 3: callFpuWithArg(common_FST_STi_Pop, E(rm)); break;
            case 4:
            {
                switch (rm & 7) {
                    case 0: callFpuNoArg(common_FCHS); break;
                    case 1: callFpuNoArg(common_FABS); break;
                    case 4: callFpuNoArg(common_FTST); break;
                    case 5: callFpuNoArg(common_FXAM); break;
                    default: invalidOp(this->inst); break;
                }
                break;
            }
            case 5:
            {
                switch (rm & 7) {
                    case 0: callFpuNoArg(common_FLD1); break;
                    case 1: callFpuNoArg(common_FLDL2T); break;
                    case 2: callFpuNoArg(common_FLDL2E); break;
                    case 3: callFpuNoArg(common_FLDPI); break;
                    case 4: callFpuNoArg(common_FLDLG2); break;
                    case 5: callFpuNoArg(common_FLDLN2); break;
                    case 6: callFpuNoArg(common_FLDZ); break;
                    case 7: invalidOp(this->inst); break;
                }
                break;
            }
            case 6:
            {
                switch (rm & 7) {
                    case 0: callFpuNoArg(common_F2XM1); break;
                    case 1: callFpuNoArg(common_FYL2X); break;
                    case 2: callFpuNoArg(common_FPTAN); break;
                    case 3: callFpuNoArg(common_FPATAN); break;
                    case 4: callFpuNoArg(common_FXTRACT); break;
                    case 5: callFpuNoArg(common_FPREM_nearest); break;
                    case 6: callFpuNoArg(common_FDECSTP); break;
                    case 7: callFpuNoArg(common_FINCSTP); break;
                }
                break;
            }
            case 7:
            {
                switch (rm & 7) {
                    case 0: callFpuNoArg(common_FPREM); break;
                    case 1: callFpuNoArg(common_FYL2XP1); break;
                    case 2: callFpuNoArg(common_FSQRT); break;
                    case 3: callFpuNoArg(common_FSINCOS); break;
                    case 4: callFpuNoArg(common_FRNDINT); break;
                    case 5: callFpuNoArg(common_FSCALE); break;
                    case 6: callFpuNoArg(common_FSIN); break;
                    case 7: callFpuNoArg(common_FCOS); break;
                }
                break;
            }
        }
    } else {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FLD_SINGLE_REAL, rm); break;
            case 1: invalidOp(this->inst); break;
            case 2: callFpuWithAddressWrite(common_FST_SINGLE_REAL, rm, 4); break;
            case 3: callFpuWithAddressWrite(common_FST_SINGLE_REAL_Pop, rm, 4); break;
            case 4: callFpuWithAddress(common_FLDENV, rm); break;
            case 5: callFpuWithAddress(common_FLDCW, rm); break;
            case 6: callFpuWithAddressWrite(common_FNSTENV, rm, (cpu->isBig()?12:6)); break;
            case 7: callFpuWithAddressWrite(common_FNSTCW, rm, 2); break;
        }
    }
}

void X64Asm::fpu2(U8 rm) {
    if (rm >= 0xc0) {       
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FCMOV_ST0_STj_CF, E(rm)); break;
            case 1: callFpuWithArg(common_FCMOV_ST0_STj_ZF, E(rm)); break;
            case 2: callFpuWithArg(common_FCMOV_ST0_STj_CF_OR_ZF, E(rm)); break;
            case 3: callFpuWithArg(common_FCMOV_ST0_STj_PF, E(rm)); break;
            case 5:
                if ((rm & 7)==1) {
                    callFpuNoArg(common_FUCOMPP);
                    break;
                }
            // intentional fall through
                [[fallthrough]];
            default:
                invalidOp(this->inst); break;
        }
    } else {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FIADD_DWORD_INTEGER, rm); break;
            case 1: callFpuWithAddress(common_FIMUL_DWORD_INTEGER, rm); break;
            case 2: callFpuWithAddress(common_FICOM_DWORD_INTEGER, rm); break;
            case 3: callFpuWithAddress(common_FICOM_DWORD_INTEGER_Pop, rm); break;
            case 4: callFpuWithAddress(common_FISUB_DWORD_INTEGER, rm); break;
            case 5: callFpuWithAddress(common_FISUBR_DWORD_INTEGER, rm); break;
            case 6: callFpuWithAddress(common_FIDIV_DWORD_INTEGER, rm); break;
            case 7: callFpuWithAddress(common_FIDIVR_DWORD_INTEGER, rm); break;
        }
    }
}

void X64Asm::fpu3(U8 rm) {
    if (rm >= 0xc0) {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FCMOV_ST0_STj_NCF, E(rm)); break;
            case 1: callFpuWithArg(common_FCMOV_ST0_STj_NZF, E(rm)); break;
            case 2: callFpuWithArg(common_FCMOV_ST0_STj_NCF_AND_NZF, E(rm)); break;
            case 3: callFpuWithArg(common_FCMOV_ST0_STj_NPF, E(rm)); break;
            case 4:
            {
                switch (rm & 7) {
                    case 2: callFpuNoArg(common_FNCLEX); break;
                    case 3: callFpuNoArg(common_FNINIT); break;
                    default: invalidOp(this->inst); break;
                }
                break;
            }
            case 5: callFpuWithArg(common_FUCOMI_ST0_STj, E(rm)); break;
            case 6: callFpuWithArg(common_FCOMI_ST0_STj, E(rm)); break;
            default: invalidOp(this->inst); break;
        }
    } else {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FILD_DWORD_INTEGER, rm); break;
            case 1: callFpuWithAddressWrite(common_FISTTP32, rm, 4); break;
            case 2: callFpuWithAddressWrite(common_FIST_DWORD_INTEGER, rm, 4); break;
            case 3: callFpuWithAddressWrite(common_FIST_DWORD_INTEGER_Pop, rm, 4); break;
            case 5: callFpuWithAddress(common_FLD_EXTENDED_REAL, rm); break;
            case 7: callFpuWithAddressWrite(common_FSTP_EXTENDED_REAL, rm, 10); break;
            default: invalidOp(this->inst); break;
        }
    }
}

void X64Asm::fpu4(U8 rm) {
    if (rm >= 0xc0) {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FADD_STi_ST0, E(rm)); break;
            case 1: callFpuWithArg(common_FMUL_STi_ST0, E(rm)); break;
            case 2: callFpuWithArg(common_FCOM_STi, E(rm)); break;
            case 3: callFpuWithArg(common_FCOM_STi_Pop, E(rm)); break;
            case 4: callFpuWithArg(common_FSUBR_STi_ST0, E(rm)); break;
            case 5: callFpuWithArg(common_FSUB_STi_ST0, E(rm)); break;
            case 6: callFpuWithArg(common_FDIVR_STi_ST0, E(rm)); break;
            case 7: callFpuWithArg(common_FDIV_STi_ST0, E(rm)); break;
        }
    } else  {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FADD_DOUBLE_REAL, rm); break;
            case 1: callFpuWithAddress(common_FMUL_DOUBLE_REAL, rm); break;
            case 2: callFpuWithAddress(common_FCOM_DOUBLE_REAL, rm); break;
            case 3: callFpuWithAddress(common_FCOM_DOUBLE_REAL_Pop, rm); break;
            case 4: callFpuWithAddress(common_FSUB_DOUBLE_REAL, rm); break;
            case 5: callFpuWithAddress(common_FSUBR_DOUBLE_REAL, rm); break;
            case 6: callFpuWithAddress(common_FDIV_DOUBLE_REAL, rm); break;
            case 7: callFpuWithAddress(common_FDIVR_DOUBLE_REAL, rm); break;
        }
    }
}

void X64Asm::fpu5(U8 rm) {
    if (rm >= 0xc0) {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FFREE_STi, E(rm)); break;
            case 1: callFpuWithArg(common_FXCH_STi, E(rm)); break;
            case 2: callFpuWithArg(common_FST_STi, E(rm)); break;
            case 3: callFpuWithArg(common_FST_STi_Pop, E(rm)); break;
            case 4: callFpuWithArg(common_FUCOM_STi, E(rm)); break;
            case 5: callFpuWithArg(common_FUCOM_STi_Pop, E(rm)); break;
            default: invalidOp(this->inst); break;
        }
    } else {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FLD_DOUBLE_REAL, rm); break;
            case 1: callFpuWithAddressWrite(common_FISTTP64, rm, 8); break;
            case 2: callFpuWithAddressWrite(common_FST_DOUBLE_REAL, rm, 8); break;
            case 3: callFpuWithAddressWrite(common_FST_DOUBLE_REAL_Pop, rm, 8); break;
            case 4: callFpuWithAddress(common_FRSTOR, rm); break;
            case 5: invalidOp(this->inst); break;
            case 6: callFpuWithAddressWrite(common_FNSAVE, rm, (cpu->isBig()?28:14)+80); break;
            case 7: callFpuWithAddressWrite(common_FNSTSW, rm, 2); break;
        }
    }
}

void X64Asm::fpu6(U8 rm) {
    if (rm >= 0xc0) {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FADD_STi_ST0_Pop, E(rm)); break;
            case 1: callFpuWithArg(common_FMUL_STi_ST0_Pop, E(rm)); break;
            case 2: callFpuWithArg(common_FCOM_STi_Pop, E(rm)); break;
            case 3:
                if ((rm & 7) == 1)
                    callFpuNoArg(common_FCOMPP);
                else {
                    invalidOp(this->inst); 
                }
                break;
            break;
            case 4: callFpuWithArg(common_FSUBR_STi_ST0_Pop, E(rm)); break;
            case 5: callFpuWithArg(common_FSUB_STi_ST0_Pop, E(rm)); break;
            case 6: callFpuWithArg(common_FDIVR_STi_ST0_Pop, E(rm)); break;
            case 7: callFpuWithArg(common_FDIV_STi_ST0_Pop, E(rm)); break;
        }
    } else {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FIADD_WORD_INTEGER, rm); break;
            case 1: callFpuWithAddress(common_FIMUL_WORD_INTEGER, rm); break;
            case 2: callFpuWithAddress(common_FICOM_WORD_INTEGER, rm); break;
            case 3: callFpuWithAddress(common_FICOM_WORD_INTEGER_Pop, rm); break;
            case 4: callFpuWithAddress(common_FISUB_WORD_INTEGER, rm); break;
            case 5: callFpuWithAddress(common_FISUBR_WORD_INTEGER, rm); break;
            case 6: callFpuWithAddress(common_FIDIV_WORD_INTEGER, rm); break;
            case 7: callFpuWithAddress(common_FIDIVR_WORD_INTEGER, rm); break;
        }
    }
}

void X64Asm::fpu7(U8 rm) {
    if (rm >= 0xc0) {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithArg(common_FFREEP_STi, E(rm)); break;
            case 1: callFpuWithArg(common_FXCH_STi, E(rm)); break;
            case 2:
            case 3: callFpuWithArg(common_FST_STi_Pop, E(rm)); break;
            case 4:
                if ((rm & 7)==0)
                    callFpuNoArg(common_FNSTSW_AX);
                else {
                    invalidOp(this->inst);
                }
                break;
            case 5: callFpuWithArg(common_FUCOMI_ST0_STj_Pop, E(rm)); break;
            case 6: callFpuWithArg(common_FCOMI_ST0_STj_Pop, E(rm)); break;
            case 7: invalidOp(this->inst); break;
        }
    } else  {
        switch ((rm >> 3) & 7) {
            case 0: callFpuWithAddress(common_FILD_WORD_INTEGER, rm); break;
            case 1: callFpuWithAddressWrite(common_FISTTP16, rm, 2); break;
            case 2: callFpuWithAddressWrite(common_FIST_WORD_INTEGER, rm, 2); break;
            case 3: callFpuWithAddressWrite(common_FIST_WORD_INTEGER_Pop, rm, 2); break;
            case 4: callFpuWithAddress(common_FBLD_PACKED_BCD, rm); break;
            case 5: callFpuWithAddress(common_FILD_QWORD_INTEGER, rm); break;
            case 6: callFpuWithAddressWrite(common_FBSTP_PACKED_BCD, rm, 10); break;
            case 7: callFpuWithAddressWrite(common_FISTP_QWORD_INTEGER, rm, 8); break;
        }
    }
}

void X64Asm::translateInstruction() {
    KMemoryData* mem = getMemData(cpu->memory);
    this->startOfOpIp = this->ip;

#ifdef _DEBUG
    //this->logOp(this->ip);
    // just makes debugging the asm output easier
#ifndef __TEST
    this->writeToMemFromValue(this->ip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
#endif
#endif

    if (mem->isAddressDynamic(ip, currentOp->len)) {
        mem->markAddressDynamic(ip, currentOp->len);
        emulateSingleOp(nullptr, true);
        ip += currentOp->len;
        done = true;
        return;
    }
    while (1) {
        this->op = this->fetch8();
        this->inst = this->baseOp + this->op;
        if (!x64Decoder[this->inst](this)) {
            break;
        }
    }    
    this->tmp1InUse = false;
    this->tmp2InUse = false;
    this->tmp3InUse = false;
}

#ifdef __TEST
void X64Asm::addReturnFromTest() {
    U8 tmpReg = getTmpReg();
    pushNativeFlags();
    popNativeReg(tmpReg, true);

    writeToMemFromReg(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EAX, 4, false);
    writeToMemFromReg(1, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ECX, 4, false);
    writeToMemFromReg(2, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDX, 4, false);
    writeToMemFromReg(3, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBX, 4, false);
    writeToMemFromReg(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESP, 4, false);
    writeToMemFromReg(5, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EBP, 4, false);
    writeToMemFromReg(6, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ESI, 4, false);
    writeToMemFromReg(7, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EDI, 4, false);
    writeToMemFromReg(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FLAGS, 4, false);
    releaseTmpReg(tmpReg);
    if (cpu->big) {
        // fxsave cpu->fpuState
        write8(0x41);
        write8(0x0f);
        write8(0xae);
        write8(0x85);
        write32((U32)(offsetof(x64CPU, fpuState)));
    }
    restoreNativeState();
    write8(0xfc); // cld
    write8(0xc3); // retn
}
#endif
#endif
