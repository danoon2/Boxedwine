#include "boxedwine.h"

#ifdef BOXEDWINE_ARMV8BT

#include "armv8btAsm.h"
#include "armv8btOps.h"

#include "../common/common_other.h"
#include "../../../../source/emulation/softmmu/kmemory_soft.h"
#include "../normal/normalCPU.h"
#include "../armv8/llvm_helper.h"
#include "armv8btCodeChunk.h"
#include "armv8btCPU.h"

static bool isWidthVector(VectorWidth width) {
    if (width == D_scaler || width == S_scaler) {
        return false;
    }
    return true;
}

U32 Armv8btAsm::flagsNeeded() {
    return DecodedOp::getNeededFlags(this->currentBlock, this->currentOp, instructionInfo[this->currentOp->inst].flagsSets & ~MAYBE);
}

void Armv8btAsm::pushPair(U8 r1, U8 r2) {
    // stp r1, r2, [sp, #-16]!
    write8(0xe0 | r1);
    write8(0x03 | (U8)(r2 << 2));
    write8(0xbf);
    write8(0xa9);
}

void Armv8btAsm::popPair(U8 r1, U8 r2) {
    // ldp r1, r2, [sp], #16
    write8(0xe0 | r1);
    write8(0x03 | (U8)(r2 << 2));
    write8(0xc1);
    write8(0xa8);
}

U8 Armv8btAsm::getFpuOffset() {
    if (!this->fpuOffsetRegSet) {
        this->fpuOffsetRegSet = true;
        addValue64(xFpuOffset, xCPU, (U32)(offsetof(CPU, fpu)));
    }
    return xFpuOffset;
}

U8 Armv8btAsm::getFpuTagOffset() {
    U8 offsetReg = getTmpReg();
    addValue64(offsetReg, getFpuOffset(), (U32)(offsetof(FPU, tags)));
    return offsetReg;
}

void Armv8btAsm::releaseFpuTagOffset(U8 offsetReg) {
    releaseTmpReg(offsetReg);
}

U8 Armv8btAsm::getFpuTopReg() {
    // if we get the cached top before a read/write and that read/write calls the host, then the instruction is not re-entrant
    if (1) {
        this->fpuTopRegSet = true;
        this->readMem32ValueOffset(xFpuTop, xCPU, (U32)(offsetof(CPU, fpu.top)));
    }
    return xFpuTop;
}

U8 Armv8btAsm::getSegReg(U8 seg) {
    switch (seg) {
    case ES: return xES;
    case CS: return xCS;
    case SS: return xSS;
    case DS: return xDS;
    case FS: return xFS;
    case GS: return xGS;
    default: kpanic("ArmV8bt: getSegReg: invalid seg: %d", seg); return 0;
    }
}

// shift can be 0, 16, 32 or 48
void Armv8btAsm::movk(U8 reg, U16 value, U8 shift) {
    U8 shiftBit = 0;
    if (shift == 16) {
        shiftBit = 0x20;
    } else if (shift == 32) {
        shiftBit = 0x40;
    } else if (shift == 48) {
        shiftBit = 0x60;
    } else if (shift != 0) {
        kpanic("armv8: bad shift value of % in movk", shift);
    }
    write8((U8)(value << 5) | reg); // bottom 3 bits of the value in the top 3 bits
    write8((U8)(value >> 3)); // bits 4-11
    write8(0x80 | (U8)(value >> 11) | shiftBit); // bits 12-16
    write8(0xf2); // 64-bit
}

// shift can be 0, 16, 32 or 48
void Armv8btAsm::movn(U8 reg, U16 value, U8 shift) {
    U8 shiftBit = 0;
    if (shift == 16) {
        shiftBit = 0x20;
    } else if (shift == 32) {
        shiftBit = 0x40;
    } else if (shift == 48) {
        shiftBit = 0x60;
    } else if (shift != 0) {
        kpanic("armv8: bad shift value of % in movn", shift);
    }
    write8((U8)(value << 5) | reg); // bottom 3 bits of the value in the top 3 bits
    write8((U8)(value >> 3)); // bits 4-11
    write8(0x80 | (U8)(value >> 11) | shiftBit); // bits 12-16
    write8(0x12); // 64-bit
}

// shift can be 0, 16, 32 or 48
void Armv8btAsm::movz(U8 reg, U16 value, U8 shift) {
    U8 shiftBit = 0;
    if (shift == 16) {
        shiftBit = 0x20;
    } else if (shift == 32) {
        shiftBit = 0x40;
    } else if (shift == 48) {
        shiftBit = 0x60;
    } else if (shift != 0) {
        kpanic("armv8: bad shift value of % in movz", shift);
    }
    write8((U8)(value << 5) | reg); // bottom 3 bits of the value in the top 3 bits
    write8((U8)(value >> 3)); // bits 4-11
    write8(0x80 | (U8)(value >> 11) | shiftBit); // bits 12-16
    write8(0xd2); // 64-bit
}

void Armv8btAsm::mov32(U8 dst, U8 src) {
    write8(dst | 0xE0);
    write8(0x3);
    write8(src);
    write8(0x2a);
}

void Armv8btAsm::mov64(U8 dst, U8 src) {
    write8(dst | 0xE0);
    write8(0x3);
    write8(src);
    write8(0xaa);
}

void Armv8btAsm::zeroReg(U8 reg) {
    movz(reg, 0, 0);
}

void Armv8btAsm::reverseBits32(U8 dst, U8 src) {
    // RBIT w0, w0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3));
    write8(0xc0);
    write8(0x5a);
}

void Armv8btAsm::reverseBytes32(U8 dst, U8 src) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x08);
    write8(0xc0);
    write8(0x5a);
}

void Armv8btAsm::loadConst(U8 reg, U64 value) {
    U32 shift = __builtin_ctzll(value);
    if (shift < 16) {
        shift = 0;
    } else if (shift < 32) {
        shift = 16;
    } else if (shift < 48) {
        shift = 32;
    } else {
        shift = 48;
    }
    U64 v = value >> shift;
    if (v <= 0xFFFF) {
        movz(reg, v, shift);
    } else {
        movz(reg, (U16)value);
        if (value > 0xFFFF) {
            U16 v = (U16)(value >> 16);
            if (v) {
                movk(reg, v, 16);
            }
        }
        if (value > 0xFFFFFFFF) {
            U16 v = (U16)(value >> 32);
            if (v) {
                movk(reg, v, 32);
            }
        }
        if (value > 0xFFFFFFFFFFFF) {
            movk(reg, (U16)(value >> 48), 48);
        }
    }
}

U8 Armv8btAsm::getRegWithConst(U64 value) {
    U8 tmp = getTmpReg();
    loadConst(tmp, value);
    return tmp;
}

void Armv8btAsm::readMem8ValueOffset(U8 dst, U8 base, S32 offset, bool signExtend) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        readMem8RegOffset(dst, base, tmp, signExtend);
        releaseTmpReg(tmp);
    } else {
        // ldrb dst, [base, offset]
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8((signExtend ? 0xc0 : 0x40) | ((offset >> 4) & 0x1F));
        write8(0x38);
    }
}

void Armv8btAsm::readMem8Lock(U8 dst, U8 base) {
    write8(dst | (U8)(base << 5));
    write8(0xfc | (U8)(base >> 3));
    write8(0x5f);
    write8(0x08);
}

void Armv8btAsm::readMem8RegOffset(U8 dst, U8 base, U8 offsetReg, bool signExtend) {
    // ldrb dst, [base, offsetReg]
    write8(dst | (U8)(base << 5));
    write8(0x68 | (U8)(base >> 3));
    write8((signExtend ? 0xe0 : 0x60) | offsetReg);
    write8(0x38);
}

void Armv8btAsm::readMem16ValueOffset(U8 dst, U8 base, S32 offset, bool signExtend) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        readMem16RegOffset(dst, base, tmp, signExtend);
        releaseTmpReg(tmp);
    } else {
        // LDRUH
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8((signExtend ? 0xc0 : 0x40) | ((offset >> 4) & 0x1F));
        write8(0x78);
    }
}

void Armv8btAsm::readMem16Lock(U8 dst, U8 base) {
    write8(dst | (U8)(base << 5));
    write8(0xfc | (U8)(base >> 3));
    write8(0x5f);
    write8(0x48);
}

void Armv8btAsm::readMem16RegOffset(U8 dst, U8 base, U8 offsetReg, bool signExtend) {
    write8(dst | (U8)(base << 5));
    write8(0x68 | (U8)(base >> 3));
    write8((signExtend ? 0xe0 : 0x60) | offsetReg);
    write8(0x78);
}

void Armv8btAsm::readMem32ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        loadConst(dst, offset);

        // LDR
        write8(dst | (U8)(base << 5));
        write8(0x68 | (U8)(base >> 3));
        write8(0x60 | dst);
        write8(0xb8);
    } else {
        // LDUR
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8(0x40 | ((offset >> 4) & 0x1F));
        write8(0xb8);
    }
}

void Armv8btAsm::readMem32Lock(U8 dst, U8 base) {
    write8(dst | (U8)(base << 5));
    write8(0xfc | (U8)(base >> 3));
    write8(0x5f);
    write8(0x88);
}

void Armv8btAsm::readMem32RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl) {
    write8(dst | (U8)(base << 5));
    if (lsl == 0) {
        write8(0x68 | (U8)(base >> 3));
    } else if (lsl == 2) {
        write8(0x78 | (U8)(base >> 3));
    } else {
        kpanic("ArmV8bt: readMem32RegOffset lsl must be 0 or 2: %d", lsl);
    }
    write8(0x60 | offsetReg);
    write8(0xb8);
}

void Armv8btAsm::readMem64ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);

        // LDR
        write8(dst | (U8)(base << 5));
        write8(0x68 | (U8)(base >> 3));
        write8(0x60 | tmp);
        write8(0xf8);
        releaseTmpReg(tmp);
    } else {
        // LDUR
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8(0x40 | ((offset >> 4) & 0x1F));
        write8(0xf8);
    }
}

void Armv8btAsm::readMem64Lock(U8 dst, U8 base) {
    write8(dst | (U8)(base << 5));
    write8(0xfc | (U8)(base >> 3));
    write8(0x5f);
    write8(0xc8);
}

void Armv8btAsm::readMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl) {
    write8(dst | (U8)(base << 5));
    if (lsl == 0) {
        write8(0x68 | (U8)(base >> 3));
    } else if (lsl == 3) {
        write8(0x78 | (U8)(base >> 3));
    } else {
        kpanic("ArmV8bt: readMem64RegOffset lsl must be 0 or 3: %d", lsl);
    }
    write8(0x60 | offsetReg);
    write8(0xf8);
}

void Armv8btAsm::writeMem8RegOffset(U8 dst, U8 base, U8 offsetReg) {
    write8(dst | (U8)(base << 5));
    write8(0x68 | (U8)(base >> 3));
    write8(0x20 | offsetReg);
    write8(0x38);
}

void Armv8btAsm::writeMem8ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);

        // STRB
        write8(dst | (U8)(base << 5));
        write8(0x68 | (U8)(base >> 3));
        write8(0x20 | tmp);
        write8(0x38);
        releaseTmpReg(tmp);
    } else {
        // STURB
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8((offset >> 4) & 0x1F);
        write8(0x38);
    }
}

void Armv8btAsm::writeMem16RegOffset(U8 dst, U8 base, U8 offsetReg) {
    write8(dst | (U8)(base << 5));
    write8(0x68 | (U8)(base >> 3));
    write8(0x20 | offsetReg);
    write8(0x78);
}

void Armv8btAsm::writeMem16ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        writeMem16RegOffset(dst, base, tmp);
        releaseTmpReg(tmp);
    } else {
        // STURH
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8((offset >> 4) & 0x1F);
        write8(0x78);
    }
}

void Armv8btAsm::writeMem32RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl) {
    write8(dst | (U8)(base << 5));

    if (lsl == 0) {
        write8(0x68 | (U8)(base >> 3));
    } else if (lsl == 2) {
        write8(0x78 | (U8)(base >> 3));
    } else {
        kpanic("ArmV8bt: writeMem32RegOffset lsl must be 0 or 2: %d", lsl);
    }

    write8(0x20 | offsetReg);
    write8(0xb8);
}

void Armv8btAsm::writeMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl) {
    write8(dst | (U8)(base << 5));
    
    if (lsl == 0) {
        write8(0x68 | (U8)(base >> 3));
    } else if (lsl == 3) {
        write8(0x78 | (U8)(base >> 3));
    } else {
        kpanic("ArmV8bt: writeMem64RegOffset lsl must be 0 or 3: %d", lsl);
    }

    write8(0x20 | offsetReg);
    write8(0xf8);
}

void Armv8btAsm::writeMem32ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        writeMem32RegOffset(dst, base, tmp);
        releaseTmpReg(tmp);
    } else {
        // STUR
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8((offset >> 4) & 0x1F);
        write8(0xb8);
    }
}

void Armv8btAsm::writeMem64ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);

        // STR
        write8(dst | (U8)(base << 5));
        write8(0x68 | (U8)(base >> 3));
        write8(0x20 | tmp);
        write8(0xf8);
        releaseTmpReg(tmp);
    } else {
        // STUR
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8((offset >> 4) & 0x1F);
        write8(0xf8);
    }
}

void Armv8btAsm::addRegs32(U8 dst, U8 src1, U8 src2, U8 src2ShiftLeft, bool flags) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | (src2ShiftLeft << 2));
    write8(src2);
    write8(flags?0x2b:0x0b); // 0b is 32-bit version (8b is 64-bit version)
}

void Armv8btAsm::addRegs64(U8 dst, U8 src1, U8 src2, U8 src2ShiftLeft, bool flags) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | (src2ShiftLeft << 2));
    write8(src2);
    write8(flags ? 0xab : 0x8b);
}

void Armv8btAsm::addValue32(U8 dst, U8 src, U32 value, bool flags) {
    if (value <= 0xFFF) {
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(value << 2));
        write8(value >> 6);
        write8(flags ? 0x31 : 0x11); // 11 is 32-bit version (91 is 64-bit version)
    } else {
        U8 tmp = getRegWithConst(value);
        addRegs32(dst, src, tmp, 0, flags);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::addValue64(U8 dst, U8 src, U32 value) {
    if (value <= 0xFFF) {
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(value << 2));
        write8(value >> 6);
        write8(0x91); // 11 is 32-bit version (91 is 64-bit version)
    } else {
        U8 tmp = getRegWithConst(value);
        addRegs64(dst, src, tmp);
        releaseTmpReg(tmp);
    }
}

// :TODO: is there a single instruction that can do this?
void Armv8btAsm::subRegFromValue32(U8 dst, U8 src, U32 value, bool flags) {
    if (value == 0) {
        subRegs32(dst, 31, src, 0, flags); // 31 WZR, the zero reg
    } else if (dst != src) {
        loadConst(dst, value);
        subRegs32(dst, dst, src, 0, flags);
    } else {
        U8 tmpReg = getTmpReg();
        loadConst(tmpReg, value);
        subRegs32(dst, tmpReg, src, 0, flags);
        releaseTmpReg(tmpReg);
    }
}

void Armv8btAsm::unsignedDivideReg32(U8 dst, U8 top, U8 bottom) {
    // udiv dst, top, bottom
    write8(dst | (U8)(top << 5));
    write8((U8)(top >> 3) | 0x8);
    write8(bottom | 0xC0);
    write8(0x1a);
}

void Armv8btAsm::unsignedDivideReg64(U8 dst, U8 top, U8 bottom) {
    // udiv dst, top, bottom
    write8(dst | (U8)(top << 5));
    write8((U8)(top >> 3) | 0x8);
    write8(bottom | 0xC0);
    write8(0x9a);
}

void Armv8btAsm::signedDivideReg32(U8 dst, U8 top, U8 bottom) {
    // sdiv dst, top, bottom
    write8(dst | (U8)(top << 5));
    write8((U8)(top >> 3) | 0xc);
    write8(bottom | 0xC0);
    write8(0x1a);
}

void Armv8btAsm::signedDivideReg64(U8 dst, U8 top, U8 bottom) {
    // sdiv dst, top, bottom
    write8(dst | (U8)(top << 5));
    write8((U8)(top >> 3) | 0xc);
    write8(bottom | 0xC0);
    write8(0x9a);
}

void Armv8btAsm::multiplySubtract32(U8 dst, U8 src1, U8 src2, U8 sub) {
    // msub dst, src1, src2, sub
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x80 | (sub << 2));
    write8(src2);
    write8(0x1b);
}

void Armv8btAsm::multiplySubtract64(U8 dst, U8 src1, U8 src2, U8 sub) {
    // msub dst, src1, src2, sub
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x80 | (sub << 2));
    write8(src2);
    write8(0x9b);
}

void Armv8btAsm::modValue32(U8 dst, U8 src, U32 value) {
    U8 tmpReg = getTmpReg();
    U8 mod = getRegWithConst(value);

    unsignedDivideReg32(tmpReg, src, mod);
    multiplySubtract32(dst, tmpReg, mod, src);
    releaseTmpReg(tmpReg);
    releaseTmpReg(mod);
}

void Armv8btAsm::subRegs32(U8 dst, U8 src1, U8 src2, U8 src2ShiftLeft, bool flags) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | (src2ShiftLeft << 2));
    write8(src2);
    write8(flags ? 0x6b : 0x4b); // 4b is 32-bit version (cb is 64-bit version)
}

void Armv8btAsm::subRegs64(U8 dst, U8 src1, U8 src2, bool flags) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(flags ? 0xeb : 0xcb);
}

