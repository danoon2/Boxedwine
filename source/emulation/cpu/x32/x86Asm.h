#ifndef __X86_ASM_H__
#define __X86_ASM_H__

class DynamicJump {
public:
	DynamicJump() = default;
	DynamicJump(U32 eip, U32 bufferPos) : eip(eip), bufferPos(bufferPos) {}
	U32 eip = 0;
	U32 bufferPos = 0;
};

class X86Asm {
public:	
	class RegName {
	public:
		RegName() {}
		static RegName from(U8 reg) {
			RegName result;
			result.reg = reg;
			return result;
		}
		U8 reg = 0xff;
	};

	class Reg8Name : public RegName {
	public:
		Reg8Name() {}
		static Reg8Name from(U8 reg) {
			Reg8Name result;
			result.reg = reg;
			return result;
		}
	};
	class Reg16Name : public RegName {
	public:
		Reg16Name() {}
		static Reg16Name from(U8 reg) {
			Reg16Name result;
			result.reg = reg;
			return result;
		}
	};
	class Reg32Name : public RegName {
	public:
		Reg32Name() {}
		static Reg32Name from(U8 reg) {
			Reg32Name result;
			result.reg = reg;
			return result;
		}
	};	
	class RegXMMName {
	public:
		RegXMMName() {}
		static RegXMMName from(U8 reg) {
			RegXMMName result;
			result.reg = reg;
			return result;
		}
		U8 reg = 0xff;
	private:
		RegXMMName(U8 reg) {}
	};
	class RegMMXName {
	public:
		RegMMXName() {}
		static RegMMXName from(U8 reg) {
			RegMMXName result;
			result.reg = reg;
			return result;
		}
		U8 reg = 0xff;
	private:
		RegMMXName(U8 reg) {}
	};
	typedef RegName Reg;
	typedef Reg8Name Reg8;
	typedef Reg16Name Reg16;
	typedef Reg32Name Reg32;	
	typedef RegXMMName RegXMM;
	typedef RegMMXName RegMMX;

	static RegXMM XMM(U8 reg) {
		return RegXMM::from(reg);
	}
	static RegMMX MMX(U8 reg) {
		return RegMMX::from(reg);
	}

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

#ifdef BOXEDWINE_64
	class Reg64Name : public RegName {
	public:
		Reg64Name() {}
		static Reg64Name from(U8 reg) {
			Reg64Name result;
			result.reg = reg;
			return result;
		}
	};
	typedef Reg64Name Reg64;

	static Reg64 rax;
	static Reg64 rcx;
	static Reg64 rdx;
	static Reg64 rbx;
	static Reg64 rsp;
	static Reg64 rbp;
	static Reg64 rsi;
	static Reg64 rdi;
	static Reg64 r8;
	static Reg64 r9;
	static Reg64 r10;
	static Reg64 r11;
	static Reg64 r12;
	static Reg64 r13;
	static Reg64 r14;
	static Reg64 r15;
#endif

	class Mem {
	public:
		Mem(U32 disp) : lsl(0), disp(disp) {}
		Mem(Reg rm, U32 disp = 0) : rm(rm), lsl(0), disp(disp) {}
		Mem(Reg rm, Reg sib, U8 lsl = 0, U32 disp = 0) : rm(rm), sib(sib), lsl(lsl), disp(disp) {}
		Mem(Reg sib, U8 lsl, U32 disp) : sib(sib), lsl(lsl), disp(disp) {}

		Reg rm;
		Reg sib;
		U8 lsl;
		U32 disp;
	};

	class Mem128 : public Mem {
	public:
		Mem128(U32 disp) : Mem(disp) {}
		Mem128(Reg rm, U32 disp = 0) : Mem(rm, disp) {}
		Mem128(Reg rm, Reg sib, U8 lsl = 0, U32 disp = 0) : Mem(rm, sib, lsl, disp) {}
		Mem128(Reg sib, U8 lsl, U32 disp) : Mem(sib, lsl, disp) {}
	};

	class Mem64 : public Mem {
	public:
		Mem64(U32 disp) : Mem(disp) {}
		Mem64(Reg rm, U32 disp = 0) : Mem(rm, disp) {}
		Mem64(Reg rm, Reg sib, U8 lsl = 0, U32 disp = 0) : Mem(rm, sib, lsl, disp) {}
		Mem64(Reg sib, U8 lsl, U32 disp) : Mem(sib, lsl, disp) {}
	};

