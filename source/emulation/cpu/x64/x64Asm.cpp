#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Asm.h"
#include "../common/common_other.h"
#include "../../../../source/emulation/hardmmu/hard_memory.h"
#include "../normal/normalCPU.h"
#include "../normal/instructions.h"
#include "../binaryTranslation/btCodeMemoryWrite.h"

#include "../common/common_fpu.h"

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
#define CPU_OFFSET_FPU_STATE (U32)(offsetof(x64CPU, fpuState))
#define CPU_OFFSET_RETURN_HOST_ADDRESS (U32)(offsetof(x64CPU, returnHostAddress))
#define CPU_OFFSET_RETRANSLATE_CHUNK_ADDRESS (U32)(offsetof(x64CPU, reTranslateChunkAddress))
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

X64Asm::X64Asm(x64CPU* cpu) : X64Data(cpu), parent(NULL), tmp1InUse(false), tmp2InUse(false), tmp3InUse(false), tmp4InUse(false), param1InUse(false), param2InUse(false), param3InUse(false), param4InUse(false)  {
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

void X64Asm::writeHostPlusTmp(U8 rm, bool checkG, bool isG8bit, bool isE8bit, U8 tmpReg) {
    this->rex |= REX_BASE | REX_SIB_INDEX|REX_MOD_RM;    
    setRM(rm, checkG, false, isG8bit, isE8bit);
    U8 hostReg = getHostMem(tmpReg, true);
    setSib(hostReg | (tmpReg << 3), false);
    if (hostReg != HOST_MEM) {
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

U8 X64Asm::getHostMem(U8 regEmulatedAddress, bool isRex) {
    if (this->useSingleMemOffset) {
        return HOST_MEM;
    } else {
        U8 resultReg = getTmpReg();
        U8 tmpReg;
        
        if (isTmpRegAvailable()) {
            tmpReg = getTmpReg();
        } else {
            pushNativeReg(HOST_MEM, true);
            tmpReg = HOST_MEM;
        }
        if (regEmulatedAddress == 4 && !isRex) {
            regEmulatedAddress = HOST_ESP;
            isRex = true;
        }
        if (x64CPU::hasBMI2) {
            writeToRegFromValue(tmpReg, true, 12, 4);
            bmi2ShiftRightReg(resultReg, regEmulatedAddress, isRex, tmpReg);
        } else {            
            pushFlagsToReg(tmpReg, true, true); // since shiftRightReg will change flags
            writeToRegFromReg(resultReg, true, regEmulatedAddress, isRex, 4);
            shiftRightReg(resultReg, true, K_PAGE_SHIFT); // get page
            popFlagsFromReg(tmpReg, true, true);            
        }
        writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_MEMOFFSET, 8, false);
        writeToRegFromMem(resultReg, true, tmpReg, true, resultReg, true, 3, 0, 8, false); // shift page << 3 (page*8), since sizeof(U64)==8 to get the value in memOffsets[page]
        if (tmpReg == HOST_MEM) {
            popNativeReg(HOST_MEM, true);
        } else {
            releaseTmpReg(tmpReg);
        }
        return resultReg;
    }
}

U8 X64Asm::getHostMemFromAddress(U32 address) {
    if (this->useSingleMemOffset) {
        return HOST_MEM;
    }
    else {
        U8 resultReg = getTmpReg();
        writeToRegFromMem(resultReg, true, HOST_CPU, true, -1, false, 0, CPU_MEMOFFSET, 8, false);
        writeToRegFromMem(resultReg, true, resultReg, true, -1, false, 0, (address >> K_PAGE_SHIFT) << 3, 8, false);
        return resultReg;
    }
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

void X64Asm::releaseHostMem(U8 reg) {
    if (reg != HOST_MEM) {
        releaseTmpReg(reg);
    }
}

// reg1 = [reg2 + (reg3 << reg3Shift) + displacement]
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void X64Asm::writeToRegFromMem(U8 toReg, bool isToRegRex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost) {
    if (translateToHost) {
        if (reg3>=0 || !this->useSingleMemOffset) {
            U8 tmpReg = getTmpReg();
            addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            U8 hostMemReg = getHostMem(tmpReg, true);
            writeToRegFromMem(toReg, isToRegRex, tmpReg, true, hostMemReg, true, 0, 0, bytes, false);
            releaseHostMem(hostMemReg);
            releaseTmpReg(tmpReg);
        } else {
            writeToRegFromMem(toReg, isToRegRex, reg2, isReg2Rex, HOST_MEM, true, 0, displacement, bytes, false);
        }
    } else {
        doMemoryInstruction(bytes==1?0x8a:0x8b, toReg, isToRegRex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
    }
}

U8 X64Asm::getRegForSeg(U8 base, U8 tmpReg) {
    if (base == ES) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ES_ADDRESS, 4, false); return tmpReg;}
    if (base == SS) {
        if (KSystem::useLargeAddressSpace) {
            writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SS_ADDRESS, 4, false); 
            return tmpReg;
        } else {
            return HOST_SMALL_ADDRESS_SPACE_SS;
        }
    }
    if (base == GS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_GS_ADDRESS, 4, false); return tmpReg;}
    if (base == FS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FS_ADDRESS, 4, false); return tmpReg;}
    if (base == DS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_ADDRESS, 4, false); return tmpReg; }
    if (base == CS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_CS_ADDRESS, 4, false); return tmpReg;}
    kpanic("unknown base in x64dynamic.c getRegForSeg");
    return 0;
}

U8 X64Asm::getRegForNegSeg(U8 base, U8 tmpReg) {
    if (base == ES) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ES_NEG_ADDRESS, 4, false); return tmpReg;}
    if (base == SS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SS_NEG_ADDRESS, 4, false); return tmpReg;}
    if (base == DS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_NEG_ADDRESS, 4, false); return tmpReg;}
    if (base == FS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FS_NEG_ADDRESS, 4, false); return tmpReg;}
    if (base == GS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_GS_NEG_ADDRESS, 4, false); return tmpReg;}
    if (base == CS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_CS_NEG_ADDRESS, 4, false); return tmpReg;}
    kpanic("unknown base in x64dynamic.c getRegForNegSeg");
    return 0;
}