void Armv8btAsm::subValue32(U8 dst, U8 src, U32 value, bool flags) {
    if (value <= 0xFFF) {
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(value << 2));
        write8(value >> 6);
        write8(flags ? 0x71 : 0x51); // 51 is 32-bit version (d1 is 64-bit version)
    } else {
        U8 tmp = getRegWithConst(value);
        subRegs32(dst, src, tmp, 0, flags);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::subValue64(U8 dst, U8 src, U32 value, bool flags) {
    if (value <= 0xFFF) {
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(value << 2));
        write8(value >> 6);
        write8(flags ? 0xf1 : 0xd1);
    } else {
        U8 tmp = getRegWithConst(value);
        subRegs64(dst, src, tmp, flags);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::cmpRegs32(U8 src1, U8 src2) {
    write8(0x1f | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(0x6b); // 6b is 32-bit version (eb is 64-bit version)
}

void Armv8btAsm::cmpRegs64(U8 src1, U8 src2) {
    write8(0x1f | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(0xeb);
}

void Armv8btAsm::cmpValue32(U8 src, U32 value) {
    if (value <= 0xFFF) {
        write8(0x1f | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(value << 2));
        write8(value >> 6);
        write8(0x71); // 71 is 32-bit version (f1 is 64-bit version)
    } else {
        U8 tmp = getRegWithConst(value);
        cmpRegs32(src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::cmpValue64(U8 src, U32 value) {
    if (value <= 0xFFF) {
        write8(0x1f | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(value << 2));
        write8(value >> 6);
        write8(0xf1); // 71 is 32-bit version (f1 is 64-bit version)
    } else {
        U8 tmp = getRegWithConst(value);
        cmpRegs64(src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::andRegs32(U8 dst, U8 src1, U8 src2, bool flags) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(flags?0x6a:0x0a); // 0a is 32-bit version (8a is 64-bit)
}

void Armv8btAsm::testRegs32(U8 src1, U8 src2) {
    write8(0x1f | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(0x6a);
}

void Armv8btAsm::testRegs64(U8 src1, U8 src2) {
    write8(0x1f | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(0xea);
}

void Armv8btAsm::andRegs64(U8 dst, U8 src1, U8 src2) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(0x8a);
}

void Armv8btAsm::notReg32(U8 dst, U8 src) {
    write8(dst | 0xe0);
    write8(0x03);
    write8(src | 0x20);
    write8(0x2a); // 2a is 32-bit version (aa is 64-bit)
}

void Armv8btAsm::andNotRegs32(U8 dst, U8 src1, U8 src2) {
    // bic v0, v0, v0
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2 | 0x20);
    write8(0x0a); // 0a is 32-bit version (8a is 64-bit)
}

void Armv8btAsm::andValue32(U8 dst, U8 src, U32 value, bool flags) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(imms << 2));
        write8((U8)(immr));
        write8(flags?0x72:0x12); // 12 is 32-bit version (92 is 64-bit)
    } else {
        U8 tmp = getRegWithConst(value);
        andRegs32(dst, src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::testValue32(U8 src, U32 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        write8(0x1f | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(imms << 2));
        write8((U8)(immr));
        write8(0x72);
    } else {
        U8 tmp = getRegWithConst(value);
        testRegs32(src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::andValue64(U8 dst, U8 src, U64 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 64, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(imms << 2));
        write8((U8)(immr) | 0x40);
        write8(0x92);
    } else {
        U8 tmp = getRegWithConst(value);
        andRegs64(dst, src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::testValue64(U8 src, U64 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 64, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        write8(0x1f | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(imms << 2));
        write8((U8)(immr) | 0x40);
        write8(0xf2);
    } else {
        U8 tmp = getRegWithConst(value);
        testRegs64(src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::orRegs32(U8 dst, U8 src1, U8 src2, U32 shiftLeft) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | (U8)(shiftLeft << 2));
    write8(src2);
    write8(0x2a); // 2a is 32-bit version (aa is 64-bit)
}

void Armv8btAsm::orRegs64(U8 dst, U8 src1, U8 src2, U32 shiftLeft) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | (U8)(shiftLeft << 2));
    write8(src2);
    write8(0xaa);
}

void Armv8btAsm::orValue32(U8 dst, U8 src, U32 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(imms << 2));
        write8((U8)(immr));
        write8(0x32); // 32 is 32-bit version
    } else {
        U8 tmp = getRegWithConst(value);
        orRegs32(dst, src, tmp);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::xorRegs32(U8 dst, U8 src1, U8 src2) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3));
    write8(src2);
    write8(0x4a); // 4a is 32-bit version (ca is 64-bit)
}

void Armv8btAsm::xorValue32(U8 dst, U8 src, U32 value) {
    U64 encoding = 0;
    if (processLogicalImmediate(value, 32, encoding)) {
        U32 imms = encoding & 0x3f;
        U32 immr = (encoding >> 6) & 0x3f;
        // N will always be 0 for 32-bit
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | (U8)(imms << 2));
        write8((U8)(immr));
        write8(0x52); // 52 is 32-bit version
    } else {
        if (value == 0) {
            xorRegs32(dst, src, 31);
        } else {
            U8 tmp = getRegWithConst(value);
            xorRegs32(dst, src, tmp);
            releaseTmpReg(tmp);
        }
    }
}

void Armv8btAsm::ubfm32(U8 dst, U8 src, U8 immr, U8 imms) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | (U8)(imms << 2));
    write8(immr);
    write8(0x53);
}

void Armv8btAsm::ubfm64(U8 dst, U8 src, U8 immr, U8 imms) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | (U8)(imms << 2));
    write8(immr | 0x40);
    write8(0xd3);
}

void Armv8btAsm::bfm32(U8 dst, U8 src, U8 immr, U8 imms) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | (U8)(imms << 2));
    write8(immr);
    write8(0x33);
}

void Armv8btAsm::bfm64(U8 dst, U8 src, U8 immr, U8 imms) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | (U8)(imms << 2));
    write8(immr | 0x40);
    write8(0xb3);
}

void Armv8btAsm::copyBitsFromSourceAtPositionToDest(U8 dst, U8 src, U8 srcLsb, U8 width, bool preserveOtherBits) {
    if (preserveOtherBits) {
        bfm32(dst, src, srcLsb, srcLsb + width - 1);
    } else {
        // UBFX
        ubfm32(dst, src, srcLsb, srcLsb + width - 1);
    }
}

void Armv8btAsm::copyBitsFromSourceAtPositionToDest64(U8 dst, U8 src, U8 srcLsb, U8 width, bool preserveOtherBits) {
    if (preserveOtherBits) {
        bfm64(dst, src, srcLsb, srcLsb + width - 1);
    } else {
        // UBFX
        ubfm64(dst, src, srcLsb, srcLsb + width - 1);
    }
}

void Armv8btAsm::copyBitsFromSourceToDestAtPosition(U8 dst, U8 src, U8 dstLsb, U8 width, bool preserveOtherBits) {
    U8 a = (U8)(-(S8)dstLsb);
    if (preserveOtherBits) {
        bfm32(dst, src, a % 32, width - 1);
    } else {
        // UBFIZ
        ubfm32(dst, src, a % 32, width - 1);
    }
}

void Armv8btAsm::copyBitsFromSourceToDestAtPosition64(U8 dst, U8 src, U8 dstLsb, U8 width, bool preserveOtherBits) {
    U8 a = (U8)(-(S8)dstLsb);
    if (preserveOtherBits) {
        bfm64(dst, src, a % 64, width - 1);
    } else {
        // UBFIZ
        ubfm64(dst, src, a % 64, width - 1);
    }
}

void Armv8btAsm::clearBits(U8 dst, U8 lsb, U8 width) {
    copyBitsFromSourceToDestAtPosition(dst, 31, lsb, width);
}

void Armv8btAsm::rotateRightWithReg32(U8 dst, U8 src, U8 amountReg) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x2c);
    write8(amountReg | 0xC0);
    write8(0x1a);
}

void Armv8btAsm::rotateRightWithValue32(U8 dst, U8 src, U8 value) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | value << 2);
    write8(0x80 | src);
    write8(0x13);
}

void Armv8btAsm::shiftRegLeftWithValue32(U8 dst, U8 src, U8 value) {
    if (value == 0) {
        if (dst != src) {
            mov32(dst, src);
        }
    } else {
        //  LSL reg, src, amount
        U8 a = (U8)(-(S8)value);
        ubfm32(dst, src, a % 32, (31 - value));
    }
}

void Armv8btAsm::shiftRegLeftWithValue64(U8 dst, U8 src, U8 value) {
    if (value == 0) {
        if (dst != src) {
            mov32(dst, src);
        }
    } else {
        //  LSL reg, src, amount
        U8 a = (U8)(-(S8)value);
        ubfm64(dst, src, a % 64, (63 - value));
    }
}

void Armv8btAsm::shiftRegLeftWithReg32(U8 dst, U8 src, U8 amountReg) {
    //  LSL reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x20);
    write8(amountReg | 0xC0);
    write8(0x1a);
}

void Armv8btAsm::shiftRegLeftWithReg64(U8 dst, U8 src, U8 amountReg) {
    //  LSL reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x20);
    write8(amountReg | 0xC0);
    write8(0x9a);
}

void Armv8btAsm::shiftRegRightWithValue32(U8 dst, U8 src, U8 value) {
    if (value == 0) {
        if (dst != src) {
            mov32(dst, src);
        }
    } else {
        ubfm32(dst, src, value, 31);
    }
}

void Armv8btAsm::shiftRegRightWithValue64(U8 dst, U8 src, U8 value) {
    if (value == 0) {
        if (dst != src) {
            mov64(dst, src);
        }
    } else {
        ubfm64(dst, src, value, 63);
    }
}

void Armv8btAsm::shiftSignedRegRightWithValue32(U8 dst, U8 src, U8 value) {
    // ASR reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x7C);
    write8(value);
    write8(0x13);
}

void Armv8btAsm::shiftSignedRegRightWithValue64(U8 dst, U8 src, U8 value) {
    // ASR reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xfC);
    write8(value | 0x40);
    write8(0x93);
}

void Armv8btAsm::shiftRegRightWithReg32(U8 dst, U8 src, U8 amountReg) {
    // LSR reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x24);
    write8(amountReg | 0xC0);
    write8(0x1a);
}

void Armv8btAsm::shiftRegRightWithReg64(U8 dst, U8 src, U8 amountReg) {
    // LSR reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x24);
    write8(amountReg | 0xC0);
    write8(0x9a);
}

void Armv8btAsm::shiftSignedRegRightWithReg32(U8 dst, U8 src, U8 amountReg) {
    // ASR reg, src, amount
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x28);
    write8(amountReg | 0xC0);
    write8(0x1a);
}

// src1 and src2 are signed 32-bit
// dst is signed 64-bit
void Armv8btAsm::signedMultiply32(U8 dst, U8 src1, U8 src2) {
    // SMULL dst, src1, src2
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x7c);
    write8(src2 | 0x20);
    write8(0x9b);
}

void Armv8btAsm::unsignedMultiply32(U8 dst, U8 src1, U8 src2) {
    // MUL dst, src1, src2
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x7c);
    write8(src2 );
    write8(0x1b);
}

void Armv8btAsm::unsignedMultiply64(U8 dst, U8 src1, U8 src2) {
    // MUL dst, src1, src2
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x7c);
    write8(src2);
    write8(0x9b);
}

// ZF = 1
void Armv8btAsm::csetEq(U8 reg) {
    write8(0xe0 | reg);
    write8(0x17);
    write8(0x9f);
    write8(0x1a);
}

// ZF = 0
void Armv8btAsm::csetNe(U8 reg) {
    write8(0xe0 | reg);
    write8(0x07);
    write8(0x9f);
    write8(0x1a);
}

// CF = 1
void Armv8btAsm::csetCc(U8 reg) {
    write8(0xe0 | reg);
    write8(0x27);
    write8(0x9f);
    write8(0x1a);
}

// OF = 1
void Armv8btAsm::csetVs(U8 reg) {
    write8(0xe0 | reg);
    write8(0x77);
    write8(0x9f);
    write8(0x1a);
}

void Armv8btAsm::csetLt(U8 reg) {
    write8(0xe0 | reg);
    write8(0xa7);
    write8(0x9f);
    write8(0x1a);
}

void Armv8btAsm::yield() {
    write8(0x3f);
    write8(0x20);
    write8(0x03);
    write8(0xd5);
}

void Armv8btAsm::cselVs(U8 dst, U8 ifTrueReg, U8 ifFalseReg) {
    // csel w0, w0, w0, vs
    write8(dst | (U8)(ifTrueReg << 5));
    write8((U8)(ifTrueReg >> 3) | 0x60);
    write8(ifFalseReg | 0x80);
    write8(0x1a);
}

void Armv8btAsm::clz32(U8 dst, U8 src) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x10);
    write8(0xc0);
    write8(0x5a); // 5a is 32-bit version (da is 64-bit)
}

void Armv8btAsm::clz64(U8 dst, U8 src) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x10);
    write8(0xc0);
    write8(0xda);
}

void Armv8btAsm::cls32(U8 dst, U8 src) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x14);
    write8(0xc0);
    write8(0x5a); // 5a is 32-bit version (da is 64-bit)
}

void Armv8btAsm::cls64(U8 dst, U8 src) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x14);
    write8(0xc0);
    write8(0xda);
}