	class Mem32 : public Mem {
	public:
		Mem32(U32 disp) : Mem(disp) {}
		Mem32(Reg rm, U32 disp = 0) : Mem(rm, disp) {}
		Mem32(Reg rm, Reg sib, U8 lsl = 0, U32 disp = 0) : Mem(rm, sib, lsl, disp) {}
		Mem32(Reg sib, U8 lsl, U32 disp) : Mem(sib, lsl, disp) {}
	};

	class Mem16 : public Mem {
	public:
		Mem16(U32 disp) : Mem(disp) {}
		Mem16(Reg rm, U32 disp = 0) : Mem(rm, disp) {}
		Mem16(Reg rm, Reg sib, U8 lsl = 0, U32 disp = 0) : Mem(rm, sib, lsl, disp) {}
		Mem16(Reg sib, U8 lsl, U32 disp) : Mem(sib, lsl, disp) {}
	};

	class Mem8 : public Mem {
	public:
		Mem8(U32 disp) : Mem(disp) {}
		Mem8(Reg rm, U32 disp = 0) : Mem(rm, disp) {}
		Mem8(Reg rm, Reg sib, U8 lsl = 0, U32 disp = 0) : Mem(rm, sib, lsl, disp) {}
		Mem8(Reg sib, U8 lsl, U32 disp) : Mem(sib, lsl, disp) {}
	};

	void lea(Reg32 dst, Mem mem);
	void lahf();
	void sahf();
	
	void add(Reg32 dst, U32 imm);
	void add(Reg16 dst, U16 imm);
	void add(Reg8 dst, U8 imm);
	void adc(Reg32 dst, U32 imm);
	void adc(Reg16 dst, U16 imm);
	void adc(Reg8 dst, U8 imm);	
	void sub(Reg32 dst, U32 imm);
	void sub(Reg16 dst, U16 imm);
	void sub(Reg8 dst, U8 imm);
	void sbb(Reg32 dst, U32 imm);
	void sbb(Reg16 dst, U16 imm);
	void sbb(Reg8 dst, U8 imm);
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
	void shld(Reg32 dst, Reg32 src, U32 imm);
	void shld(Reg16 dst, Reg16 src, U16 imm);
	void shrd(Reg32 dst, Reg32 src, U32 imm);
	void shrd(Reg16 dst, Reg16 src, U16 imm);
	void shr(Reg32 dst, U32 imm);
	void shr(Reg16 dst, U16 imm);
	void shr(Reg8 dst, U8 imm);
	void sar(Reg32 dst, U32 imm);
	void sar(Reg16 dst, U16 imm);
	void sar(Reg8 dst, U8 imm);
	void rol(Reg32 dst, U32 imm);
	void rol(Reg16 dst, U16 imm);
	void rol(Reg8 dst, U8 imm);
	void ror(Reg32 dst, U32 imm);
	void ror(Reg16 dst, U16 imm);
	void ror(Reg8 dst, U8 imm);
	void rcl(Reg32 dst, U32 imm);
	void rcl(Reg16 dst, U16 imm);
	void rcl(Reg8 dst, U8 imm);
	void rcr(Reg32 dst, U32 imm);
	void rcr(Reg16 dst, U16 imm);
	void rcr(Reg8 dst, U8 imm);

	void add(Reg32 dst, Reg32 src);
	void add(Reg16 dst, Reg16 src);
	void add(Reg8 dst, Reg8 src);
	void adc(Reg32 dst, Reg32 src);
	void adc(Reg16 dst, Reg16 src);
	void adc(Reg8 dst, Reg8 src);
	void sub(Reg32 dst, Reg32 src);
	void sub(Reg16 dst, Reg16 src);
	void sub(Reg8 dst, Reg8 src);
	void sbb(Reg32 dst, Reg32 src);
	void sbb(Reg16 dst, Reg16 src);
	void sbb(Reg8 dst, Reg8 src);
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
	void shld(Reg16 dst, Reg16 src, Reg16 cl);
	void shld(Reg32 dst, Reg32 src, Reg32 cl);
	void shrd(Reg16 dst, Reg16 src, Reg16 cl);
	void shrd(Reg32 dst, Reg32 src, Reg32 cl);
	void shr(Reg32 dst, Reg32 src);
	void shr(Reg16 dst, Reg16 src);
	void shr(Reg8 dst, Reg8 src);
	void sar(Reg32 dst, Reg32 src);
	void sar(Reg16 dst, Reg16 src);
	void sar(Reg8 dst, Reg8 src);
	void rol(Reg32 dst, Reg32 src);
	void rol(Reg16 dst, Reg16 src);
	void rol(Reg8 dst, Reg8 src);
	void ror(Reg32 dst, Reg32 src);
	void ror(Reg16 dst, Reg16 src);
	void ror(Reg8 dst, Reg8 src);
	void rcl(Reg32 dst, Reg32 src);
	void rcl(Reg16 dst, Reg16 src);
	void rcl(Reg8 dst, Reg8 src);
	void rcr(Reg32 dst, Reg32 src);
	void rcr(Reg16 dst, Reg16 src);
	void rcr(Reg8 dst, Reg8 src);
	void xadd(Reg32 dst, Reg32 src);
	void xadd(Reg16 dst, Reg16 src);
	void xadd(Reg8 dst, Reg8 src);

