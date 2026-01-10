#include "boxedwine.h"

#if defined(BOXEDWINE_DYNAMIC32) || defined(BOXEDWINE_JIT_X64)

#include "x86Asm.h"

X86Asm::Reg8 X86Asm::al = X86Asm::Reg8::from(0);
X86Asm::Reg8 X86Asm::cl = X86Asm::Reg8::from(1);
X86Asm::Reg8 X86Asm::dl = X86Asm::Reg8::from(2);
X86Asm::Reg8 X86Asm::bl = X86Asm::Reg8::from(3);
X86Asm::Reg8 X86Asm::ah = X86Asm::Reg8::from(4);
X86Asm::Reg8 X86Asm::ch = X86Asm::Reg8::from(5);
X86Asm::Reg8 X86Asm::dh = X86Asm::Reg8::from(6);
X86Asm::Reg8 X86Asm::bh = X86Asm::Reg8::from(7);

X86Asm::Reg16 X86Asm::ax = X86Asm::Reg16::from(0);
X86Asm::Reg16 X86Asm::cx = X86Asm::Reg16::from(1);
X86Asm::Reg16 X86Asm::dx = X86Asm::Reg16::from(2);
X86Asm::Reg16 X86Asm::bx = X86Asm::Reg16::from(3);
X86Asm::Reg16 X86Asm::sp = X86Asm::Reg16::from(4);
X86Asm::Reg16 X86Asm::bp = X86Asm::Reg16::from(5);
X86Asm::Reg16 X86Asm::si = X86Asm::Reg16::from(6);
X86Asm::Reg16 X86Asm::di = X86Asm::Reg16::from(7);

X86Asm::Reg32 X86Asm::eax = X86Asm::Reg32::from(0);
X86Asm::Reg32 X86Asm::ecx = X86Asm::Reg32::from(1);
X86Asm::Reg32 X86Asm::edx = X86Asm::Reg32::from(2);
X86Asm::Reg32 X86Asm::ebx = X86Asm::Reg32::from(3);
X86Asm::Reg32 X86Asm::esp = X86Asm::Reg32::from(4);
X86Asm::Reg32 X86Asm::ebp = X86Asm::Reg32::from(5);
X86Asm::Reg32 X86Asm::esi = X86Asm::Reg32::from(6);
X86Asm::Reg32 X86Asm::edi = X86Asm::Reg32::from(7);

#ifdef BOXEDWINE_64
X86Asm::Reg64 X86Asm::rax = X86Asm::Reg64::from(0);
X86Asm::Reg64 X86Asm::rcx = X86Asm::Reg64::from(1);
X86Asm::Reg64 X86Asm::rdx = X86Asm::Reg64::from(2);
X86Asm::Reg64 X86Asm::rbx = X86Asm::Reg64::from(3);
X86Asm::Reg64 X86Asm::rsp = X86Asm::Reg64::from(4);
X86Asm::Reg64 X86Asm::rbp = X86Asm::Reg64::from(5);
X86Asm::Reg64 X86Asm::rsi = X86Asm::Reg64::from(6);
X86Asm::Reg64 X86Asm::rdi = X86Asm::Reg64::from(7);
X86Asm::Reg64 X86Asm::r8 = X86Asm::Reg64::from(8);
X86Asm::Reg64 X86Asm::r9 = X86Asm::Reg64::from(9);
X86Asm::Reg64 X86Asm::r10 = X86Asm::Reg64::from(10);
X86Asm::Reg64 X86Asm::r11 = X86Asm::Reg64::from(11);
X86Asm::Reg64 X86Asm::r12 = X86Asm::Reg64::from(12);
X86Asm::Reg64 X86Asm::r13 = X86Asm::Reg64::from(13);
X86Asm::Reg64 X86Asm::r14 = X86Asm::Reg64::from(14);
X86Asm::Reg64 X86Asm::r15 = X86Asm::Reg64::from(15);

#define REX_BASE 0x40
#define REX_MOD_RM 0x1
#define REX_SIB_INDEX 0x2
#define REX_MOD_REG 0x4
#define REX_64 0x8

#endif

bool operator==(const X86Asm::Reg32& lhs, const X86Asm::Reg32& rhs) {
    return lhs.reg == rhs.reg;
}

void X86Asm::reset() {
    buffer.clear();
#ifndef BOXEDWINE_64
    patch.clear();
#endif
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

void X86Asm::outq(U64 q) {
    outd((U32)q);
    outd((U32)(q >> 32));
}

void X86Asm::outd(U32 d) {
    outb((U8)d);
    outb((U8)(d >> 8));
    outb((U8)(d >> 16));
    outb((U8)(d >> 24));
}

void X86Asm::lea(Reg32 dst, Mem mem) {
    mem32(0x8d, dst.reg, mem);   
}

void X86Asm::lahf() {
    outb(0x9f);
}

void X86Asm::sahf() {
    outb(0x9e);
}

void X86Asm::bswap(Reg32 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xc8 + (reg.reg & 7));
}

void X86Asm::mem32(U32 inst, U8 dst, const Mem& mem, bool is64) {    
    if (mem.sib.reg != 0xff && mem.rm.reg == 0xff && mem.lsl == 0) {
        mem32(inst, dst, Mem(mem.sib, mem.disp), is64);
        return;
    }
    rex(dst, mem.rm.reg == 0xff ? 0 : mem.rm.reg, mem.sib.reg == 0xff ? 0 : mem.sib.reg, is64);
    if (inst > 0xff) {
        outb(0x0f);
        inst -= 0x100;
    }
    if (mem.rm.reg != 0xff && mem.sib.reg != 0xff) {
        if (mem.lsl > 3) {
            kpanic("X86Asm::mem32 shift can not be greater than 3");
        }
        S32 sIMM = (S32)mem.disp;
        bool small = true;
        if (sIMM < -128 || sIMM > 127) {
            small = false;
        }

        outb(inst);
        bool hasDisp = mem.disp != 0;
        if (!hasDisp && mem.rm.reg == 5) {
            hasDisp = true;
        }
        if (!hasDisp) {
            outb(0x04 | ((dst & 7) << 3));
        } else if (small) {
            outb(0x44 | ((dst & 7) << 3));
        } else {
            outb(0x84 | ((dst & 7) << 3));
        }
        if (mem.lsl == 0) {
            outb(((mem.sib.reg & 7) << 3) | (mem.rm.reg & 7));
        } else {
            outb((mem.lsl << 6) | ((mem.sib.reg & 7) << 3) | (mem.rm.reg & 7));
        }
        if (hasDisp) {
            if (small) {
                outb(mem.disp);
            } else {
                outd(mem.disp);
            }
        }
    } else if (mem.rm.reg != 0xff) {
        if (!mem.disp) {
            outb(inst);
            outb(0x00 | ((dst & 7) << 3) | (mem.rm.reg & 7));
            if ((mem.rm.reg & 7) == 4) {
                // see https://wiki.osdev.org/X86-64_Instruction_Encoding#ModR/M_and_SIB_bytes
                outb((4 << 3) | (mem.rm.reg & 7));
            }
        } else {
            S32 sIMM = (S32)mem.disp;
            if (sIMM < -128 || sIMM > 127) {
                outb(inst);
                outb(0x80 | ((dst & 7) << 3) | (mem.rm.reg & 7));
                if ((mem.rm.reg & 7) == 4) {
                    // see https://wiki.osdev.org/X86-64_Instruction_Encoding#ModR/M_and_SIB_bytes
                    outb((4 << 3) | (mem.rm.reg & 7));
                }
                outd(mem.disp);
            } else {
                outb(inst);
                outb(0x40 | ((dst & 7) << 3) | (mem.rm.reg & 7));
                if ((mem.rm.reg & 7) == 4) {
                    outb((4 << 3) | (mem.rm.reg & 7));
                }
                outb(mem.disp);
            }
        }
    } else if (mem.sib.reg != 0xff) {
        if (mem.lsl) {
            // without rm, there is only one way to encode this

            outb(inst);
            outb(0x04 | ((dst & 7) << 3));
            outb((mem.lsl << 6) | ((mem.sib.reg & 7) << 3) | 0x05); // 5 is sib with disp and no rm
            outd(mem.disp); // even if 0, there is no other way to encode this
        } else if (!mem.disp) {
            outb(inst);
            outb(0x00 | ((dst & 7) << 3) | (mem.sib.reg & 7));
        } else {
            S32 sIMM = (S32)mem.disp;
            if (sIMM < -128 || sIMM > 127) {
                outb(inst);
                outb(0x80 | ((dst & 7) << 3) | (mem.sib.reg & 7));
                outd(mem.disp);
            } else {
                outb(inst);
                outb(0x40 | ((dst & 7) << 3) | (mem.sib.reg & 7));
                outb(mem.disp);
            }
        }
    } else {
        outb(inst);
        outb(0x05 | ((dst & 7) << 3));
        outd(mem.disp);
    }    
}