void X64Asm::translateMemory16(U32 rm, bool checkG, bool isG8bit, bool isE8bit, S8 r1, S8 r2, S16 disp, U8 seg) {
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

    // [HOST_MEM + tmpReg]
    writeHostPlusTmp((rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

    releaseTmpReg(tmpReg2);
}

void X64Asm::translateMemory(U32 rm, bool checkG, bool isG8bit, bool isE8bit) {
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
                translateMemory16(rm, checkG, isG8bit, isE8bit, 3, 6, disp, this->ds);
            }
            break;
        case 0x01: // bx + di           
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 3, false, 7, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {        
                translateMemory16(rm, checkG, isG8bit, isE8bit, 3, 7, disp, this->ds);
            }
            break;
        case 0x02: // bp+si     
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 5, false, 6, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {          
                translateMemory16(rm, checkG, isG8bit, isE8bit, 5, 6, disp, this->ss);
            }
            break;
        case 0x03: // bp+di             
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 5, false, 7, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {             
                translateMemory16(rm, checkG, isG8bit, isE8bit, 5, 7, disp, this->ss);
            }
            break;
        case 0x04: // si
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 6, false, -1, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {    
                translateMemory16(rm, checkG, isG8bit, isE8bit, 6, -1, disp, this->ds);
            }
            break;
        case 0x05: // di
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 7, false, -1, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {            
                translateMemory16(rm, checkG, isG8bit, isE8bit, 7, -1, disp, this->ds);
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
                    translateMemory16(rm, checkG, isG8bit, isE8bit, -1, -1, (S16)this->fetch16(), this->ds);
                } else {
                    translateMemory16(rm, checkG, isG8bit, isE8bit, 5, -1, disp, this->ss);
                }
            }
            break;
        }
        case 0x07: // bx
            if (this->ds==SEG_ZERO) {
                addWithLea(G(rm), false, 3, false, -1, false, 0, disp, 2);
                this->skipWriteOp = true;
            } else {   
                translateMemory16(rm, checkG, isG8bit, isE8bit, 3, -1, disp, this->ds);
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
                        if (hostReg != HOST_MEM) {
                            autoReleaseTmpAfterWriteOp.push_back(hostReg);
                        }
                    } else {
                        U32 tmpReg = getTmpReg();

                        // HOST_TMP = reg + SEG
                        addWithLea(tmpReg, true, E(rm), false, getRegForSeg(this->ds, tmpReg), true, 0, 0, 4);

                        // [HOST_MEM + HOST_TMP]
                        writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
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
                    if (!this->cpu->thread->process->hasSetSeg[this->ds] && disp<=0x7FFFFFFF) {
                        // converts [disp32] to [HOST_MEM+disp32]
                        this->rex |= REX_BASE | REX_MOD_RM;    
                        setRM((rm & ~(0xC7)) | 4 | 0x80, checkG, false, isG8bit, isE8bit);
                        U8 hostReg = getHostMem(E(rm), false);
                        setSib(hostReg | 0x20, false);
                        if (hostReg != HOST_MEM) {
                            autoReleaseTmpAfterWriteOp.push_back(hostReg);
                        }
                        this->setDisplacement32(disp);
                    } else {
                        // converts [disp32] to HOST_TMP = [SEG + disp32]; [HOST_TMP+HOST_MEM]

                        U32 tmpReg = getTmpReg();
                        // HOST_TMP = SEG + disp32
                        addWithLea(tmpReg, true, getRegForSeg(this->ds, tmpReg), true, -1, false, 0, disp, 4);

                        // [HOST_MEM + HOST_TMP]
                        writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                    }
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
                            writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                        }                        
                    } else { // [base + index << shift]
                        if (this->ds == SEG_ZERO) {
                            // keep the same, but convert ESP to HOST_ESP
                            setRM(rm, checkG, false, isG8bit, isE8bit);
                            setSib(sib, true);
                        } else {
                            U8 seg = base==4?this->ss:this->ds;
                            // convert [base + index << shift] to HOST_TMP=[base + index << shift];HOST_TMP=[HOST_TMP+SEG];[HOST_TMP+MEM]
                            if (!this->cpu->thread->process->hasSetSeg[seg]) {                                                                 
                                if (index==4) { // no index
                                    // probably something like mov ebx,DWORD PTR [esp] 
                                    this->rex |= REX_BASE | REX_SIB_INDEX | REX_MOD_RM;    
                                    setRM(rm, checkG, false, isG8bit, isE8bit);
                                    U8 hostReg = getHostMem(E(rm), false);
                                    setSib((hostReg << 3) | base, true);
                                    if (hostReg != HOST_MEM) {
                                        autoReleaseTmpAfterWriteOp.push_back(hostReg);
                                    }
                                } else {
                                    U32 tmpReg = getTmpReg();
                                    // HOST_TMP=[base+index<<shift];
                                    addWithLea(tmpReg, true, base, false, index, false, sib >> 6, 0, 4);
                                    // [HOST_MEM + HOST_TMP]
                                    writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                                }                                
                            } else {
                                U32 tmpReg = getTmpReg();
                                // HOST_TMP=[base + SEG]
                                addWithLea(tmpReg, true, base, false,  getRegForSeg(seg, tmpReg), true, 0, 0, 4);

                                // HOST_TMP=[HOST_TMP+index<<shift];
                                if (index!=4) {
                                    addWithLea(tmpReg, true, tmpReg, true, index, false, sib >> 6, 0, 4);
                                }

                                // [HOST_MEM + HOST_TMP]
                                writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                            }
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
                    writeHostPlusTmp((rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
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
                        writeHostPlusTmp((rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
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

// rm could be set to use the rexReg in tmpReg
void X64Asm::translateRM(U8 rm, bool checkG, bool checkE, bool isG8bit, bool isE8bit, U8 immWidth) {        
    if (rm<0xC0) {
        translateMemory(rm, checkG, isG8bit, isE8bit);
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

// it is ok to trash current op data
void X64Asm::writeToRegFromE(U8 reg, bool isRegRex, U8 rm, U8 bytes) {
    this->operandPrefix = false;
    this->multiBytePrefix = false;
    this->repZeroPrefix = false;
    this->repNotZeroPrefix = false;
    if (isRegRex) {
        this->rex = REX_BASE | REX_MOD_REG;
    } else {
        this->rex = 0;
    }
    if (bytes==2) {
        if (this->cpu->isBig()) {
            this->operandPrefix = true;
        }
        this->op = 0x8b;
    } else if (bytes==4) {
        if (!this->cpu->isBig()) {
            this->operandPrefix = true;
        }
        this->op = 0x8b;
    } else {
        kpanic("writeToRegFromE didn't handle toBytes: %d", bytes);
    }    
    rm = (rm & ~0x38) | (reg << 3);
    translateRM(rm, false, true, false, false, 0);
}

// it is ok to trash current op data
void X64Asm::getNativeAddressInRegFromE(U8 reg, bool isRegRex, U8 rm) {
    this->op = 0x8d;
    this->multiBytePrefix = false;
    if (this->cpu->isBig()) {
        this->operandPrefix = false;    
    } else {
        this->operandPrefix = true;
    }
    this->rex = REX_BASE | REX_64;
    if (isRegRex) {
        this->rex |= REX_MOD_REG;
    }
    rm = (rm & ~0x38) | (reg << 3);
    translateRM(rm, false, true, false, false, 0);
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
    writeHostPlusTmp(4, false, true, true, tmpReg);
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
        if (KSystem::useLargeAddressSpace) {
            tmpReg1 = getTmpReg();
            addWithLea(tmpReg, true, tmpReg, true, getRegForSeg(SS, tmpReg1), true, 0, 0, 4);
            releaseTmpReg(tmpReg1);
            if (reg >= 0) {
                writeToMemFromReg(reg, isRegRex, tmpReg, true, -1, false, 0, 0, bytes, true);
            } else {
                writeToMemFromValue(value, tmpReg, true, -1, false, 0, 0, bytes, true);
            }
            addWithLea(tmpReg, true, HOST_ESP, true, -1, false, 0, -bytes, 4); // need to refetch, didn't have enough tmp variable to hold on to it
        } else {
            if (reg >= 0) {
                writeToMemFromReg(reg, isRegRex, tmpReg, true, HOST_SMALL_ADDRESS_SPACE_SS, true, 0, 0, bytes, true);
            } else {
                writeToMemFromValue(value, tmpReg, true, HOST_SMALL_ADDRESS_SPACE_SS, true, 0, 0, bytes, true);
            }
        }        
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
            writeToMemFromReg(reg, isRegRex, HOST_ESP, true, -1, false, 0, -bytes, bytes, true);
        } else {
            writeToMemFromValue(value, HOST_ESP, true, -1, false, 0, -bytes, bytes, true);
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

// used by 
// MOV AL,Ob
// MOV AX,Ow
// MOV EAX,Od
void X64Asm::writeToRegFromMemAddress(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes) {    
    if (this->cpu->thread->process->hasSetSeg[seg]) {
        U8 tmpReg = getTmpReg();
        // do 32-bit add before combining with HOST_MEM because of wrapping
        addWithLea(tmpReg, true, getRegForSeg(seg, tmpReg), true, -1, false, 0, disp, 4);
        writeToRegFromMem(reg, isRegRex, tmpReg, true, -1, false, 0, 0, bytes, true);
        releaseTmpReg(tmpReg);
    } else {
        // if disp > 0x7FFFFFFF then it will be interpreted as a negative offset, so instead copy it to a 32-bit reg
        if (disp>0x7FFFFFFF) {
            U8 tmpReg = getTmpReg();
            writeToRegFromValue(tmpReg, true, disp, 4);
            writeToRegFromMem(reg, isRegRex, tmpReg, true, -1, false, 0, 0, bytes, true);
            releaseTmpReg(tmpReg);
        } else {
            U8 hostReg = getHostMemFromAddress(disp);
            writeToRegFromMem(reg, isRegRex, hostReg, true, -1, false, 0, disp, bytes, false);
            releaseHostMem(hostReg);
        }
    } 
}

void X64Asm::writeToMemAddressFromReg(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes) {
    if (this->cpu->thread->process->hasSetSeg[seg]) {
        U8 tmpReg = getTmpReg();
        // do 32-bit add before combining with HOST_MEM because of wrapping
        addWithLea(tmpReg, true, getRegForSeg(seg, tmpReg), true, -1, false, 0, disp, 4);
        writeToMemFromReg(reg, isRegRex, tmpReg, true, -1, false, 0, 0, bytes, true);
        releaseTmpReg(tmpReg);
    } else {        
        // if disp > 0x7FFFFFFF then it will be interpreted as a negative offset, so instead copy it to a 32-bit reg        
        if (disp>0x7FFFFFFF) {
            U8 tmpReg = getTmpReg();
            writeToRegFromValue(tmpReg, true, disp, 4);
            writeToMemFromReg(reg, isRegRex, tmpReg, true, -1, false, 0, 0, bytes, true);        
            releaseTmpReg(tmpReg);
        } else {
            U8 hostReg = getHostMemFromAddress(disp);
            writeToMemFromReg(reg, isRegRex, hostReg, true, -1, false, 0, disp, bytes, false);
            releaseHostMem(hostReg);
        }
    }    
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
void X64Asm::writeToMemFromReg(U8 reg1, bool isReg1Rex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost) {
    if (translateToHost) {
        if (reg3>=0 || !this->useSingleMemOffset) {
            U8 tmpReg = getTmpReg();
            addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            U8 hostMemReg = getHostMem(tmpReg, true);
            writeToMemFromReg(reg1, isReg1Rex, tmpReg, true, hostMemReg, true, 0, 0, bytes, false);
            releaseHostMem(hostMemReg);
            releaseTmpReg(tmpReg);
        } else {
            writeToMemFromReg(reg1, isReg1Rex, reg2, isReg2Rex, HOST_MEM, true, 0, displacement, bytes, false);
        }
    } else {
        // reg 1 will be ignored
        doMemoryInstruction(bytes==1?0x88:0x89, reg1, isReg1Rex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
    }
}

void X64Asm::writeToMemFromValue(U64 value, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost) {
    if (translateToHost) {
        if (reg3>=0 || !this->useSingleMemOffset) {
            U8 tmpReg = getTmpReg();
            addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
            U8 hostMemReg = getHostMem(tmpReg, true);
            writeToMemFromValue(value, tmpReg, true, hostMemReg, true, 0, 0, bytes, false);
            releaseHostMem(hostMemReg);
            releaseTmpReg(tmpReg);
        } else {
            writeToMemFromValue(value, reg2, isReg2Rex, HOST_MEM, true, 0, displacement, bytes, false);
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
    U8 tmpReg;
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
        writeToRegFromMem(reg, isRegRex, tmpReg, true, getRegForSeg(SS, tmpReg1), true, 0, false, bytes, true);
        releaseTmpReg(tmpReg1);
    } else {
        // reg = [ss:tmpReg]    
        writeToRegFromMem(reg, isRegRex, HOST_ESP, true, -1, false, 0, false, bytes, true);
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

    if (!this->cpu->thread->process->emulateFPU) {
        // fxsave
        write8(0x41);
        write8(0x0f);
        write8(0xae);
        write8(0x80 | HOST_CPU);
        write32(CPU_OFFSET_FPU_STATE);
    }
}

void X64Asm::syncRegsToHost(S8 excludeReg) {
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

    if (KSystem::useLargeAddressSpace) {
        writeToRegFromMem(HOST_LARGE_ADDRESS_SPACE_MAPPING, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP_HOST_MAPPING, 8, false);
    } else {
        writeToRegFromMem(HOST_SMALL_ADDRESS_SPACE_SS, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SS_ADDRESS, 4, false);
    }
    //writeToRegFromMem(HOST_DS, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_ADDRESS, 4, false);

    // R12-R15 are non volitile on windows and linux

    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FLAGS, 4, false);
    pushNativeReg(tmpReg, true);
    popNativeFlags();
    releaseTmpReg(tmpReg);

    if (!cpu->thread->process->emulateFPU) {
        // fninit
        write8(0xDB);
        write8(0xE3);

        // fxrstor
        write8(0x41);
        write8(0x0f);
        write8(0xae);
        write8(0x88 | HOST_CPU);
        write32(CPU_OFFSET_FPU_STATE);
    }
}

void badStack(CPU* cpu) {
    kpanic("native stack is unaligned");
}

void X64Asm::callHost(void* pfn) {
    U8 tmp = getParamSafeTmpReg();
    
#ifdef _DEBUG
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
#endif

    write8(0xfc); // cld

    writeToRegFromValue(tmp, true, (U64)pfn, 8);

#ifdef BOXEDWINE_MSVC
    // part of the x64 windows ABI, shadow store
    // sub rsp, 32
    write8(REX_BASE | REX_64);
    write8(0x83);
    write8(0xEC);
    write8(0x20);    
#endif

    // call tmp
    write8(REX_BASE | REX_MOD_RM);
    write8(0xFF);
    write8(0xd0 | tmp);

#ifdef BOXEDWINE_MSVC
    // add rsp, 20
    write8(REX_BASE | REX_64);
    write8(0x83);
    write8(0xC4);
    write8(0x20);
#endif

    releaseTmpReg(tmp);
    unlockParamReg(PARAM_1_REG, PARAM_1_REX);
    unlockParamReg(PARAM_2_REG, PARAM_2_REX);
    unlockParamReg(PARAM_3_REG, PARAM_3_REX);
    unlockParamReg(PARAM_4_REG, PARAM_4_REX);
}

void X64Asm::doIf(U8 reg, bool isRexReg, U32 equalsValue, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock) {
    // cmp reg, value
    if (isRexReg)
        write8(REX_BASE | REX_MOD_RM);
    if (equalsValue>255) {
        write8(0x81);
    } else {
        write8(0x83);
    }
    write8(0xf8 | reg);
    if (equalsValue>255) {
        write32(equalsValue);
    } else {
        write8((U8)equalsValue);
    }
    // jz 
    write8(0x74);    
    U32 pos = this->bufferPos;
    write8(0);

    // ELSE BLOCK
        elseBlock();
        // jmp jb
        write8(0xeb);
        U32 pos2 = this->bufferPos;
        write8(0);
        if (this->bufferPos - pos - 1>127) {
            kpanic("doIf needs some work");
        }
        this->buffer[pos] = this->bufferPos - pos - 1; // jump to if block
    // IF BLOCK
        ifBlock();
        if (this->bufferPos - pos2 - 1>127) {
            kpanic("doIf needs some work");
        }
        this->buffer[pos2] = this->bufferPos - pos2 - 1; // else block jumps over if block
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
    // in case an exception is thrown, the exception will need to store the regs
    // also this will help will saving ECX and EDX as required by Window ABI
    syncRegsFromHost(); 

    // call U32 common_setSegment(CPU* cpu, U32 seg, U32 value)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, seg, 4); // value param, must pass 4 so that upper part of reg is zero'd out

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    if (bytes==2)
        zeroReg(PARAM_3_REG, PARAM_3_REX, false); // upper 2 bytes need to be 0
    popReg(PARAM_3_REG, PARAM_3_REX, bytes, false); // peek stack for seg param    

    callHost((void*)common_setSegment);
    
    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp(true);
    }, [this, bytes]() {
        syncRegsToHost();
        U8 tmpReg = getTmpReg();
        adjustStack(tmpReg, (S8)bytes);
        releaseTmpReg(tmpReg);
    });   
    this->cpu->thread->process->hasSetSeg[seg] = true;
}

void X64Asm::setSeg(U8 seg, U8 rm) {
    // in case an exception is thrown, the exception will need to store the regs
    // also this will help will saving ECX and EDX as required by Window ABI
    syncRegsFromHost();  

    // set first in case rm referes the following param regs
    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromE(PARAM_3_REG, PARAM_3_REX, rm, 2);
    andReg(PARAM_3_REG, PARAM_3_REX, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references PARAM_3_REG

    // call U32 common_setSegment(CPU* cpu, U32 seg, U32 value)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
    
    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, seg, 4); // value param, must pass 4 so that upper part of reg is zero'd out    

    callHost((void*)common_setSegment);
    
    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp(true);
    }, [this]() {
        syncRegsToHost();
    });   
    this->cpu->thread->process->hasSetSeg[seg] = true;
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
    syncRegsFromHost(); 
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param    
    callHost((void*)::das);
    syncRegsToHost();
}

void X64Asm::aaa() {
    syncRegsFromHost(); 
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param    
    callHost((void*)::aaa);
    syncRegsToHost();
}

void X64Asm::aas() {
    syncRegsFromHost(); 
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param    
    callHost((void*)::aas);
    syncRegsToHost();
}

void X64Asm::aad(U8 value) {
    syncRegsFromHost(); 

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, value, 4);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param    

    callHost((void*)::aad);
    syncRegsToHost();  
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
    writeToMemFromReg(0, false, HOST_ESP, true, ssReg, true, 0, -2, 2, true);
    writeToMemFromReg(1, false, HOST_ESP, true, ssReg, true, 0, -4, 2, true);
    writeToMemFromReg(2, false, HOST_ESP, true, ssReg, true, 0, -6, 2, true);
    writeToMemFromReg(3, false, HOST_ESP, true, ssReg, true, 0, -8, 2, true);
    writeToMemFromReg(HOST_ESP, true, HOST_ESP, true, ssReg, true, 0, -10, 2, true);
    writeToMemFromReg(5, false, HOST_ESP, true, ssReg, true, 0, -12, 2, true);
    writeToMemFromReg(6, false, HOST_ESP, true, ssReg, true, 0, -14, 2, true);
    writeToMemFromReg(7, false, HOST_ESP, true, ssReg, true, 0, -16, 2, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -16, 2);
    releaseTmpReg(tmpReg);
}

// POPA
void X64Asm::popA16() {
    U8 tmpReg = getTmpReg();
    U8 ssReg = getRegForSeg(SS, tmpReg);
    writeToRegFromMem(7, false, HOST_ESP, true, ssReg, true, 0, 0, 2, true);
    writeToRegFromMem(6, false, HOST_ESP, true, ssReg, true, 0, 2, 2, true);
    writeToRegFromMem(5, false, HOST_ESP, true, ssReg, true, 0, 4, 2, true);
    // SP isn't pop, but the stack will be adjusted
    // writeToRegFromMem(4, false, HOST_ESP, true, ssReg, true, 0, 6, 2, true);
    writeToRegFromMem(3, false, HOST_ESP, true, ssReg, true, 0, 8, 2, true);
    writeToRegFromMem(2, false, HOST_ESP, true, ssReg, true, 0, 10, 2, true);
    writeToRegFromMem(1, false, HOST_ESP, true, ssReg, true, 0, 12, 2, true);
    writeToRegFromMem(0, false, HOST_ESP, true, ssReg, true, 0, 14, 2, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, 16, 2);
    releaseTmpReg(tmpReg);
}

// PUSHAD
void X64Asm::pushA32() {
    S8 seg;
    U8 tmpReg = getTmpReg();

    if (this->cpu->thread->process->hasSetSeg[SS]) {
        seg = getRegForSeg(SS, tmpReg);
    } else {
        seg = -1;
    }
    writeToMemFromReg(0, false, HOST_ESP, true, seg, true, 0, -4, 4, true);
    writeToMemFromReg(1, false, HOST_ESP, true, seg, true, 0, -8, 4, true);
    writeToMemFromReg(2, false, HOST_ESP, true, seg, true, 0, -12, 4, true);
    writeToMemFromReg(3, false, HOST_ESP, true, seg, true, 0, -16, 4, true);
    writeToMemFromReg(HOST_ESP, true, HOST_ESP, true, seg, true, 0, -20, 4, true);
    writeToMemFromReg(5, false, HOST_ESP, true, seg, true, 0, -24, 4, true);
    writeToMemFromReg(6, false, HOST_ESP, true, seg, true, 0, -28, 4, true);
    writeToMemFromReg(7, false, HOST_ESP, true, seg, true, 0, -32, 4, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -32, 4);

    releaseTmpReg(tmpReg);
}

// POPAD
void X64Asm::popA32() {
    S8 seg;
    U8 tmpReg = getTmpReg();

    if (this->cpu->thread->process->hasSetSeg[SS]) {
        seg = getRegForSeg(SS, tmpReg);
    } else {
        seg = -1;
    }
    writeToRegFromMem(7, false, HOST_ESP, true, seg, true, 0, 0, 4, true);
    writeToRegFromMem(6, false, HOST_ESP, true, seg, true, 0, 4, 4, true);
    writeToRegFromMem(5, false, HOST_ESP, true, seg, true, 0, 8, 4, true);
    // SP isn't pop, but the stack will be adjusted
    // writeToRegFromMem(4, false, HOST_ESP, true, HOST_SS, true, 0, 12, 4, true);
    writeToRegFromMem(3, false, HOST_ESP, true, seg, true, 0, 16, 4, true);
    writeToRegFromMem(2, false, HOST_ESP, true, seg, true, 0, 20, 4, true);
    writeToRegFromMem(1, false, HOST_ESP, true, seg, true, 0, 24, 4, true);
    writeToRegFromMem(0, false, HOST_ESP, true, seg, true, 0, 28, 4, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, 32, 4);

    releaseTmpReg(tmpReg);
}

void X64Asm::jumpConditional(U8 condition, U32 eip) {    
    if (this->stopAfterInstruction!=(S32)this->ipAddressCount && (this->calculatedEipLen==0 || (eip>=this->startOfDataIp && eip<this->startOfDataIp+this->calculatedEipLen))) {
        write8(0x0F);
        write8(0x80+condition);
        write32(0);
        addTodoLinkJump(eip, 4, true);
    } else {
        write8(0x70+condition);
        doLoop(eip);
    }
}

void X64Asm::write64Buffer(U8* buffer, U64 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
    buffer[4] = (U8)(value >> 32);
    buffer[5] = (U8)(value >> 40);
    buffer[6] = (U8)(value >> 48);
    buffer[7] = (U8)(value >> 56);
}

void X64Asm::write32Buffer(U8* buffer, U32 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
}

void X64Asm::write16Buffer(U8* buffer, U16 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);

}

void X64Asm::addTodoLinkJump(U32 eip, U32 size, bool sameChunk) {
    this->todoJump.push_back(TodoJump(eip, this->bufferPos-(size==4?4:11), size, sameChunk, this->ipAddressCount));
}

void X64Asm::jumpTo(U32 eip) {  
    if (!this->cpu->isBig()) {
        eip = eip & 0xffff;
    }
#ifdef _DEBUG
    this->writeToMemFromValue(this->startOfOpIp, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP_FROM, 4, false);
#endif
    // :TODO: is this necessary?  who uses it?
    this->writeToMemFromValue(eip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    if (this->stopAfterInstruction!=(S32)this->ipAddressCount && (this->calculatedEipLen==0 || (eip>=this->startOfDataIp && eip<this->startOfDataIp+this->calculatedEipLen))) {
        write8(0xE9);
        write32(0);
        addTodoLinkJump(eip, 4, true);
    } else {
        // when a chunk gets modified/replaced other chunks that point to it via this jump need to get updated
        // it is not possible to modify the executable code directly in an atomic way, so instead of embedding
        // where we will jump directly into the instruction, we will encode an instruction that reads the jump
        // address from memory (data).  That memory location can be atomically updated.
        if (0) {
            // this can result in random crashes, but it gives about a 5% boost, maybe in the future I can figure out when to use it
            write8(0xE9);
            write32(0);
            addTodoLinkJump(eip, 4, false);
        } else {
            writeToRegFromValue(HOST_TMP, true, 0x0101010101010101l, 8);
            write8(0x41);
            write8(0xff);
            write8(0x20 | HOST_TMP);
            addTodoLinkJump(eip, 8, false);
        }
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

void X64Asm::internal_addDynamicCheck(U32 address, U32 len, U32 needsFlags, bool panic, U8 tmpReg3) {    
    bool saveAllFlags = (needsFlags & OF) != 0;
    bool saveLowBitFlags = needsFlags!=0 && !saveAllFlags;    
    U32 missed = 0;

    {
        U8 tmpReg1 = getTmpReg();
        U8 tmpReg2 = getTmpReg();
        bool inlineCmp = false;
        U32 offset = address & 0xFFF;
        U32 remainingOnPage = 0x1000 - offset;        

        // make sure we don't artificially read past the end of the page since we don't know if it will be available
        if (len == 3 && remainingOnPage < 4) {
            len = 2;
            missed = 1;
        } else if (len > 4 && len != 8 && remainingOnPage < 8) { // we havea  5, 6 or 7 len instruction/partial instruction, but we don't have the space to read 8 bytes on this page
            missed = len - 4;
            len = 4;            
        }

        if ((S32)address>=0 && (len==1 || len==2 || len==4)) {
            inlineCmp = true;
        } else {
            writeToRegFromValue(tmpReg2, true, address, 4);
        }

        if (len<=4) {        
            if (len==1) {
                U8 original = readb(address);

                if (inlineCmp) {
                    write8(0x41);
                    write8(0x80);
                    write8(0xb8 | HOST_MEM);
                    write8(0x24);
                    write32(address);
                    write8(original);
                } else {
                    // mov tmpReg2, [ip]        
                    writeToRegFromMem(tmpReg2, true, HOST_MEM, true, tmpReg2, true, 0, 0, 1, false);

                    // cmp tmpReg2, original
                    write8(0x41);
                    write8(0x80);
                    write8(0xf8+tmpReg2);
                    write8(original);
                }
            } else if (len==2) {
                U16 original = readw(address);

                if (inlineCmp) {
                    write8(0x66);
                    write8(0x41);
                    write8(0x81);
                    write8(0xb8 | HOST_MEM);
                    write8(0x24);
                    write32(address);
                    write16(original);
                } else {
                    // mov tmpReg2, [ip]        
                    writeToRegFromMem(tmpReg2, true, HOST_MEM, true, tmpReg2, true, 0, 0, 2, false);

                    // cmp tmpReg2, original
                    write8(0x66);
                    write8(0x41);
                    write8(0x81);
                    write8(0xf8+tmpReg2);
                    write16(original);
                }
            } else {
                U32 original = readd(address);

                if (inlineCmp) {
                    write8(0x41);
                    write8(0x81);
                    write8(0xb8 | HOST_MEM);
                    write8(0x24);
                    write32(address);
                    write32(original);
                } else {
                    if (len==3) {
                        original&=0x00FFFFFF;
                    }

                    // mov tmpReg2, [ip]        
                    writeToRegFromMem(tmpReg2, true, HOST_MEM, true, tmpReg2, true, 0, 0, 4, false);

                    if (len==3) {
                        // and tmpReg2, 0x00FFFFFF
                        write8(0x41);
                        write8(0x81);
                        write8(0xe0+tmpReg2);
                        write32(0x00FFFFFF);;
                    }
                    // cmp tmpReg2, original
                    write8(0x41);
                    write8(0x81);
                    write8(0xf8+tmpReg2);
                    write32(original);
                }
            }
        } else if (len<=8) {
            U64 original = readq(address);

            if (len==5) {
                original&=0x000000FFFFFFFFFFl;
            } else if (len==6) {            
                original&=0x0000FFFFFFFFFFFFl;
            } else if (len==7) {
                original&=0x00FFFFFFFFFFFFFFl;
            }
            // mov tmpReg1, original
            writeToRegFromValue(tmpReg1, true, original, 8);

            // mov tmpReg2, [ip]        
            writeToRegFromMem(tmpReg2, true, HOST_MEM, true, tmpReg2, true, 0, 0, 8, false);

            U32 shift = 0;
            if (len==5) {
                shift = 24;
            } else if (len==6) {
                shift = 16;
            } else if (len==7) {
                shift = 8;
            }

            if (shift) {
                // shl tmpReg2, 16
                write8(0x49);
                write8(0xc1);
                write8(0xe0+tmpReg2);
                write8(shift);

                // shr tmpReg2, 16
                write8(0x49);
                write8(0xc1);
                write8(0xe8+tmpReg2);
                write8(shift);
            }

            // cmp tmpReg2, tmpReg1
            write8(0x4d);
            write8(0x39);
            write8(0xc0 | tmpReg1 | (tmpReg2 << 3));            
        }        
        releaseTmpReg(tmpReg1);
        releaseTmpReg(tmpReg2);
    }
    // jz amount, will jump over the code to retranslate since the original and current x86 code are the same
    U32 pos;
    if (!panic) {
        write8(0x74);
        pos = this->bufferPos;
        write8(0);
    } else {
        write8(0x0f);
        write8(0x84);
        pos = this->bufferPos;
        write32(0);
    }
    if (saveAllFlags) {
        popFlagsFromReg(tmpReg3, true, true);
    } else if (saveLowBitFlags) {
        popFlagsFromReg(tmpReg3, true, false);
    }

    if (!panic) {
        write8(0xce); // will cause an exception that will retranslate this chunk
        this->buffer[pos] = this->bufferPos-pos-1;
    } else {
        syncRegsFromHost();

        lockParamReg(PARAM_1_REG, PARAM_1_REX);
        writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

        callHost((void*)x64_changed);
        syncRegsToHost();

        U32 tmp = this->bufferPos;
        this->bufferPos = pos;
        write32(tmp-pos-4);
        this->bufferPos = tmp;
    }    
    if (missed) {
        // :TODO: could probably be more efficient about comparing the last couple of bytes, maybe combining it above with another compare
        internal_addDynamicCheck(address + len, missed, needsFlags, panic, tmpReg3);
    }
}

void X64Asm::addDynamicCheck(bool panic) {
    DecodedBlock* block = NormalCPU::getBlockForInspectionButNotUsed(this->ip+this->cpu->seg[CS].address, this->cpu->isBig());
    U32 len = block->op->len;
    U32 needsFlags = instructionInfo[block->op->inst].flagsUsed | DecodedOp::getNeededFlags(block, block->op, OF|SF|ZF|PF|AF|CF);
    U32 address = this->startOfOpIp + this->cpu->seg[CS].address;
    U8 tmpReg3 = getTmpReg();
    bool saveAllFlags = (needsFlags & OF) != 0;
    bool saveLowBitFlags = needsFlags!=0 && !saveAllFlags;    
    
    if (saveAllFlags) {
        pushFlagsToReg(tmpReg3, true, true);
    } else if (saveLowBitFlags) {
        pushFlagsToReg(tmpReg3, true, false);
    }

    // :TODO: maybe find a way to cache this block for the next instruction?       
    if (len>8) {
        internal_addDynamicCheck(address+8, len-8, needsFlags, panic, tmpReg3);    
        len = 8;
    }
    internal_addDynamicCheck(address, len, needsFlags, panic, tmpReg3);    
    if (saveAllFlags) {
        popFlagsFromReg(tmpReg3, true, true);
    } else if (saveLowBitFlags) {
        popFlagsFromReg(tmpReg3, true, false);
    }  
    releaseTmpReg(tmpReg3);
    block->dealloc(false); 
}

void X64Asm::doLoop(U32 eip) {
    // :TODO: maybe find one byte offset
    U32 pos = this->bufferPos;
    write8(0); // skip over the next jmp
    jumpTo(this->ip); // jump to next instruction after this because condition was not met
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
    write8(0x0f);
    write8(0xb7);
    write8(0xc9);

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
    syncRegsFromHost(); 

    writeToMemFromValue(oldEip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);
    // call void common_jmp(cpu, false, sel, offset, oldEip)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, big, 4); // big param

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, sel, 4); // sel param

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, offset, 4); // offset param
    
    callHost((void*)x64_jmp);
    syncRegsToHost();
    doJmp(true);
}

void x64_call(x64CPU* cpu, U32 big, U32 selector, U32 offset) {
    cpu->call(big, selector, offset, cpu->arg5);
}

// common_jmp(cpu, false, sel, offset, oldEip);
void X64Asm::call(bool big, U32 sel, U32 offset, U32 oldEip) {
    syncRegsFromHost(); 

    writeToMemFromValue(oldEip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);

    // call void common_call(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, big, 4); // big param

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, sel, 4); // sel param

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, offset, 4); // offset param
    
    callHost((void*)x64_call);
    syncRegsToHost();
    doJmp(true);
}

void X64Asm::shiftRightReg(U8 reg, bool isRegRex, U8 shiftAmount) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0xC1);
    write8(0xE8 | reg);
    write8(shiftAmount);
}

void X64Asm::andReg(U8 reg, bool isRegRex, U32 mask) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0x81);
    write8(0xE0 | reg);
    write32(mask);
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
    if (KSystem::useLargeAddressSpace) {
        if (reg != 1 || !isRex) {
            writeToRegFromReg(1, true, reg, isRex, 4);
        }
        if (this->cpu->thread->process->hasSetSeg[CS] || mightNeedCS) {
            addWithLea(1, true, 1, true, getRegForSeg(CS, 0), true, 0, 0, 4);
        }
#ifdef BOXEDWINE_BT_DEBUG_NO_EXCEPTIONS
        writeToRegFromMem(0, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_JMP_AND_TRANSLATE_IF_NECESSARY, 8, false);
        jmpNativeReg(0, true);
#else
        // must use r9, the exception handler expects it
        write8(REX_BASE | REX_MOD_RM | REX_SIB_INDEX);
        write8(0xff);
        write8(0x24);
        write8(0xc0 | HOST_LARGE_ADDRESS_SPACE_MAPPING | (1<<3));
#endif
    } else {
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
            pushFlagsToReg(HOST_TMP3, true, true);

            // shr HOST_TMP2, 12
            shiftRightReg(HOST_TMP2, true, K_PAGE_SHIFT);

            // and HOST_TMP, 0xFFF
            andReg(HOST_TMP, true, K_PAGE_MASK);

            popFlagsFromReg(HOST_TMP3, true, true);
        }

        // rax=cpu->opToAddressPages
        // mov rax, [HOST_CPU];
        writeToRegFromMem(HOST_TMP3, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_OP_PAGES, 8, false);

        // HOST_TMP3 = cpu->opToAddressPages[page]
        // mov HOST_TMP3, [HOST_TMP3+HOST_TEMP2<<3] // 3 sizeof(void*)
        writeToRegFromMem(HOST_TMP3, true, HOST_TMP3, true, HOST_TMP2, true, 3, 0, 8, false);

        // will move address to HOST_TMP3 and test that it exists, if it doesn't then we
        // will catch the exception.  We leave the address/index we need in HOST_TMP
        // and HOST_TMP2

        // HOST_TMP = cpu->opToAddressPages[page][offset]
        // mov HOST_TMP3, [HOST_TMP3 + HOST_TMP << 3]
        writeToRegFromMem(HOST_TMP3, true, HOST_TMP3, true, HOST_TMP, true, 3, 0, 8, false);

        // This will test that the value we are about to jump to exists
        // mov HOST_TMP, [HOST_TMP3]
        writeToRegFromMem(HOST_TMP, true, HOST_TMP3, true, -1, false, 0, 0, 2, false);

        // jmp HOST_TMP
        jmpNativeReg(HOST_TMP3, true);
    }
}

