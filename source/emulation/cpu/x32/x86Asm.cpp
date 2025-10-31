#include "boxedwine.h"

#ifdef BOXEDWINE_DYNAMIC32

#include "x86Asm.h"

X86Asm::Reg8 X86Asm::al = 0;
X86Asm::Reg8 X86Asm::cl = 1;
X86Asm::Reg8 X86Asm::dl = 2;
X86Asm::Reg8 X86Asm::bl = 3;
X86Asm::Reg8 X86Asm::ah = 4;
X86Asm::Reg8 X86Asm::ch = 5;
X86Asm::Reg8 X86Asm::dh = 6;
X86Asm::Reg8 X86Asm::bh = 7;

X86Asm::Reg16 X86Asm::ax = 0;
X86Asm::Reg16 X86Asm::cx = 1;
X86Asm::Reg16 X86Asm::dx = 2;
X86Asm::Reg16 X86Asm::bx = 3;
X86Asm::Reg16 X86Asm::sp = 4;
X86Asm::Reg16 X86Asm::bp = 5;
X86Asm::Reg16 X86Asm::si = 6;
X86Asm::Reg16 X86Asm::di = 7;

X86Asm::Reg32 X86Asm::eax = 0;
X86Asm::Reg32 X86Asm::ecx = 1;
X86Asm::Reg32 X86Asm::edx = 2;
X86Asm::Reg32 X86Asm::ebx = 3;
X86Asm::Reg32 X86Asm::esp = 4;
X86Asm::Reg32 X86Asm::ebp = 5;
X86Asm::Reg32 X86Asm::esi = 6;
X86Asm::Reg32 X86Asm::edi = 7;

bool operator==(const X86Asm::Reg32& lhs, const X86Asm::Reg32& rhs) {
    return lhs.reg == rhs.reg;
}

void X86Asm::reset() {
    buffer.clear();
    patch.clear();
    ifJump.clear();
    jumps.clear();
}

void X86Asm::outb(U8 b) {
    buffer.push_back(b);
}

void X86Asm::outw(U16 w) {
    outb((U8)w);
    outb((U8)(w >> 8));
}

void X86Asm::outd(U32 d) {
    outb((U8)d);
    outb((U8)(d >> 8));
    outb((U8)(d >> 16));
    outb((U8)(d >> 24));
}

void X86Asm::lea(Reg32 dst, Reg32 rm, Reg32 sib, U32 shift, U32 disp) {
    outb(0x8d);

    if (!disp) {
        outb(0x04 | (dst.reg) << 3);
        outb((sib.reg << 3) | shift << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (dst.reg) << 3);
            outb((sib.reg << 3) | shift << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (dst.reg) << 3);
            outb((sib.reg << 3) | shift << 6 | rm.reg);
            outb((U8)disp);
        }
    }    
}

void X86Asm::lahf() {
    outb(0x9f);
}

void X86Asm::sahf() {
    outb(0x9e);
}

void X86Asm::lea(Reg32 dst, Reg32 rm, U32 disp) {
    outb(0x8d);

    if (!disp) {
        outb((dst.reg << 3) | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x80 | (dst.reg << 3) | rm.reg);
            outd(disp);
        } else {
            outb(0x40 | (dst.reg << 3) | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::bswap(Reg32 reg) {
    outb(0x0f);
    outb(0xc8 + reg.reg);
}

void X86Asm::mem32(U8 inst, Reg32 dst, Reg32 rm, U32 disp) {
    if (!disp) {
        outb(inst);
        outb(0x00 | (dst.reg << 3) | rm.reg);
    } else {
        S32 sIMM = (S32)disp;
        if (sIMM < -128 || sIMM > 127) {
            outb(inst);
            outb(0x80 | (dst.reg << 3) | rm.reg);
            outd(disp);
        } else {
            outb(inst);
            outb(0x40 | (dst.reg << 3) | rm.reg);
            outb(disp);
        }
    }
}

void X86Asm::mem32(U8 inst, Reg32 dst, Reg32 sib, U8 shift, U32 disp) {
    if (!shift) {
        mem32(inst, dst, sib, disp);
    } else {
        if (shift > 2) {
            kpanic("X86Asm::mem32 shift can not be greater than 2");
        }
        // without rm, there is only one way to encode this

        outb(inst);
        outb(0x04 | (dst.reg << 3));
        outb(((shift == 1) ? 0x40 : 0x80) | (sib.reg << 3) | 0x05); // 5 is sib with disp and no rm
        outd(disp);
    }
}

void X86Asm::mem32(U8 inst, Reg32 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp) {
    if (shift > 3) {
        kpanic("X86Asm::mem32 shift can not be greater than 3");
    }
    S32 sIMM = (S32)disp;
    bool small = true;
    if (sIMM < -128 || sIMM > 127) {
        small = false;
    }

    outb(inst);
    bool hasDisp = disp != 0;
    if (!hasDisp && rm.reg == 5) {
        hasDisp = true;
    }
    if (!hasDisp) {
        outb(0x04 | (dst.reg << 3));
    } else if (small) {
        outb(0x44 | (dst.reg << 3));
    } else {
        outb(0x84 | (dst.reg << 3));
    }
    if (shift == 0) {
        outb((sib.reg << 3) | rm.reg);
    } else {
        outb((shift << 6) | (sib.reg << 3) | rm.reg);
    } 
    if (hasDisp) {
        if (small) {
            outb(disp);
        } else {
            outd(disp);
        }
    }
}

void X86Asm::mem16(U8 inst, Reg16 dst, Reg32 rm, U32 disp) {
    mem32(inst, Reg32(dst.reg), rm, disp);
}

void X86Asm::mem16(U8 inst, Reg16 dst, Reg32 sib, U8 shift, U32 disp) {
    mem32(inst, Reg32(dst.reg), sib, shift, disp);
}

void X86Asm::mem16(U8 inst, Reg16 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp) {
    mem32(inst, Reg32(dst.reg), rm, sib, shift, disp);
}

void X86Asm::mem8(U8 inst, Reg8 dst, Reg32 rm, U32 disp) {
    mem32(inst, Reg32(dst.reg), rm, disp);
}

void X86Asm::mem8(U8 inst, Reg8 dst, Reg32 rm, U8 shift, U32 disp) {
    mem32(inst, Reg32(dst.reg), rm, shift, disp);
}

void X86Asm::mem8(U8 inst, Reg8 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp) {
    mem32(inst, Reg32(dst.reg), rm, sib, shift, disp);
}

void X86Asm::addMemReg(Reg16 dst, Reg32 rm, U32 disp) {
    outb(0x66);
    mem32(0x03, Reg32(dst.reg), rm, disp);
}

void X86Asm::addMemReg(Reg32 dst, Reg32 rm, U32 disp) {
    mem32(0x03, dst, rm, disp);
}

void X86Asm::subMemReg(Reg32 dst, Reg32 rm, U32 disp) {
    mem32(0x2b, dst, rm, disp);
}

void X86Asm::addMemReg(Reg32 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0x01, reg, rm, sib, lsl, disp);
}

void X86Asm::addMemReg(Reg16 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    mem32(0x01, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::addMemReg(Reg8 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0x00, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::subMemReg(Reg32 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0x29, reg, rm, sib, lsl, disp);
}

void X86Asm::subMemReg(Reg16 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    mem32(0x29, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::subMemReg(Reg8 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0x28, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::orMemReg(Reg32 reg, Reg32 rm, U32 disp) {
    mem32(0x09, Reg32(reg.reg), rm, disp);
}

void X86Asm::orMemReg(Reg16 reg, Reg32 rm, U32 disp) {
    outb(0x66);
    mem32(0x09, Reg32(reg.reg), rm, disp);
}

void X86Asm::addMem16(Reg32 rm, U32 disp, U16 value) {
    S32 sValue = (S32)value;
    S32 sDisp = (S32)disp;
    bool smallDisp = sDisp >= -128 && sDisp <= 127;
    bool smallValue = sValue >= -128 && sValue <= 127;

    outb(0x66);
    if (smallValue) {
        outb(0x83);
    } else {
        outb(0x81);
    }

    if (!disp) {
        outb(0x00 | rm.reg);
    } else if (smallDisp) {
        outb(0x40 | rm.reg);
        outb((U8)disp);
    } else {
        outb(0x80 | rm.reg);
        outb(disp);
    }

    if (smallValue) {
        outb((U8)value);
    } else {
        outw(value);
    }
}

void X86Asm::addMem8(Reg32 rm, U32 disp, U8 value) {
    S32 sDisp = (S32)disp;
    bool smallDisp = sDisp >= -128 && sDisp <= 127;

    outb(0x80);

    if (!disp) {
        outb(0x00 | rm.reg);
    } else if (smallDisp) {
        outb(0x40 | rm.reg);
        outb((U8)disp);
    } else {
        outb(0x80 | rm.reg);
        outb(disp);
    }

    outb(value);
}

void X86Asm::addMem32(Reg32 rm, U32 disp, U32 value) {
    S32 sValue = (S32)value;
    S32 sDisp = (S32)disp;
    bool smallDisp = sDisp >= -128 && sDisp <= 127;
    bool smallValue = sValue >= -128 && sValue <= 127;

    if (smallValue) {
        outb(0x83);
    } else {
        outb(0x81);
    }

    if (!disp) {
        outb(0x00 | rm.reg);
    } else if (smallDisp) {
        outb(0x40 | rm.reg);
        outb((U8)disp);
    } else {
        outb(0x80 | rm.reg);
        outb(disp);
    }

    if (smallValue) {
        outb((U8)value);
    } else {
        outd(value);
    }
}

void X86Asm::addMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U32 value) {
    S32 sValue = (S32)value;
    bool smallValue = sValue >= -128 && sValue <= 127;

    mem32(smallValue ? 0x83 : 0x81, 0, rm, sib, lsl, disp);
    if (smallValue) {
        outb((U8)value);
    } else {
        outd(value);
    }
}

void X86Asm::addMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U16 value) {
    S32 sValue = (S32)value;
    bool smallValue = sValue >= -128 && sValue <= 127;

    outb(0x66);
    mem32(smallValue ? 0x83 : 0x81, 0, rm, sib, lsl, disp);
    if (smallValue) {
        outb((U8)value);
    } else {
        outw(value);
    }
}

void X86Asm::addMem8(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    mem32(0x80, 0, rm, sib, lsl, disp);
    outb((U8)value);
}

void X86Asm::subMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U32 value) {
    S32 sValue = (S32)value;
    bool smallValue = sValue >= -128 && sValue <= 127;

    mem32(smallValue ? 0x83 : 0x81, 5, rm, sib, lsl, disp);
    if (smallValue) {
        outb((U8)value);
    } else {
        outd(value);
    }
}

void X86Asm::andMem32(Reg32 rm, U32 disp, U32 value) {
    S32 sValue = (S32)value;
    bool smallValue = sValue >= -128 && sValue <= 127;

    mem32(smallValue ? 0x83 : 0x81, 4, rm, disp);
    if (smallValue) {
        outb((U8)value);
    } else {
        outd(value);
    }
}

void X86Asm::notMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0xf7, 2, rm, sib, lsl, disp);
}

void X86Asm::notMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    mem32(0xf7, 2, rm, sib, lsl, disp);
}

void X86Asm::notMem8(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0xf6, 2, rm, sib, lsl, disp);
}

