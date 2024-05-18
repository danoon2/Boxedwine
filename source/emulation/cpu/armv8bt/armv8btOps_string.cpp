#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
// this code is only useful for verifying that the native armv8 string code works the same as the emulated normal string code
#ifdef BOXEDWINE_ARMV8BT_EMULATE_STRINGS
#include "../normal/normal_strings.h"

void opCmpsb(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU); 
    data->loadConst(1, data->currentOp->repZero);
    data->loadConst(2, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)cmpsb16r);
        } else {
            data->callHost((void*)cmpsb16);
        }
    } else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)cmpsb32r);
        } else {
            data->callHost((void*)cmpsb32);
        }
    }      
    data->callHost((void*)common_fillFlags);
    data->syncRegsToHost();    
}

void opCmpsw(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->repZero);
    data->loadConst(2, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)cmpsw16r);
        }
        else {
            data->callHost((void*)cmpsw16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)cmpsw32r);
        }
        else {
            data->callHost((void*)cmpsw32);
        }
    }
    data->callHost((void*)common_fillFlags);
    data->syncRegsToHost();
}

void opCmpsd(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->repZero);
    data->loadConst(2, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)cmpsd16r);
        }
        else {
            data->callHost((void*)cmpsd16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)cmpsd32r);
        }
        else {
            data->callHost((void*)cmpsd32);
        }
    }
    data->callHost((void*)common_fillFlags);
    data->syncRegsToHost();
}

void opMovsb(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)movsb16r);
        }
        else {
            data->callHost((void*)movsb16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)movsb32r);
        }
        else {
            data->callHost((void*)movsb32);
        }
    }
    data->syncRegsToHost();
}
void opMovsw(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)movsw16r);
        }
        else {
            data->callHost((void*)movsw16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)movsw32r);
        }
        else {
            data->callHost((void*)movsw32);
        }
    }
    data->syncRegsToHost();
}
void opMovsd(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)movsd16r);
        }
        else {
            data->callHost((void*)movsd16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)movsd32r);
        }
        else {
            data->callHost((void*)movsd32);
        }
    }
    data->syncRegsToHost();
}


void opStosb(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)stosb16r);
        }
        else {
            data->callHost((void*)stosb16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)stosb32r);
        }
        else {
            data->callHost((void*)stosb32);
        }
    }
    data->syncRegsToHost();
}
void opStosw(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)stosw16r);
        }
        else {
            data->callHost((void*)stosw16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)stosw32r);
        }
        else {
            data->callHost((void*)stosw32);
        }
    }
    data->syncRegsToHost();
}
void opStosd(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)stosd16r);
        }
        else {
            data->callHost((void*)stosd16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)stosd32r);
        }
        else {
            data->callHost((void*)stosd32);
        }
    }
    data->syncRegsToHost();
}

void opLodsb(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)lodsb16r);
        }
        else {
            data->callHost((void*)lodsb16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)lodsb32r);
        }
        else {
            data->callHost((void*)lodsb32);
        }
    }
    data->syncRegsToHost();
}
void opLodsw(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)lodsw16r);
        }
        else {
            data->callHost((void*)lodsw16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)lodsw32r);
        }
        else {
            data->callHost((void*)lodsw32);
        }
    }
    data->syncRegsToHost();
}
void opLodsd(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->base);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)lodsd16r);
        }
        else {
            data->callHost((void*)lodsd16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)lodsd32r);
        }
        else {
            data->callHost((void*)lodsd32);
        }
    }
    data->syncRegsToHost();
}

void opScasb(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->repZero);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)scasb16r);
        }
        else {
            data->callHost((void*)scasb16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)scasb32r);
        }
        else {
            data->callHost((void*)scasb32);
        }
    }
    data->callHost((void*)common_fillFlags);
    data->syncRegsToHost();
}

void opScasw(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->repZero);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)scasw16r);
        }
        else {
            data->callHost((void*)scasw16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)scasw32r);
        }
        else {
            data->callHost((void*)scasw32);
        }
    }
    data->callHost((void*)common_fillFlags);
    data->syncRegsToHost();
}

