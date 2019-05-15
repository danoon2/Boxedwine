#include "boxedwine.h"

#ifdef BOXEDWINE_X64

#include "x64Asm.h"
#include "../common/common_other.h"

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

X64Asm::X64Asm(x64CPU* cpu) : X64Data(cpu), parent(NULL), tmp1InUse(false), tmp2InUse(false), tmp3InUse(false) {
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

void X64Asm::zeroReg(U8 reg, bool isRexReg) {
    // xor would use less bytes, but it changes the flags
    if (isRexReg)
        this->write8(REX_BASE | REX_MOD_RM);
    this->write8(0xb8 + reg);
    this->write32(0);
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
    if (isReg3Rex)
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
                
        this->write8(rm);
        this->write8((reg3Shift<<6) | (reg3 << 3) | reg2); // sib
    }

    if (reg1==HOST_ESP && isReg1Rex && bytes == 2) {
        // :TODO: should top 16-bits be cleared?
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
    setSib(HOST_MEM | (tmpReg << 3), false); 
}

// reg1 = [reg2 + (reg3 << reg3Shift) + displacement]
//
// reg3 is optional, pass -1 to ignore it
// displacement is optional, pass 0 to ignore it
void X64Asm::writeToRegFromMem(U8 toReg, bool isToRegRex, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost) {
    if (translateToHost) {
        U32 tmpReg = getTmpReg();
        addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
        writeToRegFromMem(toReg, isToRegRex, tmpReg, true, HOST_MEM, true, 0, 0, bytes, false);
        releaseTmpReg(tmpReg);
    } else {
        doMemoryInstruction(bytes==1?0x8a:0x8b, toReg, isToRegRex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
    }
}

U8 X64Asm::getRegForSeg(U8 base, U8 tmpReg) {
    if (base == ES) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_ES_ADDRESS, 4, false); return tmpReg;}
    if (base == SS) {return HOST_SS;}
    if (base == GS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_GS_ADDRESS, 4, false); return tmpReg;}
    if (base == FS) {writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FS_ADDRESS, 4, false); return tmpReg;}
    if (base == DS) {return HOST_DS;}
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
    zeroReg(tmpReg, true);
    if (r1>=0) {        
        addWithLea(tmpReg, true, r1, false, r2, false, 0, disp, 2);
    } else {
        writeToRegFromValue(tmpReg, true, disp, 2);
    }

    // tmpReg = tmpReg + seg
    addWithLea(tmpReg, true, tmpReg, true, getRegForSeg(seg, tmpReg2), true, 0, 0, 4);

    // [HOST_MEM + tmpReg]
    writeHostPlusTmp((rm & (7<<3)) | 4, checkG, isG8bit, isE8bit, tmpReg);

    releaseTmpReg(tmpReg);
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
                
                    U32 tmpReg = getTmpReg();
                    // don't need to worry about E(rm) == 5, that is handled below
                    // HOST_TMP = reg + SEG
                    addWithLea(tmpReg, true, E(rm), false, getRegForSeg(this->ds, tmpReg), true, 0, 0, 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                    releaseTmpReg(tmpReg);
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
                    // converts [disp32] to HOST_TMP = [SEG + disp32]; [HOST_TMP+HOST_MEM]

                    U32 tmpReg = getTmpReg();
                    // HOST_TMP = SEG + disp32
                    addWithLea(tmpReg, true, getRegForSeg(this->ds, tmpReg), true, -1, false, 0, this->fetch32(), 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                    releaseTmpReg(tmpReg);
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
                            releaseTmpReg(tmpReg);
                        }                        
                    } else { // [base + index << shift]
                        if (this->ds == SEG_ZERO) {
                            // keep the same, but convert ESP to HOST_ESP
                            setRM(rm, checkG, false, isG8bit, isE8bit);
                            setSib(sib, true);
                        } else {
                            // convert [base + index << shift] to HOST_TMP=[base + index << shift];HOST_TMP=[HOST_TMP+SEG];[HOST_TMP+MEM]

                            U32 tmpReg = getTmpReg();
                            // HOST_TMP=[base + SEG]
                            addWithLea(tmpReg, true, base, false,  getRegForSeg(base==4?this->ss:this->ds, tmpReg), true, 0, 0, 4);

                            // HOST_TMP=[HOST_TMP+index<<shift];
                            if (index!=4) {
                                addWithLea(tmpReg, true, tmpReg, true, index, false, sib >> 6, 0, 4);
                            }

                            // [HOST_MEM + HOST_TMP]
                            writeHostPlusTmp((rm & ~(7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
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
                    // HOST_TMP = [reg + SEG + disp]
                    addWithLea(tmpReg, true, E(rm), false, getRegForSeg(E(rm)==5?this->ss:this->ds, tmpReg), true, 0, (rm<0x80?(S8)this->fetch8():this->fetch32()), 4);

                    // [HOST_MEM + HOST_TMP]
                    writeHostPlusTmp((rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
                    releaseTmpReg(tmpReg);
                }
                break;
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

                        U32 tmpReg = getTmpReg();
                        // HOST_TMP=SEG+base+disp
                        addWithLea(tmpReg, true, base, false, getRegForSeg(base==4 || base==5?this->ss:this->ds, tmpReg), true, 0, (rm<0x80?(S8)this->fetch8():this->fetch32()), 4);

                        // HOST_TMP = HOST_TMP + index << shift
                        if (index!=4) {
                            addWithLea(tmpReg, true, tmpReg, true, index , false, sib >> 6, 0, 4);
                        }

                        // [HOST_MEM + HOST_TMP]
                        writeHostPlusTmp((rm & ~(0xC7)) | 4, checkG, isG8bit, isE8bit, tmpReg);
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
}

void X64Asm::writeToEFromCpuOffset(U8 rm, U32 offset, U8 fromBytes, U8 toBytes) {
    U8 tmpReg = getTmpReg();
    if (fromBytes==2 && toBytes==4) {
        zeroReg(tmpReg, true);
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
        if (this->cpu->big) {
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
    if (isRegRex) {
        this->rex = REX_BASE | REX_MOD_REG;
    } else {
        this->rex = 0;
    }
    if (bytes==2) {
        if (this->cpu->big) {
            this->operandPrefix = true;
        }
        this->op = 0x8b;
    } else if (bytes==4) {
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
    if (this->cpu->big) {
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
    releaseTmpReg(tmpReg);

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
void X64Asm::push(S32 reg, bool isRegRex, U32 value, S32 bytes) {
    U8 tmpReg = getTmpReg();

    if (reg==4 && !isRegRex) {
        reg = HOST_ESP;
        isRegRex = true;
    }

    // store flags because the &, - instructions will affect flags
    pushNativeFlags();

    // tmpReg = HOST_ESP - bytes
    addWithLea(tmpReg, true, HOST_ESP, true, -1, false, 0, -bytes, 4);

    // tmpReg &= cpu->stackMask
    andWriteToRegFromCPU(tmpReg, true, CPU_OFFSET_STACK_MASK);

    // [ss:tmpReg] = reg
    if (reg>=0) {
        writeToMemFromReg(reg, isRegRex, tmpReg, true, HOST_SS, true, 0, false, bytes, true);
    } else {
        writeToMemFromValue(value, tmpReg, true, HOST_SS, true, 0, false, bytes, true);
    }

    // HOST_ESP = HOST_ESP & cpu->stackNotMask
    andWriteToRegFromCPU(HOST_ESP, true, CPU_OFFSET_STACK_NOT_MASK);

    // HOST_ESP = HOST_ESP | tmpReg
    orRegReg(HOST_ESP, true, tmpReg, true);

    releaseTmpReg(tmpReg);

    // restore original flags
    popNativeFlags();
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

void X64Asm::writeToRegFromMemAddress(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes) {
    U8 tmpReg = getTmpReg();
    writeToRegFromMem(reg, isRegRex, getRegForSeg(seg, tmpReg), true, -1, false, 0, disp, bytes, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::writeToMemAddressFromReg(U8 seg, U8 reg, bool isRegRex, U32 disp, U8 bytes) {
    U8 tmpReg = getTmpReg();
    writeToMemFromReg(reg, isRegRex, getRegForSeg(seg, tmpReg), true, -1, false, 0, disp, bytes, true);
    releaseTmpReg(tmpReg);
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
        popReg16(tmpReg, true);
        writeToEFromReg(rm, tmpReg, true, 2);
        releaseTmpReg(tmpReg);
    } else {
        popReg16(rm & 7, false);
    }
}

void X64Asm::popd(U8 rm) {
    if (rm<0xC0) {     
        U32 tmpReg = getTmpReg();
        popReg32(tmpReg, true);
        writeToEFromReg(rm, tmpReg, true, 4);
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

void X64Asm::pushNativeValue64(U64 value) {
    U8 tmp = getTmpReg();
    writeToRegFromValue(tmp, true, value, 8);
    releaseTmpReg(tmp);
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
        U8 tmpReg = getTmpReg();
        addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
        writeToMemFromReg(reg1, isReg1Rex, tmpReg, true, HOST_MEM, true, 0, 0, bytes, false);
        releaseTmpReg(tmpReg);
    } else {
        // reg 1 will be ignored
        doMemoryInstruction(bytes==1?0x88:0x89, reg1, isReg1Rex, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, bytes);
    }
}

void X64Asm::writeToMemFromValue(U64 value, U8 reg2, bool isReg2Rex, S8 reg3, bool isReg3Rex, U8 reg3Shift, S32 displacement, U8 bytes, bool translateToHost) {
    if (translateToHost) {
        U8 tmpReg = getTmpReg();
        addWithLea(tmpReg, true, reg2, isReg2Rex, reg3, isReg3Rex, reg3Shift, displacement, 4);
        writeToMemFromValue(value, tmpReg, true, HOST_MEM, true, 0, 0, bytes, false);
        releaseTmpReg(tmpReg);
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

void X64Asm::adjustStack(bool big, U8 tmpReg, S8 bytes) {
    // tmpReg = HOST_ESP + bytes
    addWithLea(tmpReg, true, HOST_ESP, true, -1, false, 0, bytes, 4);

    // tmpReg &= cpu->stackMask
    andWriteToRegFromCPU(tmpReg, true, CPU_OFFSET_STACK_MASK);

    // HOST_ESP = HOST_ESP & cpu->stackNotMask
    andWriteToRegFromCPU(HOST_ESP, true, CPU_OFFSET_STACK_NOT_MASK);

    // HOST_ESP = HOST_ESP | tmpReg
    orRegReg(HOST_ESP, true, tmpReg, true);
}

// :TODO: is there a way to do this without changing flags so that I don't have to save them, 
// plus it will mess up the stack if an exception is thrown
//
// could also store flags in a tmpReg if that stack becomes a problem
//
// If this function changes, apply it to enter16/enter32

// c code
// reg = readd(cpu->thread, cpu->segAddress[SS] + (ESP & cpu->stackMask));
// ESP = (ESP & cpu->stackNotMask) | ((ESP + 4 ) & cpu->stackMask);

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

    // store flags because the &, - instructions will affect flags
    pushNativeFlags();    

    // tmpReg = HOST_ESP
    writeToRegFromReg(tmpReg, true, HOST_ESP, true, 4);

    // tmpReg &= cpu->stackMask
    andWriteToRegFromCPU(tmpReg, true, CPU_OFFSET_STACK_MASK);

    // reg = [ss:tmpReg]    
    writeToRegFromMem(reg, isRegRex, tmpReg, true, HOST_SS, true, 0, false, bytes, true);
    
    if (commit && (reg!=HOST_ESP || !isRegRex)) {
        if (!allocatedTmpReg) {
            allocatedTmpReg = true;
            tmpReg = getTmpReg();
        }
        adjustStack(bytes==4, tmpReg, bytes);
    }
    if (allocatedTmpReg)
        releaseTmpReg(tmpReg);

    // restore original flags
    popNativeFlags();
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
    }else {
        kpanic("x64: tried to release an unknown reg");
    }
}

bool X64Asm::isTmpReg(U8 tmpReg) {
    return tmpReg == HOST_TMP || tmpReg == HOST_TMP2 || tmpReg == HOST_TMP3;
}

void X64Asm::syncRegsFromHost() {
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

    writeToRegFromMem(HOST_SS, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_SS_ADDRESS, 4, false);
    writeToRegFromMem(HOST_DS, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_DS_ADDRESS, 4, false);

    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_FLAGS, 4, false);
    pushNativeReg(tmpReg, true);
    popNativeFlags();
    releaseTmpReg(tmpReg);
}

void X64Asm::callHost(void* pfn) {
    U8 tmp = HOST_TMP3;
    
    if (this->tmp3InUse) {
        kpanic("x64Asm::callHost tmp3InUse already in use");        
    }
    tmp3InUse = true;
    write8(0xfc); // cld

    writeToRegFromValue(tmp, true, (U64)pfn, 8);

    // part of the x64 windows ABI, shadow store
    // sub rsp, 32
    write8(REX_BASE | REX_64);
    write8(0x83);
    write8(0xEC);
    write8(0x20);    

    // call tmp
    write8(REX_BASE | REX_MOD_RM);
    write8(0xFF);
    write8(0xd0 | tmp);

    // add rsp, 20
    write8(REX_BASE | REX_64);
    write8(0x83);
    write8(0xC4);
    write8(0x20);

    releaseTmpReg(tmp);
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

void X64Asm::doJmp() {
    U8 tmpReg = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    jmpReg(tmpReg, true);
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

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call U32 common_setSegment(CPU* cpu, U32 seg, U32 value)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, seg, 4); // value param, must pass 4 so that upper part of reg is zero'd out
    if (bytes==2)
        zeroReg(0, true); // upper 2 bytes need to be 0
    popReg(0, true, bytes, false); // peek stack for seg param    

    callHost(common_setSegment);
    
    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp();
    }, [this, bytes]() {
        syncRegsToHost();
        U8 tmpReg = getTmpReg();
        adjustStack(bytes==4, tmpReg, (S8)bytes);
        releaseTmpReg(tmpReg);
    });   
}

void X64Asm::setSeg(U8 seg, U8 rm) {
    // in case an exception is thrown, the exception will need to store the regs
    // also this will help will saving ECX and EDX as required by Window ABI
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters    

    // put value into R8, set before ECX and EDX in case rm referes to them
    zeroReg(0, true);
    writeToRegFromE(0, true, rm, 2);

    // call U32 common_setSegment(CPU* cpu, U32 seg, U32 value)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    
    writeToRegFromValue(2, false, seg, 4); // value param, must pass 4 so that upper part of reg is zero'd out    

    callHost(common_setSegment);
    
    doIf(0, false, 0, [this]() {
        syncRegsToHost();
        doJmp();
    }, [this]() {
        syncRegsToHost();
    });   
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
    pushNativeFlags();
    popNativeReg(tmpReg, true);

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

    pushNativeReg(tmpReg, true);
    releaseTmpReg(tmpReg);    

    popNativeFlags();
}

void X64Asm::salc() {
    pushNativeFlags(); // save flags since we don't want sbb to affect them
    // sbb al, al
    write8(0x18);
    write8(0xc0);
    popNativeFlags();
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
    writeToMemFromReg(0, false, HOST_ESP, true, HOST_SS, true, 0, -2, 2, true);
    writeToMemFromReg(1, false, HOST_ESP, true, HOST_SS, true, 0, -4, 2, true);
    writeToMemFromReg(2, false, HOST_ESP, true, HOST_SS, true, 0, -6, 2, true);
    writeToMemFromReg(3, false, HOST_ESP, true, HOST_SS, true, 0, -8, 2, true);
    writeToMemFromReg(HOST_ESP, true, HOST_ESP, true, HOST_SS, true, 0, -10, 2, true);
    writeToMemFromReg(5, false, HOST_ESP, true, HOST_SS, true, 0, -12, 2, true);
    writeToMemFromReg(6, false, HOST_ESP, true, HOST_SS, true, 0, -14, 2, true);
    writeToMemFromReg(7, false, HOST_ESP, true, HOST_SS, true, 0, -16, 2, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -16, 2);
}

// POPA
void X64Asm::popA16() {
    writeToRegFromMem(7, false, HOST_ESP, true, HOST_SS, true, 0, 0, 2, true);
    writeToRegFromMem(6, false, HOST_ESP, true, HOST_SS, true, 0, 2, 2, true);
    writeToRegFromMem(5, false, HOST_ESP, true, HOST_SS, true, 0, 4, 2, true);
    // SP isn't pop, but the stack will be adjusted
    // writeToRegFromMem(4, false, HOST_ESP, true, HOST_SS, true, 0, 6, 2, true);
    writeToRegFromMem(3, false, HOST_ESP, true, HOST_SS, true, 0, 8, 2, true);
    writeToRegFromMem(2, false, HOST_ESP, true, HOST_SS, true, 0, 10, 2, true);
    writeToRegFromMem(1, false, HOST_ESP, true, HOST_SS, true, 0, 12, 2, true);
    writeToRegFromMem(0, false, HOST_ESP, true, HOST_SS, true, 0, 14, 2, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, 16, 2);
}

// PUSHAD
void X64Asm::pushA32() {
    writeToMemFromReg(0, false, HOST_ESP, true, HOST_SS, true, 0, -4, 4, true);
    writeToMemFromReg(1, false, HOST_ESP, true, HOST_SS, true, 0, -8, 4, true);
    writeToMemFromReg(2, false, HOST_ESP, true, HOST_SS, true, 0, -12, 4, true);
    writeToMemFromReg(3, false, HOST_ESP, true, HOST_SS, true, 0, -16, 4, true);
    writeToMemFromReg(HOST_ESP, true, HOST_ESP, true, HOST_SS, true, 0, -20, 4, true);
    writeToMemFromReg(5, false, HOST_ESP, true, HOST_SS, true, 0, -24, 4, true);
    writeToMemFromReg(6, false, HOST_ESP, true, HOST_SS, true, 0, -28, 4, true);
    writeToMemFromReg(7, false, HOST_ESP, true, HOST_SS, true, 0, -32, 4, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, -32, 4);
}

// POPAD
void X64Asm::popA32() {
    writeToRegFromMem(7, false, HOST_ESP, true, HOST_SS, true, 0, 0, 4, true);
    writeToRegFromMem(6, false, HOST_ESP, true, HOST_SS, true, 0, 4, 4, true);
    writeToRegFromMem(5, false, HOST_ESP, true, HOST_SS, true, 0, 8, 4, true);
    // SP isn't pop, but the stack will be adjusted
    // writeToRegFromMem(4, false, HOST_ESP, true, HOST_SS, true, 0, 12, 4, true);
    writeToRegFromMem(3, false, HOST_ESP, true, HOST_SS, true, 0, 16, 4, true);
    writeToRegFromMem(2, false, HOST_ESP, true, HOST_SS, true, 0, 20, 4, true);
    writeToRegFromMem(1, false, HOST_ESP, true, HOST_SS, true, 0, 24, 4, true);
    writeToRegFromMem(0, false, HOST_ESP, true, HOST_SS, true, 0, 28, 4, true);

    addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, 32, 4);
}

void X64Asm::jumpConditional(U8 condition, U32 eip) {    
    write8(0x0F);
    write8(0x80+condition);
    write32(0);
    addTodoLinkJump(eip);
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

void X64Asm::addTodoLinkJump(U32 eip) {
    this->todoJump.push_back(TodoJump(eip, this->bufferPos-4, 4));
}

#define JUMP_TO_SIZE 5

void X64Asm::jumpTo(U32 eip) {    
    write8(0xE9);
    write32(0);
    addTodoLinkJump(eip);
}

void X64Asm::doLoop(U32 eip) {
    // :TODO: maybe find one byte offset
    write8(JUMP_TO_SIZE); // skip over the next jmp
    jumpTo(this->ip); // jump to next instruction after this because condition was not met
    jumpTo(eip); // jmp to offset because condition was met
}

void X64Asm::jcxz(U32 eip, bool ea16) {
    if (ea16)
        write8(0x67);
    write8(0xe3);    
    doLoop(eip);
}

void X64Asm::loop(U32 eip, bool ea16) {
    if (ea16)
        write8(0x67);
    write8(0xe2);    
    doLoop(eip);
}

void X64Asm::loopz(U32 eip, bool ea16) {
    if (ea16)
        write8(0x67);
    write8(0xe1);    
    doLoop(eip);
}

void X64Asm::loopnz(U32 eip, bool ea16) {
    if (ea16)
        write8(0x67);
    write8(0xe0);    
    doLoop(eip);
}

void X64Asm::jmp(bool big, U32 sel, U32 offset, U32 oldEip) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_jmp(cpu, false, sel, offset, oldEip)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, big, 8); // big param
    writeToRegFromValue(0, true, sel, 8); // sel param
    writeToRegFromValue(1, true, offset, 8); // offset param
    pushNativeValue32(oldEip); // oldEip param
    
    callHost(common_jmp);
    popNativeReg(0, false); // balances pushing oldEip, we don't care about the reg
    syncRegsToHost();
    doJmp();
}

// common_jmp(cpu, false, sel, offset, oldEip);
void X64Asm::call(bool big, U32 sel, U32 offset, U32 oldEip) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_call(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, big, 8); // big param
    writeToRegFromValue(0, true, sel, 8); // sel param
    writeToRegFromValue(1, true, offset, 8); // offset param
    pushNativeValue32(oldEip); // oldEip param
    
    callHost(common_call);
    popNativeReg(0, false); // balances pushing oldEip, we don't care about the reg
    syncRegsToHost();
    doJmp();
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

// don't use x64_getTmpReg here, it is import that the exact reg is used for each instruction since
// the exception handler will look for it
void X64Asm::jmpReg(U8 reg, bool isRex) {    
    if (reg==HOST_TMP2 && isRex) {
        getRegForSeg(CS, HOST_TMP);
        addWithLea(reg, true, reg, true, HOST_TMP, true, 0, 0, 4);

        // mov HOST_TMP, HOST_TMP2
        writeToRegFromReg(HOST_TMP, true, reg, isRex, false);
    } else if (reg==HOST_TMP && isRex) {
        getRegForSeg(CS, HOST_TMP2);
        addWithLea(HOST_TMP, true, HOST_TMP, true, HOST_TMP2, true, 0, 0, 4);

        // mov HOST_TMP2, HOST_TMP
        writeToRegFromReg(HOST_TMP2, true, HOST_TMP, true, 4);
    } else {        
        // mov HOST_TMP, reg
        writeToRegFromReg(HOST_TMP, true, reg, isRex, false);

        getRegForSeg(CS, HOST_TMP2);
        addWithLea(HOST_TMP, true, HOST_TMP, true, HOST_TMP2, true, 0, 0, 4);
        

        // mov HOST_TMP2, HOST_TMP
        writeToRegFromReg(HOST_TMP2, true, HOST_TMP, true, 4);
    }

    pushNativeFlags();

    // shr HOST_TMP2, 12
    shiftRightReg(HOST_TMP2, true, K_PAGE_SHIFT);    

    // and HOST_TMP, 0xFFF
    andReg(HOST_TMP, true, K_PAGE_MASK);

    // :TODO: maybe use HOST_TMP3 instead of RAX
    // push rax
    pushNativeReg(0, false);        
    
    // rax=cpu->opToAddressPages
    // mov rax, [HOST_CPU];
    writeToRegFromMem(0, false, HOST_CPU, true, -1, false, 0, CPU_OFFSET_OP_PAGES, 8, false);
    
    // rax = cpu->opToAddressPages[page]
    // mov RAX, [RAX+HOST_TEMP2<<3] // 3 sizeof(void*)
    writeToRegFromMem(0, false, 0, false, HOST_TMP2, true, 3, 0, 8, false); 

    // will move address to RAX and test that it exists, if it doesn't then we
    // will catch the exception.  We leave the address/index we need in HOST_TMP
    // and HOST_TMP2

    // mov RAX, [RAX + HOST_TMP << 3]
    writeToRegFromMem(0, false, 0, false, HOST_TMP, true, 3, 0, 8, false); 

    // This will test that the value we are about to jump to exists
    // mov HOST_TMP, [RAX]
    writeToRegFromMem(HOST_TMP, true, 0, false, -1, false, 0, 0, 2, false);

    // mov HOST_TMP, RAX
    writeToRegFromReg(HOST_TMP, true, 0, false, 8);

    // pop rax
    popNativeReg(0, false);    

    popNativeFlags();

    // jmp HOST_TMP
    jmpNativeReg(HOST_TMP, true);
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
    jmpReg(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::retn32(U32 bytes) {
    U32 tmpReg = getTmpReg();
    popReg32(tmpReg, true);
    if (bytes) {
        addWithLea(HOST_ESP, true, HOST_ESP, true, -1, false, 0, bytes, 4);
    }
    jmpReg(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::retf(U32 big, U32 bytes) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_ret(CPU* cpu, U32 big, U32 bytes)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, big, 8); // big param
    writeToRegFromValue(0, true, bytes, 8); // bytes param
    
    callHost(common_ret);
    syncRegsToHost();
    doJmp();
}

void X64Asm::iret(U32 big, U32 oldEip) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_iret(CPU* cpu, U32 big, U32 oldEip)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, big, 8); // big param
    writeToRegFromValue(0, true, oldEip, 8); // sel param
    
    callHost(common_iret);
    syncRegsToHost();
    doJmp();
}

void X64Asm::signalIllegalInstruction(int code) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // void common_signalIllegalInstruction(CPU* cpu, int code)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, code, 8); // code param
    
    callHost(common_signalIllegalInstruction);
    syncRegsToHost();
    doJmp();
}

static U8 fetchByte(U32 *eip) {
    return readb((*eip)++);
}

static void x64log(CPU* cpu, U32 eip) {
    if (!cpu->logFile)
        return;
    static DecodedBlock* block;
    if (!block) {
        block = new DecodedBlock();
    }
    decodeBlock(fetchByte, eip+cpu->seg[CS].address, cpu->big, 1, K_PAGE_SIZE, 0, block);
    cpu->eip.u32 = eip;
    block->op->log(cpu);
    block->op->dealloc(false);
}

void X64Asm::logOp(U32 eip) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param    
    writeToRegFromValue(2, false, eip, 4);
    callHost(x64log);

    syncRegsToHost();
}

void X64Asm::syscall(U32 opLen, U32 eip) {
    syncRegsFromHost(); 
    writeToMemFromValue(eip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // void ksyscall(cpu, op->len)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, opLen, 8); // opLen param
    
    callHost(ksyscall);
    syncRegsToHost();
    doJmp();
}

void X64Asm::int98(U32 opLen) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // void common_int98(CPU* cpu)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, opLen, 8); // opLen param

    callHost(common_int98);
    syncRegsToHost();
    //doJmp();
}

void X64Asm::int99(U32 opLen) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // void common_int99(CPU* cpu)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    writeToRegFromValue(2, false, opLen, 8); // opLen param

    callHost(common_int99);
    syncRegsToHost();
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

// :TODO: if an exception happens during a string, esi and edi should be adjusted before passing the exception to Linux/Wine
void X64Asm::string(bool hasSi, bool hasDi, bool ea16) {
    U8 tmpReg = getTmpReg();

    if (hasDi) {
        if (ea16) {            
            pushNativeReg(7, false);
            pushNativeFlags();
            andReg(7, false, 0x0000ffff);
            popNativeFlags();
        }
        addWithLea(7, false, 7, false, getRegForSeg(ES, tmpReg), true, 0, 0, 4);
    }
    if (hasSi) {
        if (ea16) {            
            pushNativeReg(6, false);
            pushNativeFlags();
            andReg(6, false, 0x0000ffff);
            popNativeFlags();
        }
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
    U8 tmpReg2 = getTmpReg();
    writeToRegFromMem(tmpReg, true, HOST_CPU, true, -1, false, 0, CPU_OFFSET_NEG_MEM, 8, false);
    if (hasSi) {
        addWithLea(6, false, 6, false, tmpReg, true, 0, 0, 8);    
        addWithLea(6, false, 6, false, getRegForNegSeg(this->ds, tmpReg2), true, 0, 0, 4);
        if (ea16) {
            popNativeReg(tmpReg2, true);
            pushNativeFlags();
            andReg(tmpReg2, true, 0xFFFF0000);
            andReg(6, false, 0x0000FFFF);
            orRegReg(6, false, tmpReg2, true);
            popNativeFlags();
        }
    }
    if (hasDi) {
        addWithLea(7, false, 7, false, tmpReg, true, 0, 0, 8);    
        addWithLea(7, false, 7, false, getRegForNegSeg(ES, tmpReg2), true, 0, 0, 4);
        if (ea16) {
            popNativeReg(tmpReg2, true);
            pushNativeFlags();
            andReg(tmpReg2, true, 0xFFFF0000);
            andReg(7, false, 0x0000FFFF);
            orRegReg(7, false, tmpReg2, true);
            popNativeFlags();
        }
    }    
        
    releaseTmpReg(tmpReg);
    releaseTmpReg(tmpReg2);
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

        getNativeAddressInRegFromE(HOST_TMP3, true, rm);
        this->tmp3InUse = true;

        // read selector and put it into R8 for function call
        this->tmp2InUse = true;
        zeroReg(0, true);
        writeToRegFromMem(0, true, HOST_TMP3, true, -1, false, 0, b32?4:2, 2, false);

        // read value now in case it triggers an exception
        this->tmp1InUse = true;
        zeroReg(1, true);
        writeToRegFromMem(1, true, HOST_TMP3, true, -1, false, 0, 0, b32?4:2, false);

        this->tmp3InUse = false;

        // calling convention RCX, RDX, R8, R9 for first 4 parameters    

        // call U32 common_setSegment(CPU* cpu, U32 seg, U32 value)
        writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param

        writeToRegFromValue(2, false, seg, 4); // value param, must pass 4 so that upper part of reg is zero'd out


        pushNativeReg(1, true); // save value so we don't read it again        

        callHost(common_setSegment);
    
        this->tmp1InUse = false;
        this->tmp2InUse = false;

        doIf(0, false, 0, [this]() {
            syncRegsToHost();
            popNativeReg(1, true); // don't need, but will balance previous push
            doJmp();
        }, [this, rm]() {
            syncRegsToHost();
            // put the value we stored on the native stack into the emulator reg
            U8 r = G(rm);
            if (r==4) {
                popNativeReg(HOST_ESP, true);
            } else {
                popNativeReg(r, false);
            }
        }); 
    } else {
        kpanic("Invalid op: loadSeg rm=%x", rm);
    }
}

void X64Asm::enter(bool big, U32 bytes, U32 level) {
    if (level!=0) {
        // in case an exception is thrown, the exception will need to store the regs
        // also this will help will saving ECX and EDX as required by Window ABI
        syncRegsFromHost(); 

        // calling convention RCX, RDX, R8, R9 for first 4 parameters    

        // call void common_enter(CPU* cpu, U32 big, U32 bytes, U32 level)
        writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param

        writeToRegFromValue(2, false, (big?1:0), 4);
        writeToRegFromValue(0, true, bytes, 4);
        writeToRegFromValue(1, true, level, 4);

        callHost(common_enter);
    
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
            adjustStack(big, tmpReg, -((S32)bytes));
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
    push(-1, false, this->ip, (big?4:2)); 
    jmpReg(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::callJmp(bool big, U8 rm, void* pfn) {
    syncRegsFromHost(); 
     
    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_call(CPU* cpu, U32 big, U32 selector, U32 offset, U32 oldEip)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
    if (big) {
        writeToRegFromValue(2, false, 1, 4); // big param
    } else {
        zeroReg(2, false);
    }    
    U8 bytes = (big?4:2);

    if (this->tmp3InUse) {
        kpanic("X64Asm::callFar tmp3InUse was in use");
    }
    this->tmp3InUse = true;
    if (HOST_TMP3==0 || HOST_TMP3==1) {
        kpanic("X64Asm::callFar incorrectly assumed HOST_TMP3");
    }
    getNativeAddressInRegFromE(HOST_TMP3, true, rm);
    zeroReg(0, true);
    zeroReg(1, true);
    writeToRegFromMem(1, true, HOST_TMP3, true, -1, false, 0, 0, bytes, false); // offset
    writeToRegFromMem(0, true, HOST_TMP3, true, -1, false, 0, bytes, 2, false); // seg
    releaseTmpReg(HOST_TMP3);

    pushNativeValue32(this->ip); // oldEip param
    
    callHost(pfn);
    popNativeReg(0, false); // balances pushing oldEip, we don't care about the reg
    syncRegsToHost();
    doJmp();  
}

void X64Asm::callFar(bool big, U8 rm) {    
    callJmp(big, rm, common_call);   
}

void X64Asm::jmpE(bool big, U8 rm) {
    U8 tmpReg = getTmpReg();
    writeToRegFromE(tmpReg, true, rm, (big?4:2));
    jmpReg(tmpReg, true);
    releaseTmpReg(tmpReg);
}

void X64Asm::jmpFar(bool big, U8 rm) {
    callJmp(big, rm, common_jmp);       
}

void X64Asm::lsl(bool big, U8 rm) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call U32 common_lsl(CPU* cpu, U32 selector, U32 limit)
    if (!big)
        zeroReg(0, true);
    writeToRegFromReg(0, true, G(rm), false, (big?4:2)); // load reg first in case it is ECX or EDX which will be overwritten

    writeToRegFromE(2, false, rm, 2); // load before overwriting ECX
    andReg(2, false, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references edx

    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
            
    callHost(common_lsl);
    if (big) {
        if (G(rm)!=0) {
            writeToRegFromReg(G(rm), false, 0, false, 4);
        }
    } else {
        // fill in the previous upper 2 bytes
        writeToRegFromMem(G(rm), false, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, reg[G(rm)].u32)), 4, false);
        writeToRegFromReg(G(rm), false, 0, false, 2);
    }
    syncRegsToHost(G(rm));
}

void X64Asm::lar(bool big, U8 rm) {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call U32 common_lar(CPU* cpu, U32 selector, U32 limit)
    if (!big)
        zeroReg(0, true);
    writeToRegFromReg(0, true, G(rm), false, (big?4:2)); // load reg first in case it is ECX or EDX which will be overwritten

    writeToRegFromE(2, false, rm, 2); // load before overwriting ECX
    andReg(2, false, 0x0000ffff); // zero out top 2 bytes, can't do this before writeToRegFromE in case rm references edx

    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
            
    callHost(common_lar);
    if (big) {
        if (G(rm)!=0) {
            writeToRegFromReg(G(rm), false, 0, false, 4);
        }
    } else {
        // fill in the previous upper 2 bytes
        writeToRegFromMem(G(rm), false, HOST_CPU, true, -1, false, 0, (U32)(offsetof(CPU, reg[G(rm)].u32)), 4, false);
        writeToRegFromReg(G(rm), false, 0, false, 2);
    }
    syncRegsToHost(G(rm));
}

static void x64_invalidOp(U32 op) {
}

void X64Asm::invalidOp(U32 op) {
    writeToRegFromValue(1, false, op, 4);
    callHost(x64_invalidOp);
}

void X64Asm::cpuid() {
    syncRegsFromHost(); 

    // calling convention RCX, RDX, R8, R9 for first 4 parameters

    // call void common_cpuid(CPU* cpu)
    writeToRegFromReg(1, false, HOST_CPU, true, 8); // CPU* param
            
    callHost(common_cpuid);
    syncRegsToHost(G(rm));
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
    U8 tmpReg = HOST_TMP3;// hack, we should guarantee this isn't used in rm, sib;

    if (this->rex && this->has_rm && isG8bit && G(this->rm)>=4) {        
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
    if (this->cpu->big && this->operandPrefix)
        write8(0x66);
    else if (!this->cpu->big && !this->operandPrefix)
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
    popNativeFlags();
    write8(0xc3); // retn
}
#endif
#endif