void X86Asm::negMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0xf7, 3, rm, sib, lsl, disp);
}

void X86Asm::negMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    mem32(0xf7, 3, rm, sib, lsl, disp);
}

void X86Asm::negMem8(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    mem32(0xf6, 3, rm, sib, lsl, disp);
}

void X86Asm::btsMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    outb(0x0f);
    mem32(0xba, 5, rm, sib, lsl, disp);
    outb(value);
}

void X86Asm::btsMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    outb(0x66);
    outb(0x0f);
    mem32(0xba, 5, rm, sib, lsl, disp);
    outb(value);
}

void X86Asm::btsMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, Reg32 value) {
    outb(0x0f);
    mem32(0xab, value, rm, sib, lsl, disp);
}

void X86Asm::btsMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, Reg16 value) {
    outb(0x66);
    outb(0x0f);
    mem32(0xab, Reg32(value.reg), rm, sib, lsl, disp);
}

void X86Asm::btrMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    outb(0x0f);
    mem32(0xba, 6, rm, sib, lsl, disp);
    outb(value);
}

void X86Asm::btrMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    outb(0x66);
    outb(0x0f);
    mem32(0xba, 6, rm, sib, lsl, disp);
    outb(value);
}

void X86Asm::btrMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, Reg32 value) {
    outb(0x0f);
    mem32(0xb3, value, rm, sib, lsl, disp);
}

void X86Asm::btrMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, Reg16 value) {
    outb(0x66);
    outb(0x0f);
    mem32(0xb3, Reg32(value.reg), rm, sib, lsl, disp);
}

void X86Asm::btcMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    outb(0x0f);
    mem32(0xba, 7, rm, sib, lsl, disp);
    outb(value);
}

void X86Asm::btcMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    outb(0x66);
    outb(0x0f);
    mem32(0xba, 7, rm, sib, lsl, disp);
    outb(value);
}

void X86Asm::btcMem32(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, Reg32 value) {
    outb(0x0f);
    mem32(0xbb, value, rm, sib, lsl, disp);
}

void X86Asm::btcMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, Reg16 value) {
    outb(0x66);
    outb(0x0f);
    mem32(0xbb, Reg32(value.reg), rm, sib, lsl, disp);
}

void X86Asm::subMem16(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U16 value) {
    S32 sValue = (S32)value;
    bool smallValue = sValue >= -128 && sValue <= 127;

    outb(0x66);
    mem32(smallValue ? 0x83 : 0x81, 5, rm, sib, lsl, disp);
    if (smallValue) {
        outb((U8)value);
    } else {
        outw(value);
    }
}

void X86Asm::subMem8(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 value) {
    mem32(0x80, 5, rm, sib, lsl, disp);
    outb((U8)value);
}

void X86Asm::mov(Reg32 dst, U32 imm) {
    outb(0xb8 + dst.reg);
    outd(imm);
}

void X86Asm::mov(Reg16 dst, U16 imm) {
    outb(0x66);
    outb(0xb8 + dst.reg);
    outw(imm);
}

void X86Asm::mov(Reg8 dst, U8 imm) {
    outb(0xb0 + dst.reg);
    outb(imm);
}

void X86Asm::readMem(Reg32 dst, Reg32 rm, U32 disp) {
    mem32(0x8b, dst, rm, disp);
}

void X86Asm::readMem(Reg32 dst, Reg32 rm, U8 shift, U32 disp) {
    mem32(0x8b, dst, rm, shift, disp);
}

void X86Asm::readMem(Reg32 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp) {
    mem32(0x8b, dst, rm, sib, shift, disp);
}

void X86Asm::readMem(Reg16 dst, Reg32 rm, U32 disp) {
    outb(0x66);
    mem16(0x8b, dst, rm, disp);
}

void X86Asm::readMem(Reg16 dst, Reg32 rm, U8 shift, U32 disp) {
    outb(0x66);
    mem16(0x8b, dst, rm, shift, disp);
}

void X86Asm::readMem(Reg16 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp) {
    outb(0x66);
    mem16(0x8b, dst, rm, sib, shift, disp);
}