void opScasd(Armv8btAsm* data) {
    data->syncRegsFromHost();
    data->mov64(0, xCPU);
    data->loadConst(1, data->currentOp->repZero);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)scasd16r);
        }
        else {
            data->callHost((void*)scasd16);
        }
    }
    else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            data->callHost((void*)scasd32r);
        }
        else {
            data->callHost((void*)scasd32);
        }
    }
    data->callHost((void*)common_fillFlags);
    data->syncRegsToHost();
}
#else
void cmps(Armv8btAsm* data, U32 width, Arm8BtFlags* lazyFlags) {
    U32 cxZeroSkipPos = 0xffffffff;
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U32 count = CX;
            // if (count) {
            //     U8 v1 = 0;
            //     U8 v2 = 0;
            //     for (U32 i = 0; i < count; i++) {
            //         v1 = readb(dBase + DI);
            //         v2 = readb(sBase + SI);
            //         DI += inc;
            //         SI += inc;
            //         CX--;
            //         if ((v1 == v2) != rep_zero) break;
            //     }
            //     cpu->dst.u8 = v2;
            //     cpu->src.u8 = v1;
            //     cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
            //     cpu->lazyFlags = FLAGS_SUB8;

            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 incReg = data->getTmpReg();            
            U8 tmpReg = data->getTmpReg();

            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            cxZeroSkipPos = data->branchEQ();

            // S32 inc = cpu->df
            data->getDF(incReg, width);

            U32 loopPos = data->bufferPos;

            // if (count == 0) break;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            U32 skipPos = data->branchEQ();            
            
            U8 diReg = data->getTmpReg();
            U8 siReg = data->getTmpReg();            

            // v1 = readb(dBase + DI);
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(tmpReg, diReg, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // v2 = readb(sBase + SI);
            data->movRegToReg(siReg, xESI, 16, true);
            data->addRegs32(tmpReg, siReg, sBaseReg);
            data->readMemory(tmpReg, xDst, width, true);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            // SI += inc;
            data->addRegs32(siReg, siReg, incReg);
            data->movRegToReg(xESI, siReg, 16, false);

            // CX--;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->subValue32(tmpReg, tmpReg, 1);
            data->movRegToReg(xECX, tmpReg, 16, false);

            // if ((v1 == v2) != rep_zero) break;
            data->subRegs32(xResult, xDst, xSrc, 0, true);

            if (data->currentOp->repZero) {
                data->writeJumpAmount(data->branchEQ(), loopPos);
            } else {
                data->writeJumpAmount(data->branchNE(), loopPos);
            }            

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->zeroExtend(xResult, xResult, width);

            data->releaseTmpReg(tmpReg);
            data->releaseTmpReg(incReg);
            data->releaseTmpReg(siReg);
            data->releaseTmpReg(diReg);
        } else {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U8 v1;
            // U8 v2;
            // v1 = readb(dBase + DI);
            // v2 = readb(sBase + SI);
            // DI += inc;
            // SI += inc;
            // cpu->dst.u8 = v2;
            // cpu->src.u8 = v1;
            // cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
            // cpu->lazyFlags = FLAGS_SUB8;
            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 incReg = data->getTmpReg();
            U8 diReg = data->getTmpReg();
            U8 siReg = data->getTmpReg();
            
            // S32 inc = cpu->df            
            data->getDF(incReg, width);
            U8 tmpReg = data->getTmpReg();
            
            // v1 = readb(dBase + DI);
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(tmpReg, diReg, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // v2 = readb(sBase + SI);
            data->movRegToReg(siReg, xESI, 16, true);
            data->addRegs32(tmpReg, siReg, sBaseReg);
            data->readMemory(tmpReg, xDst, width, true);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            // SI += inc;
            data->addRegs32(siReg, siReg, incReg);
            data->movRegToReg(xESI, siReg, 16, false);

            data->subRegs32(xResult, xDst, xSrc, 0, true);
            data->zeroExtend(xResult, xResult, width);

            data->releaseTmpReg(tmpReg);
            data->releaseTmpReg(siReg);
            data->releaseTmpReg(diReg);
            data->releaseTmpReg(incReg);
        }
    } else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U32 count = ECX;
            // if (count) {
            //     U8 v1 = 0;
            //     U8 v2 = 0;
            //     for (U32 i = 0; i < count; i++) {
            //         v1 = readb(dBase + EDI);
            //         v2 = readb(sBase + ESI);
            //         EDI += inc;
            //         ESI += inc;
            //         ECX--;
            //         if ((v1 == v2) != rep_zero) break;
            //     }
            //     cpu->dst.u8 = v2;
            //     cpu->src.u8 = v1;
            //     cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
            //     cpu->lazyFlags = FLAGS_SUB8;
            // }            

            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 incReg = data->getTmpReg();
            U8 tmpReg = data->getTmpReg();

            data->cmpValue32(xECX, 0);
            cxZeroSkipPos = data->branchEQ();

            // S32 inc = cpu->df
            data->getDF(incReg, width);

            U32 loopPos = data->bufferPos;

            // if (count == 0) break;
            data->cmpValue32(xECX, 0);
            U32 skipPos = data->branchEQ();                                                

            // v1 = readb(dBase + EDI);
            data->addRegs32(tmpReg, xEDI, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // v2 = readb(sBase + ESI);
            data->addRegs32(tmpReg, xESI, sBaseReg);
            data->readMemory(tmpReg, xDst, width, true);

            // EDI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            // ESI += inc;
            data->addRegs32(xESI, xESI, incReg);

            // ECX--;
            data->subValue32(xECX, xECX, 1);

            // if ((v1 == v2) != rep_zero) break;
            data->subRegs32(xResult, xDst, xSrc, 0, true);

            if (data->currentOp->repZero) {
                data->writeJumpAmount(data->branchEQ(), loopPos);
            } else {
                data->writeJumpAmount(data->branchNE(), loopPos);
            }                

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->zeroExtend(xResult, xResult, width);

            data->releaseTmpReg(tmpReg);
            data->releaseTmpReg(incReg);
        } else {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U8 v1;
            // U8 v2;
            // v1 = readb(dBase + EDI);
            // v2 = readb(sBase + ESI);
            // EDI += inc;
            // ESI += inc;
            // cpu->dst.u8 = v2;
            // cpu->src.u8 = v1;
            // cpu->result.u8 = cpu->dst.u8 - cpu->src.u8;
            // cpu->lazyFlags = FLAGS_SUB8;

            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 incReg = data->getTmpReg();
            U8 tmpReg = data->getTmpReg();

            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // v1 = readb(dBase + EDI);
            data->addRegs32(tmpReg, xEDI, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // v2 = readb(sBase + ESI);
            data->addRegs32(tmpReg, xESI, sBaseReg);
            data->readMemory(tmpReg, xDst, width, true);

            // EDI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            // ESI += inc;
            data->addRegs32(xESI, xESI, incReg);

            data->subRegs32(xResult, xDst, xSrc, 0, true);
            data->zeroExtend(xResult, xResult, width);
            data->releaseTmpReg(tmpReg);
            data->releaseTmpReg(incReg);
        }
    }
    lazyFlags->setFlags(data, data->flagsNeeded());
    if (data->currentOp->repZero || data->currentOp->repNotZero) {
        data->writeJumpAmount(cxZeroSkipPos, data->bufferPos);
    }
}