	void mulEax(Reg32 src);
	void mulAx(Reg16 src);
	void mulAl(Reg8 src);
	void imul(Reg32 dst, Reg32 src);
	void imul(Reg16 dst, Reg16 src);
	void imul(Reg32 dst, U32 imm);
	void imul(Reg32 dst, Reg32 src, U32 imm);
	void imul(Reg16 dst, U16 imm);
	void imul(Reg32 src);
	void imul(Reg16 dst, Reg16 src, U16 imm);
	void imulEax(Reg32 src);
	void imulAx(Reg16 src);
	void imulAl(Reg8 src);
	void mul(Reg32 src);
	void div(Reg32 src);
	void div(Reg16 src);
	void div(Reg8 src);
	void idiv(Reg32 src);
	void idiv(Reg16 src);
	void idiv(Reg8 src);

	void neg(Reg32 dst);
	void neg(Reg16 dst);
	void neg(Reg8 dst);
	void not_(Reg32 dst);
	void not_(Reg16 dst);
	void not_(Reg8 dst);
	void lzcnt(Reg32 dst, Reg32 src);
	void lzcnt(Reg16 dst, Reg16 src);
	void cmovb(Reg32 dst, Reg32 src);
	void cmovl(Reg32 dst, Reg32 src);
	void cmovnl(Reg32 dst, Reg32 src);

	void test(Reg32 dst, U32 imm);
	void test(Reg16 dst, U16 imm);
	void test(Reg8 dst, U8 imm);	
	void test(Reg32 dst, Reg32 src);
	void test(Reg16 dst, Reg16 src);
	void test(Reg8 dst, Reg8 src);
	void test(const Mem32& mem, U32 imm);

	void cmp(Reg32 dst, U32 imm);
	void cmp(Reg16 dst, U16 imm);
	void cmp(Reg8 dst, U8 imm);	
	void cmp(Reg32 dst, Reg32 src);
	void cmp(Reg16 dst, Reg16 src);
	void cmp(Reg8 dst, Reg8 src);
	void cmp(const Mem32& mem, Reg32 reg);
	void cmp(const Mem16& mem, Reg16 reg);
	void cmp(const Mem8& mem, Reg8 reg);
	void cmp(const Mem32& mem, U32 imm);
	void cmp(const Mem16& mem, U16 imm);
	void cmp(const Mem8& mem, U8 imm);

	void bt(Reg32 reg, Reg32 rm);
	void bt(Reg16 reg, Reg16 rm);
	void bt(Reg32 reg, U8 mask);
	void bt(Reg16 reg, U8 mask);
	void bts(Reg32 reg, Reg32 rm);
	void bts(Reg16 reg, Reg16 rm);
	void bts(Reg32 reg, U8 mask);
	void bts(Reg16 reg, U8 mask);
	void btr(Reg32 reg, Reg32 rm);
	void btr(Reg16 reg, Reg16 rm);
	void btr(Reg32 reg, U8 mask);
	void btr(Reg16 reg, U8 mask);
	void btc(Reg32 reg, Reg32 rm);
	void btc(Reg16 reg, Reg16 rm);
	void btc(Reg32 reg, U8 mask);
	void btc(Reg16 reg, U8 mask);
	void bsf(Reg32 reg, Reg32 rm);
	void bsf(Reg16 reg, Reg16 rm);
	void bsr(Reg32 reg, Reg32 rm);
	void bsr(Reg16 reg, Reg16 rm);