void X86Asm::mem16(U32 inst, U8 dst, const Mem& mem) {
    mem32(inst, dst, mem);
}

void X86Asm::mem8(U32 inst, U8 dst, const Mem& mem) {
    mem32(inst, dst, mem);
}

void X86Asm::add(const Mem32& mem, Reg32 reg) {
    mem32(0x01, reg.reg, mem);
}

void X86Asm::add(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x01, reg.reg, mem);
}

void X86Asm::add(const Mem8& mem, Reg8 reg) {
    mem8(0x00, reg.reg, mem);
}

void X86Asm::add(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 0, mem);
        outd(imm);
    } else {
        mem32(0x83, 0, mem);
        outb((U8)imm);
    }
}

void X86Asm::add(const Mem16& mem, U16 imm) {
    S32 sIMM = (S32)imm;
    outb(0x66);
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 0, mem);
        outw(imm);
    } else {
        mem32(0x83, 0, mem);
        outb((U8)imm);
    }
}

void X86Asm::add(const Mem8& mem, U8 imm) {
    mem32(0x80, 0, mem);
    outb((U8)imm);
}

void X86Asm::sub(const Mem32& mem, Reg32 reg) {
    mem32(0x29, reg.reg, mem);
}

void X86Asm::sub(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x29, reg.reg, mem);
}

void X86Asm::sub(const Mem8& mem, Reg8 reg) {
    mem8(0x28, reg.reg, mem);
}

void X86Asm::sub(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 5, mem);
        outd(imm);
    } else {
        mem32(0x83, 5, mem);
        outb((U8)imm);
    }
}

void X86Asm::sub(const Mem16& mem, U16 imm) {
    S32 sIMM = (S32)imm;
    outb(0x66);
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 5, mem);
        outw(imm);
    } else {
        mem32(0x83, 5, mem);
        outb((U8)imm);
    }
}

void X86Asm::sub(const Mem8& mem, U8 imm) {
    mem32(0x80, 5, mem);
    outb((U8)imm);
}

void X86Asm::or_(const Mem32& mem, Reg32 reg) {
    mem32(0x09, reg.reg, mem);
}

void X86Asm::or_(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x09, reg.reg, mem);
}

void X86Asm::or_(const Mem8& mem, Reg8 reg) {
    mem8(0x08, reg.reg, mem);
}

void X86Asm::or_(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 1, mem);
        outd(imm);
    } else {
        mem32(0x83, 1, mem);
        outb((U8)imm);
    }
}

void X86Asm::or_(const Mem16& mem, U16 imm) {
    S32 sIMM = (S32)imm;
    outb(0x66);
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 1, mem);
        outw(imm);
    } else {
        mem32(0x83, 1, mem);
        outb((U8)imm);
    }
}

void X86Asm::or_(const Mem8& mem, U8 imm) {
    mem32(0x80, 1, mem);
    outb((U8)imm);
}

void X86Asm::cmp(const Mem32& mem, Reg32 reg) {
    mem32(0x39, reg.reg, mem);
}

void X86Asm::cmp(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x39, reg.reg, mem);
}

void X86Asm::cmp(const Mem8& mem, Reg8 reg) {
    mem8(0x38, reg.reg, mem);
}

void X86Asm::cmp(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 7, mem);
        outd(imm);
    } else {
        mem32(0x83, 7, mem);
        outb((U8)imm);
    }    
}

void X86Asm::cmp(const Mem16& mem, U16 imm) {
    outb(0x66);
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem16(0x81, 7, mem);
        outw(imm);
    } else {
        mem16(0x83, 7, mem);
        outb((U8)imm);
    }
}

void X86Asm::cmp(const Mem8& mem, U8 imm) {
    mem8(0x80, 7, mem);
    outb(imm);
}

void X86Asm::and_(const Mem32& mem, Reg32 reg) {
    mem32(0x21, reg.reg, mem);
}

void X86Asm::and_(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x21, reg.reg, mem);
}

void X86Asm::and_(const Mem8& mem, Reg8 reg) {
    mem8(0x20, reg.reg, mem);
}

void X86Asm::and_(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 4, mem);
        outd(imm);
    } else {
        mem32(0x83, 4, mem);
        outb(imm);
    }
}

void X86Asm::and_(const Mem16& mem, U16 imm) {
    S32 sIMM = (S32)imm;
    outb(0x66);
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 4, mem);
        outw(imm);
    } else {
        mem32(0x83, 4, mem);
        outb((U8)imm);
    }
}

void X86Asm::and_(const Mem8& mem, U8 imm) {
    mem32(0x80, 4, mem);
    outb((U8)imm);
}

void X86Asm::xor_(const Mem32& mem, Reg32 reg) {
    mem32(0x31, reg.reg, mem);
}

void X86Asm::xor_(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x31, reg.reg, mem);
}

void X86Asm::xor_(const Mem8& mem, Reg8 reg) {
    mem8(0x30, reg.reg, mem);
}

void X86Asm::xor_(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 6, mem);
        outd(imm);
    } else {
        mem32(0x83, 6, mem);
        outb(imm);
    }
}

void X86Asm::xor_(const Mem16& mem, U16 imm) {
    S32 sIMM = (S32)imm;
    outb(0x66);
    if (sIMM < -128 || sIMM > 127) {
        mem32(0x81, 6, mem);
        outw(imm);
    } else {
        mem32(0x83, 6, mem);
        outb((U8)imm);
    }
}

void X86Asm::xor_(const Mem8& mem, U8 imm) {
    mem32(0x80, 6, mem);
    outb((U8)imm);
}

void X86Asm::not_(const Mem32& mem) {
    mem32(0xf7, 2, mem);
}

void X86Asm::not_(const Mem16& mem) {
    outb(0x66);
    mem16(0xf7, 2, mem);
}

void X86Asm::not_(const Mem8& mem) {
    mem8(0xf6, 2, mem);
}

void X86Asm::neg(const Mem32& mem) {
    mem32(0xf7, 3, mem);
}

void X86Asm::neg(const Mem16& mem) {
    outb(0x66);
    mem16(0xf7, 3, mem);
}

void X86Asm::neg(const Mem8& mem) {
    mem32(0xf6, 3, mem);
}

void X86Asm::inc(const Mem32& mem) {
    mem32(0xff, 0, mem);
}

void X86Asm::inc(const Mem16& mem) {
    outb(0x66);
    mem16(0xff, 0, mem);
}

void X86Asm::inc(const Mem8& mem) {
    mem8(0xfe, 0, mem);
}

void X86Asm::dec(const Mem32& mem) {
    mem32(0xff, 1, mem);
}

void X86Asm::dec(const Mem16& mem) {
    outb(0x66);
    mem16(0xff, 1, mem);
}

void X86Asm::dec(const Mem8& mem) {
    mem8(0xfe, 1, mem);
}

void X86Asm::bt(const Mem32& mem, U8 value) {
    mem32(0x1ba, 4, mem);
    outb(value);
}

void X86Asm::bts(const Mem32& mem, U8 value) {
    mem32(0x1ba, 5, mem);
    outb(value);
}

void X86Asm::bts(const Mem16& mem, U8 value) {
    mem16(0x1ba, 5, mem);
    outb(value);
}

void X86Asm::bts(const Mem32& mem, Reg32 value) {
    mem32(0x1ab, value.reg, mem);
}

void X86Asm::bts(const Mem16& mem, Reg16 value) {
    outb(0x66);
    mem16(0x1ab, value.reg, mem);
}

void X86Asm::btr(const Mem32& mem, U8 value) {
    mem32(0x1ba, 6, mem);
    outb(value);
}