void X86Asm::readMem(Reg8 dst, Reg32 rm, U32 disp) {
    mem8(0x8a, dst, rm, disp);
}

void X86Asm::readMem(Reg8 dst, Reg32 sib, U8 shift, U32 disp) {
    mem8(0x8a, dst, sib, shift, disp);
}

void X86Asm::readMem(Reg8 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp) {
    mem8(0x8a, dst, rm, sib, shift, disp);
}

void X86Asm::writeMem(Reg32 rm, U32 disp, Reg32 src) {
    mem32(0x89, src, rm, disp);
}

void X86Asm::writeMem(Reg32 rm, U32 disp, Reg16 src) {
    outb(0x66);
    mem16(0x89, src, rm, disp);
}

void X86Asm::writeMem(Reg32 rm, U32 disp, Reg8 src) {
    mem8(0x88, src, rm, disp);
}

void X86Asm::writeMem(Reg32 rm, U32 disp, U32 src) {
    mem32(0xc7, Reg32(0), rm, disp);
    outd(src);
}

void X86Asm::writeMem(Reg32 rm, U32 disp, U16 src) {
    outb(0x66);
    mem32(0xc7, Reg32(0), rm, disp);
    outw(src);
}

void X86Asm::writeMem(Reg32 rm, U32 disp, U8 src) {
    mem32(0xc6, Reg32(0), rm, disp);
    outb(src);
}

void X86Asm::writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, Reg32 src) {
    mem32(0x89, src, rm, sib, shift, disp);
}

void X86Asm::writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, Reg16 src) {
    outb(0x66);
    mem32(0x89, Reg32(src.reg), rm, sib, shift, disp);
}

void X86Asm::writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, Reg8 src) {
    mem32(0x88, Reg32(src.reg), rm, sib, shift, disp);
}

void X86Asm::writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, U32 src) {
    mem32(0xc7, Reg32(0), rm, sib, shift, disp);
    outd(src);
}

void X86Asm::writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, U16 src) {
    outb(0x66);
    mem32(0xc7, Reg32(0), rm, sib, shift, disp);
    outw(src);
}

void X86Asm::writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, U8 src) {
    mem32(0xc6, Reg32(0), rm, sib, shift, disp);
    outb(src);
}

void X86Asm::group1(U8 e, U8 math, Reg32 dst, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        if (dst.reg == 0) {
            outb(e);            
        } else {
            outb(0x81);
            outb(0xC0 | (math << 3) | dst.reg);
        }
        outd(imm);
    } else {
        outb(0x83);
        outb(0xC0 | (math << 3) | dst.reg);
        outb(imm);
    }
}

void X86Asm::group1(U8 e, U8 math, Reg16 dst, U16 imm) {
    outb(0x66);
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        if (dst.reg == 0) {
            outb(e);
        } else {
            outb(0x81);
            outb(0xC0 | (math << 3) | dst.reg);
        }
        outw(imm);
    } else {
        outb(0x83);
        outb(0xC0 | (math << 3) | dst.reg);
        outb((U8)imm);
    }
}

void X86Asm::group1(U8 e, U8 math, Reg8 dst, U8 imm) {
    if (dst.reg == 0) {
        outb(e);
    } else {
        outb(0x80);
        outb(0xC0 | (math << 3) | dst.reg);
    }
    outb(imm);
}

void X86Asm::add(Reg32 dst, U32 imm) {
    group1(0x05, 0, dst, imm);
}

void X86Asm::add(Reg16 dst, U16 imm) {
    group1(0x05, 0, dst, imm);
}

void X86Asm::add(Reg8 dst, U8 imm) {
    group1(0x04, 0, dst, imm);
}

void X86Asm::adc(Reg32 dst, U32 imm) {
    group1(0x15, 2, dst, imm);
}

void X86Asm::adc(Reg16 dst, U16 imm) {
    group1(0x15, 2, dst, imm);
}

void X86Asm::adc(Reg8 dst, U8 imm) {
    group1(0x14, 2, dst, imm);
}

void X86Asm::sub(Reg32 dst, U32 imm) {
    group1(0x2d, 5, dst, imm);
}

void X86Asm::sub(Reg16 dst, U16 imm) {
    group1(0x2d, 5, dst, imm);
}

void X86Asm::sub(Reg8 dst, U8 imm) {
    group1(0x2c, 5, dst, imm);
}

void X86Asm::sbb(Reg32 dst, U32 imm) {
    group1(0x1d, 3, dst, imm);
}

void X86Asm::sbb(Reg16 dst, U16 imm) {
    group1(0x1d, 3, dst, imm);
}

void X86Asm::sbb(Reg8 dst, U8 imm) {
    group1(0x1c, 3, dst, imm);
}

void X86Asm::and_(Reg32 dst, U32 imm) {
    group1(0x25, 4, dst, imm);
}

void X86Asm::and_(Reg16 dst, U16 imm) {
    group1(0x25, 4, dst, imm);
}

void X86Asm::and_(Reg8 dst, U8 imm) {
    group1(0x24, 4, dst, imm);
}

void X86Asm::or_(Reg32 dst, U32 imm) {
    group1(0x0d, 1, dst, imm);
}

void X86Asm::or_(Reg16 dst, U16 imm) {
    group1(0x0d, 1, dst, imm);
}

void X86Asm::or_(Reg8 dst, U8 imm) {
    group1(0x0c, 1, dst, imm);
}

void X86Asm::xor_(Reg32 dst, U32 imm) {
    group1(0x35, 6, dst, imm);
}

void X86Asm::xor_(Reg16 dst, U16 imm) {
    group1(0x35, 6, dst, imm);
}

void X86Asm::xor_(Reg8 dst, U8 imm) {
    group1(0x34, 6, dst, imm);
}

void X86Asm::group2(U8 op, Reg32 dst, U32 imm) {
    outb(0xC1);
    outb(0xC0 | (op << 3) | dst.reg);
    outb((U8)imm);
}

void X86Asm::group2(U8 op, Reg16 dst, U16 imm) {
    outb(0x66);
    outb(0xC1);
    outb(0xC0 | (op << 3) | dst.reg);
    outb((U8)imm);
}

void X86Asm::group2(U8 op, Reg8 dst, U8 imm) {
    outb(0xC0);
    outb(0xC0 | (op << 3) | dst.reg);
    outb(imm);
}

void X86Asm::shl(Reg32 dst, U32 imm) {
    group2(6, dst, imm);
}

void X86Asm::shl(Reg16 dst, U16 imm) {
    group2(6, dst, imm);
}

void X86Asm::shl(Reg8 dst, U8 imm) {
    group2(6, dst, imm);
}

void X86Asm::shld(Reg32 dst, Reg32 src, U32 imm) {
    outb(0x0f);
    outb(0xa4);
    outb(0xC0 | (src.reg << 3) | dst.reg);
    outb((U8)imm);
}

void X86Asm::shld(Reg16 dst, Reg16 src, U16 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0xa4);
    outb(0xC0 | (src.reg << 3) | dst.reg);
    outb((U8)imm);
}

void X86Asm::shrd(Reg32 dst, Reg32 src, U32 imm) {
    outb(0x0f);
    outb(0xac);
    outb(0xC0 | (src.reg << 3) | dst.reg);
    outb((U8)imm);
}

void X86Asm::shrd(Reg16 dst, Reg16 src, U16 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0xac);
    outb(0xC0 | (src.reg << 3) | dst.reg);
    outb((U8)imm);
}

