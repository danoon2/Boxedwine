#ifndef __X86_ASM_H__
#define __X86_ASM_H__

class X86Asm {
public:
	class Reg8Name {
	public:
		Reg8Name(U8 reg) : reg(reg) {}
		U8 reg;
	};
	class Reg16Name {
	public:
		Reg16Name(U8 reg) : reg(reg) {}
		U8 reg;
	};
	class Reg32Name {
	public:
		Reg32Name(U8 reg) : reg(reg) {}
		U8 reg;
	};
	typedef Reg8Name Reg8;
	typedef Reg16Name Reg16;
	typedef Reg32Name Reg32;

	static Reg8 al;
	static Reg8 cl;
	static Reg8 dl;
	static Reg8 bl;
	static Reg8 ah;
	static Reg8 ch;
	static Reg8 dh;
	static Reg8 bh;

	static Reg16 ax;
	static Reg16 cx;
	static Reg16 dx;
	static Reg16 bx;
	static Reg16 sp;
	static Reg16 bp;
	static Reg16 si;
	static Reg16 di;

	static Reg32 eax;
	static Reg32 ecx;
	static Reg32 edx;
	static Reg32 ebx;
	static Reg32 esp;
	static Reg32 ebp;
	static Reg32 esi;
	static Reg32 edi;

	void lea(Reg32 dst, Reg32 rm, Reg32 sib, U32 shift, U32 disp);
	
	void add(Reg32 dst, U32 imm);
	void add(Reg16 dst, U16 imm);
	void add(Reg8 dst, U8 imm);
	void sub(Reg32 dst, U32 imm);
	void sub(Reg16 dst, U16 imm);
	void sub(Reg8 dst, U8 imm);
	void and_(Reg32 dst, U32 imm);
	void and_(Reg16 dst, U16 imm);
	void and_(Reg8 dst, U8 imm);
	void or_(Reg32 dst, U32 imm);
	void or_(Reg16 dst, U16 imm);
	void or_(Reg8 dst, U8 imm);
	void xor_(Reg32 dst, U32 imm);
	void xor_(Reg16 dst, U16 imm);
	void xor_(Reg8 dst, U8 imm);
	void shl(Reg32 dst, U32 imm);
	void shl(Reg16 dst, U16 imm);
	void shl(Reg8 dst, U8 imm);
	void shr(Reg32 dst, U32 imm);
	void shr(Reg16 dst, U16 imm);
	void shr(Reg8 dst, U8 imm);
	void sar(Reg32 dst, U32 imm);
	void sar(Reg16 dst, U16 imm);
	void sar(Reg8 dst, U8 imm);

	void add(Reg32 dst, Reg32 src);
	void add(Reg16 dst, Reg16 src);
	void add(Reg8 dst, Reg8 src);
	void sub(Reg32 dst, Reg32 src);
	void sub(Reg16 dst, Reg16 src);
	void sub(Reg8 dst, Reg8 src);
	void and_(Reg32 dst, Reg32 src);
	void and_(Reg16 dst, Reg16 src);
	void and_(Reg8 dst, Reg8 src);
	void or_(Reg32 dst, Reg32 src);
	void or_(Reg16 dst, Reg16 src);
	void or_(Reg8 dst, Reg8 src);
	void xor_(Reg32 dst, Reg32 src);
	void xor_(Reg16 dst, Reg16 src);
	void xor_(Reg8 dst, Reg8 src);
	void shl(Reg32 dst, Reg32 src);
	void shl(Reg16 dst, Reg16 src);
	void shl(Reg8 dst, Reg8 src);
	void shr(Reg32 dst, Reg32 src);
	void shr(Reg16 dst, Reg16 src);
	void shr(Reg8 dst, Reg8 src);
	void sar(Reg32 dst, Reg32 src);
	void sar(Reg16 dst, Reg16 src);
	void sar(Reg8 dst, Reg8 src);

	void neg(Reg32 dst);
	void neg(Reg16 dst);
	void neg(Reg8 dst);
	void not_(Reg32 dst);
	void not_(Reg16 dst);
	void not_(Reg8 dst);

	void test(Reg32 dst, U32 imm);
	void test(Reg16 dst, U16 imm);
	void test(Reg8 dst, U8 imm);
	void test(Reg32 dst, Reg32 src);
	void test(Reg16 dst, Reg16 src);
	void test(Reg8 dst, Reg8 src);