void X86Asm::btr(const Mem16& mem, U8 value) {
    outb(0x66);
    mem16(0x1ba, 6, mem);
    outb(value);
}

void X86Asm::btr(const Mem32& mem, Reg32 value) {
    mem32(0x1b3, value.reg, mem);
}

void X86Asm::btr(const Mem16& mem, Reg16 value) {
    outb(0x66);
    mem16(0x1b3, value.reg, mem);
}

void X86Asm::btc(const Mem32& mem, U8 value) {
    mem32(0x1ba, 7, mem);
    outb(value);
}

void X86Asm::btc(const Mem16& mem, U8 value) {
    outb(0x66);
    mem16(0x1ba, 7, mem);
    outb(value);
}

void X86Asm::btc(const Mem32& mem, Reg32 value) {
    mem32(0x1bb, value.reg, mem);
}

void X86Asm::btc(const Mem16& mem, Reg16 value) {
    outb(0x66);
    mem16(0x1bb, value.reg, mem);
}

void X86Asm::mov(Reg32 dst, U32 imm) {
    rex(dst.reg, false);
    outb(0xb8 + (dst.reg & 7));
    outd(imm);
}

void X86Asm::mov(Reg16 dst, U16 imm) {    
    outb(0x66);
    rex(dst.reg, false);
    outb(0xb8 + (dst.reg & 7));
    outw(imm);
}

void X86Asm::mov(Reg8 dst, U8 imm) {
    rex(dst.reg, false);
    outb(0xb0 + (dst.reg & 7));
    outb(imm);
}

void X86Asm::mov(Reg32 dst, const Mem32& mem) {
    mem32(0x8b, dst.reg, mem);
}

void X86Asm::mov(Reg16 dst, const Mem16& mem) {
    outb(0x66);
    mem16(0x8b, dst.reg, mem);
}

void X86Asm::mov(Reg8 dst, const Mem8& mem) {
    mem8(0x8a, dst.reg, mem);
}

void X86Asm::mov(const Mem32& mem, Reg32 src) {
    mem32(0x89, src.reg, mem);
}

void X86Asm::mov(const Mem16& mem, Reg16 src) {
    outb(0x66);
    mem16(0x89, src.reg, mem);
}

void X86Asm::mov(const Mem8& mem, Reg8 src) {
    mem8(0x88, src.reg, mem);
}

void X86Asm::mov(const Mem64& mem, U64 src) {
    S64 sSrc = (S64)src;
    if (sSrc < std::numeric_limits<int>::min() || sSrc > std::numeric_limits<int>::max()) {
        mem32(0xc7, 0, mem, false);
        outd((U32)src);
        Mem64 memPlus4 = mem;
        memPlus4.disp += 4;
        mem32(0xc7, 0, memPlus4, false);
        outd((U32)(src >> 32));
    } else {
        mem32(0xc7, 0, mem, true);
        outd((U32)src);
    }

}

void X86Asm::mov(const Mem32& mem, U32 src) {
    mem32(0xc7, 0, mem);
    outd(src);
}

void X86Asm::mov(const Mem16& mem, U16 src) {
    outb(0x66);
    mem32(0xc7, 0, mem);
    outw(src);
}

void X86Asm::mov(const Mem8& mem, U8 src) {
    mem32(0xc6, 0, mem);
    outb(src);
}

void X86Asm::group1(U8 e, U8 math, Reg32 dst, U32 imm) {
    rex(dst.reg, false);
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        if (dst.reg == 0) {
            outb(e);            
        } else {
            outb(0x81);
            outb(0xC0 | (math << 3) | (dst.reg & 7));
        }
        outd(imm);
    } else {
        outb(0x83);
        outb(0xC0 | (math << 3) | (dst.reg & 7));
        outb(imm);
    }
}

void X86Asm::group1(U8 e, U8 math, Reg16 dst, U16 imm) {    
    outb(0x66);
    rex(dst.reg, false);
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        if (dst.reg == 0) {
            outb(e);
        } else {
            outb(0x81);
            outb(0xC0 | (math << 3) | (dst.reg & 7));
        }
        outw(imm);
    } else {
        outb(0x83);
        outb(0xC0 | (math << 3) | (dst.reg & 7));
        outb((U8)imm);
    }
}

void X86Asm::group1(U8 e, U8 math, Reg8 dst, U8 imm) {
    rex(dst.reg, false);
    if (dst.reg == 0) {
        outb(e);
    } else {
        outb(0x80);
        outb(0xC0 | (math << 3) | (dst.reg & 7));
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
    rex(dst.reg, false);
    outb(0xC1);
    outb(0xC0 | (op << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::group2(U8 op, Reg16 dst, U16 imm) {    
    outb(0x66);
    rex(dst.reg, false);
    outb(0xC1);
    outb(0xC0 | (op << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::group2(U8 op, Reg8 dst, U8 imm) {
    rex(dst.reg, false);
    outb(0xC0);
    outb(0xC0 | (op << 3) | (dst.reg & 7));
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
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xa4);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::shld(Reg16 dst, Reg16 src, U16 imm) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xa4);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::shrd(Reg32 dst, Reg32 src, U32 imm) {
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xac);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::shrd(Reg16 dst, Reg16 src, U16 imm) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xac);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::shld(Reg16 dst, Reg16 src, Reg16 cl) {    
    if (cl.reg != 1) {
        kpanic("X86Asm::shld must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xa5);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::shld(Reg32 dst, Reg32 src, Reg32 cl) {
    rex(src.reg, dst.reg, false);
    if (cl.reg != 1) {
        kpanic("X86Asm::shld must use cl");
    }
    outb(0x0f);
    outb(0xa5);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::shrd(Reg16 dst, Reg16 src, Reg16 cl) {    
    if (cl.reg != 1) {
        kpanic("X86Asm::shrd must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xad);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::shrd(Reg32 dst, Reg32 src, Reg32 cl) {
    rex(src.reg, dst.reg, false);
    if (cl.reg != 1) {
        kpanic("X86Asm::shrd must use cl");
    }
    outb(0x0f);
    outb(0xad);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
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

void X86Asm::xadd(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xc1);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::xadd(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xc1);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::xadd(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0xc0);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::add(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x01);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::add(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x01);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::add(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x00);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::adc(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x11);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::adc(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x11);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::adc(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x10);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::sub(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x29);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::sub(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x29);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::sub(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x28);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::sbb(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x19);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::sbb(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x19);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::sbb(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x18);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::and_(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x21);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::and_(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x21);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::and_(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x20);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::or_(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x09);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::or_(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x09);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::or_(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x08);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::xor_(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x31);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::xor_(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x31);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::xor_(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x30);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::shl(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::shl must use cl");
    }
    outb(0xd3);
    outb(0xe0 | (dst.reg & 7));
}

void X86Asm::shl(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::shl must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xe0 | (dst.reg & 7));
}

void X86Asm::shl(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::shl must use cl");
    }
    outb(0xd2);
    outb(0xe0 | (dst.reg & 7));
}

void X86Asm::shr(Reg32 dst, Reg32 src) {
    if (src.reg != 1) {
        kpanic("X86Asm::shr must use cl");
    }
    rex(src.reg, dst.reg, false);    
    outb(0xd3);
    outb(0xe8 | (dst.reg & 7));
}

void X86Asm::shr(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::shr must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xe8 | (dst.reg & 7));
}

void X86Asm::shr(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::shr must use cl");
    }
    outb(0xd2);
    outb(0xe8 | (dst.reg & 7));
}

void X86Asm::sar(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::sar must use cl");
    }
    outb(0xd3);
    outb(0xf8 | (dst.reg & 7));
}

void X86Asm::sar(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::sar must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xf8 | (dst.reg & 7));
}

void X86Asm::sar(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::sar must use cl");
    }
    outb(0xd2);
    outb(0xf8 | (dst.reg & 7));
}

void X86Asm::rol(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::rol must use cl");
    }
    outb(0xd3);
    outb(0xc0 | (dst.reg & 7));
}

void X86Asm::rol(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::rol must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xc0 | (dst.reg & 7));
}

void X86Asm::rol(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::rol must use cl");
    }
    outb(0xd2);
    outb(0xc0 | (dst.reg & 7));
}