void opCmpsb(Armv8btAsm* data) {
    cmps(data, 8, ARM8BT_FLAGS_SUB8);
}

void opCmpsw(Armv8btAsm* data) {
    cmps(data, 16, ARM8BT_FLAGS_SUB16);
}

void opCmpsd(Armv8btAsm* data) {
    cmps(data, 32, ARM8BT_FLAGS_SUB32);
}

void movs(Armv8btAsm* data, U32 width) {
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U32 count = CX;
            // U32 i;
            // for (i = 0; i < count; i++) {
            //     writeb(dBase + DI, readb(sBase + SI));
            //     DI += inc;
            //     SI += inc;
            //     CX--;
            // }
            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 siReg = xSrc;
            U8 diReg = xDst;
            U8 addressReg = data->getTmpReg();
            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);
            U8 tmpReg = data->getTmpReg();
            
            // if (count == 0) break;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            U32 skipPos = data->branchEQ();
            U32 loopPos = data->bufferPos;

            // readb(sBase + SI)
            data->movRegToReg(siReg, xESI, 16, true); // yes, siReg already has the value after the first loop, but we need to mask it to 16 bits and reloading does that
            data->addRegs32(addressReg, siReg, sBaseReg);
            data->readMemory(addressReg, tmpReg, width, true);

            // writeb(dBase + DI
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(addressReg, diReg, dBaseReg);
            data->writeMemory(addressReg, tmpReg, width, true);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            // SI += inc;
            data->addRegs32(siReg, siReg, incReg);
            data->movRegToReg(xESI, siReg, 16, false);

            // CX--;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->subValue32(tmpReg, tmpReg, 1, true);
            data->movRegToReg(xECX, tmpReg, 16, false);

            data->writeJumpAmount(data->branchNE(), loopPos);

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(incReg);
            data->releaseTmpReg(tmpReg);

        } else {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // writeb(dBase + DI, readb(sBase + SI));
            // DI += inc;
            // SI += inc;

            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);            
            U8 siReg = xSrc;
            U8 diReg = xDst;
            U8 addressReg = data->getTmpReg();            
            U8 tmpReg = data->getTmpReg();

            // readb(sBase + SI)
            data->movRegToReg(siReg, xESI, 16, true);
            data->addRegs32(addressReg, siReg, sBaseReg);
            data->readMemory(addressReg, tmpReg, width, true);

            // writeb(dBase + DI
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(addressReg, diReg, dBaseReg);
            data->writeMemory(addressReg, tmpReg, width, true);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(tmpReg);

            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            // SI += inc;
            data->addRegs32(siReg, siReg, incReg);
            data->movRegToReg(xESI, siReg, 16, false);

            data->releaseTmpReg(incReg);
        }
    } else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U32 count = ECX;
            // U32 i;
            // for (i = 0; i < count; i++) {
            //     writeb(dBase + EDI, readb(sBase + ESI));
            //     EDI += inc;
            //     ESI += inc;
            //     ECX--;
            // }
            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 addressReg = data->getTmpReg();
            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);
            U8 tmpReg = data->getTmpReg();

            // if (count == 0) break;
            data->cmpValue32(xECX, 0);
            U32 skipPos = data->branchEQ();
            U32 loopPos = data->bufferPos;

            // readb(sBase + ESI)
            data->addRegs32(addressReg, xESI, sBaseReg);
            data->readMemory(addressReg, tmpReg, width, true);

            // writeb(dBase + EDI
            data->addRegs32(addressReg, xEDI, dBaseReg);
            data->writeMemory(addressReg, tmpReg, width, true);

            // EDI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            // ESI += inc;
            data->addRegs32(xESI, xESI, incReg);

            // ECX--;
            data->subValue32(xECX, xECX, 1, true);

            data->writeJumpAmount(data->branchNE(), loopPos);

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(incReg);
            data->releaseTmpReg(tmpReg);
        } else {
            // U32 dBase = cpu->seg[ES].address;
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // writeb(dBase + EDI, readb(sBase + ESI));
            // EDI += inc;
            // ESI += inc;
            U8 dBaseReg = data->getSegReg(ES);
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 addressReg = data->getTmpReg();
            U8 tmpReg = data->getTmpReg();

            // readb(sBase + SI)
            data->addRegs32(addressReg, xESI, sBaseReg);
            data->readMemory(addressReg, tmpReg, width, true);

            // writeb(dBase + DI
            data->addRegs32(addressReg, xEDI, dBaseReg);
            data->writeMemory(addressReg, tmpReg, width, true);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(tmpReg);

            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // DI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            // SI += inc;
            data->addRegs32(xESI, xESI, incReg);

            data->releaseTmpReg(incReg);
        }
    }
}