void X86Asm::shld(Reg16 dst, Reg16 src, Reg16 cl) {
    if (cl.reg != 1) {
        kpanic("X86Asm::shld must use cl");
    }
    outb(0x66);
    outb(0x0f);
    outb(0xa5);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::shld(Reg32 dst, Reg32 src, Reg32 cl) {
    if (cl.reg != 1) {
        kpanic("X86Asm::shld must use cl");
    }
    outb(0x0f);
    outb(0xa5);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::shrd(Reg16 dst, Reg16 src, Reg16 cl) {
    if (cl.reg != 1) {
        kpanic("X86Asm::shrd must use cl");
    }
    outb(0x66);
    outb(0x0f);
    outb(0xad);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::shrd(Reg32 dst, Reg32 src, Reg32 cl) {
    if (cl.reg != 1) {
        kpanic("X86Asm::shrd must use cl");
    }
    outb(0x0f);
    outb(0xad);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::shr(Reg32 dst, U32 imm) {
    group2(5, dst, imm);
}

void X86Asm::shr(Reg16 dst, U16 imm) {
    group2(5, dst, imm);
}

void X86Asm::shr(Reg8 dst, U8 imm) {
    group2(5, dst, imm);
}

void X86Asm::sar(Reg32 dst, U32 imm) {
    group2(7, dst, imm);
}

void X86Asm::sar(Reg16 dst, U16 imm) {
    group2(7, dst, imm);
}

void X86Asm::sar(Reg8 dst, U8 imm) {
    group2(7, dst, imm);
}

void X86Asm::rol(Reg32 dst, U32 imm) {
    group2(0, dst, imm);
}

void X86Asm::rol(Reg16 dst, U16 imm) {
    group2(0, dst, imm);
}

void X86Asm::rol(Reg8 dst, U8 imm) {
    group2(0, dst, imm);
}

void X86Asm::ror(Reg32 dst, U32 imm) {
    group2(1, dst, imm);
}

void X86Asm::ror(Reg16 dst, U16 imm) {
    group2(1, dst, imm);
}

void X86Asm::ror(Reg8 dst, U8 imm) {
    group2(1, dst, imm);
}

void X86Asm::rcl(Reg32 dst, U32 imm) {
    group2(2, dst, imm);
}

void X86Asm::rcl(Reg16 dst, U16 imm) {
    group2(2, dst, imm);
}

void X86Asm::rcl(Reg8 dst, U8 imm) {
    group2(2, dst, imm);
}

void X86Asm::rcr(Reg32 dst, U32 imm) {
    group2(3, dst, imm);
}

void X86Asm::rcr(Reg16 dst, U16 imm) {
    group2(3, dst, imm);
}

void X86Asm::rcr(Reg8 dst, U8 imm) {
    group2(3, dst, imm);
}

void X86Asm::add(Reg32 dst, Reg32 src) {
    outb(0x01);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::add(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x01);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::add(Reg8 dst, Reg8 src) {
    outb(0x00);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::adc(Reg32 dst, Reg32 src) {
    outb(0x11);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::adc(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x11);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::adc(Reg8 dst, Reg8 src) {
    outb(0x10);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::sub(Reg32 dst, Reg32 src) {
    outb(0x29);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::sub(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x29);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::sub(Reg8 dst, Reg8 src) {
    outb(0x28);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::sbb(Reg32 dst, Reg32 src) {
    outb(0x19);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::sbb(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x19);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::sbb(Reg8 dst, Reg8 src) {
    outb(0x18);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::and_(Reg32 dst, Reg32 src) {
    outb(0x21);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::and_(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x21);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::and_(Reg8 dst, Reg8 src) {
    outb(0x20);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::or_(Reg32 dst, Reg32 src) {
    outb(0x09);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::or_(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x09);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::or_(Reg8 dst, Reg8 src) {
    outb(0x08);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::xor_(Reg32 dst, Reg32 src) {
    outb(0x31);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::xor_(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x31);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::xor_(Reg8 dst, Reg8 src) {
    outb(0x30);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::shl(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shl must use cl");
    }
    outb(0xd3);
    outb(0xe0 | dst.reg);
}

void X86Asm::shl(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shl must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xe0 | dst.reg);
}

void X86Asm::shl(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shl must use cl");
    }
    outb(0xd2);
    outb(0xe0 | dst.reg);
}

void X86Asm::shr(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shr must use cl");
    }
    outb(0xd3);
    outb(0xe8 | dst.reg);
}

void X86Asm::shr(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shr must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xe8 | dst.reg);
}

void X86Asm::shr(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shr must use cl");
    }
    outb(0xd2);
    outb(0xe8 | dst.reg);
}

void X86Asm::sar(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::sar must use cl");
    }
    outb(0xd3);
    outb(0xf8 | dst.reg);
}

void X86Asm::sar(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::sar must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xf8 | dst.reg);
}

void X86Asm::sar(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::sar must use cl");
    }
    outb(0xd2);
    outb(0xf8 | dst.reg);
}

void X86Asm::rol(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rol must use cl");
    }
    outb(0xd3);
    outb(0xc0 | dst.reg);
}

void X86Asm::rol(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rol must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xc0 | dst.reg);
}

void X86Asm::rol(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rol must use cl");
    }
    outb(0xd2);
    outb(0xc0 | dst.reg);
}

void X86Asm::ror(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::ror must use cl");
    }
    outb(0xd3);
    outb(0xc8 | dst.reg);
}

void X86Asm::ror(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::ror must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xc8 | dst.reg);
}

void X86Asm::ror(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::ror must use cl");
    }
    outb(0xd2);
    outb(0xc8 | dst.reg);
}

void X86Asm::rcl(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rcl must use cl");
    }
    outb(0xd3);
    outb(0xd0 | dst.reg);
}

void X86Asm::rcl(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rcl must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xd0 | dst.reg);
}

void X86Asm::rcl(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rcl must use cl");
    }
    outb(0xd2);
    outb(0xd0 | dst.reg);
}

void X86Asm::rcr(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rcr must use cl");
    }
    outb(0xd3);
    outb(0xd8 | dst.reg);
}

void X86Asm::rcr(Reg16 dst, Reg16 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rcr must use cl");
    }
    outb(0x66);
    outb(0xd3);
    outb(0xd8 | dst.reg);
}

void X86Asm::rcr(Reg8 dst, Reg8 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::rcr must use cl");
    }
    outb(0xd2);
    outb(0xd8 | dst.reg);
}

void X86Asm::mulEax(Reg32 src) {
    outb(0xf7);
    outb(0xe0 | src.reg);
}

void X86Asm::mulAx(Reg16 src) {
    outb(0x66);
    outb(0xf7);
    outb(0xe0 | src.reg);
}

void X86Asm::mulAl(Reg8 src) {
    outb(0xf6);
    outb(0xe0 | src.reg);
}

void X86Asm::imulEax(Reg32 src) {
    outb(0xf7);
    outb(0xe8 | src.reg);
}

void X86Asm::imulAx(Reg16 src) {
    outb(0x66);
    outb(0xf7);
    outb(0xe8 | src.reg);
}

void X86Asm::imulAl(Reg8 src) {
    outb(0xf6);
    outb(0xe8 | src.reg);
}

void X86Asm::mul(Reg32 src) {
    outb(0xf7);
    outb(0xe0 | src.reg);
}

void X86Asm::imul(Reg32 dst, Reg32 src) {
    outb(0x0f);
    outb(0xaf);
    outb(0xc0 | (dst.reg << 3) | src.reg);
}

void X86Asm::imul(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x0f);
    outb(0xaf);
    outb(0xc0 | (dst.reg << 3) | src.reg);
}