void X86Asm::ror(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::ror must use cl");
    }
    outb(0xd3);
    outb(0xc8 | (dst.reg & 7));
}

void X86Asm::ror(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::ror must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xc8 | (dst.reg & 7));
}

void X86Asm::ror(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::ror must use cl");
    }
    outb(0xd2);
    outb(0xc8 | (dst.reg & 7));
}

void X86Asm::rcl(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::rcl must use cl");
    }
    outb(0xd3);
    outb(0xd0 | (dst.reg & 7));
}

void X86Asm::rcl(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::rcl must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xd0 | (dst.reg & 7));
}

void X86Asm::rcl(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::rcl must use cl");
    }
    outb(0xd2);
    outb(0xd0 | (dst.reg & 7));
}

void X86Asm::rcr(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::rcr must use cl");
    }
    outb(0xd3);
    outb(0xd8 | (dst.reg & 7));
}

void X86Asm::rcr(Reg16 dst, Reg16 src) {    
    if (src.reg != 1) {
        kpanic("X86Asm::rcr must use cl");
    }
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0xd3);
    outb(0xd8 | (dst.reg & 7));
}

void X86Asm::rcr(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    if (src.reg != 1) {
        kpanic("X86Asm::rcr must use cl");
    }
    outb(0xd2);
    outb(0xd8 | (dst.reg & 7));
}

void X86Asm::mulEax(Reg32 src) {
    rex(src.reg, false);
    outb(0xf7);
    outb(0xe0 | (src.reg & 7));
}

void X86Asm::mulAx(Reg16 src) {    
    outb(0x66);
    rex(src.reg, false);
    outb(0xf7);
    outb(0xe0 | (src.reg & 7));
}

void X86Asm::mulAl(Reg8 src) {
    rex(src.reg, false);
    outb(0xf6);
    outb(0xe0 | (src.reg & 7));
}

void X86Asm::imulEax(Reg32 src) {
    rex(src.reg, false);
    outb(0xf7);
    outb(0xe8 | (src.reg & 7));
}

void X86Asm::imulAx(Reg16 src) {    
    outb(0x66);
    rex(src.reg, false);
    outb(0xf7);
    outb(0xe8 | (src.reg & 7));
}

void X86Asm::imulAl(Reg8 src) {
    rex(src.reg, false);
    outb(0xf6);
    outb(0xe8 | (src.reg & 7));
}

void X86Asm::mul(Reg32 src) {
    rex(src.reg, false);
    outb(0xf7);
    outb(0xe0 | (src.reg & 7));
}

void X86Asm::imul(Reg32 dst, Reg32 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xaf);
    outb(0xc0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::imul(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xaf);
    outb(0xc0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::imul(Reg32 dst, U32 imm) {
    rex(dst.reg, dst.reg, false);
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
    outb(0xc0 | (dst.reg & 7) | ((dst.reg & 7) << 3));
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
    rex(dst.reg, dst.reg, false);
    if (small) {
        outb(0x6b);
    } else {
        outb(0x69);
    }
    outb(0xc0 | (dst.reg & 7) | ((dst.reg & 7) << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outw((U16)imm);
    }
}

void X86Asm::imul(Reg32 src) {
    rex(src.reg, false);
    outb(0xf7);
    outb(0xe8 | (src.reg & 7));
}
void X86Asm::imul(Reg32 dst, Reg32 src, U32 imm) {
    rex(dst.reg, src.reg, false);
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
    outb(0xc0 | (src.reg & 7) | ((dst.reg & 7) << 3));
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
    rex(dst.reg, src.reg, false);
    if (small) {
        outb(0x6b);
    } else {
        outb(0x69);
    }
    outb(0xc0 | (src.reg & 7) | ((dst.reg & 7) << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outw(imm);
    }
}

void X86Asm::div(Reg8 src) {
    rex(src.reg, false);
    outb(0xf6);
    outb(0xf0 | (src.reg & 7));
}

void X86Asm::div(Reg16 src) {    
    outb(0x66);
    rex(src.reg, false);
    outb(0xf7);
    outb(0xf0 | (src.reg & 7));
}

void X86Asm::div(Reg32 src) {
    rex(src.reg, false);
    outb(0xf7);
    outb(0xf0 | (src.reg & 7));
}

void X86Asm::idiv(Reg8 src) {
    rex(src.reg, false);
    outb(0xf6);
    outb(0xf8 | (src.reg & 7));
}

void X86Asm::idiv(Reg16 src) {    
    outb(0x66);
    rex(src.reg, false);
    outb(0xf7);
    outb(0xf8 | (src.reg & 7));
}

void X86Asm::idiv(Reg32 src) {
    rex(src.reg, false);
    outb(0xf7);
    outb(0xf8 | (src.reg & 7));
}

void X86Asm::neg(Reg32 dst) {
    rex(dst.reg, false);
    outb(0xf7);
    outb(0xd8 | (dst.reg & 7));
}

void X86Asm::neg(Reg16 dst) {    
    outb(0x66);
    rex(dst.reg, false);
    outb(0xf7);
    outb(0xd8 | (dst.reg & 7));
}

void X86Asm::neg(Reg8 dst) {
    rex(dst.reg, false);
    outb(0xf6);
    outb(0xd8 | (dst.reg & 7));
}

void X86Asm::not_(Reg32 dst) {
    rex(dst.reg, false);
    outb(0xf7);
    outb(0xd0 | (dst.reg & 7));
}

void X86Asm::not_(Reg16 dst) {    
    outb(0x66);
    rex(dst.reg, false);
    outb(0xf7);
    outb(0xd0 | (dst.reg & 7));
}

void X86Asm::not_(Reg8 dst) {
    rex(dst.reg, false);
    outb(0xf6);
    outb(0xd0 | (dst.reg & 7));
}

void X86Asm::cmovl(Reg32 dst, Reg32 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x4c);
    outb(0xc0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cmp(Reg32 dst, U32 imm) {
    group1(0x3d, 7, dst, imm);
}

void X86Asm::cmp(Reg16 dst, U16 imm) {
    outb(0x66);
    group1(0x3d, 7, dst, imm);
}

void X86Asm::cmp(Reg8 dst, U8 imm) {
    group1(0x3c, 7, dst, imm);
}

void X86Asm::cmp(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x39);
    outb(0xC0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::cmp(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x39);
    outb(0xC0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::cmp(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x38);
    outb(0xC0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::bt(Reg32 reg, Reg32 rm) {
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xa3);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::bt(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xa3);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::bt(Reg32 reg, U8 imm) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (4 << 3));
    outb(imm);
}

void X86Asm::bt(Reg16 reg, U8 imm) {    
    outb(0x66);
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (4 << 3));
    outb(imm);
}

void X86Asm::bts(Reg32 reg, Reg32 rm) {
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xab);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::bts(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xab);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::bts(Reg32 reg, U8 imm) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (5 << 3));
    outb(imm);
}

void X86Asm::bts(Reg16 reg, U8 imm) {    
    outb(0x66);
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (5 << 3));
    outb(imm);
}

void X86Asm::btr(Reg32 reg, Reg32 rm) {
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xb3);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::btr(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xb3);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::btr(Reg32 reg, U8 imm) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (6 << 3));
    outb(imm);
}

void X86Asm::btr(Reg16 reg, U8 imm) {    
    outb(0x66);
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (6 << 3));
    outb(imm);
}

void X86Asm::btc(Reg32 reg, Reg32 rm) {
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xbb);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::btc(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(rm.reg, reg.reg, false);
    outb(0x0f);
    outb(0xbb);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::btc(Reg32 reg, U8 imm) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (7 << 3));
    outb(imm);
}

void X86Asm::btc(Reg16 reg, U8 imm) {    
    outb(0x66);
    rex(reg.reg, false);
    outb(0x0f);
    outb(0xba);
    outb(0xC0 | (reg.reg & 7) | (7 << 3));
    outb(imm);
}

void X86Asm::bsf(Reg32 reg, Reg32 rm) {
    rex(reg.reg, rm.reg, false);
    outb(0x0f);
    outb(0xbc);
    outb(0xC0 | (rm.reg & 7) | ((reg.reg & 7) << 3));
}