	void add(const Mem32& mem, Reg32 reg);
	void add(const Mem16& mem, Reg16 reg);
	void add(const Mem8& mem, Reg8 reg);
	void add(const Mem32& mem, U32 imm);
	void add(const Mem16& mem, U16 imm);
	void add(const Mem8& mem, U8 imm);
	void sub(const Mem32& mem, Reg32 reg);
	void sub(const Mem16& mem, Reg16 reg);
	void sub(const Mem8& mem, Reg8 reg);
	void sub(const Mem32& mem, U32 imm);
	void sub(const Mem16& mem, U16 imm);
	void sub(const Mem8& mem, U8 imm);
	void or_(const Mem32& mem, Reg32 reg);
	void or_(const Mem16& mem, Reg16 reg);
	void or_(const Mem8& mem, Reg8 reg);	
	void or_(const Mem32& mem, U32 imm);
	void or_(const Mem16& mem, U16 imm);
	void or_(const Mem8& mem, U8 imm);
	void and_(const Mem32& mem, Reg32 reg);
	void and_(const Mem16& mem, Reg16 reg);
	void and_(const Mem8& mem, Reg8 reg);
	void and_(const Mem32& mem, U32 value);
	void and_(const Mem16& mem, U16 imm);
	void and_(const Mem8& mem, U8 imm);
	void xor_(const Mem32& mem, Reg32 reg);
	void xor_(const Mem16& mem, Reg16 reg);
	void xor_(const Mem8& mem, Reg8 reg);
	void xor_(const Mem32& mem, U32 value);
	void xor_(const Mem16& mem, U16 imm);
	void xor_(const Mem8& mem, U8 imm);
	void not_(const Mem32& mem);
	void not_(const Mem16& mem);
	void not_(const Mem8& mem);
	void neg(const Mem32& mem);
	void neg(const Mem16& mem);
	void neg(const Mem8& mem);
	void inc(const Mem32& mem);
	void inc(const Mem16& mem);
	void inc(const Mem8& mem);
	void dec(const Mem32& mem);
	void dec(const Mem16& mem);
	void dec(const Mem8& mem);

	void bt(const Mem32& mem, U8 value);
	void bts(const Mem32& mem, U8 value);
	void bts(const Mem16& mem, U8 value);
	void bts(const Mem32& mem, Reg32 value);
	void bts(const Mem16& mem, Reg16 value);
	void btr(const Mem32& mem, U8 value);
	void btr(const Mem16& mem, U8 value);
	void btr(const Mem32& mem, Reg32 value);
	void btr(const Mem16& mem, Reg16 value);
	void btc(const Mem32& mem, U8 value);
	void btc(const Mem16& mem, U8 value);
	void btc(const Mem32& mem, Reg32 value);
	void btc(const Mem16& mem, Reg16 value);

	void bswap(Reg32 reg);
	
	void mov(Reg32 dst, U32 imm);
	void mov(Reg16 dst, U16 imm);
	void mov(Reg8 dst, U8 imm);
	void mov(const Mem64& mem, U64 src);
	void mov(const Mem32& mem, U32 src);
	void mov(const Mem16& mem, U16 src);
	void mov(const Mem8& mem, U8 src);	
	void mov(Reg32 dst, const Mem32& mem);
	void mov(Reg16 dst, const Mem16& mem);
	void mov(Reg8 dst, const Mem8& mem);	
	void mov(const Mem32& mem, Reg32 src);
	void mov(const Mem16& mem, Reg16 src);
	void mov(const Mem8& mem, Reg8 src);	
	void mov(Reg32 dst, Reg32 src);
	void mov(Reg16 dst, Reg16 src);
	void mov(Reg8 dst, Reg8 src);	

	void movzx(Reg16 dst, Reg8 src);
	void movzx(Reg32 dst, Reg8 src);
	void movzx(Reg32 dst, Reg16 src);
	
	void movsx(Reg16 dst, Reg8 src);
	void movsx(Reg32 dst, Reg8 src);
	void movsx(Reg32 dst, Reg16 src);
		