void X64Asm::jmpNativeReg(U8 reg, bool isRegRex) {
    if (isRegRex)
        write8(REX_BASE | REX_MOD_RM);
    write8(0xff);
    write8((0x04 << 3) | 0xC0 | reg);
}

void X64Asm::retn16(U32 bytes) {
    U32 tmpReg = getTmpReg();
    popReg16(tmpReg, true);
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
    syncRegsFromHost(); 

    // call void common_ret(CPU* cpu, U32 big, U32 bytes)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, big, 4); // big param

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, bytes, 4); // bytes param
    
    callHost((void*)common_ret);
    syncRegsToHost();
    doJmp(true);
}

void X64Asm::iret(U32 big, U32 oldEip) {
    syncRegsFromHost(); 

    // call void common_iret(CPU* cpu, U32 big, U32 oldEip)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, big, 4); // big param

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, oldEip, 4); // sel param
    
    callHost((void*)common_iret);
    syncRegsToHost();
    doJmp(true);
}

void X64Asm::signalIllegalInstruction(int code) {
    syncRegsFromHost(); 

    // void common_signalIllegalInstruction(CPU* cpu, int code)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, code, 4); // code param
    
    callHost((void*)common_signalIllegalInstruction);
    syncRegsToHost();
    doJmp(true);
}