void opMovsb(Armv8btAsm* data) {
    movs(data, 8);
}
void opMovsw(Armv8btAsm* data) {
    movs(data, 16);
}
void opMovsd(Armv8btAsm* data) {
    movs(data, 32);
}

void stos(Armv8btAsm* data, U32 width) {
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // S32 inc = cpu->df;
            // U32 count = CX;
            // U32 i;
            // for (i = 0; i < count; i++) {
            //     writeb(dBase + DI, AL);
            //     DI += inc;
            //     CX--;
            // }
            U8 dBaseReg = data->getSegReg(ES);
            U8 diReg = data->getTmpReg();
            U8 addressReg = data->getTmpReg();
            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);
            U8 tmpReg = data->getTmpReg();

            // if (count == 0) break;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            U32 skipPos = data->branchEQ();
            U32 loopPos = data->bufferPos;

            // writeb(dBase + DI, AL)
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(addressReg, diReg, dBaseReg);
            data->writeMemory(addressReg, xEAX, width, true);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            // CX--;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->subValue32(tmpReg, tmpReg, 1, true);
            data->movRegToReg(xECX, tmpReg, 16, false);

            data->writeJumpAmount(data->branchNE(), loopPos);

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->releaseTmpReg(diReg);
            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(incReg);
            data->releaseTmpReg(tmpReg);

        } else {
            // writeb(cpu->seg[ES].address + DI, AL);
            // DI += cpu->df;

            U8 dBaseReg = data->getSegReg(ES);
            U8 diReg = data->getTmpReg();
            U8 addressReg = data->getTmpReg();
            U8 tmpReg = data->getTmpReg();

            // writeb(dBase + DI, AL)
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(addressReg, diReg, dBaseReg);
            data->writeMemory(addressReg, xEAX, width, true);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(tmpReg);

            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            data->releaseTmpReg(diReg);
            data->releaseTmpReg(incReg);
        }
    } else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // S32 inc = cpu->df;
            // U32 count = ECX;
            // U32 i;
            // for (i = 0; i < count; i++) {
            //     writeb(dBase + EDI, AL);
            //     EDI += inc;
            //     ECX--;
            // }
            U8 dBaseReg = data->getSegReg(ES);
            U8 addressReg = data->getTmpReg();
            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);
            U8 tmpReg = data->getTmpReg();

            // if (count == 0) break;
            data->cmpValue32(xECX, 0);
            U32 skipPos = data->branchEQ();
            U32 loopPos = data->bufferPos;

            // writeb(dBase + EDI, AL)
            data->addRegs32(addressReg, xEDI, dBaseReg);
            data->writeMemory(addressReg, xEAX, width, true);

            // EDI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            // ECX--;
            data->subValue32(xECX, xECX, 1, true);

            data->writeJumpAmount(data->branchNE(), loopPos);

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(incReg);
            data->releaseTmpReg(tmpReg);
        } else {
            // writeb(cpu->seg[ES].address + EDI, AL);
            // EDI += cpu->df;

            U8 dBaseReg = data->getSegReg(ES);
            U8 addressReg = data->getTmpReg();

            // writeb(dBase + DI
            data->addRegs32(addressReg, xEDI, dBaseReg);
            data->writeMemory(addressReg, xEAX, width, true);

            data->releaseTmpReg(addressReg);

            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // DI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            data->releaseTmpReg(incReg);
        }
    }
}