	void push(U32 imm);
	void pushFlags();
#ifdef BOXEDWINE_64
	void IfZero(Reg64 reg);
	void IfNotZero(Reg64 reg);
	void IfNotEqual(Reg64 reg, Reg64 reg2);
	void IfNotEqual(Reg64 reg, U32 value);
	void IfEqual(Reg64 reg, Reg64 reg2);
	void IfEqual(Reg64 reg, U32 value);
	void call(Reg64 reg);
	void mov(Reg64 dst, Reg64 src);
	void mov(const Mem64& mem, Reg64 src);
	void mov(Reg64 dst, const Mem64& mem);
	void mov(Reg64 dst, U64 imm);
	void cmp(Reg64 dst, Reg64 src);
	void cmp(Reg64 dst, U32 imm);
	void test(Reg64 dst, Reg64 src);
	void add(Reg64 dst, U32 imm);
	void sub(Reg64 dst, U32 imm);
	void push(Reg64 reg);
	void pop(Reg64 reg);
	void div(Reg64 src);
	void idiv(Reg64 src);
	void group2(U8 op, Reg64 dst, U8 imm);
	void shl(Reg64 dst, U8 imm);
	void shr(Reg64 dst, U8 imm);
	void shrd(Reg64 dst, Reg64 src, U8 imm);
	void movsx(Reg64 dst, Reg32 src);
	void cqo();
	void shrx(Reg32 dst, Reg32 src);
	void shlx(Reg32 dst, Reg32 src);
	void sarx(Reg32 dst, Reg32 src);
	void imul(Reg64 dst, Reg64 src);
	void imul(Reg64 dst, Reg64 src, U32 imm);
	void lzcnt(Reg64 dst, Reg64 src);
	void neg(Reg64 dst);
	void shl(Reg64 dst, Reg8 src);
	void cmovl(Reg64 dst, Reg64 src);
	void cmovnl(Reg64 dst, Reg64 src);
	void cvtsi2sd(RegXMM dstXMM, Reg64 reg);
	void and_(Reg64 dst, U32 imm);
	void xchg(Reg64 dst, Reg64 src);
#else
	std::vector<U32> patch;
	void call(void* address);
	void call(Reg32 reg);
	void push(Reg32 reg);
	void pop(Reg32 reg);
#endif		
	void ret();
	void jmp(Reg reg);
	void jmp(U32 address);
	void goto8(U32 amount);
	void goto32(U32 amount);
	void movsb_repeat();

	void jo(U32 address);
	void jno(U32 address);
	void jb(U32 address);
	void jnb(U32 address);
	void jz(U32 address);
	void jnz(U32 address);
	void jbe(U32 address);
	void jnbe(U32 address);
	void js(U32 address);
	void jns(U32 address);
	void jp(U32 address);
	void jnp(U32 address);
	void jl(U32 address);
	void jnl(U32 address);
	void jle(U32 address);
	void jnle(U32 address);

	void jb();
	void jnb();
	void jz();
	void jnz();
	void jbe();
	void jnbe();
	void jl();
	void jnl();
	void jle();
	void jnle();

	void setz(Reg8 reg);
	void setnz(Reg8 reg);
	void setb(Reg8 reg);	
	void setnb(Reg8 reg);
	void setbe(Reg8 reg);
	void setnbe(Reg8 reg);
	void setl(Reg8 reg);
	void setnl(Reg8 reg);
	void setle(Reg8 reg);
	void setnle(Reg8 reg);
	void seto(Reg8 reg);
	void setno(Reg8 reg);
	void sets(Reg8 reg);
	void setns(Reg8 reg);
	void setp(Reg8 reg);
	void setnp(Reg8 reg);

	void lock();
	void cmpxchg8b(const Mem64& mem);
	void cmpxchg(const Mem32& mem, Reg32 reg);
	void cmpxchg(const Mem16& mem, Reg16 reg);
	void cmpxchg(const Mem8& mem, Reg8 reg);
	void xchg(Reg32 reg, const Mem32& mem);
	void xchg(Reg16 reg, const Mem16& mem);
	void xchg(Reg8 reg, const Mem8& mem);
	void xchg(Reg8 reg, Reg8 rm);
	void xchg(Reg16 reg, Reg16 rm);
	void xchg(Reg32 reg, Reg32 rm);
	void xadd(const Mem32& mem, Reg32 reg);
	void xadd(const Mem16& mem, Reg16 reg);
	void xadd(const Mem8& mem, Reg8 reg);