void X86Asm::bsf(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(reg.reg, rm.reg, false);
    outb(0x0f);
    outb(0xbc);
    outb(0xC0 | (rm.reg & 7) | ((reg.reg & 7) << 3));
}

void X86Asm::bsr(Reg32 reg, Reg32 rm) {
    rex(reg.reg, rm.reg, false);
    outb(0x0f);
    outb(0xbd);
    outb(0xC0 | (rm.reg & 7) | ((reg.reg & 7) << 3));
}

void X86Asm::bsr(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(reg.reg, rm.reg, false);
    outb(0x0f);
    outb(0xbd);
    outb(0xC0 | (rm.reg & 7) | ((reg.reg & 7) << 3));
}

void X86Asm::test(Reg32 dst, U32 imm) {
    rex(dst.reg, false);
    if (dst.reg == 0) {
        outb(0xa9);
    } else {
        outb(0xf7);
        outb(0xC0 | (dst.reg & 7));
    }
    outd(imm);
}

void X86Asm::test(Reg16 dst, U16 imm) {    
    outb(0x66);
    rex(dst.reg, false);
    if (dst.reg == 0) {
        outb(0xa9);
    } else {
        outb(0xf7);
        outb(0xC0 | (dst.reg & 7));
    }
    outw(imm);
}

void X86Asm::test(Reg8 dst, U8 imm) {
    rex(dst.reg, false);
    if (dst.reg == 0) {
        outb(0xa8);
    } else {
        outb(0xf6);
        outb(0xC0 | (dst.reg & 7));
    }
    outb(imm);
}