U32 Armv8btAsm::branchEQ() {
    U32 pos = bufferPos;
    write8(0);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchNE() {
    U32 pos = bufferPos;
    write8(0x1);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchUnsignedGreaterThan() {
    // b.hi
    U32 pos = bufferPos;
    write8(0x8);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchUnsignedGreaterThanOrEqual() {
    // b.hs
    U32 pos = bufferPos;
    write8(0x2);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchUnsignedLessThan() {
    // b.lo
    U32 pos = bufferPos;
    write8(0x3);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchSignedGreaterThan() {
    // b.gt
    U32 pos = bufferPos;
    write8(0xc);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchUnsignedLessThanOrEqual() {
    // b.ls
    U32 pos = bufferPos;
    write8(0x9);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branchSignedLessThanOrEqual() {
    // b.le
    U32 pos = bufferPos;
    write8(0xd);
    write8(0);
    write8(0);
    write8(0x54);
    return pos;
}

U32 Armv8btAsm::branch() {
    U32 pos = bufferPos;
    write8(0);
    write8(0);
    write8(0);
    write8(0x14);
    return pos;
}

U8 Armv8btAsm::getNativeReg(U8 reg) {
    return reg;
}

U8 Armv8btAsm::getNativeMmxReg(U8 reg) {
    return reg+8;
}

U8 Armv8btAsm::getNativeFpuReg(U8 reg) {
    return reg+8;
}

U8 Armv8btAsm::getNativeSseReg(U8 reg) {
    return reg;
}

// for AH, CH, DH and BH, it will copy the the value into the bottom 8-bits of a tmp register
// otherwise it will just return the native register that represents EAX, ECX, EDX or EBX
U8 Armv8btAsm::getReadNativeReg8(U8 reg) {
    if (reg < 4) {
        return getNativeReg(reg);
    }
    U8 tmp = getTmpReg();
    U8 src = getNativeReg(reg % 4);
    // UBFX tmp, reg, 8, 8
    write8(tmp | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x3c);
    write8(0x08);
    write8(0x53);
    return tmp;
}

void Armv8btAsm::releaseNativeReg8(U8 reg) {
    if (reg >= xTmp1 && reg < xTmp1 + xNumberOfTmpRegs) {
        releaseTmpReg(reg);
    }
}

void Armv8btAsm::movRegToReg(U8 dst, U8 src, U32 width, bool zeroExtend) {
    if (width == 32) {
        mov32(dst, src);
    } else if (width == 16) {
        if (zeroExtend) {
            // UBFIZ dst, src, 0, 16
            write8(dst | (U8)(src << 5));
            write8((U8)(src >> 3) | 0x3c);
            write8(0x0);
            write8(0x53);
        } else {
            // BFXIL dst, src, 0, 16
            write8(dst | (U8)(src << 5));
            write8((U8)(src >> 3) | 0x3c);
            write8(0x0);
            write8(0x33);
        }
    } else if (width == 8) {
        kpanic("ArmV8bt: movRegToReg width=8 should use movReg8ToReg");
    } else {
        kpanic("ArmV8bt: movRegToReg with invalid width: %d", width);
    }
}

void Armv8btAsm::calculateAddress16(U8 dst) {
    // cpu->seg[op->base].address + (U16)(cpu->reg[op->rm].u16 + (S16)cpu->reg[op->sibIndex].u16 + op->disp)
    if (this->currentOp->base != SEG_ZERO) {
        if (this->currentOp->rm == regZero) {
            if (this->currentOp->sibIndex == regZero) {
                addValue32(dst, getSegReg(this->currentOp->base), (U16)this->currentOp->disp);
            } else {
                if (this->currentOp->disp) {
                    addValue32(dst, getNativeReg(this->currentOp->sibIndex), (U16)this->currentOp->disp);
                    andValue32(dst, dst, 0xFFFF);
                    addRegs32(dst, dst, getSegReg(this->currentOp->base));
                } else {
                    andValue32(dst, getNativeReg(this->currentOp->sibIndex), 0xFFFF);
                    addRegs32(dst, dst, getSegReg(this->currentOp->base));
                }
            }
        } else {
            if (this->currentOp->sibIndex == regZero) {
                if (this->currentOp->disp) {
                    addValue32(dst, getNativeReg(this->currentOp->rm), (U16)this->currentOp->disp);
                    andValue32(dst, dst, 0xFFFF);
                    addRegs32(dst, dst, getSegReg(this->currentOp->base));
                } else {
                    andValue32(dst, getNativeReg(this->currentOp->rm), 0xFFFF);
                    addRegs32(dst, dst, getSegReg(this->currentOp->base));
                }
            } else {
                addRegs32(dst, getNativeReg(this->currentOp->rm), getNativeReg(this->currentOp->sibIndex));
                if (this->currentOp->disp) {
                    addValue32(dst, dst, (U16)this->currentOp->disp);
                }
                andValue32(dst, dst, 0xFFFF);
                addRegs32(dst, dst, getSegReg(this->currentOp->base));
            }
        }
    } else {
        // special case for lea gw, we don't want to overwrite the top 16-bits of dst
        U8 tmp = getTmpReg();
        if (this->currentOp->rm == regZero) {
            if (this->currentOp->sibIndex == regZero) {
                movk(dst, this->currentOp->disp); // intentially movk to keep upper 16-bit
                releaseTmpReg(tmp);
                return;
            } else {
                if (this->currentOp->disp) {
                    addValue32(tmp, getNativeReg(this->currentOp->sibIndex), (U16)this->currentOp->disp);
                } else {
                    if (dst != getNativeReg(this->currentOp->sibIndex)) {
                        movRegToReg(dst, getNativeReg(this->currentOp->sibIndex), 16, false);
                    }
                    releaseTmpReg(tmp);
                    return;
                }
            }
        } else {
            if (this->currentOp->sibIndex == regZero) {
                if (this->currentOp->disp) {
                    addValue32(tmp, getNativeReg(this->currentOp->rm), (U16)this->currentOp->disp);
                } else {
                    if (dst != getNativeReg(this->currentOp->rm)) {
                        movRegToReg(dst, getNativeReg(this->currentOp->rm), 16, false);
                    }
                    releaseTmpReg(tmp);
                    return;                    
                }
            } else {
                addRegs32(tmp, getNativeReg(this->currentOp->rm), getNativeReg(this->currentOp->sibIndex));
                if (this->currentOp->disp) {
                    addValue32(tmp, tmp, (U16)this->currentOp->disp);
                }
            }
        }
        movRegToReg(dst, tmp, 16, false);
        releaseTmpReg(tmp);
    }
}

void Armv8btAsm::calculateAddress32(U8 dst) {
    bool needsSeg = this->currentOp->base != SEG_ZERO && KThread::currentThread()->process->hasSetSeg[this->currentOp->base];

    if (this->currentOp->rm == regZero) {
        if (this->currentOp->sibIndex == regZero) {
            // example lea eax, [0x1234]
            if (needsSeg) {
                addValue32(dst, getSegReg(this->currentOp->base), this->currentOp->disp);
                needsSeg = false;
            } else {
                loadConst(dst, this->currentOp->disp);
            }
        } else {
            if (this->currentOp->disp) {
                // example lea eax, [eax<<2 + 0x1234]
                if (this->currentOp->sibScale) {
                    shiftRegLeftWithValue32(dst, getNativeReg(this->currentOp->sibIndex), this->currentOp->sibScale);
                    addValue32(dst, dst, this->currentOp->disp);
                } else {
                    addValue32(dst, getNativeReg(this->currentOp->sibIndex), this->currentOp->disp);
                }
            } else {
                // example lea eax, [eax<<2]
                shiftRegLeftWithValue32(dst, getNativeReg(this->currentOp->sibIndex), this->currentOp->sibScale);
            }
        }
    } else {
        if (this->currentOp->sibIndex == regZero) {
            if (this->currentOp->disp) {
                // example lea eax, [eax + 0x1234]
                addValue32(dst, getNativeReg(this->currentOp->rm), this->currentOp->disp);
            } else {
                // example lea eax, [eax]
                if (needsSeg) {
                    addRegs32(dst, getNativeReg(this->currentOp->rm), getSegReg(this->currentOp->base));
                    needsSeg = false;
                } else {
                    if (dst != getNativeReg(this->currentOp->rm)) {
                        // :TODO: maybe return the reg instead of copying it if seg isn't needed?
                        mov32(dst, getNativeReg(this->currentOp->rm));
                    }
                }
            }
        } else {
            if (this->currentOp->disp) {
                // example lea eax, [eax + eax<<2 + 0x1234]
                addRegs32(dst, getNativeReg(this->currentOp->rm), getNativeReg(this->currentOp->sibIndex), this->currentOp->sibScale);
                addValue32(dst, dst, this->currentOp->disp);
            } else {
                // example lea eax, [eax + eax<<2]
                addRegs32(dst, getNativeReg(this->currentOp->rm), getNativeReg(this->currentOp->sibIndex), this->currentOp->sibScale);
            }
        }
    }
    if (needsSeg) {
        addRegs32(dst, dst, getSegReg(this->currentOp->base));
    }
}

U8 Armv8btAsm::getAddressReg() {
    U8 tmp = getTmpReg();
    if (this->currentOp->ea16) {
        calculateAddress16(tmp);
    } else {
        calculateAddress32(tmp);
    }
    return tmp;
}

U8 Armv8btAsm::getHostMem(U8 regEmulatedAddress, U32 width, bool write, bool skipAlignmentCheck, S8 tmpReg) {
    U8 resultReg = getTmpReg();
    bool needToReleaseTmpReg = false;

    if (tmpReg == -1) {
        tmpReg = getTmpReg();
        needToReleaseTmpReg = true;
    }

    shiftRegRightWithValue32(tmpReg, regEmulatedAddress, 12); // get page

    if (width != 8 && width != 16 && width != 32 && width != 64 && width != 128) {
        kpanic("Armv8btAsm::getHostMem bad width=%d %s", width, currentOp->name());
    }
    width = width / 8;
    readMem64RegOffset(resultReg, (write ? xMemWrite : xMemRead), tmpReg, 3);
    if (!skipAlignmentCheck && width > 1) {
        andValue32(tmpReg, regEmulatedAddress, 0xFFF);
        doIf(tmpReg, K_PAGE_SIZE - width, DO_IF_GREATER_THAN, [=] {
            writeToRegFromValue(resultReg, 0);
            });
    }
    doIf(resultReg, 0, DO_IF_EQUAL, [=] {
        emulateSingleOp(currentOp);
        });

    if (needToReleaseTmpReg) {
        releaseTmpReg(tmpReg);
    }
    return resultReg;
}

void Armv8btAsm::releaseHostMem(U8 reg) {
    releaseTmpReg(reg);
}

void Armv8btAsm::vReadMemory128(U8 addressReg, U8 dst, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 128, false);
        vReadMem128RegOffset(dst, addressReg, memReg);
        releaseHostMem(memReg);
    }
    else {
        vReadMem128ValueOffset(dst, addressReg, 0);
    }
}

void Armv8btAsm::vReadMemory64(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 64, false);
        addRegs64(addressReg, addressReg, memReg);
        releaseHostMem(memReg);
    }
    // ld1 {v0.d} [1], [x0]
    write8(dst | (U8)(addressReg << 5));
    write8((U8)(addressReg >> 3) | 0x84);
    write8(0x40);
    write8((index == 1) ? 0x4d : 0x0d);
}

void Armv8btAsm::vReadMemory64(U8 addressReg, U8 dst, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 64, false);
        vReadMem64RegOffset(dst, addressReg, memReg);
        releaseHostMem(memReg);
    }
    else {
        vReadMem64ValueOffset(dst, addressReg, 0);
    }
}

void Armv8btAsm::readWriteMemory(U8 addressReg, U8 readDst, U8 writeSrc, U32 width, std::function<void(void)> pfn, bool lock, bool doWrite) {
    U8 memReg = getHostMem(addressReg, width, doWrite);
    addRegs64(addressReg, addressReg, memReg);
    releaseHostMem(memReg);

    U32 restartPos = this->bufferPos;

    readMemory(addressReg, readDst, width, false, lock);
    pfn();
    if (doWrite) {
        writeMemory(addressReg, writeSrc, width, false, lock, readDst, restartPos);
    }
}

void Armv8btAsm::readMemory(U8 addressReg, U8 dst, U32 width, bool addMemOffsetToAddress, bool lock, bool signExtend) {
    if (lock) {        
        // :TODO: I'm not sure if we need this barrier, probably not
        // dmb	ish		// Full barrier
        // this->fullMemoryBarrier();
    }
    if (addMemOffsetToAddress) {
        if (lock) {
            kpanic("ArmV8bt: readMemory lock not implement for addMemOffsetToAddress = true");
        }
        U8 memReg = getHostMem(addressReg, width, false);

        if (width == 64) {
            readMem64RegOffset(dst, addressReg, memReg, 0);
        } else if (width == 32) {
            readMem32RegOffset(dst, addressReg, memReg, 0);
        } else if (width == 16) {
            readMem16RegOffset(dst, addressReg, memReg, signExtend);
        } else if (width == 8) {
            readMem8RegOffset(dst, addressReg, memReg, signExtend);
        } else {
            kpanic("ArmV8bt: readMemory with invalid width: %d", width);
        }    
        releaseHostMem(memReg);
    } else if (lock) {
        if (width == 64) {
            readMem64Lock(dst, addressReg);
        }
        else if (width == 32) {
            readMem32Lock(dst, addressReg);
        }
        else if (width == 16) {
            readMem16Lock(dst, addressReg);
        }
        else if (width == 8) {
            readMem8Lock(dst, addressReg);
        }
        else {
            kpanic("ArmV8bt: readMemory with invalid width: %d", width);
        }
    } else {
        if (width == 64) {
            readMem64ValueOffset(dst, addressReg, 0);
        } else if (width == 32) {
            readMem32ValueOffset(dst, addressReg, 0);
        } else if (width == 16) {
            readMem16ValueOffset(dst, addressReg, 0, signExtend);
        } else if (width == 8) {
            readMem8ValueOffset(dst, addressReg, 0, signExtend);
        } else {
            kpanic("ArmV8bt: readMemory with invalid width: %d", width);
        }
    }
}

void Armv8btAsm::fullMemoryBarrier() {
    // dmb ish
    write8(0xbf);
    write8(0x3b);
    write8(0x03);
    write8(0xd5);
}

void Armv8btAsm::writeMemory(U8 addressReg, U8 src, U32 width, bool addMemOffsetToAddress, bool lock, U8 regWithOriginalValue, U32 restartPos, bool generateMemoryBarrierForLock) {
    if (lock) {
        // :TODO: maybe use ldrx/stxr instead of ldarx/stlxr since we have a dmb ish at the end?
        // stlxr        
        if (addMemOffsetToAddress) {
            kpanic("ArmV8bt: writeMemory lock not implement for addMemOffsetToAddress = true");
        }
        U8 statusReg = getTmpReg();       
        // addressReg should already be added with xMem

        write8(src | (U8)(addressReg << 5));
        write8(0xfc | (U8)(addressReg >> 3));
        write8(statusReg);        

        if (width == 64) {
            // STLXR w0, x0, [x0]
            write8(0xc8);
        } else if (width == 32) {
            // STLXR w0, w0, [x0]
            write8(0x88);
        } else if (width == 16) {
            // STLXRH w0, w0, [x0]
            write8(0x48);
        } else if (width == 8) {
            // STLXRB w0, w0, [x0]
            write8(0x08);
        } else {
            kpanic("ArmV8bt: writeMemory with invalid width: %d", width);
        }

        S32 amount = -(S32)(this->bufferPos - restartPos);
        amount /= 4;
        // CBNZ
        write8(statusReg | ((U8)amount) << 5);
        write8((U8)(amount >> 3));
        write8((U8)(amount >> 11));        
        write8(0x35);

        releaseTmpReg(statusReg);

        if (generateMemoryBarrierForLock) {
            fullMemoryBarrier();
        }

        return;
    }
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, width, true);
        if (width == 64) {
            writeMem64RegOffset(src, addressReg, memReg);
        } else if (width == 32) {
            writeMem32RegOffset(src, addressReg, memReg);
        } else if (width == 16) {
            writeMem16RegOffset(src, addressReg, memReg);
        } else if (width == 8) {
            writeMem8RegOffset(src, addressReg, memReg);
        } else {
            kpanic("ArmV8bt: writeMemory with invalid width: %d", width);
        }
        releaseHostMem(memReg);
    } else {
        if (width == 64) {
            writeMem64ValueOffset(src, addressReg, 0);
        } else if (width == 32) {
            writeMem32ValueOffset(src, addressReg, 0);
        } else if (width == 16) {
            writeMem16ValueOffset(src, addressReg, 0);
        } else if (width == 8) {
            writeMem8ValueOffset(src, addressReg, 0);
        } else {
            kpanic("ArmV8bt: writeMemory with invalid width: %d", width);
        }
    }
}

void Armv8btAsm::zeroExtend(U8 dst, U8 src, U32 width) {
    if (width == 32) {
        // nothing to do
    } else if (width == 16) {
        // UXTH dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x3c);
        write8(0);
        write8(0x53);
    } else if (width == 8) {
        // UXTB dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x1c);
        write8(0);
        write8(0x53);
    } else {
        kpanic("ArmV8bt: zeroExtend with invalid width: %d", width);
    }
}

void Armv8btAsm::zeroExtend64(U8 dst, U8 src, U32 width) {
    if (width == 32) {
        // UXTW dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x7c);
        write8(0x40);
        write8(0xd3);
    } else if (width == 16) {
        // UXTH dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x3c);
        write8(0);
        write8(0x53);
    } else if (width == 8) {
        // UXTB dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x1c);
        write8(0);
        write8(0x53);
    } else {
        kpanic("ArmV8bt: zeroExtend with invalid width: %d", width);
    }
}

void Armv8btAsm::signExtend(U8 dst, U8 src, U32 width) {
    if (width == 32) {
        // nothing to do
    } else if (width == 16) {
        // SXTH dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x3c);
        write8(0);
        write8(0x13);
    } else if (width == 8) {
        // SXTB dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x1c);
        write8(0);
        write8(0x13);
    } else {
        kpanic("ArmV8bt: signExtend with invalid width: %d", width);
    }
}

void Armv8btAsm::signExtend64(U8 dst, U8 src, U32 width) {
    if (width == 32) {
        // SXTW dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x7c);
        write8(0x40);
        write8(0x93);
    } else if (width == 16) {
        // SXTH dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x3c);
        write8(0x40);
        write8(0x93);
    } else if (width == 8) {
        // SXTB dst, src
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x1c);
        write8(0x40);
        write8(0x93);
    } else {
        kpanic("ArmV8bt: signExtend with invalid width: %d", width);
    }
}

// will zero extend into dst
void Armv8btAsm::movReg8ToReg(U8 untranslatedSrc, U8 dst, bool signExtend) {
    // untranslatedDst
    // 0-3 is AL, CL, DL and BL (mask is 0x000000FF)
    // 4-7 is AH, CH, DH and BH (mask is 0x0000FF00)
    U8 translatedSrc = getNativeReg((U8)(untranslatedSrc % 4));
    if (signExtend) {
        if (untranslatedSrc < 4) {
            this->signExtend(dst, translatedSrc, 8);
        } else {
            // SBFX dst, src, 8, 8
            write8(dst | (U8)(translatedSrc << 5));
            write8((U8)(translatedSrc >> 3) | 0x3c);
            write8(0x8);
            write8(0x13);
        }
    } else {
        if (untranslatedSrc < 4) {
            zeroExtend(dst, translatedSrc, 8);
        } else {
            // UBFX dst, src, 8, 8
            write8(dst | (U8)(translatedSrc << 5));
            write8((U8)(translatedSrc >> 3) | 0x3c);
            write8(0x8);
            write8(0x53);
        }
    }
}

void Armv8btAsm::movRegToReg8(U8 src, U8 untranslatedDst) {
    // untranslatedDst
    // 0-3 is AL, CL, DL and BL (mask is 0x000000FF)
    // 4-7 is AH, CH, DH and BH (mask is 0x0000FF00)
    U8 translatedDst = getNativeReg((U8)(untranslatedDst % 4));
    if (untranslatedDst < 4) {
        // BFXIL dst, src, 0, 8
        write8(translatedDst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x1c);
        write8(0x0);
        write8(0x33);
    } else {
        // BFI dst, src, 8, 8
        // copies bottom 8 bits from src into 0x0000FF00 position
        write8(translatedDst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x1c);
        write8(0x18);
        write8(0x33);
    }
}

void Armv8btAsm::movReg8ToReg8(U8 untranslatedDst, U8 untranslatedSrc) {
    // I'm not aware of a single ARMv8 instruction that can copy the 2nd 8-bits of a src into the 2nd 8-bit of the dst
    U8 readReg = getReadNativeReg8(untranslatedSrc);
    movRegToReg8(readReg, untranslatedDst);
    releaseNativeReg8(readReg);
}

// void CPU::push16(U16 value) {
//     U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 2) & this->stackMask);
//     writew(this->seg[SS].address + (new_esp & this->stackMask), value);
//     THIS_ESP = new_esp;
// }
void Armv8btAsm::pushStack16(U8 reg, U8 resultReg, U32 amount) {
    // U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 2) & this->stackMask);
    U8 tmpReg = getTmpReg();
    U8 tmpReg2 = getTmpReg();
    andNotRegs32(tmpReg2, reg, xStackMask);
    subValue32(tmpReg, reg, amount);
    andRegs32(tmpReg, tmpReg, xStackMask);
    orRegs32(resultReg, tmpReg2, tmpReg);
    releaseTmpReg(tmpReg);
    releaseTmpReg(tmpReg2);
}
void Armv8btAsm::pushNativeReg16(U8 reg) {
    U8 addressReg = getTmpReg();    

    // U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 2) & this->stackMask);
    pushStack16(xESP, addressReg);

    U8 tmpReg = getTmpReg();    

    // this->seg[SS].address + (new_esp & this->stackMask)
    andRegs32(tmpReg, addressReg, xStackMask);
    addRegs32(tmpReg, tmpReg, xSS);

    // writew(address, value)
    writeMemory(tmpReg, reg, 16, true);

    // THIS_ESP = new_esp;
    movRegToReg(xESP, addressReg, 32, false);

    releaseTmpReg(addressReg);
    releaseTmpReg(tmpReg);
}

// U16 CPU::pop16() {
//     U16 val = readw(this->seg[SS].address + (THIS_ESP & this->stackMask));
//     THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + 2) & this->stackMask);
//     return val;
// }
void Armv8btAsm::peekNativeReg16(U8 reg, bool zeroExtend) {
    U8 tmpReg = getTmpReg();

    // this->seg[SS].address + (THIS_ESP & this->stackMask)
    andRegs32(tmpReg, xESP, xStackMask);
    addRegs32(tmpReg, tmpReg, xSS);

    if (zeroExtend) {        
        readMemory(tmpReg, reg, 16, true);        
    } else {
        readMemory(tmpReg, tmpReg, 16, true);
        movRegToReg(reg, tmpReg, 16, false);
    }
    releaseTmpReg(tmpReg);
}

void Armv8btAsm::popStack16(U8 reg, U8 resultReg) {
    U8 tmpReg = getTmpReg();
    U8 tmpReg2 = getTmpReg();
    // THIS_ESP & this->stackNotMask
    andNotRegs32(tmpReg2, reg, xStackMask);
    // (THIS_ESP + 2) & this->stackMask
    addValue32(tmpReg, reg, 2);
    andRegs32(tmpReg, tmpReg, xStackMask);

    orRegs32(resultReg, tmpReg, tmpReg2);

    releaseTmpReg(tmpReg2);
    releaseTmpReg(tmpReg);
}

void Armv8btAsm::popStack16() {
    popStack16(xESP, xESP);
}

void Armv8btAsm::popNativeReg16(U8 reg, bool zeroExtend) {
    peekNativeReg16(reg, zeroExtend);
    if (reg != 4) {
        popStack16();
    }
}

// U32 CPU::pop32() {
//     U32 val = readd(this->seg[SS].address + (THIS_ESP & this->stackMask));
//     THIS_ESP = (THIS_ESP & this->stackNotMask) | ((THIS_ESP + 4) & this->stackMask);
//     return val;
// }
void Armv8btAsm::peekNativeReg32(U8 reg) {
    if (!KThread::currentThread()->process->hasSetSeg[SS]) {
        readMemory(xESP, reg, 32, true);
    } else {
        U8 tmpReg = getTmpReg();

        // this->seg[SS].address + (THIS_ESP & this->stackMask)
        andRegs32(tmpReg, xESP, xStackMask);
        addRegs32(tmpReg, tmpReg, xSS);

        readMemory(tmpReg, reg, 32, true);

        releaseTmpReg(tmpReg);
    }    
}

void Armv8btAsm::popStack32(U8 reg, U8 resultReg) {
    if (!KThread::currentThread()->process->hasSetSeg[SS]) {
        addValue32(resultReg, reg, 4);
    } else {
        U8 tmpReg = getTmpReg();
        U8 tmpReg2 = getTmpReg();
        // THIS_ESP & this->stackNotMask
        andNotRegs32(tmpReg2, reg, xStackMask);
        // (THIS_ESP + 4) & this->stackMask
        addValue32(tmpReg, reg, 4);
        andRegs32(tmpReg, tmpReg, xStackMask);

        orRegs32(resultReg, tmpReg, tmpReg2);

        releaseTmpReg(tmpReg2);
        releaseTmpReg(tmpReg);
    }
}

void Armv8btAsm::popStack32() {
    popStack32(xESP, xESP);
}
void Armv8btAsm::popNativeReg32(U8 reg) {
    peekNativeReg32(reg);
    if (reg != 4) {
        popStack32();
    }
}

// void CPU::push32(U32 value) {
//     U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 4) & this->stackMask);
//     writed(this->seg[SS].address + (new_esp & this->stackMask), value);
//     THIS_ESP = new_esp;
// }
void Armv8btAsm::pushStack32(U8 reg, U8 resultReg, U32 amount) {
    if (!this->cpu->thread->process->hasSetSeg[SS]) {
        subValue32(resultReg, reg, amount);
    } else {
        U8 tmpReg = getTmpReg();
        U8 tmpReg2 = getTmpReg();
        andNotRegs32(tmpReg2, reg, xStackMask);
        subValue32(tmpReg, reg, amount);
        andRegs32(tmpReg, tmpReg, xStackMask);
        orRegs32(resultReg, tmpReg2, tmpReg);
        releaseTmpReg(tmpReg);
        releaseTmpReg(tmpReg2);
    }
}
void Armv8btAsm::pushNativeReg32(U8 reg) {
    U8 addressReg = getTmpReg();
    // U32 new_esp = (THIS_ESP & this->stackNotMask) | ((THIS_ESP - 4) & this->stackMask);
    pushStack32(xESP, addressReg);

    if (!this->cpu->thread->process->hasSetSeg[SS]) {
        writeMemory(addressReg, reg, 32, true);
    } else {               
        U8 tmpReg = getTmpReg();

        andRegs32(tmpReg, addressReg, xStackMask);
        addRegs32(tmpReg, tmpReg, xSS);
        writeMemory(tmpReg, reg, 32, true);        
        releaseTmpReg(tmpReg);
    }
    // THIS_ESP = new_esp;
    movRegToReg(xESP, addressReg, 32, false);
    releaseTmpReg(addressReg);
}