	void IfLessThan(Reg32 reg, U32 value);
	void IfLessThan(Reg16 reg, U16 value);
	void IfLessThan(Reg8 reg, U8 value);
	void IfLessThan(Reg32 reg1, Reg32 reg2);
	void IfLessThan(Reg16 reg1, Reg16 reg2);
	void IfLessThan(Reg8 reg1, Reg8 reg2);
	void IfEqual(Reg32 reg, U32 value);
	void IfEqual(Reg16 reg, U16 value);
	void IfEqual(Reg8 reg, U8 value);	
	void IfEqual(Reg32 reg1, Reg32 reg2);
	void IfEqual(Reg16 reg1, Reg16 reg2);
	void IfEqual(Reg8 reg1, Reg8 reg2);
	void IfNotEqual(Reg32 reg, U32 value);
	void IfNotEqual(Reg16 reg, U16 value);
	void IfNotEqual(Reg8 reg, U8 value);	
	void IfNotEqual(Reg32 reg, Reg32 reg2);
	void IfNotEqual(Reg16 reg, Reg16 reg2);
	void IfNotEqual(Reg8 reg, Reg8 reg2);
	void IfZero(Reg32 reg);
	void IfZero(Reg16 reg);
	void IfZero(Reg8 reg);	
	void IfNotZero(Reg32 reg);
	void IfNotZero(Reg16 reg);
	void IfNotZero(Reg8 reg);
	void Else();
	void EndIf();
	void IfPF();
	void IfCF();
	void IfZF();
	void IfSF();
	void IfOF();
	void IfNotZF();

	void reset();

	void stmxcsr(const Mem32& mem);
	void ldmxcsr(const Mem32& mem);
	void rdtsc();

	void movlhps(RegXMM hiDstXMM, RegXMM loSrcXMM);
	void movhlps(RegXMM hiDstXMM, RegXMM loSrcXMM);

	void movsd(const Mem64& mem, RegXMM srcXMM);
	void movsd(RegXMM dstXMM, const Mem64& mem); // 0's out high 64-bit
	void movsd(RegXMM dstXMM, RegXMM srcXMM); // copies low 64-bit and preserves high 64-bit
	void movss(const Mem32& mem, RegXMM srcXMM);	
	void movss(RegXMM dstXMM, const Mem32& mem);
	void movss(RegXMM dstXMM, RegXMM srcXMM); // copies low 32-bit and preserves high 96-bit
	void movlpd(RegXMM dstXMM, const Mem64& mem);

	void cvtss2sd(RegXMM dstXMM, RegXMM srcXMM);
	void cvtsd2ss(RegXMM dstXMM, RegXMM srcXMM);
	void cvtsi2sd(RegXMM dstXMM, const Mem32& mem);
	void cvtsi2sd(RegXMM dstXMM, Reg32 reg);
	void cvttsd2si(Reg32 dst, RegXMM srcXMM);
	void cvtsd2si(Reg32 dst, RegXMM srcXMM);
	void cvtpd2dq(RegXMM dstXMM, RegXMM srcXMM);
	void cvttpd2dq(RegXMM dstXMM, RegXMM srcXMM);
	void cvtpd2ps(RegXMM dstXMM, RegXMM srcXMM);
	void cvtdq2pd(RegXMM dstXMM, RegXMM srcXMM);
	void xorpd(RegXMM dstXMM, RegXMM srcXMM);
	void andpd(RegXMM dstXMM, RegXMM srcXMM);
	void addpd(RegXMM dstXMM, RegXMM srcXMM);
	void addsd(RegXMM dstXMM, RegXMM srcXMM);
	void mulpd(RegXMM dstXMM, RegXMM srcXMM);
	void mulsd(RegXMM dstXMM, RegXMM srcXMM);
	void subpd(RegXMM dstXMM, RegXMM srcXMM);
	void subsd(RegXMM dstXMM, RegXMM srcXMM);
	void sqrtpd(RegXMM dstXMM, RegXMM srcXMM);
	void sqrtsd(RegXMM dstXMM, RegXMM srcXMM);	

	void movd(RegMMX dst, Reg32 src);
	void movd(Reg32 dst, RegMMX src);
	void movd(const Mem32& mem, RegMMX reg);
	void movd(RegMMX reg, const Mem32& mem);

	void emms();
	void movaps(RegXMM dst, RegXMM src);
	void movaps(RegXMM reg, const Mem128& mem);
	void movaps(const Mem128& mem, RegXMM reg);
	void movups(RegXMM reg, const Mem128& mem);
	void movups(const Mem128& mem, RegXMM reg);
	void movhps(const Mem64& mem, RegXMM reg);
	void movhps(RegXMM reg, const Mem64& mem);
	void movlps(const Mem64& mem, RegXMM reg);
	void movlps(RegXMM reg, const Mem64& mem);