void X86Asm::test(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x85);
    outb(0xc0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::test(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x85);
    outb(0xc0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::test(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x84);
    outb(0xc0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::test(const Mem32& mem, U32 imm) {
    S32 sIMM = (S32)imm;
    if (sIMM < -128 || sIMM > 127) {
        mem32(0xf7, 0, mem);
        outd(imm);
    } else {
        mem32(0xf6, 6, mem);
        outb((U8)imm);
    }
}

void X86Asm::movsx(Reg16 dst, Reg8 src) {    
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xbe);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movsx(Reg32 dst, Reg8 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xbe);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movsx(Reg32 dst, Reg16 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xbf);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::mov(Reg32 dst, Reg32 src) {
    rex(src.reg, dst.reg, false);
    outb(0x89);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::mov(Reg16 dst, Reg16 src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x89);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::mov(Reg8 dst, Reg8 src) {
    rex(src.reg, dst.reg, false);
    outb(0x88);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::movzx(Reg16 dst, Reg8 src) {    
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xb6);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movzx(Reg32 dst, Reg8 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xb6);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movzx(Reg32 dst, Reg16 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xb7);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::push(U32 imm) {
    outb(0x68);
    outd(imm);
}

void X86Asm::pushFlags() {
    outb(0x9c);
}

void X86Asm::jmp(Reg reg) {
    rex(reg.reg, false);
    outb(0xff);
    outb(0xe0 | (reg.reg & 7));
}

void X86Asm::jo(U32 address) {
    outb(0xf);
    outb(0x80);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jno(U32 address) {
    outb(0xf);
    outb(0x81);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jb(U32 address) {
    outb(0xf);
    outb(0x82);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnb(U32 address) {
    outb(0xf);
    outb(0x83);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jz(U32 address) {
    outb(0xf);
    outb(0x84);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnz(U32 address) {
    outb(0xf);
    outb(0x85);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jbe(U32 address) {
    outb(0xf);
    outb(0x86);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnbe(U32 address) {
    outb(0xf);
    outb(0x87);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::js(U32 address) {
    outb(0xf);
    outb(0x88);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jns(U32 address) {
    outb(0xf);
    outb(0x89);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jp(U32 address) {
    outb(0xf);
    outb(0x8a);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnp(U32 address) {
    outb(0xf);
    outb(0x8b);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jl(U32 address) {
    outb(0xf);
    outb(0x8c);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnl(U32 address) {
    outb(0xf);
    outb(0x8d);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jle(U32 address) {
    outb(0xf);
    outb(0x8e);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::jnle(U32 address) {
    outb(0xf);
    outb(0x8f);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0); // jump over amount
}

void X86Asm::goto8(U32 amount) {
    outb(0xeb);
    outb(amount);
}

void X86Asm::goto32(U32 amount) {
    outb(0xe9);
    outd(amount);
}

void X86Asm::jmp(U32 address) {
    outb(0xe9);
    jumps.push_back(DynamicJump(address, (U32)buffer.size()));
    outd(0);
}

void X86Asm::movsb_repeat() {
    outb(0xf2);
    outb(0xa4);
}

void X86Asm::lock() {
    outb(0xf0);
}

void X86Asm::cmpxchg(const Mem32& mem, Reg32 reg) {
    mem32(0x1b1, reg.reg, mem);
}

void X86Asm::cmpxchg(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x1b1, reg.reg, mem);
}

void X86Asm::cmpxchg(const Mem8& mem, Reg8 reg) {
    mem8(0x1b0, reg.reg, mem);
}

void X86Asm::xchg(Reg32 reg, const Mem32& mem) {
    mem32(0x87, reg.reg, mem);
}

void X86Asm::xchg(Reg16 reg, const Mem16& mem) {
    outb(0x66);
    mem16(0x87, reg.reg, mem);
}

void X86Asm::xchg(Reg8 reg, const Mem8& mem) {
    mem8(0x86, reg.reg, mem);
}

void X86Asm::xchg(Reg8 reg, Reg8 rm) {
    rex(rm.reg, reg.reg, false);
    outb(0x86);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::xchg(Reg16 reg, Reg16 rm) {    
    outb(0x66);
    rex(rm.reg, reg.reg, false);
    outb(0x87);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::xchg(Reg32 reg, Reg32 rm) {
    rex(rm.reg, reg.reg, false);
    outb(0x87);
    outb(0xC0 | (reg.reg & 7) | (rm.reg << 3));
}

void X86Asm::xadd(const Mem32& mem, Reg32 reg) {
    mem32(0x1c1, reg.reg, mem);
}

void X86Asm::xadd(const Mem16& mem, Reg16 reg) {
    outb(0x66);
    mem16(0x1c1, reg.reg, mem);
}

void X86Asm::xadd(const Mem8& mem, Reg8 reg) {
    mem8(0x1c0, reg.reg, mem);
}

void X86Asm::cmpxchg8b(const Mem64& mem) {
    mem32(0x1c7, 0x01, mem);
}

void X86Asm::ret() {
    outb(0xc3);
}

void X86Asm::IfLessThan(Reg32 reg, U32 value) {
    cmp(reg, value);
    IfCF();
}

void X86Asm::IfLessThan(Reg16 reg, U16 value) {
    cmp(reg, value);
    IfCF();
}

void X86Asm::IfLessThan(Reg8 reg, U8 value) {
    cmp(reg, value);
    IfCF();
}

void X86Asm::IfLessThan(Reg32 reg1, Reg32 reg2) {
    cmp(reg1, reg2);
    IfCF();
}

void X86Asm::IfLessThan(Reg16 reg1, Reg16 reg2) {
    cmp(reg1, reg2);
    IfCF();
}

void X86Asm::IfLessThan(Reg8 reg1, Reg8 reg2) {
    cmp(reg1, reg2);
    IfCF();
}

void X86Asm::IfEqual(Reg32 reg, U32 value) {
    cmp(reg, value);
    IfZF();
}

void X86Asm::IfEqual(Reg16 reg, U16 value) {
    cmp(reg, value);
    IfZF();
}

void X86Asm::IfEqual(Reg8 reg, U8 value) {
    cmp(reg, value);
    IfZF();
}

void X86Asm::IfEqual(Reg32 reg1, Reg32 reg2) {
    cmp(reg1, reg2);
    IfZF();
}

void X86Asm::IfEqual(Reg16 reg1, Reg16 reg2) {
    cmp(reg1, reg2);
    IfZF();
}

void X86Asm::IfEqual(Reg8 reg1, Reg8 reg2) {
    cmp(reg1, reg2);
    IfZF();
}

void X86Asm::IfNotEqual(Reg32 reg, U32 value) {
    cmp(reg, value);
    IfNotZF();
}

void X86Asm::IfNotEqual(Reg16 reg, U16 value) {
    cmp(reg, value);
    IfNotZF();
}

void X86Asm::IfNotEqual(Reg8 reg, U8 value) {
    cmp(reg, value);
    IfNotZF();
}

void X86Asm::IfNotEqual(Reg32 reg, Reg32 reg2) {
    cmp(reg, reg2);
    IfNotZF();
}

void X86Asm::IfNotEqual(Reg16 reg, Reg16 reg2) {
    cmp(reg, reg2);
    IfNotZF();
}

void X86Asm::IfNotEqual(Reg8 reg, Reg8 reg2) {
    cmp(reg, reg2);
    IfNotZF();
}

void X86Asm::IfZero(Reg32 reg) {
    test(reg, reg);
    IfJump(0x05); // jnz
}

void X86Asm::IfZero(Reg16 reg) {
    test(reg, reg);
    IfJump(0x05); // jnz
}

void X86Asm::IfZero(Reg8 reg) {
    test(reg, reg);
    IfJump(0x5); // jnz
}

void X86Asm::IfPF() {
    IfJump(0xb); // jnp
}

void X86Asm::IfCF() {
    IfJump(0x3); // jnb
}

void X86Asm::IfZF() {
    IfJump(0x5); // jnz
}

void X86Asm::jb() {
    IfJump(0x2); // jb
}

void X86Asm::jnb() {
    IfJump(0x3); // jnb
}

void X86Asm::jz() {
    IfJump(0x4); // jz
}

void X86Asm::jnz() {
    IfJump(0x5); // jnz
}

void X86Asm::jbe() {
    IfJump(0x6); // jbe
}

void X86Asm::jnbe() {
    IfJump(0x7); // jnbe
}

void X86Asm::jl() {
    IfJump(0xc); // jl
}

void X86Asm::jnl() {
    IfJump(0xd); // jnl
}

void X86Asm::jle() {
    IfJump(0xe); // jle
}

void X86Asm::jnle() {
    IfJump(0xf); // jnle
}

void X86Asm::IfSF() {
    IfJump(0x9); // jns
}

void X86Asm::IfOF() {
    IfJump(0x1); // jno
}

void X86Asm::IfNotZF() {
    IfJump(0x4); // jz
}

void X86Asm::IfNotZero(Reg32 reg) {
    test(reg, reg);
    IfNotZF();
}

void X86Asm::IfNotZero(Reg16 reg) {
    test(reg, reg);
    IfNotZF();
}

void X86Asm::IfNotZero(Reg8 reg) {
    test(reg, reg);
    IfNotZF();
}

void X86Asm::Else() {
    outb(0xe9); // previous block should jump over else statement
    U32 pos = (U32)buffer.size();
    outd(0); // jump over amount

    // if statement will jump here if it wasn't true
    EndIf();

    ifJump.push_back(pos);
}

void X86Asm::EndIf() {
    U32 pos = ifJump.back();
    bool bigJump = true;
    /*
    U32 amount = (U32)buffer.size() - pos - 1;
    if (amount > 127) {
        U32 oldBufferSize = (U32)buffer.size();
        bigJump = true;
        if (buffer[pos - 1] == 0xeb) {
            buffer[pos - 1] = 0xe9;
            // make room, convert 8-bit offset to 32-bit offset
            outb(0);
            outb(0);
            outb(0);
            // move everything down 3-bytes
            memmove(&buffer[pos + 3], &buffer[pos], oldBufferSize - pos);
        } else {
            outd(0); // make room for converting 8-bit offset to 32-bit offset (3 bytes) plus 1 more for the instruction prefix
            memmove(&buffer[pos + 4], &buffer[pos], oldBufferSize - pos);
            buffer[pos] = buffer[pos - 1] + 0x10; // convert 0x71 to 0x81, etc
            buffer[pos - 1] = 0x0f;
            pos++;
        }
    }
    */
    //if (bigJump) {
        U32 amount = (U32)buffer.size() - pos - 4;
        ifJump.pop_back();
        *(U32*)&buffer.data()[pos] = (U32)(amount);
    //} else {        
    //    ifJump.pop_back();
    //    buffer[pos] = (U8)(amount);
    //}
}

void X86Asm::setz(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x94);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setnz(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x95);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setb(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x92);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setnb(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x93);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setbe(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x96);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setnbe(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x97);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setl(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x9c);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setnl(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x9d);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setle(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x9e);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setnle(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x9f);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::seto(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x90);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setno(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x91);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::sets(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x98);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setns(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x99);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setp(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x9a);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::setnp(Reg8 reg) {
    rex(reg.reg, false);
    outb(0x0f);
    outb(0x9b);
    outb(0xc0 + (reg.reg & 7));
}

void X86Asm::movlhps(RegXMM hiDstXMM, RegXMM loSrcXMM) {
    rex(hiDstXMM.reg, loSrcXMM.reg, false);
    outb(0x0f);
    outb(0x16);
    outb(0xC0 | ((hiDstXMM.reg & 7) << 3) | (loSrcXMM.reg & 7));
}

void X86Asm::movhlps(RegXMM hiDstXMM, RegXMM loSrcXMM) {
    rex(hiDstXMM.reg, loSrcXMM.reg, false);
    outb(0x0f);
    outb(0x12);
    outb(0xC0 | ((hiDstXMM.reg & 7) << 3) | (loSrcXMM.reg & 7));
}

void X86Asm::movss(const Mem32& mem, RegXMM srcXMM) {
    outb(0xf3);
    mem32(0x111, srcXMM.reg, mem);
}

void X86Asm::movss(RegXMM dstXMM, const Mem32& mem) {
    outb(0xf3);
    mem32(0x110, dstXMM.reg, mem);
}

void X86Asm::movss(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf3);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x10);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::movsd(const Mem64& mem, RegXMM srcXMM) {
    outb(0xf2);
    mem32(0x111, srcXMM.reg, mem);
}

void X86Asm::movsd(RegXMM dstXMM, const Mem64& mem) {
    outb(0xf2);
    mem32(0x110, dstXMM.reg, mem);
}

void X86Asm::movsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x10);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::movlpd(RegXMM dstXMM, const Mem64& mem) {
    outb(0x66);
    mem32(0x112, dstXMM.reg, mem);
}

void X86Asm::cvtss2sd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf3);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvtsd2ss(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::xorpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x57);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::andpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x54);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvtsi2sd(RegXMM dstXMM, const Mem32& mem) {
    outb(0xf2);
    mem32(0x12a, dstXMM.reg, mem);
}

void X86Asm::cvttsd2si(Reg32 dst, RegXMM srcXMM) {
    outb(0xf2);
    rex(dst.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x2c);
    outb(0xC0 | ((dst.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvtsd2si(Reg32 dst, RegXMM srcXMM) {
    outb(0xf2);
    rex(dst.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x2d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvtsi2sd(RegXMM dstXMM, Reg32 reg) {
    outb(0xf2);
    rex(dstXMM.reg, reg.reg, false);
    outb(0x0f);
    outb(0x2a);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (reg.reg & 7));
}

void X86Asm::cvtdq2pd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf3);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvtpd2dq(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvttpd2dq(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0xe6);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::cvtpd2ps(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::addpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::addsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::mulpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::mulsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::subpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::subsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::sqrtpd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0x66);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::sqrtsd(RegXMM dstXMM, RegXMM srcXMM) {
    outb(0xf2);
    rex(dstXMM.reg, srcXMM.reg, false);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | ((dstXMM.reg & 7) << 3) | (srcXMM.reg & 7));
}

void X86Asm::ucomisd(RegXMM xmm1, RegXMM xmm2) {
    outb(0x66);
    rex(xmm1.reg, xmm2.reg, false);
    outb(0x0f);
    outb(0x2e);
    outb(0xc0 | (xmm2.reg & 7) | ((xmm1.reg & 7) << 3));
}

void X86Asm::stmxcsr(const Mem32& mem) {
    mem32(0x1ae, 3, mem);
}

void X86Asm::ldmxcsr(const Mem32& mem) {
    mem32(0x1ae, 2, mem);
}

void X86Asm::rdtsc() {
    outb(0x0f);
    outb(0x31);
}

void X86Asm::movd(RegMMX dst, Reg32 src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movd(Reg32 dst, RegMMX src) {
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0x7e);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7)); // wonder why this is reversed where most instruction put dst at the top and src at the bottom
}

void X86Asm::movd(const Mem32& mem, RegMMX reg) {
    mem32(0x17e, reg.reg, mem);
}

void X86Asm::movd(RegMMX reg, const Mem32& mem) {
    mem32(0x16e, reg.reg, mem);
}

void X86Asm::emms() {
    outb(0x0f);
    outb(0x77);
}

void X86Asm::movaps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x10);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movaps(RegXMM reg, const Mem128& mem) {
    mem32(0x128, reg.reg, mem);
}

void X86Asm::movaps(const Mem128& mem, RegXMM reg) {
    mem32(0x129, reg.reg, mem);
}

void X86Asm::movups(RegXMM reg, const Mem128& mem) {
    mem32(0x110, reg.reg, mem);
}

void X86Asm::movups(const Mem128& mem, RegXMM reg) {
    mem32(0x111, reg.reg, mem);
}

void X86Asm::movhps(const Mem64& mem, RegXMM reg) {
    mem32(0x117, reg.reg, mem);
}

void X86Asm::movlps(const Mem64& mem, RegXMM reg) {
    mem32(0x113, reg.reg, mem);
}

void X86Asm::movhps(RegXMM reg, const Mem64& mem) {
    mem32(0x116, reg.reg, mem);
}

void X86Asm::movlps(RegXMM reg, const Mem64& mem) {
    mem32(0x112, reg.reg, mem);
}

void X86Asm::movd(RegXMM dst, Reg32 src) {    
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movd(Reg32 dst, RegXMM src) {    
    outb(0x66);
    rex(src.reg, dst.reg, false);
    outb(0x0f);
    outb(0x7e);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7)); // wonder why this is reversed where most instruction put dst at the top and src at the bottom
}

void X86Asm::movd(const Mem32& mem, RegXMM reg) {
    outb(0x66);
    mem32(0x17e, reg.reg, mem);
}

void X86Asm::movd(RegXMM reg, const Mem32& mem) {
    outb(0x66);
    mem32(0x16e, reg.reg, mem);
}

void X86Asm::movq(RegXMM reg, const Mem64& mem) {
    outb(0xf3);
    mem32(0x17e, reg.reg, mem);
}

void X86Asm::movq(const Mem64& mem, RegXMM src) {
    outb(0x66);
    mem32(0x1d6, src.reg, mem);
}

void X86Asm::movq(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x7e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movdqu(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pxor(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xef);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::por(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xeb);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pand(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xdb);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pandn(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xdf);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psllw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf1);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psllw(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x71);
    outb(0xf0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psrlw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd1);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psrlw(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x71);
    outb(0xd0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psraw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe1);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psraw(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x71);
    outb(0xe0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::pslld(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf2);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pslld(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x72);
    outb(0xf0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psrld(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd2);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psrld(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x72);
    outb(0xd0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psrad(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe2);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psrad(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x72);
    outb(0xe0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psllq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf3);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psllq(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x73);
    outb(0xf0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::pslldq(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x73);
    outb(0xf8 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psrlq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd3);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psrlq(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x73);
    outb(0xd0 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::psrldq(RegXMM dst, U32 imm) {
    outb(0x66);
    rex(dst.reg, false);
    outb(0x0f);
    outb(0x73);
    outb(0xd8 | (dst.reg & 7));
    outb(imm);
}

void X86Asm::paddb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xfc);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xfd);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xfe);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd4);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddsb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xec);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xed);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddusb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xdc);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::paddusw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xdd);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf8);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf9);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xfa);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xfb);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubsb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe8);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe9);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubusb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd8);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psubusw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd9);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmulhw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe5);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmullw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd5);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmaddwd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf5);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmuludq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf4);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pcmpeqb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x74);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pcmpeqw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x75);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pcmpeqd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x76);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pcmpgtb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x64);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pcmpgtw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x65);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pcmpgtd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x66);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::packsswb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x63);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::packssdw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6b);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::packuswb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x67);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpckhbw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x68);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpckhwd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x69);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpckhdq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6a);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpcklbw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x60);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpcklwd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x61);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpckldq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x62);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::unpcklpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x14);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::unpckhpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x15);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpckhqdq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::punpcklqdq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x6c);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::addps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::addss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x58);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::subps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::subss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5c);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::mulps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::mulss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x59);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::divps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::divpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::divss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::divsd(RegXMM dst, RegXMM src) {
    outb(0xf2);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::rcpps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x53);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::rcpss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x53);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::sqrtps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::sqrtss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x51);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::rsqrtps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x52);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::rsqrtss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x52);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::maxps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::maxpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::maxss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::maxsd(RegXMM dst, RegXMM src) {
    outb(0xf2);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::minps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::minpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::minss(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::minsd(RegXMM dst, RegXMM src) {
    outb(0xf2);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pavgb(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe0);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pavgw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe3);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::psadbw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xf6);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pextrw(Reg32 dst, RegXMM src, U8 srcIndex) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xc5);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
    outb(srcIndex);
}