Armv8btAsm::Armv8btAsm(Armv8btCPU* cpu) : Armv8btData(cpu) {
    memset(tmpRegInUse, 0, sizeof(tmpRegInUse));
    memset(vTmpRegInUse, 0, sizeof(vTmpRegInUse));
}

void Armv8btAsm::reset() {
    Armv8btData::reset();
    memset(tmpRegInUse, 0, sizeof(tmpRegInUse));
    memset(vTmpRegInUse, 0, sizeof(vTmpRegInUse));
}

// only called when the thread starts
void Armv8btAsm::saveNativeState() {
    pushPair(19, 20);
    pushPair(21, 22);
    pushPair(23, 24);
    pushPair(25, 26);
    pushPair(27, 28);
    pushPair(29, 30); // 30 is LR

    // only the bottom 64-bits of v8-v15 need to be saved
    //subValue64(31, 31, 32);
    //vWriteMemMultiple64(8, 31, 4, false);
    //subValue64(31, 31, 32);
    //vWriteMemMultiple64(12, 31, 4, false);
}

// only called when we exit from the thread
void Armv8btAsm::restoreNativeState() {
    //vReadMemMultiple64(12, 31, 4, true);
    //vReadMemMultiple64(8, 31, 4, true);

    popPair(29, 30);
    popPair(27, 28);
    popPair(25, 26);
    popPair(23, 24);
    popPair(21, 22);
    popPair(19, 20);
}

void Armv8btAsm::writeToRegFromValue(U8 reg, U64 value) {
    loadConst(reg, value);
}

void Armv8btAsm::setNativeFlags(U32 flags, U32 mask) {
    loadConst(xFLAGS, (flags & mask)|2);
}

void Armv8btAsm::branchNativeRegister(U8 reg) {
    // br reg
    write8(reg << 5);
    write8(reg >> 3);
    write8(0x1f);
    write8(0xd6);
}

void Armv8btAsm::jmpRegToxBranchEip(bool mightNeedCS) {
    // hard coded regs so that the exception handler will know what to expect

    if (this->cpu->thread->process->hasSetSeg[CS] || mightNeedCS) {
        U8 tmpReg = getTmpReg();
        addRegs32(tmpReg, xBranchEip, xCS);
        andValue32(xOffset, tmpReg, 0xFFF); // get page offset
        shiftRegRightWithValue32(xPage, tmpReg, 12); // get page
        releaseTmpReg(tmpReg);
    } else {
        andValue32(xOffset, xBranchEip, 0xFFF); // get page offset
        shiftRegRightWithValue32(xPage, xBranchEip, 12); // get page
    }
    readMem64ValueOffset(xBranch, xCPU, CPU_OFFSET_OP_PAGES); // get offset table pages
    readMem64RegOffset(xBranch, xBranch, xPage, 3); // get offset table for page

    cmpValue64(xBranch, 0);
    doIf(xBranch, 0, DO_IF_EQUAL, [=] {
        writeMem32ValueOffset(xBranchEip, xCPU, CPU_OFFSET_EIP);
        readMem64ValueOffset(xBranch, xCPU, (U32)(offsetof(Armv8btCPU, jmpAndTranslateIfNecessary)));
        branchNativeRegister(xBranch);
        }, nullptr, nullptr, false, false);

    readMem64RegOffset(xBranch, xBranch, xOffset, 3); // read value at offset for page

    cmpValue64(xBranch, 0);
    doIf(xBranch, 0, DO_IF_EQUAL, [=] {
        writeMem32ValueOffset(xBranchEip, xCPU, CPU_OFFSET_EIP);
        readMem64ValueOffset(xBranch, xCPU, (U32)(offsetof(Armv8btCPU, jmpAndTranslateIfNecessary)));
        branchNativeRegister(xBranch);
        }, nullptr, nullptr, false, false);

#ifdef _DEBUG
    // make it easier to see where we jumped from
    loadConst(13, this->startOfOpIp);
#endif
    branchNativeRegister(xBranch);
}

void Armv8btAsm::doJmp(bool mightNeedCS) {
    // ok to call 32-bit read for 16-bit instruction, cpu->eip.u32 will be properly masked by this point
    readMem32ValueOffset(xBranchEip, xCPU, CPU_OFFSET_EIP);
    jmpRegToxBranchEip(mightNeedCS);
}

static void armv8_retranslateChunkAdjustForCS() {
    Armv8btCPU* cpu = ((Armv8btCPU*)KThread::currentThread()->cpu);
    cpu->eip.u32 -= cpu->seg[CS].address;
    if (!cpu->isBig()) {
        cpu->eip.u32 = cpu->eip.u32 & 0xFFFF;
    }
    cpu->returnHostAddress = cpu->reTranslateChunk();
}

static void armv8_translateIfNecessary(Armv8btCPU* cpu) {
    if (!cpu->isBig()) {
        cpu->eip.u32 = cpu->eip.u32 & 0xFFFF;
    }
    cpu->returnHostAddress = (U64)cpu->translateEip(cpu->eip.u32);
}

void Armv8btAsm::createCodeForJmpAndTranslateIfNecessary() {
    syncRegsFromHost(true);
    mov64(0, xCPU); // param 1 (CPU)
    callHost((void*)armv8_translateIfNecessary);
    syncRegsToHost();
    readMem64ValueOffset(xBranch, xCPU, (U32)(offsetof(BtCPU, returnHostAddress)));
    branchNativeRegister(xBranch);
}

void common_runSingleOp(BtCPU* cpu);
void Armv8btAsm::createCodeForDoSingleOp() {
    callSyncRegsFromHost();
    mov64(0, xCPU); // param 1 (CPU)
    callHost((void*)common_runSingleOp);
    syncRegsToHost();
    readMem64ValueOffset(xBranch, xCPU, (U32)(offsetof(BtCPU, returnHostAddress)));
    branchNativeRegister(xBranch);
}

void Armv8btAsm::emulateSingleOp(DecodedOp* op) {
    loadConst(xBranch, this->startOfOpIp);
    writeMem32ValueOffset(xBranch, xCPU, CPU_OFFSET_EIP);
    loadConst(xBranch, (U64)op);
    writeMem64ValueOffset(xBranch, xCPU, CPU_OFFSET_CURRENT_OP);
    loadConst(xBranch, CPU_OFFSET_DO_SINGLE_OP_ADDRESS);
    readMem64RegOffset(xBranch, xCPU, xBranch);
    branchNativeRegister(xBranch); // we won't return to here

    fpuOffsetRegSet = false;
    fpuTopRegSet = false;
    clearCachedFpuRegs();
}

void signalHandler();

void Armv8btAsm::createCodeForRunSignal() {
    callHost((void*)signalHandler);
    syncRegsToHost();
    readMem64ValueOffset(xBranch, xCPU, (U32)(offsetof(BtCPU, returnHostAddress)));
    branchNativeRegister(xBranch);
}

void Armv8btAsm::write64Buffer(U8* buffer, U64 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
    buffer[4] = (U8)(value >> 32);
    buffer[5] = (U8)(value >> 40);
    buffer[6] = (U8)(value >> 48);
    buffer[7] = (U8)(value >> 56);
}

void Armv8btAsm::write32Buffer(U8* buffer, U32 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);
    buffer[2] = (U8)(value >> 16);
    buffer[3] = (U8)(value >> 24);
}

void Armv8btAsm::write16Buffer(U8* buffer, U16 value) {
    buffer[0] = (U8)value;
    buffer[1] = (U8)(value >> 8);

}

void Armv8btAsm::callRetranslateChunk() {
    kpanic("callRetranslateChunk not implemented");
}

void Armv8btAsm::addTodoLinkJump(U32 eip) {
    this->todoJump.push_back(TodoJump(eip, this->bufferPos - 4, this->ipAddressCount));
}

bool Armv8btAsm::isEipInChunk(U32 eip) {
    KMemoryData* mem = getMemData(cpu->memory);
    if (mem->getExistingHostAddress(eip)) {
        return false;
    }
    return (this->stopAfterInstruction != (S32)this->ipAddressCount && (this->calculatedEipLen == 0 || (eip >= this->startOfDataIp && eip < this->startOfDataIp + this->calculatedEipLen)));
}

void Armv8btAsm::jumpTo(U32 eip) {
    if (!this->cpu->isBig()) {
        eip = eip & 0xffff;
    }
#ifdef _DEBUG
    // this->writeToMemFromValue(this->startOfOpIp, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP_FROM, 4, false);
#endif
    // :TODO: is this necessary?  who uses it?
    //this->writeToMemFromValue(eip, HOST_CPU, true, -1, false, 0, CPU_OFFSET_EIP, 4, false);
    if (isEipInChunk(eip)) {
        // b
        write8(0);
        write8(0);
        write8(0);
        write8(0x14);
        addTodoLinkJump(eip);
    } else {
        // when a chunk gets modified/replaced other chunks that point to it via this jump need to get updated
        // it is not possible to modify the executable code directly in an atomic way, so instead of embedding
        // where we will jump directly into the instruction, we will encode an instruction that reads the jump
        // address from memory (data).  That memory location can be atomically updated.
        /*
        if (0) {
            // this can result in random crashes for dynamic code, Quake 2 will see this
            // b
            write8(0);
            write8(0);
            write8(0);
            write8(0x14);
            addTodoLinkJump(eip, 4, false);
        } else */{
            loadConst(xBranchEip, eip);
            jmpRegToxBranchEip(false);
        }
    }
}

void Armv8btAsm::addReturn() {
    write8(0xc0);
    write8(0x03);
    write8(0x5f);
    write8(0xd6);
}

U8 Armv8btAsm::getTmpReg() {
    for (int i = 0; i < xNumberOfTmpRegs; i++) {
        if (!tmpRegInUse[i]) {
            tmpRegInUse[i] = true;
            return xTmp1 + i;
        }
    }
    kpanic("Armv8btAsm ran out of tmp regs");
    return 0;
}

void Armv8btAsm::releaseTmpReg(U8 reg) {
    tmpRegInUse[reg - xTmp1] = false;
}

void Armv8btAsm::ret() {
    write8(0xc0);
    write8(0x03);
    write8(0x5f);
    write8(0xd6);
}

void Armv8btAsm::blr(U8 reg) {
    // BLR reg
    write8((U8)(reg << 5));
    write8((U8)(reg >> 3));
    write8(0x3f);
    write8(0xd6);
}

void Armv8btAsm::callHost(void* pfn) {
    U8 tmp = getRegWithConst((U64)pfn);
    blr(tmp);
    fpuTopRegSet = false;
    releaseTmpReg(tmp);
}

/*
x30 is the link register (used to return from subroutines)
x29 is the frame register
x19 to x29 are callee - saved
x18 is the 'platform register', used for some operating - system - specific special purpose, or an additional caller - saved register
x16 and x17 are the Intra - Procedure - call scratch register
x9 to x15 : used to hold local variables(caller saved)
x8 : used to hold indirect return value address
x0 to x7 : used to hold argument values passed to a subroutine, and also hold results returned from a subroutine
*/

// regs and flags (x0-x8) are wiped out and will need to saved/loaded
// all other xRegs (xCPU, xMem, xES, etc) are callee saved will be reloaded as necessary, but already have their values set on the cpu so they don't need to be set
void Armv8btAsm::syncRegsFromHost(bool eipInBranchReg) {
    if (!eipInBranchReg) {
        U8 tmpReg = getTmpReg();
        loadConst(tmpReg, this->startOfOpIp);
        writeMem32ValueOffset(tmpReg, xCPU, CPU_OFFSET_EIP);
        releaseTmpReg(tmpReg);
    }
    callSyncRegsFromHost();
}

void Armv8btAsm::callSyncRegsFromHost() {
    readMem64ValueOffset(xBranch, xCPU, CPU_OFFSET_SYNC_FROM_HOST_ADDRESS);
    blr(xBranch);
}

void Armv8btAsm::createCodeForSyncFromHost() {
    writeMem32ValueOffset(xEAX, xCPU, CPU_OFFSET_EAX);
    writeMem32ValueOffset(xECX, xCPU, CPU_OFFSET_ECX);
    writeMem32ValueOffset(xEDX, xCPU, CPU_OFFSET_EDX);
    writeMem32ValueOffset(xEBX, xCPU, CPU_OFFSET_EBX);
    writeMem32ValueOffset(xESP, xCPU, CPU_OFFSET_ESP);
    writeMem32ValueOffset(xEBP, xCPU, CPU_OFFSET_EBP);
    writeMem32ValueOffset(xESI, xCPU, CPU_OFFSET_ESI);
    writeMem32ValueOffset(xEDI, xCPU, CPU_OFFSET_EDI);
    writeMem32ValueOffset(xFLAGS, xCPU, CPU_OFFSET_FLAGS);

    U8 addressReg = getTmpReg();
    addValue64(addressReg, xCPU, (U32)(offsetof(CPU, xmm[0])));
    vWriteMemMultiple128(xXMM0, addressReg, 4, true);
    vWriteMemMultiple128(xXMM4, addressReg, 4, false);

    // when using the FPU, these will contain cached FPU values
    addValue64(addressReg, xCPU, (U32)(offsetof(CPU, reg_mmx[0])));
    vWriteMemMultiple64(vMMX0, addressReg, 4, true);
    vWriteMemMultiple64(vMMX4, addressReg, 4, false);

    releaseTmpReg(addressReg);
    ret();
}

// don't adjust flags in this, several doif calls like setSegment, assume this
void Armv8btAsm::syncRegsToHost() {
    readMem64ValueOffset(xBranch, xCPU, CPU_OFFSET_SYNC_TO_HOST_ADDRESS);
    blr(xBranch);    
    fpuOffsetRegSet = false;
    fpuTopRegSet = false;
    clearCachedFpuRegs();
}

void Armv8btAsm::createCodeForSyncToHost() {
    readMem32ValueOffset(xEAX, xCPU, CPU_OFFSET_EAX);
    readMem32ValueOffset(xECX, xCPU, CPU_OFFSET_ECX);
    readMem32ValueOffset(xEDX, xCPU, CPU_OFFSET_EDX);
    readMem32ValueOffset(xEBX, xCPU, CPU_OFFSET_EBX);
    readMem32ValueOffset(xESP, xCPU, CPU_OFFSET_ESP);
    readMem32ValueOffset(xEBP, xCPU, CPU_OFFSET_EBP);
    readMem32ValueOffset(xESI, xCPU, CPU_OFFSET_ESI);
    readMem32ValueOffset(xEDI, xCPU, CPU_OFFSET_EDI);
    readMem32ValueOffset(xFLAGS, xCPU, CPU_OFFSET_FLAGS);

    U8 addressReg = getTmpReg();
    addValue64(addressReg, xCPU, (U32)(offsetof(CPU, xmm[0])));    
    vReadMemMultiple128(xXMM0, addressReg, 4, true);
    vReadMemMultiple128(xXMM4, addressReg, 4, false);

    // when using the FPU, these will contain cached FPU values
    addValue64(addressReg, xCPU, (U32)(offsetof(CPU, reg_mmx[0])));
    vReadMemMultiple64(vMMX0, addressReg, 4, true);
    vReadMemMultiple64(vMMX4, addressReg, 4, false);

    // if we are using the FPU, these need to be restored
    // Quake 2 time demo will require this because it will cause an exception from self modifying code while using the FPU
    //addValue64(xFpuOffset, xCPU, (U32)(offsetof(CPU, fpu)));

    // these 2 instructions instead of readMem32ValueOffset because readMem32ValueOffset uses a tmp reg and we are out for the div instruction
   // loadConst(addressReg, (U32)(offsetof(CPU, fpu.top)));
    //this->readMem32RegOffset(xFpuTop, xCPU, addressReg);
    releaseTmpReg(addressReg);

    // In the future, maybe only set these if they could have changed
    this->readMem32ValueOffset(xES, xCPU, (U32)(offsetof(CPU, seg[ES].address)));
    this->readMem32ValueOffset(xCS, xCPU, (U32)(offsetof(CPU, seg[CS].address)));
    this->readMem32ValueOffset(xSS, xCPU, (U32)(offsetof(CPU, seg[SS].address)));
    this->readMem32ValueOffset(xDS, xCPU, (U32)(offsetof(CPU, seg[DS].address)));
    this->readMem32ValueOffset(xFS, xCPU, (U32)(offsetof(CPU, seg[FS].address)));
    this->readMem32ValueOffset(xGS, xCPU, (U32)(offsetof(CPU, seg[GS].address)));

    this->readMem32ValueOffset(xStackMask, xCPU, (U32)(offsetof(CPU, stackMask)));
    ret();
}

void Armv8btAsm::writeJumpAmount(U32 pos, U32 toLocation) {
    S32 amount = (S32)toLocation - (S32)pos;
    amount = amount >> 2;
    if (this->buffer[pos + 3] == 0x14) {
        if (amount > 0xFFFFFF) {
            kpanic("armv8::jump in large if/else blocks not supported: %d", amount);
        }
        this->buffer[pos] = (U8)amount;
        this->buffer[pos + 1] = (U8)(amount >> 8);
        this->buffer[pos + 2] = (U8)(amount >> 16);
        this->buffer[pos + 2] |= ((U8)(amount >> 24)) & 4;
    } else {
        if (amount >= 256*1024 || amount <= -256*1024 ) {
            kpanic("armv8::endIf large if/else blocks not supported: %d", amount);
        }
        this->buffer[pos] = (U8)(amount << 5) | this->buffer[pos];
        this->buffer[pos + 1] = (U8)(amount >> 3);
        this->buffer[pos + 2] = (U8)(amount >> 11);
    }
}

void Armv8btAsm::doIfBitSet(U8 reg, U32 bitPos, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock) {
    if (ifBlock && !elseBlock) {
        // tbz reg, bitPos, label
        U32 pos = bufferPos;
        write8(reg);
        write8(0x0);
        write8(bitPos << 3);
        write8(0x36);
        ifBlock();
        S32 amount = (S32)bufferPos - (S32)pos;
        amount = amount >> 2;
        this->buffer[pos] |= (U8)(amount << 5);
        this->buffer[pos + 1] = (U8)(amount >> 3);
        this->buffer[pos + 2] |= (U8)((amount >> 11) & 0x7);
    } else if (!ifBlock && elseBlock) {
        // tbnz reg, bitPos, label
        U32 pos = bufferPos;
        write8(reg);
        write8(0x0);
        write8(bitPos << 3);
        write8(0x37);
        elseBlock();
        S32 amount = (S32)bufferPos - (S32)pos;
        amount = amount >> 2;
        this->buffer[pos] |= (U8)(amount << 5);
        this->buffer[pos + 1] = (U8)(amount >> 3);
        this->buffer[pos + 2] |= (U8)((amount >> 11) & 0x7);
    } else if (ifBlock && elseBlock) {
        kpanic("Armv8btAsm::doIfBitSet if and else combination not implemented");
    } else {
        kpanic("Armv8btAsm::doIfBitSet if or else block must be supplied");
    }
}