	void movd(RegXMM dst, Reg32 src);
	void movd(Reg32 dst, RegXMM src);
	void movd(RegXMM reg, const Mem32& mem);
	void movd(const Mem32& mem, RegXMM reg);
	void movq(RegXMM reg, const Mem64& mem);
	void movq(const Mem64& mem, RegXMM reg);
	void movq(RegXMM dst, RegXMM src);
	void movdqu(RegXMM dst, RegXMM src);
	void pxor(RegXMM dst, RegXMM src);
	void por(RegXMM dst, RegXMM src);
	void pand(RegXMM dst, RegXMM src);
	void pandn(RegXMM dst, RegXMM src);
	void psllw(RegXMM dst, RegXMM src);
	void psllw(RegXMM dst, U32 imm);
	void psrlw(RegXMM dst, RegXMM src);
	void psrlw(RegXMM dst, U32 imm);
	void psraw(RegXMM dst, RegXMM src);
	void psraw(RegXMM dst, U32 src);
	void pslld(RegXMM dst, RegXMM src);
	void pslld(RegXMM dst, U32 imm);
	void psrld(RegXMM dst, RegXMM src);
	void psrld(RegXMM dst, U32 imm);
	void psrad(RegXMM dst, RegXMM src);
	void psrad(RegXMM dst, U32 src);
	void psllq(RegXMM dst, RegXMM src);
	void psllq(RegXMM dst, U32 imm);
	void pslldq(RegXMM dst, U32 imm);
	void psrlq(RegXMM dst, RegXMM src);
	void psrlq(RegXMM dst, U32 imm);
	void psrldq(RegXMM dst, U32 imm);

	void paddb(RegXMM dst, RegXMM src);
	void paddw(RegXMM dst, RegXMM src);
	void paddd(RegXMM dst, RegXMM src);
	void paddq(RegXMM dst, RegXMM src);
	void paddsb(RegXMM dst, RegXMM src);
	void paddsw(RegXMM dst, RegXMM src);
	void paddusb(RegXMM dst, RegXMM src);
	void paddusw(RegXMM dst, RegXMM src);

	void psubb(RegXMM dst, RegXMM src);
	void psubw(RegXMM dst, RegXMM src);
	void psubd(RegXMM dst, RegXMM src);
	void psubq(RegXMM dst, RegXMM src);
	void psubsb(RegXMM dst, RegXMM src);
	void psubsw(RegXMM dst, RegXMM src);
	void psubusb(RegXMM dst, RegXMM src);
	void psubusw(RegXMM dst, RegXMM src);

	void pmulhw(RegXMM dst, RegXMM src);
	void pmullw(RegXMM dst, RegXMM src);
	void pmaddwd(RegXMM dst, RegXMM src);
	void pmuludq(RegXMM dst, RegXMM src);
	
	void pcmpeqb(RegXMM dst, RegXMM src);
	void pcmpeqw(RegXMM dst, RegXMM src);
	void pcmpeqd(RegXMM dst, RegXMM src);
	void pcmpgtb(RegXMM dst, RegXMM src);
	void pcmpgtw(RegXMM dst, RegXMM src);
	void pcmpgtd(RegXMM dst, RegXMM src);

	void packsswb(RegXMM dst, RegXMM src);
	void packssdw(RegXMM dst, RegXMM src);
	void packuswb(RegXMM dst, RegXMM src);
	void punpckhbw(RegXMM dst, RegXMM src);
	void punpckhwd(RegXMM dst, RegXMM src);
	void punpckhdq(RegXMM dst, RegXMM src);
	void punpcklbw(RegXMM dst, RegXMM src);
	void punpcklwd(RegXMM dst, RegXMM src);
	void punpckldq(RegXMM dst, RegXMM src);
	void unpcklpd(RegXMM dst, RegXMM src);
	void unpckhpd(RegXMM dst, RegXMM src);
	void punpckhqdq(RegXMM dst, RegXMM src);
	void punpcklqdq(RegXMM dst, RegXMM src);