void X86Asm::pinsrw(RegXMM dst, Reg32 src, U8 dstIndex) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xc4);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
    outb(dstIndex);
}

void X86Asm::pmaxsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xee);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmaxub(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xde);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pminsw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xea);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pminub(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xda);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmovmskb(Reg32 dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xd7);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pmulhuw(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xe4);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::pshuflw(RegXMM dst, RegXMM src, U8 order) {
    outb(0xf2);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x70);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
    outb(order);
}

void X86Asm::pshufhw(RegXMM dst, RegXMM src, U8 order) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x70);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
    outb(order);
}

void X86Asm::pshufd(RegXMM dst, RegXMM src, U8 order) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x70);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
    outb(order);
}

void X86Asm::shufpd(RegXMM dst, RegXMM src, U8 order) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0xc6);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
    outb(order);
}

void X86Asm::andnps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x55);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::andnpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x55);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::andps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x54);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::orpd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x56);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::orps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x56);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::xorps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x57);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvtdq2ps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5b);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvtps2dq(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5b);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvtps2pd(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5a);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvttps2dq(RegXMM dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x5b);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvtsi2ss(RegXMM dst, Reg32 src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x2a);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvtss2si(Reg32 dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x2d);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cvttss2si(Reg32 dst, RegXMM src) {
    outb(0xf3);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x2c);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::movmskps(Reg32 dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x50);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::maskmovdqu(RegXMM src, RegXMM mask) {
    outb(0x66);
    rex(src.reg, mask.reg, false);
    outb(0x0f);
    outb(0xf7);
    outb(0xC0 | ((src.reg & 7) << 3) | (mask.reg & 7));
}

void X86Asm::shufps(RegXMM src, RegXMM mask, U8 imm) {
    rex(src.reg, mask.reg, false);
    outb(0x0f);
    outb(0xc6);
    outb(0xC0 | ((src.reg & 7) << 3) | (mask.reg & 7));
    outb(imm);
}

void X86Asm::cmppd(RegXMM src, RegXMM mask, U8 imm) {
    outb(0x66);
    rex(src.reg, mask.reg, false);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | ((src.reg & 7) << 3) | (mask.reg & 7));
    outb(imm);
}

void X86Asm::cmpps(RegXMM src, RegXMM mask, U8 imm) {
    rex(src.reg, mask.reg, false);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | ((src.reg & 7) << 3) | (mask.reg & 7));
    outb(imm);
}

void X86Asm::cmpsd(RegXMM src, RegXMM mask, U8 imm) {
    outb(0xf2);
    rex(src.reg, mask.reg, false);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | ((src.reg & 7) << 3) | (mask.reg & 7));
    outb(imm);
}

void X86Asm::cmpss(RegXMM src, RegXMM mask, U8 imm) {
    outb(0xf3);
    rex(src.reg, mask.reg, false);
    outb(0x0f);
    outb(0xc2);
    outb(0xC0 | ((src.reg & 7) << 3) | (mask.reg & 7));
    outb(imm);
}