	void cmp(Reg32 dst, U32 imm);
	void cmp(Reg16 dst, U16 imm);
	void cmp(Reg8 dst, U8 imm);
	void cmp(Reg32 dst, Reg32 src);
	void cmp(Reg16 dst, Reg16 src);
	void cmp(Reg8 dst, Reg8 src);

	void addMem(Reg16 dst, Reg32 rm, U32 disp);
	void addMem(Reg32 dst, Reg32 rm, U32 disp);
	void addMem(Reg32 rm, U32 disp, U32 value);

	void bswap(Reg32 reg);

	void mov(Reg32 dst, U32 imm);
	void mov(Reg16 dst, U16 imm);
	void mov(Reg8 dst, U8 imm);

	void readMem(Reg32 dst, Reg32 rm, U32 disp);
	void readMem(Reg32 dst, Reg32 sib, U8 shift, U32 disp);
	void readMem(Reg32 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp);
	void readMem(Reg16 dst, Reg32 rm, U32 disp);
	void readMem(Reg16 dst, Reg32 sib, U8 shift, U32 disp);
	void readMem(Reg16 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp);
	void readMem(Reg8 dst, Reg32 rm, U32 disp);
	void readMem(Reg8 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp);
	void writeMem(Reg32 rm, U32 disp, Reg32 src);
	void writeMem(Reg32 rm, U32 disp, Reg16 src);
	void writeMem(Reg32 rm, U32 disp, Reg8 src);
	void writeMem(Reg32 rm, U32 disp, U32 src);
	void writeMem(Reg32 rm, U32 disp, U16 src);
	void writeMem(Reg32 rm, U32 disp, U8 src);
	void writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, Reg32 src);
	void writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, Reg16 src);
	void writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, Reg8 src);
	void writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, U32 src);
	void writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, U16 src);
	void writeMem(Reg32 rm, Reg32 sib, U8 shift, U32 disp, U8 src);

	void mov(Reg32 dst, Reg32 src);
	void mov(Reg16 dst, Reg16 src);
	void mov(Reg8 dst, Reg8 src);

	void movzx(Reg16 dst, Reg8 src);
	void movzx(Reg32 dst, Reg8 src);
	void movzx(Reg32 dst, Reg16 src);
	
	void movsx(Reg16 dst, Reg8 src);
	void movsx(Reg32 dst, Reg8 src);
	void movsx(Reg32 dst, Reg16 src);

	void push(Reg32 reg);
	void push(U32 imm);
	void pop(Reg32 reg);
	void call(void* address);
	void ret();
	void jmp(Reg32 reg);

	void setz(Reg8 reg);
	void setnz(Reg8 reg);
	void setb(Reg8 reg);
	void setbe(Reg8 reg);
	void setnb(Reg8 reg);
	void setl(Reg8 reg);
	void setle(Reg8 reg);

	void IfLessThan(Reg32 reg, U32 value);
	void IfEqual(Reg32 reg, U32 value);
	void IfNotEqual(Reg32 reg, U32 value);
	void IfZero(Reg32 reg);
	void IfNotZero(Reg32 reg);
	void IfBitSet(Reg32 reg, U32 mask);
	void Else();
	void EndIf();

	void reset();

	std::vector<U8> buffer;
	std::vector<U32> ifJump;
	std::vector<U32> patch;
private:
	void outd(U32 d);
	void outw(U16 w);
	void outb(U8 b);

	void mem32(U8 inst, Reg32 dst, Reg32 rm, U32 disp);
	void mem32(U8 inst, Reg32 dst, Reg32 rm, U8 shift, U32 disp);
	void mem32(U8 inst, Reg32 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp);
	void mem16(U8 inst, Reg16 dst, Reg32 rm, U32 disp);
	void mem16(U8 inst, Reg16 dst, Reg32 sib, U8 shift, U32 disp);
	void mem16(U8 inst, Reg16 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp);
	void mem8(U8 inst, Reg8 dst, Reg32 rm, U32 disp);
	void mem8(U8 inst, Reg8 dst, Reg32 rm, Reg32 sib, U8 shift, U32 disp);

	void group1(U8 e, U8 math, Reg32 dst, U32 imm);
	void group1(U8 e, U8 math, Reg16 dst, U16 imm);
	void group1(U8 e, U8 math, Reg8 dst, U8 imm);

	void group2(U8 op, Reg32 dst, U32 imm);
	void group2(U8 op, Reg16 dst, U16 imm);
	void group2(U8 op, Reg8 dst, U8 imm);		
};

bool operator==(const X86Asm::Reg32& lhs, const X86Asm::Reg32& rhs);

#endif