void X86Asm::imul(Reg32 dst, U32 imm) {
    S32 sIMM = (S32)imm;
    bool small = true;
    if (sIMM < -128 || sIMM > 127) {
        small = false;
    }
    if (small) {
        outb(0x6b);
    } else {
        outb(0x69);
    }
    outb(0xc0 | dst.reg | (dst.reg << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outd(imm);
    }
}

void X86Asm::imul(Reg16 dst, U16 imm) {
    S32 sIMM = (S32)imm;
    bool small = true;
    if (sIMM < -128 || sIMM > 127) {
        small = false;
    }
    outb(0x66);
    if (small) {
        outb(0x6b);
    } else {
        outb(0x69);
    }
    outb(0xc0 | dst.reg | (dst.reg << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outw((U16)imm);
    }
}

void X86Asm::imul(Reg32 dst, Reg32 src, U32 imm) {
    S32 sIMM = (S32)imm;
    bool small = true;
    if (sIMM < -128 || sIMM > 127) {
        small = false;
    }

    if (small) {
        outb(0x6b);
    } else {
        outb(0x69);
    }
    outb(0xc0 | src.reg | (dst.reg << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outd(imm);
    }
}

void X86Asm::imul(Reg16 dst, Reg16 src, U16 imm) {
    S32 sIMM = (S32)imm;
    bool small = true;
    if (sIMM < -128 || sIMM > 127) {
        small = false;
    }
    outb(0x66);
    if (small) {
        outb(0x6b);
    } else {
        outb(0x69);
    }
    outb(0xc0 | src.reg | (dst.reg << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outw(imm);
    }
}

void X86Asm::div(Reg16 src) {
    outb(0x66);
    outb(0xf7);
    outb(0xf0 | src.reg);
}

void X86Asm::div(Reg32 src) {
    outb(0xf7);
    outb(0xf0 | src.reg);
}

void X86Asm::idiv(Reg16 src) {
    outb(0x66);
    outb(0xf7);
    outb(0xf8 | src.reg);
}

void X86Asm::idiv(Reg32 src) {
    outb(0xf7);
    outb(0xf8 | src.reg);
}

void X86Asm::neg(Reg32 dst) {
    outb(0xf7);
    outb(0xd8 | dst.reg);
}

void X86Asm::neg(Reg16 dst) {
    outb(0x66);
    outb(0xf7);
    outb(0xd8 | dst.reg);
}

void X86Asm::neg(Reg8 dst) {
    outb(0xf6);
    outb(0xd8 | dst.reg);
}

void X86Asm::not_(Reg32 dst) {
    outb(0xf7);
    outb(0xd0 | dst.reg);
}

void X86Asm::not_(Reg16 dst) {
    outb(0x66);
    outb(0xf7);
    outb(0xd0 | dst.reg);
}

void X86Asm::not_(Reg8 dst) {
    outb(0xf6);
    outb(0xd0 | dst.reg);
}

void X86Asm::inc(Reg32 dst) {
    outb(0x40 + dst.reg);
}

void X86Asm::inc(Reg16 dst) {
    outb(0x66);
    outb(0x40 + dst.reg);
}

void X86Asm::inc(Reg8 dst) {
    outb(0xfe);
    outb(0xc0 | dst.reg);
}

void X86Asm::dec(Reg32 dst) {
    outb(0x48 + dst.reg);
}

void X86Asm::dec(Reg16 dst) {
    outb(0x66);
    outb(0x48 + dst.reg);
}

void X86Asm::dec(Reg8 dst) {
    outb(0xfe);
    outb(0xc8 | dst.reg);
}

void X86Asm::cmp(Reg32 dst, U32 imm) {
    group1(0x3d, 7, dst, imm);
}

void X86Asm::cmp(Reg16 dst, U16 imm) {
    group1(0x3d, 7, dst, imm);
}

void X86Asm::cmp(Reg8 dst, U8 imm) {
    group1(0x3c, 7, dst, imm);
}

void X86Asm::cmp(Reg32 dst, Reg32 src) {
    outb(0x39);
    outb(0xC0 | dst.reg | (src.reg << 3));
}

void X86Asm::cmp(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x39);
    outb(0xC0 | dst.reg | (src.reg << 3));
}

void X86Asm::cmp(Reg8 dst, Reg8 src) {
    outb(0x38);
    outb(0xC0 | dst.reg | (src.reg << 3));
}

void X86Asm::test(Reg32 dst, U32 imm) {
    if (dst.reg == 0) {
        outb(0xa9);
    } else {
        outb(0xf7);
        outb(0xC0 | dst.reg);
    }
    outd(imm);
}

void X86Asm::test(Reg16 dst, U16 imm) {
    outb(0x66);
    if (dst.reg == 0) {
        outb(0xa9);
    } else {
        outb(0xf7);
        outb(0xC0 | dst.reg);
    }
    outw(imm);
}

void X86Asm::test(Reg8 dst, U8 imm) {
    if (dst.reg == 0) {
        outb(0xa8);
    } else {
        outb(0xf6);
        outb(0xC0 | dst.reg);
    }
    outb(imm);
}

void X86Asm::test(Reg32 dst, Reg32 src) {
    outb(0x85);
    outb(0xc0 | dst.reg | (src.reg << 3));
}

void X86Asm::test(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x85);
    outb(0xc0 | dst.reg | (src.reg << 3));
}

void X86Asm::test(Reg8 dst, Reg8 src) {
    outb(0x84);
    outb(0xc0 | dst.reg | (src.reg << 3));
}

void X86Asm::movsx(Reg16 dst, Reg8 src) {
    outb(0x66);
    outb(0x0f);
    outb(0xbe);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movsx(Reg32 dst, Reg8 src) {
    outb(0x0f);
    outb(0xbe);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movsx(Reg32 dst, Reg16 src) {
    outb(0x0f);
    outb(0xbf);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::mov(Reg32 dst, Reg32 src) {
    outb(0x89);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::mov(Reg16 dst, Reg16 src) {
    outb(0x66);
    outb(0x89);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::mov(Reg8 dst, Reg8 src) {
    outb(0x88);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::movzx(Reg16 dst, Reg8 src) {
    outb(0x66);
    outb(0x0f);
    outb(0xb6);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movzx(Reg32 dst, Reg8 src) {
    outb(0x0f);
    outb(0xb6);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movzx(Reg32 dst, Reg16 src) {
    outb(0x0f);
    outb(0xb7);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::push(U32 imm) {
    outb(0x68);
    outd(imm);
}

void X86Asm::pushFlags() {
    outb(0x9c);
}

void X86Asm::push(Reg32 reg) {
    outb(0x50 | reg.reg);
}

void X86Asm::pop(Reg32 reg) {
    outb(0x58 | reg.reg);
}

void X86Asm::call(void* address) {
    outb(0xe8);
    patch.push_back(buffer.size());
    outd((U32)address);
}

void X86Asm::jmp(Reg32 reg) {
    outb(0xff);
    outb(0xe0 | reg.reg);
}

void X86Asm::jo(U32 address) {
    outb(0xf);
    outb(0x80);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jno(U32 address) {
    outb(0xf);
    outb(0x81);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jb(U32 address) {
    outb(0xf);
    outb(0x82);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnb(U32 address) {
    outb(0xf);
    outb(0x83);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jz(U32 address) {
    outb(0xf);
    outb(0x84);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnz(U32 address) {
    outb(0xf);
    outb(0x85);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jbe(U32 address) {
    outb(0xf);
    outb(0x86);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnbe(U32 address) {
    outb(0xf);
    outb(0x87);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::js(U32 address) {
    outb(0xf);
    outb(0x88);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jns(U32 address) {
    outb(0xf);
    outb(0x89);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jp(U32 address) {
    outb(0xf);
    outb(0x8a);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnp(U32 address) {
    outb(0xf);
    outb(0x8b);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jl(U32 address) {
    outb(0xf);
    outb(0x8c);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnl(U32 address) {
    outb(0xf);
    outb(0x8d);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jle(U32 address) {
    outb(0xf);
    outb(0x8e);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnle(U32 address) {
    outb(0xf);
    outb(0x8f);
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jmp(U32 address) {
    outb(0xe9);
    if (address == 0x0045a821) {
        int ii = 0;
    }
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0);
}

void X86Asm::movsb_repeat() {
    outb(0xf2);
    outb(0xa4);
}

void X86Asm::lock() {
    outb(0xf0);
}

void X86Asm::cmpxchg(Reg32 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x0f);
    outb(0xb1);

    if (!disp) {
        outb(0x04 | (reg.reg << 3));
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::cmpxchg(Reg16 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    outb(0xf0);
    outb(0x0f);
    outb(0xb1);

    if (!disp) {
        outb(0x04 | (reg.reg << 3));
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::cmpxchg(Reg8 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x0f);
    outb(0xb0);

    if (!disp) {
        outb(0x04 | (reg.reg << 3));
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::xchg(Reg32 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x87);

    if (!disp) {
        outb(0x04 | (reg.reg << 3));
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::xchg(Reg16 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    outb(0xf0);
    outb(0x87);

    if (!disp) {
        outb(0x04 | (reg.reg << 3));
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::xchg(Reg8 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x86);

    if (!disp) {
        outb(0x04 | (reg.reg << 3));
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x84 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x44 | (reg.reg << 3));
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::xchg(Reg8 reg, Reg8 rm) {
    outb(0x86);
    outb(0xC0 | reg.reg | (rm.reg << 3));
}

void X86Asm::xchg(Reg16 reg, Reg16 rm) {
    outb(0x66);
    outb(0x87);
    outb(0xC0 | reg.reg | (rm.reg << 3));
}

void X86Asm::xchg(Reg32 reg, Reg32 rm) {
    outb(0x87);
    outb(0xC0 | reg.reg | (rm.reg << 3));
}

void X86Asm::xadd(Reg32 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x0f);
    mem32(0xc1, reg, rm, sib, lsl, disp);
}

void X86Asm::xadd(Reg16 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x66);
    outb(0x0f);
    mem16(0xc1, reg, rm, sib, lsl, disp);
}

void X86Asm::xadd(Reg8 reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x0f);
    mem8(0xc0, reg, rm, sib, lsl, disp);
}

void X86Asm::cmpxchg8b(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf0);
    outb(0x0f);
    outb(0xc7);

    if (!disp) {
        outb(0x0c);
        outb((sib.reg << 3) | lsl << 6 | rm.reg);
    } else {
        S32 sdisp = (S32)disp;
        if (sdisp < -128 || sdisp > 127) {
            outb(0x8c);
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outd(disp);
        } else {
            outb(0x4c);
            outb((sib.reg << 3) | lsl << 6 | rm.reg);
            outb((U8)disp);
        }
    }
}

void X86Asm::ret() {
    outb(0xc3);
}

void X86Asm::IfLessThan(Reg32 reg, U32 value) {
    cmp(reg, value);
    outb(0x73); // jnb
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfLessThan(Reg16 reg, U16 value) {
    cmp(reg, value);
    outb(0x73); // jnb
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfLessThan(Reg8 reg, U8 value) {
    cmp(reg, value);
    outb(0x73); // jnb
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfEqual(Reg32 reg, U32 value) {
    cmp(reg, value);
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfEqual(Reg16 reg, U16 value) {
    cmp(reg, value);
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfEqual(Reg8 reg, U8 value) {
    cmp(reg, value);
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfNotEqual(Reg32 reg, U32 value) {
    cmp(reg, value);
    outb(0x74); // jz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfZero(Reg32 reg) {
    test(reg, reg);
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfZero(Reg16 reg) {
    test(reg, reg);
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfZero(Reg8 reg) {
    test(reg, reg);
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfPF() {
    outb(0x7b); // jnp
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfCF() {
    outb(0x73); // jnb
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfZF() {
    outb(0x75); // jnz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
}

void X86Asm::IfNotZero(Reg32 reg, bool bigJump) {
    test(reg, reg);
    if (bigJump) {
        outb(0x0f);
        outb(0x84); // jz
        ifJump.push_back(buffer.size());
        outd(0); // jump over amount
    } else {
        outb(0x74); // jz
        ifJump.push_back(buffer.size());
        outb(0); // jump over amount
    }
}

void X86Asm::IfNotZero(Reg16 reg, bool bigJump) {
    test(reg, reg);
    if (bigJump) {
        outb(0x0f);
        outb(0x84); // jz
        ifJump.push_back(buffer.size());
        outd(0); // jump over amount
    } else {
        outb(0x74); // jz
        ifJump.push_back(buffer.size());
        outb(0); // jump over amount
    }
}

void X86Asm::IfNotZero(Reg8 reg, bool bigJump) {
    test(reg, reg);
    if (bigJump) {
        outb(0x0f);
        outb(0x84); // jz
        ifJump.push_back(buffer.size());
        outd(0); // jump over amount
    } else {
        outb(0x74); // jz
        ifJump.push_back(buffer.size());
        outb(0); // jump over amount
    }
}

void X86Asm::IfBitSet(Reg32 reg, U32 mask, bool bigJump) {
    test(reg, mask);
    if (bigJump) {
        outb(0x0f);        
        outb(0x84); // jz
        ifJump.push_back(buffer.size());
        outd(0); // jump over amount
    } else {
        outb(0x74); // jz
        ifJump.push_back(buffer.size());
        outb(0); // jump over amount
    }
}

void X86Asm::IfNotBitSet(Reg32 reg, U32 mask, bool bigJump) {
    test(reg, mask);
    if (bigJump) {
        outb(0x0f);
        outb(0x85); // jnz
        ifJump.push_back(buffer.size());
        outd(0); // jump over amount
    } else {
        outb(0x75); // jnz
        ifJump.push_back(buffer.size());
        outb(0); // jump over amount
    }
}

void X86Asm::Else(bool bigJump) {
    U32 pos;

    if (bigJump) {       
        outb(0xe9);
        pos = buffer.size();
        outd(0); // jump over amount
    } else {
        outb(0xeb); // previous block should jump over else statement
        pos = buffer.size();
        outb(0); // jump over amount
    }

    // if statement will jump here if it wasn't true
    EndIf(bigJump);

    ifJump.push_back(pos);
}

void X86Asm::EndIf(bool bigJump) {
    U32 pos = ifJump.back();
    if (bigJump) {
        U32 amount = buffer.size() - pos - 4;
        ifJump.pop_back();
        *(U16*)&buffer.data()[pos] = (U16)(amount);
    } else {
        U32 amount = buffer.size() - pos - 1;
        if (amount > 127) {
            kpanic_fmt("X86Asm::EndIf large if/else blocks not supported: %d", amount);
        }
        ifJump.pop_back();
        buffer[pos] = (U8)(amount);
    }
}

void X86Asm::setz(Reg8 reg) {
    outb(0x0f);
    outb(0x94);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnz(Reg8 reg) {
    outb(0x0f);
    outb(0x95);
    outb(0xc0 + reg.reg);
}

void X86Asm::setb(Reg8 reg) {
    outb(0x0f);
    outb(0x92);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnb(Reg8 reg) {
    outb(0x0f);
    outb(0x93);
    outb(0xc0 + reg.reg);
}

void X86Asm::setbe(Reg8 reg) {
    outb(0x0f);
    outb(0x96);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnbe(Reg8 reg) {
    outb(0x0f);
    outb(0x97);
    outb(0xc0 + reg.reg);
}

void X86Asm::setl(Reg8 reg) {
    outb(0x0f);
    outb(0x9c);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnl(Reg8 reg) {
    outb(0x0f);
    outb(0x9d);
    outb(0xc0 + reg.reg);
}

void X86Asm::setle(Reg8 reg) {
    outb(0x0f);
    outb(0x9e);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnle(Reg8 reg) {
    outb(0x0f);
    outb(0x9f);
    outb(0xc0 + reg.reg);
}

void X86Asm::seto(Reg8 reg) {
    outb(0x0f);
    outb(0x90);
    outb(0xc0 + reg.reg);
}

void X86Asm::setno(Reg8 reg) {
    outb(0x0f);
    outb(0x91);
    outb(0xc0 + reg.reg);
}

void X86Asm::sets(Reg8 reg) {
    outb(0x0f);
    outb(0x98);
    outb(0xc0 + reg.reg);
}

void X86Asm::setns(Reg8 reg) {
    outb(0x0f);
    outb(0x99);
    outb(0xc0 + reg.reg);
}

void X86Asm::setp(Reg8 reg) {
    outb(0x0f);
    outb(0x9a);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnp(Reg8 reg) {
    outb(0x0f);
    outb(0x9b);
    outb(0xc0 + reg.reg);
}

void X86Asm::movlhps(RegXMM hiDstXMM, RegXMM loSrcXMM) {
    outb(0x0f);
    outb(0x16);
    outb(0xC0 | (hiDstXMM.reg << 3) | loSrcXMM.reg);
}

void X86Asm::movhlps(RegXMM hiDstXMM, RegXMM loSrcXMM) {
    outb(0x0f);
    outb(0x12);
    outb(0xC0 | (hiDstXMM.reg << 3) | loSrcXMM.reg);
}

void X86Asm::movss(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM srcXMM) {
    outb(0xf3);
    outb(0x0f);
    mem32(0x11, Reg32(srcXMM.reg), rm, sib, lsl, disp);
}

void X86Asm::movss(RegXMM dstXMM, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf3);
    outb(0x0f);
    mem32(0x10, Reg32(dstXMM.reg), rm, sib, lsl, disp);
}

void X86Asm::movss(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf3);
    outb(0x0f);
    outb(0x10);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::movsd(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x11, Reg32(srcXMM.reg), rm, sib, lsl, disp);
}

void X86Asm::movsd(RegXMM dstXMM, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x10, Reg32(dstXMM.reg), rm, sib, lsl, disp);
}

void X86Asm::movsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x10);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::movsd(RegXMM dstXMM, Reg32 rm, U32 disp) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x10, Reg32(dstXMM.reg), rm, disp);
}

void X86Asm::movlpd(RegXMM dstXMM, Reg32 rm, U32 disp) {
    outb(0x66);
    outb(0x0f);
    mem32(0x12, Reg32(dstXMM.reg), rm, disp);
}

void X86Asm::cvtss2sd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::cvtsd2ss(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::xorpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x57);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::andpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x54);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::cvtsi2sd(RegXMM dstXMM, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x2a, Reg32(dstXMM.reg), rm, sib, lsl, disp);
}

void X86Asm::cvttsd2si(Reg32 dst, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x2c);
    outb(0xC0 | (dst.reg << 3) | srcXMM.reg);
}

void X86Asm::cvtsd2si(Reg32 dst, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x2d);
    outb(0xC0 | (dst.reg << 3) | srcXMM.reg);
}

void X86Asm::cvtsi2sd(RegXMM dstXMM, Reg32 reg) {
    outb(0xf2);
    outb(0x0f);
    outb(0x2a);
    outb(0xC0 | (dstXMM.reg << 3) | reg.reg);
}

void X86Asm::cvtdq2pd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf3);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::cvtpd2dq(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::cvttpd2dq(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::cvtpd2ps(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::addpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::addsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::mulpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::mulsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::subpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::subsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::sqrtpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::sqrtsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | (dstXMM.reg << 3) | srcXMM.reg);
}

void X86Asm::ucomisd(RegXMM xmm1, RegXMM xmm2) {
    outb(0x66);
    outb(0x0f);
    outb(0x2e);
    outb(0xc0 | xmm2.reg | (xmm1.reg << 3));
}

void X86Asm::stmxcsr(Reg32 rm, U32 disp) {
    outb(0x0f);
    outb(0xae);
    if (!disp) {
        outb(0x18 | rm.reg);
    } else {
        S32 sIMM = (S32)disp;
        if (sIMM < -128 || sIMM > 127) {
            outb(0x80 | 0x18 | rm.reg);
            outd(disp);
        } else {
            outb(0x40 | 0x18 | rm.reg);
            outb(disp);
        }
    }
}

void X86Asm::ldmxcsr(Reg32 rm, U32 disp) {
    outb(0x0f);
    outb(0xae);
    if (!disp) {
        outb(0x10 | rm.reg);
    } else {
        S32 sIMM = (S32)disp;
        if (sIMM < -128 || sIMM > 127) {
            outb(0x80 | 0x10 | rm.reg);
            outd(disp);
        } else {
            outb(0x40 | 0x10 | rm.reg);
            outb(disp);
        }
    }
}

void X86Asm::rdtsc() {
    outb(0x0f);
    outb(0x31);
}

void X86Asm::movd(RegMMX dst, Reg32 src) {
    outb(0x0f);
    outb(0x6e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movd(Reg32 dst, RegMMX src) {
    outb(0x0f);
    outb(0x7e);
    outb(0xC0 | (src.reg << 3) | dst.reg); // wonder why this is reversed where most instruction put dst at the top and src at the bottom
}

void X86Asm::movd(Reg32 rm, U32 disp, RegMMX reg) {
    outb(0x0f);
    mem32(0x7e, Reg32(reg.reg), rm, disp);
}

void X86Asm::movd(RegMMX reg, Reg32 rm, U32 disp) {
    outb(0x0f);
    mem32(0x6e, Reg32(reg.reg), rm, disp);
}

void X86Asm::emms() {
    outb(0x0f);
    outb(0x77);
}

void X86Asm::movaps(RegXMM reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x0f);
    mem32(0x28, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movaps(RegXMM reg, Reg32 rm, U32 disp) {
    outb(0x0f);
    mem32(0x28, Reg32(reg.reg), rm, disp);
}

void X86Asm::movaps(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM reg) {
    outb(0x0f);
    mem32(0x29, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movaps(Reg32 rm, U32 disp, RegXMM reg) {
    outb(0x0f);
    mem32(0x29, Reg32(reg.reg), rm, disp);
}

void X86Asm::movups(RegXMM reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x0f);
    mem32(0x10, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movups(RegXMM reg, Reg32 rm, U32 disp) {
    outb(0x0f);
    mem32(0x10, Reg32(reg.reg), rm, disp);
}

void X86Asm::movups(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM reg) {
    outb(0x0f);
    mem32(0x11, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movhps(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM reg) {
    outb(0x0f);
    mem32(0x17, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movlps(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM reg) {
    outb(0x0f);
    mem32(0x13, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movhps(RegXMM reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x0f);
    mem32(0x16, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movlps(RegXMM reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x0f);
    mem32(0x12, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movups(Reg32 rm, U32 disp, RegXMM reg) {
    outb(0x0f);
    mem32(0x11, Reg32(reg.reg), rm, disp);
}

void X86Asm::movd(RegXMM dst, Reg32 src) {
    outb(0x66);
    outb(0x0f);
    outb(0x6e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movd(Reg32 dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x7e);
    outb(0xC0 | (src.reg << 3) | dst.reg); // wonder why this is reversed where most instruction put dst at the top and src at the bottom
}

void X86Asm::movd(RegXMM reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x66);
    outb(0x0f);
    mem32(0x6e, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movq(RegXMM reg, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf3);
    outb(0x0f);
    mem32(0x7e, Reg32(reg.reg), rm, sib, lsl, disp);
}

void X86Asm::movq(RegXMM reg, Reg32 rm, U32 disp) {
    outb(0xf3);
    outb(0x0f);
    mem32(0x7e, Reg32(reg.reg), rm, disp);
}

void X86Asm::movq(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    mem32(0xd6, Reg32(src.reg), rm, sib, lsl, disp);
}

void X86Asm::movq(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x7e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movdqu(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x6f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movq(Reg32 rm, U32 disp, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    mem32(0xd6, Reg32(src.reg), rm, disp);
}

void X86Asm::movd(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    mem32(0x7e, Reg32(src.reg), rm, sib, lsl, disp);
}

void X86Asm::pxor(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xef);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::por(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xeb);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pand(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xdb);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pandn(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xdf);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psllw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf1);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psllw(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x71);
    outb(0xf0 | dst.reg);
    outb(imm);
}

void X86Asm::psrlw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd1);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psrlw(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x71);
    outb(0xd0 | dst.reg);
    outb(imm);
}

void X86Asm::psraw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe1);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psraw(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x71);
    outb(0xe0 | dst.reg);
    outb(imm);
}

void X86Asm::pslld(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf2);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pslld(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x72);
    outb(0xf0 | dst.reg);
    outb(imm);
}

void X86Asm::psrld(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd2);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psrld(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x72);
    outb(0xd0 | dst.reg);
    outb(imm);
}

void X86Asm::psrad(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe2);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psrad(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x72);
    outb(0xe0 | dst.reg);
    outb(imm);
}

void X86Asm::psllq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf3);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psllq(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x73);
    outb(0xf0 | dst.reg);
    outb(imm);
}

void X86Asm::pslldq(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x73);
    outb(0xf8 | dst.reg);
    outb(imm);
}

void X86Asm::psrlq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd3);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psrlq(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x73);
    outb(0xd0 | dst.reg);
    outb(imm);
}

void X86Asm::psrldq(RegXMM dst, U32 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0x73);
    outb(0xd8 | dst.reg);
    outb(imm);
}

void X86Asm::paddb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xfc);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xfd);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xfe);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd4);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddsb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xec);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xed);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddusb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xdc);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::paddusw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xdd);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf8);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf9);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xfa);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xfb);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubsb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe8);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe9);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubusb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd8);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psubusw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd9);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmulhw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe5);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmullw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd5);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmaddwd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf5);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmuludq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf4);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pcmpeqb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x74);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pcmpeqw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x75);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pcmpeqd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x76);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pcmpgtb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x64);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pcmpgtw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x65);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pcmpgtd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x66);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::packsswb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x63);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::packssdw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x6b);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::packuswb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x67);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpckhbw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x68);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpckhwd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x69);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpckhdq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x6a);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpcklbw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x60);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpcklwd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x61);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpckldq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x62);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::unpcklpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x14);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::unpckhpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x15);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpckhqdq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x6d);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::punpcklqdq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x6c);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::addps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::addss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::subps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::subss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::mulps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::mulss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::divps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::divpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::divss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::divsd(RegXMM dst, RegXMM src) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::rcpps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x53);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::rcpss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x53);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::sqrtps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::sqrtss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::rsqrtps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x52);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::rsqrtss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x52);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::maxps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::maxpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::maxss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::maxsd(RegXMM dst, RegXMM src) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::minps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::minpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::minss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::minsd(RegXMM dst, RegXMM src) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pavgb(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe0);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pavgw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe3);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::psadbw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xf6);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pextrw(Reg32 dst, RegXMM src, U8 srcIndex) {
    outb(0x66);
    outb(0x0f);
    outb(0xc5);
    outb(0xC0 | (dst.reg << 3) | src.reg);
    outb(srcIndex);
}