void Armv8btAsm::compareZeroAndBranch(U8 reg, bool isZero, U32 eip) {
    if (!cpu->isBig()) {
        eip &= 0xFFFF;
    }
    if (!isEipInChunk(eip)) {
        doIf(reg, 0, isZero? DO_IF_EQUAL:DO_IF_NOT_EQUAL, [eip, this] {
            loadConst(xBranchEip, eip);
            jmpRegToxBranchEip(false);
            });
    } else {
        write8(reg);
        write8(0);
        write8(0);
        if (isZero) {
            // CBZ
            write8(0x34);
        }
        else {
            // CBNZ
            write8(0x35);
        }
        addTodoLinkJump(eip);
    }
}

void Armv8btAsm::doIf(U8 reg, U32 value, DoIfOperator op, std::function<void(void)> ifBlock, std::function<void(void)> elseBlock, std::function<void(void)> afterCmpBeforeBranchBlock, bool valueIsReg, bool generateCmp) {
    if (generateCmp) {
        if (valueIsReg) {
            cmpRegs32(reg, value);
        } else {
            cmpValue32(reg, value);
        }
    }
    if (afterCmpBeforeBranchBlock) {
        afterCmpBeforeBranchBlock();
    }
    if (!elseBlock) {
        U32 pos = 0;
        if (op == DO_IF_EQUAL) {
            pos = branchNE();
        } else if (op == DO_IF_NOT_EQUAL) {
            pos = branchEQ();
        } else if (op == DO_IF_GREATER_THAN) {
            pos = branchUnsignedLessThanOrEqual();
        } else if (op == DO_IF_LESS_THAN_OR_EQUAL) {
            pos = branchUnsignedGreaterThan();
        } else if (op == DO_IF_LESS_THAN) {
            pos = branchUnsignedGreaterThanOrEqual();
        } else if (op == DO_IF_SIGNED_GREATER_THAN) {
            pos = branchSignedLessThanOrEqual();
        } else if (op == DO_IF_SIGNED_LESS_THAN_OR_EQUAL) {
            pos = branchSignedGreaterThan();
        } else {
            kpanic("Armv8btAsm::doIf unhandled op: %d", op);
        }
        ifBlock();
        writeJumpAmount(pos, this->bufferPos);
    } else {
        U32 pos;
        if (op == DO_IF_EQUAL) {
            pos = branchEQ();
        } else if (op == DO_IF_NOT_EQUAL) {
            pos = branchNE();
        } else if (op == DO_IF_GREATER_THAN) {
            pos = branchUnsignedGreaterThan();
        } else if (op == DO_IF_LESS_THAN) {
            pos = branchUnsignedLessThan();
        } else if (op == DO_IF_LESS_THAN_OR_EQUAL) {
            pos = branchUnsignedLessThanOrEqual();
        } else if (op == DO_IF_SIGNED_GREATER_THAN) {
            pos = branchSignedGreaterThan();
        } else if (op == DO_IF_SIGNED_LESS_THAN_OR_EQUAL) {
            pos = branchSignedLessThanOrEqual();
        } else {
            kpanic("Armv8btAsm::doIf unhandled op: %d", op);
        }
        elseBlock();
        if (ifBlock) {
            U32 pos2 = branch();
            writeJumpAmount(pos, this->bufferPos);
            ifBlock();
            writeJumpAmount(pos2, this->bufferPos);
        } else {
            writeJumpAmount(pos, this->bufferPos);
        }
    }
}

void Armv8btAsm::getDF(U8 dst, U32 width) {
    // :TODO: should I cache this df value like in the normal core, where it gets filled with popf and cld/std
    // if I cache this then we could change this code from 4 instructions to 1 (maybe cache df (1/-1) in top 32-bits of xFLAGS)
    U8 tmpReg = getTmpReg();
    copyBitsFromSourceAtPositionToDest(tmpReg, xFLAGS, 10, 1, false);
    copyBitsFromSourceToDestAtPosition(dst, tmpReg, 31, 1, false);
    if (width == 8) {
        shiftSignedRegRightWithValue32(dst, dst, 31);
        orValue32(dst, dst, 1); // if dst is 0, then we want 1, if dst is -1, then or'ing 1 will make no difference
    } else if (width == 16) {
        shiftSignedRegRightWithValue32(dst, dst, 30);
        orValue32(dst, dst, 2);
    } else if (width == 32) {
        shiftSignedRegRightWithValue32(dst, dst, 29);
        orValue32(dst, dst, 4);
    } else {
        kpanic("Armv8btAsm::getDF bad width %d", width);
    }
    releaseTmpReg(tmpReg);
}

void Armv8btAsm::getVirtualCounter(U8 dst) {
    write8(0x40 | dst);
    write8(0xe0);
    write8(0x3b);
    write8(0xd5);
}

void Armv8btAsm::vReadMemMultiple128(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase) {
    // ld1.16b{ v0, v1, v2, v3 }, [x0], #64
    vMemMultiple(dst, base, numberOfRegs, incrementBase?0xdf:0x40, true);
}

void Armv8btAsm::vWriteMemMultiple128(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase) {
    // st1.16b{ v0, v1, v2, v3 }, [x0], #64
    vMemMultiple(dst, base, numberOfRegs, incrementBase?0x9f:0, true);
}

void Armv8btAsm::vReadMemMultiple64(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase) {
    // ld1.8b{ v0, v1, v2, v3 }, [x0], #32
    vMemMultiple(dst, base, numberOfRegs, incrementBase ? 0xdf : 0x40, false);
}

void Armv8btAsm::vWriteMemMultiple64(U8 dst, U8 base, U32 numberOfRegs, bool incrementBase) {
    // st1.8b{ v0, v1, v2, v3 }, [x0], #32
    vMemMultiple(dst, base, numberOfRegs, incrementBase ? 0x9f : 0, false);
}

void Armv8btAsm::vMemMultiple(U8 dst, U8 base, U32 numberOfRegs, U8 thirdByte, bool is1128) {
    // ld1.16b{ v0, v1, v2, v3 }, [x0], x0
    // st1.16b{ v0, v1, v2, v3 }, [x0], x0
    write8(dst | (U8)(base << 5));
    U8 regCount = 0;
    if (numberOfRegs == 2) {
        regCount = 0xa0;
    } else if (numberOfRegs == 3) {
        regCount = 0x60;
    } else if (numberOfRegs == 4) {
        regCount = 0x20;
    } else {
        kpanic("Armv8btAsm::vMemMultiple128 numberOfRegs needs to be between 1 and 4: %d", numberOfRegs);
    }
    write8(regCount | (U8)(base >> 3));
    write8(thirdByte);
    write8(is1128?0x4c:0x0c);
}

U8 Armv8btAsm::getSSEConstant(U32 c) {
    U8 tmpReg = vGetTmpReg();
    vReadMem128ValueOffset(tmpReg, xCPU, (U32)(offsetof(Armv8btCPU, sseConstants[0])) + c * 16);
    return tmpReg;
}

void Armv8btAsm::vReadMem128RegOffset(U8 dst, U8 base, U8 offsetReg) {
    write8(dst | (U8)(base << 5));
    write8(0x68 | (U8)(base >> 3));
    write8(0xe0 | offsetReg);
    write8(0x3c);
}

void Armv8btAsm::vReadMem128ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset >= 0 && offset <= 65520 && (offset & 0xf) == 0) {
        // ldr q0, [x0, 65520]
        offset = offset >> 4;
        write8(dst | (U8)(base << 5));
        write8(((offset & 0x3F) << 2) | (U8)(base >> 3));
        write8(0xc0 | ((offset >> 6) & 0x1F));
        write8(0x3d);
    } else if (offset > 255 || offset < -256) {        
        U8 tmp = getRegWithConst(offset);
        vReadMem128RegOffset(dst, base, tmp);
        releaseTmpReg(tmp);
    } else {
        // LDR
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8(0xc0 | ((offset >> 4) & 0x1F));
        write8(0x3c);
    }
}

void Armv8btAsm::vWriteMem128RegOffset(U8 dst, U8 base, U8 offsetReg) {
    // str q0, [x0,x0]
    write8(dst | (U8)(base << 5));
    write8(0x68 | (U8)(base >> 3));
    write8(0xa0 | offsetReg);
    write8(0x3c);
}

void Armv8btAsm::vWriteMem128ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset >= 0 && offset <= 65520 && (offset & 0xf) == 0) {
        // ldr q0, [x0, 65520]
        offset = offset >> 4;
        write8(dst | (U8)(base << 5));
        write8(((offset & 0x3F) << 2) | (U8)(base >> 3));
        write8(0x80 | ((offset >> 6) & 0x1F));
        write8(0x3d);
    } else if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        vWriteMem128RegOffset(dst, base, tmp);
        releaseTmpReg(tmp);
    } else {
        // str q0, [x0,0]
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8(0x80 | ((offset >> 4) & 0x1F));
        write8(0x3c);
    }
}

void Armv8btAsm::vWriteMemory128(U8 addressReg, U8 dst, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 128, true);
        vWriteMem128RegOffset(dst, addressReg, memReg);
        releaseHostMem(memReg);
    } else {
        vWriteMem128ValueOffset(dst, addressReg, 0);
    }
}

void Armv8btAsm::vReadMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl) {
    // ldr d0, [x0,x0]
    write8(dst | (U8)(base << 5));
    if (lsl == 0) {
        write8((U8)(base >> 3) | 0x68);
    } else if (lsl == 3) {
        write8((U8)(base >> 3) | 0x78);
    } else {
        kpanic("Armv8btAsm::vReadMem64RegOffset lsl must be 0 or 3: %d", lsl);
    }
    write8(0x60 | offsetReg);
    write8(0xfc);
}

void Armv8btAsm::vWriteMem64RegOffset(U8 dst, U8 base, U8 offsetReg, U32 lsl) {
    // str d0, [x0,x0]
    write8(dst | (U8)(base << 5));
    if (lsl == 0) {
        write8((U8)(base >> 3) | 0x68);
    } else if (lsl == 3) {
        write8((U8)(base >> 3) | 0x78);
    } else {
        kpanic("Armv8btAsm::vWriteMem64RegOffset lsl must be 0 or 3: %d", lsl);
    }
    write8(0x20 | offsetReg);
    write8(0xfc);
}

void Armv8btAsm::vReadMem64ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        vReadMem64RegOffset(dst, base, tmp);
        releaseTmpReg(tmp);
    } else {
        // ldr s0, [x0,1]
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8(0x40 | ((offset >> 4) & 0x1F));
        write8(0xfc);
    }
}

void Armv8btAsm::vWriteMemory64(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 64, true);
        addRegs64(addressReg, addressReg, memReg);
        releaseHostMem(memReg);
    }
    // st1{ v0.d } [0] , [x0]
    write8(dst | (U8)(addressReg << 5));
    write8((U8)(addressReg >> 3) | 0x84);
    write8(0x0);
    write8((index == 1) ? 0x4d : 0x0d);
}

void Armv8btAsm::vReadMemory16(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 16, false);
        addRegs64(addressReg, addressReg, memReg);
        releaseHostMem(memReg);
    }
    // ld1 {v0.h} [1], [x0]
    write8(dst | (U8)(addressReg << 5));
    if (index == 0 || index == 4) {
        write8((U8)(addressReg >> 3) | 0x40);
    } else if (index == 1 || index == 5) {
        write8((U8)(addressReg >> 3) | 0x48);
    } else if (index == 2 || index == 6) {
        write8((U8)(addressReg >> 3) | 0x50);
    } else if (index == 3 || index == 7) {
        write8((U8)(addressReg >> 3) | 0x58);
    }
    write8(0x40);
    write8(index>=4 ? 0x4d : 0x0d);
}

void Armv8btAsm::vWriteMemory16(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 16, true);
        addRegs64(addressReg, addressReg, memReg);
        releaseHostMem(memReg);
    }
    // st1{ v0.h } [1] , [x0]
    write8(dst | (U8)(addressReg << 5));
    if (index == 0 || index == 4) {
        write8((U8)(addressReg >> 3) | 0x40);
    } else if (index == 1 || index == 5) {
        write8((U8)(addressReg >> 3) | 0x48);
    } else if (index == 2 || index == 6) {
        write8((U8)(addressReg >> 3) | 0x50);
    } else if (index == 3 || index == 7) {
        write8((U8)(addressReg >> 3) | 0x58);
    }
    write8(0x0);
    write8(index>=4 ? 0x4d : 0x0d);
}

void Armv8btAsm::vReadMemory32(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 32, false);
        addRegs64(addressReg, addressReg, memReg);
        releaseHostMem(memReg);
    }
    // ld1 {v0.s} [1], [x0]
    write8(dst | (U8)(addressReg << 5));
    write8((U8)(addressReg >> 3) | ((index == 0 || index == 2) ? 0x80 : 0x90));
    write8(0x40);
    write8((index == 2 || index == 3) ? 0x4d : 0x0d);
}

void Armv8btAsm::vReadMem32RegOffset(U8 dst, U8 base, U8 offsetReg) {
    // ldr s0, [x0,x0]
    write8(dst | (U8)(base << 5));
    write8((U8)(base >> 3) | 0x68);
    write8(0x60 | offsetReg);
    write8(0xbc);
}

void Armv8btAsm::vReadMemory32(U8 addressReg, U8 dst, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 32, false);
        vReadMem32RegOffset(dst, addressReg, memReg);
        releaseHostMem(memReg);
    } else {
        vReadMem32ValueOffset(dst, addressReg, 0);
    }
}

void Armv8btAsm::vReadMem32ValueOffset(U8 dst, U8 base, S32 offset) {
    if (offset > 255 || offset < -256) {
        U8 tmp = getRegWithConst(offset);
        vReadMem32RegOffset(dst, base, tmp);
        releaseTmpReg(tmp);
    } else {
        // ldr s0, [x0,1]
        write8(dst | (U8)(base << 5));
        write8(((offset & 0xF) << 4) | (U8)(base >> 3));
        write8(0x40 | ((offset >> 4) & 0x1F));
        write8(0xbc);
    }
}

void Armv8btAsm::vWriteMemory32(U8 addressReg, U8 dst, U32 index, bool addMemOffsetToAddress) {
    if (addMemOffsetToAddress) {
        U8 memReg = getHostMem(addressReg, 32, true);
        addRegs64(addressReg, addressReg, memReg);
        releaseHostMem(memReg);
    }
    // st1{ v0.s } [1] , [x0]
    write8(dst | (U8)(addressReg << 5));
    write8((U8)(addressReg >> 3) | ((index == 0 || index == 2) ? 0x80 : 0x90));
    write8(0x0);
    write8((index == 2 || index == 3) ? 0x4d : 0x0d);
}

void Armv8btAsm::vMov128(U8 dst, U8 src) {
    // mov v0.16b, v1.16b
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x1c);
    write8(src | 0xa0);
    write8(0x4e);
}

void Armv8btAsm::vIns(U8 rd, U8 rn, U8 imm4, U8 imm5) {
    write8(rd | (U8)(rn << 5));
    write8((U8)(rn >> 3) | 0x4 | (imm4 << 3));
    write8(imm5);
    write8(0x6e);
}

void Armv8btAsm::vMov64(U8 dst, U32 dstIndex, U8 src, U32 srcIndex) {
    // mov v0.d[0], v0.d[0]
    vIns(dst, src, (srcIndex << 3), 8 | (dstIndex << 4));
}

void Armv8btAsm::vMov64ToScaler(U8 dst, U8 src, U32 srcIndex) {
    // mov d0, v0.d [0]
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x4);
    write8(0x08 | srcIndex << 4);
    write8(0x5e);
}

void Armv8btAsm::vZeroExtend64To128(U8 dst, U8 src) {
    // mov d0, v0.d [0]
    vMov64ToScaler(dst, src, 0);
}

void Armv8btAsm::vMov32(U8 dst, U32 dstIndex, U8 src, U32 srcIndex) {
    // mov v0.s[1], v0.s[0]
    vIns(dst, src, srcIndex<<2, 4 | (dstIndex << 3));
}

void Armv8btAsm::vMov16(U8 dst, U32 dstIndex, U8 src, U32 srcIndex) {
    // mov v0.h[1], v0.h[0]
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x4 | (srcIndex << 4));
    write8(2 | (dstIndex << 2));
    write8(0x6e);
}

void Armv8btAsm::vMov32ToScaler(U8 dst, U8 src, U32 srcIndex) {
    // mov s0, v0.s [0]
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x4);
    write8(0x04 | srcIndex << 3);
    write8(0x5e);
}

void Armv8btAsm::vLoadConst(U8 dst, U64 value, VectorWidth width) {
    // movi v0.16b, 0
    if (width == B16) {
        write8(dst | (U8)(value << 5));
        write8((U8)((value >> 3) & 0x3) | 0xe4);
        write8((U8)(value >> 5));
        write8(0x4f);
    } else if (width == D2) {
        if (value & 0xFFFFFFFFFFFFFF00) {
            // doesn't have shifting ability for 64-bit
            U8 tmpReg = getTmpReg();
            loadConst(tmpReg, value);
            this->vMovFromGeneralReg64(dst, 0, tmpReg);
            this->vMovFromGeneralReg64(dst, 1, tmpReg);
            releaseTmpReg(tmpReg);
        } else {
            write8(dst | (U8)(value << 5));
            write8((U8)((value >> 3) & 0x3) | 0xe4);
            write8((U8)(value >> 5));
            write8(0x6f);
        }
    } else {
        kpanic("Armv8btAsm::vLoadConst not implemented for width: %d", width);
    }
}

void Armv8btAsm::vMovFromGeneralReg16(U8 dst, U32 dstIndex, U8 src) {
    // ins v0.h[0], w0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x1c);
    write8(0x2 | (dstIndex << 2));
    write8(0x4e);
}


void Armv8btAsm::vMovFromGeneralReg32(U8 dst, U32 dstIndex, U8 src) {
    // ins v0.s[0], w0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x1c);
    write8(0x4 | (dstIndex << 3));
    write8(0x4e);
}

void Armv8btAsm::vMovFromGeneralReg64(U8 dst, U32 dstIndex, U8 src) {
    // ins v0.d[0], w0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x1c);
    write8(0x8 | (dstIndex << 4));
    write8(0x4e);
}

void Armv8btAsm::vMovToGeneralReg32ZeroExtend(U8 dst, U8 src, U32 srcIndex, VectorWidth width) {    
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x3c);
    if (width == B16) {
        write8(0x1 | (srcIndex << 1));
    } else if (width == H8) {        
        write8(0x2 | (srcIndex << 2));
    } else if (width == S4) {
        write8(0x4 | (srcIndex << 3));
    }
    write8(0x0e);
}

void Armv8btAsm::vMovToGeneralReg64(U8 dst, U8 src, U32 srcIndex) {
    // mov x0, v0.d[0]
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x3c);
    write8(0x8 | (srcIndex << 4));
    write8(0x4e);
}

void Armv8btAsm::vExtractVectorFromPair(U8 dst, U8 src1, U8 src2, U32 startIndexOfSrc1) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | (U8)(startIndexOfSrc1 << 3));
    write8(src2);
    write8(0x6e);
}