void opStosb(Armv8btAsm* data) {
    stos(data, 8);
}
void opStosw(Armv8btAsm* data) {
    stos(data, 16);
}
void opStosd(Armv8btAsm* data) {
    stos(data, 32);
}

static void lods(Armv8btAsm* data, U32 width) {
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U32 count = CX;
            // U32 i;
            // for (i = 0; i < count; i++) {
            //     AL = readb(sBase + SI);
            //     SI += inc;
            //     CX--;
            // }
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 siReg = data->getTmpReg();
            U8 addressReg = data->getTmpReg();
            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);
            U8 tmpReg = data->getTmpReg();

            // if (count == 0) break;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            U32 skipPos = data->branchEQ();
            U32 loopPos = data->bufferPos;

            // readb(sBase + SI)
            data->movRegToReg(siReg, xESI, 16, true); // yes, siReg already has the value after the first loop, but we need to mask it to 16 bits and reloading does that
            data->addRegs32(addressReg, siReg, sBaseReg);
            if (width == 32) {
                data->readMemory(addressReg, xEAX, width, true);
            } else {
                data->readMemory(addressReg, tmpReg, width, true);
                if (width == 8) {
                    data->movRegToReg8(tmpReg, 0);
                } else {
                    data->movRegToReg(xEAX, tmpReg, width, false);
                }
            }

            // SI += inc;
            data->addRegs32(siReg, siReg, incReg);
            data->movRegToReg(xESI, siReg, 16, false);

            // CX--;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->subValue32(tmpReg, tmpReg, 1, true);
            data->movRegToReg(xECX, tmpReg, 16, false);

            data->writeJumpAmount(data->branchNE(), loopPos);

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->releaseTmpReg(siReg);
            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(incReg);
            data->releaseTmpReg(tmpReg);

        } else {
            // AL = readb(cpu->seg[base].address + SI);
            // SI += cpu->df;

            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 siReg = data->getTmpReg();
            U8 addressReg = data->getTmpReg();            

            // readb(sBase + SI)
            data->movRegToReg(siReg, xESI, 16, true);
            data->addRegs32(addressReg, siReg, sBaseReg);
            if (width == 32) {
                data->readMemory(addressReg, xEAX, width, true);
            } else {
                U8 tmpReg = data->getTmpReg();
                data->readMemory(addressReg, tmpReg, width, true);
                if (width == 16) {
                    data->movRegToReg(xEAX, tmpReg, width, false);
                } else {
                    data->movRegToReg8(tmpReg, 0);
                }
                data->releaseTmpReg(tmpReg);
            }

            data->releaseTmpReg(addressReg);            

            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // SI += inc;
            data->addRegs32(siReg, siReg, incReg);
            data->movRegToReg(xESI, siReg, 16, false);

            data->releaseTmpReg(incReg);
            data->releaseTmpReg(siReg);
        }
    } else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 sBase = cpu->seg[base].address;
            // S32 inc = cpu->df;
            // U32 count = ECX;
            // U32 i;
            // for (i = 0; i < count; i++) {
            //     AL = readb(sBase + ESI);
            //     ESI += inc;
            //     ECX--;
            // }

            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 addressReg = data->getTmpReg();
            U8 incReg = data->getTmpReg();

            // S32 inc = cpu->df
            data->getDF(incReg, width);            

            // if (count == 0) break;
            data->cmpValue32(xECX, 0);
            U32 skipPos = data->branchEQ();
            U32 loopPos = data->bufferPos;

            // readb(sBase + ESI)
            data->addRegs32(addressReg, xESI, sBaseReg);
            if (width == 32) {
                data->readMemory(addressReg, xEAX, width, true);
            } else {
                U8 tmpReg = data->getTmpReg();
                data->readMemory(addressReg, tmpReg, width, true);
                if (width == 8) {
                    data->movRegToReg8(tmpReg, 0);
                } else {
                    data->movRegToReg(xEAX, tmpReg, width, false);
                }
                data->releaseTmpReg(tmpReg);
            }

            // ESI += inc;
            data->addRegs32(xESI, xESI, incReg);

            // ECX--;
            data->subValue32(xECX, xECX, 1, true);

            data->writeJumpAmount(data->branchNE(), loopPos);

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->releaseTmpReg(addressReg);
            data->releaseTmpReg(incReg);            
        } else {
            // AL = readb(cpu->seg[base].address + ESI);
            // ESI += cpu->df;
            U8 sBaseReg = data->getSegReg(data->currentOp->base);
            U8 addressReg = data->getTmpReg();            

            // readb(sBase + SI)
            data->addRegs32(addressReg, xESI, sBaseReg);
            if (width == 32) {
                data->readMemory(addressReg, xEAX, width, true);
            } else {
                U8 tmpReg = data->getTmpReg();
                data->readMemory(addressReg, tmpReg, width, true);
                if (width == 8) {
                    data->movRegToReg8(tmpReg, 0);
                } else {
                    data->movRegToReg(xEAX, tmpReg, width, false);
                }
                data->releaseTmpReg(tmpReg);
            }

            data->releaseTmpReg(addressReg);            

            U8 incReg = data->getTmpReg();
            // S32 inc = cpu->df
            data->getDF(incReg, width);

            // SI += inc;
            data->addRegs32(xESI, xESI, incReg);

            data->releaseTmpReg(incReg);
        }
    }
}