void X86Asm::pinsrw(RegXMM dst, Reg32 src, U8 dstIndex) {
    outb(0x66);
    outb(0x0f);
    outb(0xc4);
    outb(0xC0 | (dst.reg << 3) | src.reg);
    outb(dstIndex);
}

void X86Asm::pmaxsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xee);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmaxub(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xde);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pminsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xea);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pminub(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xda);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmovmskb(Reg32 dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xd7);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pmulhuw(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0xe4);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::pshuflw(RegXMM dst, RegXMM src, U8 order) {
    outb(0xf2);
    outb(0x0f);
    outb(0x70);
    outb(0xC0 | (dst.reg << 3) | src.reg);
    outb(order);
}

void X86Asm::pshufhw(RegXMM dst, RegXMM src, U8 order) {
    outb(0xf3);
    outb(0x0f);
    outb(0x70);
    outb(0xC0 | (dst.reg << 3) | src.reg);
    outb(order);
}

void X86Asm::pshufd(RegXMM dst, RegXMM src, U8 order) {
    outb(0x66);
    outb(0x0f);
    outb(0x70);
    outb(0xC0 | (dst.reg << 3) | src.reg);
    outb(order);
}

void X86Asm::shufpd(RegXMM dst, RegXMM src, U8 order) {
    outb(0x66);
    outb(0x0f);
    outb(0xc6);
    outb(0xC0 | (dst.reg << 3) | src.reg);
    outb(order);
}