static U8 fetchByte(U32 *eip) {
    return readb((*eip)++);
}

static void x64log(CPU* cpu) {
    if (!cpu->logFile)
        return;
    THREAD_LOCAL static DecodedBlock* block;
    if (!block) {
        block = new DecodedBlock();
    }
    decodeBlock(fetchByte, cpu->eip.u32+cpu->seg[CS].address, cpu->isBig(), 1, K_PAGE_SIZE, 0, block);
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

    // void ksyscall(cpu, op->len)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, opLen, 4); // opLen param
    
    callHost((void*)ksyscall);
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
    minSyncRegsFromHost();

    // void common_int98(CPU* cpu)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_int98);

    minSyncRegsToHost();
    //doJmp();
}

void X64Asm::int99(U32 opLen) {
    minSyncRegsFromHost();

    // void common_int99(CPU* cpu)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_int99);
    minSyncRegsToHost();
    //doJmp();
}

void X64Asm::int9A(U32 opLen) {
    minSyncRegsFromHost();

    // void common_int9A(CPU* cpu)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_int9A);
    minSyncRegsToHost();
    //doJmp();
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
    U8 tmpReg = getTmpReg();
    addWithLea(7, false, 7, false, getRegForSeg(this->ds, tmpReg), true, 0, 0, 4);
    U8 hostReg = getHostMem(7, false);
    addWithLea(7, false, 7, false, HOST_MEM, true, 0, 0, 8);    

    this->translateRM(rm, false, false, false, false, 0);

    if (hostReg == HOST_MEM) {
        writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_NEG_MEM, 8, false);
    } else {
        pushFlagsToReg(tmpReg, true, true);
        // neg hostReg
        write8(0x49);
        write8(0xf7);
        write8(0xd8 + hostReg);
        popFlagsFromReg(tmpReg, true, true);
    }

    releaseHostMem(hostReg);

    addWithLea(7, false, 7, false, tmpReg, true, 0, 0, 8);    
    addWithLea(7, false, 7, false, getRegForNegSeg(this->ds, tmpReg), true, 0, 0, 4);    
    releaseTmpReg(tmpReg);
}