void opLodsb(Armv8btAsm* data) {
    lods(data, 8);
}
void opLodsw(Armv8btAsm* data) {
    lods(data, 16);
}
void opLodsd(Armv8btAsm* data) {
    lods(data, 32);
}

void scas(Armv8btAsm* data, U32 width, Arm8BtFlags* lazyFlags) {
    U8 siReg;
    
    U32 flags = data->flagsNeeded();
    
    if (width == 8) {
        data->movReg8ToReg(0, xDst, false);
        siReg = xDst;
    } else if (width == 16) {
        data->movRegToReg(xDst, xEAX, width, true);
        siReg = xDst;
    } else if (width == 32) {
        if (lazyFlags->usesDst(flags)) {
            data->movRegToReg(xDst, xEAX, 32, false);
        }
        siReg = xEAX;
    }
    U32 cxZeroSkipPos = 0xffffffff;

    U8 incReg = data->getTmpReg();
    data->getDF(incReg, width);
    if (data->currentOp->ea16) {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // S32 inc = cpu->df;
            // U32 count = CX;
            // if (count) {
            //     U8 v1 = 0;
            //     for (U32 i = 0; i < count; i++) {
            //         v1 = readb(dBase + DI);
            //         DI += inc;
            //         CX--;
            //         if ((AL == v1) != rep_zero) break;
            //     }
            //     cpu->dst.u8 = AL;
            //     cpu->src.u8 = v1;
            //     cpu->result.u8 = AL - v1;
            //     cpu->lazyFlags = FLAGS_SUB8;
            // }

            U8 dBaseReg = data->getSegReg(ES);            
            U8 tmpReg = data->getTmpReg();            

            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            cxZeroSkipPos = data->branchEQ();

            U32 loopPos = data->bufferPos;

            // if (count == 0) break;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->cmpValue32(tmpReg, 0);
            U32 skipPos = data->branchEQ();

            U8 diReg = data->getTmpReg();            

            // v1 = readb(dBase + DI);
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(tmpReg, diReg, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            // CX--;
            data->movRegToReg(tmpReg, xECX, 16, true);
            data->subValue32(tmpReg, tmpReg, 1);
            data->movRegToReg(xECX, tmpReg, 16, false);

            // if ((AL == v1) != rep_zero) break;
            data->subRegs32(xResult, siReg, xSrc, 0, true);

            if (data->currentOp->repZero) {
                data->writeJumpAmount(data->branchEQ(), loopPos);
            } else {
                data->writeJumpAmount(data->branchNE(), loopPos);
            }

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->zeroExtend(xResult, xResult, width);            

            data->releaseTmpReg(tmpReg);            
            data->releaseTmpReg(diReg);
        } else {
            // U32 dBase = cpu->seg[ES].address;
            // S32 inc = cpu->df;
            // U8 v1 = readb(dBase + DI);
            // DI += inc;
            // cpu->dst.u8 = AL;
            // cpu->src.u8 = v1;
            // cpu->result.u8 = AL - v1;
            // cpu->lazyFlags = FLAGS_SUB8;

            U8 dBaseReg = data->getSegReg(ES);
            U8 diReg = data->getTmpReg();            
            U8 tmpReg = data->getTmpReg();

            // v1 = readb(dBase + DI);
            data->movRegToReg(diReg, xEDI, 16, true);
            data->addRegs32(tmpReg, diReg, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // DI += inc;
            data->addRegs32(diReg, diReg, incReg);
            data->movRegToReg(xEDI, diReg, 16, false);

            data->subRegs32(xResult, siReg, xSrc, 0, true);
            data->zeroExtend(xResult, xResult, width);
            data->releaseTmpReg(tmpReg);
            data->releaseTmpReg(diReg);
        }
    } else {
        if (data->currentOp->repZero || data->currentOp->repNotZero) {
            // U32 dBase = cpu->seg[ES].address;
            // S32 inc = cpu->df;
            // U32 count = ECX;
            // if (count) {
            //     U8 v1 = 0;
            //     for (U32 i = 0; i < count; i++) {
            //         v1 = readb(dBase + EDI);
            //         EDI += inc;
            //         ECX--;
            //         if ((AL == v1) != rep_zero) break;
            //     }
            //     cpu->dst.u8 = AL;
            //     cpu->src.u8 = v1;
            //     cpu->result.u8 = AL - v1;
            //     cpu->lazyFlags = FLAGS_SUB8;
            // }

            U8 dBaseReg = data->getSegReg(ES);
            U8 tmpReg = data->getTmpReg();

            data->cmpValue32(xECX, 0);
            cxZeroSkipPos = data->branchEQ();

            U32 loopPos = data->bufferPos;

            // if (count == 0) break;
            data->cmpValue32(xECX, 0);
            U32 skipPos = data->branchEQ();

            // v1 = readb(dBase + EDI);
            data->addRegs32(tmpReg, xEDI, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // EDI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            // ECX--;
            data->subValue32(xECX, xECX, 1);

            // if ((AL == v1) != rep_zero) break;
            data->subRegs32(xResult, siReg, xSrc, 0, true);

            if (data->currentOp->repZero) {
                data->writeJumpAmount(data->branchEQ(), loopPos);
            } else {
                data->writeJumpAmount(data->branchNE(), loopPos);
            }

            data->writeJumpAmount(skipPos, data->bufferPos);

            data->zeroExtend(xResult, xResult, width);
            data->releaseTmpReg(tmpReg);
        } else {
            // U32 dBase = cpu->seg[ES].address;
            // S32 inc = cpu->df;
            // U8 v1 = readb(dBase + EDI);
            // EDI += inc;
            // cpu->dst.u8 = AL;
            // cpu->src.u8 = v1;
            // cpu->result.u8 = AL - v1;
            // cpu->lazyFlags = FLAGS_SUB8;

            U8 dBaseReg = data->getSegReg(ES);
            U8 tmpReg = data->getTmpReg();

            // v1 = readb(dBase + EDI);
            data->addRegs32(tmpReg, xEDI, dBaseReg);
            data->readMemory(tmpReg, xSrc, width, true);

            // EDI += inc;
            data->addRegs32(xEDI, xEDI, incReg);

            data->subRegs32(xResult, siReg, xSrc, 0, true);
            data->zeroExtend(xResult, xResult, width);
            data->releaseTmpReg(tmpReg);
        }
    }
    lazyFlags->setFlags(data, flags);
    data->releaseTmpReg(incReg);

    if (data->currentOp->repZero || data->currentOp->repNotZero) {
        data->writeJumpAmount(cxZeroSkipPos, data->bufferPos);
    }
}

void opScasb(Armv8btAsm* data) {
    scas(data, 8, ARM8BT_FLAGS_SUB8);
}

void opScasw(Armv8btAsm* data) {
    scas(data, 16, ARM8BT_FLAGS_SUB16);
}

void opScasd(Armv8btAsm* data) {
    scas(data, 32, ARM8BT_FLAGS_SUB32);
}
#endif
#endif