void X86Asm::andnps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x55);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::andnpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x55);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::andps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x54);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::orpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x56);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::orps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x56);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::xorps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x57);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvtdq2ps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x5b);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvtps2dq(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x5b);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvtps2pd(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvttps2dq(RegXMM dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5b);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvtsi2ss(RegXMM dst, Reg32 src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x2a);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvtss2si(Reg32 dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x2d);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::cvttss2si(Reg32 dst, RegXMM src) {
    outb(0xf3);
    outb(0x0f);
    outb(0x2c);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::movmskps(Reg32 dst, RegXMM src) {
    outb(0x0f);
    outb(0x50);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::maskmovdqu(RegXMM src, RegXMM mask) {
    outb(0x66);
    outb(0x0f);
    outb(0xf7);
    outb(0xC0 | (src.reg << 3) | mask.reg);
}

void X86Asm::shufps(RegXMM src, RegXMM mask, U8 imm) {
    outb(0x0f);
    outb(0xc6);
    outb(0xC0 | (src.reg << 3) | mask.reg);
    outb(imm);
}

void X86Asm::cmppd(RegXMM src, RegXMM mask, U8 imm) {
    outb(0x66);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | (src.reg << 3) | mask.reg);
    outb(imm);
}

void X86Asm::cmpps(RegXMM src, RegXMM mask, U8 imm) {
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | (src.reg << 3) | mask.reg);
    outb(imm);
}

void X86Asm::cmpsd(RegXMM src, RegXMM mask, U8 imm) {
    outb(0xf2);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | (src.reg << 3) | mask.reg);
    outb(imm);
}

void X86Asm::cmpss(RegXMM src, RegXMM mask, U8 imm) {
    outb(0xf3);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | (src.reg << 3) | mask.reg);
    outb(imm);
}

void X86Asm::unpckhps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x15);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::unpcklps(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x14);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::comisd(RegXMM dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x2f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::comiss(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x2f);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::ucomiss(RegXMM dst, RegXMM src) {
    outb(0x0f);
    outb(0x2e);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::sfence() {
    outb(0x0f);
    outb(0xae);
    outb(0xf8);
}

void X86Asm::lfence() {
    outb(0x0f);
    outb(0xae);
    outb(0xe8);
}

void X86Asm::mfence() {
    outb(0x0f);
    outb(0xae);
    outb(0xf0);
}

void X86Asm::pause() {
    outb(0xf3);
    outb(0x90);
}

void X86Asm::movmskpd(Reg32 dst, RegXMM src) {
    outb(0x66);
    outb(0x0f);
    outb(0x50);
    outb(0xC0 | (dst.reg << 3) | src.reg);
}

void X86Asm::clflush(Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0x0f);
    mem32(0xae, Reg32(7), rm, sib, lsl, disp);
}

#endif