void Armv8btAsm::vZipFromLow128(U8 dst, U8 src1, U8 src2, VectorWidth width) {    
    write8(dst | (U8)(src1 << 5));
    write8(0x38 | (U8)(src1 >> 3));

    if (width == B16) {
        // zip1 v0.16b, v0.16b, v0.16b
        write8(src2);
        write8(0x4e);
    } else if (width == B8) {
        // zip1 v0.8b, v0.8b, v0.8b
        write8(src2);
        write8(0x0e);
    } else if (width == H4) {
        // zip1 v0.4h, v0.4h, v0.4h
        write8(0x40 | src2);
        write8(0x0e);
    } else if (width == H8) {
        // zip1 v0.8h, v0.8h, v0.8h
        write8(0x40 | src2);
        write8(0x4e);
    } else if (width == S2) {
        // zip1 v0.2s, v0.2s, v0.2s
        write8(0x80 | src2);
        write8(0x0e);
    } else if (width == S4) {
        // zip1 v0.4s, v0.4s, v1.4s
        write8(0x80 | src2);
        write8(0x4e);
    } else if (width == D2) {
        write8(0xc0 | src2);
        write8(0x4e);
    } else {
        kpanic("Armv8btAsm::vZipFromLow128 invalid width: %d", width);
    }    
}

void Armv8btAsm::vZipFromHigh128(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    // zip2 v0.4s, v0.4s, v1.4s
    write8(dst | (U8)(src1 << 5));
    write8(0x78 | (U8)(src1 >> 3));
    U8 type = 0;
    if (width == B16) {
        type = 0;
    } else if (width == H8) {
        type = 0x40;
    } else if (width == S4) {
        type = 0x80;
    } else if (width == D2) {
        type = 0xc0;
    } else {
        kpanic("Armv8btAsm::vZipFromLow128 invalid: %d", width);
    }
    write8(type | src2);
    write8(0x4e);
}

void Armv8btAsm::vUnzipOdds(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x58 | (U8)(src1 >> 3));
    if (width == B16) {
        // UZP2 v0.16B, v0.16B, v0.16B
        write8(src2);
    } else if (width == H8) {
        // UZP2 v0.8H, v0.8H, v0.8H
        write8(0x40 | src2);
    } else if (width == S4) {
        // UZP2 v0.4S, v0.4S, v0.4S
        write8(0x80 | src2);
    } else if (width == D2) {
        // UZP2 v0.2D, v0.2D, v0.2D
        write8(0xC0 | src2);
    } else {
        kpanic("Armv8btAsm::vUnzipOdds invalid: %d", width);
    }
    
    write8(0x4e);
}

void Armv8btAsm::vUnzipEvens(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x18 | (U8)(src1 >> 3));
    if (width == B16) {
        // UZP1 v0.16B, v0.16B, v0.16B
        write8(src2);
    } else if (width == H8) {
        // UZP1 v0.8H, v0.8H, v0.8H
        write8(0x40 | src2);
    } else if (width == S4) {
        // UZP1 v0.4S, v0.4S, v0.4S
        write8(0x80 | src2);
    } else if (width == D2) {
        // UZP1 v0.2D, v0.2D, v0.2D
        write8(0xC0 | src2);
    } else {
        kpanic("Armv8btAsm::vUnzipEvens invalid width: %d", width);
    }

    write8(0x4e);
}

void Armv8btAsm::vTbx(U8 dst, U8 src, U8 srcCount, U8 srcIndex, VectorWidth width) {
    write8(dst | (U8)(src << 5));    
    write8(0x10 | (U8)(src >> 3) | (srcCount-1) << 5);
    write8(srcIndex);
    if (width == B8) {        
        write8(0x0e);
    } else if (width == B16) {
        write8(0x4e);
    } else {
        kpanic("Armv8btAsm::vTbx invalid width: %d", width);
    }        
}

void Armv8btAsm::vCmpGreaterThan(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x34 | (U8)(src1 >> 3));
        if (width == B16) {
            // CMGT v0.16b, v0.16b, v0.16b
            write8(0x20 | src2);
            write8(0x4e);
        } else if (width == B8) {
            // CMGT v0.8b, v0.8b, v0.8b
            write8(0x20 | src2);
            write8(0x0e);
        } else if (width == H8) {
            // CMGT v0.8h, v0.8h, v0.8h
            write8(0x60 | src2);
            write8(0x4e);
        } else if (width == H4) {
            // CMGT v0.4h, v0.4h, v0.4h
            write8(0x60 | src2);
            write8(0x0e);
        } else if (width == S4) {
            // CMGT v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
            write8(0x4e);
        } else if (width == S2) {
            // CMGT v0.2s, v0.2s, v0.2s
            write8(0xa0 | src2);
            write8(0x0e);
        } else if (width == D2) {
            // CMGT v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
            write8(0x4e);
        } else {
            kpanic("Armv8btAsm::vCmpGreaterThan invalid width: %d", width);
        }        
    } else {        
        write8(dst | (U8)(src1 << 5));
        write8(0x34 | (U8)(src1 >> 3));
        U8 type = 0;
        if (width == D_scaler) {
            // CMGT d0, d0, d0
            type = 0xe0;
        } else {
            kpanic("Armv8btAsm::vCmpGreaterThan(scaler) invalid width: %d", width);
        }
        write8(type | src2);
        write8(0x5e);
    }
}

void Armv8btAsm::vCmpEqual(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x8C | (U8)(src1 >> 3));
        U8 type = 0;
        if (width == B16) {
            // CMEQ v0.16b, v0.16b, v0.16b
            type = 0x20;
        } else if (width == H8) {
            // CMEQ v0.8h, v0.8h, v0.8h
            type = 0x60;
        } else if (width == S4) {
            // CMEQ v0.4s, v0.4s, v0.4s
            type = 0xa0;
        } else if (width == D2) {
            // CMEQ v0.2d, v0.2d, v0.2d
            type = 0xe0;
        } else {
            kpanic("Armv8btAsm::vCmpEqual invalid width: %d", width);
        }
        write8(type | src2);
        write8(0x6e);
    } else {
        write8(dst | (U8)(src1 << 5));
        write8(0x8C | (U8)(src1 >> 3));
        U8 type = 0;
        if (width == D_scaler) {
            // CMEQ d0, d0, d0
            type = 0xe0;
        } else {
            kpanic("Armv8btAsm::vCmpEqual(scaler) invalid width: %d", width);
        }
        write8(type | src2);
        write8(0x7e);
    }
}

void Armv8btAsm::vUnsignedMin(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x6c | (U8)(src1 >> 3));
    if (width == B16) {
        // UMIN v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x6e);
    } else if (width == B8) {
        // UMIN v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x2e);
    } else if (width == H8) {
        // UMIN v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x6e);
    } else if (width == H4) {
        // UMIN v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x2e);
    } else if (width == S4) {
        // UMIN v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x6e);
    } else if (width == S2) {
        // UMIN v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vUnsignedMin invalid width: %d", width);
    }
}

void Armv8btAsm::vSignedMin(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x6c | (U8)(src1 >> 3));
    if (width == B16) {
        // SMIN v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x4e);
    } else if (width == B8) {
        // SMIN v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x0e);
    } else if (width == H8) {
        // SMIN v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x4e);
    } else if (width == H4) {
        // SMIN v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x0e);
    } else if (width == S4) {
        // SMIN v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x4e);
    } else if (width == S2) {
        // SMIN v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vSignedMin invalid width: %d", width);
    }
}

void Armv8btAsm::vUnsignedMax(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x64 | (U8)(src1 >> 3));
    if (width == B16) {
        // UMAX v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x6e);
    } else if (width == B8) {
        // UMAX v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x2e);
    } else if (width == H8) {
        // UMAX v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x6e);
    } else if (width == H4) {
        // UMAX v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x2e);
    } else if (width == S4) {
        // UMAX v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x6e);
    } else if (width == S2) {
        // UMAX v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vUnsignedMax invalid width: %d", width);
    }
}

void Armv8btAsm::vSignedMax(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x64 | (U8)(src1 >> 3));
    if (width == B16) {
        // SMAX v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x4e);
    } else if (width == B8) {
        // SMAX v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x0e);
    } else if (width == H8) {
        // SMAX v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x4e);
    } else if (width == H4) {
        // SMAX v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x0e);
    } else if (width == S4) {
        // SMAX v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x4e);
    } else if (width == S2) {
        // SMAX v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vSignedMax invalid width: %d", width);
    }
}

void Armv8btAsm::vUnsignedRoundedAverage(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x14 | (U8)(src1 >> 3));
    if (width == B16) {
        // URHADD v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x6e);
    } else if (width == B8) {
        // URHADD v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x2e);
    } else if (width == H8) {
        // URHADD v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x6e);
    } else if (width == H4) {
        // URHADD v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x2e);
    } else if (width == S4) {
        // URHADD v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x6e);
    } else if (width == S2) {
        // URHADD v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vUnsignedMax invalid width: %d", width);
    }
}

void Armv8btAsm::vSignExtend64To128(U8 dst, U8 src, VectorWidth width) {
    // SXTL v0.2d, v0.2s
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xa4);
    
    if (width == B16) {
        write8(0x08);        
    } else if (width == H8) {
        write8(0x10);
    } else if (width == S4) {
        write8(0x20);
    } else {
        kpanic("Armv8btAsm::vSignExtend64To128 invalid width: %d", width);
    }
    write8(0x0f);
}

void Armv8btAsm::vConvertInt32ToFloat(U8 dst, U8 src, bool isVector) {
    // SCVTF s0, s0
    // SCVTF v0.4s, v0.4s
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xd8);
    write8(0x21);
    write8(isVector ? 0x4e : 0x5e);
}

void Armv8btAsm::vConvertInt64ToDouble(U8 dst, U8 src, bool isVector) {
    // SCVTF d0, d0
    // SCVTF v0.2d, v0.2d
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xd8);
    write8(0x61);
    write8(isVector ? 0x4e : 0x5e);
}

void Armv8btAsm::vConvertDoubleToInt64RoundToZero(U8 dst, U8 src, bool isVector) {
    // FCVTZS d0, d0
    // FCVTZS v0.2d, v0.2d
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xb8);
    write8(0xe1);
    write8(isVector ? 0x4e : 0x5e);
}

void Armv8btAsm::vConvertFloatToInt32RoundToZero(U8 dst, U8 src, bool isVector) {
    // FCVTZS s0, s0
    // FCVTZS v0.4s, v0.4s
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xb8);
    write8(0xa1);
    write8(isVector ? 0x4e : 0x5e);
}

void Armv8btAsm::vConvertDoubleToInt64RoundToNearest(U8 dst, U8 src, bool isVector) {
    // FCVTNS d0, d0
    // FCVTNS v0.2d, v0.2d
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xa8);
    write8(0x61);
    write8(isVector ? 0x4e : 0x5e);
}

void Armv8btAsm::vConvertFloatToInt32RoundToNearest(U8 dst, U8 src, bool isVector) {
    // FCVTNS s0, s0
    // FCVTNS v0.4s, v0.4s
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0xa8);
    write8(0x21);
    write8(isVector?0x4e:0x5e);
}

void Armv8btAsm::vConvertFloatToDouble(U8 dst, U8 src, bool isVector) {
    if (isVector) {
        // FCVTL v0.2d, v0.2s
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0x78);
        write8(0x61);
        write8(0x0e);
    } else {
        // FCVT d0, s0
        write8(dst | (U8)(src << 5));
        write8((U8)(src >> 3) | 0xc0);
        write8(0x22);
        write8(0x1e);
    }
}

void Armv8btAsm::vConvertDoubleToInt64RoundToCurrentMode(U8 dst, U8 src, bool isVector) {
    // :TODO: need to honor current rounding mode
    // quake 2 will crash if this isn't round toward 0 when converting sound sample, it must have set the current rounding mode
    vConvertDoubleToInt64RoundToZero(dst, src, isVector);
}

void Armv8btAsm::vConvertFloatToInt32RoundToCurrentMode(U8 dst, U8 src, bool isVector) {
    // :TODO: need to honor current rounding mode
    vConvertFloatToInt32RoundToZero(dst, src, isVector);
}

void Armv8btAsm::vConvertInt64ToLowerInt32(U8 dst, U8 src) {
    // XTN v0.2s, v0.2d
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x28);
    write8(0xa1);
    write8(0x0e);
}

void Armv8btAsm::vSignedSaturateToSignedNarrowToLowerAndClear(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x48);
    if (width == D2) {
        // SQXTN v0.2s, v0.2d
        write8(0xa1);
        write8(0x0e);
    } else if (width == D_scaler) {
        // SQXTN s0, d0
        write8(0xa1);
        write8(0x5e);
    } else if (width == S4) {
        // SQXTN v0.4h, v0.4s
        write8(0x61);
        write8(0x0e);
    } else if (width == S_scaler) {
        // SQXTN h0, s0
        write8(0x61);
        write8(0x5e);
    } else if (width == H8) {
        // SQXTN v0.8b, v0.8h
        write8(0x21);
        write8(0x0e);
    } else if (width == H_scaler) {
        // SQXTN b0, h0
        write8(0x21);
        write8(0x5e);
    } else {
        kpanic("Armv8btAsm::vSignedSaturateToSignedNarrowToLowerAndClear invalid width: %d", width);
    }
}

void Armv8btAsm::vSignedSaturateToSignedNarrowToUpperAndKeep(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x48);
    if (width == D2) {
        // SQXTN2 v0.4s, v0.2d
        write8(0xa1);
    } else if (width == S4) {
        // SQXTN2 v0.8h, v0.4s
        write8(0x61);
    } else if (width == H8) {
        // SQXTN2 v0.16b, v0.8h
        write8(0x21);
    } else {
        kpanic("Armv8btAsm::vSignedSaturateToSignedNarrowToUpperAndKeep invalid width: %d", width);
    }
    write8(0x4e);
}

void Armv8btAsm::vUnsignedSaturateToUnsignedNarrowToLowerAndClear(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x48);
    if (width == D2 || width == D_scaler) {
        // UQXTN v0.2s, v0.2d
        write8(0xa1);
    } else if (width == S4 || width == S_scaler) {
        // UQXTN v0.4h, v0.4s
        write8(0x61);
    } else if (width == H8 || width == H_scaler) {
        // UQXTN v0.8b, v0.8h
        write8(0x21);
    } else {
        kpanic("Armv8btAsm::vUnsignedSaturateToUnsignedNarrowToLowerAndClear invalid width: %d", width);
    }
    write8(isWidthVector(width) ? 0x2e : 0x7e);
}

void Armv8btAsm::vUnsignedSaturateToUnsignedNarrowToUpperAndKeep(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x48);
    if (width == D2) {
        // UQXTN2 v0.4s, v0.2d
        write8(0xa1);
    } else if (width == S4) {
        // UQXTN2 v0.8h, v0.4s
        write8(0x61);
    } else if (width == H8) {
        // UQXTN2 v0.16b, v0.8h
        write8(0x21);
    } else {
        kpanic("Armv8btAsm::vUnsignedSaturateToUnsignedNarrowToUpperAndKeep invalid width: %d", width);
    }
    write8(0x6e);
}

void Armv8btAsm::vSignedSaturateToUnsignedNarrowToLowerAndClear(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x28);
    if (width == D2 || width == D_scaler) {
        // SQXTUN v0.2s, v0.2d
        write8(0xa1);
    } else if (width == S4 || width == S_scaler) {
        // SQXTUN v0.4h, v0.4s
        write8(0x61);
    } else if (width == H8 || width == H_scaler) {
        // SQXTUN v0.8b, v0.8h
        write8(0x21);
    } else {
        kpanic("Armv8btAsm::vUnsignedSaturateToUnsignedNarrowToLowerAndClear invalid width: %d", width);
    }
    write8(isWidthVector(width) ? 0x2e : 0x7e);
}

void Armv8btAsm::vSignedSaturateToUnsignedNarrowToUpperAndKeep(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3) | 0x28);
    if (width == D2) {
        // SQXTUN2 v0.4s, v0.2d
        write8(0xa1);
    } else if (width == S4) {
        // SQXTUN2 v0.8h, v0.4s
        write8(0x61);
    } else if (width == H8) {
        // SQXTUN2 v0.16b, v0.8h
        write8(0x21);
    } else {
        kpanic("Armv8btAsm::vUnsignedSaturateToUnsignedNarrowToUpperAndKeep invalid width: %d", width);
    }
    write8(0x6e);
}

void Armv8btAsm::vConvertDoubleToGeneralReg32RoundToZero(U8 dst, U8 src) {
    // FCVTZS w0, d0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3));
    write8(0x78);
    write8(0x1e);
}

void Armv8btAsm::vConvertFloatToGeneralReg32RoundToZero(U8 dst, U8 src) {
    // FCVTZS w0, s0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3));
    write8(0x38);
    write8(0x1e);
}

void Armv8btAsm::vConvertDoubleToGeneralReg32RoundToNearest(U8 dst, U8 src) {
    // FCVTNS w0, d0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3));
    write8(0x60);
    write8(0x1e);
}

void Armv8btAsm::vConvertFloatToGeneralReg32RoundToNearest(U8 dst, U8 src) {
    // FCVTNS w0, s0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3));
    write8(0x20);
    write8(0x1e);
}

void Armv8btAsm::vConvertDoubleToGeneralReg32RoundToCurrentMode(U8 dst, U8 src) {
    // :TODO:
    vConvertDoubleToGeneralReg32RoundToNearest(dst, src);
}

void Armv8btAsm::vConvertFloatToGeneralReg32RoundToCurrentMode(U8 dst, U8 src) {
    // :TODO:
    vConvertFloatToGeneralReg32RoundToNearest(dst, src);
}

void Armv8btAsm::vConvertDoubleToFloatRoundToCurrentModeAndKeep(U8 dst, U8 src) {
    // FCVTN v0.2s, v0.2d
    write8(dst | (U8)(src << 5));
    write8(0x68 | (U8)(src >> 3));
    write8(0x61);
    write8(0x0e);
}

void Armv8btAsm::vConvertDoubleToFloatRoundToZeroAndKeep(U8 dst, U8 src) {
    // :TODO: couldn't find a single instruction to round towards zero
    vConvertDoubleToFloatRoundToCurrentModeAndKeep(dst, src);
}

void Armv8btAsm::vDup(U8 dst, U8 src, U8 srcIndex, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0x04 | (U8)(src >> 3));
    if (width == B16) {
        // dup v0.16b, v0.b[0]
        write8(0x01 | (srcIndex << 1));
        write8(0x4e);
    } else if (width == B8) {
        // dup v0.8b, v0.b[0]
        write8(0x01 | (srcIndex << 1));
        write8(0x0e);
    } else if (width == H8) {
        // dup v0.8h, v0.h[0]
        write8(0x02 | (srcIndex << 2));
        write8(0x4e);
    } else if (width == H4) {
        // dup v0.4h, v0.h[0]
        write8(0x02 | (srcIndex << 2));
        write8(0x0e);
    } else if (width == S4) {
        // dup v0.4s, v0.s[0]
        write8(0x04 | (srcIndex << 3));
        write8(0x4e);
    } else if (width == S2) {
        // dup v0.2s, v0.s[0]
        write8(0x04 | (srcIndex << 3));
        write8(0x0e);
    } else if (width == D2) {
        // dup v0.2d, v0.d[0]
        write8(0x08 | (srcIndex<<4));
        write8(0x4e);
    } else {
        kpanic("Armv8btAsm::vDup width must be 1, 2, 4, or 8: %d", width);
    }    
}

