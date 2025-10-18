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

void X86Asm::sub(Reg32 dst, U32 imm) {
    group1(0x2d, 5, dst, imm);
}

void X86Asm::sub(Reg16 dst, U16 imm) {
    group1(0x2d, 5, dst, imm);
}

void X86Asm::sub(Reg8 dst, U8 imm) {
    group1(0x2c, 5, dst, imm);
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
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::movsx(Reg32 dst, Reg8 src) {
    outb(0x0f);
    outb(0xbe);
    outb(0xC0 | (src.reg << 3) | dst.reg);
}

void X86Asm::movsx(Reg32 dst, Reg16 src) {
    outb(0x0f);
    outb(0xbf);
    outb(0xC0 | (src.reg << 3) | dst.reg);
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

void X86Asm::jz(U32 address) {
    outb(0xf);
    outb(0x84);
    if (address == 0x0045a821) {
        int ii = 0;
    }
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

void X86Asm::jnz(U32 address) {
    outb(0xf);
    outb(0x85);
    if (address == 0x0045a821) {
        int ii = 0;
    }
    jumps.push_back(DynamicJump(address, buffer.size()));
    outd(0); // jump over amount
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

void X86Asm::IfEqual(Reg32 reg, U32 value) {
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

void X86Asm::IfNotZero(Reg32 reg) {
    test(reg, reg);
    outb(0x74); // jz
    ifJump.push_back(buffer.size());
    outb(0); // jump over amount
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

void X86Asm::setbe(Reg8 reg) {
    outb(0x0f);
    outb(0x96);
    outb(0xc0 + reg.reg);
}

void X86Asm::setnb(Reg8 reg) {
    outb(0x0f);
    outb(0x93);
    outb(0xc0 + reg.reg);
}

void X86Asm::setl(Reg8 reg) {
    outb(0x0f);
    outb(0x9c);
    outb(0xc0 + reg.reg);
}

void X86Asm::setle(Reg8 reg) {
    outb(0x0f);
    outb(0x9e);
    outb(0xc0 + reg.reg);
}

void X86Asm::movss(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 srcXMM) {
    outb(0xf3);
    outb(0x0f);
    mem32(0x11, Reg32(srcXMM), rm, sib, lsl, disp);
}

void X86Asm::movss(U8 dstXMM, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf3);
    outb(0x0f);
    mem32(0x10, Reg32(dstXMM), rm, sib, lsl, disp);
}

void X86Asm::movsd(Reg32 rm, Reg32 sib, U8 lsl, U32 disp, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x11, Reg32(srcXMM), rm, sib, lsl, disp);
}

void X86Asm::movsd(U8 dstXMM, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x10, Reg32(dstXMM), rm, sib, lsl, disp);
}

void X86Asm::movsd(U8 dstXMM, Reg32 rm, U32 disp) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x10, Reg32(dstXMM), rm, disp);
}

void X86Asm::movlpd(U8 dstXMM, Reg32 rm, U32 disp) {
    outb(0x66);
    outb(0x0f);
    mem32(0x12, Reg32(dstXMM), rm, disp);
}

void X86Asm::cvtss2sd(U8 dstXMM, U8 srcXMM) {
    outb(0xf3);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::cvtsd2ss(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::xorpd(U8 dstXMM, U8 srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x57);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::andpd(U8 dstXMM, U8 srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0x54);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::cvtsi2sd(U8 dstXMM, Reg32 rm, Reg32 sib, U8 lsl, U32 disp) {
    outb(0xf2);
    outb(0x0f);
    mem32(0x2a, Reg32(dstXMM), rm, sib, lsl, disp);
}

void X86Asm::cvttsd2si(Reg32 dst, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x2c);
    outb(0xC0 | (dst.reg << 3) | srcXMM);
}

void X86Asm::cvtsd2si(Reg32 dst, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x2d);
    outb(0xC0 | (dst.reg << 3) | srcXMM);
}

void X86Asm::cvtsi2sd(U8 dstXMM, Reg32 reg) {
    outb(0xf2);
    outb(0x0f);
    outb(0x2a);
    outb(0xC0 | (dstXMM << 3) | reg.reg);
}

void X86Asm::cvtdq2pd(U8 dstXMM, U8 srcXMM) {
    outb(0xf3);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::cvtpd2dq(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::cvttpd2dq(U8 dstXMM, U8 srcXMM) {
    outb(0x66);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::addsd(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::mulsd(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::subsd(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::divsd(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::sqrtsd(U8 dstXMM, U8 srcXMM) {
    outb(0xf2);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | (dstXMM << 3) | srcXMM);
}

void X86Asm::ucomisd(U8 xmm1, U8 xmm2) {
    outb(0x66);
    outb(0x0f);
    outb(0x2e);
    outb(0xc0 | xmm1 | (xmm2 << 3));
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
#endif