void X64Asm::string32(bool hasSi, bool hasDi) {    
    U8 tmpReg = getTmpReg();
    bool hasSetES = this->cpu->thread->process->hasSetSeg[ES];
    bool hasSetDS = this->cpu->thread->process->hasSetSeg[this->ds];

    if (hasDi && hasSetES) {
        addWithLea(7, false, 7, false, getRegForSeg(ES, tmpReg), true, 0, 0, 4);
    }
    if (hasSi && hasSetDS) {
        addWithLea(6, false, 6, false, getRegForSeg(this->ds, tmpReg), true, 0, 0, 4);
    }
    releaseTmpReg(tmpReg);

    if (hasDi) {
        addWithLea(7, false, 7, false, HOST_MEM, true, 0, 0, 8);        
    }
    if (hasSi) {
        addWithLea(6, false, 6, false, HOST_MEM, true, 0, 0, 8);
    }
    
    writeOp();

    tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_NEG_MEM, 8, false);
    if (hasSi) {
        addWithLea(6, false, 6, false, tmpReg, true, 0, 0, 8);
    }
    if (hasDi) {
        addWithLea(7, false, 7, false, tmpReg, true, 0, 0, 8);
    }
    if (hasSi && hasSetDS) {
        addWithLea(6, false, 6, false, getRegForNegSeg(this->ds, tmpReg), true, 0, 0, 4);
    }
    if (hasDi && hasSetES) {
        addWithLea(7, false, 7, false, getRegForNegSeg(ES, tmpReg), true, 0, 0, 4);
    }    
        
    releaseTmpReg(tmpReg);
}