void Armv8btAsm::vNeg(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0xb8 | (U8)(src >> 3));
    if (width == B16) {
        // neg v0.16b, v0.16b
        write8(0x20);
        write8(0x6e);
    } else if (width == B8) {
        // neg v0.8b, v0.8b
        write8(0x20);
        write8(0x2e);
    } else if (width == H8) {
        // neg v0.8h, v0.8h
        write8(0x60);
        write8(0x6e);
    } else if (width == H4) {
        // neg v0.4h, v0.4h
        write8(0x60);
        write8(0x2e);
    } else if (width == S4) {
        // neg v0.4s, v0.4s
        write8(0xa0);
        write8(0x6e);
    } else if (width == S2) {
        // neg v0.2s, v0.2s
        write8(0xa0);
        write8(0x2e);
    } else if (width == D2) {
        // neg v0.2d, v0.2d
        write8(0xe0);
        write8(0x6e);
    } else if (width == D_scaler) {
        // neg d0, d0
        write8(0xe0);
        write8(0x7e);
    } else {
        kpanic("Armv8btAsm::vNeg invalid width: %d", width);
    }    
}

void Armv8btAsm::vAddAcrossVectorToScaler(U8 dst, U8 src, VectorWidth width) {    
    write8(dst | (U8)(src << 5));
    write8(0xb8 | (U8)(src >> 3));
    if (width == S4) {
        // addv s0, v0.4s
        write8(0xb1);
        write8(0x4e);
    } else if (width == H4) {
        // addv h0, v0.4h
        write8(0x71);
        write8(0x0e);
    } else if (width == H8) {
        // addv h0, v0.8h
        write8(0x71);
        write8(0x4e);
    } else if (width == B8) {
        // addv b0, v0.8b
        write8(0x31);
        write8(0x0e);
    } else if (width == B16) {
        // addv b0, v0.16b
        write8(0x31);
        write8(0x4e);
    } else {
        kpanic("Armv8btAsm::vAddAcrossVectorToScaler invalid width: %d", width);
    }    
}

void Armv8btAsm::vUnsignedAddPairsLong(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0x28 | (U8)(src >> 3));
    if (width == S4) {
        // uaddlp v0.2d, v0.4s
        write8(0xa0);
        write8(0x6e);
    } else if (width == H4) {
        // uaddlp v0.2s, v0.4h
        write8(0x60);
        write8(0x2e);
    } else if (width == H8) {
        // uaddlp v0.4s, v0.8h
        write8(0x60);
        write8(0x6e);
    } else if (width == B8) {
        // uaddlp v0.4h, v0.8b
        write8(0x20);
        write8(0x2e);
    } else if (width == B16) {
        // uaddlp v0.8h, v0.16b
        write8(0x20);
        write8(0x6e);
    } else {
        kpanic("Armv8btAsm::vUnsignedAddPairsLong invalid width: %d", width);
    }
}

void Armv8btAsm::vUnsignedAddAcrossVectLong(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0x38 | (U8)(src >> 3));
    if (width == S4) {
        // uaddlv d0, v0.4s
        write8(0xb0);
        write8(0x6e);
    } else if (width == H4) {
        // uaddlv s0, v0.4h
        write8(0x70);
        write8(0x2e);
    } else if (width == H8) {
        // uaddlv s0, v0.8h
        write8(0x70);
        write8(0x6e);
    } else if (width == B8) {
        // uaddlv h0, v0.8b
        write8(0x30);
        write8(0x2e);
    } else if (width == B16) {
        // addv b0, v0.16b
        write8(0x30);
        write8(0x6e);
    } else {
        kpanic("Armv8btAsm::vUnsignedAddAcrossVectLong invalid width: %d", width);
    }
}

void Armv8btAsm::vAddPairs(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xbc | (U8)(src1 >> 3));
    if (width == B16) {
        // addp v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x4e);
    } else if (width == B8) {
        // addp v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x0e);
    } else if (width == H8) {
        // addp v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x4e);
    } else if (width == H4) {
        // addp v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x0e);
    } else if (width == S4) {
        // addp v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x4e);
    } else if (width == S2) {
        // addp v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vAddPairs invalid width: %d", width);
    }
}

void Armv8btAsm::vUnsignedAbsoluteDifference(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x74 | (U8)(src1 >> 3));
    if (width == B16) {
        // uabd v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x6e);
    } else if (width == B8) {
        // uabd v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x2e);
    } else if (width == H8) {
        // uabd v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x6e);
    } else if (width == H4) {
        // uabd v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x2e);
    } else if (width == S4) {
        // uabd v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x6e);
    } else if (width == S2) {
        // uabd v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vUnsignedAbsoluteDifference invalid width: %d", width);
    }
}

void Armv8btAsm::vMul(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x9c | (U8)(src1 >> 3));
    if (width == B16) {
        // mul v0.16b, v0.16b, v0.16b
        write8(0x20 | src2);
        write8(0x4e);
    } else if (width == B8) {
        // mul v0.8b, v0.8b, v0.8b
        write8(0x20 | src2);
        write8(0x0e);
    } else if (width == H8) {
        // mul v0.8h, v0.8h, v0.8h
        write8(0x60 | src2);
        write8(0x4e);
    } else if (width == H4) {
        // mul v0.4h, v0.4h, v0.4h
        write8(0x60 | src2);
        write8(0x0e);
    } else if (width == S4) {
        // mul v0.4s, v0.4s, v0.4s
        write8(0xa0 | src2);
        write8(0x4e);
    } else if (width == S2) {
        // mul v0.2s, v0.2s, v0.2s
        write8(0xa0 | src2);
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vMul invalid width: %d", width);
    }    
}

void Armv8btAsm::vUnsignedMulLongLower(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xc0 | (U8)(src1 >> 3));
    if (width == B8) {
        // UMULL v0.8h, v0.8b, v0.8b
        write8(0x20 | src2);
    } else if (width == H4) {
        // UMULL v0.4s, v0.4h, v0.4h
        write8(0x60 | src2);        
    } else if (width == S2) {
        // UMULL v0.2d, v0.2s, v0.2s
        write8(0xa0 | src2);
    } else {
        kpanic("Armv8btAsm::vUnsignedMulLongLower invalid width: %d", width);
    }
    write8(0x2e);
}

void Armv8btAsm::vUnsignedMulLongUpper(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xc0 | (U8)(src1 >> 3));
    if (width == B16) {
        // UMULL2 v0.8h, v0.16b, v0.16b
        write8(0x20 | src2);
    } else if (width == H8) {
        // UMULL2 v0.4s, v0.8h, v0.8h
        write8(0x60 | src2);
    } else if (width == S4) {
        // UMULL2 v0.2d, v0.4s, v0.4s
        write8(0xa0 | src2);
    } else {
        kpanic("Armv8btAsm::vUnsignedMulLongLower invalid width: %d", width);
    }
    write8(0x6e);
}

void Armv8btAsm::vSignedMulLongLower(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xc0 | (U8)(src1 >> 3));
    if (width == B8) {
        // SMULL v0.8h, v0.8b, v0.8b
        write8(0x20 | src2);
    } else if (width == H4) {
        // SMULL v0.4s, v0.4h, v0.4h
        write8(0x60 | src2);
    } else if (width == S2) {
        // SMULL v0.2d, v0.2s, v0.2s
        write8(0xa0 | src2);
    } else {
        kpanic("Armv8btAsm::vSignedMulLongLower invalid width: %d", width);
    }
    write8(0x0e);
}

void Armv8btAsm::vSignedMulLongUpper(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xc0 | (U8)(src1 >> 3));
    if (width == B16) {
        // SMULL2 v0.8h, v0.16b, v0.16b
        write8(0x20 | src2);
    } else if (width == H8) {
        // SMULL2 v0.4s, v0.8h, v0.8h
        write8(0x60 | src2);
    } else if (width == S4) {
        // SMULL2 v0.2d, v0.4s, v0.4s
        write8(0xa0 | src2);
    } else {
        kpanic("Armv8btAsm::vSignedMulLongUpper invalid width: %d", width);
    }
    write8(0x4e);
}

void Armv8btAsm::vUnsignedSaturatingAdd(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x0c | (U8)(src1 >> 3));
        if (width == B16) {
            // uqadd v0.16b, v0.16b, v1.16b
            write8(0x20 | src2);
            write8(0x6e);
        } else if (width == B8) {
            // uqadd v0.8b, v0.8b, v1.8b
            write8(0x20 | src2);
            write8(0x2e);
        } else if (width == H8) {
            // uqadd v0.8h, v0.8h, v0.8h
            write8(0x60 | src2);
            write8(0x6e);
        } else if (width == H4) {
            // uqadd v0.4h, v0.4h, v0.4h
            write8(0x60 | src2);
            write8(0x2e);
        } else if (width == S4) {
            // uqadd v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
            write8(0x6e);
        } else if (width == D2) {
            // uqadd v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
            write8(0x6e);
        } else {
            kpanic("Armv8btAsm::vUnsignedSaturatingAdd invalid width: %d", width);
        }
    } else {
        // uqadd d0, d0, d0
        if (width != D_scaler) {
            kpanic("Armv8btAsm::vUnsignedSaturatingAdd invalid width: %d", width);
        }
        write8(dst | (U8)(src1 << 5));
        write8(0x2c | (U8)(src1 >> 3));
        write8(0xe0 | src2);
        write8(0x7e);
    }
}

void Armv8btAsm::vSignedSaturatingAdd(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x0c | (U8)(src1 >> 3));
        if (width == B16) {
            // sqadd v0.16b, v0.16b, v1.16b
            write8(0x20 | src2);
            write8(0x4e);
        } else if (width == B8) {
            // sqadd v0.8b, v0.8b, v1.8b
            write8(0x20 | src2);
            write8(0x0e);
        } else if (width == H8) {
            // sqadd v0.8h, v0.8h, v0.8h
            write8(0x60 | src2);
            write8(0x4e);
        } else if (width == H4) {
            // sqadd v0.4h, v0.4h, v0.4h
            write8(0x60 | src2);
            write8(0x0e);
        } else if (width == S4) {
            // sqadd v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
            write8(0x4e);
        } else if (width == D2) {
            // sqadd v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
            write8(0x4e);
        } else {
            kpanic("Armv8btAsm::vSignedSaturatingAdd invalid width: %d", width);
        }
    } else {
        // sqadd d0, d0, d0
        if (width != D_scaler) {
            kpanic("Armv8btAsm::vSignedSaturatingAdd invalid width: %d", width);
        }
        write8(dst | (U8)(src1 << 5));
        write8(0x2c | (U8)(src1 >> 3));
        write8(0xe0 | src2);
        write8(0x5e);
    }
}

void Armv8btAsm::vAdd(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x84 | (U8)(src1 >> 3));
        if (width == B16) {
            // add v0.16b, v0.16b, v1.16b
            write8(0x20 | src2);
            write8(0x4e);
        } else if (width == B8) {
            // add v0.8b, v0.8b, v1.8b
            write8(0x20 | src2);
            write8(0x0e);
        } else if (width == H8) {
            // add v0.8h, v0.8h, v1.8h
            write8(0x60 | src2);
            write8(0x4e);
        } else if (width == H4) {
            // add v0.4h, v0.4h, v1.4h
            write8(0x60 | src2);
            write8(0x0e);
        } else if (width == S4) {
            // add v0.4s, v0.4s, v1.4s
            write8(0xa0 | src2);
            write8(0x4e);
        } else if (width == S2) {
            // add v0.2s, v0.2s, v1.2s
            write8(0xa0 | src2);
            write8(0x0e);
        } else if (width == D2) {
            write8(0xe0 | src2);
            write8(0x4e);
        } else {
            kpanic("Armv8btAsm::vAdd invalid width: %d", width);
        }        
    } else {
        if (width != D_scaler) {
            kpanic("Armv8btAsm::vAdd invalid width: %d", width);
        }
        write8(dst | (U8)(src1 << 5));
        write8(0x84 | (U8)(src1 >> 3));
        write8(0xe0 | src2);
        write8(0x5e);
    }
}

void Armv8btAsm::vUnsignedSaturatingSub(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x2c | (U8)(src1 >> 3));
        if (width == B16) {
            // uqsub v0.16b, v0.16b, v1.16b
            write8(0x20 | src2);
            write8(0x6e);
        } else if (width == B8) {
            // uqsub v0.8b, v0.8b, v1.8b
            write8(0x20 | src2);
            write8(0x2e);
        } else if (width == H8) {
            // uqsub v0.8h, v0.8h, v0.8h
            write8(0x60 | src2);
            write8(0x6e);
        } else if (width == H4) {
            // uqsub v0.4h, v0.4h, v0.4h
            write8(0x60 | src2);
            write8(0x2e);
        } else if (width == S4) {
            // uqsub v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
            write8(0x6e);
        } else if (width == D2) {
            // uqsub v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
            write8(0x6e);
        } else {
            kpanic("Armv8btAsm::vUnsignedSaturatingSub invalid width: %d", width);
        }        
    } else {
        // uqsub d0, d0, d0
        if (width != D_scaler) {
            kpanic("Armv8btAsm::vUnsignedSaturatingSub invalid width: %d", width);
        }
        write8(dst | (U8)(src1 << 5));
        write8(0x2c | (U8)(src1 >> 3));
        write8(0xe0 | src2);
        write8(0x7e);
    }
}

void Armv8btAsm::vSignedSaturatingSub(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x2c | (U8)(src1 >> 3));
        if (width == B16) {
            // sqsub v0.16b, v0.16b, v1.16b
            write8(0x20 | src2);
            write8(0x4e);
        } else if (width == B8) {
            // sqsub v0.8b, v0.8b, v1.8b
            write8(0x20 | src2);
            write8(0x0e);
        } else if (width == H8) {
            // sqsub v0.8h, v0.8h, v0.8h
            write8(0x60 | src2);
            write8(0x4e);
        } else if (width == H4) {
            // sqsub v0.4h, v0.4h, v0.4h
            write8(0x60 | src2);
            write8(0x0e);
        } else if (width == S4) {
            // sqsub v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
            write8(0x4e);
        } else if (width == D2) {
            // sqsub v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
            write8(0x4e);
        } else {
            kpanic("Armv8btAsm::vSignedSaturatingSub invalid width: %d", width);
        }
    } else {
        // sqsub d0, d0, d0
        if (width != D_scaler) {
            kpanic("Armv8btAsm::vSignedSaturatingSub invalid width: %d", width);
        }
        write8(dst | (U8)(src1 << 5));
        write8(0x2c | (U8)(src1 >> 3));
        write8(0xe0 | src2);
        write8(0x5e);
    }
}

void Armv8btAsm::vSub(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0x84 | (U8)(src1 >> 3));
        if (width == B8) {
            // sub v0.8b, v0.8b, v0.8b
            write8(0x20 | src2);
            write8(0x2e);
        } else if (width == B16) {
            // sub v0.16b, v0.16b, v0.16b
            write8(0x20 | src2);
            write8(0x6e);
        } else if (width == H4) {
            // sub v0.4h, v0.4h, v0.4h
            write8(0x60 | src2);
            write8(0x2e);
        } else if (width == H8) {
            // sub v0.8h, v0.8h, v0.8h
            write8(0x60 | src2);
            write8(0x6e);
        } else if (width == S2) {
            // sub sub v0.2s, v0.2s, v0.2s
            write8(0xa0 | src2);
            write8(0x2e);
        } else if (width == S4) {
            // sub v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
            write8(0x6e);
        } else if (width == D2) {
            // sub v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
            write8(0x6e);
        } else {
            kpanic("Armv8btAsm::vSub invalid width: %d", width);
        }        
    } else {
        // sub d0, d0, d0
        if (width != D_scaler) {
            kpanic("Armv8btAsm::vSub invalid width: %d", width);
        }
        write8(dst | (U8)(src1 << 5));
        write8(0x84 | (U8)(src1 >> 3));
        write8(0xe0 | src2);
        write8(0x7e);
    }
}

void Armv8btAsm::vOr(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    // orr v0.16b, v0.16b, v0.16b
    write8(dst | (U8)(src1 << 5));
    write8(0x1c | (U8)(src1 >> 3));
    write8(0xa0 | src2);   
    if (width == B16) {
        // orr v0.16b, v0.16b, v0.16b
        write8(0x4e);
    } else if (width == B8) {
        // orr v0.8b, v0.8b, v0.8b
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vAnd invalid width: %d", width);
    }
}

void Armv8btAsm::vXor(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0x1c | (U8)(src1 >> 3));
    write8(0x20 | src2);
    if (width == B16) {
        // EOR v0.16b, v0.16b, v0.16b
        write8(0x6e);
    } else if (width == B8) {
        // EOR v0.8b, v0.8b, v0.8b
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vAnd invalid width: %d", width);
    }
}

void Armv8btAsm::vAnd(U8 dst, U8 src1, U8 src2, VectorWidth width) {    
    write8(dst | (U8)(src1 << 5));
    write8(0x1c | (U8)(src1 >> 3));
    write8(0x20 | src2);
    if (width == B16) {
        // and v0.16b, v0.16b, v0.16b
        write8(0x4e);
    } else if (width == B8) {
        // and v0.8b, v0.8b, v0.8b
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vAnd invalid width: %d", width);
    }
}

void Armv8btAsm::vAndNot(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x1c);
    write8(src2 | 0x60);
    if (width == B16) {
        // bic v0.16b, v0.16b, v0.16b
        write8(0x4e);
    } else if (width == B8) {
        // bic v0.8b, v0.8b, v0.8b
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vAndNot invalid width: %d", width);
    }
}

void Armv8btAsm::vNot(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0x58 | (U8)(src >> 3));
    write8(0x20);
    if (width == B16) {
        // not v0.16b, v0.16b
        write8(0x6e);
    } else if (width == B8) {
        // not v0.8b, v0.8b
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vNot invalid width: %d", width);
    }
}

void Armv8btAsm::vSignedShiftRightValue(U8 dst, U8 src, U8 amount, VectorWidth width) {    
    if (amount == 0) {
        kpanic("Armv8btAsm::vSignedShiftRight amount cannot be 0");
    }

    write8(dst | (U8)(src << 5));
    write8(0x04 | (U8)(src >> 3));
    if (width == D2) {
        write8(0x40 | (U8)(64 - amount));
        write8(0x4f);
    } else if (width == S2) {
        // SSHR v0.2s, v0.2s, 1
        write8(0x20 | (U8)(32 - amount));
        write8(0x0f);
    } else if (width == S4) {
        // SSHR v0.4s, v0.4s, 1
        write8(0x20 | (U8)(32 - amount));
        write8(0x4f);
    } else if (width == H4) {
        // SSHR v0.4h, v0.4h, 1
        write8(0x10 | (U8)(16 - amount));
        write8(0x0f);
    } else if (width == H8) {
        // SSHR v0.8h, v0.8h, 1
        write8(0x10 | (U8)(16 - amount));
        write8(0x4f);
    } else if (width == B8) {
        // SSHR v0.8b, v0.8b, 1
        write8(0x08 | (U8)(8 - amount));
        write8(0x0f);
    } else if (width == B16) {
        // SSHR v0.16b, v0.16b, 1
        write8(0x08 | (U8)(8 - amount));
        write8(0x4f);
    } else {
        kpanic("Armv8btAsm::vSignedShiftRightValue invalid width: %d", width);
    }    
}