	void addps(RegXMM dst, RegXMM src);
	void addss(RegXMM dst, RegXMM src);
	void subps(RegXMM dst, RegXMM src);
	void subss(RegXMM dst, RegXMM src);
	void mulps(RegXMM dst, RegXMM src);
	void mulss(RegXMM dst, RegXMM src);
	void divps(RegXMM dst, RegXMM src);
	void divpd(RegXMM dst, RegXMM src);
	void divss(RegXMM dst, RegXMM src);
	void divsd(RegXMM dst, RegXMM src);
	void rcpps(RegXMM dst, RegXMM src);
	void rcpss(RegXMM dst, RegXMM src);
	void sqrtps(RegXMM dst, RegXMM src);
	void sqrtss(RegXMM dst, RegXMM src);
	void rsqrtps(RegXMM dst, RegXMM src);
	void rsqrtss(RegXMM dst, RegXMM src);
	void maxps(RegXMM dst, RegXMM src);
	void maxpd(RegXMM dst, RegXMM src);
	void maxss(RegXMM dst, RegXMM src);
	void maxsd(RegXMM dst, RegXMM src);
	void minps(RegXMM dst, RegXMM src);
	void minpd(RegXMM dst, RegXMM src);
	void minss(RegXMM dst, RegXMM src);
	void minsd(RegXMM dst, RegXMM src);
	void pavgb(RegXMM dst, RegXMM src);
	void pavgw(RegXMM dst, RegXMM src);
	void psadbw(RegXMM dst, RegXMM src);
	void pextrw(Reg32 dst, RegXMM src, U8 srcIndex);
	void pinsrw(RegXMM dst, Reg32 src, U8 dstIndex);
	void pmaxsw(RegXMM dst, RegXMM src);
	void pmaxub(RegXMM dst, RegXMM src);
	void pminsw(RegXMM dst, RegXMM src);
	void pminub(RegXMM dst, RegXMM src);
	void pmovmskb(Reg32 dst, RegXMM src);
	void pmulhuw(RegXMM dst, RegXMM src);
	void pshuflw(RegXMM dst, RegXMM src, U8 order);
	void pshufhw(RegXMM dst, RegXMM src, U8 order);
	void pshufd(RegXMM dst, RegXMM src, U8 order);
	void shufpd(RegXMM dst, RegXMM src, U8 order);
	void andnpd(RegXMM dst, RegXMM src);
	void andnps(RegXMM dst, RegXMM src);
	void andps(RegXMM dst, RegXMM src);
	void orpd(RegXMM dst, RegXMM src);
	void orps(RegXMM dst, RegXMM src);
	void xorps(RegXMM dst, RegXMM src);
	void cvtdq2ps(RegXMM dst, RegXMM src);
	void cvtps2dq(RegXMM dst, RegXMM src);
	void cvtps2pd(RegXMM dst, RegXMM src);
	void cvttps2dq(RegXMM dst, RegXMM src);
	void cvtsi2ss(RegXMM dst, Reg32 src);
	void cvtss2si(Reg32 dst, RegXMM src);
	void cvttss2si(Reg32 dst, RegXMM src);
	void movmskps(Reg32 dst, RegXMM src);
	void maskmovdqu(RegXMM src, RegXMM mask);
	void shufps(RegXMM src, RegXMM mask, U8 imm);
	void cmppd(RegXMM src, RegXMM mask, U8 imm);
	void cmpps(RegXMM src, RegXMM mask, U8 imm);
	void cmpsd(RegXMM src, RegXMM mask, U8 imm);
	void cmpss(RegXMM src, RegXMM mask, U8 imm);
	void unpckhps(RegXMM dst, RegXMM src);
	void unpcklps(RegXMM dst, RegXMM src);
	void comisd(RegXMM dst, RegXMM src);
	void comiss(RegXMM dst, RegXMM src);
	void ucomisd(RegXMM xmm1, RegXMM xmm2);
	void ucomiss(RegXMM dst, RegXMM src);
	void sfence();
	void lfence();
	void pause();
	void mfence();
	void clflush(const Mem8& mem);
	void movmskpd(Reg32 dst, RegXMM src);

	std::vector<U8> buffer;
	std::vector<U32> ifJump;	
	std::vector<DynamicJump> jumps;

	void rexBase();
private:
	void outq(U64 d);
	void outd(U32 d);
	void outw(U16 w);
	void outb(U8 b);

	void rex(U8 reg, U8 rm, U8 sib, bool is64);
	void rex(U8 reg, U8 rm, bool is64);
	void rex(U8 reg, bool is64);

	void mem32(U32 inst, U8 dst, const Mem& mem, bool is64 = false);
	void mem16(U32 inst, U8 dst, const Mem& mem);
	void mem8(U32 inst, U8 dst, const Mem& mem);

	void group1(U8 e, U8 math, Reg32 dst, U32 imm);
	void group1(U8 e, U8 math, Reg16 dst, U16 imm);
	void group1(U8 e, U8 math, Reg8 dst, U8 imm);

	void group2(U8 op, Reg32 dst, U32 imm);
	void group2(U8 op, Reg16 dst, U16 imm);
	void group2(U8 op, Reg8 dst, U8 imm);

	void IfJump(U8 inst);
};

bool operator==(const X86Asm::Reg32& lhs, const X86Asm::Reg32& rhs);

#endif