// U16 val = readw(eaa);
// U32 selector = readw(eaa+2);
// if (cpu->setSegment(op->imm, selector)) {
//    cpu->reg[op->reg].u16 = val;
void X64Asm::loadSeg(U8 seg, U8 rm, bool b32) {
    if (rm<0xC0) {
        // in case an exception is thrown, the exception will need to store the regs
        // also this will help will saving ECX and EDX as required by Window ABI
        syncRegsFromHost(); 

        U8 tmpReg = getParamSafeTmpReg();
        getNativeAddressInRegFromE(tmpReg, true, rm);

        // read selector and put it into PARAM_3 for function call
        lockParamReg(PARAM_3_REG, PARAM_3_REX);
        zeroReg(PARAM_3_REG, PARAM_3_REX, false);
        writeToRegFromMem(PARAM_3_REG, PARAM_3_REX, tmpReg, true, -1, false, 0, b32?4:2, 2, false);

        // read value now in case it triggers an exception                
        writeToRegFromMem(tmpReg, true, tmpReg, true, -1, false, 0, 0, b32?4:2, false);        
        if (!b32) {
            andReg(tmpReg, true, 0x0000ffff);
        }
        writeToMemFromReg(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false);
        releaseTmpReg(tmpReg);

        // call U32 common_setSegment(CPU* cpu, U32 seg, U32 value)
        lockParamReg(PARAM_1_REG, PARAM_1_REX);
        writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

        lockParamReg(PARAM_2_REG, PARAM_2_REX);
        writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, seg, 4); // value param, must pass 4 so that upper part of reg is zero'd out        

        callHost((void*)common_setSegment);

        doIf(0, false, 0, [this]() {
            syncRegsToHost();
            doJmp(true);
        }, [this, rm, b32]() {
            syncRegsToHost();
            // put the value we stored on the native stack into the emulator reg
            U8 r = G(rm);
            if (r==4) {
                writeToRegFromMem(HOST_ESP, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, b32?4:2, false);
            } else {
                writeToRegFromMem(r, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, b32?4:2, false);
            }
        }); 
        this->cpu->thread->process->hasSetSeg[seg] = true;
    } else {
        this->invalidOp(this->inst);
        this->done = true;
    }
}