void X86Asm::unpckhps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x15);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::unpcklps(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x14);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::comisd(RegXMM dst, RegXMM src) {
    outb(0x66);
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x2f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::comiss(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x2f);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::ucomiss(RegXMM dst, RegXMM src) {
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x2e);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
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
    rex(dst.reg, src.reg, false);
    outb(0x0f);
    outb(0x50);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::clflush(const Mem8& mem) {
    mem32(0x1ae, 7, mem);
}

void X86Asm::IfJump(U8 inst) {
    outb(0x0f);
    outb(0x80 | inst);
    ifJump.push_back((U32)buffer.size());
    outd(0); // jump over amount
}

#ifdef BOXEDWINE_64
void X86Asm::IfNotZero(Reg64 reg) {
    test(reg, reg);
    IfJump(0x4); // jz
}

void X86Asm::IfZero(Reg64 reg) {
    test(reg, reg);
    IfJump(0x5); // jnz
}

void X86Asm::IfNotEqual(Reg64 reg, Reg64 reg2) {
    cmp(reg, reg2);
    IfNotZF();
}

void X86Asm::IfEqual(Reg64 reg, Reg64 reg2) {
    cmp(reg, reg2);
    IfZF();
}

void X86Asm::IfNotEqual(Reg64 reg, U32 value) {
    cmp(reg, value);
    IfJump(0x4); // jz
}

void X86Asm::IfEqual(Reg64 reg, U32 value) {
    cmp(reg, value);
    IfJump(0x5); // jnz
}

void X86Asm::mov(Reg64 dst, const Mem64& mem) {
    mem32(0x8b, dst.reg, mem, true);
}

void X86Asm::mov(const Mem64& mem, Reg64 src) {
    mem32(0x89, src.reg, mem, true);
}

void X86Asm::add(Reg64 dst, U32 imm) {
    group1(0x05, 0, Reg32::from(dst.reg), imm);
}

void X86Asm::sub(Reg64 dst, U32 imm) {
    group1(0x2d, 5, Reg32::from(dst.reg), imm);
}

void X86Asm::cmp(Reg64 dst, U32 imm) {
    group1(0x3d, 7, Reg32::from(dst.reg & 7), imm);
}

void X86Asm::cmp(Reg64 dst, Reg64 src) {
    rex(src.reg, dst.reg, true);
    outb(0x39);
    outb(0xC0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::test(Reg64 dst, Reg64 src) {
    rex(src.reg, dst.reg, 0, true);
    outb(0x85);
    outb(0xc0 | (dst.reg & 7) | ((src.reg & 7) << 3));
}

void X86Asm::rex(U8 reg, U8 rm, bool is64) {
    U8 rex = REX_BASE;

    if (is64) {
        rex |= REX_64;
    }
    if (rm > 7) {
        rex |= REX_MOD_RM;
    }
    if (reg > 7) {
        rex |= REX_MOD_REG;
    }
    if (rex != REX_BASE) {
        outb(rex);
    }
}

void X86Asm::rexBase() {
    outb(REX_BASE);
}

void X86Asm::rex(U8 reg, bool is64) {
    U8 rex = REX_BASE;

    if (is64) {
        rex |= REX_64;
    }
    if (reg > 7) {
        rex |= REX_MOD_RM;
    }
    if (rex != REX_BASE) {
        outb(rex);
    }
}

void X86Asm::rex(U8 reg, U8 rm, U8 sib, bool is64) {
    U8 rex = REX_BASE;

    if (is64) {
        rex |= REX_64;
    }
    if (rm > 7) {
        rex |= REX_MOD_RM;
    }
    if (reg > 7) {
        rex |= REX_MOD_REG;
    }
    if (sib > 7) {
        rex |= REX_SIB_INDEX;
    }
    if (rex != REX_BASE) {
        outb(rex);
    }
}

void X86Asm::mov(Reg64 dst, Reg64 src) {
    rex(src.reg, dst.reg, true);
    outb(0x89);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
}

void X86Asm::mov(Reg64 dst, U64 imm) {
    if (imm <= 0xffffffffl) {
        mov(X86Asm::Reg32::from(dst.reg), (U32)imm);
    } else {
        rex(dst.reg, true);
        outb(0xb8 + (dst.reg & 7));
        outq(imm);
    }
}
void X86Asm::push(Reg64 reg) {
    rex(reg.reg, true);
    outb(0x50 | (reg.reg & 7));
}

void X86Asm::pop(Reg64 reg) {
    rex(reg.reg, true);
    outb(0x58 | (reg.reg & 7));
}

void X86Asm::call(Reg64 reg) {
    rex(reg.reg, true);
    outb(0xff);
    outb(0xd0 + (reg.reg & 7));
}

void X86Asm::div(Reg64 src) {
    rex(src.reg, true);
    outb(0xf7);
    outb(0xf0 | (src.reg & 7));
}

void X86Asm::idiv(Reg64 src) {
    rex(src.reg, true);
    outb(0xf7);
    outb(0xf8 | (src.reg & 7));
}

void X86Asm::group2(U8 op, Reg64 dst, U8 imm) {
    rex(dst.reg, true);
    outb(0xC1);
    outb(0xC0 | (op << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::shl(Reg64 dst, U8 imm) {
    group2(6, dst, imm);
}

void X86Asm::shr(Reg64 dst, U8 imm) {
    group2(5, dst, imm);
}

void X86Asm::shrx(Reg32 dst, Reg32 src) {
    outb(0xc4);
    outb((dst.reg > 7) ? 0x42 : 0xe2);
    outb(3 | ((15 - src.reg) << 3));
    outb(0xf7);
    outb(0xc0 | (dst.reg & 7) << 3 | (dst.reg & 7));
}

void X86Asm::shlx(Reg32 dst, Reg32 src) {
    outb(0xc4);
    outb((dst.reg > 7) ? 0x42 : 0xe2);
    outb(1 | ((15 - src.reg) << 3));
    outb(0xf7);
    outb(0xc0 | (dst.reg & 7) << 3 | (dst.reg & 7));
}

void X86Asm::sarx(Reg32 dst, Reg32 src) {
    outb(0xc4);
    outb((dst.reg > 7) ? 0x42 : 0xe2);
    outb(2 | ((15 - src.reg) << 3));
    outb(0xf7);
    outb(0xc0 | (dst.reg & 7) << 3 | (dst.reg & 7));
}

void X86Asm::shrd(Reg64 dst, Reg64 src, U8 imm) {
    rex(src.reg, dst.reg, true);
    outb(0x0f);
    outb(0xac);
    outb(0xC0 | ((src.reg & 7) << 3) | (dst.reg & 7));
    outb((U8)imm);
}

void X86Asm::movsx(Reg64 dst, Reg32 src) {
    rex(dst.reg, src.reg, true);
    outb(0x63);
    outb(0xC0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::cqo() {
    outb(REX_BASE | REX_64);
    outb(0x99);
}

void X86Asm::imul(Reg64 dst, Reg64 src) {
    rex(dst.reg, src.reg, true);
    outb(0x0f);
    outb(0xaf);
    outb(0xc0 | ((dst.reg & 7) << 3) | (src.reg & 7));
}

void X86Asm::imul(Reg64 dst, Reg64 src, U32 imm) {
    rex(dst.reg, src.reg, true);
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
    outb(0xc0 | (src.reg & 7) | ((dst.reg & 7) << 3));
    if (small) {
        outb((U8)imm);
    } else {
        outd(imm);
    }
}

#else
void X86Asm::call(void* address) {
    outb(0xe8);
    patch.push_back((U32)buffer.size());
    outd((U32)address);
}

void X86Asm::push(Reg32 reg) {
    outb(0x50 | reg.reg);
}

void X86Asm::pop(Reg32 reg) {
    outb(0x58 | reg.reg);
}

void X86Asm::rex(U8 reg, U8 rm, bool is64) {
    if (is64 || rm > 7 || reg > 7) {
        kpanic("X86Asm::rex bad 32-bit instruction");
    }
}

void X86Asm::rexBase() {
    kpanic("X86Asm::rex bad 32-bit instruction");
}

void X86Asm::rex(U8 reg, bool is64) {
    if (is64 || reg > 7) {
        kpanic("X86Asm::rex bad 32-bit instruction");
    }
}

void X86Asm::rex(U8 reg, U8 rm, U8 sib, bool is64) {
    if (is64 || rm > 7 || reg > 7 || sib > 7) {
        kpanic("X86Asm::rex bad 32-bit instruction");
    }
}

#endif

#endif