void Armv8btAsm::vShiftLeftValue(U8 dst, U8 src, U8 amount, VectorWidth width) {
    // SHL v0.16b, v0.16b, 1
    if (amount == 0) {
        kpanic("Armv8btAsm::vShiftLeftValue amount cannot be 0");
    }

    write8(dst | (U8)(src << 5));
    write8(0x54 | (U8)(src >> 3));
    if (width == D2) {
        write8(0x40 | (U8)(amount));
    } else if (width == S4) {
        write8(0x20 | (U8)(amount));
    } else if (width == H8) {
        write8(0x10 | (U8)(amount));
    } else if (width == B16) {
        write8(0x08 | (U8)(amount));
    } else {
        kpanic("Armv8btAsm::vShiftLeftValue invalid width: %d", width);
    }
    write8(0x4f);
}

void Armv8btAsm::vShiftRightValue(U8 dst, U8 src, U8 amount, VectorWidth width) {
    // USHR v0.16b, v0.16b, 1
    if (amount == 0) {
        kpanic("Armv8btAsm::vShiftRightValue amount cannot be 0");
    }

    write8(dst | (U8)(src << 5));
    write8(0x04 | (U8)(src >> 3));
    if (width == D2) {
        write8(0x40 | (U8)(64 - amount));
    } else if (width == S4) {
        write8(0x20 | (U8)(32 - amount));
    } else if (width == H8) {
        write8(0x10 | (U8)(16 - amount));
    } else if (width == B16) {
        write8(0x08 | (U8)(8 - amount));
    } else {
        kpanic("Armv8btAsm::vShiftRightValue invalid width: %d", width);
    }
    write8(0x6f);
}

void Armv8btAsm::vShiftWithReg(U8 dst, U8 src, U8 srcAmounts, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0x44 | (U8)(src >> 3));
    if (width == D2) {
        // USHL v0.2d, v0.2d, v1.2d
        write8(0xe0 | srcAmounts);
        write8(0x6e);
    } else if (width == D_scaler) {
        // USHL d0, d0, d0
        write8(0xe0 | srcAmounts);
        write8(0x7e);
    } else if (width == S4) {
        // USHL v0.4s, v0.4s, v1.4s
        write8(0xa0 | srcAmounts);
        write8(0x6e);
    } else if (width == S2) {
        // USHL v0.2s, v0.2s, v1.2s
        write8(0xa0 | srcAmounts);
        write8(0x2e);
    } else if (width == H8) {
        // USHL v0.8h, v0.8h, v1.8h
        write8(0x60 | srcAmounts);
        write8(0x6e);
    } else if (width == H4) {
        // USHL v0.4h, v0.4h, v1.4h
        write8(0x60 | srcAmounts);
        write8(0x2e);
    } else if (width == B16) {
        // USHL v0.16b, v0.16b, v1.16b
        write8(0x20 | srcAmounts);
        write8(0x6e);
    } else if (width == B8) {
        // USHL v0.8b, v0.8b, v1.8b
        write8(0x20 | srcAmounts);
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vShiftWithReg invalid width: %d", width);
    }    
}

void Armv8btAsm::vSignedShiftWithReg(U8 dst, U8 src, U8 srcAmounts, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0x44 | (U8)(src >> 3));
    if (width == D2) {
        // SSHL v0.2d, v0.2d, v1.2d
        write8(0xe0 | srcAmounts);
        write8(0x4e);
    } else if (width == S4) {
        // SSHL v0.4s, v0.4s, v1.4s
        write8(0xa0 | srcAmounts);
        write8(0x4e);
    } else if (width == S2) {
        // SSHL v0.2s, v0.2s, v1.2s
        write8(0xa0 | srcAmounts);
        write8(0x0e);
    } else if (width == H8) {
        // SSHL v0.8h, v0.8h, v1.8h
        write8(0x60 | srcAmounts);
        write8(0x4e);
    } else if (width == H4) {
        // SSHL v0.4h, v0.4h, v1.4h
        write8(0x60 | srcAmounts);
        write8(0x0e);
    } else if (width == B16) {
        // SSHL v0.16b, v0.16b, v1.16b
        write8(0x20 | srcAmounts);
        write8(0x4e);
    } else if (width == B8) {
        // SSHL v0.8b, v0.8b, v1.8b
        write8(0x20 | srcAmounts);
        write8(0x0e);
    } else {
        kpanic("Armv8btAsm::vSignedShiftWithReg invalid width: %d", width);
    }    
}

void Armv8btAsm::vShiftRightValueAndNarrow(U8 dst, U8 src, U8 amount, VectorWidth width) {
    if (amount == 0) {
        kpanic("Armv8btAsm::vShiftRightValueAndNarrow amount cannot be 0");
    }

    write8(dst | (U8)(src << 5));
    write8(0x84 | (U8)(src >> 3));
    if (width == D2) {
        // SHRN v0.2s, v0.2d, 1
        write8(0x20 | (U8)(32 - amount));
    } else if (width == S4) {
        // SHRN v0.4h, v0.4s, 1
        write8(0x10 | (U8)(16 - amount));
    } else if (width == H8) {
        // SHRN v0.8b, v0.8h, 1
        write8(0x08 | (U8)(8 - amount));
    } else {
        kpanic("Armv8btAsm::vShiftRightValueAndNarrow invalid width: %d", width);
    }
    write8(0x0f);
}

void Armv8btAsm::vSelectBit(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    // bsl v0.16b, v0.16b, v0.16b
    write8(dst | (U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x1c);
    write8(src2 | 0x60);
    if (width == B16) {
        write8(0x6e);
    } else if (width == B8) {
        write8(0x2e);
    } else {
        kpanic("Armv8btAsm::vShiftRightValueAndNarrow invalid width: %d", width);
    }
}

void Armv8btAsm::fMovFromGeneralRegister64(U8 dst, U8 src) {
    // fmov d0, x0
    write8(dst | (U8)(src << 5));
    write8((U8)(src >> 3));
    write8(0x67);
    write8(0x9e);
}

void Armv8btAsm::fCmp64(U8 src1, U8 src2) {
    // fcmp d0, d0
    write8((U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x20);
    write8(0x60 | src2);
    write8(0x1e);
}

void Armv8btAsm::fCmp32(U8 src1, U8 src2) {
    // fcmp s0, s0
    write8((U8)(src1 << 5));
    write8((U8)(src1 >> 3) | 0x20);
    write8(0x20 | src2);
    write8(0x1e);
}

void Armv8btAsm::fCmpEqual(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xe4 | (U8)(src1 >> 3));
    if (width == D2 || width == D_scaler) {
        write8(0x60 | src2);
    } else if (width == S4 || width == S_scaler) {
        write8(0x20 | src2);
    } else {
        kpanic("Armv8btAsm::fCmpEqual invalid width: %d", width);
    }
    // FCMEQ v0.2d, v0.2d, v0.2d
    // FCMEQ d0, d0, d0        
    write8(isWidthVector(width) ? 0x4e : 0x5e);
}

void Armv8btAsm::fCmpGreaterThanOrEqual(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xe4 | (U8)(src1 >> 3));
    if (width == D2 || width == D_scaler) {
        write8(0x60 | src2);
    } else if (width == S4 || width == S_scaler) {
        write8(0x20 | src2);
    } else {
        kpanic("Armv8btAsm::fCmpGreaterThanOrEqual invalid width: %d", width);
    }
    // FCMGE v0.2d, v0.2d, v0.2d
    // FCMGE d0, d0, d0        
    write8(isWidthVector(width) ?0x6e:0x7e);
}

void Armv8btAsm::fCmpGreaterThan(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8(0xe4 | (U8)(src1 >> 3));
    if (width == D2 || width == D_scaler) {
        write8(0xe0 | src2);
    } else if (width == S4 || width == S_scaler) {
        write8(0xa0 | src2);
    } else {
        kpanic("Armv8btAsm::fCmpGreaterThan invalid width: %d", width);
    }
    // FCMGT v0.2d, v0.2d, v0.2d
    // FCMGT d0, d0, d0        
    write8(isWidthVector(width)?0x6e:0x7e);
}

void Armv8btAsm::fCmpLessThanOrEqual(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src2 << 5));
    write8(0xe4 | (U8)(src2 >> 3));
    if (width == D2 || width == D_scaler) {
        write8(0x60 | src1);
    } else if (width == S4 || width == S_scaler) {
        write8(0x20 | src1);
    } else {
        kpanic("Armv8btAsm::fCmpLessThanOrEqual invalid width: %d", width);
    }
    // FCMLE v0.2d, v0.2d, v0.2d
    // FCMLE d0, d0, d0        
    write8(isWidthVector(width) ? 0x6e : 0x7e);
}

void Armv8btAsm::fCmpLessThan(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src2 << 5));
    write8(0xe4 | (U8)(src2 >> 3));
    if (width == D2 || width == D_scaler) {
        write8(0xe0 | src1);
    } else if (width == S4 || width == S_scaler) {
        write8(0xa0 | src1);
    } else {
        kpanic("Armv8btAsm::fCmpLessThan invalid width: %d", width);
    }
    // FCMLT v0.2d, v0.2d, v0.2d
    // FCMLT d0, d0, d0        
    write8(isWidthVector(width) ? 0x6e : 0x7e);
}

void Armv8btAsm::fSqrt(U8 dst, U8 src, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src << 5));
        write8(0xf8 | (U8)(src >> 3));
        if (width == S4) {
            // FSQRT v0.4s, v0.4s       
            write8(0xa1);
        } else if (width == D2) {
            // FSQRT v0.2d, v0.2d
            write8(0xe1);
        } else {
            kpanic("Armv8btAsm::fSqrt invalid width: %d", width);
        }
        write8(0x6e);
    } else {
        write8(dst | (U8)(src << 5));
        write8(0xc0 | (U8)(src >> 3));
        if (width == S_scaler) {
            // FSQRT s0, s0
            write8(0x21);
        } else if (width == D_scaler) {
            // FSQRT d0, d0
            write8(0x61);
        } else {
            kpanic("Armv8btAsm::fSqrt invalid width: %d", width);
        }
        write8(0x1e);
    }
}

void Armv8btAsm::fRsqrt(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0xd8 | (U8)(src >> 3));
    if (width == S4 || width == S_scaler) {
        // FRSQRTE s0, s0
        write8(0xa1);
    } else if (width == D2 || width == D_scaler) {
        // FRSQRTE d0, d0
        write8(0xe1);
    } else {
        kpanic("Armv8btAsm::fRsqrt invalid width: %d", width);
    }
    write8(isWidthVector(width) ?0x6e:0x7e);
}

void Armv8btAsm::fAbs(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));    
    if (width == S_scaler) {
        // FABS s0, s0
        write8(0xc0 | (U8)(src >> 3));
        write8(0x20);
    } else if (width == S4) {
        // FABS v0.4s, v0.4s
        write8(0xf8 | (U8)(src >> 3));
        write8(0xa0);
    } else if (width == D2) {
        // FABS v0.2d, v0.2d
        write8(0xf8 | (U8)(src >> 3));        
        write8(0xe0);
    } else if (width == D_scaler) {
        // FABS d0, d0
        write8(0xc0 | (U8)(src >> 3));
        write8(0x60);
    } else {
        kpanic("Armv8btAsm::fAbs invalid width: %d", width);
    }
    write8(isWidthVector(width) ? 0x4e : 0x1e);
}

void Armv8btAsm::fNeg(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    if (width == S_scaler) {
        // FNEG s0, s0
        write8(0x40 | (U8)(src >> 3));
        write8(0x21);
    } else if (width == S4) {
        // FNEG v0.4s, v0.4s
        write8(0xf8 | (U8)(src >> 3));
        write8(0xa0);
    } else if (width == D2) {
        // FNEG v0.2d, v0.2d
        write8(0xf8 | (U8)(src >> 3));
        write8(0xe0);
    } else if (width == D_scaler) {
        // FNEG d0, d0
        write8(0x40 | (U8)(src >> 3));
        write8(0x61);
    } else {
        kpanic("Armv8btAsm::fNeg invalid width: %d", width);
    }
    write8(isWidthVector(width) ? 0x6e : 0x1e);
}

void Armv8btAsm::fReciprocal(U8 dst, U8 src, VectorWidth width) {
    write8(dst | (U8)(src << 5));
    write8(0xd8 | (U8)(src >> 3));
    if (width == S4 || width == S_scaler) {
        // FRECPE s0, s0
        write8(0xa1);
    } else if (width == D2 || width == D_scaler) {
        // FRECPE d0, d0
        write8(0xe1);
    } else {
        kpanic("Armv8btAsm::fReciprocal invalid width: %d", width);
    }
    write8(isWidthVector(width) ?0x4e:0x5e);
}

void Armv8btAsm::fAdd(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8((isWidthVector(width) ?0xd4:0x28) | (U8)(src1 >> 3));
    if (width == S4 || width == S_scaler) {
        write8(0x20 | src2);
    } else if (width == D2 || width == D_scaler) {
        write8(0x60 | src2);
    } else {
        kpanic("Armv8btAsm::fAdd invalid: %d", width);
    }
    write8(isWidthVector(width) ?0x4e:0x1e);
}

void Armv8btAsm::fSub(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0xd4 | (U8)(src1 >> 3));
        if (width == S4) {
            // fsub v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
        } else if (width == D2) {
            // fsub v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
        } else {
            kpanic("Armv8btAsm::fSub invalid width: %d", width);
        }
        write8(0x4e);
    } else {
        write8(dst | (U8)(src1 << 5));
        write8(0x38 | (U8)(src1 >> 3));
        if (width == S_scaler) {
            // fsub s0, s0, s0
            write8(0x20 | src2);
        } else if (width == D_scaler) {
            // fsub d0, d0, d0
            write8(0x60 | src2);
        } else {
            kpanic("Armv8btAsm::fSub invalid width: %d", width);
        }
        write8(0x1e);
    }
}

void Armv8btAsm::fMul(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8((isWidthVector(width) ?0xdc:0x08) | (U8)(src1 >> 3));
    if (width == S4 || width == S_scaler) {
        // fmul s0, s0, s0
        write8(0x20 | src2);
    } else if (width == D2 || width == D_scaler) {
        // fmul d0, d0, d0
        write8(0x60 | src2);
    } else {
        kpanic("Armv8btAsm::fMul invalid width: %d", width);
    }
    write8(isWidthVector(width) ? 0x6e:0x1e);
}

void Armv8btAsm::fDiv(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    write8(dst | (U8)(src1 << 5));
    write8((isWidthVector(width) ?0xfc:0x18) | (U8)(src1 >> 3));
    if (width == S4 || width == S_scaler) {
        // fdiv s0, s0, s0
        write8(0x20 | src2);
    } else if (width == D2 || width == D_scaler) {
        // fdiv d0, d0, d0
        write8(0x60 | src2);
    } else {
        kpanic("Armv8btAsm::fdiv invalid width: %d", width);
    }
    write8(isWidthVector(width) ?0x6e:0x1e);
}

void Armv8btAsm::fMin(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0xc4 | (U8)(src1 >> 3));
        if (width == D2) {
            // FMINNM v0.2d, v0.2d, v0.2d
            write8(0xe0 | src2);
        } else if (width == S4) {
            // FMINNM v0.4s, v0.4s, v0.4s
            write8(0xa0 | src2);
        } else {
            kpanic("Armv8btAsm::fMin invalid width: %d", width);
        }
        write8(0x4e);
    } else {
        write8(dst | (U8)(src1 << 5));
        write8(0x78 | (U8)(src1 >> 3));
        if (width == D_scaler) {
            // FMINNM d0, d0, d0
            write8(0x60 | src2);
        } else if (width == S_scaler) {
            // FMINNM s0, s0, s0
            write8(0x20 | src2);
        } else {
            kpanic("Armv8btAsm::fMin invalid width: %d", width);
        }
        write8(0x1e);
    }
}

void Armv8btAsm::fMax(U8 dst, U8 src1, U8 src2, VectorWidth width) {
    if (isWidthVector(width)) {
        write8(dst | (U8)(src1 << 5));
        write8(0xc4 | (U8)(src1 >> 3));
        if (width == D2) {
            // FMAXNM v0.2d, v0.2d, v0.2d
            write8(0x60 | src2);
        } else if (width == S4) {
            // FMAXNM v0.4s, v0.4s, v0.4s
            write8(0x20 | src2);
        } else {
            kpanic("Armv8btAsm::fMax invalid width: %d", width);
        }
        write8(0x4e);
    } else {
        write8(dst | (U8)(src1 << 5));
        write8(0x68 | (U8)(src1 >> 3));
        if (width == D_scaler) {
            // FMAXNM d0, d0, d0
            write8(0x60 | src2);
        } else if (width == S_scaler) {
            // FMAXNM s0, s0, s0
            write8(0x20 | src2);
        } else {
            kpanic("Armv8btAsm::fMax invalid width: %d", width);
        }
        write8(0x1e);
    }
}

U8 Armv8btAsm::vGetTmpReg() {
    for (int i = 0; i < vNumberOfTmpRegs; i++) {
        if (!vTmpRegInUse[i]) {
            vTmpRegInUse[i] = true;
            return vTmp1 + i;
        }
    }
    kpanic("Armv8btAsm ran out of vector tmp regs");
    return 0;
}

void Armv8btAsm::vReleaseTmpReg(U8 reg) {
    vTmpRegInUse[reg - vTmp1] = false;
}

static void arm_invalidOp(CPU* cpu, U32 op) {
    klog("arm_invalidOp: 0x%X", op);
    cpu->thread->signalIllegalInstruction(5);
}

void Armv8btAsm::invalidOp(U32 op) {
    emulateSingleOp(currentOp);
    done = true;
}

static U8 fetchByte(void* p, U32* eip) {
    KMemory* memory = (KMemory*)p;
    return memory->readb((*eip)++);
}

static void arm64log(CPU* cpu) {
    if (!cpu->logFile.isOpen())
        return;
    thread_local static DecodedBlock* block = new DecodedBlock();
    decodeBlock(fetchByte, cpu->memory, cpu->eip.u32 + cpu->seg[CS].address, cpu->isBig(), 1, K_PAGE_SIZE, 0, block);
    block->op->log(cpu);
    block->op->dealloc(false);
}

void Armv8btAsm::logOp(U32 eip) {
    syncRegsFromHost();
    mov64(0, xCPU);
    callHost((void*)arm64log);
    syncRegsToHost();
}

void Armv8btAsm::signalIllegalInstruction(int code) {
    emulateSingleOp(currentOp);
    done = true;
}

void Armv8btAsm::translateInstruction() {
    this->startOfOpIp = this->ip;
    this->ip += this->currentOp->len;
#ifdef _DEBUG
    if (this->cpu->logFile.isOpen()) {
        this->logOp(this->startOfOpIp);
    }
    // just makes debugging the asm output easier
#ifndef __TEST
    this->loadConst(14, this->startOfOpIp);
    //data->writeMem32ValueOffset(xTmp5, xCPU, CPU_OFFSET_EIP);
#endif
#endif
    armv8btEncoder[this->currentOp->inst](this);

    for (int i = 0; i < xNumberOfTmpRegs; i++) {
        if (this->tmpRegInUse[i]) {
            kpanic("op(%x) leaked tmp reg", this->currentOp->originalOp);
        }
    }
}

#ifdef __TEST
void Armv8btAsm::addReturnFromTest() {
    syncRegsFromHost();

    restoreNativeState();
    addReturn();
}
#endif
#endif