void X64Asm::enter(bool big, U32 bytes, U32 level) {
    if (level!=0) {
        // in case an exception is thrown, the exception will need to store the regs
        // also this will help will saving ECX and EDX as required by Window ABI
        syncRegsFromHost(); 


        // call void common_enter(CPU* cpu, U32 big, U32 bytes, U32 level)
        lockParamReg(PARAM_1_REG, PARAM_1_REX);
        writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

        lockParamReg(PARAM_2_REG, PARAM_2_REX);
        writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (big?1:0), 4);

        lockParamReg(PARAM_3_REG, PARAM_3_REX);
        writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, bytes, 4);

        lockParamReg(PARAM_4_REG, PARAM_4_REX);
        writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, level, 4);

        callHost((void*)common_enter);
    
        syncRegsToHost();
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
    if (!big) {
        zeroReg(tmpReg, true, true);
    }
    writeToRegFromE(tmpReg, true, rm, (big?4:2));
    push(-1, false, this->ip, (big?4:2)); 
    jmpReg(tmpReg, true, false);
    releaseTmpReg(tmpReg);
}

void X64Asm::callJmp(bool big, U8 rm, bool jmp) {
    syncRegsFromHost(); 
     
    // calling convention RCX, RDX, R8, R9 for first 4 parameters
    U8 tmpReg = getParamSafeTmpReg();
    getNativeAddressInRegFromE(tmpReg, true, rm);

    writeToMemFromValue(this->ip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ARG5, 4, false); // next ip

    // call void common_call(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    if (big) {
        writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, 1, 4); // big param
    } else {
        zeroReg(PARAM_2_REG, PARAM_2_REX, false);
    }    
    U8 bytes = (big?4:2);            

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    if (bytes<4) {
        zeroReg(PARAM_4_REG, PARAM_4_REX, false);
    }
    writeToRegFromMem(PARAM_4_REG, PARAM_4_REX, tmpReg, true, -1, false, 0, 0, bytes, false); // offset

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    zeroReg(PARAM_3_REG, PARAM_3_REX, false);
    writeToRegFromMem(PARAM_3_REG, PARAM_3_REX, tmpReg, true, -1, false, 0, bytes, 2, false); // seg

    releaseTmpReg(tmpReg);
    
    if (jmp) {
        callHost((void*)x64_jmp);
    } else {
        callHost((void*)x64_call);
    }
    syncRegsToHost();
    doJmp(true);  
}

void X64Asm::callFar(bool big, U8 rm) {    
    callJmp(big, rm, false);   
}

void X64Asm::jmpE(bool big, U8 rm) {
    U8 tmpReg = getTmpReg();
    if (!big) {
        zeroReg(tmpReg, true, true);
    }
    writeToRegFromE(tmpReg, true, rm, (big?4:2));
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
    syncRegsFromHost(); 

    // call U32 common_lsl(CPU* cpu, U32 selector, U32 limit)

    // G(rm) could be PARAM_2_REG which will be overwritten
    U8 tmp1 = getParamSafeTmpReg();
    writeToRegFromReg(tmp1, true, G(rm), false, 4);

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromE(PARAM_2_REG, PARAM_2_REX, rm, 2); // load before overwriting PARAM_1_REG
    andReg(PARAM_2_REG, PARAM_2_REX, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references PARAM_2_REG

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    if (!big) {
        zeroReg(PARAM_3_REG, PARAM_3_REX, false);
    }
    writeToRegFromReg(PARAM_3_REG, PARAM_3_REX, tmp1, true, (big?4:2));     
    releaseTmpReg(tmp1);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
            
    callHost((void*)common_lsl);
    if (big) {
        if (G(rm)!=0) {
            writeToRegFromReg(G(rm), false, 0, false, 4);
        }
    } else {
        U8 tmpReg = getTmpReg();
        if (G(rm)==0) {
            writeToRegFromReg(tmpReg, true, 0, false, 4);
        }
        // fill in the previous upper 2 bytes
        writeToRegFromMem(G(rm), false, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, reg[G(rm)].u32)), 4, false);
        if (G(rm)==0) {
            writeToRegFromReg(G(rm), false, tmpReg, true, 2);
        } else {
            writeToRegFromReg(G(rm), false, 0, false, 2);
        }
        releaseTmpReg(tmpReg);
    }
    syncRegsToHost(G(rm));
}

void X64Asm::lar(bool big, U8 rm) {
    syncRegsFromHost(); 

    // call U32 common_lar(CPU* cpu, U32 selector, U32 limit)

    // G(rm) could be PARAM_2_REG which will be overwritten
    U8 tmp1 = getParamSafeTmpReg();
    writeToRegFromReg(tmp1, true, G(rm), false, 4);

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromE(PARAM_2_REG, PARAM_2_REX, rm, 2); // load before overwriting PARAM_1_REG
    andReg(PARAM_2_REG, PARAM_2_REX, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references PARAM_2_REG

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    if (!big) {
        zeroReg(PARAM_3_REG, PARAM_3_REX, false);
    }
    writeToRegFromReg(PARAM_3_REG, PARAM_3_REX, tmp1, true, (big?4:2));     
    releaseTmpReg(tmp1);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
            
    callHost((void*)common_lar);
    if (big) {
        if (G(rm)!=0) {
            writeToRegFromReg(G(rm), false, 0, false, 4);
        }
    } else {
        U8 tmpReg = getTmpReg();
        if (G(rm)==0) {
            writeToRegFromReg(tmpReg, true, 0, false, 4);
        }
        // fill in the previous upper 2 bytes
        writeToRegFromMem(G(rm), false, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, reg[G(rm)].u32)), 4, false);
        if (G(rm)==0) {
            writeToRegFromReg(G(rm), false, tmpReg, true, 2);
        } else {
            writeToRegFromReg(G(rm), false, 0, false, 2);
        }
        releaseTmpReg(tmpReg);
    }
    syncRegsToHost(G(rm));
}

void X64Asm::verw(U8 rm) {
    syncRegsFromHost(); 

    // call void common_verw(CPU* cpu, U32 selector)
    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromE(PARAM_2_REG, PARAM_2_REX, rm, 2);
    andReg(PARAM_2_REG, PARAM_2_REX, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references PARAM_2_REG

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
            
    callHost((void*)common_verw);
    syncRegsToHost();
}

void X64Asm::verr(U8 rm) {
    syncRegsFromHost(); 

    // call void common_verr(CPU* cpu, U32 selector)
    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromE(PARAM_2_REG, PARAM_2_REX, rm, 2);
    andReg(PARAM_2_REG, PARAM_2_REX, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references PARAM_2_REG

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
            
    callHost((void*)common_verr);
    syncRegsToHost();
}

static void x64_invalidOp(CPU* cpu, U32 op) {
    klog("x64_invalidOp: 0x%X", op);
    cpu->thread->signalIllegalInstruction(5);
}

void X64Asm::invalidOp(U32 op) {
    syncRegsFromHost(); 
    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, op, 4);
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
    callHost((void*)x64_invalidOp);
    syncRegsToHost();
    doJmp(true);
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
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_cpuid(CPU* cpu)
    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param
            
    callHost((void*)common_cpuid);
    syncRegsToHost();
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
    if (cpu->flags & DF) {
        cpu->df = -1;
    } else {
        cpu->df = 1;
    }
    BtCodeMemoryWrite w(cpu);
    if (cpu->stringWritesToDi) {
        w.invalidateStringWriteToDi(cpu->stringRepeat!=0, size);
    }
    pfn(cpu);
    cpu->fillFlags();
}

typedef void (*pfnString1Arg)(CPU* cpu, U32 arg1);

void x64_string1Arg(x64CPU* cpu, pfnString1Arg pfn, U32 arg1, U32 size) {
    if (cpu->flags & DF) {
        cpu->df = -1;
    } else {
        cpu->df = 1;
    }
    BtCodeMemoryWrite w(cpu);
    if (cpu->stringWritesToDi) {
        w.invalidateStringWriteToDi(cpu->stringRepeat!=0, size);
    }
    pfn(cpu, arg1);
    cpu->fillFlags();
}

typedef void (*pfnString2Arg)(CPU* cpu, U32 arg1, U32 arg2);

void x64_string2Arg(x64CPU* cpu, pfnString2Arg pfn, U32 arg1, U32 arg2) {
    U32 size = arg2 >> 16;

    arg2&=0xFFFF;
    if (cpu->flags & DF) {
        cpu->df = -1;
    } else {
        cpu->df = 1;
    }
    BtCodeMemoryWrite w(cpu);
    if (cpu->stringWritesToDi) {
        w.invalidateStringWriteToDi(cpu->stringRepeat!=0, size);
    }
    pfn(cpu, arg1, arg2);
    cpu->fillFlags();
}

void X64Asm::stos(void* pfn, U32 size, bool repeat) {
    writeToMemFromValue(1, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_WRITES_DI, 4, false);
    writeToMemFromValue(repeat?1:0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_REPEAT, 4, false);
    syncRegsFromHost(); 

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, size, 4);

    callHost((void*)x64_stringNoArgs);
    syncRegsToHost();
}

void X64Asm::scas(void* pfn, U32 size, bool repeat, bool repeatZero) {
    writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_WRITES_DI, 4, false);
    writeToMemFromValue(repeat?1:0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_REPEAT, 4, false);
    syncRegsFromHost(); 

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, (U32)repeatZero?1:0, 4);

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, size, 4);

    callHost((void*)x64_string1Arg);
    syncRegsToHost();
}

void X64Asm::movs(void* pfn, U32 size, bool repeat, U32 base) {
    writeToMemFromValue(1, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_WRITES_DI, 4, false);
    writeToMemFromValue(repeat?1:0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_REPEAT, 4, false);
    syncRegsFromHost(); 

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, base, 4);

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, size, 4);

    callHost((void*)x64_string1Arg);
    syncRegsToHost();
}

void X64Asm::lods(void* pfn, U32 size, bool repeat, U32 base) {
    writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_WRITES_DI, 4, false);
    writeToMemFromValue(repeat?1:0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_REPEAT, 4, false);
    syncRegsFromHost(); 

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, base, 4);

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, size, 4);

    callHost((void*)x64_string1Arg);
    syncRegsToHost();
}

void X64Asm::cmps(void* pfn, U32 size, bool repeat, bool repeatZero, U32 base) {
    writeToMemFromValue(0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_WRITES_DI, 4, false);
    writeToMemFromValue(repeat?1:0, HOST_CPU, true, -1, false, 0, CPU_OFFSET_STRING_REPEAT, 4, false);
    syncRegsFromHost(); 

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, (U32)repeatZero?1:0, 4);

    lockParamReg(PARAM_4_REG, PARAM_4_REX);
    writeToRegFromValue(PARAM_4_REG, PARAM_4_REX, base|(size<<16), 4);

    callHost((void*)x64_string2Arg);
    syncRegsToHost();
}

// :TODO: could be inlined
// U32 common_bound16(CPU* cpu, U32 reg, U32 address)
void X64Asm::bound16(U8 rm) {
    syncRegsFromHost();

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    getNativeAddressInRegFromE(PARAM_3_REG, PARAM_3_REX, rm);

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, G(rm), 4);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_bound16);

    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp(true);
    }, [this]() {
        syncRegsToHost();
    });
}

// U32 common_bound32(CPU* cpu, U32 reg, U32 address)
void X64Asm::bound32(U8 rm) {
    syncRegsFromHost();
    
    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    getNativeAddressInRegFromE(PARAM_3_REG, PARAM_3_REX, rm);

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, G(rm), 4);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_bound32);

    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp(true);
    }, [this]() {
        syncRegsToHost();
    }); 
}

void X64Asm::movRdCrx(U32 which, U32 reg) {
    syncRegsFromHost();

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, reg, 4);

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, which, 4);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_readCrx);
    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp(true);
    }, [this]() {
        syncRegsToHost();
    }); 
}

void X64Asm::movCrxRd(U32 which, U32 reg) {
    syncRegsFromHost();

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromReg(PARAM_3_REG, PARAM_3_REX, reg, false, 4);

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, which, 4);

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)common_readCrx);
    releaseTmpReg(HOST_TMP2);
    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp(true);
    }, [this]() {
        syncRegsToHost();
    }); 
}

void common_fpu_write_address(x64CPU* cpu, PFN_FPU_ADDRESS pfn, U32 address, U32 len) {
    BtCodeMemoryWrite w(cpu, address, len);
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
    getNativeAddressInRegFromE(PARAM_2_REG, PARAM_2_REX, rm);    

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    callHost((void*)pfn);
    syncRegsToHost();
}

void X64Asm::callFpuWithAddressWrite(PFN_FPU_ADDRESS pfn, U8 rm, U32 len) {
    syncRegsFromHost();
    getNativeAddressInRegFromE(0, true, rm);  

    lockParamReg(PARAM_1_REG, PARAM_1_REX);
    writeToRegFromReg(PARAM_1_REG, PARAM_1_REX, HOST_CPU, true, 8); // CPU* param

    lockParamReg(PARAM_2_REG, PARAM_2_REX);
    writeToRegFromValue(PARAM_2_REG, PARAM_2_REX, (U64)pfn, 8);

    lockParamReg(PARAM_3_REG, PARAM_3_REX);
    writeToRegFromValue(PARAM_3_REG, PARAM_3_REX, len, 4);

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
        syncRegsFromHost(true);
        callHost((void*)x64_jmpAndTranslateIfNecessaryAdjustForCS);
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
    
    // :TODO: full rsi and rdi too?

    writeToRegFromMem(0, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(x64CPU, exceptionR8)), 8, false);
    writeToRegFromMem(1, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(x64CPU, exceptionR9)), 8, false);
    writeToRegFromMem(2, true, HOST_CPU, true, -1, false, 0, (U32)(offsetof(x64CPU, exceptionR10)), 8, false);

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
